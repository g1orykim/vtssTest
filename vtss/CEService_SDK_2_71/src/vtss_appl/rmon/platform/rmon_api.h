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
#ifndef _VTSS_RMON_API_H_
#define _VTSS_RMON_API_H_

#include <ucd-snmp/config.h>
//#include "rmon_row_api.h"
#include "rmon_agutil_api.h"
#include "rmon.h"

#ifdef VTSS_SW_OPTION_SNMP
#include "ifIndex_api.h"
#include "rfc1213_mib2.h"


#define DATASOURCE_MGMT_PORTID_START IFTABLE_MGMT_PORTID_START
#define DATASOURCE_IFINDEX_SWITCH_INTERVAL IFTABLE_IFINDEX_SWITCH_INTERVAL
#define DATASOURCE_IFINDEX_GLAG_START IFTABLE_IFINDEX_GLAG_START
#define DATASOURCE_IFINDEX_GLAG_END        IFTABLE_IFINDEX_GLAG_END
#define DATASOURCE_IFINDEX_IP_START        IFTABLE_IFINDEX_IP_START
#define DATASOURCE_IFINDEX_IP_END          IFTABLE_IFINDEX_IP_END
#define DATASOURCE_IFINDEX_LPOAG_START IFTABLE_IFINDEX_LPOAG_START
#define DATASOURCE_IFINDEX_LPOAG_END IFTABLE_IFINDEX_LPOAG_END
#define DATASOURCE_IFINDEX_END             IFTABLE_IFINDEX_END

#define DATASOURCE_PORTID_START            IFTABLE_PORTID_START
#define DATASOURCE_PORTID_END            IFTABLE_PORTID_END

#define DATASOURCE_LLAGID_START IFTABLE_LLAGID_START
#define DATASOURCE_LLAGID_END IFTABLE_LLAGID_END
#define DATASOURCE_IPID_MAX                IFTABLE_IPID_MAX
typedef iftable_info_t datasourceTable_info_t;

#else
#include "vlan_api.h"
#include "port_api.h"
#ifdef VTSS_SW_OPTION_AGGR
#include "aggr_api.h"
#endif


#define DATASOURCE_MGMT_PORTID_START   (1 - VTSS_PORT_NO_START)
#define DATASOURCE_IFINDEX_SWITCH_INTERVAL 1000
#define DATASOURCE_IFINDEX_GLAG_START      16000
#define DATASOURCE_IFINDEX_GLAG_END        16999
#define DATASOURCE_IFINDEX_IP_START        22000
#define DATASOURCE_IFINDEX_IP_END          22999
#define DATASOURCE_IFINDEX_LPOAG_START 0
#define DATASOURCE_IFINDEX_LPOAG_END 15999
#define DATASOURCE_IFINDEX_END             23000
#define DATASOURCE_PORTID_START            1
#if VTSS_SWITCH_STACKABLE
#define DATASOURCE_PORTID_END              (DATASOURCE_PORTID_START + VTSS_PORTS - 2)
#else
#define DATASOURCE_PORTID_END              (DATASOURCE_PORTID_START + VTSS_PORTS)
#endif /* VTSS_SWITCH_STACKABLE */


#ifdef VTSS_SW_OPTION_AGGR
#define DATASOURCE_LLAGID_START DATASOURCE_PORTID_END
#define DATASOURCE_LLAGID_END (DATASOURCE_LLAGID_START + AGGR_MGMT_GROUP_NO_END - AGGR_MGMT_GROUP_NO_START)
#endif


#define DATASOURCE_IPID_MAX                1

typedef struct {
    u_long               type;
    vtss_isid_t          isid;
    vtss_port_no_t       port_no;
#ifdef VTSS_SW_OPTION_AGGR
    aggr_mgmt_group_no_t aggr_no;
#endif
    vtss_vid_t           vid;
    u_long               ipid;
} datasourceTable_info_t;

#endif

#define DATASOURCE_GLAGID_MAX              (DATASOURCE_IFINDEX_GLAG_START + VTSS_GLAGS)

#if 1
/* SNMP RMON row entry max size */
#define SNMP_RMON_STAT_MAX_ROW_SIZE         128
#define SNMP_RMON_HISTORY_MAX_ROW_SIZE      256
#define SNMP_RMON_ALARM_MAX_ROW_SIZE        256
#define SNMP_RMON_EVENT_MAX_ROW_SIZE        128
#endif

