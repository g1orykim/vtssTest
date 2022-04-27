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
#include "icli_api.h"
#include "icli_porting_util.h"
#include "vlan_api.h"
#include "msg.h"
#include "vlan_trace.h"
#include "vlan_icli_functions.h" /* Just to check syntax of .h file against the function signatures of the implementation */

/****************************************************************************/
/*  Functions called by ICLI                                                */
/****************************************************************************/

const char *VLAN_ICLI_tx_tag_type_to_txt(vlan_tx_tag_type_t tx_tag_type)
{
    if (tx_tag_type == VLAN_TX_TAG_TYPE_UNTAG_THIS) {
        return "All except-native";
    } else if (tx_tag_type == VLAN_TX_TAG_TYPE_UNTAG_ALL) {
        return "None";
    } else if (tx_tag_type == VLAN_TX_TAG_TYPE_TAG_ALL) {
        return "All";
    }
    return "";
}

/******************************************************************************/
// VLAN_ICLI_show_status()
/******************************************************************************/
vtss_rc VLAN_ICLI_show_status(i32 session_id, BOOL has_interface, icli_stack_port_range_t *plist, BOOL has_combined, BOOL has_admin, BOOL has_nas, BOOL has_mvr, BOOL has_voice_vlan, BOOL has_mstp, BOOL has_vcl, BOOL has_erps, BOOL has_all, BOOL has_conflicts, BOOL has_evc, BOOL has_gvrp)
{
    switch_iter_t         sit;
    port_iter_t           pit;
    vlan_user_t           usr;
    vlan_port_conflicts_t conflicts;
    u32                   all_usrs = 0;
    vlan_port_flags_idx_t temp;
    char                  buf[200];
    i8                    interface_str[ICLI_PORTING_STR_BUF_SIZE];
    vlan_user_t           last_user  = VLAN_USER_ALL;
    vlan_user_t           first_user = VLAN_USER_STATIC;
    BOOL                  print_hdr  = TRUE;

    if (has_combined) {
        first_user = VLAN_USER_ALL;
    } else  if (has_admin) {
        first_user = VLAN_USER_STATIC;
#ifdef VTSS_SW_OPTION_DOT1X
    } else if (has_nas) {
        first_user = VLAN_USER_DOT1X;
#endif
#ifdef VTSS_SW_OPTION_MVR
    } else if (has_mvr) {
        first_user = VLAN_USER_MVR;
#endif
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    } else if (has_voice_vlan) {
        first_user = VLAN_USER_VOICE_VLAN;
#endif
#ifdef VTSS_SW_OPTION_MSTP
    } else if (has_mstp) {
        first_user = VLAN_USER_MSTP;
#endif
#ifdef VTSS_SW_OPTION_ERPS
    } else if (has_erps) {
        first_user = VLAN_USER_MSTP;
#endif
#ifdef VTSS_SW_OPTION_VCL
    } else if (has_vcl) {
        first_user = VLAN_USER_VCL;
#endif
#if defined(VTSS_SW_OPTION_EVC)
    } else if (has_evc) {
        first_user = VLAN_USER_EVC;
#endif
#if defined(VTSS_SW_OPTION_GVRP)
    } else if (has_gvrp) {
        first_user = VLAN_USER_GVRP;
#endif
    } else if (has_conflicts) {
        first_user = VLAN_USER_ALL;
    } else {
        has_all = TRUE; // Default to show all if none of the others are selected
    }

    T_IG(VTSS_TRACE_GRP_CLI, "first_user:%d, last_user:%d, has_all:%d", first_user, last_user, has_all);

    // Loop through all switches in stack
    VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID));
    while (icli_switch_iter_getnext(&sit, plist)) {
        VTSS_RC(port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL));
        while (icli_port_iter_getnext(&pit, plist)) {
            vlan_port_conf_t      conf;

            if (!has_all) {
                last_user = first_user;
            }

            VTSS_RC(vlan_mgmt_conflicts_get(sit.isid, pit.iport, &conflicts));

            // Collect all conflicting VLAN users.
            for (temp = 0; temp < VLAN_PORT_FLAGS_IDX_CNT; temp++) {
                all_usrs |= conflicts.users[temp];
            }

            (void)icli_port_info_txt(sit.usid, pit.uport, interface_str);
            strncat(&interface_str[0], " :",  sizeof(interface_str));
            icli_table_header(session_id, &interface_str[0]);

            print_hdr = TRUE;

            T_IG(VTSS_TRACE_GRP_CLI, "first_user:%d, last_user:%d, has_all:%d", first_user, last_user, has_all);
            for (usr = first_user; usr <= last_user; usr++) {
                // Skip calling for those VLAN users that never override port configuration
                if (usr != VLAN_USER_ALL && !vlan_mgmt_user_is_port_conf_changer(usr)) {
                    continue;
                }

                if (vlan_mgmt_port_conf_get(sit.isid, pit.iport, &conf, usr) != VTSS_RC_OK) {
                    T_IG_PORT(VTSS_TRACE_GRP_CLI, pit.iport, "Skipping usr:%d, sid:%d", usr, sit.isid);
                    continue;
                }

                if (print_hdr) {
                    (void)snprintf(&buf[0], sizeof(buf), "%s%-15s%-6s%-15s%s%-19s%-6s%s",
                                   "VLAN User  ",
                                   "PortType",
                                   "PVID",
                                   "Frame Type",
#ifdef VTSS_SW_OPTION_VLAN_INGRESS_FILTERING
                                   "Ing Filter  ",
#else
                                   "",
#endif  /* VTSS_SW_OPTION_VLAN_INGRESS_FILTERING */
                                   "Tx Tag",
                                   "UVID",
                                   last_user != VLAN_USER_STATIC ? "Conflicts" : "");

                    icli_table_header(session_id, buf);
                    print_hdr = FALSE;
                }

                T_NG(VTSS_TRACE_GRP_CLI, "has_conflicts:%d, conflicts.port_flags:%d", has_conflicts, conflicts.port_flags);
                if (!has_conflicts || conflicts.port_flags) {
                    ICLI_PRINTF("%-11s", vlan_mgmt_user_to_txt(usr));

                    ICLI_PRINTF("%-15s", conf.flags & VLAN_PORT_FLAGS_AWARE ? vlan_mgmt_port_type_to_txt(conf.port_type) : "");

                    if (conf.flags & VLAN_PORT_FLAGS_PVID) {
                        ICLI_PRINTF("%-6d", conf.pvid);
                    } else {
                        ICLI_PRINTF("%-6s", "");
                    }

                    ICLI_PRINTF("%-15s", (conf.flags & VLAN_PORT_FLAGS_RX_TAG_TYPE) ? vlan_mgmt_frame_type_to_txt(conf.frame_type) : "");

#ifdef VTSS_SW_OPTION_VLAN_INGRESS_FILTERING
                    ICLI_PRINTF("%-12s", (conf.flags & VLAN_PORT_FLAGS_INGR_FILT) ? (conf.ingress_filter ? "Enabled" : "Disabled") : "");
#endif  /* VTSS_SW_OPTION_VLAN_INGRESS_FILTERING */

                    ICLI_PRINTF("%-19s", (conf.flags & VLAN_PORT_FLAGS_TX_TAG_TYPE) ? VLAN_ICLI_tx_tag_type_to_txt(conf.tx_tag_type) : "");

                    if (conf.flags & VLAN_PORT_FLAGS_TX_TAG_TYPE) {
                        ICLI_PRINTF("%-6d", conf.untagged_vid);
                    } else {
                        ICLI_PRINTF("%-6s", "");
                    }

                    if (last_user != VLAN_USER_STATIC) {
                        if (usr != VLAN_USER_STATIC) {
                            if (conflicts.port_flags == 0) {
                                ICLI_PRINTF("%s", "No");
                            } else {
                                if (all_usrs & (1 << (u8)usr)) {
                                    ICLI_PRINTF("%s", "  Yes");
                                } else {
                                    ICLI_PRINTF("%s", "  No");
                                }
                            }
                        }
                    }
                    ICLI_PRINTF("\n");
                }
            }
            ICLI_PRINTF("\n");
        }
    }
    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_runtime_dot1x()
