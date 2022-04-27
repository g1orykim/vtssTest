/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

// Avoid "board_probe.h not used in module port_custom_jr1.c"
/*lint --e{766} */
#include "board_probe.h"

#if defined(VTSS_ARCH_JAGUAR_1)
#if defined(VTSS_SW_OPTION_PHY) && defined(VTSS_CHIP_10G_PHY)
#include "phy_api.h" // For PHY_INST
#endif
#include "port_custom_jr1_api.h"
#if 0
#include <cyg/infra/diag.h>
#define PRINTD(args) { diag_printf("%s: ", __FUNCTION__); diag_printf args; diag_printf("\n"); }
#else
#define PRINTD(args) 
#endif

static int board_type;
static vtss_i2c_read_t    i2c_read;     /**< I2C read function */
/*lint -esym(459, i2c_write) Ok - Not to protect i2c_write with semaphore in this file*/
static vtss_i2c_write_t   i2c_write;    /**< I2C write function */

/* For LynX-1/CE-MaX-12, chip ports and MII addresses are offset by 11 */
#if defined(VTSS_CHIP_LYNX_1) || defined(VTSS_CHIP_CE_MAX_12)
#define BOARD_PORT(port) (port + 11)
#else
#define BOARD_PORT(port) (port)
#endif /* VTSS_CHIP_SPARX_II_16 */

static u32 chip_count = 1;
static u32 jr1_port_count;
vtss_port_no_t port_tesla = VTSS_PORT_NO_NONE;


/* SGPIO LED mapping */
typedef struct {
    u8  port;
    u8  bit;
} sgpio_mapping_t;

/* LED colors */
typedef enum {
    LED_GREEN,
    LED_YELLOW,
    LED_MAX
} led_color_t;

static const sgpio_mapping_t tower_led_mapping[4][LED_MAX] = {
    {{24, 0} /* tower 0 green */, {24, 1} /* tower 0 yellow */},
    {{25, 0} /* tower 1 green */, {25, 1} /* tower 1 yellow */},
    {{26, 0} /* tower 2 green */, {26, 1} /* tower 2 yellow */},
    {{31, 0} /* tower 3 green */, {31, 1} /* tower 3 yellow */}
};

static sgpio_mapping_t port_led_mapping[VTSS_PORT_ARRAY_SIZE][LED_MAX];

static void jr1_led_tower_update(BOOL restore)
{
    vtss_sgpio_conf_t conf;
    int               i;

    /*
     * LED tower:
     *  (top)       o  mode A (link/speed)
     *              o  mode B (link/duplex)
     *              o  mode C (link/status)
     *  (button)    o  PWR save
     */

    /*lint -esym(459, jr1_tower_mode) */
    jr1_tower_mode = (restore ? LED_TOWER_MODE_LINK_SPEED : 
                      ((jr1_tower_mode + 1) % LED_TOWER_MODE_CNT));

    if (vtss_sgpio_conf_get(NULL, 0, 1, &conf) != VTSS_RC_OK) {
        return;
    }

    for (i = 0; i < 4; i++) {
        conf.port_conf[tower_led_mapping[i][0].port].mode[tower_led_mapping[i][0].bit] = 
            ((i == 0 && jr1_tower_mode == LED_TOWER_MODE_LINK_SPEED) ||
             (i == 1 && jr1_tower_mode == LED_TOWER_MODE_DUPLEX) ||
             (i == 2 && jr1_tower_mode == LED_TOWER_MODE_LINK_STATUS) ||
             (i == 3 && jr1_tower_mode == LED_TOWER_MODE_POWER_SAVE) ? VTSS_SGPIO_MODE_ON :
             VTSS_SGPIO_MODE_OFF);
        conf.port_conf[tower_led_mapping[i][1].port].mode[tower_led_mapping[i][1].bit] = 
            VTSS_SGPIO_MODE_OFF;
    }
    (void)vtss_sgpio_conf_set(NULL, 0, 1, &conf);
}

static void port_custom_sgpio_led_set(vtss_port_no_t port_no,
                                      led_color_t color,
                                      vtss_sgpio_mode_t mode,
                                      vtss_sgpio_conf_t *conf)
{
    if(port_no < VTSS_PORT_NO_END && color < LED_MAX) {
        const sgpio_mapping_t *led = &port_led_mapping[port_no][color];
        conf->port_conf[led->port].mode[led->bit] = mode;
    }
}

/* Release ports from reset */
static vtss_rc jr1_reset(void)
{
    vtss_chip_no_t chip_no;
    vtss_rc rc = VTSS_RC_OK;

    for (chip_no = 0; chip_no < chip_count; chip_no++) {
        /* Release PHYs from reset (Switch GPIO12) */
        (void)vtss_gpio_direction_set(NULL, chip_no, 12, 1);
        (void)vtss_gpio_write(NULL, chip_no, 12, 0);
        (void)vtss_gpio_write(NULL, chip_no, 12, 1);
    }
    VTSS_MSLEEP(500);

    if (board_type == VTSS_BOARD_JAG_PCB107_REF) {
      rc |= vtss_phy_pre_reset(NULL, 0);
      rc |= vtss_phy_pre_reset(NULL, 4);
      rc |= vtss_phy_pre_reset(NULL, 8);
      rc |= vtss_phy_pre_reset(NULL, 12);
      rc |= vtss_phy_pre_reset(NULL, 16);
      rc |= vtss_phy_pre_reset(NULL, 20);
    } else {
      if (port_tesla != VTSS_PORT_NO_NONE) {
        rc |= vtss_phy_pre_reset(NULL, port_tesla);
      }
    }
    return rc;
}

