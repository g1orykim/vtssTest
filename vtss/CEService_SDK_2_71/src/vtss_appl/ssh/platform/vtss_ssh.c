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
#include "vtss_ssh_api.h"
#include "vtss_ssh.h"
#ifdef VTSS_SW_OPTION_CLI_TELNET
#include "cli_api.h"
#endif /* VTSS_SW_OPTION_CLI_TELNET */
#ifdef VTSS_SW_OPTION_ICFG
#include "vtss_ssh_icfg.h"
#endif /* VTSS_SW_OPTION_ICFG */

#include <stdio.h>
#include <network.h>

#define SSH_USING_DROPBEAR_PACKAGE      1

#if SSH_USING_DROPBEAR_PACKAGE
#include "dropbear_ecos.h"
#endif /* SSH_USING_DROPBEAR_PACKAGE */


#define SSH_USING_DEFAULT_HOSTKEY       0

#if SSH_USING_DEFAULT_HOSTKEY
static u8 SSH_default_rsa_hostkey[] = {
    0x00, 0x00, 0x00, 0x07, 0x73, 0x73, 0x68, 0x2D, 0x72, 0x73, 0x61, 0x00, 0x00, 0x00, 0x03, 0x01,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x83, 0x00, 0xA6, 0x3A, 0x50, 0xDE, 0xEE, 0xB0, 0x55, 0x34, 0xFC,
    0x46, 0x1E, 0x8E, 0xE9, 0x27, 0x6A, 0xB7, 0xD5, 0x6C, 0xAE, 0x81, 0xD4, 0xAC, 0x0D, 0xCB, 0xA2,
    0x60, 0x84, 0x3A, 0x5B, 0x7A, 0x50, 0x3F, 0x7C, 0x20, 0x85, 0x56, 0x9B, 0x0F, 0x96, 0xE0, 0x90,
    0xB5, 0xF8, 0xE4, 0x4F, 0x3D, 0xF1, 0x0D, 0x37, 0x27, 0x0A, 0x20, 0x48, 0xC0, 0x55, 0x76, 0xE3,
    0x90, 0x79, 0x31, 0x29, 0x94, 0x7B, 0x09, 0xA3, 0x3B, 0xCF, 0xBD, 0xD8, 0x10, 0x1C, 0x7E, 0x2D,
    0x3C, 0xD7, 0x95, 0xD0, 0xE5, 0x40, 0x32, 0x5E, 0x48, 0x0C, 0x00, 0x96, 0xAF, 0x5A, 0x0E, 0x12,
    0xA0, 0xFA, 0x56, 0x72, 0x55, 0x4C, 0x7C, 0xD6, 0x97, 0x45, 0x73, 0x7C, 0x42, 0x5C, 0xCD, 0x50,
    0xFF, 0xB9, 0x47, 0x2D, 0x4E, 0xE2, 0x90, 0xE9, 0xE8, 0xEF, 0x2E, 0x13, 0x2C, 0x49, 0x94, 0x65,
    0x1A, 0x0C, 0xD7, 0xD0, 0x91, 0x06, 0xA9, 0x1E, 0xE1, 0x00, 0x00, 0x00, 0x82, 0x00, 0x99, 0x47,
    0x8C, 0xEB, 0x05, 0x0E, 0x74, 0x83, 0xB7, 0x5C, 0xB1, 0x30, 0xF6, 0xAD, 0xF2, 0x2A, 0x32, 0x13,
    0x55, 0x97, 0x42, 0x94, 0xA4, 0xE3, 0xF1, 0x87, 0x2E, 0x66, 0xE1, 0x93, 0x98, 0x0B, 0xF2, 0x59,
    0xCA, 0x9A, 0x61, 0xC1, 0xAE, 0x4E, 0xC5, 0x22, 0x07, 0xAF, 0xFF, 0xA2, 0x0E, 0x75, 0x31, 0x36,
    0x3E, 0x90, 0x26, 0x85, 0xB4, 0x49, 0x3C, 0xEE, 0x2B, 0xD2, 0xB9, 0x94, 0xC1, 0x1F, 0xD0, 0x77,
    0x5E, 0x86, 0xC0, 0xA9, 0xA1, 0x6C, 0x37, 0x6F, 0x14, 0x38, 0x36, 0x7A, 0x1B, 0x04, 0x75, 0xE5,
    0x19, 0x16, 0xCE, 0xCF, 0xF0, 0xB4, 0xBE, 0x9C, 0x3D, 0xB2, 0x84, 0x85, 0x41, 0x07, 0xED, 0xC7,
    0x73, 0x9C, 0xDA, 0xD6, 0x53, 0x0D, 0x30, 0xA2, 0x28, 0xE1, 0xB2, 0x02, 0xD9, 0x25, 0xA6, 0x62,
    0x41, 0x23, 0x5D, 0x23, 0x19, 0x16, 0x66, 0x9B, 0x1E, 0xD9, 0x73, 0x05, 0xAF, 0x9A, 0x11, 0x00,
    0x00, 0x00, 0x42, 0x00, 0xBB, 0x9B, 0xA3, 0x2B, 0xBE, 0x0E, 0x2E, 0xEB, 0xAD, 0x97, 0xE8, 0x4B,
    0X3C, 0x18, 0x24, 0xF6, 0xE0, 0x39, 0x4B, 0xFD, 0x43, 0x18, 0xA1, 0x67, 0xBE, 0xB7, 0x46, 0x02,
    0XAD, 0x80, 0xF8, 0xAB, 0xD8, 0xC3, 0xC7, 0x6E, 0x29, 0xEF, 0x72, 0xAF, 0xA3, 0xA0, 0x9F, 0x1C,
    0XFC, 0xA1, 0xBF, 0x19, 0x12, 0x35, 0xBB, 0xF1, 0xEC, 0x84, 0x1E, 0x67, 0xF2, 0xDB, 0xE0, 0x03,
    0X0F, 0xDE, 0x20, 0x45, 0x71, 0x00, 0x00, 0x00, 0x42, 0x00, 0xE2, 0xD3, 0x64, 0x5A, 0x1A, 0x77,
    0XF3, 0xA6, 0x4C, 0x6F, 0xF9, 0x1E, 0x27, 0x26, 0x91, 0x3A, 0xC4, 0x47, 0x07, 0xEE, 0xFF, 0xAD,
    0X1A, 0xDC, 0x8F, 0xB9, 0x23, 0xCF, 0xBB, 0x90, 0x16, 0x19, 0x76, 0x96, 0x64, 0xE4, 0xDF, 0x5D,
    0XD1, 0x5C, 0xE7, 0xF3, 0x7F, 0x59, 0xB4, 0xD2, 0xEA, 0x15, 0x20, 0x4C, 0xD7, 0x31, 0xF6, 0x8B,
    0X44, 0x3D, 0x9A, 0xA9, 0xBA, 0xED, 0xF9, 0x62, 0xF5, 0xF8, 0x71
};
static u32 SSH_default_rsa_hostkey_len = sizeof(SSH_default_rsa_hostkey);

