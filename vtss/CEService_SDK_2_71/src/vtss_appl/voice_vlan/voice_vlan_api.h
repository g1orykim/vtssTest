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

#ifndef _VTSS_VOICE_VLAN_API_H_
#define _VTSS_VOICE_VLAN_API_H_


#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */


/**
 * \file voice_vlan_api.h
 * \brief This file defines the APIs for the Voice VLAN module
 */

#if defined(VTSS_FEATURE_QCL_V1) || defined(VTSS_FEATURE_QCL_V2)
#define VOICE_VLAN_CLASS_SUPPORTED
#endif

/**
 * Voice VLAN management enabled/disabled
 */
#define VOICE_VLAN_MGMT_ENABLED         (1)       /**< Enable option  */
#define VOICE_VLAN_MGMT_DISABLED        (0)       /**< Disable option */

/**
 * Voice VLAN secure learning age time
 */
#define VOICE_VLAN_MIN_AGE_TIME         (10)        /**< Minimum allowed aging period */
#define VOICE_VLAN_MAX_AGE_TIME         (10000000)  /**< Maximum allowed aging period */

/**
 * Voice VLAN traffic class
 */
#ifdef VTSS_ARCH_LUTON28
#define VOICE_VLAN_MAX_TRAFFIC_CLASS    (4)
#else
#define VOICE_VLAN_MAX_TRAFFIC_CLASS    (7)
#endif

/**
 * \brief Voice VLAN port mode
 */
enum {
    VOICE_VLAN_PORT_MODE_DISABLED,  /**< Disjoin from Voice VLAN                                                                                                                           */
    VOICE_VLAN_PORT_MODE_AUTO,      /**< Enable auto detect mode. It detects whether there is VoIP phone attached on the specific port and configure the Voice VLAN members automatically. */
    VOICE_VLAN_PORT_MODE_FORCED     /**< Forced join to Voice VLAN.                                                                                                                        */
};

/**
 * \brief Voice VLAN discovery protocol
 */
enum {
    VOICE_VLAN_DISCOVERY_PROTOCOL_OUI,  /**< Detect telephony device by OUI address          */
    VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP, /**< Detect telephony device by LLDP                 */
    VOICE_VLAN_DISCOVERY_PROTOCOL_BOTH  /**< Detect telephony device by OUI address and LLDP */
};

/**
 * Voice VLAN OUI check conflict configuration
 */
#define VOICE_VLAN_CHECK_CONFLICT_CONF      VOICE_VLAN_MGMT_ENABLED

/**
 * Voice VLAN OUI maximum entries counter
 */
#define VOICE_VLAN_OUI_ENTRIES_CNT      (16)  /**< Maximum allowed OUI entry number */

/**
 * Voice VLAN OUI description maximum length
 */
#define VOICE_VLAN_MAX_DESCRIPTION_LEN  (32)  /**< Maximum allowed OUI description string length */

/**
 * Default Voice VLAN configuration
 */
#define VOICE_VLAN_MGMT_DEFAULT_MODE                VOICE_VLAN_MGMT_DISABLED            /**< Default global mode        */
#define VOICE_VLAN_MGMT_DEFAULT_VID                 (1000)                                /**< Default VOice VID          */
#define VOICE_VLAN_MGMT_DEFAULT_AGE_TIME            (86400)                               /**< Default age time           */
#define VOICE_VLAN_MGMT_DEFAULT_HOLD_TIME           PSEC_HOLD_TIME_MIN                  /**< Default hold time          */
#define VOICE_VLAN_MGMT_DEFAULT_TRAFFIC_CLASS       VOICE_VLAN_MAX_TRAFFIC_CLASS        /**< Default traffic class      */

#define VOICE_VLAN_MGMT_DEFAULT_PORT_MODE           VOICE_VLAN_PORT_MODE_DISABLED       /**< Default port mode          */
#define VOICE_VLAN_MGMT_DEFAULT_SECURITY            VOICE_VLAN_MGMT_DISABLED            /**< Default security mode      */
#define VOICE_VLAN_MGMT_DEFAULT_DISCOVERY_PROTOCOL  VOICE_VLAN_DISCOVERY_PROTOCOL_OUI   /**< Default discovery protocol */


