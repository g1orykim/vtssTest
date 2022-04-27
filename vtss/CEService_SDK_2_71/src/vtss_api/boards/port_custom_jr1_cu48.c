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

// Avoid "board_probe.h not used in module port_custom_jr1_cu48.c"
/*lint --e{766} */
#include "board_probe.h"

#if defined(VTSS_ARCH_JAGUAR_1)

static vtss_i2c_read_t    i2c_read;     /**< I2C read function */

#include "port_custom_jr1_api.h"
/*lint -esym(459, port_tesla) */
extern vtss_port_no_t port_tesla;

#if 0
#include <cyg/infra/diag.h>
#define PRINTD(args) { diag_printf("%s: ", __FUNCTION__); diag_printf args; diag_printf("\n"); }
#else
#define PRINTD(args) 
#endif

led_tower_mode_t jr1_tower_mode;

port_custom_entry_t jr1_port_table[VTSS_PORTS];

jr1_port_info_t jr1_port_info_table[VTSS_PORTS];

/* Release ports from reset */
static void jr1_phy_reset(void)
{
    /* Release PHYs from reset using GPIO_12 */
    (void)vtss_gpio_direction_set(NULL, 0, 12, 1);
    (void)vtss_gpio_write(NULL, 0, 12, 0);
    (void)vtss_gpio_write(NULL, 0, 12, 1);
    VTSS_MSLEEP(500);
}

/* Release ports from reset */
static vtss_rc jr1_reset(void)
{
    /* Release PHYs from reset */
    jr1_phy_reset();
    
#if 1
    /* PHY pre-reset */
    (void)vtss_phy_pre_reset(NULL, 0);
    (void)vtss_phy_pre_reset(NULL, 12);
    (void)vtss_phy_pre_reset(NULL, 24);
    (void)vtss_phy_pre_reset(NULL, 36);
#endif

    return VTSS_RC_OK;
}

static void jr1_led_tower_update(BOOL restore)
{
    /*lint -esym(459, jr1_tower_mode) */
    vtss_chip_no_t         chip_no = 0;
    vtss_sgpio_group_t     group = 1;
    vtss_sgpio_conf_t      conf;
    int                    n;
    
    n = jr1_tower_mode;
    jr1_tower_mode = (restore ? LED_TOWER_MODE_LINK_SPEED : ((n + 1) % LED_TOWER_MODE_CNT));
    if (vtss_sgpio_conf_get(NULL, chip_no, group, &conf) == VTSS_RC_OK) {
        /* Turn off old mode */
        conf.port_conf[29 + (n / 2)].mode[n % 2] = VTSS_SGPIO_MODE_OFF;

        /* Turn on new mode */
        n = jr1_tower_mode;
        conf.port_conf[29 + (n / 2)].mode[n % 2] = VTSS_SGPIO_MODE_ON;

        (void)vtss_sgpio_conf_set(NULL, chip_no, group, &conf);
    }
}

/* Initialize port LEDs */
static vtss_rc jr1_led_init(void)
{
    jr1_led_tower_update(1);
    
    return VTSS_RC_OK;
}

