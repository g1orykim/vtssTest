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
#if defined(VTSS_SW_OPTION_ICLI)
#include "icli_api.h"
#include "icli_porting_util.h"
#endif /* VTSS_SW_OPTION_ICLI */

#include "rfc2544_xcli.h"      /* For ourselves                        */
#include "rfc2544_api.h"       /* For rfc2544_mgmt_XXX()               */
#include "rfc2544_trace.h"     /* For T_E()                            */
#include "misc_api.h"          /* For misc_url_XXX()                   */
#include "firmware_api.h"      /* For firmware_tftp_err2str()          */
#include <sys/socket.h>        /* For tftp_support.h                   */
#include <netinet/in.h>        /* For tftp_support.h                   */
#include <tftp_support.h>      /* For tftp_client_put() and TFTP_OCTET */

#if defined(VTSS_SW_OPTION_ICLI)
#if defined(VTSS_SW_OPTION_VCLI)
// Both VCLI and ICLI
#define xcli_printf(...) do {                    \
    if (session_id == RFC2544_SESSION_ID_NONE) { \
        cli_printf(__VA_ARGS__);                 \
    } else {                                     \
        ICLI_PRINTF(__VA_ARGS__);                \
    }                                            \
} while (0)
#define xcli_table_header(...) do {                 \
    if (session_id == RFC2544_SESSION_ID_NONE) {    \
        cli_table_header(__VA_ARGS__);              \
    } else {                                        \
        icli_table_header(session_id, __VA_ARGS__); \
    }                                               \
} while (0)
#else
// Only ICLI
#define xcli_printf(...)       ICLI_PRINTF(__VA_ARGS__)
#define xcli_table_header(...) icli_table_header(session_id, __VA_ARGS__)
#endif
#else
// Only VCLI
#define xcli_printf(...)       cli_printf(__VA_ARGS__)
#define xcli_table_header(...) cli_table_header(__VA_ARGS__)
#endif

#define xcli_error_prefix ((session_id == RFC2544_SESSION_ID_NONE) ? "Error: " : "% ")

/******************************************************************************/
// RFC2544_xcli_error()
/******************************************************************************/
static void RFC2544_xcli_error(u32 session_id, vtss_rc rc)
{
    xcli_printf("%s%s\n", xcli_error_prefix, error_txt(rc));
}

