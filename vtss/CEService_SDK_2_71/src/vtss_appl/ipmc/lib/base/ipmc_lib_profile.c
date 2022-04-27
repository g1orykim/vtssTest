/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "critd_api.h"
#include "misc_api.h"

#include "ipmc_lib.h"
#include "ipmc_lib_porting.h"


/* ************************************************************************ **
 *
 * Defines
 *
 * ************************************************************************ */
#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_IPMC_LIB


/* ************************************************************************ **
 *
 * Public data
 *
 * ************************************************************************ */


/* ************************************************************************ **
 *
 * Local data
 *
 * ************************************************************************ */
static BOOL                         ipmc_profile_done_init = FALSE;

static critd_t                      ipmc_profile_crit;

static ipmc_lib_conf_profile_t      *ipmc_lib_profile;
static ipmc_profile_tree_t          ipmc_profile_tree[IPMC_LIB_FLTR_PROFILE_MAX_CNT];

static ipmc_profile_rule_t          ipmc_lib_def_profile_rule4[IPMC_LIB_FLTR_PROFILE_MAX_CNT];
static ipmc_profile_rule_t          ipmc_lib_def_profile_rule6[IPMC_LIB_FLTR_PROFILE_MAX_CNT];
static ipmc_lib_rule_t              ipmc_lib_def_fltr_rule4;
static ipmc_lib_rule_t              ipmc_lib_def_fltr_rule6;

#if VTSS_TRACE_ENABLED
#define IPMC_PROFILE_ENTER()        critd_enter(&ipmc_profile_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define IPMC_PROFILE_EXIT()         critd_exit(&ipmc_profile_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define IPMC_PROFILE_ENTER()        critd_enter(&ipmc_profile_crit)
#define IPMC_PROFILE_EXIT()         critd_exit(&ipmc_profile_crit)
#endif /* VTSS_TRACE_ENABLED */


static i32 ipmc_lib_profile_tree_cmp_func(void *elm1, void *elm2)
{
    ipmc_profile_rule_t *element1, *element2;

    if (!elm1 || !elm2) {
        T_W("IPMCLIB_ASSERT(ipmc_lib_profile_tree_cmp_func)");
        for (;;) {}
    }

    element1 = (ipmc_profile_rule_t *)elm1;
    element2 = (ipmc_profile_rule_t *)elm2;
    if (element1->version > element2->version) {
        return 1;
    } else if (element1->version < element2->version) {
        return -1;
    } else {
        int adr_cmp;

        if (element1->version == IPMC_IP_VERSION_IGMP) {
            adr_cmp = IPMC_LIB_ADRS_CMP4(element1->grp_adr, element2->grp_adr);
        } else {
            adr_cmp = IPMC_LIB_ADRS_CMP6(element1->grp_adr, element2->grp_adr);
        }

        if (adr_cmp > 0) {
            return 1;
        } else if (adr_cmp < 0) {
            return -1;
        } else {
            if (element1->vir_idx > element2->vir_idx) {
                return 1;
            } else if (element1->vir_idx < element2->vir_idx) {
                return -1;
            } else {
                return 0;
            }
        }
    }
}

static BOOL _ipmc_lib_profile_tree_create(ipmc_lib_grp_fltr_profile_t *profile)
{
    ipmc_db_ctrl_hdr_t  *pf_tree;
    ipmc_lib_profile_t  *data;
    ipmc_profile_rule_t *def;

    if (!profile || ((data = &profile->data) == NULL)) {
        return FALSE;
    }

    if (data->index > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return FALSE;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, data->index);
    if (!pf_tree || pf_tree->mflag) {
        return FALSE;
    }

    if (!IPMC_LIB_DB_TAKE("IPMC_PROFILE", pf_tree,
                          IPMC_LIB_RULE_CNT_PER_PFT,
                          sizeof(ipmc_profile_rule_t),
                          ipmc_lib_profile_tree_cmp_func)) {
        T_D("IPMC_LIB_DB_TAKE(pf_tree%u) failed", data->index);
        return FALSE;
    }
    pf_tree->mflag = TRUE;

    def = IPMC_LIB_PFT_DEF_RULE(ipmc_lib_def_profile_rule4, data->index);
    (void) IPMC_LIB_DB_ADD(pf_tree, def);
    def = IPMC_LIB_PFT_DEF_RULE(ipmc_lib_def_profile_rule6, data->index);
    (void) IPMC_LIB_DB_ADD(pf_tree, def);

    return TRUE;
}

static BOOL _ipmc_lib_profile_tree_delete(ipmc_lib_grp_fltr_profile_t *profile)
{
    ipmc_db_ctrl_hdr_t  *pf_tree;
    ipmc_lib_profile_t  *data;
    ipmc_profile_rule_t *ptr, *del, r_buf, *def4, *def6;

    if (!profile || ((data = &profile->data) == NULL)) {
        return FALSE;
    }

    if (data->index > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return FALSE;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, data->index);
    if (!pf_tree || !pf_tree->mflag) {
        return FALSE;
    }

    def4 = IPMC_LIB_PFT_DEF_RULE(ipmc_lib_def_profile_rule4, data->index);
    def6 = IPMC_LIB_PFT_DEF_RULE(ipmc_lib_def_profile_rule6, data->index);
    ptr = &r_buf;
    memset(ptr, 0x0, sizeof(ipmc_profile_rule_t));
    del = NULL;
    while (IPMC_LIB_DB_GET_NEXT(pf_tree, ptr)) {
        if (del) {
            if (IPMC_LIB_DB_DEL(pf_tree, del)) {
                if ((del != def4) && (del != def6)) {
                    IPMC_MEM_RULES_MGIVE(del);
                }
            }
            del = NULL;
        }

        del = ptr;
    }
    if (del && IPMC_LIB_DB_DEL(pf_tree, del)) {
        if ((del != def4) && (del != def6)) {
            IPMC_MEM_RULES_MGIVE(del);
        }
    }

    if (!IPMC_LIB_DB_GIVE(pf_tree)) {
        T_D("IPMC_LIB_DB_GIVE(pf_tree%u) failed", data->index);
        return FALSE;
    }
    pf_tree->mflag = FALSE;

    return TRUE;
}

static BOOL _ipmc_lib_profile_tree_clear(ipmc_lib_grp_fltr_profile_t *profile)
{
    ipmc_db_ctrl_hdr_t  *pf_tree;
    ipmc_lib_profile_t  *data;
    ipmc_profile_rule_t *ptr, *del, r_buf, *def4, *def6;
    ipmc_lib_rule_t     *ipmc_lib_def_fltr_rule;

    if (!profile || ((data = &profile->data) == NULL)) {
        return FALSE;
    }

    if (data->index > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return FALSE;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, data->index);
    if (!pf_tree || !pf_tree->mflag) {
        return FALSE;
    }

    def4 = IPMC_LIB_PFT_DEF_RULE(ipmc_lib_def_profile_rule4, data->index);
    def6 = IPMC_LIB_PFT_DEF_RULE(ipmc_lib_def_profile_rule6, data->index);
    ptr = &r_buf;
    memset(ptr, 0x0, sizeof(ipmc_profile_rule_t));
    del = NULL;
    while (IPMC_LIB_DB_GET_NEXT(pf_tree, ptr)) {
        if (del) {
            if (IPMC_LIB_DB_DEL(pf_tree, del)) {
                IPMC_MEM_RULES_MGIVE(del);
            }
            del = NULL;
        }

        ipmc_lib_def_fltr_rule = NULL;
        if (ptr == def4) {
            ipmc_lib_def_fltr_rule = &ipmc_lib_def_fltr_rule4;
        }
        if (ptr == def6) {
            ipmc_lib_def_fltr_rule = &ipmc_lib_def_fltr_rule6;
        }
        if (ipmc_lib_def_fltr_rule && ptr) {
            ptr->rule = ipmc_lib_def_fltr_rule;
            continue;
        }

        del = ptr;
    }
    if (del && IPMC_LIB_DB_DEL(pf_tree, del)) {
        IPMC_MEM_RULES_MGIVE(del);
    }

    return TRUE;
}

