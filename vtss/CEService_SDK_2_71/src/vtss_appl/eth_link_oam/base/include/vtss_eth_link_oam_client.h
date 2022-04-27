/*

 Vitesse ETH Link OAM software.

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

#ifndef _VTSS_ETH_LINK_OAM_CLIENT_API_H_
#define _VTSS_ETH_LINK_OAM_CLIENT_API_H_

/* For link OAM base configurations */
#include "vtss_eth_link_oam_base_api.h"
/* For Interthread serilization infrastructure */
#include "critd_api.h"

/******************************************************************************/
/* Eth Link OAM Client configuration data structures                          */
/******************************************************************************/

#define VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET           0x01
#define VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET           0x02
#define VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET 0x04
#define VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET        0x08
#define VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_RXTHRESHOLD_SET      0x10
#define VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET         0x20
#define VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET       0x40
#define VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_TOTAL_SET            0x80

/* Eth Link OAM timing parameters */
#define VTSS_ETH_LINK_OAM_SLEEP_TIME 1000
#define VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS 1


typedef struct vtss_eth_link_oam_client_link_error_symbol_period_event_oper_conf {
    u64            mgmt_event_window;
    u64            oper_event_window;   /* Update in multiples of the sleep time
                                         * in the timer thread */
    u64            oper_error_frame_threshold;
    u64            oper_error_frame_rxthreshold;
    u64            symbolErrors_at_start;   /* Store the errors at start */
    u64            symbolErrors_at_timeout; /* total errors at timeout   */
    u64            total_error_symbols;     /* total error frames        */
    u32            total_events_occured;    /* total events that exceeded
                                             * oper_error_frame_threshold*/
} vtss_eth_link_oam_client_link_error_symbol_period_event_oper_conf_t;

typedef vtss_eth_link_oam_client_link_error_symbol_period_event_oper_conf_t \
client_link_error_symbol_period_event_oper_conf_t;


typedef struct vtss_eth_link_oam_client_link_event_oper_conf {
    u16            mgmt_event_window;
    u16            oper_event_window; /* Update in multiples of the sleep
                                       *  time in the timer thread */
    u32            oper_error_frame_threshold;
    u64            ifInErrors_at_start;   /* Store the errors at start */
    u64            ifInErrors_at_timeout; /* total errors at timeout    */
    u64            total_error_frames;    /* total error frames         */
    u64            total_errors_occured;    /* total error frames         */
    u32            total_events_occured;  /* total events that exceeded
                                           * oper_error_frame_threshold */
} vtss_eth_link_oam_client_link_event_oper_conf_t;

typedef struct vtss_eth_link_oam_client_link_error_frame_period_event_oper_conf {
    u32            mgmt_event_window;
    u32            oper_event_window; /* Update in multiples of the sleep time
                                       * in the timer thread */
    u32            oper_error_frame_threshold;
    u64            oper_error_frame_rxthreshold;
    u64            frameErrors_at_start;   /* Store the errors at start */
    u64            frameErrors_at_timeout; /* total errors at timeout   */
    u64            total_error_frames;     /* total error frames        */
    u32            total_events_occured;   /* total events that exceeded
                                            * oper_error_frame_threshold*/
} vtss_eth_link_oam_client_link_error_frame_period_event_oper_conf_t;
typedef vtss_eth_link_oam_client_link_error_frame_period_event_oper_conf_t \
client_link_error_frame_period_event_oper_conf_t;

typedef struct vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf {
    u16            mgmt_event_window;
    u16            oper_event_window; /* Update in multiples of the sleep time
                                       * in the timer thread */
    u16            total_errord_seconds;
    u16            oper_secs_summary_threshold;
    u32            secErrors_at_start;   /* Store the errors at start */
    u32            secErrors_at_timeout; /* total errors at timeout    */
    u32            total_secErrorSummary_frames;/* total error frames    */
    u32            total_secErrorEvents_occured;/* total events that
x                                                      * exceeded oper_error_frame_threshold */
} vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf_t;

typedef struct vtss_eth_link_oam_client_error_frame_info {
    vtss_eth_link_oam_error_frame_event_tlv_t         error_frame_tlv;
    vtss_eth_link_oam_error_frame_event_tlv_t         remote_error_frame_tlv;
    vtss_eth_link_oam_client_link_event_oper_conf_t   error_frame_oper_conf;
} vtss_eth_link_oam_client_error_frame_info_t;

typedef struct vtss_eth_link_oam_client_symbol_period_errors_info {
    vtss_eth_link_oam_error_symbol_period_event_tlv_t  symbol_period_error_tlv;
    vtss_eth_link_oam_error_symbol_period_event_tlv_t  remote_symbol_period_error_tlv;
    client_link_error_symbol_period_event_oper_conf_t  symbol_period_error_oper_conf;
} vtss_eth_link_oam_client_symbol_period_errors_info_t;

