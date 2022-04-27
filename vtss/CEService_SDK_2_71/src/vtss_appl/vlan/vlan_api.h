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

#ifndef _VLAN_API_H_
#define _VLAN_API_H_

/**
 * First configurable VLAN ID
 */
#define VLAN_ID_MIN 1

/**
 * Last configurable VLAN ID. This is the only place to change
 * if you want the maximum VLAN ID to be 4094.
 */
#define VLAN_ID_MAX 4095

/**
 * Maximum number of VLANs that can be created in the system.
 * The number does not restrict the VLAN IDs that can be
 * created - only the number of VLANs. Changing the number
 * of VLANs that can be created affects the memory usage.
 * See VLAN_ID_MIN and VLAN_ID_MAX for VLAN ID range.
 * By default, we create the number of VLANs required
 * by the VLAN ID range.
 */
#define VLAN_ENTRY_CNT (VLAN_ID_MAX - VLAN_ID_MIN + 1)

/**
 * Defaults to VTSS_VID_DEFAULT, which is 1, but for
 * customization, you may change it to anything.
 */
#define VLAN_ID_DEFAULT VTSS_VID_DEFAULT

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/**
 * Maximum length - including terminating NULL - of a VLAN name
 */
#define VLAN_NAME_MAX_LEN 33

/**
 * Name of default VLAN (VLAN_ID_DEFAULT).
 * This cannot be changed runtime.
 */
#define VLAN_NAME_DEFAULT "default"
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

/**
 * S-tag Ethertype.
 */
#define VLAN_S_TAG_ETHERTYPE 0x88A8

/**
 * Default custom S-tag Ethertype.
 */
#define VLAN_CUSTOM_S_TAG_DEFAULT VLAN_S_TAG_ETHERTYPE

/**
 * This enum identifies VLAN users.
 *
 * A VLAN user is a module that can modify VLAN configuration
 * at runtime. VLAN_USER_STATIC corresponds to an end-user, that
 * is, an administrator using CLI, SNMP, or Web to change VLAN
 * configuration.
 *
 * Only VLAN_USER_STATIC configuration will be saved to flash.
 * All other users override the current VLAN configuration
 * in a hierarchical, prioritized way.
 *
 * The following enumeration also defines the prioritization.
 * VLAN_USER_STATIC has the lowest priority. VLAN_USER_DOT1X
 * has the highest, VLAN_USER_MVRP the next highest, and so on.
 * (note the discontinuation between VLAN_USER_STATIC and
 * VLAN_USER_DOT1X).
 *
 * This means that if e.g. VLAN_USER_STATIC has decided that PVID
 * should be 1 on a port, but VLAN_USER_DOT1X decides it should be
 * 17, then VLAN_USER_DOT1X wins.
 *
 * If later on, VLAN_USER_VOICE_VLAN decides to change the PVID to
 * e.g. 23, then a conflict emerges. Since VLAN_USER_DOT1X has
 * higher priority than VLAN_USER_VOICE_VLAN, VLAN_USER_DOT1X
 * wins, and the PVID remains at 17.
 *
 * If then VLAN_USER_DOT1X withdraws its override of the PVID, then
 * VLAN_USER_VOICE_VLAN will get to win, and the PVID will be
 * set to 23.
 *
 * When conflicts arise, a message is sent to the system log (if enabled).
 */
typedef enum {
    VLAN_USER_STATIC = 0, /**< End-user. Do not change this position */
#if defined(VTSS_SW_OPTION_DOT1X)
    VLAN_USER_DOT1X,      /**< 802.1X/NAS */
#endif



#if defined(VTSS_SW_OPTION_GVRP)
    VLAN_USER_GVRP,       /**< GVRP */
#endif
#if defined(VTSS_SW_OPTION_MVR)
    VLAN_USER_MVR,        /**< MVR */
#endif
#if defined(VTSS_SW_OPTION_VOICE_VLAN)
    VLAN_USER_VOICE_VLAN, /**< Voice VLAN */
#endif
#if defined(VTSS_SW_OPTION_MSTP)
    VLAN_USER_MSTP,       /**< MSTP */
#endif
#if defined(VTSS_SW_OPTION_ERPS)
    VLAN_USER_ERPS,       /**< ERPS */
#endif
#if defined(VTSS_SW_OPTION_MEP)
    VLAN_USER_MEP,        /**< MEP */
#endif
#if defined(VTSS_SW_OPTION_EVC)
    VLAN_USER_EVC,        /**< EVC */
#endif
#if defined(VTSS_SW_OPTION_VCL)
    VLAN_USER_VCL,        /**< VCL */
#endif
    VLAN_USER_FORBIDDEN,  /**< Not a real user, but easiest to deal with in terms of management, when forbidden VLAN is enumerated as a user. Must come just before VLAN_USER_ALL. */
    VLAN_USER_ALL,        /**< Used in XXX_get() functions to get the current configuration. Cannot be used in XXX_set() functions. Do not change this position. */
    VLAN_USER_CNT         /**< Used to size various structures and iterate. Do not change this position. */
} vlan_user_t;

/**
 * VLAN module error codes (vtss_rc)
 */
enum {
    VLAN_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_VLAN), /**< Generic error code                     */
    VLAN_ERROR_ISID,                                          /**< Invalid ISID                           */
    VLAN_ERROR_PORT,                                          /**< Invalid port number                    */
    VLAN_ERROR_MUST_BE_MASTER,                                /**< Operation only valid on master switch  */
    VLAN_ERROR_NOT_CONFIGURABLE,                              /**< Switch not configurable                */
    VLAN_ERROR_USER,                                          /**< Invalid user                           */
    VLAN_ERROR_VID,                                           /**< Invalid VLAN ID                        */
    VLAN_ERROR_FLAGS,                                         /**< Invalid vlan_port_conf_t::flags        */
    VLAN_ERROR_PVID,                                          /**< Invalid vlan_port_conf_t::pvid         */
    VLAN_ERROR_FRAME_TYPE,                                    /**< Invalid vlan_port_conf_t::frame_type   */
    VLAN_ERROR_TX_TAG_TYPE,                                   /**< Invalid vlan_port_conf_t::tx_tag_type  */
    VLAN_ERROR_UVID,                                          /**< Invalid vlan_port_conf_t::untagged_vid */
    VLAN_ERROR_TPID,                                          /**< Invalid TPID                           */
    VLAN_ERROR_PORT_MODE,                                     /**< Invalid port mode parameter            */
    VLAN_ERROR_PARM,                                          /**< Illegal parameter */
    VLAN_ERROR_ENTRY_NOT_FOUND,                               /**< VLAN not found */
    VLAN_ERROR_VLAN_TABLE_FULL,                               /**< VLAN table full */
    VLAN_ERROR_USER_PREVIOUSLY_CONFIGURED,
#if defined(VTSS_SW_OPTION_VLAN_NAMING)
    VLAN_ERROR_NAME_ALREADY_EXISTS,                           /**< VLAN Name is already configured               */
    VLAN_ERROR_NAME_RESERVED,                                 /**< The VLAN name is reserved for another VLAN ID */
    VLAN_ERROR_NAME_INVALID,                                  /**< The VLAN name contains invalid chars          */
    VLAN_ERROR_NAME_DOES_NOT_EXIST,                           /**< VLAN Name does not exist in table             */
    VLAN_ERROR_NAME_DEFAULT_VLAN,                             /**< The default VLAN's name cannot be changed     */
