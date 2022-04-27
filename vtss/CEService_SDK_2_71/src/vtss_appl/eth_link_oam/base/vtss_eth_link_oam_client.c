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

#include "vtss_eth_link_oam_client.h"
#include "vtss_eth_link_oam_control_api.h"
#include "conf_api.h"

/******************************************************************************/
/*  ETH Link OAM global variables                                             */
/******************************************************************************/
/* OAM Client configuration */
static vtss_eth_link_oam_client_oper_conf_t link_oam_client_conf;

static u32 vtss_eth_link_oam_client_port_discovery_protocol_init(const u32 port_no, BOOL rev_update);

static inline u32  vtss_eth_link_oam_client_record_info_data(const u32 port_no, vtss_eth_link_oam_info_tlv_t  *local_info, vtss_eth_link_oam_info_tlv_t  *remote_info, BOOL  reset_port_oper, BOOL  is_port_active);

static inline BOOL vtss_eth_link_oam_client_is_valid_state(const u32 port_no);


/******************************************************************************/
/* ETH Link OAM Client function definitions                                   */
/******************************************************************************/

/* Initializes the port with defaults */
u32 vtss_eth_link_oam_client_port_conf_init(const u32 port_no)
{
    u32                                    rc = VTSS_ETH_LINK_OAM_RC_OK;
    u16                                    oam_pdu_len;
    u8                                     data[VTSS_ETH_LINK_OAM_INFO_DATA_LEN];
    vtss_eth_link_oam_client_conf_t        *port_conf = NULL;
    vtss_eth_link_oam_info_tlv_t           *port_tlv = NULL;
    u8                                     mac[VTSS_ETH_LINK_OAM_MAC_LEN];

    port_conf = PORT_CLIENT_CONF(port_no);
    port_tlv = PORT_CLIENT_LOCAL_CONF(port_no);
    oam_pdu_len = vtss_eth_link_oam_htons(VTSS_ETH_LINK_OAM_PDU_MAX_LEN);
    (void)conf_mgmt_mac_addr_get(mac, 0);

    do {
        /* This logic helps to remove the installed rules while sys config clearance */
        if (GET_PORT_STATE(port_no)) {
            PORT_CLIENT_LOCAL_CONF(port_no)->state = (VTSS_ETH_LINK_OAM_MUX_FWD_STATE | VTSS_ETH_LINK_OAM_PARSER_FWD_STATE);
            memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
            PORT_CLIENT_LOCAL_CONF(port_no)->oui[0] = mac[0];
            PORT_CLIENT_LOCAL_CONF(port_no)->oui[1] = mac[1];
            PORT_CLIENT_LOCAL_CONF(port_no)->oui[2] = mac[2];
            memcpy(data, PORT_CLIENT_LOCAL_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
            rc = vtss_eth_link_oam_control_port_data_set(port_no, data, FALSE, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_client_port_state_conf_set(port_no, VTSS_ETH_LINK_OAM_MUX_FWD_STATE, VTSS_ETH_LINK_OAM_PARSER_FWD_STATE, FALSE, FALSE);
        }

        memset(port_conf, '\0', sizeof(vtss_eth_link_oam_client_conf_t));
        port_tlv->info_type    = VTSS_ETH_LINK_OAM_LOCAL_INFO_TLV;
        port_tlv->info_len     = VTSS_ETH_LINK_OAM_INFO_TLV_LEN;
        port_tlv->version      = VTSS_ETH_LINK_OAM_PROTOCOL_VERSION;
        port_tlv->state        = (VTSS_ETH_LINK_OAM_MUX_FWD_STATE | VTSS_ETH_LINK_OAM_PARSER_FWD_STATE);
        memcpy(port_tlv->oampdu_conf, &oam_pdu_len, sizeof(u16));

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    port_tlv->oui[0] = mac[0];
    port_tlv->oui[1] = mac[1];
    port_tlv->oui[2] = mac[2];

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
        memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
        memcpy(data, port_tlv, VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
        rc = vtss_eth_link_oam_control_port_conf_init(port_no, data);
    }
    return rc;
}

static u32 vtss_eth_link_oam_client_port_discovery_protocol_init(const u32 port_no, BOOL  rev_update)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_client_conf_t     *port_conf = PORT_CLIENT_CONF(port_no);
    u8                                  data[VTSS_ETH_LINK_OAM_INFO_DATA_LEN];
    u16                                 temp16;
    u16                                 temp16_2;
    u32                                 temp32;
    u64                                 temp64;
    u64                                 temp64_2;

    do {
        port_conf->remote_state_valid = FALSE;
        port_conf->local_satisfied    = FALSE;
        port_conf->remote_stable      = FALSE;

        /* Reset the Link Event specific data structures */
        port_conf->error_sequence_number = 0;
        port_conf->remote_error_sequence_num = 0;
        /* Clear Frame error operational configurations */
        temp16 = port_conf->error_frame_info.error_frame_oper_conf.mgmt_event_window;
        temp32 = port_conf->error_frame_info.error_frame_oper_conf. oper_error_frame_threshold;
        memset(&port_conf->error_frame_info.remote_error_frame_tlv, 0,
               sizeof(vtss_eth_link_oam_error_frame_event_tlv_t));
        memset(&port_conf->error_frame_info.error_frame_oper_conf, 0,
               sizeof(vtss_eth_link_oam_client_link_event_oper_conf_t));
        /* I'll change this code */
        port_conf->error_frame_info.error_frame_oper_conf.mgmt_event_window = temp16;
        port_conf->error_frame_info.error_frame_oper_conf.oper_event_window = temp16;
        port_conf->error_frame_info.error_frame_oper_conf. oper_error_frame_threshold = temp32;

        /* Clear Symbol error operational configurations */
        temp64 = port_conf->symbol_period_error_info.symbol_period_error_oper_conf.mgmt_event_window;
        temp64_2 = port_conf->symbol_period_error_info.symbol_period_error_oper_conf.oper_error_frame_threshold;
        memset(&port_conf->symbol_period_error_info.remote_symbol_period_error_tlv, 0, sizeof(vtss_eth_link_oam_error_symbol_period_event_tlv_t));
        memset(&port_conf->symbol_period_error_info.symbol_period_error_oper_conf, 0, sizeof(client_link_error_symbol_period_event_oper_conf_t));
        port_conf->symbol_period_error_info.symbol_period_error_oper_conf.mgmt_event_window = temp64;
        port_conf->symbol_period_error_info.symbol_period_error_oper_conf.oper_event_window = temp64;
        port_conf->symbol_period_error_info.symbol_period_error_oper_conf.oper_error_frame_threshold = temp64_2;
        /* Clear Frame period error operatiinal configuration */
        memset(&port_conf->error_frame_period_info.remote_frame_period_error_tlv, 0, sizeof(vtss_eth_link_oam_error_frame_period_event_tlv_t));
        memset(&port_conf->error_frame_period_info.frame_period_error_oper_conf, 0, sizeof(client_link_error_frame_period_event_oper_conf_t));
        /* Clear Frame seconds error operational configuration */
        temp16 = port_conf->error_frame_secs_summary_info.error_frame_secs_summary_oper_conf.mgmt_event_window;
        temp16_2 = port_conf->error_frame_secs_summary_info.error_frame_secs_summary_oper_conf.oper_secs_summary_threshold;
        memset(&port_conf->error_frame_secs_summary_info.remote_error_frame_secs_summary_tlv, 0, sizeof(vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t));
        memset(&port_conf->error_frame_secs_summary_info.error_frame_secs_summary_oper_conf, 0, sizeof(vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf_t));
        port_conf->error_frame_secs_summary_info.error_frame_secs_summary_oper_conf.mgmt_event_window = temp16;
        port_conf->error_frame_secs_summary_info.error_frame_secs_summary_oper_conf.oper_event_window = temp16;
        port_conf->error_frame_secs_summary_info.error_frame_secs_summary_oper_conf.oper_secs_summary_threshold = temp16_2;

        memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
        if (vtss_eth_link_oam_client_port_revision_update(port_no, rev_update) != VTSS_RC_OK) {
            break;
        }
        memcpy(data, PORT_CLIENT_LOCAL_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
        rc = vtss_eth_link_oam_control_port_data_set(port_no, data, FALSE, FALSE);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }

        if (GET_PORT_STATE(port_no)) {
            PORT_CLIENT_LOCAL_CONF(port_no)->state = (VTSS_ETH_LINK_OAM_MUX_FWD_STATE | VTSS_ETH_LINK_OAM_PARSER_FWD_STATE);
            memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
            PORT_CLIENT_LOCAL_CONF(port_no)->oui[0] = 0x00;
            PORT_CLIENT_LOCAL_CONF(port_no)->oui[1] = 0x01;
            PORT_CLIENT_LOCAL_CONF(port_no)->oui[2] = 0xc1;
            memcpy(data, PORT_CLIENT_LOCAL_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
            rc = vtss_eth_link_oam_control_port_data_set(port_no, data, FALSE, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_client_port_state_conf_set(port_no, VTSS_ETH_LINK_OAM_MUX_FWD_STATE, VTSS_ETH_LINK_OAM_PARSER_FWD_STATE, FALSE, FALSE);
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Error occured while initializing the discovery", port_no, rc, 0, 0);
    }
    return rc;
}

static BOOL vtss_eth_link_oam_client_is_valid_state(const u32 port_no)
{
    BOOL is_valid_state = FALSE;
    u32  rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_discovery_state_t state = VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT;

    rc = vtss_eth_link_oam_control_port_discovery_state_get(port_no, &state);
    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
        is_valid_state = (state == VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY) ? TRUE : FALSE;
    }

    return is_valid_state;

}

u32 vtss_eth_link_oam_client_port_control_conf_set(const u32 port_no, const vtss_eth_link_oam_control_t enable)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        rc = vtss_eth_link_oam_client_port_discovery_protocol_init(port_no, FALSE);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }

        switch (enable) {
        case VTSS_ETH_LINK_OAM_CONTROL_ENABLE:
            SET_CLIENT_PORT_CONTROL(port_no, enable);
            rc = vtss_eth_link_oam_control_port_oper_init(port_no, IS_PORT_CONF_ACTIVE(port_no, VTSS_ETH_LINK_OAM_CONF_MODE));
            break;
        case VTSS_ETH_LINK_OAM_CONTROL_DISABLE:
            rc = vtss_eth_link_oam_control_port_oper_init(port_no, IS_PORT_CONF_ACTIVE(port_no, VTSS_ETH_LINK_OAM_CONF_MODE));
            if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
                RESET_CLIENT_PORT_CONTROL(port_no);
            }
            break;
        default:
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        } /* end of switch to handle the control conf */
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Error occured while initilaizing the OAM operation", port_no, rc, 0, 0);
            break;
        }
        /* Call the Link OAM Control Functions */
        rc = vtss_eth_link_oam_control_port_control_conf_set(port_no, GET_CLIENT_PORT_CONTROL(port_no));

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;

}

