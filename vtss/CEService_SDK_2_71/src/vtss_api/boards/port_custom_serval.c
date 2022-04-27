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

// Avoid "*.h not used in module port_custom_serval.c"
/*lint --e{766} */
#include "port_custom_api.h"
#include "board_probe.h"

#if defined(VTSS_ARCH_SERVAL)

#if 0
#include <cyg/infra/diag.h>
#define PRINTD(args) { diag_printf("%s: ", __FUNCTION__); diag_printf args; diag_printf("\n"); }
#else
#define PRINTD(args) 
#endif

#define LED_TOWER_PUSH_BUTTON_INTERRUPT_SUPPORT
/*lint -esym(459, serval_port_table) */
static port_custom_entry_t serval_port_table[VTSS_PORT_ARRAY_SIZE];
static vtss_i2c_read_t    i2c_read;     /**< I2C read function */
/*lint -esym(459, i2c_write) Ok - Not to protect i2c_write with semaphore in this file*/
static vtss_i2c_write_t   i2c_write;    /**< I2C write function */

/*lint -esym(459, serval_board_type) */ // serval_board_type will only be changed at the init of the code, and before it is ever read. 
static vtss_board_type_t serval_board_type; // Use to determining which Serval board we are working on (within this file)

/* SGPIO LED mapping */
typedef struct {
    u8  port;
    u8  bit;
} sgpio_mapping_t;

/* LED colors */
typedef enum {
    LED_GREEN,
    LED_YELLOW
} led_color_t;

/* LED tower mode */
typedef enum {
    LED_TOWER_MODE_LINK_SPEED,      /**< Green: 1G link/activity; Orange: 10/100 link/activity */
    LED_TOWER_MODE_DUPLEX,          /**< Green: FDX; Orange: HDX + Collisions */
    LED_TOWER_MODE_LINK_STATUS,     /**< Green: Link/activity; Orange: port disabled/errors */
    LED_TOWER_MODE_POWER_SAVE,      /**< Disabled to save power */
    LED_TOWER_MODE_CNT
} led_tower_mode_t;

/*lint -esym(459, s_led_tower_mode) */
static led_tower_mode_t s_led_tower_mode = LED_TOWER_MODE_POWER_SAVE; // Power save mode is in fact not going to be the init state, 
                                                                      // because the led_tower_update function starts by incrementing the state, 
                                                                      // so the init state becomes LED_TOWER_MODE_LINK_SPEED.

static const sgpio_mapping_t tower_led_mapping[4][2] = {
    {{12, 0} /* tower 0 green */, {12, 1} /* tower 0 yellow */},
    {{13, 0} /* tower 1 green */, {13, 1} /* tower 1 yellow */},
    {{14, 0} /* tower 2 green */, {14, 1} /* tower 2 yellow */},
    {{15, 0} /* tower 3 green */, {15, 1} /* tower 3 yellow */}
};

void led_tower_update(void)
{
    vtss_sgpio_conf_t conf;
    int               i;

    s_led_tower_mode = (s_led_tower_mode + 1) % LED_TOWER_MODE_CNT;

    /*
     * LED tower:
     *  (top)       o  mode A (link/speed)
     *              o  mode B (link/duplex)
     *              o  mode C (link/status)
     *  (button)    o  PWR save
     */
    if (vtss_sgpio_conf_get(NULL, 0, 0, &conf) != VTSS_RC_OK) {
        return;
    }
    for (i = 0; i < 4; i++) {
        conf.port_conf[tower_led_mapping[i][0].port].mode[tower_led_mapping[i][0].bit] = 
        conf.port_conf[tower_led_mapping[i][1].port].mode[tower_led_mapping[i][1].bit] = 
        VTSS_SGPIO_MODE_OFF;
    }

    /* PTP ports - OFF */
    conf.port_conf[30].mode[0] = 
        conf.port_conf[30].mode[1] = 
        conf.port_conf[31].mode[0] = 
        conf.port_conf[31].mode[1] = 
        VTSS_SGPIO_MODE_ON;     /* Inverted polarity! */

    switch (s_led_tower_mode) {
        case LED_TOWER_MODE_LINK_SPEED:
            i = 0;
            break;
        case LED_TOWER_MODE_DUPLEX:
            i = 1;
            break;
        case LED_TOWER_MODE_LINK_STATUS:
            i = 2;
            break;
        case LED_TOWER_MODE_POWER_SAVE:
            i = 3;
            break;
        default:
            return;
    }
    conf.port_conf[tower_led_mapping[i][0].port].mode[tower_led_mapping[i][0].bit] = VTSS_SGPIO_MODE_ON;
    (void) vtss_sgpio_conf_set(NULL, 0, 0, &conf);
}

