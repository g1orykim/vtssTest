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

#ifndef _RFC2544_API_H_
#define _RFC2544_API_H_

#include "vtss_api.h" /* for various vtss_XXX types    */
#include "main.h"     /* For vtss_init_data_t          */
#include "misc_api.h" /* For MISC_RFC3339_TIME_STR_LEN */

#define RFC2544_PROFILE_NAME_LEN            33 /**< Maximum length (including null-termination) of profile name string.        */
#define RFC2544_PROFILE_DSCR_LEN           129 /**< Maximum length (including null-termination) of profile description string. */
#define RFC2544_REPORT_NAME_LEN             33 /**< Maximum length (including null-termination) of test report name.           */
#define RFC2544_REPORT_DSCR_LEN            129 /**< Maximum length (including null-termination) of report description string.  */
#define RFC2544_PROFILE_CNT                 16 /**< Number of profiles.                                                        */
#define RFC2544_REPORT_CNT                  10 /**< Number of test reports.                                                    */
// Don't set this to a number greater than 1, unless the code is updated to also check
// that the same profile is not executed twice, which is a bit more cumbersome, because a
// copy of the profile is taken when the report is created.
// Even two different profiles may use the same ports/VLANs and other parameters, which should also be checked in that case.
#define RFC2544_CONCURRENTLY_EXECUTING_MAX   1 /**< Maximum number of concurrently executing profiles.                         */

#define RFC2544_COMMON_DWELL_TIME_MIN                   1 /**< seconds     */
#define RFC2544_COMMON_DWELL_TIME_MAX                  10 /**< seconds     */
#define RFC2544_COMMON_DWELL_TIME_DEFAULT               2 /**< seconds     */

#define RFC2544_COMMON_MEG_LEVEL_MIN                    0 /**< <no units>  */
#define RFC2544_COMMON_MEG_LEVEL_MAX                    7 /**< <no units>  */
#define RFC2544_COMMON_MEG_LEVEL_DEFAULT                7 /**< <no units>  */

#define RFC2544_THROUGHPUT_TRIAL_DURATION_MIN           1 /**< second      */
#define RFC2544_THROUGHPUT_TRIAL_DURATION_MAX        1800 /**< seconds     */
#define RFC2544_THROUGHPUT_TRIAL_DURATION_DEFAULT      60 /**< seconds     */

#define RFC2544_THROUGHPUT_RATE_MIN_MIN                 1 /**< permille    */
#define RFC2544_THROUGHPUT_RATE_MIN_MAX              1000 /**< permille    */
#define RFC2544_THROUGHPUT_RATE_MIN_DEFAULT           800 /**< permille    */

#define RFC2544_THROUGHPUT_RATE_MAX_MIN                 1 /**< permille    */
#define RFC2544_THROUGHPUT_RATE_MAX_MAX              1000 /**< permille    */
#define RFC2544_THROUGHPUT_RATE_MAX_DEFAULT          1000 /**< permille    */

#define RFC2544_THROUGHPUT_RATE_STEP_MIN                1 /**< permille    */
#define RFC2544_THROUGHPUT_RATE_STEP_MAX             1000 /**< permille    */
#define RFC2544_THROUGHPUT_RATE_STEP_DEFAULT            2 /**< permille    */

#define RFC2544_THROUGHPUT_PASS_CRITERION_MIN           0 /**< permille    */
#define RFC2544_THROUGHPUT_PASS_CRITERION_MAX         100 /**< permille    */
#define RFC2544_THROUGHPUT_PASS_CRITERION_DEFAULT       0 /**< permille    */

#define RFC2544_LATENCY_TRIAL_DURATION_MIN             10 /**< seconds     */
#define RFC2544_LATENCY_TRIAL_DURATION_MAX           1800 /**< seconds     */
#define RFC2544_LATENCY_TRIAL_DURATION_DEFAULT        120 /**< seconds     */