typedef struct vtss_eth_link_oam_client_frame_period_errors_info {
    vtss_eth_link_oam_error_frame_period_event_tlv_t  frame_period_error_tlv;
    vtss_eth_link_oam_error_frame_period_event_tlv_t  remote_frame_period_error_tlv;
    client_link_error_frame_period_event_oper_conf_t  frame_period_error_oper_conf;
} vtss_eth_link_oam_client_frame_period_errors_info_t;

typedef struct vtss_eth_link_oam_client_error_frame_secs_summary_info {
    vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t error_frame_secs_summary_tlv;
    vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t remote_error_frame_secs_summary_tlv;
    vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf_t error_frame_secs_summary_oper_conf;
} vtss_eth_link_oam_client_error_frame_secs_summary_info_t;

typedef struct eth_link_oam_client_conf {
    BOOL                                 remote_state_valid;
    BOOL                                 local_satisfied;
    BOOL                                 remote_stable;
    u8                                   oam_control;
    u16                                  error_sequence_number;
    u16                                  remote_error_sequence_num;
    vtss_eth_link_oam_info_tlv_t         local_info;
    vtss_eth_link_oam_info_tlv_t         remote_info;
    void                                 (*oam_eth_link_cb)(i8 *resp);
    i8                                    *buf;
    u32                                   buf_size;
    u8                                    remote_mac_addr[VTSS_ETH_LINK_OAM_MAC_LEN];

    vtss_eth_link_oam_client_error_frame_info_t          error_frame_info;
    vtss_eth_link_oam_client_symbol_period_errors_info_t symbol_period_error_info;
    vtss_eth_link_oam_client_frame_period_errors_info_t  error_frame_period_info;
    vtss_eth_link_oam_client_error_frame_secs_summary_info_t error_frame_secs_summary_info;
} vtss_eth_link_oam_client_conf_t;

typedef struct eth_link_oam_ace_id {
    u32 pdu_ace_id;
    u32 lb_ace_id;
} vtss_eth_link_oam_ace_id_t;

typedef struct eth_link_oam_client_port_conf {
    vtss_eth_link_oam_client_conf_t  oam_conf;
    vtss_eth_link_oam_ace_id_t       oam_ace_id;
} vtss_eth_link_oam_port_client_conf_t;

typedef struct eth_link_oam_client_oper_conf {
    vtss_eth_link_oam_port_client_conf_t port_conf[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
} vtss_eth_link_oam_client_oper_conf_t;

/***********************************************************************************/
/* Eth Link OAM Module Message interactions                                        */
/***********************************************************************************/

/* Eth Link OAM Module Events data structures*/
typedef enum {
    VTSS_ETH_LINK_OAM_MGMT_PORT_CONF_INIT_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_CONTROL_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_MODE_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_MIB_RETRIVAL_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_REMOTE_LOOPBACK_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_REMOTE_LOOPBACK_OPER_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_LINK_MONITORING_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_WINDOW_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_THRESHOLD_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_WINDOW_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_THRESHOLD_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_RXTHRESHOLD_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_WINDOW_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_THRESHOLD_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_RXTHRESHOLD_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_WINDOW_CONF_EVENT,
    VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_THRESHOLD_CONF_EVENT,
    VTSS_ETH_LINK_OAM_PORT_UP_EVENT,
    VTSS_ETH_LINK_OAM_PORT_DOWN_EVENT,
    VTSS_ETH_LINK_OAM_PORT_FAULT_EVENT,
    VTSS_ETH_LINK_OAM_PDU_RX_EVENT,
    VTSS_ETH_LINK_OAM_PDU_TX_EVENT,
    VTSS_ETH_LINK_OAM_PDU_LOCAL_LOST_LINK_TIMER_EVENT,
} vtss_eth_link_oam_events_t;

/* Eth link OAM Loopback status as per RFC 4878*/
typedef enum {
    VTSS_ETH_LINK_OAM_NO_LOOPBACK          =        1,
    VTSS_ETH_LINK_OAM_INITIATING_LOOPBACK,
    VTSS_ETH_LINK_OAM_REMOTE_LOOPBACK,
    VTSS_ETH_LINK_OAM_TERMINATING_LOOPBACK,
    VTSS_ETH_LINK_OAM_LOCAL_LOOPBACK,
    VTSS_ETH_LINK_OAM_UNKNOWN,
} vtss_eth_link_oam_loopback_status_t;

typedef struct eth_link_oam_message {
    vtss_eth_link_oam_events_t event_code;
    u32                        event_on_port;
    u16                        event_data_len;
    u8                         event_data[VTSS_ETH_LINK_OAM_TAGGED_PDU_MAX_LEN];
} vtss_eth_link_oam_message_t;

/******************************************************************************/
/* Eth Link OAM client configuration Macros                                   */
/******************************************************************************/

#define PORT_CLIENT_CONF(port_no)        (\
                          (&link_oam_client_conf.port_conf[port_no].oam_conf)\
                                         )

#define PORT_CLIENT_ACE_ID(port_no)      (\
                        (&link_oam_client_conf.port_conf[port_no].oam_ace_id)\
                                         )