/* LED control according to RDR0020 */
static vtss_rc jr1_led_update(vtss_port_no_t port_no,
                              vtss_port_status_t *status,
                              vtss_port_counters_t *counters,
                              port_custom_conf_t *port_conf)
{
    /*lint -esym(459, val, timer, jr1_port_table, jr1_port_info_table, port_led_mapping) */
    static BOOL            val = 0;
    static u32             timer = 0;
    vtss_chip_no_t         chip_no;
    vtss_sgpio_group_t     group = 1;
    vtss_sgpio_conf_t      sgpio_conf[2];
    BOOL                   sgpio_update[2];
    vtss_sgpio_port_conf_t *conf;
    BOOL                   old_val, update = 0, restore;
    BOOL                   activity = 0;
    vtss_port_no_t         port_start = port_no, port_end = (port_no + 1);
    u16                    i, cu_green, cu_orange;
    vtss_sgpio_mode_t      x2_green, x2_orange;
    port_custom_entry_t    *port_entry = &jr1_port_table[port_no];
    jr1_port_info_t        *port_info = &jr1_port_info_table[port_no];
    vtss_port_status_t     *old_status = &port_info->status;
    vtss_port_counter_t    cnt = 0;
    
    /* Check if port status has changed */
    if (old_status->link != status->link || 
        old_status->speed != status->speed || 
        old_status->fdx != status->fdx) {
        update = 1;
        *old_status = *status;
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
        port_end = VTSS_PORT_NO_END;
        update = 1;
    } else if (jr1_tower_mode == LED_TOWER_MODE_POWER_SAVE)
        update = 0; /* No further processing in power saving mode */

    /* Return if no update must be done */
    if (!update)
        return VTSS_RC_OK;
    
    /* Initialize SGPIO update flags */
    for (chip_no = 0; chip_no < 2; chip_no++)
        sgpio_update[chip_no] = 0;

    /* This is for Lint only */
    memset(sgpio_conf, 0, sizeof(sgpio_conf));

    /* Update port LEDs */
    for (port_no = port_start; port_no < port_end; port_no++) {
        port_info = &jr1_port_info_table[port_no];
        status = &port_info->status;

        /* LEDs are OFF by default */
        cu_green = 14;  /* OFF */
        cu_orange = 14; /* OFF */
        x2_green = VTSS_SGPIO_MODE_OFF;
        x2_orange = VTSS_SGPIO_MODE_OFF;

        switch (jr1_tower_mode) {
        case LED_TOWER_MODE_LINK_SPEED:
            cu_green = 1;  /* Link1000/Activity */
            cu_orange = 6; /* Link10/100/Activity */
            if (status->link) 
                x2_green = (port_info->activity ? VTSS_SGPIO_MODE_0 : VTSS_SGPIO_MODE_ON);
            break;
        case LED_TOWER_MODE_DUPLEX:
            if (status->link) {
                if (status->fdx) {
                    cu_green = 15; /* ON */
                } else {
                    cu_orange = (port_info->collisions ? 9 : 15); /* Collisions or ON */
                }
                x2_green = VTSS_SGPIO_MODE_ON;
            }
            break;
        case LED_TOWER_MODE_LINK_STATUS:
            if (status->link) {
                if (port_info->errors) {
                    cu_orange = 10; /* Activity */
                    x2_orange = VTSS_SGPIO_MODE_0;
                } else {
                    cu_green = 0; /* Link/Activity */
                    x2_green = (port_info->activity ? VTSS_SGPIO_MODE_0 : VTSS_SGPIO_MODE_ON);
                }
            } else if (!port_info->enabled) {
                cu_orange = 15; /* ON */
                x2_orange = VTSS_SGPIO_MODE_ON;
            }
            break;
        case LED_TOWER_MODE_POWER_SAVE:
        default:
            break;
        }
        port_entry = &jr1_port_table[port_no];
        if (port_entry->cap & PORT_CAP_10G_FDX) {
            /* 10G port, SGPIO port 27-28 are used for chip port 29-30 LEDs */
            chip_no = port_entry->map.chip_no;
            if (sgpio_update[chip_no] ||
                vtss_sgpio_conf_get(NULL, chip_no, group, &sgpio_conf[chip_no]) == VTSS_RC_OK) {
                sgpio_update[chip_no] = 1;
                conf = &sgpio_conf[chip_no].port_conf[port_entry->map.chip_port - 2];
                conf->mode[0] = x2_green;
                conf->mode[1] = x2_orange;
            }
        } else {
            /* 1G port, LED control via PHY */
            (void)vtss_phy_write(NULL, port_no, 29, (cu_orange << 4) | cu_green);
        }
    }

    /* Set updated SGPIO config */
    for (chip_no = 0; chip_no < 2; chip_no++) {
        if (sgpio_update[chip_no])
            (void)vtss_sgpio_conf_set(NULL, chip_no, group, &sgpio_conf[chip_no]);
    }

    return VTSS_RC_OK;
}



// Function called after all ports has been initialized
static vtss_rc jr1_post_reset(void) 
{
    return vtss_phy_post_reset(NULL, 0); // Setting the coma mode pin, only need to be done for one of the chips.
}

