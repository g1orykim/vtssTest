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

// Note: This file originally auto-generated by mib2c using vtss_mib2c_ucd_snmp.conf v3.40

#ifndef _IEEE8021MSTPMIB_H_
#define _IEEE8021MSTPMIB_H_


#define IEEE8021MSTPMIB_STR_LEN_MAX    128

#define   NEWROOTTRAPINST              1
#define   TOPOLOGYCHANGETRAPINST       2


/******************************************************************************/
//
// Data structure declarations
//
/******************************************************************************/


// The table entry data structure for ieee8021MstpCistTable
typedef struct {
    // Entry keys
    u_long          ieee8021MstpCistComponentId;

    // Entry columns
    char
    ieee8021MstpCistBridgeIdentifier[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpCistBridgeIdentifier_len;
    long            ieee8021MstpCistTopologyChange;
    char
    ieee8021MstpCistRegionalRootIdentifier[IEEE8021MSTPMIB_STR_LEN_MAX
                                           + 1];
    size_t          ieee8021MstpCistRegionalRootIdentifier_len;
    u_long          ieee8021MstpCistPathCost;
    long            ieee8021MstpCistMaxHops;
} ieee8021MstpCistTable_entry_t;

// The table entry data structure for ieee8021MstpTable
typedef struct {
    // Entry keys
    u_long          ieee8021MstpComponentId;
    u_long          ieee8021MstpId;

    // Entry columns
    char            ieee8021MstpBridgeId[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpBridgeId_len;
    u_long          ieee8021MstpTimeSinceTopologyChange;
    struct counter64 ieee8021MstpTopologyChanges;
    long            ieee8021MstpTopologyChange;
    char            ieee8021MstpDesignatedRoot[IEEE8021MSTPMIB_STR_LEN_MAX
                                               + 1];
    size_t          ieee8021MstpDesignatedRoot_len;
    long            ieee8021MstpRootPathCost;
    u_long          ieee8021MstpRootPort;
    long            ieee8021MstpBridgePriority;
    char            ieee8021MstpVids0[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpVids0_len;
    char            ieee8021MstpVids1[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpVids1_len;
    char            ieee8021MstpVids2[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpVids2_len;
    char            ieee8021MstpVids3[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpVids3_len;
    long            ieee8021MstpRowStatus;
} ieee8021MstpTable_entry_t;

// The table entry data structure for ieee8021MstpCistPortTable
typedef struct {
    // Entry keys
    u_long          ieee8021MstpCistPortComponentId;
    u_long          ieee8021MstpCistPortNum;

    // Entry columns
    u_long          ieee8021MstpCistPortUptime;
    long            ieee8021MstpCistPortAdminPathCost;
    char
    ieee8021MstpCistPortDesignatedRoot[IEEE8021MSTPMIB_STR_LEN_MAX +
                                       1];
    size_t          ieee8021MstpCistPortDesignatedRoot_len;
    long            ieee8021MstpCistPortTopologyChangeAck;
    long            ieee8021MstpCistPortHelloTime;
    long            ieee8021MstpCistPortAdminEdgePort;
    long            ieee8021MstpCistPortOperEdgePort;
    long            ieee8021MstpCistPortMacEnabled;
    long            ieee8021MstpCistPortMacOperational;
    long            ieee8021MstpCistPortRestrictedRole;
    long            ieee8021MstpCistPortRestrictedTcn;
    long            ieee8021MstpCistPortRole;
    long            ieee8021MstpCistPortDisputed;
    char
    ieee8021MstpCistPortCistRegionalRootId[IEEE8021MSTPMIB_STR_LEN_MAX
                                           + 1];
    size_t          ieee8021MstpCistPortCistRegionalRootId_len;
    u_long          ieee8021MstpCistPortCistPathCost;
    long            ieee8021MstpCistPortProtocolMigration;
    long            ieee8021MstpCistPortEnableBPDURx;
    long            ieee8021MstpCistPortEnableBPDUTx;
    char
    ieee8021MstpCistPortPseudoRootId[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpCistPortPseudoRootId_len;
    long            ieee8021MstpCistPortIsL2Gp;
} ieee8021MstpCistPortTable_entry_t;

// The table entry data structure for ieee8021MstpPortTable
typedef struct {
    // Entry keys
    u_long          ieee8021MstpPortComponentId;
    u_long          ieee8021MstpPortMstId;
    u_long          ieee8021MstpPortNum;

    // Entry columns
    u_long          ieee8021MstpPortUptime;
    long            ieee8021MstpPortState;
    long            ieee8021MstpPortPriority;
    long            ieee8021MstpPortPathCost;
    char
    ieee8021MstpPortDesignatedRoot[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpPortDesignatedRoot_len;
    long            ieee8021MstpPortDesignatedCost;
    char
    ieee8021MstpPortDesignatedBridge[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpPortDesignatedBridge_len;
    u_long          ieee8021MstpPortDesignatedPort;
    long            ieee8021MstpPortRole;
    long            ieee8021MstpPortDisputed;
} ieee8021MstpPortTable_entry_t;

// The table entry data structure for ieee8021MstpFidToMstiTable
typedef struct {
    // Entry keys
    u_long          ieee8021MstpFidToMstiComponentId;
    u_long          ieee8021MstpFidToMstiFid;

    // Entry columns
    u_long          ieee8021MstpFidToMstiMstId;
} ieee8021MstpFidToMstiTable_entry_t;

// The table entry data structure for ieee8021MstpVlanTable
typedef struct {
    // Entry keys
    u_long          ieee8021MstpVlanComponentId;
    u_long          ieee8021MstpVlanId;

    // Entry columns
    u_long          ieee8021MstpVlanMstId;
} ieee8021MstpVlanTable_entry_t;

// The table entry data structure for ieee8021MstpConfigIdTable
typedef struct {
    // Entry keys
    u_long          ieee8021MstpConfigIdComponentId;

    // Entry columns
    long            ieee8021MstpConfigIdFormatSelector;
    char
    ieee8021MstpConfigurationName[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpConfigurationName_len;
    u_long          ieee8021MstpRevisionLevel;
    char
    ieee8021MstpConfigurationDigest[IEEE8021MSTPMIB_STR_LEN_MAX + 1];
    size_t          ieee8021MstpConfigurationDigest_len;
} ieee8021MstpConfigIdTable_entry_t;


/******************************************************************************/
//
// Initial function
//
/******************************************************************************/
/**
  * \brief Initializes the SNMP-part of the IEEE8021-MSTP-MIB:ieee8021MstpMib.
  **/
void            ieee8021MstpMib_init(void);


/******************************************************************************/
//
// Scalar access function declarations
//
/******************************************************************************/


/******************************************************************************/
//
// Table entry access function declarations
//
/******************************************************************************/
/**
  * \brief Get first table entry of ieee8021MstpCistTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get the first table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpCistTableEntry_getfirst(ieee8021MstpCistTable_entry_t *
                                    table_entry);

/**
  * \brief Get/Getnext table entry of ieee8021MstpCistTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get/getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpCistTableEntry_get(ieee8021MstpCistTable_entry_t *table_entry,
                               int getnext);

/**
  * \brief Set table entry of ieee8021MstpCistTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to set the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpCistTableEntry_set(ieee8021MstpCistTable_entry_t *
                               table_entry);
/**
  * \brief Get first table entry of ieee8021MstpTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get the first table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int             ieee8021MstpTableEntry_getfirst(ieee8021MstpTable_entry_t *
                                                table_entry);

/**
  * \brief Get/Getnext table entry of ieee8021MstpTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get/getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int             ieee8021MstpTableEntry_get(ieee8021MstpTable_entry_t *
                                           table_entry, int getnext);

/**
  * \brief Set table entry of ieee8021MstpTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to set the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int             ieee8021MstpTableEntry_set(ieee8021MstpTable_entry_t *
                                           table_entry);
/**
  * \brief Get first table entry of ieee8021MstpCistPortTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get the first table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpCistPortTableEntry_getfirst(ieee8021MstpCistPortTable_entry_t *
                                        table_entry);

/**
  * \brief Get/Getnext table entry of ieee8021MstpCistPortTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get/getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpCistPortTableEntry_get(ieee8021MstpCistPortTable_entry_t *
                                   table_entry, int getnext);

/**
  * \brief Set table entry of ieee8021MstpCistPortTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to set the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpCistPortTableEntry_set(ieee8021MstpCistPortTable_entry_t *
                                   table_entry);
/**
  * \brief Get first table entry of ieee8021MstpPortTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get the first table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpPortTableEntry_getfirst(ieee8021MstpPortTable_entry_t *
                                    table_entry);

/**
  * \brief Get/Getnext table entry of ieee8021MstpPortTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get/getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpPortTableEntry_get(ieee8021MstpPortTable_entry_t *table_entry,
                               int getnext);

/**
  * \brief Set table entry of ieee8021MstpPortTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to set the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpPortTableEntry_set(ieee8021MstpPortTable_entry_t *
                               table_entry);
/**
  * \brief Get first table entry of ieee8021MstpFidToMstiTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get the first table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpFidToMstiTableEntry_getfirst(ieee8021MstpFidToMstiTable_entry_t
                                         * table_entry);

/**
  * \brief Get/Getnext table entry of ieee8021MstpFidToMstiTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get/getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpFidToMstiTableEntry_get(ieee8021MstpFidToMstiTable_entry_t *
                                    table_entry, int getnext);

/**
  * \brief Set table entry of ieee8021MstpFidToMstiTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to set the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpFidToMstiTableEntry_set(ieee8021MstpFidToMstiTable_entry_t *
                                    table_entry);
/**
  * \brief Get first table entry of ieee8021MstpVlanTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get the first table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpVlanTableEntry_getfirst(ieee8021MstpVlanTable_entry_t *
                                    table_entry);

/**
  * \brief Get/Getnext table entry of ieee8021MstpVlanTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get/getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpVlanTableEntry_get(ieee8021MstpVlanTable_entry_t *table_entry,
                               int getnext);

/**
  * \brief Get first table entry of ieee8021MstpConfigIdTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get the first table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpConfigIdTableEntry_getfirst(ieee8021MstpConfigIdTable_entry_t *
                                        table_entry);

/**
  * \brief Get/Getnext table entry of ieee8021MstpConfigIdTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to get/getnext the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpConfigIdTableEntry_get(ieee8021MstpConfigIdTable_entry_t *
                                   table_entry, int getnext);

/**
  * \brief Set table entry of ieee8021MstpConfigIdTableEntry.
  *
  * \param table_entry [IN_OUT]: Pointer to structure that contains the table
  *                              entry to set the table entry.
  *
  * \return: 0 if the operation success, non-zero value otherwise.
  **/
int
ieee8021MstpConfigIdTableEntry_set(ieee8021MstpConfigIdTable_entry_t *
                                   table_entry);

#endif                          /* _IEEE8021MSTPMIB_H_ */
