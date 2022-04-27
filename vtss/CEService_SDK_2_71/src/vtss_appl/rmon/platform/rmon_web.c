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

#include "web_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_RMON

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

#include "rmon_api.h"

static cyg_int32 handler_rmon_stats_config(CYG_HTTPD_STATE *p)
{
    int                     ct;
    vtss_stat_ctrl_entry_t conf, newconf;
    ulong                   idx = 0, change_flag;
    size_t                  len = 64 * sizeof(char);
    char                    buf[64] ;
    vtss_rc                 rc;
    char                    encoded_string[3 * 64];
    int                     iport = 0;
    int                     count = 0;
    oid                     name[] = {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 0};
    int                     tbl_first_index_begin = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    tbl_first_index_begin = sizeof(name) / sizeof(oid) - 1;

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        memset(&conf, 0, sizeof(conf));
        while (rmon_mgmt_statistics_entry_get(&conf, TRUE) == VTSS_OK) {
            newconf = conf;
            change_flag = 0;

            /* delete entry */
            sprintf(buf, "del_%d", conf.id);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            //var_string = cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (cyg_httpd_form_varable_string(p, encoded_string, &len) && len > 0) {
//                change_flag = del_flag = 1;
                (void) rmon_mgmt_statistics_entry_del(&newconf);
            } else {
                //data source
                sprintf(buf, "datasource_%d", conf.id);
                (void) cyg_httpd_form_varable_int(p, buf, &iport);

                name[tbl_first_index_begin] = iport;

                newconf.data_source.length = tbl_first_index_begin + 1;
                {
                    int i = 0;
                    for (i = 0; i < ( int ) newconf.data_source.length; i ++) {
                        newconf.data_source.objid[i] = name[i];
                    }
                }

                if (name[tbl_first_index_begin] != conf.data_source.objid[tbl_first_index_begin]) {
                    change_flag = 1;
                }

            }

            if (change_flag) {
                T_D("Calling rmon_mgmt_statistics_entry_add(%u)", newconf.id);
                if (rmon_mgmt_statistics_entry_add(&newconf) < 0) {
                    T_E("rmon_mgmt_statistics_entry_add(%u): failed", newconf.id);
                }
#if 0
                if (del_flag) {
                    rmon_mgmt_statistics_entry_del(&newconf);
                }
#endif
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_id_%d", idx);
        while (cyg_httpd_form_varable_int(p, buf, &iport)  > 0) {
            memset(&newconf, 0x0, sizeof(newconf));
            //community
            newconf.id = iport;

            //sip_mask
            sprintf(buf, "new_datasource_%d", idx);
            (void) cyg_httpd_form_varable_int(p, buf, &iport);
            name[tbl_first_index_begin] = iport;

            newconf.data_source.length = tbl_first_index_begin + 1;
            {
                int i = 0;
                for (i = 0; i < ( int ) newconf.data_source.length; i ++) {
                    newconf.data_source.objid[i] = name[i];
                }
            }

            T_D("Calling rmon_mgmt_statistics_entry_add(%u)", newconf.id);
            if ((rc = rmon_mgmt_statistics_entry_add(&newconf)) < 0) {
                if (rc == RMON_ERROR_STAT_TABLE_FULL) {
                    char *err = "RMON statistics table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("rmon_mgmt_statistics_entry_add(%u): failed", newconf.id);
                }
            }
            idx++;
            sprintf(buf, "new_id_%d", idx);
        }

        redirect(p, "/rmon_statistics_config.htm");


    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <id>/<data_source>
        */
        memset(&conf, 0, sizeof(conf));
        while (rmon_mgmt_statistics_entry_get(&conf, TRUE) == VTSS_OK) {

            if (count++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            iport = conf.data_source.objid[conf.data_source.length - 1];
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u",
                          conf.id, iport);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();

    }
    return -1;
}


static cyg_int32 handler_rmon_stats_status(CYG_HTTPD_STATE *p)
{
    vtss_stat_ctrl_entry_t  entry;
    int                     ct;
    int                     count = 0;
    vtss_port_no_t          iport;
    int                     num_of_entries = 0;
    int                     nextEntry = 0;
    int                     startCtrlId = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(&entry, 0, sizeof(entry));

    /* Format: <StartCtrlId>,<NumberOfEntries>,1/0/2/0/128/0/0/0/0/1|2/175/19/23597/234/3518/0/0/0/0|... */

    /* Get number of entries per page */
    (void) cyg_httpd_form_varable_int(p, "DynNumberOfEntries", &num_of_entries);

    if (num_of_entries <= 0 || num_of_entries > 99) {
        num_of_entries = 20;
    }

    (void) cyg_httpd_form_varable_int(p, "StartCtrlId", &startCtrlId);

    (void) cyg_httpd_form_varable_int(p, "GetNextEntry", &nextEntry);

    if (nextEntry) {
        entry.id = startCtrlId;
    } else {
        entry.id = !startCtrlId ? 0 : startCtrlId - 1;
    }
    (void)cyg_httpd_start_chunked("html");

    while (!rmon_mgmt_statistics_entry_get(&entry, TRUE)) {
        /* Output the counters */

        if (count++) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        } else if (nextEntry) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%d,", entry.id, num_of_entries);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,", startCtrlId, num_of_entries);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        if (FALSE == rmon_etherStatsTable_entry_update(&entry.data_source, &entry.eth)) {
            ;
        }
        iport = entry.data_source.objid[entry.data_source.length - 1];
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u",
                      entry.id,
                      iport,
                      entry.eth.drop_events   ,
                      entry.eth.octets        ,
                      entry.eth.packets       ,
                      entry.eth.bcast_pkts    ,
                      entry.eth.mcast_pkts    ,
                      entry.eth.crc_align     ,
                      entry.eth.undersize     ,
                      entry.eth.oversize      ,
                      entry.eth.fragments     ,
                      entry.eth.jabbers       ,
                      entry.eth.collisions    ,
                      entry.eth.pkts_64       ,
                      entry.eth.pkts_65_127   ,
                      entry.eth.pkts_128_255  ,
                      entry.eth.pkts_256_511  ,
                      entry.eth.pkts_512_1023 ,
                      entry.eth.pkts_1024_1518
                     );
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        if (count == num_of_entries) {
            goto end_chunked;
        }
    }

    if (!count) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%s", startCtrlId, num_of_entries, "NoEntries");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

    }