#define PORT_CLIENT_LOCAL_CONF(port_no)  (\
                (&link_oam_client_conf.port_conf[port_no].oam_conf.local_info)\
                                         )

#define PORT_CLIENT_REMOTE_CONF(port_no) (\
              (&link_oam_client_conf.port_conf[port_no].oam_conf.remote_info)\
                                         )


#define PORT_CLIENT_ERROR_FRAME_CONF(port_no) (\
              (&(PORT_CLIENT_CONF(port_no)->error_frame_info))\
                                              )

#define PORT_CLIENT_SYMBOL_PERIOD_ERROR_CONF(port_no) (\
               (&(PORT_CLIENT_CONF(port_no)->symbol_period_error_info))\
                                                      )

#define PORT_CLIENT_FRAME_PERIOD_ERROR_CONF(port_no)  (\
               (&(PORT_CLIENT_CONF(port_no)->error_frame_period_info))\
                                                      )
#define PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_CONF(port_no) (\
      (&(PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info)))

/* Symbol Period Error Frame event Oper Conf macros */
#define GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_SEQUENCE_NUMBER(port_no) (\
       (link_oam_client_conf.port_conf[port_no].oam_conf.error_sequence_number)\
                                                                     )

#define SET_PORT_CLIENT_SYMBOL_PEROIOD_ERROR_SEQUENCE_NUMBER(port_no, conf) (\
          (GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_SEQUENCE_NUMBER(port_no) = conf)\
                                                       )

#define GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no) (\
                   (PORT_CLIENT_CONF(port_no)->symbol_period_error_info.\
                    symbol_period_error_oper_conf.mgmt_event_window))

#define SET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no, conf) (\
        (GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no) = conf)\
                                                                             )

#define GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_WINDOW(port_no)  (\
       (PORT_CLIENT_CONF(port_no)->symbol_period_error_info.\
        symbol_period_error_oper_conf.oper_event_window))

#define SET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_WINDOW(port_no, conf)(\
      (GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_WINDOW(port_no) = conf))

#define GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_START(port_no)  (\
                  (PORT_CLIENT_CONF(port_no)->symbol_period_error_info.\
                   symbol_period_error_oper_conf.symbolErrors_at_start))

#define SET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_START(port_no, conf) (\
         (GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_START(port_no) = conf))

#define GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_TIMEOUT(port_no)    (\
         (PORT_CLIENT_CONF(port_no)->symbol_period_error_info.\
          symbol_period_error_oper_conf.symbolErrors_at_timeout))

#define SET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_TIMEOUT(port_no, conf) (\
      (GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_TIMEOUT(port_no) = conf))

#define GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_THRESHOLD(port_no) (\
       (PORT_CLIENT_CONF(port_no)->symbol_period_error_info.\
        symbol_period_error_oper_conf.oper_error_frame_threshold))

#define SET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_THRESHOLD(port_no, conf) (\
       (GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_THRESHOLD(port_no) = conf))

#define GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_RXTHRESHOLD(port_no) (\
        (PORT_CLIENT_CONF(port_no)->symbol_period_error_info.\
         symbol_period_error_oper_conf.oper_error_frame_rxthreshold))

#define SET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_RXTHRESHOLD(port_no, conf) (\
       (GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_RXTHRESHOLD(port_no) = conf))

#define GET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_ERROR_SYMBOLS(port_no)    (\
    (PORT_CLIENT_CONF(port_no)->symbol_period_error_info.\
     symbol_period_error_oper_conf.total_error_symbols))

#define SET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_ERROR_SYMBOLS(port_no, conf) (\
    (GET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_ERROR_SYMBOLS(port_no) = conf))

#define GET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_EVENTS_OCCURED(port_no)    (\
    (PORT_CLIENT_CONF(port_no)->symbol_period_error_info.\
     symbol_period_error_oper_conf.total_events_occured))

#define SET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_EVENTS_OCCURED(port_no, conf) (\
      (GET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_EVENTS_OCCURED(port_no) = conf))

/* Error frame event Oper Conf Macros */
#define GET_PORT_CLIENT_ERROR_FRAME_SEQUENCE_NUMBER(port_no)     (\
                (PORT_CLIENT_CONF(port_no)->error_sequence_number))


