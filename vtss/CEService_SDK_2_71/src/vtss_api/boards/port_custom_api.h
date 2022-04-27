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
 
 $Id$
 $Revision$

*/

#ifndef _VTSS_PORT_CUSTOM_H_
#define _VTSS_PORT_CUSTOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <vtss_api.h>

/* ================================================================= *
 *  Port capabilities
 * ================================================================= */

/* Port capabilities */
#define PORT_CAP_NONE              0x00000000 /* No capabilities */
#define PORT_CAP_AUTONEG           0x00000001 /* Auto negotiation */
#define PORT_CAP_10M_HDX           0x00000002 /* 10 Mbps, half duplex */
#define PORT_CAP_10M_FDX           0x00000004 /* 10 Mbps, full duplex */
#define PORT_CAP_100M_HDX          0x00000008 /* 100 Mbps, half duplex */
#define PORT_CAP_100M_FDX          0x00000010 /* 100 Mbps, full duplex */
#define PORT_CAP_1G_FDX            0x00000020 /* 1 Gbps, full duplex */
#define PORT_CAP_2_5G_FDX          0x00000040 /* 2.5 Gbps, full duplex */
#define PORT_CAP_5G_FDX            0x00000080 /* 5Gbps, full duplex */
#define PORT_CAP_10G_FDX           0x00000100 /* 10Gbps, full duplex */
#define PORT_CAP_FLOW_CTRL         0x00001000 /* Flow control */
#define PORT_CAP_COPPER            0x00002000 /* Copper media */
#define PORT_CAP_FIBER             0x00004000 /* Fiber media */
#define PORT_CAP_DUAL_COPPER       0x00008000 /* Dual media, copper preferred */
#define PORT_CAP_DUAL_FIBER        0x00010000 /* Dual media, fiber preferred */
#define PORT_CAP_SD_ENABLE         0x00020000 /* Signal Detect enabled */
#define PORT_CAP_SD_HIGH           0x00040000 /* Signal Detect active high */
#define PORT_CAP_SD_INTERNAL       0x00080000 /* Signal Detect select internal */
#define PORT_CAP_DUAL_FIBER_100FX  0x00100000 /* Dual media (Fiber = 100FX), fiber preferred */
#define PORT_CAP_XAUI_LANE_FLIP    0x00200000 /* Flip the XAUI lanes */
#define PORT_CAP_VTSS_10G_PHY      0x00400000 /* Connected to VTSS 10G PHY */
#define PORT_CAP_SFP_DETECT        0x00800000 /* Auto detect the SFP module */
#define PORT_CAP_STACKING          0x01000000 /* Stack port candidate */
#define PORT_CAP_DUAL_SFP_DETECT   0x02000000 /* Auto detect the SFP module for dual media*/
#define PORT_CAP_SFP_ONLY          0x04000000 /* SFP only port (not dual media)*/
#define PORT_CAP_DUAL_COPPER_100FX 0x08000000 /* Dual media (Fiber = 100FX), copper preferred */

/* Half duplex */
#define PORT_CAP_HDX (PORT_CAP_10M_HDX | PORT_CAP_100M_HDX)

/* Tri-speed port */
#define PORT_CAP_TRI_SPEED_FDX (PORT_CAP_AUTONEG | PORT_CAP_1G_FDX | PORT_CAP_100M_FDX | PORT_CAP_10M_FDX | PORT_CAP_FLOW_CTRL)
#define PORT_CAP_TRI_SPEED (PORT_CAP_TRI_SPEED_FDX | PORT_CAP_HDX)

/* 1G PHY present */
#define PORT_CAP_1G_PHY (PORT_CAP_COPPER | PORT_CAP_FIBER | PORT_CAP_DUAL_COPPER | PORT_CAP_DUAL_FIBER | PORT_CAP_DUAL_FIBER_100FX)