/* Post ports reset */
static vtss_rc jr1_post_reset(void)
{
  vtss_rc rc = VTSS_RC_OK;
  if (board_type == VTSS_BOARD_JAG_PCB107_REF) {
    rc |= vtss_phy_post_reset(NULL, 0);
    rc |= vtss_phy_post_reset(NULL, 4);
    rc |= vtss_phy_post_reset(NULL, 8);
    rc |= vtss_phy_post_reset(NULL, 12);
    rc |= vtss_phy_post_reset(NULL, 16);
    rc |= vtss_phy_post_reset(NULL, 20);
  } else {
    // If the board has a Tesla daughter module the release coma mode for that board.
    if (port_tesla != VTSS_PORT_NO_NONE) {
      rc |= vtss_phy_post_reset(NULL, port_tesla);
    }
  }
  return rc;
}


/* Initialize port LEDs */
static vtss_rc jr1_led_init(void)
{
    /* Initial LED tower */
    jr1_led_tower_update(1);

    return VTSS_RC_OK;
}

/* Update port LED */
static vtss_rc jr1_led_update(vtss_port_no_t port_no,
                              vtss_port_status_t *status,
                              vtss_port_counters_t *counters,
                              port_custom_conf_t *port_conf)
{
    /*lint -esym(459, val, timer, jr1_port_table, jr1_port_info_table, port_led_mapping) */
    static BOOL         val = 0;
    static u32          timer = 0;
    port_custom_entry_t *port_entry = &jr1_port_table[port_no];
    jr1_port_info_t     *port_info = &jr1_port_info_table[port_no];
    vtss_port_status_t  *old_status = &port_info->status;
    vtss_port_counter_t cnt = 0;
    int                 i;
    BOOL                old_val, restore, update = 0, activity = 0;
    vtss_port_no_t      port_start = port_no, port_end = port_no + 1;
    vtss_sgpio_conf_t   conf;
    vtss_sgpio_mode_t   mode_green, mode_yellow;
    vtss_rc             rc = VTSS_RC_OK;
    vtss_chip_no_t      chip_no;
    
    /* NPI port status LED is controlled by the VSC8221 single-port NPI */
    if (port_entry->map.chip_port == JR1_NPI_CHIP_PORT)
        return rc;

    /* Check port status has changed */
    if (old_status->link != status->link ||
        old_status->speed != status->speed || 
        old_status->fdx != status->fdx) {
        *old_status = *status;
        update = 1;
    }

    /* For 10G ports, check if activity has been detected */
    if (port_entry->cap & PORT_CAP_10G_FDX) {
        /* Priority counters do not include super priority frames (CMEF, SPROUT, MSG).
           We use these counters to show only real data activity on stack ports */
        for (i = 0; i < VTSS_PRIOS; i++)
            cnt += (counters->prop.rx_prio[i] + counters->prop.tx_prio[i]);
        activity = (cnt > port_info->pkt_cnt ? 1 : 0);
        if (activity != port_info->activity && 
            (jr1_tower_mode == LED_TOWER_MODE_LINK_SPEED || 
             jr1_tower_mode == LED_TOWER_MODE_LINK_STATUS))
            update = 1;
        port_info->pkt_cnt = cnt;
        port_info->activity = activity;
    }

    /* For HDX capable ports, check if collisions have been detected */
    if (port_entry->cap & PORT_CAP_HDX) {
        cnt = counters->rmon.tx_etherStatsCollisions;
        activity = (cnt > port_info->col_cnt ? 1 : 0);
        if (activity != port_info->collisions && jr1_tower_mode == LED_TOWER_MODE_DUPLEX)
            update = 1;
        port_info->col_cnt = cnt;
        port_info->collisions = activity;
    }

    /* Check if Rx errors have been detected */
    cnt = counters->if_group.ifInErrors;
    activity = (cnt > port_info->err_cnt ? 1 : 0);
    if (activity != port_info->errors && jr1_tower_mode == LED_TOWER_MODE_LINK_STATUS)
        update = 1;
    port_info->err_cnt = cnt;
    port_info->errors = activity;

    /* Check if 30 seconds in non-default mode */
    if (port_no == 0 && jr1_tower_mode != LED_TOWER_MODE_LINK_SPEED)
        timer++;
    restore = (timer > 30 ? 1 : 0);

    /* Check if button has been pushed (GPIO_13 controls LED tower) */
    old_val = val;
    if (restore || 
        (vtss_gpio_read(NULL, 0, 13, &val) == VTSS_RC_OK && old_val == 0 && val != 0)) {
        /* LED mode changed, update all ports, ignoring collisions and errors */
        jr1_led_tower_update(restore);
        timer = 0;
        port_start = VTSS_PORT_NO_START;
        port_end = jr1_port_count;
        update = 1;
    } else if (jr1_tower_mode == LED_TOWER_MODE_POWER_SAVE)
        update = 0; /* No further processing in power saving mode */

    /* Return if no update must be done */
    if (!update)
        return rc;

    /* Update port LEDs */
    for (chip_no = 0; chip_no < chip_count; chip_no++) {
        if (vtss_sgpio_conf_get(NULL, chip_no, 1, &conf) != VTSS_RC_OK)
            continue;
        
        for (port_no = port_start; port_no < port_end; port_no++) {
            port_entry = &jr1_port_table[port_no];
            port_info = &jr1_port_info_table[port_no];
            status = &port_info->status;
            
            /* Skip ports on the other chip */
            if (port_entry->map.chip_no != chip_no)
                continue;

            /* NPI port status LED is controlled by the VSC8221 single-port NPI */
            if (port_entry->map.chip_port == JR1_NPI_CHIP_PORT)
                continue;
            
            mode_green = VTSS_SGPIO_MODE_OFF;
            mode_yellow = VTSS_SGPIO_MODE_OFF;
        
            switch (jr1_tower_mode) {
            case LED_TOWER_MODE_LINK_SPEED:
                if (status->link) {
                    if (port_entry->cap & PORT_CAP_10G_FDX) {
                        mode_green = (port_info->activity ? VTSS_SGPIO_MODE_0 : 
                                      VTSS_SGPIO_MODE_ON);
                    } else if (status->speed >= VTSS_SPEED_1G) {
                        mode_green = VTSS_SGPIO_MODE_0_ACTIVITY_INV;
                    } else {
                        mode_yellow = VTSS_SGPIO_MODE_0_ACTIVITY_INV;
                    }
                }
                break;
            case LED_TOWER_MODE_DUPLEX:
                if (status->link) {
                    if (status->fdx) {
                        mode_green = VTSS_SGPIO_MODE_ON;
                    } else {
                        mode_yellow = (port_info->collisions ? VTSS_SGPIO_MODE_0 : 
                                       VTSS_SGPIO_MODE_ON);
                    }
                }
                break;
            case LED_TOWER_MODE_LINK_STATUS:
                if (status->link) {
                    if (port_info->errors) {
                        mode_yellow = VTSS_SGPIO_MODE_0;
                    } else if (port_entry->cap & PORT_CAP_10G_FDX) {
                        mode_green = (port_info->activity ? VTSS_SGPIO_MODE_0 : 
                                      VTSS_SGPIO_MODE_ON);
                    } else { 
                        mode_green = VTSS_SGPIO_MODE_0_ACTIVITY_INV;
                    }
                } else if (!port_info->enabled) {
                    mode_yellow = VTSS_SGPIO_MODE_ON;
                }
                break;
            default:
                break;
            }
            port_custom_sgpio_led_set(port_no, LED_GREEN, mode_green, &conf);
            port_custom_sgpio_led_set(port_no, LED_YELLOW, mode_yellow, &conf);
        }
        rc = vtss_sgpio_conf_set(NULL, chip_no, 1, &conf);
    }
    return rc;
}

