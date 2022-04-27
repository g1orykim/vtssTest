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

#ifndef _PSEC_API_H_
#define _PSEC_API_H_

/**
 * \file psec_api.h
 * \brief This file defines the API for the Port Security module
 */

#include "port_api.h" /* For VTSS_PORTS                    */
#include "vtss_api.h" /* For vtss_rc, vtss_vid_mac_t, etc. */

//
// Compile time options constants
//

/**
 * \brief Defines the maximum number of MAC addresses that the port security
 *        module can manage.
 *
 * It doesn't make sense to set this to a value greater than the size of
 * the MAC table on a single switch. The pool of entries is shared amongst
 * all ports.
 *
 * The limit module will allow at most PSEC_MAC_ADDR_ENTRY_CNT - 1 MAC addresses
 * on a specific port. The Nth entry is to be able to detect limit exceeded.
 */
#define PSEC_MAC_ADDR_ENTRY_CNT 1025

/**
 * \brief Defines the minimum and maximum aging time in seconds
 *
 * 0 is also valid (disable aging).
 */
#define PSEC_AGE_TIME_MIN 10
#define PSEC_AGE_TIME_MAX 10000000

/**
 * \brief Defines the minimum and maximum hold time in seconds
 *
 * 0 is invalid.
 */
#define PSEC_HOLD_TIME_MIN 10
#define PSEC_HOLD_TIME_MAX 10000000

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    PSEC_ERROR_INV_USER = MODULE_ERROR_START(VTSS_MODULE_ID_PSEC), /**< Invalid user parameter.                                 */
    PSEC_ERROR_MUST_BE_MASTER,                                     /**< Operation is only allowed on the master switch.         */
    PSEC_ERROR_INV_ISID,                                           /**< isid parameter is invalid.                              */
    PSEC_ERROR_INV_PORT,                                           /**< port parameter is invalid.                              */
    PSEC_ERROR_INV_AGING_PERIOD,                                   /**< The supplied aging period is invalid.                   */
    PSEC_ERROR_INV_HOLD_TIME,                                      /**< The supplied hold time is invalid.                      */
    PSEC_ERROR_MAC_VID_NOT_FOUND,                                  /**< The <MAC, VID> was not found on the port.               */
    PSEC_ERROR_MAC_VID_ALREADY_FOUND,                              /**< The <MAC, VID> was already found on any port.           */
    PSEC_ERROR_INV_USER_MODE,                                      /**< The user is not allowed to call this function.          */
    PSEC_ERROR_SWITCH_IS_DOWN,                                     /**< The selected switch doesn't exist.                      */
    PSEC_ERROR_LINK_IS_DOWN,                                       /**< The selected port's link is down.                       */
    PSEC_ERROR_OUT_OF_MAC_STATES,                                  /**< We're out of state machines.                            */
    PSEC_ERROR_PORT_IS_SHUT_DOWN,                                  /**< The port has been shut down by the PSEC Limit module.   */
    PSEC_ERROR_LIMIT_IS_REACHED,                                   /**< The port's limit is reached. Cannot add MAC address.    */
    PSEC_ERROR_NO_USERS_ENABLED,                                   /**< No users are enabled on the port.                       */
    PSEC_ERROR_STATE_CHG_DURING_CALLBACK,                          /**< A state change occurred during callback.                */

    /* Internal error codes */
    PSEC_ERROR_INV_PARAM,                                          /**< An invalid parameter other than the above was supplied. */
    PSEC_ERROR_INV_SHAPER_FILL_LEVEL,                              /**< Max. fill-level must be greater than the minimum.       */
    PSEC_ERROR_INV_SHAPER_RATE,                                    /**< The shaper rate must be greater than 0.                 */
    PSEC_ERROR_INTERNAL_ERROR,                                     /**< An internal error occurred.                             */
};

/**
 * \brief Users of this module.
 */
typedef enum {
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
    PSEC_USER_PSEC_LIMIT,     /**< Port Security Limit Control */
#endif
#ifdef VTSS_SW_OPTION_DOT1X
    PSEC_USER_DOT1X,          /**< The 802.1X module           */
#endif
#ifdef VTSS_SW_OPTION_DHCP_SNOOPING
    PSEC_USER_DHCP_SNOOPING,  /**< The DHCP Snooping module    */
#endif
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    PSEC_USER_VOICE_VLAN,     /**< The Voice VLAN module       */
#endif

    // This must come last
    PSEC_USER_CNT,
} psec_users_t;