#endif
}; // Leave it anonymous

/******************************************************************************/
//
// GLOBAL CONFIGURATION FUNCTIONS ET AL
//
/******************************************************************************/

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
/**
 * Set ethertype for Custom S-port
 *
 * \param tpid [IN] TPID (Ethertype) for ports marked as Custom-S aware.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code.
 */
vtss_rc vlan_mgmt_s_custom_etype_set(vtss_etype_t tpid);
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
/**
 * Get ethertype for Custom S-port
 *
 * \param tpid [OT] Pointer receiving current TPID (Ethertype) for ports marked as Custom-S aware.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code.
 */
vtss_rc vlan_mgmt_s_custom_etype_get(vtss_etype_t *tpid);
#endif /* defined(VTSS_FEATURE_VLAN_PORT_V2) */

/******************************************************************************/
//
// PORT FUNCTIONS ET AL
//
/******************************************************************************/

/**
 * Controls how egress tagging occurs.
 *
 * Don't change the enumeration without also changing vlan_port.htm.
 */
typedef enum {
    VLAN_TX_TAG_TYPE_UNTAG_THIS, /**< Send .untagged_vid untagged. User module doesn't care about other VIDs.                  */
    VLAN_TX_TAG_TYPE_TAG_THIS,   /**< Send .untagged_vid tagged. User module doesn't care about other VIDs.                    */
    VLAN_TX_TAG_TYPE_TAG_ALL,    /**< All 4K VLANs shall be sent tagged, despite this user module's membership configuration   */
    VLAN_TX_TAG_TYPE_UNTAG_ALL,  /**< All 4K VLANs shall be sent untagged, despite this user module's membership configuration */
} vlan_tx_tag_type_t;

/**
 * Flags to indicate what part of a VLAN port configuration,
 * the user wants to configure.
 *
 * Do a bit-wise OR of the flags to indicate which members
 * of vlan_port_conf_t you wish to control.
 *
 * To uncontrol a feature that was previously overridden,
 * clear the corresponding flag (this only works for
 * volatile users (i.e. user != VLAN_USER_STATIC).
 */
enum {
    VLAN_PORT_FLAGS_PVID        = (1 << 0), /**< Control vlan_port_conf_t::pvid                                                         */
    VLAN_PORT_FLAGS_INGR_FILT   = (1 << 1), /**< Control vlan_port_conf_t::ingress_filter                                               */
    VLAN_PORT_FLAGS_RX_TAG_TYPE = (1 << 2), /**< Control vlan_port_conf_t::frame_type                                                   */
    VLAN_PORT_FLAGS_TX_TAG_TYPE = (1 << 3), /**< Control vlan_port_conf_t::tx_tag_type and possibly also vlan_port_conf_t::untagged_vid */
    VLAN_PORT_FLAGS_AWARE       = (1 << 4), /**< Control vlan_port_conf_t::port_type                                                    */
    VLAN_PORT_FLAGS_ALL         = (VLAN_PORT_FLAGS_PVID | VLAN_PORT_FLAGS_INGR_FILT | VLAN_PORT_FLAGS_RX_TAG_TYPE | VLAN_PORT_FLAGS_TX_TAG_TYPE | VLAN_PORT_FLAGS_AWARE)
}; // Anonymous to satisfy Lint.

/**
 * VLAN awareness and port type configuration.
 * Ports that are not configured as VLAN_PORT_TYPE_UNAWARE
 * are VLAN aware and react on the corresponding
 * tag type. VLAN aware ports always react on
 * C tags.
 */
typedef enum {
    VLAN_PORT_TYPE_UNAWARE, /**< VLAN unaware port                       */
    VLAN_PORT_TYPE_C,       /**< C-port (TPID = 0x8100)                  */
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    VLAN_PORT_TYPE_S,       /**< S-port (TPID = 0x88A8)                  */
    VLAN_PORT_TYPE_S_CUSTOM /**< S-port using customizable ethernet type */
#endif
} vlan_port_type_t;

/**
 * VLAN port configuration
 */
typedef struct {
    /**
     * Port VLAN ID. [VLAN_ID_MIN; VLAN_ID_MAX].
     */
    vtss_vid_t pvid;

    /**
     * Port Untagged VLAN ID (egress).
     *
     * If #tx_tag_type == VLAN_TX_TAG_TYPE_UNTAG_THIS:
     *   #untagged_vid indicates the only VID [VLAN_ID_MIN; VLAN_ID_MAX] not to tag.
     *
     * If #tx_tag_type == VLAN_TX_TAG_TYPE_TAG_THIS:
     *   #untagged_vid indicates the VID [VLAN_ID_MIN; VLAN_ID_MAX] to tag.
     *   In reality, this is implemented as follows:
     *   If #untagged_vid == #pvid, then all frames are tagged.
     *   If #untagged_vid != #pvid, then all but #pvid are tagged.
     *
     * If #tx_tag_type == VLAN_TX_TAG_TYPE_TAG_ALL:
     *  All frames are tagged on egress. #untagged_vid is not used, and shouldn't be shown/used.
     *
     * If #tx_tag_type == VLAN_TX_TAG_TYPE_UNTAG_ALL:
     *  All frames are untagged on egress. #untagged_vid is not used, and shouldn't be shown/used.
     */
    vtss_vid_t untagged_vid;

    /**
     * Acceptable frame type (ingress).
     * Either, accept all, accept tagged only, or accept untagged only.
     */
    vtss_vlan_frame_t frame_type;

    /**
     * Ingress filtering.
     * If enabled, incoming frames classified to a VLAN that
     * the port is not a member of are discarded.
     * It's a compile-time option to get this user-controllable
     * (see VTSS_SW_OPTION_VLAN_INGRESS_FILTERING).
     * If disabled, ingress filtering is always enabled.
     */
    BOOL ingress_filter;

    /**
     * Indicates egress tag requirements. See also #untagged_vid.
     */
    vlan_tx_tag_type_t tx_tag_type;

    /**
     * Controls VLAN awareness and whether it reacts to
     * C-tags, S-tags, and Custom-S-tags.
     */
    vlan_port_type_t port_type;

    /**
     * Flags to indicate what part of the VLAN port configuration,
     * user wants to configure in calls to vlan_mgmt_port_conf_set().
     * They are a bit-wise OR of VLAN_PORT_FLAGS_xxx.
     *
     * If a volatile user (i.e. user != VLAN_USER_STATIC) wishes
     * to un-override a feature that was previously overridden,
     * clear the flag.
     */
    u8 flags;
} vlan_port_conf_t;

