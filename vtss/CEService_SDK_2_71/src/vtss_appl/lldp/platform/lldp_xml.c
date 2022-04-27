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
#include "lldp_api.h"
#include "lldp_tlv.h"

#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_rx.h"
#endif

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "mgmt_api.h"
#include "misc_api.h"
#include "topo_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_LLDP,
#ifdef VTSS_SW_OPTION_LLDP_MED
    CX_TAG_LLDPMED,
#endif

    /* Group tags */
    CX_TAG_PORT_TABLE,
#ifdef VTSS_SW_OPTION_LLDP_MED
    CX_TAG_GROUP_TABLE,
    CX_TAG_POLICIES_TABLE,
#endif

    /* Parameter tags */
#ifdef VTSS_SW_OPTION_LLDP_MED
    CX_TAG_FAST_START_REPEAT_COUNT,
    CX_TAG_LATITUDE,
    CX_TAG_LATITUDE_DIR,
    CX_TAG_LONGITUDE_DIR,
    CX_TAG_LONGITUDE,
    CX_TAG_ALTITUDE,
    CX_TAG_ALTITUDE_TYPE,
    CX_TAG_DATUM,
    CX_TAG_COUNTRY,
    CX_TAG_CIVIC,
    CX_TAG_ECS,
    CX_TAG_POLICY_ENTRY,
    CX_TAG_POLICY,
    CX_TAG_APPLICATION_TYPE,
#endif
    CX_TAG_ENTRY,
    CX_TAG_DELAY,
    CX_TAG_INTERVAL,
    CX_TAG_HOLD,
    CX_TAG_REINIT,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t lldp_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_LLDP] = {
        .name  = "lldp",
        .descr = "Link Layer Discovery Protocol",
        .type = CX_TAG_TYPE_MODULE
    },
#ifdef VTSS_SW_OPTION_LLDP_MED
    [CX_TAG_LLDPMED] = {
        .name  = "lldpmed",
        .descr = "Link Layer Discovery Protocol Media",
        .type = CX_TAG_TYPE_MODULE
    },
#endif
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
#ifdef VTSS_SW_OPTION_LLDP_MED
    [CX_TAG_GROUP_TABLE] = {
        .name  = "group_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_POLICIES_TABLE] = {
        .name  = "policies_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_FAST_START_REPEAT_COUNT] = {
        .name  = "fast_start_repeat_count",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LATITUDE] = {
        .name  = "latitude",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LATITUDE_DIR] = {
        .name  = "latitude_direction",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LONGITUDE] = {
        .name  = "longitude",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_LONGITUDE_DIR] = {
        .name  = "longitude_direction",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_ALTITUDE] = {
        .name  = "altitude",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_ALTITUDE_TYPE] = {
        .name  = "altitude_type",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DATUM] = {
        .name  = "altitude_datum",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_APPLICATION_TYPE] = {
        .name  = "application_type",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_COUNTRY] = {
        .name  = "location_country",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_ECS] = {
        .name  = "location_ecs",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_CIVIC] = {
        .name  = "location_civic",
        .descr = "Location Civic Address Information",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_POLICY_ENTRY] = {
        .name  = "policy_entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_POLICY] = {
        .name  = "policies",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
#endif
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DELAY] = {
        .name  = "delay",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_INTERVAL] = {
        .name  = "interval",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_HOLD] = {
        .name  = "hold",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_REINIT] = {
        .name  = "reinit",
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

/* LLDP specific set state structure */
typedef struct {
    lldp_struc_0_t conf;      // Configuration persistent across calls to parser func
    cx_line_t      line_lldp; // Line number used if inter-variable check fails
} lldp_cx_set_state_t;

/* Keyword for LLDP mode */
static const cx_kw_t cx_kw_lldp_mode[] = {
    { "enabled",  LLDP_ENABLED_RX_TX },
    { "disabled", LLDP_DISABLED },
    { "rx",       LLDP_ENABLED_RX_ONLY },
    { "tx",       LLDP_ENABLED_TX_ONLY },
    { NULL,       0 }
};

#ifdef VTSS_SW_OPTION_LLDP_MED
static const cx_kw_t cx_kw_lldpmed_latitude_dir[] = {
    { "North", NORTH},
    { "South", SOUTH },
    { NULL,       0 }
};
static const cx_kw_t cx_kw_lldpmed_longitude_dir[] = {
    { "East", EAST},
    { "West", WEST },
    { NULL,       0 }
};

static const cx_kw_t cx_kw_lldpmed_altitude_type[] = {
    { "Meters", METERS},
    { "Floor",   FLOOR},
    { NULL,       0 }
};

static const cx_kw_t cx_kw_lldpmed_datum[] = {
    { "WGS84", WGS84},
    { "NAD83_NAVD88", NAD83_NAVD88},
    { "NAD83_MLLW", NAD83_MLLW},
    { NULL, 0 }
};

static const cx_kw_t cx_kw_lldpmed_application_type[] = {
    { "voice_signaling", VOICE_SIGNALING},
    { "guest_voice_signaling", GUEST_VOICE_SIGNALING},
    { "plain_voice", VOICE},
    { "plain_guest_voice", GUEST_VOICE},
    { "softphone_voice", SOFTPHONE_VOICE},
    { "video_conferencing", VIDEO_CONFERENCING},
    { "streaming_video", STREAMING_VIDEO},
    { "video_signaling", VIDEO_SIGNALING},
    { NULL, 0 }
};
#endif

#define CX_LLDP_TLV(isid, port, tlv) lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_##tlv, port, isid)
#define CX_LLDP_MATCH(isid, port_a, port_b, tlv) CX_LLDP_TLV(isid, port_a, tlv) == CX_LLDP_TLV(isid, port_b, tlv)

static BOOL cx_lldp_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    lldp_struc_0_t conf;

    lldp_mgmt_get_config(&conf, context->isid);
    return (conf.admin_state[port_a] == conf.admin_state[port_b] &&
#ifdef VTSS_SW_OPTION_CDP
            conf.cdp_aware[port_a] == conf.cdp_aware[port_b] &&
#endif // VTSS_SW_OPTION_CDP
            CX_LLDP_MATCH(context->isid, port_a, port_b, PORT_DESCR) &&
            CX_LLDP_MATCH(context->isid, port_a, port_b, SYSTEM_NAME) &&
            CX_LLDP_MATCH(context->isid, port_a, port_b, SYSTEM_DESCR) &&
            CX_LLDP_MATCH(context->isid, port_a, port_b, SYSTEM_CAPA) &&
            CX_LLDP_MATCH(context->isid, port_a, port_b, MGMT_ADDR));
}

