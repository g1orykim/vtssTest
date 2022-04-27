/*

 Vitesse Switch API software.

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

#include "main.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"

#include "rmon_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_RMON

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_RMON,

    /* Parameter tags */
    CX_TAG_ENTRY,
    /* Table tags */
    CX_TAG_RMON_STATS_TABLE,
    CX_TAG_RMON_HISTORY_TABLE,
    CX_TAG_RMON_ALARM_TABLE,
    CX_TAG_RMON_EVENT_TABLE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t rmon_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_RMON] = {
        .name  = "rmon",
        .descr = "RMON",
        .type = CX_TAG_TYPE_MODULE
    },

    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    [CX_TAG_RMON_STATS_TABLE] = {
        .name  = "rmon_stats_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    [CX_TAG_RMON_HISTORY_TABLE] = {
        .name  = "rmon_history_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    [CX_TAG_RMON_ALARM_TABLE] = {
        .name  = "rmon_alarm_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    [CX_TAG_RMON_EVENT_TABLE] = {
        .name  = "rmon_event_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

/* RMON specific set state structure */
typedef struct {
    struct {
        int                 count;
        vtss_stat_ctrl_entry_t entry[RMON_STAT_MAX_ROW_SIZE];
    } rmon_stats;
    struct {
        int                 count;
        vtss_history_ctrl_entry_t entry[RMON_HISTORY_MAX_ROW_SIZE];
    } rmon_history;
    struct {
        int                 count;
        vtss_alarm_ctrl_entry_t entry[RMON_ALARM_MAX_ROW_SIZE];
    } rmon_alarm;
    struct {
        int                 count;
        vtss_event_ctrl_entry_t entry[RMON_EVENT_MAX_ROW_SIZE];
    } rmon_event;
} rmon_cx_set_state_t;

/* Keyword for RMON Alarm sample type */
static const cx_kw_t cx_kw_rmon_alarm_sample[] = {
    {"Delta"      ,  SAMPLE_TYPE_DELTA    } ,
    {"Absolute"   ,  SAMPLE_TYPE_ABSOLUTE } ,
    { NULL, 0 }
};

/* Keyword for RMON Alarm sample type */
static const cx_kw_t cx_kw_rmon_alarm_startup[] = {
    {"Rising"   ,       ALARM_RISING     } ,
    {"Falling"  ,       ALARM_FALLING   },
    {"RisingOrFalling"   , ALARM_BOTH       },
    { NULL, 0 }
};

/* Keyword for RMON Event type */
static const cx_kw_t cx_kw_rmon_event_type[] = {
    {"none"   ,       EVENT_NONE        } ,
    {"log"  ,               EVENT_LOG            },
    {"snmptrap",  EVENT_TRAP             },
    {"logandtrap",                    EVENT_LOG_AND_TRAP },
    { NULL, 0 }
};

static char *cx_stx_rmon_txt(char *buf, ulong min, ulong max)
{
    sprintf(buf, "%u-%u characters (spaces not allowed)", min, max);
    return buf;
}

static vtss_rc rmon_cx_parse_func(cx_set_state_t *s)
{
    int                       i;
    vtss_stat_ctrl_entry_t stats;
    vtss_history_ctrl_entry_t history;
    vtss_alarm_ctrl_entry_t alarm;
    vtss_event_ctrl_entry_t event;
    datasourceTable_info_t      table_info;

    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL        global;
        rmon_cx_set_state_t *snmp_state = s->mod_state;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        if (!s->apply) {
            break;
        }

        switch (s->id) {
        case CX_TAG_RMON_STATS_TABLE  :
            snmp_state->rmon_stats.count = 0;
            break;
        case CX_TAG_RMON_HISTORY_TABLE:
            snmp_state->rmon_history.count = 0;
            break;
        case CX_TAG_RMON_ALARM_TABLE  :
            snmp_state->rmon_alarm.count = 0;
            break;
        case CX_TAG_RMON_EVENT_TABLE  :
            snmp_state->rmon_event.count = 0;
            break;
        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_RMON_STATS_TABLE  ) {
                vtss_stat_ctrl_entry_t entry;
                oid name[] = IF_ENTRY_INDEX_INST;
                int tbl_first_index_begin = 0;
                tbl_first_index_begin = sizeof(name) / sizeof(oid) - 1;
                ulong ifIndex = 0;

                memset(&entry, 0, sizeof(entry));

                entry.data_source.length = tbl_first_index_begin + 1;
                {
//                    int i = 0;
                    for (i = 0; i < ( int ) entry.data_source.length; i ++) {
                        entry.data_source.objid[i] = name[i];
                    }
                }

                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (VTSS_OK == cx_parse_ulong(s, "id", &entry.id, RMON_ID_MIN, RMON_ID_MAX)) {
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "data_source", &ifIndex, RMON_ID_MIN, RMON_ID_MAX) && TRUE == get_datasource_info(ifIndex, &table_info) ) {
                        entry.data_source.objid[tbl_first_index_begin] = ifIndex;
                    }
                }

                if (entry.id == 0) {
                    CX_RC(cx_parm_found_error(s, "id"));
                }

                if (ifIndex == 0) {
                    CX_RC(cx_parm_found_error(s, "data_source"));
                }

                if (snmp_state->rmon_stats.count < RMON_STAT_MAX_ROW_SIZE) {
                    snmp_state->rmon_stats.entry[snmp_state->rmon_stats.count++] = entry;
                }
            } else if (s->group == CX_TAG_RMON_HISTORY_TABLE) {
                vtss_history_ctrl_entry_t entry;
                oid name[] = IF_ENTRY_INDEX_INST;
                int tbl_first_index_begin = 0;
                tbl_first_index_begin = sizeof(name) / sizeof(oid) - 1;
                ulong ifIndex = 0;

                memset(&entry, 0, sizeof(entry));

                entry.data_source.length = tbl_first_index_begin + 1;
                {
//                   int i = 0;
                    for (i = 0; i < ( int ) entry.data_source.length; i ++) {
                        entry.data_source.objid[i] = name[i];
                    }
                }

                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (VTSS_OK == cx_parse_ulong(s, "id", &entry.id, RMON_ID_MIN, RMON_ID_MAX)) {
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "data_source", &ifIndex, RMON_ID_MIN, RMON_ID_MAX) && TRUE == get_datasource_info(ifIndex, &table_info)) {
                        entry.data_source.objid[tbl_first_index_begin] = ifIndex;
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "interval", (ulong *) &entry.interval, RMON_HISTORY_MIN_INTERVAL, RMON_HISTORY_MAX_INTERVAL)) {
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "bucket", (ulong *) &entry.scrlr.data_requested, 1, RMON_BUCKETS_REQ_MAX)) {
                        continue;
                    }
                }

                if (entry.id == 0) {
                    CX_RC(cx_parm_found_error(s, "id"));
                }

                if (ifIndex == 0) {
                    CX_RC(cx_parm_found_error(s, "data_source"));
                }

                if (entry.interval == 0) {
                    CX_RC(cx_parm_found_error(s, "interval"));
                }

                if (entry.scrlr.data_requested == 0) {
                    CX_RC(cx_parm_found_error(s, "bucket"));
                }


                if (snmp_state->rmon_history.count < RMON_HISTORY_MAX_ROW_SIZE) {
                    snmp_state->rmon_history.entry[snmp_state->rmon_history.count++] = entry;
                }
            } else if (s->group == CX_TAG_RMON_ALARM_TABLE  ) {
                vtss_alarm_ctrl_entry_t entry;
                oid name[] = IF_ENTRY_INST;
                char oidStr[128] = {0};
                char tmpStr[128] = {0};
                int tbl_first_index_begin = 0;
                ulong temp = 0;
                tbl_first_index_begin = sizeof(name) / sizeof(oid) - 2;

                memset(&entry, 0, sizeof(entry));

                entry.sample_type = SAMPLE_TYPE_END;
                entry.startup_type = ALARM_END;
                entry.var_name.length = tbl_first_index_begin + 2;
                {
//                    int i = 0;
                    for (i = 0; i < ( int ) entry.var_name.length; i ++) {
                        entry.var_name.objid[i] = name[i];
                    }
                }

                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (VTSS_OK == cx_parse_ulong(s, "id", &entry.id, RMON_ID_MIN, RMON_ID_MAX)) {
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "interval", &entry.interval, 1, RMON_ALARM_MAX_INTERVAL)) {
                        continue;
                    }