u32 vtss_eth_link_oam_client_port_control_conf_get (const u32  port_no, u8 *const  conf)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_CLIENT_PORT_CONTROL(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_port_admin_conf_set (const u32  port_no, const vtss_eth_link_oam_capability_conf_t conf, const BOOL capability_on)
{

    u8                                  data[VTSS_ETH_LINK_OAM_INFO_DATA_LEN];
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    BOOL                                reset_port_oper = FALSE;
    BOOL                                is_old_conf_active = FALSE;

    do {
        is_old_conf_active = IS_PORT_CONF_ACTIVE(port_no, conf);

        if (is_old_conf_active == capability_on) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        }

        if (capability_on == TRUE) {
            SET_CLIENT_PORT_CONF(port_no, conf);
        } else {
            RESET_CLIENT_PORT_CONF(port_no, conf);
        }
        if (conf == VTSS_ETH_LINK_OAM_CONF_MODE) {
            reset_port_oper = TRUE;
        }
        if (reset_port_oper == FALSE && vtss_eth_link_oam_client_port_revision_update(port_no, TRUE) != VTSS_RC_OK) {
            break;
        }
        memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
        memcpy(data, PORT_CLIENT_LOCAL_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
        memcpy(data + VTSS_ETH_LINK_OAM_INFO_TLV_LEN, PORT_CLIENT_REMOTE_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
        if (reset_port_oper == TRUE) {
            rc = vtss_eth_link_oam_client_port_discovery_protocol_init(port_no, TRUE);
        }
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        rc = vtss_eth_link_oam_control_port_data_set(port_no, data, reset_port_oper, IS_PORT_CONF_ACTIVE(port_no, conf));

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */
    return rc;
}

u32 vtss_eth_link_oam_client_port_revision_update(const u32 port_no, BOOL  rev_update)
{
    u16                                 tmp_revision = 0;
    vtss_eth_link_oam_discovery_state_t state = VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT;

    if (rev_update == TRUE) {
        if (vtss_eth_link_oam_control_port_discovery_state_get(port_no, &state) != VTSS_ETH_LINK_OAM_RC_OK) {
            return VTSS_ETH_LINK_OAM_RC_OK;
        }
        if ((state == VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT) || (state == VTSS_ETH_LINK_OAM_DISCOVERY_STATE_PASSIVE_WAIT) || (state == VTSS_ETH_LINK_OAM_DISCOVERY_STATE_ACTIVE_SEND_LOCAL)) {
            return VTSS_ETH_LINK_OAM_RC_OK;
        }
        memcpy(&tmp_revision, PORT_CLIENT_LOCAL_CONF(port_no)->revision, VTSS_ETH_LINK_OAM_REV_LEN);
        tmp_revision = vtss_eth_link_oam_ntohs(tmp_revision);
        tmp_revision++;
        tmp_revision = vtss_eth_link_oam_htons(tmp_revision);
    }
    memcpy(PORT_CLIENT_LOCAL_CONF(port_no)->revision, &tmp_revision, VTSS_ETH_LINK_OAM_REV_LEN);

    return VTSS_ETH_LINK_OAM_RC_OK;
}


u32 vtss_eth_link_oam_client_port_local_info_get (const u32 port_no, vtss_eth_link_oam_info_tlv_t *const local_info)
{
    vtss_eth_link_oam_info_tlv_t        *port_tlv = NULL;
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {

        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if (local_info == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        port_tlv = PORT_CLIENT_LOCAL_CONF(port_no);
        memcpy(local_info, port_tlv, VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_port_remote_info_get (const u32 port_no, vtss_eth_link_oam_info_tlv_t *const remote_info)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_info_tlv_t        *port_tlv;
    vtss_eth_link_oam_client_conf_t     *port_conf = NULL;

    do {

        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if (remote_info == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        port_conf = PORT_CLIENT_CONF(port_no);
        if (port_conf->remote_state_valid == TRUE) {
            port_tlv = PORT_CLIENT_REMOTE_CONF(port_no);
            memcpy(remote_info, port_tlv, VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
        } else {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
            break;
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_port_remote_seq_num_get (const u32  port_no, u16 *const  conf)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_client_conf_t     *port_conf = NULL;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        port_conf = PORT_CLIENT_CONF(port_no);
        *conf = port_conf->remote_error_sequence_num;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}


u32 vtss_eth_link_oam_client_link_monitoring_pdu_fill_info_data(const u32 port_no, u8 *const pdu, const BOOL is_error_frame_xmit_needed, const BOOL is_symbol_period_xmit_needed, const BOOL is_frame_period_xmit_needed, const BOOL is_error_frame_secs_summary_xmit_needed)
{
    u16                           sequence_number = VTSS_ETH_LINK_OAM_NULL;
    u32                           rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8                            *tmp_ptr;
    u64                           up_time = cyg_current_time();
    u16                           time_stamp;

    vtss_eth_link_oam_client_error_frame_info_t *error_frame_tlv = NULL;
    vtss_eth_link_oam_client_symbol_period_errors_info_t *symbol_period_error_tlv = NULL;
    vtss_eth_link_oam_client_frame_period_errors_info_t  *frame_period_error_tlv = NULL;
    vtss_eth_link_oam_client_error_frame_secs_summary_info_t *error_frame_secs_summary_tlv = NULL;

    time_stamp = (u16) (up_time / 10);
    time_stamp = vtss_eth_link_oam_htons(time_stamp);

    do {
        if (pdu == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_PORT_CLIENT_ERROR_FRAME_SEQUENCE_NUMBER(port_no, (GET_PORT_CLIENT_ERROR_FRAME_SEQUENCE_NUMBER(port_no) + 1));

        sequence_number = vtss_eth_link_oam_htons(GET_PORT_CLIENT_ERROR_FRAME_SEQUENCE_NUMBER(port_no));

        tmp_ptr = (pdu + VTSS_ETH_LINK_OAM_PDU_HDR_LEN );
        memcpy(tmp_ptr, &sequence_number, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_SEQUENCE_NUMBER_LEN);
        tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_SEQUENCE_NUMBER_LEN;

        if (is_error_frame_xmit_needed) {
            /* Copy the PDU */
            /* Update the fields in the error_frame_event_tlv structure */

            error_frame_tlv = PORT_CLIENT_ERROR_FRAME_CONF(port_no);
            *tmp_ptr =  error_frame_tlv->error_frame_tlv.event_type;
            tmp_ptr++;
            *tmp_ptr = error_frame_tlv->error_frame_tlv.event_length;
            tmp_ptr++;
            memcpy(tmp_ptr, &time_stamp, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TIME_STAMP_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TIME_STAMP_LEN;
            memcpy(tmp_ptr, error_frame_tlv->error_frame_tlv.error_frame_window, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_WINDOW_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_WINDOW_LEN;

            memcpy(tmp_ptr, error_frame_tlv->error_frame_tlv.error_frame_threshold, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_THRESHOLD_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_THRESHOLD_LEN;

            memcpy(tmp_ptr, error_frame_tlv->error_frame_tlv.error_frames, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_ERROR_FRAMES_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_ERROR_FRAMES_LEN;

            memcpy(tmp_ptr, error_frame_tlv->error_frame_tlv.error_running_total, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_FRAMES_LEN);

            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_FRAMES_LEN;

            memcpy(tmp_ptr, error_frame_tlv->error_frame_tlv.event_running_total, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_EVENTS_LEN);

            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_EVENTS_LEN;

        }   /* end of filling the frame error event */
        if (is_symbol_period_xmit_needed) {
            /* Copy the PDU */
            /* Update the fields in the symbol_period_error_tlv structure*/
            symbol_period_error_tlv = PORT_CLIENT_SYMBOL_PERIOD_ERROR_CONF(port_no);
            *tmp_ptr = symbol_period_error_tlv->symbol_period_error_tlv.event_type;
            tmp_ptr++;
            *tmp_ptr = symbol_period_error_tlv->symbol_period_error_tlv.event_length;
            tmp_ptr++;
            memcpy(tmp_ptr, &time_stamp, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TIME_STAMP_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TIME_STAMP_LEN;

            memcpy(tmp_ptr, symbol_period_error_tlv->symbol_period_error_tlv.error_symbol_window, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_WINDOW_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_WINDOW_LEN;

            memcpy(tmp_ptr, symbol_period_error_tlv->symbol_period_error_tlv.error_symbol_threshold, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_THRESHOLD_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_THRESHOLD_LEN;

            memcpy(tmp_ptr, symbol_period_error_tlv->symbol_period_error_tlv.error_symbols, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_ERROR_SYMBOLS_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_ERROR_SYMBOLS_LEN;

            memcpy(tmp_ptr, symbol_period_error_tlv->symbol_period_error_tlv.error_running_total, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_SYMBOLS_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_SYMBOLS_LEN;

            memcpy(tmp_ptr, symbol_period_error_tlv->symbol_period_error_tlv.event_running_total, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN;

        } /* end of filling symbol period event */
        if (is_frame_period_xmit_needed) {
            /* Copy the PDU */
            /* Update the fields in the frame_period_error_tlv structure */
            frame_period_error_tlv = PORT_CLIENT_FRAME_PERIOD_ERROR_CONF(port_no);
            *tmp_ptr = frame_period_error_tlv->frame_period_error_tlv.event_type;
            tmp_ptr++;
            *tmp_ptr = frame_period_error_tlv->frame_period_error_tlv.event_length;
            tmp_ptr++;

            memcpy(tmp_ptr, &time_stamp, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TIME_STAMP_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TIME_STAMP_LEN;

            memcpy(tmp_ptr, frame_period_error_tlv->frame_period_error_tlv.error_frame_period_window, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_WINDOW_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_WINDOW_LEN;

            memcpy(tmp_ptr, frame_period_error_tlv->frame_period_error_tlv.error_frame_threshold, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_THRESHOLD_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_THRESHOLD_LEN;

            memcpy(tmp_ptr, frame_period_error_tlv->frame_period_error_tlv.error_frames, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_ERROR_FRAMES_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_ERROR_FRAMES_LEN;

            memcpy(tmp_ptr, frame_period_error_tlv->frame_period_error_tlv.error_running_total, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_FRAMES_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_FRAMES_LEN;

            memcpy(tmp_ptr, frame_period_error_tlv->frame_period_error_tlv.event_running_total, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN);

            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN;

        } /* end of filling the frame period events */
        if (is_error_frame_secs_summary_xmit_needed) {
            /* Copy the PDU */
            /* Update the fields in the error_frame_secs_summaryt_tlv  structure */
            error_frame_secs_summary_tlv = PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_CONF(port_no);
            *tmp_ptr =  error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.event_type;
            tmp_ptr++;
            *tmp_ptr = error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.event_length;
            tmp_ptr++;
            memcpy(tmp_ptr, &time_stamp, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TIME_STAMP_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TIME_STAMP_LEN;
            memcpy(tmp_ptr, error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.secs_summary_window, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_WINDOW_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_WINDOW_LEN;

            memcpy(tmp_ptr, error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.secs_summary_threshold, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_THRESHOLD_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_THRESHOLD_LEN;

            memcpy(tmp_ptr, error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.secs_summary_events, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_ERROR_FRAMES_LEN);
            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_ERROR_FRAMES_LEN ;

            memcpy(tmp_ptr, error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.error_running_total, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_FRAMES_LEN);

            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_FRAMES_LEN ;

            memcpy(tmp_ptr, error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.event_running_total, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_EVENTS_LEN);

            tmp_ptr += VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_EVENTS_LEN ;

        } /* end of handling the seconds summary case */
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_pdu_max_len(const u32 port_no, u16   *max_pdu_len)
{
    vtss_eth_link_oam_info_tlv_t        *port_remote_tlv;
    vtss_eth_link_oam_info_tlv_t        *port_tlv;
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    u16                                 temp16;

    do {

        if (max_pdu_len == NULL) {
            return VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
        }

        port_tlv = PORT_CLIENT_LOCAL_CONF(port_no);
        port_remote_tlv = PORT_CLIENT_REMOTE_CONF(port_no);

        if (!vtss_eth_link_oam_client_is_valid_state(port_no)) {
            *max_pdu_len = VTSS_ETH_LINK_OAM_PDU_MIN_LEN;
            break;
        }

        memcpy(max_pdu_len, port_tlv->oampdu_conf, sizeof(u16));
        memcpy(&temp16, port_remote_tlv->oampdu_conf, sizeof(u16));
        *max_pdu_len = vtss_eth_link_oam_htons(*max_pdu_len);
        temp16 = vtss_eth_link_oam_htons(temp16);
        if (temp16 < *max_pdu_len) {
            *max_pdu_len = temp16;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_port_remote_mac_addr_info_get(const u32 port_no, u8 *const mac_addr)

{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_client_conf_t     *port_conf = NULL;

    do {

        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (mac_addr == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        port_conf = PORT_CLIENT_CONF(port_no);
        if (port_conf->remote_state_valid == TRUE) {
            memcpy(mac_addr, port_conf->remote_mac_addr, VTSS_ETH_LINK_OAM_MAC_LEN);
        } else {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
            break;
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_port_frame_error_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_event_tlv_t *const local_error_info, vtss_eth_link_oam_error_frame_event_tlv_t *const remote_error_info)
{

    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if ( (local_error_info == NULL) ||
             (remote_error_info == NULL)
           ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        memcpy(local_error_info, &PORT_CLIENT_ERROR_FRAME_CONF(port_no)->error_frame_tlv, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV_LEN);
        memcpy(remote_error_info, &PORT_CLIENT_ERROR_FRAME_CONF(port_no)->remote_error_frame_tlv, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV_LEN);

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_port_frame_period_error_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_period_event_tlv_t *const local_info, vtss_eth_link_oam_error_frame_period_event_tlv_t *const remote_info)
{

    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if ( (local_info == NULL) ||
             (remote_info == NULL)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        memcpy(local_info, &PORT_CLIENT_FRAME_PERIOD_ERROR_CONF(port_no)->frame_period_error_tlv, sizeof(vtss_eth_link_oam_error_frame_period_event_tlv_t));
        memcpy(remote_info, &PORT_CLIENT_FRAME_PERIOD_ERROR_CONF(port_no)->remote_frame_period_error_tlv, sizeof(vtss_eth_link_oam_error_frame_period_event_tlv_t));
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_port_symbol_period_error_info_get (const u32 port_no, vtss_eth_link_oam_error_symbol_period_event_tlv_t *const local_info, vtss_eth_link_oam_error_symbol_period_event_tlv_t *const remote_info)
{

    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if ((local_info == NULL) || (remote_info == NULL)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        memcpy(local_info, &PORT_CLIENT_SYMBOL_PERIOD_ERROR_CONF(port_no)->symbol_period_error_tlv, sizeof(vtss_eth_link_oam_error_symbol_period_event_tlv_t));
        memcpy(remote_info, &PORT_CLIENT_SYMBOL_PERIOD_ERROR_CONF(port_no)->remote_symbol_period_error_tlv, sizeof(vtss_eth_link_oam_error_symbol_period_event_tlv_t));

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}
u32 vtss_eth_link_oam_client_port_error_frame_secs_summary_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t *local_info, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t *remote_info)
{

    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if ( (local_info == NULL) ||
             (remote_info == NULL)
           ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        memcpy(local_info, &PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_CONF(port_no)->error_frame_secs_summary_tlv, sizeof(vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t));
        memcpy(remote_info, &PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_CONF(port_no)->remote_error_frame_secs_summary_tlv, sizeof(vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t));

    } while (VTSS_ETH_LINK_OAM_NULL);

    return rc;
}


u32 vtss_eth_link_oam_client_is_link_event_stats_update_needed(const u32 port_no, BOOL *const is_update_needed)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    BOOL                                is_port_up = FALSE;
    BOOL                                is_valid_state = FALSE;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (is_update_needed == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        is_valid_state = vtss_eth_link_oam_client_is_valid_state(port_no);
        if (vtss_is_port_info_get(port_no, &is_port_up) != VTSS_ETH_LINK_OAM_RC_OK) {
            is_port_up = FALSE;
        }
        if ((is_port_up == TRUE) && (is_valid_state == TRUE) && (IS_CONF_ACTIVE (GET_CLIENT_PORT_CONF(port_no), VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT) && GET_CLIENT_PORT_CONTROL(port_no))) {
            *is_update_needed = TRUE;
        } else {
            *is_update_needed = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_is_error_frame_window_expired(const u32 port_no, BOOL *const is_frame_window_expired)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (is_frame_window_expired == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (GET_PORT_CLIENT_ERROR_FRAME_WINDOW(port_no) <= VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS) {
            *is_frame_window_expired = TRUE;
        } else {
            *is_frame_window_expired = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL);

    return rc;
}

u32 vtss_eth_link_oam_client_error_frame_oper_conf_update(const u32 port_no, const vtss_eth_link_oam_client_link_event_oper_conf_t *oper_conf, const u32 flags)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (oper_conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_MGMT_WINDOW(port_no, oper_conf->mgmt_event_window);
        }

        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_WINDOW(port_no, GET_PORT_CLIENT_ERROR_FRAME_MGMT_WINDOW(port_no));
        }
        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET) {
            SET_PORT_CLIENT_ERROR_FRAME_WINDOW(port_no, ((GET_PORT_CLIENT_ERROR_FRAME_WINDOW(port_no)) - (oper_conf->oper_event_window)));
        }

        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_START(port_no, GET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_TIMEOUT(port_no));
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_TIMEOUT(port_no, oper_conf->ifInErrors_at_timeout);
        }
        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_ERROR_THRESHOLD(port_no, oper_conf->oper_error_frame_threshold);
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_error_frame_event_conf_set(const u32 port_no, BOOL *const is_error_frame_xmit_needed)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_client_error_frame_info_t *error_frame_tlv;

    u16                                  temp16;
    u32                                  temp32;
    u64                                  temp64;

    do {

        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        /* Update the fields in the error_frame_link_event_oper_structure */
        temp64 = (GET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_TIMEOUT(port_no));
        if (temp64 >= GET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_START(port_no)) {
            temp64 = temp64 - GET_PORT_CLIENT_ERROR_FRAME_ERROR_AT_START(port_no);
        } else {
            *is_error_frame_xmit_needed = FALSE;
            break;
        }
        SET_PORT_CLIENT_ERROR_FRAME_TOTAL_ERROR_FRAMES(port_no, temp64);

#if 0
        if (!GET_PORT_CLIENT_ERROR_FRAME_ERROR_THRESHOLD(port_no)) {
            *is_error_frame_xmit_needed = FALSE;
            break;
        }
#endif

        /* Check if the errors has exceeded the threshold */
        if (GET_PORT_CLIENT_ERROR_FRAME_TOTAL_ERROR_FRAMES(port_no) >= GET_PORT_CLIENT_ERROR_FRAME_ERROR_THRESHOLD(port_no) ) {

            /* Increment the total_events_occured field */
            temp32 = GET_PORT_CLIENT_ERROR_FRAME_TOTAL_EVENTS_OCCURED(port_no) + 1;
            SET_PORT_CLIENT_ERROR_FRAME_TOTAL_EVENTS_OCCURED(port_no, temp32);

            /* Update the fields in the error_frame_event_tlv structure */
            error_frame_tlv = PORT_CLIENT_ERROR_FRAME_CONF(port_no);
            /* Clear the PDU Strcture */
            memset(&(error_frame_tlv->error_frame_tlv), VTSS_ETH_LINK_OAM_NULL, sizeof(vtss_eth_link_oam_error_frame_event_tlv_t));
            /* Update the PDU Structure */
            error_frame_tlv->error_frame_tlv.event_type = (VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_EVENT);
            error_frame_tlv->error_frame_tlv.event_length = (VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_EVENT_LEN);
            memcpy(&temp16, &error_frame_tlv->error_frame_oper_conf.oper_event_window, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_WINDOW_LEN);
            temp16 = vtss_eth_link_oam_htons(temp16);
            memcpy(error_frame_tlv->error_frame_tlv.error_frame_window, &temp16, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_WINDOW_LEN);

            memcpy(&temp32, &error_frame_tlv->error_frame_oper_conf.oper_error_frame_threshold, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_THRESHOLD_LEN);
            temp32 = vtss_eth_link_oam_htonl(temp32);
            memcpy(error_frame_tlv->error_frame_tlv.error_frame_threshold, &temp32, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_THRESHOLD_LEN);

            memcpy(&temp32, &error_frame_tlv->error_frame_oper_conf.total_error_frames, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_ERROR_FRAMES_LEN);
            temp32 = vtss_eth_link_oam_htonl(temp32);
            memcpy(error_frame_tlv->error_frame_tlv.error_frames, &temp32, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_ERROR_FRAMES_LEN);

            temp64 = vtss_eth_link_oam_swap64(error_frame_tlv->error_frame_oper_conf.total_errors_occured);

            temp64 += GET_PORT_CLIENT_ERROR_FRAME_TOTAL_ERROR_FRAMES(port_no);

            /*memcpy(&temp64,
                   &(error_frame_tlv->error_frame_oper_conf.ifInErrors_at_timeout),
                   VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_FRAMES_LEN); */

            temp64 = vtss_eth_link_oam_swap64(temp64);

            memcpy(error_frame_tlv->error_frame_tlv.error_running_total, &temp64, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_FRAMES_LEN);

            error_frame_tlv->error_frame_oper_conf.total_errors_occured = temp64;

            temp32 = vtss_eth_link_oam_htonl(error_frame_tlv->error_frame_oper_conf.total_events_occured);

            memcpy(error_frame_tlv->error_frame_tlv.event_running_total, &temp32, VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TOTAL_ERROR_EVENTS_LEN);
            /* Tell the caller to send the PDU */
            *is_error_frame_xmit_needed = TRUE;
        } else {
            *is_error_frame_xmit_needed = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_is_symbol_period_frame_window_expired(const u32 port_no, BOOL *const is_frame_window_expired)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (is_frame_window_expired == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_WINDOW(port_no) <= VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS) {
            *is_frame_window_expired = TRUE;
        } else {
            *is_frame_window_expired = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_symbol_period_error_oper_conf_update(const u32 port_no, const client_link_error_symbol_period_event_oper_conf_t *oper_conf, const u32 flags)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (oper_conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET) {
            SET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no, oper_conf->mgmt_event_window);
        }

        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET) {
            SET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_WINDOW(port_no, GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no));
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET) {
            SET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_WINDOW(port_no, (GET_PORT_CLIENT_SYMBOL_PERIOD_ERROR_FRAME_WINDOW(port_no) - oper_conf->oper_event_window));
        }

        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET) {
            SET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_START(port_no, GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_TIMEOUT(port_no));
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET) {
            SET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_TIMEOUT(port_no, oper_conf->symbolErrors_at_timeout);
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET) {
            SET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_THRESHOLD(port_no, oper_conf->oper_error_frame_threshold);
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_RXTHRESHOLD_SET) {
            SET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_RXTHRESHOLD(port_no, oper_conf->oper_error_frame_rxthreshold);
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_symbol_period_error_conf_set(const u32 port_no, BOOL *const is_symbol_period_xmit_needed)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    u32                                 temp32;
    u64                                 temp64;

    vtss_eth_link_oam_client_symbol_period_errors_info_t *symbol_period_tlv;


    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        /* Update the fields in the error_frame_link_event_oper_structure */
        temp64 = GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_TIMEOUT(port_no);
        if (temp64 >= GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_START(port_no)) {
            temp64 = temp64 - GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_AT_START(port_no);
        } else {
            *is_symbol_period_xmit_needed = FALSE;
            break;
        }
        SET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_ERROR_SYMBOLS(port_no, temp64);
#if 0
        if (!GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_THRESHOLD(port_no)) {
            *is_symbol_period_xmit_needed = FALSE;
            break;
        }
#endif

        /* Check if the errors has exceeded the threshold */
        if (GET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_ERROR_SYMBOLS(port_no) >=
            GET_PORT_CLIENT_SYMBOL_PERIOD_ERRORS_THRESHOLD(port_no)) {

            /* Increment the total_events_occured field */
            temp32 = (GET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_EVENTS_OCCURED(port_no) + 1);
            SET_PORT_CLIENT_SYMBOL_PERIOD_TOTAL_EVENTS_OCCURED(port_no, temp32);

            /* Update the fields in the symbol_period_error_tlv structure */
            symbol_period_tlv = PORT_CLIENT_SYMBOL_PERIOD_ERROR_CONF(port_no);

            /* Clear the PDU Strcture */
            memset(&symbol_period_tlv->symbol_period_error_tlv, VTSS_ETH_LINK_OAM_NULL, sizeof(vtss_eth_link_oam_error_symbol_period_event_tlv_t));
            /* Update the PDU Structure */
            symbol_period_tlv->symbol_period_error_tlv.event_type = VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_SYMBOL_PERIOD_EVENT;
            symbol_period_tlv->symbol_period_error_tlv.event_length = VTSS_ETH_LINK_OAM_LINK_MONITORING_SYMBOL_PERIOD_EVENT_LEN;

            memcpy(&temp64, &symbol_period_tlv->symbol_period_error_oper_conf.oper_event_window, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_WINDOW_LEN);
            temp64 = vtss_eth_link_oam_swap64(temp64);
            memcpy(symbol_period_tlv->symbol_period_error_tlv.error_symbol_window, &temp64, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_WINDOW_LEN);
            memcpy(&temp64, &symbol_period_tlv->symbol_period_error_oper_conf.oper_error_frame_threshold, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_THRESHOLD_LEN);
            temp64 = vtss_eth_link_oam_swap64(temp64);
            memcpy(symbol_period_tlv->symbol_period_error_tlv.error_symbol_threshold, &temp64, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_THRESHOLD_LEN);

            memcpy(&temp64, &symbol_period_tlv->symbol_period_error_oper_conf.total_error_symbols, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_ERROR_SYMBOLS_LEN);
            temp64 = vtss_eth_link_oam_swap64(temp64);
            memcpy(symbol_period_tlv->symbol_period_error_tlv.error_symbols, &temp64, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_ERROR_SYMBOLS_LEN);

            memcpy(&temp64, &symbol_period_tlv->symbol_period_error_oper_conf.symbolErrors_at_timeout, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_SYMBOLS_LEN);
            temp64 = vtss_eth_link_oam_swap64(temp64);
            memcpy(symbol_period_tlv->symbol_period_error_tlv.error_running_total, &temp64, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_SYMBOLS_LEN);

            temp32 = vtss_eth_link_oam_htonl(symbol_period_tlv->symbol_period_error_oper_conf.total_events_occured);
            memcpy(symbol_period_tlv->symbol_period_error_tlv.event_running_total, &temp32, VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN);

            /* Tell the caller to send the PDU */
            *is_symbol_period_xmit_needed = TRUE;
        } else {
            *is_symbol_period_xmit_needed = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_is_frame_period_frame_window_expired(const u32 port_no, BOOL *const is_frame_window_expired)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (is_frame_window_expired == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (GET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_WINDOW(port_no) <= VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS) {
            *is_frame_window_expired = TRUE;
        } else {
            *is_frame_window_expired = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_frame_period_error_oper_conf_update(const u32 port_no, const client_link_error_frame_period_event_oper_conf_t *oper_conf, const u32 flags)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (oper_conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET) {
            SET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no, oper_conf->mgmt_event_window);
        }

        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET) {
            SET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_WINDOW(port_no, GET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_MGMT_WINDOW(port_no));
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET) {
            SET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_WINDOW(port_no, (GET_PORT_CLIENT_FRAME_PERIOD_ERROR_FRAME_WINDOW(port_no) - oper_conf->oper_event_window));
        }

        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET) {
            SET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_START(port_no, GET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_TIMEOUT(port_no));
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET) {
            SET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_TIMEOUT(port_no, oper_conf->frameErrors_at_timeout);
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET) {
            SET_PORT_CLIENT_FRAME_PERIOD_ERROR_THRESHOLD(port_no, oper_conf->oper_error_frame_threshold);
        }
        if (flags & VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_RXTHRESHOLD_SET) {
            SET_PORT_CLIENT_FRAME_PERIOD_ERROR_RXTHRESHOLD(port_no, oper_conf->oper_error_frame_rxthreshold);
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}


u32 vtss_eth_link_oam_client_frame_period_error_conf_set(const u32 port_no, BOOL *const is_frame_period_xmit_needed)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    u32                                 temp32;
    u64                                 temp64;

    vtss_eth_link_oam_client_frame_period_errors_info_t *frame_period_tlv;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        /* Update the fields in the error_frame_link_event_oper_structure */
        SET_PORT_CLIENT_FRAME_PERIOD_TOTAL_ERROR_FRAMES(port_no, (GET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_TIMEOUT(port_no) - GET_PORT_CLIENT_FRAME_PERIOD_ERRORS_AT_START(port_no)));

        if (!GET_PORT_CLIENT_FRAME_PERIOD_ERROR_THRESHOLD(port_no)) {
            *is_frame_period_xmit_needed = FALSE;
            break;
        }

        /* Check if the errors has exceeded the threshold */
        if (GET_PORT_CLIENT_FRAME_PERIOD_TOTAL_ERROR_FRAMES(port_no) >
            GET_PORT_CLIENT_FRAME_PERIOD_ERROR_THRESHOLD(port_no)) {

            /* Increment the total_events_occured field */
            SET_PORT_CLIENT_FRAME_PERIOD_TOTAL_EVENTS_OCCURED(port_no, (GET_PORT_CLIENT_FRAME_PERIOD_TOTAL_EVENTS_OCCURED(port_no) + 1));

            /* Update the fields in the frame_period_error_tlv structure */
            frame_period_tlv = PORT_CLIENT_FRAME_PERIOD_ERROR_CONF(port_no);

            /* Clear the PDU Strcture */
            memset(&(frame_period_tlv->frame_period_error_tlv), VTSS_ETH_LINK_OAM_NULL, sizeof(vtss_eth_link_oam_error_frame_period_event_tlv_t));
            /* Update the PDU Structure */
            frame_period_tlv->frame_period_error_tlv.event_type = VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_PERIOD_EVENT;
            frame_period_tlv->frame_period_error_tlv.event_length = VTSS_ETH_LINK_OAM_LINK_MONITORING_FRAME_PERIOD_EVENT_LEN;
            memcpy(&temp32, &frame_period_tlv->frame_period_error_oper_conf.oper_event_window, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_WINDOW_LEN);
            temp32 = vtss_eth_link_oam_htonl(temp32);
            memcpy(frame_period_tlv->frame_period_error_tlv.error_frame_period_window, &(temp32), VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_WINDOW_LEN);
            memcpy(&temp32, &frame_period_tlv->frame_period_error_oper_conf.oper_error_frame_threshold, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_THRESHOLD_LEN);
            temp32 = vtss_eth_link_oam_htonl(temp32);
            memcpy(frame_period_tlv->frame_period_error_tlv.error_frame_threshold, &temp32, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_THRESHOLD_LEN);
            memcpy(&temp32, &frame_period_tlv->frame_period_error_oper_conf.total_error_frames, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_ERROR_FRAMES_LEN);
            temp32 = vtss_eth_link_oam_htonl(temp32);
            memcpy(frame_period_tlv->frame_period_error_tlv.error_frames, &temp32, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_ERROR_FRAMES_LEN);
            memcpy(&temp64, &frame_period_tlv->frame_period_error_oper_conf.frameErrors_at_timeout, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_FRAMES_LEN);
            temp64 = vtss_eth_link_oam_swap64(temp64);
            memcpy(frame_period_tlv->frame_period_error_tlv.error_running_total, &temp64, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_FRAMES_LEN);

            temp32 = vtss_eth_link_oam_htonl(frame_period_tlv->frame_period_error_oper_conf.total_events_occured);
            memcpy(frame_period_tlv->frame_period_error_tlv.event_running_total, &temp32, VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TOTAL_ERROR_EVENTS_LEN);

            /* Tell the caller to send the PDU */
            *is_frame_period_xmit_needed = TRUE;
        } else {
            *is_frame_period_xmit_needed = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_is_error_frame_secs_summary_window_expired(const u32 port_no, BOOL *is_frame_window_expired)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (is_frame_window_expired == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_WINDOW(port_no) <= VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS) {
            *is_frame_window_expired = TRUE;
        } else {
            *is_frame_window_expired = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(const u32 port_no, const vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf_t *oper_conf, const u32 flags)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (oper_conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_MGMT_WINDOW(port_no, oper_conf->mgmt_event_window);
        }

        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_WINDOW(port_no, GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_MGMT_WINDOW(port_no));
        }

        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET) {
            SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_WINDOW(port_no, (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_WINDOW(port_no) - oper_conf->oper_event_window));
        }

        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_START(port_no, GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_TIMEOUT(port_no));
        }

        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_TIMEOUT(port_no, oper_conf->secErrors_at_timeout);
        }

        if (flags &  VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET) {
            SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_THRESHOLD(port_no, oper_conf->oper_secs_summary_threshold);
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_error_frame_secs_summary_event_conf_set(const u32 port_no, const BOOL is_timer_expired, BOOL *is_error_frame_xmit_needed)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_client_error_frame_secs_summary_info_t
    *error_frame_secs_summary_tlv = NULL;
    u16                                  temp16;
    u32                                  temp32;

    do {

        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        /* Update the fields in the error_frame_link_event_oper_structure */
        temp32 = (GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_TIMEOUT(port_no));
        if (temp32 >= GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_START(port_no)) {
            temp32 = temp32 - GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_AT_START(port_no);
        } else {
            *is_error_frame_xmit_needed = FALSE;
            break;
        }
        if (temp32 >= 1) { /* Increment the error seconds */
            PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info.error_frame_secs_summary_oper_conf.total_errord_seconds++;
        }
        //SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_ERROR_FRAMES(port_no,temp32);
        if (is_timer_expired == FALSE) {
            *is_error_frame_xmit_needed = FALSE;
            break;
        }
#if 0
        if (!GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_THRESHOLD(port_no)) {
            *is_error_frame_xmit_needed = FALSE;
            break;
        }
#endif
        temp16 = PORT_CLIENT_CONF(port_no)->error_frame_secs_summary_info.error_frame_secs_summary_oper_conf.total_errord_seconds;
        /* Check if the errors has exceeded the threshold */
        if (temp16 >= GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_ERROR_THRESHOLD(port_no) ) {

            /* Increment the total_events_occured field */
            temp32 = GET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_EVENTS_OCCURED(port_no) + 1;
            SET_PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_TOTAL_EVENTS_OCCURED(port_no, temp32);

            /* Update the fields in the error_frame_event_tlv structure */
            error_frame_secs_summary_tlv = PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_CONF(port_no);
            /* Clear the PDU Strcture */
            memset(&(error_frame_secs_summary_tlv->error_frame_secs_summary_tlv), VTSS_ETH_LINK_OAM_NULL, sizeof(vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t));
            /* Update the PDU Structure */
            error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.event_type = VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_SECS_SUMMARY_EVENT ;
            error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.event_length = VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_SECS_SUM_EVENT_LEN;
            memcpy(&temp16, &error_frame_secs_summary_tlv->error_frame_secs_summary_oper_conf.oper_event_window, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_WINDOW_LEN);
            temp16 = vtss_eth_link_oam_htons(temp16);
            memcpy(error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.secs_summary_window, &temp16, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_WINDOW_LEN);

            memcpy(&temp16, &error_frame_secs_summary_tlv->error_frame_secs_summary_oper_conf.oper_secs_summary_threshold, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_THRESHOLD_LEN);
            temp16 = vtss_eth_link_oam_htons(temp16);
            memcpy(error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.secs_summary_threshold, &temp16, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_THRESHOLD_LEN);

            memcpy(&temp16, &error_frame_secs_summary_tlv->error_frame_secs_summary_oper_conf.total_errord_seconds, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_ERROR_FRAMES_LEN);
            temp16 = vtss_eth_link_oam_htons(temp16);
            memcpy(error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.secs_summary_events, &temp16, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_ERROR_FRAMES_LEN);

            memcpy(&temp32, &error_frame_secs_summary_tlv->error_frame_secs_summary_oper_conf.total_secErrorSummary_frames, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_FRAMES_LEN);

            /* Accumalte all the events */
            temp32 = temp32 + error_frame_secs_summary_tlv->error_frame_secs_summary_oper_conf.total_errord_seconds;
            error_frame_secs_summary_tlv->error_frame_secs_summary_oper_conf.total_secErrorSummary_frames = temp32;
            temp32 = vtss_eth_link_oam_htonl(temp32);
            memcpy(error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.error_running_total, &temp32, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_FRAMES_LEN);

            temp32 = vtss_eth_link_oam_htonl(error_frame_secs_summary_tlv->error_frame_secs_summary_oper_conf.total_secErrorEvents_occured);

            memcpy(error_frame_secs_summary_tlv->error_frame_secs_summary_tlv.event_running_total, &temp32, VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TOTAL_EVENTS_LEN);
            /* Tell the caller to send the PDU */
            *is_error_frame_xmit_needed = TRUE;
            /* Reset the errored frame counts */
            error_frame_secs_summary_tlv->error_frame_secs_summary_oper_conf.total_errord_seconds = 0;
        } else {
            *is_error_frame_xmit_needed = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_client_port_build_var_descriptor(const u32 port_no, u8 *pdu, const u8 var_branch, const u16 var_leaf, u16 *current_pdu_len, const u16  max_pdu_len)

{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;
    u16 temp16 = vtss_eth_link_oam_htons(var_leaf); //Temp variable for htons

    do {
        if ((pdu == NULL) || (current_pdu_len == VTSS_ETH_LINK_OAM_NULL)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if ((VTSS_ETH_LINK_OAM_VAR_DESCRIPTOR_LEN + (*current_pdu_len)) > (max_pdu_len)) {
            rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
            break;
        }
        memcpy(pdu + (*current_pdu_len), &var_branch, VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN);
        memcpy(pdu + (*current_pdu_len) + VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN, &temp16, VTSS_ETH_LINK_OAM_VAR_LEAF_LEN);
        *current_pdu_len += VTSS_ETH_LINK_OAM_VAR_DESCRIPTOR_LEN;

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}


static inline void vtss_eth_link_oam_fill_var_container(const u8 var_branch, const u16 var_leaf, const u8 var_width, u8 *pdu_data, u8 *leaf_data)

{
    memcpy(pdu_data, &var_branch, VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN);
    memcpy(pdu_data + VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN, &var_leaf, VTSS_ETH_LINK_OAM_VAR_LEAF_LEN);
    memcpy(pdu_data + VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN + VTSS_ETH_LINK_OAM_VAR_LEAF_LEN, &var_width, VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_LEN);

    do {
        if (var_width) {
            if (var_width & VTSS_ETH_LINK_OAM_VAR_INDICATOR_VAL) {
                break; /* No need of filling the data */
            } else {
                memcpy(pdu_data + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN, leaf_data, var_width);
            }
        } else { /* MAX width case to indicate an error */
            memcpy(pdu_data + VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN + VTSS_ETH_LINK_OAM_VAR_LEAF_LEN, leaf_data, VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_MAX_LEN);
        }
    } while (VTSS_ETH_LINK_OAM_NULL); //End of filling the Var container
}

u32 vtss_eth_link_oam_client_port_build_remote_loop_back_tlv(const u32 port_no, u8 *pdu, u16 *current_pdu_len, u16 max_pdu_len, u8 lb_flag)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if ((pdu == NULL) || (current_pdu_len == NULL)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        pdu[*current_pdu_len] = lb_flag;
        if ( ((*current_pdu_len) + VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_REQUEST_MAX_LEN) > max_pdu_len) {
            rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
            break;
        } else {
            (*current_pdu_len)++;
        }

    } while (VTSS_ETH_LINK_OAM_NULL);

    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Error occured while building the loop-back tlv", port_no, rc, 0, 0);
    }
    return rc;
}


u32 vtss_eth_link_oam_client_port_remote_loop_back_oper_set(const u32 port_no, const BOOL enable_flag)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8  lb_data[VTSS_ETH_LINK_OAM_PDU_MIN_LEN];
    u8  lb_flag;
    u16 current_pdu_len = VTSS_ETH_LINK_OAM_PDU_HDR_LEN;
    u8  port_state;
    u8  data[VTSS_ETH_LINK_OAM_INFO_DATA_LEN];

    vtss_eth_link_oam_parser_state_t   port_parser_state;

    do {

        lb_flag = ( (enable_flag == TRUE) ? (VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_ENABLE_REQUEST) : (VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_DISABLE_REQUEST) );

        port_state = GET_PORT_STATE(port_no);
        port_parser_state = GET_PORT_PARSER_STATE(port_no);

        if (lb_flag == VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_ENABLE_REQUEST) {
            if (port_parser_state != VTSS_ETH_LINK_OAM_PARSER_FWD_STATE) {
                rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
                break;
            }
            rc = vtss_eth_link_oam_client_port_state_conf_set(port_no, VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE, VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE, TRUE, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }

        } else if (lb_flag == VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_DISABLE_REQUEST) {
            if (port_parser_state != VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE) {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }
            SET_PORT_MUX_STATE(port_state, VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE);
            SET_PORT_STATE(port_no, port_state);
        }

        memset(lb_data, '\0', VTSS_ETH_LINK_OAM_PDU_MIN_LEN);

        rc = vtss_eth_link_oam_client_port_build_remote_loop_back_tlv(port_no, lb_data, &current_pdu_len, VTSS_ETH_LINK_OAM_PDU_MIN_LEN, lb_flag);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        rc = vtss_eth_link_oam_control_port_non_info_pdu_xmit(port_no, lb_data, VTSS_ETH_LINK_OAM_PDU_MIN_LEN, VTSS_ETH_LINK_OAM_CODE_TYPE_LB);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        if (vtss_eth_link_oam_client_port_revision_update(port_no, TRUE) != VTSS_RC_OK) {
            break;
        }
        memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
        memcpy(data, PORT_CLIENT_LOCAL_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
        memcpy(data + VTSS_ETH_LINK_OAM_INFO_TLV_LEN, PORT_CLIENT_REMOTE_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
        rc = vtss_eth_link_oam_control_port_data_set(port_no, data, FALSE, FALSE);

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

i8  *vtss_eth_link_oam_get_str(const u32 var_leaf)
{
    switch (var_leaf) {
    case VTSS_ETH_LINK_OAM_ID:
        return (i8 *)"aOAMID";
    case VTSS_ETH_LINK_OAM_LOCAL_CONF:
        return (i8 *)"aOAMLocalConfiguration";
    case VTSS_ETH_LINK_OAM_LOCAL_PDU_CONF:
        return (i8 *)"aOAMLocalPDUConfiguration";
    case VTSS_ETH_LINK_OAM_LOCAL_REVISION_CONF:
        return (i8 *)"aOAMLocalRevision";
    case VTSS_ETH_LINK_OAM_LOCAL_STATE:
        return (i8 *)"aOAMLocalState";
    case VTSS_ETH_LINK_OAM_REMOTE_CONF:
        return (i8 *) "aOAMRemoteConfiguration";
    case VTSS_ETH_LINK_OAM_REMOTE_PDU_CONF:
        return (i8 *)"aOAMRemotePDUConfiguration";
    case VTSS_ETH_LINK_OAM_REMOTE_REVISION:
        return (i8 *)"aOAMRemoteRevision";
    case VTSS_ETH_LINK_OAM_REMOTE_STATE:
        return (i8 *)"aOAMRemoteState";
    default:
        return (i8 *)"Not Supported";
    }
}

/* This function needs to re-looked for giving CMIP interface */
u32 vtss_eth_link_oam_client_port_build_var_container(const u32 port_no, u8 *pdu, const u8  var_branch, const u16 var_leaf, u16  *current_pdu_len, const u16  max_pdu_len)

{
    u32                          rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8                           var_indicator;

    /* temp variables for htons/l, ntohs/l functions */
    u8                           temp8;
    u32                          temp32;
    u16                          temp16, temp162;
    vtss_eth_link_oam_info_tlv_t local_info;

    temp16 = vtss_eth_link_oam_htons(var_leaf);
    var_indicator = VTSS_ETH_LINK_OAM_VAR_INDICATOR_VAL;

    do {
        if ( (pdu == NULL) || (current_pdu_len == VTSS_ETH_LINK_OAM_NULL)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (var_branch != VTSS_ETH_LINK_OAM_VAR_ATTRIBUTE) {

            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG, "Unsupported branch is requested", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            break;
        }
        switch (var_leaf) {
        case VTSS_ETH_LINK_OAM_ID:
            if ( (*current_pdu_len + (VTSS_ETH_LINK_OAM_ID_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN)) >
                 max_pdu_len) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            temp32 = vtss_eth_link_oam_htonl(port_no);
            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_ID_LEN, pdu + (*current_pdu_len), (u8 *)&temp32);
            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + VTSS_ETH_LINK_OAM_ID_LEN);
            break;

        case VTSS_ETH_LINK_OAM_LOCAL_CONF:
            if ( (*current_pdu_len + (VTSS_ETH_LINK_OAM_LOCAL_CONF_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN)) > max_pdu_len) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            rc = vtss_eth_link_oam_client_port_local_info_get(port_no, &local_info);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
                break;
            }
            temp8 = local_info.oam_conf;
            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_LOCAL_CONF_LEN, pdu + (*current_pdu_len), &temp8);
            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN +
                                   VTSS_ETH_LINK_OAM_LOCAL_CONF_LEN);
            break;

        case VTSS_ETH_LINK_OAM_LOCAL_PDU_CONF:
            if ( (*current_pdu_len + VTSS_ETH_LINK_OAM_LOCAL_PDU_CONF_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN) > max_pdu_len
               ) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            rc = vtss_eth_link_oam_client_port_local_info_get(port_no, &local_info);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            }
            memcpy(&temp162, local_info.oampdu_conf, sizeof(temp16));
            temp32 = vtss_eth_link_oam_ntohs(temp162);
            temp32 = vtss_eth_link_oam_htonl(temp32);
            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_LOCAL_PDU_CONF_LEN, pdu + (*current_pdu_len), (u8 *)&temp32);
            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + VTSS_ETH_LINK_OAM_LOCAL_PDU_CONF_LEN);
            break;

        case VTSS_ETH_LINK_OAM_LOCAL_REVISION_CONF:
            if ( (*current_pdu_len + VTSS_ETH_LINK_OAM_LOCAL_REVISION_CONF_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN) > max_pdu_len) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            rc = vtss_eth_link_oam_client_port_local_info_get(port_no, &local_info);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            }
            memcpy(&temp162, local_info.revision, sizeof(temp16));
            temp32 = vtss_eth_link_oam_ntohs(temp162);
            temp32 = vtss_eth_link_oam_htonl(temp32);

            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_LOCAL_REVISION_CONF_LEN, pdu + (*current_pdu_len), (u8 *)&temp32);

            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + VTSS_ETH_LINK_OAM_LOCAL_REVISION_CONF_LEN);
            break;

        case VTSS_ETH_LINK_OAM_LOCAL_STATE:
            if ( (*current_pdu_len + VTSS_ETH_LINK_OAM_LOCAL_STATE_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN)
                 >    max_pdu_len) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            rc = vtss_eth_link_oam_client_port_local_info_get(port_no, &local_info);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
                break;
            }
            temp8 = local_info.state;
            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_LOCAL_STATE_LEN, pdu + (*current_pdu_len), &temp8);
            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + VTSS_ETH_LINK_OAM_LOCAL_STATE_LEN);
            break;

        case VTSS_ETH_LINK_OAM_REMOTE_CONF:
            if ( (*current_pdu_len + VTSS_ETH_LINK_OAM_REMOTE_CONF_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN) > max_pdu_len) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            rc = vtss_eth_link_oam_client_port_remote_info_get(port_no, &local_info);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
                break;
            }
            temp8 = local_info.oam_conf;
            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_LOCAL_CONF_LEN, pdu + (*current_pdu_len), &temp8);
            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + VTSS_ETH_LINK_OAM_REMOTE_CONF_LEN);
            break;

        case VTSS_ETH_LINK_OAM_REMOTE_PDU_CONF:
            if ( (*current_pdu_len + VTSS_ETH_LINK_OAM_REMOTE_PDU_CONF_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN) > max_pdu_len) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            rc = vtss_eth_link_oam_client_port_remote_info_get(port_no, &local_info);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            }
            memcpy(&temp162, local_info.oampdu_conf, sizeof(u16));
            temp32 = vtss_eth_link_oam_ntohs(temp162);
            temp32 = vtss_eth_link_oam_htonl(temp32);
            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_REMOTE_PDU_CONF_LEN, pdu + (*current_pdu_len), (u8 *)&temp32);
            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + VTSS_ETH_LINK_OAM_REMOTE_PDU_CONF_LEN);
            break;

        case VTSS_ETH_LINK_OAM_REMOTE_REVISION:
            if ( (*current_pdu_len + VTSS_ETH_LINK_OAM_REMOTE_REVISION_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN) > max_pdu_len) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            rc = vtss_eth_link_oam_client_port_remote_info_get(port_no, &local_info);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
                break;
            }
            memcpy(&temp162, local_info.revision, sizeof(temp16));
            temp32 = vtss_eth_link_oam_ntohs(temp162);
            temp32 = vtss_eth_link_oam_htonl(temp32);

            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_REMOTE_REVISION_LEN, pdu + (*current_pdu_len), (u8 *)&temp32);

            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + VTSS_ETH_LINK_OAM_REMOTE_REVISION_LEN);
            break;

        case VTSS_ETH_LINK_OAM_REMOTE_STATE:
            if ( (*current_pdu_len + VTSS_ETH_LINK_OAM_REMOTE_STATE_LEN + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN) > max_pdu_len) {
                rc = VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
                break;
            }
            rc = vtss_eth_link_oam_client_port_remote_info_get(port_no, &local_info);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
                break;
            }
            temp8 = local_info.state;
            vtss_eth_link_oam_fill_var_container(var_branch, temp16, VTSS_ETH_LINK_OAM_REMOTE_STATE_LEN, pdu + (*current_pdu_len), &temp8);
            (*current_pdu_len) += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + VTSS_ETH_LINK_OAM_REMOTE_STATE_LEN);
            break;
        default :
            rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            break;
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    switch (rc) { /* This switch case fills the variable indicators */
    case VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY:
        var_indicator |= VTSS_ETH_LINK_OAM_VAR_PDU_LEN_IS_NOT_SUFFICINET;
        vtss_eth_link_oam_fill_var_container(var_branch, temp16, (var_indicator), pdu + VTSS_ETH_LINK_OAM_PDU_HDR_LEN, (u8 *)&temp32);
        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG, "Variable response exceeds the max length", port_no, 0, 0, 0);
        break;
    case VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED:
        switch (var_branch) {
        case VTSS_ETH_LINK_OAM_VAR_ATTRIBUTE:
            var_indicator |= VTSS_ETH_LINK_OAM_VAR_ATTR_NOT_SUPPORTED;
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG, "Unsupported attribute is requested",
                                    port_no, 0, 0, 0);
            break;
        case VTSS_ETH_LINK_OAM_VAR_OBJECT:
            var_indicator |= VTSS_ETH_LINK_OAM_VAR_OBJ_NOT_SUPPORTED;
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "Unsupported object is requested",
                                    port_no, 0, 0, 0);
            break;
        case VTSS_ETH_LINK_OAM_VAR_PACKAGE:
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "Unsupported package is requested",
                                    port_no, 0, 0, 0);
            var_indicator |= VTSS_ETH_LINK_OAM_VAR_PACK_NOT_SUPPORTED;
            break;
        default:
            //This case should not happen
            break;
        } /* end of filling the indicators */
        vtss_eth_link_oam_fill_var_container(var_branch,
                                             temp16,
                                             (var_indicator),
                                             pdu + VTSS_ETH_LINK_OAM_PDU_HDR_LEN,
                                             (u8 *)&temp32);
        break;
    default :
        break;
    }
    return rc;
}

static inline u32  vtss_eth_link_oam_client_record_info_data(const u32 port_no, vtss_eth_link_oam_info_tlv_t  *local_info, vtss_eth_link_oam_info_tlv_t  *remote_info, BOOL  reset_port_oper, BOOL  is_port_active)
{
    u8              data[VTSS_ETH_LINK_OAM_INFO_DATA_LEN];
    u32             rc = VTSS_ETH_LINK_OAM_RC_OK;

    memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
    if (local_info != NULL) {
        memcpy(data, local_info, VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
    }
    if (remote_info != NULL) {
        memcpy(data + VTSS_ETH_LINK_OAM_INFO_TLV_LEN, remote_info,
               VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
    }
    rc = vtss_eth_link_oam_control_port_data_set(port_no, data,
                                                 reset_port_oper, is_port_active);

    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR,
                                "Error occured while recording OAM data",
                                port_no, rc, 0, 0);
    }

    return rc;
}


/* Function to handle the received information OAM PDU */
u32 vtss_eth_link_oam_client_port_pdu_info_handler(const u32 port_no, const u8  *pdu, const u16 flags, const u16 pdu_len)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_client_conf_t     *port_conf;
    vtss_eth_link_oam_info_tlv_t        *pdu_local_info, *pdu_remote_info;
    vtss_eth_link_oam_info_tlv_t        *port_remote_tlv;
    u8                                  tmp_port_state;
    u16                                 tmp_pdu_max_len;
    vtss_eth_link_oam_mux_state_t       old_mux_state;
    BOOL                                is_satisfied = TRUE;


    do {
        port_conf = PORT_CLIENT_CONF(port_no);

        if (pdu == NULL) {
            //Invalid TLV
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        vtss_eth_link_oam_trace (VTSS_ETH_LINK_OAM_TRACE_LEVEL_INFO, "Eval & stable",
                                 port_no, IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE),
                                 IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE), 0);


        if ( (IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE)) &&
             (IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE))) {
            // Peer wants the discovery to terminate
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "local_stable and local_evalute are turned on",
                                    port_no, 0, 0, 0);

            rc = vtss_eth_link_oam_client_port_discovery_protocol_init(port_no, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_control_port_oper_init(port_no,
                                                          IS_PORT_CONF_ACTIVE(port_no, VTSS_ETH_LINK_OAM_CONF_MODE));
            break;
        } /* end of the logic to take care of invalid satisfied fields */
        if (IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT)) {
            /* Ignore the Link Fault frames so that Link time-out occurs */
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "Peer's Link fault operation is occured",
                                    port_no, 0, 0, 0);
            break;
        } /* Drops the Link fault PDUs. We may need to customise this */

        pdu_local_info = (vtss_eth_link_oam_info_tlv_t *)(
                             pdu + VTSS_ETH_LINK_OAM_PDU_HDR_LEN);

        /* Ignore reserved fields from the Local info TLV of PDU*/
        pdu_local_info->state &= 0x07;
        pdu_local_info->oam_conf &= 0x01f;
        memcpy(&tmp_pdu_max_len, pdu_local_info->oampdu_conf, VTSS_ETH_LINK_OAM_CONF_LEN);
        tmp_pdu_max_len = vtss_eth_link_oam_ntohs(tmp_pdu_max_len);
        tmp_pdu_max_len &= 0x7ff;
        if (tmp_pdu_max_len >= 64) {
            is_satisfied &= TRUE;
        } else {
            is_satisfied &= FALSE;
        }
        tmp_pdu_max_len = vtss_eth_link_oam_htons(tmp_pdu_max_len);
        memcpy(pdu_local_info->oampdu_conf, &tmp_pdu_max_len, VTSS_ETH_LINK_OAM_CONF_LEN);

        pdu_remote_info = (vtss_eth_link_oam_info_tlv_t *)(
                              pdu + VTSS_ETH_LINK_OAM_PDU_HDR_LEN +
                              VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
        port_remote_tlv = PORT_CLIENT_REMOTE_CONF(port_no);
        if (port_conf->remote_state_valid == TRUE) {
            rc = vtss_eth_link_oam_control_port_flags_conf_set(port_no,
                                                               VTSS_ETH_LINK_OAM_FLAG_REMOTE_EVALUTE,
                                                               IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE) ? TRUE : FALSE);

            if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
                 (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED) ) {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }

            rc = vtss_eth_link_oam_control_port_flags_conf_set(port_no,
                                                               VTSS_ETH_LINK_OAM_FLAG_REMOTE_STABLE,
                                                               IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE) ? TRUE : FALSE);

            if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
                 (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED) ) {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }
        }

        if (!IS_PORT_CONF_ACTIVE(port_no, VTSS_ETH_LINK_OAM_CONF_MODE) &&
            !IS_PDU_CONF_ACTIVE(pdu_local_info->oam_conf,
                                VTSS_ETH_LINK_OAM_CONF_MODE)) {

            is_satisfied &= FALSE;
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "OAM PDUs are coming from passive peer",
                                    port_no, 0, 0, flags);
        } else {
            /* end of the logic to take care of discovery between two
                           passive devices */
            is_satisfied &= TRUE;
        }

        pdu_local_info->info_type = VTSS_ETH_LINK_OAM_REMOTE_INFO_TLV;
        if (port_conf->remote_state_valid == FALSE) {

            if ( (GET_PDU_MUX_STATE(pdu_local_info->state) !=
                  VTSS_ETH_LINK_OAM_MUX_FWD_STATE) &&
                 (GET_PDU_PARSER_STATE(pdu_local_info->state) !=
                  VTSS_ETH_LINK_OAM_PARSER_FWD_STATE) ) {
                break; //At this stage of the protocol, state should be in FWD state
            }

            memcpy(port_remote_tlv, pdu_local_info,
                   VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
            rc = vtss_eth_link_oam_client_record_info_data(port_no,
                                                           PORT_CLIENT_LOCAL_CONF(port_no),
                                                           port_remote_tlv, FALSE, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "Remote state is valid",
                                    port_no, 0, 0, 0);

            port_conf->remote_state_valid = TRUE;
            rc = vtss_eth_link_oam_control_port_remote_state_valid_set(port_no);

            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_control_port_flags_conf_set(port_no,
                                                               VTSS_ETH_LINK_OAM_FLAG_REMOTE_EVALUTE,
                                                               IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE) ? TRUE : FALSE);

            if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
                 (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED) ) {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }

            rc = vtss_eth_link_oam_control_port_flags_conf_set(port_no,
                                                               VTSS_ETH_LINK_OAM_FLAG_REMOTE_STABLE,
                                                               IS_FLAG_CONF_ACTIVE(flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE) ? TRUE : FALSE);

            if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
                 (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED) ) {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }

        } else { /* end of logic to handle the remote-state */

            if (memcmp(port_conf->remote_mac_addr,
                       pdu + VTSS_ETH_LINK_OAM_MAC_LEN,
                       VTSS_ETH_LINK_OAM_MAC_LEN)) {

                vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                        "OAM Info PDU is received from a different peer",
                                        port_no, 0, 0, 0);
                /* We are just ignoring the frame here. So that Timeout occurs and Discovery
                   restarts */
                is_satisfied &= FALSE;
            }

            if (pdu_remote_info->info_type ==
                VTSS_ETH_LINK_OAM_REMOTE_INFO_TLV) {

                /* Rember the previous MUX state prior to recording
                   PDU information */
                old_mux_state = GET_REMOTE_PORT_MUX_STATE(port_no);

                /* Copy the PDU local TLV as port remote TLV */
                memcpy(port_remote_tlv, pdu_local_info, VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
                rc = vtss_eth_link_oam_client_record_info_data(port_no,
                                                               PORT_CLIENT_LOCAL_CONF(port_no),
                                                               port_remote_tlv,
                                                               FALSE, FALSE);
                if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                    break;
                }
                if (is_satisfied == FALSE) {

                    if (GET_PORT_PARSER_STATE(port_no) != VTSS_ETH_LINK_OAM_PARSER_FWD_STATE) {
                        /* Current state indicates Remote loopback operation */
                        /* At this stage moving back to previous states can lead to flooding and
                           Standard suggests proper hand-shake to initiate or exit the RLB
                           So thinking that it's not practical we will not process further the PDUs.
                           It causes Discovery to restart */

                    } else {
                        port_conf->local_satisfied = FALSE;
                        if (port_conf->remote_stable == TRUE) {
                            port_conf->remote_stable = FALSE;
                        }

                        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                                "local-satisfied is turned off",
                                                port_no, 0, 0, 0);

                        rc = vtss_eth_link_oam_control_port_local_satisfied_set(port_no,
                                                                                FALSE);
                        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                            break;
                        }
                        rc = vtss_eth_link_oam_control_port_flags_conf_set(port_no,
                                                                           VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE, FALSE);
                    }
                } else if (port_conf->local_satisfied == FALSE) {

                    /*Valid mode(active/passive) only taken into consideration
                      for saying the local_satisfied as true.We may need to
                      change the logic, depending upon the other Vendor devices.
                      Standard is not clearly saying about this */

                    vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                            "local-satisfied is turned on",
                                            port_no, 0, 0, 0);
                    port_conf->local_satisfied = TRUE;
                    memcpy(port_remote_tlv, pdu_local_info,
                           VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
                    rc = vtss_eth_link_oam_client_record_info_data(port_no,
                                                                   PORT_CLIENT_LOCAL_CONF(port_no),
                                                                   port_remote_tlv,
                                                                   FALSE, FALSE);
                    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                        break;
                    }
                    rc = vtss_eth_link_oam_control_port_local_satisfied_set(port_no, TRUE);
                    break;
                } else { /* end of logic to handle the newly arrived remote TLV */

                    /* Verify if remote Configuration is changed */
                    if (pdu_local_info->oam_conf != port_remote_tlv->oam_conf) {
                        port_conf->local_satisfied = FALSE;
                        if (port_conf->remote_stable == TRUE) {
                            port_conf->remote_stable = FALSE;
                        }

                        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                                "Peer's OAM configuration is changed",
                                                port_no, 0, 0, 0);

                        rc = vtss_eth_link_oam_control_port_local_satisfied_set(port_no,
                                                                                FALSE);
                        break;
                    }
                }
                if (IS_FLAG_CONF_ACTIVE(flags,
                                        VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE) && (is_satisfied == TRUE)) {
                    if (port_conf->remote_stable == FALSE) {
                        port_conf->remote_stable = TRUE;

                        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                                "remote stable is turned on",
                                                port_no, 0, 0, 0);

                        rc = vtss_eth_link_oam_control_port_remote_stable_set(
                                 port_no, TRUE);
                        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                            break;
                        }
                    }


                    /* Verify if, LB condition is happening.. */
                    if ( (GET_PDU_MUX_STATE(pdu_local_info->state) ==
                          VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) &&
                         (GET_PDU_PARSER_STATE(pdu_local_info->state) ==
                          VTSS_ETH_LINK_OAM_PARSER_LB_STATE)) {
                        if (GET_PORT_MUX_STATE(port_no) ==
                            VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) {
                            tmp_port_state = GET_PORT_STATE(port_no);
                            SET_PORT_MUX_STATE(tmp_port_state,
                                               VTSS_ETH_LINK_OAM_MUX_FWD_STATE);
                            SET_PORT_STATE(port_no, tmp_port_state);

                            rc = vtss_eth_link_oam_client_port_state_conf_set(port_no,
                                                                              VTSS_ETH_LINK_OAM_MUX_FWD_STATE,
                                                                              VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE,
                                                                              TRUE, TRUE);
                            if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
                                vtss_eth_link_oam_rlb_opr_unlock();
                                break;
                            }
                            memcpy(port_remote_tlv,
                                   pdu_local_info, VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
                            rc = vtss_eth_link_oam_client_record_info_data(port_no,
                                                                           PORT_CLIENT_LOCAL_CONF(port_no),
                                                                           port_remote_tlv,
                                                                           FALSE, FALSE);
                            vtss_eth_link_oam_rlb_opr_unlock();
                            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                                break;
                            }
                        }
                    } else if ((GET_PDU_MUX_STATE(pdu_local_info->state) ==
                                VTSS_ETH_LINK_OAM_MUX_FWD_STATE) &&
                               (GET_PDU_PARSER_STATE(pdu_local_info->state) ==
                                VTSS_ETH_LINK_OAM_PARSER_FWD_STATE) &&
                               (old_mux_state ==
                                VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) &&
                               (IS_PDU_CONF_ACTIVE(pdu_local_info->oam_conf,
                                                   VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT)) ) {

                        if ( (GET_PORT_MUX_STATE(port_no) ==
                              VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) &&
                             (GET_PORT_PARSER_STATE(port_no) ==
                              VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE) ) {

                            tmp_port_state = GET_PORT_STATE(port_no);
                            SET_PORT_MUX_STATE(tmp_port_state,
                                               VTSS_ETH_LINK_OAM_MUX_FWD_STATE);
                            SET_PORT_PARSER_STATE(tmp_port_state,
                                                  VTSS_ETH_LINK_OAM_PARSER_FWD_STATE);
                            SET_PORT_STATE(port_no, tmp_port_state);
                            rc = vtss_eth_link_oam_client_port_state_conf_set(port_no,
                                                                              VTSS_ETH_LINK_OAM_MUX_FWD_STATE,
                                                                              VTSS_ETH_LINK_OAM_PARSER_FWD_STATE,
                                                                              TRUE, TRUE);
                            if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
                                vtss_eth_link_oam_rlb_opr_unlock();
                                break;
                            }
                            memcpy(port_remote_tlv,
                                   pdu_local_info,
                                   VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
                            rc = vtss_eth_link_oam_client_record_info_data(port_no,
                                                                           PORT_CLIENT_LOCAL_CONF(port_no),
                                                                           port_remote_tlv,
                                                                           FALSE, FALSE);
                            vtss_eth_link_oam_rlb_opr_unlock();
                            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                                break;
                            }
                        } else if (GET_PORT_PARSER_STATE(port_no) == VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE) {
                            /* This is a negative case. It should not happen as this port is RLB State */
                            break;
                        }

                    }
                    if (pdu_local_info->state !=  port_remote_tlv->state) {
                        memcpy(port_remote_tlv, pdu_local_info,
                               VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
                        rc = vtss_eth_link_oam_client_record_info_data(port_no,
                                                                       PORT_CLIENT_LOCAL_CONF(port_no),
                                                                       port_remote_tlv,
                                                                       FALSE, FALSE);
                        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                            break;
                        }
                    }

                } else { /* end of logic to monitor the RLB request */
                    if (port_conf->remote_stable == TRUE) {
                        port_conf->remote_stable = FALSE;

                        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                                "Peer's local-satisfied turned off",
                                                port_no, 0, 0, 0);
                        rc = vtss_eth_link_oam_control_port_remote_stable_set(
                                 port_no,
                                 FALSE);
                        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                            break;
                        }
                    }
                }

            } else { /* end of logic to handle the periodic remote TLV */
                if (port_conf->local_satisfied == TRUE) {
                    port_conf->local_satisfied = FALSE;
                    if (port_conf->remote_stable == TRUE) {
                        port_conf->remote_stable = FALSE;
                    }
                    vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                            "local-satisfied is turned off",
                                            port_no, 0, 0, 0);

                    rc = vtss_eth_link_oam_control_port_local_satisfied_set(port_no,
                                                                            FALSE);
                    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                        break;
                    }
                }
            }
        } /* end of logic to handle the remote tlv */
        rc = vtss_eth_link_oam_control_port_local_lost_timer_conf_set(port_no);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        memcpy(port_conf->remote_mac_addr, pdu + VTSS_ETH_LINK_OAM_MAC_LEN,
               VTSS_ETH_LINK_OAM_MAC_LEN);

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