end_chunked:
    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

#define GET_SID(_port) _port/1000
#define GET_PORTID(_port) _port%1000

static cyg_int32 handler_rmon_stats_detail(CYG_HTTPD_STATE *p)
{
    vtss_stat_ctrl_entry_t  entry, data;
    int                     ct, val;
    vtss_port_no_t          iport;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(&entry, 0, sizeof(entry));
    memset(&data, 0, sizeof(data));

    (void)cyg_httpd_start_chunked("html");

    if (cyg_httpd_form_varable_int(p, "index", &val)) {
        data.id = (vtss_rmon_ctrl_id_t)val;
    }

    if (rmon_mgmt_statistics_entry_get(&data, FALSE) != VTSS_OK) {
        goto out;    /* Most likely stack error - bail out */
    }

    /* all of the entries index     */
    while ((rmon_mgmt_statistics_entry_get(&entry, TRUE) == VTSS_OK)) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", entry.id);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    (void)rmon_etherStatsTable_entry_update(&data.data_source, &data.eth);
    iport = data.data_source.objid[entry.data_source.length - 1];
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u",
                  data.id,
                  iport,
                  data.eth.drop_events   ,
                  data.eth.octets        ,
                  data.eth.packets       ,
                  data.eth.bcast_pkts    ,
                  data.eth.mcast_pkts    ,
                  data.eth.crc_align     ,
                  data.eth.undersize     ,
                  data.eth.oversize      ,
                  data.eth.fragments     ,
                  data.eth.jabbers       ,
                  data.eth.collisions    ,
                  data.eth.pkts_64       ,
                  data.eth.pkts_65_127   ,
                  data.eth.pkts_128_255  ,
                  data.eth.pkts_256_511  ,
                  data.eth.pkts_512_1023 ,
                  data.eth.pkts_1024_1518);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);


out:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_rmon_history_config(CYG_HTTPD_STATE *p)
{
    int                         ct;
    vtss_history_ctrl_entry_t   conf, newconf;
    ulong                       idx = 0, change_flag;
    size_t                      len = 64 * sizeof(char);
    char                        buf[64];
    vtss_rc                     rc;
    char                        encoded_string[3 * 64];
    int                         iport = 0;
    int                         count = 0;
    oid                         name[] = {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 0};
    int                         tbl_first_index_begin = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    tbl_first_index_begin = sizeof(name) / sizeof(oid) - 1;

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        memset(&conf, 0, sizeof(conf));
        while (rmon_mgmt_history_entry_get(&conf, TRUE) == VTSS_OK) {
            newconf = conf;
            change_flag = 0;

            /* delete entry */
            sprintf(buf, "del_%d", conf.id);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
//            var_string = cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (cyg_httpd_form_varable_string(p, encoded_string, &len) && len > 0) {
//                change_flag = del_flag = 1;
                (void)rmon_mgmt_history_entry_del(&newconf);
            } else {
                //data source
                sprintf(buf, "datasource_%d", conf.id);
                (void) cyg_httpd_form_varable_int(p, buf, &iport);

                name[tbl_first_index_begin] = iport;

                newconf.data_source.length = tbl_first_index_begin + 1;
                {
                    int i = 0;
                    for (i = 0; i < ( int ) newconf.data_source.length; i ++) {
                        newconf.data_source.objid[i] = name[i];
                    }
                }

                if (name[tbl_first_index_begin] != conf.data_source.objid[tbl_first_index_begin]) {
                    change_flag = 1;
                }

                //interval
                sprintf(buf, "interval_%d", conf.id);
                (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.interval);


                if (newconf.interval != conf.interval) {
                    change_flag = 1;
                }

                //buckets
                sprintf(buf, "buckets_%d", conf.id);
                (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.scrlr.data_requested);

                if (newconf.scrlr.data_requested != conf.scrlr.data_requested) {
                    change_flag = 1;
                }


            }

            if (change_flag) {
                T_D("Calling rmon_mgmt_history_entry_add(%u)", newconf.id);
                if (rmon_mgmt_history_entry_add(&newconf) < 0) {
                    T_E("rmon_mgmt_history_entry_add(%u): failed", newconf.id);
                }
#if 0
                if (del_flag) {
                    rmon_mgmt_history_entry_del(&newconf);
                }
#endif
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_id_%d", idx);
        while (cyg_httpd_form_varable_int(p, buf, &iport)  > 0) {
            memset(&newconf, 0x0, sizeof(newconf));
            //community
            newconf.id = iport;

            //sip_mask
            sprintf(buf, "new_datasource_%d", idx);
            (void) cyg_httpd_form_varable_int(p, buf, &iport);
            name[tbl_first_index_begin] = iport;

            newconf.data_source.length = tbl_first_index_begin + 1;
            {
                int i = 0;
                for (i = 0; i < ( int ) newconf.data_source.length; i ++) {
                    newconf.data_source.objid[i] = name[i];
                }
            }

            //interval
            sprintf(buf, "new_interval_%d", idx);
            (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.interval);

            //buckets
            sprintf(buf, "new_buckets_%d", idx);
            (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.scrlr.data_requested);

            T_D("Calling rmon_mgmt_history_entry_add(%u)", idx);
            if ((rc = rmon_mgmt_history_entry_add(&newconf)) < 0) {
                if (rc == RMON_ERROR_HISTORY_TABLE_FULL) {
                    char *err = "RMON history table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("rmon_mgmt_history_entry_add(%u): failed", idx);
                }
            }
            idx++;
            sprintf(buf, "new_id_%d", idx);
        }

        redirect(p, "/rmon_history_config.htm");


    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <id>/<data_source>
        */
        memset(&conf, 0, sizeof(conf));
        while (rmon_mgmt_history_entry_get(&conf, TRUE) == VTSS_OK) {

            if (count++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            iport = conf.data_source.objid[conf.data_source.length - 1];
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%lu/%lu/%lu",
                          conf.id, iport, conf.interval, conf.scrlr.data_requested,
                          conf.scrlr.data_granted);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();

    }
    return -1;

}

static cyg_int32 handler_rmon_history_status(CYG_HTTPD_STATE *p)
{
    vtss_history_ctrl_entry_t   entry;
    vtss_history_data_entry_t   data;
    int                         ct;
    int                         count = 0;
    int                         num_of_entries = 0;
    int                         nextEntry = 0;
    int                         startCtrlId = 0, startDataId = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(&entry, 0, sizeof(entry));
    memset(&data, 0, sizeof(data));

    /*  Format: <StartCtrlId>,<StartDataId>,<NumberOfEntries>,<index>/<data_id>/<sample_start>/<drop>/<octets>/<Pkts><broadcast>/<multicast>/<crc>/
                <undersize>/<oversize>/<frag>/<jabb>/<coll>/<utilization>|... */

    /* Get number of entries per page */
    (void) cyg_httpd_form_varable_int(p, "DynNumberOfEntries", &num_of_entries);

    if (num_of_entries <= 0 || num_of_entries > 99) {
        num_of_entries = 20;
    }

    (void) cyg_httpd_form_varable_int(p, "StartCtrlId", &startCtrlId);

    (void) cyg_httpd_form_varable_int(p, "StartDataId", &startDataId);

    (void) cyg_httpd_form_varable_int(p, "GetNextEntry", &nextEntry);


    (void)cyg_httpd_start_chunked("html");

    if (nextEntry) {
        data.data_index = startDataId;
    } else {
        data.data_index = !startDataId ? 0 : startDataId - 1;
    }
    entry.id = !startCtrlId ? 0 : startCtrlId - 1;
    while (rmon_mgmt_history_entry_get(&entry, TRUE) == VTSS_OK ) {
        while (!rmon_mgmt_history_data_get(entry.id, &data, TRUE)) {
            /* Output the counters */

            if (count++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else if (nextEntry) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%lu,%d,", entry.id, data.data_index, num_of_entries);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%d,", startCtrlId, startDataId, num_of_entries);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%ld/%lu/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%lu",
                          entry.id,
                          data.data_index,
                          data.start_interval / 100,
                          data.EthData.drop_events   ,
                          data.EthData.octets        ,
                          data.EthData.packets       ,
                          data.EthData.bcast_pkts    ,
                          data.EthData.mcast_pkts    ,
                          data.EthData.crc_align     ,
                          data.EthData.undersize     ,
                          data.EthData.oversize      ,
                          data.EthData.fragments     ,
                          data.EthData.jabbers       ,
                          data.EthData.collisions    ,
                          data.utilization
                         );
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            if (count == num_of_entries) {
                goto end_chunked;
            }

        }
        memset(&data, 0, sizeof(data));
    }
    if (!count) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%d,%s", startCtrlId, startDataId, num_of_entries, "NoEntries");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

    }

