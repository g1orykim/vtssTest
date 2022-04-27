/*

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "lldp.h"


/* ************************************************************************ **
 *
 *
 * Defines
 *
 *
 *
 * ************************************************************************ */

#define LEFT  0
#define RIGHT 1

/* ************************************************************************ **
 *
 *
 * Typedefs and enums
 *
 *
 *
 * ************************************************************************ */

/* ************************************************************************ **
 *
 *
 * Prototypes for local functions
 *
 *
 *
 * ************************************************************************ */



/* ************************************************************************ **
 *
 *
 * Local data
 *
 *
 *
 * ************************************************************************ */


/* ************************************************************************ */
void print_hex_b (lldp_u8_t value, lldp_printf_t lldp_printf)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a byte as 2 hex nibbles.
 * Remarks     : value holds byte value to print.
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    (void) lldp_printf("%02X", value);
}

void ip_addr2str (const lldp_8_t *ip_addr, char *str)
/* ------------------------------------------------------------------------ --
 * Purpose     : converts a ip address a string with teh format xxx.xxx.xxx.xxx.
 * Remarks     : ip_addr points to a 4-byte array holding ip address.
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{

    sprintf(str, "%u.%u.%u.%u",
            (lldp_u8_t)ip_addr[0], (lldp_u8_t)ip_addr[1], (lldp_u8_t)ip_addr[2], (lldp_u8_t)ip_addr[3]);

}




/* ************************************************************************ */
void mac_addr2str (const lldp_8_t *mac_addr, lldp_8_t *str)
/* ------------------------------------------------------------------------ --
 * Purpose     : Convert a mac address a well formated streing
 * Remarks     : mac_addr points to a 6-byte array holding mac address.
 * ************************************************************************ */
{
    sprintf(str, "%02X-%02X-%02X-%02X-%02X-%02X",
            (lldp_u8_t)mac_addr[0], (lldp_u8_t)mac_addr[1], (lldp_u8_t)mac_addr[2], (lldp_u8_t)mac_addr[3], (lldp_u8_t)mac_addr[4], (lldp_u8_t)mac_addr[5]);
}




