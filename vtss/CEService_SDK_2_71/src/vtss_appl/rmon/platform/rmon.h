/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#ifndef _VTSS_RMON_H_
#define _VTSS_RMON_H_
#ifdef VTSS_SW_OPTION_RMON
#define EXTEND_RMON_TO_WEB_CLI
#endif /*  VTSS_SW_OPTION_RMON */
#include<rmon_row_api.h>
/* RMON Common Data Structure   */
typedef  u32  vtss_rmon_ctrl_id_t;
typedef  u32  vtss_rmon_data_id_t;

#if 0
#ifndef VTSS_TRACE_MODULE_ID
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_RMON
#endif

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1

#ifndef TRACE_GRP_CNT
#define TRACE_GRP_CNT          2
#endif
#endif

#define IF_ENTRY_INDEX_OID {1,3,6,1,2,1,2,2,1,1}
#define IF_ENTRY_INDEX_INST {1,3,6,1,2,1,2,2,1,1,0}

#define IF_ENTRY_OID {1,3,6,1,2,1,2,2,1}
#define IF_ENTRY_INST {1,3,6,1,2,1,2,2,1,0,0}

#define RMON_CONF_BLK_VERSION              1
#define RMON_STAT_CONF_BLK_VERSION         1
#define RMON_HISTORY_CONF_BLK_VERSION      1
#define RMON_ALARM_CONF_BLK_VERSION        1
#define RMON_EVENT_CONF_BLK_VERSION        1

#define MIN_historyControlBucketsRequested               1
#define MAX_historyControlBucketsRequested               65535
#define MIN_historyControlInterval                       1
#define MAX_historyControlInterval                       3600
/*
 * from MIB compilation
 */
#define MIN_alarmEventIndex             0
#define MAX_alarmEventIndex             65535
#define MIN_event_description   0
#define MAX_event_description   127
#define MIN_event_community     0
#define MAX_event_community     127
#define Leaf_event_last_time_sent 5


/* ================================================================= *
 *  RMON stack messages
 * ================================================================= */


/* RMON configuration */
#if 0
typedef struct {
} rmon_conf_t;
#endif


/* SNMP messages IDs */
typedef enum {
    RMON_MSG_ID_RMON_CONF_SET_REQ,          /* RMON configuration set request (no reply) */
} rmon_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    rmon_msg_id_t    msg_id;

#if 0
    /* Request data, depending on message ID */
    union {
        /* RMON_MSG_ID_SNMP_CONF_SET_REQ */
        struct {
            rmon_conf_t      conf;
        } conf_set;
    } req;
#endif
} rmon_msg_req_t;

/* SNMP message buffer */
typedef struct {
    vtss_os_sem_t *sem; /* Semaphore */
    void          *msg; /* Message */
} rmon_msg_buf_t;

#if 1
/* SNMP statistics row entry */
typedef struct {
    BOOL  valid;
    ulong ctrl_index;
    ulong if_index;
} rmon_stat_entry_t;

/* SNMP history row entry */
typedef struct {
    BOOL  valid;
    ulong ctrl_index;
    ulong if_index;
    ulong interval;
    ulong requested;
} rmon_history_entry_t;

/* SNMP alarm row entry */
#define SNMP_MAX_ALRAM_VARIABLE_LEN   16
#define RMON_MGMT_MAX_NAME_LEN              128

typedef struct {
    BOOL  valid;
    ulong ctrl_index;
    ulong interval;
    ulong variable[SNMP_MAX_ALRAM_VARIABLE_LEN];
    ulong variable_len;
    ulong sample_type;
    ulong startup_alarm;
    long rising_threshold;
    long falling_threshold;
    ulong rising_event_index;
    ulong falling_event_index;
} rmon_alarm_entry_t;

/* SNMP event row entry */
typedef struct {
    BOOL  valid;
    ulong ctrl_index;
    char  description[RMON_MGMT_MAX_NAME_LEN];
    uchar type;
    char community[128];
} rmon_event_entry_t;
/* SNMP event trap entry */
typedef struct {
    char  trap_community[RMON_MGMT_MAX_NAME_LEN];
    uchar is_rising;
    uint  alarm_index;
    uint  value;
    uint  the_threshold;
    ulong alarmed_var[SNMP_MAX_ALRAM_VARIABLE_LEN];
    ulong alarmed_var_length;
    uint  sample_type;
} rmon_event_trap_entry_t;

