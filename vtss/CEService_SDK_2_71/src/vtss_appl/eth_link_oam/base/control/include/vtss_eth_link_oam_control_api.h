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

#ifndef _VTSS_ETH_LINK_OAM_CONTROL_API_H_
#define _VTSS_ETH_LINK_OAM_CONTROL_API_H_

#include "vtss_eth_link_oam_base_api.h"  /* For link OAM base configurations  */

/******************************************************************************/
/* OAM Discovery State machine states as defined with 57.5 state machine      */
/******************************************************************************/
#define S1 VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT               /* State S1 */
#define S2 VTSS_ETH_LINK_OAM_DISCOVERY_STATE_ACTIVE_SEND_LOCAL   /* State S2 */
#define S3 VTSS_ETH_LINK_OAM_DISCOVERY_STATE_PASSIVE_WAIT        /* State S3 */
#define S4 VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE   /* State S4 */
#define S5 VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE_OK /* State S5 */
#define S6 VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY          /* State S6 */
#define S7 VTSS_ETH_LINK_OAM_DISCOVERY_STATE_LAST              /* Last state */

/******************************************************************************/
/* OAM Discovery State machine events as defined with 57.5 state machine      */
/******************************************************************************/
typedef enum eth_oam_discovery_event {
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_BEGIN, /* Event E1 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LINK_STATUS_FAIL, /* Event E2 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_TIMER_DONE, /* Event E3 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_ACTIVE_MODE, /* Event E4 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_PASSIVE_MODE, /* Event E5 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_REMOTE_STATE_VALID, /* Event E6 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LOCAL_SATISFIED_TRUE, /* Event E7 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LOCAL_SATISFIED_FALSE, /* Event E8 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_REMOTE_STABLE_TRUE, /* Event E9 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_REMOTE_STABLE_FALSE, /* Event E10 */
    VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LAST,
} vtss_eth_link_oam_discovery_events_t;

/******************************************************************************/
/* Link OAM Control layer data structures                                     */
/******************************************************************************/
typedef struct eth_link_oam_control_conf {
    BOOL                                    oam_control_conf;
    u16                                     oam_control_flags;
    vtss_eth_link_oam_pdu_control_t         pdu_tx_control;  /* local_pdu */
    vtss_eth_link_oam_discovery_state_t     discovery_state;
    /* Keep alive timer duration */
    u8                                      local_lost_link_timer;
    /* Max number of PDUs to be Xmited */
    u8                                      pdu_cnt;
    BOOL                                    local_link_status;
    vtss_eth_link_oam_parser_state_t        parser_state;
    vtss_eth_link_oam_mux_state_t           mux_state;
} vtss_eth_link_oam_control_conf_t;

typedef struct eth_link_oam_control_port_conf {
    vtss_eth_link_oam_control_conf_t        oam_conf;
    vtss_eth_link_oam_pdu_stats_t           oam_stats;
    vtss_eth_link_oam_critical_event_pdu_stats_t  oam_ce_stats;
    u8                     data[VTSS_ETH_LINK_OAM_INFO_DATA_LEN];/* Info data */
} vtss_eth_link_oam_control_port_conf_t;

typedef struct eth_link_oam_control_oper_conf {
    vtss_eth_link_oam_control_port_conf_t port_conf[VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT];
    u8                                    oam_supported_codes[VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT];
} vtss_eth_link_oam_control_oper_conf_t;

/******************************************************************************/
/* Eth Link OAM control configuration Macros                                  */
/******************************************************************************/

#define PORT_CONTROL_LAYER_CONF(port_no)     (\
                          (&link_oam_control_conf.port_conf[port_no].oam_conf)\
                                             )

#define PORT_CONTROL_LAYER_STATS(port_no)    (\
                         (&link_oam_control_conf.port_conf[port_no].oam_stats)\
                                           )


#define PORT_CONTROL_LAYER_CE_STATS(port_no) (\
                      (&link_oam_control_conf.port_conf[port_no].oam_ce_stats)\
                                             )


