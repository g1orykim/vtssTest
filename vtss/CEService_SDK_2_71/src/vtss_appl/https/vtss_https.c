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
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "misc_api.h"
#include "syslog_api.h"
#include "vtss_https_api.h"
#include "vtss_https.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "vtss_https_icfg.h"
#endif /* VTSS_SW_OPTION_ICFG */

#include <network.h>

#include <cyg/io/file.h>        /* iovec */
#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>

#define HTTPS_USING_DEFAULT_CERTIFICATE     0
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_HTTPS

#if HTTPS_USING_DEFAULT_CERTIFICATE
/* Using openSSL demoCA, refer to ..\openssl-0.9.8e\apps\demoCA
subject=/C=AU/SOP=QLD/O=Mincom Pty. Ltd./OU=CS/CN=SSLeay demo server
issuer= /C=AU/SOP=QLD/O=Mincom Pty. Ltd./OU=CS/CN=CA */

#define HTTPS_DEFAULT_CERTIFICATE "-----BEGIN X509 CERTIFICATE-----\n\
MIIBgjCCASwCAQQwDQYJKoZIhvcNAQEEBQAwODELMAkGA1UEBhMCQVUxDDAKBgNV\n\
BAgTA1FMRDEbMBkGA1UEAxMSU1NMZWF5L3JzYSB0ZXN0IENBMB4XDTk1MTAwOTIz\n\
MzIwNVoXDTk4MDcwNTIzMzIwNVowYDELMAkGA1UEBhMCQVUxDDAKBgNVBAgTA1FM\n\
RDEZMBcGA1UEChMQTWluY29tIFB0eS4gTHRkLjELMAkGA1UECxMCQ1MxGzAZBgNV\n\
BAMTElNTTGVheSBkZW1vIHNlcnZlcjBcMA0GCSqGSIb3DQEBAQUAA0sAMEgCQQC3\n\
LCXcScWua0PFLkHBLm2VejqpA1F4RQ8q0VjRiPafjx/Z/aWH3ipdMVvuJGa/wFXb\n\
/nDFLDlfWp+oCPwhBtVPAgMBAAEwDQYJKoZIhvcNAQEEBQADQQArNFsihWIjBzb0\n\
DCsU0BvL2bvSwJrPEqFlkDq3F4M6EGutL9axEcANWgbbEdAvNJD1dmEmoWny27Pn\n\
IMs6ZOZB\n\
-----END X509 CERTIFICATE-----\n\
"

#define HTTPS_DEFAULT_PRIVATE_KEY "-----BEGIN RSA PRIVATE KEY-----\n\
MIIBPAIBAAJBALcsJdxJxa5rQ8UuQcEubZV6OqkDUXhFDyrRWNGI9p+PH9n9pYfe\n\
Kl0xW+4kZr/AVdv+cMUsOV9an6gI/CEG1U8CAwEAAQJAXJMBZ34ZXHd1vtgL/3hZ\n\
hexKbVTx/djZO4imXO/dxPGRzG2ylYZpHmG32/T1kaHpZlCHoEPgHoSzmxYXfxjG\n\
sQIhAPmZ/bQOjmRUHM/VM2X5zrjjM6z18R1P6l3ObFwt9FGdAiEAu943Yh9SqMRw\n\
tL0xHGxKmM/YJueUw1gB6sLkETN71NsCIQCeT3RhoqXfrpXDoEcEU+gwzjI1bpxq\n\
agiNTOLfqGoA5QIhAIQFYjgzONxex7FLrsKBm16N2SFl5pXsN9SpRqqL2n63AiEA\n\
g9VNIQ3xwpw7og3IbONifeku+J9qGMGQJMKwSTwrFtI=\n\
-----END RSA PRIVATE KEY-----\n\
"

#define HTTPS_DEFAULT_PASS_PHRASE   "011E"