#define SET_PORT_CLIENT_ERROR_FRAME_SEQUENCE_NUMBER(port_no, conf) (\
       (GET_PORT_CLIENT_ERROR_FRAME_SEQUENCE_NUMBER(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_WINDOW(port_no)    (\
       (PORT_CLIENT_CONF(port_no)->error_frame_info.error_frame_oper_conf.\
        oper_event_window))

#define SET_PORT_CLIENT_ERROR_FRAME_WINDOW(port_no, conf) (\
       (GET_PORT_CLIENT_ERROR_FRAME_WINDOW(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_MGMT_WINDOW(port_no)  (\
     (PORT_CLIENT_CONF(port_no)->error_frame_info.error_frame_oper_conf.\
      mgmt_event_window))

#define SET_PORT_CLIENT_ERROR_FRAME_MGMT_WINDOW(port_no, conf) (\
     (GET_PORT_CLIENT_ERROR_FRAME_MGMT_WINDOW(port_no) = conf))


#define GET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_START(port_no)   (\
     (PORT_CLIENT_CONF(port_no)->error_frame_info.error_frame_oper_conf.\
      ifInErrors_at_start))

#define SET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_START(port_no, conf) (\
      (GET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_START(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_TIMEOUT(port_no)     (\
   (PORT_CLIENT_CONF(port_no)->error_frame_info.error_frame_oper_conf.\
    ifInErrors_at_timeout))

#define SET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_TIMEOUT(port_no, conf) (\
     (GET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_TIMEOUT(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_ERROR_THRESHOLD(port_no)    (\
    (PORT_CLIENT_CONF(port_no)->error_frame_info.error_frame_oper_conf.\
     oper_error_frame_threshold))

#define SET_PORT_CLIENT_ERROR_FRAME_ERROR_THRESHOLD(port_no, conf)  (\
    (GET_PORT_CLIENT_ERROR_FRAME_ERROR_THRESHOLD(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_TOTAL_ERROR_FRAMES(port_no)  (\
      (PORT_CLIENT_CONF(port_no)->error_frame_info.error_frame_oper_conf.\
       total_error_frames))

#define SET_PORT_CLIENT_ERROR_FRAME_TOTAL_ERROR_FRAMES(port_no, conf) (\
       (GET_PORT_CLIENT_ERROR_FRAME_TOTAL_ERROR_FRAMES(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_TOTAL_EVENTS_OCCURED(port_no)     (\
   (PORT_CLIENT_CONF(port_no)->error_frame_info.error_frame_oper_conf.\
    total_events_occured))

#define SET_PORT_CLIENT_ERROR_FRAME_TOTAL_EVENTS_OCCURED(port_no, conf) (\
    (GET_PORT_CLIENT_ERROR_FRAME_TOTAL_EVENTS_OCCURED(port_no) = conf))

/* Frame Period Error Event Oper Mactos */
#define GET_PORT_CLIENT_FRAME_PERIOD_ERROR_SEQUENCE_NUMBER(port_no)     (\
       (PORT_CLIENT_CONF(port_no)->error_sequence_number))

#define SET_PORT_CLIENT_FRAME_PERIOD_ERROR_SEQUENCE_NUMBER(port_no, conf) (\
     (GET_PORT_CLIENT_FRAME_PERIOD_ERROR_SEQUENCE_NUMBER(port_no) = conf))

#define GET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no)    (\
    (PORT_CLIENT_CONF(port_no)->error_frame_period_info.\
     frame_period_error_oper_conf.mgmt_event_window ))

#define SET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no, conf) (\
   (GET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no) = conf))

#define GET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_WINDOW(port_no) (\
       (PORT_CLIENT_CONF(port_no)->error_frame_period_info.\
        frame_period_error_oper_conf.oper_event_window))

#define SET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_WINDOW(port_no, conf) (\
       (GET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_WINDOW(port_no) = conf))

#define GET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_START(port_no)   (\
     (PORT_CLIENT_CONF(port_no)->error_frame_period_info.\
      frame_period_error_oper_conf.frameErrors_at_start))

#define SET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_START(port_no, conf) (\
  (GET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_START(port_no) = conf))

#define GET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_TIMEOUT(port_no) (\
       (PORT_CLIENT_CONF(port_no)->error_frame_period_info.\
        frame_period_error_oper_conf.frameErrors_at_timeout))

#define SET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_TIMEOUT(port_no, conf) (\
       (GET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_TIMEOUT(port_no) = conf))

#define GET_PORT_CLIENT_FRAME_PERIOD_ERROR_THRESHOLD(port_no) (\
        (PORT_CLIENT_CONF(port_no)->error_frame_period_info.\
         frame_period_error_oper_conf.oper_error_frame_threshold))

#define SET_PORT_CLIENT_FRAME_PERIOD_ERROR_THRESHOLD(port_no, conf) (\
       (GET_PORT_CLIENT_FRAME_PERIOD_ERROR_THRESHOLD(port_no) = conf))

#define GET_PORT_CLIENT_FRAME_PERIOD_ERROR_RXTHRESHOLD(port_no)   (\
     (PORT_CLIENT_CONF(port_no)->error_frame_period_info.\
      frame_period_error_oper_conf.oper_error_frame_rxthreshold))

#define SET_PORT_CLIENT_FRAME_PERIOD_ERROR_RXTHRESHOLD(port_no, conf) (\
     (GET_PORT_CLIENT_FRAME_PERIOD_ERROR_RXTHRESHOLD(port_no) = conf))

#define GET_PORT_CLIENT_FRAME_PERIOD_TOTAL_ERROR_FRAMES(port_no)     (\
    (PORT_CLIENT_CONF(port_no)->error_frame_period_info.\
     frame_period_error_oper_conf.total_error_frames))

#define SET_PORT_CLIENT_FRAME_PERIOD_TOTAL_ERROR_FRAMES(port_no, conf) (\
    (GET_PORT_CLIENT_FRAME_PERIOD_TOTAL_ERROR_FRAMES(port_no) = conf))