static u8 SSH_default_dss_hostkey[] = {
    0x00, 0x00, 0x00, 0x07, 0x73, 0x73, 0x68, 0x2D, 0x64, 0x73, 0x73, 0x00, 0x00, 0x00, 0x81, 0x00,
    0x84, 0xAC, 0xB3, 0xB9, 0x8D, 0x36, 0xED, 0xF3, 0xF7, 0xC4, 0x64, 0x29, 0x37, 0xFA, 0x45, 0x7E,
    0x3C, 0x5E, 0x88, 0xF5, 0x7C, 0xEC, 0xAB, 0x90, 0xC4, 0xAE, 0x79, 0x5F, 0x59, 0x17, 0xEE, 0xA5,
    0x68, 0x0C, 0xF1, 0x6E, 0x92, 0x77, 0xB8, 0x38, 0x17, 0xC8, 0x29, 0x78, 0xD7, 0xF8, 0x4A, 0x3D,
    0x9E, 0x92, 0x40, 0x59, 0x20, 0x08, 0x48, 0x0D, 0x95, 0x5C, 0x38, 0x65, 0x5C, 0x2A, 0x66, 0x2D,
    0x2F, 0xD0, 0x1A, 0x44, 0x49, 0x74, 0xF4, 0x74, 0xD5, 0x8C, 0xA5, 0x58, 0x35, 0x71, 0x1B, 0xD0,
    0xC2, 0xBB, 0x92, 0xE7, 0x13, 0x0F, 0xB5, 0x3C, 0x92, 0x2D, 0x2D, 0x86, 0xBE, 0xE3, 0x74, 0x0E,
    0xA1, 0x19, 0x43, 0x25, 0x09, 0x17, 0x45, 0x3C, 0x25, 0x18, 0xBB, 0xCA, 0x1C, 0x8F, 0x3B, 0x54,
    0x7E, 0xDC, 0x5C, 0x94, 0x1D, 0xDB, 0xBD, 0x19, 0x70, 0x97, 0xDB, 0xEC, 0x52, 0x80, 0x3A, 0xB9,
    0x00, 0x00, 0x00, 0x15, 0x00, 0xF7, 0xE5, 0xAC, 0x01, 0xBE, 0xF1, 0x30, 0x67, 0x02, 0x1A, 0x0D,
    0x54, 0xAF, 0xED, 0x49, 0x8C, 0x97, 0xD0, 0xF4, 0x3F, 0x00, 0x00, 0x00, 0x80, 0x2B, 0xED, 0xCC,
    0xAD, 0x98, 0x99, 0x17, 0x4D, 0x52, 0x2A, 0x22, 0x6E, 0x63, 0x54, 0x6F, 0x72, 0x19, 0x50, 0xF1,
    0xF8, 0x11, 0xE0, 0x43, 0xAB, 0x8F, 0x7C, 0x0C, 0x81, 0x3F, 0xDE, 0xED, 0xED, 0x9F, 0xDE, 0x0E,
    0x0F, 0xEA, 0x7B, 0xB4, 0xF2, 0x49, 0x0C, 0x22, 0x3E, 0x47, 0x10, 0x8A, 0x79, 0xD2, 0x4F, 0xF0,
    0xD6, 0xF1, 0x48, 0x07, 0x6A, 0x0C, 0x97, 0xAE, 0xD9, 0xE3, 0x44, 0x3F, 0xD8, 0xC8, 0x87, 0xD4,
    0x9E, 0xBB, 0xAA, 0x04, 0x1A, 0x77, 0x82, 0xBE, 0xB1, 0xA6, 0x8E, 0x70, 0x0B, 0x1B, 0x28, 0xAC,
    0x22, 0x22, 0x85, 0x84, 0xA7, 0xEC, 0xBB, 0xE5, 0x9A, 0x73, 0x2E, 0x0B, 0x2A, 0x9A, 0xD0, 0x69,
    0x52, 0x5E, 0x10, 0x6A, 0x3D, 0xB2, 0xAC, 0x3E, 0x68, 0xCF, 0x8A, 0xF4, 0xDE, 0xF4, 0xEA, 0x80,
    0xCD, 0x52, 0x3C, 0x2B, 0xBF, 0x97, 0x98, 0x54, 0x64, 0x01, 0xC7, 0xB0, 0x10, 0x00, 0x00, 0x00,
    0x80, 0x16, 0x64, 0x27, 0x51, 0x7F, 0xE4, 0xF2, 0x1E, 0x82, 0x19, 0x26, 0x08, 0xFE, 0x94, 0x67,
    0x15, 0x88, 0xA0, 0x1E, 0x2B, 0x97, 0x48, 0xD4, 0x9D, 0x3F, 0xE2, 0xB3, 0x91, 0x57, 0x8B, 0x61,
    0xA5, 0x52, 0x44, 0xD8, 0x85, 0xD4, 0x43, 0x8E, 0x4F, 0x7F, 0xA9, 0x63, 0x20, 0x6A, 0x00, 0x10,
    0x2E, 0x2B, 0x4F, 0x2E, 0x17, 0x83, 0xA5, 0x54, 0xBE, 0x02, 0x74, 0x28, 0xAD, 0xA2, 0x12, 0x1C,
    0x5B, 0x6E, 0x1C, 0xCC, 0x6C, 0x95, 0x67, 0xDB, 0x2F, 0x10, 0x52, 0xC5, 0x03, 0x10, 0xC5, 0x68,
    0x23, 0x7E, 0xBA, 0x08, 0x1D, 0x2E, 0x26, 0x0A, 0x7E, 0xFD, 0x52, 0x2B, 0x11, 0x5A, 0xD3, 0x34,
    0xBD, 0x56, 0xE9, 0x00, 0xCF, 0x7B, 0x2A, 0xC7, 0x44, 0xD6, 0x19, 0xFB, 0xAD, 0xBD, 0x76, 0x22,
    0x2B, 0x52, 0xDA, 0x6E, 0x7B, 0x15, 0x09, 0x65, 0x80, 0x46, 0xA8, 0x33, 0xDA, 0xDF, 0x3B, 0xA5,
    0xB4, 0x00, 0x00, 0x00, 0x15, 0x00, 0x8D, 0x89, 0x94, 0xA7, 0x88, 0xA3, 0xE0, 0xDC, 0x90, 0x2E,
    0x34, 0x18, 0x1B, 0xF0, 0xE0, 0x80, 0xE5, 0x9D, 0xC5, 0x06
};