/**
 * Get VLAN port configuration.
 *
 * The function returns the VLAN port configuration
 * for #isid:#port. #port must be a normal, non-stack port.
 *
 * The function can get info directly from the switch API
 * and from a S/W state. Which of these depends on the
 * value of #isid, as follows:
 *
 * #isid == VTSS_ISID_LOCAL:
 *   Can be called on both a slave and master.
 *   Either way, it will return the values currently
 *   stored in H/W on the local switch, and will therefore
 *   not be retrievable for a given VLAN user, which means
 *   that you must specify #user == VLAN_USER_ALL, which
 *   is a synonym for the VLAN port configuration combined
 *   for all users. If you fail to specify VLAN_USER_ALL,
 *   this function will return VLAN_ERROR_USER.
 *
 * #isid == [VTSS_ISID_START; VTSS_ISID_END[ (i.e. a legal ISID):
 *   Can only be called on the master.
 *   In this case, #user may  be in range
 *   [VLAN_USER_STATIC; VLAN_USER_ALL], where VLAN_USER_ALL
 *   causes this function to retrieve the combined VLAN port
 *   configuration.
 *
 * #isid == VTSS_ISID_GLOBAL:
 *   Illegal.
 *
 * \param isid [IN]  VTSS_ISID_LOCAL or legal ISID. Functionality as specified above.
 * \param port [IN]  Valid, non-stacking port number.
 * \param conf [OUT] Pointer to structure retrieving the current port configuration.
 * \param user [IN]  VLAN user to obtain configuration for. See also description above.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code.
 */
vtss_rc vlan_mgmt_port_conf_get(vtss_isid_t isid, vtss_port_no_t port, vlan_port_conf_t *conf, vlan_user_t user);

/**
 * Change VLAN port configuration.
 *
 * Change the VLAN port configuration for #user.
 * There is no guarantee that the configuration will take effect, because
 * it could happen that a higher prioritized user has already configured
 * the features that are attempted configured now.
 * Please check vlan_mgmt_conflicts_get() if this function returns
 * VTSS_RC_OK and you don't see your changes take effect.
 *
 * This function should not be called from outside with #user == VLAN_USER_STATIC,
 * since this is handled through vlan_mgmt_port_composite_conf_set().
 *
 * \param isid [IN] ISID of a configurable switch. Must be in interval [VTSS_ISID_START; VTSS_ISID_END[.
 * \param port [IN] Port number (iport) to change configuration for. Stack ports not allowed.
 * \param conf [IN] New configuration.
 * \param user [IN] VLAN user in range ]VLAN_USER_STATIC; VLAN_USER_ALL[.
 *
 * \return VTSS_RC_OK on success (but still no guarantee that the changes will take effect),
 *         anything else on error. Use error_txt(return code) to get a description.
 **/
vtss_rc vlan_mgmt_port_conf_set(vtss_isid_t isid, vtss_port_no_t port, vlan_port_conf_t *conf, vlan_user_t user);

/**
 * Index into vlan_port_conflicts_t::users[] used to
 * get the VLAN user that caused a conflict for a particular
 * port configuration parameter.
 */
typedef enum {
    VLAN_PORT_FLAGS_IDX_PVID = 0,    /**< Index into vlan_port_conflicts_t::users[] that gives PVID conflicting users                    */
    VLAN_PORT_FLAGS_IDX_INGR_FILT,   /**< Index into vlan_port_conflicts_t::users[] that gives ingress filter conflicting users          */
    VLAN_PORT_FLAGS_IDX_RX_TAG_TYPE, /**< Index into vlan_port_conflicts_t::users[] that gives acceptable frame type conflicting users   */
    VLAN_PORT_FLAGS_IDX_TX_TAG_TYPE, /**< Index into vlan_port_conflicts_t::users[] that gives egress tagging conflicting users          */
    VLAN_PORT_FLAGS_IDX_AWARE,       /**< Index into vlan_port_conflicts_t::users[] that gives awareness and port type conflicting users */
    VLAN_PORT_FLAGS_IDX_CNT          /**< Must come last. Used to size arrays and stop iteration                                         */
} vlan_port_flags_idx_t;

/**
 * This structure is used for VLAN port configuration
 * conflict displaying in the user interface.
 */
typedef struct {
    /**
     * These flags indicate type of the conflict. For example, if there
     * is an Ingress filter conflict, the VLAN_PORT_FLAGS_INGR_FILT bit will
     * be set in the mask.
     */
    u8 port_flags;

    /**
     * Each entry is indexed by a VLAN_PORT_FLAGS_IDX_xxx and
     * contains a bitmask of conflicting VLAN users.
     */
    u32 users[VLAN_PORT_FLAGS_IDX_CNT];
} vlan_port_conflicts_t;

/**
 * Get VLAN port conflicts for a given port.
 *
 * \param isid      [IN]  Legal ISID to switch to get conflicts for.
 * \param port      [IN]  Port number to get conflicts for. Stack ports not allowed.
 * \param conflicts [OUT] Pointer receiving conflicts.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code.
 **/
vtss_rc vlan_mgmt_conflicts_get(vtss_isid_t isid, vtss_port_no_t port, vlan_port_conflicts_t *conflicts);

/**
 * VLAN Port Modes, as defined and used by ICLI.
 *
 * Do not change the order of ACCESS, TRUNK, and HYBRID,
 * since these are used in iterators here and there.
 */
typedef enum {
    /**
     * Access port.
     * An access port:
     *  - is C-tag VLAN aware,
     *  - has ingress filtering enabled,
     *  - accepts both tagged and untagged frames,
     *  - has PVID set to vlan_port_composite_conf_t::access_vid,
     *  - member of vlan_port_composite_conf_t::access_vid, only,
     *  - untags all frames on egress
     *
     * An access port has these low-level properties:
     *  low_level->tx_tag_type    = VLAN_TX_TAG_TYPE_UNTAG_ALL;
     *  low_level->frame_type     = VTSS_VLAN_FRAME_ALL;
     *  low_level->ingress_filter = TRUE;
     *  low_level->port_type      = VLAN_PORT_TYPE_C;
     *  low_level->pvid           = high_level->access_vid; // Default is VLAN_ID_DEFAULT
     */
    VLAN_PORT_MODE_ACCESS,

    /**
     * Trunk port.
     * A trunk port:
     *  - is C-tag VLAN aware,
     *  - has ingress filtering enabled,
     *  - has PVID set to vlan_port_composite_conf_t::native_vid,
     *  - automatically becomes a member of all VLANs that are set in the array
     *    defining allowed VIDs for port mode = VLAN_PORT_MODE_TRUNK
     *    (see vlan_mgmt_port_composite_allowed_vids_get()),
     *  - allows for having the native_vid tagged or untagged on egress,
     *  - if tagging native_vid, the port accepts tagged frames only
     *  - if untagging native_vid, the port accepts both tagged and untagged frames.
     *
     * A trunk port has these low-level properties:
     *  low_level->pvid            = high_level->native_vid; // Default is VLAN_ID_DEFAULT
     *  low_level->untagged_vid    = high_level->native_vid; // Default is VLAN_ID_DEFAULT
     *  low_level->port_type       = VLAN_PORT_TYPE_C;
     *  low_level->ingress_filter  = TRUE;
     *  if (high_level->tag_native_vlan) {                   // Default is FALSE
     *       low_level->tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
     *       low_level->frame_type  = VTSS_VLAN_FRAME_TAGGED;
     *   } else {
     *       low_level->tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_THIS;
     *       low_level->frame_type  = VTSS_VLAN_FRAME_ALL;
     *   }
     */
    VLAN_PORT_MODE_TRUNK,

    /**
     * Hybrid port.
     * A hybrid port is completely end-user-controllable w.r.t.:
     *  - VLAN awareness and port type,
     *  - PVID,
     *  - egress tagging,
     *  - ingress filtering
     * A hybrid port's port configuration is held in vlan_port_composite_conf_t::hyb_port_conf.
     * A hybrid port automatically becomes a member of all VLANs that
     * are set in the array defining allowed VIDs for port mode = VLAN_PORT_MODE_HYBRID
     * (see vlan_mgmt_port_composite_allowed_vids_get()).
     *
     * Default low-level properties for a hybrid port are:
     *  low_level->pvid           = VLAN_ID_DEFAULT;
     *  low_level->untagged_vid   = VLAN_ID_DEFAULT;
     *  low_level->frame_type     = VTSS_VLAN_FRAME_ALL;
     *  low_level->ingress_filter = FALSE;
     *  low_level->tx_tag_type    = VLAN_TX_TAG_TYPE_UNTAG_THIS;
     *  low_level->port_type      = VLAN_PORT_TYPE_C;
     */
    VLAN_PORT_MODE_HYBRID,

    VLAN_PORT_MODE_CNT /**< Must come last. Don't use. */
} vlan_port_mode_t;