/**
  * \brief Zombie Hold Time
  *
  * When a H/W or S/W add failure is detected, the
  * port in question is disabled for CPU copying
  * for this amount of time.
  */
#define PSEC_ZOMBIE_HOLD_TIME_SECS (300)

/**
  * \brief Port status
  *
  * This structure is used to hold the current port status,
  * as returned to the Web.
  *
  * CLI has other means of getting this - as it is closer tied
  * to this module.
  */
typedef struct {
    /**
      * TRUE if the limit is reached on the
      * port, FALSE otherwise.
      */
    BOOL limit_reached;

    /**
      * TRUE if the port is shut down, FALSE otherwise.
      */
    BOOL shutdown;
} psec_port_status_t;

/**
  * \brief Switch status
  *
  * This structure is used to hold the current switch status,
  * as returned to the Web.
  *
  * CLI has other means of getting this - as it is closer tied
  * to this module.
  */
typedef struct {
    /**
      * Array of port status. Index 0 is the first port.
      * Caller needs to filter out stack ports and unused ports.
      */
    psec_port_status_t port_status[VTSS_PORTS];
} psec_switch_status_t;

/**
  * \brief Result of calling back the On-MAC-Add callback
  *
  * The On-MAC-Add callback function is only called on ports
  * on which that module is enabled.
  *
  * Since more than one module may be enabled at the same time,
  * and since one module may e.g. allow a MAC address to forward,
  * while another wants it to be blocked, a hierarchy is implemented
  * as follows:
  * The higher the enumeration value, the higher priority.
  * For example, if one module says 'forward' and another says
  * 'block', then 'block' will win.
  */
typedef enum {
    /**
      * Allow this MAC address to forward in the MAC table right
      * from the beginning. The entry will be aged according to
      * the age rules specified in psec_mgmt_time_cfg_set().
      */
    PSEC_ADD_METHOD_FORWARD,

    /**
      * Add this MAC address to the MAC table right away, but
      * don't allow it to forward. The entry will be held there
      * according to the hold-times specified in psec_mgmt_time_cfg_set().
      */
    PSEC_ADD_METHOD_BLOCK,

    /**
      * Add this MAC address to the MAC table right away, but
      * don't allow it to forward. The entry will not be removed
      * from the MAC table, and will therefore not be subject
      * to the hold-time specified in psec_mgmt_time_cfg_set().
      * The reason for this is as follows:
      * If 802.1X is enabled, then the 802.1X will start by
      * requesting the entry to be added as KEEP_BLOCKED,
      * while authentication is ongoing. In this period it
      * shall not be 'aged' out. If authentication succeeds
      * then the entry will be moved to FORWARD state,
      * whereas if the authentication fails, it will be moved
      * to the BLOCK state, where the hold time specified
      * with psec_mgmt_time_cfg_set() takes effect.
      *
      * Modules supposed to use this:
      *  802.1X
      */
    PSEC_ADD_METHOD_KEEP_BLOCKED,

    /**
      * THIS MUST COME LAST! DON'T USE
      */
    PSEC_ADD_METHOD_CNT
} psec_add_method_t;

/**
  * \brief The reason for calling back the On-MAC-Del callback.
  */
typedef enum {
    PSEC_DEL_REASON_HW_ADD_FAILED,     /**< MAC Table add failed (number of locked entries for the hash in the MAC table was exceeded).              */
    PSEC_DEL_REASON_SW_ADD_FAILED,     /**< MAC Table add failed (MAC module S/W ran out of entries, or a reserved MAC address was attempted added). */
    PSEC_DEL_REASON_SWITCH_DOWN,       /**< The switch went down                                                                                     */
    PSEC_DEL_REASON_PORT_LINK_DOWN,    /**< The port link went down                                                                                  */
    PSEC_DEL_REASON_STATION_MOVED,     /**< The MAC was suddenly seen on another port                                                                */
    PSEC_DEL_REASON_AGED_OUT,          /**< The entry aged out                                                                                       */
    PSEC_DEL_REASON_HOLD_TIME_EXPIRED, /**< The hold time expired                                                                                    */
    PSEC_DEL_REASON_USER_DELETED,      /**< The entry was deleted by another module                                                                  */
    PSEC_DEL_REASON_PORT_SHUT_DOWN,    /**< The port was shut down by PSEC LIMIT module                                                              */
    PSEC_DEL_REASON_NO_MORE_USERS,     /**< The last user-module got disabled on this port (user modules will never see this reason).                */
} psec_del_reason_t;

