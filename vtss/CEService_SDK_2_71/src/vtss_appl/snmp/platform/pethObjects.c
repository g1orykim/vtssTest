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
/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.old-api.conf,v 1.3 2009/02/18 15:11:53 rbn Exp $
 */

#include <main.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>
#include <sys/param.h>

#include <ucd-snmp/config.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif                          /* HAVE_STDLIB_H */
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif                          /* HAVE_STRING_H */

#include <ucd-snmp/mibincl.h>   /* Standard set of SNMP includes */
#include <ucd-snmp/mibgroup/util_funcs.h>       /* utility function declarations */
#include "pethObjects.h"


#include "l2proto_api.h"
#include "msg_api.h"
#include "ifIndex_api.h"
#include "poe_api.h"
#include "poe_custom_api.h"
#include "cli_trace_def.h"
#include "mibContextTable.h"  //mibContextTable_register


/*
 * +++ Start (Internal implementation declarations)
 */
typedef struct pethPsePortTable_entry {
    BOOL peth_pse_port_admin_enable;
    BOOL peth_pse_port_power_pairs_control_ability;
    u32  peth_pse_port_power_pairs;
    u32  peth_pse_port_detection_status;
    u32  peth_pse_port_power_priority;
    u32  peth_pse_port_mps_absent_counter;
    u8   peth_pse_port_type[SNMP_ADMIN_STRING_LEN];
    u32  peth_pse_port_power_classifications;
    u32  peth_pse_port_invalid_signature_counter;
    u32  peth_pse_port_power_denied_counter;
    u32  peth_pse_port_over_load_counter;
    u32  peth_pse_port_short_counter;
} pethPsePortTable_entry_t;

typedef struct pethMainPseTable_entry {
    u32   peth_main_pse_power;
    u32   peth_main_pse_oper_status;
    u32   peth_main_pse_consumption_power;
    u32   peth_main_pse_usage_threshold;
} pethMainPseTable_entry_t;

/********************************************************************
* Purpose   : Retrieves the number of indexes being used in the table.
* Arguments : table_number -> Table being referred to.
* Re-Entrant: Must be Re-entrant.
********************************************************************/
i32
getnum_poe_indices (i32 table_number)
{
    switch (table_number) {
    case 1:
        return 2;
    default:
        return -1;
    }
}