/* Tri-speed port with specific media */
#define PORT_CAP_TRI_SPEED_COPPER            (PORT_CAP_TRI_SPEED | PORT_CAP_COPPER)
#define PORT_CAP_TRI_SPEED_FIBER             (PORT_CAP_TRI_SPEED | PORT_CAP_FIBER)
#define PORT_CAP_TRI_SPEED_DUAL_COPPER       (PORT_CAP_TRI_SPEED | PORT_CAP_DUAL_COPPER)
#define PORT_CAP_TRI_SPEED_DUAL_FIBER        (PORT_CAP_TRI_SPEED | PORT_CAP_DUAL_FIBER)
#define PORT_CAP_TRI_SPEED_DUAL_FIBER_100FX  (PORT_CAP_TRI_SPEED | PORT_CAP_DUAL_FIBER_100FX)
#define PORT_CAP_ANY_FIBER                   (PORT_CAP_FIBER | PORT_CAP_DUAL_FIBER_100FX | PORT_CAP_DUAL_FIBER | PORT_CAP_DUAL_COPPER | PORT_CAP_SFP_DETECT) /**< Any fiber mode */
#define PORT_CAP_SPEED_DUAL_ANY_FIBER_FIXED_SPEED       (PORT_CAP_DUAL_FIBER_100FX | PORT_CAP_DUAL_FIBER | PORT_CAP_DUAL_COPPER) /**< Any fiber mode, but auto detection not supported */
#define PORT_CAP_SPEED_DUAL_ANY_FIBER        (PORT_CAP_DUAL_COPPER | PORT_CAP_DUAL_FIBER| PORT_CAP_DUAL_FIBER_100FX | PORT_CAP_DUAL_SFP_DETECT)
#define PORT_CAP_TRI_SPEED_DUAL_ANY_FIBER    (PORT_CAP_TRI_SPEED | PORT_CAP_SPEED_DUAL_ANY_FIBER)
#define PORT_CAP_TRI_SPEED_DUAL_ANY_FIBER_FIXED_SFP_SPEED    (PORT_CAP_TRI_SPEED | PORT_CAP_SPEED_DUAL_ANY_FIBER_FIXED_SPEED) /**< Cu & Fiber, but SFP auto detection not supported */
#define PORT_CAP_FIBER_1000X                 (PORT_CAP_DUAL_FIBER | PORT_CAP_DUAL_COPPER) /**< 1000Base-X fiber mode */
#define PORT_CAP_FIBER_100FX                 (PORT_CAP_TRI_SPEED_DUAL_FIBER_100FX) /**< 100Base-FX fiber mode */



/* SFP fiber port 100FX/1G/2.5G with auto negotiation and flow control */
#define PORT_CAP_SFP_1G   (PORT_CAP_AUTONEG | PORT_CAP_100M_FDX | PORT_CAP_1G_FDX | PORT_CAP_FLOW_CTRL | PORT_CAP_SFP_ONLY)
#define PORT_CAP_SFP_2_5G (PORT_CAP_SFP_1G | PORT_CAP_2_5G_FDX)

#define PORT_CAP_SFP_SD_HIGH (PORT_CAP_SD_ENABLE | PORT_CAP_SD_HIGH | PORT_CAP_SD_INTERNAL | PORT_CAP_SFP_DETECT | PORT_CAP_SFP_ONLY)


typedef u32 port_cap_t;

typedef struct {
    vtss_port_map_t       map;    /* Port map */
    vtss_port_interface_t mac_if; /* MAC interface */
    port_cap_t            cap;    /* Port capabilities */
} port_custom_entry_t;

/* Disable advertisement during auto negotiation */
#define PORT_ADV_DIS_10M_HDX  0x01
#define PORT_ADV_DIS_10M_FDX  0x02
#define PORT_ADV_DIS_100M_HDX 0x04
#define PORT_ADV_DIS_100M_FDX 0x08
#define PORT_ADV_DIS_1G_FDX   0x10 
#define PORT_ADV_UP_MEP_LOOP  0x20 
#define PORT_ADV_DIS_100M     0x40 /* Disable 100Mbit mode*/
#define PORT_ADV_DIS_10M      0x80 /* Disable 10Mbit mode*/
#define PORT_ADV_DIS_ALL      0xff 

/* Maximum number of tags */
#define PORT_MAX_TAGS_ONE  0 /* Backward compatible default is one tag */
#define PORT_MAX_TAGS_NONE 1
#define PORT_MAX_TAGS_TWO  2 

/** \brief Fiber Port speed */
typedef enum
{
    VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED = 0, /**< Fiber not supported/ Fiber port disabled */
    VTSS_SPEED_FIBER_100FX = 2,       /**< 100BASE-FX*/
    VTSS_SPEED_FIBER_1000X = 3,       /**< 1000BASE-X*/
    VTSS_SPEED_FIBER_AUTO = 4,        /**< Auto detection*/
    VTSS_SPEED_FIBER_DISABLED = 5,    /**< Obsolete - use VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED instead*/
} vtss_fiber_port_speed_t;


/* Port configuration */
typedef struct {
    BOOL                  enable;       /* Admin enable/disable */
    BOOL                  autoneg;      /* Auto negotiation */
    BOOL                  fdx;          /* Forced duplex mode */
    BOOL                  flow_control; /* Flow control */
    vtss_port_speed_t     speed;        /* Forced port speed */
    vtss_fiber_port_speed_t dual_media_fiber_speed;/* Speed for dual media fiber ports*/
    unsigned int          max_length;   /* Max frame length */
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    vtss_phy_power_mode_t power_mode;   /* PHY power mode */
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
    BOOL                  exc_col_cont; /* Excessive collision continuation */
    u8                    adv_dis;      /* Auto neg advertisement disable */
    u8                    max_tags;     /* Maximum number of tags */
    BOOL                  oper_up;      /* Force operational state up */
} port_custom_conf_t;

