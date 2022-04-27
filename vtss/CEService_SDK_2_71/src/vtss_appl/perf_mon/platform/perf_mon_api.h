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

#ifndef _VTSS_PERF_MON_API_H_
#define _VTSS_PERF_MON_API_H_

/**
 * \brief Default configuration
 */
/* PM module default value */
#define VTSS_PM_DEFAULT_LM_INTERVAL         15
#define VTSS_PM_DEFAULT_DM_INTERVAL         15
#define VTSS_PM_DEFAULT_EVC_INTERVAL        15
#define VTSS_PM_DEFAULT_ECE_INTERVAL        15

#define VTSS_PM_DEFAULT_FIXED_OFFSET        0
#define VTSS_PM_DEFAULT_RANDOM_OFFSET       0
#define VTSS_PM_DEFAULT_NUM_OF_INTERVAL     32

/* Enumeration Type */
#define VTSS_PM_TRANSFER_MODE_ALL       0
#define VTSS_PM_TRANSFER_MODE_NEW       1
#define VTSS_PM_TRANSFER_MODE_FIXED     2

#define VTSS_PM_TRANSFER_MINUTES_0      0
#define VTSS_PM_TRANSFER_MINUTES_15     15
#define VTSS_PM_TRANSFER_MINUTES_30     30
#define VTSS_PM_TRANSFER_MINUTES_45     45
#define VTSS_PM_TRANSFER_MINUTES_NULL   99

#define VTSS_PM_TRANSFER_HOURS_NULL     99

/* For PM Storage */
#define VTSS_PM_STORAGE_LM              0
#define VTSS_PM_STORAGE_DM              1
#define VTSS_PM_STORAGE_EVC             2
#define VTSS_PM_STORAGE_ECE             3
#define VTSS_PM_STORAGE_DISABLED        4

/* For PM Session */
#define VTSS_PM_SESSION_LM              0
#define VTSS_PM_SESSION_DM              1
#define VTSS_PM_SESSION_EVC             2
#define VTSS_PM_SESSION_ECE             3
#define VTSS_PM_SESSION_DISABLED        4

/* Default length */
#define PM_STR_LENGTH                   64

/* PM DATA TYPE */
typedef enum {
    PM_DATA_TYPE_LM = 0,
    PM_DATA_TYPE_DM,
    PM_DATA_TYPE_EVC,
    PM_DATA_TYPE_ECE
} pm_data_t;

/* PM Data Set Limit */
#define PM_DATA_SET_LIMIT               64
#define PM_LM_DATA_SET_LIMIT            PM_DATA_SET_LIMIT
#define PM_DM_DATA_SET_LIMIT            PM_DATA_SET_LIMIT
#define PM_EVC_DATA_SET_LIMIT           PM_DATA_SET_LIMIT
#define PM_ECE_DATA_SET_LIMIT           PM_DATA_SET_LIMIT

#define PM_MEASUREMENT_INTERVAL_LIMIT   96

/* for TFTP */
#define PM_BUFFER_STRING_LENGTH     1024
#define PM_BUF_LENGTH               512
#define PM_ALLOCATE_LM_BUFFER       (PM_BUF_LENGTH * PM_MEASUREMENT_INTERVAL_LIMIT * PM_LM_DATA_SET_LIMIT)
#define PM_ALLOCATE_DM_BUFFER       (PM_BUF_LENGTH * PM_MEASUREMENT_INTERVAL_LIMIT * PM_DM_DATA_SET_LIMIT)
#define PM_ALLOCATE_EVC_BUFFER      (PM_BUF_LENGTH * PM_MEASUREMENT_INTERVAL_LIMIT * PM_EVC_DATA_SET_LIMIT)
#define PM_ALLOCATE_ECE_BUFFER      (PM_BUF_LENGTH * PM_MEASUREMENT_INTERVAL_LIMIT * PM_ECE_DATA_SET_LIMIT)