/* Added by SGZ, 05/26  */
#define RMON_MAX_ENTRIES                   65535
#define RMON_HISTORY_MIN_INTERVAL          1
#define RMON_HISTORY_MAX_INTERVAL          3600
#define RMON_BUCKETS_REQ_MAX               65535
#define RMON_ALARM_MIN_INTERVAL            1
#define RMON_ALARM_MAX_INTERVAL            2147483647
#define RMON_ALARM_MIN_THRESHOLD           1
#define RMON_ALARM_MAX_THRESHOLD           2147483647
#define SNMPV3_MIN_EVENT_DESC_LEN               1
#define SNMPV3_MAX_EVENT_DESC_LEN               127
#define SNMPV3_MIN_EVENT_COMMUNITY_LEN          1
#define SNMPV3_MAX_EVENT_COMMUNITY_LEN          127
#define RMON_ID_MIN                        1
#define RMON_ID_MAX                        RMON_MAX_ENTRIES

#define RMON_MAX_OID_LEN               128
#define RMON_MAX_SUBTREE_LEN           16

typedef enum {
    ALARM_INOCTETS     = 10,
    ALARM_INUCASTPKTS  ,
    ALARM_INNUCASTPKTS ,
    ALARM_INDISCARDS   ,
    ALARM_INERRORS     ,
    ALARM_INUNKOWNPROTOS,
    ALARM_OUTOCTETS    ,
    ALARM_OUTUCASTPKTS ,
    ALARM_OUTNuCASTPKTS,
    ALARM_OUTDISCARDS  ,
    ALARM_OUTERRORS    ,
    ALARM_OUTQlEN
} vtss_rmon_alarm_var_t;

/* Added by SGZ end, 05/26  */

enum {
    DATASOURCE_IFINDEX_TYPE_PORT,
    DATASOURCE_IFINDEX_TYPE_LLAG,
    DATASOURCE_IFINDEX_TYPE_GLAG,
    DATASOURCE_IFINDEX_TYPE_VLAN,
    DATASOURCE_IFINDEX_TYPE_IP,
    DATASOURCE_IFINDEX_TYPE_UNDEF
};



#define DATASOURCE_IS_VALID(ifindex) ((ifindex) < DATASOURCE_IFINDEX_END)
#define DATASOURCE_IS_LPOAG(ifindex) (((ifindex) >= DATASOURCE_IFINDEX_LPOAG_START) && ((ifindex) <= DATASOURCE_IFINDEX_LPOAG_END))
#define DATASOURCE_IS_GLAG(ifindex) (((ifindex) >= DATASOURCE_IFINDEX_GLAG_START) && ((ifindex) <= DATASOURCE_IFINDEX_GLAG_END))
#define DATASOURCE_IS_IP(ifindex) (((ifindex) >= DATASOURCE_IFINDEX_IP_START) && ((ifindex) <= DATASOURCE_IFINDEX_IP_END))


typedef enum {
    RMON_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_RMON),  /* Generic error code */
    RMON_ERROR_STAT_TABLE_FULL,        /* RMON statistics table full */
    RMON_ERROR_HISTORY_TABLE_FULL,     /* RMON history table full */
    RMON_ERROR_ALARM_TABLE_FULL,       /* RMON alarm table full */
    RMON_ERROR_EVENT_TABLE_FULL,       /* RMON event table full */
    RMON_ERROR_STAT_ENTRY_NOT_FOUND,   /* RMON statistics entry not found */
    RMON_ERROR_HISTORY_ENTRY_NOT_FOUND,/* RMON history entry not found */
    RMON_ERROR_ALARM_ENTRY_NOT_FOUND,  /* RMON alarm entry not found */
    RMON_ERROR_EVENT_ENTRY_NOT_FOUND,  /* RMON event entry not found */
    RMON_ERROR_STACK_STATE,                 /* Illegal MASTER/SLAVE state */
} rmon_error_t;


BOOL get_datasource_info(int if_index, datasourceTable_info_t *table_info_p);
/* Get SNMP RMON statistics row entry */
vtss_rc snmp_mgmt_rmon_stat_entry_get(rmon_stat_entry_t *entry, BOOL next);

