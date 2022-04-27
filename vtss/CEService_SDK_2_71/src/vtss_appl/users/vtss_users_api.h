/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_USERS_API_H_
#define _VTSS_USERS_API_H_

#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */
#include "sysutil_api.h"

/**
 * \file vtss_users_api.h
 * \brief This file defines the APIs for the Users module
 */

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    VTSS_USERS_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_USERS), /**< Operation is only allowed on the master switch. */
    VTSS_USERS_ERROR_ISID,                                                      /**< isid parameter is invalid.                      */
    VTSS_USERS_ERROR_INV_PARAM,                                                 /**< Invalid parameter.                              */
    VTSS_USERS_ERROR_REJECT,                                                    /**< Username and password combination not found     */
    VTSS_USERS_ERROR_CFG_INVALID_USERNAME,                                      /**< Invalid chars or not null terminated            */
    VTSS_USERS_ERROR_USERS_TABLE_FULL                                           /**< Users table full                                */
};

/**
 * \brief Users module configuration.
*/
typedef struct {
    BOOL    valid;                              /**< entry valid?                           */
    char    username[VTSS_SYS_USERNAME_LEN];    /**< Add an extra byte for null termination */
    char    password[VTSS_SYS_PASSWD_LEN];      /**< Add an extra byte for null termination */
    int     privilege_level;                    /**< privilege level                        */
} users_conf_t;

/**
 * Users maximum entries counter
 */
#define VTSS_USERS_NUMBER_OF_USERS  20      /**< Maximum allowed users entry number */

/**
 * Users maximum privilege level
 */
#define VTSS_USERS_MAX_PRIV_LEVEL   15      /**< Maximum allowed privilege level */

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the Users API functions.
  *
  * \param rc [IN]: Error code that must be in the VTSS_USERS_ERROR_xxx range.
  */
char *vtss_users_error_txt(vtss_rc rc);

/**
  * \brief Get the global Users configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  * \param next     [IN]:  Getnext?
  *
  * \return
  *    VTSS_OK on success.\n
  *    VTSS_USERS_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    VTSS_USERS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VTSS_USERS_ERROR_REJECT if get fail.\n
  */
vtss_rc vtss_users_mgmt_conf_get(users_conf_t *glbl_cfg, BOOL next);

/**
  * \brief Set the global Users configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       voice VLAN module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    VTSS_USERS_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    VTSS_USERS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    VTSS_USERS_ERROR_CFG_INVALID_USERNAME if user name is null string.\n
  *    VTSS_USERS_ERROR_USERS_TABLE_FULL if users table is full.\n
  *    Others value arises from sub-function.\n
  */
vtss_rc vtss_users_mgmt_conf_set(users_conf_t *glbl_cfg);

/**
 * \brief Delete the Users configuration.
 *
 * \param user_name [IN] The user name
 * \return : VTSS_OK or one of the following
 *  VTSS_USERS_ERROR_GEN (conf is a null pointer)
 *  VTSS_USERS_ERROR_MUST_BE_MASTER
 */
vtss_rc vtss_users_mgmt_conf_del(char *user_name);

/**
 * \brief Clear the Users configuration.
 *
 * \param user_name [IN] The user name
 * \return : VTSS_OK or one of the following
 *  VTSS_USERS_ERROR_MUST_BE_MASTER
 */
vtss_rc vtss_users_mgmt_conf_clear(void);

/**
  * \brief Initialize the Users module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc vtss_users_init(vtss_init_data_t *data);

/* Check if user name string */
BOOL vtss_users_mgmt_is_valid_username(const char *str);

#endif /* _VTSS_USERS_API_H_ */

// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