static vtss_rc cx_lldp_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    lldp_struc_0_t conf;

    if (ports == NULL) {
        /* Syntax */
        (void) cx_add_stx_start(s);
        (void) cx_add_stx_port(s);
#ifdef VTSS_SW_OPTION_CDP
        (void) cx_add_stx_bool(s, "cdp_aware");
#endif // VTSS_SW_OPTION_CDP
        (void) cx_add_stx_kw(s, "mode", cx_kw_lldp_mode);
        (void) cx_add_stx_bool(s, "port_descr");
        (void) cx_add_stx_bool(s, "sys_name");
        (void) cx_add_stx_bool(s, "sys_descr");
        (void) cx_add_stx_bool(s, "sys_capa");
        (void) cx_add_stx_bool(s, "mgmt_addr");
        return cx_add_stx_end(s);
    }

    lldp_mgmt_get_config(&conf, context->isid);

    (void) cx_add_port_start(s, CX_TAG_ENTRY, ports);
    (void) cx_add_attr_kw(s, "mode", cx_kw_lldp_mode, conf.admin_state[port_no]);
#ifdef VTSS_SW_OPTION_CDP
    (void) cx_add_attr_bool(s, "cdp_aware", conf.cdp_aware[port_no]);
#endif // VTSS_SW_OPTION_CDP
    (void) cx_add_attr_bool(s, "port_descr", CX_LLDP_TLV(context->isid, port_no, PORT_DESCR));
    (void) cx_add_attr_bool(s, "sys_name", CX_LLDP_TLV(context->isid, port_no, SYSTEM_NAME));
    (void) cx_add_attr_bool(s, "sys_descr", CX_LLDP_TLV(context->isid, port_no, SYSTEM_DESCR));
    (void) cx_add_attr_bool(s, "sys_capa", CX_LLDP_TLV(context->isid, port_no, SYSTEM_CAPA));
    (void) cx_add_attr_bool(s, "mgmt_addr", CX_LLDP_TLV(context->isid, port_no, MGMT_ADDR));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

#ifdef VTSS_SW_OPTION_LLDP_MED
static BOOL cx_lldp_med_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    lldp_struc_0_t conf;
    lldp_mgmt_get_config(&conf, context->isid);
    T_RG(VTSS_TRACE_GRP_LLDP, "match  = %d, port_a = %u, port_b= %u", memcmp(conf.ports_policies[port_a], conf.ports_policies[port_b], LLDPMED_POLICIES_CNT) == 0, port_a, port_b);
    return (memcmp(conf.ports_policies[port_a], conf.ports_policies[port_b], LLDPMED_POLICIES_CNT) == 0);
}

/* This is the same as in lldpmed_cli.c but we must be able to build without the _cli file */
static char *lldpmed_list2txt(BOOL *list, int min, int max, char *buf)
{
    int  i, first = 1, count = 0;
    BOOL member;
    char *p;

    p = buf;
    *p = '\0';
    for (i = min; i <= max; i++) {
        member = list[i];
        if ((member && (count == 0 || i == max)) || (!member && count > 1)) {
            p += sprintf(p, "%s%d",
                         first ? "" : count > (member ? 1 : 2) ? "-" : ",",
                         member ? (i) : i - 1);
            first = 0;
        }
        if (member) {
            count++;
        } else {
            count = 0;
        }
    }
    return buf;
}