#define HTTPS_DEFAULT_DH_PARAMETERS "-----BEGIN DH PARAMETERS-----\n\
MIGHAoGBAJf2QmHKtQXdKCjhPx1ottPb0PMTBH9A6FbaWMsTuKG/K3g6TG1Z1fkq\n\
/Gz/PWk/eLI9TzFgqVAuPvr3q14a1aZeVUMTgo2oO5/y2UHe6VaJ+trqCTat3xlx\n\
/mNbIK9HA2RgPC3gWfVLZQrY+gz3ASHHR5nXWHEyvpuZm7m3h+irAgEC\n\
-----END DH PARAMETERS-----\n\
"

#else

#define HTTPS_DEFAULT_PASS_PHRASE   "1234"

#endif /* HTTPS_USING_DEFAULT_CERTIFICATE */


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static https_global_t HTTPS_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t HTTPS_trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "https",
    .descr     = "HTTPS"
};

static vtss_trace_grp_t HTTPS_trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define HTTPS_CRIT_ENTER() critd_enter(&HTTPS_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define HTTPS_CRIT_EXIT()  critd_exit( &HTTPS_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define HTTPS_CRIT_ENTER() critd_enter(&HTTPS_global.crit)
#define HTTPS_CRIT_EXIT()  critd_exit( &HTTPS_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/* Thread variables */
#define HTTPS_CERT_THREAD_STACK_SIZE       20480
static cyg_handle_t HTTPS_cert_thread_handle;
static cyg_thread   HTTPS_cert_thread_block;
static char         HTTPS_cert_thread_stack[HTTPS_CERT_THREAD_STACK_SIZE];

/****************************************************************************/
/*  compare functions                                                       */
/****************************************************************************/

/* Get HTTPS defaults */
void https_conf_mgmt_get_default(https_conf_t *conf)
{
    HTTPS_CRIT_ENTER();
    memset(conf, 0, sizeof(*conf));
    conf->mode = HTTPS_MGMT_DEF_MODE;
    conf->redirect = HTTPS_MGMT_DEF_REDIRECT_MODE;
    conf->self_signed_cert = TRUE;
    conf->active_sess_timeout = HTTPS_MGMT_DEF_ACTIVE_SESS_TIMEOUT;
    conf->absolute_sess_timeout = HTTPS_MGMT_DEF_ABSOLUTE_SESS_TIMEOUT;
#if HTTPS_USING_DEFAULT_CERTIFICATE
    strcpy(conf->server_cert, HTTPS_DEFAULT_CERTIFICATE);
    strcpy(conf->server_pkey, HTTPS_DEFAULT_PRIVATE_KEY);
    strcpy(conf->server_pass_phrase, HTTPS_DEFAULT_PASS_PHRASE);
    strcpy(conf->server_dh_parameters, HTTPS_DEFAULT_DH_PARAMETERS);
#else
    if (conf != &HTTPS_global.https_conf) {
        if (HTTPS_global.https_conf.self_signed_cert &&
            HTTPS_global.https_conf.server_cert[0] != '\0') {
            strcpy(conf->server_cert, HTTPS_global.https_conf.server_cert);
            strcpy(conf->server_pkey, HTTPS_global.https_conf.server_pkey);
            strcpy(conf->server_dh_parameters, HTTPS_global.https_conf.server_dh_parameters);
        }
    }

    /* The pass phrase is defined on compiled time, it doesn't allow changed on runtime */
    strcpy(conf->server_pass_phrase, HTTPS_DEFAULT_PASS_PHRASE);
#endif /* HTTPS_USING_DEFAULT_CERTIFICATE */
    HTTPS_CRIT_EXIT();
}

/* Determine if HTTPS configuration has changed */
int https_mgmt_conf_changed(const https_conf_t *const old, const https_conf_t *const new)
{
    return (new->mode != old->mode
            || new->redirect != old->redirect
            || new->self_signed_cert != old->self_signed_cert
            || new->active_sess_timeout != old->active_sess_timeout
            || new->absolute_sess_timeout != old->absolute_sess_timeout
            || strcmp(new->server_cert, old->server_cert)
            || strcmp(new->server_pkey, old->server_pkey)
            || strcmp(new->server_pass_phrase, old->server_pass_phrase)
            || strcmp(new->server_dh_parameters, old->server_dh_parameters)
           );
}

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

static void HTTPS_conf_apply(void)
{
    if (msg_switch_is_master()) {
        BOOL    https_mode, https_redirect;
        long    https_act_tmo, https_abs_tmo;

        HTTPS_CRIT_ENTER();
        https_mode = HTTPS_global.https_conf.mode;
        https_redirect = HTTPS_global.https_conf.redirect;
        https_act_tmo = HTTPS_global.https_conf.active_sess_timeout;
        https_abs_tmo = HTTPS_global.https_conf.absolute_sess_timeout;
        HTTPS_CRIT_EXIT();

        cyg_https_set_mode(https_mode);
        cyg_https_set_redirect(https_redirect);
        cyg_https_set_session_timeout(https_act_tmo, https_abs_tmo);
    }
}


/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* HTTPS error text */
char *https_error_txt(https_error_t rc)
{
    switch (rc) {
    case HTTPS_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case HTTPS_ERROR_INV_PARAM:
        return "Invalid parameter supplied to function";

    case HTTPS_ERROR_GET_CERT_INFO:
        return "Get certificate information fail";

    case HTTPS_ERROR_MUST_BE_DISABLED_MODE:
        return "Operation only valid under HTTPS mode disalbed";

    case HTTPS_ERROR_HTTPS_CORE_START:
        return "HTTPS core initial failed";

    case HTTPS_ERROR_INV_CERT:
        return "HTTPS invalid Certificate";

    case HTTPS_ERROR_INV_DH_PARAM:
        return "HTTPS invalid DH parameter";

    case HTTPS_ERROR_INTERNAL_RESOURCE:
        return "HTTPS out of internal resource";

    default:
        return "HTTPS: Unknown error code";
    }
}

/* Get HTTPS configuration */
vtss_rc https_mgmt_conf_get(https_conf_t *const glbl_cfg)
{
    /*lint --e{429}*/
    T_D("enter");

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return HTTPS_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit");
        return HTTPS_ERROR_MUST_BE_MASTER;
    }

    HTTPS_CRIT_ENTER();
    *glbl_cfg = HTTPS_global.https_conf;
    HTTPS_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set HTTPS configuration */
vtss_rc https_mgmt_conf_set(const https_conf_t *const glbl_cfg)
{
    vtss_rc rc      = VTSS_OK;
    int     changed = 0;

    T_D("enter, mode: %d redirect: %d", glbl_cfg->mode, glbl_cfg->redirect);

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return HTTPS_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit");
        return HTTPS_ERROR_MUST_BE_MASTER;
    }

    /* check illegal parameter */
    if (glbl_cfg->mode != HTTPS_MGMT_ENABLED && glbl_cfg->mode != HTTPS_MGMT_DISABLED) {
        return HTTPS_ERROR_INV_PARAM;
    }
    if (glbl_cfg->redirect != HTTPS_MGMT_ENABLED && glbl_cfg->redirect != HTTPS_MGMT_DISABLED) {
        return HTTPS_ERROR_INV_PARAM;
    }
    if (glbl_cfg->self_signed_cert != TRUE && glbl_cfg->self_signed_cert != FALSE) {
        return HTTPS_ERROR_INV_PARAM;
    }
    if (glbl_cfg->active_sess_timeout < HTTPS_MGMT_MIN_SOFT_TIMEOUT || glbl_cfg->active_sess_timeout > HTTPS_MGMT_MAX_SOFT_TIMEOUT) {
        return HTTPS_ERROR_INV_PARAM;
    }
    if (glbl_cfg->absolute_sess_timeout < HTTPS_MGMT_MIN_HARD_TIMEOUT || glbl_cfg->absolute_sess_timeout > HTTPS_MGMT_MAX_HARD_TIMEOUT) {
        return HTTPS_ERROR_INV_PARAM;
    }
    if (strlen(glbl_cfg->server_cert) >  HTTPS_MGMT_MAX_CERT_LEN ||
        strlen(glbl_cfg->server_pkey) >  HTTPS_MGMT_MAX_PKEY_LEN ||
        strlen(glbl_cfg->server_pass_phrase) >  HTTPS_MGMT_MAX_PASS_PHRASE_LEN ||
        strlen(glbl_cfg->server_dh_parameters) >  HTTPS_MGMT_MAX_DH_PARAMETERS_LEN) {
        return HTTPS_ERROR_INV_PARAM;
    }

    HTTPS_CRIT_ENTER();
    changed = https_mgmt_conf_changed(&HTTPS_global.https_conf, glbl_cfg);
    HTTPS_global.https_conf = *glbl_cfg;
    HTTPS_CRIT_EXIT();

    if (changed) {
        /* Save changed configuration */
        https_conf_blk_t *https_conf_blk_p;
        conf_blk_id_t    blk_id = CONF_BLK_HTTPS_CONF;
        if ((https_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open HTTPS table");
        } else {
            https_conf_blk_p->https_conf = *glbl_cfg;
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE    // Save the HTTPS key only
            https_conf_blk_p->https_conf.mode = HTTPS_MGMT_DEF_MODE;
            https_conf_blk_p->https_conf.redirect = HTTPS_MGMT_DEF_REDIRECT_MODE;
            https_conf_blk_p->https_conf.active_sess_timeout = HTTPS_MGMT_DEF_ACTIVE_SESS_TIMEOUT;
            https_conf_blk_p->https_conf.absolute_sess_timeout = HTTPS_MGMT_DEF_ABSOLUTE_SESS_TIMEOUT;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        }
        /* Activate changed configuration */
        HTTPS_conf_apply();
    }

    T_D("exit");
    return rc;
}

static BOOL https_mgmt_check_cert_subject_name(void)
{
    BOOL self_signed_cert, match = FALSE;
    char *cert_info = NULL;

    HTTPS_CRIT_ENTER();
    self_signed_cert = HTTPS_global.https_conf.self_signed_cert;
    HTTPS_CRIT_EXIT();
    if (!self_signed_cert) { // Don't need generate certificate
        return TRUE;
    }

    cert_info = VTSS_MALLOC(HTTPS_MGMT_MAX_CERT_LEN + 1);
    if (!cert_info) {
        return FALSE;
    }

    if (https_mgmt_cert_info(cert_info) == VTSS_OK) {
        if (strstr((char *) cert_info, VTSS_PRODUCT_NAME)) {
            match = TRUE;
        }
    }

    VTSS_FREE(cert_info);
    return match;
}


/****************************************************************************
 * Module thread
 ****************************************************************************/
/* Generate certificate will use OpenSSL API EVP_PKEY_assign_RSA(),
   it will take about 9K stack size.
   We create a new thread to do it for instead of in 'Init Modules' thread.
   That we don't need wait a long time in 'Init Modules' thread. */
static void HTTPS_cert_thread(cyg_addrword_t data)
{
    https_conf_t            https_conf, new_https_conf;
    int                     generate_cert_fail = 0;
    BOOL                    apply_flag, https_cert_gen_status;
    https_gen_cert_type_t   https_cert_gen_type;

    HTTPS_CRIT_ENTER();
    apply_flag = HTTPS_global.apply_init_conf;
    HTTPS_global.apply_init_conf = FALSE;
    https_conf = HTTPS_global.https_conf;
    https_cert_gen_status = HTTPS_global.https_cert_gen_status;
    if (https_conf.self_signed_cert && strlen(https_conf.server_pkey) < 562) {
        // re-generate certificate if the key length is less than 512
        // (the return lenght of cyg_https_generate_key(512 bits) is 561)
        https_cert_gen_status = 1;
    }
    https_cert_gen_type = HTTPS_global.https_cert_gen_type;
    HTTPS_CRIT_EXIT();

    if (https_conf.server_cert[0] == '\0' ||
        https_cert_gen_status == 1 ||
        (https_conf.server_cert[0] != '\0' && !https_mgmt_check_cert_subject_name())) {
        cyg_https_new_cert_info_t new_cert_info;
        memset(&new_cert_info, 0x0, sizeof(new_cert_info));
#ifdef VTSS_PRODUCT_NAME
        strcpy(new_cert_info.common, VTSS_PRODUCT_NAME);
#else
        strcpy(new_cert_info.common, "E-Stax-34");
#endif
        cyg_https_set_new_cert_info(&new_cert_info);
        strcpy(https_conf.server_pass_phrase, HTTPS_DEFAULT_PASS_PHRASE);
        if (cyg_https_generate_cert(https_conf.server_cert,
                                    https_conf.server_pkey,
                                    https_conf.server_pass_phrase,
                                    https_cert_gen_type == HTTPS_MGMT_GEN_CERT_TYPE_RSA ? 1 : 0) == 0) {
            /* Generating key will take a while,
            we read configuration again to prevent something loss. */
            if (msg_switch_is_master() && https_mgmt_conf_get(&new_https_conf) == VTSS_OK) {
                if (new_https_conf.server_cert[0] == '\0' || https_cert_gen_status == 1) {
                    https_conf.mode = new_https_conf.mode;
                    https_conf.redirect = new_https_conf.redirect;
                    if (https_cert_gen_status == 1) {
                        https_conf.self_signed_cert = TRUE;
                    } else {
                        https_conf.self_signed_cert = new_https_conf.self_signed_cert;
                    }
                    https_conf.active_sess_timeout = new_https_conf.active_sess_timeout;
                    https_conf.absolute_sess_timeout = new_https_conf.absolute_sess_timeout;
                    memcpy(https_conf.server_dh_parameters, new_https_conf.server_dh_parameters, sizeof(new_https_conf.server_dh_parameters));
                    if (https_mgmt_conf_set(&https_conf) != VTSS_OK) {
                        T_W("Calling https_mgmt_conf_set() failed.\n");
                    }
                } else {
                    https_conf = new_https_conf;
                }
            }
        } else {
            generate_cert_fail = 1;
            T_E("HTTPS generate certificate fail.\n");
        }
    }

    if (generate_cert_fail == 0) {
        if (https_cert_gen_status == 1) {
            cyg_https_shutdown();
        }
        if (cyg_https_start(https_conf.server_cert,
                            https_conf.server_pkey[0] == '\0' ? NULL : https_conf.server_pkey,
                            https_conf.server_pass_phrase,
                            https_conf.server_dh_parameters[0] == '\0' ? NULL : https_conf.server_dh_parameters)) {
            S_W("HTTPS start failed.");
        }
    }

    HTTPS_CRIT_ENTER();
    HTTPS_global.https_cert_gen_status = FALSE;
    HTTPS_CRIT_EXIT();

    if (apply_flag) {
        HTTPS_conf_apply();
    }
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create HTTPS stack configuration */
static void HTTPS_conf_read_stack(BOOL create)
{
    int                         changed;
    BOOL                        do_create = create;
    u32                         size;
    https_conf_t                *old_https_conf_p, new_https_conf;
    https_conf_blk_t            *conf_blk_p;
    conf_blk_id_t               blk_id;
    u32                         blk_version;

    T_D("enter, create: %d", create);

    /* Read/create HTTPS configuration */
    blk_id = CONF_BLK_HTTPS_CONF;
    blk_version = HTTPS_CONF_BLK_VERSION;

    if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*conf_blk_p)) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        T_W("conf_sec_open failed or size mismatch, creating defaults");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
        do_create = 1;
    } else if (conf_blk_p->version != blk_version) {
        T_W("version mismatch, creating defaults");
        do_create = 1;
    } else {
        do_create = create;
    }

    changed = 0;
    /* Use default values first. (Quiet lint/Coverity) */
    https_conf_mgmt_get_default(&new_https_conf);

    if (do_create) {
        if (conf_blk_p != NULL) {
            conf_blk_p->https_conf = new_https_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            new_https_conf = conf_blk_p->https_conf;
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
            // If the default setting is that https is enabled; see https_mgmt_conf_get_default().
            // That value isn't saved in conf, however, so we restore it here. The
            // reason is that since "ip http secure-server" is the default, it isn't generated by
            // running-config -- and hence not present in startup-config. Then the
            // value from the conf block wins -- and it's always "no ip http secure-server".
            new_https_conf.mode = HTTPS_MGMT_DEF_MODE;
            new_https_conf.redirect = HTTPS_MGMT_DEF_REDIRECT_MODE;
            new_https_conf.active_sess_timeout = HTTPS_MGMT_DEF_ACTIVE_SESS_TIMEOUT;
            new_https_conf.absolute_sess_timeout = HTTPS_MGMT_DEF_ABSOLUTE_SESS_TIMEOUT;
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        }
    }
    HTTPS_CRIT_ENTER();
    old_https_conf_p = &HTTPS_global.https_conf;
    if (https_mgmt_conf_changed(old_https_conf_p, &new_https_conf)) {
        changed = 1;
    }
    HTTPS_global.https_conf = new_https_conf;
    if (changed || do_create) {
        HTTPS_global.apply_init_conf = TRUE;
    }
    HTTPS_CRIT_EXIT();

    if (conf_blk_p == NULL) {
        T_W("failed to open HTTPS table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
    T_D("exit");
}

/* Module start */
static void HTTPS_start(void)
{
    https_conf_t *conf_p;

    T_D("enter");

    /* Create semaphore for critical regions */
    critd_init(&HTTPS_global.crit, "HTTPS_global.crit", VTSS_MODULE_ID_HTTPS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    HTTPS_CRIT_EXIT();
    /* Initialize HTTPS configuration */

    HTTPS_global.apply_init_conf = FALSE;
    conf_p = &HTTPS_global.https_conf;
    https_conf_mgmt_get_default(conf_p);

    /* Create HTTPS certificate thread */
    cyg_thread_create(THREAD_BELOW_NORMAL_PRIO,
                      HTTPS_cert_thread,
                      0,
                      "HTTPS certificate",
                      HTTPS_cert_thread_stack,
                      sizeof(HTTPS_cert_thread_stack),
                      &HTTPS_cert_thread_handle,
                      &HTTPS_cert_thread_block);

    T_D("exit");
}

/* Get the HTTPS certificate information */
/*lint -e{429} ... The Variable of 'cert_info' has been freed before returned */
vtss_rc https_mgmt_cert_info(char *cert_info)
{
    vtss_rc rc;
    https_conf_t conf;

    if ((rc = https_mgmt_conf_get(&conf)) != VTSS_OK) {
        return rc;
    }

    if (cyg_https_get_cert_info(conf.server_cert, cert_info)) {
        return HTTPS_ERROR_GET_CERT_INFO;
    }

    return VTSS_OK;
}

/* Get the HTTPS session information */
vtss_rc https_mgmt_sess_info(char *sess_info, u32 sess_info_len)
{
    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit");
        return HTTPS_ERROR_MUST_BE_MASTER;
    }

    cyg_https_get_sess_info(sess_info, sess_info_len);

    return VTSS_OK;
}

/* Get the HTTPS statistics */
vtss_rc https_mgmt_counter_get(https_stats_t *stats)
{
    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit");
        return HTTPS_ERROR_MUST_BE_MASTER;
    }

    cyg_https_get_stats((cyg_https_stats_t *) stats);

    return VTSS_OK;
}

/* Delete the HTTPS certification */
vtss_rc https_mgmt_cert_del(void)
{
    vtss_rc rc;
    https_conf_t conf;

    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit");
        return HTTPS_ERROR_MUST_BE_MASTER;
    }

    if ((rc = https_mgmt_conf_get(&conf)) != VTSS_OK) {
        return rc;
    }

    if (conf.mode == HTTPS_MGMT_ENABLED) {
        return HTTPS_ERROR_MUST_BE_DISABLED_MODE;
    }

    memset(conf.server_cert, 0, sizeof(conf.server_cert));
    memset(conf.server_pkey, 0, sizeof(conf.server_pkey));
    memset(conf.server_pass_phrase, 0, sizeof(conf.server_pass_phrase));
    memset(conf.server_dh_parameters, 0, sizeof(conf.server_dh_parameters));
    return https_mgmt_conf_set(&conf);
}