#define GET_PORT_CLIENT_FRAME_PERIOD_TOTAL_EVENTS_OCCURED(port_no)   (\
     (PORT_CLIENT_CONF(port_no)->error_frame_period_info.\
      frame_period_error_oper_conf.total_events_occured))

#define SET_PORT_CLIENT_FRAME_PERIOD_TOTAL_EVENTS_OCCURED(port_no, conf) (\
      (GET_PORT_CLIENT_FRAME_PERIOD_TOTAL_EVENTS_OCCURED(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_SEQUENCE_NUMBER(port_no) (\
       (link_oam_client_conf.port_conf[port_no].oam_conf.error_sequence_number))

#define SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_SEQUENCE_NUMBER(port_no, conf) (\
  (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_SEQUENCE_NUMBER(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_WINDOW(port_no)       (\
         (PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info.\
         error_frame_secs_summary_oper_conf.oper_event_window ))

#define SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_WINDOW(port_no, conf) (\
        (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_WINDOW(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_MGMT_WINDOW(port_no)  (\
      (PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info.\
             error_frame_secs_summary_oper_conf.mgmt_event_window))

#define SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_MGMT_WINDOW(port_no, conf) (\
       (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_MGMT_WINDOW(port_no) = conf))


#define GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_START(port_no)    (\
                   (PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info.\
                    error_frame_secs_summary_oper_conf.secErrors_at_start))

#define SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_START(port_no, conf) (\
      (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_START(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_TIMEOUT(port_no)     (\
                  (PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info.\
                   error_frame_secs_summary_oper_conf.secErrors_at_timeout))

#define SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_TIMEOUT(port_no, conf) (\
     (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_TIMEOUT(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_THRESHOLD(port_no)   (\
     (PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info.\
      error_frame_secs_summary_oper_conf.oper_secs_summary_threshold))

#define SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_THRESHOLD(port_no, conf) (\
   (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_THRESHOLD(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_ERROR_FRAMES(port_no) (\
       (PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info.\
        error_frame_secs_summary_oper_conf.total_secErrorSummary_frames))

#define SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_ERROR_FRAMES(port_no, conf) (\
       (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_ERROR_FRAMES(port_no) = conf))

#define GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_EVENTS_OCCURED(port_no)        (link_oam_client_conf.port_conf[port_no].oam_conf.error_frame_secs_summary_info.error_frame_secs_summary_oper_conf.total_secErrorEvents_occured)

#define SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_EVENTS_OCCURED(port_no, conf)  (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_EVENTS_OCCURED(port_no) = conf)


#define SET_CLIENT_PORT_CONTROL(port_no, enable)   (\
                    (PORT_CLIENT_CONF(port_no)->oam_control = enable))

#define RESET_CLIENT_PORT_CONTROL(port_no)         (\
                    (PORT_CLIENT_CONF(port_no)->oam_control = 0))

#define GET_CLIENT_PORT_CONTROL(port_no)           (\
                    (PORT_CLIENT_CONF(port_no)->oam_control))

#define GET_CLIENT_PORT_CONF(port_no)      (\
                    (PORT_CLIENT_CONF(port_no)->local_info.oam_conf))

#define SET_CLIENT_PORT_CONF(port_no,conf_pos)     (\
                    ((GET_CLIENT_PORT_CONF(port_no)|=conf_pos)))

#define RESET_CLIENT_PORT_CONF(port_no,conf_pos)   (\
                   ((GET_CLIENT_PORT_CONF(port_no)&=(~((u8)conf_pos)))))

#define IS_PORT_CONF_ACTIVE(port_no,conf_pos)      (\
                   ((GET_CLIENT_PORT_CONF(port_no)&conf_pos)>0))

#define IS_FLAG_CONF_ACTIVE(flags,flag_pos)        (flags&flag_pos)

#define IS_PDU_CONF_ACTIVE(oam_conf,conf_pos)      (oam_conf&conf_pos)
#define SET_PORT_STATE(port_no,new_state)          (\
                  (PORT_CLIENT_LOCAL_CONF(port_no)->state = new_state))

#define GET_PORT_STATE(port_no)        (PORT_CLIENT_LOCAL_CONF(port_no)->state)

#define SET_VAR_RESPONSE_CB(port_no,cb) (PORT_CLIENT_CONF(port_no)->\
                                         oam_eth_link_cb = cb)

#define SET_VAR_RESPONSE_BUF(port_no,buf) (PORT_CLIENT_CONF(port_no)->buf = buf)

#define SET_VAR_RESPONSE_BUF_SIZE(port_no,size) (PORT_CLIENT_CONF(port_no)->\
                                                 buf_size = size)

#define GET_VAR_RESPONSE_CB(port_no)            (PORT_CLIENT_CONF(port_no)->\
                                                 oam_eth_link_cb)

#define GET_VAR_RESPONSE_BUF(port_no)           (PORT_CLIENT_CONF(port_no)->buf)