/******************************************************************************/
BOOL VLAN_ICLI_runtime_dot1x(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_DOT1X)
        runtime->present = vlan_mgmt_user_is_port_conf_changer(VLAN_USER_DOT1X);
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

/******************************************************************************/
// VLAN_ICLI_runtime_mvr()
/******************************************************************************/
BOOL VLAN_ICLI_runtime_mvr(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_MVR)
        runtime->present = vlan_mgmt_user_is_port_conf_changer(VLAN_USER_MVR);
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

/******************************************************************************/
// VLAN_ICLI_runtime_voice_vlan()
/******************************************************************************/
BOOL VLAN_ICLI_runtime_voice_vlan(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_VOICE_VLAN)
        runtime->present = vlan_mgmt_user_is_port_conf_changer(VLAN_USER_VOICE_VLAN);
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}


/******************************************************************************/
// VLAN_ICLI_runtime_mstp()
/******************************************************************************/
BOOL VLAN_ICLI_runtime_mstp(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_MSTP)
        runtime->present = vlan_mgmt_user_is_port_conf_changer(VLAN_USER_MSTP);
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}


//******************************************************************************/
// VLAN_ICLI_runtime_erps()
/******************************************************************************/
BOOL VLAN_ICLI_runtime_erps(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_ERPS)
        runtime->present = vlan_mgmt_user_is_port_conf_changer(VLAN_USER_ERPS);
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