/**
 * Structure to hold port configuration for
 * the composite Access, Trunk, Hybrid and "none" modes.
 */
typedef struct {
    /**
     * Port mode as defined by vlan_port_mode_t above.
     */
    vlan_port_mode_t mode;

    /**
     * When #mode == VLAN_PORT_MODE_ACCESS,
     * this is the PVID the port will be assigned.
     */
    vtss_vid_t access_vid;

    /**
     * When #mode == VLAN_PORT_MODE_TRUNK,
     * this is the PVID the port will be assigned.
     */
    vtss_vid_t native_vid;

    /**
     * When #mode == VLAN_PORT_MODE_TRUNK,
     * this controls whether PVID (i.e. #native_vid) will be
     * tagged on egress or not.
     */
    BOOL tag_native_vlan;

    /**
     * When #mode == VLAN_PORT_MODE_HYBRID, this is the
     * port configuration the port will get.
     */
    vlan_port_conf_t hyb_port_conf;

} vlan_port_composite_conf_t;

/**
 * Get current composite port configuration.
 *
 * Note that VLAN_USER_STATIC is implicit, so other VLAN users
 * are not allowed to call this function.
 *
 * \param isid [IN]  Legal ISID of switch to get port mode for.
 * \param port [IN]  Port number to get port mode for. Stack ports not allowed.
 * \param mode [OUT] Pointer receiving current composite port configuration.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code.
 */
vtss_rc vlan_mgmt_port_composite_conf_get(vtss_isid_t isid, vtss_port_no_t port, vlan_port_composite_conf_t *conf);

/**
 * Set current composite port configuration.
 *
 * Note that VLAN_USER_STATIC is implicit, so other VLAN users
 * are not allowed to call this function.
 *
 * There is no guarantee that the configuration will take effect, because
 * it could happen that a higher prioritized user has already configured
 * the features that are attempted configured now.
 * Please check vlan_mgmt_conflicts_get() if this function returns
 * VTSS_RC_OK and you don't see your changes take effect.
 *
 * \param isid [IN]  Legal ISID of switch to set port mode for.
 * \param port [IN]  Port number to set port mode for. Stack ports not allowed.
 * \param mode [OUT] Pointer to composite port configuration.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code.
 */
vtss_rc vlan_mgmt_port_composite_conf_set(vtss_isid_t isid, vtss_port_no_t port, vlan_port_composite_conf_t *conf);

/**
 * Get a default composite port configuration.
 *
 * \param conf [OUT] Pointer receiving default composite port configuration.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code. Errors can only
 *         occur if #conf == NULL.
 **/
vtss_rc vlan_mgmt_port_composite_conf_default_get(vlan_port_composite_conf_t *conf);

/**
 * Number of bytes needed to represent all valid VIDs
 * ([VLAN_ID_MIN; VLAN_ID_MAX]) as a bitmask.
 *
 * Use VTSS_BF_GET() and VTSS_BF_SET() macros
 * to manipulate and obtain its entries.
 */
#define VLAN_BITMASK_LEN_BYTES VTSS_BF_SIZE(VLAN_ID_MAX + 1)

/**
 * Get current allowed VLAN IDs for a given port in a given port mode.
 *
 * Note that VLAN_USER_STATIC is implicit, so other VLAN users
 * are not allowed to call this function.
 *
 * The #vid_mask must be preallocated to a size of VLAN_BITMASK_LEN_BYTES
 * by the caller. It will receive the current VLAN IDs that are allowed
 * for port mode == #port_mode, which can be either VLAN_PORT_MODE_TRUNK or
 * VLAN_PORT_MODE_HYBRID.
 *
 * The returned #vid_mask is a bit-mask that indicates the VLANs
 * that a port will automatically become a member of when the port
 * in mode given my #port_mode.
 *
 * Use VTSS_BF_GET(#vid_mask, vid) on the returned bitmask array
 * afterwards to figure out which VIDs are enabled and which are disabled.
 *
 * \param isid     [IN]  Legal ISID of switch to get allowed VIDs for.
 * \param port     [IN]  Port number to get allowed VIDs for. Stack ports not allowed.
 * \param mode     [IN]  Either VLAN_PORT_MODE_TRUNK or VLAN_PORT_MODE_HYBRID.
 * \param vid_mask [OUT] Pointer to an array of VLAN_BITMASK_LEN_BYTES bytes receiving the allowed VLAN ID bitmask.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code.
 */
vtss_rc vlan_mgmt_port_composite_allowed_vids_get(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_mode_t port_mode, u8 vid_mask[VLAN_BITMASK_LEN_BYTES]);

/**
 * Set allowed VLAN IDs for a given port in a given port mode.
 *
 * Note that VLAN_USER_STATIC is implicit, so other VLAN users
 * are not allowed to call this function.
 *
 * The #vid_mask must point to an area of VLAN_BITMASK_LEN_BYTES bytes
 * and filled in by the caller. It is a bitmask indexed by VLAN ID, and
 * indicates the VLAN IDs that are allowed for port mode == #port_mode,
 * which can be either VLAN_PORT_MODE_TRUNK or VLAN_PORT_MODE_HYBRID.
 *
 * The #vid_mask is a bit-mask that indicates the VLANs
 * that a port will automatically become a member of, provided the port is in #port_mode.
 *
 * Use VTSS_BF_SET(#vid_mask, vid, 0|1) on the bitmask array to indicate
 * whether a given VID is allowed (1) or disallowed (0).
 *
 * \param isid     [IN]  Legal ISID of switch to set allowed VIDs for.
 * \param port     [IN]  Port number to set allowed VIDs for. Stack ports not allowed.
 * \param mode     [IN]  Either VLAN_PORT_MODE_TRUNK or VLAN_PORT_MODE_HYBRID.
 * \param vid_mask [OUT] Pointer to an array of VLAN_BITMASK_LEN_BYTES bytes containing the allowed VLAN ID bitmask.
 *
 * \return VTSS_RC_OK on success, anything else on error. Use error_txt(return code)
 *         to get a textual representation of the error code.
 */