/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    VOICE_VLAN_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_VOICE_VLAN),    /**< Operation is only allowed on the master switch.                  */
    VOICE_VLAN_ERROR_ISID,                                                              /**< isid parameter is invalid.                                       */
    VOICE_VLAN_ERROR_ISID_NON_EXISTING,                                                 /**< isid parameter is non-existing.                                  */
    VOICE_VLAN_ERROR_INV_PARAM,                                                         /**< Invalid parameter.                                               */
    VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MGMT_VID,                                     /**< Voice VID is conflict with managed VID.                          */
    VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MVR_VID,                                      /**< Voice VID is conflict with MVR VID.                              */
    VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_STATIC_VID,                                   /**< Voice VID is conflict with static VID.                           */
    VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_PVID,                                         /**< Voice VID is conflict with PVID.                                 */
    VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP,                                             /**< Configure auto detect mode by LLDP but LLDP feature is disabled. */
    VOICE_VLAN_ERROR_PARM_NULL_OUI_ADDR,                                                /**< parameter error of null OUI address.                             */
    VOICE_VLAN_ERROR_REACH_MAX_OUI_ENTRY,                                               /**< OUI table reach max entries.                                     */
    VOICE_VLAN_ERROR_ENTRY_NOT_EXIST                                                    /**< Table entry not exist.                                           */
};

/**
 * \brief Voice VLAN configuration.
 */
typedef struct {
    BOOL            mode;           /**< Voice VLAN global mode setting.                                                         */
    vtss_vid_t      vid;            /**< Voice VLAN ID. It should be a unique VLAN ID in the system.                             */
    u32             age_time;       /**< Voice VLAN secure learning age time.                                                    */
    vtss_prio_t     traffic_class;  /**< Voice VLAN traffic class. The switch can classifying and scheduling to network traffic. */
} voice_vlan_conf_t;

/**
 * \brief Voice VLAN port configuration.
 */
typedef struct {
    int     port_mode[VTSS_PORTS];              /**< Enable auto detect mode or configure manual.                                                                                          */
    BOOL    security[VTSS_PORTS];               /**< When security mode is enabled, all non-telephone MAC address in Voice VLAN will be removed.                                           */
    int     discovery_protocol[VTSS_PORTS];     /**< Detect telephony device by the discovery protocol.                                                                                    */
} voice_vlan_port_conf_t;

/**
 * \brief Voice VLAN OUI entry.
 */
typedef struct {
    BOOL    valid;                                              /**< Internal state.                                                                     */
    u8      oui_addr[3];                                        /**< An OUI address is a globally unique identifier assigned to a vendor by IEEE.        */
    char    description[VOICE_VLAN_MAX_DESCRIPTION_LEN + 1];    /**< The description of OUI address. Normaly, it descript which vendor telephony device. */
} voice_vlan_oui_entry_t;

/**
 * \brief Voice VLAN LLDP telephony MAC entry.
 */
typedef struct {
    BOOL            valid;      /**< Internal state.     */
    vtss_isid_t     isid;       /**< Internal switch ID. */
    vtss_port_no_t  port_no;    /**< Port number.        */
    u8              mac[6];     /**< MAC address.        */
} voice_vlan_lldp_telephony_mac_entry_t;


/**
  * \brief Retrieve an error string based on a return code
  *        from one of the Voice VLAN API functions.
  *
  * \param rc [IN]: Error code that must be in the VOICE_VLAN_ERROR_xxx range.
  */
char *voice_vlan_error_txt(vtss_rc rc);

/**
  * \brief Get the global Voice VLAN configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc voice_vlan_mgmt_conf_get(voice_vlan_conf_t *glbl_cfg);

/**
  * \brief Set the global Voice VLAN configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       voice VLAN module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP if it is a conflict configuration with LLDP.\n
  *    Others value arises from sub-function.\n
  */
vtss_rc voice_vlan_mgmt_conf_set(voice_vlan_conf_t *glbl_cfg);

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
  *    VOICE_VLAN_ERROR_INV_PARAM if switch_cfg is NULL.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VOICE_VLAN_ERROR_ISID if called with an invalid ISID.\n
  */
vtss_rc voice_vlan_mgmt_port_conf_get(vtss_isid_t isid, voice_vlan_port_conf_t *switch_cfg);