end_chunked:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_rmon_history_detail(CYG_HTTPD_STATE *p)
{
    vtss_history_ctrl_entry_t   entry;
    vtss_history_data_entry_t   data, traverse;
    int                         ct, val;
    vtss_rmon_ctrl_id_t         ctrl_index = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(&entry, 0, sizeof(entry));
    memset(&data, 0, sizeof(data));
    memset(&traverse, 0, sizeof(traverse));

    (void)cyg_httpd_start_chunked("html");

    if (cyg_httpd_form_varable_int(p, "index", &val)) {
        ctrl_index = (vtss_rmon_ctrl_id_t)val;
    }

    if (cyg_httpd_form_varable_int(p, "dataId", &val)) {
        data.data_index = (vtss_rmon_ctrl_id_t)val;
    }

    /*  Get the data entry  */
    if (rmon_mgmt_history_data_get(ctrl_index, &data, FALSE) != VTSS_OK) {
        goto out;    /* Most likely stack error - bail out */
    }

    /* all of the entries index     */
    memset(&entry, 0, sizeof(entry));
    while (!rmon_mgmt_history_entry_get(&entry, TRUE)) {
        while (!rmon_mgmt_history_data_get(entry.id, &traverse, TRUE)) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%ld,", entry.id, traverse.data_index);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }

    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%ld/%lu/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%lu",
                  ctrl_index,
                  data.data_index,
                  data.start_interval / 100,
                  data.EthData.drop_events   ,
                  data.EthData.octets        ,
                  data.EthData.packets       ,
                  data.EthData.bcast_pkts    ,
                  data.EthData.mcast_pkts    ,
                  data.EthData.crc_align     ,
                  data.EthData.undersize     ,
                  data.EthData.oversize      ,
                  data.EthData.fragments     ,
                  data.EthData.jabbers       ,
                  data.EthData.collisions    ,
                  data.utilization
                 );
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);


out:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}


#define VAR2IFENTRY(_VAR) ((_VAR)+9)
#define IFENTRY2VAR(_INDEX) ((_INDEX)-9)