static BOOL _ipmc_pft_add_minus1_entry(BOOL *last_rule_def,
                                       ipmc_profile_rule_t *ipmc_lib_def_profile_rule,
                                       ipmc_lib_rule_t *rule_conf,
                                       ipmc_db_ctrl_hdr_t *pf_tree,
                                       ipmc_profile_rule_t *rdx,
                                       vtss_ipv6_t *bgn,
                                       vtss_ipv6_t *end,
                                       ipmc_ip_version_t version)
{
    ipmc_profile_rule_t *rule, *rule_bak;

    if (!last_rule_def || !rule_conf || !pf_tree ||
        !ipmc_lib_def_profile_rule || !rdx || !bgn || !end) {
        return FALSE;
    }

    if (*last_rule_def)  {
        if (!IPMC_MEM_RULES_MTAKE(rule)) {
            return FALSE;
        }

        rule_bak = rule;
        IPMC_LIB_ADRS_CPY(&rule->grp_adr, bgn);
        IPMC_LIB_ADRS_MINUS_ONE(&rule->grp_adr);
        if (ipmc_lib_grp_adrs_version(&rule->grp_adr) != IPMC_IP_VERSION_ERR) {
            rule->version = version;
            rule->vir_idx = 0xFF;
            if (IPMC_LIB_DB_GET(pf_tree, rule)) {
                IPMC_MEM_RULES_MGIVE(rule_bak);
            } else {
                rule->rule = rdx->rule;
                (void) IPMC_LIB_DB_ADD(pf_tree, rule);

                if (rdx != ipmc_lib_def_profile_rule) {
                    if (!IPMC_LIB_ADRS_GREATER(&rdx->grp_adr, end)) {
                        rdx->rule = rule_conf;
                    }
                }
            }
        } else {
            IPMC_MEM_RULES_MGIVE(rule_bak);
        }
    }

    *last_rule_def = TRUE;

    return TRUE;
}

static BOOL _ipmc_lib_profile_tree_construct(ipmc_lib_grp_fltr_profile_t *profile)
{
    u32                         idx, pdx;
    BOOL                        last_rule_def;
    vtss_ipv6_t                 bgn, end;
    ipmc_db_ctrl_hdr_t          *pf_tree;
    ipmc_lib_profile_t          *data;
    ipmc_lib_rule_t             *rule_conf, *ipmc_lib_def_fltr_rule;
    ipmc_lib_grp_fltr_entry_t   *entry_conf;
    ipmc_profile_rule_t         *rule, *rule_bak, *ptr, *ipmc_lib_def_profile_rule, r_buf;

    if (!profile || ((data = &profile->data) == NULL)) {
        return FALSE;
    }

    if ((idx = data->index) > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return FALSE;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, idx);
    if (!pf_tree || !pf_tree->mflag) {
        return FALSE;
    }

    pdx = idx;
    idx = data->rule_head_idx;
    if (idx >= IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
        if (IPMC_LIB_PF_RULE_ID_INIT(idx)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    rule_conf = &profile->rule[idx];
    while (rule_conf->valid) {
        idx = rule_conf->entry_index;
        if (idx > IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
            return FALSE;
        }
        entry_conf = IPMC_LIB_FLTR_POOL_ENTRY(idx);
        if (!entry_conf || !entry_conf->valid) {
            return FALSE;
        }

        IPMC_LIB_ADRS_CPY(&bgn, &entry_conf->grp_bgn);
        IPMC_LIB_ADRS_CPY(&end, &entry_conf->grp_end);
        if (entry_conf->version == IPMC_IP_VERSION_MLD) {
            ipmc_lib_def_profile_rule = IPMC_LIB_PFT_DEF_RULE(ipmc_lib_def_profile_rule6, pdx);
            ipmc_lib_def_fltr_rule = &ipmc_lib_def_fltr_rule6;
        } else {
            ipmc_lib_def_profile_rule = IPMC_LIB_PFT_DEF_RULE(ipmc_lib_def_profile_rule4, pdx);
            ipmc_lib_def_fltr_rule = &ipmc_lib_def_fltr_rule4;
        }

        ptr = &r_buf;
        memset(ptr, 0x0, sizeof(ipmc_profile_rule_t));
        ptr->version = entry_conf->version;
        IPMC_LIB_ADRS_CPY(&ptr->grp_adr, &bgn);
        last_rule_def = TRUE;
        while (IPMC_LIB_DB_GET_NEXT(pf_tree, ptr)) {
            if (!ptr || ptr->version != entry_conf->version) {
                break;
            }

            if (IPMC_LIB_ADRS_EQUAL(&ptr->grp_adr, &bgn) ||
                IPMC_LIB_ADRS_EQUAL(&ptr->grp_adr, &end)) {
                if (ptr->rule == ipmc_lib_def_fltr_rule) {
                    if (!_ipmc_pft_add_minus1_entry(&last_rule_def,
                                                    ipmc_lib_def_profile_rule,
                                                    rule_conf, pf_tree,
                                                    ptr, &bgn, &end,
                                                    entry_conf->version)) {
                        return FALSE;
                    }

                    if (ptr != ipmc_lib_def_profile_rule) {
                        ptr->rule = rule_conf;
                    } else {
                        if (IPMC_LIB_ADRS_EQUAL(&ptr->grp_adr, &end)) {
                            ptr->rule = rule_conf;
                        }
                    }
                } else {
                    last_rule_def = FALSE;
                }

                if (!IPMC_LIB_ADRS_LESS(&ptr->grp_adr, &end)) {
                    break;
                } else {
                    continue;
                }
            }

            if (((ptr == ipmc_lib_def_profile_rule) && (ptr->rule == ipmc_lib_def_fltr_rule)) ||
                (IPMC_LIB_ADRS_GREATER(&ptr->grp_adr, &end) &&
                 last_rule_def &&
                 (ptr->rule == ipmc_lib_def_fltr_rule))) {
                if ((ptr == ipmc_lib_def_profile_rule) &&
                    IPMC_LIB_ADRS_EQUAL(&ptr->grp_adr, &end)) {
                    ptr->rule = rule_conf;
                } else {
                    if (!IPMC_MEM_RULES_MTAKE(rule)) {
                        return FALSE;
                    }

                    rule_bak = rule;
                    rule->version = entry_conf->version;
                    IPMC_LIB_ADRS_CPY(&rule->grp_adr, &end);
                    rule->vir_idx = 0xFF;
                    if (IPMC_LIB_DB_GET(pf_tree, rule)) {
                        IPMC_MEM_RULES_MGIVE(rule_bak);
                    } else {
                        rule->rule = rule_conf;
                        (void) IPMC_LIB_DB_ADD(pf_tree, rule);
                    }
                }
            }

            if (ptr->rule == ipmc_lib_def_fltr_rule) {
                if (!_ipmc_pft_add_minus1_entry(&last_rule_def,
                                                ipmc_lib_def_profile_rule,
                                                rule_conf, pf_tree,
                                                ptr, &bgn, &end,
                                                entry_conf->version)) {
                    return FALSE;
                }
            } else {
                last_rule_def = FALSE;
            }

            if (!IPMC_LIB_ADRS_LESS(&ptr->grp_adr, &end)) {
                break;
            }
        }

        idx = rule_conf->next_rule_idx;
        if (idx >= IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
            if (IPMC_LIB_PF_RULE_ID_INIT(idx)) {
                break;
            }

            return FALSE;
        }
        rule_conf = &profile->rule[idx];
    }

    return TRUE;
}

static BOOL _ipmc_lib_profile_tree_build(ipmc_lib_grp_fltr_profile_t *profile)
{
    BOOL    ret;

    if ((ret = _ipmc_lib_profile_tree_clear(profile)) != FALSE) {
        ret = _ipmc_lib_profile_tree_construct(profile);
    }

    return ret;
}

/* Set Global Filtering Profile State */
vtss_rc ipmc_lib_profile_state_set(BOOL profiling)
{
    if (ipmc_lib_profile->global_ctrl_state != profiling) {
        ipmc_lib_profile->global_ctrl_state = profiling;
    }

    return VTSS_OK;
}

/* Get Global Filtering Profile State */
vtss_rc ipmc_lib_profile_state_get(BOOL *profiling)
{
    if (!profiling) {
        return VTSS_RC_ERROR;
    }

    *profiling = ipmc_lib_profile->global_ctrl_state;

    return VTSS_OK;
}

static vtss_rc _ipmc_lib_fltr_entry_add(ipmc_lib_grp_fltr_entry_t *entry)
{
    u32                         idx;
    ipmc_lib_conf_fltr_entry_t  *conf;

    for (idx = 1; idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT; idx++) {
        conf = IPMC_LIB_FLTR_POOL_ENTRY(idx);
        if (!conf || conf->valid) {
            continue;
        }

        conf->valid = TRUE;
        conf->index = idx;
        memset(conf->name, 0x0, sizeof(conf->name));
        strncpy(conf->name, entry->name, strlen(entry->name));
        conf->version = entry->version;
        IPMC_LIB_ADRS_CPY(&conf->grp_bgn, &entry->grp_bgn);
        IPMC_LIB_ADRS_CPY(&conf->grp_end, &entry->grp_end);

        return VTSS_OK;
    }

    return VTSS_RC_ERROR;
}

static vtss_rc _ipmc_lib_fltr_entry_del(ipmc_lib_grp_fltr_entry_t *entry)
{
    u32                             idx, pdx;
    ipmc_lib_conf_fltr_entry_t      *conf;
    ipmc_lib_profile_t              *data;
    ipmc_lib_rule_t                 *rule, *prev;
    ipmc_lib_conf_fltr_profile_t    *profile;

    idx = entry->index;
    conf = IPMC_LIB_FLTR_POOL_ENTRY(idx);
    if (!conf || !conf->valid) {
        return VTSS_RC_ERROR;
    }

    if (memcmp(conf->name, entry->name, sizeof(entry->name))) {
        return VTSS_RC_ERROR;
    }

    for (pdx = 1; pdx <= IPMC_LIB_FLTR_PROFILE_MAX_CNT; pdx++) {
        if ((profile = IPMC_LIB_FLTR_POOL_PROFILE(pdx)) == NULL) {
            continue;
        }

        data = &profile->data;
        if (!data->valid) {
            continue;
        }

        prev = rule = NULL;
        idx = data->rule_head_idx;
        if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
            rule = &profile->rule[idx];
        }
        while (rule && rule->valid) {
            if (rule->entry_index == entry->index) {
                if (prev) {
                    prev->next_rule_idx = rule->next_rule_idx;
                } else {
                    data->rule_head_idx = rule->next_rule_idx;
                }

                idx = rule->idx;
                memset(rule, 0x0, sizeof(ipmc_lib_rule_t));
                rule->idx = idx;
                rule->entry_index = IPMC_LIB_FLTR_RULE_IDX_INIT;
                rule->next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;

                if (!_ipmc_lib_profile_tree_build((ipmc_lib_grp_fltr_profile_t *)profile)) {
                    T_D("Rebuild %s-PFT(%s) failed!", data->name, entry->name);
                }

                break;
            }

            idx = rule->next_rule_idx;
            if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
                prev = rule;
                rule = &profile->rule[idx];
            } else {
                break;
            }
        }
    }

    memset(conf, 0x0, sizeof(ipmc_lib_conf_fltr_entry_t));
    conf->index = entry->index;
    conf->version = IPMC_IP_VERSION_INIT;

    return VTSS_OK;
}