/******************************************************************************/
// VLAN_ICLI_runtime_vcl()
/******************************************************************************/
BOOL VLAN_ICLI_runtime_vcl(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_VCL)
        runtime->present = vlan_mgmt_user_is_port_conf_changer(VLAN_USER_VCL);
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

/******************************************************************************/
// VLAN_ICLI_runtime_evc()
/******************************************************************************/
BOOL VLAN_ICLI_runtime_evc(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_EVC)
        runtime->present = vlan_mgmt_user_is_port_conf_changer(VLAN_USER_EVC);
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

/******************************************************************************/
// VLAN_ICLI_runtime_gvrp()
/******************************************************************************/
BOOL VLAN_ICLI_runtime_gvrp(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_GVRP)
        runtime->present = vlan_mgmt_user_is_port_conf_changer(VLAN_USER_GVRP);
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

/******************************************************************************/
// VLAN_ICLI_hybrid_port_conf()
// PVID is configured through VLAN_ICLI_pvid_set()
/******************************************************************************/
vtss_rc VLAN_ICLI_hybrid_port_conf(icli_stack_port_range_t *plist, vlan_port_composite_conf_t *new_conf, BOOL frametype, BOOL ingressfilter, BOOL porttype, BOOL tx_tag)
{
    switch_iter_t              sit;
    port_iter_t                pit;
    vtss_rc                    rc;
    vlan_port_composite_conf_t old_conf;

    // Loop through all configurable switches in stack
    VTSS_RC(icli_switch_iter_init(&sit));
    while (icli_switch_iter_getnext(&sit, plist)) {
        // Loop through all ports
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL));
        while (icli_port_iter_getnext(&pit, plist)) {
            if ((rc = vlan_mgmt_port_composite_conf_get(sit.isid, pit.iport, &old_conf)) != VTSS_RC_OK) {
                T_E("%u:%u: %s", sit.isid, pit.uport, error_txt(rc));
                return rc;
            }

            old_conf.hyb_port_conf.flags = VLAN_PORT_FLAGS_ALL;

            if (frametype) {
                old_conf.hyb_port_conf.frame_type = new_conf->hyb_port_conf.frame_type;
            }

#ifdef VTSS_SW_OPTION_VLAN_INGRESS_FILTERING
            if (ingressfilter) {
                old_conf.hyb_port_conf.ingress_filter = new_conf->hyb_port_conf.ingress_filter;
            }
#endif  /* VTSS_SW_OPTION_VLAN_INGRESS_FILTERING */

            if (tx_tag) {
                if (old_conf.hyb_port_conf.tx_tag_type == VLAN_TX_TAG_TYPE_UNTAG_THIS) {
                    // Make sure to always untag the PVID
                    old_conf.hyb_port_conf.untagged_vid = old_conf.hyb_port_conf.pvid;
                }

                old_conf.hyb_port_conf.tx_tag_type = new_conf->hyb_port_conf.tx_tag_type;
            }

            if (porttype) {
                old_conf.hyb_port_conf.port_type = new_conf->hyb_port_conf.port_type;
            }

            if ((rc = vlan_mgmt_port_composite_conf_set(sit.isid, pit.iport, &old_conf)) != VTSS_RC_OK) {
                return rc;
            }
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_pvid_set()
/******************************************************************************/
vtss_rc VLAN_ICLI_pvid_set(icli_stack_port_range_t *plist, vlan_port_mode_t port_mode, vtss_vid_t new_pvid)
{
    switch_iter_t              sit;
    port_iter_t                pit;
    vtss_rc                    rc;
    vlan_port_composite_conf_t composite_conf;

    // Loop through all configurable switches in stack
    VTSS_RC(icli_switch_iter_init(&sit));
    while (icli_switch_iter_getnext(&sit, plist)) {
        // Loop through all ports
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL));
        while (icli_port_iter_getnext(&pit, plist)) {
            if ((rc = vlan_mgmt_port_composite_conf_get(sit.isid, pit.iport, &composite_conf)) != VTSS_RC_OK) {
                T_E("%u:%u: %s", sit.isid, pit.uport, error_txt(rc));
                return rc;
            }

            switch (port_mode) {
            case VLAN_PORT_MODE_ACCESS:
                if (composite_conf.access_vid == new_pvid) {
                    continue;
                }

                composite_conf.access_vid = new_pvid;
                break;

            case VLAN_PORT_MODE_TRUNK:
                if (composite_conf.native_vid == new_pvid) {
                    continue;
                }

                composite_conf.native_vid = new_pvid;
                break;

            default:
                // Hybrid:
                if (composite_conf.hyb_port_conf.pvid == new_pvid) {
                    continue;
                }

                composite_conf.hyb_port_conf.pvid         = new_pvid;
                // If port is configured to "untag this", we must set
                // uvid = pvid. Administrative user cannot configure switch to
                // untag another VID than PVID, and it cannot configure
                // the switch to tag a specific VID, so it's safe
                // to always set untagged_vid to PVID here.
                composite_conf.hyb_port_conf.untagged_vid = new_pvid;
                break;
            }

            if ((rc = vlan_mgmt_port_composite_conf_set(sit.isid, pit.iport, &composite_conf)) != VTSS_RC_OK) {
                T_E("%u:%u: %s", sit.isid, pit.uport, error_txt(rc));
                return rc;
            }
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_mode_set()
/******************************************************************************/
vtss_rc VLAN_ICLI_mode_set(icli_stack_port_range_t *plist, vlan_port_mode_t new_mode)
{
    switch_iter_t              sit;
    port_iter_t                pit;
    vtss_rc                    rc;
    vlan_port_composite_conf_t composite_conf;

    // Loop over all configurable switches in usid order...
    (void)icli_switch_iter_init(&sit);
    // ...provided they're also in the plist.
    while (icli_switch_iter_getnext(&sit, plist)) {
        // Loop over all ports in uport order...
        (void)icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        // ... provided they're also in the plist.
        while (icli_port_iter_getnext(&pit, plist)) {
            if ((rc = vlan_mgmt_port_composite_conf_get(sit.isid, pit.iport, &composite_conf)) != VTSS_RC_OK) {
                T_E("%u:%u: %s", sit.isid, pit.uport, error_txt(rc));
                return rc;
            }

            if (new_mode != composite_conf.mode) {
                composite_conf.mode = new_mode;
                if ((rc = vlan_mgmt_port_composite_conf_set(sit.isid, pit.iport, &composite_conf)) != VTSS_RC_OK) {
                    T_E("%u:%u: %s", sit.isid, pit.uport, error_txt(rc));
                    return rc;
                }
            }
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_tag_native_vlan_set()
/******************************************************************************/
vtss_rc VLAN_ICLI_tag_native_vlan_set(icli_stack_port_range_t *plist, BOOL tag_native_vlan)
{
    switch_iter_t              sit;
    port_iter_t                pit;
    vtss_rc                    rc;
    vlan_port_composite_conf_t composite_conf;

    // Loop over all configurable switches in usid order...
    (void)icli_switch_iter_init(&sit);
    // ...provided they're also in the plist.
    while (icli_switch_iter_getnext(&sit, plist)) {
        // Loop over all ports in uport order...
        (void)icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        // ... provided they're also in the plist.
        while (icli_port_iter_getnext(&pit, plist)) {
            if ((rc = vlan_mgmt_port_composite_conf_get(sit.isid, pit.iport, &composite_conf)) != VTSS_RC_OK) {
                T_E("%u:%u: %s", sit.isid, pit.uport, error_txt(rc));
                return rc;
            }

            if (tag_native_vlan != composite_conf.tag_native_vlan) {
                composite_conf.tag_native_vlan = tag_native_vlan;
                if ((rc = vlan_mgmt_port_composite_conf_set(sit.isid, pit.iport, &composite_conf)) != VTSS_RC_OK) {
                    T_E("%u:%u: %s", sit.isid, pit.uport, error_txt(rc));
                    return rc;
                }
            }
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_port_mode_txt()
/******************************************************************************/
char *VLAN_ICLI_port_mode_txt(vlan_port_mode_t mode)
{
    if (mode == VLAN_PORT_MODE_ACCESS) {
        return "access";
    } else if (mode == VLAN_PORT_MODE_TRUNK) {
        return "trunk";
    } else if (mode == VLAN_PORT_MODE_HYBRID) {
        return "hybrid";
    }
    return "";
}

/******************************************************************************/
// VLAN_ICLI_vlan_print()
// Returns TRUE if further processing should be aborted. FALSE otherwise.
/******************************************************************************/
static BOOL VLAN_ICLI_vlan_print(u32 session_id, vtss_vid_t vid, BOOL *first)
{
    u8                indent = 0; // For the first interface there is no indent, since it is printed at the same line as the VLAN
    char              str_buf[ICLI_PORTING_STR_BUF_SIZE * VTSS_PORT_ARRAY_SIZE];
    BOOL              name_indent, at_least_one_printed = FALSE;
    icli_line_mode_t  line_mode;
    vlan_mgmt_entry_t conf;
    vtss_rc           rc;
    switch_iter_t     sit;

#ifdef VTSS_SW_OPTION_VLAN_NAMING
    char             vlan_name[VLAN_NAME_MAX_LEN];

    if ((rc = vlan_mgmt_name_get(vid, vlan_name, NULL)) != VTSS_RC_OK) {
        T_E("Unable to obtain VLAN name for vid =¤%d. Error = %s", vid, error_txt(rc));
        vlan_name[0] = '\0';
    }
#endif

    if (*first) { // Print header if this is the first VLAN found
#ifdef VTSS_SW_OPTION_VLAN_NAMING
        sprintf(str_buf, "%-4s  %-32s  %s", "VLAN", "Name", "Interfaces");
#else
        sprintf(str_buf, "%-4s  %s", "VLAN", "Interfaces");
#endif
        icli_table_header(session_id, str_buf);
        *first = FALSE;
    }

#ifdef VTSS_SW_OPTION_VLAN_NAMING
    ICLI_PRINTF("%-4d  %-32s  ", vid, vlan_name);
    name_indent = 40;
#else
    ICLI_PRINTF("%-4d  ", vid);
    name_indent = 6;
#endif

    // Find all switches/ports assigned to this VLAN
    VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID_CFG));
    while (switch_iter_getnext(&sit)) {
        T_IG(VTSS_TRACE_GRP_CLI, "sid:%d, vid:%d", sit.isid, vid);

        // The VLAN may or may not exist depending on how this function is called,
        // so we don't care about the return value of vlan_mgmt_vlan_get().
        // vlan_mgmt_vlan_get() is guaranteed to memset(conf) to all-zeros if it doesn't
        // exist, so no need to do that ourselves.
        (void)vlan_mgmt_vlan_get(sit.isid, vid, &conf, FALSE, VLAN_USER_STATIC);
        (void)icli_port_list_info_txt(sit.isid, conf.ports, str_buf, TRUE);
        if (strlen(str_buf) != 0) {
            ICLI_PRINTF("%*s%s\n", indent, "", str_buf);
            indent = name_indent; // Indent if multiple lines are needed to print interface from another switch.
            at_least_one_printed = TRUE;
        }
    }

    if (!at_least_one_printed) {
        // Gotta print a newline then.
        ICLI_PRINTF("\n");
    }

    // Check if user aborted output (^C or Q at -- more -- prompt)
    return (ICLI_LINE_MODE_GET(&line_mode) == ICLI_RC_OK && line_mode == ICLI_LINE_MODE_BYPASS);
}

/******************************************************************************/
// VLAN_ICLI_show_vlan()
/******************************************************************************/
vtss_rc VLAN_ICLI_show_vlan(u32 session_id, icli_unsigned_range_t *vlan_list, char *name, BOOL has_vid, BOOL has_name)
{
    BOOL                  first = TRUE, show_defined_only = FALSE;
    icli_unsigned_range_t my_list;
    u32                   idx;
    vtss_vid_t            vid;
    vtss_rc               rc;

    if (has_name) {
#if VTSS_SW_OPTION_VLAN_NAMING
        if (name == NULL || strlen(name) == 0) {
            T_E("Huh?");
            return VTSS_RC_ERROR;
        }

        if ((rc = vlan_mgmt_name_to_vid(name, &vid)) != VTSS_RC_OK) {
            ICLI_PRINTF("%% %s\n", error_txt(rc));
            return VTSS_RC_OK; // Not a programmatic error
        }

        my_list.cnt = 1;
        my_list.range[0].min = my_list.range[0].max = vid;
#else
        T_E("VLAN naming not compile-time enabled on this switch");
        return VTSS_RC_ERROR;
#endif
    } else if (has_vid) {
        if (vlan_list == NULL) {
            T_E("Huh?");
            return VTSS_RC_ERROR;
        }

        my_list = *vlan_list;
    } else {
        my_list.cnt = 1; // Make Lint happy
        show_defined_only = TRUE;
    }

    if (show_defined_only) {
        vlan_mgmt_entry_t conf;

        conf.vid = VTSS_VID_NULL;

        while (vlan_mgmt_vlan_get(VTSS_ISID_GLOBAL, conf.vid, &conf, TRUE, VLAN_USER_STATIC) == VTSS_RC_OK) {
            if (VLAN_ICLI_vlan_print(session_id, conf.vid, &first)) {
                // Aborted output
                return VTSS_RC_OK;
            }
        }
    } else {
        // User is asking for specific VLANs.
        for (idx = 0; idx < my_list.cnt; idx++) {
            for (vid = my_list.range[idx].min; vid <= my_list.range[idx].max; vid++) {
                if (VLAN_ICLI_vlan_print(session_id, vid, &first)) {
                    // Aborted output
                    return VTSS_RC_OK;
                }
            }
        }
    }

    if (first) {
        if (has_name || has_vid) {
            T_E("Huh?");
        } else {
            ICLI_PRINTF("%% No VLANs found");
        }
    }

    ICLI_PRINTF("\n");

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_forbidden_print()
/******************************************************************************/
static vtss_rc VLAN_ICLI_forbidden_print(u32 session_id, vtss_isid_t isid, BOOL show_name, vlan_mgmt_entry_t *forbidden_conf, vtss_vid_t vid, BOOL print_header)
{
    char buf[(VTSS_PORTS + 1) / 2 * (2 + 1) + 40]; // Worst-case is every other port, which gives approximately (VTSS_PORTS + 1) / 2 * (2 digits + 1 comma) + some slack.

    if (print_header) {
        if (show_name) {
            icli_table_header(session_id, "VID   VLAN Name                         Interfaces");
        } else {
            icli_table_header(session_id, "VID   Interfaces");
        }
    }

    if (show_name) {
#if defined(VTSS_SW_OPTION_VLAN_NAMING)
        vtss_rc rc;
        char    vlan_name[VLAN_NAME_MAX_LEN];

        if ((rc = vlan_mgmt_name_get(vid, vlan_name, NULL)) != VTSS_RC_OK) {
            T_E("%s", error_txt(rc));
            return VTSS_RC_ERROR;
        }

        ICLI_PRINTF("%-4d  %-32s  %s\n", vid, vlan_name, icli_iport_list_txt(forbidden_conf->ports, buf));
#else
        T_E("VLAN naming not compile-time enabled on this switch");
        return VTSS_RC_ERROR;
#endif /* defined(VTSS_SW_OPTION_VLAN_NAMING) */
    } else {
        ICLI_PRINTF("%-4d  %s\n", vid, icli_iport_list_txt(forbidden_conf->ports, buf));
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_show_forbidden()
//   #has_vid  == TRUE: User has requested to show a specific VID, given by #vid
//   #has_name == TRUE: User has requested to show a specific named VLAN, given by #name.
//   #has_vid == FALSE and #has_name == FALSE: User wants to show all VIDs.
/******************************************************************************/
vtss_rc VLAN_ICLI_show_forbidden(u32 session_id, BOOL has_vid, vtss_vid_t vid, BOOL has_name, i8 *name)
{
    switch_iter_t     sit;
    vlan_mgmt_entry_t forbidden_conf;
    vtss_rc           rc = VTSS_RC_OK;
    u32               cnt = 0;

    VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID_CFG));
    while (switch_iter_getnext(&sit)) {
        if (has_name) {
#ifdef VTSS_SW_OPTION_VLAN_NAMING
            // Lookup specific VLAN name
            if ((rc = vlan_mgmt_name_to_vid(name, &vid)) != VTSS_RC_OK) {
                ICLI_PRINTF("%% %s\n", error_txt(rc));
                return VTSS_RC_OK; // Not a programmatic error
            }

            VTSS_RC(VLAN_ICLI_forbidden_print(session_id, sit.isid, TRUE, &forbidden_conf, vid, TRUE));
            cnt++;
#else
            T_E("VLAN naming not compile-time enabled on this switch");
            return VTSS_RC_ERROR;
#endif
        } else {
            // Lookup all VIDs or specific VID
            if (!has_vid) {
                // Lookup all VIDs
                vid = VTSS_VID_NULL;
            }

            while ((rc = vlan_mgmt_vlan_get(sit.isid, vid, &forbidden_conf, has_vid ? FALSE : TRUE, VLAN_USER_FORBIDDEN)) == VTSS_RC_OK) {
                VTSS_RC(VLAN_ICLI_forbidden_print(session_id, sit.isid, FALSE, &forbidden_conf, forbidden_conf.vid, cnt++ == 0));

                if (has_vid) {
                    break;
                }

                // Prepare for next iteration
                vid = forbidden_conf.vid;
            }

            if (rc == VLAN_ERROR_ENTRY_NOT_FOUND) {
                // Not an error. Just means that it didn't find any forbidden VLANs
                // A possible error is shown below, if none were found in any switch.
                rc = VTSS_RC_OK;
            } else if (rc != VTSS_RC_OK) {
                // Something serious wrong.
                break;
            }
        }
    }

    if (rc != VTSS_RC_OK) {
        ICLI_PRINTF("%% Error: %s\n", error_txt(rc));
    } else if (cnt == 0) {
        // Nothing has been printed at all.
        if (has_name) {
            ICLI_PRINTF("Forbidden VLAN table is empty for VLAN name %s\n", name);
        } else if (has_vid) {
            ICLI_PRINTF("Forbidden VLAN table is empty for VLAN ID %d\n", vid);
        } else {
            ICLI_PRINTF("Forbidden VLAN table is empty\n");
        }
    }

    return rc;
}

/******************************************************************************/
// VLAN_ICLI_add_remove_forbidden()
/******************************************************************************/
vtss_rc VLAN_ICLI_add_remove_forbidden(icli_stack_port_range_t *plist, icli_unsigned_range_t *vlan_list, BOOL has_add)
{
    switch_iter_t     sit;
    port_iter_t       pit;
    u32               idx;
    vlan_mgmt_entry_t conf;
    vtss_rc           rc;
    vtss_vid_t        vid;

    // Satisfy Lint:
    if (vlan_list == NULL) {
        T_E("Unexpected NULL");
        return VTSS_RC_ERROR;
    }

    for (idx = 0; idx < vlan_list->cnt; idx++) {
        for (vid = vlan_list->range[idx].min; vid <= vlan_list->range[idx].max; vid++) {

            VTSS_RC(icli_switch_iter_init(&sit));
            while (icli_switch_iter_getnext(&sit, plist)) {
                BOOL do_add = FALSE;

                if ((rc = vlan_mgmt_vlan_get(sit.isid, vid, &conf, FALSE, VLAN_USER_FORBIDDEN)) != VTSS_RC_OK) {
                    T_D("%u: %s", sit.isid, error_txt(rc));

                    // Probably doesn't exist.
                    if (!has_add) {
                        // If we're removing ports from the existing list, there's nothing more to do for this switch.
                        continue;
                    }

                    // Otherwise, initialize all to non-forbidden.
                    memset(conf.ports, 0, sizeof(conf.ports));
                }

                conf.vid = vid;

                // Loop through all ports, and set forbidden/non-forbidden for the selected ports
                VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL));
                while (icli_port_iter_getnext(&pit, plist)) {
                    conf.ports[pit.iport] = has_add;
                }

                // Gotta run through it once more to figure out whether to add or delete this forbidden VLAN.
                // This time, go through all known (non-stacking) ports on the switch.
                VTSS_RC(port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL));
                while (port_iter_getnext(&pit)) {
                    if (conf.ports[pit.iport]) {
                        do_add = TRUE;
                        break;
                    }
                }

                if (do_add) {
                    if ((rc = vlan_mgmt_vlan_add(sit.isid, &conf, VLAN_USER_FORBIDDEN)) != VTSS_RC_OK) {
                        T_E("%u:%u: %s", sit.isid, vid, error_txt(rc));
                        return rc;
                    }
                } else {
                    if ((rc = vlan_mgmt_vlan_del(sit.isid, vid, VLAN_USER_FORBIDDEN)) != VTSS_RC_OK) {
                        T_E("%u:%u: %s", sit.isid, vid, error_txt(rc));
                        return rc;
                    }
                }
            }
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_bitmask_update()
/******************************************************************************/
static void VLAN_ICLI_bitmask_update(icli_unsigned_range_t *vlan_list, u8 *bitmask, BOOL set)
{
    vtss_vid_t vid;
    u32        idx;

    for (idx = 0; idx < vlan_list->cnt; idx++) {
        for (vid = vlan_list->range[idx].min; vid <= vlan_list->range[idx].max; vid++) {
            VTSS_BF_SET(bitmask, vid, set);
        }
    }
}

/******************************************************************************/
// VLAN_ICLI_allowed_vids_set()
/******************************************************************************/
vtss_rc VLAN_ICLI_allowed_vids_set(icli_stack_port_range_t *plist, vlan_port_mode_t port_mode, icli_unsigned_range_t *vlan_list, BOOL has_default, BOOL has_all, BOOL has_none, BOOL has_add, BOOL has_remove, BOOL has_except)
{
    switch_iter_t sit;
    port_iter_t   pit;
    vtss_rc       rc;
    u8            bitmask[VLAN_BITMASK_LEN_BYTES];
    u8            *bitmask_ptr = bitmask;

    // has_default, has_all, has_none, and has_except all use a fixed bitmask,
    // which can be obtained prior to going into the sit/pit loop below.

    // has_add, has_remove, and has_set use the currently defined bitmasks, which is per-port,
    // which means that we cannot compute it prior to entering the loop below.
    if (has_default) {
        // Reset to defaults. First get whatever defaults is.
        u8 def[VLAN_BITMASK_LEN_BYTES];
        (void)vlan_mgmt_port_composite_allowed_vids_default_get(port_mode, def);
        bitmask_ptr = def;
    } else if (has_all || has_except) {
        memset(bitmask, 0xff, sizeof(bitmask));

        if (has_except) {
            // Remove VIDs in the vlan_list from bitmask
            VLAN_ICLI_bitmask_update(vlan_list, bitmask, FALSE);
        }
    } else if (has_none || (!has_remove && !has_add)) {
        // Here, either the "none" keyword or no keywords at all were used on the command line.
        memset(bitmask, 0x0, sizeof(bitmask));

        if (!has_none) {
            // Overwrite (no keywords were used on the command line).
            VLAN_ICLI_bitmask_update(vlan_list, bitmask, TRUE);
        }
    }

    // Loop through all configurable switches in stack
    VTSS_RC(icli_switch_iter_init(&sit));
    while (icli_switch_iter_getnext(&sit, plist)) {
        // Loop through all ports
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL));
        while (icli_port_iter_getnext(&pit, plist)) {
            if (has_add || has_remove) {
                // add and remove are the only two ones that require the current list of VIDs to be updated.
                if ((rc = vlan_mgmt_port_composite_allowed_vids_get(sit.isid, pit.iport, port_mode, bitmask)) != VTSS_RC_OK) {
                    T_E("%u/%u: %s", sit.usid, pit.uport, error_txt(rc));
                    return rc;
                }

                // Overwrite (no keywords were used on the command line).
                VLAN_ICLI_bitmask_update(vlan_list, bitmask, has_add);
            }

            if ((rc = vlan_mgmt_port_composite_allowed_vids_set(sit.isid, pit.iport, port_mode, bitmask_ptr)) != VTSS_RC_OK) {
                T_E("%u/%u: %s", sit.usid, pit.uport, error_txt(rc));
                return rc;
            }
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// VLAN_ICLI_vlan_mode_enter()
/******************************************************************************/
void VLAN_ICLI_vlan_mode_enter(u32 session_id, icli_unsigned_range_t *vlan_list)
{
    vlan_mgmt_entry_t conf;
    u32               idx;
    vtss_vid_t        vid;
    vtss_rc           rc;

    (void)vlan_bulk_update_begin();
    for (idx = 0; idx < vlan_list->cnt; idx++) {
        for (vid = vlan_list->range[idx].min; vid <= vlan_list->range[idx].max; vid++) {
            // Skip adding if it already exists.
            if (vlan_mgmt_vlan_get(VTSS_ISID_GLOBAL, vid, &conf, FALSE, VLAN_USER_STATIC) == VTSS_RC_OK) {
                // Only add non-existing VLANs
                continue;
            }

            // Add it to all switches, configurable as well as non-configurable,
            // so that we can update completely new switches with correct info
            // once they arrive in the stack. We don't care about memberships
            // because this is handled inside of vlan_mgmt_vlan_add() depending
            // on port configuration.
            conf.vid = vid;
            if ((rc = vlan_mgmt_vlan_add(VTSS_ISID_GLOBAL, &conf, VLAN_USER_STATIC)) != VTSS_RC_OK) {
                T_E("%d: %s\n", vid, error_txt(rc));
                ICLI_PRINTF("%% VLAN add of VID %u failed. Error message: %s\n", vid, error_txt(rc));
            }
        }
    }
    (void)vlan_bulk_update_end();
}

/******************************************************************************/
// VLAN_ICLI_runtime_vlan_name()
// Function for runtime asking if VLAN name is supported.
/******************************************************************************/
BOOL VLAN_ICLI_runtime_vlan_name(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_VLAN_NAMING)
        runtime->present = TRUE;
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        break;
    }

    return FALSE;
}


/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
