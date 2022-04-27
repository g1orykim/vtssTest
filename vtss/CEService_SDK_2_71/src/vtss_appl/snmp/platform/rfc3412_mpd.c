/*

 Vitesse Switch Software.

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

#include <ucd-snmp/config.h>
#include <ucd-snmp/mibincl.h>   /* Standard set of SNMP includes */

#include "vtss_snmp_api.h"
#include "mibContextTable.h"  //mibContextTable_register
#include "rfc3412_mpd.h"
#if defined(SNMP_HAS_UCD_SNMP)
#include "ucd_snmp_rfc3412_mpd.h"
#elif defined(SNMP_HAS_NET_SNMP)
#include "net_snmp_rfc3412_mpd.h"
#endif /* SNMP_HAS_UCD_SNMP */

/*
 * Initializes the MIBObjects module
 */
void init_snmpMPDStats(void)
{
    oid snmpMPDStats_variables_oid[] = { 1, 3, 6, 1, 6, 3, 11, 2, 1 };

    // Register mibContextTable
    mibContextTable_register(snmpMPDStats_variables_oid,
                             sizeof(snmpMPDStats_variables_oid) / sizeof(oid),
                             "SNMP-MPD-MIB : snmpMPDStats");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_snmpMPDStats();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_snmpMPDStats();
#endif /* SNMP_HAS_UCD_SNMP */
}

