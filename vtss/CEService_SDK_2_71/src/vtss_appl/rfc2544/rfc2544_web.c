/*

 Vitesse Switch Software.

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

#include "web_api.h"                /* For web_retrieve_request_sid() */
#include "rfc2544_api.h"            /* Ourselves                      */
#include "rfc2544_trace.h"          /* For VTSS_TRACE_MODULE_ID       */
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_web_api.h" /* web_process_priv_lvl()         */
#endif

// Well, RFC2544_web_cross_err_msg is indeed not protected, but I find it very unlikely
// that the error message from one Web-browser instance gets forwarded to another
// Web-browser instance.
/*lint -esym(459,RFC2544_web_cross_err_msg)*/
static const char *RFC2544_web_cross_err_msg = "";

/******************************************************************************/
// RFC2544_web_name_get()
/******************************************************************************/
static vtss_rc RFC2544_web_name_get(CYG_HTTPD_STATE *p, const i8 *id, i8 *name, BOOL not_found_is_an_error, size_t name_len)
{
    size_t   found_len;
    const i8 *found_ptr;

    name[0] = '\0';

    if ((found_ptr = cyg_httpd_form_varable_string(p, id, &found_len)) != NULL) {
        if (!cgi_unescape(found_ptr, name, found_len, name_len)) {
            // Should not be possible
            T_E("URL-encoded name");
            return VTSS_RC_ERROR;
        }
    } else if (not_found_is_an_error) {
        T_E("Can't find %s element", id);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_web_profile_name_get()
/******************************************************************************/
static vtss_rc RFC2544_web_profile_name_get(CYG_HTTPD_STATE *p, const i8 *id, i8 *profile_name, BOOL not_found_is_an_error)
{
    return RFC2544_web_name_get(p, id, profile_name, not_found_is_an_error, RFC2544_PROFILE_NAME_LEN);
}

/******************************************************************************/
// RFC2544_web_report_name_get()
/******************************************************************************/
static vtss_rc RFC2544_web_report_name_get(CYG_HTTPD_STATE *p, const i8 *id, i8 *report_name, BOOL not_found_is_an_error)
{
    return RFC2544_web_name_get(p, id, report_name, not_found_is_an_error, RFC2544_REPORT_NAME_LEN);
}

/******************************************************************************/
// RFC2544_web_u32_get()
/******************************************************************************/
static int RFC2544_web_u32_get(CYG_HTTPD_STATE *p, i8 *id, u32 *val)
{
    long arg = 0;
    if (cyg_httpd_form_varable_long_int(p, id, &arg)) {
        *val = arg;
        return 0;
    }

    T_E("Can't find %s element", id);
    return 1; // Error
}

/******************************************************************************/
// RFC2544_web_handler_profiles()
/******************************************************************************/
static cyg_int32 RFC2544_web_handler_profiles(CYG_HTTPD_STATE *p)
{
    vtss_isid_t       isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    i8                profile_name[RFC2544_PROFILE_NAME_LEN];
    rfc2544_profile_t profile;
    vtss_rc           rc = VTSS_RC_OK;
    int               cnt;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_RFC2544)) {
        return -1;
    }
#endif

    if (p->method != CYG_HTTPD_METHOD_GET) {
        T_E("Only GET method is supported");
        return -1;
    }

    // This function is also used to delete profiles. This is the case when the URL contains the string "profile=<profile_name>"
    if ((rc = RFC2544_web_profile_name_get(p, "profile", profile_name, FALSE)) == VTSS_RC_OK && profile_name[0] != '\0') {
        // Delete the profile by first getting it, setting the name to an empty string and then setting it.
        if ((rc = rfc2544_mgmt_profile_get(profile_name, &profile)) == VTSS_RC_OK) {
            profile.common.name[0] = '\0';
            rc = rfc2544_mgmt_profile_set(profile_name, &profile);
        }
    }

    // Time to return all current profiles.
    (void)cyg_httpd_start_chunked("html");

    // Format: err_msg#MaxProfileCnt#[ProfileConfigs]
    //         [ProfileConfigs] = [ProfileConfig1]#[ProfileConfig2]#...#[ProfileConfigN]
    //         [ProfileConfig]  = Name/Description
    cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s#%u", rc != VTSS_RC_OK ? error_txt(rc) : RFC2544_web_cross_err_msg, RFC2544_PROFILE_CNT);
    RFC2544_web_cross_err_msg = "";
    (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

    profile_name[0] = '\0';
    while (1) {
        if (rfc2544_mgmt_profile_names_get(profile_name) != VTSS_RC_OK) {
            T_E("How did I get here?");
            break;
        }
        if (profile_name[0] == '\0') {
            // No more profiles.
            break;
        }

        if (rfc2544_mgmt_profile_get(profile_name, &profile) != VTSS_RC_OK) {
            // Could happen if another process deletes the profile given by #profile_name
            // in between the rfc2544_mgmt_profile_names_get() call above and here.
            // Therefore only emit a warning.
            T_W("Couldn't get profile for %s", profile_name);
            break;
        }

        if (profile.common.name[0] != '\0') {
            i8 encoded_profile_name[3 * RFC2544_PROFILE_NAME_LEN];
            i8 encoded_dscr[3 * RFC2544_PROFILE_DSCR_LEN];

            (void)cgi_escape(profile_name, encoded_profile_name);
            (void)cgi_escape(profile.common.dscr, encoded_dscr);

            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%s/%s", encoded_profile_name, encoded_dscr);
            (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
        } else {
            T_E("How did I get here?");
        }
    }

    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/******************************************************************************/
// RFC2544_web_handler_profile_edit()
/******************************************************************************/
static cyg_int32 RFC2544_web_handler_profile_edit(CYG_HTTPD_STATE *p)
{
    // Well, RFC2544_web_err_msg is indeed not protected, but I find it very unlikely
    // that the error message from one Web-browser instance gets forwarded to another
    // Web-browser instance.
    /*lint -esym(459,RFC2544_web_err_msg)*/
    /*lint -esym(459,RFC2544_web_saved_profile)*/
    /*lint -esym(459,RFC2544_web_saved_profile_edit_name)*/
    static const char        *RFC2544_web_err_msg = "";
    static rfc2544_profile_t RFC2544_web_saved_profile;
    static i8                RFC2544_web_saved_profile_edit_name[RFC2544_PROFILE_NAME_LEN];
    vtss_isid_t              isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    i8                       profile_name[RFC2544_PROFILE_NAME_LEN];
    size_t                   dscr_len;
    const i8                 *dscr_ptr;
    rfc2544_profile_t        profile;
    int                      cnt, errors = 0;
    vtss_rc                  rc;
    long                     val_long = 0;
    u32                      val_u32 = 0;
    i8                       encoded_profile_name[3 * RFC2544_PROFILE_NAME_LEN];
    i8                       encoded_profile_edit_name[3 * RFC2544_PROFILE_NAME_LEN];
    i8                       encoded_dscr[3 * RFC2544_PROFILE_DSCR_LEN];
    i8                       mac_str[18];
    u8                       *mac;
    rfc2544_frame_size_t     fs;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_SFLOW)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        // Figure out which profile to update. The current profile is saved in the hidden element called "orig_name".
        if ((rc = RFC2544_web_profile_name_get(p, "orig_name", profile_name, TRUE)) == VTSS_RC_OK) {
            // This will create a new default profile if profile_name is empty, or attempt to
            // get the existing profile if it's not.
            rc = rfc2544_mgmt_profile_get(profile_name, &profile);
        }

        if (rc != VTSS_RC_OK) {
            // Apparently the profile no longer exists. Redirect the user to the
            // profiles overview while passing an error message to him.
            RFC2544_web_cross_err_msg = error_txt(rc);
            redirect(p, "/rfc2544_profiles.htm");
            return -1;
        }

        // Now profile_name is the empty string if we're creating a new profile, or
        // the name of the profile that we're currently editing.

        // Get new profile name
        if (RFC2544_web_profile_name_get(p, "cmn_name", profile.common.name, TRUE) != VTSS_RC_OK) {
            errors++;
        }

        if (profile.common.name[0] == '\0') {
            // Can't reach this if the JavaScript code behaves.
            T_E("This should be checked by JS");
            errors++;
        }

        T_D("POST: old_profile_name = %s. new_profile_name = %s", profile_name, profile.common.name);

        if ((dscr_ptr = cyg_httpd_form_varable_string(p, "cmn_dscr", &dscr_len)) != NULL) {
            if (!cgi_unescape(dscr_ptr, profile.common.dscr, dscr_len, sizeof(profile.common.dscr))) {
                errors++;
            }
        } else {
            errors++;
        }

        errors += RFC2544_web_u32_get(p, "cmn_port", &profile.common.egress_port_no);
        profile.common.egress_port_no = uport2iport(profile.common.egress_port_no);
        errors += RFC2544_web_u32_get(p, "cmn_mel",  &profile.common.meg_level);

        if (!cyg_httpd_form_variable_mac(p, "cmn_dmac", &profile.common.dmac)) {
            errors++;
        }

        for (cnt = 0; cnt < ARRSZ(profile.common.selected_frame_sizes); cnt++) {
            profile.common.selected_frame_sizes[cnt] = cyg_httpd_form_variable_check_fmt(p, "frm_sz_%d", cnt);
        }

        errors += RFC2544_web_u32_get(p, "cmn_dwell", &profile.common.dwell_time_secs);

        profile.common.sequence_number_check = cyg_httpd_form_varable_find(p, "cmn_seq") ? TRUE : FALSE; // Returns non-NULL if checked

        // Test types (a bit mask)
        profile.common.selected_tests  = cyg_httpd_form_varable_find(p, "cmn_tp") ? RFC2544_TEST_TYPE_THROUGHPUT   : 0;
        profile.common.selected_tests |= cyg_httpd_form_varable_find(p, "cmn_la") ? RFC2544_TEST_TYPE_LATENCY      : 0;
        profile.common.selected_tests |= cyg_httpd_form_varable_find(p, "cmn_fl") ? RFC2544_TEST_TYPE_FRAME_LOSS   : 0;
        profile.common.selected_tests |= cyg_httpd_form_varable_find(p, "cmn_bb") ? RFC2544_TEST_TYPE_BACK_TO_BACK : 0;

        // Port or VLAN Down-MEP?
        if (cyg_httpd_form_varable_long_int(p, "cmn_type", &val_long)) {
            profile.common.vlan_tag.vid = val_long;
            if (val_long) {
                // It's a VLAN down-mep. Get VID, PCP, and DEI.
                errors += RFC2544_web_u32_get(p, "cmn_vid", &val_u32);
                profile.common.vlan_tag.vid = val_u32;

                errors += RFC2544_web_u32_get(p, "cmn_pcp", &val_u32);
                profile.common.vlan_tag.pcp = val_u32;

                errors += RFC2544_web_u32_get(p, "cmn_dei", &val_u32);
                profile.common.vlan_tag.dei = val_u32;
            }
        } else {
            errors++;
        }

        if (profile.common.selected_tests & RFC2544_TEST_TYPE_THROUGHPUT) {
            errors += RFC2544_web_u32_get(p, "tp_dur",  &profile.throughput.trial_duration_secs);
            errors += RFC2544_web_u32_get(p, "tp_min",  &profile.throughput.rate_min_permille);
            errors += RFC2544_web_u32_get(p, "tp_max",  &profile.throughput.rate_max_permille);
            errors += RFC2544_web_u32_get(p, "tp_step", &profile.throughput.rate_step_permille);
            errors += RFC2544_web_u32_get(p, "tp_pass", &profile.throughput.pass_criterion_permille);
        }

        if (profile.common.selected_tests & RFC2544_TEST_TYPE_LATENCY) {
            errors += RFC2544_web_u32_get(p, "la_dur",  &profile.latency.trial_duration_secs);
            errors += RFC2544_web_u32_get(p, "la_dmm",  &profile.latency.dmm_interval_secs);
            errors += RFC2544_web_u32_get(p, "la_pass", &profile.latency.pass_criterion_permille);
        }

        if (profile.common.selected_tests & RFC2544_TEST_TYPE_FRAME_LOSS) {
            errors += RFC2544_web_u32_get(p, "fl_dur",  &profile.frame_loss.trial_duration_secs);
            errors += RFC2544_web_u32_get(p, "fl_min",  &profile.frame_loss.rate_min_permille);
            errors += RFC2544_web_u32_get(p, "fl_max",  &profile.frame_loss.rate_max_permille);
            errors += RFC2544_web_u32_get(p, "fl_step", &profile.frame_loss.rate_step_permille);
        }

        if (profile.common.selected_tests & RFC2544_TEST_TYPE_BACK_TO_BACK) {
            errors += RFC2544_web_u32_get(p, "bb_dur", &profile.back_to_back.trial_duration_msecs);
            errors += RFC2544_web_u32_get(p, "bb_cnt", &profile.back_to_back.trial_cnt);
        }

        RFC2544_web_saved_profile.common.name[0] = '\0'; // Prevent this from being used in subsequent GET.
        RFC2544_web_saved_profile_edit_name[0] = '\0';

        if (errors) {
            // An internal error that needs to be fixed.
            T_E("Invalid form");
            RFC2544_web_err_msg = "Internal Error: Invalid form";
        } else if ((rc = rfc2544_mgmt_profile_set(profile_name, &profile)) != VTSS_RC_OK) {
            // Something went wrong when applying the new profile.
            // Stay on this page if and only if the return code is RFC2544_ERROR_PROFILE_ALREADY_EXISTS,
            // because that means that the user has renamed a profile to a profile
            // that already exists, which is illegal.
            // If the return code is e.g. RFC2544_ERROR_OUT_OF_PROFILES, we redirect to the overview
            // page with a cross-error.
            // The same goes if the return code is RFC2544_ERROR_NO_SUCH_PROFILE, which indicates
            // that the profile we're editing has been removed behind our backs.
            if (rc == RFC2544_ERROR_PROFILE_ALREADY_EXISTS) {
                RFC2544_web_saved_profile = profile; // Send the profile back to the browser.
                strcpy(RFC2544_web_saved_profile_edit_name, profile_name);
                RFC2544_web_err_msg = error_txt(rc);
                errors++; // Make the redirect to ourselves below active.
            } else {
                RFC2544_web_cross_err_msg = error_txt(rc);
            }
        }

        if (errors) {
            // We have to redirect to ourselves with the old profile name as argument.
            i8 url[RFC2544_PROFILE_NAME_LEN + 100];
            sprintf(url, "/rfc2544_profile_edit.htm?profile=%s", profile_name);
            redirect(p, url);
        } else {
            // Everything went well. Go back to the profile overview page.
            redirect(p, "/rfc2544_profiles.htm");
        }
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */

        // The URL may contain a profile name to get. If it doesn't, we attempt to create a new.
        if (RFC2544_web_profile_name_get(p, "profile", profile_name, FALSE) != VTSS_RC_OK) {
            profile_name[0] = '\0';
            T_D("GET: No URL profile name");
        } else {
            T_D("GET: URL profile_name = %s", profile_name);
        }

        if (RFC2544_web_saved_profile.common.name[0] != '\0') {
            // An error occurred just before.

            T_D("GET-error: edit_profile_name = %s. profile profile_name = %s", RFC2544_web_saved_profile_edit_name, RFC2544_web_saved_profile.common.name);

            profile = RFC2544_web_saved_profile;
            RFC2544_web_saved_profile.common.name[0] = '\0';

            (void)cgi_escape(RFC2544_web_saved_profile_edit_name, encoded_profile_edit_name);
            RFC2544_web_saved_profile_edit_name[0] = '\0';
        } else {
            RFC2544_web_saved_profile_edit_name[0] = '\0';

            // This will create a new default profile if profile_name is empty, or attempt to
            // get the existing profile if it's not.
            if ((rc = rfc2544_mgmt_profile_get(profile_name, &profile)) != VTSS_RC_OK) {
                // Couldn't get existing or create a new. Redirect to overview page.
                RFC2544_web_err_msg = "";
                RFC2544_web_cross_err_msg = error_txt(rc);
                redirect(p, "/rfc2544_profiles.htm");
                return -1;
            }

            (void)cgi_escape(profile_name, encoded_profile_edit_name);

            T_D("GET: edit_profile_name = %s. profile profile_name = %s", profile_name, profile.common.name);
        }

        // Time to fill in the reply
        cyg_httpd_start_chunked("html");

        // Format: err_msg#[Common]#[FrameSizes]#[Throughput]#[Latency]#[FrameLoss]#[BackToBack]
        //         [Common]     = edit_profile_name/name/dscr/vlan_id/vlan_pcp/vlan_dei/egress_port_no/meg_level/dmac/dwell_time/seq_number_check/sel_tests_mask
        //         [FrameSizes] = frame_size_1/frame_size_sel_1/frame_size_2/frame_size_sel_2/.../frame_size_N/frame_size_sel_N
        //         [Throughput] = trial_duration/rate_min/rate_max/rate_step/pass_criterion
        //         [Latency]    = trial_duration/dmm_interval/pass_criterion
        //         [FrameLoss]  = trial_duration/rate_min/rate_max/rate_step
        //         [BackToBack] = trial_duration/trial_cnt

        (void)cgi_escape(profile.common.name, encoded_profile_name);
        (void)cgi_escape(profile.common.dscr, encoded_dscr);
        mac = &profile.common.dmac.addr[0];
        sprintf(mac_str, "%02x-%02x-%02x-%02x-%02x-%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s#%s/%s/%s/%u/%u/%u/%u/%u/%s/%u/%u/%u#",
                       RFC2544_web_err_msg,
                       encoded_profile_edit_name,
                       encoded_profile_name,
                       encoded_dscr,
                       profile.common.vlan_tag.vid,
                       profile.common.vlan_tag.pcp,
                       profile.common.vlan_tag.dei,
                       iport2uport(profile.common.egress_port_no),
                       profile.common.meg_level,
                       mac_str,
                       profile.common.dwell_time_secs,
                       profile.common.sequence_number_check,
                       profile.common.selected_tests);

        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
        RFC2544_web_err_msg = "";

        cnt = 0;
        for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
            cnt += snprintf(p->outbuffer + cnt, sizeof(p->outbuffer) - cnt, "%s%u/%d",
                            fs == 0 ? "" : "/",
                            rfc2544_mgmt_util_frame_size_enum_to_number(fs),
                            profile.common.selected_frame_sizes[fs]);
        }

        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%u/%u/%u/%u/%u#%u/%u/%u#%u/%u/%u/%u#%u/%u",
                       profile.throughput.trial_duration_secs,
                       profile.throughput.rate_min_permille,
                       profile.throughput.rate_max_permille,
                       profile.throughput.rate_step_permille,
                       profile.throughput.pass_criterion_permille,
                       profile.latency.trial_duration_secs,
                       profile.latency.dmm_interval_secs,
                       profile.latency.pass_criterion_permille,
                       profile.frame_loss.trial_duration_secs,
                       profile.frame_loss.rate_min_permille,
                       profile.frame_loss.rate_max_permille,
                       profile.frame_loss.rate_step_permille,
                       profile.back_to_back.trial_duration_msecs,
                       profile.back_to_back.trial_cnt);
        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);


        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/******************************************************************************/