static vtss_rc _ipmc_lib_fltr_entry_upd(ipmc_lib_grp_fltr_entry_t *entry)
{
    BOOL                        chk_tree;
    u32                         idx;
    ipmc_lib_conf_fltr_entry_t  *conf;

    idx = entry->index;
    conf = IPMC_LIB_FLTR_POOL_ENTRY(idx);
    if (!conf || !conf->valid) {
        return VTSS_RC_ERROR;
    }

    if (memcmp(conf->name, entry->name, sizeof(entry->name))) {
        return VTSS_RC_ERROR;
    }

    conf->version = entry->version;
    chk_tree = FALSE;
    if (!IPMC_LIB_ADRS_EQUAL(&conf->grp_bgn, &entry->grp_bgn)) {
        chk_tree = TRUE;
        IPMC_LIB_ADRS_CPY(&conf->grp_bgn, &entry->grp_bgn);
    }
    if (!IPMC_LIB_ADRS_EQUAL(&conf->grp_end, &entry->grp_end)) {
        chk_tree = TRUE;
        IPMC_LIB_ADRS_CPY(&conf->grp_end, &entry->grp_end);
    }

    if (chk_tree) {
        u32                             pdx;
        ipmc_lib_profile_t              *data;
        ipmc_lib_rule_t                 *rule;
        ipmc_lib_conf_fltr_profile_t    *profile;

        for (pdx = 1; pdx <= IPMC_LIB_FLTR_PROFILE_MAX_CNT; pdx++) {
            if ((profile = IPMC_LIB_FLTR_POOL_PROFILE(pdx)) == NULL) {
                continue;
            }

            data = &profile->data;
            if (!data->valid) {
                continue;
            }

            rule = NULL;
            idx = data->rule_head_idx;
            if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
                rule = &profile->rule[idx];
            }
            while (rule && rule->valid) {
                if (rule->entry_index == entry->index) {
                    if (!_ipmc_lib_profile_tree_build((ipmc_lib_grp_fltr_profile_t *)profile)) {
                        T_D("Rebuild %s-PFT(%s) failed!", data->name, entry->name);
                    }

                    break;
                }

                idx = rule->next_rule_idx;
                if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
                    rule = &profile->rule[idx];
                } else {
                    break;
                }
            }
        }
    }

    return VTSS_OK;
}

static BOOL _ipmc_lib_fltr_entry_sanity(ipmc_lib_grp_fltr_entry_t *entry)
{
    ipmc_ip_version_t   version;

    if (entry->index > IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
        return FALSE;
    }

    if (!strlen(entry->name)) {
        return FALSE;
    }

    if ((version = ipmc_lib_grp_adrs_version(&entry->grp_bgn)) == IPMC_IP_VERSION_ERR) {
        return FALSE;
    }
    if (version != ipmc_lib_grp_adrs_version(&entry->grp_end)) {
        return FALSE;
    }

    if (IPMC_LIB_ADRS_GREATER(&entry->grp_bgn, &entry->grp_end)) {
        return FALSE;
    }

    entry->version = version;
    return TRUE;
}

/* Add/Delete/Update IPMC Profile Entry */
vtss_rc ipmc_lib_fltr_entry_set(ipmc_operation_action_t action, ipmc_lib_grp_fltr_entry_t *fltr_entry)
{
    vtss_rc retVal;
    i8      v6adrString1[40], v6adrString2[40];

    if (!fltr_entry || !_ipmc_lib_fltr_entry_sanity(fltr_entry)) {
        return VTSS_RC_ERROR;
    }

    memset(v6adrString1, 0x0, sizeof(v6adrString1));
    memset(v6adrString2, 0x0, sizeof(v6adrString1));
    T_I("Start %s %s %s %s", ipmc_lib_op_action_txt(action),
        ipmc_lib_version_txt(fltr_entry->version, IPMC_TXT_CASE_UPPER),
        misc_ipv6_txt(&fltr_entry->grp_bgn, v6adrString1),
        misc_ipv6_txt(&fltr_entry->grp_end, v6adrString2));
    retVal = VTSS_RC_ERROR;
    switch ( action ) {
    case IPMC_OP_ADD:
        retVal = _ipmc_lib_fltr_entry_add(fltr_entry);

        break;
    case IPMC_OP_DEL:
        retVal = _ipmc_lib_fltr_entry_del(fltr_entry);

        break;
    case IPMC_OP_UPD:
        retVal = _ipmc_lib_fltr_entry_upd(fltr_entry);

        break;
    default:

        break;
    }

    T_I("Complete with RC=%d", retVal);
    return retVal;
}

