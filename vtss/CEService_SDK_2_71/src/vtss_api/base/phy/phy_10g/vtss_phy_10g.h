/*

 Vitesse API software.

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

#ifndef _VTSS_PHY_10G_H_
#define _VTSS_PHY_10G_H_

/* MMD sublayers */
#define MMD_PMA     1
#define MMD_WIS     2
#define MMD_PCS     3
#define MMD_XS      4
#define MMD_NVR_DOM 30 // 8486
#define MMD_GLOBAL  30 // 8484/8487/8488

/* Sublayer Registers */
#define REG_CONTROL_1      0
#define REG_STATUS_1       1
#define REG_DEVICE_ID_1    2
#define REG_DEVICE_ID_2    3
#define REG_SPEED_ABILITY  4
#define REG_PACKAGE_1      5
#define REG_PACKAGE_2      6
#define REG_CONTROL_2      7
#define REG_STATUS_2       8

typedef struct _vtss_phy_10g_port_state_info_t {
    vtss_phy_10g_family_t        family;     /* Family          */
    vtss_phy_10g_type_t          type;       /* Type            */
    u16                          revision;   /* Revision number */
    u16                          device_feature_status;    /* Device features depending on EFUSE */
    vtss_phy_10g_status_t        status;     /* Status          */
    vtss_phy_10g_mode_t          mode;       /* Operating mode  */
    vtss_phy_10g_loopback_t      loopback;   /* Loopbacks      */
    vtss_phy_10g_power_t         power;      /* Power */
    vtss_phy_10g_failover_mode_t failover;   /* Failover mode */
    vtss_phy_10g_clause_37_control_t clause_37;  /* Clause 37 (1G Serdes mode) */
    u16                          channel_id; /* Phy Channel id  */
    BOOL                         channel_id_lock; /* Make the channel id 'read only'  */
    vtss_port_no_t               phy_api_base_no; /* First API no within this phy */
    u16                          gpio_count; /* Number of gpios for this Phy.  Note that multiple phy channels shares GPIOs */
    BOOL                         edc_fw_api_load; /* Is the EDC FW loaded through the API? */
#if defined(VTSS_FEATURE_SYNCE_10G)
    BOOL synce_clkout;          /* Clock out for recovered clock is enabled/disabled */
    BOOL xfp_clkout;            /* Clock out for XFP is enabled/disabled */
    vtss_phy_10g_rxckout_conf_t  rxckout;   /* RXCKOUT configuration */
    vtss_phy_10g_txckout_conf_t  txckout;   /* TXCKOUT configuration */
    vtss_phy_10g_srefclk_mode_t srefclk; /* SREFCLK configuration for venice family */
#endif /* VTSS_FEATURE_SYNCE_10G */
#ifdef VTSS_FEATURE_10GBASE_KR
    vtss_phy_10g_base_kr_conf_t kr_conf; /* 10gBASE-KR configuration data */
#endif /* VTSS_FEATURE_10GBASE_KR */
    
    vtss_phy_10g_event_t         ev_mask;
    BOOL                         event_86_enable;
    vtss_gpio_10g_gpio_mode_t    gpio_mode[VTSS_10G_PHY_GPIO_MAX];
    BOOL                         warm_start_reg_changed;
} vtss_phy_10g_port_state_t;

vtss_rc vtss_phy_10g_init_conf_set(struct vtss_state_s *vtss_state);
vtss_rc vtss_phy_10g_restart_conf_set(struct vtss_state_s *vtss_state);

vtss_rc vtss_phy_10g_sync(struct vtss_state_s *vtss_state, const vtss_port_no_t port_no);

vtss_rc vtss_phy_10g_debug_info_print(struct vtss_state_s *vtss_state,
                                      const vtss_debug_printf_t pr,
                                      const vtss_debug_info_t   *const info,
                                      BOOL                      ail);
vtss_rc vtss_phy_10g_inst_venice_create(struct vtss_state_s *vtss_state);

#endif /* _VTSS_PHY_10G_H_ */

