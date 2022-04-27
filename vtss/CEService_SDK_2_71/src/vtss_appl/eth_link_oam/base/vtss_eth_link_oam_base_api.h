/*

 Vitesse ETH Link OAM software.

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

#ifndef _VTSS_ETH_LINK_OAM_BASE_API_H_
#define _VTSS_ETH_LINK_OAM_BASE_API_H_

#include <stdlib.h>
#include "vtss_types.h"

/******************************************************************************/
/* Eth Link OAM Module error defintions                                       */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_RC_OK                   0
#define VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER    1
#define VTSS_ETH_LINK_OAM_RC_NOT_ENABLED          2
#define VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED   3
#define VTSS_ETH_LINK_OAM_RC_NO_MEMORY            4
#define VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED        5
/*Variable request/response specific errors*/
#define VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY 6
/*Variable request/response specific errors*/
#define VTSS_ETH_LINK_OAM_RC_INVALID_STATE        7
#define VTSS_ETH_LINK_OAM_RC_INVALID_FLAGS        8
#define VTSS_ETH_LINK_OAM_RC_INVALID_CODES        9
#define VTSS_ETH_LINK_OAM_RC_INVALID_PDU_CNT     10
#define VTSS_ETH_LINK_OAM_RC_TIMED_OUT           11

/******************************************************************************/
/* Eth Link OAM Module trace defintions                                       */
/******************************************************************************/
/* This enum should be clone of vtss_trace_level_t enum */
typedef enum {
    VTSS_ETH_LINK_OAM_TRACE_LEVEL_NONE,    /* No trace */
    VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR,   /* Error trace */
    VTSS_ETH_LINK_OAM_TRACE_LEVEL_INFO,    /* Information trace */
    VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,   /* Debug trace */
    VTSS_ETH_LINK_OAM_TRACE_LEVEL_NOISE,   /* More debug information */
    VTSS_ETH_LINK_OAM_TRACE_LEVEL_COUNT    /* Number of trace levels */
} vtss_eth_link_oam_trace_level_t;

/******************************************************************************/
/* Eth Link OAM Port specific definitions                                     */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT   (VTSS_PORT_NO_END)
#define VTSS_ETH_LINK_OAM_CONF_PORT_FIRST  0
#define VTSS_ETH_LINK_OAM_CONF_PORT_LAST   (VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT-1)
#define VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT  (VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT)

/******************************************************************************/
/* OAM operation Control; Section 30.3.6.2                                    */
/******************************************************************************/
typedef enum {
    VTSS_ETH_LINK_OAM_CONTROL_DISABLE, /* Enable OAM operation on the Port */
    VTSS_ETH_LINK_OAM_CONTROL_ENABLE,  /* Disable OAM Operation on the Port */
} vtss_eth_link_oam_control_t;

/******************************************************************************/
/* OAM Mode; Section 30.3.6.1.3                                               */
/******************************************************************************/
typedef enum {
    VTSS_ETH_LINK_OAM_MODE_PASSIVE,   /* Passive Mode of the OAM Operation */
    VTSS_ETH_LINK_OAM_MODE_ACTIVE,    /* Active Mode of the OAM Operation */
} vtss_eth_link_oam_mode_t;

/******************************************************************************/
/* OAM configuration as defined under 30.3.6.1.6                              */
/******************************************************************************/
typedef enum eth_link_oam_capability_conf {
    VTSS_ETH_LINK_OAM_CONF_MODE = 0x1,
    VTSS_ETH_LINK_OAM_CONF_UNI_DIRECTIONAL_SUPPORT = 0x2,
    VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT = 0x4,
    VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT = 0x8,
    VTSS_ETH_LINK_OAM_CONF_VARIABLE_RETRIVEL_SUPPORT = 0x10,
} vtss_eth_link_oam_capability_conf_t ;

/******************************************************************************/
/* Macros to define the Length values                                         */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_MAC_LEN            6
#define VTSS_ETH_LINK_OAM_ETH_TYPE_LEN       2
#define VTSS_ETH_LINK_OAM_FLAGS_LEN          2
#define VTSS_ETH_LINK_OAM_REV_LEN            2
#define VTSS_ETH_LINK_OAM_CONF_LEN           2
#define VTSS_ETH_LINK_OAM_OUI_LEN            3
#define VTSS_ETH_LINK_OAM_VENDOR_INFO_LEN    4
#define VTSS_ETH_LINK_OAM_INFO_TLV_LEN       16
#define VTSS_ETH_LINK_OAM_INFO_DATA_LEN      42