#define RFC2544_LATENCY_DMM_INTERVAL_MIN                1 /**< seconds     */
#define RFC2544_LATENCY_DMM_INTERVAL_MAX               60 /**< seconds     */
#define RFC2544_LATENCY_DMM_INTERVAL_DEFAULT           10 /**< seconds     */

#define RFC2544_LATENCY_PASS_CRITERION_MIN              0 /**< permille    */
#define RFC2544_LATENCY_PASS_CRITERION_MAX            100 /**< permille    */
#define RFC2544_LATENCY_PASS_CRITERION_DEFAULT          0 /**< permille    */

#define RFC2544_FRAME_LOSS_TRIAL_DURATION_MIN           1 /**< seconds     */
#define RFC2544_FRAME_LOSS_TRIAL_DURATION_MAX        1800 /**< seconds     */
#define RFC2544_FRAME_LOSS_TRIAL_DURATION_DEFAULT      60 /**< seconds     */

#define RFC2544_FRAME_LOSS_RATE_MIN_MIN                 1 /**< permille    */
#define RFC2544_FRAME_LOSS_RATE_MIN_MAX              1000 /**< permille    */
#define RFC2544_FRAME_LOSS_RATE_MIN_DEFAULT           800 /**< permille    */

#define RFC2544_FRAME_LOSS_RATE_MAX_MIN                 1 /**< permille    */
#define RFC2544_FRAME_LOSS_RATE_MAX_MAX              1000 /**< permille    */
#define RFC2544_FRAME_LOSS_RATE_MAX_DEFAULT          1000 /**< permille    */

#define RFC2544_FRAME_LOSS_RATE_STEP_MIN                1 /**< permille    */
#define RFC2544_FRAME_LOSS_RATE_STEP_MAX             1000 /**< permille    */
#define RFC2544_FRAME_LOSS_RATE_STEP_DEFAULT            5 /**< permille     */

#define RFC2544_BACK_TO_BACK_TRIAL_DURATION_MIN       100 /**< milliseconds */
#define RFC2544_BACK_TO_BACK_TRIAL_DURATION_MAX     10000 /**< milliseconds */
#define RFC2544_BACK_TO_BACK_TRIAL_DURATION_DEFAULT  2000 /**< milliseconds */

#define RFC2544_BACK_TO_BACK_TRIAL_CNT_MIN              1 /**< times        */
#define RFC2544_BACK_TO_BACK_TRIAL_CNT_MAX            100 /**< times        */
#define RFC2544_BACK_TO_BACK_TRIAL_CNT_DEFAULT         50 /**< times        */

/**
 * Definition of error return code.
 * See also rfc2544_error_txt() in rfc2544.c
 */