static vtss_rc cx_lldp_med_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    lldp_struc_0_t conf;
    char buf[80];
    if (ports == NULL) {

        /* Syntax */
        (void) cx_add_stx_start(s);
        (void) cx_add_stx_port(s);
        (void) cx_add_attr_txt(s, "port_policies", cx_list_txt(buf, LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX));
        return cx_add_stx_end(s);
    }

    lldp_mgmt_get_config(&conf, context->isid);

    (void) cx_add_port_start(s, CX_TAG_ENTRY, ports);
    T_DG(VTSS_TRACE_GRP_LLDP, "port_NO = %u", port_no);
    (void) lldpmed_list2txt(&conf.ports_policies[port_no][0], LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX, buf);
    (void) cx_add_attr_txt(s, "port_policies", &buf[0]);

    return cx_add_port_end(s, CX_TAG_ENTRY);
}
#endif /* VTSS_SW_OPTION_LLDP_MED */

static vtss_rc lldp_cx_parse_func(cx_set_state_t *s)
{
    lldp_cx_set_state_t *lldp_state = s->mod_state;

    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        port_iter_t    pit;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE + 1];
        ulong          val;
        BOOL           global = (s->isid == VTSS_ISID_GLOBAL);

        if (s->mod_tag == CX_TAG_LLDP) {
            switch (s->id) {
            case CX_TAG_INTERVAL:
                if (global && cx_parse_val_ulong(s, &val, 5, 32768) == VTSS_RC_OK) {
                    lldp_state->conf.msgTxInterval = val;
                    lldp_state->line_lldp = s->line; // For inter-variable dependency check.
                } else {
                    s->ignored = 1;
                }
                break;
            case CX_TAG_HOLD:
                if (global && cx_parse_val_ulong(s, &val, 2, 10) == VTSS_RC_OK) {
                    lldp_state->conf.msgTxHold = val;
                } else {
                    s->ignored = 1;
                }
                break;
            case CX_TAG_DELAY:
                if (global && cx_parse_val_ulong(s, &val, 1, 8192) == VTSS_RC_OK) {
                    lldp_state->conf.txDelay = val;
                    lldp_state->line_lldp = s->line; // For inter-variable dependency check.
                } else {
                    s->ignored = 1;
                }
                break;
            case CX_TAG_REINIT:
                if (global && cx_parse_val_ulong(s, &val, 1, 10) == VTSS_RC_OK) {
                    lldp_state->conf.reInitDelay = val;
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
                if (!global && s->group == CX_TAG_PORT_TABLE && cx_parse_ports(s, port_list, 1) == VTSS_RC_OK) {
                    uchar state = 0;

                    BOOL mode = FALSE, p_descr = FALSE, s_name = FALSE, s_descr = FALSE, s_capa = FALSE, m_addr = FALSE;
                    BOOL tlv_p_descr = FALSE, tlv_s_name = FALSE, tlv_s_descr = FALSE, tlv_s_capa = FALSE, tlv_m_addr = FALSE;

#ifdef VTSS_SW_OPTION_CDP
                    BOOL tlv_c_aware;
                    BOOL cdp_aware = 0;
                    uchar cdp_aware_val = 0;
#endif // VTSS_SW_OPTION_CDP
                    s->p = s->next;
                    for (; s->rc == VTSS_RC_OK && cx_parse_attr(s) == VTSS_RC_OK; s->p = s->next) {
                        if (cx_parse_kw(s, "mode", cx_kw_lldp_mode, &val, 1) == VTSS_RC_OK) {
                            mode = 1;
                            state = val;
#ifdef VTSS_SW_OPTION_CDP
                        } else if (cx_parse_bool(s, "cdp_aware", &tlv_c_aware, 1) == VTSS_RC_OK) {
                            cdp_aware = 1;
                            cdp_aware_val = tlv_c_aware;
                            T_DG(VTSS_TRACE_GRP_LLDP, "cdp_aware_val = %d", cdp_aware_val);
#endif // VTSS_SW_OPTION_CDP
                        } else if (cx_parse_bool(s, "port_descr", &tlv_p_descr, 1) == VTSS_RC_OK) {
                            p_descr = 1;
                        } else if (cx_parse_bool(s, "sys_name", &tlv_s_name, 1) == VTSS_RC_OK) {
                            s_name = 1;
                        } else if (cx_parse_bool(s, "sys_descr", &tlv_s_descr, 1) == VTSS_RC_OK) {
                            s_descr = 1;
                        } else if (cx_parse_bool(s, "sys_capa", &tlv_s_capa, 1) == VTSS_RC_OK) {
                            s_capa = 1;
                        } else if (cx_parse_bool(s, "mgmt_addr", &tlv_m_addr, 1) == VTSS_RC_OK) {
                            m_addr = 1;
                        } else {
                            (void)cx_parm_unknown(s);
                        }
                    }

                    /* Port setup */
                    (void)port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
                    while (port_iter_getnext(&pit)) {
                        if (port_list[iport2uport(pit.iport)]) {
                            if (mode) {
                                lldp_state->conf.admin_state[pit.iport] = state;
                            }
#ifdef VTSS_SW_OPTION_CDP
                            if (cdp_aware) {
                                lldp_state->conf.cdp_aware[pit.iport] = cdp_aware_val;
                                T_DG_PORT(VTSS_TRACE_GRP_LLDP, pit.iport, "conf.cdp_aware = %d", lldp_state->conf.cdp_aware[pit.iport]);
                            }
#endif // VTSS_SW_OPTION_CDP

                            if (p_descr) {
                                lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_PORT_DESCR,   tlv_p_descr, &lldp_state->conf, pit.iport);
                            }
                            if (s_name) {
                                lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_NAME,  tlv_s_name,  &lldp_state->conf, pit.iport);
                            }
                            if (s_descr) {
                                lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR, tlv_s_descr, &lldp_state->conf, pit.iport);
                            }
                            if (s_capa) {
                                lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA,  tlv_s_capa,  &lldp_state->conf, pit.iport);
                            }
                            if (m_addr) {
                                lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_MGMT_ADDR,    tlv_m_addr,  &lldp_state->conf, pit.iport);
                            }
                        }
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
        } /* CX_TAG_LLDP */
