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
#include "port_api.h"
#include "psec_api.h"
#include "voice_vlan_api.h"
#include "mac_api.h" /* for MAC_AGE_TIME_MIN & MAC_AGE_TIME_MAX */

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "vlan_api.h"

#if defined(VTSS_FEATURE_QCL_V1)
static char *cx_prio_txt = "1-4 or low/normal/medium/high";
#else
static char *cx_prio_txt = "0-7";
#endif /* VTSS_FEATURE_QCL_V1 */

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_VOICE_VLAN,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_OUI_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_VID,
    CX_TAG_MODE,
    CX_TAG_AGETIME,
    CX_TAG_TRAFFIC_CLASS,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t voice_vlan_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_VOICE_VLAN] = {
        .name  = "voice_vlan",
        .descr = "Voice VLAN",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_OUI_TABLE] = {
        .name  = "oui_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_VID] = {
        .name  = "vid",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_AGETIME] = {
        .name  = "agetime",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_TRAFFIC_CLASS] = {
        .name  = "traffic_class",
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

/* Keyword for Voice VLAN port mode */
static const cx_kw_t cx_kw_voice_vlan_port_mode[] = {
    { "disabled",   VOICE_VLAN_PORT_MODE_DISABLED },
    { "auto",       VOICE_VLAN_PORT_MODE_AUTO },
    { "forced",     VOICE_VLAN_PORT_MODE_FORCED },
    { NULL,       0 }
};

#ifdef VTSS_SW_OPTION_LLDP
/* Lint determine it is a "459 error : whose address was taken has an
   unprotected access to variable. We called from the httpd thread, it
   is a single process at the same time therefore unprotected access is
   acceptable */
/*lint -esym(459,cx_kw_voice_vlan_discovery_protocol)*/
/* Keyword for Voice VLAN discovery protocol */
static cx_kw_t cx_kw_voice_vlan_discovery_protocol[] = {
    { "oui",    VOICE_VLAN_DISCOVERY_PROTOCOL_OUI },
    { "lldp",   VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP },
    { "both",   VOICE_VLAN_DISCOVERY_PROTOCOL_BOTH },
    { NULL,       0 }
};
#endif

/* Specific set state structure */
typedef struct {
    int ouis_count;
} voice_vlan_cx_set_state_t;

static BOOL cx_voice_vlan_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    voice_vlan_port_conf_t conf;

    if (voice_vlan_mgmt_port_conf_get(context->isid, &conf) == VTSS_OK) {
        return (conf.port_mode[port_a] == conf.port_mode[port_b]
                && conf.security[port_a] == conf.security[port_b]
#ifdef VTSS_SW_OPTION_LLDP
                && conf.discovery_protocol[port_a] == conf.discovery_protocol[port_b]
#endif
               );
    } else {
        return FALSE;
    }
}

static vtss_rc cx_voice_vlan_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    voice_vlan_port_conf_t  voice_vlan_port_conf;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_kw(s, "mode", cx_kw_voice_vlan_port_mode));
        CX_RC(cx_add_stx_bool(s, "security"));
#ifdef VTSS_SW_OPTION_LLDP
        CX_RC(cx_add_stx_kw(s, "discovery_protocol", cx_kw_voice_vlan_discovery_protocol));
#endif
        return cx_add_stx_end(s);
    }

    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(voice_vlan_mgmt_port_conf_get(context->isid, &voice_vlan_port_conf));
    CX_RC(cx_add_attr_kw(s, "mode", cx_kw_voice_vlan_port_mode, voice_vlan_port_conf.port_mode[port_no]));
    CX_RC(cx_add_attr_bool(s, "security", voice_vlan_port_conf.security[port_no]));
#ifdef VTSS_SW_OPTION_LLDP
    CX_RC(cx_add_attr_kw(s, "discovery_protocol", cx_kw_voice_vlan_discovery_protocol, voice_vlan_port_conf.discovery_protocol[port_no]));
#endif

    return cx_add_port_end(s, CX_TAG_ENTRY);
}