enum {
    RFC2544_ERROR_EGRESS_PORT = MODULE_ERROR_START(VTSS_MODULE_ID_RFC2544),
    RFC2544_ERROR_NO_SUCH_PROFILE,
    RFC2544_ERROR_PROFILE_ALREADY_EXISTS,
    RFC2544_ERROR_OUT_OF_PROFILES,
    RFC2544_ERROR_PROFILE_NAME_TOO_LONG,
    RFC2544_ERROR_PROFILE_NAME,
    RFC2544_ERROR_COMMON_DWELL_TIME,
    RFC2544_ERROR_COMMON_MEG_LEVEL,
    RFC2544_ERROR_COMMON_DMAC,
    RFC2544_ERROR_COMMON_FRAME_SIZE,
    RFC2544_ERROR_COMMON_SELECTED_TESTS,
    RFC2544_ERROR_COMMON_THROUGHPUT_WHEN_LATENCY,
    RFC2544_ERROR_THROUGHPUT_TRIAL_DURATION,
    RFC2544_ERROR_THROUGHPUT_RATE_MIN,
    RFC2544_ERROR_THROUGHPUT_RATE_MAX,
    RFC2544_ERROR_THROUGHPUT_RATE_STEP,
    RFC2544_ERROR_THROUGHPUT_RATE_MIN_MAX,
    RFC2544_ERROR_THROUGHPUT_PASS_CRITERION,
    RFC2544_ERROR_LATENCY_TRIAL_DURATION,
    RFC2544_ERROR_LATENCY_DMM_INTERVAL,
    RFC2544_ERROR_LATENCY_PASS_CRITERION,
    RFC2544_ERROR_LATENCY_TRIAL_DURATION_DMM_INTERVAL,
    RFC2544_ERROR_FRAME_LOSS_TRIAL_DURATION,
    RFC2544_ERROR_FRAME_LOSS_RATE_MIN,
    RFC2544_ERROR_FRAME_LOSS_RATE_MAX,
    RFC2544_ERROR_FRAME_LOSS_RATE_STEP,
    RFC2544_ERROR_FRAME_LOSS_RATE_MIN_MAX,
    RFC2544_ERROR_FRAME_LOSS_STEP_VS_MAX_MINUS_MIN,
    RFC2544_ERROR_BACK_TO_BACK_TRIAL_DURATION,
    RFC2544_ERROR_BACK_TO_BACK_TRIAL_CNT,
    RFC2544_ERROR_REPORT_NAME_TOO_LONG,
    RFC2544_ERROR_REPORT_NAME,
    RFC2544_ERROR_NO_SUCH_REPORT,
    RFC2544_ERROR_REPORT_ALREADY_EXISTS,
    RFC2544_ERROR_TOO_MANY_EXECUTING,
    RFC2544_ERROR_NOT_EXECUTING,
    RFC2544_ERROR_REPORT_DELETE_EXECUTING_OR_CANCELLING,
    RFC2544_ERROR_OUT_OF_MEMORY,
    RFC2544_ERROR_REPORT_LINE_TOO_LONG,
    RFC2544_ERROR_NO_LINK,
    RFC2544_ERROR_OUT_OF_MEPS,
    RFC2544_ERROR_TST_RX_HIGHER_THAN_TX,
    RFC2544_ERROR_TST_NO_FRAMES_SENT,
    RFC2544_ERROR_TST_OUT_OF_ORDER,
    RFC2544_ERROR_TST_ALLOWED_LOSS_EXCEEDED,
    RFC2544_ERROR_1DM_RX_HIGHER_THAN_TX,
    RFC2544_ERROR_1DM_RX_LOWER_THAN_TX,
    RFC2544_ERROR_1DM_NO_FRAMES_SENT,
    RFC2544_ERROR_ARGUMENT,
};
char *rfc2544_error_txt(vtss_rc rc);

/**
 * Test types supported by this module.
 * The individual items are ORed together to
 * form a mask of selected tests in a given
 * profile.
 */
typedef enum {
    RFC2544_TEST_TYPE_THROUGHPUT   = 0x01, /**< Throughput test   */
    RFC2544_TEST_TYPE_LATENCY      = 0x02, /**< Latency test      */
    RFC2544_TEST_TYPE_FRAME_LOSS   = 0x04, /**< Frame loss test   */
    RFC2544_TEST_TYPE_BACK_TO_BACK = 0x08, /**< back-to-back test */
} rfc2544_test_type_t;

/**
 * Supported Frame sizes.
 */
typedef enum {
    RFC2544_FRAME_SIZE_64   = 0, /**<   64 byte frames */
    RFC2544_FRAME_SIZE_128,      /**<  128 byte frames */
    RFC2544_FRAME_SIZE_256,      /**<  256 byte frames */
    RFC2544_FRAME_SIZE_512,      /**<  512 byte frames */
    RFC2544_FRAME_SIZE_1024,     /**< 1024 byte frames */
    RFC2544_FRAME_SIZE_1280,     /**< 1280 byte frames */
    RFC2544_FRAME_SIZE_1518,     /**< 1518 byte frames */
    RFC2544_FRAME_SIZE_2000,     /**< 2000 byte frames */
    RFC2544_FRAME_SIZE_9600,     /**< 9600 byte frames */

    RFC2544_FRAME_SIZE_CNT,      /**< This must come last. */
} rfc2544_frame_size_t;

