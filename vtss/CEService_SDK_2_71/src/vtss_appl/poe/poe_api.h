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

#ifndef _VTSS_APPL_POE_API_H_
#define _VTSS_APPL_POE_API_H_

// for getting VTSS_PORTS
#include "port_api.h"

/**********************************************************************
 ** PoE types
 **********************************************************************/
typedef enum {LOW, HIGH, CRITICAL}       poe_priority_t;
//Default priority configuration
#define POE_PRIORITY_DEFAULT LOW

typedef enum {CLASS_RESERVED, CLASS_CONSUMP, ALLOCATED_RESERVED, ALLOCATED_CONSUMP, LLDPMED_RESERVED, LLDPMED_CONSUMP, INIT_STATE} poe_power_mgmt_t;
//Define power management configuration
#define POE_MGMT_MODE_DEFAULT CLASS_RESERVED

typedef enum {POE_NOT_SUPPORTED,
              POWER_BUDGET_EXCEEDED,
              NO_PD_DETECTED,
              PD_ON,
              PD_OFF,
              PD_OVERLOAD,
              UNKNOWN_STATE,
              POE_DISABLED
             } poe_port_status_t; // Port status - See the poe_statuss2str function

typedef enum {ACTUAL, REQUESTED} poe_mgmt_mode_t;
typedef enum {UNKNOWN, PRIMARY, BACKUP, RESERVED} poe_power_source_t; // power source, see TIA-1057 table 16 or IEEE 802.3at table 33-22 (bits 5:4)
typedef enum {FOUND, NOT_FOUND, DETECTION_NEEDED}       poe_chip_found_t;

typedef enum {POE_MODE_POE_DISABLED, POE_MODE_POE, POE_MODE_POE_PLUS} poe_mode_t; // Configuration of PoE.

// Default mode configuration.
#define POE_MODE_DEFAULT POE_MODE_POE_DISABLED

//Default maximum power configuration for a port in allocation mode
#define POE_MAX_POWER_DEFAULT 154

/**********************************************************************
 ** Configuration structs
 **********************************************************************/

/* Configuration for the local switch */
typedef struct {
    int                primary_power_supply ; // The Max power that the primary power supply can give.Given with one digit. E.g. 209 equals 20.9W
    int                backup_power_supply  ; // The Max power that the backup (normally a battery) power supply can give.Given with one digit. E.g. 209 equals 20.9W
    poe_priority_t     priority[VTSS_PORTS] ; // Priority for each port;
    int                max_port_power[VTSS_PORTS]; // Maximum power allowed for each port ( Set by user )
    poe_mode_t         poe_mode[VTSS_PORTS]; // It is possible to select mode for each port individually.
    poe_power_mgmt_t   power_mgmt_mode; // Power management mode
} poe_local_conf_t;


/* Configuration that is only valid for master switch  */
typedef struct {
    poe_power_mgmt_t power_mgmt_mode; // Power management mode
} poe_master_conf_t;


/* Configuration for the whole stack  */
typedef struct {
    poe_master_conf_t master_conf;
    poe_local_conf_t  local_conf[VTSS_ISID_END];
} poe_stack_conf_t;

// Definitions of PoE chipset we support. Note: NO_POE_CHIPSET_FOUND MUST be the last.
typedef enum {PD63000, SI3452, SLUS787, PD690xx, NO_POE_CHIPSET_FOUND} poe_chipset_t;

/**********************************************************************
 ** Status structs
 **********************************************************************/

/* PoE status for the switch */
typedef struct {
    int               power_allocated[VTSS_PORTS]; // The power for reserved by the PD
    int               power_requested[VTSS_PORTS]; // The power for requested by the PD
    int               power_used[VTSS_PORTS];      // The power that the PD is using right now
    int               current_used[VTSS_PORTS];    // The current the PD is using right now
    poe_port_status_t port_status[VTSS_PORTS];     // Port status - Is on/off and why.
    int               pd_class[VTSS_PORTS];        // The class of the PD is using right now
    poe_chipset_t     poe_chip_found[VTSS_PORTS];  // List with the poe chip found for corresponding port.
} poe_status_t;

/**********************************************************************
 ** PoE functions
 **********************************************************************/

// Function returning the maximum power the a port can be configured to.
u16 poe_max_power_mode_dependent(vtss_port_no_t iport, poe_mode_t poe_mode);

/* Initialize module */
vtss_rc poe_init(vtss_init_data_t *data);

// Function that returns the current configuration for a local switch
void poe_mgmt_get_local_config(poe_local_conf_t *conf, vtss_isid_t isid) ;

// Function that can set the current configuration for a local switch
void poe_mgmt_set_local_config(poe_local_conf_t *new_conf, vtss_isid_t isid) ;

// Function that returns specific master configurations
void poe_mgmt_get_master_config(poe_master_conf_t *conf) ;

// Function that sets specific master configurations
void poe_mgmt_set_master_config(poe_master_conf_t *new_conf) ;

// Function for getting current status for all ports
void poe_mgmt_get_status(vtss_isid_t isid, poe_status_t *status);

// Function for getting current status for all ports
void poe_mgmt_get_port_status(vtss_isid_t isid, poe_port_status_t *port_status);

// Function for getting the PD classes for all ports
void poe_mgmt_get_pd_classes(vtss_isid_t isid, char *classes);

// Function Converting a integer to a string with one digit. E.g. 102 becomes 10.2.
char *one_digi_float2str(int val, char *string_ptr);

char *poe_status2str(poe_port_status_t status, vtss_port_no_t port_index, poe_local_conf_t *conf);

char *poe_class2str(const poe_status_t *status, vtss_port_no_t port_index, char *class_str);

BOOL poe_mgmt_is_backup_power_supported(void);



// Function for getting a list with which PoE chipset there is associated to a given port.
// In     : isid           - The internal switch id for which to get the information.
// In/out : poe_chip_found - pointer to array to return the list.
void poe_mgmt_is_chip_found(vtss_isid_t isid, poe_chipset_t *poe_chip_found);


// Debug function for doing PoE register reads
void poe_mgmt_reg_rd(vtss_port_no_t port_index, u16 addr, u16 *data);

// Debug function for doing PoE register writes
void poe_mgmt_reg_wr(vtss_port_no_t port_index, u16 addr, u16 data);

// Debug function
void poe_mgmt_capacitor_detection_set(vtss_port_no_t port_index, BOOL enable);
BOOL poe_mgmt_capacitor_detection_get(vtss_port_no_t port_index);
void poe_mgmt_firmware_update(u8 *firmware, u32 firmware_size);
i8 *poe_mgmt_firmware_info_get(vtss_port_no_t port_index, i8 *firmware_string);

// Function for getting which PoE chipset is found from outside the poe.c file (In order to do semaphore protection)
poe_chipset_t poe_is_chip_found(vtss_port_no_t port_index);
#endif  /* _VTSS_APPL_POE_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