#define GET_VAR_RESPONSE_BUF_SIZE(port_no)      (PORT_CLIENT_CONF(port_no)->\
                                                 buf_size)

#define SET_PORT_PARSER_STATE(port_state,parser_state)  (port_state = \
                                        ((port_state & 4)|parser_state))

#define GET_PORT_PARSER_STATE(port_no)  ((PORT_CLIENT_LOCAL_CONF(port_no)->\
                                          state)&3)

#define GET_REMOTE_PORT_PARSER_STATE(port_no)  ((PORT_CLIENT_REMOTE_CONF(port_no)->\
                                                 state)&3)

#define SET_PORT_MUX_STATE(port_state,mux_state)   (port_state = \
                                     ((port_state&3)|mux_state<<2))

#define GET_PORT_MUX_STATE(port_no)   (((PORT_CLIENT_LOCAL_CONF(port_no)->\
                                         state)&4) >> 2)

#define GET_REMOTE_PORT_MUX_STATE(port_no)   (((PORT_CLIENT_REMOTE_CONF(port_no)->\
                                                state)&4) >> 2)

#define GET_PDU_MUX_STATE(pdu_state)    ((pdu_state&4)>>2)
#define GET_PDU_PARSER_STATE(pdu_state) (pdu_state&3)

#define GET_PORT_PARSER_OAM_ACE_ID(port_no) (PORT_CLIENT_ACE_ID(port_no)->\
                                             pdu_ace_id)
#define GET_PORT_PARSER_LB_ACE_ID(port_no)  (PORT_CLIENT_ACE_ID(port_no)->\
                                             lb_ace_id)

#define SET_PORT_PARSER_OAM_ACE_ID(port_no,ace_id) (\
                 (GET_PORT_PARSER_OAM_ACE_ID(port_no) = ace_id))
#define SET_PORT_PARSER_LB_ACE_ID(port_no,ace_id)  (\
                 (GET_PORT_PARSER_LB_ACE_ID(port_no) = ace_id))


/******************************************************************************/
/* Eth Link OAM Module Client function prototypes                             */
/******************************************************************************/

/* port_no:    l2 port number                                                 */
/* Initializes the OAM Conf on the port                                       */
u32 vtss_eth_link_oam_client_port_conf_init (const u32  port_no);


/* port_no:    l2 port number                                                 */
/* conf:       Enable/Disable the OAM Control                                 */
/* Enables/Disables the OAM Control on the port                               */
u32 vtss_eth_link_oam_client_port_control_conf_set (const u32  port_no, const vtss_eth_link_oam_control_t conf);

/* port_no:    l2 port number                                                 */
/* conf:       Enable/Disable the OAM Control                                 */
/* Retrievs the port's OAM Control                                            */
u32 vtss_eth_link_oam_client_port_control_conf_get (const u32  port_no, u8 *const conf);


/* port_no:    l2 port number                                                 */
/* conf:       Port configuration including the port mode                     */
/* Configures the Port with OAM capabilities                                  */
u32 vtss_eth_link_oam_client_port_admin_conf_set (const u32  port_no, const vtss_eth_link_oam_capability_conf_t conf, const BOOL capability_on);

/* oam_code:   OAM code to be supported by OAM client                         */
/* Configures OAM Control support for the specified OAM code                  */
u32 vtss_eth_link_oam_client_port_supported_code_conf_set (const u8 oam_code, const BOOL support_enable);

/* oam_code:   OAM code to be supported by OAM client                         */
/* Retrives the OAM Control support for the specified OAM code                */
u32 vtss_eth_link_oam_client_port_supported_code_conf_get (const u8 oam_code, const BOOL *support_enable);

/* port_no:         port number                                               */
/* rev_update:      Updates/resets the revision                               */
/* Updates the Port's local information revison based on local information    */
u32 vtss_eth_link_oam_client_port_revision_update(const u32 port_no, BOOL  rev_update);


/* port_no:    port number                                                    */
/* info:       local info                                                     */
/* Retrives the Port OAM Local information                                    */
u32 vtss_eth_link_oam_client_port_local_info_get (const u32 port_no, vtss_eth_link_oam_info_tlv_t *const local_info);

/* port_no:    port number                                                    */
/* info:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) information                             */
u32 vtss_eth_link_oam_client_port_remote_info_get (const u32 port_no, vtss_eth_link_oam_info_tlv_t *const remote_info);

/* port_no:    port number                                                    */
/* conf:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) information                             */
u32 vtss_eth_link_oam_client_port_remote_seq_num_get (const u32  port_no, u16 *const  conf);


u32 vtss_eth_link_oam_client_error_frame_event_fill_info_data(const u32 port_no, u8 *pdu);

/* port_no:    port number                                                    */
/* info:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) MAC address                             */
u32 vtss_eth_link_oam_client_port_remote_mac_addr_info_get (const u32 port_no, u8 *const remote_mac_addr);

