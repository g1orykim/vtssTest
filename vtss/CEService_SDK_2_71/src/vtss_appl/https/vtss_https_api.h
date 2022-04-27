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

#ifndef _VTSS_HTTPS_API_H_
#define _VTSS_HTTPS_API_H_


/**
 * If HTTPS management supported HTTPS automatic redirect function.
 * It will automatic redirect web browser to HTTPS during HTTPS mode enabled.
 */
#define HTTPS_MGMT_SUPPORTED_REDIRECT       1

/**
 * HTTPS management enabled/disabled
 */
#define HTTPS_MGMT_ENABLED       (1)    /**< Enable option  */
#define HTTPS_MGMT_DISABLED      (0)    /**< Disable option */

/**
 * HTTPS managent default setting
 */
#define HTTPS_MGMT_DEF_MODE                     HTTPS_MGMT_DISABLED
#define HTTPS_MGMT_DEF_REDIRECT_MODE            HTTPS_MGMT_DISABLED
#define HTTPS_MGMT_DEF_ACTIVE_SESS_TIMEOUT      0   /* 0: disable active session timeout */
#define HTTPS_MGMT_DEF_ABSOLUTE_SESS_TIMEOUT    0   /* 0: disable absolute session timeout */

/**
 * Maximum allowed length
 */
#define HTTPS_MGMT_MAX_CERT_LEN             (2048)  /**< Maximum allowed certification length */
#define HTTPS_MGMT_MAX_PKEY_LEN             (1024)  /**< Maximum allowed private key length   */
#define HTTPS_MGMT_MAX_PASS_PHRASE_LEN      (64)    /**< Maximum allowed pass phrase length   */
#define HTTPS_MGMT_MAX_DH_PARAMETERS_LEN    (256)   /**< Maximum allowed DH (1024)            */

/**
 * Minimum/Maximum allowed timeout value (seconds)
 */
#define HTTPS_MGMT_MIN_SOFT_TIMEOUT     (0)     // The value 0 means disable active session timeout
#define HTTPS_MGMT_MAX_SOFT_TIMEOUT     (3600)  // 60 minutes
#define HTTPS_MGMT_MIN_HARD_TIMEOUT     (0)     // The value 0 means disable absolute session timeout
#define HTTPS_MGMT_MAX_HARD_TIMEOUT     (10080) // 168 hours

/**
 * \brief API Error Return Codes (vtss_rc)
 */
typedef enum {
    HTTPS_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_HTTPS),  /**< Operation is only allowed on the master switch. */
    HTTPS_ERROR_INV_PARAM,                                                  /**< Invalid parameter.                              */
    HTTPS_ERROR_GET_CERT_INFO,                                              /**< Illegal get certificate information */
    HTTPS_ERROR_MUST_BE_DISABLED_MODE,                                      /**< Operation is only allowed under HTTPS mode disalbed. */
    HTTPS_ERROR_HTTPS_CORE_START,                                           /**< HTTPS core initial failed. */
    HTTPS_ERROR_INV_CERT,                                                   /**< Invalid Certificate. */
    HTTPS_ERROR_INV_DH_PARAM,                                               /**< Invalid DH parameter. */
    HTTPS_ERROR_INTERNAL_RESOURCE                                           /**< Out of internal resource. */
} https_error_t;

/**
 * \brief HTTPS configuration.
 */
typedef struct {
    BOOL mode;                                                          /**< HTTPS global mode setting.                                              */
    BOOL redirect;                                                      /**< Automatic Redirect HTTP to HTTPS during HTTPS mode enabled              */
    BOOL self_signed_cert;                                              /**< The certificate is attested to by the switch itself                     */
    long active_sess_timeout;                                           /**< The inactivity timeout for HTTPS sessions if there is no user activity  */
    long absolute_sess_timeout;                                         /**< The hard timeout for HTTPS sessions, regardless of recent user activity */
    char server_cert[HTTPS_MGMT_MAX_CERT_LEN + 1];                      /**< Server certificate                                                      */
    char server_pkey[HTTPS_MGMT_MAX_PKEY_LEN + 1];                      /**< Server private key                                                      */
    char server_pass_phrase[HTTPS_MGMT_MAX_PASS_PHRASE_LEN + 1];        /**< Privary key pass phrase                                                 */
    char server_dh_parameters[HTTPS_MGMT_MAX_DH_PARAMETERS_LEN + 1];    /**< DH parameters, that provide algorithms for encrypting the key exchanges */
} https_conf_t;

/**
 * \brief HTTPS statistics.
 */