#endif

/* SNMP RMON row entry max size */
#define RMON_STAT_MAX_ROW_SIZE         128
#define RMON_HISTORY_MAX_ROW_SIZE      256
#define RMON_ALARM_MAX_ROW_SIZE        256
#define RMON_EVENT_MAX_ROW_SIZE        128


/* RMON configuration block */
typedef struct {
    unsigned long    version;       /* Block version */
//    rmon_conf_t      snmp_conf;     /* SNMP configuration */
} rmon_conf_blk_t;

/* RMON statistics configuration block */
typedef struct {
    unsigned long          version;                                           /* Block version */
    unsigned long          rmon_stat_entry_num;                          /* SNMP RMON statistics row entries number */
    rmon_stat_entry_t      rmon_stat_entry[RMON_STAT_MAX_ROW_SIZE]; /* SNMP RMON statistics row entries */
} rmon_stat_conf_blk_t;

/* RMON history configuration block */
typedef struct {
    unsigned long             version;                                                 /* Block version */
    unsigned long             rmon_history_entry_num;                             /* SNMP RMON history row entries number */
    rmon_history_entry_t      rmon_history_entry[RMON_HISTORY_MAX_ROW_SIZE]; /* SNMP RMON history row entries */
} rmon_history_conf_blk_t;

/* RMON alarm configuration block */
typedef struct {
    unsigned long           version;                                                /* Block version */
    unsigned long           rmon_alarm_entry_num;                              /* SNMP RMON alarm row entries number */
    rmon_alarm_entry_t      rmon_alarm_entry[RMON_ALARM_MAX_ROW_SIZE];    /* SNMP RMON alarm row entries */
} rmon_alarm_conf_blk_t;

/* RMON event configuration block */
typedef struct {
    unsigned long           version;                                                /* Block version */
    unsigned long           rmon_event_entry_num;                              /* SNMP RMON event row entries number */
    rmon_event_entry_t      rmon_event_entry[RMON_EVENT_MAX_ROW_SIZE];    /* SNMP RMON event row entries */
} rmon_event_conf_blk_t;



/* RMON global structure */
typedef struct {
    critd_t                   crit;
    unsigned long             rmon_stat_entry_num;
    rmon_stat_entry_t    rmon_stat_entry[RMON_STAT_MAX_ROW_SIZE];
    unsigned long             rmon_history_entry_num;
    rmon_history_entry_t rmon_history_entry[RMON_HISTORY_MAX_ROW_SIZE];
    unsigned long             rmon_alarm_entry_num;
    rmon_alarm_entry_t   rmon_alarm_entry[RMON_ALARM_MAX_ROW_SIZE];
    unsigned long             rmon_event_entry_num;
    rmon_event_entry_t   rmon_event_entry[RMON_EVENT_MAX_ROW_SIZE];

    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t sem;
        rmon_msg_req_t msg;
    } request;

} rmon_global_t;


#ifdef EXTEND_RMON_TO_WEB_CLI
typedef struct {
    size_t          length;
    oid             objid[MAX_OID_LEN];
//    u8             objid[MAX_OID_LEN];
} vtss_var_oid_t;

typedef enum {
    RMON_STATS_TABLE_INDEX  ,
    RMON_HISTORY_TABLE_INDEX,
    RMON_ALARM_TABLE_INDEX  ,
    RMON_EVENT_TABLE_INDEX  ,
    RMON_EVENT_TABLE_NUM
} RMON_TABLE_INDEX_T;


typedef struct {
    u32           ifIndex;
    u32          drop_events;
    u32          octets;
    u32          packets;
    u32          bcast_pkts;
    u32          mcast_pkts;
    u32          crc_align;
    u32          undersize;
    u32          oversize;
    u32          fragments;
    u32          jabbers;
    u32          collisions;
    u32          pkts_64;
    u32          pkts_65_127;
    u32          pkts_128_255;
    u32          pkts_256_511;
    u32          pkts_512_1023;
    u32          pkts_1024_1518;
} vtss_eth_stats_t;


/**
 * \brief RMON statistics control entry
 */
typedef struct {
    vtss_rmon_ctrl_id_t     id;                     /**< the entry ID. */
    vtss_var_oid_t          data_source;            /**< the ifIndex of ifEntry. */
    u32                     etherStatsCreateTime;   /**< the time when the entry is activated. */
    vtss_eth_stats_t        eth;                    /**< the statistics data    */
} vtss_stat_ctrl_entry_t;