/* The default power mode value */
#define CONF_POWER_MODE_DEFAULT VTSS_PHY_POWER_NOMINAL

/* Initialize board */
void port_custom_init(void);

/* Release ports from reset */
vtss_rc port_custom_reset(void);


/* Post ports reset */
vtss_rc post_port_custom_reset(void);

/* Initialize port LEDs */
vtss_rc port_custom_led_init(void);

/* Update port LED */
vtss_rc port_custom_led_update(vtss_port_no_t port_no, 
                               vtss_port_status_t *status,
                               vtss_port_counters_t *counters,
                               port_custom_conf_t *port_conf);


// Function for doing special port configuration that depends upon the platform
// Forexample do the enzo board requires that if the stack ports uses SFPs, the SFPs must be turn on 
// using a special serialised GPIO system.
void port_custom_conf(vtss_port_no_t port, 
                      port_custom_conf_t *port_conf, 
                      vtss_port_status_t *port_status);

/* Called when system is reset. */
void port_custom_pre_reset(void);

/* Functions for handling SFP modules */
vtss_rc board_sfp_i2c_lock(BOOL lock);
vtss_rc board_sfp_i2c_enable(vtss_port_no_t port_no);
vtss_rc board_sfp_update_if(vtss_port_no_t port_no, vtss_port_interface_t mac_if);
vtss_rc board_sfp_i2c_read(vtss_port_no_t port_no, u8 i2c_addr, u8 addr, u8 *const data, u8 cnt);
vtss_rc board_sfp_i2c_write(vtss_port_no_t port_no, u8 i2c_addr, u8 addr, u8 *const data);
vtss_rc port_custom_sfp_mod_detect(BOOL *detect_status);
BOOL port_custom_sfp_accept(u8 *sfp_rom);

typedef enum {
    VTSS_BOARD_UNKNOWN = 0,
    VTSS_BOARD_ESTAX_34_REF,
    VTSS_BOARD_ESTAX_34_ENZO,
    VTSS_BOARD_ESTAX_34_ENZO_SFP,
    VTSS_BOARD_LUTON10_REF,
    VTSS_BOARD_LUTON26_REF = 5,
    VTSS_BOARD_JAG_CU24_REF,
    VTSS_BOARD_JAG_SFP24_REF,
    VTSS_BOARD_JAG_PCB107_REF,
    VTSS_BOARD_UNUSED,          /* Vacant entry, used to be JAG_CU24_DUAL_REF (obsolete) */
    VTSS_BOARD_JAG_CU48_REF,
    VTSS_BOARD_SERVAL_REF,
    VTSS_BOARD_SERVAL_PCB106_REF
} vtss_board_type_t;

enum vtss_board_feature_e {
    VTSS_BOARD_FEATURE_AMS      = (1 << 0), /**< TBD */
    VTSS_BOARD_FEATURE_LOS      = (1 << 1), /**< Loss of Signal detect */
    VTSS_BOARD_FEATURE_POE      = (1 << 2), /**< Power Over Ethernet */
    VTSS_BOARD_FEATURE_VCXO     = (1 << 3), /**< Voltage-controlled oscillator */
    VTSS_BOARD_FEATURE_STACKING = (1 << 4), /**< Stacking support */
};

/* Board information for exchanging data between application and probe function */
typedef struct {
    int                      board_type;   /* Board type */
    vtss_target_type_t       target;       /* Target ID */
    u32                      port_count;   /* Number of ports */
    vtss_reg_read_t          reg_read;     /* Register read function */
    vtss_reg_write_t         reg_write;    /* Register write function */
    vtss_i2c_read_t          i2c_read;     /**< I2C read function */
    vtss_i2c_write_t         i2c_write;    /**< I2C write function */
    volatile u32             *base_addr_1; /* Second base address for access to secondary device */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
    vtss_serdes_macro_conf_t serdes;       /* Serdes settings for this board */
#endif
#endif
} vtss_board_info_t;

int vtss_board_probe(vtss_board_info_t *board_info, const port_custom_entry_t **port_custom_table);

const char *vtss_board_name(void);

vtss_board_type_t vtss_board_type(void);

u32 vtss_board_features(void);

u32 vtss_board_chipcount(void);

#if defined(VTSS_FEATURE_VSTAX)
u32 vtss_board_default_stackport(BOOL port_0);
#endif  /* VTSS_FEATURE_VSTAX */

port_cap_t vtss_board_port_cap(int board_type, vtss_port_no_t port_no);

void led_tower_update(void);

#ifdef __cplusplus
}
#endif

#endif /* _VTSS_PORT_CUSTOM_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