/* Generate the HTTPS certificate */
vtss_rc https_mgmt_cert_gen(https_gen_cert_type_t type)
{
    vtss_rc rc;
    https_conf_t conf;

    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit");
        return HTTPS_ERROR_MUST_BE_MASTER;
    }

    if ((rc = https_mgmt_conf_get(&conf)) != VTSS_OK) {
        return rc;
    }

    /* if (conf.server_cert[0] != '\0' && conf.self_signed_cert) {
        return VTSS_OK;
    } */

    HTTPS_CRIT_ENTER();
    HTTPS_global.https_cert_gen_status = TRUE;
    HTTPS_global.https_cert_gen_type = type;
    HTTPS_CRIT_EXIT();

    /* Starting HTTPS certification thread again */
    cyg_thread_resume(HTTPS_cert_thread_handle);

    return VTSS_OK;
}

/* Update the HTTPS certificate */
vtss_rc https_mgmt_cert_update(https_conf_t *cfg)
{
    vtss_rc rc;

    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit");
        return HTTPS_ERROR_MUST_BE_MASTER;
    }

    /* Check Certificate */
    if (cfg->server_cert[0] != '\0' && cyg_https_check_cert(cfg->server_cert, cfg->server_pkey[0] == '\0' ? NULL : cfg->server_pkey, cfg->server_pass_phrase)) {
        return HTTPS_ERROR_INV_CERT;
    }

    /* Check DH parameters */
    if (cfg->server_dh_parameters[0] != '\0' && cyg_https_check_dh_parameters(cfg->server_dh_parameters)) {
        return HTTPS_ERROR_INV_DH_PARAM;
    }

    cyg_https_shutdown();
    if (cfg->server_cert[0] != '\0') {
        /* cyg_https_start() maybe return error if the update sequence is unexpected */
        if (cyg_https_start(cfg->server_cert,
                            cfg->server_pkey[0] == '\0' ? NULL : cfg->server_pkey,
                            cfg->server_pass_phrase,
                            cfg->server_dh_parameters[0] == '\0' ? NULL : cfg->server_dh_parameters)) {
            S_W("HTTPS start failed.");
        }
    }

    rc = https_mgmt_conf_set(cfg);
    return rc;
}

BOOL https_mgmt_cert_gen_status(void)
{
    return HTTPS_global.https_cert_gen_status;
}

/* Initialize module */
vtss_rc https_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc     rc = VTSS_OK;
#endif

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&HTTPS_trace_reg, HTTPS_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&HTTPS_trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        HTTPS_start();
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = vtss_https_icfg_init()) != VTSS_OK) {
            T_D("Calling vtss_https_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif /* VTSS_SW_OPTION_ICFG */
        break;
    case INIT_CMD_START:
        T_D("START");
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            HTTPS_conf_read_stack(1);
            HTTPS_conf_apply();
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }

        /* Starting HTTPS certification thread again */
        //cyg_thread_resume(HTTPS_cert_thread_handle);

        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

        /* Read stack and switch configuration */
        HTTPS_conf_read_stack(0);

        /* Starting HTTPS certificate thread (became master) */
        cyg_thread_resume(HTTPS_cert_thread_handle);
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply configuration to switch */
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit");

    return VTSS_OK;
}


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