// jr1_pre_reset(), called before system reset.
// Disables the SGPIO controller and waits the required amount of time for it to settle.
// The potential problem is that: When resetting the SIO controller (and GPIOs)
// when the chain is only partially shifted out is that; if the shift registers on
// the PCB latches the data during reset (i.e. GPIO latch / clk is tristating and
// then being pulled to an active value) then all sorts of bad things can happen
//(for example a customers could use serial GPIO controlling board reset).
static void jr1_pre_reset(void)
{
    vtss_chip_no_t    chip_no;
    vtss_gpio_no_t    gpio;
    vtss_sgpio_conf_t conf;
    u32               port;

    for (chip_no = 0; chip_no < chip_count; chip_no++) {
        /* Turn off all LEDs */
        if (vtss_sgpio_conf_get(NULL, chip_no, 1, &conf) == VTSS_RC_OK) {
            for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
                port_custom_sgpio_led_set(port, LED_GREEN, VTSS_SGPIO_MODE_OFF, &conf);
                port_custom_sgpio_led_set(port, LED_YELLOW, VTSS_SGPIO_MODE_OFF, &conf);
            }
            (void)vtss_sgpio_conf_set(NULL, chip_no, 1, &conf);
            VTSS_MSLEEP(100); /* Waits a period time */
        }

        // Disable SGPIO interrupts in order not to get interrupts during the reboot.
        for (port = 0; port < VTSS_SGPIO_PORTS; port++) {
          u8 bit;
          u8 group = 0;
          for (bit = 0; bit <= conf.bit_count; bit ++) {
            (void) vtss_sgpio_event_enable(NULL, chip_no, group, port, bit, FALSE);
            (void) vtss_sgpio_event_enable(NULL, chip_no, group, port, bit, FALSE);
          }
        }

        /* Disable SGPIO controller (GPIO 0-3 for SGPIO, GPIO 4-5 for LED) */
        for (gpio = 0; gpio < 6; gpio++) {
          (void)vtss_gpio_mode_set(NULL, chip_no, gpio, VTSS_GPIO_IN);
        }

        VTSS_MSLEEP(10); /* Waits a period time */
    }
}


