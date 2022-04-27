/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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


#ifndef _DHCP_HELPER_API_H_
#define _DHCP_HELPER_API_H_


#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */


/**
 * \file voice_vlan_api.h
 * \brief This file defines the APIs for the DHCP Helper module
 */


/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    DHCP_HELPER_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_DHCP_HELPER),  /**< Operation is only allowed on the master switch. */
    DHCP_HELPER_ERROR_ISID,                                                             /**< isid parameter is invalid.                      */
    DHCP_HELPER_ERROR_ISID_NON_EXISTING,                                                /**< isid parameter is non-existing.                 */
    DHCP_HELPER_ERROR_INV_PARAM,                                                        /**< Invalid parameter.                              */
    DHCP_HELPER_ERROR_REQ_TIMEOUT,                                                      /**< Request timeout.                                */
    DHCP_HELPER_ERROR_FRAME_BUF_ALLOCATED,                                              /**< Frame buffer allocated fail.                    */
};

/**
 * \brief DHCP users declaration.
 */
typedef enum {
    DHCP_HELPER_USER_HELPER,

#if defined(VTSS_SW_OPTION_DHCP_SERVER)
    DHCP_HELPER_USER_SERVER,
#endif /* VTSS_SW_OPTION_DHCP_SERVER */

#if defined(VTSS_SW_OPTION_DHCP_CLIENT)
    DHCP_HELPER_USER_CLIENT,
#endif /* VTSS_SW_OPTION_DHCP_CLIENT */

#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    DHCP_HELPER_USER_SNOOPING,
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */

#if defined(VTSS_SW_OPTION_DHCP_RELAY)
    DHCP_HELPER_USER_RELAY,
#endif /* VTSS_SW_OPTION_DHCP_RELAY */

    DHCP_HELPER_USER_CNT
} dhcp_helper_user_t;

extern const char *const dhcp_helper_user_names[DHCP_HELPER_USER_CNT];

#ifdef _DHCP_HELPER_USER_NAME_C_
/**
 * \brief DHCP Helper user names.
 * \brief Each user is associated with a text string,
 * \brief which can be used when showing DHCP detailed statistics in the Web.
 */
const char *const dhcp_helper_user_names[DHCP_HELPER_USER_CNT] = {
    [DHCP_HELPER_USER_HELPER]   = "Normal Forward",

#if defined(VTSS_SW_OPTION_DHCP_SERVER)
    [DHCP_HELPER_USER_SERVER]   = "Server",
#endif /* VTSS_SW_OPTION_DHCP_SERVER */

#if defined(VTSS_SW_OPTION_DHCP_CLIENT)
    [DHCP_HELPER_USER_CLIENT]   = "Client",
#endif /* VTSS_SW_OPTION_DHCP_CLIENT */

#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    [DHCP_HELPER_USER_SNOOPING] = "Snooping",
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */

#if defined(VTSS_SW_OPTION_DHCP_RELAY)
    [DHCP_HELPER_USER_RELAY]    =   "Relay"
#endif /* VTSS_SW_OPTION_DHCP_RELAY */
};
#endif /* _DHCP_HELPER_USER_NAME_C_ */

/* DHCP direction types */
typedef enum {
    DHCP_HELPER_DIRECTION_RX,   /* Frame receive indication */
    DHCP_HELPER_DIRECTION_TX    /* Frame transmit indication */
} dhcp_helper_direction_t;

/**
 * \brief DHCP Helper statistics.
 */
