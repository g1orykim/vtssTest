/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

 $Id$
 $Revision$

*/

#include "main.h"
#include "cli.h"
#include "cli_grp_help.h"
#include "rmon_cli.h"

#include "rmon_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_RMON

typedef struct {
    oid oid[RMON_MAX_OID_LEN + 1];
    u32 oid_len;
    u8 oid_mask[RMON_MAX_SUBTREE_LEN];
    u32 oid_mask_len;

    u32 entry_idx;

    /* Keywords */
    BOOL read;
    BOOL write;
    cli_spec_t             interval_spec;
    u32                    interval;
    cli_spec_t             buckets_spec;
    u32 buckets;
    u32 rising_threshold;
    u32 rising_event_index;
    u32 falling_threshold;
    u32 falling_event_index;
    vtss_alarm_sample_type_t sample_type;
    vtss_alarm_type_t startup_type;
    vtss_event_type_t event_type;
    i8 event_description[ SNMPV3_MAX_EVENT_DESC_LEN + 1];
    i8 event_community[SNMPV3_MAX_EVENT_COMMUNITY_LEN + 1];
} rmon_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void rmon_cli_init(void)
{
    /* register the size required for snmp req. structure */
    cli_req_size_register(sizeof(rmon_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* SNMP configuration */

static char *oid2str(oid *name, int name_len)
{
    int i = 0, j = 0;

    /*lint -esym(459,buf)*/
    /* This table is read only. */
    static char buf[65];
    for (i = 0; i < name_len; i++) {
        sprintf(buf + j, ".%ld", name[i]);
        j = strlen(buf);
    }
    return buf;
}

/* Parse raw text OID subtree string */
static int cli_parse_oid_subtree(char *str, oid *oidSubTree, u32 *oid_len, u8 *oid_mask, u32 *oid_mask_len)
{
    char *value_char;
    int  num = 0;
    u32  i, mask = 0x80, maskpos = 0;

    value_char = str;
    *oid_len = *oid_mask_len = 0;

    //check if OID format .x.x.x
    if (value_char[0] != '.') {
        return 1;
    }
    for (i = 0; i < strlen(str); i++) {
        if (((value_char[i] != '.') && (value_char[i] != '*')) &&
            (value_char[i] < '0' || value_char[i] > '9')) {
            return 1;
        }
        if (value_char[i] == '*') {
            if (i == 0 || value_char[i - 1] != '.') {
                return 1;
            }
        }
        if (value_char[i] == '.') {
            if (i == strlen(str) - 1) {
                return 1;
            } else if (value_char[i + 1] == '.') {
                return 1;
            }
            num++;
            if (num > 128) {
                return 1;
            }
        }
    }
    *oid_mask_len = *oid_len = num;

    /* convert OID string (RFC1447)
       Each bit of this bit mask corresponds to the (8*i - 7)-th
       sub-identifier, and the least significant bit of the i-th
       octet of this octet string corresponding to the (8*i)-th
       sub-identifier, where i is in the range 1 through 16. */
    for (i = 0; i < *oid_len; i++) {
        if (!memcmp(value_char, ".*", 2)) {
            oidSubTree[i] = 0;
            oid_mask[maskpos] &= (~mask);
            value_char = value_char + 2;
        } else {
            oid_mask[maskpos] |= mask;
            sscanf(value_char++, ".%ld", &oidSubTree[i]);
        }

        if (i == *oid_len - 1) {
            break; //last OID node
        }
        while (*value_char != '.') {
            value_char++;
        }

        if (mask == 1) {
            mask = 0x80;
            maskpos++;
        } else {
            mask >>= 1;
        }
    }

    return 0;
}

static int32_t RMON_cli_oid_subtree_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    rmon_cli_req_t *rmon_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_oid_subtree(cmd_org, rmon_req->oid, &rmon_req->oid_len, rmon_req->oid_mask, &rmon_req->oid_mask_len);

    return error;
}

static void RMON_cli_cmd_rmon_stats_add(cli_req_t *req)
{
    vtss_rc rc = 0;
    vtss_stat_ctrl_entry_t conf;
    rmon_cli_req_t *rmon_req = req->module_req;
    oid ifOid[] = IF_ENTRY_INDEX_OID;
    datasourceTable_info_t      table_info;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        conf.id = rmon_req->entry_idx;

        if (rmon_mgmt_statistics_entry_get (&conf, FALSE) == VTSS_OK) {
            ;
        }

        {
            int i = 0;
            for (i = 0; i < ( int ) rmon_req->oid_len; i ++) {
                conf.data_source.objid[i] = rmon_req->oid[i];
            }
        }

        conf.data_source.length = rmon_req->oid_len;

        if (memcmp(rmon_req->oid, ifOid, sizeof(ifOid)) || rmon_req->oid_len != sizeof(ifOid) / sizeof(oid) + 1 ||
            rmon_req->oid[rmon_req->oid_len - 1] == 0) {
            CPRINTF("Invalid parameter <data_source>\n");
            return;
        }

        if (!get_datasource_info(rmon_req->oid[rmon_req->oid_len - 1], &table_info) ||
            (table_info.type != DATASOURCE_IFINDEX_TYPE_PORT &&
             table_info.type != DATASOURCE_IFINDEX_TYPE_LLAG &&
             table_info.type != DATASOURCE_IFINDEX_TYPE_GLAG)) {
            CPRINTF("<data_source> dosen't exit\n");
            return;
        }

        if ((rc = rmon_mgmt_statistics_entry_add(&conf)) != VTSS_OK) {
            CPRINTF("rmon_mgmt_statistics_entry_add(%d, %s): failed(%s)\n",
                    conf.id,
                    oid2str(rmon_req->oid, rmon_req->oid_len), error_txt(rc));
        }
    }
}

static void RMON_cli_cmd_rmon_stats_del(cli_req_t *req)
{
    vtss_stat_ctrl_entry_t conf;
    vtss_rc rc = 0;
    rmon_cli_req_t *rmon_req = req->module_req;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        conf.id = rmon_req->entry_idx;

        if ((rc = rmon_mgmt_statistics_entry_del(&conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    }

}

static void RMON_cli_cmd_rmon_stats_lookup(cli_req_t *req)
{

    vtss_stat_ctrl_entry_t conf;
    rmon_cli_req_t *rmon_req = req->module_req;
    int cnt = 0;

    memset(&conf, 0, sizeof(vtss_stat_ctrl_entry_t));
    while (rmon_mgmt_statistics_entry_get(&conf, TRUE) == VTSS_OK) {
        if (req->set) {
            if (conf.id != rmon_req->entry_idx ) {
                continue;
            } else {
                if (FALSE == rmon_etherStatsTable_entry_update(&conf.data_source, &conf.eth)) {
//                    CPRINTF("GET fail\n");
//                    return;
                }
                CPRINTF("Entry Index                   : %d\n", conf.id);
                CPRINTF("Data Source                   : %s\n", oid2str(conf.data_source.objid, conf.data_source.length));
                CPRINTF("etherStatsDropEvents          : %u\n",          conf.eth.drop_events    );
                CPRINTF("etherStatsOctets              : %u\n",          conf.eth.octets        );
                CPRINTF("etherStatsPkts                : %u\n",          conf.eth.packets       );
                CPRINTF("etherStatsBroadcastPkts       : %u\n",          conf.eth.bcast_pkts    );
                CPRINTF("etherStatsMulticastPkts       : %u\n",          conf.eth.mcast_pkts    );
                CPRINTF("etherStatsCRCAlignErrors      : %u\n",          conf.eth.crc_align     );
                CPRINTF("etherStatsUndersizePkts       : %u\n",          conf.eth.undersize     );
                CPRINTF("etherStatsOversizePkts        : %u\n",          conf.eth.oversize      );
                CPRINTF("etherStatsFragments           : %u\n",          conf.eth.fragments     );
                CPRINTF("etherStatsJabbers             : %u\n",          conf.eth.jabbers       );
                CPRINTF("etherStatsCollisions          : %u\n",          conf.eth.collisions    );
                CPRINTF("etherStatsPkts64Octets        : %u\n",          conf.eth.pkts_64       );
                CPRINTF("etherStatsPkts65to127Octets   : %u\n",          conf.eth.pkts_65_127   );
                CPRINTF("etherStatsPkts128to255Octets  : %u\n",          conf.eth.pkts_128_255  );
                CPRINTF("etherStatsPkts256to511Octets  : %u\n",          conf.eth.pkts_256_511  );
                CPRINTF("etherStatsPkts512to1023Octets : %u\n",          conf.eth.pkts_512_1023 );
                CPRINTF("etherStatsPkts1024to1518Octets: %u\n",          conf.eth.pkts_1024_1518);

                return;
            }
        }

        if (++cnt == 1) {
            CPRINTF("Id     Data Source                      etherStatsOctets   etherStatsPkts  etherStatsCRCAlignErrors\n");
            CPRINTF("----- -------------------------------- ------------------ --------------- ---------------------\n");
        }

        if (FALSE == rmon_etherStatsTable_entry_update(&conf.data_source, &conf.eth)) {
//            CPRINTF("GET fail\n");
//            memset(&conf.eth, 0, sizeof(conf.eth));
//            return;
        }
        CPRINTF("%-5u %-32s %-18u %-18u %-18u\n", conf.id, oid2str(conf.data_source.objid,
                                                                   conf.data_source.length), conf.eth.octets, conf.eth.packets, conf.eth.crc_align);
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    }


}


static void RMON_cli_cmd_rmon_history_add(cli_req_t *req)
{
    vtss_rc rc = 0;
    vtss_history_ctrl_entry_t conf;
    rmon_cli_req_t *rmon_req = req->module_req;
    u8 found = 0x0;
    oid ifOid[] = IF_ENTRY_INDEX_OID;
    datasourceTable_info_t      table_info;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        conf.id = rmon_req->entry_idx;
        if (rmon_mgmt_history_entry_get (&conf, FALSE) == VTSS_OK) {
            found = 0x1;

        }

        {
            int i = 0;
            for (i = 0; i < ( int ) rmon_req->oid_len; i ++) {
                conf.data_source.objid[i] = rmon_req->oid[i];
            }
        }

        conf.data_source.length = rmon_req->oid_len;

        if (memcmp(rmon_req->oid, ifOid, sizeof(ifOid)) || rmon_req->oid_len != sizeof(ifOid) / sizeof(oid) + 1 ||
            rmon_req->oid[rmon_req->oid_len - 1] == 0) {
            CPRINTF("Invalid parameter <data_source>\n");
            return;
        }

        if (!get_datasource_info(rmon_req->oid[rmon_req->oid_len - 1], &table_info)) {
            CPRINTF("<data_source> dosen't exit\n");
            return;
        }
        if (rmon_req->interval_spec == CLI_SPEC_VAL) {
            conf.interval = rmon_req->interval;
        } else if (!found) {
            conf.interval = 1800;
        }
        if (rmon_req->buckets_spec == CLI_SPEC_VAL) {
            conf.scrlr.data_requested = rmon_req->buckets;
        } else if (!found) {
            conf.scrlr.data_requested = 50;
        }

        if ((rc = rmon_mgmt_history_entry_add(&conf)) != VTSS_OK) {
            CPRINTF("rmon_mgmt_statistics_entry_del(%d): failed(%s)\n",
                    conf.id, error_txt(rc));
        }
    }

    return ;
}

static void RMON_cli_cmd_rmon_history_del(cli_req_t *req)
{
    vtss_history_ctrl_entry_t conf;
    vtss_rc rc = 0;
    rmon_cli_req_t *rmon_req = req->module_req;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        conf.id = rmon_req->entry_idx;
#if 0
        if (rmon_mgmt_history_entry_get (&conf, FALSE) == VTSS_OK) {
            CPRINTF("The entry '%ld, %s' is deleted now\n",
                    conf.id, oid2str(conf.data_source.objid, conf.data_source.length));
        }
#endif

        if ((rc = rmon_mgmt_history_entry_del(&conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    }

}

static void RMON_cli_cmd_rmon_history_lookup(cli_req_t *req)
{
    vtss_history_ctrl_entry_t conf;
    vtss_history_data_entry_t data;
    rmon_cli_req_t *rmon_req = req->module_req;
    int cnt = 0;

    memset(&conf, 0, sizeof(vtss_history_ctrl_entry_t));
    memset(&data, 0, sizeof(vtss_history_data_entry_t));
    while (rmon_mgmt_history_entry_get(&conf, TRUE) == VTSS_OK) {
        if (req->set) {
            if (conf.id != rmon_req->entry_idx ) {
                continue;
            } else {
//                rmon_mgmt_history_entry_get (&conf, FALSE);
                CPRINTF("Entry Index                   : %d\n", conf.id);
                CPRINTF("Data Source                   : %s\n",  oid2str(conf.data_source.objid, conf.data_source.length));
                CPRINTF("Data Bucket Request           : %lu\n", conf.scrlr.data_requested);
                CPRINTF("Data Bucket Granted           : %lu\n", conf.scrlr.data_granted);
                CPRINTF("Data Interval                 : %lu\n", conf.interval);

#if 1
                data.data_index = 0;
                while (rmon_mgmt_history_data_get(conf.id, &data, TRUE) == VTSS_OK )

#else
                data.ctrl_index = conf.id;
                data.data_index = 0;
                while (rmon_mgmt_history_data_get(&data, TRUE) == VTSS_OK && data.ctrl_index == conf.id)
#endif
                {
//                    CPRINTF("etherHistoryIndex             : %ld\n", data.ctrl_index);
                    CPRINTF("etherHistorySampleIndex       : %lu\n", data.data_index);
                    CPRINTF("    etherHistoryIntervalStart : %s(%lu)\n", misc_time2interval((data.start_interval) / 100), data.start_interval / 100);
                    CPRINTF("    etherHistoryDropEvents    : %u\n", data.EthData.drop_events   );
                    CPRINTF("    etherHistoryOctets        : %u\n", data.EthData.octets        );
                    CPRINTF("    etherHistoryPkts          : %u\n", data.EthData.packets       );
                    CPRINTF("    etherHistoryBroadcastPkts : %u\n", data.EthData.bcast_pkts    );
                    CPRINTF("    etherHistoryMulticastPkts : %u\n", data.EthData.mcast_pkts    );
                    CPRINTF("    etherHistoryCRCAlignErrors: %u\n", data.EthData.crc_align     );
                    CPRINTF("    etherHistoryUndersizePkts : %u\n", data.EthData.undersize     );
                    CPRINTF("    etherHistoryOversizePkts  : %u\n", data.EthData.oversize      );
                    CPRINTF("    etherHistoryFragments     : %u\n", data.EthData.fragments     );
                    CPRINTF("    etherHistoryJabbers       : %u\n", data.EthData.jabbers       );
                    CPRINTF("    etherHistoryCollisions    : %u\n", data.EthData.collisions    );
                    CPRINTF("    etherHistoryUtilization   : %lu\n", data.utilization       );

                }
                return;
            }
        }

        if (++cnt == 1) {
            CPRINTF("Id     Data Source                      controlBucketsRequested   controlBucketsGranted    Interval\n");
            CPRINTF("----- -------------------------------- ------------------------  ------------------------ ---------\n");

        }

        if (rmon_mgmt_history_entry_get (&conf, FALSE)) {
            CPRINTF("GET fail\n");
            return ;
        }
        CPRINTF("%-5u %-32s %-26lu %-26lu %-9lu\n", conf.id, oid2str(conf.data_source.objid,
                                                                     conf.data_source.length), conf.scrlr.data_requested, conf.scrlr.data_granted, conf.interval);
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    }

    return ;
}

static void RMON_cli_cmd_rmon_alarm_add(cli_req_t *req)
{
    vtss_rc rc = 0;
    vtss_alarm_ctrl_entry_t conf;
    rmon_cli_req_t *rmon_req = req->module_req;
    oid ifOid[] = IF_ENTRY_OID;
    iftable_info_t      table_info;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        conf.id = rmon_req->entry_idx;

        if (rmon_mgmt_alarm_entry_get (&conf, FALSE) == VTSS_OK) {
        }

        {
            int i = 0;
            for (i = 0; i < (int) rmon_req->oid_len; i ++) {
                conf.var_name.objid[i] = rmon_req->oid[i];
            }
        }

        conf.var_name.length = rmon_req->oid_len;

        if (memcmp(rmon_req->oid, ifOid, sizeof(ifOid)) || rmon_req->oid_len != sizeof(ifOid) / sizeof(oid) + 2  ||
            rmon_req->oid[rmon_req->oid_len - 2] == 0  || rmon_req->oid[rmon_req->oid_len - 1] == 0) {
            CPRINTF("Invalid parameter <data_source>\n");
            return;
        }

        table_info.ifIndex = (ifIndex_id_t) rmon_req->oid[rmon_req->oid_len - 1];
        if (!ifIndex_get_valid(&table_info)) {
            CPRINTF("<data_source> dosen't exit\n");
            return;
        }

        if (rmon_req->sample_type) {
            conf.sample_type = rmon_req->sample_type;
        } else {
            conf.sample_type = SAMPLE_TYPE_DELTA;
        }
        if (rmon_req->startup_type) {
            conf.startup_type = rmon_req->startup_type;
        } else {
            conf.startup_type = ALARM_BOTH;
        }

        conf.rising_threshold = rmon_req->rising_threshold;
        conf.falling_threshold = rmon_req->falling_threshold;;
        conf.rising_event_index = rmon_req->rising_event_index;
        conf.falling_event_index = rmon_req->falling_event_index;
        conf.interval = rmon_req->interval;

        if (conf.rising_threshold <= conf.falling_threshold) {
            CPRINTF("'Rising threshold' must larger than 'Falling threshold'");
            return ;
        }
        if ((rc = rmon_mgmt_alarm_entry_add(&conf)) != VTSS_OK) {
            CPRINTF("rmon_mgmt_statistics_entry_add(%d, %s): failed(%s)\n",
                    conf.id,
                    oid2str(rmon_req->oid, rmon_req->oid_len), error_txt(rc));
        }
    }
}

static void RMON_cli_cmd_rmon_alarm_del(cli_req_t *req)
{
    vtss_alarm_ctrl_entry_t conf;
    vtss_rc rc = 0;
    rmon_cli_req_t *rmon_req = req->module_req;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        conf.id = rmon_req->entry_idx;
#if 0
        if (rmon_mgmt_alarm_entry_get (&conf, FALSE) == VTSS_OK) {
            CPRINTF("The entry '%ld, %s' is deleted now\n",
                    conf.id, oid2str(conf.var_name.objid, conf.var_name.length));
        }
#endif

        if ((rc = rmon_mgmt_alarm_entry_del(&conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    }

}

static void RMON_cli_cmd_rmon_alarm_lookup(cli_req_t *req)
{
    vtss_alarm_ctrl_entry_t conf;
    rmon_cli_req_t *rmon_req = req->module_req;
    int cnt = 0;

    memset(&conf, 0, sizeof(vtss_alarm_ctrl_entry_t));
    while (rmon_mgmt_alarm_entry_get(&conf, TRUE) == VTSS_OK) {
        if (req->set) {
            if (conf.id != rmon_req->entry_idx ) {
                continue;
            } else {
                CPRINTF("Alarm Index                  : %d\n", conf.id);
                CPRINTF("Alarm Interval               : %u\n", conf.interval);
                CPRINTF("Alarm Variable               : %s\n",  oid2str(conf.var_name.objid, conf.var_name.length));
                CPRINTF("Alarm SampleType             : %s\n",
                        (SAMPLE_TYPE_ABSOLUTE == conf.sample_type ) ? "absoluteValue" :
                        (SAMPLE_TYPE_DELTA == conf.sample_type ) ? "deltaValue" : "unKnown");
                CPRINTF("Alarm Value                  : %u\n", conf.value);
                CPRINTF("Alarm Startup                : %s\n",
                        (ALARM_RISING == conf.startup_type ) ? "risingAlarm" : ( ALARM_FALLING == conf.startup_type ) ? "fallingAlarm" :
                        (ALARM_BOTH == conf.startup_type ) ? "risingOrFallingAlarm" : "unKnown");
                CPRINTF("Alarm RisingThrld            : %u\n", conf.rising_threshold);
                CPRINTF("Alarm FallingThrld           : %u\n", conf.falling_threshold);
                CPRINTF("Alarm RisingEventIndex       : %u\n", conf.rising_event_index);
                CPRINTF("Alarm FallingEventIndex      : %u\n", conf.falling_event_index);

                return;
            }
        }

        if (++cnt == 1) {
            CPRINTF("Id     Interval  Alarm Variable            Alarm SampleType\n");
            CPRINTF("----- --------- ------------------------  ------------------------\n");

        }

        if (rmon_mgmt_alarm_entry_get (&conf, FALSE)) {
            CPRINTF("GET fail\n");
            return;
        }
        CPRINTF("%-5u %-9u %-32s %-26s \n", conf.id, conf.interval, oid2str(conf.var_name.objid,
                                                                            conf.var_name.length), (SAMPLE_TYPE_ABSOLUTE == conf.sample_type ) ? "absoluteValue" :
                (SAMPLE_TYPE_DELTA == conf.sample_type ) ? "deltaValue" : "unKnown" );
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    }

    return ;
}


static void RMON_cli_cmd_rmon_event_add(cli_req_t *req)
{
    vtss_rc rc = 0;
    vtss_event_ctrl_entry_t conf;
    rmon_cli_req_t *rmon_req = req->module_req;
    u8 found = 0x0;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        conf.id = rmon_req->entry_idx;

        if (rmon_mgmt_event_entry_get (&conf, FALSE) == VTSS_OK) {
            found = 0x1;
        }


        if (rmon_req->event_type) {
            conf.event_type = rmon_req->event_type;
        } else if (!found) {
            conf.event_type = EVENT_NONE;
        }

        if (rmon_req->event_description[0] != '\0') {
            conf.event_description = VTSS_STRDUP(rmon_req->event_description);
        } else if (!found) {
            conf.event_description = NULL;
        }


        if (rmon_req->event_community[0] != '\0') {
            conf.event_community = VTSS_STRDUP(rmon_req->event_community);
        } else if (!found) {
            conf.event_community = VTSS_STRDUP("public");
        }


        if ((rc = rmon_mgmt_event_entry_add(&conf)) != VTSS_OK) {
            CPRINTF("rmon_mgmt_event_entry_add(%d, %s): failed(%s)\n",
                    conf.id,
                    oid2str(rmon_req->oid, rmon_req->oid_len), error_txt(rc));
        }
    }

}

static void RMON_cli_cmd_rmon_event_del(cli_req_t *req)
{
    vtss_event_ctrl_entry_t conf;
    vtss_rc rc = 0;
    rmon_cli_req_t *rmon_req = req->module_req;

    if (req->set) {
        memset(&conf, 0x0, sizeof(conf));
        conf.id = rmon_req->entry_idx;

        if ((rc = rmon_mgmt_event_entry_del(&conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    }

}

static void RMON_cli_cmd_rmon_event_lookup(cli_req_t *req)
{
    vtss_event_ctrl_entry_t conf;
    vtss_event_data_entry_t data;
    rmon_cli_req_t *rmon_req = req->module_req;
    int cnt = 0;

    memset(&conf, 0, sizeof(vtss_event_ctrl_entry_t));
    memset(&data, 0, sizeof(vtss_event_data_entry_t));
    while (rmon_mgmt_event_entry_get(&conf, TRUE) == VTSS_OK) {
        if (req->set) {
            if (conf.id != rmon_req->entry_idx ) {
                continue;
            } else {
                CPRINTF("Event Index                  : %d\n", conf.id);
                CPRINTF("Event Description            : %s\n", (conf.event_description) ? conf.event_description : "");
                CPRINTF("Event Type                   : %s\n", (EVENT_NONE == conf.event_type) ? "none" :
                        (EVENT_LOG == conf.event_type) ? "log" : (EVENT_TRAP == conf.event_type) ? "snmptrap" :
                        (EVENT_LOG_AND_TRAP == conf.event_type) ? "logandtrap" : "unKnown");
                CPRINTF("Event Community              : %s\n", conf.event_community);
                CPRINTF("Event LastSent               : %s\n", misc_time2interval(conf.event_last_time_sent / 100));

                while (rmon_mgmt_event_data_get(conf.id, &data, TRUE) == VTSS_OK ) {
                    CPRINTF("Log Index                : %lu\n", data.data_index);
                    CPRINTF("Log Time                 : %s\n", misc_time2interval(data.log_time / 100));
                    CPRINTF("Log Description          : %s\n", data.log_description);
                }


                return;
            }
        }

        if (++cnt == 1) {
            CPRINTF("Id     Description  Type  Community   LastSent\n");
            CPRINTF("----- ------------ ----- ----------- ---------\n");

        }

        if (rmon_mgmt_event_entry_get (&conf, FALSE)) {
            CPRINTF("GET fail\n");
            return;
        }
        CPRINTF("%-5u %-9s %-5s %-9s %-9s\n", conf.id, (conf.event_description) ? conf.event_description : "",
                (EVENT_NONE == conf.event_type) ? "none" : (EVENT_LOG == conf.event_type) ? "log" : (EVENT_TRAP == conf.event_type) ? "snmptrap" :
                (EVENT_LOG_AND_TRAP == conf.event_type) ? "logandtrap" : "unKnown", conf.event_community, misc_time2interval(conf.event_last_time_sent / 100) );
    }
    if (cnt) {
        CPRINTF("\nNumber of entries: %d\n", cnt);
    }

    return ;
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t RMON_cli_parse_keyword (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    rmon_cli_req_t *rmon_req = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "absolute", 8)) {
            rmon_req->sample_type = SAMPLE_TYPE_ABSOLUTE;
        } else if (!strncmp(found, "delta", 5)) {
            rmon_req->sample_type = SAMPLE_TYPE_DELTA;
        } else if (!strncmp(found, "rising", 6)) {
            rmon_req->startup_type = ALARM_RISING;
        } else if (!strncmp(found, "falling", 7)) {
            rmon_req->startup_type = ALARM_FALLING;
        } else if (!strncmp(found, "both", 4)) {
            rmon_req->startup_type = ALARM_BOTH;
        } else if (!strncmp(found, "none", 4)) {
            rmon_req->event_type = EVENT_NONE;
        } else if (!strncmp(found, "log", 3) && strlen(found) <= 3) {
            rmon_req->event_type = EVENT_LOG;
        } else if (!strncmp(found, "trap", 4)) {
            rmon_req->event_type = EVENT_TRAP;
        } else if (!strncmp(found, "log_trap", 8)) {
            rmon_req->event_type = EVENT_LOG_AND_TRAP;
        }

    }

    return (found == NULL ? 1 : 0);
}


static int32_t RMON_cli_rmon_index_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    rmon_cli_req_t  *rmon_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 1, RMON_MAX_ENTRIES);
    rmon_req->entry_idx = value;

    return error;
}

static int32_t RMON_cli_rmon_history_interval_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    rmon_cli_req_t  *rmon_req = req->module_req;

    error = cli_parse_ulong_wc(cmd, &value, 1, RMON_HISTORY_MAX_INTERVAL, &rmon_req->interval_spec);
    if (!error && rmon_req->interval_spec == CLI_SPEC_VAL) {
        rmon_req->interval = value;
    }

    return error;
}

static int32_t RMON_cli_rmon_alarm_interval_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    rmon_cli_req_t  *rmon_req = req->module_req;

    error = cli_parse_ulong_wc(cmd, &value, 1, RMON_ALARM_MAX_INTERVAL, &rmon_req->interval_spec);
    if (!error && rmon_req->interval_spec == CLI_SPEC_VAL) {
        rmon_req->interval = value;
    }

    return error;

}

static int32_t RMON_cli_rmon_history_buckets_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    rmon_cli_req_t  *rmon_req = req->module_req;

    error = cli_parse_ulong_wc(cmd, &value, 1, RMON_BUCKETS_REQ_MAX, &rmon_req->buckets_spec);
    if (!error && rmon_req->buckets_spec == CLI_SPEC_VAL) {
        rmon_req->buckets = value;
    }

    return error;
}