#if 0
                    else if (VTSS_OK == cx_parse_kw(s, "variable", cx_kw_rmon_alarm_var, &variable, 1)) {
                        entry.var_name.objid[tbl_first_index_begin - 1] = variable;
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "data_source", &ifIndex, RMON_ID_MIN, RMON_ID_MAX)) {
                        entry.var_name.objid[tbl_first_index_begin] = ifIndex;
                        continue;
                    }
#else
                    else if (cx_parse_attr_name(s, "variable") == VTSS_OK) {
                        ulong   subtree[RMON_MAX_OID_LEN + 1];   /* key */
                        ulong   subtree_len;                          /* key */
                        uchar   subtree_mask[RMON_MAX_SUBTREE_LEN];

                        memcpy(tmpStr, s->val, (s->val_len > 127) ? 127 : s->val_len);
                        sprintf(oidStr, ".%s", tmpStr);
                        if (mgmt_txt2oid(oidStr, (s->val_len > 127) ? 128 : s->val_len + 1, subtree,
                                         subtree_mask, &subtree_len) != VTSS_OK) {
                            CX_RC(cx_parm_invalid(s));
                        }
                        entry.var_name.objid[tbl_first_index_begin] =  subtree[0];
                        if ( TRUE == get_datasource_info(subtree[1], &table_info)) {
                            entry.var_name.objid[tbl_first_index_begin + 1] =  subtree[1];
                        }
                    }

