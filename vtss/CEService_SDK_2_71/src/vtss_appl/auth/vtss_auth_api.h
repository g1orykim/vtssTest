/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_AUTH_API_H_
#define _VTSS_AUTH_API_H_

/**
 * \file vtss_auth_api.h
 * \brief This API provides typedefs and functions for the Auth interface
 */

/**
 * \brief Auth error codes (vtss_rc)
 */
enum {
    VTSS_AUTH_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_AUTH), /**< General error */
    VTSS_AUTH_ERROR_CLIENT_WAITING,                                /**< Client is waiting for reply */
    VTSS_AUTH_ERROR_CLIENT_TIMEOUT,                                /**< Request timed out in client */
    VTSS_AUTH_ERROR_SERVER_TIMEOUT,                                /**< Request timed out in server */
    VTSS_AUTH_ERROR_SERVER_REJECT,                                 /**< Invalid username or password */
    VTSS_AUTH_ERROR_SERVER_BADRESP,                                /**< Invalid response received from server */
    VTSS_AUTH_ERROR_MUST_BE_MASTER,                                /**< Operation only valid on master switch */
    VTSS_AUTH_ERROR_CFG_TIMEOUT,                                   /**< Invalid timeout configuration parameter */
    VTSS_AUTH_ERROR_CFG_RETRANSMIT,                                /**< Invalid retransmit configuration parameter */
    VTSS_AUTH_ERROR_CFG_DEADTIME,                                  /**< Invalid deadtime configuration parameter */
    VTSS_AUTH_ERROR_CFG_HOST,                                      /**< Invalid host name or IP address */
    VTSS_AUTH_ERROR_CFG_PORT,                                      /**< Invalid port configuration parameter */
    VTSS_AUTH_ERROR_CFG_HOST_PORT,                                 /**< Invalid host and port combination */
    VTSS_AUTH_ERROR_CFG_HOST_TABLE_FULL,                           /**< Host table is full */
    VTSS_AUTH_ERROR_CFG_HOST_NOT_FOUND,                            /**< Host not found */
    VTSS_AUTH_ERROR_CFG_AGENT_AUTH_METHOD,                         /**< Invalid agent authentication method */
    VTSS_AUTH_ERROR_NO_RADIUS,                                     /**< RADIUS is not present in this build */
    VTSS_AUTH_ERROR_NO_TACPLUS,                                    /**< TACACS+ is not present in this build */
    VTSS_AUTH_ERROR_CACHE_EXPIRED,                                 /**< The cache entry matches username and password, but is expired */
    VTSS_AUTH_ERROR_CACHE_INVALID,                                 /**< The cache entry is invalid */
    VTSS_AUTH_ERROR_CLIENT_MSG_TX_FAILED,                          /**< Temporary error used in message communication with master */
};

/**
 * \brief Auth configuration
 */
#define VTSS_AUTH_NUMBER_OF_SERVERS            5 /**< We use the same number for both RADIUS and TACACS+ servers */

#define VTSS_AUTH_TIMEOUT_DEFAULT              5 /**< Seconds */
#define VTSS_AUTH_TIMEOUT_MIN                  1 /**< Seconds */
#define VTSS_AUTH_TIMEOUT_MAX               1000 /**< Seconds */

#define VTSS_AUTH_RETRANSMIT_DEFAULT           3 /**< Times */
#define VTSS_AUTH_RETRANSMIT_MIN               1 /**< Times */
#define VTSS_AUTH_RETRANSMIT_MAX            1000 /**< Times */

#define VTSS_AUTH_DEADTIME_DEFAULT             0 /**< Minutes */
#define VTSS_AUTH_DEADTIME_MIN                 0 /**< Minutes */
#define VTSS_AUTH_DEADTIME_MAX              1440 /**< Minutes */

#define VTSS_AUTH_RADIUS_AUTH_PORT_DEFAULT  1812 /**< UDP port number */
#define VTSS_AUTH_RADIUS_ACCT_PORT_DEFAULT  1813 /**< UDP port number */
#define VTSS_AUTH_TACACS_PORT_DEFAULT         49 /**< TCP port number */

