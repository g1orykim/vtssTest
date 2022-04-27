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
#include "vtss_eth_link_oam_api.h"
#include "vtss_eth_link_oam_control_api.h"

/*lint -sem( vtss_eth_link_oam_crit_oper_data_lock, thread_lock ) */
/*lint -sem( vtss_eth_link_oam_crit_oper_data_unlock, thread_unlock ) */
/*lint -sem( vtss_eth_link_oam_crit_data_lock, thread_lock ) */
/*lint -sem( vtss_eth_link_oam_crit_data_unlock, thread_unlock ) */
/*lint -sem( vtss_eth_link_oam_message_post,custodial(1)) */

/****************************************************************************/
/*  ETH Link OAM global variables                                           */
/****************************************************************************/

// OAM Admin configuration
static vtss_eth_link_oam_mgmt_conf_t link_oam_conf;

/* Set Link OAM Port defaults */
u32 vtss_eth_link_oam_default_set()
{
    u32           port_index = 0;

    memset(link_oam_conf.port_conf, 0, sizeof(link_oam_conf.port_conf));

    for (port_index = 0; port_index < VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT; port_index++) {
        SET_MGMT_PORT_CONTROL(port_index, VTSS_ETH_LINK_OAM_DEFAULT_PORT_CONTROL);
        SET_MGMT_PORT_MODE(port_index, VTSS_ETH_LINK_OAM_DEFAULT_PORT_MODE);
    }
    return VTSS_ETH_LINK_OAM_RC_OK;
}

/* ready:     Enable/Disable the readiness of the Module                      */
/* Set Module Ready flag                                                      */
u32 vtss_eth_link_oam_ready_conf_set(BOOL ready)
{
    u32       rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (link_oam_conf.ready == ready) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        }
        link_oam_conf.ready = ready;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();
    return rc;
}

/************************************************************************************/
/* ETH Link OAM mgmt API definitions                                                */
/************************************************************************************/