static cyg_int32 handler_rmon_alarm_config(CYG_HTTPD_STATE *p)
{
    int                     ct, val;
    vtss_alarm_ctrl_entry_t conf, newconf;
    ulong                   idx = 0, change_flag;
    const char              *var_string;
    size_t                  len = 64 * sizeof(char);
    char                    buf[64] ;
    vtss_rc                 rc;
    char                    encoded_string[3 * 64];
    int                     iport;
    ulong                   variable;
    char                    oidStr[128] = {0};
    int                     count = 0;
    oid                     name[] = {1, 3, 6, 1, 2, 1, 2, 2, 1, 0, 0};
    int                     tbl_first_index_begin = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    tbl_first_index_begin = sizeof(name) / sizeof(oid) - 2;

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        memset(&conf, 0, sizeof(conf));

        while (rmon_mgmt_alarm_entry_get(&conf, TRUE) == VTSS_OK) {
            newconf = conf;
            change_flag = 0;

            /* delete entry */
            sprintf(buf, "del_%d", conf.id);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            ( void ) cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (len > 0) {
//                change_flag = del_flag = 1;
                (void)rmon_mgmt_alarm_entry_del(&newconf);
            } else {
                //interval
                sprintf(buf, "interval_%d", conf.id);
                (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.interval);

                if (newconf.interval != conf.interval) {
                    change_flag = 1;
                }


                //variable

                sprintf(buf, "variable_%d", conf.id);
                if (cgi_escape(buf, encoded_string) == 0) {
                    continue;
                }
                var_string = cyg_httpd_form_varable_string(p, encoded_string, &len);
                strncpy(oidStr, var_string, len );
                {
                    char *output = NULL;
                    char *tok = ".";
                    len = 0;
                    output = strtok(oidStr, tok);
                    while (output && len < 2) {
                        name[tbl_first_index_begin + len] = atoi(output);
                        output = strtok(NULL, tok);
                        len++;
                    }
                }

                newconf.var_name.length = tbl_first_index_begin + 2;
                {
                    int i = 0;
                    for (i = 0; i < ( int ) newconf.var_name.length; i ++) {
                        newconf.var_name.objid[i] = name[i];
                    }
                }

                if (name[tbl_first_index_begin] != conf.var_name.objid[tbl_first_index_begin] ||
                    name[tbl_first_index_begin + 1] != conf.var_name.objid[tbl_first_index_begin + 1]) {
                    change_flag = 1;
                }
                //sample
                sprintf(buf, "sample_%d", conf.id);
                (void) cyg_httpd_form_varable_int(p, buf, &val);
                newconf.sample_type = val;

                if (newconf.sample_type != conf.sample_type) {
                    change_flag = 1;
                }

                //startup
                sprintf(buf, "startup_%d", conf.id);
                (void) cyg_httpd_form_varable_int(p, buf, &val);
                newconf.startup_type = val;
                if (newconf.startup_type != conf.startup_type) {
                    change_flag = 1;
                }

                //risingthrld
                sprintf(buf, "risingthrld_%d", conf.id);
                (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.rising_threshold);

                if (newconf.rising_threshold != conf.rising_threshold) {
                    change_flag = 1;
                }
                //risingidx
                sprintf(buf, "risingidx_%d", conf.id);
                (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.rising_event_index);

                if (newconf.rising_event_index != conf.rising_event_index) {
                    change_flag = 1;
                }
                //fallingthrld
                sprintf(buf, "fallingthrld_%d", conf.id);
                (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.falling_threshold);

                if (newconf.falling_threshold != conf.falling_threshold) {
                    change_flag = 1;
                }
                //fallingidx
                sprintf(buf, "fallingidx_%d", conf.id);
                (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.falling_event_index);

                if (newconf.falling_event_index != conf.falling_event_index) {
                    change_flag = 1;
                }


            }

            if (change_flag) {
                T_D("Calling rmon_mgmt_alarm_entry_add(%u)", newconf.id);
                if (rmon_mgmt_alarm_entry_add(&newconf) < 0) {
                    T_E("rmon_mgmt_alarm_entry_add(%u): failed", newconf.id);
                }
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_id_%d", idx);
        while (cyg_httpd_form_varable_int(p, buf, &iport)) {
            memset(&newconf, 0x0, sizeof(newconf));
            //community
            newconf.id = iport;

            //interval
            sprintf(buf, "new_interval_%d", idx);
            (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.interval);

            //variable
            sprintf(buf, "new_variable_%d", idx);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            var_string = cyg_httpd_form_varable_string(p, encoded_string, &len);
            strncpy(oidStr, var_string, len);
            {
                char *output = NULL;
                char *tok = ".";
                len = 0;
                output = strtok(oidStr, tok);
                while (output && len < 2) {
                    name[tbl_first_index_begin + len] = atoi(output);
                    output = strtok(NULL, tok);
                    len++;
                }
            }

            newconf.var_name.length = tbl_first_index_begin + 2;
            {
                int i = 0;
                for (i = 0; i < ( int ) newconf.var_name.length; i ++) {
                    newconf.var_name.objid[i] = name[i];
                }
            }

            //sample
            sprintf(buf, "new_sample_%d", idx);
            (void) cyg_httpd_form_varable_int(p, buf, &val);
            newconf.sample_type = val;

            //startup
            sprintf(buf, "new_startup_%d", idx);
            (void) cyg_httpd_form_varable_int(p, buf, &val);
            newconf.startup_type = val;
            //risingthrld
            sprintf(buf, "new_risingthrld_%d", idx);
            (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.rising_threshold);

            //risingidx
            sprintf(buf, "new_risingidx_%d", idx);
            (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.rising_event_index);

            //fallingthrld
            sprintf(buf, "new_fallingthrld_%d", idx);
            (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.falling_threshold);

            //fallingidx
            sprintf(buf, "new_fallingidx_%d", idx);
            (void) cyg_httpd_form_varable_long_int(p, buf, &newconf.falling_event_index);



            T_D("Calling rmon_mgmt_alarm_entry_add(%u)", idx);
            if ((rc = rmon_mgmt_alarm_entry_add(&newconf)) < 0) {
                if (rc == RMON_ERROR_ALARM_TABLE_FULL) {
                    char *err = "RMON alarm table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("rmon_mgmt_alarm_entry_add(%u): failed", idx);
                }
            }
            idx++;
            sprintf(buf, "new_id_%d", idx);
        }
        redirect(p, "/rmon_alarm_config.htm");


    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <id>/<var_name>
        */
        memset(&conf, 0, sizeof(conf));
        while (rmon_mgmt_alarm_entry_get(&conf, TRUE) == VTSS_OK) {

            if (count++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            iport = conf.var_name.objid[conf.var_name.length - 1];
            variable = conf.var_name.objid[conf.var_name.length - 2];
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u.%u/%d/%u/%d/%d/%u/%d/%u",
                          conf.id, conf.interval, variable, iport, conf.sample_type, conf.value,
                          conf.startup_type, conf.rising_threshold, conf.rising_event_index,
                          conf.falling_threshold, conf.falling_event_index);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();

    }
    return -1;

}
static cyg_int32 handler_rmon_alarm_status(CYG_HTTPD_STATE *p)
{
    vtss_alarm_ctrl_entry_t entry;
    int                     ct;
    int                     count = 0;
    int                     num_of_entries = 0;
    int                     nextEntry = 0;
    int                     startCtrlId = 0;
    vtss_port_no_t          iport;
    ulong                   variable;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(&entry, 0, sizeof(entry));

    /*  Format: <StartCtrlId>,<NumberOfEntries>,<index>/<data_id>/<sample_start>/<drop>/<octets>/<Pkts><broadcast>/<multicast>/<crc>/
                <undersize>/<oversize>/<frag>/<jabb>/<coll>/<utilization>|... */

    /* Get number of entries per page */
    (void) cyg_httpd_form_varable_int(p, "DynNumberOfEntries", &num_of_entries);

    if (num_of_entries <= 0 || num_of_entries > 99) {
        num_of_entries = 20;
    }

    (void) cyg_httpd_form_varable_int(p, "StartCtrlId", &startCtrlId);

    (void) cyg_httpd_form_varable_int(p, "GetNextEntry", &nextEntry);

    if (nextEntry) {
        entry.id = startCtrlId;
    } else {
        entry.id = !startCtrlId ? 0 : startCtrlId - 1;
    }
    (void)cyg_httpd_start_chunked("html");
    while (rmon_mgmt_alarm_entry_get(&entry, TRUE) == VTSS_OK ) {
        /* Output the counters */

        if (count++) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        } else if (nextEntry) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%d,", entry.id, num_of_entries);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,", startCtrlId, num_of_entries);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        iport = entry.var_name.objid[entry.var_name.length - 1];
        variable = entry.var_name.objid[entry.var_name.length - 2];
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u.%u/%d/%d/%d/%d/%u/%d/%u",
                      entry.id, entry.interval, variable, iport, entry.sample_type, entry.value,
                      entry.startup_type, entry.rising_threshold, entry.rising_event_index,
                      entry.falling_threshold, entry.falling_event_index);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        if (count == num_of_entries) {
            goto end_chunked;
        }

    }
    if (!count) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%s", startCtrlId, num_of_entries, "NoEntries");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

    }

