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

#ifndef _VTSS_BOARD_PROBE_H_
#define _VTSS_BOARD_PROBE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "port_custom_api.h"

typedef struct {
    /*
     * Identification & Features
     */
    vtss_board_type_t type;
    const char *name;
    u32 features;
    int chipcount;
    const port_custom_entry_t *custom_port_table;

    /*
     * Stacking support
     */
    u32 default_stackport_a, default_stackport_b;

    /* 
     * Board specific functions
     */
    
    /* Initialize board */
    void (*init)(void);

    /* Release ports from reset */
    vtss_rc (*reset)(void);

    /* Post ports reset */
    vtss_rc (*post_reset)(void);

    /* Called when system is reset. */
    void (*pre_reset)(void);

    /* Initialize port LEDs */
    vtss_rc (*led_init)(void);

    /* Update port LED */
    vtss_rc (*led_update)(vtss_port_no_t port_no,
                          vtss_port_status_t *status,
                          vtss_port_counters_t *counters,
                          port_custom_conf_t *port_conf);
    
    /* Function for doing special port configuration that depends upon the platform */
    void (*port_conf)(vtss_port_no_t port, 
                      port_custom_conf_t *port_conf, 
                      vtss_port_status_t *port_status);

    /* Update port map with new SFP interface */
    void (*sfp_update_if)(vtss_port_no_t port_no, 
                          vtss_port_interface_t mac_if);

    /* Do SFP i2c reads */
    vtss_rc (*sfp_i2c_read)(vtss_port_no_t port_no, 
                            u8 i2c_addr, 
                            u8 addr, 
                            u8 *const data, 
                            u8 cnt);

    vtss_rc (*sfp_i2c_write)(vtss_port_no_t port_no, 
                            u8 i2c_addr, 
                            u8 addr, 
                            u8 *const data);

   /* Do i2c reads */
    vtss_rc (*i2c_read)(vtss_port_no_t port_no, 
                        u8 i2c_addr, 
                        u8 addr, 
                        u8 *const data, 
                        u8 cnt);

    /* SFP module detect */
    vtss_rc (*sfp_mod_detect)(BOOL *detect_status);

    /* SFP module accept/deny */
    BOOL (*sfp_accept)(u8 *sfp_rom);

    /* Enable i2c access to a SFP module connected to at certain port */
    vtss_rc (*sfp_i2c_enable)(vtss_port_no_t port_no);

    /* Lock the SFP access while accessing the SFP via I2C (avoid a thread enable another SFP while accesseing via I2C ) */
    vtss_rc (*board_sfp_i2c_lock)(BOOL lock);
    BOOL sfp_i2c_lock;

    /* Return capabilities for board and port */
    port_cap_t (*port_cap)(int board_type, vtss_port_no_t port_no);
} vtss_board_t;

typedef BOOL (*vtss_board_probe_t)(vtss_board_t *, vtss_board_info_t *);

#if defined(VTSS_ARCH_LUTON26)
extern BOOL vtss_board_probe_lu26(vtss_board_t *board, vtss_board_info_t *board_info);
#endif /* VTSS_ARCH_LUTON26 */

#if defined(VTSS_ARCH_LUTON28)
extern BOOL vtss_board_probe_lu28(vtss_board_t *board, vtss_board_info_t *board_info);
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_ARCH_JAGUAR_1)
extern BOOL vtss_board_probe_jr1(vtss_board_t *board, vtss_board_info_t *board_info);
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_SERVAL)
extern BOOL vtss_board_probe_serval(vtss_board_t *board, vtss_board_info_t *board_info);
#endif /* VTSS_ARCH_JAGUAR_1 */

#ifdef __cplusplus
}
#endif

#endif /* _VTSS_BOARD_PROBE_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