/* Set SNMP RMON statistics row entry */
vtss_rc snmp_mgmt_rmon_stat_entry_set(rmon_stat_entry_t *entry);

/* Get SNMP RMON history row entry */
vtss_rc snmp_mgmt_rmon_history_entry_get(rmon_history_entry_t *entry, BOOL next);

/* Set SNMP RMON history row entry */
vtss_rc snmp_mgmt_rmon_history_entry_set(rmon_history_entry_t *entry);

/* Get SNMP RMON alarm row entry */
vtss_rc snmp_mgmt_rmon_alarm_entry_get(rmon_alarm_entry_t *entry, BOOL next);

/* Set SNMP RMON alarm row entry */
vtss_rc snmp_mgmt_rmon_alarm_entry_set(rmon_alarm_entry_t *entry);

/* Get SNMP RMON event row entry */
vtss_rc snmp_mgmt_rmon_event_entry_get(rmon_event_entry_t *entry, BOOL next);
/* Set SNMP RMON event row entry */
vtss_rc snmp_mgmt_rmon_event_entry_set(rmon_event_entry_t *entry);

void            rmon_create_stat_default_entry(void);
BOOL get_etherStatsTable_entry(VAR_OID_T *data_source, ETH_STATS_T *table_entry_p);

void            rmon_create_history_default_entry(void);

void            rmon_create_alarm_default_entry(void);
void            rmon_create_event_default_entry(void);
void            event_send_trap(u_char is_rising,
                                u_int alarm_index,
                                u_int value,
                                u_int the_threshold,
                                oid *alarmed_var,
                                size_t alarmed_var_length,
                                u_int sample_type);

void            init_rmon_statistics(void);
void            init_rmon_history(void);
void            init_rmon_alarm(void);
void            init_rmon_event(void);
/* Initialize module */
vtss_rc rmon_init(vtss_init_data_t *data);


RMON_ENTRY_T   *RMON_header_ControlEntry(struct variable *vp, oid *name,
                                         size_t *length, int exact,
                                         size_t *var_len,
                                         RMON_TABLE_INDEX_T table_index,
                                         void *entry_ptr,
                                         size_t entry_size);

RMON_ENTRY_T   *RMON_header_DataEntry(struct variable *vp,
                                      oid *name, size_t *length,
                                      int exact, size_t *var_len,
                                      RMON_TABLE_INDEX_T table_index,
                                      size_t data_size,
                                      void *entry_ptr);

int             RMON_do_another_action(oid *name,
                                       int tbl_first_index_begin,
                                       int action, int *prev_action,
                                       RMON_TABLE_INDEX_T table_index,
                                       size_t entry_size);

RMON_ENTRY_T   *RMON_find(RMON_TABLE_INDEX_T table_index,
                          u_long ctrl_index);

RMON_ENTRY_T   *RMON_next(RMON_TABLE_INDEX_T table_index,
                          u_long ctrl_index);

void            RMON_delete_clone(RMON_TABLE_INDEX_T table_index,
                                  u_long ctrl_index);

/* peter, 2007/8, modify for dynamic row entry delete */
void RMON_delete(RMON_ENTRY_T *eold);

int             RMON_new(RMON_TABLE_INDEX_T table_index,
                         u_long ctrl_index);

int             RMON_commit(RMON_TABLE_INDEX_T table_index,
                            u_long ctrl_index);

#if 0
void            RMON_init_table(RMON_TABLE_INDEX_T table_index,
                                char *name,
                                u_long max_number_of_entries,
                                ENTRY_CALLBACK_T *ClbkCreate,
                                ENTRY_CALLBACK_T *ClbkClone,
                                ENTRY_CALLBACK_T *ClbkDelete,
                                ENTRY_CALLBACK_T *ClbkValidate,
                                ENTRY_CALLBACK_T *ClbkActivate,
                                ENTRY_CALLBACK_T *ClbkDeactivate,
                                ENTRY_CALLBACK_T *ClbkCopy);
#endif


void           *RMON_locate_new_data(SCROLLER_T *scrlr);
void            RMON_descructor(SCROLLER_T *scrlr);
void
RMON_set_size(SCROLLER_T *scrlr,
              u_long data_requested,
              u_char do_allocation);

