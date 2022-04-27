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

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "access_mgmt_api.h"
#include "vlan_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_ACCESS_MGMT,

    /* Group tags */
    CX_TAG_ACCESS_MGMT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t access_mgmt_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_ACCESS_MGMT] = {
        .name  = "access_mgmt",
        .descr = "Access Management",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_ACCESS_MGMT_TABLE] = {
        .name  = "access_mgmt_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
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

/* Access mgmt specific set state structure */
typedef struct {
    int access_mgmt_entries_count;
} access_mgmt_cx_set_state_t;

static vtss_rc access_mgmt_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL                mode, global;
        access_mgmt_conf_t  conf;
        int                 access_id;
        ulong               access_vid;
        ulong               service_type;
        vtss_ipv4_t         start_ipv4, end_ipv4;
        access_mgmt_entry_t entry;
        access_mgmt_cx_set_state_t *access_mgmt_state = s->mod_state;
#ifdef VTSS_SW_OPTION_IPV6
        vtss_ipv6_t         start_ipv6, end_ipv6;
#endif /* VTSS_SW_OPTION_IPV6 */

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        if (s->apply && access_mgmt_conf_get(&conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            if (cx_parse_val_bool(s, &mode, 1) == VTSS_OK && s->apply) {
                conf.mode = mode;
                CX_RC(access_mgmt_conf_set(&conf));
            }
            break;
        case CX_TAG_ACCESS_MGMT_TABLE:
            access_mgmt_state->access_mgmt_entries_count = 0;
            if (s->apply) {
                CX_RC(access_mgmt_entry_clear());
            }
            break;
        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_ACCESS_MGMT_TABLE) {
                memset(&entry, 0x0, sizeof(entry));

                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    u32 temp_mask;
                    if (cx_parse_long(s, "access_id", (long *)&access_id, ACCESS_MGMT_ACCESS_ID_START, ACCESS_MGMT_MAX_ENTRIES) == VTSS_OK) {
                        entry.valid = 1;
                    } else if (cx_parse_ulong(s, "access_vid", &access_vid, VLAN_ID_MIN, VLAN_ID_MAX) == VTSS_OK) {
                        entry.vid = (vtss_vid_t) access_vid;
                    } else if (cx_parse_ulong(s, "service_type", &service_type, 1, ACCESS_MGMT_SERVICES_TYPE) == VTSS_OK) {
                        entry.service_type = service_type;
                    } else if (cx_parse_ipv4(s, "start_ip", &start_ipv4, &temp_mask, 0) == VTSS_OK) {
                        entry.entry_type = ACCESS_MGMT_ENTRY_TYPE_IPV4;
                        entry.start_ip = start_ipv4;
                    } else if (cx_parse_ipv4(s, "end_ip", &end_ipv4, &temp_mask, 0) == VTSS_OK) {
                        if (entry.entry_type != ACCESS_MGMT_ENTRY_TYPE_IPV4) {
                            sprintf(s->msg, "Different IP type in same entry");
                            s->rc = CONF_XML_ERROR_FILE_PARM;
                        }
                        entry.end_ip = end_ipv4;
                    }
#ifdef VTSS_SW_OPTION_IPV6
                    else if (cx_parse_ipv6(s, "start_ip", &start_ipv6) == VTSS_OK) {
                        entry.entry_type = ACCESS_MGMT_ENTRY_TYPE_IPV6;
                        entry.start_ipv6 = start_ipv6;
                    } else if (cx_parse_ipv6(s, "end_ip", &end_ipv6) == VTSS_OK) {
                        if (entry.entry_type != ACCESS_MGMT_ENTRY_TYPE_IPV6) {
                            sprintf(s->msg, "Different IP type in same entry");
                            s->rc = CONF_XML_ERROR_FILE_PARM;
                        }
                        entry.end_ipv6 = end_ipv6;
                    }
#endif /* VTSS_SW_OPTION_IPV6 */
                    else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (s->rc != VTSS_OK) {
                    break;
                }

                if (entry.entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
                    if (entry.start_ip > entry.end_ip) {
                        vtss_ipv4_t start_ip;
                        start_ip = entry.start_ip;
                        entry.start_ip = entry.end_ip;
                        entry.end_ip = start_ip;
                        break;
                    }
                }
#ifdef VTSS_SW_OPTION_IPV6
                else {
                    vtss_ipv6_t temp_ipv6addr;
                    if (memcmp(&entry.start_ipv6, &entry.end_ipv6, sizeof(vtss_ipv6_t)) > 0) {
                        memcpy(&temp_ipv6addr, &entry.start_ipv6, sizeof(vtss_ipv6_t));
                        memcpy(&entry.start_ipv6, &entry.end_ipv6, sizeof(vtss_ipv6_t));
                        memcpy(&entry.start_ipv6, &temp_ipv6addr, sizeof(vtss_ipv6_t));
                    }
                }
#endif /* VTSS_SW_OPTION_IPV6 */

                access_mgmt_state->access_mgmt_entries_count++;
                if (access_mgmt_state->access_mgmt_entries_count > ACCESS_MGMT_MAX_ENTRIES) {
                    sprintf(s->msg, "The maximum access management entry number is %d", ACCESS_MGMT_MAX_ENTRIES);
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                    break;
                }

                if (s->apply) {
                    int duplicated_id;
                    if ((duplicated_id = access_mgmt_entry_content_is_duplicated(access_id, &entry)) != 0) {
                        sprintf(s->msg, "The entry content is duplicated of entry ID %d", duplicated_id);
                        s->rc = CONF_XML_ERROR_FILE_PARM;
                        break;
                    }
                    CX_RC(access_mgmt_entry_add(access_id, &entry));
                }
            } else {
                s->ignored = 1;
            }
            break;
        default:
            s->ignored = 1;
            break;
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

static vtss_rc access_mgmt_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - access_mgmt */
        T_D("global - access_mgmt");
        CX_RC(cx_add_tag_line(s, CX_TAG_ACCESS_MGMT, 0));
        {
            int                 access_id;
            access_mgmt_conf_t  conf;

            if (access_mgmt_conf_get(&conf) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, conf.mode));
                CX_RC(cx_add_tag_line(s, CX_TAG_ACCESS_MGMT_TABLE, 0));

                /* Entry syntax */
                CX_RC(cx_add_stx_start(s));
                CX_RC(cx_add_stx_ulong(s, "access_id", ACCESS_MGMT_ACCESS_ID_START, ACCESS_MGMT_MAX_ENTRIES));
                CX_RC(cx_add_stx_ulong(s, "access_vid", VLAN_ID_MIN, VLAN_ID_MAX));
                CX_RC(cx_add_stx_ipv4(s, "start_ip"));
                CX_RC(cx_add_stx_ipv4(s, "end_ip"));
                CX_RC(cx_add_stx_ulong(s, "service_type", 1, ACCESS_MGMT_SERVICES_TYPE));
                CX_RC(cx_add_stx_end(s));

                for (access_id = ACCESS_MGMT_ACCESS_ID_START;
                     access_id < ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES; access_id++) {
                    if (conf.entry[access_id].valid) {
                        CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                        CX_RC(cx_add_attr_ulong(s, "access_id", access_id));
                        CX_RC(cx_add_attr_ulong(s, "access_vid", conf.entry[access_id].vid));
                        if (conf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
                            CX_RC(cx_add_attr_ipv4(s, "start_ip", conf.entry[access_id].start_ip));
                            CX_RC(cx_add_attr_ipv4(s, "end_ip", conf.entry[access_id].end_ip));
                        }
#ifdef VTSS_SW_OPTION_IPV6
                        else {
                            CX_RC(cx_add_attr_ipv6(s, "start_ip", conf.entry[access_id].start_ipv6));
                            CX_RC(cx_add_attr_ipv6(s, "end_ip", conf.entry[access_id].end_ipv6));
                        }
#endif /* VTSS_SW_OPTION_IPV6 */
                        CX_RC(cx_add_attr_ulong(s, "service_type", conf.entry[access_id].service_type));
                        CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                    }
                }

                CX_RC(cx_add_tag_line(s, CX_TAG_ACCESS_MGMT_TABLE, 1));
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_ACCESS_MGMT, 1));
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
    VTSS_MODULE_ID_ACCESS_MGMT,
    access_mgmt_cx_tag_table,
    sizeof(access_mgmt_cx_set_state_t),
    0,
    NULL,                     /* init function       */
    access_mgmt_cx_gen_func,  /* Generation fucntion */
    access_mgmt_cx_parse_func /* parse fucntion      */
);

