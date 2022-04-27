/**************************************************************
 * Copyright (C) 2001 Alex Rozin, Optical Access
 *
 *                     All Rights Reserved
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * ALEX ROZIN DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * ALEX ROZIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 ******************************************************************/

#include <main.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/infra/cyg_ass.h>
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

#ifdef VTSS_SW_OPTION_RMON
#include "vtss_snmp_api.h"
#include "rfc2819_rmon.h"
#include "ucd_snmp_rfc2819_rmon.h"
#include "rmon_api.h"
#endif


#ifdef EXTEND_RMON_TO_WEB_CLI

#define ROWAPI_header_ControlEntry          RMON_header_ControlEntry
#define ROWDATAAPI_header_DataEntry         RMON_header_DataEntry
#define ROWAPI_find                         RMON_find
#define ROWAPI_next                         RMON_next
#define ROWAPI_delete_clone                 RMON_delete_clone
#define rowapi_delete                       RMON_delete
#define ROWAPI_new                          RMON_new
#define ROWAPI_commit                       RMON_commit
#define ROWAPI_init_table                   RMON_init_table
#define ROWDATAAPI_init                     RMON_data_init
#define ROWDATAAPI_locate_new_data          RMON_locate_new_data
#define ROWDATAAPI_descructor               RMON_descructor
#define ROWDATAAPI_set_size                 RMON_set_size
#define ROWDATAAPI_get_total_number         RMON_get_total_number
#define ROWAPI_do_another_action            RMON_do_another_action
#endif



/*
 * +++ Start (Internal implementation declarations)
 */
#include "rmon_agutil_api.h"
#ifndef EXTEND_RMON_TO_WEB_CLI
#include "rmon_row_api.h"
#endif

/* statistics ----------------------------------------------------------*/
/*
* File scope definitions section
*/

#define etherStatsEntryFirstIndexBegin  11
#define historyControlEntryFirstIndexBegin      11
/*
 * File scope definitions section
 */
#define alarmEntryFirstIndexBegin       11
#define MMM_MAX                         0xFFFFFFFFl

#define eventEntryFirstIndexBegin       11

#ifndef EXTEND_RMON_TO_WEB_CLI
typedef struct {
    VAR_OID_T
    data_source;
    u_long
    etherStatsCreateTime;
    ETH_STATS_T
    eth;
} STAT_CRTL_ENTRY_T;
#else
typedef vtss_stat_ctrl_entry_t STAT_CRTL_ENTRY_T;
#endif

typedef vtss_history_ctrl_entry_t HISTORY_CRTL_ENTRY_T;
typedef vtss_history_data_entry_t HISTORY_DATA_ENTRY_T;
typedef vtss_alarm_ctrl_entry_t ALARM_CRTL_ENTRY_T;
typedef vtss_event_ctrl_entry_t EVENT_CRTL_ENTRY_T;
typedef vtss_event_data_entry_t EVENT_DATA_ENTRY_T;
/*
 * Main section
 */


/*
 * --- End (Internal implementation declarations)
 */

#if RFC2819_SUPPORTED_STATISTICS
/* statistics ----------------------------------------------------------*/
/*
 * statistics_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid             statistics_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 1 };

/*
 * variable4 statistics_variables:
 *   this variable defines function callbacks and type return information
 *   for the statistics mib section
 */

struct variable4 statistics_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */

#define ETHERSTATSINDEX     1
    {ETHERSTATSINDEX, ASN_INTEGER, RONLY, var_etherStatsTable, 3, {1, 1, 1}},
#define ETHERSTATSDATASOURCE        2
    {ETHERSTATSDATASOURCE, ASN_OBJECT_ID, RWRITE, var_etherStatsTable, 3, {1, 1, 2}},
#define ETHERSTATSDROPEVENTS        3
    {ETHERSTATSDROPEVENTS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 3}},
#define ETHERSTATSOCTETS        4
    {ETHERSTATSOCTETS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 4}},
#define ETHERSTATSPKTS      5
    {ETHERSTATSPKTS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 5}},
#define ETHERSTATSBROADCASTPKTS     6
    {ETHERSTATSBROADCASTPKTS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 6}},
#define ETHERSTATSMULTICASTPKTS     7
    {ETHERSTATSMULTICASTPKTS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 7}},
#define ETHERSTATSCRCALIGNERRORS        8
    {ETHERSTATSCRCALIGNERRORS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 8}},
#define ETHERSTATSUNDERSIZEPKTS     9
    {ETHERSTATSUNDERSIZEPKTS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 9}},
#define ETHERSTATSOVERSIZEPKTS      10
    {ETHERSTATSOVERSIZEPKTS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 10}},
#define ETHERSTATSFRAGMENTS     11
    {ETHERSTATSFRAGMENTS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 11}},
#define ETHERSTATSJABBERS       12
    {ETHERSTATSJABBERS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 12}},
#define ETHERSTATSCOLLISIONS        13
    {ETHERSTATSCOLLISIONS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 13}},
#define ETHERSTATSPKTS64OCTETS      14
    {ETHERSTATSPKTS64OCTETS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 14}},
#define ETHERSTATSPKTS65TO127OCTETS     15
    {ETHERSTATSPKTS65TO127OCTETS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 15}},
#define ETHERSTATSPKTS128TO255OCTETS        16
    {ETHERSTATSPKTS128TO255OCTETS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 16}},
#define ETHERSTATSPKTS256TO511OCTETS        17
    {ETHERSTATSPKTS256TO511OCTETS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 17}},
