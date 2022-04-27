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

#include "vtss_eth_link_oam_control_api.h"
#include <string.h>

/******************************************************************************/
/*  Various local functions                                                   */
/******************************************************************************/
static u32 vtss_eth_link_oam_discovery_state_machine (const u32 port_no, const vtss_eth_link_oam_discovery_events_t discoveryevent);

static u32 vtss_eth_link_oam_discovery_fault_state_action(const u32 port_no);
static u32 vtss_eth_link_oam_discovery_active_state_action(const u32 port_no);
static u32 vtss_eth_link_oam_discovery_passive_state_action(const u32 port_no);
static u32 vtss_eth_link_oam_discovery_send_local_remote_state_action(const u32 port_no);
static u32 vtss_eth_link_oam_discovery_send_local_remote_ok_state_action(const u32 port_no);
static u32 vtss_eth_link_oam_discovery_send_any_state_action(const u32 port_no);

/******************************************************************************/
/*  File level Global variables                                               */
/******************************************************************************/
static vtss_eth_link_oam_control_oper_conf_t link_oam_control_conf;

static vtss_eth_link_oam_discovery_state_t
discovery_state[VTSS_ETH_LINK_OAM_DISCOVERY_STATE_LAST]
[VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LAST] =

{
    {S1, S1, S1, S2, S3, S1, S1, S1, S1, S1},
    {S1, S1, S1, S2, S2, S4, S2, S2, S2, S2},
    {S1, S1, S1, S3, S3, S4, S3, S3, S3, S3},
    {S1, S1, S1, S4, S4, S4, S5, S4, S4, S4},
    {S1, S1, S1, S5, S5, S5, S5, S4, S6, S5},
    {S1, S1, S1, S6, S6, S6, S6, S4, S6, S5},
};

static u8 const VTSS_OAM_MULTICAST_MACADDR[] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x02 };

/******************************************************************************/
/* Eth Link OAM Module Control function definitions                           */
/******************************************************************************/
void vtss_eth_link_oam_control_init(void)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;
    u32 port_no;

    memset(&link_oam_control_conf, '\0', sizeof(link_oam_control_conf));

    /* By default Info PDUs are only allowed. Rest all are treated as
       unsupported code. This function is place holder to register the
       information code as supported
    */
    for (port_no = 0; port_no < VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT; port_no++) {
        rc = vtss_eth_link_oam_control_layer_supported_codes_conf_set(port_no, VTSS_ETH_LINK_OAM_CODE_TYPE_INFO, TRUE);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            /* Log the message */
            vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR, "Error occured while registering for codes", 0, 0, VTSS_ETH_LINK_OAM_CODE_TYPE_INFO, 0);
        }
    }

    return;
}

/******************************************************************************/
/* ETH Link OAM Control function definitions                                  */
/******************************************************************************/
u32 vtss_eth_link_oam_control_layer_port_conf_init(const u32 port_no, const u8 *data)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if ((!PORT_CONTROL_LAYER_DATA(port_no)) || (!data)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        memcpy(PORT_CONTROL_LAYER_DATA(port_no), data, VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
    } while (VTSS_ETH_LINK_OAM_NULL); /*end of do-while to init the port */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_oper_init(const u32 port_no, const BOOL is_port_active)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    RESET_PORT_LOCAL_LOST_LINK_TIMER(port_no);

    if (IS_PORT_CONTROL_CONF_ENABLE(port_no)) {

        rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_BEGIN);

        if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
            switch (is_port_active) {
            case TRUE:
                rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_ACTIVE_MODE);
                break;
            case FALSE:
                rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_PASSIVE_MODE);
                break;
            }
        }
    } else {
        rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_BEGIN);
    }
    return rc;
}