vtss_rc vlan_mgmt_port_composite_allowed_vids_set(vtss_isid_t isid, vtss_port_no_t port_no, vlan_port_mode_t port_mode, u8 vid_mask[VLAN_BITMASK_LEN_BYTES]);

/**
 * Get a default allowed VLAN ID bitmask.
 *
 * \param port_mode [IN]  Mode for which to get defaults. Must be either VLAN_PORT_MODE_TRUNK or VLAN_PORT_MODE_HYBRID.
 * \param vid_mask  [OUT] Pointer receiving default VLAN ID bitmask. Must be VLAN_BITMASK_LEN_BYTES bytes long.
 *
 * \return VTSS_RC_OK unless one of the input parameters is erroneous. Use error_txt(return code)
 *         to get a textual representation of a possible error code.
 */
vtss_rc vlan_mgmt_port_composite_allowed_vids_default_get(vlan_port_mode_t port_mode, u8 vid_mask[VLAN_BITMASK_LEN_BYTES]);

/******************************************************************************/
//
// MEMBERSHIP FUNCTIONS
//
/******************************************************************************/

/**
 * Structure required in all VLAN membership manipulation functions.
 */
typedef struct {
    /**
     * VLAN ID.
     *
     * In vlan_mgmt_vlan_get(), this is an [OUT] parameter ranging
     * from [0; VLAN_ID_MAX]. 0 (VTSS_VID_NULL) indicates that the
     * requested VLAN doesn't exist.
     *
     * In vlan_mgmt_vlan_add(), this is an [IN] parameter ranging
     * from [VLAN_ID_MIN; VLAN_ID_MAX].
     */
    vtss_vid_t vid;

    /**
     * Array of ports memberships.
     *
     * In vlan_mgmt_vlan_get(), this is an [OUT] parameter.
     *
     * In vlan_mgmt_vlan_add(), this is an [IN] parameter.
     *
     * If an entry is TRUE, the corresponding port is a
     * member of the VLAN, if FALSE, it's not.
     *
     * If getting/setting forbidden VLANs, a TRUE entry
     * indicates that the #vid is not allowed on the
     * indexed port.
     */
    BOOL ports[VTSS_PORT_ARRAY_SIZE];
} vlan_mgmt_entry_t;

/**
 * Get VLAN membership.
 *
 * Use this function go get port memberships for either a
 * specific VID or the next defined VID.
 * The function can also be used to simply figure out
 * whether a given VLAN ID is defined or not.
 *
 * #user must be a VLAN user in range [VLAN_USER_STATIC; VLAN_USER_ALL].
 *
 * What the #user has configured is not necessarily what is in hardware,
 * because of forbidden VLANs, which override everything.
 *
 * If invoked with VLAN_USER_ALL, the returned value is the combined
 * membership of all VLAN users as programmed to hardware.
 *
 * #next == FALSE:
 *   Get specific VID membership.
 *
 *   #vid must be a legal VID in range [VLAN_ID_MIN; VLAN_ID_MAX].
 *
 *   If #isid is VTSS_ISID_LOCAL, the function will read directly
 *   from hardware. This is the only value of #isid that is allowed
 *   on a slave.
 *
 *   If #isid is a legal ISID ([VTSS_ISID_START; VTSS_ISID_END[),
 *   and the #vid exists on that switch for #user, this function will
 *   return VTSS_RC_OK and #membership::ports will contain the
 *   membership information and #membership::vid will be set to #vid.
 *
 *   If #isid is a legal ISID but #vid does not exist for #user on this
 *   switch, the function returns VLAN_ERROR_ENTRY_NOT_FOUND, and
 *   #membership::vid will be VTSS_VID_NULL.
 *
 *   If #isid is VTSS_ISID_GLOBAL and at least one switch has
 *   #vid defined for #user, this function will return VTSS_RC_OK
 *   and #membership::vid will contain #vid. The #membership::ports
 *   will NOT be valid. This can be used to figure out whether
 *   a VID is defined on any switch, but it can't be used to
 *   get membership.
 *
 *   If #isid is VTSS_ISID_GLOBAL but #vid does not exist for #user on any
 *   configurable switch in the stack, the function returns
 *   VLAN_ERROR_ENTRY_NOT_FOUND, and #membership::vid will be VTSS_VID_NULL.
 *
 * #next == TRUE:
 *   Get the next defined VID greater than #vid for #user (so
 *   it doesn't make sence to invoke the function with #next = TRUE
 *   and #vid = VLAN_ID_MAX).
 *
 *   If #isid is VTSS_ISID_LOCAL, the function will read directly
 *   from hardware. This is the only value of #isid that is allowed
 *   on a slave.
 *
 *   If #isid is a legal ISID ([VTSS_ISID_START; VTSS_ISID_END[),
 *   only this specific switch is searched for the next, closest
 *   VID > #vid installed by #user. If such a VID is found, this
 *   function returns VTSS_RC_OK and sets both #membership::ports
 *   and #membership::vid.
 *   If no such VID is found, this function returns
 *   VLAN_ERROR_ENTRY_NOT_FOUND and sets #membership::vid to VTSS_VID_NULL.
 *
 *   If #isid is VTSS_ISID_GLOBAL, all switches are searched for
 *   the next, closest VID > #vid installed by #user. If such a VID
 *   is found, this function returns VTSS_RC_OK and sets #membership::vid,
 *   while leaving #membership:::ports undefined.
 *   If no such VID is found, this function returns
 *   VLAN_ERROR_ENTRY_NOT_FOUND and sets #membership::vid to VTSS_VID_NULL.
 *
 * \param isid       [IN]  VTSS_ISID_LOCAL, VTSS_ISID_GLOBAL, or legal ISID (see above).
 * \param vid        [IN]  VID to get (#next == FALSE) or to start from (#next == TRUE).
 * \param membership [OUT] Result of doing the get (see above).
 * \param next       [IN]  If FALSE, get #vid, only. If TRUE, search sequentially from [#vid + 1; VLAN_ID_MAX].
 * \param user       [IN]  The VLAN user to lookup. VLAN_USER_ALL holds the combined state and will be available if at least one other VLAN user has installed a VID.
 *
 * \return VTSS_RC_OK if a VID was found for #user. #membership contains valid information (hereunder the found #vid).
 *         Returns VLAN_ERROR_ENTRY_NOT_FOUND if no entries were found. #membership::vid is VTSS_VID_NULL.
 *         Returns anything else on input parameter errors.
 */
vtss_rc vlan_mgmt_vlan_get(vtss_isid_t isid, vtss_vid_t vid, vlan_mgmt_entry_t *membership, BOOL next, vlan_user_t user);

