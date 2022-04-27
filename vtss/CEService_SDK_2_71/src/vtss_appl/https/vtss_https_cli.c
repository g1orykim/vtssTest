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

#include "main.h"
#include "cli.h"
#include "cli_grp_help.h"
#include "vtss_https_api.h"
#include "cli_trace_def.h"
#include "network.h"
#include <arpa/inet.h>
#include <tftp_support.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_HTTPS

typedef struct {
    BOOL https_cert_gen_type;
} https_cli_req_t;

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/
void https_cli_req_init(void)
{
    /* register the size required for https req. structure */
    cli_req_size_register(sizeof(https_cli_req_t));
}

static void HTTPS_cli_cmd_conf(cli_req_t *req, BOOL mode, BOOL redirect)
{
    vtss_rc         rc;
    https_conf_t    *conf;

    if ((conf = VTSS_MALLOC(sizeof(https_conf_t))) == NULL) {
        return;
    }

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        https_mgmt_conf_get(conf) != VTSS_OK) {
        VTSS_FREE(conf);
        return;
    }

    if (req->set) {
        if (mode) {
            conf->mode = req->enable;
        }
        if (redirect) {
            if (conf->mode == 0 && req->enable) {
                CPRINTF("Can not enable HTTPS redirect function when the HTTPS operation mode is disabled.\n");
                VTSS_FREE(conf);
                return;
            }
            conf->redirect = req->enable;
        }
        if (conf->mode == 0) {
            conf->redirect = 0;
        }
        if ((rc = https_mgmt_conf_set(conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    } else {
        if (mode) {
            CPRINTF("HTTPS Mode          : %s\n", cli_bool_txt(conf->mode));
        }
#if HTTPS_MGMT_SUPPORTED_REDIRECT
        if (redirect) {
            CPRINTF("HTTPS Redirect Mode : %s\n", cli_bool_txt(conf->redirect));
        }
#endif /* HTTPS_MGMT_SUPPORTED_REDIRECT */
    }
    VTSS_FREE(conf);
}

static void HTTPS_cli_cmd_conf_disp(cli_req_t *req)
{
    if (!req->set) {
        cli_header("HTTPS Configuration", 1);
    }
    HTTPS_cli_cmd_conf(req, 1, 1);
    return;
}

static void HTTPS_cli_cmd_mode(cli_req_t *req)
{
    HTTPS_cli_cmd_conf(req, 1, 0);
    return;
}

static void HTTPS_cli_cmd_redirect(cli_req_t *req)
{
    HTTPS_cli_cmd_conf(req, 0, 1);
    return;
}

static void HTTPS_cli_cmd_cert_info(cli_req_t *req)
{
    char *cert_info = VTSS_MALLOC(HTTPS_MGMT_MAX_CERT_LEN + 1);

    if (!cert_info) {
        return;
    }
    if (https_mgmt_cert_info(cert_info) == VTSS_OK) {
        CPRINTF("Certificate Information: (length: %d)\n%s", strlen(cert_info), cert_info);
    }
    VTSS_FREE(cert_info);
    return;
}

static void HTTPS_cli_cmd_sess_info(cli_req_t *req)
{
#define HTTPS_SESS_INFO_MAX_LEN     20480
    char *sess_info = VTSS_MALLOC(HTTPS_SESS_INFO_MAX_LEN);

    if (!sess_info) {
        return;
    }
    memset(sess_info, 0, sizeof(char) * HTTPS_SESS_INFO_MAX_LEN);
    if (https_mgmt_sess_info(sess_info, HTTPS_SESS_INFO_MAX_LEN) == VTSS_OK) {
        CPRINTF("Session Information: (length: %d)\n%s", strlen(sess_info), sess_info);
    }
    VTSS_FREE(sess_info);
    return;
}

static void HTTPS_cli_cmd_sess_counter(cli_req_t *req)
{
    https_stats_t stats;

    if (https_mgmt_counter_get(&stats) == VTSS_OK) {
        CPRINTF("Statistics:\n");
        CPRINTF("Server connects that finished: %d\n", stats.sess_accept_good);
        CPRINTF("Session renegotiate          : %d\n", stats.sess_accept_renegotiate);
        CPRINTF("Session cache items          : %d\n", stats.sess_accept);
        CPRINTF("Session cache hits           : %d\n", stats.sess_hit);
        CPRINTF("Session cache misses         : %d\n", stats.sess_miss);
        CPRINTF("Session cache timeout        : %d\n", stats.sess_timeout);
        CPRINTF("Session cache full           : %d\n", stats.sess_cache_full);
        CPRINTF("Session ID that not in cache : %d\n", stats.sess_cb_hit);
    }
    return;
}

static void HTTPS_cli_cmd_gen_cert(cli_req_t *req)
{
    https_cli_req_t *https_req = req->module_req;
    https_conf_t    *conf;

    if ((conf = VTSS_MALLOC(sizeof(https_conf_t))) == NULL) {
        return;
    }

    if (https_mgmt_conf_get(conf) == VTSS_OK) {
        // Check HTTPS mode
        if (conf->mode == HTTPS_MGMT_ENABLED) {
            CPRINTF("Please disable HTTPS mode first!\n");
            VTSS_FREE(conf);
            return;
        }
        if (https_mgmt_cert_gen(https_req->https_cert_gen_type) != VTSS_OK) {
            CPRINTF("Generate certificate fail!\n");
        }
    }

    VTSS_FREE(conf);
    return;
}

static int32_t
cli_https_parse_keyword(char *cmd, char *cmd2,
                        char *stx, char *cmd_org,
                        cli_req_t *req)
{
    https_cli_req_t *https_req = req->module_req;
    char            *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "rsa", 3)) {
            https_req->https_cert_gen_type = HTTPS_MGMT_GEN_CERT_TYPE_RSA;
        } else if (!strncmp(found, "dsa", 3)) {
            https_req->https_cert_gen_type = HTTPS_MGMT_GEN_CERT_TYPE_DSA;
        }
    }

    return (found == NULL ? 1 : 0);
}