#define ETHERSTATSPKTS512TO1023OCTETS       18
    {ETHERSTATSPKTS512TO1023OCTETS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 18}},
#define ETHERSTATSPKTS1024TO1518OCTETS      19
    {ETHERSTATSPKTS1024TO1518OCTETS, ASN_COUNTER, RONLY, var_etherStatsTable, 3, {1, 1, 19}},
#define ETHERSTATSOWNER     20
    {ETHERSTATSOWNER, ASN_OCTET_STR, RWRITE, var_etherStatsTable, 3, {1, 1, 20}},
#define ETHERSTATSSTATUS        21
    {ETHERSTATSSTATUS, ASN_INTEGER, RWRITE, var_etherStatsTable, 3, {1, 1, 21}},
};

/*
 * (L = length of the oidsuffix)
 */

/*
 * Initializes the statistics module
 */
void
ucd_snmp_init_rmon_statisticsMIB(void)
{
    DEBUGMSGTL(("rmon/statistics", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("rmon/statistics", statistics_variables, variable4,
                 statistics_variables_oid);


}

/*
 * var_statistics():
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
var_statistics(struct variable *vp,
               oid *name,
               size_t *length,
               int exact, size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    if (header_generic(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED) {
        return NULL;
    }

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_statistics\n",
                    vp->magic));
    }
    return NULL;
}

static int
write_etherStatsTable(int action, u_char *var_val, u_char var_val_type,
                      size_t var_val_len, u_char *statP,
                      oid *name, size_t name_len)
{
    long                   long_temp;
    int                    leaf_id, snmp_status;
    static int             prev_action = COMMIT;
    RMON_ENTRY_T           *hdr;
    STAT_CRTL_ENTRY_T      *cloned_body;
    STAT_CRTL_ENTRY_T      *body;
    int                    max_size;
    long                   intval;

    switch (action) {
    case RESERVE1:
        leaf_id = (int) name[etherStatsEntryFirstIndexBegin - 1];

        switch (leaf_id) {
        case ETHERSTATSDATASOURCE:
            max_size = 12 * sizeof(oid);
            if (var_val_type != ASN_OBJECT_ID) {
                snmp_log(LOG_ERR,
                         "write to etherStatsDataSource: not ASN_OBJECT_ID\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to etherStatsDataSource: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case ETHERSTATSOWNER:
            /* Only supported read operation in E-StaX34 project */
            return SNMP_ERR_NOTWRITABLE;

            max_size = MAX_OWNERSTRING;
            if (var_val_type != ASN_OCTET_STR) {
                snmp_log(LOG_ERR,
                         "write to etherStatOwner: not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to etherStatOwner: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case ETHERSTATSSTATUS:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to etherStatStatus: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to etherStatStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval != 1 && intval != 2 && intval != 3 && intval != 4) {
                snmp_log(LOG_ERR, "write to etherStatStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        }
    case FREE:
    case UNDO:
    case ACTION:
    case COMMIT:
    default:
        snmp_status =
            RMON_do_another_action(name, etherStatsEntryFirstIndexBegin,
                                   action, &prev_action, RMON_STATS_TABLE_INDEX,
                                   sizeof(STAT_CRTL_ENTRY_T));
        if (SNMP_ERR_NOERROR != snmp_status) {
            ag_trace("failed action %d with %d", action, snmp_status);
            return SNMP_ERR_BADVALUE;
        }
        break;

    case RESERVE2:
        /*
         * get values from PDU, check them and save them in the cloned entry
         */
        long_temp = name[etherStatsEntryFirstIndexBegin];
        leaf_id = (int) name[etherStatsEntryFirstIndexBegin - 1];
        hdr = ROWAPI_find(RMON_STATS_TABLE_INDEX, long_temp);        /* it MUST be OK */
        cloned_body = (STAT_CRTL_ENTRY_T *) hdr->tmp;
        body = (STAT_CRTL_ENTRY_T *) hdr->body;
#ifdef EXTEND_RMON_TO_WEB_CLI
        cloned_body->id = long_temp;
#endif
        switch (leaf_id) {
        case ETHERSTATSDATASOURCE:
            snmp_status = AGUTIL_get_oid_value(var_val, var_val_type,
                                               var_val_len,
                                               &cloned_body->data_source);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                snmp_oid_compare(cloned_body->data_source.objid,
                                 cloned_body->data_source.length,
                                 body->data_source.objid,
                                 body->data_source.length)) {
                return SNMP_ERR_BADVALUE;
            }
            break;

        case ETHERSTATSOWNER:
            if (hdr->new_owner) {
                AGFREE(hdr->new_owner);
            }
            hdr->new_owner = AGMALLOC(MAX_OWNERSTRING);
            if (!hdr->new_owner) {
                return SNMP_ERR_TOOBIG;
            }
            snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                  var_val_len,
                                                  MAX_OWNERSTRING,
                                                  1, NULL, hdr->new_owner);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            break;
        case ETHERSTATSSTATUS:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               RMON1_ENTRY_VALID,
                                               RMON1_ENTRY_INVALID,
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                ag_trace("cannot browse etherStatsStatus");
                return snmp_status;
            }
            hdr->new_status = long_temp;
            break;
        default:
            ag_trace("%s:unknown leaf_id=%d\n", "Statistics",
                     (int) leaf_id);
            return SNMP_ERR_NOSUCHNAME;
        }                       /* of switch by 'leaf_id' */
        break;
    } /* of switch by 'action' */

    prev_action = action;
    return SNMP_ERR_NOERROR;
}

