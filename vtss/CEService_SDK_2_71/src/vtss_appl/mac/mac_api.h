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

#ifndef _VTSS_MAC_API_H_
#define _VTSS_MAC_API_H_

#include "vtss_api.h"

/* MAC module defines */
#define MAC_AGE_TIME_DISABLE       0
#define MAC_AGE_TIME_MIN           10
#define MAC_AGE_TIME_MAX           1000000
#define MAC_AGE_TIME_DEFAULT       300
#define MAC_LEARN_MAX              VTSS_MAC_ADDRS
#define MAC_ADDR_NON_VOLATILE_MAX  64
#define MAC_ALL_VLANS              4097

#ifdef VTSS_SW_OPTION_PSEC
// When compiling with the PSEC option we need a volatile entry
// for every possible MAC address that that module can handle.
#include "psec_api.h"

#define MAC_ADDR_VOLATILE_MAX   PSEC_MAC_ADDR_ENTRY_CNT
#else
// Perhaps we could go down to 0 here, if no other module needs it.
#define MAC_ADDR_VOLATILE_MAX   64
#endif

/* Mac API error codes (vtss_rc) */
enum {
    MAC_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_MAC),  /* Generic error code */
    MAC_ERROR_MAC_RESERVED,      /* MAC address is reserved */
    MAC_ERROR_REG_TABLE_FULL,    /* Registration table full */
    MAC_ERROR_REQ_TIMEOUT,       /* Timeout on message request */
    MAC_ERROR_STACK_STATE,       /* Illegal MASTER/SLAVE state */
    MAC_ERROR_MAC_EXIST,         /* MAC address already exists */
    MAC_ERROR_MAC_SYSTEM_EXIST,  /* MAC address exists and is a system address */
    MAC_ERROR_MAC_VOL_EXIST,     /* Volatile MAC address exists  */
    MAC_ERROR_MAC_NOT_EXIST,     /* MAC address does not exist */
    MAC_ERROR_LEARN_FORCE_SECURE,/* Learn force secure is 'on' */
    MAC_ERROR_NOT_FOUND,          /* Not found */
    MAC_ERROR_MAC_ONE_DESTINATION_ALLOWED /* Only one destination is allowed */
}; // Let it be anonymous (tagless) for the sake of Lint.

/* Mac age time configuration */
typedef struct {
    ulong         mac_age_time;    /* Mac table age time */
} mac_age_conf_t;

/* MAC address table statistics */
typedef struct {
    ulong learned[VTSS_PORT_ARRAY_SIZE]; /* Number of learned entries per port */
    ulong learned_total;                 /* Total number of learned entries */
    ulong static_total;                  /* Total number of static entries */
} mac_table_stats_t;

/* MAC address properties for mgmt API */
typedef struct {
    vtss_vid_mac_t  vid_mac;                           /* VLAN ID and MAC address                        */
    BOOL            destination[VTSS_PORT_ARRAY_SIZE]; /* Dest. ports  for this mac address              */
    BOOL            dynamic;                           /* Dynamic (1) or Static (locked) (0)             */
    BOOL            not_stack_ports;                   /* Add address to stack port (0) or not (1)       */
    BOOL            volatil;                           /* Volatile (0) (saved to flash) or not (1)       */
    BOOL            copy_to_cpu;                       /* Make a CPU copy of frames to/from this MAC (1) */
} mac_mgmt_addr_entry_t;


typedef struct {
    BOOL only_this_vlan; /* Only look in this VLAN        */
    BOOL not_dynamic;    /* Not dynamic learn addresses   */
    BOOL not_static;     /* Not static addresses          */
    BOOL not_cpu;        /* Not addresses destined to cpu */
    BOOL not_mc;         /* Not MC addresses              */
    BOOL not_uc;         /* Not UC addresses              */
} mac_mgmt_addr_type_t;

typedef struct {
    vtss_vid_mac_t vid_mac;                                 /* VLAN ID and MAC addr */
    BOOL  destination[VTSS_ISID_CNT + 1][VTSS_PORT_ARRAY_SIZE]; /* Dest. ports per ISID */
    BOOL           copy_to_cpu;                             /* CPU copy flag */
    BOOL           locked;                                  /* Locked/static flag */
} mac_mgmt_table_stack_t;


/* Get the age time. Unit sec. This is the local age time but is should be the same for the whole stack.*/
vtss_rc mac_mgmt_age_time_get(mac_age_conf_t *conf);

/* Set the age time. Unit sec. All switches is the stack is given the same age-value */
vtss_rc mac_mgmt_age_time_set(mac_age_conf_t *conf);

/* Get the next Mac-address, dynamic or static per switch
 * 'next=1' means get-next while 'next=0' means a lookup
 * Address 00-00-00-00-00-00 will get the first entry. */
