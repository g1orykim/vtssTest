/*

 Vitesse API software.

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
#include "vtss_api.h"   // For board initialization
#include "vtss_appl.h"  // For vtsss_board_t 
#include "vtss_appl_cu_phy.h" // For board init
#include <netdb.h>  // For socket
#include <stdarg.h> // For va_list

/* ================================================================= *
 *  Board init.
 * ================================================================= */


/* Board specifics */
static vtss_board_t board_table[1];

// Function defining the board
// 
// In : Pointer to the board definition
void vtss_board_phy_init(vtss_board_t *board)
{
    board->descr = "PHY"; // Description
    board->target = VTSS_TARGET_CU_PHY; // Target

// Please select which EVAL board you are using.
    board->board_init = atom12_board_init; // Pointer to function initializing the board
    board->board_init = tesla_board_init;  // Pointer to function initializing the board
}

/* ================================================================= *
 *  API lock/unlock functions - If the Application support threads 
 *  the API must be protected via Mutexes  
 * ================================================================= */
BOOL api_locked = FALSE;

// Function call by the API, when the API shall do mutex lock.
void vtss_callout_lock(const vtss_api_lock_t *const lock)
{
    // For testing we don't get a deadlock. The API must be unlocked before "locking"
    if (api_locked) {
        T_E("API lock problem");
    }
    api_locked = TRUE;
}

// Function call by the API, when the API shall do mutex unlock.
void vtss_callout_unlock(const vtss_api_lock_t *const lock)
{
    // For testing we don't get a deadlock. vtss_callout_lock must have been called before vtss_callout_unlock is called.
    if (!api_locked) {
        T_E("API unlock problem");
    }
    api_locked = FALSE;
}

/* ================================================================= *
 *  Debug trace
 * ================================================================= */
vtss_trace_conf_t vtss_appl_trace_conf = {
    .level[0] = VTSS_TRACE_LEVEL_DEBUG
};

// Trace callout function - This function is called for printing out debug information from the API
// Different levels of trace are support. The level are :
// 1) VTSS_TRACE_LEVEL_NONE   - No information from the API will be printed
// 2) VTSS_TRACE_LEVEL_ERROR  - Printout of T_E/VTSS_E trace. Error messages for malfunctioning API
// 3) VTSS_TRACE_LEVEL_WARNING- Printout of T_W/VTSS_W trace. Warning messages for unexpected API behavior.
// 4) VTSS_TRACE_LEVEL_INFO   - Printout of T_I/VTSS_I trace. Debug messages.
// 5) VTSS_TRACE_LEVEL_DEBUG  - Printout of T_D/VTSS_D trace. Even more debug messages.
// 6) VTSS_TRACE_LEVEL_NOISE  - Printout of T_N/VTSS_N trace. Even more debug messages.
void vtss_callout_trace_printf(const vtss_trace_layer_t layer,
                               const vtss_trace_group_t group,
                               const vtss_trace_level_t level,
                               const char *file,
                               const int line,
                               const char *function,
                               const char *format,
                               ...)
{
    va_list va;
    printf("Lvl:%s file:%s func:%s line:%d - ", 
               level == VTSS_TRACE_LEVEL_ERROR ? "Error" :
               level == VTSS_TRACE_LEVEL_INFO ? "Info " :
               level == VTSS_TRACE_LEVEL_DEBUG ? "Debug" :
               level == VTSS_TRACE_LEVEL_NOISE ? "Noise" : "?????",
               file,
               function,
               line);

    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    printf("\n");
}

/* ================================================================= *
 *  // Example of how to use the loop back function
 * ================================================================= */
void phy_loop_back(void) {
    vtss_phy_loopback_t loopback;
    vtss_port_no_t port_no;

    loopback.near_end_enable = FALSE;  // Set to TRUE for enabling Near End loopback 
    loopback.far_end_enable  = TRUE;  // Set to TRUE for enabling Far End loopback 
    
    // Setup the loopback
   for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORTS; port_no++) {
       vtss_phy_loopback_set(NULL, port_no, loopback);
   }

    // Example of getting the current loopback settings
   for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORTS; port_no++) {
       vtss_phy_loopback_get(NULL, port_no, &loopback);
       T_I("loopback for port:%d near_end_loopback=%d, far_end_loopback = %d \n",
           port_no,
           loopback.near_end_enable,
           loopback.far_end_enable);
   }
} 