/* port_no:      port number                                                  */
/* max_pdu_len:  max pdu len                                                  */
/* Specifies the max pdu length that can be used for communication between    */
/* peers                                                                      */
u32 vtss_eth_link_oam_client_pdu_max_len(const u32 port_no, u16   *max_pdu_len);


/* port_no:           port number                                             */
/* local_error_info:       local error info                                   */
/* local_error_info:       Remote error info                                  */
/* Retrives the Port's frame error  information                               */
u32 vtss_eth_link_oam_client_port_frame_error_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_event_tlv_t  *local_error_info, vtss_eth_link_oam_error_frame_event_tlv_t  *remote_error_info);

/* port_no:                port number                                        */
/* local_error_info:       local error info                                   */
/* local_error_info:       Remote error info                                  */
/* Retrives the Port's frame period error  information                        */
u32 vtss_eth_link_oam_client_port_frame_period_error_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_period_event_tlv_t *const local_info, vtss_eth_link_oam_error_frame_period_event_tlv_t *const remote_info);

/* port_no:                port number                                        */
/* local_info:             local error info                                   */
/* local_info:             Remote error info                                  */
/* Retrives the Port's frame period error  information                        */
u32 vtss_eth_link_oam_client_port_symbol_period_error_info_get (const u32 port_no, vtss_eth_link_oam_error_symbol_period_event_tlv_t *const local_info, vtss_eth_link_oam_error_symbol_period_event_tlv_t *const remote_info);

/* port_no:                port number                                        */
/* local_info:             local error info                                   */
/* local_info:             Remote error info                                  */
/* Retrives the Port's frame period error  information                        */
u32 vtss_eth_link_oam_client_port_error_frame_secs_summary_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t *local_info, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t *remote_info);


/* port_no:                   port number                                     */
/* is_update_needed:          Bool Value to Update                            */
u32 vtss_eth_link_oam_client_is_link_event_stats_update_needed(const u32 port_no, BOOL *const is_update_needed);

/* port_no:                      port number                                  */
/* is_frame_window_expires:      Bool Value to Update                         */
u32 vtss_eth_link_oam_client_is_error_frame_window_expired(const u32 port_no, BOOL *const is_frame_window_expired);

/* port_no:           port number                                             */
/* oper_conf:         Data Structure containing all the operational Values    */
/* flags:             Fields that need to be updated                          */
u32 vtss_eth_link_oam_client_error_frame_oper_conf_update(const u32 port_no, const vtss_eth_link_oam_client_link_event_oper_conf_t *oper_conf, const u32 flags);

/* port_no:                   port number                                     */
/* is_update_needed:          Bool Value to Update                            */
u32 vtss_eth_link_oam_client_error_frame_event_conf_set(const u32 port_no, BOOL *is_error_frame_xmit_needed);
/* port_no:    port number                                                   */
/* pdu    :    Char Ptr to update the event pdu                              */
u32 vtss_eth_link_oam_client_symbol_period_error_frame_event_fill_info_data(const u32 port_no, u8 *pdu);
/* port_no:                      port number                                  */
/* is_frame_window_expires:      Bool Value to Update                         */
u32 vtss_eth_link_oam_client_is_symbol_period_frame_window_expired(const u32 port_no, BOOL *const is_frame_window_expired);

/* port_no  :    port number                                                  */
/* oper_con :    Data that needs to be updated to the oper conf structure     */
/* flags    :    Used to store update the oper values in the events           */
u32 vtss_eth_link_oam_client_symbol_period_error_oper_conf_update(const u32 port_no, const client_link_error_symbol_period_event_oper_conf_t *oper_conf, const u32 flags);

/* port_no  :      port number                                                */
/* is_xmit_needed: verifies the need to transmit the symbole period event     */
u32 vtss_eth_link_oam_client_symbol_period_error_conf_set(const u32 port_no, BOOL *const is_xmit_needed);

/* port_no  :      port number                                                */
/* pdu:            fills the pdu                                              */
u32 vtss_eth_link_oam_client_frame_period_error_frame_event_fill_info_data(const u32 port_no, u8 *pdu);

/* port_no  :      port number                                                */
/* is_frame_window_expired: verifies the expiration of time                   */
u32 vtss_eth_link_oam_client_is_frame_period_frame_window_expired(const u32 port_no, BOOL *const is_frame_window_expired);

/* port_no  :      port number                                                */
/* oper_conf:      operation configuration                                    */
/* flags    :      flags                                                      */
/* updates the error configurations                                           */
u32 vtss_eth_link_oam_client_frame_period_error_oper_conf_update(const u32 port_no, const client_link_error_frame_period_event_oper_conf_t *oper_conf, const u32 flags);

/* port_no  :      port number                                                */
/* is_xmit_needed: verifies the need to transmit the frame period event       */
u32 vtss_eth_link_oam_client_frame_period_error_conf_set(const u32 port_no, BOOL *is_frame_period_xmit_needed);

u32 vtss_eth_link_oam_client_is_error_frame_secs_summary_window_expired(const u32 port_no, BOOL *is_frame_window_expired);

u32 vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(const u32 port_no, const vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf_t *oper_conf, const u32 flags);