/********************************************************************
* Purpose   : Customized version of Header_simple_table to track more
              than one index.
* Arguments : vp -> Variable pointer, name: Oid name, length: Length of
              Oid, exact: Get or Get next request, var_len: variable len
              write_method: Pointer to the write function, max: Table Max
          size.
* Re-Entrant: Must be Re-entrant.
********************************************************************/
i32
header_poe_table (struct variable *vp, oid *name, size_t *length,
                  int exact, size_t *var_len,
                  WriteMethod **write_method, i32 max)
{
    i32 i, rtest;                 /* Set to:  -1  If name < vp->name,
                   *      1   If name > vp->name,
                   *      0   Otherwise.
                   */
    i32 table_num;
    oid newname[MAX_OID_LEN];

    for (i = 0, rtest = 0;
         i < (i32) vp->namelen && i < (i32) (*length) && !rtest; i++) {
        if (name[i] != vp->name[i]) {
            if (name[i] < vp->name[i]) {
                rtest = -1;
            } else {
                rtest = 1;
            }
        }
    }
    table_num = vp->name[vp->namelen - 3];
    //It will retrieve the table number in the MSTP objects

    if (rtest > 0 ||
        /* peter, 2007/7, upgrade to new version 1.58.2.16
           (rtest == 0 && !exact && (i32)(vp->namelen+1) < (i32) *length) || */
        (exact == 1
         && (rtest
             || (i32) *length !=
             (i32) (vp->namelen + getnum_poe_indices (table_num))))) {
        if (var_len) {
            *var_len = 0;
        }
        return MATCH_FAILED;
    }

    memset (newname, 0, sizeof (newname));

    if (((i32) *length) < (i32) vp->namelen + getnum_poe_indices (table_num)
        || rtest == -1) {
        memmove (newname, vp->name, (i32) vp->namelen * sizeof (oid));
        for (i = 0; i < getnum_poe_indices (table_num); i++) {
            newname[vp->namelen + i] = 1;
        }
        *length = vp->namelen + i;
        /* It will make the length equal to the number
         * of indices present in the table.
         */
        /* +++ peter, 2007/7, upgrade to new version 1.58.2.16 */
    } else if (((i32) *length) > (i32) vp->namelen + getnum_poe_indices (table_num)) {
        /* exact case checked earlier */
        *length = vp->namelen + getnum_poe_indices (table_num);
        memmove (newname, name, (*length) * sizeof (oid));
        if (name[*length - 1] < ULONG_MAX) {
            newname[*length - 1] = name[*length - 1] + 1;
        } else {
            newname[*length - 1] = name[*length - 1];
        }
        /* Careful not to overflow */
        /* --- peter, 2007/7, upgrade to new version 1.58.2.16 */
    } else {
        *length = vp->namelen + getnum_poe_indices (table_num);
        memmove (newname, name, (*length) * sizeof (oid));
        /* peter, 2007/7, upgrade to new version 1.58.2.16
           if (!exact) */
        if (!exact && name[*length - 1] < ULONG_MAX) {
            newname[*length - 1] = name[*length - 1] + 1;
        } else {
            newname[*length - 1] = name[*length - 1];
        }
    }
    if ((max >= 0 && (newname[*length - 1] > (oid)max)) ||
        (0 == newname[*length - 1])) {
        if (var_len) {
            *var_len = 0;
        }
        newname[*length - 2]++;
        newname[*length - 1] = 1;
        if (getnum_poe_indices (table_num) == 1) {
            return MATCH_FAILED;
        }
    }

    memmove (name, newname, (*length) * sizeof (oid));
    if (write_method) {
        *write_method = 0;
    }
    if (var_len) {
        *var_len = sizeof (i32);    /* default */
    }
    return (MATCH_SUCCEEDED);
}
/********************************************************************
* Purpose   : Retrieves the stack id of the switch.
* Arguments : idx_num: Given stack id number.
* Re-Entrant: Must be Re-entrant.
********************************************************************/

u32 get_available_group_index(u32 idx_num)
{
    u32 isid;
    for (isid = idx_num; isid < VTSS_ISID_END; isid++) {
        if (!msg_switch_exists (isid)) {
            continue;
        }
        return isid;
    }
    return isid;
}
/********************************************************************
* Purpose   : Retrieves the total power used by the switch module
              in the stack.
* Arguments : idx_num: stack id number.
* Re-Entrant: Must be Re-entrant.
********************************************************************/

u32 get_total_power(poe_status_t *poe_status)
{
    u32 port;
    u32 total_power = 0;
    for (port = 0; port < VTSS_FRONT_PORT_COUNT; port++) {
        total_power += poe_status->power_used[port];
    }
    return total_power;
}