end_chunked:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_rmon_alarm_detail(CYG_HTTPD_STATE *p)
{
    vtss_alarm_ctrl_entry_t entry, traverse;
    int                     ct, val;
    vtss_rmon_ctrl_id_t     ctrl_index = 0;
    vtss_port_no_t          iport;
    ulong                   variable;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(&entry, 0, sizeof(entry));

    (void)cyg_httpd_start_chunked("html");

    if (cyg_httpd_form_varable_int(p, "index", &val)) {
        ctrl_index = (vtss_rmon_ctrl_id_t)val;
    }

    /* all of the entries index     */
    memset(&entry, 0, sizeof(entry));
    memset(&traverse, 0, sizeof(traverse));

    /*  Get the data entry  */
    entry.id = ctrl_index;
    if (rmon_mgmt_alarm_entry_get(&entry, FALSE) != VTSS_OK) {
        goto out;    /* Most likely stack error - bail out */
    }

    while (!rmon_mgmt_alarm_entry_get(&traverse, TRUE)) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", traverse.id );
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    iport = entry.var_name.objid[entry.var_name.length - 1];
    variable = entry.var_name.objid[entry.var_name.length - 2];
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u.%u/%d/%u/%d/%u/%u/%u/%u",
                  entry.id, entry.interval, variable, iport, entry.sample_type, entry.value,
                  entry.startup_type, entry.rising_threshold, entry.rising_event_index,
                  entry.falling_threshold, entry.falling_event_index);

    (void)cyg_httpd_write_chunked(p->outbuffer, ct);


out:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}


