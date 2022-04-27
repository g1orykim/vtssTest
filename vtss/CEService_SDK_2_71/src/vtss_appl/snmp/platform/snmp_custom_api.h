/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_SNMP_CUSTOM_H_
#define _VTSS_SNMP_CUSTOM_H_

/*
Organization of Private MIB:

In the following, <product_id> is only inserted when SNMP_PRIVATE_MIB_ENTERPRISE == 6603 == VTSS.
It is encoded like this:
  Bits 31:24 is the software type (see misc_softwaretype()).
  Bits 15:00 is the chip ID with which the API is instantiated (see misc_chiptype()).

{iso, org, dod, internet, private, enterprise, VTSS, ...}
{1,   3,   6,   1,        4,       1,          6603, <product_id>, <sw_module_id>} concatenated with:

|---<switchMgmt(1)>
    |---<systemMgmt(1)>
    |---<ipMgmt(1)>
    |---<vlanMgmt(2)>
    |---<portMgmt(3)>
    |---...
|---<switchNotifications(2)>
    |---<SwitchTraps(1)>
        |---<trap_a(1)>
        |---<trap_b(2)>
        |---...
*/

#define SNMP_PRIVATE_MIB_ENTERPRISE               6603 /* VTSS */
#define SNMP_PRIVATE_MIB_PRODUCT_ID               0 /* Auto-filled in when SNMP_PRIVATE_MIB_ENTERPRISE is 6603 */

/* Management branch */
#define SNMP_PRIVATE_MIB_SWITCH_MGMT              1

/* Notification branch */
#define SNMP_PRIVATE_MIB_SWITCH_NOTIFICATIONS     2
#define SNMP_PRIVATE_MIB_SWITCH_TRAPS             1
#define SNMP_PRIVATE_MIB_TRAP_PSEC_LIMIT_EXCEEDED 1

// Set iport to VTSS_PORT_NO_NONE to avoid sending trap with an ifIndex.
void    snmp_private_mib_trap_send(vtss_isid_t isid, vtss_port_no_t iport, oid trap_number);
extern  oid snmp_private_mib_oid[];
u32     snmp_private_mib_oid_len_get(void);

#endif /* _VTSS_SNMP_CUSTOM_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