#define PORT_CONTROL_LAYER_DATA(port_no)     (\
                      (link_oam_control_conf.port_conf[port_no].data)\
                                             )

#define SET_PORT_CONTROL_CONF(port_no, conf) (\
                    (PORT_CONTROL_LAYER_CONF(port_no)->oam_control_conf = conf)\
                                             )

#define GET_PORT_CONTROL_CONF(port_no)       (\
                    (PORT_CONTROL_LAYER_CONF(port_no)->oam_control_conf)\
                                             )

#define IS_PORT_CONTROL_CONF_ENABLE(port_no) (\
                    ((GET_PORT_CONTROL_CONF(port_no))&1) \
                                             )

#define SET_SUPPORTED_CODE_CONF(port_no, oam_code)    (\
                   (link_oam_control_conf.oam_supported_codes[port_no]|=(1<<oam_code))\
                                             )
#define RESET_SUPPORTED_CODE_CONF(port_no, oam_code)  (\
                   (link_oam_control_conf.oam_supported_codes[port_no]&=(~(1u<<oam_code)))\
                                             )
#define GET_SUPPORTED_CODE_CONF(port_no, oam_code)    (\
                   (link_oam_control_conf.oam_supported_codes[port_no]&(1<<oam_code))\
                                             )


/***********************************************************************************/
/* Eth Link OAM control discovery state machine Macros                             */
/***********************************************************************************/
#define SET_PORT_DISCOVERY_STATE(port_conf, nextstate)  (\
                                       (port_conf->discovery_state = nextstate)\
                                                     )

#define GET_PORT_DISCOVERY_STATE(port_conf)  (port_conf->discovery_state)

#define GET_NEXT_PORT_DISCOVERY_STATE(port_conf,event)  (\
                           (discovery_state[port_conf->discovery_state][event])\
                                                        )

/******************************************************************************/
/* Eth Link OAM control Transmit state machine Macros                         */
/******************************************************************************/

#define SET_PORT_PDU_TX_CONTROL(port_conf,pdu_control)  (\
                                 (port_conf->pdu_tx_control = pdu_control)\
                                                        )

#define GET_PORT_PDU_TX_CONTROL(port_conf)     (port_conf->pdu_tx_control)

#define IS_PORT_IN_LF_STATE(port_no)           (\
                            PORT_CONTROL_LAYER_CONF(port_no)->pdu_tx_control\
                                 == VTSS_ETH_LINK_OAM_PDU_CONTROL_LF_INFO)\
 
#define IS_PORT_IN_RX_STATE(port_no)           (\
                            (PORT_CONTROL_LAYER_CONF(port_no)->pdu_tx_control\
                                == VTSS_ETH_LINK_OAM_PDU_CONTROL_RX_INFO)\
                                                )

#define SET_PORT_LOCAL_LOST_LINK_TIMER(port_no)        (\
                    (PORT_CONTROL_LAYER_CONF(port_no)->local_lost_link_timer\
                       = VTSS_ETH_LINK_OAM_LOCAL_LOST_LINK_TIMER)\
                                                       )
#define RESET_PORT_LOCAL_LOST_LINK_TIMER(port_no)      (\
                    (PORT_CONTROL_LAYER_CONF(port_no)->local_lost_link_timer\
                       = VTSS_ETH_LINK_OAM_NULL)\
                                                       )
#define GET_PORT_LOCAL_LOST_LINK_TIMER(port_no)        (\
                     (PORT_CONTROL_LAYER_CONF(port_no)->local_lost_link_timer)\
                                                       )

#define DEC_PORT_LOCAL_LOST_LINK_TIMER(port_no)        (\
                   (PORT_CONTROL_LAYER_CONF(port_no)->local_lost_link_timer--)\
                                                       )

#define SET_PORT_LOCAL_LINK_STATUS(port_no,status)    (\
                (PORT_CONTROL_LAYER_CONF(port_no)->local_link_status = status)\
                                                      )