#endif
                    else if (VTSS_OK == cx_parse_kw(s, "sample_type", cx_kw_rmon_alarm_sample, &temp, 1)) {
                        entry.sample_type = (vtss_alarm_sample_type_t)temp;
                        continue;
#if 1
                    } else if (VTSS_OK == cx_parse_kw(s, "startup_type", cx_kw_rmon_alarm_startup, &temp, 1)) {
                        entry.startup_type = (vtss_alarm_type_t)temp;
                        continue;
#endif
                    } else if (VTSS_OK == cx_parse_ulong(s, "rising_threshold", (u32 *)&entry.rising_threshold, 1, RMON_ALARM_MAX_THRESHOLD)) {
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "rising_event_index", &entry.rising_event_index, RMON_ID_MIN, RMON_ID_MAX)) {
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "falling_threshold", (u32 *)&entry.falling_threshold, 1, RMON_ALARM_MAX_THRESHOLD)) {
                        continue;
                    } else if (VTSS_OK == cx_parse_ulong(s, "falling_event_index", &entry.falling_event_index, RMON_ID_MIN, RMON_ID_MAX)) {
                        continue;
                    }
                }

                if (entry.id == 0) {
                    CX_RC(cx_parm_found_error(s, "id"));
                }

                if (entry.interval == 0) {
                    CX_RC(cx_parm_found_error(s, "interval"));
                }

                if (entry.var_name.objid[tbl_first_index_begin + 1] == 0 ||
                    entry.var_name.objid[tbl_first_index_begin] == 0 ) {
                    CX_RC(cx_parm_found_error(s, "variable"));
                }

                if (entry.sample_type == SAMPLE_TYPE_END) {
                    CX_RC(cx_parm_found_error(s, "sample_type"));
                }

#if 1
                if (entry.startup_type == ALARM_END) {
                    CX_RC(cx_parm_found_error(s, "startup_type"));
                }