/**
 * Test report status.
 */
typedef enum {
    RFC2544_TEST_STATUS_INACTIVE,   /**< Current test has never been executed. */
    RFC2544_TEST_STATUS_EXECUTING,  /**< Test is currently executing.          */
    RFC2544_TEST_STATUS_CANCELLING, /**< Test is under cancellation.           */
    RFC2544_TEST_STATUS_CANCELLED,  /**< Test was cancelled by user.           */
    RFC2544_TEST_STATUS_PASSED,     /**< Test terminated successfully.         */
    RFC2544_TEST_STATUS_FAILED,     /**< Test failed.                          */
} rfc2544_test_status_t;

/**
 * This structure holds parameters common to all test types.
 */
typedef struct {

    /**
     * Profile name used for identification of a given profile.
     * Defaults to empty string. Two profiles can't have the same name.
     * An empty name is illegal. Only isascii() characters are allowed.
     */
    i8 name[RFC2544_PROFILE_NAME_LEN];

    /**
     * Profile description.
     */
    i8 dscr[RFC2544_PROFILE_DSCR_LEN];

    /**
     * For VLAN-based downmeps, a tag is required.
     * This composite allows for specifying PCP, DEI, and VID.
     * If vlan_tag.vid == 0, this is interpreted as a port-based profile.
     */
    vtss_vlan_tag_t vlan_tag;

    /**
     * The port on which generated traffic egresses.
     */
    vtss_port_no_t egress_port_no;

    /**
     * The MEG level (0 - 7) used in the generated OAM frames.
     */
    u32 meg_level;

    /**
     * The destination MAC address used in the generated frames.
     * The source MAC address will be determined by the egress port.
     *
     * The MEP module doesn't support the NULL MAC Address, so
     * neither can we.
     */
    vtss_mac_t dmac;

    /**
     * Selected frame sizes to run the selected test suites with.
     */
    BOOL selected_frame_sizes[RFC2544_FRAME_SIZE_CNT];

    /**
     * The time (in seconds) to wait after each trial for the system to settle before
     * acquiring counters and other statistics from hardware.
     */
    u32 dwell_time_secs;

    /**
     * When TRUE, generated OAM frames are sequence numbered, and looped
     * frames are checked for correct order upon reception.
     */
    BOOL sequence_number_check;

    /**
     * A non-zero mask of selected test types to run in
     * this profile.
     */
    rfc2544_test_type_t selected_tests;

} rfc2544_profile_common_t;

/**
 * Parameters related to througput test.
 */
typedef struct {

    /**
     * Time - in seconds - to run one trial.
     */
    u32 trial_duration_secs;

    /**
     * Minimum rate measured in permille of the line rate.
     */
    u32 rate_min_permille;

    /**
     * Maximum rate measured in permille of the line rate.
     */
    u32 rate_max_permille;

    /**
     * The step to change rate between trials, measured in permille of the line rate.
     */
    u32 rate_step_permille;

    /**
     * Allowable frame loss (in permille) for the test to be considered passed.
     */
    u32 pass_criterion_permille;

} rfc2544_profile_throughput_t;

/**
 * Parameters related to latency test.
 */
typedef struct {

    /**
     * Time - in seconds - to run one trial.
     */
    u32 trial_duration_secs;

    /**
     * Delay measurement interval, that is, the time (in seconds) between transmission
     * of a Y.1731 1DM test frame.
     */
    u32 dmm_interval_secs;

    /**
     * Allowable frame loss (in permille) for the test to be considered passed.
     */
    u32 pass_criterion_permille;

} rfc2544_profile_latency_t;

/**
 * Parameters related to frame loss test.
 */
