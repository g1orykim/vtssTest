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

#ifndef _DHCP_RELAY_API_H_
#define _DHCP_RELAY_API_H_


#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */


/**
 * \file dhcp_relay_api.h
 * \brief This file defines the APIs for the DHCP Relay module
 */


/**
 * DHCP relay management enabled/disabled
 */
#define DHCP_RELAY_MGMT_ENABLED         (1)       /**< Enable option  */
#define DHCP_RELAY_MGMT_DISABLED        (0)       /**< Disable option */

/**
 * DHCP relay maximum DHCP server
 */
#define DHCP_RELAY_MGMT_MAX_DHCP_SERVER (1)


/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    DHCP_RELAY_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_DHCP_RELAY),    /**< Operation is only allowed on the master switch. */
    DHCP_RELAY_ERROR_INV_PARAM,                                                         /**< Invalid parameter.                              */
};

/**
 * DHCP relay information policy configuration
 */
#define DHCP_RELAY_INFO_POLICY_REPLACE      0x0
#define DHCP_RELAY_INFO_POLICY_KEEP         0x1
#define DHCP_RELAY_INFO_POLICY_DROP         0x2

/**
 * \brief DHCP Relay default configuration values
 */
#define DHCP4R_DEF_MODE                     DHCP_RELAY_MGMT_DISABLED
#define DHCP4R_DEF_SRV_CNT                  0x0
#define DHCP4R_DEF_INFO_MODE                DHCP_RELAY_MGMT_DISABLED
#define DHCP4R_DEF_INFO_POLICY              DHCP_RELAY_INFO_POLICY_KEEP

/**
 * \brief DHCP Relay configuration.
 */
typedef struct {
    u32         relay_mode;                                     /* DHCP Relay Mode */
    u32         relay_server_cnt;                               /* DHCP Relay Mode */
    vtss_ipv4_t relay_server[DHCP_RELAY_MGMT_MAX_DHCP_SERVER];  /* DHCP Relay Server */
    u32         relay_info_mode;                                /* DHCP Relay Information Mode */
    u32         relay_info_policy;                              /* DHCP Relay Information Policy */
} dhcp_relay_conf_t;

/**
 * \brief DHCP Relay statistics.
 */
typedef struct {
    u32 server_packets_relayed;       /* Packets relayed from server to client. */
    u32 server_packet_errors;         /* Errors sending packets to servers. */
    u32 client_packets_relayed;       /* Packets relayed from client to server. */
    u32 client_packet_errors;         /* Errors sending packets to clients. */
    u32 agent_option_errors;          /* Number of packets forwarded without
                                           agent options because there was no room. */
    u32 missing_agent_option;         /* Number of packets dropped because no
                                           RAI option matching our ID was found. */
    u32 bad_circuit_id;               /* Circuit ID option in matching RAI option
                                           did not match any known circuit ID. */
    u32 missing_circuit_id;           /* Circuit ID option in matching RAI option
                                           was missing. */
    u32 bad_remote_id;                /* Remote ID option in matching RAI option
                                           did not match any known remote ID. */
    u32 missing_remote_id;            /* Remote ID option in matching RAI option
                                           was missing. */
    u32 receive_server_packets;       /* Receive DHCP message from server */
    u32 receive_client_packets;       /* Receive DHCP message from client */
    u32 receive_client_agent_option;  /* Receive relay agent information option from client */
    u32 replace_agent_option;         /* Replace relay agent information option */
    u32 keep_agent_option;            /* Keep relay agent information option */
    u32 drop_agent_option;            /* Drop relay agent information option */
} dhcp_relay_stats_t;

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the DHCP Relay API functions.
  *
  * \param rc [IN]: Error code that must be in the DHCP_RELAY_ERROR_xxx range.
  */
char *dhcp_relay_error_txt(vtss_rc rc);

/**
  * \brief Get the global DHCP relay configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    DHCP_RELAY_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    DHCP_RELAY_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc dhcp_relay_mgmt_conf_get(dhcp_relay_conf_t *glbl_cfg);

/**
  * \brief Set the global DHCP rleay configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       voice VLAN module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    DHCP_RELAY_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    DHCP_RELAY_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc dhcp_relay_mgmt_conf_set(dhcp_relay_conf_t *glbl_cfg);

/**
  * \brief Initialize the DHCP Relay module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc dhcp_relay_init(vtss_init_data_t *data);

/**
  * \brief Notify DHCP relay module when system IP address changed
  *
  * \return
  *   Nothing.
  */
void dhcp_realy_sysip_changed(u32 ip_addr);

/**
  * \brief  Get DHCP relay statistics
  *
  * \return
  *   Nothing.
  */
void dhcp_relay_stats_get(dhcp_relay_stats_t *stats);

/**
  * \brief Clear DHCP relay statistics
  *
  * \return
  *   Nothing.
  */
void dhcp_relay_stats_clear(void);

#endif /* _DHCP_RELAY_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