/* Get IPMC Profile Entry */
ipmc_lib_grp_fltr_entry_t *ipmc_lib_fltr_entry_get(ipmc_lib_grp_fltr_entry_t *fltr_entry, BOOL by_name)
{
    u32                         idx;
    ipmc_lib_grp_fltr_entry_t   *ptr;

    if (!fltr_entry) {
        return NULL;
    }

    ptr = NULL;
    if (by_name) {
        ipmc_lib_grp_fltr_entry_t   *conf;

        for (idx = 1; idx <= IPMC_LIB_FLTR_ENTRY_MAX_CNT; idx++) {
            conf = IPMC_LIB_FLTR_POOL_ENTRY(idx);
            if (!conf || !conf->valid ||
                memcmp(conf->name, fltr_entry->name, sizeof(fltr_entry->name))) {
                continue;
            }

            ptr = conf;
            break;
        }
    } else {
        ptr = IPMC_LIB_FLTR_POOL_ENTRY(fltr_entry->index);
        if (ptr && !ptr->valid) {
            ptr = NULL;
        }
    }

    return ptr;
}

/* GetNext IPMC Profile Entry */
ipmc_lib_grp_fltr_entry_t *ipmc_lib_fltr_entry_get_next(ipmc_lib_grp_fltr_entry_t *fltr_entry, BOOL by_name)
{
    u32                         idx;
    ipmc_lib_grp_fltr_entry_t   *ptr;

    if (!fltr_entry) {
        return NULL;
    }

    ptr = NULL;
    if (by_name) {
        ipmc_lib_grp_fltr_entry_t   *conf, *tmp;

        tmp = NULL;
        for (idx = 1; idx <= IPMC_LIB_FLTR_ENTRY_MAX_CNT; idx++) {
            conf = IPMC_LIB_FLTR_POOL_ENTRY(idx);
            if (!conf || !conf->valid) {
                continue;
            }

            if (memcmp(conf->name, fltr_entry->name, sizeof(fltr_entry->name)) > 0) {
                if (tmp) {
                    if (memcmp(conf->name, tmp->name, sizeof(tmp->name)) < 0) {
                        tmp = conf;
                    }
                } else {
                    tmp = conf;
                }
            }
        }

        ptr = tmp;
    } else {
        for (idx = fltr_entry->index + 1; idx <= IPMC_LIB_FLTR_ENTRY_MAX_CNT; idx++) {
            if (IPMC_LIB_FLTR_POOL_EN_VALID(idx)) {
                ptr = IPMC_LIB_FLTR_POOL_ENTRY(idx);
                break;
            }
        }
    }

    return ptr;
}

static vtss_rc _ipmc_lib_fltr_profile_add(ipmc_lib_grp_fltr_profile_t *profile)
{
    u32                             idx;
    int                             str_len;
    ipmc_lib_profile_t              *data, *entry;
    ipmc_lib_conf_fltr_profile_t    *conf;

    entry = &profile->data;
    for (idx = 1; idx <= IPMC_LIB_FLTR_PROFILE_MAX_CNT; idx++) {
        if ((conf = IPMC_LIB_FLTR_POOL_PROFILE(idx)) == NULL) {
            continue;
        }

        data = &conf->data;
        if (data->valid) {
            continue;
        }

        data->valid = TRUE;
        data->index = idx;
        memset(data->name, 0x0, sizeof(data->name));
        strncpy(data->name, entry->name, strlen(entry->name));
        memset(data->desc, 0x0, sizeof(data->desc));
        if ((str_len = strlen(entry->desc)) != 0) {
            strncpy(data->desc, entry->desc, str_len);
        }
        data->version = IPMC_IP_VERSION_INIT;
        data->rule_head_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;

        if (_ipmc_lib_profile_tree_create((ipmc_lib_grp_fltr_profile_t *)conf)) {
            return VTSS_OK;
        } else {
            memset(data, 0x0, sizeof(ipmc_lib_profile_t));
            data->index = idx;
            data->version = IPMC_IP_VERSION_INIT;
            data->rule_head_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
        }

        break;
    }

    return VTSS_RC_ERROR;
}

static vtss_rc _ipmc_lib_fltr_profile_del(ipmc_lib_grp_fltr_profile_t *profile)
{
    u32                             idx;
    ipmc_lib_profile_t              *data, *entry;
    ipmc_lib_rule_t                 *rule;
    ipmc_lib_conf_fltr_profile_t    *conf;

    entry = &profile->data;
    idx = entry->index;
    if ((conf = IPMC_LIB_FLTR_POOL_PROFILE(idx)) == NULL) {
        return VTSS_RC_ERROR;
    }

    data = &conf->data;
    if (!data->valid) {
        return VTSS_RC_ERROR;
    }

    if (memcmp(data->name, entry->name, sizeof(entry->name))) {
        return VTSS_RC_ERROR;
    }

    if (!_ipmc_lib_profile_tree_delete((ipmc_lib_grp_fltr_profile_t *)conf)) {
        return VTSS_RC_ERROR;
    }

    memset(conf, 0x0, sizeof(ipmc_lib_conf_fltr_profile_t));
    data->index = idx;
    data->version = IPMC_IP_VERSION_INIT;
    data->rule_head_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
    for (idx = 0; idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT; idx++) {
        rule = &conf->rule[idx];

        rule->idx = idx;
        rule->entry_index = IPMC_LIB_FLTR_RULE_IDX_INIT;
        rule->next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
    }

    return VTSS_OK;
}

static vtss_rc _ipmc_lib_fltr_profile_upd(ipmc_lib_grp_fltr_profile_t *profile)
{
    u32                             idx;
    BOOL                            chk_tree;
    int                             str_len;
    ipmc_lib_profile_t              *data, *entry;
    ipmc_lib_rule_t                 *rule;
    ipmc_lib_conf_fltr_entry_t      *adrs;
    ipmc_lib_conf_fltr_profile_t    *conf;

    entry = &profile->data;
    idx = entry->index;
    if ((conf = IPMC_LIB_FLTR_POOL_PROFILE(idx)) == NULL) {
        return VTSS_RC_ERROR;
    }

    data = &conf->data;
    if (!data->valid) {
        return VTSS_RC_ERROR;
    }

    if (memcmp(data->name, entry->name, sizeof(entry->name))) {
        return VTSS_RC_ERROR;
    }

    chk_tree = FALSE;
    if (data->rule_head_idx != entry->rule_head_idx) {
        data->rule_head_idx = entry->rule_head_idx;
        chk_tree = TRUE;
    }

    memset(data->desc, 0x0, sizeof(data->desc));
    if ((str_len = strlen(entry->desc)) != 0) {
        strncpy(data->desc, entry->desc, str_len);
    }

    rule = NULL;
    idx = data->rule_head_idx;
    if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
        rule = &conf->rule[idx];
    }
    while (rule && rule->valid) {
        idx = rule->entry_index;
        if (idx <= IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
            adrs = IPMC_LIB_FLTR_POOL_ENTRY(idx);
            if (adrs && adrs->valid) {
                if (data->version == IPMC_IP_VERSION_INIT) {
                    data->version = adrs->version;
                    chk_tree = TRUE;
                } else {
                    if (data->version != adrs->version) {
                        data->version = IPMC_IP_VERSION_ALL;
                        chk_tree = TRUE;
                        break;
                    }
                }
            }
        }

        idx = rule->next_rule_idx;
        if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
            rule = &conf->rule[idx];
        } else {
            break;
        }
    }

    if (chk_tree) {
        if (!_ipmc_lib_profile_tree_build((ipmc_lib_grp_fltr_profile_t *)conf)) {
            return VTSS_RC_ERROR;
        }
    }

    return VTSS_OK;
}