u_long          RMON_get_total_number(SCROLLER_T *scrlr);

int             RMON_data_init(SCROLLER_T *scrlr,
                               u_long max_number_of_entries,
                               u_long data_requested,
                               size_t data_size,
                               int (*data_destructor) (struct
                                                       data_scroller *,
                                                       void *));


void RMON_init_stats_table(void);
void RMON_init_history_table(void);
void RMON_init_alarm_table(void);
void RMON_init_event_table(void);

BOOL rmon_etherStatsTable_entry_update(vtss_var_oid_t *data_source, vtss_eth_stats_t *table_entry_p);


/**
  * \brief Add/Set the statistics entry.
  *
  * \param entry [IN]:  the entry that need to be added in the table\n
  *                         entry->id: the entry ID\n
  *                     entry->data_source: the ifIndex of ifEntry
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *     SNMP_ERROR_RMON_STAT_TABLE_FULL if table is full
  */
vtss_rc rmon_mgmt_statistics_entry_add (vtss_stat_ctrl_entry_t *entry);

/**
  * \brief Delete the statistics entry.
  *
  * \param entry [IN]:  the entry that need to be deleted from the table\n
  *                         entry->id: the entry ID
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  */
vtss_rc rmon_mgmt_statistics_entry_del (vtss_stat_ctrl_entry_t *entry);
/**
  * \brief Get the statistics entry.
  *
  * \param entry [INOUT]:   the entry that need to be got from the table\n
  *                         entry->id [INOUT]: the entry ID got from the table or return the next entry when next parameter is TRUE
  * \param next [IN]: Next or specific entry
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *    SNMP_ERROR_RMON_STAT_ENTRY_NOT_FOUND if entry not found.\n
  */
vtss_rc rmon_mgmt_statistics_entry_get (vtss_stat_ctrl_entry_t *entry, BOOL next);

/**
  * \brief Add/Set the history control entry.
  *
  * \param entry [IN]:  the entry that need to be added in the table\n
  *                         entry->id: the entry ID\n
  *                     entry->data_source: the ifIndex of ifEntry\n
  *                         entry->interval: the sampling interval\n
  *                         entry->data_request: the maximum data entries stored in data table
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *    SNMP_ERROR_RMON_STAT_TABLE_FULL if table is full
  */
vtss_rc
rmon_mgmt_history_entry_add (vtss_history_ctrl_entry_t *entry);

/**
  * \brief Delete the history control entry.
  *
  * \param entry [IN]:  the entry that need to be deleted from the table\n
  *                         entry->id: the entry ID
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  */
vtss_rc rmon_mgmt_history_entry_del ( vtss_history_ctrl_entry_t *entry);

/**
  * \brief Get the history control entry.
  *
  * \param entry [INOUT]:   the entry that need to be got from the table\n
  *                         entry->id [INOUT]: the entry ID got from the table or return the next entry when next parameter is TRUE
  * \param next [IN]: Next or specific entry
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *    SNMP_ERROR_RMON_STAT_ENTRY_NOT_FOUND if entry not found.\n
  */
vtss_rc rmon_mgmt_history_entry_get ( vtss_history_ctrl_entry_t *entry, BOOL next );

/**
  * \brief Get the history data entry.
  *
  * \param entry [INOUT]:   the entry that need to be got from the table\n
  *                         entry->ctrl_index[INOUT]: the control entry ID\n
  *                         entry->data_index[INOUT]: the data entry ID
  * \param next [IN]: Next or specific entry
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *    SNMP_ERROR_RMON_STAT_ENTRY_NOT_FOUND if entry not found.\n
  */
#if 1
vtss_rc rmon_mgmt_history_data_get ( ulong ctrl_index, vtss_history_data_entry_t *entry, BOOL next );
#else
vtss_rc rmon_mgmt_history_data_get ( vtss_history_data_entry_t *entry, BOOL next );
#endif



