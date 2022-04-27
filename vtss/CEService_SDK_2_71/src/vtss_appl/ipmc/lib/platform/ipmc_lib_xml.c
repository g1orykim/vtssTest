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
#include "misc_api.h"
#include "ipmc_lib.h"


#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IPMC_LIB

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_IPMC_LIB,

    /* Group Tags */
    CX_TAG_IPMCPF_ENTRY_TABLE,
    CX_TAG_IPMCPF_OBJECT_TABLE,
    CX_TAG_IPMCPF_RULE_TABLE,

    /* Parameter tags */
    CX_TAG_IPMCPF_MODE,
    CX_TAG_IPMCPF_ADRS_ENTRY,
    CX_TAG_IPMCPF_PROF_ENTRY,
    CX_TAG_IPMCPF_RULE_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t ipmc_lib_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_IPMC_LIB] = {
        .name  = "ipmc_library",
        .descr = "IP Multicast Library",
        .type = CX_TAG_TYPE_MODULE
    },

    [CX_TAG_IPMCPF_OBJECT_TABLE] = {
        .name  = "profile_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_IPMCPF_RULE_TABLE] = {
        .name  = "rule_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_IPMCPF_ENTRY_TABLE] = {
        .name  = "entry_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },

    [CX_TAG_IPMCPF_MODE] = {
        .name  = "mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },

    [CX_TAG_IPMCPF_ADRS_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_IPMCPF_PROF_ENTRY] = {
        .name  = "profile",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_IPMCPF_RULE_ENTRY] = {
        .name  = "rule",
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

#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
/* Keyword for rule action */
static const cx_kw_t cx_kw_ipmcpf_rule_action[] = {
    { "permit",     IPMC_ACTION_PERMIT },
    { "deny",       IPMC_ACTION_DENY },
    { NULL,         0 }
};

/* Keyword for log capability */
static const cx_kw_t cx_kw_ipmcpf_rule_log[] = {
    { "enable",        TRUE },
    { "disable",     FALSE },
    { NULL,         0 }
};

typedef struct {
    BOOL                            valid;
    i8                              name[VTSS_IPMC_NAME_MAX_LEN];
} ipmcpf_xml_fltr_chk_t;

/*lint -esym(459, ipmcpf_xml_fltr_en_pool) */
/*lint -esym(459, ipmcpf_xml_fltr_pf_pool) */
static ipmcpf_xml_fltr_chk_t        ipmcpf_xml_fltr_en_pool[IPMC_LIB_FLTR_ENTRY_MAX_CNT];
static ipmcpf_xml_fltr_chk_t        ipmcpf_xml_fltr_pf_pool[IPMC_LIB_FLTR_PROFILE_MAX_CNT];

#define IPMCPF_XML_FLTR_EN_SET(x,y) do {                            \
    int str_len = strlen((y));                                      \
    ipmcpf_xml_fltr_en_pool[(x) - 1].valid = TRUE;                  \
    strncpy(ipmcpf_xml_fltr_en_pool[(x) - 1].name, (y), str_len);   \
} while (0)
#define IPMCPF_XML_FLTR_PF_SET(x,y) do {                            \
    int str_len = strlen((y));                                      \
    ipmcpf_xml_fltr_pf_pool[(x) - 1].valid = TRUE;                  \
    strncpy(ipmcpf_xml_fltr_pf_pool[(x) - 1].name, (y), str_len);   \
} while (0)

#define IPMCPF_XML_FLTR_EN_GET(x,y) (ipmc_lib_cx_entry_search((x), (y)))
#define IPMCPF_XML_FLTR_PF_GET(x,y) (ipmc_lib_cx_profile_search((x), (y)))

static void ipmc_lib_cx_entry_insert(i8 *name)
{
    int str_len, i;

    if (!name) {
        return;
    }

    str_len = strlen(name);
    for (i = 0; i < IPMC_LIB_FLTR_PROFILE_MAX_CNT; i++) {
        if (!ipmcpf_xml_fltr_en_pool[i].valid) {
            ipmcpf_xml_fltr_en_pool[i].valid = TRUE;
            strncpy(ipmcpf_xml_fltr_en_pool[i].name, name, str_len);
            break;
        }
    }
}

static BOOL ipmc_lib_cx_entry_search(i8 *name, u32 *idx)
{
    int str_len, i;

    if (!name || !idx) {
        return FALSE;
    }

    *idx = 0;
    str_len = strlen(name);
    for (i = 0; i < IPMC_LIB_FLTR_PROFILE_MAX_CNT; i++) {
        if (ipmcpf_xml_fltr_en_pool[i].valid) {
            if (!strncmp(ipmcpf_xml_fltr_en_pool[i].name, name, str_len)) {
                *idx = i + 1;
                return TRUE;
            }
        }
    }

    return FALSE;
}

static void ipmc_lib_cx_profile_insert(i8 *name)
{
    int str_len, i;

    if (!name) {
        return;
    }

    str_len = strlen(name);
    for (i = 0; i < IPMC_LIB_FLTR_PROFILE_MAX_CNT; i++) {
        if (!ipmcpf_xml_fltr_pf_pool[i].valid) {
            ipmcpf_xml_fltr_pf_pool[i].valid = TRUE;
            strncpy(ipmcpf_xml_fltr_pf_pool[i].name, name, str_len);
            break;
        }
    }
}

static BOOL ipmc_lib_cx_profile_search(i8 *name, u32 *idx)
{
    int str_len, i;

    if (!name || !idx) {
        return FALSE;
    }

    *idx = 0;
    str_len = strlen(name);
    for (i = 0; i < IPMC_LIB_FLTR_PROFILE_MAX_CNT; i++) {
        if (ipmcpf_xml_fltr_pf_pool[i].valid) {
            if (!strncmp(ipmcpf_xml_fltr_pf_pool[i].name, name, str_len)) {
                *idx = i + 1;
                return TRUE;
            }
        }
    }

    return FALSE;
}

static vtss_rc cx_parse_ipmcpf_name(cx_set_state_t *s, char *name, char *naming)
{
    u32     name_len, idx;
    BOOL    valid_flag;

    CX_RC(cx_parse_txt(s, name, naming, VTSS_IPMC_NAME_STRING_MAX_LEN));

    valid_flag = FALSE;
    if ((name_len = strlen(naming)) != 0) {
        for (idx = 0; idx < name_len; idx++) {
            if ((naming[idx] < 48) || (naming[idx] > 122)) {
                valid_flag = FALSE;
                break;
            } else {
                if (((naming[idx] > 64) && (naming[idx] < 91)) ||
                    (naming[idx] > 96)) {
                    valid_flag = TRUE;
                } else {
                    if (naming[idx] > 57) {
                        valid_flag = FALSE;
                        break;
                    }
                }
            }
        }
    }

    if (!valid_flag) {
        CX_RC(cx_parm_invalid(s));
    }

    return s->rc;
}

static vtss_rc cx_parse_ipmcpf_desc(cx_set_state_t *s, char *name, char *desc)
{
    u32     desc_len, idx;
    BOOL    valid_flag;

    CX_RC(cx_parse_txt(s, name, desc, VTSS_IPMC_DESC_STRING_MAX_LEN));

    valid_flag = TRUE;
    if ((desc_len = strlen(desc)) != 0) {
        valid_flag = FALSE;
        for (idx = 0; idx < desc_len; idx++) {
            if ((desc[idx] < 45) || (desc[idx] > 122)) {
                if (desc[idx] != 32) {
                    valid_flag = FALSE;
                    break;
                }
            } else {
                if ((desc[idx] == 96) ||
                    ((desc[idx] >= 46) && (desc[idx] <= 47)) ||
                    ((desc[idx] >= 58) && (desc[idx] <= 64)) ||
                    ((desc[idx] >= 91) && (desc[idx] <= 94))) {
                    valid_flag = FALSE;
                    break;
                } else {
                    if (((desc[idx] >= 65) && (desc[idx] <= 90)) ||
                        ((desc[idx] >= 97) && (desc[idx] <= 122))) {
                        valid_flag = TRUE;
                    }
                }
            }
        }
    }

    if (!valid_flag) {
        CX_RC(cx_parm_invalid(s));
    }

    return s->rc;
}
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

static vtss_rc ipmc_lib_cx_parse_func(cx_set_state_t *s)
{
#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *fltr_profile;
    ipmc_lib_profile_t          *data;
    ipmc_lib_rule_t             fltr_rule;
    ipmc_lib_grp_fltr_entry_t   fltr_entry;

    switch ( s->cmd ) {
    case CX_PARSE_CMD_PARM: {
        BOOL    global, state;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = TRUE;
            break;
        }

        switch ( s->id ) {
        case CX_TAG_IPMCPF_MODE:
            if ((cx_parse_val_bool(s, &state, 1) == VTSS_OK) && s->apply) {
                CX_RC(ipmc_lib_mgmt_profile_state_set(state));
            }

            break;
        case CX_TAG_IPMCPF_ENTRY_TABLE:
            memset(ipmcpf_xml_fltr_en_pool, 0x0, sizeof(ipmcpf_xml_fltr_en_pool));

            if (s->apply) {
                CX_RC(ipmc_lib_mgmt_clear_profile(VTSS_ISID_GLOBAL));
            }

            break;
        case CX_TAG_IPMCPF_ADRS_ENTRY:
            if (s->group == CX_TAG_IPMCPF_ENTRY_TABLE) {
                vtss_ipv4_t adrs4bgn, adrs4end;

                adrs4bgn = adrs4end = 0;
                memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
                fltr_entry.version = IPMC_IP_VERSION_INIT;
                for (; (s->rc == VTSS_OK) && (cx_parse_attr(s) == VTSS_OK); s->p = s->next) {
                    if ((cx_parse_ipmcpf_name(s, "name", fltr_entry.name) != VTSS_OK) &&
                        (cx_parse_ipmcv6(s, "start", &fltr_entry.grp_bgn) != VTSS_OK) &&
                        (cx_parse_ipmcv4(s, "start", &adrs4bgn) != VTSS_OK) &&
                        (cx_parse_ipmcv6(s, "end", &fltr_entry.grp_end) != VTSS_OK) &&
                        (cx_parse_ipmcv4(s, "end", &adrs4end) != VTSS_OK)) {
                        CX_RC(cx_parm_unknown(s));
                    } else {
                        if (adrs4bgn) {
                            fltr_entry.version = IPMC_IP_VERSION_IGMP;
                        } else {
                            if (fltr_entry.grp_bgn.addr[0] == 0xFF) {
                                fltr_entry.version = IPMC_IP_VERSION_MLD;
                            }
                        }

                        if (fltr_entry.version != IPMC_IP_VERSION_INIT) {
                            if (adrs4end) {
                                if (fltr_entry.version != IPMC_IP_VERSION_IGMP) {
                                    CX_RC(cx_parm_unknown(s));
                                }
                            } else {
                                if (fltr_entry.grp_end.addr[0] == 0xFF) {
                                    if (fltr_entry.version != IPMC_IP_VERSION_MLD) {
                                        CX_RC(cx_parm_unknown(s));
                                    }
                                }
                            }
                        }
                    }
                }

                if (s->apply) {
                    if (fltr_entry.version == IPMC_IP_VERSION_IGMP) {
                        adrs4end = htonl(adrs4end);
                        IPMC_LIB_ADRS_4TO6_SET(adrs4end, fltr_entry.grp_end);
                        adrs4bgn = htonl(adrs4bgn);
                        IPMC_LIB_ADRS_4TO6_SET(adrs4bgn, fltr_entry.grp_bgn);
                    }

                    CX_RC(ipmc_lib_mgmt_fltr_entry_set(IPMC_OP_ADD, &fltr_entry));
                    if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, TRUE) == VTSS_OK) {
                        IPMCPF_XML_FLTR_EN_SET(fltr_entry.index, fltr_entry.name);
                    }
                } else {
                    ipmc_lib_cx_entry_insert(fltr_entry.name);
                }
            }

            break;
        case CX_TAG_IPMCPF_OBJECT_TABLE:
            memset(ipmcpf_xml_fltr_pf_pool, 0x0, sizeof(ipmcpf_xml_fltr_pf_pool));

            break;
        case CX_TAG_IPMCPF_PROF_ENTRY:
            if (s->group == CX_TAG_IPMCPF_OBJECT_TABLE) {
                if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
                    break;
                }

                fltr_profile = &pfm->profile;
                memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
                data = &fltr_profile->data;
                for (; (s->rc == VTSS_OK) && (cx_parse_attr(s) == VTSS_OK); s->p = s->next) {
                    if ((cx_parse_ipmcpf_name(s, "name", data->name) != VTSS_OK) &&
                        (cx_parse_ipmcpf_desc(s, "desc", data->desc) != VTSS_OK)) {
                        IPMC_MEM_PROFILE_MGIVE(pfm);
                        CX_RC(cx_parm_unknown(s));
                        s->rc = CONF_XML_ERROR_FILE_PARM;
                        return s->rc;
                    }
                }

                if (s->apply) {
                    vtss_rc pf_rc = ipmc_lib_mgmt_fltr_profile_set(IPMC_OP_ADD, fltr_profile);

                    if (pf_rc == VTSS_OK) {
                        if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK) {
                            IPMCPF_XML_FLTR_PF_SET(data->index, data->name);
                        }
                    } else {
                        IPMC_MEM_PROFILE_MGIVE(pfm);
                        return pf_rc;
                    }
                } else {
                    ipmc_lib_cx_profile_insert(data->name);
                }

                IPMC_MEM_PROFILE_MGIVE(pfm);
            }

            break;
        case CX_TAG_IPMCPF_RULE_TABLE:

            break;
        case CX_TAG_IPMCPF_RULE_ENTRY:
            if (s->group == CX_TAG_IPMCPF_RULE_TABLE) {
                u32     pf_idx, en_idx, r_act, r_log;
                BOOL    pf_chk, en_chk;
                i8      pf_name[VTSS_IPMC_NAME_MAX_LEN], en_name[VTSS_IPMC_NAME_MAX_LEN];

                r_act = r_log = 0x0;
                pf_idx = en_idx = 0;
                pf_chk = en_chk = FALSE;
                memset(&fltr_rule, 0x0, sizeof(ipmc_lib_rule_t));
                memset(pf_name, 0x0, sizeof(pf_name));
                memset(en_name, 0x0, sizeof(en_name));
                for (; (s->rc == VTSS_OK) && (cx_parse_attr(s) == VTSS_OK); s->p = s->next) {
                    if ((cx_parse_ipmcpf_name(s, "profile", pf_name) != VTSS_OK) &&
                        (cx_parse_kw(s, "action", cx_kw_ipmcpf_rule_action, &r_act, TRUE) != VTSS_OK) &&
                        (cx_parse_kw(s, "log", cx_kw_ipmcpf_rule_log, &r_log, TRUE) != VTSS_OK) &&
                        (cx_parse_ipmcpf_name(s, "entry", en_name) != VTSS_OK)) {
                        CX_RC(cx_parm_unknown(s));
                    } else {
                        if (!pf_chk && strlen(pf_name)) {
                            if (!IPMCPF_XML_FLTR_PF_GET(pf_name, &pf_idx) || !pf_idx) {
                                CX_RC(cx_parm_unknown(s));
                            }

                            pf_chk = TRUE;
                        }

                        if (!en_chk && strlen(en_name)) {
                            if (!IPMCPF_XML_FLTR_EN_GET(en_name, &en_idx) || !en_idx) {
                                CX_RC(cx_parm_unknown(s));
                            }

                            en_chk = TRUE;
                        }
                    }
                }

                if (!pf_chk || !en_chk) {
                    sprintf(s->msg, "Invalid %s%s name!",
                            pf_chk ? "" : "Profile",
                            en_chk ? "" : (pf_chk ? "Entry" : " and Entry"));
                    s->rc = CONF_XML_ERROR_FILE_PARM;
                    return s->rc;
                }

                if (s->apply && IPMC_MEM_PROFILE_MTAKE(pfm)) {
                    BOOL    pf_found;

                    fltr_profile = &pfm->profile;
                    memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
                    fltr_profile->data.index = pf_idx;
                    pf_found = (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, FALSE) == VTSS_OK);
                    IPMC_MEM_PROFILE_MGIVE(pfm);

                    if (pf_found) {
                        if (ipmc_lib_mgmt_fltr_profile_rule_search(pf_idx, en_idx, &fltr_rule) != VTSS_OK) {
                            memset(&fltr_rule, 0x0, sizeof(ipmc_lib_rule_t));
                            fltr_rule.idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
                            fltr_rule.entry_index = en_idx;
                            fltr_rule.next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
                        }

                        fltr_rule.action = r_act;
                        fltr_rule.log = r_log;
                        CX_RC(ipmc_lib_mgmt_fltr_profile_rule_set(IPMC_OP_ADD, pf_idx, &fltr_rule));
                    }
                }
            }

            break;
        default:
            s->ignored = TRUE;

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
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

    return s->rc;
}