static vtss_rc _ipmc_lib_fltr_profile_set(ipmc_lib_grp_fltr_profile_t *profile)
{
    u32                             idx;
    BOOL                            chk_tree;
    int                             str_len;
    ipmc_lib_profile_t              *data, *entry;
    ipmc_lib_rule_t                 *rule;
    ipmc_lib_conf_fltr_entry_t      *adrs;
    ipmc_lib_conf_fltr_profile_t    *conf;

    entry = &profile->data;
    idx = entry->index;
    if ((conf = IPMC_LIB_FLTR_POOL_PROFILE(idx)) == NULL) {
        return VTSS_RC_ERROR;
    }

    data = &conf->data;
    if (!data->valid) {
        if (_ipmc_lib_fltr_profile_add(profile) != VTSS_OK) {
            return VTSS_RC_ERROR;
        }
    } else {
        ipmc_db_ctrl_hdr_t  *pf_tree;

        pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, data->index);
        if (!pf_tree || !pf_tree->mflag) {
            if (!_ipmc_lib_profile_tree_create((ipmc_lib_grp_fltr_profile_t *)conf)) {
                return VTSS_RC_ERROR;
            }
        }
    }

    chk_tree = FALSE;
    if (data != entry) {
        if (memcmp(data->name, entry->name, sizeof(entry->name))) {
            return VTSS_RC_ERROR;
        }

        if (data->rule_head_idx != entry->rule_head_idx) {
            data->rule_head_idx = entry->rule_head_idx;
            chk_tree = TRUE;
        }

        memset(data->desc, 0x0, sizeof(data->desc));
        if ((str_len = strlen(entry->desc)) != 0) {
            strncpy(data->desc, entry->desc, str_len);
        }
    } else {
        if (!_ipmc_lib_profile_tree_build((ipmc_lib_grp_fltr_profile_t *)conf)) {
            return VTSS_RC_ERROR;
        } else {
            return VTSS_OK;
        }
    }

    rule = NULL;
    idx = data->rule_head_idx;
    if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
        rule = &conf->rule[idx];
    }
    while (rule && rule->valid) {
        idx = rule->entry_index;
        if (idx <= IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
            adrs = IPMC_LIB_FLTR_POOL_ENTRY(idx);
            if (adrs && adrs->valid) {
                if (data->version == IPMC_IP_VERSION_INIT) {
                    data->version = adrs->version;
                    chk_tree = TRUE;
                } else {
                    if (data->version != adrs->version) {
                        data->version = IPMC_IP_VERSION_ALL;
                        chk_tree = TRUE;
                        break;
                    }
                }
            }
        }

        idx = rule->next_rule_idx;
        if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
            rule = &conf->rule[idx];
        } else {
            break;
        }
    }

    if (chk_tree) {
        if (!_ipmc_lib_profile_tree_build((ipmc_lib_grp_fltr_profile_t *)conf)) {
            return VTSS_RC_ERROR;
        }
    }

    return VTSS_OK;
}

static BOOL _ipmc_lib_fltr_profile_sanity(ipmc_lib_grp_fltr_profile_t *entry)
{
    ipmc_lib_profile_t  *data = &entry->data;

    if (data->index > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return FALSE;
    }

    if (!strlen(data->name)) {
        return FALSE;
    }

    return TRUE;
}

/* Add/Delete/Update IPMC Profile */
vtss_rc ipmc_lib_fltr_profile_set(ipmc_operation_action_t action, ipmc_lib_grp_fltr_profile_t *fltr_profile)
{
    vtss_rc                     retVal;
    u32                         rdx, edx;
    ipmc_lib_profile_t          *data;
    ipmc_lib_rule_t             *rule;
    ipmc_lib_grp_fltr_entry_t   *entry;

    if (!fltr_profile || !_ipmc_lib_fltr_profile_sanity(fltr_profile)) {
        return VTSS_RC_ERROR;
    }

    T_I("Start %s %s %s", ipmc_lib_op_action_txt(action),
        ipmc_lib_version_txt(fltr_profile->data.version, IPMC_TXT_CASE_UPPER),
        fltr_profile->data.name);
    retVal = VTSS_RC_ERROR;
    switch ( action ) {
    case IPMC_OP_ADD:
        retVal = _ipmc_lib_fltr_profile_add(fltr_profile);

        break;
    case IPMC_OP_DEL:
        retVal = _ipmc_lib_fltr_profile_del(fltr_profile);

        break;
    case IPMC_OP_UPD:
        retVal = _ipmc_lib_fltr_profile_upd(fltr_profile);

        break;
    case IPMC_OP_SET:
        retVal = _ipmc_lib_fltr_profile_set(fltr_profile);
        if (retVal != VTSS_OK) {
            break;
        }

        data = &fltr_profile->data;
        data->version = IPMC_IP_VERSION_INIT;
        for (rdx = data->rule_head_idx; IPMC_LIB_PF_RULE_ID_VALID(rdx); rdx = rule->next_rule_idx) {
            rule = &fltr_profile->rule[rdx];
            edx = rule->entry_index;
            if ((edx != IPMC_LIB_FLTR_RULE_IDX_INIT) &&
                (edx <= IPMC_LIB_FLTR_ENTRY_MAX_CNT)) {
                entry = IPMC_LIB_FLTR_POOL_ENTRY(edx);
                if (!entry) {
                    continue;
                }

                if (data->version == IPMC_IP_VERSION_INIT) {
                    data->version = entry->version;
                } else {
                    if (data->version != entry->version) {
                        data->version = IPMC_IP_VERSION_ALL;
                        break;
                    }
                }
            }
        }

        break;
    default:

        break;
    }

    T_I("Complete with RC=%d", retVal);
    return retVal;
}

/* Get IPMC Profile */
ipmc_lib_grp_fltr_profile_t *ipmc_lib_fltr_profile_get(ipmc_lib_grp_fltr_profile_t *fltr_profile, BOOL by_name)
{
    u32                         idx;
    ipmc_lib_grp_fltr_profile_t *ptr;

    if (!fltr_profile) {
        return NULL;
    }

    ptr = NULL;
    if (by_name) {
        ipmc_lib_profile_t          *data, *p;
        ipmc_lib_grp_fltr_profile_t *conf;

        p = &fltr_profile->data;
        for (idx = 1; idx <= IPMC_LIB_FLTR_PROFILE_MAX_CNT; idx++) {
            if ((conf = IPMC_LIB_FLTR_POOL_PROFILE(idx)) == NULL) {
                continue;
            }

            data = &conf->data;
            if (!data->valid ||
                memcmp(data->name, p->name, sizeof(p->name))) {
                continue;
            }

            ptr = conf;
            break;
        }
    } else {
        ptr = IPMC_LIB_FLTR_POOL_PROFILE(fltr_profile->data.index);
        if (ptr && !ptr->data.valid) {
            ptr = NULL;
        }
    }

    return ptr;
}

/* GetNext IPMC Profile */
ipmc_lib_grp_fltr_profile_t *ipmc_lib_fltr_profile_get_next(ipmc_lib_grp_fltr_profile_t *fltr_profile, BOOL by_name)
{
    u32                         idx;
    ipmc_lib_grp_fltr_profile_t *ptr;

    if (!fltr_profile) {
        return NULL;
    }

    ptr = NULL;
    if (by_name) {
        ipmc_lib_profile_t          *data, *p, *q;
        ipmc_lib_grp_fltr_profile_t *conf, *tmp;

        tmp = NULL;
        p = &fltr_profile->data;
        for (idx = 1; idx <= IPMC_LIB_FLTR_PROFILE_MAX_CNT; idx++) {
            if ((conf = IPMC_LIB_FLTR_POOL_PROFILE(idx)) == NULL) {
                continue;
            }

            data = &conf->data;
            if (!data->valid) {
                continue;
            }

            if (memcmp(data->name, p->name, sizeof(p->name)) > 0) {
                if (tmp) {
                    q = &tmp->data;
                    if (memcmp(data->name, q->name, sizeof(q->name)) < 0) {
                        tmp = conf;
                    }
                } else {
                    tmp = conf;
                }
            }
        }

        ptr = tmp;
    } else {
        for (idx = fltr_profile->data.index + 1; idx <= IPMC_LIB_FLTR_PROFILE_MAX_CNT; idx++) {
            if (IPMC_LIB_FLTR_POOL_PF_VALID(idx)) {
                ptr = IPMC_LIB_FLTR_POOL_PROFILE(idx);
                break;
            }
        }
    }

    return ptr;
}