/* ================================================================= *
 *  Main
 * ================================================================= */
int main(int argc, const char **argv) {
    vtss_board_t            *board;
    vtss_inst_create_t      create;
    vtss_port_no_t          port_no;
    vtss_phy_reset_conf_t   phy_reset;
    vtss_init_conf_t        init_conf;
    vtss_port_interface_t   mac_if;
    vtss_phy_media_interface_t media_if;
#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_mode_t     oper_mode;
#endif /* VTSS_CHIP_10G_PHY */

    // Setup trace level for PHY group
    vtss_trace_conf_t vtss_appl_trace_conf = {.level = VTSS_TRACE_LEVEL_INFO };
    vtss_trace_conf_set(VTSS_TRACE_GROUP_PHY, &vtss_appl_trace_conf);

    // In this case we only have one board. Assign point to the board definition
    board = &board_table[0];    

    // Initialize 
    vtss_board_phy_init(board);   

    // "Create" the board
    vtss_inst_get(board->target, &create);
    vtss_inst_create(&create, &board->inst);
    vtss_init_conf_get(board->inst, &init_conf);

    printf ("//Comment: Setup of VSC8574 for  Auto neg.  and SGMII\n");
    printf ("//Comment: miim_read format  : miim_read(port_number, address) \n");
    printf ("//Comment: miim_write format : miim_write(port_number, address, value(hex)) \n");
    printf ("\n\n");

    board->init.init_conf = &init_conf;
    if (board->board_init(argc, argv, board)) {
        T_E("Could not initialize board");
        return 1;
    } else {
        printf ("//Comment: Board being initialized\n");
    }

    if (vtss_init_conf_set(board->inst, &init_conf) == VTSS_RC_ERROR) {
        T_E("Could not initialize");
        return 1;
    };

    // Reset stuff needed before PHY port reset, any port will do (Only at start-up initialization) 
    printf ("//Comment: PHY pre-reset\n");
    vtss_phy_pre_reset(board->inst, 0);

    // Example for MAC interfaces
    mac_if = VTSS_PORT_INTERFACE_SERDES;
    mac_if = VTSS_PORT_INTERFACE_QSGMII;
    mac_if = VTSS_PORT_INTERFACE_SGMII;


    // Example for Media interfaces
    media_if = VTSS_PHY_MEDIA_IF_FI_1000BX;
    media_if = VTSS_PHY_MEDIA_IF_AMS_FI_1000BX;    
    media_if = VTSS_PHY_MEDIA_IF_AMS_FI_100FX;    
    media_if = VTSS_PHY_MEDIA_IF_FI_100FX; 
    media_if= VTSS_PHY_MEDIA_IF_SFP_PASSTHRU;
    media_if = VTSS_PHY_MEDIA_IF_CU;    

    vtss_phy_conf_t         phy;

    // Example for PHY moode  
    phy.mode = VTSS_PHY_MODE_FORCED;
    phy.mode = VTSS_PHY_MODE_ANEG;

    // Example for PHY speed support for auto-neg 
    phy.aneg.speed_10m_hdx = 1;
    phy.aneg.speed_10m_fdx = 1;
    phy.aneg.speed_100m_hdx = 1;
    phy.aneg.speed_100m_fdx = 1;
    phy.aneg.speed_1g_fdx = 1;

    // Example for PHY flow control settings
    phy.aneg.symmetric_pause =  1;
    
    // Example for PHY speed (non-auto neg.)
    phy.forced.speed = VTSS_SPEED_1G;
    phy.forced.speed = VTSS_SPEED_100M;
    phy.forced.speed = VTSS_SPEED_10M;
    phy.forced.fdx = 1;

    phy.mdi = VTSS_PHY_MDIX_AUTO; // always enable auto detection of crossed/non-crossed cables
    
    // Do PHY port reset + Port setup
    printf ("//Comment: PHY port reset\n");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORTS; port_no++) {
        phy_reset.mac_if = mac_if; // Set the port interface
        phy_reset.media_if = media_if;         // Set media interface to CU
        vtss_phy_reset(board->inst, port_no, &phy_reset); // Do port reset
        vtss_phy_conf_set(board->inst, port_no, &phy);
    }

    // Do reset stuff that is needed after port reset, any port will do (Only at start-up initialization) 
    printf ("//Comment: PHY post-reset\n");
    vtss_phy_post_reset(board->inst, 0);