typedef struct {
    int sess_accept;                /**< SSL new accept - started           */
    int sess_accept_renegotiate;    /**< SSL reneg - requested              */
    int sess_accept_good;           /**< SSL accept/reneg - finished        */
    int sess_miss;                  /**< session lookup misses              */
    int sess_timeout;               /**< reuse attempt on timeouted session */
    int sess_cache_full;            /**< session removed due to full cache  */
    int sess_hit;                   /**< session reuse actually done        */
    int sess_cb_hit;                /**< session-id that was not
                                     * in the cache was
                                     * passed back via the callback.  This
                                     * indicates that the application is
                                     * supplying session-id's from other
                                     * processes - spooky :-)               */
} https_stats_t;

/**
 * \brief HTTPS generate certificate type.
 */
typedef enum {
    HTTPS_MGMT_GEN_CERT_TYPE_RSA,    /**< Generate RSA certificate. */
    HTTPS_MGMT_GEN_CERT_TYPE_DSA   /**< Generate DSA certificate. */
} https_gen_cert_type_t;


/**
  * \brief Retrieve an error string based on a return code
  *        from one of the HTTPS API functions.
  *
  * \param rc [IN]: Error code that must be in the HTTPS_xxx range.
  */
char *https_error_txt(https_error_t rc);

/**
  * \brief Get the global HTTPS configuration.
  * Notice the https configuration take about 3K bytes, it may occurs thread
  * stacksize overflow, use allocate the memory before call the API.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    HTTPS_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    HTTPS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc https_mgmt_conf_get(https_conf_t *const glbl_cfg);

/**
  * \brief Set the global HTTPS configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       voice VLAN module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    HTTPS_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    HTTPS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc https_mgmt_conf_set(const https_conf_t *const glbl_cfg);

/**
  * \brief Get the HTTPS certificate information.
  *
  * \param cert_info   [OUT]: The length of certificate information
  *                           should be at least 2048.
  *
  * \return
  *    VTSS_OK on success.\n
  *    HTTPS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    HTTPS_ERROR_GET_CERT_INFO if get fail.\n
  */
vtss_rc https_mgmt_cert_info(char *cert_info);

/**
  * \brief Get the HTTPS session information.
  *
  * \param sess_info   [OUT]: The length of session information
  *                           should be at least 2048.
  *
  * \return
  *    VTSS_OK on success.\n
  *    HTTPS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc https_mgmt_sess_info(char *sess_info, u32 sess_info_len);

/**
  * \brief Get the HTTPS statistics.
  *
  * \param counter [OUT]: Pointer to structure that receives
  *                       the current statistics.
  *
  * \return
  *    VTSS_OK on success.\n
  *    HTTPS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc https_mgmt_counter_get(https_stats_t *stats);

/**
  * \brief Delete the HTTPS certificate.
  *
  * \return
  *    VTSS_OK on success.\n
  *    HTTPS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    HTTPS_ERROR_MUST_BE_DISABLED_MODE if called on HTTPS enabled mode.\n
  */
vtss_rc https_mgmt_cert_del(void);

/**
  * \brief Generate the HTTPS certificate.
  *
  * \return
  *    VTSS_OK on success.\n
  *    HTTPS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc https_mgmt_cert_gen(https_gen_cert_type_t type);

/**
  * \brief Update the HTTPS certificate.
  *        Before calling this API, https_mgmt_cert_del() must be called first
  *        Or disable HTTPS mode first.
  *
  * \param cfg [IN]: Pointer to structure that contains the
  *                  global configuration to apply to the
  *                  voice VLAN module.
  * \return
  *    VTSS_OK on success.\n
  *    HTTPS_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  *    HTTPS_ERROR_INV_CERT if Certificate is invaled.\n
  *    HTTPS_ERROR_INV_DH_PARAM if DH parameters is invaled.\n
  */
vtss_rc https_mgmt_cert_update(https_conf_t *cfg);

/**
  * \brief Get the HTTPS certificate generation status.
  *
  * \return
  *    TRUE  - Certificate generation in progress.\n
  *    FALSE - No certificate generation in progress.
  */
BOOL https_mgmt_cert_gen_status(void);

/**
  * \brief Initialize the HTTPS module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc https_init(vtss_init_data_t *data);

/**
  * \brief Get the HTTPS default configuration.
  *
  * \param glbl_cfg [IN_OUT]: Pointer to structure that contains the
  *                           configuration to get the default setting.
  *
  * \return
  *   Nothing.
  */
void https_conf_mgmt_get_default(https_conf_t *conf);

/**
  * \brief Determine if HTTPS configuration has changed.
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
int https_mgmt_conf_changed(const https_conf_t *const old, const https_conf_t *const new);

#endif /* _VTSS_HTTPS_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