static cyg_int32 handler_rmon_event_config(CYG_HTTPD_STATE *p)
{
    int                     ct, val;
    vtss_event_ctrl_entry_t conf, newconf;
    ulong                   idx = 0, change_flag;
    const char              *var_string;
    size_t                  len = 64 * sizeof(char);
    char                    buf[64] ;
    vtss_rc                 rc;
    char                    encoded_string[3 * 64];
    int                     iport;
    char                    strbuf[129];
    int                     count = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(strbuf, 0, sizeof(strbuf));
    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        memset(&conf, 0, sizeof(conf));
        while (rmon_mgmt_event_entry_get(&conf, TRUE) == VTSS_OK) {
            newconf = conf;
            change_flag  = 0;

            /* delete entry */
            sprintf(buf, "del_%d", conf.id);
            if (cgi_escape(buf, encoded_string) == 0) {
                continue;
            }
            ( void ) cyg_httpd_form_varable_string(p, encoded_string, &len);
            if (len > 0) {
//                change_flag = del_flag = 1;
                (void)rmon_mgmt_event_entry_del(&newconf);
            } else {
                //desc
                sprintf(buf, "desc_%d", conf.id);
                var_string = cyg_httpd_form_varable_string(p, buf, &len);
                if (cgi_unescape(var_string, strbuf, len, sizeof(strbuf)) == FALSE) {
                    continue;
                }
                newconf.event_description = VTSS_STRDUP(strbuf);
                if ( !newconf.event_description) {
                    T_E("out of memeoy");
                    redirect(p, "/rmon_event_config.htm");
                    return -1;
                }
                if ( (conf.event_description && newconf.event_description && strcmp(newconf.event_description, conf.event_description)) ||
                     (conf.event_description && !newconf.event_description) ||
                     (!conf.event_description && newconf.event_description)) {
                    change_flag = 1;
                }
                //type
                sprintf(buf, "type_%d", conf.id);
                (void) cyg_httpd_form_varable_int(p, buf, &val);
                newconf.event_type = val;

                if (newconf.event_type != conf.event_type) {
                    change_flag = 1;
                }
                //community
                sprintf(buf, "community_%d", conf.id);
                var_string = cyg_httpd_form_varable_string(p, buf, &len);
                if (cgi_unescape(var_string, strbuf, len, sizeof(strbuf)) == FALSE) {
                    continue;
                }
                newconf.event_community = VTSS_STRDUP(strbuf);
                if ( !newconf.event_community) {
                    T_E("out of memeoy");
                    redirect(p, "/rmon_event_config.htm");
                    return -1;
                }
                if ( (newconf.event_community && conf.event_community && strcmp(newconf.event_community, conf.event_community)) ||
                     (!newconf.event_community && conf.event_community) ||
                     (newconf.event_community && !conf.event_community)) {
                    change_flag = 1;
                }

            }

            if (change_flag) {
                T_D("Calling rmon_mgmt_event_entry_add(%u)", newconf.id);
                if (rmon_mgmt_event_entry_add(&newconf) < 0) {
                    T_E("rmon_mgmt_event_entry_add(%u): failed", newconf.id);
                }
            }
        }

        /* new entry */
        idx = 1;
        sprintf(buf, "new_id_%d", idx);
        while (cyg_httpd_form_varable_int(p, buf, &iport)  > 0) {
            memset(&newconf, 0x0, sizeof(newconf));
            //community
            newconf.id = iport;

            //desc
            sprintf(buf, "new_desc_%d", idx);
            var_string = cyg_httpd_form_varable_string(p, buf, &len);
            if (len > 0) {
                if (cgi_unescape(var_string, strbuf, len, sizeof(strbuf)) == FALSE) {
                    continue;
                }
                newconf.event_description = VTSS_STRDUP(strbuf);
                if (!newconf.event_description) {
                    T_E("out of memory");
                    redirect(p, "/rmon_event_config.htm");
                    return -1;
                }
            }
            //type
            sprintf(buf, "new_type_%d", idx);
            (void) cyg_httpd_form_varable_int(p, buf, &val);
            newconf.event_type = val;

            //community
            sprintf(buf, "new_community_%d", idx);
            var_string = cyg_httpd_form_varable_string(p, buf, &len);
            if (len > 0) {
                if (cgi_unescape(var_string, strbuf, len, sizeof(strbuf)) == FALSE) {
                    continue;
                }
                newconf.event_community = VTSS_STRDUP(strbuf);
                if (!newconf.event_community) {
                    T_E("out of memeoy");
                    redirect(p, "/rmon_event_config.htm");
                    return -1;
                }
            }

            T_D("Calling rmon_mgmt_event_entry_add(%u)", idx);
            if ((rc = rmon_mgmt_event_entry_add(&newconf)) < 0) {
                if (rc == RMON_ERROR_EVENT_TABLE_FULL) {
                    char *err = "RMON event table is full";
                    send_custom_error(p, err, err, strlen(err));
                    return -1; // Do not further search the file system.
                } else if (rc != VTSS_OK) {
                    T_E("rmon_mgmt_event_entry_add(%u): failed", idx);
                }
            }
            idx++;
            sprintf(buf, "new_id_%d", idx);
        }

        redirect(p, "/rmon_event_config.htm");


    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <id>/<data_source>
        */
        memset(&conf, 0, sizeof(conf));
        while (rmon_mgmt_event_entry_get(&conf, TRUE) == VTSS_OK) {

            if (count++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%s/%d/%s/%lu",
                          conf.id, (!conf.event_description) ? "" : conf.event_description,
                          conf.event_type, (!conf.event_community) ? "" : conf.event_community,
                          conf.event_last_time_sent / 100 );
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void)cyg_httpd_end_chunked();

    }
    return -1;

}