/**
  * \brief The action involved with adding a MAC address.
  *
  * THIS IS ONLY TO BE USED BY THE PSEC LIMIT MODULE. OTHER
  * MODULES MUST NOT ALTER ITS VALUE.
  *
  * It is used in the On-MAC-Add callback function to signal
  * to this module whether the limit is reached on a port,
  * it should be shut-down, or we can keep on going.
  */
typedef enum {
    /**
      * Keep the port open for secure learning after this
      * MAC address is added.
      */
    PSEC_ADD_ACTION_NONE,

    /**
      * This one new MAC address caused the limit to
      * be reached, but there is no PORT LIMIT action
      * involved with this. Simply disable CPU copying
      * on this port (but keep it in secure learning mode).
      * Once a MAC address is deleted, CPU copying will be
      * re-enabled.
      */
    PSEC_ADD_ACTION_LIMIT_REACHED,

    /**
      * This one new MAC address caused the limit to
      * be exceeded, and a port-shut-down action was
      * attached with the port.
      * This module will remove all MAC addresses attached
      * to the port, and make sure that the port doesn't
      * learn new MAC addresses until it is administratively
      * reopened (psec_mgmt_reopen_port()).
      */
    PSEC_ADD_ACTION_SHUT_DOWN,
} psec_add_action_t;

/**
  * \brief The action involved with changing the enabledness on a port
  *
  * THIS IS ONLY TO BE USED BY THE PSEC LIMIT MODULE. OTHER
  * MODULES MUST NOT ALTER ITS VALUE.
  *
  * It is used in the On-Loop-Through callback function to signal
  * to this module whether the limit is reached on a port,
  * it should be shut-down, or we can keep on going.
  */
typedef enum {
    /**
      * Don't change the current limit or shut-down properties
      * on the port.
      */
    PSEC_LOOP_THROUGH_ACTION_NONE,

    /**
      * Clear the "limit-reached" of the port.
      * This may result in re-opening the port for
      * CPU-copying.
      */
    PSEC_LOOP_THROUGH_ACTION_CLEAR_LIMIT_REACHED,

    /**
      * Clear the "shut-down" and "limit-reached" properties
      * of the port. This may result in re-opening the port for
      * CPU-copying.
      */
    PSEC_LOOP_THROUGH_ACTION_CLEAR_SHUT_DOWN,

    /**
      * Set the "limit-reached" property of the port.
      */
    PSEC_LOOP_THROUGH_ACTION_SET_LIMIT_REACHED,
} psec_loop_through_action_t;

/**
  * \brief Set aging and hold times.
  *
  * If a MAC address is in forwarding mode (all enabled modules have
  * returned PSEC_ADD_METHOD_FORWARD), then the
  * \@aging_period_secs will be used.
  * Setting age_period_secs to 0 disables aging.
  * If more than one module have different aging requirements,
  * then the shortest aging time will be used.
  * If one module sets aging to X (X > 0) and another module
  * sets it to 0 (disable), then aging *will* be enabled.
  * Valid aging periods are in the range [10; 10,000,000] secs
  * and 0 (disable aging).
  *
  * If a MAC address is in blocking mode (at least one module
  * has set it to PSEC_ADD_METHOD_BLOCK, and none has set
  * it to PSEC_ADD_METHOD_KEEP_BLOCKED), then the
  * \@hold_time_secs will be used.
  * Valid hold-times are in the range [10; 10,000,000] secs.
  * 0 is invalid.
  *
  * \param user              [IN]: The user calling this function.
  * \param aging_period_secs [IN]: See description above.
  * \param hold_time_secs    [IN]: See description above.
  *
  * \return
  *   VTSS_OK if applying the new aging and hold time settings succeeded.\n
  *   PSEC_ERROR_MUST_BE_MASTER if the switch is not currently master.\n
  *   PSEC_ERROR_INV_USER if the \@user parameter is invalid.\n
  *   PSEC_ERROR_INV_AGING_PERIOD if the supplied aging period is out of bounds.\n
  *   PSEC_ERROR_INV_HOLD_TIME if the supplied hold time is out of bounds.
  */
vtss_rc psec_mgmt_time_cfg_set(psec_users_t user, u32 aging_period_secs, u32 hold_time_secs);