/********************************************************************
 * Purpose   : Retrieve the PSE (Power Source Entity) table information
 * Arguments : isid: stack id, table_index: index for the table i.e. port
               number,table_entry: Pointer to the entire table entry.
 * Re-Entrant: Must be Re-entrant.
********************************************************************/
BOOL get_pethPsePortTable_entry(i32 isid, i32 table_index, pethPsePortTable_entry_t *table_entry)
{
    iftable_info_t         table_info;
    poe_status_t           poe_status;
    poe_custom_entry_t     hw_cfg;
    poe_local_conf_t       conf;

    if (!get_ifTableIndex_info (table_index, &table_info)) {
        return FALSE;
    }

    memset(&hw_cfg, 0, sizeof(hw_cfg));
    memset(&poe_status, 0, sizeof(poe_status));
    memset(&conf, 0, sizeof(conf));

    poe_mgmt_get_status(isid, &poe_status);
    poe_mgmt_get_local_config(&conf, isid);
    hw_cfg = poe_custom_get_hw_config(table_info.if_id, &hw_cfg);

    if (!hw_cfg.available) {
        return FALSE;
    }
    table_entry->peth_pse_port_admin_enable = (conf.poe_mode[table_info.if_id] == POE_MODE_POE_DISABLED) ? PSE_PORT_ADMIN_OFF : PSE_PORT_ADMIN_ON ;
    table_entry->peth_pse_port_power_pairs_control_ability = hw_cfg.pse_pairs_control_ability ? PSE_PORT_POWER_CONTROL_ENABLE : PSE_PORT_POWER_CONTROL_DISABLE;
    table_entry->peth_pse_port_power_pairs = (hw_cfg.pse_power_pair == PSE_PORT_POWER_PAIR_SIGNAL) ? PSE_PORT_POWER_PAIR_SIGNAL : PSE_PORT_POWER_PAIR_SPARE;
    table_entry->peth_pse_port_detection_status = PSE_PORT_DETECTION_DISABLED;
    if (conf.priority[table_info.if_id] == LOW) {
        table_entry->peth_pse_port_power_priority = PSE_PORT_POWER_PRIORITY_LOW;
    } else if (conf.priority[table_info.if_id] == HIGH) {
        table_entry->peth_pse_port_power_priority = PSE_PORT_POWER_PRIORITY_HIGH;
    } else {
        table_entry->peth_pse_port_power_priority = PSE_PORT_POWER_PRIORITY_CRITICAL;
    }

    table_entry->peth_pse_port_mps_absent_counter = 0;
    memset(table_entry->peth_pse_port_type, 0, sizeof(table_entry->peth_pse_port_type));
    table_entry->peth_pse_port_power_classifications = poe_status.pd_class[table_info.if_id];
    table_entry->peth_pse_port_invalid_signature_counter = 0;
    table_entry->peth_pse_port_power_denied_counter = 0;
    table_entry->peth_pse_port_over_load_counter = 0;
    table_entry->peth_pse_port_short_counter = 0;
    return TRUE;
}

/********************************************************************
 * Purpose   : Retrieves the Main PSE table contents
 * Arguments : isid: stack id,table_entry: Pointer to the entire table
               entry.
 * Re-Entrant: Must be Re-entrant.
********************************************************************/
BOOL get_pethMainPseTable_entry(i32 isid, pethMainPseTable_entry_t *table_entry)
{
    poe_status_t           poe_status;
    poe_custom_entry_t     hw_cfg;
    poe_local_conf_t       conf;
    u32                    chip;

    if (!msg_switch_exists (isid)) {
        return FALSE;
    }

    memset(&hw_cfg, 0, sizeof(hw_cfg));
    memset(&poe_status, 0, sizeof(poe_status));
    memset(&conf, 0, sizeof(conf));
    poe_mgmt_get_local_config(&conf, isid);
    poe_mgmt_get_status(isid, &poe_status);
    chip = poe_custom_is_chip_available();
    table_entry->peth_main_pse_power = conf.primary_power_supply;
    table_entry->peth_main_pse_oper_status = ((chip != NO_POE_CHIPSET_FOUND) ? MAIN_PSE_POWER_ON : MAIN_PSE_POWER_OFF);
    table_entry->peth_main_pse_consumption_power = get_total_power(&poe_status);
    table_entry->peth_main_pse_usage_threshold = 0;
    return TRUE;
}
/*
 * --- End (Internal implementation declarations)
 */

/*
 * pethObjects_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid             pethObjects_variables_oid[] = { 1, 3, 6, 1, 2, 1, 105, 1 };

/*
 * variable4 pethObjects_variables:
 *   this variable defines function callbacks and type return information
 *   for the pethObjects mib section
 */

struct variable4 pethObjects_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */
#define PETHPSEPORTADMINENABLE      3
    {
        PETHPSEPORTADMINENABLE, ASN_INTEGER, RWRITE, var_pethPsePortTable, 3,
        {1, 1, 3}
    },
#define PETHPSEPORTPOWERPAIRSCONTROLABILITY     4
    {
        PETHPSEPORTPOWERPAIRSCONTROLABILITY, ASN_INTEGER, RONLY,
        var_pethPsePortTable, 3, {1, 1, 4}
    },
#define PETHPSEPORTPOWERPAIRS       5
    {
        PETHPSEPORTPOWERPAIRS, ASN_INTEGER, RWRITE, var_pethPsePortTable, 3,
        {1, 1, 5}
    },