// Function for doing SFP i2c reads. 
// In : port_no - The physical port number
//      i2c_addr - The address for the i2c device
//      addr     - The address with the i2c device to access.
//      cnt      - The number of data to read
// In/Out  data     - The data read
static vtss_rc jr1_sfp_i2c_read(vtss_port_no_t port_no, u8 i2c_addr, u8 addr, u8 *const data, u8 cnt) 
{
    if (i2c_read != NULL) {
        /* Custom board SFP i2c enable function */
        if (board_sfp_i2c_enable(port_no) != VTSS_RC_OK) { 
            return VTSS_RC_ERROR;
        }
        
        return i2c_read(port_no, i2c_addr, addr, data, cnt, (-1));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr1_sfp_i2c_write(vtss_port_no_t port_no, u8 i2c_addr, u8 addr, u8 *const data)
{
    u8 i2c_data[3];
    if (i2c_write != NULL) {
        /* Custom board SFP i2c enable function */
        if (board_sfp_i2c_enable(port_no) != VTSS_RC_OK) { 
            return VTSS_RC_ERROR;
        }
     
        i2c_data[0] = addr;
        memcpy(&i2c_data[1], data, 2);
        return i2c_write(port_no, i2c_addr, i2c_data, 3, (-1));
    }

    return VTSS_RC_OK;
}
/* All SFPs have the same i2c address. One is enabled at a time through SGPIOs */
static vtss_rc jr1_sfp_i2c_enable(vtss_port_no_t port_no)
{
    vtss_rc                rc = VTSS_RC_ERROR;
    vtss_sgpio_conf_t      conf;
    vtss_sgpio_port_conf_t *port_conf;
    u32                    x;

    if (board_type == VTSS_BOARD_JAG_SFP24_REF && (port_no < 24) && 
        (rc = vtss_sgpio_conf_get(NULL, 0, 0, &conf)) == VTSS_RC_OK) {
        /*  OFF means enabled */
        port_conf = &conf.port_conf[16];
        port_conf->mode[0] = VTSS_SGPIO_MODE_ON;
        port_conf->mode[1] = VTSS_SGPIO_MODE_ON;
        port_conf->mode[2] = VTSS_SGPIO_MODE_ON;
        port_conf->mode[port_no / 8] = VTSS_SGPIO_MODE_OFF;
        
        port_conf = &conf.port_conf[17];
        x = (port_no % 8);
        port_conf->mode[0] = (x % 2 ? VTSS_SGPIO_MODE_ON : VTSS_SGPIO_MODE_OFF); /* 1/3/5/7 */
        port_conf->mode[1] = (x & 2 ? VTSS_SGPIO_MODE_ON : VTSS_SGPIO_MODE_OFF); /* 2/3/6/7 */
        port_conf->mode[2] = (x > 3 ? VTSS_SGPIO_MODE_ON : VTSS_SGPIO_MODE_OFF); /* 4/5/6/7 */
        
        rc = vtss_sgpio_conf_set(NULL, 0, 0, &conf);
        VTSS_MSLEEP(50);
    }
    return rc;
}

/* Update the capabilities for SFP interfaces */
static void jr1_sfp_update_if(vtss_port_no_t port_no, vtss_port_interface_t mac_if)
{
    jr1_port_table[port_no].mac_if = mac_if;
}

static vtss_rc jr1_sfp_mod_detect(BOOL *detect_status)
{
    vtss_sgpio_port_data_t   data[VTSS_SGPIO_PORTS];
    vtss_port_no_t port_no;
   
    if (vtss_sgpio_read(NULL, 0, 0, data) != VTSS_RC_OK)
        return VTSS_RC_ERROR;

    /* one-to-one mapping between API numbers and SGPIO numbers */
    for (port_no = VTSS_PORT_NO_START; (port_no < VTSS_PORT_NO_END) && ((port_no - VTSS_PORT_NO_START) < VTSS_SGPIO_PORTS); port_no++) {
        detect_status[port_no] = data[port_no - VTSS_PORT_NO_START].value[1] ? 0 : 1;  /* Bit one is mod_detect, 0=detect */
    }

    return VTSS_RC_OK;
}

static void jr1_init_chip(vtss_chip_no_t chip_no)
{

    vtss_gpio_no_t     gpio;
    vtss_sgpio_group_t group = 1;
    vtss_sgpio_conf_t  conf;
    u32                port;
    u32                bit;
    vtss_sgpio_port_data_t data[VTSS_SGPIO_PORTS];
    
    /* GPIO pins 0-5 are used for SGPIOs. */
    for (gpio = 0; gpio < 6; gpio++) {
        (void)vtss_gpio_mode_set(NULL, chip_no, gpio, VTSS_GPIO_ALT_0);
    }

    /* Setup SGPIO group 1 */
    if (vtss_sgpio_conf_get(NULL, chip_no, group, &conf) == VTSS_RC_OK) {
        /* The blink mode 0 is 5 HZ for link activity and collion in half duplex. */
        conf.bmode[0] = VTSS_SGPIO_BMODE_5;
        conf.bit_count = 2;
        for (port = 0; port < VTSS_SGPIO_PORTS; port++) {
            conf.port_conf[port].enabled = 1;
        }
        (void)vtss_sgpio_conf_set(NULL, chip_no, group, &conf);
    }
    
    /* Setup SGPIO group 0 */
    group = 0;
    if (vtss_sgpio_conf_get(NULL, chip_no, group, &conf) != VTSS_RC_OK)
        return;
    conf.bmode[0] = VTSS_SGPIO_BMODE_5;
    /* The API default is that all SGPIO ports are disabled and all modes are OFF */

    switch (board_type) {
    case VTSS_BOARD_JAG_SFP24_REF:
        /* 0-15: SFPs (off-on to reset), 18-19 FANS (on), 20: X2s (on) */
        conf.bit_count = 3;
        for (port = 0; port < VTSS_SGPIO_PORTS; port++) {
            conf.port_conf[port].enabled = 1;
            for (bit = 0; bit <= conf.bit_count; bit++) {
              conf.port_conf[port].int_pol_high[bit] = TRUE; // LOS of signal for VTSS_BOARD_JAG_SFP24_REF is active high.  
            }
        }
        (void)vtss_sgpio_conf_set(NULL, chip_no, group, &conf);
        
        /* Wait before releasing SFP transceivers from reset */
        VTSS_MSLEEP(200);
        /* SFP Tx_disable=enabled and RateSel enabled */
        for (port = 0; port < 16; port++)  {
            conf.port_conf[port].mode[0] = VTSS_SGPIO_MODE_ON;
            conf.port_conf[port].mode[1] = VTSS_SGPIO_MODE_ON;
            conf.port_conf[port].mode[2] = VTSS_SGPIO_MODE_ON;
        }
        /* Port 20: X2s */
        conf.port_conf[20].mode[0] = VTSS_SGPIO_MODE_ON;  /* X2 TXONOFF */
        conf.port_conf[20].mode[1] = VTSS_SGPIO_MODE_ON;  /* X2 TXONOFF */
        (void)vtss_sgpio_conf_set(NULL, chip_no, group, &conf);
        break;

    case VTSS_BOARD_JAG_CU24_REF:
        /* Port 23-30 inputs are X2_LOS.
           Port 23-26 outputs are FAN 0-3 (SGPIO OFF means FAN ON)
           Port 27-28 outputs are X2_TXONOFF (SGPIO ON means X2 ON) */
        conf.bit_count = 1;
        for (port = 23; port < 31; port++) {
            conf.port_conf[port].enabled = 1;
            if (port== 27 || port == 28) /* X2 enabled */
                conf.port_conf[port].mode[0] = VTSS_SGPIO_MODE_ON;
        }
        (void)vtss_sgpio_conf_set(NULL, chip_no, group, &conf);
        break;
    case VTSS_BOARD_JAG_PCB107_REF:
        // GPIO 6,7 and 21 should be set to output (request from HW group)
        (void)vtss_gpio_direction_set(NULL, chip_no, 6, TRUE);
        (void)vtss_gpio_direction_set(NULL, chip_no, 7, TRUE);
        (void)vtss_gpio_direction_set(NULL, chip_no, 21, TRUE);
      
      /* Port 15-22 inputs are interrupts.
         Port 23-30 inputs are X2_LOS.
         Port 31    input is syncE present

         Port 23-26 outputs are FAN 0-3 (SGPIO OFF means FAN ON)
      */
      
      conf.bit_count = 1;

      for (port = 15; port < 32; port++) {
        conf.port_conf[port].enabled = 1;

        // Port 19-20 outputs are X2_TXONOFF (SGPIO ON means X2 ON) 
        if (port== 19 || port == 20) /* X2 enabled */
          conf.port_conf[port].mode[0] = VTSS_SGPIO_MODE_ON;

        for (bit = 0; bit <= conf.bit_count; bit++) {
          conf.port_conf[port].int_pol_high[bit] = FALSE; // Interrupt for 1G PHY is active low.
        }
      }

      (void)vtss_sgpio_conf_set(NULL, chip_no, group, &conf);

      // Enable the SGPIO interrupt
      for (port = 15; port < 23; port++) {
        (void) vtss_sgpio_event_enable(NULL, 0, 0, port, 0, TRUE);
      }
                

      //sgpio port 31, bit 0 indicates if there is a syncE board. If one there is no SyncE board.         
      if (vtss_sgpio_read(NULL, 0, 0, data) == VTSS_RC_OK) {
          for (port = 23; port < 32; port++) {
              PRINTD (("port:%d, data.value[0]:0x%X, enable:%d", port, data[port].value[0],conf.port_conf[port].enabled));
          }

          // Setup Clock controller if there is no SyncE board (Magic writes comes from Martin Galster)
          if (data[31].value[0]) {
              u8 i2c_data[2];

              i2c_data[0] = 38;
              i2c_data[1] = 1;
              (void) i2c_write(0, 0x70, &i2c_data[0], 2, (-1));
        
              i2c_data[0] = 33;
              i2c_data[1] = 0xc0;
              (void) i2c_write(0, 0x70, &i2c_data[0], 2, (-1));
        
              i2c_data[0] = 39;
              i2c_data[1] = 0x1;
              (void) i2c_write(0, 0x70, &i2c_data[0], 2, (-1));
        
              i2c_data[0] = 34;
              i2c_data[1] = 0xc0;
              (void) i2c_write(0, 0x70, &i2c_data[0], 2, (-1));
          }
      }
      break;
    default:
        break;
    }
}

// Function for doing special port initialization that depends upon the platform
static void jr1_init(void)
{
    vtss_chip_no_t chip_no;
    
    for (chip_no = 0; chip_no < chip_count; chip_no++) {
        jr1_init_chip(chip_no);
    }
}

static void jr1_port_conf(vtss_port_no_t port_no, port_custom_conf_t *port_conf, 
                          vtss_port_status_t *port_status)
{
    vtss_chip_no_t    chip_no = jr1_port_table[port_no].map.chip_no;
    u32               port = jr1_port_table[port_no].map.chip_port, gpio_no;
    vtss_sgpio_conf_t conf;
    int sgpio_port[] = {0,0,1,2,2,3,4,4,5,6,6,7,8,8,9,10,10,11,12,12,13,14,14,15,-1,-1,-1,20,20};
    int sgpio_bit[] = {0,2,1,0,2,1,0,2,1,0,2,1,0,2,1,0,2,1,0,2,1,0,2,1,-1,-1,-1,0,1};
    vtss_gpio_10g_gpio_mode_t gpio_mode;
  
    if (board_type == VTSS_BOARD_JAG_PCB107_REF && (port == 29 || port == 30)) {
#if defined(VTSS_CHIP_10G_PHY)
        gpio_mode.mode = VTSS_10G_PHY_GPIO_OUT;
        gpio_mode.port = 0;
        gpio_no = (port == 29) ? 3 : 2;
        (void)vtss_phy_10g_gpio_mode_set(PHY_INST, port_no, gpio_no, &gpio_mode);
        (void)vtss_phy_10g_gpio_write(PHY_INST, port_no, gpio_no, port_conf->enable ? 0 : 1);
#endif
    } else {
        if (((port < 24 && board_type == VTSS_BOARD_JAG_SFP24_REF) || port == 27 || port == 28) &&
            vtss_sgpio_conf_get(NULL, chip_no, 0, &conf) == VTSS_RC_OK) {
            conf.port_conf[sgpio_port[port]].mode[sgpio_bit[port]] = 
                (port_conf->enable ? VTSS_SGPIO_MODE_ON : VTSS_SGPIO_MODE_OFF);
            (void)vtss_sgpio_conf_set(NULL, chip_no, 0, &conf);
        }
    }
    jr1_port_info_table[port_no].enabled = port_conf->enable;
}

/* Update port and LED mapping */
static void jr1_map_update(vtss_port_no_t port_no,
                           vtss_chip_no_t chip_no,
                           i32 chip_port,
                           vtss_miim_controller_t miim_controller,
                           u8 miim_addr,
                           vtss_port_interface_t mac_if,
                           port_cap_t cap,
                           u8 led_port)
{
    port_custom_entry_t *entry;

    if (port_no < jr1_port_count && port_no < VTSS_PORTS) {
        entry = &jr1_port_table[port_no];
        entry->map.chip_no = chip_no;
        entry->map.chip_port = chip_port;
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
        entry->map.lport_no = VTSS_LPORT_MAP_DEFAULT;
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
        entry->map.miim_controller = miim_controller;
        entry->map.miim_addr = miim_addr;
        entry->map.miim_chip_no = chip_no;
        entry->mac_if = mac_if;
        entry->cap = cap;
        port_led_mapping[port_no][LED_GREEN].port = led_port;
        port_led_mapping[port_no][LED_YELLOW].port = led_port;
        port_led_mapping[port_no][LED_GREEN].bit = 0;
        port_led_mapping[port_no][LED_YELLOW].bit = 1;
    }
}

/*
 * Jaguar-1 board probe function.
 */
void jr1_board_setup_ref(vtss_board_t *board, vtss_board_info_t *board_info)
{
    vtss_port_no_t         port_idx, idx;
    vtss_chip_no_t         chip_no;
    vtss_miim_controller_t miim_controller = VTSS_MIIM_CONTROLLER_0;
    vtss_port_interface_t  mac_if = VTSS_PORT_INTERFACE_SGMII;
    port_cap_t             cap;
    jr1_10g_detect_t       detect;
    BOOL                   x0_done = 0, x1_done = 0;

    board_type = board_info->board_type;
    i2c_read   = board_info->i2c_read;
    i2c_write  = board_info->i2c_write;

    jr1_port_count = board_info->port_count;
    board->features = (VTSS_BOARD_FEATURE_LOS |  
                       VTSS_BOARD_FEATURE_POE |
#if defined(VTSS_FEATURE_VSTAX)
                       VTSS_BOARD_FEATURE_STACKING |
#endif /* VTSS_FEATURE_VSTAX */
                       VTSS_BOARD_FEATURE_VCXO);
    board->init = jr1_init;
    board->reset = jr1_reset;
    board->post_reset = jr1_post_reset;
    board->pre_reset = jr1_pre_reset;
    board->led_init = jr1_led_init;
    board->led_update = jr1_led_update;
    board->sfp_i2c_read = jr1_sfp_i2c_read;
    board->sfp_i2c_write = jr1_sfp_i2c_write;
    board->sfp_update_if = jr1_sfp_update_if;
    board->sfp_mod_detect = jr1_sfp_mod_detect;
    board->sfp_i2c_enable = jr1_sfp_i2c_enable;
    board->port_conf = jr1_port_conf;
    board->sfp_i2c_lock = 0;
    
    /* Make CU/SFP updates to port map and LED map */
    switch (board_type) {
    case VTSS_BOARD_JAG_CU24_REF: 
        board->name = "Jaguar-1 CU24 Reference"; 
        board->features |= VTSS_BOARD_FEATURE_AMS;
        break;

    case VTSS_BOARD_JAG_PCB107_REF: 
        board->name = "Jaguar-1 Tesla CU24 Reference"; 
        break;

    case VTSS_BOARD_JAG_SFP24_REF: 
        board->name = "Jaguar-1 SFP24 Reference"; 
        miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        mac_if = VTSS_PORT_INTERFACE_SERDES;
        board->features |= VTSS_BOARD_FEATURE_AMS;
        break;
    default:
        board->name = "Unknown";
        break;
    }           
    
    /* 1G ports */
    cap = board->port_cap(board_type, VTSS_PORT_NO_START);
    for (port_idx = 0; port_idx < board_info->port_count; port_idx++) {
        /* Stop when a port with other capabilities is found */
        if (board->port_cap(board_type, port_idx) != cap)
            break;

        idx = port_idx;
        chip_no = 0;
#if (VTSS_PORTS > 32) /* Avoid confusing Lint */
        if (port_idx > 23) {
            idx -= 24;
            chip_no = 1;
        }
#endif
        idx = BOARD_PORT(idx);
        jr1_map_update(port_idx, chip_no, idx, miim_controller, idx, mac_if, cap, idx);
    }


#if defined(VTSS_SW_OPTION_PORT_MUX)
    cap = board->port_cap(board_type, port_idx);
    miim_controller = VTSS_MIIM_CONTROLLER_NONE;
    mac_if = VTSS_PORT_INTERFACE_SERDES;
    x0_done = 1;
#if (VTSS_SW_OPTION_PORT_MUX == 1)
    vtss_port_no_t tesla_ports = 4;
    idx = 23;
    if (board_type != VTSS_BOARD_JAG_PCB107_REF){
      if (jr1_1g_phy_detect(1, 0, board_info)) { /* MIIM ctrl 1, phy address 0 */
        tesla_ports = jr_telsa_phy_detect_ports(1, 0, board_info); // Get the number of port for this Tesla chip
        /* Tesla module detected, assumed to be in XAUI_0 slot */
        port_tesla = port_idx;
        cap = board->port_cap(VTSS_BOARD_JAG_CU24_REF, VTSS_PORT_NO_START); /* CU24 capabilities */
        miim_controller = VTSS_MIIM_CONTROLLER_1;
        mac_if = VTSS_PORT_INTERFACE_SGMII;
      }
    }
#else
    x1_done = 1;
    idx = 19;
#endif /* VTSS_SW_OPTION_PORT_MUX == 1 */
    {
        u8 miim_addr = 0;
        for ( ; idx < 27; idx++, port_idx++, miim_addr++) {
#if (VTSS_SW_OPTION_PORT_MUX == 1)
            if (port_tesla != VTSS_PORT_NO_NONE) { // Check for Tesla plug-in board
                if (idx >= 23 + tesla_ports) { // Some Tesla's have 4 port while other have 2 port. For the 2 port version we need to disable the 2 "unused" ports.  
                    cap = 0;
                    miim_controller = VTSS_MIIM_CONTROLLER_NONE;
                    jr1_port_info_table[port_idx].enabled = FALSE;
                }
            }
#endif
            jr1_map_update(port_idx, 0, idx, miim_controller, miim_addr, mac_if, cap, port_idx);
        }
    }
#endif /* VTSS_SW_OPTION_PORT_MUX */
    
    /* 10G ports */
    cap = board->port_cap(board_type, port_idx);
    miim_controller = VTSS_MIIM_CONTROLLER_1;
    mac_if = VTSS_PORT_INTERFACE_XAUI;
    if (chip_count == 1) {
        /* Detect 10G PHY */
        detect.port_no = port_idx;
        detect.miim_addr_0 = 24;
        detect.miim_addr_1 = 25;
        detect.old_found[0] = 0;
        detect.old_found[1] = 0;
        detect.cap = cap;
        jr1_10g_detect(&detect, board_info);

        if (!x0_done) {
            /* Port 13/25: X2 port (XAUI0) */
            jr1_map_update(port_idx, 0, 27, miim_controller, detect.miim_addr_0, mac_if, detect.cap, 27);
            PRINTD (("T1: port_idx:%d, cap:0x%X", port_idx, detect.cap));
            port_idx++;
        }
        if (!x1_done) {
            /* Port 14/26: X2 port (XAUI1) */
            jr1_map_update(port_idx, 0, 28, miim_controller, detect.miim_addr_1, mac_if, detect.cap, 28);
            PRINTD (("T2: port_idx:%d, cap:0x%X", port_idx, detect.cap));
            port_idx++;
        }
    }
    
    /* Add VTSS_10G_PHY and remove 1G cap */
    cap = board->port_cap(board_type, port_idx); 
    PRINTD (("T3: port_idx:%d, cap:0x%X", port_idx, cap));

    board->default_stackport_a = port_idx;
    if (board_type != VTSS_BOARD_JAG_PCB107_REF){
        /* on old Jr1 board we assume a 8486 PHY without 1G mode support */
        cap = (cap | PORT_CAP_VTSS_10G_PHY) & (0xFFFFFFFF ^ (PORT_CAP_AUTONEG | PORT_CAP_1G_FDX)); 
        PRINTD (("T4: port_idx:%d, cap:0x%X", port_idx, cap));
        /* Port 25/27/49: SFP+ port (XAUI2) channel 0 on old board*/
        jr1_map_update(port_idx, 0, 29, miim_controller, 26, mac_if, cap, 29);
        port_idx++;
        
        /* Port 26/28/50: SFP+ port (XAUI3) */
        board->default_stackport_b = port_idx;
        jr1_map_update(port_idx, 0, 30, miim_controller, 27, mac_if, cap, 30);
        port_idx++;
    } else {
        /* on PCB107 Jr1 board we assume a 8488 or 8490 PHY with 1G mode support */
        cap = cap | PORT_CAP_VTSS_10G_PHY; 
        PRINTD (("T5: port_idx:%d, cap:0x%X", port_idx, cap));
        /* Port 25/27/49: SFP+ port (XAUI2) channel 1 on PCB107 board */
        jr1_map_update(port_idx, 0, 29, miim_controller, 27, mac_if, cap, 29);
        port_idx++;
        
        /* Port 26/28/50: SFP+ port (XAUI3) */
        board->default_stackport_b = port_idx;
        jr1_map_update(port_idx, 0, 30, miim_controller, 26, mac_if, cap, 30);
        port_idx++;
    }

    if (chip_count == 2) {
        /* Port 51: SFP+ port (XAUI2) on second device */
        board->default_stackport_b = port_idx;
        jr1_map_update(port_idx, 1, 29, miim_controller, 26, mac_if, cap, 29);
        port_idx++;

        /* Port 52: SFP+ port (XAUI3) on second device */
        jr1_map_update(port_idx, 1, 30, miim_controller, 27, mac_if, cap, 30);
        port_idx++;
    }

    /* Port 15/29/53: NPI port */
    cap = board->port_cap(board_type, port_idx);
    idx = JR1_NPI_CHIP_PORT;

    if (board_type == VTSS_BOARD_JAG_PCB107_REF){
      miim_controller = VTSS_MIIM_CONTROLLER_1;
    } else {
      miim_controller = VTSS_MIIM_CONTROLLER_0;
    }
    mac_if = VTSS_PORT_INTERFACE_SGMII;
    jr1_map_update(port_idx, 0, idx, miim_controller, 28, mac_if, cap, idx);
    port_idx++;
    
    /* Port 54: NPI port on second device */
    jr1_map_update(port_idx, 1, idx, miim_controller, 28, mac_if, cap, idx);
}



// Function for controlling the fan speed. The fan is controlled via the serial GPIOs
// In : level - The speed of the fan (0 = stopped, 255 = full speed)
vtss_rc jr1_fan_control_set(u8 level) {
    vtss_sgpio_conf_t sgpio_conf;                                       
    int port;                                
    
    level = (level >> 4) & 0xF; // Because the JR hardware only support 4 bits for speed selection, we skip the least significant bits

    (void) vtss_sgpio_conf_get(NULL, 0, 0, &sgpio_conf); // Get current serial GPIO configuration in order to modify the FAN bits.

    // Fan bits are port 23-26 bit 0 (See UG1043, Table 17)
    for (port = 23; port < 27; port++) {                                
        sgpio_conf.port_conf[port].enabled = TRUE; // Enable SGPIO                      
        sgpio_conf.port_conf[port].mode[0] = (level & (1 << (port-23)) ? VTSS_SGPIO_MODE_OFF : VTSS_SGPIO_MODE_ON); // Set the speed (4 bits)
    }
    (void) vtss_sgpio_conf_set(NULL, 0, 0, &sgpio_conf); // Update the sgpio configuration                   
    return VTSS_RC_OK;
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
