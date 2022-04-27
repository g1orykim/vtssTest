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

// Avoid "*.h not used in module port_custom_lu28.c"
/*lint --e{766} */
#include "board_probe.h"

#if defined(VTSS_ARCH_LUTON28)
#include "board_probe.h"

static int board_type;

#ifdef VTSS_SW_OPTION_BOARD
#include "ser_gpio.h"
#endif

#if defined(VTSS_CHIP_SPARX_II_16)
#define PORT_OFFSET 8
#else
#define PORT_OFFSET 0
#endif /* VTSS_CHIP_SPARX_II_16 */

/*lint -esym(459, lu28_port_table) */
static port_custom_entry_t lu28_port_table[VTSS_PORT_ARRAY_SIZE] = {
#if !defined(VTSS_CHIP_SPARX_II_16)
    /* Port 1 */
    [0] = {
        .map.chip_port = 0,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_0,
        .map.miim_addr = 0,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 2 */
    [1] = {
        .map.chip_port = 1,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_0,
        .map.miim_addr = 1,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 3 */
    [2] = {
        .map.chip_port = 2,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_0,
        .map.miim_addr = 2,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 4 */
    [3] = {
        .map.chip_port = 3,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_0,
        .map.miim_addr = 3,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 5 */
    [4] = {
        .map.chip_port = 4,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_0,
        .map.miim_addr = 4,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 6 */
    [5] = {
        .map.chip_port = 5,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_0,
        .map.miim_addr = 5,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 7 */
    [6] = {
        .map.chip_port = 6,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_0,
        .map.miim_addr = 6,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 8 */
    [7] = {
        .map.chip_port = 7,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_0,
        .map.miim_addr = 7,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },
#endif /* !VTSS_CHIP_SPARX_II_16 */

    /* Port 9 */
    [8 - PORT_OFFSET] = {
        .map.chip_port = 8,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 8,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 10 */
    [9 - PORT_OFFSET] = {
        .map.chip_port = 9,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 9,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 11 */
    [10 - PORT_OFFSET] = {
        .map.chip_port = 10,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 10,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 12 */
    [11 - PORT_OFFSET] = {
        .map.chip_port = 11,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 11,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 13 */
    [12 - PORT_OFFSET] = {
        .map.chip_port = 12,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 12,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 14 */
    [13 - PORT_OFFSET] = {
        .map.chip_port = 13,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 13,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 15 */
    [14 - PORT_OFFSET] = {
        .map.chip_port = 14,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 14,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 16 */
    [15 - PORT_OFFSET] = {
        .map.chip_port = 15,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 15,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 17 */
    [16 - PORT_OFFSET] = {
        .map.chip_port = 16,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 16,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 18 */
    [17 - PORT_OFFSET] = {
        .map.chip_port = 17,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 17,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 19 */
    [18 - PORT_OFFSET] = {
        .map.chip_port = 18,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 18,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 20 */
    [19 - PORT_OFFSET] = {
        .map.chip_port = 19,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 19,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER
    },

    /* Port 21 */
    [20 - PORT_OFFSET] = {
        .map.chip_port = 20,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 20,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER /* Enzo => 3speed Dual Fiber */
    },

    /* Port 22 */
    [21 - PORT_OFFSET] = {
        .map.chip_port = 21,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 21,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER /* Enzo => 3speed Dual Fiber */
    },

    /* Port 23 */
    [22 - PORT_OFFSET] = {
        .map.chip_port = 22,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 22,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER /* Enzo => 3speed Dual Fiber */
    },

    /* Port 24 */
    [23 - PORT_OFFSET] = {
        .map.chip_port = 23,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_1,
        .map.miim_addr = 23,
        .mac_if = VTSS_PORT_INTERFACE_SGMII,
        .cap = PORT_CAP_TRI_SPEED_COPPER /* Enzo => 3speed Dual Fiber */
    },

#if (VTSS_PORTS > 24)
#if VTSS_OPT_INT_AGGR
    /* Port 25 */
    [24] = {
        .map.chip_port = 24,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_NONE,
        .map.miim_addr = (u8) -1,
        .mac_if = VTSS_PORT_INTERFACE_VAUI,
        .cap = PORT_CAP_5G_FDX | PORT_CAP_FLOW_CTRL /* Enzo-SFP |= SD_Enb */
    },

    /* Port 26 */
    [25] = {
        .map.chip_port = 26,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_NONE,
        .map.miim_addr = (u8) -1,
        .mac_if = VTSS_PORT_INTERFACE_VAUI,
        .cap = PORT_CAP_5G_FDX | PORT_CAP_FLOW_CTRL /* Enzo-SFP |= SD_Enb */
    }
#else
    /* Port 25 */
    [24] = {
        .map.chip_port = 24,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_NONE,
        .map.miim_addr = (u8) -1,
        .mac_if = VTSS_PORT_INTERFACE_VAUI,
        .cap = PORT_CAP_2_5G_FDX | PORT_CAP_1G_FDX | PORT_CAP_FLOW_CTRL /* Enzo-SFP |= SD_Enb+autoneg */
    },

    /* Port 26 */
    [25] = {
        .map.chip_port = 25,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_NONE,
        .map.miim_addr = (u8) -1,
        .mac_if = VTSS_PORT_INTERFACE_VAUI,
        .cap = PORT_CAP_2_5G_FDX | PORT_CAP_1G_FDX | PORT_CAP_FLOW_CTRL /* Enzo-SFP |= SD_Enb+autoneg */
    },

    /* Port 27 */
    [26] = {
        .map.chip_port = 26,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_NONE,
        .map.miim_addr = (u8) -1,
        .mac_if = VTSS_PORT_INTERFACE_VAUI,
        .cap = PORT_CAP_2_5G_FDX | PORT_CAP_1G_FDX | PORT_CAP_FLOW_CTRL /* Enzo+stack |= SD_Enb+autoneg */
    },

    /* Port 28 */
    [27] = {
        .map.chip_port = 27,
        .map.miim_controller = VTSS_MIIM_CONTROLLER_NONE,
        .map.miim_addr = (u8) -1,
        .mac_if = VTSS_PORT_INTERFACE_VAUI,
        .cap = PORT_CAP_2_5G_FDX | PORT_CAP_1G_FDX | PORT_CAP_FLOW_CTRL /* Enzo+stack |= SD_Enb+autoneg */
    }
#endif /* VTSS_OPT_INT_AGGR */
#endif /* VTSS_PORTS > 24 */

};

/* Release ports from reset */
static vtss_rc lu28_reset(void)
{
    /* Release PHYs from reset (Switch GPIO9) */
    (void) vtss_gpio_direction_set(NULL, 0, 9, 1);
    (void) vtss_gpio_write(NULL, 0, 9, 0);
    (void) vtss_gpio_write(NULL, 0, 9, 1);
    VTSS_MSLEEP(500);

    return VTSS_RC_OK;
}

/* Post ports reset */
static vtss_rc lu28_post_reset(void)
{
    return VTSS_RC_OK;
}


/* Initialize port LEDs */
static vtss_rc lu28_led_init(void)
{
    vtss_port_no_t        port_no;
    const vtss_port_map_t *port_map;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port_map = &lu28_port_table[port_no].map;
        if (port_map->miim_controller == VTSS_MIIM_CONTROLLER_NONE)
            continue;
        
        if (port_map->miim_addr == 8 || port_map->miim_addr == 16) {
            /* Make PHY GPIO0 and GPIO1 output for VAUI port LED control */
            (void) vtss_phy_write(NULL, port_no, 13 | VTSS_PHY_REG_GPIO, 0xffff);
            (void) vtss_phy_write(NULL, port_no, 17 | VTSS_PHY_REG_GPIO, 0x0003);
        }
        
        /* Tri speed port LED control */
        if(board_type == VTSS_BOARD_ESTAX_34_ENZO ||
           board_type == VTSS_BOARD_ESTAX_34_ENZO_SFP) {
            if (lu28_port_table[port_no].cap & (PORT_CAP_DUAL_COPPER | PORT_CAP_DUAL_FIBER)) {
                (void) vtss_phy_write(NULL, port_no, 29, 0x7e64);
                (void) vtss_phy_write(NULL, port_no, 17 | VTSS_PHY_REG_GPIO, 0xF000);
            } else {
                (void) vtss_phy_write(NULL, port_no, 29, 0xee64);
            }
        } else {
            (void) vtss_phy_write(NULL, port_no, 29, 0xee64);
        }
    }
    
    return VTSS_RC_OK;
}

/* Update port LED */
static vtss_rc lu28_led_update(vtss_port_no_t port_no, 
                               vtss_port_status_t *status,
                               vtss_port_counters_t *counters,
                               port_custom_conf_t *port_conf)
{
    if(board_type == VTSS_BOARD_ESTAX_34_ENZO_SFP) {
        /*lint -esym(459, rx_old, mode_24, mode_25) */
        static vtss_port_counter_t rx_old[VTSS_PORT_ARRAY_SIZE];
        static vtss_led_mode_t     mode_24 = VTSS_LED_MODE_IGNORE;
        static vtss_led_mode_t     mode_25 = VTSS_LED_MODE_IGNORE;
        vtss_port_counter_t        rx_new;
        vtss_led_mode_t            led_mode[3], mode;
        vtss_led_port_t            led_port;
        int                        chip_port;
        chip_port = lu28_port_table[port_no].map.chip_port;
        if (chip_port >= 24) {
            /* Calculate LED mode based on port status and counters */
            rx_new = counters->rmon.rx_etherStatsPkts;
            mode = (status->link_down ? VTSS_LED_MODE_OFF :
                    (rx_old[port_no] == rx_new ? VTSS_LED_MODE_ON : VTSS_LED_MODE_2_5));
            rx_old[port_no] = rx_new;
        
            /* Update based on RBM0027 table 3 */
            switch (chip_port) {
            case 24:
            case 25:
                led_port = 27;
                if (chip_port == 24)
                    mode_24 = mode;
                else
                    mode_25 = mode;

                led_mode[0] = VTSS_LED_MODE_IGNORE;
                led_mode[1] = mode_24;
                led_mode[2] = mode_25;
                break;
            case 26:
                led_port = 28;
                led_mode[0] = mode;
                led_mode[1] = VTSS_LED_MODE_IGNORE;
                led_mode[2] = VTSS_LED_MODE_IGNORE;
                break;
            case 28:
            default:
                led_port = 29;
                led_mode[0] = mode;
                led_mode[1] = VTSS_LED_MODE_IGNORE;
                led_mode[2] = VTSS_LED_MODE_IGNORE;
                break;
            }
            (void) vtss_serial_led_set(NULL, led_port, led_mode);
        }
    } else {
        int     chip_port;
        BOOL    phy_port;
        chip_port = lu28_port_table[port_no].map.chip_port;
        if (chip_port == 24 || chip_port == 26) {
            /* Update stack port LEDs using PHY GPIOs */
            phy_port = lu28_port_table[chip_port == 24 ? 9 : 17].map.miim_controller == VTSS_MIIM_CONTROLLER_NONE ? 0 : 1;
            if(phy_port) {
                (void) vtss_phy_write(NULL, (chip_port == 24 ? 9 : 17) - PORT_OFFSET, 
                                      16 | VTSS_PHY_REG_GPIO, 
                                      (1<<1) | ((status->link ? 0 : 1)<<0));
            }
        }
    }
    return VTSS_RC_OK;
}


// Function for doing special port configuration that depends upon the platform
// Forexample do the enzo board requires that if the stack ports uses SFPs, the SFPs must be turn on 
// using a special serialised GPIO system.
static void lu28_port_conf(vtss_port_no_t port, 
                           port_custom_conf_t *port_conf, 
                           vtss_port_status_t *port_status)
{
    if(board_type == VTSS_BOARD_ESTAX_34_ENZO_SFP) {
        // Determine which fiber port to enable depending upon if they are arggregated or not.
#if VTSS_OPT_INT_AGGR
        // See VSC7407_VSC86x4_Hardware_Manual section 2.6.3
        int SFP_TX_ENA_BIT_POS[2] = {0x24,0x900}; 
#else
        // See VSC7407_VSC86x4_Hardware_Manual section 2.6.3
        int SFP_TX_ENA_BIT_POS[4] = {0x4,0x20,0x100,0x800}; 
#endif  /* VTSS_OPT_INT_AGGR */
        if (lu28_port_table[port].map.chip_port >= 24) {
            int sfp_index = lu28_port_table[port].map.chip_port -24;
            if (port_conf->enable) {
                serialized_gpio_data_wr(SFP_TX_ENA_BIT_POS[sfp_index],SFP_TX_ENA_BIT_POS[sfp_index]);
            } else {
                serialized_gpio_data_wr(1,SFP_TX_ENA_BIT_POS[sfp_index]);
            }
        }
    }
}

// Function for doing special port initialization that depends upon the platform
// Forexample do the enzo board requires that if the stack ports uses SFPs, the GPIO 12-15 
// must be configured to use the LOS overlay functions.
static void lu28_init(void)
{
    if(board_type == VTSS_BOARD_ESTAX_34_ENZO_SFP) {
        (void) vtss_gpio_mode_set(NULL, 0, 12, VTSS_GPIO_ALT_0);
        (void) vtss_gpio_mode_set(NULL, 0, 13, VTSS_GPIO_ALT_0);
        (void) vtss_gpio_mode_set(NULL, 0, 14, VTSS_GPIO_ALT_0);
        (void) vtss_gpio_mode_set(NULL, 0, 15, VTSS_GPIO_ALT_0);
    }
}

static inline int get_board_type(int configured_board_type)
{
    u16 oui, ident;

#if !VTSS_OPT_INT_AGGR
    if(configured_board_type == VTSS_BOARD_UNKNOWN)
        return VTSS_BOARD_ESTAX_34_ENZO_SFP;
#endif

    if(configured_board_type == VTSS_BOARD_ESTAX_34_REF ||
       configured_board_type == VTSS_BOARD_ESTAX_34_ENZO ||
       configured_board_type == VTSS_BOARD_ESTAX_34_ENZO_SFP)
        return configured_board_type; /* Trust configured board type */
 
    /* Read OUI on the first port */
    if(vtss_miim_read(NULL, 0, VTSS_MIIM_CONTROLLER_0, 0, 2, &oui) != VTSS_RC_OK ||
       oui != 0x0007)
        (void) lu28_reset();    /* Try to (un)-reset PHYs */

    /* Read OUI and ID on the first port */
    if (vtss_miim_read(NULL, 0, VTSS_MIIM_CONTROLLER_0, 0, 2, &oui) == VTSS_RC_OK &&
        vtss_miim_read(NULL, 0, VTSS_MIIM_CONTROLLER_0, 0, 3, &ident) == VTSS_RC_OK) {
        ident = ((ident >> 4) & 0x3f);
        if(oui == 0x0007 && (ident == 0x28 || ident == 0x35 || ident == 0x38)) /* VSC8558 Spyder */
            return VTSS_BOARD_ESTAX_34_REF;
        if(oui == 0x0007 && ident == 0x24) /* VSC8634 Enzo */
            return VTSS_BOARD_ESTAX_34_ENZO;
    }
    return VTSS_BOARD_UNKNOWN;
}

/*
 * E-StaX-32 board probe function.
 */
BOOL vtss_board_probe_lu28(vtss_board_t *board, vtss_board_info_t *board_info)
{
    /*lint -esym(459, board_type) */
    memset(board, 0, sizeof(*board));

    if((board->type = board_type = get_board_type(board_info->board_type)) != VTSS_BOARD_UNKNOWN) {
        board_info->board_type = board_type;
        board->features = 0;
        switch(board->type) {
        case VTSS_BOARD_ESTAX_34_REF: 
            board->name = "E-StaX-34 Reference"; 
            break;
        case VTSS_BOARD_ESTAX_34_ENZO: 
            board->name = "E-StaX-34 Enzo"; 
            board->features |= 
                VTSS_BOARD_FEATURE_AMS |
                VTSS_BOARD_FEATURE_LOS |
                VTSS_BOARD_FEATURE_POE;
            break;
        case VTSS_BOARD_ESTAX_34_ENZO_SFP: 
            board->name = "E-StaX-34 Enzo SFP"; 
            board->features |= 
                VTSS_BOARD_FEATURE_AMS |
                VTSS_BOARD_FEATURE_LOS |
                VTSS_BOARD_FEATURE_POE;
            break;
        default:  board->name = "Unknown";
        }
        board->custom_port_table = lu28_port_table;

        board->init = lu28_init;
        board->reset = lu28_reset;
        board->post_reset = lu28_post_reset;
        board->port_conf = lu28_port_conf;
        board->led_init = lu28_led_init;
        board->led_update = lu28_led_update;

        if(board->type == VTSS_BOARD_ESTAX_34_ENZO ||
           board->type == VTSS_BOARD_ESTAX_34_ENZO_SFP) {
            vtss_port_no_t port_idx;
            for(port_idx = (20 - PORT_OFFSET); port_idx < (24 - PORT_OFFSET); port_idx++)
                lu28_port_table[port_idx].cap = PORT_CAP_TRI_SPEED_DUAL_FIBER;
            port_cap_t excap = (board->type == VTSS_BOARD_ESTAX_34_ENZO_SFP ?
                                PORT_CAP_SD_ENABLE : PORT_CAP_NONE);
#if VTSS_OPT_INT_AGGR
            for(port_idx = 24; port_idx < 26; port_idx++)
                lu28_port_table[port_idx].cap |= excap;
#else
            for(port_idx = 24; port_idx < 28; port_idx++)
                lu28_port_table[port_idx].cap |= (excap | PORT_CAP_AUTONEG);
#endif  /* VTSS_OPT_INT_AGGR */
        }

#if defined(VTSS_OPT_STACK_A_MASK) && defined(VTSS_OPT_STACK_B_MASK)
        board->features |= VTSS_BOARD_FEATURE_STACKING;
        board->default_stackport_a = board->default_stackport_b = VTSS_PORT_NO_NONE;
        if(board->features & VTSS_BOARD_FEATURE_STACKING) {
            vtss_port_no_t port_no;
            u32 mask;
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                mask = (1 << lu28_port_table[port_no].map.chip_port);
                if (mask & VTSS_OPT_STACK_A_MASK && board->default_stackport_a == VTSS_PORT_NO_NONE) {
                    lu28_port_table[port_no].cap |= PORT_CAP_STACKING;
                    board->default_stackport_a = port_no;
                }
                if (mask & VTSS_OPT_STACK_B_MASK && board->default_stackport_b == VTSS_PORT_NO_NONE) {
                    lu28_port_table[port_no].cap |= PORT_CAP_STACKING;
                    board->default_stackport_b = port_no;
                }
            }
        }
#endif

        return TRUE;
    }
    return FALSE;
}

#endif /* defined(VTSS_ARCH_LUTON28) */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