#endif
                if (entry.rising_threshold == 0) {
                    CX_RC(cx_parm_found_error(s, "rising_threshold"));
                }

                if (entry.rising_event_index == 0) {
                    CX_RC(cx_parm_found_error(s, "rising_event_index"));
                }

                if (entry.falling_threshold == 0) {
                    CX_RC(cx_parm_found_error(s, "falling_threshold"));
                }

                if (entry.falling_event_index == 0) {
                    CX_RC(cx_parm_found_error(s, "falling_event_index"));
                }

                if (snmp_state->rmon_alarm.count < RMON_ALARM_MAX_ROW_SIZE) {
                    snmp_state->rmon_alarm.entry[snmp_state->rmon_alarm.count++] = entry;
                }


            } else if (s->group == CX_TAG_RMON_EVENT_TABLE  ) {
                vtss_event_ctrl_entry_t entry;
                ulong temp = 0;
                char strBuf[128] = {0};
                memset(&entry, 0, sizeof(entry));
                entry.event_type = EVENT_END;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (VTSS_OK == cx_parse_ulong(s, "id", &entry.id, RMON_ID_MIN, RMON_ID_MAX)) {
                        continue;
                    } else if (VTSS_OK == cx_parse_kw(s, "event_type", cx_kw_rmon_event_type, &temp, 1)) {
                        entry.event_type = (vtss_event_type_t )temp;
                        continue;
                    } else if (VTSS_OK == cx_parse_txt(s, "event_description", strBuf, SNMPV3_MAX_EVENT_DESC_LEN + 1)) {
                        char *ptr = strBuf;
                        for (i = 0; i < (int)strlen(strBuf); i++, ptr++) {
                            if (*ptr < 33 || *ptr > 126 ) {
                                if (entry.event_community) {
                                    VTSS_FREE(entry.event_community);
                                }
                                CX_RC(cx_parm_invalid(s));
                            }
                        }
                        entry.event_description = VTSS_STRDUP(strBuf);
                        continue;
                    }

                    else if (VTSS_OK == cx_parse_txt(s, "event_community", strBuf, SNMPV3_MAX_EVENT_COMMUNITY_LEN + 1)) {
                        char *ptr = strBuf;
                        for (i = 0; i < (int)strlen(strBuf); i++, ptr++) {
                            if (*ptr < 33 || *ptr > 126 ) {
                                if (entry.event_description) {
                                    VTSS_FREE(entry.event_description);
                                }
                                CX_RC(cx_parm_invalid(s));
                            }
                        }
                        entry.event_community = VTSS_STRDUP(strBuf);
                        continue;
                    }
                }

                if (entry.id == 0) {
                    if (entry.event_community) {
                        VTSS_FREE(entry.event_community);
                    }
                    if (entry.event_description) {
                        VTSS_FREE(entry.event_description);
                    }
                    CX_RC(cx_parm_found_error(s, "id"));
                }
                if (entry.event_type == EVENT_END) {
                    if (entry.event_community) {
                        VTSS_FREE(entry.event_community);
                    }
                    if (entry.event_description) {
                        VTSS_FREE(entry.event_description);
                    }
                    CX_RC(cx_parm_found_error(s, "event_type"));
                }

                if (snmp_state->rmon_event.count < RMON_EVENT_MAX_ROW_SIZE) {
                    snmp_state->rmon_event.entry[snmp_state->rmon_event.count++] = entry;
                } else {
                    if (entry.event_community) {
                        VTSS_FREE(entry.event_community);
                    }
                    if (entry.event_description) {
                        VTSS_FREE(entry.event_description);
                    }
                }

            } else {
                s->ignored = 1;
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
//            CX_RC(snmp_mgmt_snmp_conf_set(&conf));
        }
        break;
        } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL: {
        rmon_cx_set_state_t *snmp_state = s->mod_state;

        if (s->init) {
            i = 0;
            memset(&stats  , 0, sizeof(stats));
            while (!rmon_mgmt_statistics_entry_get(&stats, TRUE)) {
                snmp_state->rmon_stats.entry[i++] = stats;
            }
            snmp_state->rmon_stats.count = i;
            i = 0;
            memset(&history, 0, sizeof(history));
            while (!rmon_mgmt_history_entry_get(&history, TRUE)) {
                snmp_state->rmon_history.entry[i++] = history;
            }
            snmp_state->rmon_history.count = i;

            i = 0;
            memset(&alarm  , 0, sizeof(alarm));
            while (!rmon_mgmt_alarm_entry_get(&alarm, TRUE)) {
                snmp_state->rmon_alarm.entry[i++] = alarm;
            }
            snmp_state->rmon_alarm.count = i;

            i = 0;
            memset(&event  , 0, sizeof(event));
            while (!rmon_mgmt_event_entry_get(&event, TRUE)) {
                snmp_state->rmon_event.entry[i++] = event;
            }
            snmp_state->rmon_event.count = i;

        } else if (s->apply) {

            /* Delete RMON statistics table */
            while (1) {
                memset(&stats, 0, sizeof(stats));
                if (rmon_mgmt_statistics_entry_get(&stats, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(rmon_mgmt_statistics_entry_del(&stats));
            }

            /* Delete RMON history table */
            while (1) {
                memset(&history, 0, sizeof(history));
                if (rmon_mgmt_history_entry_get(&history, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(rmon_mgmt_history_entry_del(&history));
            }

            /* Delete RMON alarm table */
            while (1) {
                memset(&alarm, 0, sizeof(alarm));
                if (rmon_mgmt_alarm_entry_get(&alarm, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(rmon_mgmt_alarm_entry_del(&alarm));
            }

            /* Delete RMON event table */
            while (1) {
                memset(&event, 0, sizeof(event));
                if (rmon_mgmt_event_entry_get(&event, TRUE) != VTSS_OK) {
                    break;
                }
                CX_RC(rmon_mgmt_event_entry_del(&event));
            }

            /* Add RMON statistics table */
            for (i = 0; i < snmp_state->rmon_stats.count; i++) {
                CX_RC(rmon_mgmt_statistics_entry_add(&snmp_state->rmon_stats.entry[i]));
            }

            /* Add RMON history table */
            for (i = 0; i < snmp_state->rmon_history.count; i++) {
                CX_RC(rmon_mgmt_history_entry_add(&snmp_state->rmon_history.entry[i]));
            }

            /* Add RMON alarm table */
            for (i = 0; i < snmp_state->rmon_alarm.count; i++) {
                CX_RC(rmon_mgmt_alarm_entry_add(&snmp_state->rmon_alarm.entry[i]));
            }

            /* Add RMON event table */
            for (i = 0; i < snmp_state->rmon_event.count; i++) {
                CX_RC(rmon_mgmt_event_entry_add(&snmp_state->rmon_event.entry[i]));
            }
        } else {
        }
        break;
    }
    case CX_PARSE_CMD_SWITCH:
        break;
    default:
        break;
    }

    return s->rc;
}

static vtss_rc rmon_cx_gen_func(cx_get_state_t *s)
{
    char buf[128];
    datasourceTable_info_t      table_info;
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - SNMP */
#ifdef VTSS_SW_OPTION_RMON
        T_D("global - RMON");
        CX_RC(cx_add_tag_line(s, CX_TAG_RMON, 0));
        CX_RC(cx_add_tag_line(s, CX_TAG_RMON_STATS_TABLE, 0));
        {
            vtss_stat_ctrl_entry_t entry;

            memset(&entry, 0, sizeof(entry));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "id", RMON_ID_MIN, RMON_ID_MAX));
            CX_RC(cx_add_stx_ulong(s, "data_source", RMON_ID_MIN, RMON_ID_MAX));
            CX_RC(cx_add_stx_end(s));

            while (!rmon_mgmt_statistics_entry_get(&entry, TRUE)) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "id", entry.id));
                CX_RC(cx_add_attr_ulong(s, "data_source", (ulong)entry.data_source.objid[entry.data_source.length - 1]));
                if (FALSE == get_datasource_info(entry.data_source.objid[entry.data_source.length - 1], &table_info)) {
                    CX_RC(!VTSS_OK);
                }
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }

        }
        CX_RC(cx_add_tag_line(s, CX_TAG_RMON_STATS_TABLE, 1));

        CX_RC(cx_add_tag_line(s, CX_TAG_RMON_HISTORY_TABLE, 0));
        {
            vtss_history_ctrl_entry_t entry;

            memset(&entry, 0, sizeof(entry));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "id", RMON_ID_MIN, RMON_ID_MAX));
            CX_RC(cx_add_stx_ulong(s, "data_source", RMON_ID_MIN, RMON_ID_MAX));
            CX_RC(cx_add_stx_ulong(s, "interval", RMON_HISTORY_MIN_INTERVAL, RMON_HISTORY_MAX_INTERVAL));
            CX_RC(cx_add_stx_ulong(s, "bucket", 1, RMON_BUCKETS_REQ_MAX));
            CX_RC(cx_add_stx_end(s));

            while (!rmon_mgmt_history_entry_get(&entry, TRUE)) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "id", entry.id));
                CX_RC(cx_add_attr_ulong(s, "data_source", (ulong)entry.data_source.objid[entry.data_source.length - 1]));
                if (FALSE == get_datasource_info(entry.data_source.objid[entry.data_source.length - 1], &table_info)) {
                    CX_RC(!VTSS_OK);
                }
                CX_RC(cx_add_attr_ulong(s, "interval", (ulong)entry.interval));
                CX_RC(cx_add_attr_ulong(s, "bucket", (ulong)entry.scrlr.data_requested));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }

        }
        CX_RC(cx_add_tag_line(s, CX_TAG_RMON_HISTORY_TABLE, 1));

        CX_RC(cx_add_tag_line(s, CX_TAG_RMON_ALARM_TABLE, 0));
        {
            vtss_alarm_ctrl_entry_t entry;
            char oidStr[128] = {0};

            memset(&entry, 0, sizeof(entry));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "id", RMON_ID_MIN, RMON_ID_MAX));
            CX_RC(cx_add_stx_ulong(s, "interval", RMON_ALARM_MIN_INTERVAL, RMON_ALARM_MAX_INTERVAL));