#define GET_PORT_LOCAL_LINK_STATUS(port_no)           (\
                 (PORT_CONTROL_LAYER_CONF(port_no)->local_link_status)\
                                                      )

#define GET_PORT_PDU_CNT(port_no)  (PORT_CONTROL_LAYER_CONF(port_no)->pdu_cnt)
#define SET_PORT_PDU_CNT(port_no)                     (\
   (PORT_CONTROL_LAYER_CONF(port_no)->pdu_cnt = VTSS_ETH_LINK_OAM_PORT_PDU_CNT)\
                                                      )
#define DEC_PORT_PDU_CNT(port_no)  (PORT_CONTROL_LAYER_CONF(port_no)->pdu_cnt--)

/*******************************************************************************/
/* Eth Link OAM control flags specific Macros                                  */
/*******************************************************************************/
#define SET_PORT_FLAGS(port_no,flag_pos)      (\
           (PORT_CONTROL_LAYER_CONF(port_no)->oam_control_flags |=\
            flag_pos)\
                                              )

#define RESET_PORT_FLAGS(port_no,flag_pos)    (\
           (PORT_CONTROL_LAYER_CONF(port_no)->oam_control_flags &=\
           (~(flag_pos)))\
                                              )

#define GET_PORT_FLAGS(port_no)               (\
           (PORT_CONTROL_LAYER_CONF(port_no)->oam_control_flags)\
           )

#define IS_PORT_FLAG_ACTIVE(port_no,flag_pos)  (GET_PORT_FLAGS(port_no)&flag_pos)

/******************************************************************************/
/* Eth Link OAM control flags specific Macros                                 */
/******************************************************************************/

#define SET_PORT_CONTROL_LAYER_MUX_STATE(port_no,state) (\
                            (PORT_CONTROL_LAYER_CONF(port_no)->mux_state = \
                             state)\
                                                        )

#define GET_PORT_CONTROL_LAYER_MUX_STATE(port_no)       (\
                            (PORT_CONTROL_LAYER_CONF(port_no)->mux_state)\
                                                        )

#define SET_PORT_CONTROL_LAYER_PARSER_STATE(port_no,state) (\
                            (PORT_CONTROL_LAYER_CONF(port_no)->parser_state = \
                             state)\
                                                           )
#define GET_PORT_CONTROL_LAYER_PARSER_STATE(port_no)     (\
                             (PORT_CONTROL_LAYER_CONF(port_no)->parser_state)\
                                                         )

/******************************************************************************/
/* Eth Link OAM Stats specific Macros                                         */
/******************************************************************************/
#define INC_UNSUPPORTED_CODE_RX_STATS(port_no)   (\
                ((PORT_CONTROL_LAYER_STATS(port_no)->unsupported_codes_rx)++)\
                                                 )

#define INC_UNSUPPORTED_CODE_TX_STATS(port_no)  (\
                ((PORT_CONTROL_LAYER_STATS(port_no)->unsupported_codes_tx)++)\
                                                )

#define INC_INFO_RX_STATS(port_no)              (\
                ((PORT_CONTROL_LAYER_STATS(port_no)->information_rx)++)\
                                                )
#define INC_INFO_TX_STATS(port_no)              (\
                 ((PORT_CONTROL_LAYER_STATS(port_no)->information_tx)++)\
                                                )

#define INC_UNIQUE_EVENT_NOTIFICATION_TX_STATS(port_no) (\
                  ((PORT_CONTROL_LAYER_STATS(port_no)->\
                   unique_event_notification_tx)++)\
                                               )

#define INC_UNIQUE_EVENT_NOTIFICATION_RX_STATS(port_no) (\
                                  ((PORT_CONTROL_LAYER_STATS(port_no)->\
                                  unique_event_notification_rx)++)\
                                                        )

#define INC_DUP_EVENT_NOTIFICATION_TX_STATS(port_no)  (\
                                  (PORT_CONTROL_LAYER_STATS(port_no)->\
                                   duplicate_event_notification_tx)++)\
 