#define VTSS_AUTH_HOST_LEN                   256 /**< Maximum length for a hostname (incl. NULL) */
#define VTSS_AUTH_KEY_LEN                     64 /**< Maximum length for a key (incl. NULL) */

/**
 * \brief Auth agents (who is authenticating)
 */
typedef enum {
    VTSS_AUTH_AGENT_CONSOLE,           /* serial port CLI */
    VTSS_AUTH_AGENT_TELNET,            /* telnet CLI */
    VTSS_AUTH_AGENT_SSH,               /* SSH CLI */
    VTSS_AUTH_AGENT_HTTP,              /* HTTP and HTTPS WEB interface */
    VTSS_AUTH_AGENT_LAST               /* The last one - ALWAYS insert above this entry */
} vtss_auth_agent_t;

extern const char *const vtss_auth_agent_names[VTSS_AUTH_AGENT_LAST];

/**
 * \brief Auth agent authentication method
 */
typedef enum {
    VTSS_AUTH_METHOD_NONE,   /* Authentication disabled - login not possible */
    VTSS_AUTH_METHOD_LOCAL,  /* Local authentication */
    VTSS_AUTH_METHOD_RADIUS, /* Remote RADIUS authentication */
    VTSS_AUTH_METHOD_TACACS, /* Remote TACACS+ authentication */
    VTSS_AUTH_METHOD_LAST    /* The last one - ALWAYS insert above this entry */
} vtss_auth_method_t;

extern const char *const vtss_auth_method_names[VTSS_AUTH_METHOD_LAST];

/**
 * \brief Auth protocols
 */
typedef enum {
    VTSS_AUTH_PROTO_RADIUS, /* RADIUS protocol */
    VTSS_AUTH_PROTO_TACACS, /* TACACS+ protocol */
    VTSS_AUTH_PROTO_LAST    /* The last one - ALWAYS insert above this entry */
} vtss_auth_proto_t;

extern const char *const vtss_auth_proto_names[VTSS_AUTH_PROTO_LAST];

/**
 * \brief Auth agent configuration
 */
typedef struct {
    vtss_auth_method_t method[VTSS_AUTH_METHOD_LAST]; /* List of authentication methods */
} vtss_auth_agent_conf_t;

extern const vtss_auth_agent_conf_t vtss_auth_agent_conf_default;

/**
 * \brief Get agent configuration
 *
 * \param agent [IN]  Agent
 * \param conf  [OUT] Configuration.
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_agent_conf_get(vtss_auth_agent_t agent, vtss_auth_agent_conf_t *const conf);

/**
 * \brief Set agent configuration
 *
 * \param agent [IN]  Agent
 * \param conf  [IN]  Configuration.
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_agent_conf_set(vtss_auth_agent_t agent, const vtss_auth_agent_conf_t *const conf);

/*
 * Auth remote host configuration
 */
typedef struct {
    BOOL        nas_ip_address_enable;              /* Enable NAS-IP-Address if TRUE. */
    vtss_ipv4_t nas_ip_address;                     /* Global NAS-IP-Address. */
    BOOL        nas_ipv6_address_enable;            /* Enable NAS-IPv6-Address if TRUE. */
    vtss_ipv6_t nas_ipv6_address;                   /* Global NAS-IPv6-Address. */
    char        nas_identifier[VTSS_AUTH_HOST_LEN]; /* Global NAS-Identifier. */
} vtss_auth_radius_conf_t;

typedef struct {
    u32  timeout;                  /* Global timeout for this group. Can be overridden by each host. */
    u32  retransmit;               /* Global retransmit for this group. Can be overridden by each host (RADIUS only). */
    u32  deadtime;                 /* Global deadtime for this group. */
    char key[VTSS_AUTH_KEY_LEN];   /* Global secret key for this group. Can be overridden by each host. */
} vtss_auth_global_host_conf_t;

typedef struct {
    char host[VTSS_AUTH_HOST_LEN]; /* IPv4, IPv6 or hostname of this server. Entry not used if zero. */
    u32  auth_port;                /* Authentication port number to use on this server (TACACS+ uses this as acct_port also). */
    u32  acct_port;                /* Accounting port number to use on this server (RADIUS only). */
    u32  timeout;                  /* Seconds to wait for a response from this server. Use global timeout if zero. */
    u32  retransmit;               /* Number of times a request is resent to an unresponding server (RADIUS only). Use global retransmit if zero. */
    char key[VTSS_AUTH_KEY_LEN];   /* The secret key to use on this server. Use global key if zero */
} vtss_auth_host_conf_t;

