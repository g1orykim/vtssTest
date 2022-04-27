/*
 *
 * Vitesse Switch Software.
 *
 * Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 * Rights Reserved.
 *
 * Unpublished rights reserved under the copyright laws of the United States of
 * America, other countries and international treaties. Permission to use, copy,
 * store and modify, the software and its source code is granted. Permission to
 * integrate into other products, disclose, transmit and distribute the software
 * in an absolute machine readable format (e.g. HEX file) is also granted.  The
 * source code of the software may not be disclosed, transmitted or distributed
 * without the written permission of Vitesse. The software and its source code
 * may only be used in products utilizing the Vitesse switch products.
 *
 * This copyright notice must appear in any copy, modification, disclosure,
 * transmission or distribution of the software. Vitesse retains all ownership,
 * copyright, trade secret and proprietary rights in the software.
 *
 * THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 * INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR USE AND NON-INFRINGEMENT.
 *
 */


#include <ucd-snmp/config.h>    /* For HAVE_STDLIB_H, etc.       */
#include <ucd-snmp/mibincl.h>   /* Standard set of SNMP includes */
#include <ucd-snmp/snmp_api.h>  /* snmp_oid_compare()            */

#include "snmp_mib_redefine.h"
#include "vtss_avl_tree_api.h"

/* Trace module */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_SNMP
#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

/* mutex for global protection */
#include "critd_api.h"
static critd_t SNMP_MIB_REDEFINE_global_critd_mutex;

#if (VTSS_TRACE_ENABLED)
#define SNMP_MIBS_REDEFINED_CRIT_ENTER() critd_enter(&SNMP_MIB_REDEFINE_global_critd_mutex, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SNMP_MIBS_REDEFINED_CRIT_EXIT()  critd_exit( &SNMP_MIB_REDEFINE_global_critd_mutex, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define SNMP_MIBS_REDEFINED_CRIT_ENTER() critd_enter(&SNMP_MIB_REDEFINE_global_critd_mutex)
#define SNMP_MIBS_REDEFINED_CRIT_EXIT()  critd_exit( &SNMP_MIB_REDEFINE_global_critd_mutex)
#endif /* VTSS_TRACE_ENABLED */

static BOOL SNMP_MIB_REDEFINE_global_init_flag = FALSE;

/* AVL Tree utility library */
static i32 SNMP_MIB_REDEFINE_compare_func(void *elm1, void *elm2);
VTSS_AVL_TREE(SNMP_MIB_REDEFINE_global_table, "SNMP_MIB_REDEFINED", VTSS_MODULE_ID_SNMP, SNMP_MIB_REDEFINE_compare_func, SNMP_MIB_REDEFINE_MAX_ENTRY_CNT)

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SNMP    // VTSS_MALLOC()


/******************************************************************************/
//
// Local functions
//
/******************************************************************************/
static i32 SNMP_MIB_REDEFINE_compare_func(void *element1, void *element2)
{
    snmp_mib_redefine_entry_t *elm1 = (snmp_mib_redefine_entry_t *)element1;
    snmp_mib_redefine_entry_t *elm2 = (snmp_mib_redefine_entry_t *)element2;
    int                         strcmp_rc, oid_cmp_rc;

    if ((strcmp_rc = strcmp(elm1->mib_name, elm2->mib_name)) > 0) {
        return 1;
    } else if (strcmp_rc < 0) {
        return -1;
    } else if ((oid_cmp_rc = snmp_oid_compare(elm1->oid, elm1->oid_len, elm2->oid, elm2->oid_len)) > 0) {
        return 1;
    } else if (oid_cmp_rc < 0) {
        return -1;
    } else {
        return 0;
    }
}


/******************************************************************************/
// Initial function
/******************************************************************************/
/**
  * \brief Initializes the SNMP MIB redefined Table.
  **/