#define PETHPSEPORTDETECTIONSTATUS      6
    {
        PETHPSEPORTDETECTIONSTATUS, ASN_INTEGER, RONLY, var_pethPsePortTable,
        3, {1, 1, 6}
    },
#define PETHPSEPORTPOWERPRIORITY        7
    {
        PETHPSEPORTPOWERPRIORITY, ASN_INTEGER, RWRITE, var_pethPsePortTable,
        3, {1, 1, 7}
    },
#define PETHPSEPORTMPSABSENTCOUNTER     8
    {
        PETHPSEPORTMPSABSENTCOUNTER, ASN_COUNTER, RONLY, var_pethPsePortTable,
        3, {1, 1, 8}
    },
#define PETHPSEPORTTYPE     9
    {
        PETHPSEPORTTYPE, ASN_OCTET_STR, RWRITE, var_pethPsePortTable, 3,
        {1, 1, 9}
    },
#define PETHPSEPORTPOWERCLASSIFICATIONS     10
    {
        PETHPSEPORTPOWERCLASSIFICATIONS, ASN_INTEGER, RONLY,
        var_pethPsePortTable, 3, {1, 1, 10}
    },
#define PETHPSEPORTINVALIDSIGNATURECOUNTER      11
    {
        PETHPSEPORTINVALIDSIGNATURECOUNTER, ASN_COUNTER, RONLY,
        var_pethPsePortTable, 3, {1, 1, 11}
    },
#define PETHPSEPORTPOWERDENIEDCOUNTER       12
    {
        PETHPSEPORTPOWERDENIEDCOUNTER, ASN_COUNTER, RONLY,
        var_pethPsePortTable, 3, {1, 1, 12}
    },
#define PETHPSEPORTOVERLOADCOUNTER      13
    {
        PETHPSEPORTOVERLOADCOUNTER, ASN_COUNTER, RONLY, var_pethPsePortTable,
        3, {1, 1, 13}
    },
#define PETHPSEPORTSHORTCOUNTER     14
    {
        PETHPSEPORTSHORTCOUNTER, ASN_COUNTER, RONLY, var_pethPsePortTable, 3,
        {1, 1, 14}
    },
#define PETHMAINPSEGROUPINDEX       15
    {
        PETHMAINPSEGROUPINDEX, ASN_INTEGER, RONLY, var_pethMainPseTable, 4,
        {3, 1, 1, 1}
    },
#define PETHMAINPSEPOWER        16
    {
        PETHMAINPSEPOWER, ASN_GAUGE, RONLY, var_pethMainPseTable, 4,
        {3, 1, 1, 2}
    },
#define PETHMAINPSEOPERSTATUS       17
    {
        PETHMAINPSEOPERSTATUS, ASN_INTEGER, RONLY, var_pethMainPseTable, 4,
        {3, 1, 1, 3}
    },
#define PETHMAINPSECONSUMPTIONPOWER     18
    {
        PETHMAINPSECONSUMPTIONPOWER, ASN_GAUGE, RONLY, var_pethMainPseTable,
        4, {3, 1, 1, 4}
    },
#define PETHMAINPSEUSAGETHRESHOLD       19
    {
        PETHMAINPSEUSAGETHRESHOLD, ASN_INTEGER, RWRITE, var_pethMainPseTable,
        4, {3, 1, 1, 5}
    },
};

/*
 * (L = length of the oidsuffix)
 */


/*
 * Initializes the pethObjects module
 */
void init_pethObjects(void)
{
    // Register mibContextTable
    mibContextTable_register(pethObjects_variables_oid,
                             sizeof(pethObjects_variables_oid) / sizeof(oid),
                             "POWER-ETHERNET-MIB : pethObjects");

    DEBUGMSGTL(("pethObjects", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("pethObjects", pethObjects_variables, variable4,
                 pethObjects_variables_oid);

    /*
     * place any other initialization junk you need here
     */
}

/*
 * var_pethObjects():
 *   This function is called every time the agent gets a request for
 *   a scalar variable that might be found within your mib section
 *   registered above.  It is up to you to do the right thing and
 *   return the correct value.
 *     You should also correct the value of "var_len" if necessary.
 *
 *   Please see the documentation for more information about writing
 *   module extensions, and check out the examples in the examples
 *   and mibII directories.
 */
u_char         *
var_pethObjects(struct variable *vp,
                oid *name,
                size_t *length,
                int exact, size_t *var_len, WriteMethod **write_method)
{

    if (header_generic(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED) {
        return NULL;
    }

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_pethObjects\n",
                    vp->magic));
    }
    return NULL;
}