static void board_sgpio_led_set(vtss_port_no_t port_no,
                                led_color_t color,
                                vtss_sgpio_mode_t mode,
                                vtss_sgpio_conf_t *conf)
{
    u32 port = serval_port_table[port_no].map.chip_port;
    
    if (port == 10 && serval_board_type == VTSS_BOARD_SERVAL_REF) {
        /* For PCB105, activity for management port are swapped */
        mode = (mode == VTSS_SGPIO_MODE_ON ? VTSS_SGPIO_MODE_OFF :
                mode == VTSS_SGPIO_MODE_0_ACTIVITY ? VTSS_SGPIO_MODE_0_ACTIVITY_INV :
                mode == VTSS_SGPIO_MODE_0_ACTIVITY_INV ? VTSS_SGPIO_MODE_0_ACTIVITY :
                VTSS_SGPIO_MODE_ON);
    }
    conf->port_conf[port].mode[color == LED_GREEN ? 0 : 1] = mode;
}

static void serval_init(void)
{
    vtss_gpio_no_t    gpio_no;
    vtss_sgpio_conf_t conf;
    u32               port;

    /* Enable GPIO_0 to GPIO_3 as SGPIO signals */
    for (gpio_no = 0; gpio_no < 4; gpio_no++) {
        (void)vtss_gpio_mode_set(NULL, 0, gpio_no, VTSS_GPIO_ALT_0);
    }

    /* Enable GPIO_9 and GPIO_10 as MIIM controllers */
    (void)vtss_gpio_mode_set(NULL, 0, 9, VTSS_GPIO_ALT_1);
    (void)vtss_gpio_mode_set(NULL, 0, 10, VTSS_GPIO_ALT_1);

    /* Enable SGPIO ports 0-15,24-31 with two bits */
    if (vtss_sgpio_conf_get(NULL, 0, 0, &conf) == VTSS_RC_OK) {
        /* The blink mode 0 is 5 HZ for link activity and collion in half duplex. */
        conf.bmode[0] = VTSS_SGPIO_BMODE_5;
        conf.bit_count = 2;
        for (port = 0; port < VTSS_SGPIO_PORTS; port++) {
            if (serval_board_type == VTSS_BOARD_SERVAL_PCB106_REF) {
                // For the PCB106 all serial GPIO are used. See Schematic
                // Even though p18, p19, p30 and p31 are not connected, they are still in the chain.
                conf.port_conf[port].enabled = 1;
            } else {
                // For the PCB105 p16 to p23 is not used. See Schematic
                // Even though p30 and p31 are not connected, they are still in the chain.
                if (port < 16 || port > 23) {
                    conf.port_conf[port].enabled = 1;
                }
            }
        }
        (void) vtss_sgpio_conf_set(NULL, 0, 0, &conf);
    }
}

static vtss_rc serval_reset(void)
{
    // SFP Signal detect shall be active low (register 19E1 bit 1 = TRUE)
    // Dual media ports (user port 1-4)
    vtss_port_no_t port_no;

    if (serval_port_table[0].mac_if == VTSS_PORT_INTERFACE_SGMII) {
        for (port_no = 0; port_no < 4; port_no++) {
            (void) vtss_phy_write_masked(NULL, port_no, 19 |VTSS_PHY_REG_EXTENDED , 0x1, 0x1);
        }

        return vtss_phy_pre_reset(NULL, 0); /* Tesla PHY */
    }
    return VTSS_RC_OK;
}

