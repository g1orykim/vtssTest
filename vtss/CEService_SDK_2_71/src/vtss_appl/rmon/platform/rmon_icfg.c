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
/*
******************************************************************************

    Revision history
    > CP.Wang, 2012/09/24 11:22
        - create

******************************************************************************
*/

/*
******************************************************************************

    Include files

******************************************************************************
*/
#include <stdlib.h>
#include "icfg_api.h"
#include "rmon_api.h"
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "ifIndex_api.h"
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_RMON

#define VTSS_TRACE_GRP_DEFAULT  0
#define TRACE_GRP_CRIT          1

#ifndef TRACE_GRP_CNT
#define TRACE_GRP_CNT           2
#endif

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/
static char *_oid_2_str(IN oid *name, IN int name_len, OUT char *buf)
{
    int i = 0, j = 0;

    for (i = 0; i < name_len; i++) {
        sprintf(buf + j, ".%ld", name[i]);
        j = strlen(buf);
    }
    return buf;
}

/* ICFG callback functions */
static vtss_rc _rmon_icfg(
    IN const vtss_icfg_query_request_t  *req,
    IN vtss_icfg_query_result_t         *result
)
{
    vtss_rc                     rc;
    char                        buf[80];
    vtss_alarm_ctrl_entry_t     alarm_entry;
    vtss_event_ctrl_entry_t     event_entry;
    iftable_info_t              iftable_info;
    vtss_stat_ctrl_entry_t      stat_entry;
    vtss_isid_t                 isid;
    vtss_port_no_t              iport;
    vtss_history_ctrl_entry_t   history_entry;

    if ( req == NULL ) {
        T_E("req == NULL\n");
        return VTSS_RC_ERROR;
    }

    if ( result == NULL ) {
        T_E("result == NULL\n");
        return VTSS_RC_ERROR;
    }

    switch ( req->cmd_mode ) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG:
        /* rmon alarm */
        memset(&alarm_entry, 0, sizeof(alarm_entry));
        while ( rmon_mgmt_alarm_entry_get(&alarm_entry, TRUE) == VTSS_OK ) {
            rc = vtss_icfg_printf(result, "rmon alarm %u %s %u %s rising-threshold %d %u falling-threshold %d %u %s\n",
                                  alarm_entry.id,
                                  _oid_2_str(alarm_entry.var_name.objid, alarm_entry.var_name.length, buf),
                                  alarm_entry.interval,
                                  (alarm_entry.sample_type == SAMPLE_TYPE_ABSOLUTE) ? "absolute" : "delta",
                                  (i32)(alarm_entry.rising_threshold),
                                  alarm_entry.rising_event_index,
                                  (i32)(alarm_entry.falling_threshold),
                                  alarm_entry.falling_event_index,
                                  (alarm_entry.startup_type == ALARM_RISING) ? "rising" : (alarm_entry.startup_type == ALARM_FALLING) ? "falling" : "both"
                                 );
            if ( rc != VTSS_RC_OK ) {
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* rmon event */
        memset(&event_entry, 0, sizeof(event_entry));
        while ( rmon_mgmt_event_entry_get(&event_entry, TRUE) == VTSS_OK ) {
            switch ( event_entry.event_type ) {
            case EVENT_NONE:
                if ( event_entry.event_description ) {
                    rc = vtss_icfg_printf(result, "rmon event %u description %s\n",
                                          event_entry.id,
                                          event_entry.event_description
                                         );
                } else {
                    rc = vtss_icfg_printf(result, "rmon event %u\n",
                                          event_entry.id
                                         );
                }
                break;

            case EVENT_LOG:
                if ( event_entry.event_description ) {
                    rc = vtss_icfg_printf(result, "rmon event %u log description %s\n",
                                          event_entry.id,
                                          event_entry.event_description
                                         );
                } else {
                    rc = vtss_icfg_printf(result, "rmon event %u log\n",
                                          event_entry.id
                                         );
                }
                break;

            case EVENT_TRAP:
                if ( event_entry.event_description ) {
                    rc = vtss_icfg_printf(result, "rmon event %u trap %s description %s\n",
                                          event_entry.id,
                                          event_entry.event_community,
                                          event_entry.event_description
                                         );
                } else {
                    rc = vtss_icfg_printf(result, "rmon event %u trap %s\n",
                                          event_entry.id,
                                          event_entry.event_community
                                         );
                }
                break;

            case EVENT_LOG_AND_TRAP:
                if ( event_entry.event_description ) {
                    rc = vtss_icfg_printf(result, "rmon event %u log trap %s description %s\n",
                                          event_entry.id,
                                          event_entry.event_community,
                                          event_entry.event_description
                                         );
                } else {
                    rc = vtss_icfg_printf(result, "rmon event %u log trap %s\n",
                                          event_entry.id,
                                          event_entry.event_community
                                         );
                }
                break;

            default:
                T_E("invalid event type %u\n", event_entry.event_type);
                return VTSS_RC_ERROR;
            }

            if ( rc != VTSS_RC_OK ) {
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }
        break;

    case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
        /* get isid and iport */
        isid = topo_usid2isid(req->instance_id.port.usid);
        iport = uport2iport(req->instance_id.port.begin_uport);

        /* rmon collection stats */
        memset(&stat_entry, 0, sizeof(stat_entry));
        while ( rmon_mgmt_statistics_entry_get(&stat_entry, TRUE) == VTSS_OK ) {
            iftable_info.ifIndex = (ifIndex_id_t)(stat_entry.data_source.objid[10]);
            if ( ifIndex_get(&iftable_info) == FALSE ) {
                T_E("fail to get from ifIndex\n");
                return VTSS_RC_ERROR;
            }
            if ( iftable_info.type != IFTABLE_IFINDEX_TYPE_PORT ) {
                T_E("invalid ifindex type\n");
                return VTSS_RC_ERROR;
            }
            if ( iftable_info.isid == isid && iftable_info.if_id == iport ) {
                rc = vtss_icfg_printf(result, " rmon collection stats %u\n",
                                      stat_entry.id
                                     );
                if ( rc != VTSS_RC_OK ) {
                    T_E("fail to print to icfg\n");
                    return VTSS_RC_ERROR;
                }
            }
        }

        /* rmon collection history */
        memset(&history_entry, 0, sizeof(history_entry));
        while ( rmon_mgmt_history_entry_get(&history_entry, TRUE) == VTSS_OK ) {
            iftable_info.ifIndex = (ifIndex_id_t)(history_entry.data_source.objid[10]);
            if ( ifIndex_get(&iftable_info) == FALSE ) {
                T_E("fail to get from ifIndex\n");
                return VTSS_RC_ERROR;
            }
            if ( iftable_info.type != IFTABLE_IFINDEX_TYPE_PORT ) {
                T_E("invalid ifindex type\n");
                return VTSS_RC_ERROR;
            }
            if ( iftable_info.isid == isid && iftable_info.if_id == iport ) {
                rc = vtss_icfg_printf(result, " rmon collection history %u buckets %lu interval %lu\n",
                                      history_entry.id,
                                      history_entry.scrlr.data_requested,
                                      history_entry.interval
                                     );
                if ( rc != VTSS_RC_OK ) {
                    T_E("fail to print to icfg\n");
                    return VTSS_RC_ERROR;
                }
            }
        }
        break;

    default:
        /* no config in other modes */
        break;
    }
    return VTSS_OK;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc rmon_icfg_init(void)
{
    vtss_rc rc;

    /*
        Register Global config callback function to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_GLOBAL_RMON, "rmon", _rmon_icfg);
    if ( rc != VTSS_OK ) {
        return rc;
    }

    /*
        Register Interface port list callback function to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_INTERFACE_ETHERNET_RMON, "rmon", _rmon_icfg);
    return rc;
}