/* ready:     Get the readiness of the Module                                       */
/* Set Module Ready flag                                                            */
u32 vtss_eth_link_oam_ready_conf_get(BOOL *ready)
{
    u32       rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (ready == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *ready = link_oam_conf.ready;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc ;
}

/* Initializes the OAM Control on the port                                    */
u32 vtss_eth_link_oam_mgmt_port_conf_init(const u32 port_no)
{
    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_CONF_INIT_EVENT;
            vtss_eth_link_oam_message_post(message);
        } else {
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Enables/Disables the OAM Control on the port                               */
u32 vtss_eth_link_oam_mgmt_port_control_conf_set (u32 port_no, const vtss_eth_link_oam_control_t conf)
{
    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if ( (conf != VTSS_ETH_LINK_OAM_CONTROL_DISABLE) &&
             (conf != VTSS_ETH_LINK_OAM_CONTROL_ENABLE)
           ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (GET_MGMT_PORT_CONTROL(port_no) == conf) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        } else {
            SET_MGMT_PORT_CONTROL(port_no, conf);
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_CONTROL_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(vtss_eth_link_oam_control_t));
            vtss_eth_link_oam_message_post(message);
        } else {
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Simulates the Link OAM fault of the port                               */
u32 vtss_eth_link_oam_mgmt_port_fault_conf_set(const u32 port_no, const BOOL port_fault_enable)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {

        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_code = VTSS_ETH_LINK_OAM_PORT_FAULT_EVENT;
            message->event_on_port = port_no;
            vtss_eth_link_oam_message_post(message);
            break;
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrieve the OAM Control on the port                                       */
u32 vtss_eth_link_oam_mgmt_port_control_conf_get (const u32 port_no, vtss_eth_link_oam_control_t *conf)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_CONTROL(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the Port's OAM mode                                             */
u32 vtss_eth_link_oam_mgmt_port_mode_conf_set (const u32 port_no, const vtss_eth_link_oam_mode_t conf, const BOOL init_flag)
{
    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;
    vtss_eth_link_oam_info_tlv_t      local_info;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if ( (conf != VTSS_ETH_LINK_OAM_MODE_PASSIVE) && (conf != VTSS_ETH_LINK_OAM_MODE_ACTIVE)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        rc = vtss_eth_link_oam_client_port_local_info_get(port_no, &local_info);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        if (local_info.state && init_flag == FALSE) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_STATE;
            break;
        }
        if (GET_MGMT_PORT_MODE(port_no) == conf) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        } else {
            SET_MGMT_PORT_MODE(port_no, conf);
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL);

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_MODE_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(vtss_eth_link_oam_mode_t));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrieves the Link OAM port mode                                           */
u32 vtss_eth_link_oam_mgmt_port_mode_conf_get (const u32 port_no, vtss_eth_link_oam_mode_t *conf)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_MODE(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL);

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the Port MIB retrival support                                   */
u32 vtss_eth_link_oam_mgmt_port_mib_retrival_conf_set (const u32  port_no, const BOOL conf)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;
    static BOOL                       oper = FALSE;

    vtss_eth_link_oam_crit_data_lock();

    do {

        if (oper == TRUE) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_STATE;
            break;
        }
        oper = TRUE;

        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if ((conf != TRUE) && (conf != FALSE)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if (GET_MGMT_PORT_MIB_RETRIVAL_SUPPORT(port_no) == conf) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        } else {
            SET_MGMT_PORT_MIB_RETRIVAL_SUPPORT(port_no, conf);
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL);

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_MIB_RETRIVAL_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(BOOL));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }
    if (rc != VTSS_ETH_LINK_OAM_RC_INVALID_STATE) {
        oper = FALSE;
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the Port MIB retrival support configuration                       */
u32 vtss_eth_link_oam_mgmt_port_mib_retrival_conf_get (const u32  port_no, BOOL *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_MIB_RETRIVAL_SUPPORT(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL);

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the Remote loopback support                                     */
u32 vtss_eth_link_oam_mgmt_port_remote_loopback_conf_set (const u32  port_no, const BOOL conf, const BOOL init_flag)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;
    vtss_eth_link_oam_info_tlv_t      local_info;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if ((conf != TRUE) && (conf != FALSE)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        rc = vtss_eth_link_oam_client_port_local_info_get(port_no, &local_info);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }

        if (local_info.state && init_flag == FALSE) {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            break;
        }

        if (GET_MGMT_PORT_REMOTE_LOOPBACK_SUPPORT(port_no) == conf) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        } else {
            SET_MGMT_PORT_REMOTE_LOOPBACK_SUPPORT(port_no, conf);
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_REMOTE_LOOPBACK_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(BOOL));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* port_no:    l2 port number                                                 */
/* conf:       Port's Remote loopback support configuration                   */
/* Retrives the Port remote loopback support configuration                    */
u32 vtss_eth_link_oam_mgmt_port_remote_loopback_conf_get (const u32  port_no, BOOL *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_REMOTE_LOOPBACK_SUPPORT(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}


/* Initiates the remote loop back operation                                   */
u32 vtss_eth_link_oam_mgmt_port_remote_loopback_oper_conf_set (const u32  port_no, const BOOL conf)
{

    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t         *message = NULL;
    vtss_eth_link_oam_mode_t            mode_conf;
    BOOL                                rlb_support;
    vtss_eth_link_oam_info_tlv_t        local_info;
    vtss_eth_link_oam_discovery_state_t state;
    static BOOL                         oper = FALSE;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (oper == TRUE) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_STATE;
            break;
        }
        oper = TRUE;

        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        mode_conf = GET_MGMT_PORT_MODE(port_no);

        if (mode_conf == VTSS_ETH_LINK_OAM_MODE_PASSIVE) {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            break;
        }

        rlb_support = GET_MGMT_PORT_REMOTE_LOOPBACK_SUPPORT(port_no);

        if (rlb_support == FALSE) {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
            break;
        }

        if ( (conf != TRUE) &&
             (conf != FALSE)
           ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        rc = vtss_eth_link_oam_control_layer_port_discovery_state_get(port_no, &state);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }

        if ((state != VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY)) {
            oper = FALSE; /* Need to place it correctly */
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_STATE;
            break;
        }

        rc = vtss_eth_link_oam_client_port_local_info_get(port_no, &local_info);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        if ( (conf == TRUE) && local_info.state) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        }
        if ( (conf == FALSE) && (local_info.state == 0)) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL);

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_REMOTE_LOOPBACK_OPER_EVENT;
            message->event_on_port = port_no;
            memcpy(message->event_data, &conf, sizeof(BOOL));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    if ( rc == VTSS_ETH_LINK_OAM_RC_OK) {
        if (vtss_eth_link_oam_rlb_opr_lock() == FALSE) {
            if (conf == TRUE) {
                rc = vtss_eth_link_oam_client_port_state_conf_set(port_no, VTSS_ETH_LINK_OAM_MUX_FWD_STATE, VTSS_ETH_LINK_OAM_PARSER_FWD_STATE, TRUE, FALSE);
            }
            rc = VTSS_ETH_LINK_OAM_RC_TIMED_OUT; /* Carry the time out */
        }
    }

    if (rc != VTSS_ETH_LINK_OAM_RC_INVALID_STATE) {
        oper = FALSE;
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the link monitoring support                                     */
u32 vtss_eth_link_oam_mgmt_port_link_monitoring_conf_set (const u32  port_no, const BOOL conf)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if ( (conf != TRUE) &&
             (conf != FALSE)
           ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        /* if (GET_MGMT_PORT_LINK_MONITORING_SUPPORT(port_no) == conf) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        } else*/ {
            SET_MGMT_PORT_LINK_MONITORING_SUPPORT(port_no, conf);
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL);

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code =
                VTSS_ETH_LINK_OAM_MGMT_PORT_LINK_MONITORING_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(BOOL));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the Port link monitoring support configuration                    */
u32 vtss_eth_link_oam_mgmt_port_link_monitoring_conf_get (const u32  port_no, BOOL *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_LINK_MONITORING_SUPPORT(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the port's error frame event's window threshold                   */
u32 vtss_eth_link_oam_mgmt_port_link_error_frame_window_set (const u32  port_no, const u16 conf)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if ( (conf < VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MIN_ERROR_WINDOW) || (conf > VTSS_ETH_LINK_OAM_ERROR_FRAME_EVENT_MAX_ERROR_WINDOW) ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_ERROR_FRAME_EVENT_FRAME_WINDOW(port_no, conf);
    } while (VTSS_ETH_LINK_OAM_NULL);

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_WINDOW_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(u16));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }
    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the port's error frame event's window threshold                   */
u32 vtss_eth_link_oam_mgmt_port_link_error_frame_window_get (const u32  port_no, u16 *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_ERROR_FRAME_EVENT_FRAME_WINDOW(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the port's error frame event's erro  threshold                   */
u32 vtss_eth_link_oam_mgmt_port_link_error_frame_threshold_set (const u32  port_no, const u32 conf)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_ERROR_FRAME_EVENT_FRAME_THRESHOLD(port_no, conf);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_THRESHOLD_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(u32));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the Port Error Frame Event configuration                          */
u32 vtss_eth_link_oam_mgmt_port_link_error_frame_threshold_get (const u32  port_no, u32 *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_ERROR_FRAME_EVENT_FRAME_THRESHOLD(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* configures the symbol period's  window threshold                         */
u32 vtss_eth_link_oam_mgmt_port_link_symbol_period_error_window_set (const u32  port_no, const u64  conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if ( (conf < VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MIN_ERROR_WINDOW) || (conf > VTSS_ETH_LINK_OAM_ERROR_SYMBOL_PERIOD_EVENT_MAX_ERROR_WINDOW)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_SYMBOL_PERIOD_ERROR_EVENT_FRAME_WINDOW(port_no, conf);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_WINDOW_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(u64));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the symbol period event window threshold                      */
u32 vtss_eth_link_oam_mgmt_port_link_symbol_period_error_window_get (const u32 port_no, u64 *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_SYMBOL_PERIOD_ERROR_EVENT_FRAME_WINDOW(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* configures the port's symbol period event error threshold                   */
u32 vtss_eth_link_oam_mgmt_port_link_symbol_period_error_threshold_set (const u32  port_no, const u64  conf)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_SYMBOL_PERIOD_ERROR_EVENT_FRAME_THRESHOLD(port_no, conf);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_THRESHOLD_CONF_EVENT;
            message->event_on_port = port_no;
            memcpy(message->event_data, &conf, sizeof(u64));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the Port symbole period event error threshold configuration    */
u32 vtss_eth_link_oam_mgmt_port_link_symbol_period_error_threshold_get (const u32  port_no, u64 *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_SYMBOL_PERIOD_ERROR_EVENT_FRAME_THRESHOLD(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the port's symbol period event rx symbol count threshold            */
u32 vtss_eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_set (const u32  port_no, const u64 conf)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_SYMBOL_PERIOD_ERROR_EVENT_RXPACKETS_THRESHOLD(port_no, conf);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_SYMBOL_PERIOD_FRAME_RXTHRESHOLD_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(u64));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the Port's symbole period event rx symbols count threshold        */
u32 vtss_eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_get (const u32  port_no, u64 *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_SYMBOL_PERIOD_ERROR_EVENT_RXPACKETS_THRESHOLD(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the port's frame period window threshold                    */
u32 vtss_eth_link_oam_mgmt_port_link_frame_period_error_window_set (const u32  port_no, const u32  conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_FRAME_PERIOD_ERROR_EVENT_FRAME_WINDOW(port_no, conf);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_WINDOW_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(u32));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the port's frame period window threshold                    */
u32 vtss_eth_link_oam_mgmt_port_link_frame_period_error_window_get (const u32  port_no, u32 *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_FRAME_PERIOD_ERROR_EVENT_FRAME_WINDOW(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the port frame period error threshold                        */
u32 vtss_eth_link_oam_mgmt_port_link_frame_period_error_threshold_set (const u32  port_no, const u32  conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_FRAME_PERIOD_ERROR_EVENT_FRAME_THRESHOLD(port_no, conf);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL);

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_THRESHOLD_CONF_EVENT;
            message->event_on_port = port_no;
            memcpy(message->event_data, &conf, sizeof(u32));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the port's frame period error threshold configuration         */
u32 vtss_eth_link_oam_mgmt_port_link_frame_period_error_threshold_get (const u32 port_no, u32 *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_FRAME_PERIOD_ERROR_EVENT_FRAME_THRESHOLD(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Configures the port's frame period error event rx  threshold                 */
u32 vtss_eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_set (const u32  port_no, const u64 conf)
{

    u32                               rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_FRAME_PERIOD_ERROR_EVENT_RXPACKETS_THRESHOLD(port_no, conf);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_FRAME_PERIOD_FRAME_RXTHRESHOLD_CONF_EVENT;
            message->event_on_port = port_no;
            memcpy(message->event_data, &conf, sizeof(u64));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* Retrives the port's frame period error event rx  threshold                 */
u32 vtss_eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_get (const u32  port_no, u64 *conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_FRAME_PERIOD_ERROR_EVENT_RXPACKETS_THRESHOLD(port_no);
        break;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/******************************************************************************/
/* ETH Link OAM Control status specific management functions                  */
/******************************************************************************/

/* port_no:     port number                                                   */
/* pdu_control: pdu control                                                   */
/* Retrives the PDU control status of the discovery protocol                  */
u32 vtss_eth_link_oam_mgmt_control_port_pdu_control_status_get(const u32 port_no, vtss_eth_link_oam_pdu_control_t *pdu_control)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc =  vtss_eth_link_oam_control_port_pdu_control_status_get(port_no, pdu_control);

    return rc;

}

/* port_no:    port number                                                    */
/* state:      discovery state                                                */
/* Retrives the port's discovery state                                        */
u32 vtss_eth_link_oam_mgmt_control_port_discovery_state_get(const u32 port_no, vtss_eth_link_oam_discovery_state_t *state)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_control_port_discovery_state_get(port_no, state);

    return rc;

}

/* port_no:         port number                                               */
/* oam_client_pdu:  client pdu to be transmitted                              */
/* oam_pdu_len:     OAM PDU length to be transmitted                          */
/* oam_pdu_code:    OAM PDU code to be transmitted                            */
/* Builds and transmits the non-info OAM pdu to be transmitted                */
u32 vtss_eth_link_oam_mgmt_control_port_non_info_pdu_xmit(const u32 port_no, u8 *oam_client_pdu, const u16 oam_pdu_len, const u8 oam_pdu_code)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_control_port_non_info_pdu_xmit(port_no, oam_client_pdu, oam_pdu_len, oam_pdu_code);

    return rc;

}


/* port_no:    port number                                                    */
/* pdu_stats:  port's pdu statistics                                          */
/* Retrives the port's standard PDU statistics                                */
u32 vtss_eth_link_oam_mgmt_control_port_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_pdu_stats_t *pdu_stats)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_control_port_pdu_stats_get(port_no, pdu_stats);

    return rc;
}

/* port_no:    port number                                                    */
/* pdu_stats:  critical event pdu statistics                                  */
/* Retrives the port's critical event statistics                              */
u32 vtss_eth_link_oam_mgmt_control_port_critical_event_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_critical_event_pdu_stats_t *pdu_stats)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_control_port_critical_event_pdu_stats_get(port_no, pdu_stats);

    return rc;

}

/* port_no:         port number                                               */
/* flag:            OAM flag                                                  */
/* enable_flag:     enable/disable the specified flag                         */
/* Sets the port's OAM Flags like link_fault,dying_gasp                       */
u32 vtss_eth_link_oam_mgmt_control_port_flags_conf_set(const u32 port_no, const u8 flag, const BOOL enable_flag)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_control_port_flags_conf_set(port_no, flag, enable_flag);

    return rc;
}


/* port_no:         port number                                               */
/* flag:            OAM flag                                                  */
/* Retrives the port's OAM Flags like link_fault,dying_gasp                   */
u32 vtss_eth_link_oam_mgmt_control_port_flags_conf_get(const u32 port_no, u8 *flag)

{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_control_port_flags_conf_get(port_no, flag);

    return rc;
}

/* Resets the port counters                                                   */
u32 vtss_eth_link_oam_mgmt_control_clear_statistics(const u32 port_no)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_control_clear_statistics(port_no);

    return rc;
}

/* Handles the received function                                              */
u32 vtss_eth_link_oam_mgmt_control_rx_pdu_handler(const u32 port_no, const u8 *pdu, const u16 pdu_len, const u8 oam_code)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_control_rx_pdu_handler(port_no, pdu, pdu_len, oam_code);

    return rc;

}



/******************************************************************************/
/* ETH Link OAM Client status specific management functions                   */
/******************************************************************************/

/* port_no:    l2 port number                                                 */
/* conf:       Enable/Disable the OAM Control                                 */
/* Retrievs the port's OAM Control                                            */
u32 vtss_eth_link_oam_mgmt_client_port_control_conf_get (const u32  port_no, u8 *const conf)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();

    rc = vtss_eth_link_oam_client_port_control_conf_get (port_no, conf);

    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;
}

/* port_no:    port number                                                    */
/* info:       local info                                                     */
/* Retrives the Port OAM Local information                                    */
u32 vtss_eth_link_oam_mgmt_client_port_local_info_get (const u32 port_no, vtss_eth_link_oam_info_tlv_t *const local_info)

{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();

    rc = vtss_eth_link_oam_client_port_local_info_get(port_no, local_info);
    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;

}

/* port_no:    port number                                                    */
/* info:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) information                             */
u32 vtss_eth_link_oam_mgmt_client_port_remote_info_get (const u32 port_no, vtss_eth_link_oam_info_tlv_t *const remote_info)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();

    rc = vtss_eth_link_oam_client_port_remote_info_get(port_no, remote_info);

    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;
}

/* port_no:    port number                                                    */
/* info:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) information                             */
u32 vtss_eth_link_oam_mgmt_client_port_remote_seq_num_get (const u32 port_no, u16 *const remote_info)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();

    rc = vtss_eth_link_oam_client_port_remote_seq_num_get(port_no, remote_info);

    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;
}



/* port_no:    port number                                                    */
/* info:       Remote info                                                    */
/* Retrives the Port OAM Peer(remote) MAC address                             */
u32 vtss_eth_link_oam_mgmt_client_port_remote_mac_addr_info_get (const u32 port_no, u8 *const remote_mac_addr)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();
    rc = vtss_eth_link_oam_client_port_remote_mac_addr_info_get(port_no, remote_mac_addr);
    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;

}

/* port_no:                port number                                        */
/* local_error_info:       local error info                                   */
/* local_error_info:       Remote error info                                  */
/* Retrives the Port's frame error  information                               */
u32 vtss_eth_link_oam_mgmt_client_port_frame_error_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_event_tlv_t  *local_error_info, vtss_eth_link_oam_error_frame_event_tlv_t  *remote_error_info)

{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();
    rc = vtss_eth_link_oam_client_port_frame_error_info_get(port_no, local_error_info, remote_error_info);
    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;

}

/* port_no:                port number                                        */
/* local_error_info:       local error info                                   */
/* local_error_info:       Remote error info                                  */
/* Retrives the Port's frame period error  information                        */
u32 vtss_eth_link_oam_mgmt_client_port_frame_period_error_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_period_event_tlv_t *const local_info, vtss_eth_link_oam_error_frame_period_event_tlv_t *const remote_info)

{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();

    rc = vtss_eth_link_oam_client_port_frame_period_error_info_get(port_no, local_info, remote_info);

    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;
}

/* Retrives the Port's frame period error  information                        */
u32 vtss_eth_link_oam_mgmt_client_port_error_frame_secs_summary_info_get (const u32 port_no, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t *local_info, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t *remote_info)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();
    rc = vtss_eth_link_oam_client_port_error_frame_secs_summary_info_get(port_no, local_info, remote_info);
    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;

}

/* port_no:                port number                                        */
/* local_info:             local error info                                   */
/* local_info:             Remote error info                                  */
/* Retrives the Port's frame period error  information                        */
u32 vtss_eth_link_oam_mgmt_client_port_symbol_period_error_info_get (const u32 port_no, vtss_eth_link_oam_error_symbol_period_event_tlv_t *const local_info, vtss_eth_link_oam_error_symbol_period_event_tlv_t *const remote_info)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();
    rc = vtss_eth_link_oam_client_port_symbol_period_error_info_get(port_no, local_info, remote_info);
    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;
}