/**
 * \brief Get RADIUS specific configuration
 *
 * \param conf  [OUT] Configuration.
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_radius_conf_get(vtss_auth_radius_conf_t *const conf);

/**
 * \brief Set RADIUS specific configuration
 *
 * \param conf  [IN]  Configuration
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_radius_conf_set(const vtss_auth_radius_conf_t *const conf);

/**
 * \brief Get global configuration for a protocol
 *
 * \param proto [IN]  Protocol
 * \param conf  [OUT] Configuration.
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_global_host_conf_get(vtss_auth_proto_t proto, vtss_auth_global_host_conf_t *const conf);

/**
 * \brief Set global configuration for a protocol
 *
 * \param proto [IN]  Protocol
 * \param conf  [IN]  Configuration
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_global_host_conf_set(vtss_auth_proto_t proto, const vtss_auth_global_host_conf_t *const conf);

/**
 * \brief Add a host configuration for a protocol
 *
 * If host AND auth_port AND acct_port matches an existing entry, this entry is updated.
 * Otherwise the entry is added to the end of list.
 *
 * Adding an entry where host and only one of the port matches is treated as an error.
 *
 * \param proto [IN]  Protocol
 * \param conf  [OUT] Configuration
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_host_add(vtss_auth_proto_t proto, vtss_auth_host_conf_t *const conf);

/**
 * \brief Delete a host configuration for a protocol
 *
 * If host AND auth_port AND acct_port matches an existing entry, this entry is deleted.
 * Otherwise an error is returned.
 *
 * \param proto [IN]  Protocol
 * \param conf  [OUT] Configuration
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_host_del(vtss_auth_proto_t proto, vtss_auth_host_conf_t *const conf);

/**
 * \brief User defined callback called by vtss_auth_mgmt_host_iterate().
 *
 * \param proto  [IN]   Protocol
 * \param contxt [IN]   User defined callback context
 * \param conf   [IN]   Host configuration
 * \param number [IN]   Zero-based number.
 *
 * \return nothing
 */
typedef void (*vtss_auth_host_cb_t)(vtss_auth_proto_t proto, const void *const contxt, const vtss_auth_host_conf_t *const conf, int number);

/**
 * \brief Iterates through all host configurations for a protocol and calls a user specified function for each host.
 *
 * \param proto  [IN]   Protocol
 * \param contxt [IN]   User defined callback context or NULL
 * \param cb     [IN]   User defined callback function
 * \param count  [OUT]  Number of times the cb was called (NULL is valid)
 *
 * \return VTSS_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc vtss_auth_mgmt_host_iterate(vtss_auth_proto_t proto, const void *const contxt, const vtss_auth_host_cb_t cb, int *count);

/**
 * \brief Clear HTTPD Cache
 */
void vtss_auth_mgmt_httpd_cache_expire(void);

/**
 * \brief Authenticate. When an agent wants to authenticate it must call this function.
 *
 * \param agent     [IN]  The agent e.g. telnet or SSH.
 * \param username  [IN]  The username.
 * \param password  [IN]  The password.
 * \param userlevel [OUT] The userlevel for this username/password combination.
 *
 * \return : Return code
 */
vtss_rc vtss_authenticate(vtss_auth_agent_t agent, char *username, char *password, int *userlevel);

/**
 * \brief Auth error txt - converts error code to text
 */
char *vtss_auth_error_txt(vtss_rc rc);

/**
 * \brief Initialize Auth module
 */
vtss_rc vtss_auth_init(vtss_init_data_t *data);

/**
 * \brief Auth debug. Must be called from CLI only.
 */
typedef int (*vtss_auth_dbg_printf_t)(const char *fmt, ...) __attribute__ ((format (__printf__, 1, 2)));
void vtss_auth_dbg(vtss_auth_dbg_printf_t dbg_printf, vtss_auth_agent_t agent, char *username, char *password);

#endif /* _VTSS_AUTH_API_H_ */