#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TLV            0x01
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV                    0x02
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TLV             0x03
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TLV       0x04

#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_SEQUENCE_NUMBER_LEN       2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TIME_STAMP_LEN            2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_WINDOW_LEN                2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_THRESHOLD_LEN             4
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_ERROR_FRAMES_LEN          4
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_FRAMES_LEN    8
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_EVENTS_LEN    4
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MIN_ERROR_WINDOW          1
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MAX_ERROR_WINDOW          60
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_DEF_ERROR_WINDOW          1
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MIN_ERROR_THRESHOLD       0
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MAX_ERROR_THRESHOLD       0xffffffff
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_DEF_ERROR_THRESHOLD       0

#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_SEQUENCE_NUMBER_LEN     2
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TIME_STAMP_LEN          2
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_WINDOW_LEN              8
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_THRESHOLD_LEN           8
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_ERROR_SYMBOLS_LEN       8
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_SYMBOLS_LEN 8
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN  4
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MIN_ERROR_WINDOW        1
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MAX_ERROR_WINDOW        60
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_DEF_ERROR_WINDOW        1
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MIN_ERROR_THRESHOLD     0
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MAX_ERROR_THRESHOLD     0xffffffff
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_DEF_ERROR_THRESHOLD     0


#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_SEQUENCE_NUMBER_LEN      2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TIME_STAMP_LEN           2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_WINDOW_LEN               4
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_THRESHOLD_LEN            4
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_ERROR_FRAMES_LEN         4
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_FRAMES_LEN   8
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN   4

#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_SEQUENCE_NUMBER_LEN   2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TIME_STAMP_LEN        2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_WINDOW_LEN            2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_THRESHOLD_LEN         2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_ERROR_FRAMES_LEN      2
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_FRAMES_LEN      4
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_EVENTS_LEN      4
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MIN_ERROR_WINDOW      10
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MAX_ERROR_WINDOW      900
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_DEF_ERROR_WINDOW      60
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MIN_ERROR_THRESHOLD   0
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MAX_ERROR_THRESHOLD   0xffff
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_DEF_ERROR_THRESHOLD   1


#define VTSS_ETH_LINK_OAM_ERROR_TLV_HDR_LEN                                 18
#define VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TLV_LEN                 40
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV_LEN                         26
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TLV_LEN                  28
#define VTSS_ETH_LINK_OAM_ERROR_FRAME_SECONDS_EVENT_TLV_LEN                 18

#define VTSS_ETH_LINK_OAM_ERROR_TLV_MAX_LEN         ( \
                      (VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TLV_LEN)+\
                      (VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV_LEN)+\
                      (VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TLV_LEN)+\
                      (VTSS_ETH_LINK_OAM_ERROR_FRAME_SECONDS_EVENT_TLV_LEN)+\
                                                    )

/******************************************************************************/
/* Eth Link OAM type as defined under 57.4.2                                  */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_ETH_TYPE                 0x8809

/******************************************************************************/
/* Eth Link OAM Subtype as defined under 57.4.2                               */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_SUB_TYPE                 0x03
#define VTSS_ETH_LINK_OAM_SUB_TYPE_POS             14
#define VTSS_ETH_LINK_OAM_CODE_POS                 17

/******************************************************************************/
/* Eth Link OAM PDU size as defined under 57.4.2                              */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_PDU_HDR_LEN              18
#define VTSS_ETH_LINK_OAM_PDU_MIN_LEN              64
#define VTSS_ETH_LINK_OAM_PDU_MAX_LEN              1500 /* with 18 bytes coming 
from ethernet headers */
#define VTSS_ETH_LINK_OAM_TAGGED_PDU_MAX_LEN       1522

/******************************************************************************/
/* Eth Link OAM Flags as defined under table 57.3                             */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT       0x01
#define VTSS_ETH_LINK_OAM_FLAG_DYING_GASP       0x02
#define VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT       0x04
#define VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE    0x08
#define VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE     0x10
#define VTSS_ETH_LINK_OAM_FLAG_REMOTE_EVALUTE   0x20
#define VTSS_ETH_LINK_OAM_FLAG_REMOTE_STABLE    0x40

