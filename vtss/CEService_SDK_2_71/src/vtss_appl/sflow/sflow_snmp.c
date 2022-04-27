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

#include <ucd-snmp/config.h>              /* For HAVE_STDLIB_H, etc.        */
#include <ucd-snmp/mibincl.h>             /* Standard set of SNMP includes  */
#include <ucd-snmp/mibgroup/util_funcs.h> /* For header_generic()           */
#include "sflow_snmp.h"                   /* To check our own signatures    */
#include "sflow_api.h"                    /* Access to the sFlow API        */
#include "sflow_trace.h"                  /* Re-use sFlow T_x()             */
#include "misc_api.h"                     /* For misc_ip_txt()              */
#include "ifIndex_api.h"                  /* ifIndex to isid, iport conv.   */
#include "mibContextTable.h"              /* For mibContextTable_register() */

/*lint -esym(459, SFLOW_snmp_global_ret) */

/******************************************************************************/
// sFlowRcvrTable_entry_t
// The entry data structure for sFlowRcvrTable
/******************************************************************************/
typedef struct {
    // Entry key
    long rcvr_idx;

    // R/W Configuration
    sflow_rcvr_t rcvr;

    // R/O info.
    sflow_rcvr_info_t info;
} sFlowRcvrTable_entry_t;

/******************************************************************************/
// sflow_snmp_table_keys_t
/******************************************************************************/
typedef struct {
    u32 if_index;
    u32 instance;
} sflow_snmp_table_keys_t;

/******************************************************************************/
// sFlowFsTable_entry_t
// The entry data structure for sFlowFsTable
/******************************************************************************/
typedef struct {
    // Entry keys. This is a two-dimensional table.
    sflow_snmp_table_keys_t keys;

    // R/W Configuration
    sflow_fs_t fs;
} sFlowFsTable_entry_t;

/******************************************************************************/
// sFlowCpTable_entry_t
// The entry data structure for sFlowCpTable
/******************************************************************************/
typedef struct {
    // Entry keys. This is a two-dimensional table.
    sflow_snmp_table_keys_t keys;

    // R/W Configuration
    sflow_cp_t cp;
} sFlowCpTable_entry_t;

/******************************************************************************/
// The UCD-SNMP engine needs as address point for processing get operation
/******************************************************************************/
static struct {
    long   long_ret;
    char   string_ret[SPRINT_MAX_LEN];
    oid    objid_ret[MAX_OID_LEN];
    size_t objid_len_ret;
} SFLOW_snmp_global_ret;

/******************************************************************************/
// OID Suffixes
/******************************************************************************/
#define SFLOWVERSION                  1
#define SFLOWAGENTADDRESSTYPE         2
#define SFLOWAGENTADDRESS             3
#define SFLOWRCVROWNER                4
#define SFLOWRCVRTIMEOUT              5
#define SFLOWRCVRMAXIMUMDATAGRAMSIZE  6
#define SFLOWRCVRADDRESSTYPE          7
#define SFLOWRCVRADDRESS              8
#define SFLOWRCVRPORT                 9
#define SFLOWRCVRDATAGRAMVERSION     10
#define SFLOWFSRECEIVER              11
#define SFLOWFSPACKETSAMPLINGRATE    12
#define SFLOWFSMAXIMUMHEADERSIZE     13
#define SFLOWCPRECEIVER              14
#define SFLOWCPINTERVAL              15

/******************************************************************************/
//
// Common private functions
//
/******************************************************************************/

/******************************************************************************/
// SFLOW_snmp_ip2bin()
/******************************************************************************/
static void SFLOW_snmp_ip2bin(vtss_ip_addr_t *ip_addr, char *buf, size_t *len)
{
    if (ip_addr->type == VTSS_IP_TYPE_IPV4) {
        buf[0] = (ip_addr->addr.ipv4 >> 24) & 0xFF;
        buf[1] = (ip_addr->addr.ipv4 >> 16) & 0xFF;
        buf[2] = (ip_addr->addr.ipv4 >>  8) & 0xFF;
        buf[3] = (ip_addr->addr.ipv4 >>  0) & 0xFF;
        *len = 4;
    } else {
        memcpy(buf, ip_addr->addr.ipv6.addr, 16);
        *len = 16;
    }
}

/******************************************************************************/
// SFLOW_snmp_oid2str()
/******************************************************************************/
static char *SFLOW_snmp_oid2str(uchar magic)
{
    switch (magic) {
    case SFLOWVERSION:
        return "AgentVersion";

    case SFLOWAGENTADDRESSTYPE:
        return "AddressType";

    case SFLOWAGENTADDRESS:
        return "AgentAddress";

    case SFLOWRCVROWNER:
        return "RcvrOwner";

    case SFLOWRCVRTIMEOUT:
        return "RcvrTimeout";

    case SFLOWRCVRMAXIMUMDATAGRAMSIZE:
        return "RcvrMaximumDatagramSize";

    case SFLOWRCVRADDRESSTYPE:
        return "RcvrAddressType";

    case SFLOWRCVRADDRESS:
        return "RcvrAddress";

    case SFLOWRCVRPORT:
        return "RcvrPort";

    case SFLOWRCVRDATAGRAMVERSION:
        return "RcvrDatagramVersion";

    case SFLOWFSRECEIVER:
        return "FsRcvrIdx";

    case SFLOWFSPACKETSAMPLINGRATE:
        return "FsSamplingRate";

    case SFLOWFSMAXIMUMHEADERSIZE:
        return "FsMaximumHeaderSize";

    case SFLOWCPRECEIVER:
        return "CpRcvrIdx";

    case SFLOWCPINTERVAL:
        return "CpInterval";

    default:
        return "Unknown";
    }
}