#if 0
            CX_RC(cx_add_stx_kw(s, "variable", cx_kw_rmon_alarm_var));
            CX_RC(cx_add_stx_ulong(s, "data_source", RMON_ID_MIN, RMON_ID_MAX));
#else
            CX_RC(cx_add_attr_txt(s, "variable", "a.b OID string of 1-128 numbers "));

#endif
            CX_RC(cx_add_stx_kw(s, "sample_type", cx_kw_rmon_alarm_sample));
            CX_RC(cx_add_stx_kw(s, "starup_type", cx_kw_rmon_alarm_startup));
            CX_RC(cx_add_stx_ulong(s, "rising_threshold", 1, RMON_BUCKETS_REQ_MAX));
            CX_RC(cx_add_stx_ulong(s, "rising_event_index", 1, RMON_ID_MAX));
            CX_RC(cx_add_stx_ulong(s, "falling_threshold", 1, RMON_BUCKETS_REQ_MAX));
            CX_RC(cx_add_stx_ulong(s, "falling_event_index", 1, RMON_ID_MAX));
            CX_RC(cx_add_stx_end(s));

            while (!rmon_mgmt_alarm_entry_get(&entry, TRUE)) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "id", entry.id));
                CX_RC(cx_add_attr_ulong(s, "interval", entry.interval));