/******************************************************************************/
/* Eth Link OAM Code types as defined in table 57.4                           */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_CODE_TYPE_INFO     0x0           //Information Type
#define VTSS_ETH_LINK_OAM_CODE_TYPE_EVENT    0x1           //Error events
#define VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_REQ  0x2           //Variable requests
#define VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_RESP 0x3           //Variable responses
#define VTSS_ETH_LINK_OAM_CODE_TYPE_LB       0x4           //Loop back
#define VTSS_ETH_LINK_OAM_CODE_TYPE_ORG      0xFE          //Organization Specific

/* Only above types are handlded */
#define VTSS_ETH_LINK_OAM_SUPPORTED_CODES_CNT 6

/******************************************************************************/
/* Eth Link OAM Info TLV types as defined in table 57.6                       */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_LOCAL_INFO_TLV   0x01         //Local info TLV
#define VTSS_ETH_LINK_OAM_REMOTE_INFO_TLV  0x02         //Remote info TLV
#define VTSS_ETH_LINK_OAM_VENDOR_INFO_TLV  0xFE         //Vendor specific TLV

/* Current supported Version */
#define VTSS_ETH_LINK_OAM_PROTOCOL_VERSION 0x01

/******************************************************************************/
/* Eth Link OAM timers as defined under section 57.3.1.5                      */
/******************************************************************************/
/*Periodic time interval */
#define VTSS_ETH_LINK_OAM_PDU_TIMER               1
/*Wait time interval to reset the OAM */
#define VTSS_ETH_LINK_OAM_LOCAL_LOST_LINK_TIMER   5
/*Max number of PDUs to be Xmited */
#define VTSS_ETH_LINK_OAM_PORT_PDU_CNT            10

/******************************************************************************/
/*Link OAM Discovery State machine states as defined with 57.5 state machine  */
/******************************************************************************/
typedef enum eth_link_oam_discovery_state {
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT, /* State S1 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_ACTIVE_SEND_LOCAL, /* State S2 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_PASSIVE_WAIT, /* State S3 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE, /* State S4 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE_OK, /* State S5 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY, /* State S6 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_LAST,
} vtss_eth_link_oam_discovery_state_t;

/******************************************************************************/
/* local_pdu control as specified in table 57.7                               */
/******************************************************************************/
typedef enum eth_link_oam_pdu_control {
    VTSS_ETH_LINK_OAM_PDU_CONTROL_RX_INFO,
    VTSS_ETH_LINK_OAM_PDU_CONTROL_LF_INFO,
    VTSS_ETH_LINK_OAM_PDU_CONTROL_INFO,
    VTSS_ETH_LINK_OAM_PDU_CONTROL_ANY,
} vtss_eth_link_oam_pdu_control_t;

/******************************************************************************/
/* Link OAM MUX states as specified in table 57.7                             */
/******************************************************************************/
typedef enum eth_oam_mux_state {
    VTSS_ETH_LINK_OAM_MUX_FWD_STATE,
    VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE,
} vtss_eth_link_oam_mux_state_t;

/*******************************************************************************/
/* Link OAM Parser states as specified in table 57.7                           */
/*******************************************************************************/
typedef enum eth_oam_parser_state {
    VTSS_ETH_LINK_OAM_PARSER_FWD_STATE,
    VTSS_ETH_LINK_OAM_PARSER_LB_STATE,
    VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE,
} vtss_eth_link_oam_parser_state_t;

/*******************************************************************************/
/* Link OAM Link Event TLV type value in table 57.12                           */
/*******************************************************************************/

#define VTSS_ETH_LINK_OAM_LINK_MONITORING_END_OF_TLV_MARKER            0x00
#define VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_SYMBOL_PERIOD_EVENT    0x01
#define VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_EVENT            0x02
#define VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_PERIOD_EVENT     0x03

#define VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_SECS_SUMMARY_EVENT  0x04

#define VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_EVENT_LEN        0x1a
#define VTSS_ETH_LINK_OAM_LINK_MONITORING_SYMBOL_PERIOD_EVENT_LEN      0x28
#define VTSS_ETH_LINK_OAM_LINK_MONITORING_FRAME_PERIOD_EVENT_LEN       0x1c

#define VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_SECS_SUM_EVENT_LEN  0x12

/******************************************************************************/
/* Loop back Enable/Disable request and responses                             */
/******************************************************************************/
#define VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_ENABLE_REQUEST    1
#define VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_DISABLE_REQUEST   2
#define VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_REQUEST_MAX_LEN   1

/******************************************************************************/
/* Variable response/request Macros                                           */
/******************************************************************************/

#define VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN                1
#define VTSS_ETH_LINK_OAM_VAR_LEAF_LEN                  2
#define VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_LEN       1

#define VTSS_ETH_LINK_OAM_VAR_DESCRIPTOR_LEN            (\
                                        (VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN) +\
                                        (VTSS_ETH_LINK_OAM_VAR_LEAF_LEN)\
                                                        )

