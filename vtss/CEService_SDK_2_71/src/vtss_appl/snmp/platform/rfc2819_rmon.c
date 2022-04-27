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
#include <ucd-snmp/mibgroup/util_funcs.h>       /* utility function declarations */

#include "vtss_snmp_api.h"
#include "mibContextTable.h"  //mibContextTable_register
#include "rfc2819_rmon.h"
#if defined(SNMP_HAS_UCD_SNMP)
#include "ucd_snmp_rfc2819_rmon.h"
#elif defined(SNMP_HAS_NET_SNMP)
#include "net_snmp_rfc2819_rmon.h"
#endif /* SNMP_HAS_UCD_SNMP */

#include "rmon_api.h"
#include "ifIndex_api.h"
#include "rmon_agutil_api.h"
#ifndef EXTEND_RMON_TO_WEB_CLI
#include "rmon_row_api.h"
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

typedef vtss_history_ctrl_entry_t HISTORY_CRTL_ENTRY_T;
typedef vtss_history_data_entry_t HISTORY_DATA_ENTRY_T;
typedef vtss_alarm_ctrl_entry_t ALARM_CRTL_ENTRY_T;
typedef vtss_event_ctrl_entry_t EVENT_CRTL_ENTRY_T;
typedef vtss_event_data_entry_t EVENT_DATA_ENTRY_T;

#if RFC2819_SUPPORTED_STATISTICS
/*
 * Initializes the statistics module
 */
void init_rmon_statisticsMIB(void)
{
    oid statistics_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 1 };

    // Register mibContextTable
    mibContextTable_register(statistics_variables_oid,
                             sizeof(statistics_variables_oid) / sizeof(oid),
                             "RMON-MIB : statistics");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_rmon_statisticsMIB();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_rmon_statisticsMIB();
#endif /* SNMP_HAS_UCD_SNMP */
}
#endif /* RFC2819_SUPPORTED_STATISTICS */

#if RFC2819_SUPPORTED_HISTORY
#define historyControlEntryFirstIndexBegin  11

#define HISTORYCONTROLINDEX                 1
#define HISTORYCONTROLDATASOURCE            2
#define HISTORYCONTROLBUCKETSREQUESTED      3
#define HISTORYCONTROLBUCKETSGRANTED        4
#define HISTORYCONTROLINTERVAL              5
#define HISTORYCONTROLOWNER                 6
#define HISTORYCONTROLSTATUS                7
#define ETHERHISTORYINDEX                   8
#define ETHERHISTORYSAMPLEINDEX             9
#define ETHERHISTORYINTERVALSTART           10
#define ETHERHISTORYDROPEVENTS              11
#define ETHERHISTORYOCTETS                  12
#define ETHERHISTORYPKTS                    13
#define ETHERHISTORYBROADCASTPKTS           14
#define ETHERHISTORYMULTICASTPKTS           15
#define ETHERHISTORYCRCALIGNERRORS          16
#define ETHERHISTORYUNDERSIZEPKTS           17
#define ETHERHISTORYOVERSIZEPKTS            18
#define ETHERHISTORYFRAGMENTS               19
#define ETHERHISTORYJABBERS                 20
#define ETHERHISTORYCOLLISIONS              21
#define ETHERHISTORYUTILIZATION             22


/*
 * Initializes the statistics module
 */
void init_rmon_historyMIB(void)
{
    oid history_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 2 };

    // Register mibContextTable
    mibContextTable_register(history_variables_oid,
                             sizeof(history_variables_oid) / sizeof(oid),
                             "RMON-MIB : history");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_rmon_historyMIB();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_rmon_historyMIB();
#endif /* SNMP_HAS_UCD_SNMP */
}