/* for error handler */
#define PM_DEBUG_LM_MAX_LOOP        (2 * PM_MEASUREMENT_INTERVAL_LIMIT * PM_LM_DATA_SET_LIMIT)
#define PM_DEBUG_DM_MAX_LOOP        (2 * PM_MEASUREMENT_INTERVAL_LIMIT * PM_DM_DATA_SET_LIMIT)
#define PM_DEBUG_EVC_MAX_LOOP       (2 * PM_MEASUREMENT_INTERVAL_LIMIT * PM_EVC_DATA_SET_LIMIT)
#define PM_DEBUG_ECE_MAX_LOOP       (2 * PM_MEASUREMENT_INTERVAL_LIMIT * PM_ECE_DATA_SET_LIMIT)

/**
 * \brief Bit field macros for PM module
 */
#define VTSS_PM_BF_GET(a, n)        ((a & 1<<(n)) ? 1 : 0)
#define VTSS_PM_BF_SET(a, n, v)     { if (v) { a |= (1<<(n)); } else { a &= ~(1<<(n)); }}

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    PM_ERROR_CREATE_LM_FLASH_BLOCK = MODULE_ERROR_START(VTSS_MODULE_ID_PERF_MON),    /**< Create LM Report Block error. */
    PM_ERROR_CREATE_DM_FLASH_BLOCK,                                                  /**< Create DM Report Block error. */
    PM_ERROR_CREATE_EVC_FLASH_BLOCK,                                                 /**< Create EVC Report Block error. */
    PM_ERROR_CREATE_ECE_FLASH_BLOCK,                                                 /**< Create ECE Report Block error. */
    PM_ERROR_LOAD_LM_FLASH_BLOCK,                                                    /**< Load LM Report Block error. */
    PM_ERROR_LOAD_DM_FLASH_BLOCK,                                                    /**< Load DM Report Block error. */
    PM_ERROR_LOAD_EVC_FLASH_BLOCK,                                                   /**< Load EVC Report Block error. */
    PM_ERROR_LOAD_ECE_FLASH_BLOCK                                                    /**< Load ECE Report Block error. */
};

/**
 * \brief Performance Monitor Configuration
 */
typedef struct {

    /* Session and Storage configuration */

    u8      lm_session_mode;                    /* Loss Measurement Session Mode */
    u8      dm_session_mode;                    /* Delay Measurement Session Mode */
    u8      evc_session_mode;                   /* EVC Session Mode */
    u8      ece_session_mode;                   /* ECE Session Mode */

    u8      lm_storage_mode;                    /* Loss Measurement Storage Mode */
    u8      dm_storage_mode;                    /* Delay Measurement Storage Mode */
    u8      evc_storage_mode;                   /* EVC Storage Mode */
    u8      ece_storage_mode;                   /* ECE Storage Mode */

    u8      lm_interval;                        /* Loss Measurement Interval */
    u8      dm_interval;                        /* Delay Measurement Interval */
    u8      evc_interval;                       /* EVC Measurement Interval */
    u8      ece_interval;                       /* ECE Measurement Interval */

    /* Transfer */

    u8      transfer_mode;                      /* PM Transfer Mode */

    u32     transfer_scheduled_hours;           /* PM Scheduled Hours */

    u16     transfer_scheduled_random_offset;   /* PM Scheduled Random Offset */
    u8      transfer_scheduled_minutes;         /* PM Scheduled Minutes */
    u8      transfer_scheduled_offset;          /* PM Scheduled Offset */

    u8      transfer_interval_mode;             /* PM Transfer Interval Mode */
    u8      transfer_interval_num;              /* PM Transfer Number of Intervals */
    u8      transfer_incompleted;               /* PM Transfer Incomplete Transfers */

    char    transfer_server_url[PM_STR_LENGTH]; /* PM Transfer Server URL */

} perf_mon_conf_t;