static vtss_rc serval_post_reset(void)
{
    return (serval_port_table[0].mac_if == VTSS_PORT_INTERFACE_SGMII ? 
            vtss_phy_post_reset(NULL, 0) : VTSS_RC_OK); /* Tesla PHY */
}

static void serval_pre_reset(void)
{
}

static vtss_rc serval_led_init(void)
{
   return VTSS_RC_OK;
}

static BOOL serval_sfp_accept(u8 *sfp_rom)
{
    return TRUE;
}

static vtss_rc serval_sfp_i2c_read(vtss_port_no_t port_no, u8 i2c_addr, u8 addr, 
                                   u8 *const data, u8 cnt) 
{
    port_custom_entry_t *entry = &serval_port_table[port_no];
    u32                 port = entry->map.chip_port;
    i8                  i2c_clk_sel;

    if (entry->cap & PORT_CAP_DUAL_COPPER) {
        // Due to a hardware board issue only SFP i2c mux 0 works, so that is always used.
        return vtss_phy_i2c_read(NULL, port_no, port_no, addr, i2c_addr, data, cnt);
    } else if (port == 3) {
        /*  Map the port to an GPIO Clock Line which enables the I2C on the SPF.*/
        i2c_clk_sel = 11;
    } else if (port == 2) {
        i2c_clk_sel = 12;
    } else if (port == 1) {
        i2c_clk_sel = 18;
    } else if (port == 0) {
        i2c_clk_sel = 19;
    } else if (port == 8) {
        i2c_clk_sel = 20;
    } else if (port == 9) {
        i2c_clk_sel = 21;
    } else {
        return VTSS_RC_ERROR;
    }

    if (i2c_read != NULL) {
        return i2c_read(port_no, i2c_addr, addr, data, cnt, i2c_clk_sel);
    }
    return VTSS_RC_OK;
}

static vtss_rc serval_sfp_i2c_write(vtss_port_no_t port_no, u8 i2c_addr, u8 addr, u8 *const data)
{
    port_custom_entry_t *entry = &serval_port_table[port_no];
    u32                 port = entry->map.chip_port;
    i8                  i2c_clk_sel;
    u8                  i2c_data[3];

    if (entry->cap & PORT_CAP_DUAL_COPPER) {
        // Due to a hardware board issue only SFP i2c mux 0 works, so that is always used.
        return vtss_phy_i2c_write(NULL, port_no, port_no, addr, i2c_addr, data, 2);
    } else if (port == 3) {
        /*  Map the port to an GPIO Clock Line which enables the I2C on the SPF.*/
        i2c_clk_sel = 11;
    } else if (port == 2) {
        i2c_clk_sel = 12;
    } else if (port == 1) {
        i2c_clk_sel = 18;
    } else if (port == 0) {
        i2c_clk_sel = 19;
    } else if (port == 8) {
        i2c_clk_sel = 20;
    } else if (port == 9) {
        i2c_clk_sel = 21;
    } else {
        return VTSS_RC_ERROR;
    }

    if (i2c_read != NULL) {
        i2c_data[0] = addr;
        memcpy(&i2c_data[1], data, 2);
        return i2c_write(port_no, i2c_addr, i2c_data, 3, i2c_clk_sel);
    }
    return VTSS_RC_OK;
}

static void serval_sfp_update_if(vtss_port_no_t port_no, vtss_port_interface_t mac_if)
{
    serval_port_table[port_no].mac_if = mac_if;
}

static vtss_rc serval_sfp_mod_detect(BOOL *detect_status)
{
    vtss_sgpio_port_data_t data[VTSS_SGPIO_PORTS];
    vtss_port_no_t         port_no;
    port_custom_entry_t    *entry;
   
    if (vtss_sgpio_read(NULL, 0, 0, data) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }        
    
    /* The status in 'data' is inverted i.e. '0' means detected */
    /* We return '1' when a module is detected */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        entry = &serval_port_table[port_no];
        if (entry->cap & PORT_CAP_DUAL_COPPER) {
            /* Dual media port for PCB106, see U46 in schematic. */
            detect_status[port_no] = (data[16 + (port_no / 2)].value[port_no % 2] ? 0 : 1); 
        } else if (entry->cap & PORT_CAP_SFP_DETECT) {
            detect_status[port_no] = (data[entry->map.chip_port].value[1] ? 0 : 1);
        } else {
            detect_status[port_no] = 0;
        }
    }
    return VTSS_RC_OK;    
}

