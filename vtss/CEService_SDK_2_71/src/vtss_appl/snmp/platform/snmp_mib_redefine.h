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


#ifndef _SNMP_MIB_REDEFINE_H_
#define _SNMP_MIB_REDEFINE_H_


#include <ucd-snmp/asn1.h>  // oid
#include "vtss_types.h"     // BOOL

#define SNMP_MIB_REDEFINE_SYNTAX_STR_LEN_MAX    63
#define SNMP_MIB_REDEFINE_STR_LEN_MAX           63
#define SNMP_MIB_REDEFINE_OID_LEN_MAX           32

#define SNMP_MIB_REDEFINE_MAX_ENTRY_CNT     256

typedef enum {
    SNMP_MIB_ACCESS_TYPE_NOT_IMPLEMENTED = 0,   // not implemented
    SNMP_MIB_ACCESS_TYPE_RONLY,                 // read-only
    SNMP_MIB_ACCESS_TYPE_RWRITE,                // read-write
    SNMP_MIB_ACCESS_TYPE_RCREATE                // read-create
} snmp_mib_access_type_t;

/******************************************************************************/
//
// Data structure declarations
//
/******************************************************************************/


// The table entry data structure for SNMP standard MIBs redefine
typedef struct {
    // Entry keys
    char    mib_name[SNMP_MIB_REDEFINE_STR_LEN_MAX + 1];   // primary
    oid     oid[SNMP_MIB_REDEFINE_OID_LEN_MAX + 1];        // secondary
    size_t  oid_len;

    // Entry columns
    char                    oid_name[SNMP_MIB_REDEFINE_STR_LEN_MAX + 1];
    char                    syntax[SNMP_MIB_REDEFINE_SYNTAX_STR_LEN_MAX + 1];
    snmp_mib_access_type_t  standard_access_type;
    snmp_mib_access_type_t  redefine_access_type;
    BOOL                    redefine_size;
    char                    redefined_descr[SNMP_MIB_REDEFINE_STR_LEN_MAX + 1];
} snmp_mib_redefine_entry_t;


/******************************************************************************/
//
// Initial function
//
/******************************************************************************/
/**
  * \brief Initializes the SNMP MIB redefined Table.
  **/
void snmp_mib_redefine_init(void);


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
void snmp_mib_redefine_register(oid *oidin, size_t oidlen, const char *object, const char *syntax, snmp_mib_access_type_t standard_access_type, snmp_mib_access_type_t redefine_access_type, BOOL redefine_size, const char *redefine_descr);


/******************************************************************************/
//
// Table entry access function declarations
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
int snmp_mib_redefine_entry_get_next(snmp_mib_redefine_entry_t *table_entry);


#endif /* _SNMP_MIB_REDEFINE_H_ */