static cyg_int32 handler_rmon_event_status(CYG_HTTPD_STATE *p)
{
    vtss_event_ctrl_entry_t entry;
    vtss_event_data_entry_t data;
    int                     ct;
    int                     count = 0;
    int                     num_of_entries = 0;
    int                     nextEntry = 0;
    int                     startCtrlId = 0, startDataId = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(&entry, 0, sizeof(entry));
    memset(&data, 0, sizeof(data));

    /* Format: <StartCtrlId>#<StartDataId>#<NumberOfEntries>#<index>/<data_id>/<logTime>/<logDescription>|... */

    /* Get number of entries per page */
    (void) cyg_httpd_form_varable_int(p, "DynNumberOfEntries", &num_of_entries);

    if (num_of_entries <= 0 || num_of_entries > 99) {
        num_of_entries = 20;
    }

    (void) cyg_httpd_form_varable_int(p, "StartCtrlId", &startCtrlId);

    (void) cyg_httpd_form_varable_int(p, "StartDataId", &startDataId);

    (void) cyg_httpd_form_varable_int(p, "GetNextEntry", &nextEntry);


    (void)cyg_httpd_start_chunked("html");

    if (nextEntry) {
        data.data_index = startDataId;
    } else {
        data.data_index = !startDataId ? 0 : startDataId - 1;
    }
    entry.id = !startCtrlId ? 0 : startCtrlId - 1;
    while (rmon_mgmt_event_entry_get(&entry, TRUE) == VTSS_OK ) {
        while (!rmon_mgmt_event_data_get(entry.id, &data, TRUE)) {
            /* Output the counters */

            if (count++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else if (nextEntry) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#%lu#%d#", entry.id, data.data_index, num_of_entries);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d#%d#%d#", startCtrlId, startDataId, num_of_entries);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%ld/%lu/%s",
                          entry.id,
                          data.data_index,
                          data.log_time / 100,
                          data.log_description
                         );
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            if (count == num_of_entries) {
                goto end_chunked;
            }

        }
        memset(&data, 0, sizeof(data));
    }
    if (!count) {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d#%d#%d#%s", startCtrlId, startDataId, num_of_entries, "NoEntries");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

    }

end_chunked:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_rmon_event_detail(CYG_HTTPD_STATE *p)
{
    vtss_event_ctrl_entry_t entry;
    vtss_event_data_entry_t data, traverse;
    int                     ct, val;
    vtss_rmon_ctrl_id_t     ctrl_index = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_SNMP)) {
        return -1;
    }
#endif

    memset(&entry, 0, sizeof(entry));
    memset(&data, 0, sizeof(data));
    memset(&traverse, 0, sizeof(traverse));

    (void)cyg_httpd_start_chunked("html");

    if (cyg_httpd_form_varable_int(p, "index", &val)) {
        ctrl_index = (vtss_rmon_ctrl_id_t)val;
    }

    if (cyg_httpd_form_varable_int(p, "dataId", &val)) {
        data.data_index = (vtss_rmon_ctrl_id_t)val;
    }

    /*  Get the data entry  */
    if (rmon_mgmt_event_data_get(ctrl_index, &data, FALSE) != VTSS_OK) {
        goto out;    /* Most likely stack error - bail out */
    }

    /* all of the entries index     */
    memset(&entry, 0, sizeof(entry));
    while (!rmon_mgmt_event_entry_get(&entry, TRUE)) {
        while (!rmon_mgmt_event_data_get(entry.id, &traverse, TRUE)) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%ld#", entry.id, traverse.data_index);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }

    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%ld/%lu/%s",
                  ctrl_index,
                  data.data_index,
                  data.log_time / 100,
                  data.log_description
                 );
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);


out:
    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}




/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

#define SNMP_WEB_BUF_LEN 512
static size_t rmon_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[SNMP_WEB_BUF_LEN];
    (void) snprintf(buff, SNMP_WEB_BUF_LEN,
                    "function configIndexName(index, long) {\n"
                    " var indexname = String(index);\n"
                    " if(long) indexname = \"ID \" + indexname;\n"
                    " return indexname;\n"
                    "}\n\n"
                    "var if_switch_interval = %d;\n"
                    "var if_llag_start = %d;\n"
                    "var if_glag_start = %d;\n"
                    "var if_llag_cnt = %d;\n"
                    "var if_glag_cnt = %d;\n",
                    DATASOURCE_IFINDEX_SWITCH_INTERVAL,
                    rmon_mgmt_llag_start_valid_get(),
                    rmon_mgmt_ifIndex_glag_start_valid_get(),
                    AGGR_LLAG_CNT,
                    AGGR_GLAG_CNT
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(rmon_lib_config_js);


CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_stats_config, "/config/rmon_statistics_config", handler_rmon_stats_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_stats_status, "/stat/rmon_statistics_status", handler_rmon_stats_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_stats_detail, "/stat/rmon_statistics_detail", handler_rmon_stats_detail);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_history_config, "/config/rmon_history_config", handler_rmon_history_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_history_status, "/stat/rmon_history_status", handler_rmon_history_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_history_detail, "/stat/rmon_history_detail", handler_rmon_history_detail);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_alarm_config, "/config/rmon_alarm_config", handler_rmon_alarm_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_alarm_status, "/stat/rmon_alarm_status", handler_rmon_alarm_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_alarm_detail, "/stat/rmon_alarm_detail", handler_rmon_alarm_detail);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_event_config, "/config/rmon_event_config", handler_rmon_event_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_event_status, "/stat/rmon_event_status", handler_rmon_event_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_rmon_event_detail, "/stat/rmon_event_detail", handler_rmon_event_detail);