static u32 SSH_default_dss_hostkey_len = sizeof(SSH_default_dss_hostkey);
#endif /* SSH_USING_DEFAULT_HOSTKEY */


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static ssh_global_t SSH_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t SSH_trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "ssh",
    .descr     = "SSH"
};

static vtss_trace_grp_t SSH_trace_grps[TRACE_GRP_CNT] = {
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
#define SSH_CRIT_ENTER() critd_enter(&SSH_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SSH_CRIT_EXIT()  critd_exit( &SSH_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define SSH_CRIT_ENTER() critd_enter(&SSH_global.crit)
#define SSH_CRIT_EXIT()  critd_exit( &SSH_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/* Thread variables */
#define SSH_CERT_THREAD_STACK_SIZE       16384
static cyg_handle_t SSH_thread_handle;
static cyg_thread   SSH_thread_block;
static char         SSH_thread_stack[SSH_CERT_THREAD_STACK_SIZE];


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Get SSH public key fingerprint
   type 0: RSA
   type 1: DSS */
void ssh_mgmt_publickey_get(int type, u8 *str_buff)
{
#if SSH_USING_DROPBEAR_PACKAGE
    ssh_conf_t conf;

    if (ssh_mgmt_conf_get(&conf) == VTSS_OK) {
        if (type == 0) {
            dropbear_printpubkey(conf.rsa_hostkey, conf.rsa_hostkey_len, str_buff);
        } else if (type == 1) {
            dropbear_printpubkey(conf.dss_hostkey, conf.dss_hostkey_len, str_buff);
        }
    }
#endif /* SSH_USING_DROPBEAR_PACKAGE */
}

/* Determine if SSH configuration has changed */
int ssh_mgmt_conf_changed(ssh_conf_t *old, ssh_conf_t *new)
{
    return (new->mode != old->mode
            || memcmp(new->rsa_hostkey, old->rsa_hostkey, new->rsa_hostkey_len > old->rsa_hostkey_len ? new->rsa_hostkey_len : old->rsa_hostkey_len)
            || new->rsa_hostkey_len != old->rsa_hostkey_len
            || memcmp(new->dss_hostkey, old->dss_hostkey, new->dss_hostkey_len > old->dss_hostkey_len ? new->dss_hostkey_len : old->dss_hostkey_len)
            || new->dss_hostkey_len != old->dss_hostkey_len);
}

/* Get SSH defaults */
void ssh_mgmt_conf_get_default(ssh_conf_t *conf)
{
    SSH_CRIT_ENTER();
    conf->mode = SSH_MGMT_DEF_MODE;
#if SSH_USING_DEFAULT_HOSTKEY
    memcpy(conf->rsa_hostkey, SSH_default_rsa_hostkey, SSH_default_rsa_hostkey_len);
    conf->rsa_hostkey_len = SSH_default_rsa_hostkey_len;
    memcpy(conf->dss_hostkey, SSH_default_dss_hostkey, SSH_default_dss_hostkey_len);
    conf->dss_hostkey_len = SSH_default_dss_hostkey_len;
#else
    if (conf != &SSH_global.ssh_conf &&
        SSH_global.ssh_conf.rsa_hostkey_len != 0 &&
        SSH_global.ssh_conf.rsa_hostkey[3] != 0) {
        /* When system is running, don't change the hostkey
           rsa_hostkey[3] means length, it should be not zero */
        memcpy(conf->rsa_hostkey, SSH_global.ssh_conf.rsa_hostkey, SSH_global.ssh_conf.rsa_hostkey_len);
        conf->rsa_hostkey_len = SSH_global.ssh_conf.rsa_hostkey_len;
        memcpy(conf->dss_hostkey, SSH_global.ssh_conf.dss_hostkey, SSH_global.ssh_conf.dss_hostkey_len);
        conf->dss_hostkey_len = SSH_global.ssh_conf.dss_hostkey_len;
    } else {
        memset(conf->rsa_hostkey, 0x0, sizeof(conf->rsa_hostkey));
        conf->rsa_hostkey_len = 0;
        memset(conf->dss_hostkey, 0x0, sizeof(conf->dss_hostkey));
        conf->dss_hostkey_len = 0;
    }
#endif /* SSH_USING_DEFAULT_HOSTKEY */
    SSH_CRIT_EXIT();
}

static void SSH_conf_apply(void)
{
    if (msg_switch_is_master()) {
        BOOL ssh_mode;

        SSH_CRIT_ENTER();
        ssh_mode = SSH_global.ssh_conf.mode;
        SSH_CRIT_EXIT();

        dropbear_set_ssh_mode(ssh_mode);
    }
}


/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* SSH error text */
char *ssh_error_txt(vtss_rc rc)
{
    switch (rc) {
    case SSH_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case SSH_ERROR_ISID:
        return "Invalid Switch ID";

    case SSH_ERROR_INV_PARAM:
        return "Invalid parameter supplied to function";

    case SSH_ERROR_GET_CERT_INFO:
        return "Illegal get certificate information";

    case SSH_ERROR_INTERNAL_RESOURCE:
        return "SSH out of internal resource";

    default:
        return "SSH: Unknown error code";
    }
}

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
vtss_rc ssh_mgmt_conf_get(ssh_conf_t *glbl_cfg)
{
    if (glbl_cfg == NULL) {
        T_W("not master");
        return SSH_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_D("not master");
        return SSH_ERROR_MUST_BE_MASTER;
    }

    SSH_CRIT_ENTER();
    *glbl_cfg = SSH_global.ssh_conf;
    SSH_CRIT_EXIT();

    return VTSS_OK;
}

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
vtss_rc ssh_mgmt_conf_set(ssh_conf_t *glbl_cfg)
{
    vtss_rc         rc      = VTSS_OK;
    int             changed = 0;

    T_D("enter, mode: %d", glbl_cfg->mode);

    if (glbl_cfg == NULL) {
        T_D("exit");
        T_W("not master");
        return SSH_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_D("exit");
        T_W("not master");
        return SSH_ERROR_MUST_BE_MASTER;
    }

    /* check illegal parameter */
    if (glbl_cfg->mode != SSH_MGMT_ENABLED && glbl_cfg->mode != SSH_MGMT_DISABLED) {
        return SSH_ERROR_INV_PARAM;
    }
    if (glbl_cfg->rsa_hostkey_len > SSH_MGMT_MAX_HOSTKEY_LEN ||
        glbl_cfg->dss_hostkey_len > SSH_MGMT_MAX_HOSTKEY_LEN) {
        return SSH_ERROR_INV_PARAM;
    }

    SSH_CRIT_ENTER();
    changed = ssh_mgmt_conf_changed(&SSH_global.ssh_conf, glbl_cfg);
    SSH_global.ssh_conf = *glbl_cfg;
    SSH_CRIT_EXIT();

    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t  blk_id = CONF_BLK_SSH_CONF;
        ssh_conf_blk_t *ssh_conf_blk_p;
        if ((ssh_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SSH table");
        } else {
            ssh_conf_blk_p->ssh_conf = *glbl_cfg;
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE    // Save the SSH key only
            ssh_conf_blk_p->ssh_conf.mode = SSH_MGMT_DEF_MODE;
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
        SSH_conf_apply();

#if defined(VTSS_SW_OPTION_CLI_TELNET) && defined(TELNET_SECURITY_SUPPORTED)
        telnet_set_security_mode(glbl_cfg->mode);
#endif /* VTSS_SW_OPTION_CLI_TELNET & TELNET_SECURITY_SUPPORTED */
    }

    T_D("exit");
    return rc;
}


/****************************************************************************
 * Module thread
 ****************************************************************************/
/* Generate SSH keys will use Dropbear API dropbear_generate_hostkey(),
   it will take about 9K stack size.
   We create a new thread to do it for instead of in 'Init Modules' thread.
   That we don't need wait a long time in 'Init Modules' thread. */
static void SSH_thread(cyg_addrword_t data)
{
#if SSH_USING_DROPBEAR_PACKAGE
    BOOL apply_flag;
    ssh_conf_t ssh_conf, new_ssh_conf;

    if (msg_switch_is_master()) {
        SSH_CRIT_ENTER();
        apply_flag = SSH_global.apply_init_conf;
        SSH_global.apply_init_conf = FALSE;
        ssh_conf = SSH_global.ssh_conf;
        SSH_CRIT_EXIT();

        if (ssh_conf.rsa_hostkey_len == 0 || ssh_conf.dss_hostkey_len == 0) {
            dropbear_generate_hostkey(1/*DROPBEAR_SIGNKEY_RSA*/,
                                      ssh_conf.rsa_hostkey, (unsigned long *) &ssh_conf.rsa_hostkey_len);
            dropbear_generate_hostkey(2/*DROPBEAR_SIGNKEY_DSS*/,
                                      ssh_conf.dss_hostkey, (unsigned long *) &ssh_conf.dss_hostkey_len);

            /* Generating key will take a while,
               we read configuration again to prevent something loss. */
            if (msg_switch_is_master() && ssh_mgmt_conf_get(&new_ssh_conf) == VTSS_OK) {
                if (new_ssh_conf.rsa_hostkey_len == 0 || new_ssh_conf.dss_hostkey_len == 0) {
                    ssh_conf.mode = new_ssh_conf.mode;
                    if (ssh_mgmt_conf_set(&ssh_conf) != VTSS_OK) {
                        T_W("ssh_mgmt_conf_set() failed");
                    }
                } else {
                    ssh_conf = new_ssh_conf;
                }
            }
        }

        dropbear_hostkey_init(ssh_conf.rsa_hostkey, ssh_conf.rsa_hostkey_len,
                              ssh_conf.dss_hostkey, ssh_conf.dss_hostkey_len);

        /* Enter normal priority once we have the key ready */
        cyg_thread_set_priority(SSH_thread_handle, THREAD_DEFAULT_PRIO);

        if (apply_flag) {
            SSH_conf_apply();
        }

        /* There's a forever loop in the function */
        dropbear_ecos_daemon();
    } else {
        /* Suspend SSH thread (Only needed in master mode) */
        cyg_thread_suspend(SSH_thread_handle);
    }
#else
    while (1) {
        VTSS_OS_MSLEEP(1000);
    }
#endif /* SSH_USING_DROPBEAR_PACKAGE */
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create SSH stack configuration */
static void SSH_conf_read_stack(BOOL create)
{
    int             changed;
    BOOL            do_create;
    u32             size;
    ssh_conf_t      *old_ssh_conf_p, new_ssh_conf;
    ssh_conf_blk_t  *conf_blk_p;
    conf_blk_id_t   blk_id;
    u32             blk_version;

    T_D("enter, create: %d", create);

    /* Read/create SSH configuration */
    blk_id = CONF_BLK_SSH_CONF;
    blk_version = SSH_CONF_BLK_VERSION;

    if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*conf_blk_p)) {
        T_W("conf_sec_open failed or size mismatch, creating defaults");
        conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
        do_create = 1;
    } else if (conf_blk_p->version != blk_version) {
        T_W("version mismatch, creating defaults");
        do_create = 1;
    } else {
        do_create = create;
    }

    changed = 0;
    if (do_create) {
        /* Use default values */
        ssh_mgmt_conf_get_default(&new_ssh_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->ssh_conf = new_ssh_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            new_ssh_conf = conf_blk_p->ssh_conf;
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
            // If the default setting is that ssh is enabled; see ssh_mgmt_conf_get_default().
            // That value isn't saved in conf, however, so we restore it here. The
            // reason is that since "ip ssh" is the default, it isn't generated by
            // running-config -- and hence not present in startup-config. Then the
            // value from the conf block wins -- and it's always "no ip ssh".
            new_ssh_conf.mode = SSH_MGMT_DEF_MODE;
#endif
        }
    }
    SSH_CRIT_ENTER();
    old_ssh_conf_p = &SSH_global.ssh_conf;
    if (ssh_mgmt_conf_changed(old_ssh_conf_p, &new_ssh_conf)) {
        changed = 1;
    }
    SSH_global.ssh_conf = new_ssh_conf;
    if (changed || create) {
        SSH_global.apply_init_conf = TRUE;
    }
    SSH_CRIT_EXIT();

    if (conf_blk_p == NULL) {
        T_W("failed to open SSH table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

#if defined(VTSS_SW_OPTION_CLI_TELNET) && defined(TELNET_SECURITY_SUPPORTED)
    telnet_set_security_mode(new_ssh_conf.mode);
#endif /* VTSS_SW_OPTION_CLI_TELNET & TELNET_SECURITY_SUPPORTED */

    T_D("exit");
}

/* SSH exist log callback function */
static void SSH_exist_log_callback(int priority, const char *format, va_list param)
{
    char printbuf[1024];

    (void) vsnprintf(printbuf, sizeof(printbuf), format, param);
    T_D("%s", printbuf);
}

/* Module start */
static void SSH_start(void)
{
    ssh_conf_t  *conf_p;

    T_D("enter");

    /* Create semaphore for critical regions */
    critd_init(&SSH_global.crit, "SSH_global.crit", VTSS_MODULE_ID_SSH, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    SSH_CRIT_EXIT();

    /* Register dropbear exist log callback function */
    dropbear_exist_log_register(SSH_exist_log_callback);

    /* Initialize SSH configuration */
    SSH_global.apply_init_conf = FALSE;
    conf_p = &SSH_global.ssh_conf;
    ssh_mgmt_conf_get_default(conf_p);

    /* Create SSH thread */
    cyg_thread_create(THREAD_BELOW_NORMAL_PRIO,
                      SSH_thread,
                      0,
                      "SSH Main",
                      SSH_thread_stack,
                      sizeof(SSH_thread_stack),
                      &SSH_thread_handle,
                      &SSH_thread_block);

    T_D("exit");
}

/**
  * \brief Initialize the SSH module
  *
  * \param cmd [IN]: Reason why calling this function.
  * \param p1  [IN]: Parameter 1. Usage varies with cmd.
  * \param p2  [IN]: Parameter 2. Usage varies with cmd.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc ssh_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;

#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc     rc = VTSS_OK;
#endif /* VTSS_SW_OPTION_ICFG */

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&SSH_trace_reg, SSH_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&SSH_trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        SSH_start();
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = vtss_ssh_icfg_init()) != VTSS_OK) {
            T_D("Calling vtss_ssh_icfg_init() failed rc = %s", error_txt(rc));
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
            SSH_conf_read_stack(1);
            SSH_conf_apply();
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

        /* Read stack and switch configuration */
        SSH_conf_read_stack(0);

        /* Starting SSH thread (became master) */
        cyg_thread_resume(SSH_thread_handle);
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

#if defined(VTSS_SW_OPTION_AUTH)
/**
  * \brief SSH user authentication register function
  *
  * \param cb [IN]: Callback function point.
  */
void ssh_user_auth_register(ssh_user_auth_callback_t cb)
{
#if SSH_USING_DROPBEAR_PACKAGE
    dropbear_auth_register((dropbear_auth_callback_t)cb);
#endif
}
#endif

/**
  * \brief Close all SSH session
  */
void ssh_close_all_session(void)
{
#if SSH_USING_DROPBEAR_PACKAGE
    dropbear_cli_child_socket_close();
#endif
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