/*
 * var_etherStatsTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_statistics above.
 */
u_char         *
var_etherStatsTable(struct variable *vp,
                    oid *name,
                    size_t *length,
                    int exact,
                    size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    static u_long            ulong_ret;
    static u_char            string[256];
    static oid               objid[MAX_OID_LEN];
    static STAT_CRTL_ENTRY_T theEntry;
    RMON_ENTRY_T             *hdr;
    BOOL                     valid_flag = FALSE;

    *write_method = write_etherStatsTable;
    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     RMON_STATS_TABLE_INDEX,
                                     &theEntry, sizeof(STAT_CRTL_ENTRY_T));
    if (!hdr) {
        return NULL;
    }

    *var_len = sizeof(long);

    if (hdr->status == RMON1_ENTRY_VALID) {
        get_etherStatsTable_entry(&theEntry.data_source, &theEntry.eth);
        valid_flag = TRUE;
    }

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    case ETHERSTATSINDEX: {
        ulong_ret = hdr->ctrl_index;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSDATASOURCE: {
        //*write_method = write_etherStatsDataSource;
        memcpy(objid, theEntry.data_source.objid, theEntry.data_source.length * sizeof(oid));
        *var_len = theEntry.data_source.length * sizeof(oid);
        return (u_char *) objid;
    }
    case ETHERSTATSDROPEVENTS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.drop_events;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSOCTETS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.octets;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSPKTS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.packets;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSBROADCASTPKTS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.bcast_pkts;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSMULTICASTPKTS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.mcast_pkts;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSCRCALIGNERRORS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.crc_align;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSUNDERSIZEPKTS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.undersize;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSOVERSIZEPKTS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.oversize;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSFRAGMENTS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.fragments;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSJABBERS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.jabbers;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSCOLLISIONS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.collisions;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSPKTS64OCTETS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.pkts_64;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSPKTS65TO127OCTETS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.pkts_65_127;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSPKTS128TO255OCTETS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.pkts_128_255;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSPKTS256TO511OCTETS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.pkts_256_511;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSPKTS512TO1023OCTETS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.pkts_512_1023;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSPKTS1024TO1518OCTETS: {
        if (!valid_flag) {
            return NULL;
        }
        ulong_ret = theEntry.eth.pkts_1024_1518;
        return (u_char *) &ulong_ret;
    }
    case ETHERSTATSOWNER: {
        //*write_method = write_etherStatsOwner;
        if (hdr->owner) {
            strcpy(string, hdr->owner);
            *var_len = strlen(hdr->owner);
        } else {
            strcpy(string, "");
            *var_len = 0;
        }
        return (u_char *) string;
    }
    case ETHERSTATSSTATUS: {
        //*write_method = write_etherStatsStatus;
        ulong_ret = hdr->status;
        return (u_char *) &ulong_ret;
    }
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_etherStatsTable\n",
                    vp->magic));
    }
    return NULL;
}

#endif /* RFC2819_SUPPORTED_STATISTICS */

#if RFC2819_SUPPORTED_HISTORY
/* history ----------------------------------------------------------*/
/*
 * history_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid             history_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 2 };

/*
 * variable4 history_variables:
 *   this variable defines function callbacks and type return information
 *   for the history mib section
 */

struct variable4 history_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */

#define HISTORYCONTROLINDEX     1
    {HISTORYCONTROLINDEX, ASN_INTEGER, RONLY, var_historyControlTable, 3, {1, 1, 1}},
#define HISTORYCONTROLDATASOURCE        2
    {HISTORYCONTROLDATASOURCE, ASN_OBJECT_ID, RWRITE, var_historyControlTable, 3, {1, 1, 2}},
#define HISTORYCONTROLBUCKETSREQUESTED      3
    {HISTORYCONTROLBUCKETSREQUESTED, ASN_INTEGER, RWRITE, var_historyControlTable, 3, {1, 1, 3}},
#define HISTORYCONTROLBUCKETSGRANTED        4
    {HISTORYCONTROLBUCKETSGRANTED, ASN_INTEGER, RONLY, var_historyControlTable, 3, {1, 1, 4}},
#define HISTORYCONTROLINTERVAL      5
    {HISTORYCONTROLINTERVAL, ASN_INTEGER, RWRITE, var_historyControlTable, 3, {1, 1, 5}},
#define HISTORYCONTROLOWNER     6
    {HISTORYCONTROLOWNER, ASN_OCTET_STR, RONLY, var_historyControlTable, 3, {1, 1, 6}},
#define HISTORYCONTROLSTATUS        7
    {HISTORYCONTROLSTATUS, ASN_INTEGER, RWRITE, var_historyControlTable, 3, {1, 1, 7}},
#define ETHERHISTORYINDEX       8
    {ETHERHISTORYINDEX, ASN_INTEGER, RONLY, var_etherHistoryTable, 3, {2, 1, 1}},
#define ETHERHISTORYSAMPLEINDEX     9
    {ETHERHISTORYSAMPLEINDEX, ASN_INTEGER, RONLY, var_etherHistoryTable, 3, {2, 1, 2}},
#define ETHERHISTORYINTERVALSTART       10
    {ETHERHISTORYINTERVALSTART, ASN_TIMETICKS, RONLY, var_etherHistoryTable, 3, {2, 1, 3}},
#define ETHERHISTORYDROPEVENTS      11
    {ETHERHISTORYDROPEVENTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 4}},