/**
 * \brief Measurement Interval info
 */
typedef struct {
    u32 measurement_interval_id;
    //u32 interval_start_time;      // the start time interval can get from end time and elapsed time.
    u32 interval_end_time;
    u32 elapsed_time;
    pm_data_t   type;
} vtss_perf_mon_measurement_info_t;

/**
 * \brief Currect Configuration
 */
typedef struct {
    u32         current_interval_id;
    u32         current_interval_cnt;
} vtss_perf_mon_conf_t;

/**
 * \brief LM : Loss Measurement
 */
typedef struct {
    //vtss_perf_mon_measurement_info_t info;
    u32 measurement_interval_id;
    u32 tx_cnt;
    u32 rx_cnt;
    u32 near_end_loss_cnt;
    u32 far_end_loss_cnt;
    u16 mep_instance;
    u16 mep_flow;
    u16 mep_vlan;
    u8  tx_rate;
    u8  tx_priority;
    u8  near_end_frame_loss_ratio;
    u8  far_end_loss_ratio;
    u8  mep_residence_port;
    u8  mep_direction;
    u8  mep_domain;
    u8  mep_level;
    u8  mep_id;
    u8  mep_peer_id;
    u8  mep_mac[6];
    u8  mep_peer_mac[6];
    BOOL    valid;
} vtss_perf_mon_lm_info_t;

/**
 * \brief DM : Delay Measurement
 */
typedef struct {
    //vtss_perf_mon_measurement_info_t info;
    u32 measurement_interval_id;
    u32 tx_cnt;
    u32 rx_cnt;
    u32 two_way_avg_delay;
    u32 two_way_avg_delay_variation;
    u32 two_way_max_delay;
    u32 two_way_min_delay;
    u32 far_to_near_avg_delay;
    u32 far_to_near_avg_delay_variation;
    u32 far_to_near_max_delay;
    u32 far_to_near_min_delay;
    u32 near_to_far_avg_delay;
    u32 near_to_far_avg_delay_variation;
    u32 near_to_far_max_delay;
    u32 near_to_far_min_delay;
    u16 mep_instance;
    u16 mep_flow;
    u16 mep_vlan;
    u8  tx_rate;
    u8  tx_priority;
    u8  measurement_unit;
    u8  mep_residence_port;
    u8  mep_direction;
    u8  mep_domain;
    u8  mep_level;
    u8  mep_id;
    u8  mep_peer_id;
    u8  mep_mac[6];
    u8  mep_peer_mac[6];
    BOOL    valid;
} vtss_perf_mon_dm_info_t;

/**
 * \brief EVC and ECE Statistics
 */
typedef struct {
    //vtss_perf_mon_measurement_info_t info;
    u64 rx_green;
    u64 rx_yellow;
    u64 rx_red;
    u64 rx_discard;
    u64 tx_green;
    u64 tx_yellow;
    u64 tx_red;
    u64 tx_discard;
    u64 rx_green_b;
    u64 rx_yellow_b;
    u64 rx_red_b;
    u64 rx_discard_b;
    u64 tx_green_b;
    u64 tx_yellow_b;
    u64 tx_red_b;
    u64 tx_discard_b;
    u32 measurement_interval_id;
    u16 evc_instance;
    u8  evc_port;
    BOOL    valid;
} vtss_perf_mon_evc_info_t;

#define PM_CHK_DATA_DEF_RV                  FALSE
#define PM_CHK_DATA_VLD_BRK(x, y)           if ((x) && (x)->valid) {(y) = TRUE; break;}
#define PM_CHK_LM_DATA_SET_VALID(x, y)      do {    \
    u32 lml = 0;                                    \
    (y) = PM_CHK_DATA_DEF_RV;                       \
    for (; lml < PM_LM_DATA_SET_LIMIT; lml++)       \
        PM_CHK_DATA_VLD_BRK(&((x)[lml]), (y));      \
} while (0);

