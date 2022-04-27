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

#ifndef _PSEC_LIMIT_API_H_
#define _PSEC_LIMIT_API_H_

#include "port_api.h" /* For VTSS_PORTS              */
#include "psec_api.h" /* For PSEC_MAC_ADDR_ENTRY_CNT */

/**
 * \file psec_limit_api.h
 * \brief This file defines the Limit Control API for the Port Security module
 */

#define PSEC_LIMIT_AGING_PERIOD_SECS_MIN     10                            /**< Minimum allowed aging period                                                                    */
#define PSEC_LIMIT_AGING_PERIOD_SECS_MAX     10000000                      /**< Maximum allowed aging period                                                                    */
#define PSEC_LIMIT_AGING_PERIOD_SECS_DEFAULT 3600                          /**< Default aging period of 1 hour                                                                  */
#define PSEC_LIMIT_LIMIT_MIN                 1                             /**< At least this number of MAC addresses per port                                                  */
#define PSEC_LIMIT_LIMIT_MAX                 (PSEC_MAC_ADDR_ENTRY_CNT - 1) /**< At most this number of MAC addresses per port. Preserve one entry for detecting limit exceeded. */
#define PSEC_LIMIT_LIMIT_DEFAULT             MIN(4, PSEC_LIMIT_LIMIT_MAX)  /**< At most this number of MAC addresses per port                                                   */

/**
  * In the special case where action is set to PSEC_LIMIT_ACTION_TRAP,
  * the MAC address that causes the limit to be exceeded must be
  * held for some time so that we don't keep on sending traps.
  * This define defines that number of seconds.
  */
#define PSEC_LIMIT_HOLD_TIME_SECS            300

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    PSEC_LIMIT_ERROR_INV_PARAM = MODULE_ERROR_START(VTSS_MODULE_ID_PSEC_LIMIT), /**< Invalid user parameter.                         */
    PSEC_LIMIT_ERROR_MUST_BE_MASTER,                                            /**< Operation is only allowed on the master switch. */
    PSEC_LIMIT_ERROR_INV_ISID,                                                  /**< isid parameter is invalid.                      */
    PSEC_LIMIT_ERROR_INV_PORT,                                                  /**< Port parameter is invalid.                      */
    PSEC_LIMIT_ERROR_INV_AGING_PERIOD,                                          /**< The supplied aging period is invalid.           */
    PSEC_LIMIT_ERROR_INV_LIMIT,                                                 /**< The supplied MAC address limit is out of range. */
    PSEC_LIMIT_ERROR_INV_ACTION,                                                /**< The supplied action is out of range.            */
    PSEC_LIMIT_ERROR_STATIC_AGGR_ENABLED,                                       /**< Cannot enable if also static aggregated.        */
    PSEC_LIMIT_ERROR_DYNAMIC_AGGR_ENABLED,                                      /**< Cannot enable if also dynamic aggregated.       */
}; // Anonymous enum to satisfy Lint.

/**
 * \brief List of available actions to be taken when limit is reached.
 */
typedef enum {
    PSEC_LIMIT_ACTION_NONE,              /**< Do nothing, except disallowing further clients.        */
    PSEC_LIMIT_ACTION_TRAP,              /**< Send an SNMP trap notification.                        */
    PSEC_LIMIT_ACTION_SHUTDOWN,          /**< Shut-down the port.                                    */
    PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN, /**< Send an SNMP trap notification and shut-down the port. */

    // The following must come last:
    PSEC_LIMIT_ACTION_LAST,
} psec_limit_action_t;

/**
 * \brief Global Port Security Limit Configuration.
 */
typedef struct {

    /**
      * Globally enable/disable Port Security Limit Control feature
      */
    BOOL enabled;

    /**
      * Globally enable/disable aging of secured entries.
      * This doesn't affect aging of addresses secured by
      * other modules.
      */
    BOOL enable_aging;

    /**
     * If aging is globally enabled, this is the aging period in seconds.
     * Valid range is [10; 10000000] seconds (max is around 115 days).
     */
    u32 aging_period_secs;

} psec_limit_glbl_cfg_t;

/**
 * \brief Per-port Port Security Limit Control Configuration.
 */