/******************************************************************************/
// SFLOW_snmp_action2str()
/******************************************************************************/
static char *SFLOW_snmp_action2str(int action)
{
    switch (action) {
    case RESERVE1:
        return "RESERVE1";

    case RESERVE2:
        return "RESERVE2";

    case ACTION:
        return "ACTION";

    case COMMIT:
        return "COMMIT";

    case FREE:
        return "FREE";

    case UNDO:
        return "UNDO";

    case FINISHED_SUCCESS:
        return "FINISHED_SUCCESS";

    case FINISHED_FAILURE:
        return "FINISHED_FAILURE";

    default:
        return "Unknown";
    }
}

/******************************************************************************/
// SFLOW_snmp_ifindex_get()
//
// Returns the ifIndex of the first (#ifindex_input == 0) or
// the next (#ifindex_input > 0) non-stacking port.
// ifIndex == 0 is an invalid ifIndex, so that's safe. Another possiblity
// is to use VTSS_PORT_NO_NONE, but that would mix port numbers and if_indices.
//
// If an error occurs or there are no more non-stacking ports, the function
// returns 0. Otherwise it returns a valid ifIndex.
/******************************************************************************/
static u32 SFLOW_snmp_ifindex_getnext(u32 ifindex_input)
{
    iftable_info_t info;
    info.type    = IFTABLE_IFINDEX_TYPE_PORT;
    info.ifIndex = ifindex_input;

    do {
        if (ifIndex_get_next(&info) == FALSE) {
            return 0;
        }
        if (info.type != IFTABLE_IFINDEX_TYPE_PORT) {
            return 0;
        }
    } while (port_isid_port_no_is_stack(info.isid, info.if_id)); // Skip stack ports
    return info.ifIndex;
}

// This is the top level OID that we want to register under. This
// is essentially a prefix, with the suffix appearing in the
// variable below.
static oid SFLOW_snmp_oid[] = {
    1, 3, 6, 1, 4, 1, 14706, 1, 1
};

static const oid SFLOW_ifindex_oid[] = {
    1, 3, 6, 1, 2, 1, 2, 2, 1, 1
};

/******************************************************************************/
// SFLOW_snmp_oid_table_parse()
// Shared among FS and CP tables.
//
// Returns:
//  -1 on error
//   0 on getnext or getexact
//   1 on getfirst
//
// First index is the datasource, second is the instance.
//
// Layout:
// 1.3.6.1.4.1.14706.1.1: OID of sFlowAgent
// [5|6].1              : Points out sFlowFsEntry or sFlowCpEntry, which are both indexed by an <ifIndex, instance>
// [3|4|5]              : Members of the entry. [1|2] are keys, which come next.
// 11                   : Length of ifIndex OID + ifIndex itself
// 1.3.6.1.2.1.2.2.1.1  : ifIndex OID
// ifIndex              : ifIndex
// instance             : Instance.
/******************************************************************************/
static int SFLOW_snmp_oid_table_parse(oid *name, size_t length, int exact, sflow_snmp_table_keys_t *keys)
{
    int    len;
    u32    ifindex_oid_idx = ARRSZ(SFLOW_snmp_oid) + 3 + 1;
    u32    ifindex_len     = name[ifindex_oid_idx - 1];
    u32    exact_len       = ARRSZ(SFLOW_snmp_oid)    +
                             3                        +  // Entry member
                             1                        +  // ifIndex length
                             ARRSZ(SFLOW_ifindex_oid) +
                             1                        +  // port #
                             1;                          // instance #
    int cmp_rc = -1;

    if (length > exact_len) {
        // exact_len is the longest OID we support.
        // Returning -1 just means that the SNMP engine will continue
        // with the next OID if #exact == 0.
        return -1; // Error
    }

    if (length > ifindex_oid_idx) {
        // At least one element of the ifIndex OID contained in the input.

        // First check length value of the input
        if (ifindex_len > ARRSZ(SFLOW_ifindex_oid) + 1) {
            // The <= case is covered further down.
            return -1; // Error
        }

        // snmp_oid_compare() returns
        //   -1 if input <  ifIndex OID.
        //    0 if input == ifIndex OID
        //    1 if input >  ifIndex OID.

        // Compare at most real ifIndex OID elements.
        len = length - ifindex_oid_idx;
        if (len > (int)ARRSZ(SFLOW_ifindex_oid)) {
            len = ARRSZ(SFLOW_ifindex_oid);
        }

        cmp_rc = snmp_oid_compare(&name[ifindex_oid_idx], len, SFLOW_ifindex_oid, ARRSZ(SFLOW_ifindex_oid));
        if (cmp_rc > 0) {
            // If input OID is lexically greater than real ifIndex OID,
            // we return -1, which simply means that the SNMP engine
            // will continue with the next OID if #exact == 0.
            return -1; // Error
        }
    }

    if (exact) {
        // Everything must match.
        if (length      != exact_len                    || // Length must match exactly
            ifindex_len != ARRSZ(SFLOW_ifindex_oid) + 1 || // Exactly 11 elements in ifIndex OID (incl. port #)
            cmp_rc      != 0) {                            // ifIndex OID must match 100%
            return -1; // Error
        }
        keys->if_index = name[exact_len - 2];
        keys->instance = name[exact_len - 1];
        return 0;
    }

    // Get next. Check for get first.
    if (length      < ifindex_oid_idx              || // Not enough in input to check length of ifIndex OID
        ifindex_len < ARRSZ(SFLOW_ifindex_oid) + 1 || // ifIndex OID not completely specified.
        cmp_rc      < 0) {                            // input OID lexically smaller than that of a real ifIndex OID.
        return 1; // getfirst
    }

    memset(keys, 0, sizeof(*keys));

    if (length >= exact_len - 1) {
        keys->if_index = name[exact_len - 2];
    }
    if (length >= exact_len - 1) {
        keys->instance = name[exact_len - 1];
    }

    return 0; // getnext
}

/******************************************************************************/
// SFLOW_snmp_oid_table_fill()
// Fills in #name starting at ifIndex OID's length.
/******************************************************************************/
static void SFLOW_snmp_oid_table_fill(oid *name, size_t *length, sflow_snmp_table_keys_t *keys)
{
    int pos = ARRSZ(SFLOW_snmp_oid) + 3;

    // ifIndex OID
    name[pos++] = ARRSZ(SFLOW_ifindex_oid) + 1; // Length
    memcpy(&name[pos], SFLOW_ifindex_oid, sizeof(SFLOW_ifindex_oid));
    pos += ARRSZ(SFLOW_ifindex_oid);
    name[pos++] = (oid)keys->if_index;

    // Instance
    name[pos++] = (oid)keys->instance;
    *length = pos;
}