#define ETHERHISTORYOCTETS      12
    {ETHERHISTORYOCTETS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 5}},
#define ETHERHISTORYPKTS        13
    {ETHERHISTORYPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 6}},
#define ETHERHISTORYBROADCASTPKTS       14
    {ETHERHISTORYBROADCASTPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 7}},
#define ETHERHISTORYMULTICASTPKTS       15
    {ETHERHISTORYMULTICASTPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 8}},
#define ETHERHISTORYCRCALIGNERRORS      16
    {ETHERHISTORYCRCALIGNERRORS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 9}},
#define ETHERHISTORYUNDERSIZEPKTS       17
    {ETHERHISTORYUNDERSIZEPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 10}},
#define ETHERHISTORYOVERSIZEPKTS        18
    {ETHERHISTORYOVERSIZEPKTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 11}},
#define ETHERHISTORYFRAGMENTS       19
    {ETHERHISTORYFRAGMENTS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 12}},
#define ETHERHISTORYJABBERS     20
    {ETHERHISTORYJABBERS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 13}},
#define ETHERHISTORYCOLLISIONS      21
    {ETHERHISTORYCOLLISIONS, ASN_COUNTER, RONLY, var_etherHistoryTable, 3, {2, 1, 14}},
#define ETHERHISTORYUTILIZATION     22
    {ETHERHISTORYUTILIZATION, ASN_INTEGER, RONLY, var_etherHistoryTable, 3, {2, 1, 15}},
};

/*
 * (L = length of the oidsuffix)
 */

/*
 * Initializes the history module
 */
