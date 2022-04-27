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
#include "vtss_https_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_HTTPS

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_HTTPS,

    /* Parameter tags */
    CX_TAG_MODE,
    CX_TAG_MODE_REDIRECT,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t https_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_HTTPS] = {
        .name  = "https",
        .descr = "Hypertext Transfer Protocol over Secure Socket Layer",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE_REDIRECT] = {
        .name  = "redirect",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

static vtss_rc https_cx_parse_func(cx_set_state_t *s)
{
    https_conf_t *conf;

    if ((conf = VTSS_MALLOC(sizeof(https_conf_t))) == NULL) {
        return HTTPS_ERROR_INTERNAL_RESOURCE;
    }

    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL         mode;
        BOOL         global;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        if (s->apply && https_mgmt_conf_get(conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            if (cx_parse_val_bool(s, &mode, 1) == VTSS_OK) {
                conf->mode = mode;
            }
            break;
        case CX_TAG_MODE_REDIRECT:
            if (cx_parse_val_bool(s, &mode, 1) == VTSS_OK) {
                conf->redirect = mode;
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
            s->rc = https_mgmt_conf_set(conf);
        }
        break;
        } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL:
        break;
    case CX_PARSE_CMD_SWITCH:
        break;
    default:
        break;
    }

    VTSS_FREE(conf);

    return s->rc;
}

static vtss_rc https_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - HTTPS */
        T_D("global - https");
        CX_RC(cx_add_tag_line(s, CX_TAG_HTTPS, 0));
        {
            https_conf_t *conf = (https_conf_t *) VTSS_MALLOC(sizeof(https_conf_t));

            if (!conf) {
                return HTTPS_ERROR_INTERNAL_RESOURCE;
            }

            if (https_mgmt_conf_get(conf) == VTSS_OK) {
                (void) cx_add_val_bool(s, CX_TAG_MODE, conf->mode);
                (void) cx_add_val_bool(s, CX_TAG_MODE_REDIRECT, conf->redirect);
            }
            VTSS_FREE(conf);
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_HTTPS, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_HTTPS, https_cx_tag_table,
                    0, 0,
                    NULL,                    /* init function       */
                    https_cx_gen_func,       /* Generation fucntion */
                    https_cx_parse_func);    /* parse fucntion      */