static vtss_rc _ipmc_lib_fltr_rule_add(ipmc_lib_conf_fltr_profile_t *profile, ipmc_lib_rule_t *entry, BOOL *chk)
{
    u32                 idx, rdx, ndx;
    BOOL                next;
    ipmc_lib_profile_t  *data;
    ipmc_lib_rule_t     *rule;

    if (!IPMC_LIB_FLTR_POOL_EN_VALID(entry->entry_index)) {
        return VTSS_RC_ERROR;
    }

    for (idx = 0; idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT; idx++) {
        rule = &profile->rule[idx];
        if (!rule->valid) {
            break;
        }
    }

    next = FALSE;
    rdx = ndx = IPMC_LIB_FLTR_RULE_IDX_INIT;
    if (idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
        rdx = idx;

        idx = entry->next_rule_idx;
        if (idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
            ndx = idx;
            next = TRUE;
        } else {
            if (!IPMC_LIB_PF_RULE_ID_INIT(idx)) {
                return VTSS_RC_ERROR;
            }
        }
    } else {
        return VTSS_RC_ERROR;
    }

    data = &profile->data;
    idx = data->rule_head_idx;
    if (IPMC_LIB_PF_RULE_ID_INIT(idx)) {
        if (next) {
            return VTSS_RC_ERROR;
        }

        rule->next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
        data->rule_head_idx = rdx;
    } else {
        ipmc_lib_rule_t *ptr, *prev;

        prev = ptr = NULL;
        if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
            ptr = &profile->rule[idx];
        }
        while (ptr && ptr->valid) {
            idx = ptr->idx;
            if (next && (idx == ndx)) {
                if (prev) {
                    rule->next_rule_idx = prev->next_rule_idx;
                    prev->next_rule_idx = rdx;
                } else {
                    rule->next_rule_idx = data->rule_head_idx;
                    data->rule_head_idx = rdx;
                }

                break;
            }

            idx = ptr->next_rule_idx;
            if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
                prev = ptr;
                ptr = &profile->rule[idx];
            } else {
                ptr->next_rule_idx = rdx;
                rule->next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;

                break;
            }
        }
    }

    rule->valid = TRUE;
    rule->idx = rdx;
    rule->entry_index = entry->entry_index;
    rule->action = entry->action;
    rule->log = entry->log;
    *chk = TRUE;

    return VTSS_OK;
}

static vtss_rc _ipmc_lib_fltr_rule_del(ipmc_lib_conf_fltr_profile_t *profile, ipmc_lib_rule_t *entry, BOOL *chk)
{
    u32                 idx, rdx;
    ipmc_lib_profile_t  *data;
    ipmc_lib_rule_t     *rule;

    if (!IPMC_LIB_FLTR_POOL_EN_VALID(entry->entry_index)) {
        return VTSS_RC_ERROR;
    }

    idx = entry->idx;
    if (idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
        rule = &profile->rule[idx];
        if (!rule->valid) {
            return VTSS_RC_ERROR;
        }

        rdx = idx;
    } else {
        return VTSS_RC_ERROR;
    }

    data = &profile->data;
    idx = data->rule_head_idx;
    if (IPMC_LIB_PF_RULE_ID_INIT(idx)) {
        return VTSS_RC_ERROR;
    } else {
        ipmc_lib_rule_t *ptr, *prev;

        prev = ptr = NULL;
        if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
            ptr = &profile->rule[idx];
        } else {
            return VTSS_RC_ERROR;
        }
        while (ptr && ptr->valid) {
            if (ptr->idx == rdx) {
                if (prev) {
                    prev->next_rule_idx = rule->next_rule_idx;
                } else {
                    data->rule_head_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
                }

                break;
            }

            idx = ptr->next_rule_idx;
            if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
                prev = ptr;
                ptr = &profile->rule[idx];
            } else {
                return VTSS_RC_ERROR;
            }
        }
    }

    memset(rule, 0x0, sizeof(ipmc_lib_rule_t));
    rule->idx = rdx;
    rule->entry_index = IPMC_LIB_FLTR_RULE_IDX_INIT;
    rule->next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
    *chk = TRUE;

    return VTSS_OK;
}

static vtss_rc _ipmc_lib_fltr_rule_upd(ipmc_lib_conf_fltr_profile_t *profile, ipmc_lib_rule_t *entry, BOOL *chk)
{
    u32                 idx, rdx;
    ipmc_lib_profile_t  *data;
    ipmc_lib_rule_t     *rule;

    if (!IPMC_LIB_FLTR_POOL_EN_VALID(entry->entry_index)) {
        return VTSS_RC_ERROR;
    }

    idx = entry->idx;
    if (idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
        rule = &profile->rule[idx];
        if (!rule->valid) {
            return VTSS_RC_ERROR;
        }

        if (rule->next_rule_idx != entry->next_rule_idx) {
            if (_ipmc_lib_fltr_rule_del(profile, entry, chk) != VTSS_OK) {
                *chk = FALSE;
                return VTSS_RC_ERROR;
            }
            if (_ipmc_lib_fltr_rule_add(profile, entry, chk) != VTSS_OK) {
                *chk = FALSE;
                return VTSS_RC_ERROR;
            }

            *chk = TRUE;
            return VTSS_OK;
        }

        rdx = idx;
    } else {
        return VTSS_RC_ERROR;
    }

    data = &profile->data;
    idx = data->rule_head_idx;
    if (IPMC_LIB_PF_RULE_ID_INIT(idx)) {
        return VTSS_RC_ERROR;
    } else {
        ipmc_lib_rule_t *ptr;

        ptr = NULL;
        if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
            ptr = &profile->rule[idx];
        } else {
            return VTSS_RC_ERROR;
        }
        while (ptr && ptr->valid) {
            if (ptr->idx == rdx) {
                if (rule->action != entry->action) {
                    *chk = TRUE;
                    rule->action = entry->action;
                }
                rule->log = entry->log;

                break;
            }

            idx = ptr->next_rule_idx;
            if (IPMC_LIB_PF_RULE_ID_VALID(idx)) {
                ptr = &profile->rule[idx];
            } else {
                return VTSS_RC_ERROR;
            }
        }
    }

    return VTSS_OK;
}