#define PM_CHK_DM_DATA_SET_VALID(x, y)      do {    \
    u32 dml = 0;                                    \
    (y) = PM_CHK_DATA_DEF_RV;                       \
    for (; dml < PM_DM_DATA_SET_LIMIT; dml++)       \
        PM_CHK_DATA_VLD_BRK(&((x)[dml]), (y));      \
} while (0);

#define PM_CHK_EVC_DATA_SET_VALID(x, y)     do {    \
    u32 evcl = 0;                                   \
    (y) = PM_CHK_DATA_DEF_RV;                       \
    for (; evcl < PM_EVC_DATA_SET_LIMIT; evcl++)    \
        PM_CHK_DATA_VLD_BRK(&((x)[evcl]), (y));     \
} while (0);

#define PM_CHK_ECE_DATA_SET_VALID(x, y)     do {    \
    u32 ecel = 0;                                   \
    (y) = PM_CHK_DATA_DEF_RV;                       \
    for (; ecel < PM_ECE_DATA_SET_LIMIT; ecel++)    \
        PM_CHK_DATA_VLD_BRK(&((x)[ecel]), (y));     \
} while (0);

#define PM_CHK_DATA_SET_VALID(a, b, v)      do {    \
    if ((a).type == PM_DATA_TYPE_LM) {              \
        PM_CHK_LM_DATA_SET_VALID((b), (v));         \
    } else if ((a).type == PM_DATA_TYPE_DM) {       \
        PM_CHK_DM_DATA_SET_VALID((b), (v));         \
    } else if ((a).type == PM_DATA_TYPE_EVC) {      \
        PM_CHK_EVC_DATA_SET_VALID((b), (v));        \
    } else if ((a).type == PM_DATA_TYPE_ECE) {      \
        PM_CHK_ECE_DATA_SET_VALID((b), (v));        \
    } else {                                        \
    u32 lmt = 0;                                    \
    (v) = PM_CHK_DATA_DEF_RV;                       \
    for (; lmt < PM_DATA_SET_LIMIT; lmt++)          \
        PM_CHK_DATA_VLD_BRK(&((b)[lmt]), (v));      \
    }                                               \
} while (0);

/**
  * \brief Decompose URL string for PM upload
  *
  * \return
  *   TRUE: URL string is decomposed succesfully
  *   FALSE: Invalid/Unsupported URL string
  */
BOOL vtss_perf_mon_decompose_url(const i8 *src_or_dest_text,
                                 const i8 *url_str,
                                 int url_len,
                                 void *myurl);

/**
  * \brief Get data from loss measurement
  *
  * \return
  *   None.
  */
BOOL vtss_perf_mon_lm_conf_get(vtss_perf_mon_conf_t *data);
BOOL vtss_perf_mon_lm_data_get(vtss_perf_mon_lm_info_t *data);
BOOL vtss_perf_mon_lm_data_get_first(vtss_perf_mon_lm_info_t *data);
BOOL vtss_perf_mon_lm_data_get_next(vtss_perf_mon_lm_info_t *data);
BOOL vtss_perf_mon_lm_data_get_previous(vtss_perf_mon_lm_info_t *data);
BOOL vtss_perf_mon_lm_data_get_last(vtss_perf_mon_lm_info_t *data);
BOOL vtss_perf_mon_lm_data_clear(void);

/**
  * \brief Get data from delay measurement
  *
  * \return
  *   None.
  */
BOOL vtss_perf_mon_dm_conf_get(vtss_perf_mon_conf_t *data);
BOOL vtss_perf_mon_dm_data_get(vtss_perf_mon_dm_info_t *data);
BOOL vtss_perf_mon_dm_data_get_first(vtss_perf_mon_dm_info_t *data);
BOOL vtss_perf_mon_dm_data_get_next(vtss_perf_mon_dm_info_t *data);
BOOL vtss_perf_mon_dm_data_get_previous(vtss_perf_mon_dm_info_t *data);
BOOL vtss_perf_mon_dm_data_get_last(vtss_perf_mon_dm_info_t *data);
BOOL vtss_perf_mon_dm_data_clear(void);