typedef struct {
    u32 discover_rx;            /* The number of discover (option 53 with value 1) packets received */
    u32 offer_rx;               /* The number of offer (option 53 with value 2) packets received */
    u32 request_rx;             /* The number of request (option 53 with value 3) packets received */
    u32 decline_rx;             /* The number of decline (option 53 with value 4) packets received */
    u32 ack_rx;                 /* The number of ACK (option 53 with value 5) packets received */
    u32 nak_rx;                 /* The number of NAK (option 53 with value 6) packets received */
    u32 release_rx;             /* The number of release (option 53 with value 7) packets received */
    u32 inform_rx;              /* The number of inform (option 53 with value 8) packets received */
    u32 leasequery_rx;          /* The number of lease query (option 53 with value 10) packets received */
    u32 leaseunassigned_rx;     /* The number of lease unassigned (option 53 with value 11) packets received */
    u32 leaseunknown_rx;        /* The number of lease unknown (option 53 with value 12) packets received */
    u32 leaseactive_rx;         /* The number of lease active (option 53 with value 13) packets received */
    u32 discard_chksum_err_rx;  /* The number of discard packet that IP/UDP checksum is error */
    u32 discard_untrust_rx;     /* The number of discard packet that are coming from untrusted port */
} dhcp_helper_stats_rx_t;

typedef struct {
    u32 discover_tx;            /* The number of discover (option 53 with value 1) packets transmited */
    u32 offer_tx;               /* The number of offer (option 53 with value 2) packets transmited */
    u32 request_tx;             /* The number of request (option 53 with value 3) packets transmited */
    u32 decline_tx;             /* The number of decline (option 53 with value 4) packets transmited */
    u32 ack_tx;                 /* The number of ACK (option 53 with value 5) packets transmited */
    u32 nak_tx;                 /* The number of NAK (option 53 with value 6) packets transmited */
    u32 release_tx;             /* The number of release (option 53 with value 7) packets transmited */
    u32 inform_tx;              /* The number of inform (option 53 with value 8) packets transmited */
    u32 leasequery_tx;          /* The number of lease query (option 53 with value 10) packets transmited */
    u32 leaseunassigned_tx;     /* The number of lease unassigned (option 53 with value 11) packets transmited */
    u32 leaseunknown_tx;        /* The number of lease unknown (option 53 with value 12) packets transmited */
    u32 leaseactive_tx;         /* The number of lease active (option 53 with value 13) packets transmited */
} dhcp_helper_stats_tx_t;

typedef struct {
    dhcp_helper_stats_rx_t rx_stats;
    dhcp_helper_stats_tx_t tx_stats;
} dhcp_helper_stats_t;

/**
 * DHCP helper port mode
 */
#define DHCP_HELPER_PORT_MODE_TRUSTED           0   /* trust port mode */
#define DHCP_HELPER_PORT_MODE_UNTRUSTED         1   /* untrust port mode */

/**
 * DHCP message type
 */
#define DHCP_HELPER_MSG_TYPE_DISCOVER           1
#define DHCP_HELPER_MSG_TYPE_OFFER              2
#define DHCP_HELPER_MSG_TYPE_REQUEST            3
#define DHCP_HELPER_MSG_TYPE_DECLINE            4
#define DHCP_HELPER_MSG_TYPE_ACK                5
#define DHCP_HELPER_MSG_TYPE_NAK                6
#define DHCP_HELPER_MSG_TYPE_RELEASE            7
#define DHCP_HELPER_MSG_TYPE_INFORM             8
#define DHCP_HELPER_MSG_TYPE_LEASEQUERY         10
#define DHCP_HELPER_MSG_TYPE_LEASEUNASSIGNED    11
#define DHCP_HELPER_MSG_TYPE_LEASEUNKNOWN       12
#define DHCP_HELPER_MSG_TYPE_LEASEACTIVE        13
#define DHCP_HELPER_MSG_FROM_SERVER(msg)        ((msg) == DHCP_HELPER_MSG_TYPE_OFFER || (msg) == DHCP_HELPER_MSG_TYPE_ACK || (msg) == DHCP_HELPER_MSG_TYPE_NAK)

/**
 * \brief DHCP Helper frame information maximum entry count
 */
#define DHCP_HELPER_FRAME_INFO_MAX_CNT          1024

/**
 * \brief DHCP Helper port configuration.
 */
typedef struct {
    u8 port_mode[VTSS_PORTS];  /* DHCP helper port mode */
} dhcp_helper_port_conf_t;

/**
 * \brief Incoming DHCP frame information.
 */