/**
  * \brief Add/Set the alarm entry.
  *
  * \param entry [IN]:  the entry that need to be added in the table\n
  *                     entry->id [IN]: the entry index in the table\n
  *                     entry->alarm_variable [IN]: the ifIndex of ifEntry\n
  *                     entry->interval [IN]: the sampling interval\n
  *                     entry->sample_type [IN]: the sampling interval\n
  *                     entry->rising_threshold [IN]: the maximum data entries stored in data table\n
  *                     entry->rising_event_index [IN]: the maximum data entries stored in data table\n
  *                     entry->falling_threshold [IN]: the maximum data entries stored in data table\n
  *                     entry->falling_event_index [IN]: the maximum data entries stored in data table\n
  *                     entry->startup_type [IN]: the maximum data entries stored in data table\n
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *     SNMP_ERROR_RMON_STAT_TABLE_FULL if table is full
  */
vtss_rc
rmon_mgmt_alarm_entry_add ( vtss_alarm_ctrl_entry_t *entry );

/**
  * \brief Delete the alarm entry.
  *
  * \brief Delete the history control entry.
  *
  * \param entry [IN]:  the entry that need to be deleted from the table\n
  *                         entry->id [IN]: the entry ID
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  */
vtss_rc
rmon_mgmt_alarm_entry_del ( vtss_alarm_ctrl_entry_t *entry );
/**
  * \brief Get the alarm entry.
  *
  * \param entry [INOUT]:   the entry that need to be got from the table\n
  *                         entry->id [INOUT]: the entry ID got from the table or return the next entry when next parameter is TRUE
  * \param next [IN]: Next or specific entry
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *    SNMP_ERROR_RMON_STAT_ENTRY_NOT_FOUND if entry not found.\n
  */
vtss_rc
rmon_mgmt_alarm_entry_get ( vtss_alarm_ctrl_entry_t *entry, BOOL next );


/**
  * \brief Add/Set the event control entry.
  *
  * \param entry [IN]:  the entry that need to be added in the table\n
  *                     entry->id : the entry index in the table\n
  *                     entry->event_type : RMON event stored type\n
  *                     entry->event_community : the community when sending trap\n
  *                     entry->event_description : event description\n
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *     SNMP_ERROR_RMON_STAT_TABLE_FULL if table is full
  */
vtss_rc
rmon_mgmt_event_entry_add ( vtss_event_ctrl_entry_t *entry );

/**
  * \brief Delete the event control entry.
  *
  * \param entry [IN]:  the entry that need to be deleted from the table\n
  *                         entry->id [IN]: the entry ID
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  */
vtss_rc
rmon_mgmt_event_entry_del ( vtss_event_ctrl_entry_t *entry );
/**
  * \brief Get the event control entry.
  *
  * \param entry [INOUT]:   the entry that need to be got from the table\n
  *                         entry->id [INOUT]: the entry ID got from the table or return the next entry when next parameter is TRUE
  * \param next [IN]: Next or specific entry
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  *    SNMP_ERROR_RMON_STAT_ENTRY_NOT_FOUND if entry not found.\n
  */
vtss_rc
rmon_mgmt_event_entry_get ( vtss_event_ctrl_entry_t *entry, BOOL next );

/**
  * \brief Get the event data entry.
  *
  * \param entry [INOUT]:   the entry that need to be got from the table\n
  *                         entry->ctrl_index[INOUT]: the control entry ID\n
  *                         entry->data_index[INOUT]: the data entry ID
  * \param next [IN]: Next or specific entry
  *
  * \return
  *    VTSS_OK on success.\n
  *    SNMP_ERROR_PARAM if parameters error.\n
  */

#if 1
vtss_rc rmon_mgmt_event_data_get ( ulong ctrl_index, vtss_event_data_entry_t *entry, BOOL next );
#else
vtss_rc rmon_mgmt_event_data_get ( vtss_history_data_entry_t *entry, BOOL next );
#endif

/**
  * \brief Get the start ifIndex of LLAG.
  *
  * \return
  *    the start ifIndex of LLAG\n
  */

int rmon_mgmt_llag_start_valid_get ( void );

/**
  * \brief Get the start ifIndex of GLAG.
  *
  * \return
  *    the start ifIndex of GLAG\n
  */
int rmon_mgmt_ifIndex_glag_start_valid_get ( void );

#ifndef VTSS_SW_OPTION_SNMP
/* Convert text (e.g. .1.2.*.3) to OID */
vtss_rc mgmt_txt2oid(char *buf, int len,
                     ulong *oid, uchar *oid_mask, ulong *oid_len);
#endif



#endif