vtss_rc mac_mgmt_table_get_next(vtss_isid_t isid, vtss_vid_mac_t *vid_mac,
                                vtss_mac_table_entry_t *entry, BOOL next);

/* Another version of 'get next mac-address'
   Mac address is search across the stack and all destination front ports are returned (per ISID).
   Addresses only learned on stack ports are not returned. The function will search for  specific addresses
   defined in *type. */
vtss_rc mac_mgmt_stack_get_next(vtss_vid_mac_t *vid_mac, mac_mgmt_table_stack_t *entry,
                                mac_mgmt_addr_type_t *type, BOOL next);

/* Get MAC address statistics per switch*/
vtss_rc mac_mgmt_table_stats_get(vtss_isid_t isid, mac_table_stats_t *stats);

/* Get MAC address statistics per VLAN */
vtss_rc mac_mgmt_table_vlan_stats_get(vtss_isid_t isid, vtss_vid_t vlan, mac_table_stats_t *stats);

/* Flush dynamic MAC address table, static entry are not touched. All switches in the stack are flushed. */
vtss_rc mac_mgmt_table_flush(void);

/* Use another age time for specified time.
 * Units: seconds. Zero will disable aging. No master/slave support */
vtss_rc mac_age_time_set(ulong mac_age_time, ulong time);

/* Add a static mac address to the mac table. Per switch
 * The address is automatically added to all stack ports in the stack, making it known everywhere  */
vtss_rc mac_mgmt_table_add(vtss_isid_t isid, mac_mgmt_addr_entry_t *entry);

/* Delete a volatile or non-volatile static mac address from the mac table. Per switch.
 * vol = 0 : non-volatile group - saved in flash
 * vol = 1 : volatile group - not saved in flash
 * The address is deleted from the ports given and the function will by it self find out if it should
 * delete the address from the Stack ports (if the address no longer owned by any front ports)
 * Use <MAC_ALL_VLANS> as an vlan id to make the function ignore vlans       */
vtss_rc mac_mgmt_table_del(vtss_isid_t isid, vtss_vid_mac_t *entry, BOOL vol);

/* Delete all volatile or non-volatile mac addresses from the port regardless of VLAN id. Per switch.
 * vol = 0 : non-volatile group - saved in flash
 * vol = 1 : volatile group - not saved in flash
 * This function is basically using <mac_mgmt_table_del> to delete the addresses */
vtss_rc mac_mgmt_table_port_del(vtss_isid_t isid, vtss_port_no_t port_no, BOOL vol);

/* Get the next  Mac-address which have been added through this API. Could be static or dynamic.
 * Address 00-00-00-00-00-00 will get the first entry.
 * 'next=1' means get-next while 'next=0' means a lookup
 * vol = 0 : search non-volatile group (saved in flash)
 * vol = 1 : search volatile group (not saved in flash)                     */
vtss_rc mac_mgmt_static_get_next(vtss_isid_t isid, vtss_vid_mac_t *search_mac,
                                 mac_mgmt_addr_entry_t *return_mac, BOOL next, BOOL vol);


/* Set the learning mode for each port in the switch.
 *    Learn modes: automatic (normal learning, default)
 *               : cpu       (learning disabled)
 *               : discard   (learning frames dropped)
 * Dynamic learned addresses are flushed in the process            */
vtss_rc mac_mgmt_learn_mode_set(vtss_isid_t isid, vtss_port_no_t port_no,
                                vtss_learn_mode_t *learn_mode);

/* Get the learning mode for each port in the switch. */
void mac_mgmt_learn_mode_get(vtss_isid_t isid, vtss_port_no_t port_no, vtss_learn_mode_t *learn_mode, BOOL *chg_allowed);

/* Force the learning mode for the port to secure (discard).
 * Mode is not saved to flash and can not be changed by <mac_mgmt_learn_mode_set>
 * Dynamic learned addresses are flushed in the process            */
vtss_rc mac_mgmt_learn_mode_force_secure(vtss_isid_t isid, vtss_port_no_t port_no, BOOL cpu_copy);

/* Revert the learning mode to saved value */
vtss_rc mac_mgmt_learn_mode_revert(vtss_isid_t isid, vtss_port_no_t port_no);

#if defined(VTSS_FEATURE_VSTAX)
/* UPSID to USID. For convinence.  Returns 0 is executed from a slave. */
vtss_usid_t mac_mgmt_upsid2usid(const vtss_vstax_upsid_t upsid);
#endif /* VTSS_FEATURE_VSTAX */

/* Initialize module */
vtss_rc mac_init(vtss_init_data_t *data);

/* Return an error text string based on a return code. */
char *mac_error_txt(vtss_rc rc);

#endif /* _VTSS_MAC_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