/**
  * \brief Signature of On-Loop-Through callback function
  *
  * - Reentrancy: The callback will not be called back while already
  *               being called back. The callback may not call other
  *               functions in this module while being called back.
  *               If your module has taken its own critical section
  *               before the call to psec_mgmt_port_cfg_set(), then that
  *               section will still be taken while the loop-through
  *               function is called.
  *
  * See psec_mgmt_port_cfg_set() for details.
  *
  * The called back function must return ASAP.
  * If your module is enabling Port Security on this port,
  * the \@keep, \@action, and return value are used as specificed
  * below.
  * If your module is disabling Port Security on this port,
  * the loop-through-callback will still be called as a courtesy
  * to your module, so that you can clean up your own state,
  * but the \@keep, \@action, and return value will not be used
  * for anything.
  *
  * [IN] and [OUT] is seen from the called back module's perspective.
  *
  * \param user_ctx                [IN]:  The context passed to psec_mgmt_port_cfg_set().
  * \param isid                    [IN]:  Switch ID for the MAC address being added.
  * \param port                    [IN]:  Port number for the MAC address being added.
  * \param vid_mac                 [IN]:  VLAN ID and MAC address to add.
  * \param mac_cnt_before_callback [IN]:  The number of MAC addresses learned on this port (including the one you're called back for).
  * \param keep                    [OUT]: Set to FALSE to remove this entry, TRUE to keep it. In case
  *                                       you set it to FALSE, the psec_add_method_t return value
  *                                       doesn't matter. If you set it to TRUE, the psec_add_method_t
  *                                       is used to determine you module's view of this MAC address.
  * \param action                  [OUT]: ONLY TO BE USED BY PSEC LIMIT. Other modules must leave it alone!
  *                                       See psec_loop_through_action_t for a detailed description.
  *
  * \return
  *   Your module must decide and return its new forwarding decision.\n
  *   This will not be used if you set \@keep to FALSE.
  */
typedef psec_add_method_t (psec_on_mac_loop_through_callback_f)(void                       *user_ctx,
                                                                vtss_isid_t                isid,
                                                                vtss_port_no_t             port,
                                                                vtss_vid_mac_t             *vid_mac,
                                                                u32                        mac_cnt_before_callback,
                                                                BOOL                       *keep,
                                                                psec_loop_through_action_t *action);

/**
  * \brief The required state of the port.
  *
  * Some modules may require that the initial state of the port
  * is blocked, and only MAC addresses that the given module determines
  * are OK are added.
  *
  * Such a module must obtain frames (MAC-addresses) by other means
  * than through the PSEC module (e.g. through BPDUs).
  *
  * Once such a module wants to add a MAC address, it calls
  * the psec_mgmt_mac_add(), which will take care of adding
  * the MAC address to the table, and call all other enabled modules
  * to get their view of that MAC address.
  */
typedef enum {
    /**
      * This is the normal state for user modules to use.
      * The port will be enabled for CPU copying until a user
      * module tells the PSEC module to stop.
      */
    PSEC_PORT_MODE_NORMAL,

    /**
      * With this mode, a user module can keep the port blocked
      * so that not one single MAC address can reach the PSEC
      * module from the packet module. All allowed MAC addresses
      * must come through the psec_mgmt_mac_add() function.
      */
    PSEC_PORT_MODE_KEEP_BLOCKED,
} psec_port_mode_t;