/**
 * Set membership of a range of ports to a specific VID.
 *
 * This function sets the membership for #user on #membership::vid,
 * i.e. it overwrites any previous configuration that #user may have
 * had on that VID.
 *
 * #user must be in range [VLAN_USER_STATIC; VLAN_USER_ALL[.
 *
 * If #user == VLAN_USER_STATIC:
 *   The values of #isid and #membership don't matter, since the
 *   function will automatically add membership for ports on all
 *   switches that are going into this VID.
 *
 * If #user == VLAN_USER_FORBIDDEN:
 *   Ports set in #membership will override all other users'
 *   membership by disallowing these ports on a given VID,
 *   so that the final membership value written to H/W always
 *   will have zeroes for forbidden ports.
 *
 * If #user != VLAN_USER_STATIC && #user != VLAN_USER_FORBIDDEN:
 *   #isid may be legal or VTSS_ISID_GLOBAL. Ports will become
 *   members of whatever is specified in #membership.
 *   If the VLAN doesn't exist prior to the call, it will be
 *   added to all switches in the stack. This means that on
 *   some switches it will be added with no ports as members.
 *   In order to be backward compatible and ease the module
 *   implementation, a VLAN will automatically get deleted
 *   when all switches have a zero memberset. This means that
 *   you may call this function with #membership->ports set to
 *   all-zeros.
 *
 * VLAN_USER_STATIC and VLAN_USER_FORBIDDEN should only be used
 * from administrative interfaces, i.e. Web, CLI, SNMP.
 *
 * The final port membership written to hardware is a bitwise OR
 * of all user modules' requests, with the exception that forbidden
 * ports are cleared.
 *
 * \param isid       [IN] Legal ISID or VTSS_ISID_GLOBAL.
 * \param membership [IN] The membership.
 * \param user       [IN] The VLAN user ([VLAN_USER_STATIC; VLAN_USER_ALL[) to change membership for.
 *
 * \return VTSS_RC_OK in most cases. If something different from VTSS_RC_OK is returned,
 *         it's because of parameters passed to the function or because the switch is
 *         currently not master.
 */
vtss_rc vlan_mgmt_vlan_add(vtss_isid_t isid, vlan_mgmt_entry_t *membership, vlan_user_t user);

/**
 * Delete VLAN membership for #user on #vid.
 *
 * This function backs out #user's contribution to VLAN membership for
 * VLAN ID #vid on the switch given by #isid.
 *
 * #isid must be a legal ISID or VTSS_ISID_GLOBAL. If VTSS_ISID_GLOBAL,
 * membership is removed on all switches for #user.
 *
 * #user may be any VLAN user, including VLAN_USER_ALL, i.e. in
 * range [VLAN_USER_STATIC; VLAN_USER_ALL].
 *
 * If invoked with VLAN_USER_ALL, all users membership will be
 * deleted, except for the VLAN_FORBIDDEN_USER.
 *
 * To change the forbidden user's "anti-membership", this function
 * must be invoked directly with #user == VLAN_FORBIDDEN_USER.
 *
 * The function returns VTSS_RC_OK even if membership for a given
 * user doesn't exist (that is, even if the function has done nothing).
 *
 * VLAN_USER_STATIC and VLAN_USER_FORBIDDEN should only be used
 * from administrative interfaces, i.e. Web, CLI, SNMP.
 *
 * \param isid [IN] Legal ISID or VTSS_ISID_GLOBAL.
 * \param vid  [IN] The VID to delete membership for ([VLAN_ID_MIN; VLAN_ID_MAX]).
 * \param user [IN] The user ([VLAN_USER_STATIC; VLAN_USER_ALL]) to unregister membership for.
 *
 * \return VTSS_RC_OK in most cases. If something different from VTSS_RC_OK is returned,
 *         it's because of parameters passed to the function or because the switch is
 *         currently not master.
 */
vtss_rc vlan_mgmt_vlan_del(vtss_isid_t isid, vtss_vid_t vid, vlan_user_t user);

/**
 * Get membership info for a given port and VLAN user.
 *
 * This function looks up #user's contribution to the
 * resulting VLAN mask on a given #isid:#port.
 * If a bit is set, the #user has added membership
 * for the corresponding VID on that #isid:#port.
 *
 * Use VTSS_BF_GET(vid_mask, vid) to traverse #vid_mask.
 *
 * Call with #user set to VLAN_USER_FORBIDDEN to get the
 * forbidden VLANs (a '1' in bit-positions that are forbidden).
 *
 * Call with #user set to VLAN_USER_ALL to get the memberships
 * as written to hardware.
 *
 * \param isid     [IN]  Legal ISID.
 * \param port     [IN]  Valid non-stack port.
 * \param user     [IN]  Must be in range [VLAN_USER_STATIC; VLAN_USER_ALL].
 * \param vid_mask [OUT] Pointer to an array of VLAN_BITMASK_LEN_BYTES receiving the resulting per-VID memberships.
 *
 * \return VTSS_RC_OK on success. Anything else means that the caller
 *         has passed erroneous parameters to the function.
 */
vtss_rc vlan_mgmt_membership_per_port_get(vtss_isid_t isid, vtss_port_no_t port, vlan_user_t user, u8 vid_mask[VLAN_BITMASK_LEN_BYTES]);

/**
 * A more complex way of looking at VLAN membership registrations.
 */
typedef enum {
    VLAN_REGISTRATION_TYPE_NORMAL = 0, /**< Not member */
    VLAN_REGISTRATION_TYPE_FIXED,      /**< Member     */
    VLAN_REGISTRATION_TYPE_FORBIDDEN,  /**< Forbidden  */
} vlan_registration_type_t;

/**
 * Get complex registration for a given port and VLAN user.
 *
 * This function looks up VLAN_USER_STATIC and VLAN_USER_FORBIDDEN's
 * contribution to the resulting VLAN mask and enumerates per
 * VLAN ID the high-level registration type like this:
 *
 * If a VID is forbidden, set #reg[vid] to VLAN_REGISTRATION_TYPE_FORBIDDEN.
 * Otherwise if #port is member of VID, set #reg[vid] to VLAN_REGISTRATION_TYPE_FIXED,
 * Otherwise set #reg[vid] to VLAN_REGISTRATION_TYPE_NORMAL.
 *
 * Notice that #reg possibly requires either static or dynamic allocation (that is
 * normally you will not want to allocate it on the stack).
 *
 * \param isid [IN]  Legal ISID.
 * \param port [IN]  Valid non-stack port.
 * \param reg  [OUT] Pointer to an array of VLAN_ID_MAX +1 entries receiving the registration type.
 *
 * \return VTSS_RC_OK on success. Anything else means that the caller
 *         has passed erroneous parameters to the function.
 */
vtss_rc vlan_mgmt_registration_per_port_get(vtss_isid_t isid, vtss_port_no_t port, vlan_registration_type_t reg[VLAN_ID_MAX + 1]);

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/**
 * Get a VLAN name given a VLAN ID.
 *
 * \param vid        [IN]  VID to look up.
 * \param name       [OUT] Pointer to string receiving the resulting name.
 * \param is_default [OUT] Pointer to a BOOL that gets set to FALSE if this is the default VLAN name for this VID, FALSE otherwise. NULL is an OK value to pass.
 *
 * \return VTSS_RC_OK if entry was found and #name filled in.
 *         Anything else means input parameter error.
 */
