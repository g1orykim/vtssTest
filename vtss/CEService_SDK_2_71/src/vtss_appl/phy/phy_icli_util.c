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

#include "main.h"
#include "icli_api.h"
#include "icli_porting_util.h"
#include "phy_icli_util.h"
#include "port_api.h"

static void phy_icli_port_iter_init(port_iter_t *pit)
{
    (void)icli_port_iter_init(pit, VTSS_ISID_START, PORT_ITER_FLAGS_NORMAL);
}

static void phy_icli_phy_reg(phy_icli_req_t *req, port_iter_t *pit, u8 addr)
{
    u32    session_id = req->session_id;
    u32    addr_page = ((req->page << 5) + addr);
    ushort value;
    int    i;

    /* Ignore non-PHY ports */
    if (!is_port_phy(pit->iport))
        return;

    if (req->write) {
        /* Write */
        if (vtss_phy_write(NULL, pit->iport, addr_page, req->value) != VTSS_OK)
            req->count++;
    } else if (vtss_phy_read(NULL, pit->iport, addr_page, &value) != VTSS_OK) {
        /* Read failure */
        req->count++;
    } else {
        if (req->header) {
            req->header = 0;
            ICLI_PRINTF("Port  Addr     Value   15      8 7       0\n");
        }
        ICLI_PRINTF("%-6u0x%02x/%-4u0x%04x  ", pit->uport, addr, addr, value);
        for (i = 15; i >= 0; i--) {
            ICLI_PRINTF("%u%s", value & (1<<i) ? 1 : 0, i == 0 ? "\n" : (i % 4) ? "" : ".");
        }
    }
}

#define PHY_ICLI_PHY_ADDR_MAX 32

void phy_icli_debug_phy(phy_icli_req_t *req)
{
    u32                   session_id = req->session_id;
    icli_unsigned_range_t *list = req->addr_list;
    u8                    i, j, addr, addr_list[PHY_ICLI_PHY_ADDR_MAX];
    port_iter_t           pit;
    
    /* Build address list */
    for (addr = 0; addr < PHY_ICLI_PHY_ADDR_MAX; addr++) {
        addr_list[addr] = (list == NULL ? 1 : 0);
    }
    for (i = 0; list != NULL && i < list->cnt; i++) {
        for (j = list->range[i].min; j < PHY_ICLI_PHY_ADDR_MAX && j <= list->range[i].max; j++) {
            addr_list[j] = 1;
        }
    }

    if (req->addr_sort) {
        /* Iterate in (address, port) order */
        for (addr = 0; addr < PHY_ICLI_PHY_ADDR_MAX; addr++) {
            if (addr_list[addr]) {
                phy_icli_port_iter_init(&pit);
                while (icli_port_iter_getnext(&pit, req->port_list)) {
                    phy_icli_phy_reg(req, &pit, addr);
                }
            }
        }
    } else {
        /* Iterate in (port, address) order */
        phy_icli_port_iter_init(&pit);
        while (icli_port_iter_getnext(&pit, req->port_list)) {
            for (addr = 0; addr < PHY_ICLI_PHY_ADDR_MAX; addr++) {
                if (addr_list[addr]) {
                    phy_icli_phy_reg(req, &pit, addr);
                }
            }
        }
    }
    if (req->count) {
        ICLI_PRINTF("%u operations failed\n", req->count);
    }
}


