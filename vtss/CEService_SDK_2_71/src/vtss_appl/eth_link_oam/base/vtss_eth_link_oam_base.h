/*

 Vitesse ETH Link OAM software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_ETH_LINK_OAM_BASE_H_
#define _VTSS_ETH_LINK_OAM_BASE_H_

#include "vtss_types.h"
#include "l2proto_api.h"                /* For port specific Macros  */
#include "vtss_common_os.h"             /* it should not be here     */
#include "netdb.h"

/* Eth Link OAM Module error defintions */
#define VTSS_ETH_LINK_OAM_RC_OK                   0
#define VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER    1
#define VTSS_ETH_LINK_OAM_RC_NOT_ENABLED          2
#define VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED   3
#define VTSS_ETH_LINK_OAM_RC_NO_MEMORY            4
#define VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED        5
#define VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY 6 //Variable request/response specific errors
#define VTSS_ETH_LINK_OAM_RC_INVALID_STATE        7 //Variable request/response specific errors
#define VTSS_ETH_LINK_OAM_RC_INVALID_FLAGS        8
#define VTSS_ETH_LINK_OAM_RC_INVALID_CODES        9

/* Eth Link OAM Port specific definitions */
#define VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT          (VTSS_PORT_NO_END)
#define VTSS_ETH_LINK_OAM_CONF_PORT_FIRST         0
#define VTSS_ETH_LINK_OAM_CONF_PORT_LAST          (VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT-1)
#define VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT         (VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT)

/***********************************************************************************/
/* OAM operation Control; Section 30.3.6.2                                         */
/***********************************************************************************/
typedef enum {
    VTSS_ETH_LINK_OAM_CONTROL_DISABLE, /* Enable OAM operation on the Port */
    VTSS_ETH_LINK_OAM_CONTROL_ENABLE,  /* Disable OAM Operation on the Port */
} vtss_eth_link_oam_control_t;

/***********************************************************************************/
/* OAM Mode; Section 30.3.6.1.3                                                    */
/***********************************************************************************/
typedef enum {
    VTSS_ETH_LINK_OAM_MODE_PASSIVE,   /* Passive Mode of the OAM Operation */
    VTSS_ETH_LINK_OAM_MODE_ACTIVE,    /* Active Mode of the OAM Operation */
} vtss_eth_link_oam_mode_t;

/***********************************************************************************/
/* OAM configuration as defined under 30.3.6.1.6 */
/***********************************************************************************/
typedef enum eth_link_oam_capability_conf {
    VTSS_ETH_LINK_OAM_CONF_MODE = 1,
    VTSS_ETH_LINK_OAM_CONF_UNI_DIRECTIONAL_SUPPORT = 2,
    VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT = 4,
    VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT = 8,
    VTSS_ETH_LINK_OAM_CONF_VARIABLE_RETRIVEL_SUPPORT = 16,
} vtss_eth_link_oam_capability_conf_t ;

/***********************************************************************************/
/* Macros to define the Length values                                              */
/***********************************************************************************/
#define VTSS_ETH_LINK_OAM_MAC_LEN            6
#define VTSS_ETH_LINK_OAM_ETH_TYPE_LEN       2
#define VTSS_ETH_LINK_OAM_FLAGS_LEN          2
#define VTSS_ETH_LINK_OAM_REV_LEN            2
#define VTSS_ETH_LINK_OAM_CONF_LEN           2
#define VTSS_ETH_LINK_OAM_OUI_LEN            3
#define VTSS_ETH_LINK_OAM_VENDOR_INFO_LEN    4

#define VTSS_ETH_LINK_OAM_INFO_TLV_LEN       16
#define VTSS_ETH_LINK_OAM_INFO_DATA_LEN      42

/***********************************************************************************/
/* Eth Link OAM type as defined under 57.4.2                                       */
/***********************************************************************************/
#define VTSS_ETH_LINK_OAM_ETH_TYPE                 0x8809