vtss_rc vlan_mgmt_name_get(vtss_vid_t vid, char name[VLAN_NAME_MAX_LEN], BOOL *is_default);
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/**
 * Set a VLAN name for a VLAN ID.
 *
 * #name must be an alphanumeric string of up to VLAN_NAME_MAX_LEN - 1
 * characters, starting with a non-digit.
 *
 * Setting it to the empty string corresponds to defaulting the VLAN name.
 * Setting it to "VLANxxxx", where xxxx is four decimal digits (with leading
 * zeroes) and these digits translates to a valid VLAN ID and that VLAN ID
 * is not equal to #vid, an error is returned. If it *is* equal to #vid, it
 * corresponds to defaulting it.
 *
 * The word VLAN_NAME_DEFAULT is reserved for VLAN_ID_DEFAULT. The name of
 * VLAN_ID_DEFAULT cannot be changed.
 *
 * Accepted characters are in the range [33; 126].
 *
 * \param vid  [IN]  VID to change the name of. Allowed range is [VLAN_ID_MIN; VLAN_ID_MAX] except VLAN_ID_DEFAULT, unless #name is VLAN_NAME_DEFAULT.
 * \param name [OUT] New name of VLAN.
 *
 * \return VTSS_RC_OK on success.
 *         VLAN_ERROR_NAME_RESERVED if a reserved VLAN name (VLAN_NAME_DEFAULT or "VLANxxxx") is used for a VLAN ID that is not supposed to have this name.
 *         VLAN_ERROR_NAME_ALREADY_EXISTS if a VLAN with that name is already configured.
 *         Anything else means input parameter error.
 */
vtss_rc vlan_mgmt_name_set(vtss_vid_t vid, const char name[VLAN_NAME_MAX_LEN]);
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

#if defined(VTSS_SW_OPTION_VLAN_NAMING)
/**
 * Get a VLAN ID given a VLAN name.
 *
 * \param name [IN]  Name to look up.
 * \param vid  [OUT] Pointer to resulting VLAN ID.
 *
 * \return VTSS_RC_OK if entry was found and #name filled in.
 *         VLAN_ERROR_NAME_DOES_NOT_EXIST if no such name matched. #vid will be VTSS_VID_NULL in that case.
 *         Anything else means input parameter error.
 */
vtss_rc vlan_mgmt_name_to_vid(const char name[VLAN_NAME_MAX_LEN], vtss_vid_t *vid);
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */

/******************************************************************************/
//
// UTILITY FUNCTIONS
//
/******************************************************************************/

/**
 * Utility function that determines whether a given VID gets tagged or not on egress.
 *
 * \param p   [IN] Pointer to a port configuration previously obtained with a call to vlan_mgmt_port_conf_get()
 * \param vid [IN] VID to check.
 *
 * \return TRUE if #vid gets tagged on egress, FALSE otherwise.
 */
BOOL vlan_mgmt_vid_gets_tagged(vlan_port_conf_t *p, vtss_vid_t vid);

/**
 * Maximum buffer size needed in order to
 * convert a VLAN bit mask to a textual representation.
 *
 * Worst case is if every other VLAN is not defined, so
 * that the resulting string is something along these lines:
 * "1,3,5,...,4093,4095".
 *
 * Such a string is at most:
 *   [   1;    9]:    5 * (1 digit  + 1 comma) =   10 bytes
 *   [  10;   99]:   45 * (2 digits + 1 comma) =  135 bytes
 *   [ 100;  999]:  450 * (3 digits + 1 comma) = 1800 bytes
 *   [1000; 4095]: 1548 * (4 digits + 1 comma) = 7740 bytes
 * --------------------------------------------------------
 * Total                                         9685 bytes
 *
 * One could think that room for a terminating '\0' is
 * needed, but actually, it is not, because a comma is not
 * required after the last "4095" string, but such a comma
 * was already included in the compuatations above.
 */
#define VLAN_VID_LIST_AS_STRING_LEN_BYTES 9685

/**
 * Utility function that converts a VLAN bitmask to a textual representation.
 *
 * #bitmask is a mask of VLAN_BITMASK_LEN_BYTES bytes, and
 * #txt is a string of at least VLAN_VID_LIST_AS_STRING_LEN_BYTES
 * bytes (which should normally be VTSS_MALLOC()ed).
 *
 * The resulting #txt could contain e.g. "1-4095" or "1,17-23,45".
 *
 * \param bitmask [IN]  Pointer to the binary bitmask.
 * \param txt     [OUT] Pointer to resulting text string.
 *
 * \return Pointer to txt, so that this can be used directly in a printf()-like function.
 */
char *vlan_mgmt_vid_bitmask_to_txt(u8 bitmask[VLAN_BITMASK_LEN_BYTES], char *txt);

/**
 * Utility function that checks if two VLAN bitmasks are identical.
 *
 * Both #bitmask1 and #bitmask2 are arrays of VLAN_BITMASK_LEN_BYTES bytes.
 *
 * The reason that you should call this function rather than
 * memcmp() is that not all bits of #bitmask1/#bitmask2 need to be
 * valid, and may therefore have an arbitrary value.
 *
 * \param bitmask1 [IN] Pointer to the first binary bitmask.
 * \param bitmask2 [IN] Pointer to the second binary bitmask.
 *
 * \return TRUE if #bitmask1 and #bitmask2 are VLAN-wise identical,
 *         FALSE otherwise.
 */
BOOL vlan_mgmt_bitmasks_identical(u8 *bitmask1, u8 *bitmask2);

/**
 * Function for converting a VLAN error
 * (see VLAN_ERROR_xxx above) to a textual string.
 * Only errors in the VLAN module's range can
 * be converted.
 *
 * \param rc [IN] Binary form of error
 *
 * \return Static string containing textual representation of #rc.
 */
const char *vlan_error_txt(vtss_rc rc);

/**
 * Get a textual representation of a vlan_user_t.
 *
 * \param user [IN] Binary form of VLAN user.
 *
 * \return Static string containing textual representation of #user.
 */
const char *vlan_mgmt_user_to_txt(vlan_user_t user);

/**
 * Get a textual representation of a vlan_port_type_t.
 *
 * \param port_type [IN] Binary form of port type.
 *
 * \return Static string containing textual representation of #port_type
 */
const char *vlan_mgmt_port_type_to_txt(vlan_port_type_t port_type);

/**
 * Get a textual representation of a vtss_vlan_frame_t.
 *
 * \param frame_type [IN] Binary form of frame type.
 *
 * \return Static string containing textual representation of #frame_type
 */
const char *vlan_mgmt_frame_type_to_txt(vtss_vlan_frame_t frame_type);

/**
 * Get a textual representation of a vlan_tx_tag_type_t.
 *
 * \param tx_tag_type     [IN] Binary form of Tx tag type.
 * \param can_be_any_uvid [IN] Used only to show non-static overrides of Tx tag type, which can tag or untag any particular VID, not just PVID.
 *
 * \return Static string containing textual representation of #tx_tag_type
 */
const char *vlan_mgmt_tx_tag_type_to_txt(vlan_tx_tag_type_t tx_tag_type, BOOL can_be_any_uvid);

/**
 * Get to know whether #user is a valid caller of vlan_mgmt_port_conf_set()
 *
 * \param user [IN] VLAN user to ask for.
 *
 * \return TRUE if #user really may call vlan_mgmt_port_conf_set(), FALSE otherwise.
 */
BOOL vlan_mgmt_user_is_port_conf_changer(vlan_user_t user);

/**
 * Get to know whether #user is a valid caller of vlan_mgmt_vlan_add().
 *
 * \param user [IN] VLAN user to ask for.
 *
 * \return TRUE if #user really may call vlan_mgmt_vlan_add(), FALSE otherwise.
 */