static vtss_rc serval_sfp_i2c_enable(vtss_port_no_t port_no)
{
    return VTSS_RC_OK; /* The i2c mux is integrated into serval and is activated through i2c_read */
}

static vtss_rc serval_led_update(vtss_port_no_t port_no,
                                 vtss_port_status_t *status,
                                 vtss_port_counters_t *counters,
                                 port_custom_conf_t *port_conf)
{
    // It is OK to access these with semaphore protection.
    /*lint -esym(459, status_old, port_collision_cnt, fiber_link_old, port_conf_old, s_led_tower_mode_old) */
    static vtss_port_status_t  status_old[VTSS_PORTS];
    static vtss_port_counter_t port_collision_cnt[VTSS_PORTS];
    BOOL                       need_process = FALSE, collision = FALSE, tower_mode_changed;
    vtss_port_no_t             process_cnt = port_no + 1;
    BOOL                       fiber_link = FALSE;
    static BOOL                fiber_link_old[VTSS_PORTS]; // If the CU link were up when the SFP was inserted we might not detect the link change, so therefore we need to do a "need_process" if the link changes between fiber and cu, so we have to be able to remember the last link type.
    BOOL                       cu_sfp = FALSE;
    u16                        phy_reg;
    static port_custom_conf_t port_conf_old[VTSS_PORTS];
    static led_tower_mode_t s_led_tower_mode_old[VTSS_PORTS];
    
    // Tesla SFP ports are ports 0-3. Because the SFP LED are controlled by Tesla, we need to disable the CU LED if the link up is due to SFP link (Found in Tesla register 28 bit 1:0).
    if ((serval_port_table[port_no].cap & PORT_CAP_DUAL_COPPER) &&
        vtss_phy_read(NULL, port_no,  28, &phy_reg) == VTSS_RC_OK &&
        (phy_reg & 0x3) == 0x2) { // Bit 1:0   0x1 = Cu, 0x2 = FIBER

        fiber_link = TRUE;

        if (vtss_phy_read(NULL, port_no,  23, &phy_reg) == VTSS_RC_OK) {
            if ((phy_reg & 0x700) == 0x100) { // If we are in the PASS through mode we are running CU SFP and should be handled specially .
                cu_sfp = TRUE;
            }
        }
    } 

    /* Check if port's link/speed/fdx has changed */
    if (status_old[port_no].link != status->link ||
        memcmp(&port_conf_old[port_no], port_conf, sizeof(port_conf_old[port_no])) ||
        (status_old[port_no].link == status->link && (status_old[port_no].speed != status->speed || status_old[port_no].fdx != status->fdx ||fiber_link_old[port_no] != fiber_link))) {
        need_process = TRUE;
    }
    status_old[port_no] = *status;
    fiber_link_old[port_no] = fiber_link; 
    memcpy(&port_conf_old[port_no], port_conf, sizeof(port_conf_old[port_no]));

    /* Check if port collision occured */
    if (s_led_tower_mode == LED_TOWER_MODE_DUPLEX &&
        status->link &&
        !status->fdx &&
        port_collision_cnt[port_no] != counters->rmon.tx_etherStatsCollisions) {
        need_process = TRUE;
        collision = TRUE;
    }
    port_collision_cnt[port_no] = counters->rmon.tx_etherStatsCollisions;

    // find out if tower mode has changed since the last time the port was served.
    tower_mode_changed = s_led_tower_mode_old[port_no] != s_led_tower_mode;
    s_led_tower_mode_old[port_no] = s_led_tower_mode;

    /* Return here if nothing has changed or in power saving mode */
    if ((!need_process && !tower_mode_changed) ||
        (!tower_mode_changed && s_led_tower_mode == LED_TOWER_MODE_POWER_SAVE)) {
      return VTSS_RC_OK;
    }
    
    /* Need to update? */
    if(need_process || tower_mode_changed) {
        vtss_sgpio_conf_t conf;
        if (vtss_sgpio_conf_get(NULL, 0, 0, &conf) == VTSS_RC_OK) {
            led_color_t color;
            vtss_sgpio_mode_t activity;
            for(; port_no < process_cnt; port_no++) {
                board_sgpio_led_set(port_no, LED_GREEN, VTSS_SGPIO_MODE_OFF, &conf);
                board_sgpio_led_set(port_no, LED_YELLOW, VTSS_SGPIO_MODE_OFF, &conf);

                color = LED_GREEN;
                activity = VTSS_SGPIO_MODE_OFF;
                
                if (!fiber_link) { // Only turn on LED if the link up is a CU port.
                    switch (s_led_tower_mode) {
                    case LED_TOWER_MODE_LINK_SPEED:
                        if (status_old[port_no].link) {
                            if (status_old[port_no].speed >= VTSS_SPEED_1G) {
                                /* Green: 1G link/activity */
                                color = LED_GREEN;
                                activity = VTSS_SGPIO_MODE_0_ACTIVITY_INV;
                            } else {
                                /* Yellow: 100/10 link/activity */
                                color = LED_YELLOW;
                                activity = VTSS_SGPIO_MODE_0_ACTIVITY_INV;
                            }
                        }
                        break;
                    case LED_TOWER_MODE_DUPLEX:
                        if (status_old[port_no].link) {
                            if (status_old[port_no].fdx) {
                                /* Green: FDX */
                                color = LED_GREEN;
                                activity = VTSS_SGPIO_MODE_0_ACTIVITY_INV;
                            } else if (collision) {
                                /* Blink yellow: HDX with collision */
                                color = LED_YELLOW;
                                activity = VTSS_SGPIO_MODE_0;
                            } else {
                                /* Yellow: HDX without collision */
                                color = LED_YELLOW;
                                activity = VTSS_SGPIO_MODE_ON;
                            }
                        }
                        break;
                    case LED_TOWER_MODE_LINK_STATUS:
                      if (port_conf->enable == FALSE) {
                          color = LED_YELLOW;
                          activity = VTSS_SGPIO_MODE_ON;
                      } else  if (status_old[port_no].link) {
                          /* Green: link on */
                          color = LED_GREEN;
                          activity = VTSS_SGPIO_MODE_ON;
                      }
                      break;
                    case LED_TOWER_MODE_POWER_SAVE:
                        break;
                    default:
                        break;
                    }
                }

                board_sgpio_led_set(port_no, color, activity, &conf);

                // Setup SFP LED for Dual media ports (port 1-4)
                if (serval_port_table[port_no].cap & PORT_CAP_DUAL_COPPER) {
                    
                  // LED0 = Yellow, LED1 = Green
                  vtss_phy_led_mode_select_t led0_blink_mode, led1_blink_mode;

                  led0_blink_mode.number = LED0; // LED0 is the yellow LED
                  led1_blink_mode.number = LED1; // LED1 is the green LED

                  
                  // In Link status mode the Yellow LED shall be turn on if port is disabled, else green
                  switch (s_led_tower_mode) {
                  case LED_TOWER_MODE_LINK_STATUS :
                    led1_blink_mode.mode = LINK100BASE_FX_1000BASE_X_ACTIVITY; // Both 100FX and 1000Base-x should be green in this mode.

                    // If port is disabled then turn yellow LED on.
                    if (port_conf->enable == FALSE) {
                      led0_blink_mode.mode = FORCE_LED_ON;
                    } else {
                      led0_blink_mode.mode = FORCE_LED_OFF;
                    }
                    break;

                  case LED_TOWER_MODE_POWER_SAVE:
                    led0_blink_mode.mode = FORCE_LED_OFF;
                    led1_blink_mode.mode = FORCE_LED_OFF;
                    break;

                  default: 
                      if (cu_sfp) { // For Cu SFP we need to select the corresponding LED
                          switch (status->speed) {
                          case VTSS_SPEED_10M:
                          case VTSS_SPEED_100M:
                              led0_blink_mode.mode = LINK100_1000_ACTIVITY; // Set yellow LED
                              led1_blink_mode.mode = FORCE_LED_OFF;         // Set green LED
                              break;
                          default:
                              led0_blink_mode.mode = FORCE_LED_OFF;         // Set yellow LED
                              led1_blink_mode.mode = LINK100_1000_ACTIVITY; // Set green LED
                          }
                      } else {
                          led0_blink_mode.mode = LINK100BASE_FX_ACTIVITY; // Default mode for the yellow LED
                          led1_blink_mode.mode = LINK1000BASE_X_ACTIVITY; // Default mode for the green LED
                      }
                    break;
                  }

                  if (vtss_phy_led_mode_set(NULL, port_no, led0_blink_mode) != VTSS_RC_OK) {
                    return VTSS_RC_ERROR;
                  }


                  if (vtss_phy_led_mode_set(NULL, port_no, led1_blink_mode) != VTSS_RC_OK) {
                    return VTSS_RC_ERROR;
                  }
                }
            }


            (void) vtss_sgpio_conf_set(NULL, 0, 0, &conf);
        }
    }

    return VTSS_RC_OK;
}

