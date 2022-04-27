/*

 Vitesse Switch API software.

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

#ifndef _IPMC_LIB_PROFILE_H_
#define _IPMC_LIB_PROFILE_H_

#define IPMC_LIB_FLTR_ENTRY_MAX_CNT     128
#define IPMC_LIB_FLTR_PROFILE_MAX_CNT   64

#define IPMC_LIB_FLTR_PROFILE_DEF_STATE VTSS_IPMC_FALSE
#define IPMC_LIB_FLTR_RULE_DEF_LOG      VTSS_IPMC_FALSE
#define IPMC_LIB_FLTR_RULE_DEF_ACTION   IPMC_ACTION_DENY

#define IPMC_LIB_FLTR_PROFILE_IDX_NONE  0x0
#define IPMC_LIB_FLTR_RULE_IDX_INIT     0xFFFFFFFF
#define IPMC_LIB_FLTR_RULE_IDX_DFLT     0xFFFFFFFE
#define IPMC_LIB_PF_RULE_ID_INIT(x)     ((x) == IPMC_LIB_FLTR_RULE_IDX_INIT)
#define IPMC_LIB_PF_RULE_ID_DFLT(x)     ((x) == IPMC_LIB_FLTR_RULE_IDX_DFLT)
#define IPMC_LIB_PF_RULE_ID_VALID(x)    (((x) != IPMC_LIB_FLTR_RULE_IDX_INIT) && ((x) != IPMC_LIB_FLTR_RULE_IDX_DFLT) && ((x) < IPMC_LIB_FLTR_ENTRY_MAX_CNT))

#define IPMC_SNP_ASM_FLTR_MAX_CNT       1
#define IPMC_MVR_ASM_FLTR_MAX_CNT       1
#define IPMC_LIB_ASM_FLTR_MAX_CNT       1
#if (IPMC_LIB_ASM_FLTR_MAX_CNT > 1)
#define IPMC_LIB_FLTR_MULTIPLE_PROFILE  1
#else
#define IPMC_LIB_FLTR_MULTIPLE_PROFILE  0
#endif /* (IPMC_LIB_ASM_FLTR_MAX_CNT > 1) */

#define IPMC_LIB_RULE_CNT_PER_PFT       (2 * IPMC_LIB_FLTR_ENTRY_MAX_CNT + 2)
#define IPMC_LIB_SUPPORTED_RULES        (IPMC_LIB_RULE_CNT_PER_PFT * IPMC_LIB_FLTR_PROFILE_MAX_CNT)
#define IPMC_LIB_FLTR_PROFILE_MAX_CALC  24  /* CLI sessions + WEB + XML + MSG + RESERVED */
#define IPMC_LIB_SUPPORTED_PROFILES     (IPMC_LIB_FLTR_PROFILE_MAX_CNT + IPMC_LIB_FLTR_PROFILE_MAX_CALC)

#define IPMC_LIB_PFT_HDR_ADDR(x, y)     ((y) ? (&(((x)[(y) - 1]).hdr)) : NULL)
#define IPMC_LIB_PFT_DEF_RULE(x, y)     ((y) ? (&(((x)[(y) - 1]))) : NULL)
#define IPMC_LIB_PFT_VID_GET(x, y)      ((y) ? (((x)[(y) - 1]).vid) : VTSS_IPMC_VID_VOID)
#define IPMC_LIB_PFT_VID_SET(x, y, z)   (((x)[(y) - 1]).vid = (z))

#define IPMC_LIB_PERMIT(v, w, x, y, z)  (ipmc_lib_profile_match((v), (w), (x), (y), (z)) == IPMC_ACTION_PERMIT)
#define IPMC_LIB_DENY(v, w, x, y, z)    (ipmc_lib_profile_match((v), (w), (x), (y), (z)) != IPMC_ACTION_PERMIT)