#define VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN         (\
                                         (VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN) + \
                                         (VTSS_ETH_LINK_OAM_VAR_LEAF_LEN) + \
                               (VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_LEN)\
                                         )

#define VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_MAX_LEN   128
#define VTSS_ETH_LINK_OAM_VAR_INDICATOR_VAL             128
#define VTSS_ETH_LINK_OAM_RESPONSE_BUF                  1518

/* Variable branch types */
#define VTSS_ETH_LINK_OAM_VAR_OBJECT     0x03
#define VTSS_ETH_LINK_OAM_VAR_PACKAGE    0x04
#define VTSS_ETH_LINK_OAM_VAR_ATTRIBUTE  0x07

/* Link OAM object leaf identifires */
#define VTSS_ETH_LINK_OAM_OBJECT_ID      0x14

/*Link OAM packages leaf identifiers,Currently Link OAM has no packages*/

/* Link OAM attribute identifiers */
#define VTSS_ETH_LINK_OAM_ID                               236
#define VTSS_ETH_LINK_OAM_ID_LEN                           4

#define VTSS_ETH_LINK_OAM_ADMIN_STATE                      237
#define VTSS_ETH_LINK_OAM_ADMIN_STATE_LEN                  4

#define VTSS_ETH_LINK_OAM_MODE                             238
#define VTSS_ETH_LINK_OAM_MODE_LEN                         4

#define VTSS_ETH_LINK_OAM_REMOTE_MAC_ADDR                  239
#define VTSS_ETH_LINK_OAM_REMOTE_MAC_ADDR_LEN              6

#define VTSS_ETH_LINK_OAM_REMOTE_CONF                      240
#define VTSS_ETH_LINK_OAM_REMOTE_CONF_LEN                  1

#define VTSS_ETH_LINK_OAM_REMOTE_PDU_CONF                  241
#define VTSS_ETH_LINK_OAM_REMOTE_PDU_CONF_LEN              4

#define VTSS_ETH_LINK_OAM_LOCAL_FLAGS                      242
#define VTSS_ETH_LINK_OAM_LOCAL_FLAGS_LEN                  1

#define VTSS_ETH_LINK_OAM_REMOTE_FLAGS                     243
#define VTSS_ETH_LINK_OAM_REMOTE_FLAGS_LEN                 1

#define VTSS_ETH_LINK_OAM_REMOTE_REVISION                  244
#define VTSS_ETH_LINK_OAM_REMOTE_REVISION_LEN              4

#define VTSS_ETH_LINK_OAM_REMOTE_STATE                     245
#define VTSS_ETH_LINK_OAM_REMOTE_STATE_LEN                 1

#define VTSS_ETH_LINK_OAM_REMOTE_VENDOR_OUI                246
#define VTSS_ETH_LINK_OAM_REMOTE_VENDOR_OUI_LEN            3

#define VTSS_ETH_LINK_OAM_REMOTE_VENDOR_SPECIFIC_INFO      247
#define VTSS_ETH_LINK_OAM_REMOTE_VENDOR_SPECIFIC_INFO_LEN  4

#define VTSS_ETH_LINK_OAM_UNSUPPORTED_CODES_RX             250
#define VTSS_ETH_LINK_OAM_UNSUPPORTED_CODES_RX_LEN         4

#define VTSS_ETH_LINK_OAM_INFO_TX                          251
#define VTSS_ETH_LINK_OAM_INFO_TX_LEN                      4
#define VTSS_ETH_LINK_OAM_INFO_RX                          252
#define VTSS_ETH_LINK_OAM_INFO_RX_LEN                      4

#define VTSS_ETH_LINK_OAM_UNIQUE_EVENT_NOTIFICATION_RX     254
#define VTSS_ETH_LINK_OAM_UNIQUE_EVENT_NOTIFICATION_RX_LEN 4
#define VTSS_ETH_LINK_OAM_DUPLICATE_EVENT_NOTIFICATION_RX  255
#define VTSS_ETH_LINK_OAM_DUPLICATE_EVENT_NOTIFICATION_RX_LEN 4

