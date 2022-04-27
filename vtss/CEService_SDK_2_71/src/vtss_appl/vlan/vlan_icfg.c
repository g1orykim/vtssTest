/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "icfg_api.h"
#include "vlan_api.h"
#include "misc_api.h"   /* For str_tolower() */
#include "port_api.h"
#include "vlan_icfg.h"
#include "vlan_trace.h" /* For T_xxx() */

#undef  VTSS_ALLOC_MODULE_ID
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_VLAN

/******************************************************************************/
// VLAN_ICFG_global_conf()
/******************************************************************************/
static vtss_rc VLAN_ICFG_global_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    vtss_etype_t tpid;
    int          conf_changed = 0;

    VTSS_RC(vlan_mgmt_s_custom_etype_get(&tpid));

    // If conf has changed from default
    conf_changed = tpid != VLAN_CUSTOM_S_TAG_DEFAULT;
    if (req->all_defaults || conf_changed) {
        VTSS_RC(vtss_icfg_printf(result, "vlan ethertype s-custom-port 0x%x\n", tpid));
    }
#endif

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICFG_tag_type_to_txt()
/******************************************************************************/
static char *VLAN_ICFG_tag_type_to_txt(vlan_tx_tag_type_t tx_tag_type)
{
    switch (tx_tag_type) {
    case VLAN_TX_TAG_TYPE_UNTAG_THIS:
        return "all except-native";

    case VLAN_TX_TAG_TYPE_TAG_ALL:
        return "all";

    case VLAN_TX_TAG_TYPE_UNTAG_ALL:
        return "none";

    default:
        T_E("Que? (%d)", tx_tag_type);
        return "";
    }
}

/******************************************************************************/
// VLAN_ICFG_port_mode_to_str[]
/******************************************************************************/
static const char *const VLAN_ICFG_port_mode_to_str[3] = {
    [VLAN_PORT_MODE_ACCESS] = "access",
    [VLAN_PORT_MODE_TRUNK]  = "trunk",
    [VLAN_PORT_MODE_HYBRID] = "hybrid"
};

/******************************************************************************/
// VLAN_ICFG_port_mode_native_to_str[]
/******************************************************************************/
static const char *const VLAN_ICFG_port_mode_native_to_str[3] = {
    [VLAN_PORT_MODE_ACCESS] = "",
    [VLAN_PORT_MODE_TRUNK]  = " native",
    [VLAN_PORT_MODE_HYBRID] = " native"
};