#if 0
                CX_RC(cx_add_attr_kw(s, "variable", cx_kw_rmon_alarm_var, entry.var_name.objid[entry.var_name.length - 2]));
                CX_RC(cx_add_attr_ulong(s, "data_source", (ulong)entry.var_name.objid[entry.var_name.length - 1]));
#endif
                if (FALSE == get_datasource_info(entry.var_name.objid[entry.var_name.length - 1], &table_info)) {
                    CX_RC(!VTSS_OK);
                }
                sprintf(oidStr, "%u.%u", (ulong)entry.var_name.objid[entry.var_name.length - 2],
                        (ulong)entry.var_name.objid[entry.var_name.length - 1] );
                CX_RC(cx_add_attr_txt(s, "variable", oidStr));
                CX_RC(cx_add_attr_kw(s, "sample_type", cx_kw_rmon_alarm_sample, entry.sample_type));
                CX_RC(cx_add_attr_kw(s, "startup_type", cx_kw_rmon_alarm_startup, entry.startup_type));
                CX_RC(cx_add_attr_ulong(s, "rising_threshold", entry.rising_threshold));
                CX_RC(cx_add_attr_ulong(s, "rising_event_index", entry.rising_event_index));
                CX_RC(cx_add_attr_ulong(s, "falling_threshold", entry.falling_threshold));
                CX_RC(cx_add_attr_ulong(s, "falling_event_index", entry.falling_event_index));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }

        }
        CX_RC(cx_add_tag_line(s, CX_TAG_RMON_ALARM_TABLE, 1));

        CX_RC(cx_add_tag_line(s, CX_TAG_RMON_EVENT_TABLE, 0));
        {
            vtss_event_ctrl_entry_t entry;

            memset(&entry, 0, sizeof(entry));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_stx_ulong(s, "id", RMON_ID_MIN, RMON_ID_MAX));
            CX_RC(cx_add_attr_txt(s, "event_description", cx_stx_rmon_txt(buf, 0, SNMPV3_MAX_EVENT_DESC_LEN)));
            CX_RC(cx_add_stx_kw(s, "event_type", cx_kw_rmon_event_type));
            CX_RC(cx_add_attr_txt(s, "event_community", cx_stx_rmon_txt(buf, 0, SNMPV3_MAX_EVENT_COMMUNITY_LEN)));
            CX_RC(cx_add_stx_end(s));

            while (!rmon_mgmt_event_entry_get(&entry, TRUE)) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_ulong(s, "id", entry.id));
                CX_RC(cx_add_attr_txt(s, "event_description", entry.event_description));
                CX_RC(cx_add_attr_kw(s, "event_type", cx_kw_rmon_event_type, entry.event_type));
                CX_RC(cx_add_attr_txt(s, "event_community", entry.event_community));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            }

        }
        CX_RC(cx_add_tag_line(s, CX_TAG_RMON_EVENT_TABLE, 1));
        CX_RC(cx_add_tag_line(s, CX_TAG_RMON, 1));
#endif /* VTSS_SW_OPTION_RMON */
        break;
    case CX_GEN_CMD_SWITCH:
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_RMON,
    rmon_cx_tag_table,
    sizeof(rmon_cx_set_state_t),
    0,
    NULL,                   /* init function       */
    rmon_cx_gen_func,       /* Generation fucntion */
    rmon_cx_parse_func      /* parse fucntion      */
);