typedef struct {
    u8              mac[6];         /* Entry Key 1 */
    vtss_vid_t      vid;            /* Entry Key 2 */
    vtss_isid_t     isid;
    vtss_port_no_t  port_no;
    vtss_glag_no_t  glag_no;
    u32             op_code;
    u32             transaction_id; /* Entry Key 3, only use for uncompleted entry */
    u32             assigned_ip;
    u32             assigned_mask;
    u32             dhcp_server_ip;
    u32             gateway_ip;
    u32             dns_server_ip;
    u32             lease_time;
    u32             timestamp;
    BOOL            local_dhcp_server;
} dhcp_helper_frame_info_t;

/**
 * Callback function for DHCP packet receive
  *
  * \return
  *   TRUE  - Accept to process the DHCP packet.\n
  *   FALSE - Won't process the DHCP packet.\n
 */
typedef BOOL (*dhcp_helper_stack_rx_callback_t)(const u8 *const packet,
                                                size_t len,
                                                vtss_vid_t vid,
                                                vtss_isid_t src_isid,
                                                vtss_port_no_t src_port_no,
                                                vtss_glag_no_t src_glag_no);

/**
 * Callback function for incoming frame information
 */
typedef void (*dhcp_helper_frame_info_callback_t)(dhcp_helper_frame_info_t *info);

/**
 * Callback function for clear user's local statistics
 */
typedef void (*dhcp_helper_user_clear_local_stat_callback_t)(void);

/**
  * \brief Retrieve an error string based on a return code
  * \brief from one of the DHCP Helper API functions.
  *
  * \param rc [IN]: Error code that must be in the DHCP_HELPER_ERROR_xxx range.
  */
char *dhcp_helper_error_txt(vtss_rc rc);

/**
  * \brief Get a switch's per-port configuration.
  *
  * \param isid        [IN]: The Switch ID for which to retrieve the
  *                          configuration.
  * \param switch_cfg [OUT]: Pointer to structure that receives
  *                          the switch's per-port configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    DHCP_HELPER_ERROR_INV_PARAM if switch_cfg is NULL.\n
  *    DHCP_HELPER_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    DHCP_HELPER_ERROR_ISID if called with an invalid ISID.\n
  */
vtss_rc dhcp_helper_mgmt_port_conf_get(vtss_isid_t isid, dhcp_helper_port_conf_t *switch_cfg);

/**
  * \brief Set a switch's per-port configuration.
  *
  * \param isid       [IN]: The switch ID for which to set the configuration.
  * \param switch_cfg [IN]: Pointer to structure that contains
  *                         the switch's per-port configuration to be applied.
  *
  * \return
  *    VTSS_OK on success.\n
  *    DHCP_HELPER_ERROR_INV_PARAM if switch_cfg is NULL or parameters error.\n
  *    DHCP_HELPERG_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    DHCP_HELPER_ERROR_ISID if called with an invalid ISID.\n
  */
vtss_rc dhcp_helper_mgmt_port_conf_set(vtss_isid_t isid, dhcp_helper_port_conf_t *switch_cfg);

/**
  * \brief Initialize the DHCP helper module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc dhcp_helper_init(vtss_init_data_t *data);

/**
  * \brief Register dynmaic IP address obtained
  * \brief Callback after the IP assigned action is completed
  * \brief This API is used to notify DHCP Snooping module
  *
  * \return
  *   None.
  */
void dhcp_helper_notify_ip_addr_obtained_register(dhcp_helper_frame_info_callback_t cb);

/**
  * \brief Register dynmaic IP address released
  * \brief Callback when receive a DHCP release frame
  * \brief This API is used to notify DHCP Snooping module
  * \return
  *   None.
  */
void dhcp_helper_release_ip_addr_register(dhcp_helper_frame_info_callback_t cb);

/**
  * \brief Register DHCP user frame receive
  *
  * \return
  *   None.
  */
void dhcp_helper_user_receive_register(dhcp_helper_user_t user, dhcp_helper_stack_rx_callback_t cb);

/**
  * \brief Unregister DHCP user frame receive
  *
  * \return
  *   None.
  */