int write_historyControl(int action, u_char *var_val, u_char var_val_type,
                         size_t var_val_len, u_char *statP,
                         oid *name, size_t name_len)
{
    long                 long_temp;
    int                  leaf_id, snmp_status;
    static int           prev_action = COMMIT;
    RMON_ENTRY_T         *hdr;
    vtss_history_ctrl_entry_t *cloned_body;
    vtss_history_ctrl_entry_t *body;
    int                  max_size;
    long                 intval;

    switch (action) {
    case RESERVE1:
        leaf_id = (int) name[historyControlEntryFirstIndexBegin - 1];

        switch (leaf_id) {
        case HISTORYCONTROLDATASOURCE:
            max_size = 12 * sizeof(oid);
            if (var_val_type != ASN_OBJECT_ID) {
                snmp_log(LOG_ERR,
                         "write to historyControlDataSource: not ASN_OBJECT_ID\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to historyControlDataSource: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case HISTORYCONTROLBUCKETSREQUESTED:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to historyControlBucketsRequested: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to historyControlBucketsRequested: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval < MIN_historyControlBucketsRequested || intval > MAX_historyControlBucketsRequested) {
                snmp_log(LOG_ERR, "write to historyControlBucketsRequested: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case HISTORYCONTROLINTERVAL:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to historyControlInterval: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to historyControlInterval: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval < MIN_historyControlInterval || intval > MAX_historyControlInterval) {
                snmp_log(LOG_ERR, "write to historyControlInterval: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case HISTORYCONTROLOWNER:
            max_size = MAX_OWNERSTRING;
            if (var_val_type != ASN_OCTET_STR) {
                snmp_log(LOG_ERR,
                         "write to historyControlOwner: not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to historyControlOwner: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case HISTORYCONTROLSTATUS:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to historyControlStatus: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to historyControlStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval != 1 && intval != 2 && intval != 3 && intval != 4) {
                snmp_log(LOG_ERR, "write to historyControlStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        }
        goto force_default;
    case FREE:
    case UNDO:
    case ACTION:
    case COMMIT:
    default:
force_default:
        snmp_status =
            ROWAPI_do_another_action(name, historyControlEntryFirstIndexBegin,
                                     action, &prev_action, RMON_HISTORY_TABLE_INDEX,
                                     sizeof(HISTORY_CRTL_ENTRY_T));
        if (SNMP_ERR_NOERROR != snmp_status) {
            ag_trace("failed action %d with %d", action, snmp_status);
            return SNMP_ERR_BADVALUE;
        }

        /* Added by SGZ for fixing the system crash issue, this issue result from long_temp is not be assigned, so
           ROWAPI_find will return NULL pointer, happy honeymoon, YA ~~~, 05/27/2011  */
        long_temp = name[historyControlEntryFirstIndexBegin];
        /*  Added by SGZ end, 05/27/2011    */

        hdr = ROWAPI_find(RMON_HISTORY_TABLE_INDEX, long_temp);        /* it MUST be OK */
        /*  Modified by SGZ for breaking the case if the row status is INVALID(4), 05/27/2011     */
        if ( !hdr || hdr->status != RMON1_ENTRY_VALID) {
            /*  Modified by SGZ end, 05/27/2011     */
            break;    /* only save SNMP RMON history entry under valid status */
        }
        leaf_id = (int) name[historyControlEntryFirstIndexBegin - 1];
        if (leaf_id == HISTORYCONTROLBUCKETSREQUESTED) {
            /* save SNMP RMON history entry */
            rmon_history_entry_t entry;

            entry.ctrl_index = name[historyControlEntryFirstIndexBegin];
            if (snmp_mgmt_rmon_history_entry_get(&entry, FALSE) != VTSS_OK) {
                return SNMP_ERR_RESOURCEUNAVAILABLE;
            }

            intval = *((u_long *) var_val);

            if (leaf_id == HISTORYCONTROLBUCKETSREQUESTED) {
                if (entry.requested != intval) {
                    entry.requested = intval;
                } else {
                    break;    /* same requested, do nothing */
                }
            }
            entry.ctrl_index = name[historyControlEntryFirstIndexBegin];
            if (snmp_mgmt_rmon_history_entry_set(&entry) != VTSS_OK) {
                return SNMP_ERR_RESOURCEUNAVAILABLE;
            }
        }
        break;

    case RESERVE2:
        /*
         * get values from PDU, check them and save them in the cloned entry
         */
        long_temp = name[historyControlEntryFirstIndexBegin];
        leaf_id = (int) name[historyControlEntryFirstIndexBegin - 1];
        hdr = ROWAPI_find(RMON_HISTORY_TABLE_INDEX, long_temp);        /* it MUST be OK */
        if (hdr->status == RMON1_ENTRY_VALID && leaf_id == HISTORYCONTROLINTERVAL) {
            /*'historyControlInterval' not be modified if the associated
            'historyControlStatus' object is equal to valid(1) */
            return SNMP_ERR_BADVALUE;
        }
        cloned_body = (HISTORY_CRTL_ENTRY_T *) hdr->tmp;
        body = (HISTORY_CRTL_ENTRY_T *) hdr->body;
#ifdef EXTEND_RMON_TO_WEB_CLI
        /* Added by SGZ, 05/27     */
        cloned_body->id = long_temp;
        /* Added by SGZ end, 05/27     */
#endif

        switch (leaf_id) {
        case HISTORYCONTROLDATASOURCE:
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

        case HISTORYCONTROLBUCKETSREQUESTED:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               MIN_historyControlBucketsRequested,
                                               MAX_historyControlBucketsRequested,
                                               &cloned_body->scrlr.
                                               data_requested);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
#if 0
            if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                cloned_body->scrlr.data_requested !=
                body->scrlr.data_requested) {
                return SNMP_ERR_BADVALUE;
            }
#endif
            break;

        case HISTORYCONTROLINTERVAL:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               MIN_historyControlInterval,
                                               MAX_historyControlInterval,
                                               &cloned_body->interval);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
#if 0
            if (RMON1_ENTRY_UNDER_CREATION != hdr->status &&
                cloned_body->interval != body->interval) {
                return SNMP_ERR_BADVALUE;
            }
#endif
            break;

        case HISTORYCONTROLOWNER:
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
        case HISTORYCONTROLSTATUS:
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
            ag_trace("%s:unknown leaf_id=%d\n", "History",
                     (int) leaf_id);
            return SNMP_ERR_NOSUCHNAME;
        }                       /* of switch by 'leaf_id' */
        break;
    } /* of switch by 'action' */

    prev_action = action;
    return SNMP_ERR_NOERROR;
}
#endif /* RFC2819_SUPPORTED_HISTORY */

#if RFC2819_SUPPORTED_AlARM
/*
 * Initializes the statistics module
 */
void init_rmon_alarmMIB(void)
{
    oid alarm_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 3 };

    // Register mibContextTable
    mibContextTable_register(alarm_variables_oid,
                             sizeof(alarm_variables_oid) / sizeof(oid),
                             "RMON-MIB : alarm");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_rmon_alarmMIB();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_rmon_alarmMIB();
#endif /* SNMP_HAS_UCD_SNMP */
}
#endif /* RFC2819_SUPPORTED_AlARM */

#if RFC2819_SUPPORTED_EVENT
#define eventEntryFirstIndexBegin   11

#define EVENTINDEX                  1
#define EVENTDESCRIPTION            2
#define EVENTTYPE                   3
#define EVENTCOMMUNITY              4
#define EVENTLASTTIMESENT           5
#define EVENTOWNER                  6
#define EVENTSTATUS                 7
#define LOGEVENTINDEX               8
#define LOGINDEX                    9
#define LOGTIME                     10
#define LOGDESCRIPTION              11

/*
 * Initializes the statistics module
 */
void init_rmon_eventMIB(void)
{
    oid event_variables_oid[] = { 1, 3, 6, 1, 2, 1, 16, 9 };

    // Register mibContextTable
    mibContextTable_register(event_variables_oid,
                             sizeof(event_variables_oid) / sizeof(oid),
                             "RMON-MIB : event");

#if defined(SNMP_HAS_UCD_SNMP)
    ucd_snmp_init_rmon_eventMIB();
#elif defined(SNMP_HAS_NET_SNMP)
    net_snmp_init_rmon_eventMIB();
#endif /* SNMP_HAS_UCD_SNMP */
}

int write_eventControl(int action, u_char *var_val, u_char var_val_type,
                       size_t var_val_len, u_char *statP,
                       oid *name, size_t name_len)
{
    long               long_temp;
    char               *char_temp;
    int                leaf_id, snmp_status;
    static int         prev_action = COMMIT;
    RMON_ENTRY_T       *hdr;
    EVENT_CRTL_ENTRY_T *cloned_body;
    EVENT_CRTL_ENTRY_T *body;
    int                max_size;
    long               intval;

    switch (action) {
    case RESERVE1:
        leaf_id = (int) name[eventEntryFirstIndexBegin - 1];

        switch (leaf_id) {
        case EVENTDESCRIPTION:
            if (var_val_type != ASN_OCTET_STR) {
                snmp_log(LOG_ERR,
                         "write to eventDescription: not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > MAX_event_description) {
                snmp_log(LOG_ERR,
                         "write to eventDescription: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case EVENTTYPE:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to eventType: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to eventType: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval != 1 && intval != 2 && intval != 3 && intval != 4) {
                snmp_log(LOG_ERR, "write to eventType: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case EVENTCOMMUNITY:
            if (var_val_type != ASN_OCTET_STR) {
                snmp_log(LOG_ERR,
                         "write to eventCommunity: not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > MAX_event_community) {
                snmp_log(LOG_ERR,
                         "write to eventCommunity: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case EVENTOWNER:
            max_size = MAX_OWNERSTRING;
            if (var_val_type != ASN_OCTET_STR) {
                snmp_log(LOG_ERR,
                         "write to eventOwner: not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to eventOwner: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
        case EVENTSTATUS:
            max_size = sizeof(long);
            if (var_val_type != ASN_INTEGER) {
                snmp_log(LOG_ERR,
                         "write to eventStatus: not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > max_size) {
                snmp_log(LOG_ERR,
                         "write to eventStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            intval = *((long *) var_val);
            if (intval != 1 && intval != 2 && intval != 3 && intval != 4) {
                snmp_log(LOG_ERR, "write to eventStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        }
        goto force_default;
    case FREE:
    case UNDO:
    case ACTION:
    case COMMIT:
    default:
force_default:
        snmp_status =
            ROWAPI_do_another_action(name, eventEntryFirstIndexBegin,
                                     action, &prev_action, RMON_EVENT_TABLE_INDEX,
                                     sizeof(EVENT_CRTL_ENTRY_T));
        if (SNMP_ERR_NOERROR != snmp_status) {
            ag_trace("failed action %d with %d", action, snmp_status);
            return SNMP_ERR_BADVALUE;
        }

        long_temp = name[eventEntryFirstIndexBegin];
        hdr = ROWAPI_find(RMON_EVENT_TABLE_INDEX, long_temp);        /* it MUST be OK */
        /*  Modified by SGZ for breaking the case if the row status is INVALID(4), 05/27/2011     */
        if ( !hdr || hdr->status != RMON1_ENTRY_VALID) {
            /*  Modified by SGZ end, 05/27/2011     */
            break;    /* only save SNMP RMON history entry under valid status */
        }
        leaf_id = (int) name[eventEntryFirstIndexBegin - 1];
        if (leaf_id == EVENTDESCRIPTION ||
            leaf_id == EVENTTYPE ||
            leaf_id == EVENTCOMMUNITY) {
            /* save SNMP RMON event entry */
            rmon_event_entry_t entry;

            entry.ctrl_index = name[eventEntryFirstIndexBegin];
            if (snmp_mgmt_rmon_event_entry_get(&entry, FALSE) != VTSS_OK) {
                return SNMP_ERR_RESOURCEUNAVAILABLE;
            }
            if (leaf_id == EVENTDESCRIPTION) {
                if (memcmp(entry.description, var_val, var_val_len)) {
                    memcpy(entry.description, var_val, var_val_len);
                    entry.description[var_val_len] = '\0';
                } else {
                    break;    /* same description, do nothing */
                }
            }
            if (leaf_id == EVENTTYPE) {
                intval = *((u_long *) var_val);
                if (entry.type != intval) {
                    entry.type = intval;
                } else {
                    break;    /* same type, do nothing */
                }
            }
            if (leaf_id == EVENTCOMMUNITY) {
                if (memcmp(entry.community, var_val, var_val_len)) {
                    memcpy(entry.community, var_val, var_val_len);
                    entry.community[var_val_len] = '\0';
                } else {
                    break;    /* same community, do nothing */
                }
            }
            entry.ctrl_index = name[eventEntryFirstIndexBegin];
            if (snmp_mgmt_rmon_event_entry_set(&entry) != VTSS_OK) {
                return SNMP_ERR_RESOURCEUNAVAILABLE;
            }
        }
        break;

    case RESERVE2:
        /*
         * get values from PDU, check them and save them in the cloned entry
         */
        long_temp = name[eventEntryFirstIndexBegin];
        leaf_id = (int) name[eventEntryFirstIndexBegin - 1];
        hdr = ROWAPI_find(RMON_EVENT_TABLE_INDEX, long_temp);        /* it MUST be OK */
        cloned_body = (EVENT_CRTL_ENTRY_T *) hdr->tmp;
        body = (EVENT_CRTL_ENTRY_T *) hdr->body;
#ifdef EXTEND_RMON_TO_WEB_CLI
        /* Added by SGZ, 05/27     */
        cloned_body->id = long_temp;
        /* Added by SGZ end, 05/27     */
#endif

        switch (leaf_id) {
        case EVENTDESCRIPTION:
            char_temp = AGMALLOC(1 + MAX_event_description);
            if (!char_temp) {
                return SNMP_ERR_TOOBIG;
            }
            snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                  var_val_len,
                                                  MAX_event_description,
                                                  1, NULL, char_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                AGFREE(char_temp);
                return snmp_status;
            }

            if (cloned_body->event_description) {
                AGFREE(cloned_body->event_description);
            }

            cloned_body->event_description = AGSTRDUP(char_temp);
            /*
             * ag_trace ("rx: event_description=<%s>", cloned_body->event_description);
             */
            AGFREE(char_temp);
            break;

        case EVENTTYPE:
            snmp_status = AGUTIL_get_int_value(var_val, var_val_type,
                                               var_val_len,
                                               EVENT_NONE,
                                               EVENT_LOG_AND_TRAP,
                                               &long_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                return snmp_status;
            }
            cloned_body->event_type = long_temp;
            break;

        case EVENTCOMMUNITY:
            char_temp = AGMALLOC(1 + MAX_event_community);
            if (!char_temp) {
                return SNMP_ERR_TOOBIG;
            }
            snmp_status = AGUTIL_get_string_value(var_val, var_val_type,
                                                  var_val_len,
                                                  MAX_event_community,
                                                  1, NULL, char_temp);
            if (SNMP_ERR_NOERROR != snmp_status) {
                AGFREE(char_temp);
                return snmp_status;
            }

            if (cloned_body->event_community) {
                AGFREE(cloned_body->event_community);
            }

            cloned_body->event_community = AGSTRDUP(char_temp);
            AGFREE(char_temp);
            break;

        case EVENTOWNER:
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
        case EVENTSTATUS:
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
            ag_trace("%s:unknown leaf_id=%d\n", "Event",
                     (int) leaf_id);
            return SNMP_ERR_NOSUCHNAME;
        }                       /* of switch by 'leaf_id' */
        break;
    } /* of switch by 'action' */

    prev_action = action;
    return SNMP_ERR_NOERROR;
}
#endif /* RFC2819_SUPPORTED_EVENT */