/***********************************************************************************/
/* Eth Link OAM Subtype as defined under 57.4.2                                    */
/***********************************************************************************/
#define VTSS_ETH_LINK_OAM_SUB_TYPE                 0x03

/***********************************************************************************/
/* Eth Link OAM PDU size as defined under 57.4.2                                   */
/***********************************************************************************/
#define VTSS_ETH_LINK_OAM_PDU_HDR_LEN              18
#define VTSS_ETH_LINK_OAM_PDU_MIN_LEN              64  //FCS is not included
#define VTSS_ETH_LINK_OAM_PDU_MAX_LEN              1516

/***********************************************************************************/
/* Eth Link OAM Flags as defined under table 57.3                                  */
/***********************************************************************************/
#define VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT       0x01
#define VTSS_ETH_LINK_OAM_FLAG_DYING_GASP       0x02
#define VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT       0x04
#define VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE    0x08
#define VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE     0x10
#define VTSS_ETH_LINK_OAM_FLAG_REMOTE_EVALUTE   0x20
#define VTSS_ETH_LINK_OAM_FLAG_REMOTE_STABLE    0x40

/***********************************************************************************/
/* Eth Link OAM Code types as defined in table 57.4                                */
/***********************************************************************************/
#define VTSS_ETH_LINK_OAM_CODE_TYPE_INFO     0x0           //Information Type
#define VTSS_ETH_LINK_OAM_CODE_TYPE_EVENT    0x1            //Error events
#define VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_REQ  0x2            //Variable requests
#define VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_RESP 0x3            //Variable responses
#define VTSS_ETH_LINK_OAM_CODE_TYPE_LB       0x4           //Loop back
#define VTSS_ETH_LINK_OAM_CODE_TYPE_ORG      0xFE          //Organization Specific

#define VTSS_ETH_LINK_OAM_SUPPORTED_CODES_CNT 6            //Only above types are handlded 

/***********************************************************************************/
/* Eth Link OAM Info TLV types as defined in table 57.6                            */
/***********************************************************************************/
#define VTSS_ETH_LINK_OAM_LOCAL_INFO_TLV   0x01         //Local info TLV
#define VTSS_ETH_LINK_OAM_REMOTE_INFO_TLV  0x02         //Remote info TLV
#define VTSS_ETH_LINK_OAM_VENDOR_INFO_TLV  0xFE         //Vendor specific TLV

#define VTSS_ETH_LINK_OAM_PROTOCOL_VERSION 0x01         //Current supported Version

/***********************************************************************************/
/* Eth Link OAM timers as defined under section 57.3.1.5                           */
/***********************************************************************************/
#define VTSS_ETH_LINK_OAM_PDU_TIMER                   1  //Periodic time interval
#define VTSS_ETH_LINK_OAM_LOCAL_LOST_LINK_TIMER       10 //Wait time interval to reset the OAM 

/**********************************************************************************/
/*Link OAM Discovery State machine states as defined with 57.5 state machine      */
/**********************************************************************************/
typedef enum eth_link_oam_discovery_state {
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT, /* State S1 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_ACTIVE_SEND_LOCAL, /* State S2 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_PASSIVE_WAIT, /* State S3 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE, /* State S4 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE_OK, /* State S5 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY, /* State S6 */
    VTSS_ETH_LINK_OAM_DISCOVERY_STATE_LAST,
} vtss_eth_link_oam_discovery_state_t;

/*************************************************************************************/
/* local_pdu control as specified in table 57.7                                      */
/*************************************************************************************/
typedef enum eth_link_oam_pdu_control {
    VTSS_ETH_LINK_OAM_PDU_CONTROL_RX_INFO,
    VTSS_ETH_LINK_OAM_PDU_CONTROL_LF_INFO,
    VTSS_ETH_LINK_OAM_PDU_CONTROL_INFO,
    VTSS_ETH_LINK_OAM_PDU_CONTROL_ANY,
} vtss_eth_link_oam_pdu_control_t;