//  see port_icli_functions.h
vtss_rc phy_icli_debug_phy_pass_through_speed(i32 session_id, icli_stack_port_range_t *plist, BOOL has_1g, BOOL has_100M, BOOL has_10M)
{
    vtss_phy_reset_conf_t reset_conf;
    vtss_phy_conf_t       setup_conf;
    switch_iter_t         sit;
    port_iter_t           pit;
  
    // Loop through all switches in a stack
    VTSS_RC(icli_switch_iter_init(&sit));
    while (icli_switch_iter_getnext(&sit, plist)) {
        // Loop though the ports
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
        while (icli_port_iter_getnext(&pit, plist)) {
            VTSS_RC(vtss_phy_reset_get(NULL, pit.iport, &reset_conf));
            reset_conf.media_if = VTSS_PHY_MEDIA_IF_SFP_PASSTHRU;
            VTSS_RC(vtss_phy_reset(NULL, pit.iport, &reset_conf));

            VTSS_RC(vtss_phy_conf_get(NULL, pit.iport, &setup_conf));
            setup_conf.mode = VTSS_PHY_MODE_FORCED;
            if (has_100M) {
                setup_conf.forced.speed = VTSS_SPEED_100M;                
            } 

            if (has_10M) {
                setup_conf.forced.speed = VTSS_SPEED_10M;              
            }

            if (has_1g) {
                setup_conf.forced.speed = VTSS_SPEED_1G;                
            }
            
            VTSS_RC(vtss_phy_conf_set(NULL, pit.iport, &setup_conf));
        }
    }
    return VTSS_RC_OK;
}



//  see port_icli_functions.h
vtss_rc phy_icli_debug_do_page_chk(i32 session_id, BOOL has_enable, BOOL has_disable) {
    if (has_enable) {
        VTSS_RC(vtss_phy_do_page_chk_set(NULL, TRUE));
    } else if (has_disable) {
        VTSS_RC(vtss_phy_do_page_chk_set(NULL, FALSE));
    } else {
        BOOL enabled;
        VTSS_RC(vtss_phy_do_page_chk_get(NULL, &enabled));
        ICLI_PRINTF("Do page check is %s \n", enabled ? "enabled" : "disabled");
    }
    return VTSS_RC_OK;
}

//  see phy_icli_functions.h
vtss_rc phy_icli_debug_phy_reset(i32 session_id, icli_stack_port_range_t *plist) {
    port_iter_t   pit;
    switch_iter_t sit;
    // Loop through all switches in a stack
    VTSS_RC(icli_switch_iter_init(&sit));
    while (icli_switch_iter_getnext(&sit, plist)) {
        
        // Loop though the ports
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
        while (icli_port_iter_getnext(&pit, plist)) {
            VTSS_RC(do_phy_reset(pit.iport));
        }
    }
    return VTSS_RC_OK;
}

//  see phy_icli_functions.h
vtss_rc phy_icli_debug_phy_gpio(i32 session_id, icli_stack_port_range_t *plist, BOOL has_mode_output, BOOL has_mode_input, BOOL has_mode_alternative, BOOL has_gpio_get, BOOL has_gpio_set, BOOL value, u8 gpio_no) {
    port_iter_t   pit;
    switch_iter_t sit;
    // Loop through all switches in a stack
    VTSS_RC(icli_switch_iter_init(&sit));
    while (icli_switch_iter_getnext(&sit, plist)) {
        
        // Loop though the ports
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
        while (icli_port_iter_getnext(&pit, plist)) {
            if (has_mode_output) {
                VTSS_RC(vtss_phy_gpio_mode(NULL, pit.iport, gpio_no, VTSS_PHY_GPIO_OUT));
            }

            if (has_mode_input) {
                VTSS_RC(vtss_phy_gpio_mode(NULL, pit.iport, gpio_no, VTSS_PHY_GPIO_IN));
            }

            if (has_mode_alternative) {
                VTSS_RC(vtss_phy_gpio_mode(NULL, pit.iport, gpio_no, VTSS_PHY_GPIO_ALT_0));
            }
            
            if (has_gpio_get) {
                VTSS_RC(vtss_phy_gpio_get(NULL, pit.iport, gpio_no, &value));
                ICLI_PRINTF("GPIO:%d is %s\n", gpio_no, value ? "high" : "low");
            }

            if (has_gpio_set) {
                VTSS_RC(vtss_phy_gpio_set(NULL, pit.iport, gpio_no, value));
            }
        }
    }
    return VTSS_RC_OK;
}

