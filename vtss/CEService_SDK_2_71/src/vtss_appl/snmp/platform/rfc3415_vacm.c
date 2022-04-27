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
#ifdef SNMP_SUPPORT_V3
#include "rfc3415_vacm.h"
#if defined(SNMP_HAS_UCD_SNMP)
#include "ucd_snmp_rfc3415_vacm.h"
#elif defined(SNMP_HAS_NET_SNMP)
#include "net_snmp_rfc3415_vacm.h"
#endif /* SNMP_HAS_UCD_SNMP */
#endif /* SNMP_SUPPORT_V3 */

#if RFC3415_SUPPORTED_VACMCONTEXTTABLE
/*
 * Initializes the MIBObjects module
 */
void init_vacmContextTable(void)
{
    oid vacmContextTable_variables_oid[] = { 1, 3, 6, 1, 6, 3, 16, 1, 1 };

    // Register mibContextTable
    mibContextTable_register(vacmContextTable_variables_oid,
                             sizeof(vacmContextTable_variables_oid) / sizeof(oid),
                             "SNMP-VIEW-BASED-ACM-MIB : vacmContextTable");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_vacmContextTable();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_vacmContextTable();
#endif /* SNMP_HAS_UCD_SNMP */
}
#endif /* RFC3415_SUPPORTED_VACMCONTEXTTABLE */

#if RFC3415_SUPPORTED_VACMSECURITYTOGROUPTABLE
/*
 * Initializes the MIBObjects module
 */
void init_vacmSecurityToGroupTable(void)
{
    oid vacmSecurityToGroupTable_variables_oid[] = { 1, 3, 6, 1, 6, 3, 16, 1, 2 };

    // Register mibContextTable
    mibContextTable_register(vacmSecurityToGroupTable_variables_oid,
                             sizeof(vacmSecurityToGroupTable_variables_oid) / sizeof(oid),
                             "SNMP-VIEW-BASED-ACM-MIB : vacmSecurityToGroupTable");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_vacmSecurityToGroupTable();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_vacmSecurityToGroupTable();
#endif /* SNMP_HAS_UCD_SNMP */
}
#endif /* RFC3415_SUPPORTED_VACMSECURITYTOGROUPTABLE */

#if RFC3415_SUPPORTED_VACMACCESSTABLE
/*
 * Initializes the MIBObjects module
 */
void init_vacmAccessTable(void)
{
    oid vacmAccessTable_variables_oid[] = { 1, 3, 6, 1, 6, 3, 16, 1, 4 };

    // Register mibContextTable
    mibContextTable_register(vacmAccessTable_variables_oid,
                             sizeof(vacmAccessTable_variables_oid) / sizeof(oid),
                             "SNMP-VIEW-BASED-ACM-MIB : vacmAccessTable");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_vacmAccessTable();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_vacmAccessTable();
#endif /* SNMP_HAS_UCD_SNMP */
}
#endif /* RFC3415_SUPPORTED_VACMACCESSTABLE */

#if RFC3415_SUPPORTED_VACMMIBVIEWS
/*
 * Initializes the MIBObjects module
 */
void init_vacmMIBViews(void)
{
    oid vacmMIBViews_variables_oid[] = { 1, 3, 6, 1, 6, 3, 16, 1, 5 };

    // Register mibContextTable
    mibContextTable_register(vacmMIBViews_variables_oid,
                             sizeof(vacmMIBViews_variables_oid) / sizeof(oid),
                             "SNMP-VIEW-BASED-ACM-MIB : vacmMIBViews");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_vacmMIBViews();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_vacmMIBViews();
#endif /* SNMP_HAS_UCD_SNMP */
}
#endif /* RFC3415_SUPPORTED_VACMMIBVIEWS */