// RFC2544_web_handler_reports()
/******************************************************************************/
static cyg_int32 RFC2544_web_handler_reports(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    i8                    report_name[RFC2544_REPORT_NAME_LEN];
    rfc2544_report_info_t info;
    vtss_rc               rc = VTSS_RC_OK;
    int                   cnt;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_RFC2544)) {
        return -1;
    }
#endif

    // This function is also used to delete a report, stop an ongoing test suite, and saving a report.
    //  - Delete report:   URL contains "report=<report_name>&clear=1"  THIS IS A CYG_HTTPD_METHOD_GET
    //  - Stop test suite: URL contains "report=<report_name>&stop=1"   THIS IS A CYG_HTTPD_METHOD_GET
    //  - Save report:     URL contains "report=<report_name>"          THIS IS A CYG_HTTPD_METHOD_POST
    if ((rc = RFC2544_web_report_name_get(p, "report", report_name, FALSE)) == VTSS_RC_OK && report_name[0] != '\0') {
        int clear, stop;

        // Called with a report name. Check to see if we're going to stop or delete the report.
        if (cyg_httpd_form_varable_int(p, "stop", &stop) && stop == 1) {
            // Stop execution.
            rc = rfc2544_mgmt_test_stop(report_name);
        } else if (cyg_httpd_form_varable_int(p, "clear", &clear) && clear == 1) {
            // Delete report.
            rc = rfc2544_mgmt_report_delete(report_name);
        } else {
            // Save report to file.
            i8 *report_as_txt;

            if ((rc = rfc2544_mgmt_report_as_txt(report_name, &report_as_txt)) == VTSS_RC_OK) {
                cyg_httpd_ires_table_entry entry;
                i8 report_dot_txt[RFC2544_REPORT_NAME_LEN + 4];

                (void)sprintf(report_dot_txt, "%s.txt", report_name);
                entry.f_pname = (char *)report_dot_txt;
                entry.f_ptr   = (unsigned char *)report_as_txt;
                entry.f_size  = strlen(report_as_txt);
                cyg_httpd_send_content_disposition(&entry);
                VTSS_FREE(report_as_txt);
                return -1;
            }
        }
    }

    // Time to return all current profiles.
    (void)cyg_httpd_start_chunked("html");

    // Format: err_msg#[Reports]
    //         [Reports] = [Report1]#[Report2]#...#[ReportN]
    //         [Report]  = report_name/description/creation_timestamp/status
    // 'status' is enumerated in #status_as_str

    if (rc != VTSS_RC_OK || RFC2544_web_cross_err_msg[0] != '\0') {
        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", rc != VTSS_RC_OK ? error_txt(rc) : RFC2544_web_cross_err_msg);
        RFC2544_web_cross_err_msg = "";
        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
    }

    info.name[0] = '\0';
    while (1) {
        i8 encoded_report_name[3 * RFC2544_REPORT_NAME_LEN];
        i8 encoded_dscr[3 * RFC2544_REPORT_DSCR_LEN];

        if (rfc2544_mgmt_report_info_get(&info) != VTSS_RC_OK) {
            T_E("How did I get here?");
            break;
        }
        if (info.name[0] == '\0') {
            // No more reports.
            break;
        }

        (void)cgi_escape(info.name, encoded_report_name);
        (void)cgi_escape(info.dscr, encoded_dscr);

        cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%s/%s/%s/%u", encoded_report_name, encoded_dscr, info.creation_time, info.status);
        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
    }

    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/******************************************************************************/
