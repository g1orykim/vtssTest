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

#ifndef _ETH_LINK_OAM_API_H_
#define _ETH_LINK_OAM_API_H_

#include "l2proto_api.h"         /* For port specific Macros  */
#include "vtss_eth_link_oam_api.h"
#include "vtss_eth_link_oam_control_api.h"

/* Eth Link OAM Module error defintions */
typedef enum {
    ETH_LINK_OAM_RC_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_ETH_LINK_OAM),
    ETH_LINK_OAM_RC_INVALID_PARAMETER,          /* invalid parameter */
    ETH_LINK_OAM_RC_NOT_ENABLED,                /* Link OAM is not enable on the port */
    ETH_LINK_OAM_RC_ALREADY_CONFIGURED,         /* Management is applying same configuration */
    ETH_LINK_OAM_RC_NO_MEMORY,                  /* No Valid memory is available */
    ETH_LINK_OAM_RC_NOT_SUPPORTED,              /* Operation is not supported   */
    ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY,
    ETH_LINK_OAM_RC_INVALID_STATE,
    ETH_LINK_OAM_RC_INVALID_FLAGS,
    ETH_LINK_OAM_RC_INVALID_CODES,
    ETH_LINK_OAM_RC_INVALID_PDU_CNT,
    ETH_LINK_OAM_RC_TIMED_OUT,
} eth_link_oam_rc_t;


/* Initialize module */
vtss_rc eth_link_oam_init(vtss_init_data_t *data);



