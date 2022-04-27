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

#ifndef _DHCP_SNOOPING_API_H_
#define _DHCP_SNOOPING_API_H_


#include "dhcp_helper_api.h"
#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */


/**
 * \file dhcp_snooping_api.h
 * \brief This file defines the APIs for the DHCP Snooping module
 */


/**
 * DHCP Snooping managent enabled/disabled
 */
#define DHCP_SNOOPING_MGMT_ENABLED         (1)       /**< Enable option  */
#define DHCP_SNOOPING_MGMT_DISABLED        (0)       /**< Disable option */

/**
 * DHCP snooping port mode
 */
#define DHCP_SNOOPING_PORT_MODE_TRUSTED     DHCP_HELPER_PORT_MODE_TRUSTED   /* trust port mode */
#define DHCP_SNOOPING_PORT_MODE_UNTRUSTED   DHCP_HELPER_PORT_MODE_UNTRUSTED   /* untrust port mode */

/**
 * \brief DHCP Snooping frame information maximum entry count
 */
#define DHCP_SNOOPING_FRAME_INFO_MAX_CNT          1024

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    DHCP_SNOOPING_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_DHCP_SNOOPING),  /**< Operation is only allowed on the master switch. */
    DHCP_SNOOPING_ERROR_ISID,                                                               /**< isid parameter is invalid.                      */
    DHCP_SNOOPING_ERROR_ISID_NON_EXISTING,                                                  /**< isid parameter is non-existing.                 */
    DHCP_SNOOPING_ERROR_INV_PARAM,                                                          /**< Invalid parameter.                              */
};

/**
 * Default configuration
 */
#define DHCP_SNOOPING_DEFAULT_PORT_MODE     DHCP_HELPER_PORT_MODE_TRUSTED   /**< Default port mode */

/**
 * \brief DHCP Snooping configuration
 */
typedef struct {
    u32 snooping_mode;  /* DHCP snooping mode */
} dhcp_snooping_conf_t;

/**
 * \brief DHCP Snooping port configuration
 */
typedef struct {
    u8 port_mode[VTSS_PORTS];  /* DHCP helper port mode */
    u8 veri_mode[VTSS_PORTS];  /* DHCP snooping verification mode */
} dhcp_snooping_port_conf_t;

/**
 * DHCP snooping port statistics
 */
typedef dhcp_helper_stats_t dhcp_snooping_stats_t;

/**
 * DHCP snooping frame information
 */
typedef dhcp_helper_frame_info_t dhcp_snooping_ip_assigned_info_t;

/**
 * Callback function for IP assigned information
 */
typedef enum {
    DHCP_SNOOPING_INFO_REASON_ASSIGN_COMPLETED,
    DHCP_SNOOPING_INFO_REASON_RELEASE,
    DHCP_SNOOPING_INFO_REASON_LEASE_TIMEOUT,
    DHCP_SNOOPING_INFO_REASON_MODE_DISABLED,
    DHCP_SNOOPING_INFO_REASON_PORT_LINK_DOWN,
    DHCP_SNOOPING_INFO_REASON_SWITCH_DOWN,
    DHCP_SNOOPING_INFO_REASON_ENTRY_DUPLEXED
} dhcp_snooping_info_reason_t;

/**
 * DHCP snooping IP assigned infomation callback
 */
typedef void (*dhcp_snooping_ip_assigned_info_callback_t)(dhcp_snooping_ip_assigned_info_t *info, dhcp_snooping_info_reason_t reason);

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the DHCP Snooping API functions.
  *
  * \param rc [IN]: Error code that must be in the DHCP_SNOOPING_ERROR_xxx range.
  */
char *dhcp_snooping_error_txt(vtss_rc rc);

/**
  * \brief Get the global DHCP snooping configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    DHCP_SNOOPING_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    DHCP_SNOOPING_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc dhcp_snooping_mgmt_conf_get(dhcp_snooping_conf_t *glbl_cfg);

/**
  * \brief Set the global DHCP snooping configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       voice VLAN module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    DHCP_SNOOPING_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    DHCP_SNOOPING_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc dhcp_snooping_mgmt_conf_set(dhcp_snooping_conf_t *glbl_cfg);

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
  *    DHCP_SNOOPING_ERROR_INV_PARAM if switch_cfg is NULL.\n
  *    DHCP_SNOOPING_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    DHCP_SNOOPING_ERROR_ISID if called with an invalid ISID.\n
  */