#define INC_DUP_EVENT_NOTIFICATION_RX_STATS(port_no)  (\
                                   (PORT_CONTROL_LAYER_STATS(port_no)->\
                                    duplicate_event_notification_rx)++)\
 
#define INC_LB_RX_STATS(port_no)             (\
               (PORT_CONTROL_LAYER_STATS(port_no)->loopback_control_rx)++\
                                            )
#define INC_LB_TX_STATS(port_no)            (\
               (PORT_CONTROL_LAYER_STATS(port_no)->loopback_control_tx)++\
                                            )

#define INC_VAR_REQ_RX_STATS(port_no)       (\
               (PORT_CONTROL_LAYER_STATS(port_no)->variable_request_rx)++\
                                            )
#define INC_VAR_REQ_TX_STATS(port_no)       (\
               (PORT_CONTROL_LAYER_STATS(port_no)->variable_request_tx)++\
                                            )
#define INC_VAR_RESP_RX_STATS(port_no)      (\
               (PORT_CONTROL_LAYER_STATS(port_no)->variable_response_rx)++\
                                            )
#define INC_VAR_RESP_TX_STATS(port_no)      (\
               (PORT_CONTROL_LAYER_STATS(port_no)->variable_response_tx)++\
                                            )

#define INC_ORG_SPECIFIC_RX_STATS(port_no)  (\
               (PORT_CONTROL_LAYER_STATS(port_no)->org_specific_rx)++\
                                            )
#define INC_ORG_SPECIFIC_TX_STATS(port_no)  (\
               (PORT_CONTROL_LAYER_STATS(port_no)->org_specific_tx)++\
                                            )

#define INC_LINK_FAULT_RX_STATS(port_no)    (\
               (PORT_CONTROL_LAYER_CE_STATS(port_no)->link_fault_rx)++\
                                            )
#define INC_LINK_FAULT_TX_STATS(port_no)    (\
               (PORT_CONTROL_LAYER_CE_STATS(port_no)->link_fault_tx)++\
                                            )

#define INC_DYING_GASP_RX_STATS(port_no)    (\
               (PORT_CONTROL_LAYER_CE_STATS(port_no)->dying_gasp_rx)++\
                                            )
#define INC_DYING_GASP_TX_STATS(port_no)    (\
               (PORT_CONTROL_LAYER_CE_STATS(port_no)->dying_gasp_tx)++\
                                            )

#define INC_CRITICAL_EVENT_RX_STATS(port_no) (\
              (PORT_CONTROL_LAYER_CE_STATS(port_no)->critical_event_rx)++\
                                             )
#define INC_CRITICAL_EVENT_TX_STATS(port_no) (\
              (PORT_CONTROL_LAYER_CE_STATS(port_no)->critical_event_tx)++\
                                             )

/******************************************************************************/
/* Eth Link OAM Module Control function prototypes                            */
/******************************************************************************/
/* Initializes the link OAM control layer */
void vtss_eth_link_oam_control_init(void);

/******************************************************************************/
/* ETH Link OAM Control layer function prototypes                             */
/******************************************************************************/

/* port_no:         port number                                               */
/* data:            port's Information PDU data                               */
/* Initializes the port's information data                                    */
u32 vtss_eth_link_oam_control_layer_port_conf_init(const u32 port_no, const u8 *data);

/* port_no:         port number                                               */
/* is_port_active:  port's OAM mode                                           */
/* Initializes the port's discovery protocol with specified mode              */
u32 vtss_eth_link_oam_control_layer_port_oper_init(const u32 port_no, const BOOL is_port_active);