/*************************************************************************************/
/* Link OAM MUX states as specified in table 57.7                                    */
/*************************************************************************************/

typedef enum eth_oam_mux_state {
    VTSS_ETH_LINK_OAM_MUX_FWD_STATE,
    VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE,
} vtss_eth_link_oam_mux_state_t;

/*************************************************************************************/
/* Link OAM Parser states as specified in table 57.7                                 */
/*************************************************************************************/

typedef enum eth_oam_parser_state {
    VTSS_ETH_LINK_OAM_PARSER_FWD_STATE,
    VTSS_ETH_LINK_OAM_PARSER_LB_STATE,
    VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE,
} vtss_eth_link_oam_parser_state_t;

/*************************************************************************************/
/* Loop back Enable/Disable request and responses                                    */
/*************************************************************************************/
#define VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_ENABLE_REQUEST    1
#define VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_DISABLE_REQUEST   2

#define VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_REQUEST_MAX_LEN   1

/*************************************************************************************/
/* Variable response/request Macros                                                  */
/*************************************************************************************/

#define VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN                1
#define VTSS_ETH_LINK_OAM_VAR_LEAF_LEN                  2
#define VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_LEN       1

#define VTSS_ETH_LINK_OAM_VAR_DESCRIPTOR_LEN            ( (VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN) +\
                                                          (VTSS_ETH_LINK_OAM_VAR_LEAF_LEN))

#define VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN         ( (VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN) + \
                                                          (VTSS_ETH_LINK_OAM_VAR_LEAF_LEN) + \
                                                          (VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_LEN))

#define VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_MAX_LEN   128
#define VTSS_ETH_LINK_OAM_VAR_INDICATOR_VAL             128
#define VTSS_ETH_LINK_OAM_RESPONSE_BUF                  1518

//Variable branch types
#define VTSS_ETH_LINK_OAM_VAR_OBJECT     0x03
#define VTSS_ETH_LINK_OAM_VAR_PACKAGE    0x04
#define VTSS_ETH_LINK_OAM_VAR_ATTRIBUTE  0x07

//Link OAM object leaf identifires.
#define VTSS_ETH_LINK_OAM_OBJECT_ID      0x14 //20

//Link OAM packages leaf identifiers
//Currently Link OAM has no packages

//Link OAM attribute identifiers
#define VTSS_ETH_LINK_OAM_ID                               236
#define VTSS_ETH_LINK_OAM_ID_LEN                           4    //Length in bytes

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

//Var indicators definitions
#define VTSS_ETH_LINK_OAM_VAR_PDU_LEN_IS_NOT_SUFFICINET         0x01

//Attribute specific Var indicators definitions
#define VTSS_ETH_LINK_OAM_VAR_ATTR_NOT_SUPPORTED                0x21

//Object specific Var indicators definitions
#define VTSS_ETH_LINK_OAM_VAR_OBJ_NOT_SUPPORTED                 0x42

//Package specific Var indicators definitions
#define VTSS_ETH_LINK_OAM_VAR_PACK_NOT_SUPPORTED                0x62


/***********************************************************************************/
/* Eth Link OAM header definition                                                  */
/***********************************************************************************/
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

/***********************************************************************************/
/* Eth Link OAM utility Macros                                                     */
/***********************************************************************************/
#define IS_CONF_ACTIVE(byte,bitpos)                  (byte&bitpos)
#define VTSS_ETH_LINK_OAM_NULL                       0 //To check the NULL conditions
#define IS_FLAG_CONF_ACTIVE(flags,flag_pos)          (flags&flag_pos)
#define IS_PDU_CONF_ACTIVE(oam_conf,conf_pos)        (oam_conf&conf_pos)



#endif /* _VTSS_ETH_LINK_OAM_BASE_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