/******************************************************************************/
// RFC2544_xcli_cmd_profile_show()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_profile_show(u32 session_id, char *profile_name)
{
    rfc2544_profile_t profile;
    i8                profile_iter_name[RFC2544_PROFILE_NAME_LEN];
    vtss_rc           rc;
    u32               cnt = 0;

    if (profile_name != NULL) {
        // Show a specific profile
        i8   *profile_as_txt;
        BOOL printed = FALSE;

        if ((rc = rfc2544_mgmt_profile_as_txt(profile_name, &profile_as_txt)) != VTSS_RC_OK) {
            RFC2544_xcli_error(session_id, rc);
            return rc;
        }

        // My goodness. ICLI doesn't support printing more than ICLI_STR_MAX_LEN + 64 characters in one go...
#if defined(VTSS_SW_OPTION_ICLI)
        if (session_id != RFC2544_SESSION_ID_NONE) {
            // Not only requires ICLI a special function to print it. The function also modifies my string :-(
            ICLI_PRINTF_LSTR(profile_as_txt);
            printed = TRUE;
        }
#endif
        if (!printed) {
            xcli_printf("%s", profile_as_txt);
        }
        VTSS_FREE(profile_as_txt);
    } else {
        // List all profiles
        xcli_printf("\n");
        xcli_table_header("Profile Name                      Description                       ");
        profile_iter_name[0] = '\0';

        while (1) {
            if ((rc = rfc2544_mgmt_profile_names_get(profile_iter_name)) != VTSS_RC_OK) {
                T_E("How did I get here?");
                RFC2544_xcli_error(session_id, rc);
                return rc;
            }

            if (profile_iter_name[0] == '\0') {
                // No more profiles.
                break;
            }

            if (rfc2544_mgmt_profile_get(profile_iter_name, &profile) != VTSS_RC_OK) {
                // Could happen if someone deletes the profile between the profile_names_get() and profile_get() call.
                break;
            }

            xcli_printf("%-32s  %s\n", profile_iter_name, profile.common.dscr);
            cnt++;
        }

        if (cnt == 0) {
            xcli_printf("<No profiles>\n");
        }
        xcli_printf("\n");
    }
    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_xcli_cmd_test_start()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_test_start(u32 session_id, char *profile_name, char *report_name, char *report_dscr)
{
    vtss_rc rc;

    if ((rc = rfc2544_mgmt_test_start(profile_name, report_name, report_dscr)) != VTSS_RC_OK) {
        RFC2544_xcli_error(session_id, rc);
    }

    return rc;
}

/******************************************************************************/
// RFC2544_xcli_cmd_test_stop()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_test_stop(u32 session_id, char *report_name)
{
    vtss_rc rc;

    if ((rc = rfc2544_mgmt_test_stop(report_name)) != VTSS_RC_OK) {
        RFC2544_xcli_error(session_id, rc);
    }

    return rc;
}

/******************************************************************************/
// RFC2544_xcli_cmd_report_show()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_report_show(u32 session_id, char *report_name)
{
    rfc2544_report_info_t info;
    vtss_rc               rc;
    u32                   cnt = 0;

    if (report_name) {
        // Show a specific report
        i8   *report_as_txt;
        BOOL printed = FALSE;

        if ((rc = rfc2544_mgmt_report_as_txt(report_name, &report_as_txt)) != VTSS_RC_OK) {
            RFC2544_xcli_error(session_id, rc);
            return rc;
        }

        // My goodness. ICLI doesn't support printing more than ICLI_STR_MAX_LEN + 64 characters in one go...
#if defined(VTSS_SW_OPTION_ICLI)
        if (session_id != RFC2544_SESSION_ID_NONE) {
            // Not only requires ICLI a special function to print it. The function also modifies my string :-(
            ICLI_PRINTF_LSTR(report_as_txt);
            printed = TRUE;
        }
#endif
        if (!printed) {
            xcli_printf("%s", report_as_txt); // Use ("%s", report_as_txt) rather than (report_as_txt) to avoid cli_printf() eating any percent signs contained in the report.
        }
        VTSS_FREE(report_as_txt);
    } else {
        // List all reports
        xcli_printf("\n");
        xcli_table_header("Report Name                       Created                    Status             ");
        info.name[0] = '\0';

        while (1) {
            if ((rc = rfc2544_mgmt_report_info_get(&info)) != VTSS_RC_OK) {
                T_E("How did I get here?");
                RFC2544_xcli_error(session_id, rc);
                return rc;
            }

            if (info.name[0] == '\0') {
                // No more reports.
                break;
            }

            xcli_printf("%-32s  %25s  %s\n", info.name, info.creation_time, rfc2544_mgmt_util_status_to_str(info.status));
            cnt++;
        }

        if (cnt == 0) {
            xcli_printf("<No reports>\n");
        }
    }
    xcli_printf("\n");
    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_xcli_cmd_report_save()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_report_save(u32 session_id, char *report_name, char *tftp_url)
{
    vtss_rc          rc;
    misc_url_parts_t dest_url_parts;
    i8               *report_as_txt;
    int              res, err;

    if (!misc_url_decompose(tftp_url, &dest_url_parts) || strcmp(dest_url_parts.protocol, "tftp") != 0) {
        xcli_printf("%sInvalid syntax. Expected tftp://server[:port]/path-to-file\n", xcli_error_prefix);
        return VTSS_RC_ERROR;
    }

    // Get report
    if ((rc = rfc2544_mgmt_report_as_txt(report_name, &report_as_txt)) != VTSS_RC_OK) {
        RFC2544_xcli_error(session_id, rc);
        return rc;
    }

    if ((res = tftp_client_put(dest_url_parts.path, dest_url_parts.host, dest_url_parts.port, report_as_txt, strlen(report_as_txt), TFTP_OCTET, &err)) > 0) {
        xcli_printf("Saved %d bytes to TFTP server %s: %s\n", res, dest_url_parts.host, dest_url_parts.path);
        rc = VTSS_RC_OK;
    } else {
        char tftp_err_str[100];
        firmware_tftp_err2str(err, tftp_err_str);
        xcli_printf("%sTFTP save error: %s\n", xcli_error_prefix, tftp_err_str);
        rc = VTSS_RC_ERROR;
    }

    VTSS_FREE(report_as_txt);
    return rc;
}

/******************************************************************************/
// RFC2544_xcli_cmd_report_del()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_report_del(u32 session_id, char *report_name)
{
    vtss_rc rc;

    if ((rc = rfc2544_mgmt_report_delete(report_name)) != VTSS_RC_OK) {
        RFC2544_xcli_error(session_id, rc);
    }

    return rc;
}

/******************************************************************************/
// RFC2544_xcli_cmd_profile_del()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_profile_del(u32 session_id, char *profile_name)
{
    rfc2544_profile_t profile;
    vtss_rc           rc;

    // Setting the profile's name to NULL will delete it. The remaining fields will not be used.
    profile.common.name[0] = '\0';

    if ((rc = rfc2544_mgmt_profile_set(profile_name, &profile)) != VTSS_RC_OK) {
        RFC2544_xcli_error(session_id, rc);
    }

    return rc;
}

/******************************************************************************/
// RFC2544_xcli_cmd_profile_rename()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_profile_rename(u32 session_id, char *old_profile_name, char *new_profile_name)
{
    rfc2544_profile_t profile;
    vtss_rc           rc;

    if ((rc = rfc2544_mgmt_profile_get(old_profile_name, &profile)) != VTSS_RC_OK) {
        RFC2544_xcli_error(session_id, rc);
        return rc;
    }

    strcpy(profile.common.name, new_profile_name);

    if ((rc = rfc2544_mgmt_profile_set(old_profile_name, &profile)) != VTSS_RC_OK) {
        RFC2544_xcli_error(session_id, rc);
        return rc;
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_xcli_cmd_profile_add()
/******************************************************************************/
vtss_rc RFC2544_xcli_cmd_profile_add(u32 session_id, rfc2544_xcli_req_t *r)
{
    rfc2544_profile_t profile;
    vtss_rc           rc;
    BOOL              new_profile = FALSE;

#define RFC2544_XCLI_TRANSFER_VAL(_n_) do { \
    if (r->_n_ ## _seen) {                  \
        profile._n_ = r->profile._n_;       \
    }                                       \
} while (0)

#define RFC2544_XCLI_TRANSFER_TEST(_n_, _N_) do {                           \
    if (r->_n_.ena_dis_seen) {                                              \
        if (r->profile.common.selected_tests & RFC2544_TEST_TYPE_ ## _N_) { \
            profile.common.selected_tests |=  RFC2544_TEST_TYPE_ ## _N_;    \
        } else {                                                            \
            profile.common.selected_tests &= ~RFC2544_TEST_TYPE_ ## _N_;    \
        }                                                                   \
    }                                                                       \
} while (0)

    // Try to get profile requested by user.
    if ((rc = rfc2544_mgmt_profile_get(r->profile.common.name, &profile)) == RFC2544_ERROR_NO_SUCH_PROFILE) {
        // It didn't exist. Get a default profile
        if ((rc == rfc2544_mgmt_profile_get(NULL, &profile)) != VTSS_RC_OK) {
            // Couldn't even create a default profile.
            RFC2544_xcli_error(session_id, rc);
            return rc;
        }

        new_profile = TRUE;
        strcpy(profile.common.name, r->profile.common.name);
    } else if (rc != VTSS_RC_OK) {
        // What happened?
        RFC2544_xcli_error(session_id, rc);
        return rc;
    }

    if (r->common.dscr_seen) {
        strcpy(profile.common.dscr, r->profile.common.dscr);
    }

    RFC2544_XCLI_TRANSFER_VAL(common.meg_level);
    RFC2544_XCLI_TRANSFER_VAL(common.egress_port_no);
    RFC2544_XCLI_TRANSFER_VAL(common.sequence_number_check);
    RFC2544_XCLI_TRANSFER_VAL(common.dwell_time_secs);
    RFC2544_XCLI_TRANSFER_VAL(common.vlan_tag.vid);

    if (profile.common.vlan_tag.vid != VTSS_VID_NULL) {
        RFC2544_XCLI_TRANSFER_VAL(common.vlan_tag.pcp);
        RFC2544_XCLI_TRANSFER_VAL(common.vlan_tag.dei);
    }

    if (r->common.dmac_seen) {
        profile.common.dmac = r->profile.common.dmac;
    }

    if (r->common.frame_sizes_seen) {
        memcpy(profile.common.selected_frame_sizes, r->profile.common.selected_frame_sizes, sizeof(profile.common.selected_frame_sizes));
    }

    RFC2544_XCLI_TRANSFER_TEST(throughput, THROUGHPUT);
    if (profile.common.selected_tests & RFC2544_TEST_TYPE_THROUGHPUT) {
        RFC2544_XCLI_TRANSFER_VAL(throughput.trial_duration_secs);
        RFC2544_XCLI_TRANSFER_VAL(throughput.rate_min_permille);
        RFC2544_XCLI_TRANSFER_VAL(throughput.rate_max_permille);
        RFC2544_XCLI_TRANSFER_VAL(throughput.rate_step_permille);
        RFC2544_XCLI_TRANSFER_VAL(throughput.pass_criterion_permille);
    }

    RFC2544_XCLI_TRANSFER_TEST(latency, LATENCY);
    if (profile.common.selected_tests & RFC2544_TEST_TYPE_LATENCY) {
        RFC2544_XCLI_TRANSFER_VAL(latency.trial_duration_secs);
        RFC2544_XCLI_TRANSFER_VAL(latency.dmm_interval_secs);
        RFC2544_XCLI_TRANSFER_VAL(latency.pass_criterion_permille);
    }

    RFC2544_XCLI_TRANSFER_TEST(frame_loss, FRAME_LOSS);
    if (profile.common.selected_tests & RFC2544_TEST_TYPE_FRAME_LOSS) {
        RFC2544_XCLI_TRANSFER_VAL(frame_loss.trial_duration_secs);
        RFC2544_XCLI_TRANSFER_VAL(frame_loss.rate_min_permille);
        RFC2544_XCLI_TRANSFER_VAL(frame_loss.rate_max_permille);
        RFC2544_XCLI_TRANSFER_VAL(frame_loss.rate_step_permille);
    }

    RFC2544_XCLI_TRANSFER_TEST(back_to_back, BACK_TO_BACK);
    if (profile.common.selected_tests & RFC2544_TEST_TYPE_BACK_TO_BACK) {
        RFC2544_XCLI_TRANSFER_VAL(back_to_back.trial_duration_msecs);
        RFC2544_XCLI_TRANSFER_VAL(back_to_back.trial_cnt);
    }

    if ((rc = rfc2544_mgmt_profile_set(new_profile ? NULL : r->profile.common.name, &profile)) != VTSS_RC_OK) {
        RFC2544_xcli_error(session_id, rc);
        return rc;
    }

    return VTSS_RC_OK;

#undef RFC2544_XCLI_TRANSFER_VAL
#undef RFC2544_XCLI_TRANSFER_TEST
}

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/

