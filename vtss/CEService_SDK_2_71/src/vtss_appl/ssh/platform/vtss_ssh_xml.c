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
#include "vtss_ssh_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_SSH,

    /* Parameter tags */
    CX_TAG_MODE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t ssh_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_SSH] = {
        .name  = "ssh",
        .descr = "Secure Shell",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
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

static vtss_rc ssh_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL           global;
        ssh_conf_t     conf;
        BOOL           mode;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        if (s->apply && ssh_mgmt_conf_get(&conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            CX_RC(cx_parse_val_bool(s, &mode, 1));
            conf.mode = mode;
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
            CX_RC(ssh_mgmt_conf_set(&conf));
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

    return s->rc;
}

static vtss_rc ssh_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - SSH */
        T_D("global - ssh");
        CX_RC(cx_add_tag_line(s, CX_TAG_SSH, 0));
        {
            ssh_conf_t conf;

            if (ssh_mgmt_conf_get(&conf) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, conf.mode));
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_SSH, 1));
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
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_SSH,
    ssh_cx_tag_table,
    0,
    0,
    NULL,                /* init function       */
    ssh_cx_gen_func,     /* Generation fucntion */
    ssh_cx_parse_func    /* parse fucntion      */
);

