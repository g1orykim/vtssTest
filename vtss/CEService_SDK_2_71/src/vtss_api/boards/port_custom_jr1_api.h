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

#ifndef _VTSS_PORT_CUSTOM_JR1_API_H_
#define _VTSS_PORT_CUSTOM_JR1_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Jaguar NPI chip port */
#define JR1_NPI_CHIP_PORT 31

/* LED tower mode */
typedef enum {
    LED_TOWER_MODE_LINK_SPEED,  /* Green: 1G link/activity; Orange: 10/100 link/activity */
    LED_TOWER_MODE_DUPLEX,      /* Green: FDX; Orange: HDX + Collisions */
    LED_TOWER_MODE_LINK_STATUS, /* Green: Link/activity; Orange: port disabled/errors */
    LED_TOWER_MODE_POWER_SAVE,  /* Disabled to save power */
    LED_TOWER_MODE_CNT
} led_tower_mode_t;

extern led_tower_mode_t jr1_tower_mode;

void jr1_board_setup_ref(vtss_board_t *board, vtss_board_info_t *board_info);
void jr1_board_setup_cu48(vtss_board_t *board, vtss_board_info_t *board_info);

/* 10G PHY detection data */
typedef struct  {
    vtss_port_no_t port_no;      /* Port number (debugging) */
    u8             miim_addr_0;  /* Primary MIIM address */ 
    u8             miim_addr_1;  /* Secondary MIIM address */
    BOOL           old_found[2]; /* Only one 'old' module can be detected */
    port_cap_t     cap;          /* Port capability */
} jr1_10g_detect_t;

/* Detect Vitesse 10G PHY module on MIIM controller 1 */
void jr1_10g_detect(jr1_10g_detect_t *detect, vtss_board_info_t *board_info);
BOOL jr1_1g_phy_detect(u8 ctrl, u8 miim_addr, vtss_board_info_t *board_info);

u8 jr_telsa_phy_detect_ports(u8 ctrl, u8 miim_addr, vtss_board_info_t *board_info);
/* Port information for LED control */
typedef struct {
    BOOL                enabled;    /* Port enabled */
    vtss_port_status_t  status;     /* Port status */
    vtss_port_counter_t col_cnt;    /* Collision counter */
    vtss_port_counter_t err_cnt;    /* Error counter */
    vtss_port_counter_t pkt_cnt;    /* Rx and Tx packets (activity) */
    BOOL                collisions; /* Collisions detected */
    BOOL                errors;     /* Rx errors detected */
    BOOL                activity;   /* Activity detected */
} jr1_port_info_t;

extern jr1_port_info_t jr1_port_info_table[VTSS_PORTS];

extern port_custom_entry_t jr1_port_table[VTSS_PORTS];

#ifdef __cplusplus
}
#endif

#endif /* _VTSS_PORT_CUSTOM_JR1_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