/******************************************************************************/
//
// sFlowAgent functions
//
/******************************************************************************/

/******************************************************************************/
// SFLOW_snmp_cb_agent()
/******************************************************************************/
static u_char *SFLOW_snmp_cb_agent(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
    sflow_agent_t agent_info;
    vtss_rc       rc;

    *write_method = NULL;

    if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED) {
        return NULL;
    }

    T_D("R(Agent.%s)", SFLOW_snmp_oid2str(vp->magic));

    if ((rc = sflow_mgmt_agent_cfg_get(&agent_info)) != VTSS_RC_OK) {
        T_E("%s", error_txt(rc));
        return NULL;
    }

    // This is where we do the value assignments for the MIB results.
    switch (vp->magic) {
    case SFLOWVERSION:
        strncpy(SFLOW_snmp_global_ret.string_ret, agent_info.version, sizeof(SFLOW_snmp_global_ret.string_ret));
        *var_len = strlen(SFLOW_snmp_global_ret.string_ret);
        return (u_char *)SFLOW_snmp_global_ret.string_ret;

    case SFLOWAGENTADDRESSTYPE:
        SFLOW_snmp_global_ret.long_ret = agent_info.agent_ip_addr.type; // Can be used directly according to its definition in ip_api.h
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    case SFLOWAGENTADDRESS:
        SFLOW_snmp_ip2bin(&agent_info.agent_ip_addr, SFLOW_snmp_global_ret.string_ret, var_len);
        return (u_char *)SFLOW_snmp_global_ret.string_ret;

    default:
        DEBUGMSGTL(("snmpd", "Unknown sub-id %d in SFLOW_snmp_cb_agent()\n", vp->magic));
    }

    return NULL;
}

/******************************************************************************/
//
// sFlowRcvrTable functions
//
/******************************************************************************/

/******************************************************************************/
// SFLOW_snmp_rcvr_get()
/******************************************************************************/
static vtss_rc SFLOW_snmp_rcvr_get(sFlowRcvrTable_entry_t *table_entry)
{
    vtss_rc rc;

    if ((rc = sflow_mgmt_rcvr_cfg_get(table_entry->rcvr_idx, &table_entry->rcvr, &table_entry->info)) != VTSS_RC_OK) {
        T_W("rc = %s", error_txt(rc));
    }
    return rc;
}

/******************************************************************************/
// SFLOW_snmp_rcvr_set()
/******************************************************************************/
static vtss_rc SFLOW_snmp_rcvr_set(sFlowRcvrTable_entry_t *table_entry)
{
    vtss_rc rc;
    if ((rc = sflow_mgmt_rcvr_cfg_set(table_entry->rcvr_idx, &table_entry->rcvr)) != VTSS_RC_OK) {
        T_W("rc = %s", error_txt(rc));
    }
    return rc;
}

/******************************************************************************/
// getfirst_sFlowRcvrTableEntry()
// Return non-zero value when fail.
/******************************************************************************/
static int getfirst_sFlowRcvrTableEntry(sFlowRcvrTable_entry_t *table_entry)
{
    table_entry->rcvr_idx = 1;
    return SFLOW_snmp_rcvr_get(table_entry) == VTSS_RC_OK ? 0 : 1;
}

/******************************************************************************/
// get_sFlowRcvrTableEntry()
// Return non-zero value when fail.
/******************************************************************************/
static int get_sFlowRcvrTableEntry(sFlowRcvrTable_entry_t *table_entry, BOOL getnext)
{
    if (getnext) {
        if (++table_entry->rcvr_idx > SFLOW_RECEIVER_CNT) {
            return 1;
        }
    }
    return SFLOW_snmp_rcvr_get(table_entry) == VTSS_RC_OK ? 0 : 1;
}

/******************************************************************************/
// parse_sFlowRcvrTable()
// Returns:
//  -1 on error
//   0 on getnext or getexact
//   1 on getfirst
//
// Only table_entry->rcvr_idx is updated when called with #exact == TRUE (i.e.
// remaining fields of table_entry must be updated manually.
/******************************************************************************/
static int parse_sFlowRcvrTable(oid *name, size_t length, int exact, sFlowRcvrTable_entry_t *table_entry)
{
    size_t op_pos = 10 + 2;
    oid    *op    = (oid *)(name + op_pos);

    if (exact && length < (10 + 1 + 1)) {
        return -1;
    } else if (!exact && length <= op_pos) {
        return getfirst_sFlowRcvrTableEntry(table_entry) ? -1 : 1; // Error : getfirst
    }

    memset(table_entry, 0, sizeof(*table_entry));
    if (length > op_pos) {
        table_entry->rcvr_idx = *(op++);
        op_pos++;
    } else {
        return exact ? -1 : 0; // Error : getnext
    }

    return (exact && length != op_pos) ? -1 : 0; // Error : getnext
}

/******************************************************************************/
// fillobj_sFlowRcvrTable()
/******************************************************************************/
static int fillobj_sFlowRcvrTable(oid *name, size_t *length, sFlowRcvrTable_entry_t *table_entry)
{
    int name_pos = 10 + 2;
    name[name_pos++] = (oid)table_entry->rcvr_idx;
    *length = name_pos;
    return 0;
}