/* port_no:      port number                                                  */
/* max_pdu_len:  max pdu len                                                  */
/* Specifies the max pdu length that can be used for communication between    */
/* peers                                                                      */
u32 vtss_eth_link_oam_mgmt_client_pdu_max_len(const u32 port_no, u16 *max_pdu_len)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();
    rc = vtss_eth_link_oam_mgmt_client_pdu_max_len(port_no, max_pdu_len);
    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;
}

/* port_no:           port number                                             */
/* cb:                call back function                                      */
/* buf:               buffer                                                  */
/* size:              buffer size                                             */
/* Registers a call back function to get notified on variable response        */
void vtss_eth_link_oam_mgmt_client_register_cb(const u32 port_no, void (*cb)(i8 *response), i8 *buf, u32 size)
{
    vtss_eth_link_oam_crit_data_lock();
    vtss_eth_link_oam_client_register_cb(port_no, cb, buf, size);
    vtss_eth_link_oam_crit_data_unlock();

    return;
}

/* port_no:           port number                                             */
/* Deregisters the call back function to get notified on variable response    */
void vtss_eth_link_oam_mgmt_client_deregister_cb(const u32 port_no)
{
    vtss_eth_link_oam_crit_data_lock();
    vtss_eth_link_oam_client_deregister_cb(port_no);
    vtss_eth_link_oam_crit_data_unlock();
}