u32 vtss_eth_link_oam_control_layer_supported_codes_conf_set(const u32 port_no, const u8 oam_code, const BOOL support_enable)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (oam_code >= VTSS_ETH_LINK_OAM_SUPPORTED_CODES_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (GET_SUPPORTED_CODE_CONF(port_no, oam_code) == support_enable) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        }
        if (support_enable == TRUE) {
            SET_SUPPORTED_CODE_CONF(port_no, oam_code);
        } else {
            RESET_SUPPORTED_CODE_CONF(port_no, oam_code);
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_supported_codes_conf_get(const u32 port_no, const u8 oam_code, BOOL *const support_conf)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!support_conf) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (oam_code == VTSS_ETH_LINK_OAM_CODE_TYPE_ORG) {
            rc = VTSS_ETH_LINK_OAM_RC_OK;
            *support_conf = TRUE;
            break;
        }
        if (oam_code >= VTSS_ETH_LINK_OAM_SUPPORTED_CODES_CNT) {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            break;
        }
        *support_conf = GET_SUPPORTED_CODE_CONF(port_no, oam_code);

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_data_set(const u32 port_no, const u8 *oam_data, const BOOL  reset_port_oper, const BOOL  is_port_active)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if ( (!PORT_CONTROL_LAYER_DATA(port_no)) || (!oam_data) ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        memcpy(PORT_CONTROL_LAYER_DATA(port_no), oam_data, VTSS_ETH_LINK_OAM_INFO_DATA_LEN);

        if (reset_port_oper == TRUE) {
            rc = vtss_eth_link_oam_control_layer_port_oper_init(port_no, is_port_active);
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_control_conf_set(const u32 port_no, const u8 oam_control)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        SET_PORT_CONTROL_CONF(port_no, oam_control);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_control_conf_get(const u32 port_no, u8 *const oam_control)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!oam_control) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *oam_control = GET_PORT_CONTROL_CONF(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_flags_conf_set(const u32 port_no, const u8  flag, const BOOL enable_flag)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (flag & VTSS_ETH_LINK_OAM_INVALID_FLAGS) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (IS_PORT_FLAG_ACTIVE(port_no, flag) == enable_flag) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        }
        if (enable_flag == TRUE) {
            SET_PORT_FLAGS(port_no, flag);
        } else {
            RESET_PORT_FLAGS(port_no, flag);
        }
        /* This mimics the Link fault event */
        if (flag == VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT) {
            SET_PORT_LOCAL_LINK_STATUS(port_no, enable_flag);
            if (enable_flag == TRUE) {
                rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LINK_STATUS_FAIL);
            } else {
                rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_BEGIN);
            }
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_flags_conf_get(const u32 port_no, u8 *const flag)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!flag) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        /* If Port Number is not in range return Error*/
        *flag =  GET_PORT_FLAGS(port_no) ;
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}