static void serval_port_conf(vtss_port_no_t port_no, 
                             port_custom_conf_t *port_conf, 
                             vtss_port_status_t *port_status)
{
    port_custom_entry_t *entry = &serval_port_table[port_no];
    u32                 port = entry->map.chip_port;
    vtss_sgpio_conf_t   conf;

    if (port < 4) {
        /* Chip port 0-3, SFP port 4-7, port 24-27 used for tx_enable */
        port = (27 - port);
    } else if (port < 8) {
        /* Chip port 4-7, SFP port 0-3 (if dual media), SGPIO port 20-23 used for tx_enable */
        port = ((entry->cap & PORT_CAP_DUAL_COPPER) ? (27 - port) : 0);
    } else if (port < 10) {
        /* Chip port 8-9, SFP port 8-9, port 28-29 used for tx_enable */
        port = (20 + port);
    } else {
        port = 0;
    }

    if (port != 0 && vtss_sgpio_conf_get(NULL, 0, 0, &conf) == VTSS_RC_OK) {
        conf.port_conf[port].mode[0] = (port_conf->enable ? VTSS_SGPIO_MODE_ON :
                                        VTSS_SGPIO_MODE_OFF);
        (void)vtss_sgpio_conf_set(NULL, 0, 0, &conf);
    }
}