static void jr1_pre_reset(void)
{
    vtss_chip_no_t     chip_no;
    vtss_gpio_no_t     gpio;
    vtss_sgpio_group_t group = 1;
    vtss_sgpio_conf_t  conf;
    u32                port;
    
    for (chip_no = 0; chip_no < 2; chip_no++) {
        /* Turn off LEDs */
        if (vtss_sgpio_conf_get(NULL, chip_no, group, &conf) == VTSS_RC_OK) {
            for (port = 27; port < 31; port++) {
                conf.port_conf[port].mode[0] = VTSS_SGPIO_MODE_OFF;
                conf.port_conf[port].mode[1] = VTSS_SGPIO_MODE_OFF;
            }
            (void)vtss_sgpio_conf_set(NULL, chip_no, group, &conf);
            VTSS_MSLEEP(100); /* Wait a period time */
        }
        
        /* Disable SGPIOs */
        for (gpio = 4; gpio < 6; gpio++) {
            (void)vtss_gpio_mode_set(NULL, chip_no, gpio, VTSS_GPIO_IN);
        }
        VTSS_MSLEEP(10); /* Wait a period time */
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

static vtss_rc jr1_sfp_i2c_enable(vtss_port_no_t port_no)
{
    return VTSS_RC_ERROR;
}

static void jr1_sfp_update_if(vtss_port_no_t port_no, vtss_port_interface_t mac_if)
{

}

static void jr1_init(void)
{
    vtss_chip_no_t     chip_no;
    vtss_gpio_no_t     gpio;
    vtss_sgpio_group_t group = 1;
    vtss_sgpio_conf_t  conf;
    u32                port;
    
    for (chip_no = 0; chip_no < 2; chip_no++) {
        /* GPIO pins 4-5 are used for SGPIO (serial LED) */
        for (gpio = 4; gpio < 6; gpio++) {
            (void)vtss_gpio_mode_set(NULL, chip_no, gpio, VTSS_GPIO_ALT_0);
        }

        /* GPIO pins 12-16 on slave are used for X2 TXONOFF */
        if (chip_no == 1) {
            for (gpio = 12; gpio < 16; gpio++) {
                (void)vtss_gpio_mode_set(NULL, chip_no, gpio, VTSS_GPIO_OUT);
                (void)vtss_gpio_write(NULL, chip_no, gpio, 1); 
            }
        }
        
        /* Setup SGPIO group 1 (serial LED) */
        if (vtss_sgpio_conf_get(NULL, chip_no, group, &conf) == VTSS_RC_OK) {
            /* The blink mode 0 is 5 HZ for link activity, errors and collisions. */
            conf.bmode[0] = VTSS_SGPIO_BMODE_5;
            conf.bit_count = 2;
            for (port = 27; port < 31; port++) {
                conf.port_conf[port].enabled = 1;
            }
            (void)vtss_sgpio_conf_set(NULL, chip_no, group, &conf);
        }
    }
}

static void jr1_port_conf(vtss_port_no_t port_no, 
                          port_custom_conf_t *port_conf, 
                          vtss_port_status_t *port_status)
{
    jr1_port_info_table[port_no].enabled = port_conf->enable;
}

static void jr1_map_update(vtss_port_no_t port_no,
                           vtss_chip_no_t chip_no,
                           i32 chip_port,
                           vtss_miim_controller_t miim_bus,
                           u8 miim_addr,
                           vtss_chip_no_t miim_chip_no,
                           vtss_port_interface_t mac_if,
                           port_cap_t cap)
{
    port_custom_entry_t *entry;
    
    if (port_no < VTSS_PORTS) {
        entry = &jr1_port_table[port_no];
        entry->map.chip_no = chip_no;
        entry->map.chip_port = chip_port;
        entry->map.miim_controller = miim_bus;
        entry->map.miim_addr = miim_addr;
        entry->map.miim_chip_no = miim_chip_no;
        entry->mac_if = mac_if;
        entry->cap = cap;
    }
}

/* Jaguar-1 register targets and addresses */
#define JR1_TGT_GCB            0x01
#define JR1_ADDR_GPIO_OUT      0x1c
#define JR1_ADDR_GPIO_IN       0x1d
#define JR1_ADDR_GPIO_OE       0x1e
#define JR1_ADDR_GPIO_ALT0     0x22
#define JR1_ADDR_GPIO_ALT1     0x23
#define JR1_ADDR_MIIM_STATUS   0x28
#define JR1_ADDR_MIIM_CMD      0x2a
#define JR1_ADDR_MIIM_DATA     0x2b
#define JR1_ADDR_MIIM_CFG      0x2c

/* Read from primary device */
static vtss_rc jr_rd(u32 tgt, u32 addr, u32 *value, vtss_board_info_t *board_info)
{
    vtss_rc rc = VTSS_RC_ERROR;

    *value = 0xdeadbeef;
    if (board_info->reg_read != NULL) {
        rc = board_info->reg_read(0, (tgt << 14) + addr, value);
        PRINTD(("tgt: 0x%x, addr: 0x%x, value: 0x%08x", tgt, addr, *value));
    }
    return rc;
}

/* Write to primary device */
static vtss_rc jr_wr(u32 tgt, u32 addr, u32 value, vtss_board_info_t *board_info)
{
    vtss_rc rc = VTSS_RC_ERROR;

    if (board_info->reg_write != NULL) {
        PRINTD(("tgt: 0x%x, addr: 0x%x, value: 0x%08x", tgt, addr, value));
        rc = board_info->reg_write(0, (tgt << 14) + addr, value);
    }
    return rc;
}

#define PHY_CMD_ADDRESS  0 /* 10G: Address */
#define PHY_CMD_WRITE    1 /* 1G/10G: Write */
#define PHY_CMD_READ_INC 2 /* 1G: Read, 10G: Read and increment */
#define PHY_CMD_READ     3 /* 10G: Read */

/* MIIM bus command function for primary device */
static vtss_rc jr1_miim_cmd(u8 cmd, u8 sof, u8 ctrl, u8 miim_addr, u8 reg_addr, u16 *data, 
                            vtss_board_info_t *board_info)
{
    vtss_rc rc;
    u32     value, offs = (ctrl ? 9 : 0);
    int     i;

    PRINTD(("cmd: %u, sof: %u, ctrl: %u, miim_addr: %u, reg_addr: %u",
            cmd, sof, ctrl, miim_addr, reg_addr));
    rc = jr_wr(JR1_TGT_GCB, JR1_ADDR_MIIM_CFG + offs, (sof<<9) | (0x32<<0), board_info);
    if (rc != VTSS_RC_OK)
        return rc;
    rc = jr_wr(JR1_TGT_GCB, JR1_ADDR_MIIM_CMD + offs, 
               (1UL<<31) | (miim_addr<<25) | (reg_addr<<20) | ((*data)<<4) | (cmd<<1), 
               board_info);
    if (rc != VTSS_RC_OK)
        return rc;

    for (i = 0; i < 1000; i++) {
        rc = jr_rd(JR1_TGT_GCB, JR1_ADDR_MIIM_STATUS + offs, &value, board_info);
        if (rc != VTSS_RC_OK)
            return rc;
        if (value != 0)
            continue;
        
        if (cmd == PHY_CMD_READ_INC || cmd == PHY_CMD_READ) {
            rc = jr_rd(JR1_TGT_GCB, JR1_ADDR_MIIM_DATA + offs, &value, board_info);
            if (rc != VTSS_RC_OK)
                return rc;
            if (value & 0x30000)
                break;
            *data = (value & 0xffff);
        }
        return VTSS_RC_OK;
    }
    return VTSS_RC_ERROR;
}

/* Read PHY register on primary device */
static vtss_rc jr1_miim_rd(u8 ctrl, u8 miim_addr, u8 reg_addr, u16 *data, 
                           vtss_board_info_t *board_info)
{
    return jr1_miim_cmd(PHY_CMD_READ_INC, 1, ctrl, miim_addr, reg_addr, data, board_info); 
}

/* Read MMD register on primary device */ 
static vtss_rc jr1_mmd_rd(u8 ctrl, u8 miim_addr, u8 mmd, u16 addr, u16 *value,
                          vtss_board_info_t *board_info)
{
    vtss_rc rc;

    rc = jr1_miim_cmd(PHY_CMD_ADDRESS, 0, ctrl, miim_addr, mmd, &addr, board_info);
    if (rc == VTSS_RC_OK)
        rc = jr1_miim_cmd(PHY_CMD_READ, 0, ctrl, miim_addr, mmd, value, board_info);
    return rc;
}

/* Detect Vitesse 10G PHY module on MIIM controller 1 */
void jr1_10g_detect(jr1_10g_detect_t *detect, vtss_board_info_t *board_info)
{
    u16 model;
    u8  i, miim_addr = detect->miim_addr_0; /* Try default MIIM address first */
    
    for (i = 0; i < 3; i++) {
        if (i) {
            /* Skip modules already detected at address 1/3 */
            if (detect->old_found[i - 1]) 
                continue;
            miim_addr = (i == 1 ? 1 : 3);
        }
        if (jr1_mmd_rd(1, miim_addr, 30, 0, &model, board_info) == VTSS_RC_OK) {
            if (model == 0x8484 || model == 0x8487 || model == 0x8488 ||
                model == 0x8489 || model == 0x8490 || model == 0x8491) {
                PRINTD(("found model 0x%x at port_no %u", model, detect->port_no));
                if (i) {
                    /* Found PHY at address 1/3 */
                    detect->old_found[i - 1] = 1;
                    detect->miim_addr_0 = miim_addr;
                    detect->miim_addr_1 = (miim_addr - 1);
                } else {
                    detect->miim_addr_0 = detect->miim_addr_1;
                    detect->miim_addr_1 = miim_addr;
                }
                
                detect->cap |= (PORT_CAP_VTSS_10G_PHY | PORT_CAP_AUTONEG | PORT_CAP_1G_FDX);
            }
            return;
        }
    }
}

// We supports 2 Tesla plug in modules. One with 4 ports, and one with 2 ports. This function auto-detect the number of port for the current plug ing module.
u8 jr_telsa_phy_detect_ports(u8 ctrl, u8 miim_addr, vtss_board_info_t *board_info)
{
   u16 data;
   if (jr1_miim_rd(ctrl, miim_addr, 3, &data, board_info) == VTSS_RC_OK) {
       if (((data >> 4) & 0x3F) == 0x0D) { // VSC8572 2 ports chip 
           return 2;
       }
       return 4; // Default all other Tesla chips has 4 ports
   }
   return 0;
}

BOOL jr1_1g_phy_detect(u8 ctrl, u8 miim_addr, vtss_board_info_t *board_info)
{
    u16 data;

    if (jr1_miim_rd(ctrl, miim_addr, 3, &data, board_info) != VTSS_RC_OK) {
        return FALSE;
    }
    return TRUE;
}

/* Calculate port capabilities for board and port */
static port_cap_t jr1_port_cap(int board_type, vtss_port_no_t port_no)
{
    port_cap_t cap;
    u32        cnt_1g, cnt_2g5 = 0, cnt_10g = 4;
    u32        fc_mask = (0xffffffff - PORT_CAP_FLOW_CTRL);

#if (VTSS_PORTS > 24)
    cnt_1g = 24;
#else
    cnt_1g = 12; /* LynX-1 */
#endif

    /* Default capabilities for 1G ports */
    cap = PORT_CAP_TRI_SPEED_COPPER;
    switch (board_type) {
    case VTSS_BOARD_JAG_CU24_REF: 
    case VTSS_BOARD_JAG_PCB107_REF: 
        break;
    case VTSS_BOARD_JAG_CU48_REF:
        cnt_1g = 48;
        break;
    case VTSS_BOARD_JAG_SFP24_REF: 
        /* Flow control is not supported for SFP boards (SGMII aneg does not support FC) */
        cap = ((PORT_CAP_SFP_1G | PORT_CAP_SFP_SD_HIGH) & fc_mask);
        break;
    default:
        return 0;
    }

#if defined(VTSS_SW_OPTION_PORT_MUX)
    /* Mux mode 1/7 adds 2.5G ports at the cost of 1G and 10G ports */


    // Hack when Tesla module is detected (Work around for Buzilla#7919). This will NOT work for a stacking solution, and is only working for the 24 ports JR boards
    if (port_tesla != VTSS_PORT_NO_NONE) {
        cnt_1g = 27;
        cnt_2g5 = 3;
        cnt_10g = 0;
    } else if (cnt_1g != 48) {
#if (VTSS_SW_OPTION_PORT_MUX == 1)
        cnt_2g5 = 4;
#else
        cnt_2g5 = 8;
#endif /* VTSS_SW_OPTION_PORT_MUX == 1 */
        if (cnt_1g == 12) {
            cnt_1g -= (cnt_2g5 - 4); /* LynX-1: The first 4x2.5G ports come for free */
        } else {
            cnt_1g -= (cnt_2g5 - 3); /* The first 4x2.5G ports cost one 1G port */
        }
        cnt_10g -= (cnt_2g5/4);
    }
#endif /* VTSS_SW_OPTION_PORT_MUX */
    if (port_no < cnt_1g) {
        /* 1G port, use defaults */
    } else if (port_no < (cnt_1g + cnt_2g5)) {
        /* 2.5G port */
        cap = (PORT_CAP_AUTONEG | PORT_CAP_100M_FDX | PORT_CAP_1G_FDX | PORT_CAP_2_5G_FDX | PORT_CAP_FLOW_CTRL | 
               PORT_CAP_SFP_SD_HIGH);
    } else if (port_no < (cnt_1g + cnt_2g5 + cnt_10g)) {
        /* 10G port */
        cap = (PORT_CAP_AUTONEG | PORT_CAP_1G_FDX | PORT_CAP_10G_FDX | PORT_CAP_FLOW_CTRL | PORT_CAP_XAUI_LANE_FLIP | 
               PORT_CAP_STACKING);
    } else {
        /* NPI port */
        cap = PORT_CAP_TRI_SPEED_COPPER;
    }
    
#if defined(VTSS_ARCH_JAGUAR_1_CE_SWITCH)
    /* Flow control not supported for CE */ 
    cap &= fc_mask;  
#endif
    
    return cap;
}

/* Number of ports on JAG_CU24 and JAG_SFP24 boards */
#define JR1_REF_PORT_CNT 29

/* Common probe function for all Jaguar-1 reference boards */
BOOL vtss_board_probe_jr1(vtss_board_t *board, vtss_board_info_t *board_info)
{
    /*lint -esym(459, i2c_read, board_type, jr1_port_count) */
    u32             value, mask, port_count;
    u16             reg;
    vtss_port_no_t  port_no;
    // A pulldown has been added to VSC7460 GPIO_21 - this signal was pulled up by VSC7460 internal pullup on PCB082/PCB083 so can be used to distinguish PCB107 from these.
    BOOL gpio_val = FALSE;
    u32 gpio_oe;
    u32 gpio_oe_masked;
    u32 gpio_oe_mask;
    u32 gpio_alt0,gpio_alt1, gpio_alt0_masked;
    
    PRINTD(("enter"));

    // Read the current GPIO setting in order to change GPIO21
    (void) jr_rd(JR1_TGT_GCB, JR1_ADDR_GPIO_ALT0, &gpio_alt0, board_info);
    (void) jr_rd(JR1_TGT_GCB, JR1_ADDR_GPIO_ALT1, &gpio_alt1, board_info);
    (void) jr_rd(JR1_TGT_GCB, JR1_ADDR_GPIO_OE, &gpio_oe, board_info);
    
    // Mask bit GPIO 21
    gpio_oe_mask = ~(1 << 21); 
    gpio_oe_masked = gpio_oe & gpio_oe_mask;
    gpio_alt0_masked = gpio_alt0 & gpio_oe_mask;
    
    // Setup GPIO with new GPIO21 setting and read the value
    (void) jr_wr(JR1_TGT_GCB, JR1_ADDR_GPIO_OE, gpio_oe_masked, board_info);
    (void) jr_wr(JR1_TGT_GCB, JR1_ADDR_GPIO_ALT0, gpio_alt0_masked, board_info);
    (void) jr_rd(JR1_TGT_GCB, JR1_ADDR_GPIO_IN, &value, board_info);
    gpio_val = (value & (1 << 21)) ? TRUE : FALSE;
    
    //Restore the original GPIO settings
    (void) jr_wr(JR1_TGT_GCB, JR1_ADDR_GPIO_OE, gpio_oe, board_info);
    (void) jr_wr(JR1_TGT_GCB, JR1_ADDR_GPIO_ALT0, gpio_alt0, board_info);


    mask = (1 << 12); /* GPIO_12 is used for PHY reset */
    if (jr_rd(JR1_TGT_GCB, JR1_ADDR_GPIO_OE, &value, board_info) == VTSS_RC_OK &&
        (value & mask) == 0 &&
        /* Not driving GPIO_12, must reset PHYs */
        jr_wr(JR1_TGT_GCB, JR1_ADDR_GPIO_OE, mask, board_info) == VTSS_RC_OK &&
        jr_wr(JR1_TGT_GCB, JR1_ADDR_GPIO_OUT, 0, board_info) == VTSS_RC_OK &&
        jr_wr(JR1_TGT_GCB, JR1_ADDR_GPIO_OUT, mask, board_info) == VTSS_RC_OK) {
        VTSS_MSLEEP(500);
    }

    if (jr1_miim_rd(0, 0, 2, &reg, board_info) != VTSS_RC_OK || reg != 0x0007) {
        /* If a Vitesse PHY is not found, it is assumed to be JAG_SFP24 board */
        PRINTD(("JAG_SFP24 found"));
        board_info->board_type = VTSS_BOARD_JAG_SFP24_REF;
        port_count = JR1_REF_PORT_CNT;
    } else if (jr1_miim_rd(0, 0, 3, &reg, board_info) == VTSS_RC_OK && ((reg & 0xFFF0) == 0x06e0)) {
        /* If an Atom12 PHY is found, it is assumed to be a JAG_CU48 board */
        PRINTD(("JAG_CU48 found"));
        board_info->board_type = VTSS_BOARD_JAG_CU48_REF;
        port_count = 53;
        board_info->base_addr_1 = (u32 *)0x5C000000;
    } else {
      if (gpio_val) {
        PRINTD(("VTSS_BOARD_JAG_CU24_REF board found"));
        board_info->board_type = VTSS_BOARD_JAG_CU24_REF;
      } else {
        PRINTD(("VTSS_BOARD_JAG_PCB107_REF board found"));
        board_info->board_type = VTSS_BOARD_JAG_PCB107_REF;
      }
      port_count = JR1_REF_PORT_CNT;
    }
    board->type = board_info->board_type;
    
    /* Override target */
    if (port_count == JR1_REF_PORT_CNT) {
        switch (board_info->target) {
        case VTSS_TARGET_E_STAX_III_24_DUAL:
            board_info->target = VTSS_TARGET_E_STAX_III_48;
            break;
        case VTSS_TARGET_E_STAX_III_68_DUAL:
            board_info->target = VTSS_TARGET_E_STAX_III_68;
            break;
        default:
            /* For single device targets, we do not want to override the target 
               and port count. For example to support mux mode 1 (31 ports) */
            port_count = board_info->port_count;
            break;
        }
        /* Override port count */
        if (port_count < board_info->port_count)
            board_info->port_count = port_count;
    }

    /* Initialize all ports to unused */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORTS; port_no++)
        jr1_port_table[port_no].map.chip_port = CHIP_PORT_UNUSED;
    board->custom_port_table = jr1_port_table;

    board->port_cap = jr1_port_cap;
    if (board_info->board_type == VTSS_BOARD_JAG_CU48_REF)
        jr1_board_setup_cu48(board, board_info);
    else
        jr1_board_setup_ref(board, board_info);


    PRINTD(("exit"));

    return TRUE;
}

void jr1_board_setup_cu48(vtss_board_t *board, vtss_board_info_t *board_info)
{
    u32                    port;
    vtss_chip_no_t         chip_no;
    vtss_miim_controller_t miim_bus = VTSS_MIIM_CONTROLLER_0;
    port_cap_t             cap;
    vtss_port_interface_t  mac_if = VTSS_PORT_INTERFACE_SGMII;
    vtss_port_no_t         port_no;
    jr1_10g_detect_t       detect;

    i2c_read   = board_info->i2c_read;
    
    board->name = "Jaguar-1 CU48 Reference"; 
    board->features = (VTSS_BOARD_FEATURE_AMS | 
                       VTSS_BOARD_FEATURE_LOS |  
                       VTSS_BOARD_FEATURE_POE |
                       VTSS_BOARD_FEATURE_STACKING |
                       VTSS_BOARD_FEATURE_VCXO);
    board->chipcount = 2;
    board->init = jr1_init;
    board->reset = jr1_reset;
    board->pre_reset = jr1_pre_reset;
    board->post_reset = jr1_post_reset;
    board->led_init = jr1_led_init;
    board->led_update = jr1_led_update;
    board->sfp_update_if = jr1_sfp_update_if;
    board->sfp_i2c_read = jr1_sfp_i2c_read;
    board->sfp_i2c_enable = jr1_sfp_i2c_enable;
    board->port_conf = jr1_port_conf;
    
    /* Port 1-48: Cu ports */
    cap = jr1_port_cap(board->type, VTSS_PORT_NO_START);
    for (port_no = VTSS_PORT_NO_START; port_no < 48; port_no++) {
        if (port_no < 24) {
            /* First device */
            chip_no = 0;
            port = port_no;
        } else {
            /* Second device */
            chip_no = 1;
            port = (port_no - 24);
        } 
        jr1_map_update(port_no, chip_no, port, miim_bus, port, chip_no, mac_if, cap);
    }

    /* X2 ports, all at the second MIIM bus on the master chip */
    chip_no = 0;
    miim_bus = VTSS_MIIM_CONTROLLER_1;
    mac_if = VTSS_PORT_INTERFACE_XAUI;
    cap = jr1_port_cap(board->type, port_no);
    
    /* Detect 10G PHY */
    detect.port_no = port_no;
    detect.miim_addr_0 = 26;
    detect.miim_addr_1 = 27;
    detect.old_found[0] = 0;
    detect.old_found[1] = 0;
    detect.cap = cap;
    jr1_10g_detect(&detect, board_info);
    
    /* Port 49: XAUI_2 on the second device (left-most X2 port seen from the back) */
    board->default_stackport_b = port_no;
    jr1_map_update(port_no, 1, 29, miim_bus, detect.miim_addr_0, chip_no, mac_if, detect.cap);
    port_no++;

    /* Port 50: XAUI_3 on the second device */
    jr1_map_update(port_no, 1, 30, miim_bus, detect.miim_addr_1, chip_no, mac_if, detect.cap);
    port_no++;

    /* Detect 10G PHY */
    detect.port_no = port_no;
    detect.miim_addr_0 = 24;
    detect.miim_addr_1 = 25;
    detect.cap = cap;
    jr1_10g_detect(&detect, board_info);

    /* Port 51: XAUI_2 on the first device */
    board->default_stackport_a = port_no;
    jr1_map_update(port_no, 0, 29, miim_bus, detect.miim_addr_0, chip_no, mac_if, detect.cap);
    port_no++;

    /* Port 51: XAUI_3 on the first device (right-most X2 port seen from the back) */
    jr1_map_update(port_no, 0, 30, miim_bus, detect.miim_addr_1, chip_no, mac_if, detect.cap);
    port_no++;

    /* Port 53: NPI port on first device */
    miim_bus = VTSS_MIIM_CONTROLLER_0;
    mac_if = VTSS_PORT_INTERFACE_SGMII;
    cap = jr1_port_cap(board->type, port_no);
    jr1_map_update(port_no, 0, JR1_NPI_CHIP_PORT, miim_bus, 28, 0, mac_if, cap);
}

#endif /* defined(VTSS_ARCH_JAGUAR_1) */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