vtss_rc dhcp_snooping_mgmt_port_conf_get(vtss_isid_t isid, dhcp_snooping_port_conf_t *switch_cfg);

/**
  * \brief Set a switch's per-port configuration.
  *
  * \param isid       [IN]: The switch ID for which to set the configuration.
  * \param switch_cfg [IN]: Pointer to structure that contains
  *                         the switch's per-port configuration to be applied.
  *
  * \return
  *    VTSS_OK on success.\n
  *    DHCP_SNOOPING_ERROR_INV_PARAM if switch_cfg is NULL or parameters error.\n
  *    DHCP_SNOOPING_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    DHCP_SNOOPING_ERROR_ISID if called with an invalid ISID.\n
  */
vtss_rc dhcp_snooping_mgmt_port_conf_set(vtss_isid_t isid, dhcp_snooping_port_conf_t *switch_cfg);

/**
  * \brief Initialize the DHCP Snooping module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc dhcp_snooping_init(vtss_init_data_t *data);

/**
  * \brief Get DHCP snooping statistics
  *
  * \return
  *   0 on success.\n
  *   -1 if fail.\n
  */
int dhcp_snooping_stats_get(vtss_isid_t isid, vtss_port_no_t port_no, dhcp_snooping_stats_t *stats);

/**
  * \brief Clear DHCP snooping statistics
  *
  * \return
  *   0 on success.\n
  *   -1 if fail.\n
  */
int dhcp_snooping_stats_clear(vtss_isid_t isid, vtss_port_no_t port_no);

/**
  * \brief Getnext DHCP snooping IP assigned information entry
  *
  * \return
  *   TRUE on success.\n
  *   FALSE if get fail.\n
  */
BOOL dhcp_snooping_ip_assigned_info_getnext(u8 *mac, vtss_vid_t vid, dhcp_snooping_ip_assigned_info_t *info);

/**
  * \brief Register IP assigned information
  *
  * \return
  *   Nothing.
  */
void dhcp_snooping_ip_assigned_info_register(dhcp_snooping_ip_assigned_info_callback_t cb);

/**
  * \brief Unregister IP assigned information
  *
  * \return
  *   Nothing.
  */
void dhcp_snooping_ip_assigned_info_unregister(dhcp_snooping_ip_assigned_info_callback_t cb);

/**
  * \brief Get the global DHCP snooping default configuration.
  *
  * \param glbl_cfg [IN_OUT]: Pointer to structure that contains the
  *                           configuration to get the default setting.
  *
  * \return
  *   Nothing.
  */
void dhcp_snooping_mgmt_conf_get_default(dhcp_snooping_conf_t *conf);

/**
  * \brief Determine if DHCP snooping configuration has changed.
  *
  * \param old [IN]: Pointer to structure that contains the
  *                  old configuration.
  * \param new [IN]: Pointer to structure that contains the
  *                  new configuration.
  *
  * \return
  *   0: No change.\n
  *   none zero: Configuration changed.\n
  */
int dhcp_snooping_mgmt_conf_changed(dhcp_snooping_conf_t *old, dhcp_snooping_conf_t *new);

/**
  * \brief Get the global DHCP snooping port default configuration.
  *
  * \param glbl_cfg [IN_OUT]: Pointer to structure that contains the
  *                           configuration to get the default setting.
  *
  * \return
  *   Nothing.
  */
void dhcp_snooping_mgmt_port_get_default(vtss_isid_t isid, dhcp_snooping_port_conf_t *conf);

/**
  * \brief Determine if DHCP snooping port configuration has changed.
  *
  * \param old [IN]: Pointer to structure that contains the
  *                  old configuration.
  * \param new [IN]: Pointer to structure that contains the
  *                  new configuration.
  *
  * \return
  *   0: No change.\n
  *   none zero: Configuration changed.\n
  */
int dhcp_snooping_mgmt_port_conf_changed(dhcp_snooping_port_conf_t *old, dhcp_snooping_port_conf_t *new);

#endif /* _DHCP_SNOOPING_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