/** \brief Enable or disable a user-module on a given port
  *
  * Use this function to enable or disable your module on a given
  * isid:port.
  *
  * Besides the enable parameter determining whether you're about
  * to enable or disable your module, this function takes a callback
  * function. The purpose of this callback function is to allow your
  * module to determine whether existing entries should be deleted or
  * kept.
  *
  * Your callback function will be called during the duration of
  * the call to psec_mgmt_port_cfg_set(). This means that you may have taken your
  * own critical section before the call to psec_mgmt_port_cfg_set().
  * psec_mgmt_port_cfg_set() will then iterate over all existing entries for
  * that isid:port, and call you back. The called back function
  * should therefore not acquire your module's critical section,
  * and is not allowed to call other psec_XXX() functions, since
  * that will result in a deadlock.
  *
  * This operation must be handled atomically, i.e. without
  * letting go of the PSEC's critical section, because if we didn't
  * it might happen that your module missed an entry that was added
  * or deleted while changing the configuration.
  *
  * For each entry your module determines to delete, the On-MAC-Del
  * callback function will be called for all enabled modules but yours.
  *
  * This way of informing a new module of existing entries is useful
  * for e.g. Voice VLAN, where existing entries must be removed
  * if they aren't considered part of the Voice VLAN.
  *
  * The 802.1X module will delete all entries, and the PSEC LIMIT
  * module will delete entries until the maximum is reached.
  *
  * psec_mgmt_port_cfg_set() will call back with latest added MAC address
  * first.
  *
  * In case your module calls psec_mgmt_port_cfg_set() with enable = TRUE
  * while already being enabled, the loop_through_callback()
  * will still be called. This is useful for both 802.1X and PSEC
  * LIMIT, as follows:
  * If 802.1X moves from one MAC-table-based mode to another, then
  * all the current entries must be flushed, so the loop-through
  * will remove all these entries.
  * If the maximum allowed number of MAC addresses changes on PSEC
  * LIMIT, and the new number is smaller than the old number, then
  * the PSEC LIMIT will re-call psec_mgmt_port_cfg_set() with enable = TRUE
  * and can thus flush out until the current number of learned
  * MAC addresses corresponds to the new maximum number of clients.
  *
  * In case your module disables its membership, the supplied
  * loop-through callback function will still be called (if non-NULL),
  * so that you get a chance to clean up your state. The @action,
  * @keep, and return value from your loop-through callback will not be
  * used for anything when your module disables membership.
  *
  * If you call psec_mgmt_port_cfg_set() with enable = TRUE, but don't specify
  * a callback function, all existing entries will be kept, but be
  * aware that you now will be called whenever one of these entries
  * is deleted. Furthermore, the add_method for your module will be
  * considered to be PSEC_ADD_METHOD_FORWARD.
  *
  * If you call psec_mgmt_port_cfg_set() with @port_mode = PSEC_PORT_MODE_KEEP_BLOCKED,
  * no new MAC addresses will be learned through the packet module
  * (CPU copying will be turned off with the port in secure learning).
  * This is useful if your module gets the allowed MAC addresses
  * by other means than through the PSEC module (e.g. through BPDUs).
  * Most user modules will have to use the @port_mode = PSEC_PORT_MODE_NORMAL.
  *
  * When the last user disables its usage of a port, all previously
  * learned MAC addresses will be removed and the port will return
  * to H/W-based learning.
  *
  * \param user                  [IN]: The user-module identifying you!
  * \param user_ctx              [IN]: Any value. This will be passed back in the
  *                                    loop-through-callback function.
  * \param isid                  [IN]: The switch ID you're trying to configure.
  * \param port                  [IN]: The port on \@isid that you're trying to configure.
  *                                    You may call it with any port number between VTSS_PORT_NO_START
  *                                    and VTSS_PORT_NO_END, even stack ports. Unavailable ports
  *                                    will not really be configured, though.
  * \param enable                [IN]: Set to TRUE to enable your module on this port,
  *                                    FALSE to disable.
  * \param reopen_port           [IN]: MAY ONLY BE USED BY PSEC LIMIT. Always set to
  *                                    FALSE by any other user module. Only used when
  *                                    PSEC LIMIT disables security on the port.
  * \param loop_through_callback [IN]: See description above.
  * \param port_mode             [IN]: See description above. Only used if @enable is TRUE.
  *
  * \return
  *   VTSS_OK if applying the new enabledness succeeded.\n
  *   PSEC_ERROR_MUST_BE_MASTER if the switch is not currently master.\n
  *   PSEC_ERROR_INV_USER if the \@user parameter is invalid.\n
  *   PSEC_ERROR_INV_ISID if the supplied \@isid parameter is invalid.\n
  *   PSEC_ERROR_INV_PORT if the supplied \@port parameter is invalid.
  */
vtss_rc psec_mgmt_port_cfg_set(psec_users_t                        user,
                               void                                *user_ctx,
                               vtss_isid_t                         isid,
                               vtss_port_no_t                      port,
                               BOOL                                enable,
                               BOOL                                reopen_port,
                               psec_on_mac_loop_through_callback_f *loop_through_callback,
                               psec_port_mode_t                    port_mode);