void snmp_mib_redefine_init(void)
{
    T_D("enter");

    if (SNMP_MIB_REDEFINE_global_init_flag) {
        return;
    }
    SNMP_MIB_REDEFINE_global_init_flag = TRUE;

    /* Create semaphore for critical regions */
    critd_init(&SNMP_MIB_REDEFINE_global_critd_mutex, "SNMP_MIB_REDEFINE_global_critd_mutex", VTSS_MODULE_ID_SNMP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    if (vtss_avl_tree_init(&SNMP_MIB_REDEFINE_global_table) != TRUE) {
        T_E("Initialize fail");
    }
    SNMP_MIBS_REDEFINED_CRIT_EXIT();

    T_D("exit");
}


/******************************************************************************/
//
// snmpMibRedefineTable entry register
//
/******************************************************************************/
/**
  * \brief Register table entry of snmpMibRedefineTable
  *
  * \param oidin                [IN]: The MIB OID which will register to snmpMibRedefineTable.
  * \param oidlen               [IN]: The OID length of input parameter "oidin".
  * \param object               [IN]: The desciption of MIB node. Format: <MIB_File_Name> : <Scalar_or_Table_Name>.
  * \param syntax               [IN]: The syntax of MIB object.
  * \param standard_access_type [IN]: The access type that defined in standard MIB.
  * \param redefine_access_type [IN]: The access type that we redefine in our platform. If we only redefine the size, the value is the same as "standard_access_type".
  * \param redefine_size        [IN]: The boolean value if redefine size is needed. TURE if size is redefined.
  * \param redefine_descr       [IN]: The redefine desciption of MIB node.
  *                                   Define new range or size using the form {lowerbound upperboud}, for example: {3 100} {80 80} {128 256}
  *                                   Define new enumeration using the form {value label}, for example: {1 firstLabel} {2 secondLabel}
  *                                   If there are range/enum definition for the object, this parameter should not be NULL.
  *                                   Even if we doesn't redefine it. This parameter is used to generate the MIB filter file for Silver Creek.
  **/
void snmp_mib_redefine_register(oid *oidin, size_t oidlen, const char *object, const char *syntax, snmp_mib_access_type_t standard_access_type, snmp_mib_access_type_t redefine_access_type, BOOL redefine_size, const char *redefine_descr)
{
    snmp_mib_redefine_entry_t   *entry_p;
    char                        object_name[SNMP_MIB_REDEFINE_STR_LEN_MAX + 1];
    char                        *delimit = ":", *ch_p;

    /* Check input parameters */
    SNMP_MIBS_REDEFINED_CRIT_ENTER();
    if (oidlen == 0 ||
        oidlen > SNMP_MIB_REDEFINE_OID_LEN_MAX ||
        strlen(object) == 0 ||
        strlen(object) > SNMP_MIB_REDEFINE_STR_LEN_MAX ||
        strlen(syntax) > SNMP_MIB_REDEFINE_SYNTAX_STR_LEN_MAX ||
        strlen(redefine_descr) > SNMP_MIB_REDEFINE_STR_LEN_MAX) {
        T_E("Input parameter out of valid range");
        SNMP_MIBS_REDEFINED_CRIT_EXIT();
        return;
    }

    /* Allocate memory for table entry */
    if ((entry_p = (snmp_mib_redefine_entry_t *)VTSS_MALLOC(sizeof(*entry_p))) == NULL) {
        T_E("Memory allocated fail");
        SNMP_MIBS_REDEFINED_CRIT_EXIT();
        return;
    }

    /* Fill table entry elements */
    memset(entry_p, 0, sizeof(*entry_p));
    memcpy(entry_p->oid, oidin, sizeof(oid) * oidlen);
    entry_p->oid_len = oidlen;
    strcpy(object_name, object);
    if ((ch_p = strtok(object_name, delimit)) != NULL) {
        strcpy(entry_p->mib_name, ch_p);
    }
    if ((ch_p = strtok(NULL, delimit)) != NULL) {
        strcpy(entry_p->oid_name, ch_p);
    }
    strcpy(entry_p->syntax, syntax);
    entry_p->standard_access_type = standard_access_type;
    entry_p->redefine_access_type = redefine_access_type;
    entry_p->redefine_size = redefine_size;
    strcpy(entry_p->redefined_descr, redefine_descr);

    /* Add to AVL tree */
    if (vtss_avl_tree_add(&SNMP_MIB_REDEFINE_global_table, entry_p) != TRUE) {
        VTSS_FREE(entry_p);
        T_W("Register snmpMibRedefineTable failed");
    }
    SNMP_MIBS_REDEFINED_CRIT_EXIT();
}


/******************************************************************************/
//
// Table entry access functions
//
/******************************************************************************/
/**
  * \brief Getnext table entry of snmpMibRedefineTable
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
snmp_mib_redefine_entry_get_next(snmp_mib_redefine_entry_t *table_entry)
{
    int                         rc = -1;
    snmp_mib_redefine_entry_t  *entry_p = table_entry;

    T_D("enter");

    SNMP_MIBS_REDEFINED_CRIT_ENTER();
    if (vtss_avl_tree_get(&SNMP_MIB_REDEFINE_global_table, (void **) &entry_p, VTSS_AVL_TREE_GET_NEXT) == TRUE) {
        rc = 0;
        *table_entry = *entry_p;
    }
    SNMP_MIBS_REDEFINED_CRIT_EXIT();

    T_D("exit");
    return rc;
}