/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Enable/Disable the OAM Control                                 */
/* Enables/Disables the OAM Control on the port                               */
vtss_rc eth_link_oam_mgmt_port_control_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const vtss_eth_link_oam_control_t conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       port's OAM Control configuration                               */
/* Retrives the OAM Control configuration of the port                         */
vtss_rc eth_link_oam_mgmt_port_control_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_control_t  *conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Active or Passive mode of the port                             */
/* Configures the Port OAM mode to Active or Passive                          */
vtss_rc eth_link_oam_mgmt_port_mode_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const vtss_eth_link_oam_mode_t conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Active or Passive mode of the port                             */
/* Retrives the Port OAM mode of the port                                     */
vtss_rc eth_link_oam_mgmt_port_mode_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_mode_t *conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Enable/disable the MIB retrival support                        */
/* Configures the Port's MIB retrival support                                 */
vtss_rc eth_link_oam_mgmt_port_mib_retrival_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const BOOL conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* enable:     Port's MIB retrival support configuration                      */
/* Retrives the Port's MIB retrival support                                   */
vtss_rc eth_link_oam_mgmt_port_mib_retrival_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, BOOL  *conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* enable:     Port's MIB retrival support configuration                      */
/* Retrives the Port's MIB retrival support                                   */
vtss_rc eth_link_oam_mgmt_port_mib_retrival_oper_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const u8 conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* enable:     Port's remote loopback configuration                      */
/* Retrives the Port's remote loopback support                                   */
vtss_rc eth_link_oam_mgmt_port_remote_loopback_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, BOOL  *conf);
/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Enable/disable the port remote loopback support                */
/* Configures the Port's remote loopback support                              */
vtss_rc eth_link_oam_mgmt_port_remote_loopback_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, BOOL conf);
/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Enable/disable the port link monitoring loopback support       */
/* Configures the Port's link monitoring supporting                           */
vtss_rc eth_link_oam_mgmt_port_link_monitoring_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const BOOL conf);
/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Enable/disable the port link monitoring loopback support       */
/* Retrives the Port's link monitoring supporting                             */
vtss_rc eth_link_oam_mgmt_port_link_monitoring_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, BOOL *conf);
/* KPV */
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Window                           */
/* Configures the Error Frame Event Window Configuration                      */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_window_set (const vtss_isid_t isid, const u32  port_no, const u16 conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Window                           */
/* Retrieves the Error Frame Event Window Configuration                       */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_window_get (const vtss_isid_t isid, const u32  port_no, u16 *conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Threshold                        */
/* Configures the Error Frame Event Threshold Configuration                   */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_threshold_set(const vtss_isid_t isid, const u32  port_no, const u32 conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Threshold                        */
/* Retrieves the Error Frame Event Threshold Configuration                    */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_threshold_get (const vtss_isid_t isid, const u32  port_no, u32 *conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error Window                         */
/* Configures the Symbol Period Error Window Configuration                    */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_window_set (const vtss_isid_t isid, const u32  port_no, const u64  conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error Window                         */
/* Retrieves the Symbol Period Error Window Configuration                     */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_window_get (const vtss_isid_t isid, const u32  port_no, u64  *conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error Period Threshold               */
/* Configures the Symbol Period Error Period Threshold Configuration          */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_threshold_set (const vtss_isid_t isid, const u32  port_no, const u64  conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error Period Threshold               */
/* Retrieves the Symbol Period Error Period Threshold Configuration           */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_threshold_get (const vtss_isid_t isid, const u32  port_no, u64 *conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error RxPackets Threshold            */
/* Configures the Symbol Period Error RxPackets Threshold Configuration       */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_set (const vtss_isid_t isid, const u32 port_no, const u64  conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error RxPackets Threshold            */
/* Retreives the Symbol Period Error RxPackets Threshold Configuration        */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_get (const vtss_isid_t isid, const u32  port_no, u64 *conf);
/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error Window                          */
/* Configures the Frame Period Error Window Configuration                     */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_window_set (const vtss_isid_t isid, const u32  port_no, const u32  conf);
/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error Window                          */
/* Retrieves the Frame Period Error Window Configuration                      */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_window_get (const vtss_isid_t isid, const u32  port_no, u32  *conf);
/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error Period Threshold                */
/* Configures the Frame Period Error Period Threshold Configuration           */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_threshold_set (const vtss_isid_t isid, const u32 port_no, const u32 conf);
/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error Period Threshold                */
/* Retrieves the Frame Period Error Period Threshold Configuration            */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_threshold_get (const vtss_isid_t isid, const u32 port_no, u32 *conf);
/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error RxPackets Threshold             */
/* Configures the Frame Period Error RxPackets Threshold Configuration        */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_set (const vtss_isid_t isid, const u32  port_no, const u64  conf);
/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error RxPackets Threshold             */
/* Retrives the Frame Period Error RxPackets Threshold Configuration          */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_get (const vtss_isid_t isid, const u32  port_no, u64 *conf);

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Seconds Summary Event Window           */
/* Configures the Error Frame Seconds Summary Event Window Configuration      */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_set (const vtss_isid_t isid, const u32 port_no, const u16 conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Seconds Summary Event Window           */
/* Retrieves the Error Frame Seconds Summary Event Window Configuration       */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_get (const vtss_isid_t isid, const u16  port_no, u16 *conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Seconds Summary Event Threshold        */
/* Configures the Error Frame Seconds Summary Event Threshold Configuration   */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_set(const vtss_isid_t isid, const u32 port_no, const u16 conf);
/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Seconds Summary Event Threshold        */
/* Retrieves the Error Frame Seconds Summary Event Threshold Configuration    */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_get (const vtss_isid_t isid, const u16 port_no, u16 *conf);


/* END */

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Enable/disable the remote loop back operation                  */
/* Enables/disables the remote loop back operation                            */
vtss_rc eth_link_oam_mgmt_port_remote_loopback_oper_conf_set(const vtss_isid_t isid, const vtss_port_no_t port_no, const BOOL conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Enable/disable the port link monitoring support                */
/* Configures the Port's link monitoring support */
vtss_rc eth_link_oam_mgmt_port_link_monitoring_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const BOOL conf);
/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       port link monitoring configuration                             */
/* Retrieves the port's link monitoring  support                              */
vtss_rc eth_link_oam_mgmt_port_link_monitoring_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, BOOL *conf);

/******************************************************************************/
/* Link OAM Port's Client status reterival functions                          */
/******************************************************************************/

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* conf:       Control configuration                                          */
/* Retrives the Port OAM Control information                                  */
vtss_rc eth_link_oam_client_port_control_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, u8 *conf);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* info:       local info                                                     */
/* Retrives the Port OAM Local information                                    */
vtss_rc eth_link_oam_client_port_local_info_get (const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_info_tlv_t *local_info);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* info:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) information                             */
vtss_rc eth_link_oam_client_port_remote_info_get (const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_info_tlv_t *remote_info);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* info:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) information                             */
vtss_rc eth_link_oam_client_port_remote_seq_num_get (const vtss_isid_t isid, const vtss_port_no_t port_no, u16 *remote_info);


/* isid:                   stack unit id                                      */
/* port_no:                port number                                        */
/* info:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) MAC information                         */
vtss_rc eth_link_oam_client_port_remote_mac_addr_info_get (const vtss_isid_t isid, const vtss_port_no_t port_no, u8 *remote_mac_addr);


/* isid:                   stack unit id                                      */
/* port_no:                port number                                        */
/* local_error_info:       Local error info                                   */
/* remote_error_info:      Local error info                                   */
/* Retrives the Port OAM frame error event information                        */
vtss_rc eth_link_oam_client_port_frame_error_info_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_error_frame_event_tlv_t  *local_error_info, vtss_eth_link_oam_error_frame_event_tlv_t  *remote_error_info);

/* isid:                   stack unit id                                      */
/* port_no:                port number                                        */
/* local_error_info:       Local error info                                   */
/* remote_error_info:      Local error info                                   */
/* Retrives the Port OAM frame period error event information                 */
vtss_rc eth_link_oam_client_port_frame_period_error_info_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_error_frame_period_event_tlv_t  *local_info, vtss_eth_link_oam_error_frame_period_event_tlv_t   *remote_info);

/* isid:                   stack unit id                                      */
/* port_no:                port number                                        */
/* local_error_info:       Local error info                                   */
/* remote_error_info:      Local error info                                   */
/* Retrives the Port OAM frame symbol error event information                 */
vtss_rc eth_link_oam_client_port_symbol_period_error_info_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_error_symbol_period_event_tlv_t  *local_info, vtss_eth_link_oam_error_symbol_period_event_tlv_t   *remote_info);

vtss_rc eth_link_oam_client_port_error_frame_secs_summary_info_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t   *local_info, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t  *remote_info);


/*tx_control:  transmit control structure                                     */
/*Converts the PDU permission code to appropriate string                      */
i8 *pdu_tx_control_to_str(vtss_eth_link_oam_pdu_control_t tx_control);

/*discovery_state: Discovery state of the port                                */
/*Converts the Discovery state of the port to the appropriate string           */
i8 *discovery_state_to_str(vtss_eth_link_oam_discovery_state_t discovery_state);

/*mux_state:  Multiplexer state of the port                                   */
/*Converts the Multiplexer state to a  equivalent string information          */
i8 *mux_state_to_str(vtss_eth_link_oam_mux_state_t mux_state);

/*parse_state:  Parser state of the port                                      */
/*Converts the Parser state to a  equivalent string information               */
i8 *parser_state_to_str(vtss_eth_link_oam_parser_state_t parse_state);

/*var_response: Response received from the peer                               */
/*Prints the response received from the peer for the particular variable request*/
void vtss_eth_link_oam_send_response_to_cli(i8 *var_response);


/******************************************************************************/
/* Link OAM Port's status reterival functions                          */
/******************************************************************************/

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* info:       Remote info                                                    */
/* Retrives the Port OAM information                             */
vtss_rc eth_link_oam_mgmt_port_conf_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_conf_t *conf);

/* port_no:    port number                                                    */
vtss_rc eth_link_oam_clear_statistics(l2_port_no_t l2port);

/******************************************************************************/
/* Link OAM Port's Control status reterival functions                         */
/******************************************************************************/

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* Retrives the PDU control status of the discovery protocol                  */
vtss_rc eth_link_oam_control_layer_port_pdu_control_status_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_pdu_control_t *pdu_control);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* Retrives the port's discovery state                                        */
vtss_rc eth_link_oam_control_layer_port_discovery_state_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_discovery_state_t *state);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* Retrives the port's standard PDU statistics                                */
vtss_rc eth_link_oam_control_layer_port_pdu_stats_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_pdu_stats_t *pdu_stats);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* Retrives the port's critical event statistics                              */
vtss_rc eth_link_oam_control_layer_port_critical_event_pdu_stats_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_critical_event_pdu_stats_t *pdu_stats);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* flag:       flag to be enable or disabled                                  */
/* enable_flag: enable/disable the flag                                       */
/* Enables/disables the specified flag                                        */
vtss_rc eth_link_oam_control_port_flags_conf_set(const vtss_isid_t isid, const vtss_port_no_t port_no, const u8 flag, const BOOL enable_flag);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* flags:      port's Link OAM flags                                          */
/* Retrives the port's OAM flags status                                       */
vtss_rc eth_link_oam_control_port_flags_conf_get(const vtss_isid_t isid, const vtss_port_no_t port_no, u8 *flag);

/* isid:       stack unit id                                                  */
/* port_no:    port number                                                    */
/* loopback_status: status of the loopback                                    */
/* Retrieves the port's Loopback status in accordance with RFC 4878           */
vtss_rc eth_link_oam_port_loopback_oper_status_get(const vtss_isid_t isid, const vtss_port_no_t port, vtss_eth_link_oam_loopback_status_t *loopback_status);
#endif /* _ETH_LINK_OAM_API_H_ */