static vtss_rc ipmc_lib_cx_gen_func(cx_get_state_t *s)
{
#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
    BOOL                        state;
    vtss_ipv4_t                 adrs4;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *fltr_profile;
    ipmc_lib_profile_t          *data;
    ipmc_lib_rule_t             fltr_rule;
    ipmc_lib_grp_fltr_entry_t   fltr_entry;

    switch ( s->cmd ) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - IPMC */
        T_D("global - ipmc_lib");
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMC_LIB, 0));

        if (ipmc_lib_mgmt_profile_state_get(&state) == VTSS_OK) {
            CX_RC(cx_add_val_bool(s, CX_TAG_IPMCPF_MODE, state));
        }

        /* Fill global table entry */
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMCPF_ENTRY_TABLE, 0));
        // Fill entry syntax
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_attr_txt(s, "name", "Entry Name"));
        CX_RC(cx_add_attr_txt(s, "start", "IPv4/IPv6 Multicast Address"));
        CX_RC(cx_add_attr_txt(s, "end", "IPv4/IPv6 Multicast Address"));
        CX_RC(cx_add_stx_end(s));
        // Fill entry database
        memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
        while (ipmc_lib_mgmt_fltr_entry_get_next(&fltr_entry, FALSE) == VTSS_OK) {
            CX_RC(cx_add_attr_start(s, CX_TAG_IPMCPF_ADRS_ENTRY));
            CX_RC(cx_add_attr_txt(s, "name", fltr_entry.name));
            if (ipmc_lib_grp_adrs_version(&fltr_entry.grp_bgn) == IPMC_IP_VERSION_IGMP) {
                IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_bgn, adrs4);
                CX_RC(cx_add_attr_ipv4(s, "start", htonl(adrs4)));
                IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_end, adrs4);
                CX_RC(cx_add_attr_ipv4(s, "end", htonl(adrs4)));
            } else {
                CX_RC(cx_add_attr_ipv6(s, "start", fltr_entry.grp_bgn));
                CX_RC(cx_add_attr_ipv6(s, "end", fltr_entry.grp_end));
            }
            CX_RC(cx_add_attr_end(s, CX_TAG_IPMCPF_ADRS_ENTRY));
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMCPF_ENTRY_TABLE, 1));

        /* Fill global table entry */
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMCPF_OBJECT_TABLE, 0));
        // Fill entry syntax
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_attr_txt(s, "name", "Profile Name"));
        CX_RC(cx_add_attr_txt(s, "desc", "Profile Description"));
        CX_RC(cx_add_stx_end(s));
        // Fill entry database
        if (IPMC_MEM_PROFILE_MTAKE(pfm)) {
            vtss_rc pfo_rc;

            fltr_profile = &pfm->profile;
            memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            data = &fltr_profile->data;
            while (ipmc_lib_mgmt_fltr_profile_get_next(fltr_profile, FALSE) == VTSS_OK) {
                if ((pfo_rc = cx_add_attr_start(s, CX_TAG_IPMCPF_PROF_ENTRY)) != VTSS_OK) {
                    IPMC_MEM_PROFILE_MGIVE(pfm);
                    return pfo_rc;
                }
                if ((pfo_rc = cx_add_attr_txt(s, "name", data->name)) != VTSS_OK) {
                    IPMC_MEM_PROFILE_MGIVE(pfm);
                    return pfo_rc;
                }
                if ((pfo_rc = cx_add_attr_txt(s, "desc", data->desc)) != VTSS_OK) {
                    IPMC_MEM_PROFILE_MGIVE(pfm);
                    return pfo_rc;
                }
                if ((pfo_rc = cx_add_attr_end(s, CX_TAG_IPMCPF_PROF_ENTRY)) != VTSS_OK) {
                    IPMC_MEM_PROFILE_MGIVE(pfm);
                    return pfo_rc;
                }
            }

            IPMC_MEM_PROFILE_MGIVE(pfm);
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMCPF_OBJECT_TABLE, 1));

        /* Fill global table entry */
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMCPF_RULE_TABLE, 0));
        // Fill entry syntax
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_attr_txt(s, "profile", "Profile Name"));
        CX_RC(cx_add_attr_txt(s, "entry", "Entry Name"));
        CX_RC(cx_add_stx_kw(s, "action", cx_kw_ipmcpf_rule_action));
        CX_RC(cx_add_stx_kw(s, "log", cx_kw_ipmcpf_rule_log));
        CX_RC(cx_add_stx_end(s));
        // Fill entry database
        if (IPMC_MEM_PROFILE_MTAKE(pfm)) {
            vtss_rc pfr_rc;

            fltr_profile = &pfm->profile;
            memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            while (ipmc_lib_mgmt_fltr_profile_get_next(fltr_profile, FALSE) == VTSS_OK) {
                data = &fltr_profile->data;
                if (ipmc_lib_mgmt_fltr_profile_rule_get_first(data->index, &fltr_rule) == VTSS_OK) {
                    do {
                        fltr_entry.index = fltr_rule.entry_index;
                        if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, FALSE) == VTSS_OK) {
                            if ((pfr_rc = cx_add_attr_start(s, CX_TAG_IPMCPF_RULE_ENTRY)) != VTSS_OK) {
                                IPMC_MEM_PROFILE_MGIVE(pfm);
                                return pfr_rc;
                            }
                            if ((pfr_rc = cx_add_attr_txt(s, "profile", data->name)) != VTSS_OK) {
                                IPMC_MEM_PROFILE_MGIVE(pfm);
                                return pfr_rc;
                            }
                            if ((pfr_rc = cx_add_attr_txt(s, "entry", fltr_entry.name)) != VTSS_OK) {
                                IPMC_MEM_PROFILE_MGIVE(pfm);
                                return pfr_rc;
                            }
                            if ((pfr_rc = cx_add_attr_kw(s, "action", cx_kw_ipmcpf_rule_action, fltr_rule.action)) != VTSS_OK) {
                                IPMC_MEM_PROFILE_MGIVE(pfm);
                                return pfr_rc;
                            }
                            if ((pfr_rc = cx_add_attr_kw(s, "log", cx_kw_ipmcpf_rule_log, fltr_rule.log)) != VTSS_OK) {
                                IPMC_MEM_PROFILE_MGIVE(pfm);
                                return pfr_rc;
                            }
                            if ((pfr_rc = cx_add_attr_end(s, CX_TAG_IPMCPF_RULE_ENTRY)) != VTSS_OK) {
                                IPMC_MEM_PROFILE_MGIVE(pfm);
                                return pfr_rc;
                            }
                        }
                    } while (ipmc_lib_mgmt_fltr_profile_rule_get_next(data->index, &fltr_rule) == VTSS_OK);
                }
            }

            IPMC_MEM_PROFILE_MGIVE(pfm);
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_IPMCPF_RULE_TABLE, 1));

        CX_RC(cx_add_tag_line(s, CX_TAG_IPMC_LIB, 1));
        break;
    case CX_GEN_CMD_SWITCH:

        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_IPMC_LIB, ipmc_lib_cx_tag_table,
                    0, 0,
                    NULL,                       /* init function       */
                    ipmc_lib_cx_gen_func,       /* Generation fucntion */
                    ipmc_lib_cx_parse_func);    /* parse fucntion      */