/* port_no:         port number                                               */
/* data:            port's Information PDU data                               */
/* reset_port_oper: Flag to reset the discovery proctocol                     */
/* is_port_active:  port's OAM mode                                           */
/* Modifies the Port's information data and resets the discovery protocol     */
u32 vtss_eth_link_oam_control_layer_port_data_set(const u32 port_no, const u8 *data, const BOOL  reset_port_oper, const BOOL  is_port_active);
/* port_no:         port number                                               */
/* oam_control:     port's OAM control configuration                          */
/* Sets the port's OAM Control flag of the port                               */
u32 vtss_eth_link_oam_control_layer_port_control_conf_set(const u32 port_no, const u8 oam_control);
/* port_no:         port number                                               */
/* oam_control:     port's OAM control configuration                          */
/* Retrives the port's OAM Control flag of the port                           */
u32 vtss_eth_link_oam_control_layer_port_control_conf_get(const u32 port_no, u8 *oam_control);

/* port_no:         port number                                               */
/* Sets the port's OAM remote state valid flag                                */
u32 vtss_eth_link_oam_control_layer_port_remote_state_valid_set(const u32 port_no);

/* port_no:         port number                                               */
/* is_local_satisfied: local_satisfied flag                                   */
/* Sets the port's OAM local satisfied flag                                   */
u32 vtss_eth_link_oam_control_layer_port_local_satisfied_set(const u32 port_no, BOOL is_local_satisfied);

/* port_no:         port number                                               */
/* is_remote_stable: remote stable flag                                       */
/* Sets the port's remote satble flag                                         */
u32 vtss_eth_link_oam_control_layer_port_remote_stable_set(const u32 port_no, BOOL is_remote_stable);

/* port_no:         port number                                               */
/* pdu_control:     local_pdu flag status                                     */
/* retrives the local_pdu flag                                                */
u32 vtss_eth_link_oam_control_layer_port_pdu_control_status_get(const u32 port_no, vtss_eth_link_oam_pdu_control_t *const pdu_control);

/* port_no:         port number                                               */
/* state:           discovery protocol state                                  */
/* retrives the discovery protocol state                                      */
u32 vtss_eth_link_oam_control_layer_port_discovery_state_get(const u32 port_no, vtss_eth_link_oam_discovery_state_t *const state);

/* port_no:         port number                                               */
/* oam_code:        oam code to be supported                                  */
/* support_enable:  enable/disable the support for the specified code         */
/* Enables/Disables the support for the specified oam code                    */
u32 vtss_eth_link_oam_control_layer_supported_codes_conf_set(const u32 port_no, const u8 oam_code, const BOOL support_enable);

/* port_no:         port number                                               */
/* oam_code:        oam code to be supported                                  */
/* support_enable:  support configuration for the specified oam code          */
/* Retrives the support configuration for the specified oam code              */
u32 vtss_eth_link_oam_control_layer_supported_codes_conf_get(const u32 port_no, const u8 oam_code, BOOL *const support_conf);

/* port_no:         port number                                               */
/* flag:            OAM flag                                                  */
/* enable_flag:     enable/disable the specified flag                         */
/* Sets the port's OAM Flags like link_fault,dying_gasp                       */
u32 vtss_eth_link_oam_control_layer_port_flags_conf_set(const u32 port_no, const u8  flag, const BOOL enable);

/* oam_code:        oam code to be supported                                  */
/* support_enable:  support configuration for the specified oam code          */
/* Retrives the support configuration for the specified oam code              */
u32 vtss_eth_link_oam_control_layer_port_flags_conf_get(const u32 port_no, u8 *const flag);

/* port_no:         port number                                               */
/* Resets the pdu count to standard value on reception of OAM pdu             */
u32 vtss_eth_link_oam_control_layer_port_pdu_cnt_conf_set(const u32 port_no);

/* port_no:         port number                                               */
/* Resets the local last timer to standard value on reception of OAM pdu      */
u32 vtss_eth_link_oam_control_layer_port_local_lost_timer_conf_set(const u32 port_no);


/* port_no:         port number                                               */
/* Resets the port counters                                                   */
u32 vtss_eth_link_oam_control_layer_clear_statistics(const u32 port_no);


/******************************************************************************/
/* ETH Link OAM Control layer Multiplexer/Parser functions prototypes         */
/******************************************************************************/