#define IPMC_LIB_FLTR_POOL_ENTRY(x)     ((!(x) || (x) > IPMC_LIB_FLTR_ENTRY_MAX_CNT) ? NULL : (&(ipmc_lib_profile->fltr_entry_pool[(x) - 1])))
#define IPMC_LIB_FLTR_POOL_PROFILE(x)   ((!(x) || (x) > IPMC_LIB_FLTR_PROFILE_MAX_CNT) ? NULL : (&(ipmc_lib_profile->fltr_profile_pool[(x) - 1])))
#define IPMC_LIB_FLTR_POOL_PF_VALID(x)  ((!(x) || (x) > IPMC_LIB_FLTR_PROFILE_MAX_CNT) ? FALSE : ((ipmc_lib_profile->fltr_profile_pool[(x) - 1]).data.valid))
#define IPMC_LIB_FLTR_POOL_EN_VALID(x)  ((!(x) || (x) > IPMC_LIB_FLTR_ENTRY_MAX_CNT) ? FALSE : ((ipmc_lib_profile->fltr_entry_pool[(x) - 1]).valid))
#define IPMC_LIB_FLTR_POOL_PF_NAME(x)   (((x) && (x) <= IPMC_LIB_FLTR_PROFILE_MAX_CNT) ? ((ipmc_lib_profile->fltr_profile_pool[(x) - 1]).data.name) : NULL)
#define IPMC_LIB_FLTR_POOL_EN_NAME(x)   (((x) && (x) <= IPMC_LIB_FLTR_ENTRY_MAX_CNT) ? ((ipmc_lib_profile->fltr_entry_pool[(x) - 1]).name) : NULL)


typedef struct {
    BOOL                valid;
    u32                 index;
    i8                  name[VTSS_IPMC_NAME_MAX_LEN];
    ipmc_ip_version_t   version;
    vtss_ipv6_t         grp_bgn;
    vtss_ipv6_t         grp_end;
} ipmc_lib_grp_fltr_entry_t;

typedef struct {
    BOOL                valid;
    u32                 idx;
    u32                 entry_index;
    u32                 next_rule_idx;
    ipmc_action_t       action;
    BOOL                log;
} ipmc_lib_rule_t;

typedef struct {
    BOOL                valid;
    u32                 index;
    i8                  name[VTSS_IPMC_NAME_MAX_LEN];
    i8                  desc[VTSS_IPMC_DESC_MAX_LEN];
    ipmc_ip_version_t   version;
    u32                 rule_head_idx;
} ipmc_lib_profile_t;

typedef struct {
    ipmc_lib_profile_t  data;
    ipmc_lib_rule_t     rule[IPMC_LIB_FLTR_ENTRY_MAX_CNT];
} ipmc_lib_grp_fltr_profile_t;

typedef struct ipmc_lib_profile_mem_s {
    ipmc_lib_grp_fltr_profile_t     profile;

    struct ipmc_lib_profile_mem_s   *next;
    BOOL                            mflag;
} ipmc_lib_profile_mem_t;

typedef struct ipmc_profile_rule_s {
    /* INDEX */
    ipmc_ip_version_t               version;
    vtss_ipv6_t                     grp_adr;
    u8                              vir_idx;

    ipmc_lib_rule_t                 *rule;

    struct ipmc_profile_rule_s      *next;
    BOOL                            mflag;
} ipmc_profile_rule_t;

typedef struct ipmc_profile_tree_s {
    ipmc_db_ctrl_hdr_t              hdr;
    vtss_vid_t                      vid;
} ipmc_profile_tree_t;

typedef ipmc_lib_grp_fltr_entry_t   ipmc_lib_conf_fltr_entry_t;
typedef ipmc_lib_grp_fltr_profile_t ipmc_lib_conf_fltr_profile_t;

typedef struct {
    BOOL                            global_ctrl_state;
    ipmc_lib_conf_fltr_entry_t      fltr_entry_pool[IPMC_LIB_FLTR_ENTRY_MAX_CNT];
    ipmc_lib_conf_fltr_profile_t    fltr_profile_pool[IPMC_LIB_FLTR_PROFILE_MAX_CNT];
} ipmc_lib_conf_profile_t;