#if defined(VTSS_CHIP_10G_PHY)
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORTS; port_no++) {
        if(port_is_10g_phy(port_no)) {
            if (vtss_phy_10g_mode_set(board->inst, port_no, &oper_mode) == VTSS_RC_ERROR) {
                T_E("Could not initialize 10G phy");
                return 1;
            }
        }
    }
#endif /* VTSS_CHIP_10G_PHY */
    phy_loop_back(); // Example for loopbacks 

    u16  value;
    char command[255];
    char cmd[255];
    char port_no_str[255];
    char no_str[255];
    char addr_str[255];
    BOOL read_cmd = TRUE;
    int addr;

    while (1) {
        scanf("%s", &command[0]);
        if (strcmp(command, "?")  == 0) {
            printf ("**** Help ****");
            printf ("The following command is support\n");
            printf ("rd <port_no> <addr>\n");
            printf ("wr <port_no> <addr> <value> - Value MUST be in hex\n");
            printf ("gpio <port_no> <gpio_no> mode <A,I or O> - Configure GPIO mode A = alternative mode, I = input, O = output  \n");
            printf ("gpio <port_no> <gpio_no> set <0 or 1> \n");
            printf ("gpio <port_no> <gpio_no> get \n");
            printf ("status <port_no>  - Return PHY API status \n");
            continue;
        } else if (strcmp(command, "rd")  == 0) {
            read_cmd = TRUE;
        } else if (strcmp(command, "wr")  == 0) {
            read_cmd = FALSE;

        } else if (strcmp(command, "gpio")  == 0) {
            scanf("%s", &port_no_str[0]);
            port_no = atoi(port_no_str);

            u16 gpio_no;
            scanf("%s", &no_str[0]);
            gpio_no = atoi(no_str);

            scanf("%s", &cmd[0]);
            
            vtss_rc rc = VTSS_RC_OK;            
            if (strcmp(cmd, "mode")  == 0) {
                scanf("%s", &cmd[0]);
                if (strcmp(cmd, "A")  == 0) {
                    rc = vtss_phy_gpio_mode(NULL, port_no, gpio_no, VTSS_PHY_GPIO_ALT_0);
                } else if (strcmp(cmd, "I")  == 0) {
                    rc = vtss_phy_gpio_mode(NULL, port_no, gpio_no, VTSS_PHY_GPIO_IN);
                } else if (strcmp(cmd, "O")  == 0) {
                    rc = vtss_phy_gpio_mode(NULL, port_no, gpio_no, VTSS_PHY_GPIO_OUT);
                } else {
                    printf("Unknown GPIO mode command\n");
                }
            } else if (strcmp(cmd, "set")  == 0) {
                scanf("%s", &no_str[0]);
                value = atoi(no_str);
                rc = vtss_phy_gpio_set(NULL, port_no, gpio_no, value);

            } else if (strcmp(cmd, "get")  == 0) {
                BOOL val;
                rc = vtss_phy_gpio_get(NULL, port_no, gpio_no, &val);
                printf ("GPIO:%d is %s\n", gpio_no, val ? "high" : "low");
    
            } else {
                printf("Unknown GPIO command\n");
            }

            if (rc != VTSS_RC_OK) {
                printf("Port:%d, GPIO:%d - Error code not OK : RC:%d\n", port_no, gpio_no, rc);
            }
            continue;
            
        } else if (strcmp(command, "status")  == 0) {
            vtss_port_status_t status;
            scanf("%s", &port_no_str[0]);
            port_no = atoi(port_no_str);
            
            vtss_phy_status_get(NULL, port_no, &status);   
            printf("Link down:%d \n", status.link_down);
            continue;

        } else {
            printf ("Unknown command:%s \n", command);    
            continue;
        }

        scanf("%s", &port_no_str[0]);
        port_no = atoi(port_no_str);

        scanf("%s", &addr_str[0]);
        addr = atoi(addr_str);

        if (read_cmd) {
            miim_read(NULL, port_no, addr, &value);
            printf("%02d: 0x%04X \n", addr, (int)value);
        } else {
            int hex;
            scanf("%X", &hex);
            value = hex;
            miim_write(NULL, port_no, addr, value);
            miim_read(NULL, port_no, addr, &value);
            printf("New register value for %02d: 0x%04X \n", addr, value);

        }
    }
    return 0; // All done 
}