static int32_t RMON_cli_rmon_rising_threshold_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    rmon_cli_req_t  *rmon_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, RMON_ALARM_MIN_THRESHOLD, RMON_ALARM_MAX_THRESHOLD);
    rmon_req->rising_threshold = value;

    return error;
}

static int32_t RMON_cli_rmon_falling_threshold_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    rmon_cli_req_t  *rmon_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, RMON_ALARM_MIN_THRESHOLD, RMON_ALARM_MAX_THRESHOLD);
    rmon_req->falling_threshold = value;

    return error;
}

static int32_t RMON_cli_rmon_rising_event_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    rmon_cli_req_t  *rmon_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 1, RMON_MAX_ENTRIES);
    rmon_req->rising_event_index = value;

    return error;
}

static int32_t RMON_cli_rmon_falling_event_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             value = 0;
    rmon_cli_req_t  *rmon_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 1, RMON_MAX_ENTRIES);
    rmon_req->falling_event_index = value;

    return error;
}

static int32_t RMON_cli_rmon_event_description_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    rmon_cli_req_t *rmon_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, rmon_req->event_description, SNMPV3_MIN_EVENT_DESC_LEN, SNMPV3_MAX_EVENT_DESC_LEN);

    return error;
}

static int32_t RMON_cli_rmon_event_community_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    rmon_cli_req_t *rmon_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_string(cmd_org, rmon_req->event_community, SNMPV3_MIN_EVENT_COMMUNITY_LEN, SNMPV3_MAX_EVENT_COMMUNITY_LEN);

    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t snmp_cli_parm_table[] = {
    {
        "<stats_id>",
        "Statistics ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        NULL
    },

    {
        "<history_id>",
        "History ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        NULL
    },

    {
        "<alarm_id>",
        "Alarm ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        NULL
    },

    {
        "<event_id>",
        "Event ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        NULL
    },

    {
        "<stats_id>",
        "Statistics ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        RMON_cli_cmd_rmon_stats_add
    },
    {
        "<data_source>",
        "The OID that indicates that the ifIndex in ifEntry.\n"
        "The value should be like .1.3.6.1.2.1.2.2.1.1.xxx.",
        CLI_PARM_FLAG_SET,
        RMON_cli_oid_subtree_parse,
        RMON_cli_cmd_rmon_stats_add
    },
    {
        "<stats_id>",
        "Statistics ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        RMON_cli_cmd_rmon_stats_del
    },

    {
        "<history_id>",
        "Statistics ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        RMON_cli_cmd_rmon_history_add
    },
    {
        "<data_source>",
        "The OID that indicates that the ifIndex in ifEntry.\n"
        "The value should be like .1.3.6.1.2.1.2.2.1.1.xxx.",
        CLI_PARM_FLAG_SET,
        RMON_cli_oid_subtree_parse,
        RMON_cli_cmd_rmon_history_add
    },
    {
        "<interval>",
        "Sampling interval (1-3600) (default: 1800).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_history_interval_parse,
        RMON_cli_cmd_rmon_history_add
    },
    {
        "<buckets>",
        "The maximum data entries associated this History control entry stored in RMON(1-65535) (default: 50).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_history_buckets_parse,
        RMON_cli_cmd_rmon_history_add
    },

    {
        "<history_id>",
        "Statistics ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        RMON_cli_cmd_rmon_history_del
    },

    {
        "<alarm_id>",
        "Alarm ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        RMON_cli_cmd_rmon_alarm_add
    },

    {
        "<interval>",
        "Sampling interval (1-2147483647) (default: 30).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_alarm_interval_parse,
        RMON_cli_cmd_rmon_alarm_add
    },

    {
        "<alarm_variable>",
        "The MIB OID that need to be referenced.\n"
        ".1.3.6.1.2.1.2.2.1.10.xxx ¡V ifInOctets\n"
        ".1.3.6.1.2.1.2.2.1.11.xxx ¡V ifInUcastPkts\n"
        ".1.3.6.1.2.1.2.2.1.12.xxx ¡V ifInNUcastPkts\n"
        ".1.3.6.1.2.1.2.2.1.13.xxx ¡V ifInDiscards\n"
        ".1.3.6.1.2.1.2.2.1.14.xxx ¡V ifInErrors\n"
        ".1.3.6.1.2.1.2.2.1.15.xxx ¡V ifInUnkownProtos\n"
        ".1.3.6.1.2.1.2.2.1.16.xxx ¡V ifOutOctets\n"
        ".1.3.6.1.2.1.2.2.1.17.xxx ¡V ifOutUcastPkts\n"
        ".1.3.6.1.2.1.2.2.1.18.xxx ¡V ifOutNUcastPkts\n"
        ".1.3.6.1.2.1.2.2.1.19.xxx ¡V ifOutDiscards\n"
        ".1.3.6.1.2.1.2.2.1.20.xxx ¡V ifOutErrors\n"
        ".1.3.6.1.2.1.2.2.1.21.xxx ¡V ifOutQLen\n"
        " \"xxx\" means the interface identified by a particular value of this index is the same interface as identified by the same value of OID  \'ifIndex\'.",
        CLI_PARM_FLAG_SET,
        RMON_cli_oid_subtree_parse,
        RMON_cli_cmd_rmon_alarm_add
    },

    {
        "absolute|delta",
        "absolute             : Get the sample directly.\n"
        "delta                : Calculate the difference between samples (default).",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        RMON_cli_parse_keyword,
        RMON_cli_cmd_rmon_alarm_add
    },

    {
        "<rising_threshold>",
        "Rising threshold value (-2147483648¡V2147483647).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_rising_threshold_parse,
        RMON_cli_cmd_rmon_alarm_add
    },

    {
        "<rising_event_index>",
        "Rising event index (1-65535).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_rising_event_parse,
        RMON_cli_cmd_rmon_alarm_add
    },

    {
        "<falling_threshold>",
        "Falling threshold value (-2147483648¡V2147483647).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_falling_threshold_parse,
        RMON_cli_cmd_rmon_alarm_add
    },

    {
        "<falling_event_index>",
        "Falling event index (1-65535).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_falling_event_parse,
        RMON_cli_cmd_rmon_alarm_add
    },

    {
        "rising|falling|both",
        "rising               : Trigger alarm when the first value is larger than the rising threshold.\n"
        "falling              : Trigger alarm when the first value is less than the falling threshold.\n"
        "both                 : Trigger alarm when the first value is larger than the rising threshold or less than the falling threshold (default)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        RMON_cli_parse_keyword,
        RMON_cli_cmd_rmon_alarm_add

    },

    {
        "<alarm_id>",
        "Alarm ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        RMON_cli_cmd_rmon_alarm_del
    },

    {
        "<event_id>",
        "Event ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        RMON_cli_cmd_rmon_event_add
    },

    {
        "<description>",
        "The string for describing this event (the string length is 0~127) (default: null string).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_event_description_parse,
        RMON_cli_cmd_rmon_event_add
    },

    {
        "none|log|trap|log_trap",
        "none           : Get the sample directly.\n"
        "log            : Get the sample directly.\n"
        "trap           : Get the sample directly.\n"
        "log_trap       : Calculate the difference between samples (default).",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        RMON_cli_parse_keyword,
        RMON_cli_cmd_rmon_event_add
    },

    {
        "<community>",
        "Specify the community when trap is sent (the string length is 0~127) (default: public).",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_event_community_parse,
        RMON_cli_cmd_rmon_event_add
    },
    {
        "<event_id>",
        "Alarm ID (1-"vtss_xstr(RMON_MAX_ENTRIES)").",
        CLI_PARM_FLAG_SET,
        RMON_cli_rmon_index_parse,
        RMON_cli_cmd_rmon_event_del
    },


    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    RMON_STATS_ADD,
    RMON_STATS_DEL,
    RMON_STATS_LOOKUP,
    RMON_HISTORY_ADD,
    RMON_HISTORY_DEL,
    RMON_HISTORY_LOOKUP,
    RMON_ALARM_ADD,
    RMON_ALARM_DEL,
    RMON_ALARM_LOOKUP,
    RMON_EVENT_ADD,
    RMON_EVENT_DEL,
    RMON_EVENT_LOOKUP
};

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Statistics Add <stats_id> <data_source>",
    "Add or modify RMON Statistics entry.\n"
    "The entry index key is <stats_id>",
    RMON_STATS_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_stats_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Statistics Delete <stats_id>",
    "Delete RMON Statistics entry.\n"
    "The entry index key is <stats_id>",
    RMON_STATS_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_stats_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Statistics Lookup [<stats_id>]",
    NULL,
    "Show RMON Statistics entries",
    RMON_STATS_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_stats_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON History Add <history_id> <data_source> [<interval>] [<buckets>]",
    "Add or modify RMON Hisotry entry.\n"
    "The entry index key is <history_id>",
    RMON_HISTORY_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_history_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON History Delete <history_id>",
    "Delete RMON Hisotry entry.\n"
    "The entry index key is <history_id>",
    RMON_HISTORY_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_history_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON History Lookup [<history_id>]",
    NULL,
    "Show RMON History entries",
    RMON_HISTORY_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_history_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Alarm Add <alarm_id> <interval> <alarm_variable> [absolute|delta]\n"
    "          <rising_threshold> <rising_event_index> <falling_threshold>\n"
    "          <falling_event_index> [rising|falling|both]",
    "Add or modify RMON Alarm entry.\n"
    "The entry index key is <alarm_id>",
    RMON_ALARM_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_alarm_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Alarm Delete <alarm_id>",
    "Delete RMON Alarm entry.\n"
    "The entry index key is <alarm_id>",
    RMON_ALARM_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_alarm_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Alarm Lookup [<alarm_id>]",
    NULL,
    "Show RMON Alarm entries",
    RMON_ALARM_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_alarm_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Event Add <event_id> [none|log|trap|log_trap] [<community>]\n"
    "          [<description>]",
    "Add or modify RMON Event entry.\n"
    "The entry index key is <event_id>",
    RMON_EVENT_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_event_add,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Event Delete <event_id>",
    "Delete RMON Event entry.\n"
    "The entry index key is <event_id>",
    RMON_EVENT_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_event_del,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON Event Lookup [<event_id>]",
    NULL,
    "Show RMON Event entries",
    RMON_EVENT_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_RMON,
    RMON_cli_cmd_rmon_event_lookup,
    NULL,
    snmp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);



/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/