typedef struct {

    /**
     * Time - in seconds - to run one trial.
     */
    u32 trial_duration_secs;

    /**
     * Minimum rate measured in permille of the line rate.
     */
    u32 rate_min_permille;

    /**
     * Maximum rate (start rate) measured in permille of the line rate.
     */
    u32 rate_max_permille;

    /**
     * The size with which rate is reduced per trial, measured in permille of the line rate.
     */
    u32 rate_step_permille;

} rfc2544_profile_frame_loss_t;

/**
 * Parameters related to back-to-back test.
 */
typedef struct {

    /**
     * Time - in milliseconds - to run one trial.
     */
    u32 trial_duration_msecs;

    /**
     * Number of times the trial will be executed.
     */
    u32 trial_cnt;

} rfc2544_profile_back_to_back_t;

/**
 * This structure holds parameters related to one test suite profile.
 */
typedef struct {

    /**
     * Parameters common to all test types.
     */
    rfc2544_profile_common_t common;

    /**
     * Throughput test parameters.
     */
    rfc2544_profile_throughput_t throughput;

    /**
     * Latency test parameters.
     */
    rfc2544_profile_latency_t latency;

    /**
     * Frame loss test parameters.
     */
    rfc2544_profile_frame_loss_t frame_loss;

    /**
     * back-to-back test parameters.
     */
    rfc2544_profile_back_to_back_t back_to_back;

} rfc2544_profile_t;

/**
 * This structure holds meta info about a report.
 */
typedef struct {
    /**
     * Name of report.
     */
    i8 name[RFC2544_REPORT_NAME_LEN];

    /**
     * Description of report.
     */
    i8 dscr[RFC2544_REPORT_DSCR_LEN];

    /**
     * Time of creation of this report.
     */
    i8 creation_time[MISC_RFC3339_TIME_STR_LEN];

    /**
     * Execution status of this report.
     */
    rfc2544_test_status_t status;
} rfc2544_report_info_t;

/**
 * Get names of existing profiles.
 *
 * Repeatedly call this function to get all profiles.
 * The first time, set name[0] to '\0', and leave it at its
 * previous value subsequent times.
 * As long as the function returns a non-zero length in the
 * name argument, more profiles are defined. Once the function
 * sets name[0] to '\0', the last profile is obtained.
 *
 * \param name [INOUT] See description above. Length must be at least RFC2544_PROFILE_NAME_LEN bytes.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_text() to convert to string.
 */
vtss_rc rfc2544_mgmt_profile_names_get(i8 *name);

/**
 * Get an existing profile or fill-in a default one.
 *
 * If the name argument is NULL or has length == 0, the profile argument
 * is filled with a default profile.
 *
 * If the name argument has length > 0, the profile argument is filled
 * with the corresponding profile data.
 *
 * \param name    [IN]  If NULL or has length == 0, default profile, otherwise the name of profile to retrieve.
 * \param profile [OUT] Pointer receiving profile.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_text() to convert to string.
 */
vtss_rc rfc2544_mgmt_profile_get(const i8 *const name, rfc2544_profile_t *profile);

/**
 * Set a profile.
 *
 * Use this function to create, update, or delete a profile.
 *
 * Setting profile->common.name to an empty string and old_name argument to the name of the
 * profile to delete, effectively deletes it, i.e. sets all fields to defaults.
 *
 * Setting profile->common.name to a non-empty string and old_name argument to the name of an
 * existing profile, will update the existing profile.
 *
 * Setting profile->common.name to a non-empty string and old_name to NULL or letting
 * it have length == 0, will create a new profile.
 *
 * \param old_name [IN] If updating or deleting an existing, must be a valid, existing profile name. Otherwise (creating a new profile), it must be NULL or have length == 0.
 * \param profile  [IN] Pointer to profile data.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_text() to convert to string.
 */
vtss_rc rfc2544_mgmt_profile_set(i8 *old_name, rfc2544_profile_t *profile);