/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Window                           */
/* Retrives the Port Error Frame Second Summary Event configuration           */
u32 vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_set (const u32  port_no, const u16 conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf < VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MIN_ERROR_WINDOW || conf > VTSS_ETH_LINK_OAM_ERROR_FRAME_SECS_SUMMARY_EVENT_MAX_ERROR_WINDOW) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_EVENT_FRAME_WINDOW(port_no, conf);

    } while (VTSS_ETH_LINK_OAM_NULL);
    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_WINDOW_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(u16));
            vtss_eth_link_oam_message_post(message);
        } else {
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
        }
    }
    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Window                           */
/* Retrives the Port Error Frame Event configuration                          */
u32 vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_get (const u32  port_no, u16 *conf)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_EVENT_FRAME_WINDOW(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL);

    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;
}

/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Threshold                        */
/* Retrives the Port Error Frame Event configuration                          */
u32 vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_set (const u32  port_no, const u32 conf)
{

    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_message_t       *message = NULL;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf > VTSS_ETH_LINK_OAM_DEFAULT_EVENT_SECS_SUMMARY_THRESHOLD_MAX) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        SET_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_EVENT_FRAME_THRESHOLD(port_no, conf);
    } while (VTSS_ETH_LINK_OAM_NULL);

    if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

        message = vtss_eth_link_oam_malloc(sizeof(vtss_eth_link_oam_message_t));
        if (message != NULL) {
            message->event_on_port = port_no;
            message->event_code = VTSS_ETH_LINK_OAM_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_THRESHOLD_CONF_EVENT;
            memcpy(message->event_data, &conf, sizeof(u32));
            vtss_eth_link_oam_message_post(message);
        } else {
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Unable to allocate memory", port_no, 0, 0, 0);
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
        }
    }

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Window                           */
/* Retrives the Port Error Frame Event configuration                          */
u32 vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_get(const u32  port_no, u16 *conf)
{
    u32                  rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_oper_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (conf == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *conf = GET_MGMT_PORT_ERROR_FRAME_SECS_SUMMARY_EVENT_FRAME_THRESHOLD(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL);

    vtss_eth_link_oam_crit_oper_data_unlock();

    return rc;
}

u32 vtss_eth_link_oam_mgmt_loopback_oper_status_get(const u32 port_no, vtss_eth_link_oam_loopback_status_t *status)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();

    do {
        if (port_no >= VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (status == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        rc = vtss_eth_link_oam_client_loopback_oper_status_get(port_no, status);
    } while (VTSS_ETH_LINK_OAM_NULL);

    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

u32 vtss_eth_link_oam_mgmt_port_state_conf_set(const u32 port_no, const vtss_eth_link_oam_mux_state_t mux_state, const vtss_eth_link_oam_parser_state_t parser_state)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    vtss_eth_link_oam_crit_data_lock();
    rc = vtss_eth_link_oam_client_port_state_conf_set(port_no, mux_state, parser_state, TRUE, FALSE);
    vtss_eth_link_oam_crit_data_unlock();

    return rc;
}