BOOL vlan_mgmt_user_is_membership_changer(vlan_user_t user);

/******************************************************************************/
//
// CONFIGURATION CHANGE CALLBACKS
//
/******************************************************************************/

/**
 * Signature of function to call back upon port configuration changes.
 *
 * Below, [IN] is seen from the called back function's p.o.v.
 * If called back on the master, #isid is a legal ISID ([VTSS_ISID_START; VTSS_ISID_END[),
 * whereas it is VTSS_ISID_LOCAL if called back on the local switch (see
 * description under vlan_port_conf_change_register() for details).
 *
 * The callback function may call into the VLAN module again if it likes, without
 * risking deadlocks if called back on the local switch. If called back on
 * the master switch, it must not.
 *
 * The callback function is only invoked if #isid exists in the stack.
 *
 * \param isid     [IN] Switch ID on which a configuration change is about to occur (on master) or occurred (on local switch, in which case it is VTSS_ISID_LOCAL).
 * \param port_no  [IN] Port number on which a configuration change is about to occur (on master) or occurred (on local switch).
 * \param new_conf [IN] Pointer to the new port configuration.
 *
 * \return Nothing.
 */
typedef void (*vlan_port_conf_change_callback_t)(vtss_isid_t isid, vtss_port_no_t port_no, const vlan_port_conf_t *new_conf);

/**
 * Register for VLAN port configuration changes.
 *
 * The caller may choose between getting called back on the local switch after
 * a change has just happened or on the master switch. If called back on the
 * master switch, the change may or may not already have happened on the switch
 * in question.
 *
 * Currently, there is no support for unregistering once registered.
 *
 * \param cb           [IN] Pointer to a function to call back when port configuration changes.
 * \param cb_on_master [IN] If TRUE, only call #cb on the master, otherwise only call #cb on the local switch on which change has just occurred.
 * \param modid        [IN] Module ID of registrant. Only used for debug purposes.
 *
 * \return Nothing.
 */
void vlan_port_conf_change_register(vtss_module_id_t modid, vlan_port_conf_change_callback_t cb, BOOL cb_on_master);

/**
 * Structure used to pass bit-arrays of ports
 * back and forth between VLAN module and users of it.
 * Use VTSS_BF_GET()/VTSS_BF_SET() to manipulate
 */
typedef struct {
    /**
     * Port bit-array indexed by vtss_port_no_t.
     * Use VTSS_BF_GET()/VTSS_BF_SET() to manipulate
     * individual bits.
     */
    u8 ports[VTSS_PORT_BF_SIZE];
} vlan_ports_t;

/**
 * Structure used in membership change callbacks.
 *
 * Use VTSS_BF_GET() to access bits in the bit arrays.
 */
typedef struct {
    /**
     * This one indicates whether VLAN_USER_STATIC has
     * added member-ports to a given VLAN.
     * It is not necessarily possible to
     * derive from #static_ports whether the static user
     * has added a VLAN, because it may contain no members.
     */
    BOOL static_vlan_exists;

    /**
     * Contains a bit per port that tells whether a change has occurred in
     * static user's or forbidden user's VLAN membership on that port (TRUE if so).
     */
    vlan_ports_t changed_ports;

    /**
     * Current (new) static port membership.
     */
    vlan_ports_t static_ports;

    /**
     * Current (new) forbidden port "membership".
     */
    vlan_ports_t forbidden_ports;
} vlan_membership_change_t;

/**
 * Signature of callback function invoked when VLAN membership changes on a given switch.
 *
 * [IN] is seen from the callback function's perspective.
 *
 * The callback function is only invoked for administrative changes, that is, when
 * the underlying VLAN user is VLAN_USER_STATIC or VLAN_USER_FORBIDDEN, so any changes made
 * by other volatile VLAN users (e.g. GVRP) are not reported anywhere.
 *
 * The callback function is only invoked on the master.
 * The callback function may call into the VLAN module again if it likes, without risking deadlocks.
 * The callback function is only invoked if #isid exists in the stack.
 *
 * \param isid    [IN] A legal ISID identifying the switch on which the change is about to occur.
 * \param vid     [IN] A legal VLAN ID for which membership is about to occur.
 * \param changes [IN] The changes that have occurred. See structure for more details.
 *
 * \return Nothing.
 */
typedef void (*vlan_membership_change_callback_t)(vtss_isid_t isid, vtss_vid_t vid, vlan_membership_change_t *changes);

/**
 * Register for VLAN membership changes.
 *
 * The callback function will only be invoked on the master.
 * There is no guarantee that the changes have propagated all the way
 * to hardware when called back.
 *
 * \param cb    [IN] Function to call back upon VLAN membership changes.
 * \param modid [IN] Module ID of registrant. Only used for debug purposes.
 *
 * \return Nothing.
 */
void vlan_membership_change_register(vtss_module_id_t modid, vlan_membership_change_callback_t cb);

/**
 * Signature of callback function invoked when VLAN membership changes on a given switch.
 *
 * The function may be invoked immediately when the changes occur, or it may be invoked
 * after a while. There is no indication of which changes have occurred.
 *
 * \return Nothing.
 */
typedef void (*vlan_membership_bulk_change_callback_t)(void);

/**
 * Register for VLAN membership changes.
 *
 * The callback function will only be invoked on the master. It may take
 * a while for the callback function to be invoked. Only after a given management
 * interface is done updating VLAN memberships will the callback be invoked.
 *
 * \param cb    [IN] Function to call back upon VLAN membership changes.
 * \param modid [IN] Module ID of registrant. Only used for debug purposes.
 *
 * \return Nothing.
 */
void vlan_membership_bulk_change_register(vtss_module_id_t modid, vlan_membership_bulk_change_callback_t cb);

/* Start/Stop bulk updates.
 * Every call to vlan_bulk_update_begin() must be balanced with a call to vlan_bulk_update_end().
 * Useful e.g. when a large number of VLANs are changed
 */
void vlan_bulk_update_begin(void);
void vlan_bulk_update_end(void);
u32  vlan_bulk_update_ref_cnt_get(void);

/**
 * Signature of callback function invoked when S-custom tag EtherType changes.
 *
 * [IN] is seen from the callback function's perspective.
 *
 * The callback function will only be invoked on the master.
 *
 * \param tpid [IN] The new custom S-tag EtherType set by management.
 *
 * \return Nothing.
 */
typedef void (*vlan_s_custom_etype_change_callback_t)(vtss_etype_t tpid);

/**
 * Register for S-custom EtherType changes.
 *
 * \param cb    [IN] Function to call back upon S-custom tag EtherType changes.
 * \param modid [IN] Module ID of registrant. Only used for debug purposes.
 *
 * \return Nothing.
 */
void vlan_s_custom_etype_change_register(vtss_module_id_t modid, vlan_s_custom_etype_change_callback_t cb);

/******************************************************************************/
//
// OTHER NON-MANAGEMENT FUNCTIONS
//
/******************************************************************************/

/**
 * Module initialization function.
 *
 * \param data [IN] Pointer to state
 *
 * \return VTSS_RC_OK unless something serious is wrong.
 */
vtss_rc vlan_init(vtss_init_data_t *data);

#endif /* _VLAN_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