#define VTSS_ETH_LINK_OAM_LB_CONTROL_TX                    256
#define VTSS_ETH_LINK_OAM_LB_CONTROL_TX_LEN                4
#define VTSS_ETH_LINK_OAM_LB_CONTROL_RX                    257
#define VTSS_ETH_LINK_OAM_LB_CONTROL_RX_LEN                4

#define VTSS_ETH_LINK_OAM_VAR_REQ_TX                       258
#define VTSS_ETH_LINK_OAM_VAR_REQ_TX_LEN                   4
#define VTSS_ETH_LINK_OAM_VAR_REQ_RX                       259
#define VTSS_ETH_LINK_OAM_VAR_REQ_RX_LEN                   4

#define VTSS_ETH_LINK_OAM_VAR_RESP_TX                      260
#define VTSS_ETH_LINK_OAM_VAR_RESP_TX_LEN                  4
#define VTSS_ETH_LINK_OAM_VAR_RESP_RX                      261
#define VTSS_ETH_LINK_OAM_VAR_RESP_RX_LEN                  4

#define VTSS_ETH_LINK_OAM_ORG_SPECIFIC_TX                  262
#define VTSS_ETH_LINK_OAM_ORG_SPECIFIC_TX_LEN              4
#define VTSS_ETH_LINK_OAM_ORG_SPECIFIC_RX                  263
#define VTSS_ETH_LINK_OAM_ORG_SPECIFIC_RX_LEN              4

#define VTSS_ETH_LINK_OAM_DISCOVERY_STATE                  333
#define VTSS_ETH_LINK_OAM_DISCOVERY_STATE_LEN              4

#define VTSS_ETH_LINK_OAM_LOCAL_CONF                       334
#define VTSS_ETH_LINK_OAM_LOCAL_CONF_LEN                   1

#define VTSS_ETH_LINK_OAM_LOCAL_PDU_CONF                   335
#define VTSS_ETH_LINK_OAM_LOCAL_PDU_CONF_LEN               4

#define VTSS_ETH_LINK_OAM_LOCAL_REVISION_CONF              336
#define VTSS_ETH_LINK_OAM_LOCAL_REVISION_CONF_LEN          4

#define VTSS_ETH_LINK_OAM_LOCAL_STATE                      337
#define VTSS_ETH_LINK_OAM_LOCAL_STATE_LEN                  1

#define VTSS_ETH_LINK_OAM_UNSUPPORTED_CODES_TX             338
#define VTSS_ETH_LINK_OAM_UNSUPPORTED_CODES_TX_LEN         4

#define VTSS_ETH_LINK_OAM_UNIQUE_EVENT_NOTIFICATION_TX     339
#define VTSS_ETH_LINK_OAM_UNIQUE_EVENT_NOTIFICATION_TX_LEN 4

#define VTSS_ETH_LINK_OAM_DUPLICATE_EVENT_NOTIFICATION_TX  340
#define VTSS_ETH_LINK_OAM_DUPLICATE_EVENT_NOTIFICATION_TX_LEN 4

/* Var indicators definitions */
#define VTSS_ETH_LINK_OAM_VAR_PDU_LEN_IS_NOT_SUFFICINET         0x01

/* Attribute specific Var indicators definitions */
#define VTSS_ETH_LINK_OAM_VAR_ATTR_NOT_SUPPORTED                0x21

/* Object specific Var indicators definitions    */
#define VTSS_ETH_LINK_OAM_VAR_OBJ_NOT_SUPPORTED                 0x42

/* Package specific Var indicators definitions   */
#define VTSS_ETH_LINK_OAM_VAR_PACK_NOT_SUPPORTED                0x62


/******************************************************************************/
/* Eth Link OAM header definition                                             */
/******************************************************************************/
typedef struct {
    u8                  dst_mac[VTSS_ETH_LINK_OAM_MAC_LEN];
    u8                  src_mac[VTSS_ETH_LINK_OAM_MAC_LEN];
    u8                  eth_type[VTSS_ETH_LINK_OAM_ETH_TYPE_LEN];
    u8                  subtype;
    u8                  flags[VTSS_ETH_LINK_OAM_FLAGS_LEN];
    u8                  code;
} vtss_eth_link_oam_frame_header_t;

