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
#include "mibContextTable.h" //mibContextTable_register()
#include "snmp_mib_redefine.h"  //snmp_mib_redefine_register()
#ifdef SNMP_SUPPORT_V3
#include "rfc3414_usm.h"
#if defined(SNMP_HAS_UCD_SNMP)
#include "ucd_snmp_rfc3414_usm.h"
#elif defined(SNMP_HAS_NET_SNMP)
#include "net_snmp_rfc3414_usm.h"
#endif /* SNMP_HAS_UCD_SNMP */
#endif /* SNMP_SUPPORT_V3 */

#if RFC3414_SUPPORTED_USMSTATS
/*
 * Initializes the MIBObjects module
 */
void init_usmStats(void)
{
    oid  usmStats_variables_oid[] = { 1, 3, 6, 1, 6, 3, 15, 1, 1 };

    // Register mibContextTable
    mibContextTable_register(usmStats_variables_oid,
                             sizeof(usmStats_variables_oid) / sizeof(oid),
                             "SNMP-USER-BASED-SM-MIB : usmStats");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_usmStats();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_usmStats();
#endif /* SNMP_HAS_UCD_SNMP */
}
#endif /* RFC3414_SUPPORTED_USMSTATS */

#if RFC3414_SUPPORTED_USMUSER
/*
 * Initializes the MIBObjects module
 */
void init_usmUser(void)
{
    /*
     * Register SysORTable
     */
    oid usmUser_variables_oid[] = { 1, 3, 6, 1, 6, 3, 15, 1, 2 };
    mibContextTable_register(usmUser_variables_oid,
                             sizeof(usmUser_variables_oid) / sizeof(oid),
                             "SNMP-USER-BASED-SM-MIB : usmUserTable");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_usmUser();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_usmUser();
#endif /* SNMP_HAS_UCD_SNMP */

    /*
     * Register snmpMibRedefineTable
     */

    // usmUserSpinLock
    oid             usmUserSpinLock_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 1 };
    snmp_mib_redefine_register(usmUserSpinLock_variables_oid,
                               sizeof(usmUserSpinLock_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserSpinLock",
                               "TestAndIncr", SNMP_MIB_ACCESS_TYPE_RWRITE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE,
                               "{0 2147483647}");

    // usmUserCloneFrom
    oid             usmUserCloneFrom_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 4 };
    snmp_mib_redefine_register(usmUserCloneFrom_variables_oid,
                               sizeof(usmUserCloneFrom_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserCloneFrom",
                               "RowPointer", SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE, "");

    // usmUserAuthProtocol
    oid             usmUserAuthProtocol_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 5 };
    snmp_mib_redefine_register(usmUserAuthProtocol_variables_oid,
                               sizeof(usmUserAuthProtocol_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserAuthProtocol",
                               "AutonomousType",
                               SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE, "");

    // usmUserAuthKeyChange
    oid             usmUserAuthKeyChange_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 6 };
    snmp_mib_redefine_register(usmUserAuthKeyChange_variables_oid,
                               sizeof(usmUserAuthKeyChange_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserAuthKeyChange",
                               "KeyChange", SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE, "");

    // usmUserOwnAuthKeyChange
    oid             usmUserOwnAuthKeyChange_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 7 };
    snmp_mib_redefine_register(usmUserOwnAuthKeyChange_variables_oid,
                               sizeof
                               (usmUserOwnAuthKeyChange_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserOwnAuthKeyChange",
                               "KeyChange", SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE, "");

    // usmUserPrivProtocol
    oid             usmUserPrivProtocol_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 8 };
    snmp_mib_redefine_register(usmUserPrivProtocol_variables_oid,
                               sizeof(usmUserPrivProtocol_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserPrivProtocol",
                               "AutonomousType",
                               SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE, "");

    // usmUserPrivKeyChange
    oid             usmUserPrivKeyChange_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 9 };
    snmp_mib_redefine_register(usmUserPrivKeyChange_variables_oid,
                               sizeof(usmUserPrivKeyChange_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserPrivKeyChange",
                               "KeyChange", SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE, "");

    // usmUserOwnPrivKeyChange
    oid             usmUserOwnPrivKeyChange_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 10 };
    snmp_mib_redefine_register(usmUserOwnPrivKeyChange_variables_oid,
                               sizeof
                               (usmUserOwnPrivKeyChange_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserOwnPrivKeyChange",
                               "KeyChange", SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE, "");

    // usmUserPublic
    oid             usmUserPublic_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 11 };
    snmp_mib_redefine_register(usmUserPublic_variables_oid,
                               sizeof(usmUserPublic_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserPublic",
                               "OCTETSTR", SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, FALSE,
                               "{0 32}");

    // usmUserStorageType
    oid             usmUserStorageType_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 12 };
    snmp_mib_redefine_register(usmUserStorageType_variables_oid,
                               sizeof(usmUserStorageType_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserStorageType",
                               "StorageType", SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, TRUE,
                               "{4 permanent}");

    // usmUserStatus
    oid             usmUserStatus_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 15, 1, 2, 2, 1, 13 };
    snmp_mib_redefine_register(usmUserStatus_variables_oid,
                               sizeof(usmUserStatus_variables_oid) /
                               sizeof(oid),
                               "SNMP-USER-BASED-SM-MIB : usmUserStatus",
                               "RowStatus", SNMP_MIB_ACCESS_TYPE_RCREATE,
                               SNMP_MIB_ACCESS_TYPE_RONLY, TRUE,
                               "{1 active}");
}
#endif /* RFC3414_SUPPORTED_USMUSER */

