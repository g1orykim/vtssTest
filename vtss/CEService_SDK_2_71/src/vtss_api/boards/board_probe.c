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

#include "port_custom_api.h"
#include "board_probe.h"

static vtss_board_probe_t probes[] = { 
#if defined(VTSS_ARCH_LUTON26)
    vtss_board_probe_lu26, 
#endif /* VTSS_ARCH_LUTON26 */
#if defined(VTSS_ARCH_LUTON28)
    vtss_board_probe_lu28, 
#endif /* VTSS_ARCH_LUTON28 */
#if defined(VTSS_ARCH_JAGUAR_1)
    vtss_board_probe_jr1, 
#endif /* VTSS_ARCH_JAGUAR_1 */
#if defined(VTSS_ARCH_SERVAL)
    vtss_board_probe_serval, 
#endif /* VTSS_ARCH_JAGUAR_1 */
    NULL 
};

static vtss_board_t board;

int vtss_board_probe(vtss_board_info_t *board_info, const port_custom_entry_t **port_custom_table)
{
    int i;
    /*lint -e{661} */ // Make lint behave for B2 API targets
    for(i = 0; probes[i] != NULL; i++) {
        if(probes[i](&board, board_info)) {
            *port_custom_table = board.custom_port_table;
            return board.type;
        }
    }
    return VTSS_BOARD_UNKNOWN;
}

const char *vtss_board_name(void)
{
    return board.name ? board.name : "Unknown";
}

vtss_board_type_t vtss_board_type(void)
{
    return board.type;
}

u32 vtss_board_features(void)
{
    return board.features;
}

u32 vtss_board_chipcount(void)
{
    return board.chipcount > 1 ? board.chipcount : 1; /* Maps 0 => 1 */
}

#if defined(VTSS_FEATURE_VSTAX)
u32 vtss_board_default_stackport(BOOL port_0)
{
    return ((board.features & VTSS_BOARD_FEATURE_STACKING) ? 
            (port_0 ? board.default_stackport_a : board.default_stackport_b) :
            VTSS_PORT_NO_NONE);
}
#endif  /* VTSS_FEATURE_VSTAX */

port_cap_t vtss_board_port_cap(int board_type, vtss_port_no_t port_no)
{
    return (port_no >= VTSS_PORTS ? 0 :
            board.port_cap ? board.port_cap(board_type, port_no) : 
            board.custom_port_table[port_no].cap);
}

/* Initialize board */
void port_custom_init(void)
{
    if(board.init)
        board.init();
}


/* Release ports from reset */
vtss_rc port_custom_reset(void)
{
    if(board.reset)
        return board.reset();
    return VTSS_RC_OK;
}


/* Post ports reset */
vtss_rc post_port_custom_reset(void)
{
    if(board.post_reset)
        return board.post_reset();
    return VTSS_RC_OK;
}


/* Initialize port LEDs */
vtss_rc port_custom_led_init(void)
{
    if(board.led_init)
        return board.led_init();
    return VTSS_RC_OK;
}

/* Update port LED */
vtss_rc port_custom_led_update(vtss_port_no_t port_no, 
                               vtss_port_status_t *status,
                               vtss_port_counters_t *counters,
                               port_custom_conf_t *port_conf)
{
    if(board.led_update)
        return board.led_update(port_no, status, counters, port_conf);
    return VTSS_RC_INCOMPLETE;
}


// Function for doing special port configuration that depends upon the platform
// Forexample do the enzo board requires that if the stack ports uses SFPs, the SFPs must be turn on 
// using a special serialised GPIO system.
void port_custom_conf(vtss_port_no_t port, 
                      port_custom_conf_t *port_conf, 
                      vtss_port_status_t *port_status)
{
    if(board.port_conf)
        board.port_conf(port, port_conf, port_status);
}

/* Called when system is reset. */
void port_custom_pre_reset(void)
{
    if(board.pre_reset)
        board.pre_reset();
}

/* Update the port map with a new interface */
vtss_rc board_sfp_update_if(vtss_port_no_t port_no,  
                            vtss_port_interface_t mac_if)
{
    if(board.sfp_update_if)
        board.sfp_update_if(port_no, mac_if);
    return VTSS_RC_OK;
}

/* Do PHY i2c read */
vtss_rc board_sfp_i2c_read(vtss_port_no_t port_no, u8 i2c_addr, u8 addr, u8 *const data, u8 cnt)
{
    if(board.sfp_i2c_read)
        return board.sfp_i2c_read(port_no, i2c_addr, addr, data, cnt);

    return VTSS_RC_OK;
}

/* Do PHY i2c write */
vtss_rc board_sfp_i2c_write(vtss_port_no_t port_no, u8 i2c_addr, u8 addr, u8 *const data)
{
    if(board.sfp_i2c_write)
        return board.sfp_i2c_write(port_no, i2c_addr, addr, data);

    return VTSS_RC_OK;
}

/* Do a SFP module detect  */
vtss_rc port_custom_sfp_mod_detect(BOOL *detect_status)
{
    if(board.sfp_mod_detect)
        return  board.sfp_mod_detect(detect_status);
    return VTSS_RC_OK;
}
/* Accept or deny detected SFP module */
BOOL port_custom_sfp_accept(u8 *sfp_rom)
{
    if(board.sfp_accept)
        return  board.sfp_accept(sfp_rom);
    return TRUE;
}



/* Enable the I2C access on a specific SFP  */
vtss_rc board_sfp_i2c_enable(vtss_port_no_t port_no)
{
    if (!board.sfp_i2c_lock)
        return VTSS_RC_ERROR; /* Must be locked before access  */
    if(board.sfp_i2c_enable)
        return  board.sfp_i2c_enable(port_no);
    return VTSS_RC_OK;
}

/* All SFPs are configured to the same I2C address. Specifc SFP is made active with SGPIOs */
/* A BOOL lock/unlock is used to lock the SFP while reading from it */
vtss_rc board_sfp_i2c_lock(BOOL lock)
{
    if ((lock && !board.sfp_i2c_lock) || (!lock && board.sfp_i2c_lock)) {
        board.sfp_i2c_lock = lock;
        return VTSS_RC_OK;
    } else {
        return VTSS_RC_ERROR;
    }    
}