static void HTTPS_cli_cmd_load_cert(cli_req_t *req)
{
    int                 tftp_rc;
    struct hostent      *host;
    struct in_addr      address;
    unsigned char       *buffer = NULL;
    ulong               buffer_size;
    struct sockaddr_in  server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_len = sizeof(server_addr);
    server_addr.sin_family = AF_INET;

    /* Look up the DNS host name or IPv4 address */
    if ((server_addr.sin_addr.s_addr = inet_addr(req->host_name)) == (unsigned long) - 1) {
        if ((host = gethostbyname(req->host_name)) != NULL &&
            (host->h_length == sizeof(struct in_addr))) {
            address = *((struct in_addr **)host->h_addr_list)[0];
            server_addr.sin_addr.s_addr = address.s_addr;
        } else {
            cli_printf("Error: Cannot find the TFTP server %s\n", req->host_name);
            return;
        }
    }

    if ((buffer = VTSS_MALLOC(HTTPS_MGMT_MAX_CERT_LEN + 1)) != NULL) {
        vtss_rc      rc;
        https_conf_t *https_conf, *newconf;

        if ((https_conf = VTSS_MALLOC(sizeof(https_conf_t))) == NULL) {
            VTSS_FREE(buffer);
            return;
        }
        if ((newconf = VTSS_MALLOC(sizeof(https_conf_t))) == NULL) {
            VTSS_FREE(buffer);
            VTSS_FREE(https_conf);
            return;
        }

        // Get tftp data
        if ((buffer_size = tftp_get(req->parm, &server_addr, (char *) buffer, HTTPS_MGMT_MAX_CERT_LEN, TFTP_OCTET, &tftp_rc)) <= 0) {
            cli_printf("Error: File %s was not found\n", req->parm);
            VTSS_FREE(buffer);
            VTSS_FREE(newconf);
            VTSS_FREE(https_conf);
            return;
        }

        // Update certificate
        if (https_mgmt_conf_get(https_conf) == VTSS_OK) {
            *newconf = *https_conf;

            /* Check global mode */
            if (https_conf->mode) {
                cli_printf("Error: Please disable HTTPS mode first\n");
                VTSS_FREE(buffer);
                VTSS_FREE(newconf);
                VTSS_FREE(https_conf);
                return;
            }

            /* Check certificate size */
            if (buffer_size <= HTTPS_MGMT_MAX_CERT_LEN) {
                newconf->self_signed_cert = FALSE;
                memcpy(newconf->server_cert, buffer, buffer_size);
                memset(newconf->server_pkey, 0, sizeof(newconf->server_pkey));
                memset(newconf->server_pass_phrase, 0, sizeof(newconf->server_pass_phrase));
            } else {
                cli_printf("Error: SSL Certificate PEM file size too big.\n");
                VTSS_FREE(buffer);
                VTSS_FREE(newconf);
                VTSS_FREE(https_conf);
                return;
            }

            if (memcmp(newconf, https_conf, sizeof(*newconf))) {
                T_D("Calling https_mgmt_conf_set()");
                if ((rc = https_mgmt_cert_update(newconf)) != VTSS_OK) {
                    cli_printf("Error: %s\n", error_txt(rc));
                }
            }
        }
        VTSS_FREE(buffer);
        VTSS_FREE(newconf);
        VTSS_FREE(https_conf);
    }
}