// RFC2544_web_handler_test_start()
/******************************************************************************/
static cyg_int32 RFC2544_web_handler_test_start(CYG_HTTPD_STATE *p)
{
    // Well, RFC2544_web_err_msg is indeed not protected, but I find it very unlikely
    // that the error message from one Web-browser instance gets forwarded to another
    // Web-browser instance.
    /*lint -esym(459,RFC2544_web_err_msg)*/
    /*lint -esym(459,RFC2544_web_saved_name)*/
    /*lint -esym(459,RFC2544_web_saved_dscr)*/
    /*lint -esym(459,RFC2544_web_saved_prof)*/
    static const char *RFC2544_web_err_msg = "";
    static i8         RFC2544_web_saved_name[RFC2544_REPORT_NAME_LEN];
    static i8         RFC2544_web_saved_dscr[RFC2544_REPORT_DSCR_LEN];
    static i8         RFC2544_web_saved_prof[RFC2544_PROFILE_NAME_LEN];
    vtss_isid_t       isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_rc           rc = VTSS_RC_OK;
    int               cnt, errors = 0;
    size_t            dscr_len;
    const i8          *dscr_ptr;
    BOOL              stay_on_page = FALSE, first = TRUE;
    i8                report_name[RFC2544_REPORT_NAME_LEN];
    i8                report_dscr[RFC2544_REPORT_DSCR_LEN];
    i8                profile_name[RFC2544_PROFILE_NAME_LEN];
    i8                encoded_profile_name[3 * RFC2544_PROFILE_NAME_LEN];

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_RFC2544)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        // Get report name, description, and profile name and start the test.
        if (RFC2544_web_report_name_get(p, "report_name", report_name, TRUE) != VTSS_RC_OK) {
            errors++;
        }

        if (report_name[0] == '\0') {
            // Can't reach this if the JavaScript code behaves.
            T_E("This should be checked by JS");
            errors++;
        }

        if (RFC2544_web_profile_name_get(p, "report_profile", profile_name, TRUE) != VTSS_RC_OK) {
            errors++;
        }

        if (profile_name[0] == '\0') {
            // Can't reach this if the JavaScript code behaves.
            T_E("This should be checked by JS");
            errors++;
        }

        report_dscr[0] = '\0';
        if ((dscr_ptr = cyg_httpd_form_varable_string(p, "report_dscr", &dscr_len)) != NULL) {
            if (!cgi_unescape(dscr_ptr, report_dscr, dscr_len, sizeof(report_dscr))) {
                errors++;
            }
        } else {
            errors++;
        }

        if (!errors && (rc = rfc2544_mgmt_test_start(profile_name, report_name, report_dscr)) != VTSS_RC_OK) {
            if (rc == RFC2544_ERROR_REPORT_ALREADY_EXISTS) {
                stay_on_page = TRUE;
            }
            errors++;
        }

        RFC2544_web_cross_err_msg = "";
        RFC2544_web_err_msg = "";

        if (rc == VTSS_RC_OK && errors) {
            RFC2544_web_cross_err_msg = "Internal Error: Invalid form";
        } else if (rc != VTSS_RC_OK) {
            if (stay_on_page) {
                RFC2544_web_err_msg = error_txt(rc);
                strcpy(RFC2544_web_saved_name, report_name);
                strcpy(RFC2544_web_saved_dscr, report_dscr);
                strcpy(RFC2544_web_saved_prof, profile_name);
            } else {
                RFC2544_web_cross_err_msg = error_txt(rc);
            }
        }
        if (stay_on_page) {
            redirect(p, "/rfc2544_test_start.htm");
        } else {
            redirect(p, "/rfc2544_reports.htm");
        }
    } else {
        // Return all currently defined profiles.
        (void)cyg_httpd_start_chunked("html");

        // Format: [Housekeeping]#[ProfilesNames]
        //         [Housekeeping] = err_msg/saved_report_name/saved_dscr/saved_profile_name
        //         [ProfileNames] = ProfileName_1/ProfileName_2/.../ProfileName_N

        if (RFC2544_web_err_msg[0] != '\0') {
            i8 encoded_report_name[3 * RFC2544_REPORT_NAME_LEN];
            i8 encoded_dscr[3 * RFC2544_REPORT_DSCR_LEN];

            (void)cgi_escape(RFC2544_web_saved_name, encoded_report_name);
            (void)cgi_escape(RFC2544_web_saved_dscr, encoded_dscr);
            (void)cgi_escape(RFC2544_web_saved_prof, encoded_profile_name);

            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%s/%s/%s", RFC2544_web_err_msg, encoded_report_name, encoded_dscr, encoded_profile_name);
            (void)cyg_httpd_write_chunked(p->outbuffer, cnt);

            RFC2544_web_err_msg = "";
        }

        (void)cyg_httpd_write_chunked("#", 1);

        profile_name[0] = '\0';
        while (1) {
            if (rfc2544_mgmt_profile_names_get(profile_name) != VTSS_RC_OK) {
                T_E("How did I get here?");
                break;
            }
            if (profile_name[0] == '\0') {
                // No more profiles.
                break;
            }

            (void)cgi_escape(profile_name, encoded_profile_name);
            cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%s", first ? "" : "/", encoded_profile_name);
            (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
            first = FALSE;
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/******************************************************************************/
// RFC2544_web_handler_report_view()
/******************************************************************************/
static cyg_int32 RFC2544_web_handler_report_view(CYG_HTTPD_STATE *p)
{
    vtss_isid_t isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_rc     rc = VTSS_RC_OK;
    int         cnt, errors = 0;
    i8          *report_as_txt;
    i8          report_name[RFC2544_REPORT_NAME_LEN];

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_RFC2544)) {
        return -1;
    }
#endif

    if (p->method != CYG_HTTPD_METHOD_GET) {
        T_E("Only GET method is supported");
        return -1;
    }

    // Get report name to get
    if (RFC2544_web_report_name_get(p, "report", report_name, TRUE) != VTSS_RC_OK) {
        errors++;
    }

    if (report_name[0] == '\0') {
        // Can't reach this if the JavaScript code behaves.
        T_E("This should be checked by JS");
        errors++;
    }

    if (!errors && (rc = rfc2544_mgmt_report_as_txt(report_name, &report_as_txt)) == VTSS_RC_OK) {
        u32 report_len = strlen(report_as_txt), act_written = 0;

        // Return all currently defined profiles.
        (void)cyg_httpd_start_chunked("html");

        while (report_len > 0) {
            // Gotta figure out how much data we can write per time to the outbuffer.
            // Since encoding requires up to 3 times the input string length, we can
            // at most write 1/3 of the outbuffer. The cgi_escape_n() function also
            // writes a terminating NULL char.
            u32 maxlen = report_len < (sizeof(p->outbuffer) / 3) - 1 ? report_len : (sizeof(p->outbuffer) / 3) - 1; // Max number of unencoded bytes to take from report_as_txt.
            cnt = cgi_escape_n(&report_as_txt[act_written], p->outbuffer, maxlen);
            (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
            report_len  -= maxlen;
            act_written += maxlen;
        }

        (void)cyg_httpd_end_chunked();
        VTSS_FREE(report_as_txt);
    }

    if (rc != VTSS_RC_OK || errors) {
        // If someone has deleted the report in the meanwhile, send an empty.
        const char *errstr;

        if (rc == VTSS_RC_OK) {
            errstr = "Internal Error: Invalid form";
            T_E("%s", errstr);
        } else {
            errstr = error_txt(rc);
        }

        T_W("%s", errstr);
        (void)cyg_httpd_start_chunked("html");
        cnt = cgi_escape(errstr, p->outbuffer);
        (void)cyg_httpd_write_chunked(p->outbuffer, cnt);
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
// rfc2544_lib_config_js()
//  Module JS lib config routine
/****************************************************************************/
static size_t rfc2544_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
#define RFC2544_WEB_BUF_LEN 1500
    char buf[RFC2544_WEB_BUF_LEN];

    buf[RFC2544_WEB_BUF_LEN - 1] = '\0';

    (void)snprintf(buf, RFC2544_WEB_BUF_LEN - 1,
                   "var rfc2544_profile_cnt = %d;\n"
                   "var rfc2544_report_cnt = %d;\n"
                   "var rfc2544_dwell_min = %d;\n"
                   "var rfc2544_dwell_max = %d;\n"
                   "var rfc2544_dwell_def = %d;\n"
                   "var rfc2544_tp_dur_min = %d;\n"
                   "var rfc2544_tp_dur_max = %d;\n"
                   "var rfc2544_tp_dur_def = %d;\n"
                   "var rfc2544_tp_min_min = %d;\n"
                   "var rfc2544_tp_min_max = %d;\n"
                   "var rfc2544_tp_min_def = %d;\n"
                   "var rfc2544_tp_max_min = %d;\n"
                   "var rfc2544_tp_max_max = %d;\n"
                   "var rfc2544_tp_max_def = %d;\n"
                   "var rfc2544_tp_step_min = %d;\n"
                   "var rfc2544_tp_step_max = %d;\n"
                   "var rfc2544_tp_step_def = %d;\n"
                   "var rfc2544_tp_pass_min = %d;\n"
                   "var rfc2544_tp_pass_max = %d;\n"
                   "var rfc2544_tp_pass_def = %d;\n"
                   "var rfc2544_la_dur_min = %d;\n"
                   "var rfc2544_la_dur_max = %d;\n"
                   "var rfc2544_la_dur_def = %d;\n"
                   "var rfc2544_la_dmm_min = %d;\n"
                   "var rfc2544_la_dmm_max = %d;\n"
                   "var rfc2544_la_dmm_def = %d;\n"
                   "var rfc2544_la_pass_min = %d;\n"
                   "var rfc2544_la_pass_max = %d;\n"
                   "var rfc2544_la_pass_def = %d;\n"
                   "var rfc2544_fl_dur_min = %d;\n"
                   "var rfc2544_fl_dur_max = %d;\n"
                   "var rfc2544_fl_dur_def = %d;\n"
                   "var rfc2544_fl_min_min = %d;\n"
                   "var rfc2544_fl_min_max = %d;\n"
                   "var rfc2544_fl_min_def = %d;\n"
                   "var rfc2544_fl_max_min = %d;\n"
                   "var rfc2544_fl_max_max = %d;\n"
                   "var rfc2544_fl_max_def = %d;\n"
                   "var rfc2544_fl_step_min = %d;\n"
                   "var rfc2544_fl_step_max = %d;\n"
                   "var rfc2544_fl_step_def = %d;\n"
                   "var rfc2544_bb_dur_min = %d;\n"
                   "var rfc2544_bb_dur_max = %d;\n"
                   "var rfc2544_bb_dur_def = %d;\n"
                   "var rfc2544_bb_cnt_min = %d;\n"
                   "var rfc2544_bb_cnt_max = %d;\n"
                   "var rfc2544_bb_cnt_def = %d;\n",
                   RFC2544_PROFILE_CNT,
                   RFC2544_REPORT_CNT,
                   RFC2544_COMMON_DWELL_TIME_MIN,
                   RFC2544_COMMON_DWELL_TIME_MAX,
                   RFC2544_COMMON_DWELL_TIME_DEFAULT,
                   RFC2544_THROUGHPUT_TRIAL_DURATION_MIN,
                   RFC2544_THROUGHPUT_TRIAL_DURATION_MAX,
                   RFC2544_THROUGHPUT_TRIAL_DURATION_DEFAULT,
                   RFC2544_THROUGHPUT_RATE_MIN_MIN,
                   RFC2544_THROUGHPUT_RATE_MIN_MAX,
                   RFC2544_THROUGHPUT_RATE_MIN_DEFAULT,
                   RFC2544_THROUGHPUT_RATE_MAX_MIN,
                   RFC2544_THROUGHPUT_RATE_MAX_MAX,
                   RFC2544_THROUGHPUT_RATE_MAX_DEFAULT,
                   RFC2544_THROUGHPUT_RATE_STEP_MIN,
                   RFC2544_THROUGHPUT_RATE_STEP_MAX,
                   RFC2544_THROUGHPUT_RATE_STEP_DEFAULT,
                   RFC2544_THROUGHPUT_PASS_CRITERION_MIN,
                   RFC2544_THROUGHPUT_PASS_CRITERION_MAX,
                   RFC2544_THROUGHPUT_PASS_CRITERION_DEFAULT,
                   RFC2544_LATENCY_TRIAL_DURATION_MIN,
                   RFC2544_LATENCY_TRIAL_DURATION_MAX,
                   RFC2544_LATENCY_TRIAL_DURATION_DEFAULT,
                   RFC2544_LATENCY_DMM_INTERVAL_MIN,
                   RFC2544_LATENCY_DMM_INTERVAL_MAX,
                   RFC2544_LATENCY_DMM_INTERVAL_DEFAULT,
                   RFC2544_LATENCY_PASS_CRITERION_MIN,
                   RFC2544_LATENCY_PASS_CRITERION_MAX,
                   RFC2544_LATENCY_PASS_CRITERION_DEFAULT,
                   RFC2544_FRAME_LOSS_TRIAL_DURATION_MIN,
                   RFC2544_FRAME_LOSS_TRIAL_DURATION_MAX,
                   RFC2544_FRAME_LOSS_TRIAL_DURATION_DEFAULT,
                   RFC2544_FRAME_LOSS_RATE_MIN_MIN,
                   RFC2544_FRAME_LOSS_RATE_MIN_MAX,
                   RFC2544_FRAME_LOSS_RATE_MIN_DEFAULT,
                   RFC2544_FRAME_LOSS_RATE_MAX_MIN,
                   RFC2544_FRAME_LOSS_RATE_MAX_MAX,
                   RFC2544_FRAME_LOSS_RATE_MAX_DEFAULT,
                   RFC2544_FRAME_LOSS_RATE_STEP_MIN,
                   RFC2544_FRAME_LOSS_RATE_STEP_MAX,
                   RFC2544_FRAME_LOSS_RATE_STEP_DEFAULT,
                   RFC2544_BACK_TO_BACK_TRIAL_DURATION_MIN,
                   RFC2544_BACK_TO_BACK_TRIAL_DURATION_MAX,
                   RFC2544_BACK_TO_BACK_TRIAL_DURATION_DEFAULT,
                   RFC2544_BACK_TO_BACK_TRIAL_CNT_MIN,
                   RFC2544_BACK_TO_BACK_TRIAL_CNT_MAX,
                   RFC2544_BACK_TO_BACK_TRIAL_CNT_DEFAULT);

    if (strlen(buf) == RFC2544_WEB_BUF_LEN - 1) {
        T_E("Increase size of RFC2544_WEB_BUF_LEN");
    }

#undef RFC2544_WEB_BUF_LEN
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buf);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/
web_lib_config_js_tab_entry(rfc2544_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_rfc2544_profiles,     "/config/rfc2544_profiles",     RFC2544_web_handler_profiles);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_rfc2544_profile_edit, "/config/rfc2544_profile_edit", RFC2544_web_handler_profile_edit);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_rfc2544_reports,      "/config/rfc2544_reports",      RFC2544_web_handler_reports);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_rfc2544_test_start,   "/config/rfc2544_test_start",   RFC2544_web_handler_test_start);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_rfc2544_report_view,  "/stat/rfc2544_report_view",    RFC2544_web_handler_report_view);