void
ucd_snmp_init_rmon_historyMIB(void)
{
    DEBUGMSGTL(("rmon/history", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("rmon/history", history_variables, variable4,
                 history_variables_oid);


}

/*
 * var_history():
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
var_history(struct variable *vp,
            oid *name,
            size_t *length,
            int exact, size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    if (header_generic(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED) {
        return NULL;
    }

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_history\n",
                    vp->magic));
    }
    return NULL;
}

/*
 * var_historyControlTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_history above.
 */
u_char         *
var_historyControlTable(struct variable *vp,
                        oid *name,
                        size_t *length,
                        int exact,
                        size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    static u_long               ulong_ret;
    static u_char               string[256];
    static oid                  objid[MAX_OID_LEN];
    static vtss_history_ctrl_entry_t theEntry;
    RMON_ENTRY_T   *hdr;

    *write_method = write_historyControl;
    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     RMON_HISTORY_TABLE_INDEX,
                                     &theEntry, sizeof(HISTORY_CRTL_ENTRY_T));
    if (!hdr) {
        return NULL;
    }

    *var_len = sizeof(long);    /* default */

    switch (vp->magic) {
    case HISTORYCONTROLINDEX: {
        ulong_ret = hdr->ctrl_index;
        return (u_char *) &ulong_ret;
    }
    case HISTORYCONTROLDATASOURCE: {
        //*write_method = write_historyControlDataSource;
        memcpy(objid, theEntry.data_source.objid, theEntry.data_source.length * sizeof(oid));
        *var_len = theEntry.data_source.length * sizeof(oid);
        return (u_char *) objid;
    }
    case HISTORYCONTROLBUCKETSREQUESTED: {
        //*write_method = write_historyControlBucketsRequested;
        ulong_ret = theEntry.scrlr.data_requested;
        return (u_char *) &ulong_ret;
    }
    case HISTORYCONTROLBUCKETSGRANTED: {
        ulong_ret = theEntry.scrlr.data_granted;
        return (u_char *) &ulong_ret;
    }
    case HISTORYCONTROLINTERVAL: {
        //*write_method = write_historyControlInterval;
        ulong_ret = theEntry.interval;
        return (u_char *) &ulong_ret;
    }
    case HISTORYCONTROLOWNER: {
        //*write_method = write_historyControlOwner;
        if (hdr->owner) {
            strcpy(string, hdr->owner);
            *var_len = strlen(hdr->owner);
        } else {
            strcpy(string, "");
            *var_len = 0;
        }
        return (u_char *) string;
    }
    case HISTORYCONTROLSTATUS: {
        //*write_method = write_historyControlStatus;
        ulong_ret = hdr->status;
        return (u_char *) &ulong_ret;
    }
    default:
        DEBUGMSGTL(("snmpd",
                    "unknown sub-id %d in var_historyControlTable\n",
                    vp->magic));
    }
    return NULL;
}

/*
 * var_etherHistoryTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_history above.
 */
u_char         *
var_etherHistoryTable(struct variable *vp,
                      oid *name,
                      size_t *length,
                      int exact,
                      size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    static u_long               ulong_ret;
    static HISTORY_DATA_ENTRY_T theBucket;
    RMON_ENTRY_T                *hdr;
    HISTORY_CRTL_ENTRY_T        *ctrl;

    *write_method = NULL;
    hdr = ROWDATAAPI_header_DataEntry(vp, name, length, exact, var_len,
                                      RMON_HISTORY_TABLE_INDEX,
                                      sizeof(HISTORY_DATA_ENTRY_T), &theBucket);
    if (!hdr) {
        return NULL;
    }

    *var_len = sizeof(long);    /* default */

    ctrl = (HISTORY_CRTL_ENTRY_T *) hdr->body;

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    case ETHERHISTORYINDEX: {
        ulong_ret = hdr->ctrl_index;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYSAMPLEINDEX: {
        ulong_ret = theBucket.data_index;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYINTERVALSTART: {
        ulong_ret = theBucket.start_interval;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYDROPEVENTS: {
        ulong_ret = 0;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYOCTETS: {
        ulong_ret = theBucket.EthData.octets;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYPKTS: {
        ulong_ret = theBucket.EthData.packets;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYBROADCASTPKTS: {
        ulong_ret = theBucket.EthData.bcast_pkts;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYMULTICASTPKTS: {
        ulong_ret = theBucket.EthData.mcast_pkts;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYCRCALIGNERRORS: {
        ulong_ret = theBucket.EthData.crc_align;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYUNDERSIZEPKTS: {
        ulong_ret = theBucket.EthData.undersize;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYOVERSIZEPKTS: {
        ulong_ret = theBucket.EthData.oversize;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYFRAGMENTS: {
        ulong_ret = theBucket.EthData.fragments;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYJABBERS: {
        ulong_ret = theBucket.EthData.jabbers;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYCOLLISIONS: {
        ulong_ret = theBucket.EthData.collisions;
        return (u_char *) &ulong_ret;
    }
    case ETHERHISTORYUTILIZATION: {
        ulong_ret = theBucket.utilization;
        return (u_char *) &ulong_ret;
    }
    default:
        DEBUGMSGTL(("snmpd",
                    "unknown sub-id %d in var_etherHistoryTable\n",
                    vp->magic));
    }
    return NULL;
}

#endif /* RFC2819_SUPPORTED_HISTORY */

#if RFC2819_SUPPORTED_AlARM
/* alram ----------------------------------------------------------*/
/*
 * alarm_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid             alarm_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 3 };

/*
 * variable4 alarm_variables:
 *   this variable defines function callbacks and type return information
 *   for the alarm mib section
 */

struct variable4 alarm_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */

#define ALARMINDEX      1
    {ALARMINDEX, ASN_INTEGER, RONLY, var_alarmTable, 3, {1, 1, 1}},
#define ALARMINTERVAL       2
    {ALARMINTERVAL, ASN_INTEGER, RWRITE, var_alarmTable, 3, {1, 1, 2}},
#define ALARMVARIABLE       3
    {ALARMVARIABLE, ASN_OBJECT_ID, RWRITE, var_alarmTable, 3, {1, 1, 3}},
#define ALARMSAMPLETYPE     4
    {ALARMSAMPLETYPE, ASN_INTEGER, RWRITE, var_alarmTable, 3, {1, 1, 4}},
#define ALARMVALUE      5
    {ALARMVALUE, ASN_INTEGER, RONLY, var_alarmTable, 3, {1, 1, 5}},
#define ALARMSTARTUPALARM       6
    {ALARMSTARTUPALARM, ASN_INTEGER, RWRITE, var_alarmTable, 3, {1, 1, 6}},
#define ALARMRISINGTHRESHOLD        7
    {ALARMRISINGTHRESHOLD, ASN_INTEGER, RWRITE, var_alarmTable, 3, {1, 1, 7}},
#define ALARMFALLINGTHRESHOLD       8
    {ALARMFALLINGTHRESHOLD, ASN_INTEGER, RWRITE, var_alarmTable, 3, {1, 1, 8}},
#define ALARMRISINGEVENTINDEX       9
    {ALARMRISINGEVENTINDEX, ASN_INTEGER, RWRITE, var_alarmTable, 3, {1, 1, 9}},
#define ALARMFALLINGEVENTINDEX      10
    {ALARMFALLINGEVENTINDEX, ASN_INTEGER, RWRITE, var_alarmTable, 3, {1, 1, 10}},
#define ALARMOWNER      11
    {ALARMOWNER, ASN_OCTET_STR, RWRITE, var_alarmTable, 3, {1, 1, 11}},
#define ALARMSTATUS     12
    {ALARMSTATUS, ASN_INTEGER, RWRITE, var_alarmTable, 3, {1, 1, 12}},
};

/*
 * (L = length of the oidsuffix)
 */

/*
 * Initializes the alarm module
 */
void
ucd_snmp_init_rmon_alarmMIB(void)
{
    DEBUGMSGTL(("rmon/alarm", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("rmon/alarm", alarm_variables, variable4, alarm_variables_oid);


}

/*
 * var_alarm():
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
var_alarm(struct variable *vp,
          oid *name,
          size_t *length,
          int exact, size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    if (header_generic(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED) {
        return NULL;
    }

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_alarm\n",
                    vp->magic));
    }
    return NULL;
}

static int
write_alarmEntry(int action, u_char *var_val, u_char var_val_type,
                 size_t var_val_len, u_char *statP,
                 oid *name, size_t name_len)
{
    long               long_temp;
    int                leaf_id, snmp_status;
    static int         prev_action = COMMIT;
    RMON_ENTRY_T       *hdr;
    ALARM_CRTL_ENTRY_T *cloned_body;
    ALARM_CRTL_ENTRY_T *body;
    int                max_size;
    long               intval;

    switch (action) {
    case RESERVE1:
        leaf_id = (int) name[alarmEntryFirstIndexBegin - 1];

        switch (leaf_id) {
        case ALARMINTERVAL:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to alarmInterval: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmInterval: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case ALARMVARIABLE:
            max_size = 12 * sizeof(oid);
            if (var_val_type != ASN_OBJECT_ID) {
                snmp_log(LOG_ERR,
                         "write to alarmVariable: not ASN_OBJECT_ID\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmVariable: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case ALARMSAMPLETYPE:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to alarmSampleType: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmSampleType: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval != 1 && intval != 2) {
                snmp_log(LOG_ERR, "write to alarmSampleType: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case ALARMSTARTUPALARM:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to alarmStartupAlarm: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmStartupAlarm: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval != 1 && intval != 2 && intval != 3) {
                snmp_log(LOG_ERR, "write to alarmStartupAlarm: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case ALARMRISINGTHRESHOLD:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to alarmRisingThreshold: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmRisingThreshold: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case ALARMFALLINGTHRESHOLD:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to alarmFallingThreshold: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmFallingThreshold: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case ALARMRISINGEVENTINDEX:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to alarmRisingEventIndex: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmRisingEventIndex: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval < MIN_alarmEventIndex || intval > MAX_alarmEventIndex) {
                snmp_log(LOG_ERR, "write to alarmRisingEventIndex: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case ALARMFALLINGEVENTINDEX:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to alarmFallingEventIndex: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmFallingEventIndex: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval < MIN_alarmEventIndex || intval > MAX_alarmEventIndex) {
                snmp_log(LOG_ERR, "write to alarmFallingEventIndex: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case ALARMOWNER:
            /* Only supported read operation in E-StaX34 project */
            return SNMP_ERR_NOTWRITABLE;

            max_size = MAX_OWNERSTRING;
            if (var_val_type != ASN_OCTET_STR) {
                snmp_log(LOG_ERR,
                         "write to alarmOwner: not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmOwner: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case ALARMSTATUS:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to alarmStatus: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to alarmStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval != 1 && intval != 2 && intval != 3 && intval != 4) {
                snmp_log(LOG_ERR, "write to alarmStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        }
    case FREE:
    case UNDO:
    case ACTION:
    case COMMIT:
    default:
        snmp_status =
            ROWAPI_do_another_action(name, alarmEntryFirstIndexBegin,
                                     action, &prev_action, RMON_ALARM_TABLE_INDEX,
                                     sizeof(ALARM_CRTL_ENTRY_T));
        if (SNMP_ERR_NOERROR != snmp_status) {
            ag_trace("failed action %d with %d", action, snmp_status);
            return SNMP_ERR_BADVALUE;
        }
        break;

    case RESERVE2:
        /*
         * get values from PDU, check them and save them in the cloned entry
         */
        long_temp = name[alarmEntryFirstIndexBegin];
        leaf_id = (int) name[alarmEntryFirstIndexBegin - 1];
        hdr = ROWAPI_find(RMON_ALARM_TABLE_INDEX, long_temp);        /* it MUST be OK */
        if (hdr->status == RMON1_ENTRY_VALID) {
            if (leaf_id == ALARMINTERVAL ||
                leaf_id == ALARMVARIABLE ||
                leaf_id == ALARMSAMPLETYPE ||
                leaf_id == ALARMSTARTUPALARM ||
                leaf_id == ALARMRISINGTHRESHOLD ||
                leaf_id == ALARMFALLINGTHRESHOLD ||
                leaf_id == ALARMRISINGEVENTINDEX ||
                leaf_id == ALARMFALLINGEVENTINDEX) {
                /*'alarmInterval', 'alarmVariable',
                  'alarmSampleType', 'alarmStartupAlarm',
                  'alarmRisingThhreshold', 'alarmFallingThhreshold',
                  'alarmRisingEventIndex' and 'alarmFallingThhreshold'
                  not be modified if the associated 'alarmStatus' object
                  is equal to valid(1) */
                return SNMP_ERR_BADVALUE;
            }
        }
        cloned_body = (ALARM_CRTL_ENTRY_T *) hdr->tmp;
        body = (ALARM_CRTL_ENTRY_T *) hdr->body;
#ifdef EXTEND_RMON_TO_WEB_CLI
        cloned_body->id = long_temp;
#endif

        switch (leaf_id) {
        case ALARMINTERVAL:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               0, MMM_MAX, &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->interval = long_temp;
            break;

        case ALARMVARIABLE:
            snmp_status = AGUTIL_get_oid_value(var_val, var_val_type,
                                               var_val_len,
                                               &cloned_body->var_name);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                snmp_oid_compare(cloned_body->var_name.objid,
                                 cloned_body->var_name.length,
                                 body->var_name.objid,
                                 body->var_name.length)) {
                return SNMP_ERR_BADVALUE;
            }
            break;

        case ALARMSAMPLETYPE:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               SAMPLE_TYPE_ABSOLUTE,
                                               SAMPLE_TYPE_DELTA,
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->sample_type = long_temp;
            break;

        case ALARMSTARTUPALARM:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               ALARM_RISING,
                                               ALARM_BOTH, &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->startup_type = long_temp;
            break;

        case ALARMRISINGTHRESHOLD:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               0, MMM_MAX, &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->rising_threshold = long_temp;
            break;

        case ALARMFALLINGTHRESHOLD:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               0, 0xFFFFFFFFl, &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->falling_threshold = long_temp;
            break;

        case ALARMRISINGEVENTINDEX:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type, var_val_len, 0,   /* min. value */
                                               0,       /* max. value */
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->rising_event_index = long_temp;
            break;

        case ALARMFALLINGEVENTINDEX:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type, var_val_len, 0,   /* min. value */
                                               0,       /* max. value */
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->falling_event_index = long_temp;
            break;

        case ALARMOWNER:
            if (hdr->new_owner) {
                AGFREE(hdr->new_owner);
            }
            hdr->new_owner = AGMALLOC(MAX_OWNERSTRING);
            if (!hdr->new_owner) {
                return SNMP_ERR_TOOBIG;
            }
            snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                  var_val_len,
                                                  MAX_OWNERSTRING,
                                                  1, NULL, hdr->new_owner);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            break;
        case ALARMSTATUS:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               RMON1_ENTRY_VALID,
                                               RMON1_ENTRY_INVALID,
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                ag_trace("cannot browse etherStatsStatus");
                return snmp_status;
            }
            hdr->new_status = long_temp;
            break;
        default:
            ag_trace("%s:unknown leaf_id=%d\n", "Alarm",
                     (int) leaf_id);
            return SNMP_ERR_NOSUCHNAME;
        }                       /* of switch by 'leaf_id' */
        break;
    } /* of switch by 'action' */

    prev_action = action;
    return SNMP_ERR_NOERROR;
}

/*
 * var_alarmTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_alarm above.
 */
u_char         *
var_alarmTable(struct variable *vp,
               oid *name,
               size_t *length,
               int exact, size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    static u_long             ulong_ret;
    static u_char             string[256];
    static oid                objid[MAX_OID_LEN];
    static ALARM_CRTL_ENTRY_T theEntry;
    RMON_ENTRY_T   *hdr;

    *write_method = write_alarmEntry;
    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     RMON_ALARM_TABLE_INDEX,
                                     &theEntry, sizeof(ALARM_CRTL_ENTRY_T));
    if (!hdr) {
        return NULL;
    }

    *var_len = sizeof(long);    /* default */

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    case ALARMINDEX: {
        ulong_ret = hdr->ctrl_index;
        return (u_char *) &ulong_ret;
    }
    case ALARMINTERVAL: {
        //*write_method = write_alarmInterval;
        ulong_ret = theEntry.interval;
        return (u_char *) &ulong_ret;
    }
    case ALARMVARIABLE: {
        //*write_method = write_alarmVariable;
        memcpy(objid, theEntry.var_name.objid, theEntry.var_name.length * sizeof(oid));
        *var_len = theEntry.var_name.length * sizeof(oid);
        return (u_char *) objid;
    }
    case ALARMSAMPLETYPE: {
        //*write_method = write_alarmSampleType;
        ulong_ret = theEntry.sample_type;
        return (u_char *) &ulong_ret;
    }
    case ALARMVALUE: {
        ulong_ret = theEntry.value;
        return (u_char *) &ulong_ret;
    }
    case ALARMSTARTUPALARM: {
        //*write_method = write_alarmStartupAlarm;
        ulong_ret = theEntry.startup_type;
        return (u_char *) &ulong_ret;
    }
    case ALARMRISINGTHRESHOLD: {
        //*write_method = write_alarmRisingThreshold;
        ulong_ret = theEntry.rising_threshold;
        return (u_char *) &ulong_ret;
    }
    case ALARMFALLINGTHRESHOLD: {
        //*write_method = write_alarmFallingThreshold;
        ulong_ret = theEntry.falling_threshold;
        return (u_char *) &ulong_ret;
    }
    case ALARMRISINGEVENTINDEX: {
        //*write_method = write_alarmRisingEventIndex;
        ulong_ret = theEntry.rising_event_index;
        return (u_char *) &ulong_ret;
    }
    case ALARMFALLINGEVENTINDEX: {
        //*write_method = write_alarmFallingEventIndex;
        ulong_ret = theEntry.falling_event_index;
        return (u_char *) &ulong_ret;
    }
    case ALARMOWNER: {
        //*write_method = write_alarmOwner;
        if (hdr->owner) {
            strcpy(string, hdr->owner);
            *var_len = strlen(hdr->owner);
        } else {
            strcpy(string, "");
            *var_len = 0;
        }
        return (u_char *) string;
    }
    case ALARMSTATUS: {
        //*write_method = write_alarmStatus;
        ulong_ret = hdr->status;
        return (u_char *) &ulong_ret;
    }
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_alarmTable\n",
                    vp->magic));
    }
    return NULL;
}

#endif /* RFC2819_SUPPORTED_AlARM */

#if RFC2819_SUPPORTED_EVENT
/* event ----------------------------------------------------------*/
/*
 * event_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid             event_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 9 };

/*
 * variable4 event_variables:
 *   this variable defines function callbacks and type return information
 *   for the event mib section
 */

struct variable4 event_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix
     */

#define EVENTINDEX      1
    {EVENTINDEX, ASN_INTEGER, RONLY, var_eventTable, 3, {1, 1, 1}},
#define EVENTDESCRIPTION        2
    {EVENTDESCRIPTION, ASN_OCTET_STR, RWRITE, var_eventTable, 3, {1, 1, 2}},
#define EVENTTYPE       3
    {EVENTTYPE, ASN_INTEGER, RWRITE, var_eventTable, 3, {1, 1, 3}},
#define EVENTCOMMUNITY      4
    {EVENTCOMMUNITY, ASN_OCTET_STR, RWRITE, var_eventTable, 3, {1, 1, 4}},
#define EVENTLASTTIMESENT       5
    {EVENTLASTTIMESENT, ASN_TIMETICKS, RONLY, var_eventTable, 3, {1, 1, 5}},
#define EVENTOWNER      6
    {EVENTOWNER, ASN_OCTET_STR, RONLY, var_eventTable, 3, {1, 1, 6}},
#define EVENTSTATUS     7
    {EVENTSTATUS, ASN_INTEGER, RWRITE, var_eventTable, 3, {1, 1, 7}},
#define LOGEVENTINDEX       8
    {LOGEVENTINDEX, ASN_INTEGER, RONLY, var_logTable, 3, {2, 1, 1}},
#define LOGINDEX        9
    {LOGINDEX, ASN_INTEGER, RONLY, var_logTable, 3, {2, 1, 2}},
#define LOGTIME     10
    {LOGTIME, ASN_TIMETICKS, RONLY, var_logTable, 3, {2, 1, 3}},
#define LOGDESCRIPTION      11
    {LOGDESCRIPTION, ASN_OCTET_STR, RONLY, var_logTable, 3, {2, 1, 4}},
};

/*
 * (L = length of the oidsuffix)
 */

/*
 * Initializes the event module
 */
void
ucd_snmp_init_rmon_eventMIB(void)
{
    DEBUGMSGTL(("rmon/event", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("rmon/event", event_variables, variable4,
                 event_variables_oid);
}

/*
 * var_event():
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
var_event(struct variable *vp,
          oid *name,
          size_t *length,
          int exact, size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    if (header_generic(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED) {
        return NULL;
    }

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_event\n",
                    vp->magic));
    }
    return NULL;
}

/*
 * var_eventTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_event above.
 */
u_char         *
var_eventTable(struct variable *vp,
               oid *name,
               size_t *length,
               int exact, size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    static u_long             ulong_ret;
    static u_char             string[256];
    static EVENT_CRTL_ENTRY_T theEntry;
    RMON_ENTRY_T              *hdr;


    *write_method = write_eventControl;
    hdr = ROWAPI_header_ControlEntry(vp, name, length, exact, var_len,
                                     RMON_EVENT_TABLE_INDEX,
                                     &theEntry, sizeof(EVENT_CRTL_ENTRY_T));
    if (!hdr) {
        return NULL;
    }

    *var_len = sizeof(long);    /* default */

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    case EVENTINDEX: {
        ulong_ret = hdr->ctrl_index;
        return (u_char *) &ulong_ret;
    }
    case EVENTDESCRIPTION: {
        //*write_method = write_eventDescription;
        if (theEntry.event_description) {
            strcpy(string, theEntry.event_description);
            *var_len = strlen(theEntry.event_description);
        } else {
            strcpy(string, "");
            *var_len = 0;
        }
        return (u_char *) string;
    }
    case EVENTTYPE: {
        //*write_method = write_eventType;
        ulong_ret = theEntry.event_type;
        return (u_char *) &ulong_ret;
    }
    case EVENTCOMMUNITY: {
        //*write_method = write_eventCommunity;
        if (theEntry.event_community) {
            strcpy(string, theEntry.event_community);
            *var_len = strlen(theEntry.event_community);
        } else {
            strcpy(string, "");
            *var_len = 0;
        }
        return (u_char *) string;
    }
    case EVENTLASTTIMESENT: {
        ulong_ret = theEntry.event_last_time_sent;
        return (u_char *) &ulong_ret;
    }
    case EVENTOWNER: {
        //*write_method = write_eventOwner;
        if (hdr->owner) {
            strcpy(string, hdr->owner);
            *var_len = strlen(hdr->owner);
        } else {
            strcpy(string, "");
            *var_len = 0;
        }
        return (u_char *) string;
    }
    case EVENTSTATUS: {
        //*write_method = write_eventStatus;
        ulong_ret = hdr->status;
        return (u_char *) &ulong_ret;
    }
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_eventTable\n",
                    vp->magic));
    }
    return NULL;
}

/*
 * var_logTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_event above.
 */
u_char         *
var_logTable(struct variable *vp,
             oid *name,
             size_t *length,
             int exact, size_t *var_len, WriteMethod **write_method)
{
    /*
     * variables we may use later
     * static long long_ret;
     * static u_long ulong_ret;
     * static u_char string[SPRINT_MAX_LEN];
     * static oid objid[MAX_OID_LEN];
     * static struct counter64 c64;
     */

    static u_long             ulong_ret;
    static u_char             string[256];
    static EVENT_DATA_ENTRY_T theEntry;
    RMON_ENTRY_T              *hdr;
    EVENT_CRTL_ENTRY_T        *ctrl;

    *write_method = NULL;
    hdr = ROWDATAAPI_header_DataEntry(vp, name, length, exact, var_len,
                                      RMON_EVENT_TABLE_INDEX,
                                      sizeof(EVENT_DATA_ENTRY_T), &theEntry);
    if (!hdr) {
        return NULL;
    }

    ctrl = (EVENT_CRTL_ENTRY_T *) hdr->body;

    *var_len = sizeof(long);    /* default */

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    case LOGEVENTINDEX: {
        ulong_ret = hdr->ctrl_index;
        return (u_char *) &ulong_ret;
    }
    case LOGINDEX: {
        ulong_ret = theEntry.data_index;
        return (u_char *) &ulong_ret;
    }
    case LOGTIME: {
        ulong_ret = theEntry.log_time;
        return (u_char *) &ulong_ret;
    }
    case LOGDESCRIPTION: {
        if (theEntry.log_description) {
            strcpy(string, theEntry.log_description);
            *var_len = strlen(theEntry.log_description);
        } else {
            strcpy(string, "");
            *var_len = 0;
        }
        return (u_char *) string;
    }
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_logTable\n",
                    vp->magic));
    }
    return NULL;
}

#endif /* RFC2819_SUPPORTED_EVENT */