static int32_t HTTPS_cli_file_name_parse(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                         cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_raw(cmd_org, req);

    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t https_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable HTTPS\n"
        "disable: Disable HTTPS\n"
        "(default: Show HTTPS mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        HTTPS_cli_cmd_mode
    },
    {
        "enable|disable",
        "enable : Enable HTTPS redirect\n"
        "disable: Disable HTTPS redirect\n"
        "(default: Show HTTPS redirect mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        HTTPS_cli_cmd_redirect
    },
    {
        "rsa|dsa",
        "rsa : An Algorithm which stands for Rivest, Shamir and Adleman who first publicly described it\n"
        "dsa: Digital Signature Algorithm\n"
        "(default: RSA)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_https_parse_keyword,
        HTTPS_cli_cmd_gen_cert
    },
    {
        "<file_name>",
        "certificate file name",
        CLI_PARM_FLAG_NONE,
        HTTPS_cli_file_name_parse,
        NULL
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    HTTPS_PRIO_CONF,
    HTTPS_PRIO_MODE,
    HTTPS_PRIO_REDIRECT,
};

/* Command table entries */
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "HTTPS Configuration",
    NULL,
    "Show HTTPS configuration",
    HTTPS_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    HTTPS_cli_cmd_conf_disp,
    NULL,
    https_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "HTTPS Mode",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "HTTPS Mode [enable|disable]",
    "Set or show the HTTPS mode",
    HTTPS_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    HTTPS_cli_cmd_mode,
    NULL,
    https_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if HTTPS_MGMT_SUPPORTED_REDIRECT
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_SWITCH_PATH "HTTPS Redirect",
    VTSS_CLI_GRP_SEC_SWITCH_PATH "HTTPS Redirect [enable|disable]",
    "Set or show the HTTPS redirect mode.\n"
    "Automatic redirect web browser to HTTPS during HTTPS mode enabled",
    HTTPS_PRIO_REDIRECT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    HTTPS_cli_cmd_redirect,
    NULL,
    https_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* HTTPS_MGMT_SUPPORTED_REDIRECT */

cli_cmd_tab_entry(
    "Debug HTTPS Certificate",
    NULL,
    "Show HTTPS certificate",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    HTTPS_cli_cmd_cert_info,
    NULL,
    https_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug HTTPS Session",
    NULL,
    "Show HTTPS session information",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    HTTPS_cli_cmd_sess_info,
    NULL,
    https_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug HTTPS Statistics",
    NULL,
    "Show HTTPS statistics",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    HTTPS_cli_cmd_sess_counter,
    NULL,
    https_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug HTTPS Generate Certificate [rsa|dsa]",
    NULL,
    "Generate HTTPS certificate",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    HTTPS_cli_cmd_gen_cert,
    NULL,
    https_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug HTTPS Load Certificate <ip_addr_string> <file_name>",
    "Load new certificate from TFTP server",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    HTTPS_cli_cmd_load_cert,
    NULL,
    https_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
