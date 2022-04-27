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

#ifndef _VTSS_SSH_API_H_
#define _VTSS_SSH_API_H_

#if defined(VTSS_SW_OPTION_AUTH)
#include "vtss_auth_api.h"
#endif
#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */


/**
 * \file vtss_ssh_api.h
 * \brief This file defines the APIs for the SSH module
 */


/**
 * SSH managent enabled/disabled
 */
#define SSH_MGMT_ENABLED            (1)     /**< Enable option  */
#define SSH_MGMT_DISABLED           (0)     /**< Disable option */

/**
 * SSH managent default mode
 */
#define SSH_MGMT_DEF_MODE           SSH_MGMT_ENABLED

/**
 * SSH host key length
 */
#define SSH_MGMT_MAX_HOSTKEY_LEN    (512)   /**< Maximum host key length */

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    SSH_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_SSH),  /**< Operation is only allowed on the master switch. */
    SSH_ERROR_ISID,                                                     /**< isid parameter is invalid.                      */
    SSH_ERROR_INV_PARAM,                                                /**< Invalid parameter.                              */
    SSH_ERROR_GET_CERT_INFO,                                            /**< Illegal get certificate information             */
    SSH_ERROR_INTERNAL_RESOURCE                                         /**< Out of internal resource.                       */
};

/**
 * \brief SSH configuration.
 */
typedef struct {
    u32 mode;                                   /* SSH global mode */
    u8  rsa_hostkey[SSH_MGMT_MAX_HOSTKEY_LEN];  /* Server certificate */
    u32 rsa_hostkey_len;                        /* Server certificate key length */
    u8  dss_hostkey[SSH_MGMT_MAX_HOSTKEY_LEN];  /* Server private key */
    u32 dss_hostkey_len;                        /* Server private key length */
} ssh_conf_t;

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the SSH API functions.
  *
  * \param rc [IN]: Error code that must be in the SSH_ERROR_xxx range.
  */
char *ssh_error_txt(vtss_rc rc);

/**
  * \brief Get the global SSH configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    SSH_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    SSH_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc ssh_mgmt_conf_get(ssh_conf_t *glbl_cfg);

/**
  * \brief Set the global SSH configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       voice VLAN module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    SSH_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    SSH_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc ssh_mgmt_conf_set(ssh_conf_t *glbl_cfg);

/* Get SSH public key fingerprint
   type 0: RSA
   type 1: DSS */
void ssh_mgmt_publickey_get(int type, u8 *str_buff);

/**
  * \brief Initialize the SSH module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc ssh_init(vtss_init_data_t *data);

#if defined(VTSS_SW_OPTION_AUTH)
/**
  * \brief Declare user authentication callback function
  */
typedef int (*ssh_user_auth_callback_t)(vtss_auth_agent_t agent, char *username, char *password, int *userlevel);

/**
  * \brief SSH user authentication register function
  *
  * \param cb [IN]: Callback function point.
  */
void ssh_user_auth_register(ssh_user_auth_callback_t cb);
#endif

/**
  * \brief Close all SSH session
  */
void ssh_close_all_session(void);

/**
  * \brief Get the SSH default configuration.
  *
  * \param glbl_cfg [IN_OUT]: Pointer to structure that contains the
  *                           configuration to get the default setting.
  *
  * \return
  *   Nothing.
  */
void ssh_mgmt_conf_get_default(ssh_conf_t *conf);

/**
  * \brief Determine if SSH configuration has changed.
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
int ssh_mgmt_conf_changed(ssh_conf_t *old, ssh_conf_t *new);

#endif /* _VTSS_SSH_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