typedef struct {
    u32                                 pdx[IPMC_LIB_ASM_FLTR_MAX_CNT];
    u8                                  port;
    vtss_vid_t                          vid;
    vtss_ipv6_t                         dst;
    vtss_ipv6_t                         src;

    u32                                 filter_cnt;
} specific_grps_fltr_t;


/* Set Global Filtering Profile State */
vtss_rc ipmc_lib_profile_state_set(BOOL profiling);

/* Get Global Filtering Profile State */
vtss_rc ipmc_lib_profile_state_get(BOOL *profiling);

/* Add/Delete/Update IPMC Profile Entry */
vtss_rc ipmc_lib_fltr_entry_set(ipmc_operation_action_t action, ipmc_lib_grp_fltr_entry_t *fltr_entry);

/* Get IPMC Profile Entry */
ipmc_lib_grp_fltr_entry_t *ipmc_lib_fltr_entry_get(ipmc_lib_grp_fltr_entry_t *fltr_entry, BOOL by_name);

/* GetNext IPMC Profile Entry */
ipmc_lib_grp_fltr_entry_t *ipmc_lib_fltr_entry_get_next(ipmc_lib_grp_fltr_entry_t *fltr_entry, BOOL by_name);

/* Add/Delete/Update IPMC Profile */
vtss_rc ipmc_lib_fltr_profile_set(ipmc_operation_action_t action, ipmc_lib_grp_fltr_profile_t *fltr_profile);

/* Get IPMC Profile */
ipmc_lib_grp_fltr_profile_t *ipmc_lib_fltr_profile_get(ipmc_lib_grp_fltr_profile_t *fltr_profile, BOOL by_name);

/* GetNext IPMC Profile */
ipmc_lib_grp_fltr_profile_t *ipmc_lib_fltr_profile_get_next(ipmc_lib_grp_fltr_profile_t *fltr_profile, BOOL by_name);

/* Add/Delete/Update IPMC Profile Rule */
vtss_rc ipmc_lib_fltr_profile_rule_set(ipmc_operation_action_t action, u32 profile_index, ipmc_lib_rule_t *fltr_rule);

/* Search IPMC Profile Rule by Entry Index */
ipmc_lib_rule_t *ipmc_lib_fltr_profile_rule_search(u32 profile_index, u32 entry_index);

/* Get IPMC Profile Rule */
ipmc_lib_rule_t *ipmc_lib_fltr_profile_rule_get(u32 profile_index, ipmc_lib_rule_t *fltr_rule);

/* GetFirst IPMC Profile Rule */
ipmc_lib_rule_t *ipmc_lib_fltr_profile_rule_get_first(u32 profile_index);

/* GetNext IPMC Profile Rule */
ipmc_lib_rule_t *ipmc_lib_fltr_profile_rule_get_next(u32 profile_index, ipmc_lib_rule_t *fltr_rule);

/* Set the configuration pointer of profile */
BOOL ipmc_lib_profile_conf_ptr_set(void *ptr);

/* Validate the group address by using the designated profile */
ipmc_action_t ipmc_lib_profile_match(u32 index, u8 port, vtss_vid_t *vid, vtss_ipv6_t *grp, vtss_ipv6_t *src);

/* VID Get/Set functions for internal profile tree */
BOOL ipmc_lib_profile_tree_vid_set(u32 tdx, vtss_vid_t pf_vid);
BOOL ipmc_lib_profile_tree_vid_get(u32 tdx, vtss_vid_t *pf_vid);

/* Get functions for internal profile database */
ipmc_profile_rule_t *ipmc_lib_profile_tree_get(u32 tdx, ipmc_profile_rule_t *entry, BOOL *is_avl);
ipmc_profile_rule_t *ipmc_lib_profile_tree_get_next(u32 tdx, ipmc_profile_rule_t *entry, BOOL *is_avl);

#endif /* _IPMC_LIB_PROFILE_H_ */
