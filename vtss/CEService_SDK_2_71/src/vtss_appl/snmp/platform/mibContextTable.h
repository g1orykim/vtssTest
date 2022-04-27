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

#ifndef _MIBCONTEXTTABLE_H_
#define _MIBCONTEXTTABLE_H_


#include <ucd-snmp/asn1.h>  //oid


#define MIBCONTEXTTABLE_STR_LEN_MAX    63
#define MIBCONTEXTTABLE_OID_LEN_MAX    32


#define MIBCONTEXTTABLE_MAX_ENTRY_CNT    256


/******************************************************************************/
//
// Data structure declarations
//
/******************************************************************************/


// The table entry data structure for mibContextTable
typedef struct {
    // Entry keys
    char            mib_name[MIBCONTEXTTABLE_STR_LEN_MAX + 1];
    oid             oid[MIBCONTEXTTABLE_OID_LEN_MAX + 1];
    size_t          oid_len;

    // Entry columns
    char            descr[MIBCONTEXTTABLE_STR_LEN_MAX + 1];
    size_t          descr_len;
} mibContextTable_entry_t;


/******************************************************************************/
//
// Initial function
//
/******************************************************************************/
/**
  * \brief Initialize mibContextTable semaphore for critical regions.
  **/
void            mibContextTable_init(void);


/******************************************************************************/
//
// mibContextTable entry register
//
/******************************************************************************/
/**
  * \brief Register table entry of mibContextTable
  *
  * \param oidin  [IN]: The MIB OID which will register to mibContextTable
  * \param oidlen [IN]: The OID length of input parameter "oidin"
  * \param descr  [IN]: The desciption of MIB node. Format: <MIB_File_Name> : <Scalar_or_Table_Name>
  **/
void mibContextTable_register(oid *oidin, size_t oidlen, const char *descr);


/******************************************************************************/
//
// Table entry access function declarations
//
/******************************************************************************/
/**
  * \brief Getnext table entry by MIB name of mibContextTableEntry
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get/getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
mibContextTableEntry_getnext_by_mib_name(mibContextTable_entry_t *table_entry);


#endif                          /* _MIBCONTEXTTABLE_H_ */