/** \brief Change a MAC address's forwarding state
  *
  * Once the forwarding state has been determined by a module,
  * that module may change it if something should happen in the
  * module's internal state.
  *
  * The 802.1X module, for instance, may move the MAC
  * address from PSEC_ADD_METHOD_KEEP_BLOCKED to
  * PSEC_ADD_METHOD_FORWARD when authentication succeeds or
  * PSEC_ADD_METHOD_BLOCK when authentication fails.
  *
  * \param user       [IN]: The user-module identifying you!
  * \param isid       [IN]: The switch ID you're trying to change.
  * \param port       [IN]: The port on \@isid that you're trying to change.
  * \param vid_mac    [IN]: The <MAC, VID> you're trying to change.
  * \param new_method [IN]: The new forward decision made by your module.
  *
  * \return
  *   VTSS_OK if applying the new forwarding state succeeded.\n
  *   PSEC_ERROR_MUST_BE_MASTER if the switch is not currently master.\n
  *   PSEC_ERROR_INV_USER if the \@user parameter is invalid.\n
  *   PSEC_ERROR_INV_ISID if the supplied \@isid parameter is invalid.\n
  *   PSEC_ERROR_INV_PORT if the supplied \@port parameter is invalid.\n
  *   PSEC_ERROR_MAC_VID_NOT_FOUND if the supplied \@vid_mac was not found
  *     among the attached MAC addresses on the port (both MAC and VID must match).
  */
vtss_rc psec_mgmt_mac_chg(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, psec_add_method_t new_method);

/** \brief Add a MAC address
  *
  * Add a MAC address to the MAC table.
  * Only users that have called psec_mgmt_port_cfg_set() with
  * port_mode == PSEC_PORT_MODE_KEEP_BLOCKED are allowed to call
  * this function. Others will be rejected.
  *
  * This serves as an alternative way to secure MAC addresses.
  * All modules but the calling module will get a chance to
  * allow or block the MAC address.
  *
  * \param user       [IN]: The user-module identifying you!
  * \param isid       [IN]: The switch ID you're trying to change.
  * \param port       [IN]: The port on \@isid that you're trying to change.
  * \param vid_mac    [IN]: The <MAC, VID> you're trying to change.
  * \param new_method [IN]: The new forward decision made by your module.
  *
  * \return
  *   VTSS_OK if applying the new forwarding state succeeded.\n
  *   PSEC_ERROR_MUST_BE_MASTER if the switch is not currently master.\n
  *   PSEC_ERROR_INV_USER if the \@user parameter is invalid.\n
  *   PSEC_ERROR_INV_ISID if the supplied \@isid parameter is invalid.\n
  *   PSEC_ERROR_INV_PORT if the supplied \@port parameter is invalid.\n
  *   PSEC_ERROR_MAC_VID_ALREADY_FOUND if the supplied \@vid_mac was already found
  *     among the attached MAC addresses on any port (both MAC and VID are used in match).
  *   PSEC_ERROR_INV_USER_MODE if the \@user is not enabled on the port or if he has not called the
  *     psec_mgmt_port_cfg_set() function with \@port_mode == PSEC_PORT_MODE_KEEP_BLOCKED.
  *   PSEC_ERROR_SWITCH_IS_DOWN if the switch pointed to by \@isid doesn't exist.
  *   PSEC_ERROR_LINK_IS_DOWN if the port pointed to by \@port has link-down.
  *   PSEC_ERROR_OUT_OF_MAC_STATES if the PSEC module is out of state machines.
  *   PSEC_ERROR_PORT_IS_SHUT_DOWN if the PSEC Limit module has already shut the port
  *     down or shut the port down as a result of attempting to add this one.
  *   PSEC_ERROR_LIMIT_IS_REACHED if the PSEC Limit module has proclamed that the limit
  *     is reached (and it doesn't result in a port-shut-down to attempt to add next MAC address).
  *   PSEC_ERROR_NO_USERS_ENABLED if no users are enabled on the port (cannot be returned by this func,
  *     since it's caught by the PSEC_ERROR_INV_USER_MODE).
  *   PSEC_ERROR_STATE_CHG_DURING_CALLBACK if e.g. the switch was deleted or port had link-down
  *     while the enabled users were called back (which can happen because the internal
  *     mutex has to be released during callbacks to avoid deadlocks).
  *   PSEC_ERROR_INTERNAL_ERROR if there's a bug in this module.
  */
vtss_rc psec_mgmt_mac_add(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, psec_add_method_t method);