BOOL vtss_board_probe_serval(vtss_board_t *board, vtss_board_info_t *board_info)
{
    /*lint -esym(459, i2c_read) */
    vtss_port_no_t      port_no;
    port_custom_entry_t *entry;
    u16 gpio_in_reg = 0;
    
    memset(board, 0, sizeof(*board));

    //
    // For PCB106 Tesla GPIO9 is pulled high, for PCB105 it is pulled low.
    //
    board->type = VTSS_BOARD_SERVAL_REF; // Default to PCB105

    /* Enable GPIO_9 and GPIO_10 as MIIM controllers for being able to access the PHY*/
    (void)vtss_gpio_mode_set(NULL, 0, 9, VTSS_GPIO_ALT_1);
    (void)vtss_gpio_mode_set(NULL, 0, 10, VTSS_GPIO_ALT_1);

    // At this point the PHY access function is not ready, so we have to do direct MIIM access
    (void)vtss_miim_write(NULL, 0, VTSS_MIIM_CONTROLLER_1, 16, 31, 0x10); // Setup GPIO page
    if (vtss_miim_read(NULL, 0, VTSS_MIIM_CONTROLLER_1, 16, 15, &gpio_in_reg) == VTSS_RC_OK) {
        if (gpio_in_reg & 0x0200) {
            board->type = VTSS_BOARD_SERVAL_PCB106_REF;
        }
    }
    (void)vtss_miim_write(NULL, 0, VTSS_MIIM_CONTROLLER_1, 16, 31, 0); // Setup STANDARD page
    

    //
    // Setup Board info
    //
    i2c_read   = board_info->i2c_read;
    i2c_write  = board_info->i2c_write;
    board_info->board_type = board->type;
    board->name = "Serval";
    board->features = (VTSS_BOARD_FEATURE_AMS | VTSS_BOARD_FEATURE_LOS | VTSS_BOARD_FEATURE_VCXO);
    board->custom_port_table = serval_port_table;
    board->init = serval_init;
    board->reset = serval_reset;
    board->pre_reset = serval_pre_reset;
    board->post_reset = serval_post_reset;
    board->led_init = serval_led_init;
    board->led_update = serval_led_update;
    board->port_conf = serval_port_conf;
    board->sfp_i2c_read = serval_sfp_i2c_read;
    board->sfp_i2c_write = serval_sfp_i2c_write;
    board->sfp_update_if = serval_sfp_update_if;
    board->sfp_mod_detect = serval_sfp_mod_detect;
    board->sfp_i2c_enable = serval_sfp_i2c_enable;
    board->sfp_i2c_lock = 0;
    board->sfp_accept = serval_sfp_accept;
    
    /* Fill out port mapping table */
    for (port_no = 0; port_no < VTSS_PORTS; port_no++) {
        entry = &serval_port_table[port_no];
        if (port_no == (VTSS_PORTS - 1)) {
            /* Last port maps to chip port 10: NPI port */
            entry->map.chip_port = 10;
            entry->map.miim_controller = VTSS_MIIM_CONTROLLER_1;
            entry->map.miim_addr = 28;
            entry->mac_if = VTSS_PORT_INTERFACE_SGMII;
            entry->cap = PORT_CAP_TRI_SPEED_COPPER;
#if (VTSS_PORTS == 11)
        } else if (port_no < 4) {
            /* Chip port 7-4: Copper */
            if (board->type == VTSS_BOARD_SERVAL_PCB106_REF) {
                entry->cap = PORT_CAP_TRI_SPEED_DUAL_ANY_FIBER;
            } else {
                entry->cap = PORT_CAP_TRI_SPEED_COPPER;
            }
            entry->map.chip_port = (7 - port_no);
            entry->map.miim_controller = VTSS_MIIM_CONTROLLER_1;
            entry->map.miim_addr = (16 + port_no);
            entry->mac_if = VTSS_PORT_INTERFACE_SGMII;
#endif
        } else if (port_no < (VTSS_PORTS - 3)) {
            /* Chip port 3-0: 1G SFP */
            entry->map.chip_port = (VTSS_PORTS - 4 - port_no);
            entry->map.miim_controller = VTSS_MIIM_CONTROLLER_NONE;
            entry->mac_if = VTSS_PORT_INTERFACE_SERDES;
            entry->cap = PORT_CAP_SFP_1G | PORT_CAP_SFP_SD_HIGH;
        } else {
            /* Chip port 8-9: 2.5G SFP */
            entry->map.chip_port = (port_no + 11 - VTSS_PORTS);
            entry->map.miim_controller = VTSS_MIIM_CONTROLLER_NONE;
            entry->mac_if = VTSS_PORT_INTERFACE_SERDES;
            entry->cap = PORT_CAP_SFP_2_5G | PORT_CAP_SFP_SD_HIGH;
        }
    }

#if defined(VTSS_SW_OPTION_MEP_LOOP_PORT)
    {
        /* If loop port is used, swap the last two ports.
           The loop port will be the last port and we want the management port available. */
        port_custom_entry_t tmp;

        tmp = serval_port_table[VTSS_PORTS - 1];
        serval_port_table[VTSS_PORTS - 1] = serval_port_table[VTSS_PORTS - 2];
        serval_port_table[VTSS_PORTS - 2] = tmp;
        board_info->port_count--;
    }
#endif /* VTSS_SW_OPTION_MEP_LOOP_PORT */

    serval_board_type = board->type;
    return TRUE;
}

#endif /* defined(VTSS_ARCH_SERVAL) */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/