void dhcp_helper_user_receive_unregister(dhcp_helper_user_t user);

/**
  * \brief Register DHCP user clear local statistics
  *
  * \return
  *   None.
  */
void dhcp_helper_user_clear_local_stat_register(dhcp_helper_user_t user, dhcp_helper_user_clear_local_stat_callback_t cb);

/**
  * \brief  Alloc memory for transmit DHCP frame
  *
  * \return
  *   None.
  */
void *dhcp_helper_alloc_xmit(size_t len, vtss_isid_t isid, void **pbufref);

/* Transmit DHCP frame.

   Input parameter of "frame_p" must cotains DA + SA + ETYPE + UDP header + UDP payload.
   Calling API dhcp_helper_alloc_xmit() to allocate the resource for "frame_p" and get the "bufref_p".

   For DHCP Server and Client, the input parameters should be like this:
   "dst_isid" = VTSS_ISID_GLOBAL
   "dst_port_mask" = 0
   "src_isid" = VTSS_ISID_END
   "src_port_no" = VTSS_PORT_NO_NONE
   "src_glag_no" = VTSS_GLAG_NO_NONE

   Return 0  : Success
   Return -1 : Fail */
int dhcp_helper_xmit(dhcp_helper_user_t user,
                     void *frame_p,
                     size_t len,
                     vtss_vid_t vid,
                     vtss_isid_t dst_isid,
                     u64 dst_port_mask,
                     vtss_isid_t src_isid,
                     vtss_port_no_t src_port_no,
                     vtss_glag_no_t src_glag_no,
                     void *bufref_p);

/**
  * \brief Lookup DHCP helper frame information entry
  *
  * Note: Use vid = 0 to ignore the VID lookup key
  *
  * \return
  *   TRUE on success.\n
  *   FALSE if get fail.\n
  */
BOOL dhcp_helper_frame_info_lookup(u8 *mac, vtss_vid_t vid, uint transaction_id, dhcp_helper_frame_info_t *info);

/**
  * \brief Getnext DHCP helper frame information entry
  *
  * \return
  *   TRUE on success.\n
  *   FALSE if get fail.\n
  *
  * Note: This API is only for debug purpose.
  *       Use {NULL mac, vid=0, transaction_id = 0 } to get the first entry.
  *       Otherwise, the return value 'TURE' only happened when the seach keys
  *       is exact matched an existing entry keys.
  */
BOOL dhcp_helper_frame_info_getnext(u8 *mac, vtss_vid_t vid, uint transaction_id, dhcp_helper_frame_info_t *info);

/**
  * \brief Get DHCP helper statistics
  *
  * \return
  *   VTSS_OK on success.\n
  *   Others value if fail.\n
  */
vtss_rc dhcp_helper_stats_get(dhcp_helper_user_t user, vtss_isid_t isid, vtss_port_no_t port_no, dhcp_helper_stats_t *stats);

/**
  * \brief Clear DHCP helper statistics
  *
  * \return
  *   VTSS_OK on success.\n
  *   Others value if fail.\n
  */
vtss_rc dhcp_helper_stats_clear(dhcp_helper_user_t user, vtss_isid_t isid, vtss_port_no_t port_no);

/**
  * \brief Clear DHCP helper statistics by user
  *
  * \return
  *   VTSS_OK on success.\n
  *   Others value if fail.\n
  */
vtss_rc dhcp_helper_stats_clear_by_user(dhcp_helper_user_t user);

/**
  * \brief Get DHCP helper statistics
  *
  * \return
  *   VTSS_OK on success.\n
  *   Others value if fail.\n
  */
vtss_rc dhcp_helper_stats_get(dhcp_helper_user_t user, vtss_isid_t isid, vtss_port_no_t port_no, dhcp_helper_stats_t *stats);

/**
  * \brief Add DHCP statistics
  *
  */
void DHCP_HELPER_stats_add(dhcp_helper_user_t user, vtss_isid_t isid, u64 dst_port_mask, u8 dhcp_message, dhcp_helper_direction_t dhcp_message_direction);

#endif /* _DHCP_HELPER_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