/***********************************************************************************/
/* Eth Link OAM information TLV                                                    */
/***********************************************************************************/
typedef struct {
    u8 info_type;
    u8 info_len;
    u8 version;
    u8 revision[VTSS_ETH_LINK_OAM_REV_LEN];
    u8 state;
    u8 oam_conf;
    u8 oampdu_conf[VTSS_ETH_LINK_OAM_CONF_LEN];
    u8 oui[VTSS_ETH_LINK_OAM_OUI_LEN];
    u8 vendor_info[VTSS_ETH_LINK_OAM_VENDOR_INFO_LEN];
} vtss_eth_link_oam_info_tlv_t;

typedef struct vtss_eth_link_oam_error_symbol_period_event_tlv {
    u8 event_type;
    u8 event_length;
    u8 event_time_stamp[VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TIME_STAMP_LEN];
    u8 error_symbol_window[VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_WINDOW_LEN];
    u8 error_symbol_threshold[VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_THRESHOLD_LEN];
    u8 error_symbols[VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_ERROR_SYMBOLS_LEN];
    u8 error_running_total[VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_SYMBOLS_LEN];
    u8 event_running_total[VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN];
} vtss_eth_link_oam_error_symbol_period_event_tlv_t;

typedef struct vtss_eth_link_oam_error_frame_event_tlv {
    u8 event_type;
    u8 event_length;
    u8 event_time_stamp[VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TIME_STAMP_LEN];
    u8 error_frame_window[VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_WINDOW_LEN];
    u8 error_frame_threshold[VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_THRESHOLD_LEN];
    u8 error_frames[VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_ERROR_FRAMES_LEN];
    u8 error_running_total[VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_FRAMES_LEN];
    u8 event_running_total[VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_EVENTS_LEN];
} vtss_eth_link_oam_error_frame_event_tlv_t;

typedef struct vtss_eth_link_oam_error_frame_period_event_tlv {
    u8 event_type;
    u8 event_length;
    u8 event_time_stamp[VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TIME_STAMP_LEN];
    u8 error_frame_period_window[VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_WINDOW_LEN];
    u8 error_frame_threshold[VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_THRESHOLD_LEN];
    u8 error_frames[VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_ERROR_FRAMES_LEN];
    u8 error_running_total[VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_FRAMES_LEN];
    u8 event_running_total[VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN];
} vtss_eth_link_oam_error_frame_period_event_tlv_t;

typedef struct vtss_eth_link_oam_error_frame_secs_summary_event_tlv {
    u8 event_type;
    u8 event_length;
    u8 event_time_stamp[VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TIME_STAMP_LEN];
    u8 secs_summary_window[VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_WINDOW_LEN];
    u8 secs_summary_threshold[VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_THRESHOLD_LEN];
    u8 secs_summary_events[VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_ERROR_FRAMES_LEN];
    u8 error_running_total[VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_FRAMES_LEN];
    u8 event_running_total[VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_EVENTS_LEN];
} vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t;


/* All the counters are 32-bit as specified in the standard */
typedef struct eth_link_oam_pdu_stats {
    u32 unsupported_codes_tx;
    u32 unsupported_codes_rx;
    u32 information_rx;
    u32 information_tx;
    u32 unique_event_notification_tx;
    u32 unique_event_notification_rx;
    u32 duplicate_event_notification_tx;
    u32 duplicate_event_notification_rx;
    u32 loopback_control_tx;
    u32 loopback_control_rx;
    u32 variable_request_tx;
    u32 variable_request_rx;
    u32 variable_response_tx;
    u32 variable_response_rx;
    u32 org_specific_rx;
    u32 org_specific_tx;
} vtss_eth_link_oam_pdu_stats_t;

/* The following are non-standard stats for critical events.
   So keeping them in different data structures     */
typedef struct eth_link_oam_critical_event_pdu_stats {
    u32 link_fault_tx;
    u32 dying_gasp_tx;
    u32 critical_event_tx;
    u32 link_fault_rx;
    u32 dying_gasp_rx;
    u32 critical_event_rx;
} vtss_eth_link_oam_critical_event_pdu_stats_t;


/*******************************************************************************/
/* Eth Link OAM utility Macros                                                 */
/*******************************************************************************/
#define IS_CONF_ACTIVE(byte,bitpos)                  (byte&bitpos)
#define IS_FLAG_CONF_ACTIVE(flags,flag_pos)          (flags&flag_pos)
#define IS_PDU_CONF_ACTIVE(oam_conf,conf_pos)        (oam_conf&conf_pos)