/******************************************************************************/
// VLAN_ICFG_port_mode_pvid_print()
/******************************************************************************/
static vtss_rc VLAN_ICFG_port_mode_pvid_print(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result, vlan_port_mode_t port_mode, vlan_port_composite_conf_t *conf)
{
    vtss_vid_t pvid;

    switch (port_mode) {
    case VLAN_PORT_MODE_ACCESS:
        pvid = conf->access_vid;
        break;

    case VLAN_PORT_MODE_TRUNK:
        pvid = conf->native_vid;
        break;

    default:
        // Hybrid:
        pvid = conf->hyb_port_conf.pvid;
        break;
    }

    if (req->all_defaults || pvid != VLAN_ID_DEFAULT) {
        VTSS_RC(vtss_icfg_printf(result, " switchport %s%s vlan %u\n", VLAN_ICFG_port_mode_to_str[port_mode], VLAN_ICFG_port_mode_native_to_str[port_mode], pvid));
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICFG_port_mode_allowed_vids_print_cmd()
/******************************************************************************/
static vtss_rc VTSS_ICFG_port_mode_allowed_vlan_print(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result, vlan_port_mode_t port_mode, i8 *keyword, u8 *vid_bitmask)
{
    vtss_rc rc;
    i8      *buf = NULL;

    if (keyword == NULL) {
        // Caller doesn't have a special keyword to print, so print the #vid_bitmask.
        if ((buf = VTSS_MALLOC(VLAN_VID_LIST_AS_STRING_LEN_BYTES)) == NULL) {
            T_E("Out of memory");
            return VTSS_RC_ERROR;
        }

        (void)vlan_mgmt_vid_bitmask_to_txt(vid_bitmask, buf);
    }

    rc = vtss_icfg_printf(result, " switchport %s allowed vlan %s\n", VLAN_ICFG_port_mode_to_str[port_mode], keyword ? keyword : buf);

    if (buf) {
        VTSS_FREE(buf);
    }

    return rc;
}

/******************************************************************************/
// VLAN_ICFG_port_mode_allowed_vids()
/******************************************************************************/
static vtss_rc VLAN_ICFG_port_mode_allowed_vids(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result, vlan_port_mode_t port_mode)
{
    u8   vid_bitmask[VLAN_BITMASK_LEN_BYTES], vid_bitmask_all[VLAN_BITMASK_LEN_BYTES];
    BOOL is_all_zeros, is_all_ones;

    VTSS_RC(vlan_mgmt_port_composite_allowed_vids_get(req->instance_id.port.isid, req->instance_id.port.begin_iport, port_mode, vid_bitmask));

    // In the following, we have to use the vlan_mgmt_bitmasks_identical()
    // utility function to get to know whether tw VLAN bitmasks are
    // identical or not, because not all bits in the array may be in use.

    if (req->all_defaults) {
        // Here, we always print the whole range, whether it's all VLANs or just partial set.
        // There is one exception, which is when it's empty, in which case we print "none"
        memset(vid_bitmask_all, 0x0, sizeof(vid_bitmask_all));
        is_all_zeros = vlan_mgmt_bitmasks_identical(vid_bitmask, vid_bitmask_all);

        VTSS_RC(VTSS_ICFG_port_mode_allowed_vlan_print(req, result, port_mode, is_all_zeros ? "none" : NULL, vid_bitmask));
    } else {
        u8 def_bitmask[VLAN_BITMASK_LEN_BYTES];

        (void)vlan_mgmt_port_composite_allowed_vids_default_get(port_mode, def_bitmask);

        if (!vlan_mgmt_bitmasks_identical(vid_bitmask, def_bitmask)) {
            // The current bitmask is not default. Gotta print a string.
            memset(vid_bitmask_all, 0x0, sizeof(vid_bitmask_all));
            is_all_zeros = vlan_mgmt_bitmasks_identical(vid_bitmask, vid_bitmask_all);

            if (is_all_zeros) {
                // Cannot be all-zeros and all-ones at the same time :)
                is_all_ones = FALSE;
            } else {
                memset(vid_bitmask_all, 0xFF, sizeof(vid_bitmask_all));
                is_all_ones = vlan_mgmt_bitmasks_identical(vid_bitmask, vid_bitmask_all);
            }

            VTSS_RC(VTSS_ICFG_port_mode_allowed_vlan_print(req, result, port_mode, is_all_zeros ? "none" : is_all_ones ? "all" : NULL, vid_bitmask));
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICFG_port_conf()
/******************************************************************************/
static vtss_rc VLAN_ICFG_port_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    vtss_vid_t                 vid;
    vtss_isid_t                isid = req->instance_id.port.isid;
    vtss_port_no_t             iport = req->instance_id.port.begin_iport;
    vlan_port_composite_conf_t composite_conf_default, composite_conf;
    vlan_port_mode_t           port_mode;
    vlan_mgmt_entry_t          forbidden_conf;
    u8                         forbidden_vid_mask[VLAN_BITMASK_LEN_BYTES];
    BOOL                       found, conf_changed;
    char                       str[32];

    str[sizeof(str) - 1] = '\0';

    if (port_isid_port_no_is_stack(isid, iport)) {
        return VTSS_RC_OK;
    }

    (void)vlan_mgmt_port_composite_conf_default_get(&composite_conf_default);
    VTSS_RC(vlan_mgmt_port_composite_conf_get(isid, iport, &composite_conf));

    // Access, Trunk, and Hybrid PVIDs (access_vid/native_vid/hyb_port_conf.pvid)
    for (port_mode = VLAN_PORT_MODE_ACCESS; port_mode <= VLAN_PORT_MODE_HYBRID; port_mode++) {
        VTSS_RC(VLAN_ICFG_port_mode_pvid_print(req, result, port_mode, &composite_conf));
    }

    // Trunk and Hybrid: Allowed VLANs
    for (port_mode = VLAN_PORT_MODE_TRUNK; port_mode <= VLAN_PORT_MODE_HYBRID; port_mode++) {
        VTSS_RC(VLAN_ICFG_port_mode_allowed_vids(req, result, port_mode));
    }

    // Trunk native VLAN tagging
    conf_changed = composite_conf.tag_native_vlan != composite_conf_default.tag_native_vlan;
    if (req->all_defaults || conf_changed) {
        VTSS_RC(vtss_icfg_printf(result, " %sswitchport trunk vlan tag native\n", composite_conf.tag_native_vlan ? "" : "no "));
    }

    // Hybrid port mode details
    // Acceptable frame type (accept all, accept tagged only, accept untagged only).
    conf_changed = composite_conf.hyb_port_conf.frame_type != composite_conf_default.hyb_port_conf.frame_type;
    if (req->all_defaults || conf_changed) {
        strncpy(str, vlan_mgmt_frame_type_to_txt(composite_conf.hyb_port_conf.frame_type), sizeof(str) - 1);
        VTSS_RC(vtss_icfg_printf(result, " switchport hybrid acceptable-frame-type %s\n", str_tolower(str)));
    }

    // Ingress filtering
    conf_changed = composite_conf.hyb_port_conf.ingress_filter != composite_conf_default.hyb_port_conf.ingress_filter;
    if (req->all_defaults || conf_changed) {
        VTSS_RC(vtss_icfg_printf(result, " %sswitchport hybrid ingress-filtering\n", composite_conf.hyb_port_conf.ingress_filter ? "" : "no "));
    }

    // Tx Tag type (untag all ("none"), untag this ("all except-native"), or tag all ("all");
    conf_changed = composite_conf.hyb_port_conf.tx_tag_type != composite_conf_default.hyb_port_conf.tx_tag_type;
    if (req->all_defaults || conf_changed) {
        VTSS_RC(vtss_icfg_printf(result, " switchport hybrid egress-tag %s\n", VLAN_ICFG_tag_type_to_txt(composite_conf.hyb_port_conf.tx_tag_type)));
    }

    // Port Type (Unaware, C-tag-aware, S-tag aware, Custom-S-tag aware)
    conf_changed = composite_conf.hyb_port_conf.port_type != composite_conf_default.hyb_port_conf.port_type;
    if (req->all_defaults || conf_changed) {
        // vlan_mgmt_port_type_to_txt() returns a pointer to a fixed string, so we should not lowercase that copy.
        strncpy(str, vlan_mgmt_port_type_to_txt(composite_conf.hyb_port_conf.port_type), sizeof(str) - 1);
        VTSS_RC(vtss_icfg_printf(result, " switchport hybrid port-type %s\n", str_tolower(str)));
    }

    // Port Mode
    conf_changed = composite_conf.mode != composite_conf_default.mode;
    if (req->all_defaults || conf_changed) {
        VTSS_RC(vtss_icfg_printf(result, " switchport mode %s\n", VLAN_ICFG_port_mode_to_str[composite_conf.mode]));
    }

    // Forbidden VLANs
    vid   = VTSS_VID_NULL;
    found = FALSE;
    memset(forbidden_vid_mask, 0, sizeof(forbidden_vid_mask));

    while (vlan_mgmt_vlan_get(isid, vid, &forbidden_conf, TRUE, VLAN_USER_FORBIDDEN) == VTSS_RC_OK) {
        vid = forbidden_conf.vid; // Select next entry
        if (forbidden_conf.ports[iport]) {
            found = TRUE;
            VTSS_BF_SET(forbidden_vid_mask, vid, 1);
        }
    }

    if (found) {
        i8      *buf;
        vtss_rc rc;

        // Gotta convert the forbidden_vid_mask to a string. This string
        // can potentially be quite long, so we have to dynamically allocate
        // some memory for it.
        if ((buf = VTSS_MALLOC(VLAN_VID_LIST_AS_STRING_LEN_BYTES)) == NULL) {
            T_E("Out of memory");
            return VTSS_RC_ERROR;
        }

        // Do convert to string
        (void)vlan_mgmt_vid_bitmask_to_txt(forbidden_vid_mask, buf);

        rc = vtss_icfg_printf(result, " switchport forbidden vlan add %s\n", buf);

        // And free it again
        VTSS_FREE(buf);
        if (rc != VTSS_RC_OK) {
            return rc;
        }

    } else if (req->all_defaults) {
        // No forbidden VLANs found on this interface. This is the default.
        // Only print a command if requested to (all_defaults)
        // And do not print the no-form. Use remove <all-vlans> instead.
        VTSS_RC(vtss_icfg_printf(result, " switchport forbidden vlan remove %u-%u\n", VLAN_ID_MIN, VLAN_ID_MAX));
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICFG_name_conf()
/******************************************************************************/
static vtss_rc VLAN_ICFG_name_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    BOOL    is_default;
    vtss_rc rc;

    char vlan_name[VLAN_NAME_MAX_LEN];

    if ((rc = vlan_mgmt_name_get(req->instance_id.vlan, vlan_name, &is_default)) != VTSS_RC_OK) {
        // All VLANs have a name, so what's going on?
        T_E("Huh? %s", error_txt(rc));
        return VTSS_RC_ERROR;
    }

    if (req->all_defaults || !is_default) {
        VTSS_RC(vtss_icfg_printf(result, " name %s\n", vlan_name));
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_icfg_init()
/******************************************************************************/
vtss_rc VLAN_icfg_init(void)
{
    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_VLAN_GLOBAL_CONF, "vlan", VLAN_ICFG_global_conf));
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_VLAN_PORT_CONF,   "vlan", VLAN_ICFG_port_conf));
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_VLAN_NAME_CONF,   "vlan", VLAN_ICFG_name_conf));
    return VTSS_RC_OK;
}