u32 vtss_eth_link_oam_client_error_frame_secs_summary_event_conf_set(const u32 port_no, const BOOL is_timer_expired, BOOL *is_error_frame_xmit_needed);


/* port_no:           port number                                             */
/* pdu:               OAM pdu                                                 */
/* var_branch:        variable branch                                         */
/* var_leaf:          variable leaf                                           */
/* current_pdu_len:   Index of PDU position to fill the Variable              */
/* max_pdu_len:       Maximum PDU length                                      */
/* Fills the Variable descriptor                                              */
u32 vtss_eth_link_oam_client_port_build_var_descriptor(const u32 port_no, u8 *pdu, const u8 var_branch, const u16 var_leaf, u16  *current_pdu_len, u16 max_pdu_len);

/* port_no:           port number                                             */
/* pdu:               OAM pdu                                                 */
/* var_branch:        variable branch                                         */
/* var_leaf:          variable leaf                                           */
/* current_pdu_len:   Index of PDU position to fill the Variable              */
/* max_pdu_len:       Maximum PDU length                                      */
/* Fills the Variable container                                               */
u32 vtss_eth_link_oam_client_port_build_var_container(const u32 port_no, u8 *pdu, const u8 var_branch, const u16 var_leaf, u16  *current_pdu_len, u16 max_pdu_len);

/* port_no:           port number                                             */
/* enable_flag:       Enable/Disable the remote loop back operation           */
u32 vtss_eth_link_oam_client_port_remote_loop_back_oper_set(const u32 port_no, const BOOL enable_flag);

/* port_no:           port number                                             */
/* pdu:               OAM pdu                                                 */
/* current_pdu_len:   Index of PDU position to fill the Variable              */
/* max_pdu_len:       Maximum PDU length                                      */
/* enable_flag:       Enable/Disable the loop back operation                  */
/* Enables/Disables the Loop back operation                                   */
u32 vtss_eth_link_oam_client_port_build_remote_loop_back_tlv(const u32 port_no, u8 *pdu, u16 *current_pdu_len, u16 max_pdu_len, const BOOL enable_flag);

/* port_no:               port number                                             */
/* mux_state:             port MUX state                                          */
/* parser_state:          port parser state                                       */
/* update_mux_state_only: Flag to indicate to update mux state only               */
/* Configures the port's MUX/PARSER states                                    */
u32 vtss_eth_link_oam_client_port_state_conf_set(const u32 port_no, const vtss_eth_link_oam_mux_state_t mux_state, const vtss_eth_link_oam_parser_state_t parser_state, const BOOL rev_update, const BOOL update_mux_state_only);

/* port_no:           port number                                             */
/* cb:                call back function                                      */
/* buf:               buffer                                                  */
/* size:              buffer size                                             */
/* Registers a call back function to get notified on variable response        */
void vtss_eth_link_oam_client_register_cb(const u32 port_no, void (*cb)(i8 *response), i8 *buf, u32 size);

/* port_no:           port number                                             */
/* Deregisters the call back function to get notified on variable response    */
void vtss_eth_link_oam_client_deregister_cb(const u32 port_no);

/* port_no:           port number                                             */
/* status:            Loopback status                                         */
/*Retrieves the Loopback status in accordance with RFC 4878                   */
u32 vtss_eth_link_oam_client_loopback_oper_status_get(const u32 port_no, vtss_eth_link_oam_loopback_status_t *status);

/******************************************************************************/
/*  ETH Link OAM module inter thread message handlers                         */
/******************************************************************************/
/* This is call by ETH Link OAM upper logic to post event                     */
void vtss_eth_link_oam_message_post(vtss_eth_link_oam_message_t *event);

/* This is call by ETH Link OAM to handle the event                           */
u32 vtss_eth_link_oam_message_handler(vtss_eth_link_oam_message_t *event);

/******************************************************************************/
/*  ETH Link OAM platform call out interface                                  */
/******************************************************************************/

/* port_no:           port number                                             */
/* pdu:               link event OAM pdu                                      */
/* is_error_frame_xmit_needed: Verifies for frame error                       */
/* is_symbol_period_xmit_needed: Verifies for symbol error                    */
/* is_frame_period_xmit_needed:  Verifies for frame period                    */
/* Fills the OAM Link event PDU                                               */
u32 vtss_eth_link_oam_client_link_monitoring_pdu_fill_info_data(const u32 port_no, u8 *const pdu, const BOOL is_error_frame_xmit_needed, const BOOL is_symbol_period_xmit_needed, const BOOL is_frame_period_xmit_needed, const BOOL is_error_frame_secs_summary_xmit_needed);

/* Call out functions to lock/unlock the events                               */
BOOL vtss_eth_link_oam_mib_retrival_opr_lock(void);
void vtss_eth_link_oam_mib_retrival_opr_unlock(void);
BOOL vtss_eth_link_oam_rlb_opr_lock(void);
void vtss_eth_link_oam_rlb_opr_unlock(void);



#endif /* _VTSS_ETH_LINK_OAM_CLIENT_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/