/* To check the NULL conditions */
#define VTSS_ETH_LINK_OAM_NULL                       0
#define VTSS_ETH_LINK_OAM_INVALID_FLAGS              0xff80

/******************************************************************************/
/* Eth Link OAM call out function prototypes.                                 */
/* These functions are need to be defined by the platform code for proper     */
/* functionality of the Link OAM.                                             */
/******************************************************************************/

/* port_no:         port number                                               */
/* data:            port's Information PDU data                               */
/* Initializes the port's information data                                    */
u32 vtss_eth_link_oam_control_port_conf_init(const u32 port_no, const u8 *data);

/* port_no:         port number                                               */
/* is_port_active:  port's OAM mode                                           */
/* Initializes the port's discovery protocol with specified mode              */
u32 vtss_eth_link_oam_control_port_oper_init(const u32 port_no, BOOL is_port_active);

/* port_no:         port number                                               */
/* data:            port's Information PDU data                               */
/* reset_port_oper: Flag to reset the discovery proctocol                     */
/* is_port_active:  port's OAM mode                                           */
/* Modifies the Port's information data and resets the discovery protocol     */
u32 vtss_eth_link_oam_control_port_data_set(const u32 port_no, const u8 *data, BOOL  reset_port_oper, BOOL  is_port_active);
/* port_no:         port number                                               */
/* oam_control:     port's OAM control configuration                          */
/* Sets the port's OAM Control flag of the port                               */
u32 vtss_eth_link_oam_control_port_control_conf_set(const u32 port_no, const u8 oam_control);

/* port_no:         port number                                               */
/* oam_control:     port's OAM control configuration                          */
/* Retrives the port's OAM Control flag of the port                           */
u32 vtss_eth_link_oam_control_port_control_conf_get(const u32 port_no, u8 *oam_control);

/* port_no:               port number                                               */
/* parser_state:          port's parser state                                       */
/* oam_ace_id:            port's ACE id to allow OAM PDU only                       */
/* lb_ace_id:             port's ACE id to deny rest all traffic                    */
/* update_mux_state_only: Flag to indicate to update only mux state                 */
vtss_rc vtss_eth_link_oam_control_port_mux_parser_conf_set(const u32 port_no, const vtss_eth_link_oam_mux_state_t  mux_state, const vtss_eth_link_oam_parser_state_t parser_state, u32 *oam_ace_id, u32 *lb_ace_id, const BOOL update_mux_state_only);
/* port_no:         port number                                               */
/* mux_state:       port's parser state                                       */
/* Retrievs the port's parser state                                           */
vtss_rc vtss_eth_link_oam_control_port_parser_conf_get(const u32 port_no, vtss_eth_link_oam_parser_state_t *parser_state);

/* port_no:         port number                                               */
/* flag:            OAM flag                                                  */
/* enable_flag:     enable/disable the specified flag                         */
/* Sets the port's OAM Flags like link_fault,dying_gasp                       */
u32 vtss_eth_link_oam_control_port_flags_conf_set(const u32 port_no, const u8 flag, const BOOL enable_flag);

/* port_no:         port number                                               */
/* flag:            OAM flag                                                  */
/* Retrives the port's OAM Flags like link_fault,dying_gasp                   */
u32 vtss_eth_link_oam_control_port_flags_conf_get(const u32 port_no, u8 *flag);

/* port_no:         port number                                               */
/* Sets the port's OAM remote state valid flag                                */
u32 vtss_eth_link_oam_control_port_remote_state_valid_set(const u32 port_no);

/* port_no:         port number                                               */
/* is_local_satisfied: local_satisfied flag                                   */
/* Sets the port's OAM local satisfied flag                                   */
u32 vtss_eth_link_oam_control_port_local_satisfied_set(const u32 port_no, const BOOL is_local_satisfied);

/* port_no:         port number                                               */
/* is_remote_stable: remote stable flag                                       */
/* Sets the port's remote satble flag                                         */
u32 vtss_eth_link_oam_control_port_remote_stable_set(const u32 port_no, const BOOL is_remote_stable);

/* port_no:         port number                                               */
/* oam_client_pdu:  client pdu to be transmitted                              */
/* Builds and transmits the info OAM pdu to be transmitted                    */
u32 vtss_eth_link_oam_control_port_info_pdu_xmit(const u32 port_no);