/* This function handles the Var request and sends back the response */
u32 vtss_eth_link_oam_client_port_pdu_var_req_handler(const u32 port_no, const u8  *pdu, const u16 flags, const u16 pdu_len)
{
    u32   rc = VTSS_ETH_LINK_OAM_RC_OK;
    u32   var_resp_rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8    var_branch;
    u16   var_leaf;
    u16   next_var = VTSS_ETH_LINK_OAM_PDU_HDR_LEN;
    BOOL  continue_flag = TRUE;
    u8    var_resp_data[VTSS_ETH_LINK_OAM_PDU_MAX_LEN + VTSS_ETH_LINK_OAM_PDU_HDR_LEN];
    u16   var_resp_len = VTSS_ETH_LINK_OAM_PDU_HDR_LEN;

    do {
        if (pdu == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        memset(var_resp_data, '\0', (VTSS_ETH_LINK_OAM_PDU_MAX_LEN + VTSS_ETH_LINK_OAM_PDU_HDR_LEN));
        do {
            var_branch = *(pdu + next_var);
            if (var_branch == VTSS_ETH_LINK_OAM_NULL) {
                continue_flag = FALSE;
                break;
            }
            memcpy(&var_leaf, (pdu + next_var + VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN),
                   VTSS_ETH_LINK_OAM_VAR_LEAF_LEN);
            var_leaf = vtss_eth_link_oam_ntohs(var_leaf);
            var_resp_rc = vtss_eth_link_oam_client_port_build_var_container(port_no,
                                                                            var_resp_data,
                                                                            var_branch,
                                                                            var_leaf,
                                                                            &var_resp_len,
                                                                            VTSS_ETH_LINK_OAM_PDU_MAX_LEN + VTSS_ETH_LINK_OAM_PDU_HDR_LEN);

            if (var_resp_rc != VTSS_ETH_LINK_OAM_RC_OK) {
                var_resp_len = (VTSS_ETH_LINK_OAM_PDU_HDR_LEN +
                                VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN);
                continue_flag = FALSE;
            }
            next_var += VTSS_ETH_LINK_OAM_VAR_DESCRIPTOR_LEN;
        } while (continue_flag); //End of filling the Variable container

        /* Valid response exists, needs to send it out */
        if (var_resp_len > VTSS_ETH_LINK_OAM_PDU_HDR_LEN) {
            if (var_resp_len < VTSS_ETH_LINK_OAM_PDU_MIN_LEN) {
                var_resp_len = VTSS_ETH_LINK_OAM_PDU_MIN_LEN;
            }
            var_resp_rc = vtss_eth_link_oam_control_port_non_info_pdu_xmit(port_no, var_resp_data, var_resp_len, VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_RESP);
        } //Send out the Variable response

    } while (VTSS_ETH_LINK_OAM_NULL); //End of the handling the variable request

    return rc;
}

/* This function handles the Var response and prints the response */
u32 vtss_eth_link_oam_client_port_pdu_var_resp_handler(const u32 port_no, const u8  *pdu, const u16 flags, const u16 pdu_len)
{
    u32   rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8    var_branch;
    u16   var_leaf;
    u8    var_leaf_width;
    u16   next_var = VTSS_ETH_LINK_OAM_PDU_HDR_LEN;
    BOOL  continue_flag = TRUE;
    u8    var_resp_data[VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_MAX_LEN + 1];
    u8    tmp_index;
    i8    buf[VTSS_ETH_LINK_OAM_RESPONSE_BUF] = {0};
    i8    temp[VTSS_ETH_LINK_OAM_RESPONSE_BUF] = {0};

    do {
        if (pdu == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        do {

            var_branch = *(pdu + next_var);
            if (var_branch == VTSS_ETH_LINK_OAM_NULL) {
                continue_flag = FALSE;
                break;
            }
            (void)snprintf (temp, sizeof(temp), "\r\n  Branch:%u \r\n", var_branch);
            (void)strncat(buf, temp, sizeof(buf) - 1);
            memcpy(&var_leaf,
                   (pdu + next_var + VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN),
                   VTSS_ETH_LINK_OAM_VAR_LEAF_LEN);
            var_leaf = vtss_eth_link_oam_ntohs(var_leaf);
            (void)snprintf (temp, sizeof(temp), "  Leaf:%s \r\n", vtss_eth_link_oam_get_str(var_leaf));
            strncat(buf, temp, sizeof(buf) - 1);
            var_leaf_width = *(pdu + next_var +
                               VTSS_ETH_LINK_OAM_VAR_BRANCH_LEN +
                               VTSS_ETH_LINK_OAM_VAR_LEAF_LEN);
            if (var_leaf_width & VTSS_ETH_LINK_OAM_VAR_INDICATOR_VAL) {
                (void)snprintf (temp, sizeof(temp), "Variable Indicator: %d\r\n", (var_leaf_width));
                strncat(buf, temp, sizeof(buf) - 1);
                next_var += VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN;
                continue;
            } else if (var_leaf_width == VTSS_ETH_LINK_OAM_NULL) {
                var_leaf_width = VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_MAX_LEN;
            }
            if ( (next_var +
                  VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + var_leaf_width) > pdu_len) {
                continue_flag = FALSE;
                break;
            }
            memset(var_resp_data, '\0',
                   VTSS_ETH_LINK_OAM_VAR_CONTAINER_WIDTH_MAX_LEN + 1);
            memcpy(var_resp_data,
                   pdu + next_var + VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN,
                   var_leaf_width);
            (void)snprintf (temp, sizeof(temp), "  Data: ");
            strncat(buf, temp, sizeof(buf) - 1);
            for (tmp_index = 0; tmp_index < var_leaf_width; tmp_index++) {
                (void)snprintf (temp, sizeof(temp), "%02x-",
                                var_resp_data[tmp_index]);
                strncat(buf, temp, sizeof(buf) - 1);
            }
            (void)snprintf (temp, sizeof(temp), "\r\n");
            strncat(buf, temp, sizeof(buf) - 1);
            next_var += (VTSS_ETH_LINK_OAM_VAR_CONTAINER_HDR_LEN + var_leaf_width);
            if (next_var >= pdu_len) {
                continue_flag = FALSE;
            }
        } while (continue_flag); // End of parsing the Variable response

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while(0) */

    if (GET_VAR_RESPONSE_CB(port_no)) {
        if (GET_VAR_RESPONSE_BUF(port_no))
            memcpy(GET_VAR_RESPONSE_BUF(port_no), buf,
                   GET_VAR_RESPONSE_BUF_SIZE(port_no));
        GET_VAR_RESPONSE_CB(port_no)(buf);
    }
    return rc;
}

u32 vtss_eth_link_oam_client_port_state_conf_set(const u32 port_no, const vtss_eth_link_oam_mux_state_t mux_state, const vtss_eth_link_oam_parser_state_t parser_state, const BOOL rev_update, const BOOL update_only_mux_state)
{
    u32  mux_ace_id = 0, parser_ace_id = 0;
    u32  rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8   port_state;
    u8   data[VTSS_ETH_LINK_OAM_INFO_DATA_LEN];

    do {

        mux_ace_id = GET_PORT_PARSER_OAM_ACE_ID(port_no);
        parser_ace_id = GET_PORT_PARSER_LB_ACE_ID(port_no);
        port_state = GET_PORT_STATE(port_no);

        rc = vtss_eth_link_oam_control_port_mux_parser_conf_set(port_no,
                                                                mux_state,
                                                                parser_state,
                                                                &mux_ace_id,
                                                                &parser_ace_id, update_only_mux_state);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR,
                                    "Error occured in remote loop back",
                                    port_no, rc, 0, 0);
            break;
        }

        SET_PORT_MUX_STATE(port_state, mux_state);
        SET_PORT_PARSER_STATE(port_state, parser_state);
        SET_PORT_STATE(port_no, port_state);
        SET_PORT_PARSER_OAM_ACE_ID(port_no, mux_ace_id);
        SET_PORT_PARSER_LB_ACE_ID(port_no, parser_ace_id);

        /* Update the OAM control layer's Local TLV with new parser & multiplexer values */
        memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
        if (rev_update == TRUE) {
            if (vtss_eth_link_oam_client_port_revision_update(port_no, rev_update) != VTSS_RC_OK) {
                break;
            }
            memcpy(data, PORT_CLIENT_LOCAL_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
            memcpy(data + VTSS_ETH_LINK_OAM_INFO_TLV_LEN, PORT_CLIENT_REMOTE_CONF(port_no),
                   VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
            rc = vtss_eth_link_oam_control_port_data_set(port_no, data, FALSE, FALSE);
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;

}

u32 vtss_eth_link_oam_client_port_pdu_link_event_handler(const u32 port_no, const u8  *pdu, const u16 flags, const u16 pdu_len)

{
    u32       rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8        error_code;
    BOOL      continue_flag = TRUE;
    BOOL      is_frame_error = FALSE;
    BOOL      is_symbol_error = FALSE;
    BOOL      is_frame_period_error = FALSE;
    BOOL      is_frame_secs_summary_error = FALSE;
    u16       pdu_offset = VTSS_ETH_LINK_OAM_ERROR_TLV_HDR_LEN + 2;
    u16       pdu_seq_num;

    vtss_eth_link_oam_client_conf_t               *port_conf = NULL;
    vtss_eth_link_oam_error_frame_event_tlv_t     *frame_event_tlv;
    vtss_eth_link_oam_client_error_frame_info_t   *error_frame_info;
    vtss_eth_link_oam_error_frame_period_event_tlv_t      *frame_period_event_tlv;
    vtss_eth_link_oam_client_frame_period_errors_info_t   *error_frame_period_info;
    vtss_eth_link_oam_error_symbol_period_event_tlv_t     *symbol_period_event_tlv;
    vtss_eth_link_oam_client_symbol_period_errors_info_t  *error_symbol_period_info;
    vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t *frame_secs_summary_tlv;
    vtss_eth_link_oam_client_error_frame_secs_summary_info_t *frame_secs_summary_info;

    port_conf = PORT_CLIENT_CONF(port_no);

    memcpy(&pdu_seq_num, pdu + VTSS_ETH_LINK_OAM_ERROR_TLV_HDR_LEN, sizeof(u16));
    pdu_seq_num = vtss_eth_link_oam_ntohs(pdu_seq_num);

    if (port_conf->remote_error_sequence_num == pdu_seq_num) {
        /* duplicate event notification. ignore it */
        return rc;
    }

    do {
        do {
            if (pdu == NULL) {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }
            if (pdu_offset >= pdu_len) {
                continue_flag = FALSE;
                break;
            }
            error_code = *(pdu + pdu_offset);
            if (error_code == VTSS_ETH_LINK_OAM_NULL) {
                continue_flag = FALSE;
                break;
            }
            switch (error_code) {
            case VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TLV:
                if (is_symbol_error == TRUE) {

                    vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                            "multiple symbol errors in received frame",
                                            port_no, 0, 0, 0);
                    continue_flag = FALSE;
                    rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                    break;
                }
                error_symbol_period_info =
                    PORT_CLIENT_SYMBOL_PERIOD_ERROR_CONF(port_no);
                symbol_period_event_tlv  =
                    (vtss_eth_link_oam_error_symbol_period_event_tlv_t *)
                    (pdu + pdu_offset);

                if ( (symbol_period_event_tlv == NULL) ||
                     (symbol_period_event_tlv->event_length == 0) ||
                     (symbol_period_event_tlv->event_length >
                      VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TLV_LEN)) {
                    rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                    break;
                }
                memcpy(&error_symbol_period_info->remote_symbol_period_error_tlv,
                       pdu + pdu_offset,
                       sizeof(vtss_eth_link_oam_error_symbol_period_event_tlv_t));
                pdu_offset += symbol_period_event_tlv->event_length;
                is_symbol_error = TRUE;
                break;

            case VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV:

                if (is_frame_error == TRUE) {
                    continue_flag = FALSE;
                    vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                            "multiple frame errors in received frame",
                                            port_no, 0, 0, 0);
                    rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                    break;
                }
                frame_event_tlv = (vtss_eth_link_oam_error_frame_event_tlv_t *)
                                  (pdu + pdu_offset);
                error_frame_info = PORT_CLIENT_ERROR_FRAME_CONF(port_no);

                if ( (frame_event_tlv == NULL) ||
                     (frame_event_tlv->event_length ==
                      VTSS_ETH_LINK_OAM_NULL) ||
                     (frame_event_tlv->event_length >
                      VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV_LEN)) {
                    rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                    break;
                }
                memcpy((u8 *) & (error_frame_info->remote_error_frame_tlv),
                       pdu + pdu_offset,
                       VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV_LEN);
                pdu_offset += (frame_event_tlv->event_length);
                is_frame_error = TRUE;
                break;

            case VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TLV:
                if (is_frame_period_error == TRUE) {

                    vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                            "multiple frame period errors in received frame",
                                            port_no, 0, 0, 0);
                    continue_flag = FALSE;
                    rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                    break;
                }
                error_frame_period_info =
                    PORT_CLIENT_FRAME_PERIOD_ERROR_CONF(port_no);
                frame_period_event_tlv =
                    (vtss_eth_link_oam_error_frame_period_event_tlv_t *)
                    (pdu + pdu_offset);
                memcpy(
                    &error_frame_period_info->remote_frame_period_error_tlv,
                    pdu + pdu_offset,
                    sizeof(vtss_eth_link_oam_error_frame_period_event_tlv_t));
                pdu_offset += frame_period_event_tlv->event_length;
                is_frame_period_error = TRUE;

                break;

            case VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_TLV :

                if (is_frame_secs_summary_error == TRUE) {

                    vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                            "multiple frame seconds errors in received frame",
                                            port_no, 0, 0, 0);
                    continue_flag = FALSE;
                    rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                    break;
                }
                frame_secs_summary_tlv = (vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t *)
                                         (pdu + pdu_offset);
                frame_secs_summary_info = PORT_CLIENT_ERROR_FRAME_SECS_SUMMARY_CONF(port_no);

                if ( (frame_secs_summary_tlv == NULL) ||
                     /*                         (frame_secs_summary_tlv->event_length ==
                                                                   VTSS_ETH_LINK_OAM_NULL) || */
                     (frame_secs_summary_tlv->event_length !=
                      VTSS_ETH_LINK_OAM_ERROR_FRAME_SECONDS_EVENT_TLV_LEN)
                   ) {
                    rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                    break;
                }
                memcpy((u8 *) & (frame_secs_summary_info->remote_error_frame_secs_summary_tlv),
                       pdu + pdu_offset,
                       VTSS_ETH_LINK_OAM_ERROR_FRAME_SECONDS_EVENT_TLV_LEN);
                pdu_offset += (frame_secs_summary_tlv->event_length);
                is_frame_secs_summary_error = TRUE;
                break;
            default :
                continue_flag = FALSE;
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;

            } /* end of switch-case logic to handle the event PDUs */
        } while (VTSS_ETH_LINK_OAM_NULL); /* end of the do-while(0) */
    } while (continue_flag); /* end of the event TLV parser */

    if (rc == VTSS_RC_OK) {
        port_conf->remote_error_sequence_num = pdu_seq_num; /* Update the sequence number */
    }

    return rc;
}

/* This function handles the loopback response/request */
u32 vtss_eth_link_oam_client_port_pdu_lb_handler(const u32 port_no, const u8  *pdu, const u16 flags, const u16 pdu_len)
{
    u32                                rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8                                 data[VTSS_ETH_LINK_OAM_INFO_DATA_LEN];
    BOOL                               state_change = FALSE;
    vtss_eth_link_oam_mux_state_t      port_mux_state;
    vtss_eth_link_oam_parser_state_t   port_parser_state;
    u8                                 port_mac_addr[VTSS_ETH_LINK_OAM_MAC_LEN];

    do {
        if (!IS_PORT_CONF_ACTIVE(port_no, VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT)) {
            break;
        }
        if (pdu == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        port_mux_state = GET_PORT_MUX_STATE(port_no);
        port_parser_state = GET_PORT_PARSER_STATE(port_no);

        if (pdu[VTSS_ETH_LINK_OAM_PDU_HDR_LEN] ==
            VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_ENABLE_REQUEST) {
            if ( (port_mux_state == VTSS_ETH_LINK_OAM_MUX_FWD_STATE) &&
                 (port_parser_state == VTSS_ETH_LINK_OAM_PARSER_FWD_STATE) ) {

                rc = vtss_eth_link_oam_client_port_state_conf_set(port_no,
                                                                  VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE,
                                                                  VTSS_ETH_LINK_OAM_PARSER_LB_STATE, TRUE, FALSE);

                if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                    break;
                }

                state_change = TRUE;

            } else if ((port_mux_state == VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) &&
                       (port_parser_state == VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) ) {

                rc = vtss_eth_link_oam_control_layer_port_mac_conf_get(port_no,
                                                                       port_mac_addr);

                if (memcmp(port_mac_addr, pdu + VTSS_ETH_LINK_OAM_MAC_LEN,
                           VTSS_ETH_LINK_OAM_MAC_LEN) >= 0) {
                    break;
                }

                /* Reset back the MUX & parser states */
                rc = vtss_eth_link_oam_client_port_state_conf_set(port_no,
                                                                  VTSS_ETH_LINK_OAM_MUX_FWD_STATE,
                                                                  VTSS_ETH_LINK_OAM_PARSER_FWD_STATE, TRUE, FALSE);
                /* And then enter into LB state       */
                rc = vtss_eth_link_oam_client_port_state_conf_set(port_no,
                                                                  VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE,
                                                                  VTSS_ETH_LINK_OAM_PARSER_LB_STATE, TRUE, FALSE);
                state_change = TRUE;
                /* Release the semaphore */
                vtss_eth_link_oam_rlb_opr_unlock();

            }

        } else if (pdu[VTSS_ETH_LINK_OAM_PDU_HDR_LEN] ==
                   VTSS_ETH_LINK_OAM_REMOTE_LOOP_BACK_DISABLE_REQUEST) {

            if ( (port_mux_state == VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) &&
                 (port_parser_state == VTSS_ETH_LINK_OAM_PARSER_LB_STATE) ) {

                rc = vtss_eth_link_oam_client_port_state_conf_set(port_no,
                                                                  VTSS_ETH_LINK_OAM_MUX_FWD_STATE,
                                                                  VTSS_ETH_LINK_OAM_PARSER_FWD_STATE, FALSE, FALSE);
                if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                    break;
                }
                state_change = TRUE;
            }
        }

        if (state_change == TRUE) {
            memset(data, '\0', VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
            if (vtss_eth_link_oam_client_port_revision_update(port_no, TRUE) != VTSS_RC_OK) {
                break;
            }
            memcpy(data, PORT_CLIENT_LOCAL_CONF(port_no), VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
            memcpy(data + VTSS_ETH_LINK_OAM_INFO_TLV_LEN,  PORT_CLIENT_REMOTE_CONF(port_no),
                   VTSS_ETH_LINK_OAM_INFO_TLV_LEN);
            rc = vtss_eth_link_oam_control_port_data_set(port_no,
                                                         data, FALSE, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
        }
        rc = vtss_eth_link_oam_control_port_info_pdu_xmit(port_no);

    } while (0);

    return rc;
}

/* Common function to dispatch the rx OAM pdus to
   the pdu handlers based on the OAM code
*/
u32 vtss_eth_link_oam_client_port_pdu_handler(const u32 port_no, const u16 pdu_len, const u8  *pdu)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    u16                                 tmp_flags = VTSS_ETH_LINK_OAM_NULL;
    u16                                 pdu_max_len;
    vtss_eth_link_oam_info_tlv_t        *port_remote_tlv;
    vtss_eth_link_oam_frame_header_t    *oam_header = NULL;


    do {
        if (pdu == NULL) {
            //Invalid TLV
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "Invalid PDU is received",
                                    port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        oam_header = (vtss_eth_link_oam_frame_header_t *)pdu;
        memcpy(&tmp_flags, oam_header->flags, VTSS_ETH_LINK_OAM_FLAGS_LEN);
        tmp_flags = vtss_eth_link_oam_ntohs(tmp_flags);

        if ( (IS_FLAG_CONF_ACTIVE(tmp_flags,
                                  VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE)) &&
             (IS_FLAG_CONF_ACTIVE(tmp_flags,
                                  VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE))) {
            // Peer wants the discovery to terminate
            rc = vtss_eth_link_oam_control_port_oper_init(port_no,
                                                          IS_PORT_CONF_ACTIVE(port_no,
                                                                              VTSS_ETH_LINK_OAM_CONF_MODE));
            break;
        }
        port_remote_tlv = PORT_CLIENT_REMOTE_CONF(port_no);

        rc = vtss_eth_link_oam_client_pdu_max_len(port_no, &pdu_max_len);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        if ( (oam_header->code != VTSS_ETH_LINK_OAM_CODE_TYPE_INFO) &&
             (pdu_max_len < pdu_len) ) {

            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "Received PDU length exceeds the agreed length",
                                    port_no, 0, 0, 0);
            break;
        }

        switch (oam_header->code) {
        case VTSS_ETH_LINK_OAM_CODE_TYPE_INFO:
            rc = vtss_eth_link_oam_client_port_pdu_info_handler(port_no, pdu, tmp_flags, pdu_len);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_EVENT:
            rc = vtss_eth_link_oam_client_port_pdu_link_event_handler(port_no, pdu, tmp_flags, pdu_len);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_REQ:
            if (IS_PDU_CONF_ACTIVE(port_remote_tlv->oam_conf,
                                   VTSS_ETH_LINK_OAM_CONF_MODE)) {
                rc = vtss_eth_link_oam_client_port_pdu_var_req_handler(port_no, pdu, tmp_flags, pdu_len);
            }
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_RESP:
            rc = vtss_eth_link_oam_client_port_pdu_var_resp_handler(port_no, pdu, tmp_flags, pdu_len);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_LB:
            if (IS_PDU_CONF_ACTIVE(port_remote_tlv->oam_conf, VTSS_ETH_LINK_OAM_CONF_MODE)) {
                rc = vtss_eth_link_oam_client_port_pdu_lb_handler(port_no, pdu, tmp_flags, pdu_len);
            }
            break;
        default :
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG,
                                    "Unsupported OAM PDU is received",
                                    port_no, oam_header->code, 0, 0);
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of the do-while */
    return rc;
}


/******************************************************************************/
/* ETH Link OAM Message Handlers                                              */
/******************************************************************************/

/* Handles the Module Specific Events                                         */
/* The functions acts like a OAM client dispatcer                             */
u32 vtss_eth_link_oam_message_handler(vtss_eth_link_oam_message_t *event_message)
{
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_control_t         oam_control_conf;
    vtss_eth_link_oam_mode_t            oam_mode_conf;
    BOOL                                conf_set_flag = FALSE;

    vtss_eth_link_oam_client_link_event_oper_conf_t link_event_oper_conf;
    client_link_error_symbol_period_event_oper_conf_t symbol_period_oper_conf;
    client_link_error_frame_period_event_oper_conf_t frame_period_oper_conf;
    vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf_t
    error_frame_secs_summary_oper_conf;

    do {
        if (event_message == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        switch (event_message->event_code) {
        case VTSS_ETH_LINK_OAM_MGMT_PORT_CONF_INIT_EVENT:
            rc = vtss_eth_link_oam_client_port_conf_init(event_message->event_on_port);
            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_CONTROL_CONF_EVENT:
            memcpy(&oam_control_conf,
                   event_message->event_data,
                   sizeof(vtss_eth_link_oam_control_t));
            rc = vtss_eth_link_oam_client_port_control_conf_set(event_message->event_on_port,
                                                                oam_control_conf);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_control_port_oper_init(event_message->event_on_port,
                                                          IS_PORT_CONF_ACTIVE(event_message->event_on_port,
                                                                              VTSS_ETH_LINK_OAM_CONF_MODE));

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_MODE_CONF_EVENT:
            memcpy(&oam_mode_conf,
                   event_message->event_data,
                   sizeof(vtss_eth_link_oam_mode_t));
            conf_set_flag = (oam_mode_conf ==
                             VTSS_ETH_LINK_OAM_MODE_ACTIVE);
            rc = vtss_eth_link_oam_client_port_admin_conf_set(event_message->event_on_port,
                                                              VTSS_ETH_LINK_OAM_CONF_MODE,
                                                              conf_set_flag);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_MIB_RETRIVAL_CONF_EVENT:
            memcpy(&conf_set_flag,
                   event_message->event_data, sizeof(BOOL));
            rc = vtss_eth_link_oam_control_supported_code_conf_set(event_message->event_on_port,
                                                                   VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_REQ,
                                                                   conf_set_flag);
            if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
                 (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED)) {
                break;
            }
            rc = vtss_eth_link_oam_control_supported_code_conf_set(event_message->event_on_port,
                                                                   VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_RESP,
                                                                   conf_set_flag);
            if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
                 (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED)) {
                break;
            }
            rc = vtss_eth_link_oam_client_port_admin_conf_set(event_message->event_on_port,
                                                              VTSS_ETH_LINK_OAM_CONF_VARIABLE_RETRIVEL_SUPPORT,
                                                              conf_set_flag);
            break;

        case VTSS_ETH_LINK_OAM_MGMT_PORT_REMOTE_LOOPBACK_CONF_EVENT:
            memcpy(&conf_set_flag,
                   event_message->event_data, sizeof(BOOL));
            rc = vtss_eth_link_oam_control_supported_code_conf_set(event_message->event_on_port,
                                                                   VTSS_ETH_LINK_OAM_CODE_TYPE_LB,
                                                                   conf_set_flag);
            if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
                 (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED)) {
                break;
            }
            rc = vtss_eth_link_oam_client_port_admin_conf_set(event_message->event_on_port,
                                                              VTSS_ETH_LINK_OAM_CONF_REMOTE_LOOP_BACK_CONTROL_SUPPORT,
                                                              conf_set_flag);
            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_LINK_MONITORING_CONF_EVENT:
            memcpy(&conf_set_flag,
                   event_message->event_data,
                   sizeof(BOOL));
            rc = vtss_eth_link_oam_control_supported_code_conf_set(event_message->event_on_port,
                                                                   VTSS_ETH_LINK_OAM_CODE_TYPE_EVENT,
                                                                   conf_set_flag);
            if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
                 (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED)) {
                break;
            }
            rc = vtss_eth_link_oam_client_port_admin_conf_set(event_message->event_on_port,
                                                              VTSS_ETH_LINK_OAM_CONF_LINK_EVENTS_SUPPORT,
                                                              conf_set_flag);
            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_REMOTE_LOOPBACK_OPER_EVENT:
            memcpy(&conf_set_flag,
                   event_message->event_data,
                   sizeof(BOOL));
            rc = vtss_eth_link_oam_client_port_remote_loop_back_oper_set(event_message->event_on_port,
                                                                         conf_set_flag);
            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_WINDOW_CONF_EVENT:
            memset(&link_event_oper_conf, '\0',
                   VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV_LEN);
            memcpy(&(link_event_oper_conf.mgmt_event_window),
                   event_message->event_data,
                   sizeof(link_event_oper_conf.mgmt_event_window));
            rc = vtss_eth_link_oam_client_error_frame_oper_conf_update(
                     event_message->event_on_port,
                     &link_event_oper_conf,
                     VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_THRESHOLD_CONF_EVENT:
            memset(&link_event_oper_conf, '\0',
                   VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_TLV_LEN);
            memcpy(&(link_event_oper_conf.oper_error_frame_threshold),
                   event_message->event_data,
                   sizeof(link_event_oper_conf.oper_error_frame_threshold));
            rc = vtss_eth_link_oam_client_error_frame_oper_conf_update(event_message->event_on_port,
                                                                       &link_event_oper_conf,
                                                                       VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_WINDOW_CONF_EVENT:
            /* memset(&symbol_period_oper_conf,'\0',
                   VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TLV_LEN);*/
            memcpy(&(symbol_period_oper_conf.mgmt_event_window),
                   event_message->event_data,
                   sizeof(symbol_period_oper_conf.mgmt_event_window));
            rc = vtss_eth_link_oam_client_symbol_period_error_oper_conf_update(event_message->event_on_port,
                                                                               &symbol_period_oper_conf,
                                                                               VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET );

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_THRESHOLD_CONF_EVENT:
            /*memset(&symbol_period_oper_conf,'\0',
                   VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TLV_LEN);*/
            memcpy(&symbol_period_oper_conf.oper_error_frame_threshold,
                   event_message->event_data,
                   sizeof(symbol_period_oper_conf.oper_error_frame_threshold));
            rc = vtss_eth_link_oam_client_symbol_period_error_oper_conf_update(event_message->event_on_port,
                                                                               &symbol_period_oper_conf,
                                                                               VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_RXTHRESHOLD_CONF_EVENT:
            /*memset(&symbol_period_oper_conf,'\0',
                   VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_TLV_LEN);*/
            memcpy(&(symbol_period_oper_conf.oper_error_frame_rxthreshold),
                   event_message->event_data,
                   sizeof(symbol_period_oper_conf.oper_error_frame_rxthreshold));

            rc = vtss_eth_link_oam_client_symbol_period_error_oper_conf_update(event_message->event_on_port,
                                                                               &symbol_period_oper_conf,
                                                                               VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_RXTHRESHOLD_SET);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_WINDOW_CONF_EVENT:
            memset(&frame_period_oper_conf, '\0',
                   VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TLV_LEN);
            memcpy(&(frame_period_oper_conf.mgmt_event_window),
                   event_message->event_data,
                   sizeof(frame_period_oper_conf.mgmt_event_window));
            rc = vtss_eth_link_oam_client_frame_period_error_oper_conf_update(event_message->event_on_port,
                                                                              &frame_period_oper_conf,
                                                                              VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_THRESHOLD_CONF_EVENT:
            memset(&frame_period_oper_conf, '\0',
                   VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TLV_LEN);
            memcpy(&(frame_period_oper_conf.oper_error_frame_threshold),
                   event_message->event_data,
                   sizeof(frame_period_oper_conf.oper_error_frame_threshold));
            rc = vtss_eth_link_oam_client_frame_period_error_oper_conf_update(event_message->event_on_port,
                                                                              &frame_period_oper_conf,
                                                                              VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_RXTHRESHOLD_CONF_EVENT:
            memset(&frame_period_oper_conf, '\0',
                   VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_TLV_LEN);
            memcpy(&(frame_period_oper_conf.oper_error_frame_rxthreshold),
                   event_message->event_data,
                   sizeof(frame_period_oper_conf.oper_error_frame_rxthreshold));

            rc = vtss_eth_link_oam_client_frame_period_error_oper_conf_update(event_message->event_on_port,
                                                                              &frame_period_oper_conf,
                                                                              VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_RXTHRESHOLD_SET);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_WINDOW_CONF_EVENT:
            /*memset(&error_frame_secs_summary_oper_conf,'\0',
                  VTSS_ETH_LINK_OAM_ERROR_FRAME_SECONDS_EVENT_TLV_LEN); */
            memcpy(&(error_frame_secs_summary_oper_conf.mgmt_event_window),
                   event_message->event_data,
                   sizeof(error_frame_secs_summary_oper_conf.mgmt_event_window));
            rc = vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(
                     event_message->event_on_port,
                     &error_frame_secs_summary_oper_conf,
                     VTSS_ETH_LINK_OAM_CLIENT_LINK_MGMT_CONF_EVENT_WINDOW_SET);

            break;
        case VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_THRESHOLD_CONF_EVENT:
            /*memset(&error_frame_secs_summary_oper_conf,'\0',
               VTSS_ETH_LINK_OAM_ERROR_FRAME_SECONDS_EVENT_TLV_LEN);*/
            memcpy(&(error_frame_secs_summary_oper_conf.oper_secs_summary_threshold),
                   event_message->event_data,
                   sizeof(error_frame_secs_summary_oper_conf.oper_secs_summary_threshold));
            rc = vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(
                     event_message->event_on_port,
                     &error_frame_secs_summary_oper_conf,
                     VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_THRESHOLD_SET );

            break;

        case VTSS_ETH_LINK_OAM_PORT_UP_EVENT:
            rc = vtss_eth_link_oam_client_port_discovery_protocol_init(
                     event_message->event_on_port, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_control_port_oper_init(
                     event_message->event_on_port,
                     IS_PORT_CONF_ACTIVE(event_message->event_on_port,
                                         VTSS_ETH_LINK_OAM_CONF_MODE));
            break;
        case VTSS_ETH_LINK_OAM_PORT_DOWN_EVENT:
            rc = vtss_eth_link_oam_client_port_discovery_protocol_init(
                     event_message->event_on_port, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_control_port_oper_init(
                     event_message->event_on_port,
                     IS_PORT_CONF_ACTIVE(event_message->event_on_port,
                                         VTSS_ETH_LINK_OAM_CONF_MODE));

            break;
        case VTSS_ETH_LINK_OAM_PORT_FAULT_EVENT:
            rc = vtss_eth_link_oam_client_port_discovery_protocol_init(
                     event_message->event_on_port, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_control_port_oper_init(
                     event_message->event_on_port,
                     IS_PORT_CONF_ACTIVE(event_message->event_on_port,
                                         VTSS_ETH_LINK_OAM_CONF_MODE));
            break;

        case VTSS_ETH_LINK_OAM_PDU_RX_EVENT:
            rc = vtss_eth_link_oam_client_port_pdu_handler(
                     event_message->event_on_port,
                     event_message->event_data_len,
                     event_message->event_data);
            break;

        case VTSS_ETH_LINK_OAM_PDU_LOCAL_LOST_LINK_TIMER_EVENT:
            rc = vtss_eth_link_oam_client_port_discovery_protocol_init(
                     event_message->event_on_port, FALSE);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                break;
            }
            rc = vtss_eth_link_oam_control_port_oper_init(
                     event_message->event_on_port,
                     IS_PORT_CONF_ACTIVE(event_message->event_on_port,
                                         VTSS_ETH_LINK_OAM_CONF_MODE));

            break;

        default:
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while(0) */

    return rc;
}

void vtss_eth_link_oam_client_register_cb(const u32 port_no, void (*cb)(i8 *response), i8 *buf, u32 size)
{
    SET_VAR_RESPONSE_CB(port_no, cb);
    SET_VAR_RESPONSE_BUF(port_no, buf);
    SET_VAR_RESPONSE_BUF_SIZE(port_no, size);

    return;
}

void vtss_eth_link_oam_client_deregister_cb(const u32 port_no)
{
    SET_VAR_RESPONSE_CB(port_no, NULL);
    memset(GET_VAR_RESPONSE_BUF(port_no), 0, sizeof(u32));
    SET_VAR_RESPONSE_BUF_SIZE(port_no, 0);
}

/*
   Following Table was followed as mentioned in the RFC 4878.
   value         |   LclPrsr   LclMux    RmtPrsr   RmtMux
   -----------------------------------------------------
   noLoopback    |     FWD       FWD       FWD       FWD
   initLoopback  |   DISCARD   DISCARD     FWD       FWD
   rmtLoopback   |   DISCARD     FWD      LPBK    DISCARD
   tmtngLoopback |   DISCARD   DISCARD    LPBK    DISCARD
   lclLoopback   |     LPBK    DISCARD   DISCARD     FWD
   unknown       |    ***   any other combination   ***
   -----------------------------------------------------*/
u32 vtss_eth_link_oam_client_loopback_oper_status_get(const u32 port_no, vtss_eth_link_oam_loopback_status_t *status)
{
    vtss_eth_link_oam_info_tlv_t local_info;
    vtss_eth_link_oam_info_tlv_t remote_info;
    u32 local_mux;
    u32 remote_mux;
    u32 local_parser;
    u32 remote_parser;

    memset (&local_info, 0, sizeof (local_info));
    memset (&remote_info, 0, sizeof (remote_info));

    memcpy (&local_info, PORT_CLIENT_LOCAL_CONF (port_no), sizeof (local_info));
    memcpy (&remote_info, PORT_CLIENT_REMOTE_CONF (port_no),
            sizeof (local_info));

    local_mux = (local_info.state & 4) >> 2;
    remote_mux = (remote_info.state & 4) >> 2;
    local_parser = (local_info.state & 3);
    remote_parser = (remote_info.state & 3);

    if (local_parser == VTSS_ETH_LINK_OAM_PARSER_FWD_STATE
        && local_mux == VTSS_ETH_LINK_OAM_MUX_FWD_STATE
        && remote_parser == VTSS_ETH_LINK_OAM_PARSER_FWD_STATE
        && remote_mux == VTSS_ETH_LINK_OAM_MUX_FWD_STATE) {
        *status = VTSS_ETH_LINK_OAM_NO_LOOPBACK;
        return VTSS_ETH_LINK_OAM_RC_OK;
    }
    if (local_parser == VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE
        && local_mux == VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE
        && remote_parser == VTSS_ETH_LINK_OAM_PARSER_FWD_STATE
        && remote_mux == VTSS_ETH_LINK_OAM_MUX_FWD_STATE) {
        *status = VTSS_ETH_LINK_OAM_INITIATING_LOOPBACK;
        return VTSS_ETH_LINK_OAM_RC_OK;
    }
    if (local_parser == VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE
        && local_mux == VTSS_ETH_LINK_OAM_MUX_FWD_STATE
        && remote_parser == VTSS_ETH_LINK_OAM_PARSER_LB_STATE
        && remote_mux == VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) {
        *status = VTSS_ETH_LINK_OAM_REMOTE_LOOPBACK;
        return VTSS_ETH_LINK_OAM_RC_OK;
    }
    if (local_parser == VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE
        && local_mux == VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE
        && remote_parser == VTSS_ETH_LINK_OAM_PARSER_LB_STATE
        && remote_mux == VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE) {
        *status = VTSS_ETH_LINK_OAM_TERMINATING_LOOPBACK;
        return VTSS_ETH_LINK_OAM_RC_OK;
    }
    if (local_parser == VTSS_ETH_LINK_OAM_PARSER_LB_STATE
        && local_mux == VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE
        && remote_parser == VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE
        && remote_mux == VTSS_ETH_LINK_OAM_MUX_FWD_STATE) {
        *status = VTSS_ETH_LINK_OAM_LOCAL_LOOPBACK;
        return VTSS_ETH_LINK_OAM_RC_OK;
    }
    *status = VTSS_ETH_LINK_OAM_UNKNOWN;
    return VTSS_ETH_LINK_OAM_RC_OK;
}