u32 vtss_eth_link_oam_control_layer_port_remote_state_valid_set(const u32 port_no)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_REMOTE_STATE_VALID);

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_local_satisfied_set(const u32 port_no, const BOOL is_local_satisfied)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    if (is_local_satisfied == TRUE) {
        rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LOCAL_SATISFIED_TRUE);
    } else {
        rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LOCAL_SATISFIED_FALSE);

    }

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_remote_stable_set(const u32 port_no, const BOOL is_remote_stable)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    if (is_remote_stable == TRUE) {
        rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_REMOTE_STABLE_TRUE);
    } else {
        rc = vtss_eth_link_oam_discovery_state_machine(port_no, VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_REMOTE_STABLE_FALSE);
    }

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_discovery_state_get(const u32 port_no, vtss_eth_link_oam_discovery_state_t *const state)
{
    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        if (!state)  {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *state = GET_PORT_DISCOVERY_STATE(port_conf);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_pdu_control_status_get(const u32 port_no, vtss_eth_link_oam_pdu_control_t *const pdu_control)
{
    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        if (!pdu_control) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *pdu_control = GET_PORT_PDU_TX_CONTROL(port_conf);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_local_lost_timer_conf_set(const u32 port_no)
{
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        SET_PORT_LOCAL_LOST_LINK_TIMER(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_pdu_cnt_conf_set(const u32 port_no)
{
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        SET_PORT_PDU_CNT(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

/******************************************************************************/
/* ETH Link OAM Discovery Protocol state machine function definitions         */
/******************************************************************************/

static u32 vtss_eth_link_oam_discovery_fault_state_action(const u32 port_no)
{
    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        SET_PORT_DISCOVERY_STATE(port_conf, VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT);
        if (GET_PORT_LOCAL_LINK_STATUS(port_no) == FALSE) {
            SET_PORT_PDU_TX_CONTROL(port_conf, VTSS_ETH_LINK_OAM_PDU_CONTROL_RX_INFO);
        } else {
            SET_PORT_PDU_TX_CONTROL(port_conf, VTSS_ETH_LINK_OAM_PDU_CONTROL_LF_INFO);
        }
        RESET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE);
        SET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE);
        RESET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_REMOTE_STABLE);
        RESET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_REMOTE_EVALUTE);

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

static u32 vtss_eth_link_oam_discovery_active_state_action(const u32 port_no)
{
    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        if (GET_PORT_LOCAL_LINK_STATUS(port_no) == TRUE) {
            /* Link fault is on, so no need of state change */
            break;
        }
        SET_PORT_DISCOVERY_STATE(port_conf, VTSS_ETH_LINK_OAM_DISCOVERY_STATE_ACTIVE_SEND_LOCAL);
        SET_PORT_PDU_TX_CONTROL(port_conf, VTSS_ETH_LINK_OAM_PDU_CONTROL_INFO);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

static u32 vtss_eth_link_oam_discovery_passive_state_action(const u32 port_no)
{
    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        if (GET_PORT_LOCAL_LINK_STATUS(port_no) == TRUE) {
            /* Link fault is on, so no need of state change */
            break;
        }
        SET_PORT_DISCOVERY_STATE(port_conf, VTSS_ETH_LINK_OAM_DISCOVERY_STATE_PASSIVE_WAIT);
        SET_PORT_PDU_TX_CONTROL(port_conf, VTSS_ETH_LINK_OAM_PDU_CONTROL_RX_INFO);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

static u32 vtss_eth_link_oam_discovery_send_local_remote_state_action(const u32 port_no)
{
    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        SET_PORT_DISCOVERY_STATE(port_conf, VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE);
        SET_PORT_PDU_TX_CONTROL(port_conf, VTSS_ETH_LINK_OAM_PDU_CONTROL_INFO);
        RESET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE);
        SET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE);
        //RESET_PORT_FLAGS(port_no,VTSS_ETH_LINK_OAM_FLAG_REMOTE_STABLE);
        //RESET_PORT_FLAGS(port_no,VTSS_ETH_LINK_OAM_FLAG_REMOTE_EVALUTE);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

static u32 vtss_eth_link_oam_discovery_send_local_remote_ok_state_action(const u32 port_no)
{
    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        SET_PORT_DISCOVERY_STATE(port_conf, VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE_OK);
        SET_PORT_PDU_TX_CONTROL(port_conf, VTSS_ETH_LINK_OAM_PDU_CONTROL_INFO);
        SET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE);
        RESET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

static u32 vtss_eth_link_oam_discovery_send_any_state_action(const u32 port_no)
{
    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        SET_PORT_DISCOVERY_STATE(port_conf, VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY);
        SET_PORT_PDU_TX_CONTROL(port_conf, VTSS_ETH_LINK_OAM_PDU_CONTROL_ANY);
        SET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE);
        RESET_PORT_FLAGS(port_no, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

static u32 vtss_eth_link_oam_discovery_state_machine (const u32 port_no, const vtss_eth_link_oam_discovery_events_t discovery_event)
{

    vtss_eth_link_oam_control_conf_t         *port_conf;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_discovery_state_t      next_state = VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT;

    do {

        if (discovery_event > VTSS_ETH_LINK_OAM_DISCOVERY_EVENT_LAST) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);

        if (!(IS_PORT_CONTROL_CONF_ENABLE(port_no))) {
            next_state = VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT;
        } else {
            next_state = GET_NEXT_PORT_DISCOVERY_STATE(port_conf, discovery_event);
        }

        vtss_eth_link_oam_trace(VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG, "Discovery SM:", port_no, next_state, discovery_event, 0);

        if (next_state != VTSS_ETH_LINK_OAM_DISCOVERY_STATE_LAST) {
            switch (next_state) {
            case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT:
                rc = vtss_eth_link_oam_discovery_fault_state_action(port_no);
                break;
            case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_ACTIVE_SEND_LOCAL:
                rc = vtss_eth_link_oam_discovery_active_state_action(port_no);
                break;
            case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_PASSIVE_WAIT:
                rc = vtss_eth_link_oam_discovery_passive_state_action(port_no);
                break;
            case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE:
                rc = vtss_eth_link_oam_discovery_send_local_remote_state_action(port_no);
                break;
            case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE_OK:
                rc = vtss_eth_link_oam_discovery_send_local_remote_ok_state_action(port_no);
                break;
            case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY:
                rc = vtss_eth_link_oam_discovery_send_any_state_action(port_no);
                break;
            default:
                break;
            }
        } else {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
        }

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of the do-while loop */

    return rc;
}

/******************************************************************************/
/* ETH Link OAM Transmit State machine function definitions                   */
/******************************************************************************/

/* This function mainly fills the OAM header for any out going OAM PDU.
   This function also increments the TX stats of the port and applies
   conditions to forward the OAM pdu.
*/

u32 vtss_eth_link_oam_control_layer_fill_header(const u32 port_no, u8 *const pdu, const u8 code)
{
    vtss_eth_link_oam_frame_header_t         *oam_header;
    u32                                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    u16                                      eth_type;
    u16                                      tmp_flags = 0;
    vtss_eth_link_oam_discovery_state_t      state;

    do {
        eth_type = vtss_eth_link_oam_htons(VTSS_ETH_LINK_OAM_ETH_TYPE);
        if (!pdu) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        rc = vtss_eth_link_oam_control_layer_port_discovery_state_get(port_no, &state);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        if ( (code != VTSS_ETH_LINK_OAM_CODE_TYPE_INFO) && (state != VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY)
           ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_STATE;
            break;
        }
        if (GET_PORT_PDU_CNT(port_no) == 0) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PDU_CNT;
            break;
        }
        DEC_PORT_PDU_CNT(port_no);
        oam_header = (vtss_eth_link_oam_frame_header_t *)pdu;
        memcpy(oam_header->dst_mac, VTSS_OAM_MULTICAST_MACADDR, VTSS_ETH_LINK_OAM_MAC_LEN);
        rc = vtss_eth_link_oam_control_layer_port_mac_conf_get(port_no, oam_header->src_mac);
        memcpy(oam_header->eth_type, &eth_type, VTSS_ETH_LINK_OAM_ETH_TYPE_LEN);
        oam_header->subtype = VTSS_ETH_LINK_OAM_SUB_TYPE;
        tmp_flags = vtss_eth_link_oam_htons(GET_PORT_FLAGS(port_no));
        memcpy(oam_header->flags, &tmp_flags, (VTSS_ETH_LINK_OAM_FLAGS_LEN));
        oam_header->code = code;

        if (IS_PORT_FLAG_ACTIVE(port_no, VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT)) {
            INC_LINK_FAULT_TX_STATS(port_no);
        }
        if (IS_PORT_FLAG_ACTIVE(port_no, VTSS_ETH_LINK_OAM_FLAG_DYING_GASP)) {
            INC_DYING_GASP_TX_STATS(port_no);
        }
        if (IS_PORT_FLAG_ACTIVE(port_no, VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT)) {
            INC_CRITICAL_EVENT_TX_STATS(port_no);
        }
        switch (code) {
        case VTSS_ETH_LINK_OAM_CODE_TYPE_INFO:
            INC_INFO_TX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_EVENT:
            INC_UNIQUE_EVENT_NOTIFICATION_TX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_REQ:
            INC_VAR_REQ_TX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_RESP:
            INC_VAR_RESP_TX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_LB:
            INC_LB_TX_STATS(port_no);
            break;
        default:
            break;
        } /* End of the switch case */

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of the do-while */

    return rc;
}


u32 vtss_eth_link_oam_control_layer_fill_info_data(const u32 port_no, u8 *const pdu)
{
    u32  rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!pdu) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        rc = vtss_eth_link_oam_control_layer_fill_header(port_no, pdu, VTSS_ETH_LINK_OAM_CODE_TYPE_INFO);

        if ( (GET_PORT_LOCAL_LINK_STATUS(port_no) == FALSE) && (rc == VTSS_ETH_LINK_OAM_RC_OK) ) {
            memcpy(pdu + VTSS_ETH_LINK_OAM_PDU_HDR_LEN, PORT_CONTROL_LAYER_DATA(port_no), VTSS_ETH_LINK_OAM_INFO_DATA_LEN);
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_is_periodic_xmit_needed(const u32 port_no, BOOL *const is_xmit_needed)
{
    u32  rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!(IS_PORT_CONTROL_CONF_ENABLE(port_no))) {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
            break;
        }
        if (!is_xmit_needed) {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
            break;
        }
        if (!IS_PORT_IN_RX_STATE(port_no)) {
            *is_xmit_needed = TRUE;
        } else {
            *is_xmit_needed = FALSE;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_rx_pdu_handler(const u32 port_no, const u8 *pdu, const u16 pdu_len, const u8  oam_code)
{
    u32                                  rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_eth_link_oam_discovery_state_t  state;
    BOOL                                 code_support = FALSE;
    vtss_eth_link_oam_frame_header_t     *oam_header;
    u16                                  tmp_flags = 0;
    BOOL                                 flag_enable = FALSE;
    vtss_eth_link_oam_control_conf_t     *port_conf;

    do {

        if (!pdu) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        rc = vtss_eth_link_oam_control_layer_supported_codes_conf_get(port_no, oam_code, &code_support);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            if (rc == VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED) {
                INC_UNSUPPORTED_CODE_RX_STATS(port_no);
            }
            break;
        } else {
            if (code_support == FALSE) {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
                INC_UNSUPPORTED_CODE_RX_STATS(port_no);
                break;
            }
        }
        port_conf = PORT_CONTROL_LAYER_CONF(port_no);
        state = GET_PORT_DISCOVERY_STATE(port_conf);

        if ( (oam_code != VTSS_ETH_LINK_OAM_CODE_TYPE_INFO) && (state != VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY)
           ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_STATE;
            break;
        }

        switch (oam_code) {
        case VTSS_ETH_LINK_OAM_CODE_TYPE_INFO:
            INC_INFO_RX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_EVENT:
            INC_UNIQUE_EVENT_NOTIFICATION_RX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_REQ:
            INC_VAR_REQ_RX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_RESP:
            INC_VAR_RESP_RX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_LB:
            INC_LB_RX_STATS(port_no);
            break;
        case VTSS_ETH_LINK_OAM_CODE_TYPE_ORG:
            INC_ORG_SPECIFIC_RX_STATS(port_no);
            break;
        default:
            break;
        } /* end of switch for RX stats */

        oam_header = (vtss_eth_link_oam_frame_header_t *)pdu;
        memcpy(&tmp_flags, oam_header->flags, VTSS_ETH_LINK_OAM_FLAGS_LEN);
        tmp_flags = vtss_eth_link_oam_ntohs(tmp_flags);

        if (tmp_flags & VTSS_ETH_LINK_OAM_INVALID_FLAGS) { //Invalid flags are set
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_FLAGS;
            break;
        }

        if ( (IS_FLAG_CONF_ACTIVE(tmp_flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE)) && (IS_FLAG_CONF_ACTIVE(tmp_flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE))
           ) { // 0x03 is not supported value
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_FLAGS;
            break;
        }

        if ( (oam_code == VTSS_ETH_LINK_OAM_CODE_TYPE_INFO) && (state >= VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE) ) {

            if (IS_FLAG_CONF_ACTIVE(tmp_flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_STABLE)) {
                flag_enable = TRUE;
            } else {
                flag_enable = FALSE;
            }
            rc = vtss_eth_link_oam_control_layer_port_flags_conf_set(port_no, VTSS_ETH_LINK_OAM_FLAG_REMOTE_STABLE, flag_enable);

            if (IS_FLAG_CONF_ACTIVE(tmp_flags, VTSS_ETH_LINK_OAM_FLAG_LOCAL_EVALUTE)) {
                flag_enable = TRUE;
            } else {
                flag_enable = FALSE;
            }
            rc = vtss_eth_link_oam_control_layer_port_flags_conf_set(port_no, VTSS_ETH_LINK_OAM_FLAG_REMOTE_EVALUTE, flag_enable);
        }
        if (IS_FLAG_CONF_ACTIVE(tmp_flags, VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT)) {
            INC_LINK_FAULT_RX_STATS(port_no);
        }
        if (IS_FLAG_CONF_ACTIVE(tmp_flags, VTSS_ETH_LINK_OAM_FLAG_DYING_GASP)) {
            INC_DYING_GASP_RX_STATS(port_no);
        }
        if (IS_FLAG_CONF_ACTIVE(tmp_flags, VTSS_ETH_LINK_OAM_FLAG_CRIT_EVENT)) {
            INC_CRITICAL_EVENT_RX_STATS(port_no);
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_is_local_lost_link_timer_done(const u32 port_no, BOOL *const is_local_lost_link_timer)
{
    u32   rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!is_local_lost_link_timer) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        *is_local_lost_link_timer = FALSE;

        if (GET_PORT_LOCAL_LOST_LINK_TIMER(port_no) > VTSS_ETH_LINK_OAM_NULL) {
            DEC_PORT_LOCAL_LOST_LINK_TIMER(port_no);
            if (GET_PORT_LOCAL_LOST_LINK_TIMER(port_no) == VTSS_ETH_LINK_OAM_NULL) {
                *is_local_lost_link_timer = TRUE;
            }
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_pdu_stats_t *const port_stats)
{
    u32   rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!port_stats) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        memcpy(port_stats, PORT_CONTROL_LAYER_STATS(port_no), sizeof(vtss_eth_link_oam_pdu_stats_t));

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;

}

u32 vtss_eth_link_oam_control_layer_port_critical_event_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_critical_event_pdu_stats_t *const port_ce_stats)
{
    u32   rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!port_ce_stats) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        memcpy(port_ce_stats, PORT_CONTROL_LAYER_CE_STATS(port_no), sizeof(vtss_eth_link_oam_critical_event_pdu_stats_t));

    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;

}


u32 vtss_eth_link_oam_control_layer_clear_statistics(const u32 port_no)
{
    vtss_eth_link_oam_pdu_stats_t                  *port_stats;
    vtss_eth_link_oam_critical_event_pdu_stats_t   *port_ce_stats;
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        port_stats = PORT_CONTROL_LAYER_STATS(port_no);
        port_ce_stats = PORT_CONTROL_LAYER_CE_STATS(port_no);

        if ( (!port_stats) ||
             (!port_ce_stats) ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        memset(port_stats, 0, sizeof(vtss_eth_link_oam_pdu_stats_t));
        memset(port_ce_stats, 0, sizeof(vtss_eth_link_oam_critical_event_pdu_stats_t));
    } while (VTSS_ETH_LINK_OAM_NULL);

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_mux_conf_set(const u32 port_no, const vtss_eth_link_oam_mux_state_t mux_state)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if ( (mux_state != VTSS_ETH_LINK_OAM_MUX_FWD_STATE) ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (GET_PORT_CONTROL_LAYER_MUX_STATE(port_no) == mux_state) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        }
        SET_PORT_CONTROL_LAYER_MUX_STATE(port_no, mux_state);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_mux_conf_get(const u32 port_no, vtss_eth_link_oam_mux_state_t *const mux_state)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!mux_state) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *mux_state = GET_PORT_CONTROL_LAYER_MUX_STATE(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_parser_conf_set(const u32 port_no, const vtss_eth_link_oam_parser_state_t parser_state)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if ( (parser_state < VTSS_ETH_LINK_OAM_PARSER_FWD_STATE) ||
             (parser_state > VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE)) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (GET_PORT_CONTROL_LAYER_PARSER_STATE(port_no) == parser_state) {
            rc = VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
            break;
        }
        SET_PORT_CONTROL_LAYER_PARSER_STATE(port_no, parser_state);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}

u32 vtss_eth_link_oam_control_layer_port_parser_conf_get(const u32 port_no, vtss_eth_link_oam_parser_state_t *const parser_state)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (!parser_state) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        *parser_state = GET_PORT_CONTROL_LAYER_PARSER_STATE(port_no);
    } while (VTSS_ETH_LINK_OAM_NULL); /* end of do-while */

    return rc;
}