static BOOL _ipmc_lib_fltr_rule_sanity(ipmc_operation_action_t action,
                                       ipmc_lib_conf_fltr_profile_t *profile,
                                       ipmc_lib_rule_t *entry)
{
    u32 rdx = entry->idx;

    if (!IPMC_LIB_PF_RULE_ID_INIT(rdx) && (rdx >= IPMC_LIB_FLTR_ENTRY_MAX_CNT)) {
        return FALSE;
    }

    if (entry->entry_index > IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
        return FALSE;
    }

    if ((action != IPMC_OP_DEL) && !IPMC_LIB_PF_RULE_ID_INIT(entry->next_rule_idx)) {
        u32     ndx;
        BOOL    next;

        next = FALSE;
        ndx = entry->next_rule_idx;
        if (ndx < IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
            next = TRUE;
        } else {
            return FALSE;
        }

        if (next) {
            ipmc_lib_profile_t  *data;

            /* cannot form a loop in precedence */
            if (!IPMC_LIB_PF_RULE_ID_INIT(rdx) && (ndx == rdx)) {
                return FALSE;
            }

            data = &profile->data;
            rdx = data->rule_head_idx;
            if (IPMC_LIB_PF_RULE_ID_VALID(rdx)) {
                ipmc_lib_rule_t *ptr = &profile->rule[rdx];

                next = FALSE;
                while (ptr && ptr->valid) {
                    if (ptr->idx == ndx) {
                        next = TRUE;
                        break;
                    }

                    rdx = ptr->next_rule_idx;
                    if (IPMC_LIB_PF_RULE_ID_VALID(rdx)) {
                        ptr = &profile->rule[rdx];
                    } else {
                        break;
                    }
                }
            } else {
                next = FALSE;
            }

            if (!next) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

/* Add/Delete/Update IPMC Profile Rule */
vtss_rc ipmc_lib_fltr_profile_rule_set(ipmc_operation_action_t action, u32 profile_index, ipmc_lib_rule_t *fltr_rule)
{
    vtss_rc                         retVal;
    u32                             rdx, edx;
    BOOL                            chk_tree;
    ipmc_lib_grp_fltr_entry_t       *entry;
    ipmc_lib_rule_t                 *rule;
    ipmc_lib_profile_t              *data;
    ipmc_lib_conf_fltr_profile_t    *conf;

    if (!fltr_rule) {
        return VTSS_RC_ERROR;
    }

    conf = IPMC_LIB_FLTR_POOL_PROFILE(profile_index);
    if (!conf || ((data = &conf->data) == NULL)) {
        return IPMC_ERROR_ENTRY_INVALID;
    }

    if (!data->valid) {
        return IPMC_ERROR_ENTRY_NOT_FOUND;
    }

    if (!_ipmc_lib_fltr_rule_sanity(action, conf, fltr_rule)) {
        return IPMC_ERROR_ENTRY_INVALID;
    }

    T_I("Start %s %s %s RDX:%u/EDX:%u/NDX:%u/%s/%s",
        ipmc_lib_op_action_txt(action),
        ipmc_lib_version_txt(data->version, IPMC_TXT_CASE_UPPER),
        data->name,
        fltr_rule->idx,
        fltr_rule->entry_index,
        fltr_rule->next_rule_idx,
        ipmc_lib_fltr_action_txt(fltr_rule->action, IPMC_TXT_CASE_CAPITAL),
        fltr_rule->log ? "LOG" : "BYPASS");
    retVal = VTSS_RC_ERROR;
    chk_tree = FALSE;
    switch ( action ) {
    case IPMC_OP_ADD:
        retVal = _ipmc_lib_fltr_rule_add(conf, fltr_rule, &chk_tree);

        break;
    case IPMC_OP_DEL:
        retVal = _ipmc_lib_fltr_rule_del(conf, fltr_rule, &chk_tree);

        break;
    case IPMC_OP_UPD:
        retVal = _ipmc_lib_fltr_rule_upd(conf, fltr_rule, &chk_tree);

        break;
    default:

        break;
    }

    data->version = IPMC_IP_VERSION_INIT;
    for (rdx = data->rule_head_idx; IPMC_LIB_PF_RULE_ID_VALID(rdx); rdx = rule->next_rule_idx) {
        rule = &conf->rule[rdx];
        edx = rule->entry_index;
        if ((edx != IPMC_LIB_FLTR_RULE_IDX_INIT) &&
            (edx <= IPMC_LIB_FLTR_ENTRY_MAX_CNT)) {
            entry = IPMC_LIB_FLTR_POOL_ENTRY(edx);
            if (!entry) {
                continue;
            }

            if (data->version == IPMC_IP_VERSION_INIT) {
                data->version = entry->version;
            } else {
                if (data->version != entry->version) {
                    data->version = IPMC_IP_VERSION_ALL;
                    break;
                }
            }
        }
    }

    if (chk_tree && !_ipmc_lib_profile_tree_build((ipmc_lib_grp_fltr_profile_t *)conf)) {
        T_D("Rebuild %s-PFT failed!", data->name);
    }
    T_I("Complete with RC=%d (%s %s)", retVal, ipmc_lib_version_txt(data->version, IPMC_TXT_CASE_UPPER), data->name);
    return retVal;
}

/* Search IPMC Profile Rule by Entry Index */
ipmc_lib_rule_t *ipmc_lib_fltr_profile_rule_search(u32 profile_index, u32 entry_index)
{
    u32                         rule_idx;
    ipmc_lib_rule_t             *rule;
    ipmc_lib_grp_fltr_profile_t *profile;

    if (!IPMC_LIB_FLTR_POOL_PF_VALID(profile_index)) {
        return NULL;
    }

    if ((profile = IPMC_LIB_FLTR_POOL_PROFILE(profile_index)) == NULL) {
        return NULL;
    }

    for (rule_idx = 0; rule_idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT; rule_idx++) {
        rule = &profile->rule[rule_idx];
        if (!rule->valid) {
            continue;
        }

        if (rule->entry_index == entry_index) {
            return rule;
        }
    }

    return NULL;
}

/* Get IPMC Profile Rule */
ipmc_lib_rule_t *ipmc_lib_fltr_profile_rule_get(u32 profile_index, ipmc_lib_rule_t *fltr_rule)
{
    u32                         rule_idx;
    ipmc_lib_rule_t             *rule;
    ipmc_lib_grp_fltr_profile_t *profile;

    if (!fltr_rule) {
        return NULL;
    }

    if (!IPMC_LIB_FLTR_POOL_PF_VALID(profile_index)) {
        return NULL;
    }

    if ((profile = IPMC_LIB_FLTR_POOL_PROFILE(profile_index)) == NULL) {
        return NULL;
    }

    rule_idx = fltr_rule->idx;
    if (rule_idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
        rule = &profile->rule[rule_idx];
    } else {
        return NULL;
    }

    if (!rule->valid) {
        return NULL;
    } else {
        return rule;
    }
}

/* GetFirst IPMC Profile Rule */
ipmc_lib_rule_t *ipmc_lib_fltr_profile_rule_get_first(u32 profile_index)
{
    u32                         rule_idx;
    ipmc_lib_rule_t             *rule;
    ipmc_lib_grp_fltr_profile_t *profile;

    if (!IPMC_LIB_FLTR_POOL_PF_VALID(profile_index)) {
        return NULL;
    }

    if ((profile = IPMC_LIB_FLTR_POOL_PROFILE(profile_index)) == NULL) {
        return NULL;
    }

    rule = NULL;
    rule_idx = profile->data.rule_head_idx;
    if (IPMC_LIB_PF_RULE_ID_VALID(rule_idx)) {
        rule = &profile->rule[rule_idx];
        if (!rule->valid) {
            rule = NULL;
        }
    }

    return rule;
}

/* GetNext IPMC Profile Rule */
ipmc_lib_rule_t *ipmc_lib_fltr_profile_rule_get_next(u32 profile_index, ipmc_lib_rule_t *fltr_rule)
{
    u32                         rule_idx;
    ipmc_lib_rule_t             *rule;
    ipmc_lib_grp_fltr_profile_t *profile;

    if (!fltr_rule) {
        return NULL;
    }

    if (!IPMC_LIB_FLTR_POOL_PF_VALID(profile_index)) {
        return NULL;
    }

    if ((profile = IPMC_LIB_FLTR_POOL_PROFILE(profile_index)) == NULL) {
        return NULL;
    }

    rule_idx = fltr_rule->idx;
    if (rule_idx < IPMC_LIB_FLTR_ENTRY_MAX_CNT) {
        rule = &profile->rule[rule_idx];
    } else {
        return NULL;
    }

    if (!rule->valid) {
        return NULL;
    }

    rule_idx = rule->next_rule_idx;
    if (IPMC_LIB_PF_RULE_ID_VALID(rule_idx)) {
        rule = &profile->rule[rule_idx];
        if (!rule->valid) {
            return NULL;
        }

        return rule;
    } else {
        return NULL;
    }
}

/* Validate the group address by using the designated profile */
ipmc_action_t ipmc_lib_profile_match(u32 pdx, u8 port, vtss_vid_t *vid, vtss_ipv6_t *grp, vtss_ipv6_t *src)
{
    ipmc_db_ctrl_hdr_t  *pf_tree;
    ipmc_profile_rule_t *ptr, r_buf;
    ipmc_lib_rule_t     *rule;
    ipmc_action_t       action;
    ipmc_ip_version_t   match_version;

    if (!ipmc_lib_profile->global_ctrl_state) {
        return IPMC_ACTION_PERMIT;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, pdx);
    if (!pf_tree || !pf_tree->mflag) {
        return IPMC_ACTION_DENY;
    }

    if (!grp || !src || !vid) {
        return IPMC_ACTION_DENY;
    }

    ptr = &r_buf;
    memset(ptr, 0x0, sizeof(ipmc_profile_rule_t));
    if ((match_version = ipmc_lib_grp_adrs_version(grp)) == IPMC_IP_VERSION_ERR) {
        return IPMC_ACTION_DENY;
    }
    ptr->version = match_version;
    IPMC_LIB_ADRS_CPY(&ptr->grp_adr, grp);

    /* default is deny */
    action = IPMC_ACTION_DENY;
    if (IPMC_LIB_DB_GET_NEXT(pf_tree, ptr) &&
        ((rule = ptr->rule) != NULL)) {
        vtss_vid_t  pf_vid;

        action = rule->action;
        pf_vid = IPMC_LIB_PFT_VID_GET(ipmc_profile_tree, pdx);
        if (pf_vid) {
            if (*vid == VTSS_IPMC_VID_NULL) {
                *vid = pf_vid;
            } else {
                if (*vid != pf_vid) {
                    action = IPMC_ACTION_DENY;
                }
            }
        }

        if (rule->log) {
            ipmc_lib_log_t  pf_log;

            memset(&pf_log, 0x0, sizeof(ipmc_lib_log_t));
            pf_log.vid = *vid;
            pf_log.port = port;
            pf_log.version = match_version;
            pf_log.dst = grp;
            pf_log.src = src;
            pf_log.event.profile.action = action;
            pf_log.event.profile.name = IPMC_LIB_FLTR_POOL_PF_NAME(pdx);
            pf_log.event.profile.entry = IPMC_LIB_FLTR_POOL_EN_NAME(rule->entry_index);
            IPMC_LIB_LOG_PROFILE(&pf_log, IPMC_SEVERITY_Normal);
        }
    }

    return action;
}

BOOL ipmc_lib_profile_tree_vid_set(u32 tdx, vtss_vid_t pf_vid)
{
    ipmc_db_ctrl_hdr_t  *pf_tree;

    if (pf_vid > VTSS_IPMC_VID_MAX) {
        return FALSE;
    }

    if (tdx > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return FALSE;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, tdx);
    if (!pf_tree || !pf_tree->mflag) {
        return FALSE;
    }

    IPMC_LIB_PFT_VID_SET(ipmc_profile_tree, tdx, pf_vid);

    return TRUE;
}

BOOL ipmc_lib_profile_tree_vid_get(u32 tdx, vtss_vid_t *pf_vid)
{
    ipmc_db_ctrl_hdr_t  *pf_tree;

    if (!pf_vid) {
        return FALSE;
    }

    if (tdx > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return FALSE;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, tdx);
    if (!pf_tree || !pf_tree->mflag) {
        return FALSE;
    }

    *pf_vid = IPMC_LIB_PFT_VID_GET(ipmc_profile_tree, tdx);

    return TRUE;
}

ipmc_profile_rule_t *ipmc_lib_profile_tree_get(u32 tdx, ipmc_profile_rule_t *entry, BOOL *is_avl)
{
    ipmc_db_ctrl_hdr_t  *pf_tree;
    ipmc_profile_rule_t *ptr;

    if (!entry) {
        return NULL;
    }

    if (tdx > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return NULL;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, tdx);
    if (!pf_tree || !pf_tree->mflag) {
        return NULL;
    } else {
        *is_avl = TRUE;
    }

    ptr = entry;
    if (IPMC_LIB_DB_GET(pf_tree, ptr)) {
        return ptr;
    }

    return NULL;
}

ipmc_profile_rule_t *ipmc_lib_profile_tree_get_next(u32 tdx, ipmc_profile_rule_t *entry, BOOL *is_avl)
{
    ipmc_db_ctrl_hdr_t  *pf_tree;
    ipmc_profile_rule_t *ptr;

    if (tdx > IPMC_LIB_FLTR_PROFILE_MAX_CNT) {
        return NULL;
    }

    pf_tree = IPMC_LIB_PFT_HDR_ADDR(ipmc_profile_tree, tdx);
    if (!pf_tree || !pf_tree->mflag) {
        return NULL;
    } else {
        *is_avl = TRUE;
    }

    if (!entry) {
        ptr = NULL;
        if (!IPMC_LIB_DB_GET_FIRST(pf_tree, ptr)) {
            ptr = NULL;
        }
    } else {
        ptr = entry;
        if (!IPMC_LIB_DB_GET_NEXT(pf_tree, ptr)) {
            ptr = NULL;
        }
    }

    return ptr;
}

static void _ipmc_profile_default_value_set(void)
{
    u32 idx;

    memset(&ipmc_lib_def_fltr_rule4, 0xFF, sizeof(ipmc_lib_rule_t));
    memset(&ipmc_lib_def_fltr_rule6, 0xFF, sizeof(ipmc_lib_rule_t));

    ipmc_lib_def_fltr_rule4.action = IPMC_ACTION_DENY;
    ipmc_lib_def_fltr_rule4.idx = IPMC_LIB_FLTR_RULE_IDX_DFLT;
    ipmc_lib_def_fltr_rule4.entry_index = IPMC_LIB_FLTR_RULE_IDX_DFLT;
    ipmc_lib_def_fltr_rule4.log = FALSE;
    ipmc_lib_def_fltr_rule4.valid = TRUE;

    ipmc_lib_def_fltr_rule6.action = IPMC_ACTION_DENY;
    ipmc_lib_def_fltr_rule6.idx = IPMC_LIB_FLTR_RULE_IDX_DFLT;
    ipmc_lib_def_fltr_rule6.entry_index = IPMC_LIB_FLTR_RULE_IDX_DFLT;
    ipmc_lib_def_fltr_rule6.log = FALSE;
    ipmc_lib_def_fltr_rule6.valid = TRUE;

    memset(ipmc_lib_def_profile_rule4, 0x0, sizeof(ipmc_lib_def_profile_rule4));
    memset(ipmc_lib_def_profile_rule6, 0x0, sizeof(ipmc_lib_def_profile_rule6));

    for (idx = 0; idx < IPMC_LIB_FLTR_PROFILE_MAX_CNT; idx++) {
        ipmc_lib_def_profile_rule4[idx].version = IPMC_IP_VERSION_IGMP;
        ipmc_lib_def_profile_rule4[idx].grp_adr.addr[12] = 0xEF;
        ipmc_lib_def_profile_rule4[idx].grp_adr.addr[13] = 0xFF;
        ipmc_lib_def_profile_rule4[idx].grp_adr.addr[14] = 0xFF;
        ipmc_lib_def_profile_rule4[idx].grp_adr.addr[15] = 0xFF;
        ipmc_lib_def_profile_rule4[idx].vir_idx = 0xFF;
        ipmc_lib_def_profile_rule4[idx].rule = &ipmc_lib_def_fltr_rule4;
        ipmc_lib_def_profile_rule4[idx].mflag = TRUE;

        ipmc_lib_def_profile_rule6[idx].version = IPMC_IP_VERSION_MLD;
        IPMC_LIB_ADRS_SET(&ipmc_lib_def_profile_rule6[idx].grp_adr, 0xFF);
        ipmc_lib_def_profile_rule6[idx].vir_idx = 0xFF;
        ipmc_lib_def_profile_rule6[idx].rule = &ipmc_lib_def_fltr_rule6;
        ipmc_lib_def_profile_rule6[idx].mflag = TRUE;
    }
}

/* Set the configuration pointer of profile */
BOOL ipmc_lib_profile_conf_ptr_set(void *ptr)
{
    IPMC_PROFILE_ENTER();
    ipmc_lib_profile = (ipmc_lib_conf_profile_t *)ptr;
    IPMC_PROFILE_EXIT();

    return TRUE;
}

vtss_rc ipmc_lib_profile_init(void)
{
    if (ipmc_profile_done_init) {
        return VTSS_OK;
    }

    critd_init(&ipmc_profile_crit, "ipmc_profile_crit", VTSS_MODULE_ID_IPMC_LIB, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    IPMC_PROFILE_EXIT();

    IPMC_PROFILE_ENTER();
    ipmc_lib_profile = NULL;

    memset(ipmc_profile_tree, 0x0, sizeof(ipmc_profile_tree));
    _ipmc_profile_default_value_set();
    IPMC_PROFILE_EXIT();

    ipmc_profile_done_init = TRUE;
    return VTSS_OK;
}