/**
  * \brief Set a switch's per-port configuration.
  *
  * \param isid       [IN]: The switch ID for which to set the configuration.
  * \param switch_cfg [IN]: Pointer to structure that contains
  *                         the switch's per-port configuration to be applied.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_INV_PARAM if switch_cfg is NULL or parameters error.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VOICE_VLAN_ERROR_ISID if called with an invalid ISID.\n
  *    VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP if it is a conflict configuration with LLDP.\n
  */
vtss_rc voice_vlan_mgmt_port_conf_set(vtss_isid_t isid, voice_vlan_port_conf_t *switch_cfg);


//
// Other public Voice VLAN functions.
//

/**
  * \brief Add or set Voice VLAN OUI entry
  *
  * \param entry [IN]: Pointer to structure that contains
  *                    the entry configuration to be applied.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_INV_PARAM if entry is NULL.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VOICE_VLAN_ERROR_PARM_NULL_OUI_ADDR if parameter "oui_addr" is null OUI address.\n
  *    VOICE_VLAN_ERROR_REACH_MAX_OUI_ENTRY if reach maximum entries number.\n
  */
vtss_rc voice_vlan_oui_entry_add(voice_vlan_oui_entry_t *entry);

/**
  * \brief Delete Voice VLAN OUI entry
  *
  * \param entry [IN]: Pointer to structure that contains
  *                    the entry configuration to be applied.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_INV_PARAM if entry is NULL.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VOICE_VLAN_ERROR_ENTRY_NOT_EXIST if delete entry not exist.\n
  *    Others value arises from sub-function.\n
  */
vtss_rc voice_vlan_oui_entry_del(voice_vlan_oui_entry_t *entry);

/**
  * \brief Clear Voice VLAN OUI entry
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc voice_vlan_oui_entry_clear(void);

/**
  * \brief Get Voice VLAN OUI entry
  *
  * \param entry [IN]: Pointer to structure that contains
  *                    the entry configuration to be applied.
  *                    The entry key is OUI address.
  *                    Use null OUI address to get first entry.
  * \param next  [IN]: Set 0 to get current entry.
  *                    Set 1 to get next valid entry.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_INV_PARAM if entry is NULL.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VTSS_RC_ERROR if get or getnext operation fail.\n
  */
vtss_rc voice_vlan_oui_entry_get(voice_vlan_oui_entry_t *entry, BOOL next);

/**
  * \brief Initialize the Voice VLAN module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc voice_vlan_init(vtss_init_data_t *data);

/* The API uses for checking conflicted configuration with LLDP module.
 * User cannot set LLDP port mode to ¡§disabled or TX only¡¨ when Voice-VLAN
 * support LLDP discovery protocol. */
/**
  * \brief Get Voice VLAN is supported LLDP discovery protocol
  *
  * \param isid    [IN]: The switch ID for which to set the configuration.
  * \param port_no [IN]: The port number for which to set the configuration.
  *
  * \return
  *    TRUE if supported.\n
  *    FALSE if not supported.\n
  */
BOOL voice_vlan_is_supported_LLDP_discovery(vtss_isid_t isid, vtss_port_no_t port_no);

/**
  * \brief Check Voice VLAN ID is conflict with other configurations
  *
  * \param voice_vid [IN]: The Voice VLAN ID.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MGMT_VID if Voice VID is conflict with managed VID.\n
  *    VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MVR_VID if Voice VID is conflict with MVR VID.\n
  *    VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_STATIC_VID if Voice VID is conflict with static VID.\n
  *    VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_PVID if Voice VID is conflict with PVID.\n
  */
vtss_rc VOICE_VLAN_is_valid_voice_vid(vtss_vid_t voice_vid);

/**
  * \brief Get Voice VLAN telephony MAC entry (It is only used for debug command)
  *
  * \param entry [IN]: Pointer to structure that contains
  *                    the entry configuration to be applied.
  *                    The entry key is MAC address.
  *                    Use MAC OUI address to get first entry.
  * \param next  [IN]: Set 0 to get current entry.
  *                    Set 1 to get next valid entry.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VOICE_VLAN_ERROR_INV_PARAM if entry is NULL.\n
  *    VOICE_VLAN_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VTSS_RC_ERROR if get or getnext operation fail.\n
  */
vtss_rc voice_vlan_lldp_telephony_mac_entry_get(voice_vlan_lldp_telephony_mac_entry_t *entry, BOOL next);


#endif /* _VTSS_VOICE_VLAN_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