/**
 * Execute a profile.
 *
 * The test report will be saved as report_name.
 * Only one profile can execute at a time. If more than one profile execution is
 * attempted started, this function will return an error.
 *
 * \param profile_name [IN] Name of profile to execute.
 * \param report_name  [IN] Name of test report. Length will be truncated to RFC2544_REPORT_NAME_LEN chars.
 * \param report_dscr  [IN] Description of test report. Length with be truncated to RFC2544_REPORT_DSCR_LEN chars.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_text() to convert to string.
 */
vtss_rc rfc2544_mgmt_test_start(i8 *profile_name, i8 *report_name, i8 *report_dscr);

/**
 * Stop executing the profile indicated by report_name.
 *
 * \param report_name [IN] Name of test report currently in progress.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_text() to convert to string.
 */
vtss_rc rfc2544_mgmt_test_stop(i8 *report_name);

/**
 * Get a profile as clear text.
 *
 * The function allocates a string of a suitable size (with VTSS_MALLOC()) and
 * prints the profile into it as clear text. The length of the string can be found by
 * strlen(profile_as_txt).
 *
 * Iff the function call succeeds (return value == VTSS_RC_OK), the pointer
 * to the string must be freed with a call to VTSS_FREE().
 *
 * \param profile_name   [IN] Name of profile to get as text.
 * \param profile_as_txt [OUT] Pointer to a char pointer receiving the VTSS_MALLOC()ed string.
 */
vtss_rc rfc2544_mgmt_profile_as_txt(i8 *profile_name, i8 **profile_as_txt);

/**
 * Get names and info of existing reports.
 *
 * Repeatedly call this function to get info and names of all reports.
 * The first time you invoke the function, set info.name[0] == '\0'.
 * Subsequent times, leave it as it was upon return.
 * Stop when the function returns info.name[0] == '\0'.
 *
 * Reports are returned in chronological order - oldest first.
 *
 * \param info [INOUT] Pointer to a structure receiving info about a given report. See description above on how to control the .name member.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_text() to convert to string.
 */
vtss_rc rfc2544_mgmt_report_info_get(rfc2544_report_info_t *info);

/**
 * Delete test report.
 *
 * \param name [IN] Name of report to delete
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_text() to convert to string.
 */
vtss_rc rfc2544_mgmt_report_delete(i8 *name);

/**
 * Get a report as clear text.
 *
 * The function allocates a string of a suitable size (with VTSS_MALLOC()) and
 * prints the report into it as clear text. The length of the string can be found
 * with strlen(report_as_txt).
 *
 * Iff the function call succeeds (return value == VTSS_RC_OK), the pointer
 * to the report must be freed with a call to VTSS_FREE().
 *
 * \param report_name   [IN] Name of report to get as text.
 * \param report_as_txt [OUT] Pointer to a char pointer receiving the VTSS_MALLOC()ed string.
 */
vtss_rc rfc2544_mgmt_report_as_txt(i8 *report_name, i8 **report_as_txt);

/**
 * \brief Utility function to convert frame size enumeration to frame size in bytes.
 *
 * \param fs [IN] Enumerated frame size type.
 *
 * \return Non-zero frame size in bytes on success, zero on error.
 */
u32 rfc2544_mgmt_util_frame_size_enum_to_number(rfc2544_frame_size_t fs);

/**
 * \brief Utility function to convert test execution status to a string.
 *
 * \param status [IN] Enumerated test status type.
 *
 * \return String with a textual representation of the status.
 */
const i8 *rfc2544_mgmt_util_status_to_str(rfc2544_test_status_t status);

/**
 * \brief RFC2544 module initialization function.
 *
 * \param data [IN] Initialization state.
 *
 * \return VTSS_RC_OK always.
 */
vtss_rc rfc2544_init(vtss_init_data_t *data);

#endif /* _RFC2544_API_H_ */