/* port_no:         port number                                               */
/* mux_state:       port's multiplexer state                                  */
/* Sets the port's multiplexer state                                          */
u32 vtss_eth_link_oam_control_layer_port_mux_conf_set(const u32 port_no, const vtss_eth_link_oam_mux_state_t mux_state);
/* port_no:         port number                                               */
/* mux_state:       port's multiplexer state                                  */
/* Retrievs the port's multiplexer state                                      */
u32 vtss_eth_link_oam_control_layer_port_mux_conf_get(const u32 port_no, vtss_eth_link_oam_mux_state_t *const mux_state);

/* port_no:         port number                                               */
/* parser_state:    port's parser state                                       */
/* oam_ace_id:      port's ACE id to allow OAM PDU only                       */
/* lb_ace_id:       port's ACE id to deny rest all traffic                    */
u32 vtss_eth_link_oam_control_layer_port_parser_conf_set(const u32 port_no, const vtss_eth_link_oam_parser_state_t parser_state);

/* port_no:         port number                                               */
/* mux_state:       port's parser state                                       */
/* Retrievs the port's parser state                                           */
u32 vtss_eth_link_oam_control_layer_port_parser_conf_get(const u32 port_no, vtss_eth_link_oam_parser_state_t *const parser_state);


/******************************************************************************/
/* ETH Link OAM Control layer PDU handler funnctions prototypes               */
/******************************************************************************/

/* port_no:         port number                                               */
/* is_xmit_needed:  Flag to indicate whether to send an OAM frame or not      */
/* Verifies the need for transmitting the OAM pdus                            */
u32 vtss_eth_link_oam_control_layer_is_periodic_xmit_needed(const u32 port_no, BOOL *const is_xmit_needed);

/* port_no:         port number                                               */
/* oam_pdu:         OAM pdu                                                   */
/* Fills OAM pdu with the information data                                    */
u32 vtss_eth_link_oam_control_layer_fill_info_data(const u32 port_no, u8 *const pdu);

/* port_no:         port number                                               */
/* oam_pdu:         OAM pdu                                                   */
/* pdu_len:         length of the pdu to be transmitted                       */
/* oam_code:        code of the received pdu                                  */
/* Handles the received function                                              */
u32 vtss_eth_link_oam_control_layer_rx_pdu_handler(const u32 port_no, const u8 *pdu, const u16 pdu_len, const u8 oam_code);

/* port_no:         port number                                               */
/* port_stats:      port statistics                                           */
/* Retrives the port's OAM stats                                              */
u32 vtss_eth_link_oam_control_layer_port_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_pdu_stats_t *const port_stats);

/* port_no:         port number                                               */
/* port_stats:      port critical event statistics                            */
/* Retrives the port's OAM stats                                              */
u32 vtss_eth_link_oam_control_layer_port_critical_event_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_critical_event_pdu_stats_t *const port_ce_stats);


/* port_no:         port number                                               */
/* oam_pdu:         OAM pdu                                                   */
/* oam_code:        OAM code                                                  */
/* Fills the OAM pdu header                                                   */
u32 vtss_eth_link_oam_control_layer_fill_header(const u32 port_no, u8 *const pdu, const u8 code);

/* port_no:         port number                                               */
/* is_local_lost_link_timer: flag to indicate the expiration of the timer     */
/* Verifies the expiration of the link timer                                  */
u32 vtss_eth_link_oam_control_layer_is_local_lost_link_timer_done(const u32 port_no, BOOL *const is_local_lost_link_timer);

/******************************************************************************/
/* ETH Link OAM Control layer specific call out functions                     */
/******************************************************************************/

/* port_no:         port number                                               */
/* port_mac_addr:   port number                                               */
/* Gets the port's MAC address to fill the source MAC address                 */
vtss_rc vtss_eth_link_oam_control_layer_port_mac_conf_get(const u32 port_no, u8 *port_mac_addr);

#endif /* _VTSS_ETH_LINK_OAM_CONTROL_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/