/******************************************************************************/
// write_sFlowRcvrOwner()
/******************************************************************************/
static int write_sFlowRcvrOwner(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowRcvrTable_entry_t table_entry;

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG) /* Compile check */
    if (TRACE_IS_ENABLED(VTSS_TRACE_MODULE_ID, VTSS_TRACE_GRP_DEFAULT, VTSS_TRACE_LVL_DEBUG)) { /* Runtime check */
        size_t sz = var_val_len < sizeof(table_entry.rcvr.owner) - 1 ? var_val_len : sizeof(table_entry.rcvr.owner) - 1;
        memcpy(table_entry.rcvr.owner, var_val, sz);
        table_entry.rcvr.owner[sz] = 0;
        T_D("W(Rcvr.%s).%s = %s", SFLOW_snmp_oid2str(SFLOWRCVROWNER), SFLOW_snmp_action2str(action), table_entry.rcvr.owner);
    }
#endif

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_OCTET_STR) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrOwner: Not ASN_OCTET_STR\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > SPRINT_MAX_LEN) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrOwner: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (var_val_len > 127) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrOwner: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case COMMIT:
        if (parse_sFlowRcvrTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowRcvrTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        memcpy(table_entry.rcvr.owner, var_val, var_val_len);
        table_entry.rcvr.owner[var_val_len] = '\0';
        if (SFLOW_snmp_rcvr_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowRcvrTimeout()
/******************************************************************************/
static int write_sFlowRcvrTimeout(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowRcvrTable_entry_t table_entry;
    u_long                 set_value = var_val ? *((u_long *)var_val) : 0;

    T_D("W(Rcvr.%s).%s = %lu", SFLOW_snmp_oid2str(SFLOWRCVRTIMEOUT), SFLOW_snmp_action2str(action), set_value);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrTimeout: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrTimeout: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value > SFLOW_RECEIVER_TIMEOUT_MAX) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrTimeout: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        if (parse_sFlowRcvrTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowRcvrTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.rcvr.timeout = set_value;
        if (SFLOW_snmp_rcvr_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowRcvrMaximumDatagramSize()
/******************************************************************************/
static int write_sFlowRcvrMaximumDatagramSize(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowRcvrTable_entry_t table_entry;
    long                   set_value = var_val ? *((long *)var_val) : 0;

    T_D("W(Rcvr.%s).%s = %ld", SFLOW_snmp_oid2str(SFLOWRCVRMAXIMUMDATAGRAMSIZE), SFLOW_snmp_action2str(action), set_value);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrMaximumDatagramSize: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrMaximumDatagramSize: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value < SFLOW_RECEIVER_DATAGRAM_SIZE_MIN || set_value > SFLOW_RECEIVER_DATAGRAM_SIZE_MAX) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrAddressType: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        if (parse_sFlowRcvrTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowRcvrTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.rcvr.max_datagram_size = set_value;
        if (SFLOW_snmp_rcvr_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowRcvrAddressType()
/******************************************************************************/
static int write_sFlowRcvrAddressType(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    long set_value = var_val ? *((long *)var_val) : 0;

    T_D("W(Rcvr.%s).%s = %ld", SFLOW_snmp_oid2str(SFLOWRCVRADDRESSTYPE), SFLOW_snmp_action2str(action), set_value);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrAddressType: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrAddressType: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value != VTSS_IP_TYPE_IPV4
#ifdef VTSS_SW_OPTION_IPV6
            && set_value != VTSS_IP_TYPE_IPV6
#endif
           ) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrAddressType: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        // We react on setting the IP address in write_sFlowRcvrAddress().
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowRcvrAddress()
/******************************************************************************/
static int write_sFlowRcvrAddress(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowRcvrTable_entry_t table_entry;
    vtss_ip_addr_t         ip_addr;

    T_D("W(Rcvr.%s).%s = %u.%u.%u.%u", SFLOW_snmp_oid2str(SFLOWRCVRADDRESS), SFLOW_snmp_action2str(action), var_val[0], var_val[1], var_val[2], var_val[3]);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_OCTET_STR) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrAddress: Not ASN_OCTET_STR\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len != 4
#ifdef VTSS_SW_OPTION_IPV6
            && var_val_len != 16
#endif
           ) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrAddress: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case COMMIT:
        if (parse_sFlowRcvrTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowRcvrTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (var_val_len == 4) {
            ip_addr.type = VTSS_IP_TYPE_IPV4;
            ip_addr.addr.ipv4 = (var_val[0] << 24) | (var_val[1] << 16) | (var_val[2] << 8) | (var_val[3] << 0);
        } else {
            ip_addr.type = VTSS_IP_TYPE_IPV6;
            memcpy(ip_addr.addr.ipv6.addr, var_val, sizeof(ip_addr.addr.ipv6.addr));
        }
        (void)misc_ip_txt(&ip_addr, (char *)table_entry.rcvr.hostname);
        if (SFLOW_snmp_rcvr_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowRcvrPort()
/******************************************************************************/
static int write_sFlowRcvrPort(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowRcvrTable_entry_t table_entry;
    long                   set_value = var_val ? *((long *)var_val) : 0;

    T_D("W(Rcvr.%s).%s", SFLOW_snmp_oid2str(SFLOWRCVRPORT), SFLOW_snmp_action2str(action));

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrPort: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrPort: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value < 0 || set_value > SFLOW_RECEIVER_UDP_PORT_MAX) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrPort: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        if (parse_sFlowRcvrTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowRcvrTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.rcvr.udp_port = set_value;
        if (SFLOW_snmp_rcvr_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowRcvrDatagramVersion()
/******************************************************************************/
static int write_sFlowRcvrDatagramVersion(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowRcvrTable_entry_t table_entry;
    long                   set_value = var_val ? *((long *)var_val) : 0;

    T_D("W(Rcvr.%s).%s = %ld", SFLOW_snmp_oid2str(SFLOWRCVRDATAGRAMVERSION), SFLOW_snmp_action2str(action), set_value);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrDatagramVersion: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrDatagramVersion: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value != SFLOW_DATAGRAM_VERSION) {
            (void)snmp_log(LOG_ERR, "Write to sFlowRcvrAddressType: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        if (parse_sFlowRcvrTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowRcvrTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.rcvr.datagram_version = set_value;
        if (SFLOW_snmp_rcvr_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// SFLOW_snmp_cb_rcvr_table()
/******************************************************************************/
static u_char *SFLOW_snmp_cb_rcvr_table(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
    int                    rc;
    oid                    newname[MAX_OID_LEN];
    size_t                 newname_len;
    sFlowRcvrTable_entry_t table_entry;

    *write_method = NULL;
    memcpy((char *)newname, (char *)vp->name, (int)(vp->namelen * sizeof(oid)));
    newname_len = vp->namelen;
    if (memcmp(name, vp->name, vp->namelen * sizeof(oid)) != 0) {
        memcpy(name, vp->name, vp->namelen * sizeof(oid));
        *length = vp->namelen;
    }

    if ((rc = parse_sFlowRcvrTable(name, *length, exact, &table_entry)) < 0) {
        // Error
        return NULL;
    } else if (rc > 0) {
        // Get First
        if (fillobj_sFlowRcvrTable(newname, &newname_len, &table_entry)) {
            return NULL;
        }
    } else {
        // Get next
        do {
            if (get_sFlowRcvrTableEntry(&table_entry, exact ? FALSE : TRUE)) {
                return NULL;
            }
            if (fillobj_sFlowRcvrTable(newname, &newname_len, &table_entry)) {
                return NULL;
            }
            if (exact) {
                break;
            }
            rc = snmp_oid_compare(newname, newname_len, name, *length);
        } while (rc < 0);
    }

    // Fill in object part of name for current entry
    memcpy((char *)name, (char *)newname, (int)(newname_len * sizeof(oid)));
    *length = newname_len;

    T_D("R(Rcvr[%ld].%s)", table_entry.rcvr_idx, SFLOW_snmp_oid2str(vp->magic));

    // This is where we do the value assignments for the MIB results.
    switch (vp->magic) {
    case SFLOWRCVROWNER:
        *write_method = write_sFlowRcvrOwner;
        strcpy(SFLOW_snmp_global_ret.string_ret, table_entry.rcvr.owner);
        *var_len = strlen(SFLOW_snmp_global_ret.string_ret);
        return (u_char *)SFLOW_snmp_global_ret.string_ret;

    case SFLOWRCVRTIMEOUT:
        *write_method = write_sFlowRcvrTimeout;
        SFLOW_snmp_global_ret.long_ret = table_entry.info.timeout_left;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    case SFLOWRCVRMAXIMUMDATAGRAMSIZE:
        *write_method = write_sFlowRcvrMaximumDatagramSize;
        SFLOW_snmp_global_ret.long_ret = table_entry.rcvr.max_datagram_size;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    case SFLOWRCVRADDRESSTYPE:
        *write_method = write_sFlowRcvrAddressType;
        SFLOW_snmp_global_ret.long_ret = table_entry.info.ip_addr.type;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    case SFLOWRCVRADDRESS:
        *write_method = write_sFlowRcvrAddress;
        SFLOW_snmp_ip2bin(&table_entry.info.ip_addr, SFLOW_snmp_global_ret.string_ret, var_len);
        return (u_char *)SFLOW_snmp_global_ret.string_ret;

    case SFLOWRCVRPORT:
        *write_method = write_sFlowRcvrPort;
        SFLOW_snmp_global_ret.long_ret = table_entry.rcvr.udp_port;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    case SFLOWRCVRDATAGRAMVERSION:
        *write_method = write_sFlowRcvrDatagramVersion;
        SFLOW_snmp_global_ret.long_ret = table_entry.rcvr.datagram_version;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    default:
        T_E("Unknown sub-id (%d) Idx = %ld", vp->magic, table_entry.rcvr_idx);
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in SFLOW_snmp_cb_rcvr_table()\n", vp->magic));
    }

    return NULL;
}

/******************************************************************************/
//
// sFlowFsTable functions
//
/******************************************************************************/

/******************************************************************************/
// SFLOW_snmp_fs_get()
/******************************************************************************/
static vtss_rc SFLOW_snmp_fs_get(sFlowFsTable_entry_t *table_entry)
{
    vtss_rc        rc;
    iftable_info_t info;

    info.ifIndex = table_entry->keys.if_index;
    if (ifIndex_get(&info) == FALSE || info.type != IFTABLE_IFINDEX_TYPE_PORT) {
        return VTSS_RC_ERROR;
    }

    if ((rc = sflow_mgmt_flow_sampler_cfg_get(info.isid, (vtss_port_no_t)info.if_id, table_entry->keys.instance, &table_entry->fs)) != VTSS_RC_OK) {
        T_W("rc = %s", error_txt(rc));
    }
    return rc;
}

/******************************************************************************/
// SFLOW_snmp_fs_set()
/******************************************************************************/
static vtss_rc SFLOW_snmp_fs_set(sFlowFsTable_entry_t *table_entry)
{
    vtss_rc        rc;
    iftable_info_t info;

    // Convert ifIndex to <isid, port>
    info.ifIndex = table_entry->keys.if_index;
    if (ifIndex_get(&info) == FALSE || info.type != IFTABLE_IFINDEX_TYPE_PORT) {
        return VTSS_RC_ERROR;
    }

    if ((rc = sflow_mgmt_flow_sampler_cfg_set(info.isid, (vtss_port_no_t)info.if_id, table_entry->keys.instance, &table_entry->fs)) != VTSS_RC_OK) {
        T_W("rc = %s", error_txt(rc));
    }
    return rc;
}

/******************************************************************************/
// getfirst_sFlowFsTableEntry()
// Return non-zero value when fail
/******************************************************************************/
static int getfirst_sFlowFsTableEntry(sFlowFsTable_entry_t *table_entry)
{
    if ((table_entry->keys.if_index = SFLOW_snmp_ifindex_getnext(0)) == 0) {
        T_E("Odd. No normal front ports on this board?");
        return 1;
    }

    table_entry->keys.instance = 1;
    return SFLOW_snmp_fs_get(table_entry) == VTSS_RC_OK ? 0 : 1;
}

/******************************************************************************/
// get_sFlowFsTableEntry()
// The table is two-dimensional. Fastest running index is the instance number
// while the slowest running is the ifIndex (port number/data source).
// Return non-zero when failing.
/******************************************************************************/
static int get_sFlowFsTableEntry(sFlowFsTable_entry_t *table_entry, BOOL getnext)
{
    if (getnext) {
        if (++table_entry->keys.instance > SFLOW_INSTANCE_CNT) {
            // Instances exhausted. Back to first instance on next port.
            table_entry->keys.instance = 1;

            // Get next non-stack-port ifIndex.
            if ((table_entry->keys.if_index = SFLOW_snmp_ifindex_getnext(table_entry->keys.if_index)) == 0) {
                return 1;
            }
        }
    }

    return SFLOW_snmp_fs_get(table_entry) == VTSS_RC_OK ? 0 : 1;
}

/******************************************************************************/
// parse_sFlowFsTable()
// Returns:
//  -1 on error
//   0 on getnext or getexact
//   1 on getfirst
//
// First index is the datasource, second is the instance.
/******************************************************************************/
static int parse_sFlowFsTable(oid *name, size_t length, int exact, sFlowFsTable_entry_t *table_entry)
{
    int result = SFLOW_snmp_oid_table_parse(name, length, exact, &table_entry->keys);
    if (result == 1) {
        result = getfirst_sFlowFsTableEntry(table_entry) ? -1 : 1; // Error : getfirst

    }
    return result;
}

/******************************************************************************/
// write_sFlowFsReceiver()
/******************************************************************************/
static int write_sFlowFsReceiver(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowFsTable_entry_t table_entry;
    u_long               set_value = var_val ? *((u_long *)var_val) : 0;

    T_D("W(Fs.%s).%s = %lu", SFLOW_snmp_oid2str(SFLOWFSRECEIVER), SFLOW_snmp_action2str(action), set_value);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsReceiver: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsReceiver: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value > SFLOW_RECEIVER_CNT) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsReceiver: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        if (parse_sFlowFsTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowFsTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.fs.receiver = set_value;
        table_entry.fs.enabled  = set_value != 0;
        if (SFLOW_snmp_fs_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowFsPacketSamplingRate()
/******************************************************************************/
static int write_sFlowFsPacketSamplingRate(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowFsTable_entry_t table_entry;
    u_long               set_value = var_val ? *((u_long *)var_val) : 0;

    T_D("W(Fs.%s).%s = %lu", SFLOW_snmp_oid2str(SFLOWFSPACKETSAMPLINGRATE), SFLOW_snmp_action2str(action), set_value);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsPacketSamplingRate: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsPacketSamplingRate: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value > 0x7FFFFFFF) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsPacketSamplingRate: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        if (parse_sFlowFsTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowFsTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.fs.sampling_rate = set_value;
        if (SFLOW_snmp_fs_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowFsMaximumHeaderSize()
/******************************************************************************/
static int write_sFlowFsMaximumHeaderSize(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowFsTable_entry_t table_entry;
    long                 set_value = var_val ? *((long *)var_val) : 0;

    T_D("W(Fs.%s).%s", SFLOW_snmp_oid2str(SFLOWFSMAXIMUMHEADERSIZE), SFLOW_snmp_action2str(action));

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsMaximumHeaderSize: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsMaximumHeaderSize: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value < SFLOW_FLOW_HEADER_SIZE_MIN || set_value > SFLOW_FLOW_HEADER_SIZE_MAX) {
            (void)snmp_log(LOG_ERR, "Write to sFlowFsMaximumHeaderSize: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        if (parse_sFlowFsTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowFsTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.fs.max_header_size = set_value;
        if (SFLOW_snmp_fs_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// SFLOW_snmp_cb_fs_table()
/******************************************************************************/
static u_char *SFLOW_snmp_cb_fs_table(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
    int                  rc;
    oid                  newname[MAX_OID_LEN];
    size_t               newname_len;
    sFlowFsTable_entry_t table_entry;

    *write_method = NULL;
    memcpy((char *)newname, (char *)vp->name, (int)(vp->namelen * sizeof(oid)));
    newname_len = vp->namelen;
    if (memcmp(name, vp->name, sizeof(oid) * vp->namelen) != 0) {
        memcpy(name, vp->name, sizeof(oid) * vp->namelen);
        *length = vp->namelen;
    }

    if ((rc = parse_sFlowFsTable(name, *length, exact, &table_entry)) < 0) {
        return NULL;
    } else if (rc > 0) {       /* getfirst */
        SFLOW_snmp_oid_table_fill(newname, &newname_len, &table_entry.keys);
    } else {
        do {
            if (get_sFlowFsTableEntry(&table_entry, exact ? FALSE : TRUE)) {
                return NULL;
            }
            SFLOW_snmp_oid_table_fill(newname, &newname_len, &table_entry.keys);
            if (exact) {
                break;
            }
            rc = snmp_oid_compare(newname, newname_len, name, *length);
        } while (rc < 0);
    }

    // Fill in object part of name for current entry
    memcpy((char *)name, (char *)newname, (int)(newname_len * sizeof(oid)));
    *length = newname_len;

    T_D("R(Fs[%d][%d].%s)", table_entry.keys.if_index, table_entry.keys.instance, SFLOW_snmp_oid2str(vp->magic));

    // This is where we do the value assignments for the MIB results.
    switch (vp->magic) {
    case SFLOWFSRECEIVER:
        *write_method = write_sFlowFsReceiver;
        SFLOW_snmp_global_ret.long_ret = table_entry.fs.enabled ? table_entry.fs.receiver : 0;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    case SFLOWFSPACKETSAMPLINGRATE:
        *write_method = write_sFlowFsPacketSamplingRate;
        SFLOW_snmp_global_ret.long_ret = table_entry.fs.sampling_rate;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    case SFLOWFSMAXIMUMHEADERSIZE:
        *write_method = write_sFlowFsMaximumHeaderSize;
        SFLOW_snmp_global_ret.long_ret = table_entry.fs.max_header_size;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    default:
        DEBUGMSGTL(("snmpd", "Unknown sub-id %d in SFLOW_snmp_cb_fs_table()\n", vp->magic));
    }

    return NULL;
}

/******************************************************************************/
//
// sFlowCpTable functions
//
/******************************************************************************/

/******************************************************************************/
// SFLOW_snmp_cp_get()
/******************************************************************************/
static vtss_rc SFLOW_snmp_cp_get(sFlowCpTable_entry_t *table_entry)
{
    vtss_rc        rc;
    iftable_info_t info;

    info.ifIndex = table_entry->keys.if_index;
    if (ifIndex_get(&info) == FALSE || info.type != IFTABLE_IFINDEX_TYPE_PORT) {
        return VTSS_RC_ERROR;
    }

    if ((rc = sflow_mgmt_counter_poller_cfg_get(info.isid, (vtss_port_no_t)info.if_id, table_entry->keys.instance, &table_entry->cp)) != VTSS_RC_OK) {
        T_W("rc = %s", error_txt(rc));
    }
    return rc;
}

/******************************************************************************/
// SFLOW_snmp_cp_set()
/******************************************************************************/
static vtss_rc SFLOW_snmp_cp_set(sFlowCpTable_entry_t *table_entry)
{
    vtss_rc        rc;
    iftable_info_t info;

    // Convert ifIndex to <isid, port>
    info.ifIndex = table_entry->keys.if_index;
    if (ifIndex_get(&info) == FALSE || info.type != IFTABLE_IFINDEX_TYPE_PORT) {
        return VTSS_RC_ERROR;
    }

    if ((rc = sflow_mgmt_counter_poller_cfg_set(info.isid, (vtss_port_no_t)info.if_id, table_entry->keys.instance, &table_entry->cp)) != VTSS_RC_OK) {
        T_W("rc = %s", error_txt(rc));
    }
    return rc;
}

/******************************************************************************/
// getfirst_sFlowCpTableEntry()
// Return non-zero value when fail
/******************************************************************************/
static int getfirst_sFlowCpTableEntry(sFlowCpTable_entry_t *table_entry)
{
    if ((table_entry->keys.if_index = SFLOW_snmp_ifindex_getnext(0)) == 0) {
        T_E("Odd. No normal front ports on this board?");
        return 1;
    }

    table_entry->keys.instance = 1;
    return SFLOW_snmp_cp_get(table_entry) == VTSS_RC_OK ? 0 : 1;
}

/******************************************************************************/
// get_sFlowCpTableEntry()
// The table is two-dimensional. Fastest running index is the instance number
// while the slowest running is the ifIndex (port number/data source).
// Return non-zero when failing.
/******************************************************************************/
static int get_sFlowCpTableEntry(sFlowCpTable_entry_t *table_entry, BOOL getnext)
{
    if (getnext) {
        if (++table_entry->keys.instance > SFLOW_INSTANCE_CNT) {
            // Instances exhausted. Back to first instance on next port.
            table_entry->keys.instance = 1;

            // Get next non-stack-port ifIndex.
            if ((table_entry->keys.if_index = SFLOW_snmp_ifindex_getnext(table_entry->keys.if_index)) == 0) {
                return 1;
            }
        }
    }

    return SFLOW_snmp_cp_get(table_entry) == VTSS_RC_OK ? 0 : 1;
}

/******************************************************************************/
// parse_sFlowCpTableEntry()
// Returns:
//  -1 on error
//   0 on getnext or getexact
//   1 on getfirst
//
// First index is the datasource, second is the instance.
/******************************************************************************/
static int parse_sFlowCpTable(oid *name, size_t length, int exact, sFlowCpTable_entry_t *table_entry)
{
    int result = SFLOW_snmp_oid_table_parse(name, length, exact, &table_entry->keys);
    if (result == 1) {
        result = getfirst_sFlowCpTableEntry(table_entry) ? -1 : 1; // Error : getfirst

    }
    return result;
}

/******************************************************************************/
// write_sFlowCpReceiver()
/******************************************************************************/
static int write_sFlowCpReceiver(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowCpTable_entry_t table_entry;
    u_long               set_value = var_val ? *((u_long *)var_val) : 0;

    T_D("W(Cp.%s).%s = %lu", SFLOW_snmp_oid2str(SFLOWCPRECEIVER), SFLOW_snmp_action2str(action), set_value);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowCpReceiver: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowCpReceiver: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value > SFLOW_RECEIVER_CNT) {
            (void)snmp_log(LOG_ERR, "Write to sFlowCpReceiver: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }
        break;

    case COMMIT:
        if (parse_sFlowCpTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowCpTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.cp.receiver = set_value;
        table_entry.cp.enabled  = set_value != 0;
        if (SFLOW_snmp_cp_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// write_sFlowCpInterval()
/******************************************************************************/
static int write_sFlowCpInterval(int action, u_char *var_val, u_char var_val_type, size_t var_val_len, u_char *statP, oid *name, size_t name_len)
{
    sFlowCpTable_entry_t table_entry;
    long                 set_value = var_val ? *((long *)var_val) : 0;

    T_D("W(Cp.%s).%s = %ld", SFLOW_snmp_oid2str(SFLOWCPINTERVAL), SFLOW_snmp_action2str(action), set_value);

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            (void)snmp_log(LOG_ERR, "Write to sFlowCpInterval: Not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            (void)snmp_log(LOG_ERR, "Write to sFlowCpInterval: Bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if (set_value < SFLOW_POLLING_INTERVAL_MIN || set_value > SFLOW_POLLING_INTERVAL_MAX) {
            (void)snmp_log(LOG_ERR, "Write to sFlowCpInterval: Bad value\n");
            return SNMP_ERR_WRONGVALUE;
        }

        break;

    case COMMIT:
        if (parse_sFlowCpTable(name, name_len, TRUE, &table_entry) != 0) {
            return SNMP_ERR_COMMITFAILED;
        }
        if (get_sFlowCpTableEntry(&table_entry, FALSE)) {
            return SNMP_ERR_COMMITFAILED;
        }
        table_entry.cp.interval = set_value;
        if (SFLOW_snmp_cp_set(&table_entry) != VTSS_RC_OK) {
            return SNMP_ERR_COMMITFAILED;
        }
        break;
    }

    return SNMP_ERR_NOERROR;
}

/******************************************************************************/
// SFLOW_snmp_cb_cp_table()
/******************************************************************************/
static u_char *SFLOW_snmp_cb_cp_table(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
    int            rc;
    oid newname[MAX_OID_LEN];
    size_t         newname_len;
    sFlowCpTable_entry_t table_entry;
    *write_method = NULL;
    memcpy((char *) newname, (char *) vp->name, (int)(vp->namelen * sizeof(oid)));
    newname_len = vp->namelen;
    if (memcmp(name, vp->name, sizeof(oid) * vp->namelen) != 0) {
        memcpy(name, vp->name, sizeof(oid) * vp->namelen);
        *length = vp->namelen;
    }

    if ((rc = parse_sFlowCpTable(name, *length, exact, &table_entry)) < 0) {
        return NULL;
    } else if (rc > 0) {       /* getfirst */
        SFLOW_snmp_oid_table_fill(newname, &newname_len, &table_entry.keys);
    } else {
        do {
            if (get_sFlowCpTableEntry(&table_entry, exact ? FALSE : TRUE)) {
                return NULL;
            }
            SFLOW_snmp_oid_table_fill(newname, &newname_len, &table_entry.keys);
            if (exact) {
                break;
            }
            rc = snmp_oid_compare(newname, newname_len, name, *length);
        } while (rc < 0);
    }

    // Fill in object part of name for current entry
    memcpy((char *)name, (char *)newname, (int)(newname_len * sizeof(oid)));
    *length = newname_len;

    T_D("R(Cp[%d][%d].%s)", table_entry.keys.if_index, table_entry.keys.instance, SFLOW_snmp_oid2str(vp->magic));

    // This is where we do the value assignments for the MIB results.
    switch (vp->magic) {
    case SFLOWCPRECEIVER:
        *write_method = write_sFlowCpReceiver;
        SFLOW_snmp_global_ret.long_ret = table_entry.cp.enabled ? table_entry.cp.receiver : 0;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    case SFLOWCPINTERVAL:
        *write_method = write_sFlowCpInterval;
        SFLOW_snmp_global_ret.long_ret = table_entry.cp.interval;
        *var_len = sizeof(SFLOW_snmp_global_ret.long_ret);
        return (u_char *)&SFLOW_snmp_global_ret.long_ret;

    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in SFLOW_snmp_cb_cp_table()\n", vp->magic));
    }

    return NULL;
}

/******************************************************************************/
// This variable defines function callbacks and type return information
// for the sFlowAgent MIB section, and can't be defined until here unless
// we wanted to forward-declare the callback functions referenced.
//
// variable4 is declared as follows:
//  u_char          magic;          /* passed to function as a hint */
//  u_char          type;           /* type of variable */
//  u_short         acl;            /* access control list for variable */
//  FindVarMethod  *findVar;        /* function that finds variable */
//  u_char          namelen;        /* length of name below */
//  oid             name[4];        /* object identifier of variable */
/******************************************************************************/
static struct variable4 sFlowAgent_variables[] = {
    // Magic number,               variable type, ro/rw,  callback fn,              L, oidsuffix
    {SFLOWVERSION,                 ASN_OCTET_STR, RONLY,  SFLOW_snmp_cb_agent,      1, {1      }},
    {SFLOWAGENTADDRESSTYPE,        ASN_INTEGER,   RONLY,  SFLOW_snmp_cb_agent,      1, {2      }},
    {SFLOWAGENTADDRESS,            ASN_OCTET_STR, RONLY,  SFLOW_snmp_cb_agent,      1, {3      }},
    {SFLOWRCVROWNER,               ASN_OCTET_STR, RWRITE, SFLOW_snmp_cb_rcvr_table, 3, {4, 1, 2}},
    {SFLOWRCVRTIMEOUT,             ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_rcvr_table, 3, {4, 1, 3}},
    {SFLOWRCVRMAXIMUMDATAGRAMSIZE, ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_rcvr_table, 3, {4, 1, 4}},
    {SFLOWRCVRADDRESSTYPE,         ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_rcvr_table, 3, {4, 1, 5}},
    {SFLOWRCVRADDRESS,             ASN_OCTET_STR, RWRITE, SFLOW_snmp_cb_rcvr_table, 3, {4, 1, 6}},
    {SFLOWRCVRPORT,                ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_rcvr_table, 3, {4, 1, 7}},
    {SFLOWRCVRDATAGRAMVERSION,     ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_rcvr_table, 3, {4, 1, 8}},
    {SFLOWFSRECEIVER,              ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_fs_table,   3, {5, 1, 3}},
    {SFLOWFSPACKETSAMPLINGRATE,    ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_fs_table,   3, {5, 1, 4}},
    {SFLOWFSMAXIMUMHEADERSIZE,     ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_fs_table,   3, {5, 1, 5}},
    {SFLOWCPRECEIVER,              ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_cp_table,   3, {6, 1, 3}},
    {SFLOWCPINTERVAL,              ASN_INTEGER,   RWRITE, SFLOW_snmp_cb_cp_table,   3, {6, 1, 4}},
};

/******************************************************************************/
// PUBLIC FUNCTIONS
/******************************************************************************/

/******************************************************************************/
// sflow_snmp_init();
// Initializes the SNMP-part of the sFlow module.
// Only public function in this file.
/******************************************************************************/
void sflow_snmp_init(void)
{
    // Register mibContextTable
    mibContextTable_register(SFLOW_snmp_oid,
                             sizeof(SFLOW_snmp_oid) / sizeof(oid),
                             "SFLOW-MIB : sFlowAgent");

    DEBUGMSGTL(("sFlowAgent", "Initializing\n"));

    // Register ourselves with the agent to handle our MIB tree
    REGISTER_MIB("sFlowAgent", sFlowAgent_variables, variable4, SFLOW_snmp_oid);
}