typedef struct  {
    /**
      * Controls whether Port Security Limit Control is enabled for this port.
      */
    BOOL enabled;

    /**
      * Maximum number of MAC addresses allowed on this port.
      * Valid values = [PSEC_LIMIT_LIMIT_MIN; PSEC_LIMIT_LIMIT_MAX].
      */
    u32 limit;

    /**
      * Action to take if number of MAC addresses exceeds the limit.
      */
    psec_limit_action_t action;

} psec_limit_port_cfg_t;

/**
  * \brief Per-switch Port Security Limit Control Configuration.
  */
typedef struct  {
    /**
      * \brief Array of port configurations - one per front port on the switch.
      *
      * Index 0 is the first port.
      */
    psec_limit_port_cfg_t port_cfg[VTSS_PORTS];
} psec_limit_switch_cfg_t;

//
// Configuration Management Interface
//

/**
  * \brief Get the global Port Security Limit Control configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  *
  * \return
  *    VTSS_RC_OK on success.\n
  *    PSEC_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    PSEC_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc psec_limit_mgmt_glbl_cfg_get(psec_limit_glbl_cfg_t *glbl_cfg);

/**
  * \brief Set the global Port Security Limit Control configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the port
  *                       security limit control module.
  *
  * \return
  *    VTSS_RC_OK on success.\n
  *    PSEC_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    PSEC_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    PSEC_ERROR_INV_AGING_PERIOD if aging_period is out of valid range.\n
  */
vtss_rc psec_limit_mgmt_glbl_cfg_set(psec_limit_glbl_cfg_t *glbl_cfg);

/**
  * \brief Get a switch's per-port configuration.
  *
  * \param isid        [IN]: The Switch ID for which to retrieve the
  *                          configuration.
  * \param switch_cfg [OUT]: Pointer to structure that receives
  *                          the switch's per-port configuration.
  *
  * \return
  *    VTSS_RC_OK on success.\n
  *    PSEC_ERROR_INV_PARAM if switch_cfg is NULL.\n
  *    PSEC_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    PSEC_ERROR_ISID if called with an invalid ISID.\n
  */
vtss_rc psec_limit_mgmt_switch_cfg_get(vtss_isid_t isid, psec_limit_switch_cfg_t *switch_cfg);

/**
  * \brief Set a switch's per-port configuration.
  *
  * \param isid [IN]: The switch ID for which to set the configuration.
  * \param switch_cfg [IN]: Pointer to structure that contains
  *        the switch's per-port configuration to be applied.
  *
  * \return
  *    VTSS_RC_OK on success.\n
  *    PSEC_ERROR_INV_PARAM if switch_cfg is NULL.\n
  *    PSEC_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    PSEC_ERROR_ISID if called with an invalid ISID.\n
  *    PSEC_ERROR_INV_LIMIT if called with an invalid limit.\n
  *    PSEC_ERROR_INV_ACTION if called with an invalid action.
  */
vtss_rc psec_limit_mgmt_switch_cfg_set(vtss_isid_t isid, psec_limit_switch_cfg_t *switch_cfg);

//
// Other public Port Security Limit Control functions.
//

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the Port Security Limit Control API functions.
  *
  * \param rc [IN]: Error code that must be in the PSEC_LIMIT_ERROR_xxx range.
  */
char *psec_limit_error_txt(vtss_rc rc);

/**
  * \brief Initialize the Port Security Limit Control module
  *
  * \param cmd [IN]: Reason why calling this function.
  * \param p1  [IN]: Parameter 1. Usage varies with cmd.
  * \param p2  [IN]: Parameter 2. Usage varies with cmd.
  *
  * \return
  *    VTSS_RC_OK.
  */
vtss_rc psec_limit_init(vtss_init_data_t *data);

/**
  * \brief Getting the default global configuration
  *
  * \param cfg [IN]: Pointer to where to put the configuration.
  * \return
  *    None
  */
void PSEC_LIMIT_cfg_default_glbl(psec_limit_glbl_cfg_t *cfg);

/**
  * \brief Getting the default switch configuration
  *
  * \param cfg [IN]: Pointer to where to put the configuration.
  * \return
  *    None
  */
void PSEC_LIMIT_cfg_default_switch(psec_limit_switch_cfg_t *cfg);
//
// Status Management Interface
//

/* NOTHING THUS FAR */

#endif /* _PSEC_LIMIT_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