/**
  * \brief Get data from EVC statistics
  *
  * \return
  *   None.
  */
BOOL vtss_perf_mon_evc_conf_get(vtss_perf_mon_conf_t *data);
BOOL vtss_perf_mon_evc_data_get(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_evc_data_get_first(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_evc_data_get_next(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_evc_data_get_previous(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_evc_data_get_last(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_evc_data_clear(void);

/**
  * \brief Get data from ECE statistics
  *
  * \return
  *   None.
  */
BOOL vtss_perf_mon_ece_conf_get(vtss_perf_mon_conf_t *data);
BOOL vtss_perf_mon_ece_data_get(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_ece_data_get_first(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_ece_data_get_next(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_ece_data_get_previous(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_ece_data_get_last(vtss_perf_mon_evc_info_t *data);
BOOL vtss_perf_mon_ece_data_clear(void);

/**
  * \brief Get measurement interval from loss measurement
  *
  * \return
  *   None.
  */
BOOL vtss_perf_mon_lm_interval_get(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_lm_interval_get_first(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_lm_interval_get_next(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_lm_interval_get_previous(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_lm_interval_get_last(vtss_perf_mon_measurement_info_t *data);

/**
  * \brief Get measurement interval from delay measurement
  *
  * \return
  *   None.
  */
BOOL vtss_perf_mon_dm_interval_get(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_dm_interval_get_first(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_dm_interval_get_next(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_dm_interval_get_previous(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_dm_interval_get_last(vtss_perf_mon_measurement_info_t *data);

/**
  * \brief Get measurement interval from EVC statistics
  *
  * \return
  *   None.
  */
BOOL vtss_perf_mon_evc_interval_get(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_evc_interval_get_first(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_evc_interval_get_next(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_evc_interval_get_previous(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_evc_interval_get_last(vtss_perf_mon_measurement_info_t *data);

/**
  * \brief Get measurement interval from ECE statistics
  *
  * \return
  *   None.
  */
BOOL vtss_perf_mon_ece_interval_get(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_ece_interval_get_first(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_ece_interval_get_next(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_ece_interval_get_previous(vtss_perf_mon_measurement_info_t *data);
BOOL vtss_perf_mon_ece_interval_get_last(vtss_perf_mon_measurement_info_t *data);

/**
  * \brief PM Utility for adjusting current time w.r.t time zone
  *
  * \return
  *   None.
  */
void perf_mon_util_adjust_time_tz(void *now);

/**
  * \brief PM sending colloected statistics functions
  *
  * \return
  *   FALSE on any error, otherwise TRUE.
  */
BOOL vtss_perf_mon_transfer_lm_data_set(void *pm_url_dir);
BOOL vtss_perf_mon_transfer_dm_data_set(void *pm_url_dir);
BOOL vtss_perf_mon_transfer_evc_data_set(void *pm_url_dir);
BOOL vtss_perf_mon_transfer_ece_data_set(void *pm_url_dir);

/**
  * \brief Clear All Collection on performance monitor
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_clear_all_collection(void);

/**
  * \brief Reset flash data to default
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_reset_flash_data(void);

/**
  * \brief Get configuration of performance monitor
  *
  * \return
  *   None.
  */
void perf_mon_conf_get(perf_mon_conf_t *conf);

/**
  * \brief Set configuration of performance monitor
  *
  * \return
  *   None.
  */
void perf_mon_conf_set(perf_mon_conf_t *conf);

/**
  * \brief Initialize module
  *
  * \return
  *   (vtss_rc).
  */
vtss_rc perf_mon_init(vtss_init_data_t *data);

#endif /* _VTSS_PERF_MON_API_H_ */

// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