/** \brief Delete a MAC address
  *
  * Delete a MAC address from the MAC table.
  * Only users that have called psec_mgmt_port_cfg_set() with
  * port_mode == PSEC_PORT_MODE_KEEP_BLOCKED are allowed to call
  * this function. Others will be rejected.
  *
  * This serves as an alternative way to unsecure MAC addresses.
  * Rather than letting the PSEC module handle timeouts, the
  * user-module may decide (e.g. as a reaction to BPDUs) to
  * remove it immediately.
  * All modules but the calling module will get notified through
  * a call to On-MAC-del-callback().
  *
  * \param user       [IN]: The user-module identifying you!
  * \param isid       [IN]: The switch ID you're trying to delete.
  * \param port       [IN]: The port on \@isid that you're trying to delete.
  * \param vid_mac    [IN]: The <MAC, VID> you're trying to delete.
  *
  * \return
  *   VTSS_OK if applying the new forwarding state succeeded.\n
  *   PSEC_ERROR_MUST_BE_MASTER if the switch is not currently master.\n
  *   PSEC_ERROR_INV_USER if the \@user parameter is invalid.\n
  *   PSEC_ERROR_INV_ISID if the supplied \@isid parameter is invalid.\n
  *   PSEC_ERROR_INV_PORT if the supplied \@port parameter is invalid.\n
  *   PSEC_ERROR_MAC_VID_NOT_FOUND if the supplied \@vid_mac was not found
  *     among the attached MAC addresses on the port (both MAC and VID are used in match).
  *   PSEC_ERROR_INV_USER_MODE if the \@user is not enabled on the port or if he has not called the
  *     psec_mgmt_port_cfg_set() function with \@port_mode == PSEC_PORT_MODE_KEEP_BLOCKED.
  */
vtss_rc psec_mgmt_mac_del(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac);

/**
  * \brief Signature of On-MAC-Add callback function
  *
  * - Reentrancy: The callback may be called back while already being
  *               called back. The callback should not call other
  *               functions in this module will being called back.
  *
  * Only modules that are enabled on isid:port will be called back
  * and asked whether it's OK to add this MAC address on a port as
  * seen from that module's perspective. The module returns its
  * decision as a psec_add_method_t (see that type for details).
  *
  * The called back function must return ASAP.
  *
  * [IN] and [OUT] is seen from the called back module's perspective.
  *
  * \param isid                    [IN]: Switch ID for the MAC address being added.
  * \param port                    [IN]: Port number for the MAC address being added.
  * \param vid_mac                 [IN]: VLAN ID and MAC address to add.
  * \param mac_cnt_before_callback [IN]: The number of MAC addresses already learned on this port.
  * \param action                 [OUT]: ONLY TO BE USED BY PSEC LIMIT. Other modules must leave it alone!
  *                                      See psec_add_action_t for a detailed description.
  * \return
  *   Your callback must return one of the methods specified by psec_add_method_t.
  */
typedef psec_add_method_t (psec_on_mac_add_callback_f)(vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, u32 mac_cnt_before_callback, psec_add_action_t *action);

/**
  * \brief Signature of On-MAC-Del callback function
  *
  * - Reentrancy: The callback will not be called back while already
  *               being called back. The callback may not call other
  *               functions in this module while being called back.
  *
  * Only modules that are enabled on isid:port will be notified that
  * a MAC address is being deleted from the MAC table.
  * See psec_del_reason_t for details.
  *
  * The called back function must return ASAP.
  *
  * [IN] and [OUT] is seen from the called back module's perspective.
  *
  * \param isid       [IN]: Switch ID for the MAC address being removed.
  * \param port       [IN]: Port number for the MAC address being removed.
  * \param vid_mac    [IN]: VLAN ID and MAC address being deleted.
  * \param reason     [IN]: Reason for the removal.
  * \param add_method [IN]: The user module's own add method for this MAC address. Useful for ref-counting.
  *
  * \return
  *   Nothing.
  */
typedef void (psec_on_mac_del_callback_f)(vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, psec_del_reason_t reason, psec_add_method_t add_method);

/**
  * \brief Register callback functions.
  *
  * Register On-MAC-Add and on-MAC-Del callback functions.
  *
  * Specifying NULL for a given callback function means that the
  * module doesn't want to be called back for the operation in
  * question.
  *
  * Modules will only be called back if they are enabled on a given
  * port.
  *
  * If the called back module calls another function in this module,
  * a deadlock will occur.
  *
  * This function should only be called once, and in the INIT_CMD_START
  * phase. The callback functions will only be used on the master.
  *
  * \param user                [IN]: The user that calls this function.
  * \param on_mac_add_callback [IN]: Pointer to a callback function that gets
  *                                  called when a new MAC address is about to
  *                                  be added to the MAC table on a port that
  *                                  the \@user has enabled. May be NULL.
  * \param on_mac_del_callback [IN]: Pointer to a callback function that gets
  *                                  called when a MAC address is about to
  *                                  be deleted from the MAC table on a port that
  *                                  the \@user has enabled. May be NULL.
  *
  * \return
  *   VTSS_OK: Registration succeeded.\n
  *   PSEC_ERR_INV_USER if parameter supplied in \@user is invalid.
  */