/*
 * var_pethPsePortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_pethObjects above.
 */
u_char         *
var_pethPsePortTable(struct variable *vp,
                     oid *name,
                     size_t *length,
                     int exact,
                     size_t *var_len, WriteMethod **write_method)
{

    i32                      table_size;
    i32                      idx_num_1;
    i32                      idx_num_2;
    pethPsePortTable_entry_t table_entry;
    i32                      table_index_1;
    i32                      table_index_2;
    static u32               ulong_ret;
    static u8                string[SPRINT_MAX_LEN];

    table_size = VTSS_FRONT_PORT_COUNT;

    /*
     * This assumes that the table is a 'simple' table.
     *  See the implementation documentation for the meaning of this.
     *  You will need to provide the correct value for the TABLE_SIZE parameter
     *
     * If this table does not meet the requirements for a simple table,
     *  you will need to provide the replacement code yourself.
     *  Mib2c is not smart enough to write this for you.
     *    Again, see the implementation documentation for what is required.
     */
    if (header_poe_table
        (vp, name, length, exact, var_len, write_method, table_size)
        == MATCH_FAILED) {
        return NULL;
    }

    idx_num_1 = name[(*length) - 2];

    table_index_1 = get_available_group_index(idx_num_1);

    if (table_index_1 >= VTSS_ISID_END) {
        return NULL;
    }

    idx_num_2 = name[(*length) - 1];

    table_index_2 = get_available_ifTableIndex (idx_num_2);

    if (exact && ((table_index_1 != idx_num_1) || (table_index_2 != idx_num_2) )) {
        return NULL;
    }

    memset(&table_entry, 0, sizeof(table_entry));

    if (!get_pethPsePortTable_entry(table_index_1, table_index_2, &table_entry)) {
        return NULL;
    }

    /*
     * Save this one as the "next one", if "table_index" is not sequence
     */
    name[(*length) - 1] = table_index_2;
    name[(*length) - 2] = table_index_1;

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    case PETHPSEPORTADMINENABLE: {
        *write_method = write_pethPsePortAdminEnable;
        ulong_ret = table_entry.peth_pse_port_admin_enable;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTPOWERPAIRSCONTROLABILITY: {
        ulong_ret = table_entry.peth_pse_port_power_pairs_control_ability;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTPOWERPAIRS: {
        ulong_ret = table_entry.peth_pse_port_power_pairs;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTDETECTIONSTATUS: {
        ulong_ret = table_entry.peth_pse_port_detection_status;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTPOWERPRIORITY: {
        *write_method = write_pethPsePortPowerPriority;
        ulong_ret = table_entry.peth_pse_port_power_priority;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTMPSABSENTCOUNTER: {
        ulong_ret = table_entry.peth_pse_port_mps_absent_counter;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTTYPE: {
        memcpy(string, table_entry.peth_pse_port_type, SNMP_ADMIN_STRING_LEN);
        *var_len = strlen((i8 *)string);
        return (u_char *)string;
    }
    case PETHPSEPORTPOWERCLASSIFICATIONS: {
        ulong_ret = table_entry.peth_pse_port_power_classifications;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTINVALIDSIGNATURECOUNTER: {
        ulong_ret = table_entry.peth_pse_port_invalid_signature_counter;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTPOWERDENIEDCOUNTER: {
        ulong_ret = table_entry.peth_pse_port_power_denied_counter;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTOVERLOADCOUNTER: {
        ulong_ret = table_entry.peth_pse_port_over_load_counter;
        return (u_char *) &ulong_ret;
    }
    case PETHPSEPORTSHORTCOUNTER: {
        ulong_ret = table_entry.peth_pse_port_short_counter;
        return (u_char *) &ulong_ret;
    }
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_pethPsePortTable\n",
                    vp->magic));
    }
    return NULL;
}

/*
 * var_pethMainPseTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_pethObjects above.
 */
u_char         *
var_pethMainPseTable(struct variable *vp,
                     oid *name,
                     size_t *length,
                     int exact,
                     size_t *var_len, WriteMethod **write_method)
{

    i32             table_size;
    i32             table_index;
    i32             idx_num;
    pethMainPseTable_entry_t table_entry;
    static u_long ulong_ret;

    table_size = VTSS_ISID_CNT;

    /*
     * This assumes that the table is a 'simple' table.
     *  See the implementation documentation for the meaning of this.
     *  You will need to provide the correct value for the TABLE_SIZE parameter
     *
     * If this table does not meet the requirements for a simple table,
     *  you will need to provide the replacement code yourself.
     *  Mib2c is not smart enough to write this for you.
     *    Again, see the implementation documentation for what is required.
     */
    if (header_simple_table
        (vp, name, length, exact, var_len, write_method, table_size)
        == MATCH_FAILED) {
        return NULL;
    }

    idx_num = name[(*length) - 1];

    table_index = get_available_group_index(idx_num);

    if (table_index == VTSS_ISID_END) {
        return NULL;
    }
    if (exact && (table_index != idx_num)) {
        return NULL;
    }

    memset(&table_entry, 0, sizeof(table_entry));

    if (!get_pethMainPseTable_entry(table_index, &table_entry)) {
        return NULL;
    }
    /*
     * Save this one as the "next one", if "table_index" is not sequence
     */
    name[(*length) - 1] = table_index;

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    case PETHMAINPSEPOWER: {
        ulong_ret = table_entry.peth_main_pse_power;
        return (u_char *) &ulong_ret;
    }
    case PETHMAINPSEOPERSTATUS: {
        ulong_ret = table_entry.peth_main_pse_oper_status;
        return (u_char *) &ulong_ret;
    }
    case PETHMAINPSECONSUMPTIONPOWER: {
        ulong_ret = table_entry.peth_main_pse_consumption_power;
        return (u_char *) &ulong_ret;
    }
    case PETHMAINPSEUSAGETHRESHOLD: {
        ulong_ret  = table_entry.peth_main_pse_usage_threshold;
        return (u_char *)&ulong_ret;
    }
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_pethMainPseTable\n",
                    vp->magic));
    }
    return NULL;
}


int
write_pethPsePortAdminEnable(int action,
                             u_char *var_val,
                             u_char var_val_type,
                             size_t var_val_len,
                             u_char *statP, oid *name, size_t name_len)
{
    u_long          *buf, *old_buf;
    u_long           temp_buf, temp_old_buf;
    int              log_rc = 0;
    vtss_isid_t      isid = 0;
    int              port_num;
    iftable_info_t   table_info;
    size_t           max_size;
    u_long           intval;
    poe_local_conf_t conf;

    isid = name[name_len - 2];
    port_num = name[name_len - 1];
    buf = &temp_buf;
    old_buf = &temp_old_buf;
    max_size = sizeof(u_long);
    intval = *((u_long *) var_val);


    switch (action) {
    case RESERVE1: {
        if (var_val_type != ASN_INTEGER) {
            log_rc = snmp_log(LOG_ERR,
                              "write to pethPsePortAdminEnable: not ASN_INTEGER\n");
            SNMP_DEBUG_LOG_ERROR_POE;
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > max_size) {
            log_rc = snmp_log(LOG_ERR,
                              "write to pethPsePortAdminEnable: bad length\n");
            SNMP_DEBUG_LOG_ERROR_POE;
            return SNMP_ERR_WRONGLENGTH;
        }
        if (intval != PSE_PORT_ADMIN_ON && intval != PSE_PORT_ADMIN_OFF) {
            log_rc = snmp_log(LOG_ERR,
                              "write to pethPsePortAdminEnable: bad value\n");
            SNMP_DEBUG_LOG_ERROR_POE;
            return SNMP_ERR_WRONGVALUE;
        }
        break;
    }
    case RESERVE2: {
        /*
         * Allocate memory and similar resources
         */
        break;
    }
    case FREE: {
        /*
         * Release any resources that have been allocated
         */
        break;
    }
    case ACTION: {
        /*
         * The variable has been stored in 'value' for you to use,
         * and you have just been asked to do something with it.
         * Note that anything done here must be reversable in the UNDO case
         */
        /*
         * Save to current configuration
         */
        if (!get_ifTableIndex_info (port_num, &table_info)) {
            return FALSE;
        }
        poe_mgmt_get_local_config(&conf, isid);
        if (intval == PSE_PORT_ADMIN_OFF) {
            conf.poe_mode[table_info.if_id] = POE_MODE_POE_DISABLED;
        } else {
            conf.poe_mode[table_info.if_id] = POE_MODE_POE;
        }
        poe_mgmt_set_local_config(&conf, isid);
        *buf = *((long *) var_val);
        break;
    }
    case UNDO: {
        /*
         * Back out any changes made in the ACTION case
         */
        /*
         * Restore current configuration form old configuration
         */
        *buf = *old_buf;
        break;
    }
    case COMMIT: {
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        /*
         * Update old configuration
         */
        *old_buf = *buf;
        break;
    }
    }
    return SNMP_ERR_NOERROR;
}

int
write_pethPsePortPowerPriority(int action,
                               u_char *var_val,
                               u_char var_val_type,
                               size_t var_val_len,
                               u_char *statP, oid *name, size_t name_len)
{
    u_long          *buf, *old_buf;
    u_long           temp_buf, temp_old_buf;
    int              log_rc = 0;
    vtss_isid_t      isid = 0;
    int              port_num;
    iftable_info_t   table_info;
    size_t           max_size;
    u_long           intval;
    poe_local_conf_t conf;

    isid = name[name_len - 2];
    port_num = name[name_len - 1];
    buf = &temp_buf;
    old_buf = &temp_old_buf;
    max_size = sizeof(u_long);
    intval = *((u_long *) var_val);

    switch (action) {
    case RESERVE1: {
        if (var_val_type != ASN_INTEGER) {
            log_rc = snmp_log(LOG_ERR,
                              "write to pethPsePortPowerPriority: not ASN_INTEGER\n");
            SNMP_DEBUG_LOG_ERROR_POE;
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > max_size) {
            log_rc = snmp_log(LOG_ERR,
                              "write to pethPsePortPowerPriority: bad length\n");
            SNMP_DEBUG_LOG_ERROR_POE;
            return SNMP_ERR_WRONGLENGTH;
        }
        if (intval < PSE_PORT_POWER_PRIORITY_CRITICAL || intval > PSE_PORT_POWER_PRIORITY_LOW) {
            log_rc = snmp_log(LOG_ERR,
                              "write to pethPsePortPowerPriority: bad value\n");
            SNMP_DEBUG_LOG_ERROR_POE;
            return SNMP_ERR_WRONGVALUE;
        }
        break;
    }
    case RESERVE2: {
        /*
         * Allocate memory and similar resources
         */
        break;
    }
    case FREE: {
        /*
         * Release any resources that have been allocated
         */
        break;
    }
    case ACTION: {
        /*
         * The variable has been stored in 'value' for you to use,
         * and you have just been asked to do something with it.
         * Note that anything done here must be reversable in the UNDO case
         */
        /*
         * Save to current configuration
         */
        if (!get_ifTableIndex_info (port_num, &table_info)) {
            return FALSE;
        }
        poe_mgmt_get_local_config(&conf, isid);
        if (intval == PSE_PORT_POWER_PRIORITY_CRITICAL) {
            conf.priority[table_info.if_id] = CRITICAL;
        } else if (intval == PSE_PORT_POWER_PRIORITY_HIGH) {
            conf.priority[table_info.if_id] = HIGH;
        } else {
            conf.priority[table_info.if_id] = LOW;
        }
        poe_mgmt_set_local_config(&conf, isid);

        *buf = *((long *) var_val);
        break;
    }
    case UNDO: {
        /*
         * Back out any changes made in the ACTION case
         */
        /*
         * Restore current configuration form old configuration
         */
        *buf = *old_buf;
        break;
    }
    case COMMIT: {
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        /*
         * Update old configuration
         */
        *old_buf = *buf;
        break;
    }
    }
    return SNMP_ERR_NOERROR;
}