/**
 * \brief RMON history data entry
 */
typedef struct history_data_struct_t {
    struct history_data_struct_t    *next;              /**< the next entry in the table */
#if 0
    vtss_rmon_ctrl_id_t             ctrl_index;         /**< the entry ID. */
#endif
    u_long                          data_index;         /**< data entry index */
    u_long                          start_interval;     /**< data entry startup time */
    u_long                          utilization;        /**< the untilization   */
    vtss_eth_stats_t                EthData;            /**< the statistics data    */
} vtss_history_data_entry_t;

/**
 * \brief RMON history control entry
 */
typedef struct {
    vtss_rmon_ctrl_id_t           id;               /**< the entry ID. */
    u_long               interval;      /**< sampling interval */
    u_long               timer_id;      /**< timer_id that register in alarm */
    vtss_var_oid_t       data_source;   /**< the ifIndex of ifEntry. */
    u_long               coeff;         /**< .  */
    vtss_history_data_entry_t previous_bucket; /**< previous data entry     */
    SCROLLER_T           scrlr;             /**< the associated data entry  */
} vtss_history_ctrl_entry_t;

/**
 * \brief RMON alarm sampling type
 */
typedef enum {
    SAMPLE_TYPE_ABSOLUTE =  1,      /**< absolute sampling  */
    SAMPLE_TYPE_DELTA,               /**< dalta sampling     */
    SAMPLE_TYPE_END
} vtss_alarm_sample_type_t;

/**
 * \brief RMON alarm trigger type
 */
typedef enum {
    ALARM_NOTHING = 0,              /**< none trigger   */
    ALARM_RISING,                   /**< rising trigger     */
    ALARM_FALLING,                  /**< falling trigger    */
    ALARM_BOTH,                     /**< rising or falling trigger  */
    ALARM_END
} vtss_alarm_type_t;

/**
 * \brief RMON alarm control entry
 */
typedef struct {
    vtss_rmon_ctrl_id_t id;                     /**< the entry ID. */
    u32                 interval;       /**< sampling interval */
    u32                 timer_id;       /**< timer id that register in alarm */
    vtss_var_oid_t          var_name;       /**< the source data to be compared */
    vtss_alarm_sample_type_t        sample_type; /**< the source data to be computed with absolute value or delta value */
    vtss_alarm_type_t           startup_type;      /**< trigger rising or falling alarm when startup */

    i32                 rising_threshold;       /**< rising threshold */
    i32                 falling_threshold;      /**< falling threshold */
    u32                 rising_event_index;     /**< rising event index*/
    u32                 falling_event_index;    /**< falling event index */
    u32                 last_abs_value;         /**< the last sampling absolute value */
    i32                 value;                      /**< sampling value */
    vtss_alarm_type_t       prev_alarm;         /**< previous alarm     */
} vtss_alarm_ctrl_entry_t;

/**
 * \brief RMON event log entry.
 */
typedef struct event_data_struct_t {
    struct data_struct_t *next;             /**< the next entry in the table */
#if 0
    vtss_rmon_ctrl_id_t ctrl_id;                    /**< the entry ID. */
#endif
    u_long          data_index;             /**< data entry index */
    u_long          log_time;               /**< log time   */
    char           *log_description;            /**< log descripton     */
} vtss_event_data_entry_t;

/**
 * \brief RMON event stored type.
 */
typedef enum {
    EVENT_NONE = 1,         /**< none store event */
    EVENT_LOG,              /**< store event in log table   */
    EVENT_TRAP,             /**< send event to trap destination */
    EVENT_LOG_AND_TRAP,      /**< store event in log table and send it to trap destination   */
    EVENT_END
} vtss_event_type_t;

/**
 * \brief RMON event control entry.
 */
typedef struct {
    vtss_rmon_ctrl_id_t id;                     /**< the entry ID. */
    char           *event_description;      /**< event description */
    char           *event_community;            /**< the community when sending trap */
    vtss_event_type_t event_type;               /**< RMON event stored type */
    u_long          event_last_time_sent;   /**< the last time when event happen */
    SCROLLER_T      scrlr;
} vtss_event_ctrl_entry_t;

#endif

#endif