vtss_rc psec_mgmt_register_callbacks(psec_users_t user, psec_on_mac_add_callback_f *on_mac_add_callback, psec_on_mac_del_callback_f *on_mac_del_callback);

//
// Interface to be used by the Port Security Limit Control module only.
//

/**
  * \brief Re-open a port that is shut-down by a PSEC LIMIT action.
  *
  * \param user [IN]: The user requestion this action. Currently, this
  *                   is not being used.
  * \param isid [IN]: Switch ID in question.
  * \param port [IN]: Port on which to allow secure learning again.
  *
  * \return
  *   VTSS_OK if re-opening the port succeeded. The call may succeed
  *    whether the port was already open or not.
  *   PSEC_ERROR_MUST_BE_MASTER if the switch is not currently master.\n
  *   PSEC_ERROR_INV_USER if the \@user parameter is invalid.\n
  *   PSEC_ERROR_INV_ISID if the supplied \@isid parameter is invalid.\n
  *   PSEC_ERROR_INV_PORT if the supplied \@port parameter is invalid.
  */
vtss_rc psec_mgmt_reopen_port(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port);

//
// Interface towards Web (CLI uses a semi-public interface defined in psec.h)
//


/**
  * \brief Get Port Security status for the switch
  *
  * \param isid           [IN]: Switch ID to get status for.
  * \param switch_status [OUT]: Pointer to structure receiving the current switch status.
  *
  * \return
  *    VTSS_RC_OK on success.\n
  *    PSEC_ERROR_INV_PARAM if status is NULL.\n
  *    PSEC_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    PSEC_ERROR_INV_ISID if called with an invalid ISID.\n
  */
vtss_rc psec_mgmt_switch_status_get(vtss_isid_t isid, psec_switch_status_t *switch_status);

/**
  * \brief Don't stop CPU copying even if limit is reached.
  *
  * Due to a bug on EStaX-34, BPDUs will not be copied to the CPU if a given
  * port is in secure learning without CPU-copying.
  * The 802.1X module needs to have BPDUs in all BPDU-based modes, even
  * if another module has enabled its use of the PSEC module and causes a limit-reached.
  *
  * If PSEC_FIX_GNATS_6935 is undefined, this call will always return VTSS_OK and
  * have no side-effects.
  *
  * This has no effect unless at least one user is enabled in this module.
  *
  * \param user   [IN]: User that calls this function.
  * \param isid   [IN]: Switch ID to enable or disable force-CPU-copying for.
  * \param port   [IN]: Port number to enable or disable force-CPU-copying for.
  * \param enable [IN]: Set to TRUE to force CPU-copying active, FALSE to let the PSEC module handle this.
  *
  * \return
  *    VTSS_RC_OK on success.\n
  *    PSEC_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    PSEC_ERROR_INV_USER if called with an invalid user.\n
  *    PSEC_ERROR_INV_ISID if called with an invalid ISID.\n
  *    PSEC_ERROR_INV_PORT if called with an invalid port.\n
  */
vtss_rc psec_mgmt_force_cpu_copy(psec_users_t user, vtss_isid_t isid, vtss_port_no_t port, BOOL enable);

//
// Other public Port Security functions.
//

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the Port Security API functions.
  *
  * \param rc [IN]: Error code that must be in the PSEC_ERROR_xxx range.
  *
  * \return
  *   A static string describing the error.
  */
char *psec_error_txt(vtss_rc rc);

/**
  * \brief Initialize the Port Security module
  *
  * \param cmd [IN]: Reason why calling this function.
  * \param p1  [IN]: Parameter 1. Usage varies with cmd.
  * \param p2  [IN]: Parameter 2. Usage varies with cmd.
  *
  * \return
  *    VTSS_RC_OK.
  */
vtss_rc psec_init(vtss_init_data_t *data);

/**
  * \brief Convert delete reason to string.
  *
  * Primarily for debugging purposes.
  *
  * \param reason [IN]: Reason why calling this function.
  *
  * \return
  *    Pointer to static string with the explanation.
  */
char *psec_del_reason_to_str(psec_del_reason_t reason);

/**
  * \brief Convert add method reason to string.
  *
  * Primarily for debugging purposes.
  *
  * \param reason [IN]: add_method to be converted.
  *
  * \return
  *    Pointer to static string with the explanation.
  */
char *psec_add_method_to_str(psec_add_method_t add_method);

#endif /* _PSEC_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