/* Parse OUI address */
static vtss_rc cx_parse_oui_addr(cx_set_state_t *s, char *name, uchar *oui_addr)
{
    char buf[32];
    int  i, j, c;

    CX_RC(cx_parse_txt(s, name, buf, sizeof(buf)));
    if (strlen(buf) != 8) {
        CX_RC(cx_parm_invalid(s));
    }
    for (i = 0; i < 8; i++) {
        j = (i % 3);
        c = tolower(buf[i]);
        if (j == 2) {
            if (c != '-') {
                return cx_parm_invalid(s);
            }
        } else {
            if (!isxdigit(c)) {
                return cx_parm_invalid(s);
            }
            c = (isdigit(c) ? (c - '0') : (10 + c - 'a'));
            if (j == 0) {
                oui_addr[i / 3] = c * 16;
            } else {
                oui_addr[i / 3] += c;
            }
        }
    }
    return s->rc;
}

static vtss_rc voice_vlan_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        port_iter_t    pit;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE + 1];
        ulong          val;
        BOOL           global;
        voice_vlan_conf_t         conf;
#if defined(VOICE_VLAN_CLASS_SUPPORTED)
        vtss_prio_t               prio = VTSS_PRIO_END;
#endif /* VOICE_VLAN_CLASS_SUPPORTED */
        voice_vlan_oui_entry_t    entry;
        voice_vlan_cx_set_state_t *voice_vlan_state = s->mod_state;

        global = (s->isid == VTSS_ISID_GLOBAL);

        if (global && s->apply && voice_vlan_mgmt_conf_get(&conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            if (global) {
                CX_RC(cx_parse_val_bool(s, &conf.mode, 1));
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_VID:
            if (global &&
                cx_parse_val_ulong(s, &val, VLAN_ID_MIN, VLAN_ID_MAX) == VTSS_OK &&
                !VOICE_VLAN_is_valid_voice_vid(val)) {
                conf.vid = val;
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_AGETIME:
            if (global && cx_parse_val_ulong(s, &val, PSEC_AGE_TIME_MIN, PSEC_AGE_TIME_MAX) == VTSS_OK) {
                conf.age_time = val;
            } else {
                s->ignored = 1;
            }
            break;
#if defined(VOICE_VLAN_CLASS_SUPPORTED)
        case CX_TAG_TRAFFIC_CLASS:
            if (global && cx_parse_class(s, "val", &prio) == VTSS_OK) {
                conf.traffic_class = prio;
            } else {
                s->ignored = 1;
            }
            break;
#endif /* VOICE_VLAN_CLASS_SUPPORTED */
        case CX_TAG_OUI_TABLE:
            if (global) {
                voice_vlan_state->ouis_count = 0;
                if (s->apply) {
                    CX_RC(voice_vlan_oui_entry_clear());
                }
            } else {
                s->ignored = 1;
            }
            break;
        case CX_TAG_PORT_TABLE:
            if (global) {
                s->ignored = 1;
            }
            break;
        case CX_TAG_ENTRY:
            if (global && s->group == CX_TAG_OUI_TABLE) {
                memset(&entry, 0x0, sizeof(entry));

                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_oui_addr(s, "telephony_oui", entry.oui_addr) == VTSS_OK) {
                        //do nothing
                    } else if (!cx_parse_txt(s, "description", entry.description, sizeof(entry.description)) == VTSS_OK) {
                        CX_RC(cx_parm_unknown(s));
                    }
                }

                voice_vlan_state->ouis_count++;
                if (voice_vlan_state->ouis_count > VOICE_VLAN_OUI_ENTRIES_CNT) {
                    sprintf(s->msg, "The maximum entry count is %d", VOICE_VLAN_OUI_ENTRIES_CNT);
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                    break;
                }

                if (s->apply) {
                    entry.valid = 1;
                    CX_RC(voice_vlan_oui_entry_add(&entry));
                }
            } else if (!global && s->group == CX_TAG_PORT_TABLE && cx_parse_ports(s, port_list, 1) == VTSS_OK) {
                /* Handle port table in switch section */
                voice_vlan_port_conf_t  voice_vlan_port_conf;
                ulong                   port_mode = VOICE_VLAN_MGMT_DEFAULT_PORT_MODE, discovery_protocol = VOICE_VLAN_MGMT_DEFAULT_DISCOVERY_PROTOCOL;
                BOOL                    security = VOICE_VLAN_MGMT_DEFAULT_SECURITY, found_port_mode = FALSE, found_security = FALSE, found_discovery_protocol = FALSE;

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_kw(s, "mode", cx_kw_voice_vlan_port_mode, &port_mode, 1) == VTSS_OK) {
                        found_port_mode = TRUE;
                    } else if (cx_parse_bool(s, "security", &security, 1) == VTSS_OK) {
                        found_security = TRUE;
                    }
#ifdef VTSS_SW_OPTION_LLDP
                    else if (cx_parse_kw(s, "discovery_protocol", cx_kw_voice_vlan_discovery_protocol, &discovery_protocol, 1) == VTSS_OK) {
                        found_discovery_protocol = TRUE;
                    }
#endif
                    else {
                        CX_RC(cx_parm_unknown(s));
                    }
                } /* for loop */

                if (s->apply && voice_vlan_mgmt_port_conf_get(s->isid, &voice_vlan_port_conf) == VTSS_OK) {
                    (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
                    while (port_iter_getnext(&pit)) {
                        if (!port_list[pit.uport]) {
                            continue;
                        }
                        if (found_port_mode) {
                            voice_vlan_port_conf.port_mode[pit.iport] = port_mode;
                        }
                        if (found_security) {
                            voice_vlan_port_conf.security[pit.iport] = security;
                        }
                        if (found_discovery_protocol) {
                            voice_vlan_port_conf.discovery_protocol[pit.iport] = discovery_protocol;
                        }
                    }
                    CX_RC(voice_vlan_mgmt_port_conf_set(s->isid, &voice_vlan_port_conf));
                }
            } else {
                s->ignored = 1;
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (global && s->apply) {
            CX_RC(voice_vlan_mgmt_conf_set(&conf));
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

static vtss_rc voice_vlan_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - voice_vlan */
        T_D("global - voice_vlan");
        CX_RC(cx_add_tag_line(s, CX_TAG_VOICE_VLAN, 0));
        {
            voice_vlan_conf_t       conf;
            voice_vlan_oui_entry_t  entry;
            char                    buf[128];

            CX_RC(voice_vlan_mgmt_conf_get(&conf));
            CX_RC(cx_add_val_bool(s, CX_TAG_MODE, conf.mode));
            CX_RC(cx_add_val_ulong(s, CX_TAG_VID, conf.vid, VLAN_ID_MIN, VLAN_ID_MAX));
            sprintf(buf, "%u-%u or 0 (disabled)", MAC_AGE_TIME_MIN, MAC_AGE_TIME_MAX);
            CX_RC(cx_add_val_ulong(s, CX_TAG_AGETIME, conf.age_time, PSEC_AGE_TIME_MIN, PSEC_AGE_TIME_MAX));
#if defined(VOICE_VLAN_CLASS_SUPPORTED)
            sprintf(buf, "%s", cx_prio_txt);
            CX_RC(cx_add_val_ulong_stx(s, CX_TAG_TRAFFIC_CLASS, iprio2uprio(conf.traffic_class), buf));
#endif /* VOICE_VLAN_CLASS_SUPPORTED */

            CX_RC(cx_add_tag_line(s, CX_TAG_OUI_TABLE, 0));

            /* Entry syntax */
            CX_RC(cx_add_stx_start(s));
            CX_RC(cx_add_attr_txt(s, "telephony_oui", "xx-xx-xx (hexadecimal digit characters)"));
            sprintf(buf, "The description of OUI address. The allowed string length is 0 to %d.",
                    VOICE_VLAN_MAX_DESCRIPTION_LEN);
            CX_RC(cx_add_attr_txt(s, "description", buf));
            CX_RC(cx_add_stx_end(s));

            memset(&entry, 0x0, sizeof(entry));
            while (voice_vlan_oui_entry_get(&entry, TRUE) == VTSS_OK) {
                CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                CX_RC(cx_add_attr_txt(s, "telephony_oui", misc_oui_addr_txt(entry.oui_addr, buf)));
                CX_RC(cx_add_attr_txt(s, "description", entry.description));
                CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            };

            CX_RC(cx_add_tag_line(s, CX_TAG_OUI_TABLE, 1));

        }
        CX_RC(cx_add_tag_line(s, CX_TAG_VOICE_VLAN, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - Voice VLAN */
        T_D("switch - voice_vlan");
        CX_RC(cx_add_tag_line(s, CX_TAG_VOICE_VLAN, 0));
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_voice_vlan_match, cx_voice_vlan_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_VOICE_VLAN, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_VOICE_VLAN,
    voice_vlan_cx_tag_table,
    sizeof(voice_vlan_cx_set_state_t),
    0,
    NULL,                         /* init function       */
    voice_vlan_cx_gen_func,       /* Generation fucntion */
    voice_vlan_cx_parse_func      /* parse fucntion      */
);