/* port_no:         port number                                               */
/* oam_client_pdu:  client pdu to be transmitted                              */
/* oam_pdu_len:     OAM PDU length to be transmitted                          */
/* oam_pdu_code:    OAM PDU code to be transmitted                            */
/* Builds and transmits the non-info OAM pdu to be transmitted                */
u32 vtss_eth_link_oam_control_port_non_info_pdu_xmit(const u32 port_no, u8  *oam_client_pdu, const u16 oam_pdu_len, const u8 oam_pdu_code);

/* port_no:         port number                                               */
/* oam_code:        oam code to be supported                                  */
/* support_enable:  enable/disable the support for the specified code         */
/* Enables/Disables the support for the specified oam code                    */
u32 vtss_eth_link_oam_control_supported_code_conf_set (const u32 port_no, const u8 oam_code, const BOOL support_enable);

/* port_no:         port number                                               */
/* oam_code:        oam code to be supported                                  */
/* support_enable:  support configuration for the specified oam code          */
/* Retrives the support configuration for the specified oam code              */
u32 vtss_eth_link_oam_control_supported_code_conf_get (const u32 port_no, const u8 oam_code, BOOL *support_enable);
/* port_no:         port number                                               */
/* Resets the local last timer to standard value on reception of OAM pdu      */
u32 vtss_eth_link_oam_control_port_local_lost_timer_conf_set(const u32 port_no);

/* port_no:         port number                                               */
/* pdu_control:     port's PDU control                                        */
/* retrives the PDU control of the port                                       */
u32 vtss_eth_link_oam_control_port_pdu_control_status_get(const u32 port_no, vtss_eth_link_oam_pdu_control_t *const pdu_control);

/* port_no:    port number                                                    */
/* state:      discovery state                                                */
/* Retrives the port's discovery state                                        */
u32  vtss_eth_link_oam_control_port_discovery_state_get(const u32 port_no, vtss_eth_link_oam_discovery_state_t *state);

/* port_no:    port number                                                    */
/* pdu_stats:  port's pdu statistics                                          */
/* Retrives the port's standard PDU statistics                                */
u32  vtss_eth_link_oam_control_port_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_pdu_stats_t *pdu_stats);

/* port_no:    port number                                                    */
/* pdu_stats:  critical event pdu statistics                                  */
/* Retrives the port's critical event statistics                              */
u32 vtss_eth_link_oam_control_port_critical_event_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_critical_event_pdu_stats_t *pdu_stats);

/* port_no:         port number                                               */
/* Resets the port counters                                                   */
u32 vtss_eth_link_oam_control_clear_statistics(const u32 port_no);

/* port_no:         port number                                               */
/* oam_pdu:         OAM pdu                                                   */
/* pdu_len:         length of the pdu to be transmitted                       */
/* oam_code:        code of the received pdu                                  */
/* Handles the received function                                              */
u32 vtss_eth_link_oam_control_rx_pdu_handler(const u32 port_no, const u8 *pdu, const u16 pdu_len, const u8 oam_code);



/******************************************************************************/
/* Call out functions for htonl/s, ntohl/s and 64-bit swap functionalities    */
/******************************************************************************/
u16 vtss_eth_link_oam_htons(u16 host_short);
u16 vtss_eth_link_oam_ntohs(u16 net_short);

u32 vtss_eth_link_oam_htonl(u32 host_long);
u32 vtss_eth_link_oam_ntohl(u32 net_long);

u64 vtss_eth_link_oam_swap64(u64 num);

u16 vtss_eth_link_oam_ntohs_from_bytes(u8 tmp[]);
u32 vtss_eth_link_oam_ntohl_from_bytes(u8 tmp[]);
u64 vtss_eth_link_oam_swap64_from_bytes(u8 num[]);

u32 vtss_is_port_info_get(const u32 port_no, BOOL *is_up);

void *vtss_eth_link_oam_malloc(size_t sz);

/*****************************************************************************/
/* Call out functions to support module specific trace functions             */
/*****************************************************************************/
void vtss_eth_link_oam_trace(const vtss_eth_link_oam_trace_level_t trace_level, const char *string, const u32   param1, const u32 param2, const u32 param3, const u32   param4);

#endif /* _VTSS_ETH_LINK_OAM_BASE_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