#ifdef VTSS_SW_OPTION_LLDP_MED
        else if (s->mod_tag == CX_TAG_LLDPMED) {
            long val_long;
            char buf[256];
            char str_buf[32];
            u16  policy_index;
            BOOL policies_list[LLDPMED_POLICIES_CNT];
            BOOL policy_id_found = FALSE;

            switch (s->id) {
            case CX_TAG_FAST_START_REPEAT_COUNT:
                if (global && cx_parse_val_ulong(s, &val, FAST_START_REPEAT_COUNT_MIN, FAST_START_REPEAT_COUNT_MAX) == VTSS_RC_OK) {
                    T_DG(VTSS_TRACE_GRP_LLDP, "fast_start %u", val);
                    lldp_state->conf.medFastStartRepeatCount = (lldp_u8_t)val;
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_LATITUDE:
                if (global && cx_parse_val_txt(s, str_buf, 10) == VTSS_RC_OK) {
                    // Convert string to long. Ignore if converion fails
                    if (mgmt_str_float2long(str_buf, &val_long, LLDPMED_LATITUDE_VALUE_MIN, LLDPMED_LATITUDE_VALUE_MAX, TUDE_DIGIT) == VTSS_RC_OK) {
                        T_DG(VTSS_TRACE_GRP_LLDP, "latitude = %ld", val_long);
                        lldp_state->conf.location_info.latitude = val_long;
                    } else {
                        s->ignored = 1;
                    }
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_LATITUDE_DIR:
                if (global && cx_parse_kw(s, "latitude_dir", cx_kw_lldpmed_latitude_dir, &val, 1) == VTSS_RC_OK) {
                    T_DG(VTSS_TRACE_GRP_LLDP, "latitude_dir = %u", val);
                    lldp_state->conf.location_info.latitude_dir = val;
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_LONGITUDE_DIR:
                if (global && cx_parse_kw(s, "longitude_dir", cx_kw_lldpmed_longitude_dir, &val, 1) == VTSS_RC_OK) {
                    lldp_state->conf.location_info.longitude_dir = val;
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_ALTITUDE_TYPE:
                if (global && cx_parse_kw(s, "altitude_type", cx_kw_lldpmed_altitude_type, &val, 1) == VTSS_RC_OK) {
                    lldp_state->conf.location_info.altitude_type = val;
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_DATUM:
                if (global && cx_parse_kw(s, "datum", cx_kw_lldpmed_datum, &val, 1) == VTSS_RC_OK) {
                    lldp_state->conf.location_info.datum = val;
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_LONGITUDE:
                if (global && cx_parse_val_txt(s, str_buf, 10) == VTSS_RC_OK && mgmt_str_float2long(str_buf, &val_long, LLDPMED_LONGITUDE_VALUE_MIN, LLDPMED_LONGITUDE_VALUE_MAX, TUDE_DIGIT) == VTSS_RC_OK) {
                    lldp_state->conf.location_info.longitude = val_long;
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_COUNTRY:
                if (global && cx_parse_val_txt(s, str_buf, 3) == VTSS_RC_OK) {
                    strcpy(lldp_state->conf.location_info.ca_country_code, str_buf);
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_ECS:
                if (global && cx_parse_val_txt_numbers_only(s, str_buf, ECS_VALUE_LEN_MAX + 1) == VTSS_RC_OK) {
                    misc_strncpyz(lldp_state->conf.location_info.ecs, str_buf, ECS_VALUE_LEN_MAX);
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_ALTITUDE:
                if (global && cx_parse_val_txt(s, str_buf, 10) == VTSS_RC_OK && mgmt_str_float2long(str_buf, &val_long, LLDPMED_ALTITUDE_VALUE_MIN, LLDPMED_ALTITUDE_VALUE_MAX, TUDE_DIGIT) == VTSS_RC_OK) {
                    lldp_state->conf.location_info.altitude = val_long;
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_POLICIES_TABLE:
                if (global) {
                    T_DG(VTSS_TRACE_GRP_LLDP, "CX_TAG_POLICIES_TABLE");
                    // Delete all policies, so we can start at a known point.
                    for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
                        lldp_state->conf.policies_table[policy_index].in_use = 0;
                    }
                } else {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_GROUP_TABLE:
                if (!global) {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_PORT_TABLE:
                if (global) {
                    s->ignored = 1;
                }
                break;

            case CX_TAG_ENTRY:
                if (global && s->group == CX_TAG_GROUP_TABLE) {
                    char group_name_buf[32];
                    char ca_type_str[32];
                    BOOL civic_name = FALSE, value = FALSE;
                    u16 ca_index;

                    for (; s->rc == VTSS_RC_OK && cx_parse_attr(s) == VTSS_RC_OK; s->p = s->next) {
                        if (cx_parse_txt(s, "group_name", group_name_buf, 32) == VTSS_RC_OK) {
                            civic_name = 1;
                            T_DG(VTSS_TRACE_GRP_LLDP, "civic_name =1");
                        } else if (cx_parse_txt(s, "val" , str_buf, 32) == VTSS_RC_OK) {

                            value = 1;

                            for (ca_index = 0 ; ca_index < LLDPMED_CATYPE_CNT; ca_index ++) {
                                lldpmed_catype2str(lldpmed_index2catype(ca_index), &ca_type_str[0]);
                                T_RG(VTSS_TRACE_GRP_LLDP, "str_buf = %s, group_name_buf = %s, ca_type_str = %s, strcmp(buf,ca_type_str) = %d, ca_index = %d", str_buf, group_name_buf, ca_type_str, strcmp(group_name_buf, ca_type_str), ca_index);
                                if (strcmp(group_name_buf, ca_type_str) == 0) {
                                    lldpmed_update_civic_info(&lldp_state->conf.location_info.civic, lldpmed_index2catype(ca_index), str_buf);
                                    break;
                                }
                            }
                        }
                    }
                    if (civic_name == 0) {
                        (void)cx_parm_found_error(s, "civic_name");
                    }
                    if (value == 0) {
                        (void)cx_parm_found_error(s, "val");
                    }
                } else if (global && s->group == CX_TAG_POLICIES_TABLE) {
                    BOOL                       application_type_found = FALSE, tag_found = FALSE, vid_found = FALSE;
                    BOOL                       l2_prio_found = FALSE, dscp_found = FALSE;
                    BOOL                       tag_val = FALSE;
                    char                       policy_id_buf[64];
                    lldp_u16_t                 vid_val = 0;
                    lldp_u8_t                  l2_priority_val = 0;
                    lldp_u8_t                  dscp_val = 0;
                    lldpmed_application_type_t application_type_val = VOICE; // Default value

                    memset(policies_list, 0, sizeof(policies_list));

                    for (; s->rc == VTSS_RC_OK && cx_parse_attr(s) == VTSS_RC_OK; s->p = s->next) {
                        if (cx_parse_txt(s, "policy_id", policy_id_buf, sizeof(policy_id_buf)) == VTSS_RC_OK) {
                            if (mgmt_txt2list(policy_id_buf, policies_list, LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX, 0) != VTSS_RC_OK) {
                                T_WG(VTSS_TRACE_GRP_LLDP, "Problem converting policies id to list");
                            } else {
                                policy_id_found = TRUE;
                            }
                            T_NG(VTSS_TRACE_GRP_LLDP, "policy_id_found, policy_id_buf = %s", policy_id_buf);
                        } else if (cx_parse_kw(s, "application_type", cx_kw_lldpmed_application_type, &val, 1) == VTSS_RC_OK) {
                            T_NG(VTSS_TRACE_GRP_LLDP, "application_type_found");
                            application_type_found = TRUE;
                            application_type_val = val;
                        } else if (cx_parse_bool(s, "tag", &tag_val, 1) == VTSS_RC_OK) {
                            tag_found = TRUE;
                            T_NG(VTSS_TRACE_GRP_LLDP, "tag_found = %d", tag_found);
                        } else if (cx_parse_ulong(s, "vid", &val, LLDPMED_VID_MIN, LLDPMED_VID_MAX) == VTSS_RC_OK) {
                            vid_found = TRUE;
                            vid_val  = val;
                        } else if (cx_parse_ulong(s, "l2_prio", &val, LLDPMED_L2_PRIORITY_MIN, LLDPMED_L2_PRIORITY_MAX) == VTSS_RC_OK) {
                            l2_prio_found = TRUE;
                            l2_priority_val  = val;
                        } else if (cx_parse_ulong(s, "dscp", &val, LLDPMED_DSCP_MIN, LLDPMED_DSCP_MAX) == VTSS_RC_OK) {
                            dscp_found = TRUE;
                            dscp_val  = val;
                        } else {
                            (void)cx_parm_unknown(s);
                            T_IG(VTSS_TRACE_GRP_LLDP, "parse result not found");
                        }
                    }

                    if (policy_id_found == FALSE) {
                        (void)cx_parm_found_error(s, "policy_id");
                        break;
                    }

                    for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
                        T_NG(VTSS_TRACE_GRP_LLDP, "policy_index = %d apply = %d", policy_index, s->apply);
                        if (policies_list[policy_index]) {
                            lldp_state->conf.policies_table[policy_index].in_use = 1;
                            T_NG(VTSS_TRACE_GRP_LLDP, "policy_index = %d ,tag_found =%d, tag_val = %d", policy_index, tag_found, tag_val);
                            if (application_type_found) {
                                lldp_state->conf.policies_table[policy_index].application_type = application_type_val;
                            }
                            if (tag_found) {
                                lldp_state->conf.policies_table[policy_index].tagged_flag = tag_val;
                            }
                            if (vid_found) {
                                lldp_state->conf.policies_table[policy_index].vlan_id = vid_val;
                            }
                            if (l2_prio_found) {
                                lldp_state->conf.policies_table[policy_index].l2_priority = l2_priority_val;
                            }
                            if (dscp_found) {
                                lldp_state->conf.policies_table[policy_index].dscp_value = dscp_val;
                            }
                        }
                    }

                } else if (!global && s->group == CX_TAG_PORT_TABLE && cx_parse_ports(s, port_list, 1) == VTSS_RC_OK) {
                    BOOL policy_list_empty = TRUE;

                    // Port Table
                    memset(policies_list, 0, sizeof(policies_list));

                    s->p = s->next;
                    for (; s->rc == VTSS_RC_OK && cx_parse_attr(s) == VTSS_RC_OK; s->p = s->next) {
                        if (cx_parse_txt(s, "port_policies", buf, sizeof(buf)) == VTSS_RC_OK) {
                            if (mgmt_txt2list(buf, policies_list, LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX, 0) == VTSS_RC_OK) {
                                policy_id_found = TRUE;
                                for (policy_index = LLDPMED_POLICY_MIN; policy_index < LLDPMED_POLICY_MAX; policy_index++) {
                                    if (policies_list[policy_index]) {
                                        policy_list_empty = FALSE;
                                        break;
                                    }
                                }
                            } else {
                                T_WG(VTSS_TRACE_GRP_LLDP, "Problem converting policies id to list");
                            }
                        }  else {
                            (void)cx_parm_unknown(s);
                        }
                    }

                    if (!policy_id_found) {
                        break;
                    }

                    /* Port setup */
                    (void)port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
                    while (port_iter_getnext(&pit)) {
                        if (port_list[iport2uport(pit.iport)]) {
                            for (policy_index = LLDPMED_POLICY_MIN; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
                                T_NG(VTSS_TRACE_GRP_LLDP, "pit.iport = %u, policy_index = %d, policies_list[policy_index] = %d", pit.iport, policy_index, policies_list[policy_index]);
                                if (policies_list[policy_index]) {
                                    lldp_state->conf.ports_policies[pit.iport][policy_index] = 1;
                                } else if (policy_list_empty) {
                                    lldp_state->conf.ports_policies[pit.iport][policy_index] = 0;
                                }
                            }
                        }
                    }
                } else {
                    T_IG(VTSS_TRACE_GRP_LLDP, "Parsing policy_id  - ignore");
                    s->ignored = 1;
                }
                break;
            default:
                break;
            }
            break;
        } /* CX_TAG_LLDPMED */
#endif /* VTSS_SW_OPTION_LLDP_MED */
        } /* CX_PARSE_CMD_PARM */

    case CX_PARSE_CMD_GLOBAL:
        T_IG(VTSS_TRACE_GRP_LLDP, "Global, I=%d, A=%d", s->init, s->apply);
        if (s->init) {
            lldp_state->line_lldp.number = 0;
            lldp_mgmt_get_config(&lldp_state->conf, s->isid);
        } else if (s->apply) {
            (void)lldp_mgmt_set_config(&lldp_state->conf, s->isid);
        } else {
            if (lldp_state->conf.txDelay * 4 > lldp_state->conf.msgTxInterval) {
                (void)cx_parm_error(s, "LLDP Delay must be less than 1/4 of Interval");
                if (lldp_state->line_lldp.number != 0) {
                    s->line = lldp_state->line_lldp;
                }
                return s->rc;
            }
        }
        break;

    case CX_PARSE_CMD_SWITCH:
        T_IG(VTSS_TRACE_GRP_LLDP, "Switch(usid=%u), I=%d, A=%d", topo_isid2usid(s->isid), s->init, s->apply);
        if (s->init) {
            lldp_mgmt_get_config(&lldp_state->conf, s->isid);
        } else if (s->apply) {
            (void)lldp_mgmt_set_config(&lldp_state->conf, s->isid);
        }
        break;

    default:
        break;
    }
    return s->rc;
}

static vtss_rc lldp_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - LLDP */
        T_DG(VTSS_TRACE_GRP_LLDP, "global - lldp");
        (void) cx_add_tag_line(s, CX_TAG_LLDP, 0);
        {
            lldp_struc_0_t lldp_conf;

            lldp_mgmt_get_config(&lldp_conf, VTSS_ISID_LOCAL);
            (void) cx_add_val_ulong(s, CX_TAG_INTERVAL, lldp_conf.msgTxInterval, 5, 32768);
            (void) cx_add_val_ulong(s, CX_TAG_HOLD, lldp_conf.msgTxHold, 2, 10);
            (void) cx_add_val_ulong(s, CX_TAG_DELAY, lldp_conf.txDelay, 1, 8192);
            (void) cx_add_val_ulong(s, CX_TAG_REINIT, lldp_conf.reInitDelay, 1, 10);
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_LLDP, 1));

#ifdef VTSS_SW_OPTION_LLDP_MED
        /* Global - LLDP */
        T_DG(VTSS_TRACE_GRP_LLDP, "global - LLDPMED");
        (void) cx_add_tag_line(s, CX_TAG_LLDPMED, 0);
        {
            lldp_struc_0_t lldp_conf;
            char           buf[128];
            char           stx[32];
            char           tude_max_str[32];
            char           tude_min_str[32];

            lldp_mgmt_get_config(&lldp_conf, VTSS_ISID_LOCAL);
            (void) cx_add_val_ulong(s, CX_TAG_FAST_START_REPEAT_COUNT,
                                    lldp_conf.medFastStartRepeatCount,
                                    FAST_START_REPEAT_COUNT_MIN,
                                    FAST_START_REPEAT_COUNT_MAX);

            // Location latitude
            mgmt_long2str_float(tude_max_str, LLDPMED_LATITUDE_VALUE_MAX, TUDE_DIGIT);
            sprintf(stx, "%d-%s", LLDPMED_LATITUDE_VALUE_MIN, tude_max_str);
            mgmt_long2str_float(buf, abs(lldp_conf.location_info.latitude), TUDE_DIGIT);
            (void) cx_add_val_txt(s, CX_TAG_LATITUDE, buf, stx);


            // Location latitude direction
            // Help text
            (void) cx_add_stx_start(s);
            (void) cx_add_stx_kw(s, "latitude_dir", cx_kw_lldpmed_latitude_dir);
            (void) cx_add_stx_end(s);

            (void) cx_add_attr_start(s, CX_TAG_LATITUDE_DIR);
            (void) cx_add_attr_kw(s, "latitude_dir", cx_kw_lldpmed_latitude_dir,
                                  lldp_conf.location_info.latitude_dir);
            (void) cx_add_attr_end(s, CX_TAG_LATITUDE_DIR);


            // Location longitude
            mgmt_long2str_float(tude_max_str, LLDPMED_LONGITUDE_VALUE_MAX, TUDE_DIGIT);
            sprintf(stx, "%d-%s", LLDPMED_LONGITUDE_VALUE_MIN, tude_max_str);
            mgmt_long2str_float(buf, abs(lldp_conf.location_info.longitude), TUDE_DIGIT);
            (void) cx_add_val_txt(s, CX_TAG_LONGITUDE, buf, stx);

            // Location longitude direction
            // Help text
            (void) cx_add_stx_start(s);
            (void) cx_add_stx_kw(s, "longitude_dir", cx_kw_lldpmed_longitude_dir);
            (void) cx_add_stx_end(s);

            (void) cx_add_attr_start(s, CX_TAG_LONGITUDE_DIR);
            (void) cx_add_attr_kw(s, "longitude_dir", cx_kw_lldpmed_longitude_dir,
                                  lldp_conf.location_info.longitude_dir);
            (void) cx_add_attr_end(s, CX_TAG_LONGITUDE_DIR);

            // Location Altitude
            mgmt_long2str_float(tude_max_str, LLDPMED_ALTITUDE_VALUE_MAX, TUDE_DIGIT);
            mgmt_long2str_float(tude_min_str, LLDPMED_ALTITUDE_VALUE_MIN, TUDE_DIGIT);
            sprintf(stx, "%s-%s", tude_min_str, tude_max_str);
            mgmt_long2str_float(buf, lldp_conf.location_info.altitude, TUDE_DIGIT);
            (void) cx_add_val_txt(s, CX_TAG_ALTITUDE, buf, stx);

            // help txt.
            (void) cx_add_stx_start(s);
            (void) cx_add_stx_kw(s, "altitude_type", cx_kw_lldpmed_altitude_type);
            (void) cx_add_stx_end(s);

            (void) cx_add_attr_start(s, CX_TAG_ALTITUDE_TYPE);
            (void) cx_add_attr_kw(s, "altitude_type", cx_kw_lldpmed_altitude_type,
                                  lldp_conf.location_info.altitude_type);
            (void) cx_add_attr_end(s, CX_TAG_ALTITUDE_TYPE);

            // help txt.
            (void) cx_add_stx_start(s);
            (void) cx_add_stx_kw(s, "datum", cx_kw_lldpmed_datum);
            (void) cx_add_stx_end(s);

            (void) cx_add_attr_start(s, CX_TAG_DATUM);
            (void) cx_add_attr_kw(s, "datum", cx_kw_lldpmed_datum,
                                  lldp_conf.location_info.datum);
            (void) cx_add_attr_end(s, CX_TAG_DATUM);


            sprintf(stx, "0-2 characters");
            (void) cx_add_val_txt(s, CX_TAG_COUNTRY, lldp_conf.location_info.ca_country_code,
                                  stx);

            sprintf(stx, "0-%d characters", ECS_VALUE_LEN_MAX);
            (void) cx_add_val_txt(s, CX_TAG_ECS, lldp_conf.location_info.ecs, stx);
            (void) cx_add_tag_line(s, CX_TAG_CIVIC, 0);
            (void) cx_add_tag_line(s, CX_TAG_GROUP_TABLE, 0);
            {
                lldp_u8_t ca_index;
                lldp_8_t   catype_buf[255];
                lldp_16_t ptr;

                /* Entry syntax */
                (void) cx_add_stx_start(s);
                (void) cx_add_attr_txt(s, "civic_type", "civic_type");
                (void) cx_add_stx_end(s);

                for (ca_index = 0 ; ca_index < LLDPMED_CATYPE_CNT; ca_index ++) {
                    (void) cx_add_attr_start(s, CX_TAG_ENTRY);
                    ptr = lldp_conf.location_info.civic.civic_str_ptr_array[ca_index];
                    lldpmed_catype2str(lldpmed_index2catype(ca_index), &catype_buf[0]);
                    (void) cx_add_attr_txt(s, "group_name", catype_buf);
                    (void) cx_add_attr_txt(s, "val", &lldp_conf.location_info.civic.ca_value[ptr]);
                    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                }
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_GROUP_TABLE, 1));
            CX_RC(cx_add_tag_line(s, CX_TAG_CIVIC, 1));

            // POLICIES
            (void) cx_add_tag_line(s, CX_TAG_POLICY, 0);
            (void) cx_add_tag_line(s, CX_TAG_POLICIES_TABLE, 0);

            (void) cx_add_stx_start(s);
            (void) cx_add_stx_ulong(s, "policy_id", LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX);
            (void) cx_add_stx_kw(s, "application_type", cx_kw_lldpmed_application_type);
            (void) cx_add_stx_bool(s, "tag");
            (void) cx_add_stx_ulong(s, "vid", LLDPMED_VID_MIN, LLDPMED_VID_MAX);
            (void) cx_add_stx_ulong(s, "l2_prio", LLDPMED_L2_PRIORITY_MIN,
                                    LLDPMED_L2_PRIORITY_MAX);
            (void) cx_add_stx_ulong(s, "dscp", LLDPMED_DSCP_MIN, LLDPMED_DSCP_MAX);
            (void) cx_add_stx_end(s);

            u16 policy_index = 0;
            for (policy_index = LLDPMED_POLICY_MIN ;
                 policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
                T_DG(VTSS_TRACE_GRP_LLDP, "Policy_index = %u, LLDPMED_POLICY_MIN =%u, LLDPMED_POLICY_MAX=%u", policy_index, LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX);
                if (lldp_conf.policies_table[policy_index].in_use == 1) {
                    T_IG(VTSS_TRACE_GRP_LLDP, "Policy in use");
                    (void)cx_add_attr_start(s, CX_TAG_ENTRY);
                    (void)cx_add_attr_ulong(s, "policy_id",        policy_index);
                    (void)cx_add_attr_kw(   s, "application_type", cx_kw_lldpmed_application_type, lldp_conf.policies_table[policy_index].application_type);
                    (void)cx_add_attr_bool( s, "tag",              lldp_conf.policies_table[policy_index].tagged_flag);
                    (void)cx_add_attr_ulong(s, "vid",              lldp_conf.policies_table[policy_index].vlan_id);
                    (void)cx_add_attr_ulong(s, "l2_prio",          lldp_conf.policies_table[policy_index].l2_priority);
                    (void)cx_add_attr_ulong(s, "dscp",             lldp_conf.policies_table[policy_index].dscp_value);
                    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                }
            }
            (void) cx_add_tag_line(s, CX_TAG_POLICIES_TABLE, 1);
            CX_RC(cx_add_tag_line(s, CX_TAG_POLICY, 1));
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_LLDPMED, 1));
#endif /* VTSS_SW_OPTION_LLDP_MED */
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - LLDP */
        T_DG(VTSS_TRACE_GRP_LLDP, "switch - lldp");
        (void) cx_add_tag_line(s, CX_TAG_LLDP, 0);
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_lldp_match, cx_lldp_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_LLDP, 1));
#ifdef VTSS_SW_OPTION_LLDP_MED
        (void) cx_add_tag_line(s, CX_TAG_LLDPMED, 0);
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_lldp_med_match, cx_lldp_med_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_LLDPMED, 1));
#endif /* VTSS_SW_OPTION_LLDP_MED */
        break;
    default:
        T_EG(VTSS_TRACE_GRP_LLDP, "Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_RC_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_LLDP,
    lldp_cx_tag_table,
    sizeof(lldp_cx_set_state_t),
    0,
    NULL,                   /* init function       */
    lldp_cx_gen_func,       /* Generation fucntion */
    lldp_cx_parse_func      /* parse fucntion      */
);
