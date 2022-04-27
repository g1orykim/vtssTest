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

#include "rfc2544_api.h"       /* Ourselves                     */
#include "critd_api.h"         /* For critd_t                   */
#include "port_api.h"          /* For port_isid_port_XXX()      */
#include "conf_api.h"          /* For conf_sec_XXX()            */
#include "misc_api.h"          /* For misc_conf_read_use()      */
#include "rfc2544_trace.h"     /* For trace definitions         */
#include "mep_api.h"           /* For (vtss_)mep_XXX()          */
#include "mgmt_api.h"          /* For mgmt_long2str_float()     */
#if defined(VTSS_SW_OPTION_VCLI)
#include "rfc2544_vcli.h"      /* For rfc2544_vcli_init()       */
#endif
#if defined(VTSS_SW_OPTION_ICLI)
#include "icli_porting_util.h" /* For ICLI_PORTING_STR_BUF_SIZE */
#endif
#if defined(VTSS_SW_OPTION_ICFG)
#include "icfg_api.h"          /* For vtss_icfg_XXX()           */
#endif
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_RFC2544

#define RFC2544_PROFILE_IDX_NONE ((u32)-1)
#define RFC2544_REPORT_IDX_NONE  ((u32)-1)

#if VTSS_TRACE_ENABLED

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "RFC2544",
    .descr     = "RFC2544"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [TRACE_GRP_DEFAULT] = {
        .name        = "default",
        .descr       = "Default",
        .lvl         = VTSS_TRACE_LVL_ERROR,
        .timestamp   = 1,
        .usec        = 1,
    },
    [TRACE_GRP_MEP_CALLS] = {
        .name        = "mep",
        .descr       = "MEP module calls",
        .lvl         = VTSS_TRACE_LVL_ERROR,
        .timestamp   = 1,
        .usec        = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name        = "crit",
        .descr       = "Critical regions",
        .lvl         = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};

#define RFC2544_CRIT_ENTER()         critd_enter(        &RFC2544_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define RFC2544_CRIT_EXIT()          critd_exit(         &RFC2544_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define RFC2544_CRIT_ASSERT_LOCKED() critd_assert_locked(&RFC2544_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
#define RFC2544_CRIT_ENTER()         critd_enter(        &RFC2544_crit)
#define RFC2544_CRIT_EXIT()          critd_exit (        &RFC2544_crit)
#define RFC2544_CRIT_ASSERT_LOCKED() critd_assert_locked(&RFC2544_crit)
#endif /* VTSS_TRACE_ENABLED */

/******************************************************************************/
//
// PRIVATE DATA
//
/******************************************************************************/

/**
 * Common properties of a test report.
 */
typedef struct {

    /**
     * The time the test was started.
     */
    i8 start_time[MISC_RFC3339_TIME_STR_LEN];

    /**
     * The time the test ended.
     * If still running, this is 0.
     */
    i8 end_time[MISC_RFC3339_TIME_STR_LEN];

    /**
     * TRUE if sequence numbering is enabled, and
     * at least one of the returned frames is
     * received out of order.
     */
    BOOL received_out_of_order;

    /**
     * Test status.
     */
    rfc2544_test_status_t status;

    /**
     * More detailed status. Use error_txt() to convert to an error string if != VTSS_RC_OK
     */
    vtss_rc rc;

} rfc2544_report_common_t;

/**
 * Structure holding statistics for a given trial.
 */
typedef struct {

    /**
     * Actual Tx rate, measured in kbps.
     */
    u32 tx_rate_kbps;

    /**
     * Actual Rx rate, measured in kbps.
     */
    u32 rx_rate_kbps;

    /**
     * Number of transmitted frames.
     */
    u64 tx_frame_cnt;

    /**
     * Number of received frames.
     */
    u64 rx_frame_cnt;

} rfc2544_statistics_t;

/**
 * Structure holding a list of statistics
 */
typedef struct rfc2544_statistics_list_s {

    /**
     * Statistics
     */
    rfc2544_statistics_t stat;

    /**
     * Next item in list. NULL if last.
     */
    struct rfc2544_statistics_list_s *next;

} rfc2544_statistics_list_t;

/**
 * Structure holding delay statistics for a given trial.
 */
typedef struct {

    /**
     * Minimum delay measured in nanoseconds.
     */
    u64 min_delay_ns;

    /**
     * Maximum delay measured in nanoseconds.
     */
    u64 max_delay_ns;

    /**
     * Average delay measured in nanoseconds.
     */
    u64 avg_delay_ns;

} rfc2544_delay_statistics_t;

/**
 * Throughput test results.
 */
typedef struct {

    /**
     * Common properties of this test.
     */
    rfc2544_report_common_t common;

    /**
     * Statistics per selected frame size
     */
    rfc2544_statistics_t statistics[RFC2544_FRAME_SIZE_CNT];

    /**
     * Pass/fail per selected frame size.
     */
    BOOL pass[RFC2544_FRAME_SIZE_CNT];

} rfc2544_report_throughput_t;

/**
 * Latency test results.
 */
typedef struct {

    /**
     * Common properties of this test.
     */
    rfc2544_report_common_t common;

    /**
     * Background (TST) traffic per selected frame size
     */
    rfc2544_statistics_t tst_statistics[RFC2544_FRAME_SIZE_CNT];

    /**
     * DMM frame statistics per selected frame size
     */
    rfc2544_statistics_t dmm_statistics[RFC2544_FRAME_SIZE_CNT];

    /*
     * Delay statistics
     */
    rfc2544_delay_statistics_t delay[RFC2544_FRAME_SIZE_CNT];

    /*
     * Delay variation statistics
     */
    rfc2544_delay_statistics_t delay_variation[RFC2544_FRAME_SIZE_CNT];

    /**
     * Pass/fail per selected frame size.
     */
    BOOL pass[RFC2544_FRAME_SIZE_CNT];

} rfc2544_report_latency_t;

/**
 * Frame loss and back-to-back test results.
 */
typedef struct {

    /**
     * Common properties of this test.
     */
    rfc2544_report_common_t common;

    /**
     * A given entry in the following array is pointing to
     * a list of statistics.
     *
     * The individual items in the list are dynamically
     * allocated, so when a report is no longer needed,
     * the RFC2544_report_dispose() function must be called.
     */
    rfc2544_statistics_list_t *statistics_list[RFC2544_FRAME_SIZE_CNT];

    /**
     * Pass/fail per selected frame size.
     */
    BOOL pass[RFC2544_FRAME_SIZE_CNT];

} rfc2544_report_frame_loss_and_back_to_back_t;

typedef struct {
    BOOL                      missing_frame_sizes[RFC2544_FRAME_SIZE_CNT];
    rfc2544_frame_size_t      cur_frame_size;
    u32                       cur_frame_size_bytes;
    u32                       cur_rate_kbps;   /* kbps!! */
    u32                       link_speed_mbps; /* Mbps!! */
    u32                       min_rate_kbps, max_rate_kbps;
    u32                       rate_step_kbps;
    u32                       dmm_interval_secs;
    u32                       mep_instance_id;
    BOOL                      dwelling;
    u32                       trials_succeeded_cnt;
    rfc2544_statistics_list_t *tail;
    vtss_mep_mgmt_tst_conf_t  tst_conf;
    vtss_mep_mgmt_dm_conf_t   dm_conf;
    i8                        msg[200]; /* Can be output in textual report while executing */
} rfc2544_report_state_t;

/**
 * This structure holds the binary form of a test report.
 * Embedded in the report is the currently execution state.
 */
typedef struct {
    /**
     * Executional internal state.
     */
    rfc2544_report_state_t state;

    /**
     * This part of the report is saved and restored to/from flash.
     * Since the report is dynamic during execution, it doesn't
     * make sense to save it to flash at that time. And since
     * certain parts of the report uses dynamic memory allocation,
     * it doesn't make sense to save the report as binary to flash.
     * Therefore, the last part step after executing a report
     * (whether it fails or succeeds or was cancelled by the user),
     * is to convert the report to text and save the text to flash
     * along with some meta data (report name, status, description,
     * creation time or basically what is kept in an rfc2544_report_info_t
     * structure).
     */
    rfc2544_report_info_t info;

    /**
     * Report as text. Once the report is saved to flash,
     * the binary report is no longer valid. Instead, we use
     * the following VTSS_MALLOC()ed version of report
     */
    i8 *report_as_txt;

    /**
     * Profile related to this report.
     * A copy of the input profile is generated
     * when an execution starts, so that the
     * profile can later be deleted without
     * affecting the report.
     */
    rfc2544_profile_t profile;

    /**
     * Common test results.
     */
    rfc2544_report_common_t common;

    /**
     * Throughput test results.
     */
    rfc2544_report_throughput_t throughput;

    /**
     * Latency test results.
     */
    rfc2544_report_latency_t latency;

    /**
     * Frame loss test results.
     */
    rfc2544_report_frame_loss_and_back_to_back_t frame_loss;

    /**
     * back-to-back test results.
     */
    rfc2544_report_frame_loss_and_back_to_back_t back_to_back;

} rfc2544_report_t;

static critd_t           RFC2544_crit;
static cyg_flag_t        RFC2544_wakeup_thread_flag;
static cyg_handle_t      RFC2544_thread_handle;
static cyg_thread        RFC2544_thread_block;
static char              RFC2544_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static rfc2544_profile_t RFC2544_profiles[RFC2544_PROFILE_CNT];
static rfc2544_report_t  RFC2544_reports[RFC2544_REPORT_CNT];

/* Report get state */
typedef struct {
    u32              allocated; // Current allocation in bytes.
    i8               *txt;      // Pointer to start of report.
    u32              free;      // Number of bytes not yet used (allocated - (#p - #txt)).
    i8               *p;        // Pointer to end of report.
    rfc2544_report_t *report;   // Snapshot of report to textualize.
} rfc2544_report_get_state_t;

// Overall profile configuration as saved in flash.
typedef struct {
    // Current version of the profile configuration in flash.
    u32 version;

    // Overall config
    rfc2544_profile_t profiles[RFC2544_PROFILE_CNT];
} rfc2544_profile_flash_t;

// Overall report structure as saved in flash.
typedef struct {
    // Current version of the flash reports.
    u32 version;

    // Since the report flash section has variable length, we also need to know the size of this structure as stored in flash.
    u32 struct_size;

    // Overall reports
    struct {
        // Meta info about report.
        rfc2544_report_info_t info;

        // Length of report as text in bytes including the terminating NULL.
        // If the report is empty, report_len is 1 for an empty string, so it can't be 0.
        u32 report_len;
    } r[RFC2544_REPORT_CNT];

} rfc2544_report_flash_t;

#define RFC2544_FLASH_CFG_VERSION      1
#define RFC2544_FLASH_REPORT_VERSION   1
#define RFC2544_RC(expr)               {vtss_rc __rc__ = (expr); if (__rc__ < VTSS_RC_OK) {return __rc__;}}
#define RFC2544_BOOL_TO_STR(_b_)       (_b_) ? "Enabled" : "Disabled"
#define RFC2544_PRINT_PLURAL(_val_)    (_val_), (_val_) == 1 ? "" : "s"
#define RFC2544_REPORT_EMPTY(_r_)      ((_r_)->info.name[0] == '\0')
#define RFC2544_PRINT_LINE_LENGTH      80    /* Number of characaters on one line                               */
#define RFC2544_PRINT_LINE_LENGTH_MAX  200   /* An error occurs if any print operation exceeds this line length */
#define RFC2544_PRINT_REALLOC_SIZE     10000 /* Increase by this amount per re-allocation.                      */
#define RFC2544_FLAG_EXECUTE           0x01
#define RFC2544_FLAG_STOP              0x02

/******************************************************************************/
//
// PRIVATE FUNCTIONS
//
/******************************************************************************/

/* Rate computation considerations.
 *
 * tr():
 *   Function that truncates a floating point number to nearest lower integer (i.e. normal integer division).
 *   Example: tr(7.8) = 7
 *
 * ru():
 *   Function that rounds a floating point up to the nearest higher integer.
 *   Example: ru(7.3) = 8
 *
 * fcs:
 *   Frame check sum (a.k.a. CRC). 4 bytes.
 *
 * preamble:
 *   Number of bytes to sync up the MACs. This is usually 8 bytes.
 *
 * ifg:
 *   Interframe gap in bytes. This is usually 12 bytes.
 *
 * fs_c:
 *   Customer-facing side frame size [bytes], incl. fcs, but excl. preamble and ifg.
 *   This is the frame size specified in the profiles.
 *
 * r_d_c:
 *   Desired rate [bps] on the customer-facing side.
 *   This is the rate specified in the profiles.
 *
 * bpf_c:
 *   Bits per frame on the customer-facing side.
 *   bpf_c = 8 * (preamble + fs_c + ifg)
 *
 * fps_d_c:
 *   Desired frame rate [frames per second] on the customer-facing side.
 *   If we had the perfect world, this should be a fractional, floating point
 *   number, but since we don't, we have to do an up-rounding division so that
 *   we guarantee that the customer gets what he asks for (MEP module).
 *   fps_c = ru(r_d_c / bpf_c)
 *
 * fps_d_n:
 *   Desired frame rate on the network-facing side. The key to understanding
 *   the whole thing lies here: The desired frame rate on the network-facing
 *   side must be identical to the desired frame rate on the customer-facing
 *   side, so:
 *   fps_d_n = fps_d_c
 *
 * fps_a_n [H/W-dependent]:
 *   Number of frame actually transmitted per second on the network-facing side.
 *   This depends on how the hardware allows for programming fps_d_n.
 *   On Serval, a timer controls when to transmit the next instance of a given frame.
 *   The timer can be programmed with a number of ticks, where one tick is 198.4 ns.
 *   Without using any tricks, fps_a_n on Serval can be computed as:
 *   ticks = tr(1E12 / (fps_d_n * 198,400))
 *   fps_a_n =  1E12 / (ticks   * 198,400) fps
 *   Notice: fps_a_n should not be truncated, since the H/W is free running.
 *
 * encap:
 *   Number of bytes added by the switch core to encapsulate a frame arriving on
 *   a customer-facing port, before transmitting it on the network-facing port.
 *   Encapsulation could be nothing (Port Down-MEPs) or 4 bytes (VLAN Down-MEPs).
 *   The encap is only needed in order to assess the actual, required bandwidth on
 *   the network-facing side.
 *
 * fs_n:
 *   Frame size on the network-facing side [bytes], incl. fcs, but excl. preamble and ifg.
 *   fs_n = fs_c + encap.
 *
 * bpf_n:
 *   Bits per frame on the network-facing side. This includes the fcs, preamble and ifg.
 *   bpf_n = 8 * (preamble + fs_n + ifg)
 *
 * r_d_n:
 *   The desired rate (bps) on the network-facing side.
 *   r_d_n = bpf_n * fps_d_n
 *
 * r_a_n [H/W-dependent]:
 *   The achieved rate (bps) on the network-facing side.
 *   r_a_n = bpf_n * fps_a_n
 *
 * Example [Serval]:
 *   fs_c     = 64 bytes
 *   r_d_c    = 200 Mbps
 *   encap    = 4 bytes
 *   preamble = 8 bytes
 *   ifg      = 12 bytes
 *
 *   The desired frame rate on the customer-facing side is therefore:
 *   bpf_c   = 8 * (8 + 64 + 12) = 672 bits
 *   fps_d_n = fps_d_c = ru(200,000,000 / 672) = ru(297,619.05) = 297,620 fps.
 *   bpf_n   = 8 * (8 + 64 + 4 + 12) = 704 bits
 *   r_d_n   = 704 * 297,620 = 209,524,480 bps
 *
 *   Now, on Serval, we cannot achieve exactly 297,620 fps (without tricks).
 *   Let's compute the error we get by simply using one single timer on Serval:
 *
 *   The tick count (ticks) is computed as (using picoseconds and 64-bit integer divisions):
 *   ticks = tr(1E12 / (297,620 * 198,400)) = tr(1E12 / 59,047,808,000)) = tr(16.94) = 16
 *
 *   From this, we can compute the achievable fps and rate
 *   fps_a_n = 1E12 / (16 * 198,400) fps = 315,020.1613 (no truncation here) fps.
 *   r_a_n   = bpf_n * fps_a_n = 8 * (8 + (64 + 4) + 12) * 315,020.1613 = 704 * 315,020.1613 = 221,774,193.6 bps
 *
 *   In this example, the error is r_a_n - r_d_n = 221,774,193.6 - 209,524,480 = 12,249,713.6 bps.
 *   So we would test with a rate more than 12 Mbps higher than what is requested.
 *
 * SERVAL APPROACH:
 *
 *   In the example above, the error is roughly 12 Mbps, which is too much, given that the
 *   step size is measured in permille of the line rate. The higher the desired rate, the bigger the error
 *   (see also Excel sheet in the RS1079 folder).
 *
 *   Now, what should the maximum error be? Given that we measure rates in permille of the line rate,
 *   I would say that anything within 0.5 permille of the actual line rate suffices.
 *
 *   The solution to the problem is to ask Serval's AFI to inject the same frame at multiple rates.
 *   A proof-of-concept can be found in .../rfc2544/test/simul.c
 */

/****************************************************************************/
// RFC2544_loss_criterion_fail()
/****************************************************************************/
static BOOL RFC2544_loss_criterion_fail(u64 tx, u64 rx, u32 pass_criterion_permille)
{
    u64 actual_drop, allowed_drop;

    if (tx < rx) {
        return FALSE;
    }

    actual_drop  = tx - rx;
    allowed_drop = (tx * pass_criterion_permille) / 1000LLU;

    return (actual_drop > allowed_drop);
}

/******************************************************************************/
// RFC2544_print_alloc_check()
/******************************************************************************/
static vtss_rc RFC2544_print_alloc_check(rfc2544_report_get_state_t *s)
{
    if (s->free < RFC2544_PRINT_LINE_LENGTH_MAX) {
        i8 *new_txt;

        if ((new_txt = VTSS_REALLOC(s->txt, s->allocated + RFC2544_PRINT_REALLOC_SIZE)) == NULL) {
            // Leave s->txt as is. It will be freed by rfc2544_mgmt_report_as_txt()
            return RFC2544_ERROR_OUT_OF_MEMORY;
        }

        s->txt        = new_txt;
        s->allocated += RFC2544_PRINT_REALLOC_SIZE;
        s->free      += RFC2544_PRINT_REALLOC_SIZE;
        s->p          = new_txt + (s->allocated - s->free);
        memset(s->p, 0, s->free); // NULL-terminate at any length
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_vsprintf()
/******************************************************************************/
static vtss_rc RFC2544_vsprintf(rfc2544_report_get_state_t *s, const char *fmt, va_list ap)
{
    u32 bytes;

    RFC2544_RC(RFC2544_print_alloc_check(s));

    bytes = vsnprintf(s->p, s->free - 1, fmt, ap);

    if (bytes >= RFC2544_PRINT_LINE_LENGTH_MAX) {
        return RFC2544_ERROR_REPORT_LINE_TOO_LONG;
    }

    s->p    += bytes;
    s->free -= bytes;

    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_printf()
/******************************************************************************/
static vtss_rc RFC2544_printf(rfc2544_report_get_state_t *s, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
static vtss_rc RFC2544_printf(rfc2544_report_get_state_t *s, const char *fmt, ...)
{
    va_list ap = NULL;
    vtss_rc rc;

    va_start(ap, fmt);
    rc = RFC2544_vsprintf(s, fmt, ap);
    va_end(ap);

    return rc;
}

/******************************************************************************/
// RFC2544_print_ch()
/******************************************************************************/
static vtss_rc RFC2544_print_ch(rfc2544_report_get_state_t *s, i8 ch)
{
    RFC2544_RC(RFC2544_print_alloc_check(s));

    *s->p++ = ch;
    s->free--;

    // No need to NULL-terminate, since the string is all-zeroed during creation.
    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_print_line_item()
/******************************************************************************/
static vtss_rc RFC2544_print_line_item(rfc2544_report_get_state_t *s, const i8 *name, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
static vtss_rc RFC2544_print_line_item(rfc2544_report_get_state_t *s, const i8 *name, const char *fmt, ...)
{
    va_list ap = NULL;
    vtss_rc rc;

    RFC2544_RC(RFC2544_printf(s, "  %-21s: ", name));

    if (fmt == NULL || fmt[0] == '\0') {
        // The caller may wish to print his own items one by one, and just
        // needs this function in order to get the format of the "name: " right.
        return VTSS_RC_OK;
    }

    va_start(ap, fmt);
    rc = RFC2544_vsprintf(s, fmt, ap);
    va_end(ap);

    if (rc != VTSS_RC_OK) {
        return rc;
    }

    RFC2544_RC(RFC2544_print_ch(s, '\n'));

    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_print_star_line()
/******************************************************************************/
static vtss_rc RFC2544_print_star_line(rfc2544_report_get_state_t *s, i8 *title)
{
    int i, half, title_len;

    title_len = title == NULL ? 0 : strlen(title) + 2 /* +2 for two spaces around the title */;
    half = (RFC2544_PRINT_LINE_LENGTH - title_len) / 2;

    for (i = 0; i < half; i++) {
        RFC2544_RC(RFC2544_print_ch(s, '*'));
    }

    if (title != NULL) {
        RFC2544_RC(RFC2544_print_ch(s, ' '));
        RFC2544_RC(RFC2544_printf(s, "%s", title));
        RFC2544_RC(RFC2544_print_ch(s, ' '));
    }

    half = RFC2544_PRINT_LINE_LENGTH - title_len - half;
    for (i = 0; i < half; i++) {
        RFC2544_RC(RFC2544_print_ch(s, '*'));
    }

    RFC2544_RC(RFC2544_print_ch(s, '\n'));

    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_print_header()
/******************************************************************************/
static vtss_rc RFC2544_print_header(rfc2544_report_get_state_t *s, i8 *str)
{
    RFC2544_RC(RFC2544_print_star_line(s, NULL));
    RFC2544_RC(RFC2544_printf(s, "* %s\n", str));
    RFC2544_RC(RFC2544_print_star_line(s, NULL));
    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_print_status()
/******************************************************************************/
static vtss_rc RFC2544_print_status(rfc2544_report_get_state_t *s, i8 *title, rfc2544_report_common_t *common)
{
    RFC2544_RC(RFC2544_printf(s, title));
    RFC2544_RC(RFC2544_print_line_item(s, "Started at", "%s", strlen(common->start_time) > 0 ? common->start_time : (i8 *)"-"));
    RFC2544_RC(RFC2544_print_line_item(s, "Ended at",   "%s", strlen(common->end_time)   > 0 ? common->end_time   : (i8 *)"-"));
    RFC2544_RC(RFC2544_print_line_item(s, "Status",     "%s", rfc2544_mgmt_util_status_to_str(common->status)));
    if (common->status == RFC2544_TEST_STATUS_FAILED && common->rc != VTSS_RC_OK) {
        RFC2544_RC(RFC2544_print_line_item(s, "Details", "%s", error_txt(common->rc)));
    }
    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_print_report_test_header()
/******************************************************************************/
static vtss_rc RFC2544_print_report_test_header(rfc2544_report_get_state_t *s, i8 *test_name, rfc2544_report_common_t *common, vtss_rc (*profile_print_func)(rfc2544_report_get_state_t *s))
{
    i8 buf[100];

    RFC2544_RC(RFC2544_print_ch(s, '\n'));

    // Configuration
    strcpy(buf, test_name);
    strcat(buf, " Test");
    RFC2544_RC(RFC2544_print_star_line(s, buf));
    RFC2544_RC(profile_print_func(s));

    // Status
    strcpy(buf, "\n");
    strcat(buf, test_name);
    strcat(buf, " status:\n");
    RFC2544_RC(RFC2544_print_status(s, buf, common));

    // If we're currently executing, print what it's doing.
    if (common->status == RFC2544_TEST_STATUS_EXECUTING) {
        RFC2544_RC(RFC2544_print_line_item(s, "Activity", "%s", s->report->state.msg));
    }

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_rate_to_str()
/****************************************************************************/
static i8 *RFC2544_rate_to_str(i8 *buf, u64 frame_cnt, u32 frame_size, u32 dur_msecs, u64 *out_rate_fps)
{
    u64 rate_bps, rate_fps;
    u32 bpf;

    rate_fps = (1000LLU * frame_cnt) / dur_msecs;
    bpf      = 8 * (8 + frame_size + 12); // Bits per frame: 8 bits/byte, 8 byte preamble, 12 byte IFG
    rate_bps = rate_fps * bpf;

    // Print rate with 1 decimal.
    // Divide by 1,000,000 to get to Mbps. Then multiply by 10 to get it printed correctly with 1 decimal => divide by 100,000
    mgmt_long2str_float(buf, rate_bps / 100000, 1);

    if (out_rate_fps) {
        // Only some callers need the frame rate.
        *out_rate_fps = rate_fps;
    }

    return buf; // Allow it to be used directly in a printf()
}

/****************************************************************************/
// RFC2544_print_report_tp_fl_bb()
/****************************************************************************/
static vtss_rc RFC2544_print_report_tp_fl_bb(rfc2544_report_get_state_t *s, rfc2544_statistics_t *stat, rfc2544_frame_size_t fs, u32 pass_criterion_permille, u32 trial_duration_msecs, BOOL *first)
{
    i8   buf1[100], buf2[100];
    BOOL pass;
    u64  tx_rate_fps, rx_rate_fps;
    u32  fs_bytes = rfc2544_mgmt_util_frame_size_enum_to_number(fs);

    if (*first) {
        RFC2544_RC(RFC2544_print_ch(s, '\n'));
        RFC2544_RC(RFC2544_printf(s, "Frame   Tx     Rx     Tx      Rx      Tx         Rx         Frame Status\n"));
        RFC2544_RC(RFC2544_printf(s, "Size    Rate   Rate   Rate    Rate    Frames     Frames     Loss\n"));
        RFC2544_RC(RFC2544_printf(s, "[bytes] [Mbps] [Mbps] [fps]   [fps]                         [%%]\n"));
        RFC2544_RC(RFC2544_printf(s, "------- ------ ------ ------- ------- ---------- ---------- ----- ------\n"));
        *first = FALSE;
    }

    // Compute Tx and Rx rates, and print them in Mbps with one decimal.
    (void)RFC2544_rate_to_str(buf1, stat->tx_frame_cnt, fs_bytes, trial_duration_msecs, &tx_rate_fps);
    (void)RFC2544_rate_to_str(buf2, stat->rx_frame_cnt, fs_bytes, trial_duration_msecs, &rx_rate_fps);

    RFC2544_RC(RFC2544_printf(s, "%7u %6s %6s %7llu %7llu %10llu %10llu ",
                              fs_bytes,             // Frame size
                              buf1,                 // Tx Rate [Mbps]
                              buf2,                 // Rx Rate [Mbps]
                              tx_rate_fps,          // Tx Rate [fps]
                              rx_rate_fps,          // Rx Rate [fps]
                              stat->tx_frame_cnt,   // Tx Frames
                              stat->rx_frame_cnt)); // Rx Frames

    // Frame loss with one decimal
    if (stat->tx_frame_cnt != 0 && stat->tx_frame_cnt >= stat->rx_frame_cnt) {
        pass = !RFC2544_loss_criterion_fail(stat->tx_frame_cnt, stat->rx_frame_cnt, pass_criterion_permille);
        u64 frame_loss = (1000LLU * (stat->tx_frame_cnt - stat->rx_frame_cnt)) / stat->tx_frame_cnt; // Now in permille
        mgmt_long2str_float(buf1, frame_loss /* in permille, which will be scaled back to percent ... */, 1 /* ... when printing with one decimal */);
    } else {
        strcpy(buf1, "N/A");
        pass = FALSE;
    }

    RFC2544_RC(RFC2544_printf(s, "%5s %s\n",
                              buf1,                     // Frame Loss
                              pass ? "PASS" : "FAIL")); // Status

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_print_profile_common()
/****************************************************************************/
static vtss_rc RFC2544_print_profile_common(rfc2544_report_get_state_t *s)
{
    rfc2544_frame_size_t fs;
    vtss_mac_t           smac;
    BOOL                 first = TRUE;
#if defined(VTSS_SW_OPTION_ICLI)
    i8                   interface_str[ICLI_PORTING_STR_BUF_SIZE];
#endif /* VTSS_SW_OPTION_ICLI */

    RFC2544_RC(RFC2544_printf(s, "Common configuration:\n"));
    RFC2544_RC(RFC2544_print_line_item(s, "Profile name",          "%s",          s->report->profile.common.name));
    RFC2544_RC(RFC2544_print_line_item(s, "Description",           "%s",          s->report->profile.common.dscr));
    RFC2544_RC(RFC2544_print_line_item(s, "MEG Level",             "%u",          s->report->profile.common.meg_level));
#if defined(VTSS_SW_OPTION_ICLI)
    // Use "GigabitEthernet 1/X" when we have ICLI
    RFC2544_RC(RFC2544_print_line_item(s, "Egress interface",      "%s",          icli_port_info_txt(VTSS_USID_START, iport2uport(s->report->profile.common.egress_port_no), interface_str)));
#else
    // Use uport if we don't have ICLI.
    RFC2544_RC(RFC2544_print_line_item(s, "Egress port number",    "%u",          iport2uport(s->report->profile.common.egress_port_no)));
#endif /* VTSS_SW_OPTION_ICLI */
    RFC2544_RC(RFC2544_print_line_item(s, "Sequence number check", "%s",          RFC2544_BOOL_TO_STR(s->report->profile.common.sequence_number_check)));
    RFC2544_RC(RFC2544_print_line_item(s, "Dwell time",            "%u second%s", RFC2544_PRINT_PLURAL(s->report->profile.common.dwell_time_secs)));
    RFC2544_RC(RFC2544_print_line_item(s, "Type",                  "%s",          s->report->profile.common.vlan_tag.vid == VTSS_VID_NULL ? "Port Down-MEP" : "VLAN-based Down-MEP"));
    if (s->report->profile.common.vlan_tag.vid != VTSS_VID_NULL) {
        RFC2544_RC(RFC2544_print_line_item(s, "VLAN ID", "%u", s->report->profile.common.vlan_tag.vid));
        RFC2544_RC(RFC2544_print_line_item(s, "PCP",     "%u", s->report->profile.common.vlan_tag.pcp));
        RFC2544_RC(RFC2544_print_line_item(s, "DEI",     "%u", s->report->profile.common.vlan_tag.dei));
    }
    RFC2544_RC(RFC2544_print_line_item(s, "Destination MAC",       "%s", misc_mac2str(s->report->profile.common.dmac.addr)));
    (void)conf_mgmt_mac_addr_get(smac.addr, iport2uport(s->report->profile.common.egress_port_no));
    RFC2544_RC(RFC2544_print_line_item(s, "Source MAC",            "%s", misc_mac2str(smac.addr)));
    RFC2544_RC(RFC2544_print_line_item(s, "Frame sizes",           NULL));
    for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
        if (s->report->profile.common.selected_frame_sizes[fs]) {
            RFC2544_RC(RFC2544_printf(s, "%s%u", first ? "" : " ", rfc2544_mgmt_util_frame_size_enum_to_number(fs)));
            first = FALSE;
        }
    }
    RFC2544_RC(RFC2544_print_ch(s, '\n'));
    RFC2544_RC(RFC2544_print_line_item(s, "Throughput test",   "%s", RFC2544_BOOL_TO_STR(s->report->profile.common.selected_tests & RFC2544_TEST_TYPE_THROUGHPUT)));
    RFC2544_RC(RFC2544_print_line_item(s, "Latency test",      "%s", RFC2544_BOOL_TO_STR(s->report->profile.common.selected_tests & RFC2544_TEST_TYPE_LATENCY)));
    RFC2544_RC(RFC2544_print_line_item(s, "Frame loss test",   "%s", RFC2544_BOOL_TO_STR(s->report->profile.common.selected_tests & RFC2544_TEST_TYPE_FRAME_LOSS)));
    RFC2544_RC(RFC2544_print_line_item(s, "Back-to-back test", "%s", RFC2544_BOOL_TO_STR(s->report->profile.common.selected_tests & RFC2544_TEST_TYPE_BACK_TO_BACK)));

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_print_profile_throughput()
/****************************************************************************/
static vtss_rc RFC2544_print_profile_throughput(rfc2544_report_get_state_t *s)
{
    RFC2544_RC(RFC2544_printf(s, "Throughput configuration:\n"));
    RFC2544_RC(RFC2544_print_line_item(s, "Trial duration",     "%u second%s", RFC2544_PRINT_PLURAL(s->report->profile.throughput.trial_duration_secs)));
    RFC2544_RC(RFC2544_print_line_item(s, "Minimum rate",       "%u permille", s->report->profile.throughput.rate_min_permille));
    RFC2544_RC(RFC2544_print_line_item(s, "Maximum rate",       "%u permille", s->report->profile.throughput.rate_max_permille));
    RFC2544_RC(RFC2544_print_line_item(s, "Accuracy",           "%u permille", s->report->profile.throughput.rate_step_permille));
    RFC2544_RC(RFC2544_print_line_item(s, "Allowed frame loss", "%u permille", s->report->profile.throughput.pass_criterion_permille));

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_print_profile_latency()
/****************************************************************************/
static vtss_rc RFC2544_print_profile_latency(rfc2544_report_get_state_t *s)
{
    RFC2544_RC(RFC2544_printf(s, "Latency configuration:\n"));
    RFC2544_RC(RFC2544_print_line_item(s, "Trial duration",       "%u second%s", RFC2544_PRINT_PLURAL(s->report->profile.latency.trial_duration_secs)));
    RFC2544_RC(RFC2544_print_line_item(s, "Delay meas. interval", "%u second%s", RFC2544_PRINT_PLURAL(s->report->profile.latency.dmm_interval_secs)));
    RFC2544_RC(RFC2544_print_line_item(s, "Allowed frame loss",   "%u permille", s->report->profile.latency.pass_criterion_permille));

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_print_profile_frame_loss()
/****************************************************************************/
static vtss_rc RFC2544_print_profile_frame_loss(rfc2544_report_get_state_t *s)
{
    RFC2544_RC(RFC2544_printf(s, "Frame loss configuration:\n"));
    RFC2544_RC(RFC2544_print_line_item(s, "Trial duration", "%u second%s", RFC2544_PRINT_PLURAL(s->report->profile.frame_loss.trial_duration_secs)));
    RFC2544_RC(RFC2544_print_line_item(s, "Minimum rate",   "%u permille", s->report->profile.frame_loss.rate_min_permille));
    RFC2544_RC(RFC2544_print_line_item(s, "Maximum rate",   "%u permille", s->report->profile.frame_loss.rate_max_permille));
    RFC2544_RC(RFC2544_print_line_item(s, "Rate step",      "%u permille", s->report->profile.frame_loss.rate_step_permille));

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_print_profile_back_to_back()
/****************************************************************************/
static vtss_rc RFC2544_print_profile_back_to_back(rfc2544_report_get_state_t *s)
{
    RFC2544_RC(RFC2544_printf(s, "Back-to-back configuration:\n"));
    RFC2544_RC(RFC2544_print_line_item(s, "Trial duration", "%u millisecond%s", RFC2544_PRINT_PLURAL(s->report->profile.back_to_back.trial_duration_msecs)));
    RFC2544_RC(RFC2544_print_line_item(s, "Trial count",    "%u",               s->report->profile.back_to_back.trial_cnt));

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_timestamp()
// We store the absolute time as a string, because if we saved it to flash as
// as binary, relative time, we couldn't reload it from flash later without
// losing the absolution.
// We could save it to flash as a binary, absolute time, but currently we don't
// have generic functions for converting an absolute time to an ISO string.
/****************************************************************************/
static void RFC2544_timestamp(rfc2544_report_common_t *common, BOOL end_time)
{
    i8 *p = end_time ? common->end_time : common->start_time;

    (void)misc_time2str_r(time(NULL), p);
}

/****************************************************************************/
// RFC2544_delay_to_str()
/****************************************************************************/
static char *RFC2544_delay_to_str(char *buf, rfc2544_delay_statistics_t *d)
{
    sprintf(buf, "%llu/%llu/%llu", d->min_delay_ns / 1000, d->avg_delay_ns / 1000, d->max_delay_ns / 1000);
    return buf;
}

/****************************************************************************/
// RFC2544_frame_size_next()
// Returns TRUE if still missing a frame size, FALSE if not, in which case
// the test can stop.
/****************************************************************************/
static BOOL RFC2544_frame_size_next(rfc2544_report_state_t *s)
{
    rfc2544_frame_size_t fs;

    for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
        if (s->missing_frame_sizes[fs]) {
            s->cur_frame_size = fs;
            s->cur_frame_size_bytes = rfc2544_mgmt_util_frame_size_enum_to_number(fs);
            s->missing_frame_sizes[fs] = FALSE;
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************/
// RFC2544_rate_permille_to_kpbs()
/****************************************************************************/
static u32 RFC2544_rate_permille_to_kbps(rfc2544_report_state_t *s, u32 rate_permille)
{
    return s->link_speed_mbps * rate_permille;
}

/****************************************************************************/
// RFC2544_test_suite_init()
/****************************************************************************/
static BOOL RFC2544_test_suite_init(u32 idx)
{
    rfc2544_report_t         *r = &RFC2544_reports[idx];
    rfc2544_report_state_t   *s = &r->state;
    port_status_t            port_status;
    vtss_mep_mgmt_def_conf_t mep_all_conf;
    vtss_mep_mgmt_conf_t     *mep_conf;
    BOOL                     found = FALSE;

    memset(s, 0, sizeof(*s));

    // Get info about egress port. We need the link status and speed.
    if ((r->common.rc = port_mgmt_status_get(VTSS_ISID_LOCAL, r->profile.common.egress_port_no, &port_status)) != VTSS_RC_OK) {
        T_E("port_mgmt_status_get() failed (rc = %u = %s)", r->common.rc, error_txt(r->common.rc));
        return FALSE;
    }

    // There must be link on the selected egress port.
    if (!port_status.status.link) {
        r->common.rc = RFC2544_ERROR_NO_LINK;
        return FALSE;
    }

    // Since all the configured rates are specified in permille, we need to know the
    // port's actual link speed to convert to Mbps.
    switch (port_status.status.speed)  {
    case VTSS_SPEED_10M:
        s->link_speed_mbps = 10;
        break;

    case VTSS_SPEED_100M:
        s->link_speed_mbps = 100;
        break;

    case VTSS_SPEED_1G:
        s->link_speed_mbps = 1000;
        break;

    case VTSS_SPEED_2500M:
        s->link_speed_mbps = 2500;
        break;

    case VTSS_SPEED_5G:
        s->link_speed_mbps = 5000;
        break;

    case VTSS_SPEED_10G:
        s->link_speed_mbps = 10000;
        break;

    case VTSS_SPEED_12G:
        s->link_speed_mbps = 12000;
        break;

    default:
        T_E("Unknown link speed");
        r->common.rc = VTSS_RC_ERROR;
        return FALSE;
    }

    // So far so good. Now, create a MEP.

    // Get defaults for a bunch of configurations:
    mep_mgmt_def_conf_get(&mep_all_conf);
    T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_def_conf_get()");

    // Create the MEP
    mep_conf = &mep_all_conf.config;
    mep_conf->enable      = TRUE;
    mep_conf->mode        = VTSS_MEP_MGMT_MEP;
    mep_conf->direction   = VTSS_MEP_MGMT_DOWN;
    mep_conf->domain      = r->profile.common.vlan_tag.vid == 0 ? VTSS_MEP_MGMT_PORT : VTSS_MEP_MGMT_VLAN;
    mep_conf->flow        = r->profile.common.vlan_tag.vid == 0 ? r->profile.common.egress_port_no : r->profile.common.vlan_tag.vid;
    mep_conf->port        = r->profile.common.egress_port_no;
    mep_conf->level       = r->profile.common.meg_level;
    mep_conf->vid         = r->profile.common.vlan_tag.vid;
    mep_conf->voe         = TRUE;
    mep_conf->peer_count  = 1;
    mep_conf->peer_mep[0] = 1;
    memcpy(mep_conf->peer_mac[0], r->profile.common.dmac.addr, sizeof(mep_conf->peer_mac[0]));

    // Search for an unused instance ID.
    // The following handling of mep_instance_id allows the loop to terminate correctly,
    // even when it's an unsigned.
    s->mep_instance_id = MEP_INSTANCE_MAX;
    while (s->mep_instance_id > 0) {
        u8                   dummy_mac[6];
        vtss_mep_mgmt_conf_t dummy_conf;

        s->mep_instance_id--;
        r->common.rc = mep_mgmt_conf_get(s->mep_instance_id, dummy_mac, NULL, NULL, &dummy_conf);
        T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_conf_get(rc = %d)", r->common.rc);

        if (r->common.rc == VTSS_RC_OK || r->common.rc == MEP_RC_VOLATILE) {
            r->common.rc = VTSS_RC_OK; // Get rid of the volatile return code.
            if (!dummy_conf.enable) {
                // This entry is vacant. Use it.
                r->common.rc = mep_mgmt_volatile_conf_set(s->mep_instance_id, mep_conf);
                T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_volatile_conf_set(rc = %d)", r->common.rc);
                found = TRUE;
                break;
            }
        } else {
            // Couldn't even get configuration.
            break;
        }

    }

    if (!found) {
        // All MEPs are in use.
        r->common.rc = RFC2544_ERROR_OUT_OF_MEPS;
        return FALSE;
    }

    if (r->common.rc != VTSS_RC_OK) {
        s->mep_instance_id = MEP_INSTANCE_MAX; // Invalid ID, so that we don't happen to disable some other module's MEP.
        return FALSE;
    }

    // Also initialize the tst_conf and dm_conf with general profile data.
    s->tst_conf           = mep_all_conf.tst_conf;
    s->tst_conf.enable_rx = TRUE;
    s->tst_conf.prio      = r->profile.common.vlan_tag.vid == 0 ? 7 /* CoS */ : r->profile.common.vlan_tag.pcp;
    s->tst_conf.dei       = r->profile.common.vlan_tag.vid == 0 ? 0 : r->profile.common.vlan_tag.dei;
    s->tst_conf.sequence  = r->profile.common.sequence_number_check;

    s->dm_conf            = mep_all_conf.dm_conf;
    s->dm_conf.prio       = r->profile.common.vlan_tag.vid == 0 ? 7 /* CoS */ : r->profile.common.vlan_tag.pcp;
    s->dm_conf.dei        = r->profile.common.vlan_tag.vid == 0 ? 0 : r->profile.common.vlan_tag.dei;
    s->dm_conf.cast       = VTSS_MEP_MGMT_UNICAST;
    s->dm_conf.ended      = VTSS_MEP_MGMT_DUAL_ENDED; // 1DM frames.
    s->dm_conf.tunit      = VTSS_MEP_MGMT_NS; // Ask the MEP module to compute things in nanoseconds rather than microseconds

    return TRUE; // Success!!
}

/****************************************************************************/
// RFC2544_mep_delete()
/****************************************************************************/
static vtss_rc RFC2544_mep_delete(rfc2544_report_t *r)
{
    rfc2544_report_state_t *s = &r->state;
    vtss_rc                rc = VTSS_RC_OK;

    if (s->mep_instance_id < MEP_INSTANCE_MAX) {
        vtss_mep_mgmt_conf_t mep_conf;
        u8                   dummy_mac[6];

        T_D("Deleting MEP (%u)", s->mep_instance_id);

        rc = mep_mgmt_conf_get(s->mep_instance_id, dummy_mac, NULL, NULL, &mep_conf);
        T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_conf_get(rc = %d)", rc);

        if (rc == MEP_RC_VOLATILE) {
            mep_conf.enable = FALSE;
            if ((rc = mep_mgmt_volatile_conf_set(s->mep_instance_id, &mep_conf)) != VTSS_RC_OK) {
                T_E("Failed to disable MEP #%u. Return code = %u = %s", s->mep_instance_id, rc, error_txt(rc));
            }
            T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_volatile_conf_set(rc = %d)", rc);
        } else {
            T_E("Someone has changed my volatile entry #%u to non-volatile. Return code = %u = %s", s->mep_instance_id, rc, error_txt(rc));
            rc = VTSS_RC_ERROR; // Generic error
        }
    }

    s->mep_instance_id = MEP_INSTANCE_MAX; // Avoid deleting some other module's MEP.
    return rc;
}

/****************************************************************************/
// RFC2544_stat_transfer()
/****************************************************************************/
static void RFC2544_stat_transfer(rfc2544_statistics_t *stat, rfc2544_report_state_t *s, vtss_mep_mgmt_tst_state_t *ctrs)
{
    stat->tx_rate_kbps = s->cur_rate_kbps;
    stat->rx_rate_kbps = s->cur_rate_kbps;
    stat->tx_frame_cnt = ctrs->tx_counter;
    stat->rx_frame_cnt = ctrs->rx_counter;
}

/****************************************************************************/
// RFC2544_dichotomist()
// Returns 0 if we can't get any closer. Otherwise a value in range ]min; max[
/****************************************************************************/
static u32 RFC2544_dichotomist(u32 min, u32 max, u32 step)
{
    u32 diff = max - min;
    if (step >= diff) {
        // Can't get any closer with the selected step size.
        return 0;
    }

    // Pick the middle between min and max, rounded up.
    return (min + ((diff + 1) / 2));
}

/****************************************************************************/
// RFC2544_tst_cnt_get_and_chk()
/****************************************************************************/
static vtss_rc RFC2544_tst_cnt_get_and_chk(rfc2544_report_t *r, rfc2544_statistics_t *stat, vtss_mep_mgmt_tst_state_t *ctrs, BOOL always_tfr_ctrs)
{
    rfc2544_report_state_t *s = &r->state;
    vtss_rc                rc;

    // Get TST results.
    rc = mep_mgmt_tst_state_get(s->mep_instance_id, ctrs);
    T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_tst_state_get(rc = %d)", rc);

    if (rc != VTSS_RC_OK) {
        T_E("Unable to get TST counters for mep ID = %u", s->mep_instance_id);
        return rc;
    }

    T_D("Get TST results (tx_counter = %llu, rx_counter = %llu, oo_counter = %llu, rx_rate = %u, time = %u, MEP ID = %u)", ctrs->tx_counter, ctrs->rx_counter, ctrs->oo_counter, ctrs->rx_rate, ctrs->time, s->mep_instance_id);

    if (ctrs->rx_counter > ctrs->tx_counter) {
        T_E("Received more TST frames than were sent (rx = %llu, tx = %llu)", ctrs->rx_counter, ctrs->tx_counter);
        rc = RFC2544_ERROR_TST_RX_HIGHER_THAN_TX;
    } else if (ctrs->tx_counter == 0) {
        T_E("No TST frames were sent");
        rc = RFC2544_ERROR_TST_NO_FRAMES_SENT;
    } else if (r->profile.common.sequence_number_check && ctrs->oo_counter != 0) {
        // Sequence number check is enabled, and at least one frame was
        // received out of order.
        // RBNTBD: This has got to be moved down, so that we don't just return an error as soon
        // as an out-of-order indication is seen. It should only be tested when the "allowed frame loss" is met.
        rc = RFC2544_ERROR_TST_OUT_OF_ORDER;
    }

    if (always_tfr_ctrs || rc != VTSS_RC_OK) {
        RFC2544_stat_transfer(stat, s, ctrs);
    }

    return rc;
}

/****************************************************************************/
// RFC2544_stat_alloc_and_tst_cnt_get_and_chk()
/****************************************************************************/
static vtss_rc RFC2544_stat_alloc_and_tst_cnt_get_and_chk(rfc2544_report_t *r, rfc2544_statistics_list_t **stat_list, vtss_mep_mgmt_tst_state_t *ctrs)
{
    rfc2544_statistics_list_t *stat_item;
    rfc2544_report_state_t    *s = &r->state;

    // Allocate a new statistics item.
    if ((stat_item = VTSS_MALLOC(sizeof(*stat_item))) == NULL) {
        return RFC2544_ERROR_OUT_OF_MEMORY;
    }

    // Link it in at the end of the current list.
    stat_item->next = NULL;
    if (*stat_list) {
        s->tail->next = stat_item;
    } else {
        *stat_list = stat_item;
    }
    s->tail = stat_item;

    // Get and check TST results.
    return RFC2544_tst_cnt_get_and_chk(r, &stat_item->stat, ctrs, TRUE);
}

/****************************************************************************/
// RFC2544_latency_1dm_cnt_get_and_chk()
/****************************************************************************/
static vtss_rc RFC2544_latency_1dm_cnt_get_and_chk(rfc2544_report_t *r)
{
    vtss_mep_mgmt_dm_state_t   dmr_unused, f2n, n2f;
    rfc2544_report_state_t     *s = &r->state;
    rfc2544_statistics_t       *stat = &r->latency.dmm_statistics[s->cur_frame_size];
    rfc2544_delay_statistics_t *d1, *d2;
    vtss_rc                    rc;
    u64                        mul;

    rc = mep_mgmt_dm_state_get(s->mep_instance_id, &dmr_unused, &f2n, &n2f);
    T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_dm_state_get(rc = %d)", rc);

    if (rc != VTSS_RC_OK) {
        return rc;
    }

    stat->tx_rate_kbps = 0; // Unused
    stat->rx_rate_kbps = 0; // Unused
    stat->tx_frame_cnt = n2f.tx_cnt;
    stat->rx_frame_cnt = f2n.rx_cnt;

    mul = f2n.tunit == VTSS_MEP_MGMT_US ? 1000LLU : 1LLU;
    d1 = &r->latency.delay[s->cur_frame_size];
    d1->min_delay_ns = f2n.min_delay * mul;
    d1->avg_delay_ns = f2n.avg_delay * mul;
    d1->max_delay_ns = f2n.max_delay * mul;

    d2 = &r->latency.delay_variation[s->cur_frame_size];
    d2->min_delay_ns = f2n.min_delay_var * mul;
    d2->avg_delay_ns = f2n.avg_delay_var * mul;
    d2->max_delay_ns = f2n.max_delay_var * mul;

    // f2n holds the far-to-near counters/statistics, and n2f the near-to-far counters/statistics.
    T_I("1DM (fs=%4u) tx=%u, rx=%u, min/avg/max=%llu/%llu/%llu ns, min/avg/max DV=%llu/%llu/%llu ns",
        r->state.cur_frame_size_bytes, n2f.tx_cnt, f2n.rx_cnt,
        d1->min_delay_ns, d1->avg_delay_ns, d1->max_delay_ns,
        d2->min_delay_ns, d2->avg_delay_ns, d2->max_delay_ns);

    if (f2n.rx_cnt > n2f.tx_cnt) {
        T_E("Received more 1DM frames than were sent (rx = %u, tx = %u)", f2n.rx_cnt, n2f.tx_cnt);
        return RFC2544_ERROR_1DM_RX_HIGHER_THAN_TX;
    }

    if (n2f.tx_cnt == 0) {
        T_E("No 1DM frames were sent");
        return RFC2544_ERROR_1DM_NO_FRAMES_SENT;
    }

    if (f2n.rx_cnt < n2f.tx_cnt) {
        // This is a valid runtime condition, so don't flag it with a T_E().
        T_I("Received less 1DM frame than were sent (rx = %u, tx = %u)", f2n.rx_cnt, n2f.tx_cnt);
        return RFC2544_ERROR_1DM_RX_LOWER_THAN_TX;
    }

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_throughput_analyze()
//
// On entry:
//   s->cur_rate_kbps contains the rate that we just tested with.
//   s->max_rate_kbps is the same as s->cur_rate_kbps the very first time
//   this function is invoked. In subsequent calls, it is the highest rate
//   at which the test case was failing.
//   s->min_rate_kbps is MAX_UINT the very first time this function is invoked,
//   unless the min and max rates specified by the user are identical, in
//   which case it is equal to s->max_rate_kbps. In subsequent calls, it is
//   the lowest rate at which the test case succeeded.
//
// On exit:
//   If return code is != VTSS_RC_OK, the s->XXX are no longer valid, and
//   the test case has failed.
//   If return code is == VTSS_RC_OK, s->cur_rate_kbps indicates the new
//   rate to test with, unless it is 0, in which case the test has passed.
//   In case the test should be repeated with a new rate, it is either because
//   the current rate succeeded, in which case s->min_rate_kbps is updated
//   to the current rate, or it is because the current rate failed, in which
//   case s->max_rate_kbps is lowered to the current rate.
/****************************************************************************/
static vtss_rc RFC2544_throughput_analyze(rfc2544_report_t *r)
{
    rfc2544_report_state_t    *s = &r->state;
    vtss_mep_mgmt_tst_state_t ctrs;
    vtss_rc                   rc;
    char                      buf1[100], buf2[100];
    u32                       old_rate = s->cur_rate_kbps;
    BOOL                      loss_criterion_fail;

    // Get and check TST results.
    if ((rc = RFC2544_tst_cnt_get_and_chk(r, &r->throughput.statistics[s->cur_frame_size], &ctrs, FALSE)) != VTSS_RC_OK) {
        return rc;
    }

    if ((loss_criterion_fail = RFC2544_loss_criterion_fail(ctrs.tx_counter, ctrs.rx_counter, r->profile.throughput.pass_criterion_permille))) {
        // We failed at s->cur_rate_kbps. Move the bar down.
        s->max_rate_kbps = s->cur_rate_kbps;

        // Check to see if this is at the lowest rate ever tried.
        if (s->cur_rate_kbps == s->min_rate_kbps) {
            // It is. This means that even the lowest rate fails.
            RFC2544_stat_transfer(&r->throughput.statistics[s->cur_frame_size], s, &ctrs);
            return RFC2544_ERROR_TST_ALLOWED_LOSS_EXCEEDED;
        }

        if (s->min_rate_kbps == 0xFFFFFFFFUL) {
            // The minimum rate has not been tried yet.
            s->cur_rate_kbps = s->min_rate_kbps = RFC2544_rate_permille_to_kbps(s, r->profile.throughput.rate_min_permille);
            goto do_exit_with_success;
        }

        // If we get here, the minimum rate must have passed successfully at some point in time, so
        // we gotta do a binary search between minimum and maximum tried to get a better shot at the
        // best throughput.
    } else {
        // We succeeded at s->cur_rate_kbps. Move the bar up.
        RFC2544_stat_transfer(&r->throughput.statistics[s->cur_frame_size], s, &ctrs);

        s->min_rate_kbps = s->cur_rate_kbps;
    }

    s->cur_rate_kbps = RFC2544_dichotomist(s->min_rate_kbps, s->max_rate_kbps, s->rate_step_kbps);

do_exit_with_success:
    mgmt_long2str_float(buf1, old_rate / 100, 1);
    if (s->cur_rate_kbps == 0) {
        mgmt_long2str_float(buf2, r->throughput.statistics[s->cur_frame_size].tx_rate_kbps / 100, 1);
    } else {
        mgmt_long2str_float(buf2, s->cur_rate_kbps / 100, 1);
    }

    T_I("Throughput: Frame size = %4u bytes, rate = %s Mbps: %s. %s rate = %s Mbps", s->cur_frame_size_bytes, buf1, loss_criterion_fail ? "FAILED" : "PASSED", s->cur_rate_kbps == 0 ? "Done. Final success" : "Next", buf2);

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_latency_analyze()
/****************************************************************************/
static vtss_rc RFC2544_latency_analyze(rfc2544_report_t *r)
{
    vtss_mep_mgmt_tst_state_t ctrs;
    rfc2544_report_state_t    *s = &r->state;
    vtss_rc                   rc;

    // Get and check TST results.
    if ((rc = RFC2544_tst_cnt_get_and_chk(r, &r->latency.tst_statistics[s->cur_frame_size], &ctrs, TRUE)) != VTSS_RC_OK) {
        return rc;
    }

    // Get and check 1DM results.
    if ((rc = RFC2544_latency_1dm_cnt_get_and_chk(r)) != VTSS_RC_OK) {
        return rc;
    }

    // Check loss
    if (RFC2544_loss_criterion_fail(ctrs.tx_counter, ctrs.rx_counter, r->profile.latency.pass_criterion_permille)) {
        return RFC2544_ERROR_TST_ALLOWED_LOSS_EXCEEDED;
    }

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_frame_loss_analyze()
/****************************************************************************/
static vtss_rc RFC2544_frame_loss_analyze(rfc2544_report_t *r)
{
    vtss_mep_mgmt_tst_state_t ctrs;
    rfc2544_report_state_t    *s = &r->state;
    vtss_rc                   rc;
    char                      buf1[100], buf2[100];
    u32                       old_rate = s->cur_rate_kbps;
    BOOL                      failed;

    if ((rc = RFC2544_stat_alloc_and_tst_cnt_get_and_chk(r, &r->frame_loss.statistics_list[s->cur_frame_size], &ctrs)) != VTSS_RC_OK) {
        return rc;
    }

    failed = RFC2544_loss_criterion_fail(ctrs.tx_counter, ctrs.rx_counter, 0); // 0.0% loss is tolerated.
    if (failed) {
        // Two consecutive trials must succeed, so reset the counter here.
        s->trials_succeeded_cnt = 0;
    } else {
        if (++s->trials_succeeded_cnt == 2) {
            s->cur_rate_kbps = 0; // Prevent caller from applying a new rate.
            goto do_exit_with_success;
        }
    }

    // If we get here, we should try with a new rate, if possible.
    if (s->cur_rate_kbps <= s->rate_step_kbps || // Needed because we do unsigned arithmetics
        s->cur_rate_kbps  - s->rate_step_kbps < s->min_rate_kbps) {
        // We can't try at a lower rate.
        return RFC2544_ERROR_TST_ALLOWED_LOSS_EXCEEDED;
    }

    s->cur_rate_kbps -= s->rate_step_kbps;

do_exit_with_success:
    mgmt_long2str_float(buf1, old_rate / 100, 1);
    if (s->cur_rate_kbps != 0) {
        mgmt_long2str_float(buf2, s->cur_rate_kbps / 100, 1);
        strcat(buf2, " Mbps");
    }

    T_I("Frame Loss: Frame size = %4u bytes, rate = %s Mbps: %s. %s%s", s->cur_frame_size_bytes, buf1, failed ? "FAILED" : "PASSED", s->cur_rate_kbps == 0 ? "Done." : "Next rate = ", s->cur_rate_kbps == 0 ? "" : buf2);

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_back_to_back_analyze()
/****************************************************************************/
static vtss_rc RFC2544_back_to_back_analyze(rfc2544_report_t *r)
{
    vtss_mep_mgmt_tst_state_t ctrs;
    rfc2544_report_state_t    *s = &r->state;
    vtss_rc                   rc;
    char                      buf[100];
    u32                       old_rate = s->cur_rate_kbps;
    BOOL                      failed;

    if ((rc = RFC2544_stat_alloc_and_tst_cnt_get_and_chk(r, &r->back_to_back.statistics_list[s->cur_frame_size], &ctrs)) != VTSS_RC_OK) {
        return rc;
    }

    failed = RFC2544_loss_criterion_fail(ctrs.tx_counter, ctrs.rx_counter, 0); // 0.0% loss is tolerated.
    if (failed) {
        return RFC2544_ERROR_TST_ALLOWED_LOSS_EXCEEDED;
    }

    // In all other cases, we either try once more with the same rate
    // or go to the next frame size.
    if (++s->trials_succeeded_cnt == r->profile.back_to_back.trial_cnt) {
        s->cur_rate_kbps = 0; // Prevent caller from applying the same rate once more.
    }

    mgmt_long2str_float(buf, old_rate / 100, 1);
    T_I("Back-to-back: Frame size = %4u bytes, rate = %s Mbps, trial number #%u/%u", s->cur_frame_size_bytes, buf, s->trials_succeeded_cnt, r->profile.back_to_back.trial_cnt);

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_traffic_start()
// This function assumes that s->tst_conf, s->mep_instance_id, s->cur_rate_kbps,
// and s->cur_frame_size_bytes are filled in on entry.
//
// The function clears the MEP counters, starts TST traffic, and updates
// s->dwelling, and s->msg.
//
// If s->dmm_interval_secs != 0, it also starts 1DM traffic.
/****************************************************************************/
static vtss_rc RFC2544_traffic_start(rfc2544_report_state_t *s, u32 trial_duration_msecs)
{
    vtss_rc rc;
    char    buf1[100], buf2[100];
    BOOL    print_plural_s;

    T_D("Clearing TST statistics (MEP ID = %u)", s->mep_instance_id);

    rc = mep_mgmt_tst_state_clear_set(s->mep_instance_id);
    T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_tst_state_clear_set(rc = %d)", rc);

    if (rc != VTSS_RC_OK) {
        T_E("Couldn't clear TST statistics (%s)", error_txt(rc));
        return rc;
    }

    s->tst_conf.enable = TRUE;
    s->tst_conf.rate   = s->cur_rate_kbps;
    s->tst_conf.size   = s->cur_frame_size_bytes;

    mgmt_long2str_float(buf1, s->cur_rate_kbps / 100, 1);
    T_D("Start TST. Frame size = %4u bytes, rate = %s Mbps", s->tst_conf.size, buf1);

    rc = mep_mgmt_tst_conf_set(s->mep_instance_id, &s->tst_conf);
    T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_tst_conf_set(rc = %d)", rc);

    if (rc != VTSS_RC_OK) {
        T_E("Unable to set TST conf (%s)", error_txt(rc));
        return rc;
    }

    if (s->dmm_interval_secs != 0) {
        T_D("Clearing 1DM statistics (MEP ID = %u)", s->mep_instance_id);

        rc = mep_mgmt_dm_state_clear_set(s->mep_instance_id);
        T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_dm_state_clear_set(rc = %d)", rc);

        if (rc != VTSS_RC_OK) {
            T_E("Couldn't clear DM statistics (%s)", error_txt(rc));
            return rc;
        }

        s->dm_conf.enable   = TRUE;
        s->dm_conf.interval = 100 * s->dmm_interval_secs; // Units: 10ms.

        rc = mep_mgmt_dm_conf_set(s->mep_instance_id, &s->dm_conf);
        T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_dm_conf_set(rc = %d)", rc);

        if (rc != VTSS_RC_OK) {
            T_E("Unable to set DM conf (%s)", error_txt(rc));
            return rc;
        }
    }

    s->dwelling = FALSE;

    if (trial_duration_msecs % 1000) {
        // Trial duration must be printed in seconds with one decimal.
        mgmt_long2str_float(buf2, trial_duration_msecs / 100, 1);
        print_plural_s = TRUE;
    } else {
        sprintf(buf2, "%u", trial_duration_msecs / 1000);
        print_plural_s = trial_duration_msecs != 1000;
    }

    sprintf(s->msg, "Tx %u byte TST frames at %s Mbps for %s second%s", s->cur_frame_size_bytes, buf1, buf2, print_plural_s ? "s" : "");
    if (s->dmm_interval_secs != 0) {
        sprintf(s->msg + strlen(s->msg), " and 1DM frames every %u second%s", RFC2544_PRINT_PLURAL(s->dmm_interval_secs));
    }
    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_traffic_stop()
// This function assumes that s->tst_conf and s->mep_instance_id are filled
// on entry.
// The function stops TST traffic, and updates s->dwelling, and s->msg.
/****************************************************************************/
static vtss_rc RFC2544_traffic_stop(rfc2544_report_state_t *s, u32 dwell_time_secs)
{
    vtss_rc rc;

    if (s->tst_conf.enable) {
        s->tst_conf.enable = FALSE;
        T_D("Stop TST (rate = %u Mbps, frame size = %u, MEP ID = %u). Now dwelling for %u second%s", s->tst_conf.rate / 1000, s->tst_conf.size, s->mep_instance_id, RFC2544_PRINT_PLURAL(dwell_time_secs));

        rc = mep_mgmt_tst_conf_set(s->mep_instance_id, &s->tst_conf);
        T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_tst_conf_set(rc = %d)", rc);

        if (rc != VTSS_RC_OK) {
            T_E("Unable to set TST conf (%s)", error_txt(rc));
            return rc;
        }
    }

    if (s->dm_conf.enable) {
        s->dm_conf.enable = FALSE;
        T_D("Stop 1DM (Interval = %u second%s, MEP ID = %u)", RFC2544_PRINT_PLURAL(s->dm_conf.interval / 100), s->mep_instance_id);

        rc = mep_mgmt_dm_conf_set(s->mep_instance_id, &s->dm_conf);
        T_DG(TRACE_GRP_MEP_CALLS, "mep_mgmt_dm_conf_set(rc = %d)", rc);

        if (rc != VTSS_RC_OK) {
            T_E("Unable to set DM conf (%s)", error_txt(rc));
            return rc;
        }
    }

    s->dwelling = TRUE;
    sprintf(s->msg, "Dwelling for %u second%s", RFC2544_PRINT_PLURAL(dwell_time_secs));

    return VTSS_RC_OK;
}

/****************************************************************************/
// RFC2544_throughput_rate_init()
/****************************************************************************/
static void RFC2544_throughput_rate_init(rfc2544_report_t *r)
{
    rfc2544_report_state_t *s = &r->state;

    s->cur_rate_kbps = RFC2544_rate_permille_to_kbps(s, r->profile.throughput.rate_max_permille);
    if (r->profile.throughput.rate_min_permille == r->profile.throughput.rate_max_permille) {
        s->min_rate_kbps = s->cur_rate_kbps;
    } else {
        s->min_rate_kbps = 0xFFFFFFFFUL; // Not tried yet.
    }
    s->max_rate_kbps = s->cur_rate_kbps;
}

/****************************************************************************/
// RFC2544_throughput_run()
/****************************************************************************/
static u32 RFC2544_throughput_run(u32 idx)
{
    rfc2544_report_t       *r = &RFC2544_reports[idx];
    rfc2544_report_state_t *s = &r->state;
    vtss_rc                rc = VTSS_RC_OK;

    s->msg[0] = '\0';

    if (r->throughput.common.status == RFC2544_TEST_STATUS_INACTIVE) {
        // Prepare internal state.
        memcpy(s->missing_frame_sizes, r->profile.common.selected_frame_sizes, sizeof(s->missing_frame_sizes));

        s->rate_step_kbps = RFC2544_rate_permille_to_kbps(s, r->profile.throughput.rate_step_permille);

        if (!RFC2544_frame_size_next(s)) {
            T_E("No frame sizes selected?!?");
            rc = VTSS_RC_ERROR; // Generic internal error.
            goto do_stop;
        }

        RFC2544_throughput_rate_init(r);

        // Start the TST traffic (no 1DM frames):
        s->dmm_interval_secs = 0;
        if ((rc = RFC2544_traffic_start(s, r->profile.throughput.trial_duration_secs * 1000)) != VTSS_RC_OK) {
            goto do_stop;
        }

        r->throughput.common.status = RFC2544_TEST_STATUS_EXECUTING;
        return (1000 * r->profile.throughput.trial_duration_secs);

    } else if (r->throughput.common.status == RFC2544_TEST_STATUS_EXECUTING) {
        if (s->dwelling) {
            // RFC2544_throughput_analyze() returns VTSS_RC_OK if the test passed or
            // if we need to apply a new rate with the same configuration. The new
            // rate is given by s->cur_rate_kbps.
            // It returns != VTSS_RC_OK if the test failed.
            if ((rc = RFC2544_throughput_analyze(r)) != VTSS_RC_OK) {
                r->throughput.pass[s->cur_frame_size] = FALSE;
                goto do_stop;
            }

            if (s->cur_rate_kbps == 0) {
                // This frame size is successfully completed. Goto next.
                r->throughput.pass[s->cur_frame_size] = TRUE;

                if (!RFC2544_frame_size_next(s)) {
                    // Done with test.
                    goto do_stop;
                } else {
                    // Re-initialize min, max, and current rates.
                    RFC2544_throughput_rate_init(r);

                    if ((rc = RFC2544_traffic_start(s, r->profile.throughput.trial_duration_secs * 1000)) != VTSS_RC_OK) {
                        goto do_stop;
                    }
                    return (1000 * r->profile.throughput.trial_duration_secs);
                }
            } else {
                // Try with another rate.
                if ((rc = RFC2544_traffic_start(s, r->profile.throughput.trial_duration_secs * 1000)) != VTSS_RC_OK) {
                    goto do_stop;
                }
                return (1000 * r->profile.throughput.trial_duration_secs);
            }
        } else {
            // We are not dwelling, which must mean that we're invoked because
            // the trial period has expired. Stop TST traffic and start dwelling.
            if ((rc = RFC2544_traffic_stop(s, r->profile.common.dwell_time_secs)) != VTSS_RC_OK) {
                goto do_stop;
            }

            return (1000 * r->profile.common.dwell_time_secs);
        }
    } else {
        T_E("Internal error");
        rc = VTSS_RC_ERROR; // Generic error
        goto do_stop;
    }

do_stop:
    r->throughput.common.rc = rc;
    r->throughput.common.status = rc == VTSS_RC_OK ? RFC2544_TEST_STATUS_PASSED : RFC2544_TEST_STATUS_FAILED;
    return 0;
}

/****************************************************************************/
// RFC2544_latency_run()
/****************************************************************************/
static u32 RFC2544_latency_run(u32 idx)
{
    rfc2544_report_t       *r = &RFC2544_reports[idx];
    rfc2544_report_state_t *s = &r->state;
    vtss_rc                rc = VTSS_RC_OK;

    s->msg[0] = '\0';

    if (r->latency.common.status == RFC2544_TEST_STATUS_INACTIVE) {
        // About to start. Prepare internal state.
        memcpy(s->missing_frame_sizes, r->profile.common.selected_frame_sizes, sizeof(s->missing_frame_sizes));

        if (!RFC2544_frame_size_next(s)) {
            T_E("No frame sizes selected?!?");
            rc = VTSS_RC_ERROR; // Generic internal error.
            goto do_stop;
        }

        // Here, we use the max rate found in the throughput test.
        if (!r->throughput.pass[s->cur_frame_size]) {
            T_E("Internal error: How to get here when corresponding TP test failed?");
            rc = VTSS_RC_ERROR;
            goto do_stop;
        }

        s->cur_rate_kbps = r->throughput.statistics[s->cur_frame_size].tx_rate_kbps - 200; // Run at max. throughput less 200 kbps to reserve B/W for 1DM frames.
        s->dmm_interval_secs = r->profile.latency.dmm_interval_secs;

        // Start the TST traffic and send a 1DM frame every dmm_interval_secs seconds (first sent after the interval has elapsed once, according to API).
        if ((rc = RFC2544_traffic_start(s, r->profile.latency.trial_duration_secs * 1000)) != VTSS_RC_OK) {
            goto do_stop;
        }

        r->latency.common.status = RFC2544_TEST_STATUS_EXECUTING;

        // Get invoked after trial_duration_secs seconds.
        return (1000 * r->profile.latency.trial_duration_secs);
    } else if (r->latency.common.status == RFC2544_TEST_STATUS_EXECUTING) {
        if (s->dwelling) {
            // TST & 1DM frames are now stopped, and it's time to read statistics.
            // RFC2544_latency_analyze() returns VTSS_RC_OK if the test passed.
            // It returns != VTSS_RC_OK if the test failed.
            if ((rc = RFC2544_latency_analyze(r)) != VTSS_RC_OK) {
                r->latency.pass[s->cur_frame_size] = FALSE;
                goto do_stop;
            }

            // This frame size is successfully completed. Goto next.
            r->latency.pass[s->cur_frame_size] = TRUE;

            if (!RFC2544_frame_size_next(s)) {
                // Done with test.
                s->dmm_interval_secs = 0; // Prevent next test from starting 1DM.
                goto do_stop;
            } else {
                // Not done yet. Use the max rate from the throughput test.
                if (!r->throughput.pass[s->cur_frame_size]) {
                    T_E("Internal error: How to get here when corresponding TP test failed?");
                    rc = VTSS_RC_ERROR;
                    goto do_stop;
                }

                s->cur_rate_kbps = r->throughput.statistics[s->cur_frame_size].tx_rate_kbps - 200; // Run at max. throughput less 200 kbps to reserve B/W for 1DM frames.

                if ((rc = RFC2544_traffic_start(s, r->profile.latency.trial_duration_secs * 1000)) != VTSS_RC_OK) {
                    goto do_stop;
                }
                return (1000 * r->profile.latency.trial_duration_secs);
            }
        } else {
            // Stop both TST and 1DM frames.
            if ((rc = RFC2544_traffic_stop(s, r->profile.common.dwell_time_secs)) != VTSS_RC_OK) {
                goto do_stop;
            }

            return (1000 * r->profile.common.dwell_time_secs);
        }
    } else {
        T_E("Internal error");
        rc = VTSS_RC_ERROR; // Generic error
        goto do_stop;
    }

do_stop:
    r->latency.common.rc = rc;
    r->latency.common.status = rc == VTSS_RC_OK ? RFC2544_TEST_STATUS_PASSED : RFC2544_TEST_STATUS_FAILED;
    return 0;
}

/****************************************************************************/
// RFC2544_frame_loss_run()
/****************************************************************************/
static u32 RFC2544_frame_loss_run(u32 idx)
{
    rfc2544_report_t       *r = &RFC2544_reports[idx];
    rfc2544_report_state_t *s = &r->state;
    vtss_rc                rc = VTSS_RC_OK;

    s->msg[0] = '\0';

    if (r->frame_loss.common.status == RFC2544_TEST_STATUS_INACTIVE) {
        // Prepare internal state.
        memcpy(s->missing_frame_sizes, r->profile.common.selected_frame_sizes, sizeof(s->missing_frame_sizes));

        if (!RFC2544_frame_size_next(s)) {
            T_E("No frame sizes selected?!?");
            rc = VTSS_RC_ERROR; // Generic internal error.
            goto do_stop;
        }

        s->trials_succeeded_cnt = 0;
        s->tail                 = NULL; // No statistics items yet for this frame size
        s->cur_rate_kbps        = RFC2544_rate_permille_to_kbps(s, r->profile.frame_loss.rate_max_permille);
        s->min_rate_kbps        = RFC2544_rate_permille_to_kbps(s, r->profile.frame_loss.rate_min_permille);
        s->rate_step_kbps       = RFC2544_rate_permille_to_kbps(s, r->profile.frame_loss.rate_step_permille);

        // Start the TST traffic (no 1DM frames):
        s->dmm_interval_secs = 0;
        if ((rc = RFC2544_traffic_start(s, r->profile.frame_loss.trial_duration_secs * 1000)) != VTSS_RC_OK) {
            goto do_stop;
        }

        r->frame_loss.common.status = RFC2544_TEST_STATUS_EXECUTING;
        return (1000 * r->profile.frame_loss.trial_duration_secs);

    } else if (r->frame_loss.common.status == RFC2544_TEST_STATUS_EXECUTING) {
        if (s->dwelling) {
            // RFC2544_frame_loss_analyze() returns VTSS_RC_OK if the test passed or
            // if we need to apply a new rate with the same configuration. The new
            // rate is given by s->cur_rate_kbps.
            // It returns != VTSS_RC_OK if the test failed.
            if ((rc = RFC2544_frame_loss_analyze(r)) != VTSS_RC_OK) {
                r->frame_loss.pass[s->cur_frame_size] = FALSE;
                goto do_stop;
            }

            if (s->cur_rate_kbps == 0) {
                // This frame size is successfully completed. Goto next.
                r->frame_loss.pass[s->cur_frame_size] = TRUE;

                if (!RFC2544_frame_size_next(s)) {
                    // Done with test.
                    goto do_stop;
                } else {
                    // Re-initialize
                    s->trials_succeeded_cnt = 0;
                    s->tail                 = NULL; // No statistics items yet for this frame size
                    s->cur_rate_kbps        = RFC2544_rate_permille_to_kbps(s, r->profile.frame_loss.rate_max_permille);

                    if ((rc = RFC2544_traffic_start(s, r->profile.frame_loss.trial_duration_secs * 1000)) != VTSS_RC_OK) {
                        goto do_stop;
                    }
                    return (1000 * r->profile.frame_loss.trial_duration_secs);
                }
            } else {
                // Try with another rate.
                if ((rc = RFC2544_traffic_start(s, r->profile.frame_loss.trial_duration_secs * 1000)) != VTSS_RC_OK) {
                    goto do_stop;
                }
                return (1000 * r->profile.frame_loss.trial_duration_secs);
            }
        } else {
            // We are not dwelling, which must mean that we're invoked because
            // the trial period has expired. Stop TST traffic and start dwelling.
            if ((rc = RFC2544_traffic_stop(s, r->profile.common.dwell_time_secs)) != VTSS_RC_OK) {
                goto do_stop;
            }

            return (1000 * r->profile.common.dwell_time_secs);
        }
    } else {
        T_E("Internal error");
        rc = VTSS_RC_ERROR; // Generic error
        goto do_stop;
    }

do_stop:
    r->frame_loss.common.rc = rc;
    r->frame_loss.common.status = rc == VTSS_RC_OK ? RFC2544_TEST_STATUS_PASSED : RFC2544_TEST_STATUS_FAILED;
    return 0;
}

/****************************************************************************/
// RFC2544_back_to_back_run()
/****************************************************************************/
static u32 RFC2544_back_to_back_run(u32 idx)
{
    rfc2544_report_t       *r = &RFC2544_reports[idx];
    rfc2544_report_state_t *s = &r->state;
    vtss_rc                rc = VTSS_RC_OK;

    s->msg[0] = '\0';

    if (r->back_to_back.common.status == RFC2544_TEST_STATUS_INACTIVE) {
        // About to start. Prepare internal state.
        memcpy(s->missing_frame_sizes, r->profile.common.selected_frame_sizes, sizeof(s->missing_frame_sizes));

        if (!RFC2544_frame_size_next(s)) {
            T_E("No frame sizes selected?!?");
            rc = VTSS_RC_ERROR; // Generic internal error.
            goto do_stop;
        }

        s->trials_succeeded_cnt = 0;
        s->tail                 = NULL; // No statistics items yet for this frame size
        s->cur_rate_kbps        = s->link_speed_mbps * 1000 - 200; // Run at line rate less 200 kbps.

        // Start the TST traffic (no 1DM frames):
        s->dmm_interval_secs = 0;
        if ((rc = RFC2544_traffic_start(s, r->profile.back_to_back.trial_duration_msecs)) != VTSS_RC_OK) {
            goto do_stop;
        }

        r->back_to_back.common.status = RFC2544_TEST_STATUS_EXECUTING;
        return r->profile.back_to_back.trial_duration_msecs;

    } else if (r->back_to_back.common.status == RFC2544_TEST_STATUS_EXECUTING) {
        if (s->dwelling) {
            // RFC2544_back_to_back_analyze() returns VTSS_RC_OK if the test passed or
            // if we need to re-apply a rate with the same configuration. If s->cur_rate_kbps
            // is non-zero, we should reapply the rate. If zero, go to next frame size.
            // It returns != VTSS_RC_OK if the test failed.
            if ((rc = RFC2544_back_to_back_analyze(r)) != VTSS_RC_OK) {
                r->back_to_back.pass[s->cur_frame_size] = FALSE;
                goto do_stop;
            }

            if (s->cur_rate_kbps == 0) {
                // This frame size is successfully completed. Goto next.
                r->back_to_back.pass[s->cur_frame_size] = TRUE;

                if (!RFC2544_frame_size_next(s)) {
                    // Done with test.
                    goto do_stop;
                } else {
                    // Not done yet. Run at line rate less 200 kbps.
                    // Re-initialize
                    s->trials_succeeded_cnt = 0;
                    s->tail                 = NULL; // No statistics items yet for this frame size
                    s->cur_rate_kbps        = s->link_speed_mbps * 1000 - 200;

                    if ((rc = RFC2544_traffic_start(s, r->profile.back_to_back.trial_duration_msecs)) != VTSS_RC_OK) {
                        goto do_stop;
                    }
                    return r->profile.back_to_back.trial_duration_msecs;
                }
            } else {
                // Next trial
                if ((rc = RFC2544_traffic_start(s, r->profile.back_to_back.trial_duration_msecs)) != VTSS_RC_OK) {
                    goto do_stop;
                }
                return r->profile.back_to_back.trial_duration_msecs;
            }
        } else {
            // We are not dwelling, which must mean that we're invoked because
            // the trial period has expired. Stop TST traffic and start dwelling.
            if ((rc = RFC2544_traffic_stop(s, r->profile.common.dwell_time_secs)) != VTSS_RC_OK) {
                goto do_stop;
            }

            return (1000 * r->profile.common.dwell_time_secs);
        }
    } else {
        T_E("Internal error");
        rc = VTSS_RC_ERROR; // Generic error
        goto do_stop;
    }

do_stop:
    r->back_to_back.common.rc = rc;
    r->back_to_back.common.status = rc == VTSS_RC_OK ? RFC2544_TEST_STATUS_PASSED : RFC2544_TEST_STATUS_FAILED;
    return 0;
}

/****************************************************************************/
// RFC2544_report_flash_write()
// The layout in flash is as follows:
// One rfc2544_report_flash_t structure followed by
// up to RFC2544_REPORT_CNT NULL-terminated strings containing the reports
// as text.
/****************************************************************************/
static void RFC2544_report_flash_write(void)
{
    u32                    required_flash_size_bytes, idx;
    ulong                  size;
    BOOL                   do_create = FALSE;
    rfc2544_report_flash_t *flash_report;
    i8                     *ptr;

    RFC2544_CRIT_ASSERT_LOCKED();

    // Compute the required size:
    required_flash_size_bytes = sizeof(rfc2544_report_flash_t);
    required_flash_size_bytes += ARRSZ(RFC2544_reports); // +1 for NULL-terminating byte of each string, even if empty.

    for (idx = 0; idx < ARRSZ(RFC2544_reports); idx++) {
        rfc2544_report_t *r = &RFC2544_reports[idx];
        if (r->report_as_txt) {
            required_flash_size_bytes += strlen(r->report_as_txt);
        }
    }

    if ((flash_report = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_REPORTS, &size)) == NULL) {
        T_W("Failed to open flash configuration");
        do_create = TRUE;
    } else if (size != required_flash_size_bytes) {
        // Perfectly normal to have a size different from the required, since the reports may vary in size.
        T_I("Size mismatch: cur = %u, req = %u", size, required_flash_size_bytes);
        do_create = TRUE;
    }

    if (do_create) {
        flash_report = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_REPORTS, required_flash_size_bytes);
    }

    if (flash_report == NULL) {
        T_E("Unable to create a section of %u bytes in flash", required_flash_size_bytes);
        return;
    }

    memset(flash_report, 0, required_flash_size_bytes);
    flash_report->version = RFC2544_FLASH_REPORT_VERSION;
    flash_report->struct_size = sizeof(rfc2544_report_flash_t);

    for (idx = 0; idx < ARRSZ(RFC2544_reports); idx++) {
        rfc2544_report_t *r = &RFC2544_reports[idx];

        flash_report->r[idx].report_len = (r->report_as_txt ? strlen(r->report_as_txt) : 0) + 1;

        if (r->report_as_txt != NULL) {
            flash_report->r[idx].info = RFC2544_reports[idx].info;
        } else {
            // Better not save an ongoing execution to flash.
        }
    }

    // Time to copy all the reports.
    ptr = (i8 *)flash_report + sizeof(rfc2544_report_flash_t);
    for (idx = 0; idx < ARRSZ(RFC2544_reports); idx++) {
        rfc2544_report_t *r = &RFC2544_reports[idx];
        if (r->report_as_txt) {
            strcpy(ptr, r->report_as_txt);
            ptr += flash_report->r[idx].report_len; // Includes terminating NULL.
        } else {
            *(ptr++) = '\0';
        }
    }

    // Sanity check
    if (ptr - required_flash_size_bytes != (i8 *)flash_report) {
        T_E("Internal Error: %p - %u != %p", ptr, required_flash_size_bytes, flash_report);
    }

    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_REPORTS);
}

/******************************************************************************/
// RFC2544_report_flash_read()
/******************************************************************************/
static void RFC2544_report_flash_read(void)
{
    rfc2544_report_flash_t *flash_report;
    ulong                  size;
    u32                    idx, expected_minimum_size, expected_size;
    i8                     *ptr;
    BOOL                   not_in_use_seen = FALSE;

    // Do not call misc_conf_read_use() here. The RFC2544 reports are saved to a block
    // in flash that is not killed by ICFG during silent upgrade.
    if ((flash_report = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_REPORTS, &size)) == NULL) {
        // Nothing more to do. We stick to the reports already in RAM (which already are all-zeroes, which is fine).
        return;
    }

    // The minimum size we can accept is ARRSZ(RFC2544_reports) empty reports:
    expected_minimum_size = sizeof(rfc2544_report_flash_t) + ARRSZ(RFC2544_reports) /* X empty strings coming after the binary descriptor */;
    if (size < expected_minimum_size) {
        T_W("Unexpected size (%u). Expected at least %u bytes", size, expected_minimum_size);
        return;
    }

    if (flash_report->version != RFC2544_FLASH_REPORT_VERSION) {
        T_W("Unknown version: %u. Expected %u. Not loading reports", flash_report->version, RFC2544_FLASH_REPORT_VERSION);
        return;
    }

    if (flash_report->struct_size != sizeof(rfc2544_report_flash_t)) {
        T_W("Unexpected struct size (%u). Expected %u", flash_report->struct_size, sizeof(rfc2544_report_flash_t));
        return;
    }

    expected_size = sizeof(rfc2544_report_flash_t);
    for (idx = 0; idx < ARRSZ(flash_report->r); idx++) {
        expected_size += flash_report->r[idx].report_len;
    }

    if (size != expected_size) {
        T_W("Unexpected size (%u). Expected exactly %u bytes", size, expected_size);
        return;
    }

    // Let ptr point to the first string after the binary data.
    ptr = (i8 *)flash_report + sizeof(rfc2544_report_flash_t);

    for (idx = 0; idx < ARRSZ(flash_report->r); idx++) {
        rfc2544_test_status_t status;
        rfc2544_report_t      *r  = &RFC2544_reports[idx];
        u32                   len = flash_report->r[idx].report_len;

        r->info = flash_report->r[idx].info;

        // Copy a few fields from the non-volatile part to the volatile part (needed
        // by rfc2544_mgmt_report_info_get()).
        strcpy(r->common.start_time, r->info.creation_time);
        r->common.status = r->info.status;

        if (len > 1) {

            // Invariant: All used reports must come first:
            if (not_in_use_seen) {
                T_W("An empty report has been seen");
                goto do_exit_with_error;
            }

            if (RFC2544_REPORT_EMPTY(r)) {
                T_W("Report length (%u) != 0, but report has no name", len);
                goto do_exit_with_error;
            }

            status = r->info.status;
            if (status != RFC2544_TEST_STATUS_CANCELLED && status != RFC2544_TEST_STATUS_PASSED && status != RFC2544_TEST_STATUS_FAILED) {
                T_W("Invalid status (%s)", rfc2544_mgmt_util_status_to_str(status));
                goto do_exit_with_error;
            }

            r->report_as_txt = VTSS_MALLOC(len); // Remember it includes the terminating NULL.
            if (r->report_as_txt == NULL) {
                T_W("Unable to allocate %u bytes", len);
                goto do_exit_with_error;
            }

            (void)strncpy(r->report_as_txt, ptr, len);

            // Check that the resulting string is NULL-terminated at the expected position
            // and that it doesn't contain a NULL before that (one could skip the first NULL
            // check, but if something is wrong, we might end up searching far beyond the allocated
            // string, and perhaps not end before an access violation is thrown).
            if (r->report_as_txt[len - 1] != '\0' || strlen(r->report_as_txt) + 1 != len) {
                T_W("Report string not terminated or NULL-terminated before the expected position");
                goto do_exit_with_error;
            }

            ptr += len;
        } else {
            if (!RFC2544_REPORT_EMPTY(r)) {
                T_W("Report has a name (%s), but it's reported to be empty", flash_report->r[idx].info.name);
                goto do_exit_with_error;
            }

            if (*(ptr++) != '\0') {
                T_W("Expected empty string for nonamed report");
                goto do_exit_with_error;
            }

            not_in_use_seen = TRUE;
        }
    }

    // Also check that no two reports have the same name.
    for (idx = 0; idx < ARRSZ(RFC2544_reports) - 1; idx++) {
        i8  *name = RFC2544_reports[idx].info.name;
        u32 idx2;

        if (name[0] == '\0') {
            // Done, due to the invariant that all used reports must come first.
            break;
        }

        for (idx2 = idx + 1; idx2 < ARRSZ(RFC2544_reports); idx2++) {
            i8 *name2 = RFC2544_reports[idx2].info.name;
            if (name2[0] == '\0') {
                break;
            }
            if (strcmp(name, name2) == 0) {
                T_W("Two reports with the same name (%s)", name);
                goto do_exit_with_error;
            }
        }
    }

    // If we (ever) get here, we've read the reports from flash successfully.
    return;

do_exit_with_error:
    // Something went wrong above. Gotta reset the reports back to defaults.
    // Here, we might have VTSS_MALLOC()ed some of the reports, so better
    // free them again.
    for (idx = 0; idx < ARRSZ(RFC2544_reports); idx++) {
        VTSS_FREE(RFC2544_reports[idx].report_as_txt);
    }
    memset(RFC2544_reports, 0, sizeof(RFC2544_reports));
}

/******************************************************************************/
// RFC2544_report_as_txt()
/******************************************************************************/
vtss_rc RFC2544_report_as_txt(rfc2544_report_get_state_t *s)
{
    rfc2544_frame_size_t       fs;
    rfc2544_test_status_t      status;
    rfc2544_statistics_t       *stat;
    rfc2544_statistics_list_t  *list;
    u32                        fs_bytes;
    vtss_rc                    rc;
    i8                         buf1[100], buf2[100];
    const i8                   *code_rev = misc_software_code_revision_txt();

    // From this point on, we must go through the do_exit label below to exit this function,
    // because we need to possibly free the string. It's up to the caller to possibly
    // dispose off the report.

#define RFC2544_DO_EXIT_RC(expr) {rc = (expr); if (rc != VTSS_RC_OK) {goto do_exit;}}

    // Report heading
    RFC2544_DO_EXIT_RC(RFC2544_print_header(s, "RFC2544 Conformance Test Suite"));

    // Software configuration
    RFC2544_DO_EXIT_RC(RFC2544_printf(s, "Software configuration:\n"));
    RFC2544_DO_EXIT_RC(RFC2544_print_line_item(s, "Version",    "%s", misc_software_version_txt()));
    RFC2544_DO_EXIT_RC(RFC2544_print_line_item(s, "Build date", "%s", misc_software_date_txt()));
    if (strlen(code_rev)) {
        RFC2544_DO_EXIT_RC(RFC2544_print_line_item(s, "Code revision", "%s", code_rev));
    }

    // Report configuration
    RFC2544_DO_EXIT_RC(RFC2544_printf(s, "\nReport configuration:\n"));
    RFC2544_DO_EXIT_RC(RFC2544_print_line_item(s, "Report name", "%s", s->report->info.name));
    RFC2544_DO_EXIT_RC(RFC2544_print_line_item(s, "Description", "%s", s->report->info.dscr));

    // Overall report status
    RFC2544_DO_EXIT_RC(RFC2544_print_status(s, "\nOverall execution status:\n", &s->report->common));

    // Common Profile settings
    RFC2544_DO_EXIT_RC(RFC2544_print_ch(s, '\n'));
    RFC2544_DO_EXIT_RC(RFC2544_print_profile_common(s));

    // Throughput test
    if ((s->report->profile.common.selected_tests & RFC2544_TEST_TYPE_THROUGHPUT) && s->report->throughput.common.status != RFC2544_TEST_STATUS_INACTIVE) {
        BOOL only_print_passed = FALSE, first = TRUE;

        // Print configuration, status, and possibly current activity
        RFC2544_DO_EXIT_RC(RFC2544_print_report_test_header(s, "Throughput", &s->report->throughput.common, RFC2544_print_profile_throughput));

        // Report
        status = s->report->throughput.common.status;
        if (status == RFC2544_TEST_STATUS_EXECUTING || status == RFC2544_TEST_STATUS_CANCELLING || status == RFC2544_TEST_STATUS_CANCELLED) {
            // In either of these cases, the "pass" flag is not necessarily valid (unless it's TRUE), because we don't know how far
            // the execution has reached or reached before cancelling.
            only_print_passed = TRUE;
        }

        for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
            if (s->report->profile.common.selected_frame_sizes[fs]) {
                stat = &s->report->throughput.statistics[fs];

                if (stat->tx_rate_kbps == 0) {
                    // Nothing more to do, since this has not been executed at all, and therefore
                    // subsequent frame sizes also haven't.
                    break;
                }

                if (!only_print_passed || s->report->throughput.pass[fs]) {
                    RFC2544_DO_EXIT_RC(RFC2544_print_report_tp_fl_bb(s, stat, fs, s->report->profile.throughput.pass_criterion_permille, 1000 * s->report->profile.throughput.trial_duration_secs, &first));
                }
            }
        }
    }

    // Latency test
    if ((s->report->profile.common.selected_tests & RFC2544_TEST_TYPE_LATENCY) && s->report->latency.common.status != RFC2544_TEST_STATUS_INACTIVE) {
        BOOL only_print_passed = FALSE, first = TRUE;

        // Print configuration, status, and possibly current activity
        RFC2544_DO_EXIT_RC(RFC2544_print_report_test_header(s, "Latency", &s->report->latency.common, RFC2544_print_profile_latency));

        // Report
        status = s->report->latency.common.status;
        if (status == RFC2544_TEST_STATUS_EXECUTING || status == RFC2544_TEST_STATUS_CANCELLING || status == RFC2544_TEST_STATUS_CANCELLED) {
            // In either of these cases, the "pass" flag is not necessarily valid (unless it's TRUE), because we don't know how far
            // the execution has reached or reached before cancelling.
            only_print_passed = TRUE;
        }

        for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
            if (s->report->profile.common.selected_frame_sizes[fs]) {
                stat = &s->report->latency.tst_statistics[fs];

                if (stat->tx_rate_kbps == 0) {
                    // Nothing more to do, since this has not been executed at all, and therefore
                    // subsequent frame sizes also haven't.
                    break;
                }

                if (!only_print_passed || s->report->latency.pass[fs]) {
                    if (first) {
                        RFC2544_DO_EXIT_RC(RFC2544_print_ch(s, '\n'));
                        RFC2544_DO_EXIT_RC(RFC2544_printf(s, "Frame   Tx     Tx         Rx         Min/Avg/Max       Min/Avg/Max       Status\n"));
                        RFC2544_DO_EXIT_RC(RFC2544_printf(s, "Size    Rate   Frames     Frames     Delay             Delay Var.\n"));
                        RFC2544_DO_EXIT_RC(RFC2544_printf(s, "[bytes] [Mbps]                       [usecs]           [usecs]\n"));
                        RFC2544_DO_EXIT_RC(RFC2544_printf(s, "------- ------ ---------- ---------- ----------------- ----------------- ------\n"));
                        first = FALSE;
                    }

                    // Print Tx rate with 1 decimal.
                    // Divide by 1001 to get to Mbps. Then multiply by 10 to get it printed correctly with 1 decimal.
                    fs_bytes = rfc2544_mgmt_util_frame_size_enum_to_number(fs);
                    (void)RFC2544_rate_to_str(buf1, stat->tx_frame_cnt, fs_bytes, s->report->profile.latency.trial_duration_secs * 1000, NULL);

                    RFC2544_DO_EXIT_RC(RFC2544_printf(s, "%7u %6s %10llu %10llu ",
                                                      fs_bytes,             // Frame size
                                                      buf1,                 // Tx Rate
                                                      stat->tx_frame_cnt,   // Tx Frames
                                                      stat->rx_frame_cnt)); // Rx Frames

                    // Delay, delay variation, and status.
                    RFC2544_DO_EXIT_RC(RFC2544_printf(s, "%17s %17s %s\n",
                                                      RFC2544_delay_to_str(buf1, &s->report->latency.delay[fs]),
                                                      RFC2544_delay_to_str(buf2, &s->report->latency.delay_variation[fs]),
                                                      s->report->latency.pass[fs] ? "PASS" : "FAIL"));
                }
            }
        }
    }

    // Frame loss test
    if ((s->report->profile.common.selected_tests & RFC2544_TEST_TYPE_FRAME_LOSS) && s->report->frame_loss.common.status != RFC2544_TEST_STATUS_INACTIVE) {
        BOOL first = TRUE;

        // Print configuration, status, and possibly current activity
        RFC2544_DO_EXIT_RC(RFC2544_print_report_test_header(s, "Frame Loss", &s->report->frame_loss.common, RFC2544_print_profile_frame_loss));

        // Report
        for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
            if (s->report->profile.common.selected_frame_sizes[fs]) {
                list = s->report->frame_loss.statistics_list[fs];

                while (list) {
                    stat = &list->stat;
                    RFC2544_DO_EXIT_RC(RFC2544_print_report_tp_fl_bb(s, stat, fs, 0, 1000 * s->report->profile.frame_loss.trial_duration_secs, &first));
                    list = list->next;
                }
            }
        }
    }

    // Back-to-back test
    if ((s->report->profile.common.selected_tests & RFC2544_TEST_TYPE_BACK_TO_BACK) && s->report->back_to_back.common.status != RFC2544_TEST_STATUS_INACTIVE) {
        BOOL first = TRUE;

        // Print configuration, status, and possibly current activity
        RFC2544_DO_EXIT_RC(RFC2544_print_report_test_header(s, "Back-to-back", &s->report->back_to_back.common, RFC2544_print_profile_back_to_back));

        // Report
        for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
            if (s->report->profile.common.selected_frame_sizes[fs]) {
                list = s->report->back_to_back.statistics_list[fs];

                while (list) {
                    stat = &list->stat;
                    RFC2544_DO_EXIT_RC(RFC2544_print_report_tp_fl_bb(s, stat, fs, 0, s->report->profile.back_to_back.trial_duration_msecs, &first));
                    list = list->next;
                }
            }
        }
    }

    // Final status, only printed when not currently executing.
    if (s->report->common.status == RFC2544_TEST_STATUS_CANCELLED ||
        s->report->common.status == RFC2544_TEST_STATUS_PASSED    ||
        s->report->common.status == RFC2544_TEST_STATUS_FAILED) {
        // Overall report status
        RFC2544_DO_EXIT_RC(RFC2544_print_ch(s, '\n'));
        RFC2544_DO_EXIT_RC(RFC2544_print_star_line(s, "Overall Result"));
        RFC2544_DO_EXIT_RC(RFC2544_print_line_item(s, "Ended at", "%s", strlen(s->report->common.end_time) > 0 ? s->report->common.end_time : (i8 *)"-"));
        RFC2544_DO_EXIT_RC(RFC2544_print_line_item(s, "Status",   "%s", rfc2544_mgmt_util_status_to_str(s->report->common.status)));
        RFC2544_DO_EXIT_RC(RFC2544_print_star_line(s, NULL));
    }

#undef RFC2544_DO_EXIT_RC

do_exit:
    if (rc != VTSS_RC_OK) {
        VTSS_FREE(s->txt);
        s->txt = NULL;
    }
    return rc;
}

/******************************************************************************/
// RFC2544_report_dispose_item()
/******************************************************************************/
static void RFC2544_report_dispose_item(rfc2544_report_frame_loss_and_back_to_back_t *item)
{
    rfc2544_frame_size_t fs;

    for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
        rfc2544_statistics_list_t *p = item->statistics_list[fs];

        while (p) {
            rfc2544_statistics_list_t *temp = p;
            p = p->next;
            VTSS_FREE(temp);
        }
    }
}

/******************************************************************************/
// RFC2544_report_dispose()
/******************************************************************************/
static void RFC2544_report_dispose(rfc2544_report_t *report)
{
    RFC2544_report_dispose_item(&report->frame_loss);
    RFC2544_report_dispose_item(&report->back_to_back);
    if (report->report_as_txt) {
        VTSS_FREE(report->report_as_txt);
    }
    memset(report, 0, sizeof(*report));
}

/****************************************************************************/
// RFC2544_report_flash_save()
// Converts a binary report to a textual and writes it to flash. The binary
// report is disposed afterwards.
/****************************************************************************/
static void RFC2544_report_flash_save(rfc2544_report_t *r)
{
    rfc2544_report_get_state_t s;
    rfc2544_report_common_t    common;
    rfc2544_report_info_t      info;
    vtss_rc                    rc;

    RFC2544_CRIT_ASSERT_LOCKED();

    memset(&s, 0, sizeof(s));
    s.report = r;

    // First check that it's not already in textual representation.
    if (r->report_as_txt != NULL) {
        T_E("Why is this report already present as text?");
        // It will get disposed in RFC2544_report_dispose() below,
        // so no need to free it here.
    }

    // Then convert the report to textual representation.
    if ((rc = RFC2544_report_as_txt(&s)) != VTSS_RC_OK) {
        T_E("Couldn't convert report to text (error=%s)", error_txt(rc));
        return;
    }

    common = r->common;
    info   = r->info;

    // Copy a few fields from the volatile part to the part saved to flash.
    strcpy(info.creation_time, common.start_time);
    info.status = common.status;

    // Now that we have converted it to text, we can safely dispose off the report.
    (void)RFC2544_report_dispose(r);

    r->report_as_txt = s.txt;
    r->info = info;

    // Even the common section of the volatile part of the report must be written
    // again for the sake of rfc2544_mgmt_report_info_get(), which always use
    // these fields.
    r->common = common;

    // And write the updated reports to flash.
    RFC2544_report_flash_write();
}

/****************************************************************************/
// RFC2544_thread()
/****************************************************************************/
static void RFC2544_thread(cyg_addrword_t data)
{
    u32 idx, next_timeout_ms = 0;

// For a given test to be runnable, the following must be fulfilled:
// 1) The previous test must return status PASSED (_s_ argument).
// 2) This test must be among the selected.
// 3) This test must either not have been invoked yet (INACTIVE)
//    or it must be EXECUTING.
#define RFC2544_RUNNING(_n_, _N_)                                      \
    ((r->profile.common.selected_tests & RFC2544_TEST_TYPE_ ## _N_) && \
      r->_n_.common.status == RFC2544_TEST_STATUS_EXECUTING)

// For a given test to be runnable, the following must be fulfilled:
// 1) The previous test must return last_status PASSED.
// 2) This test must be among the selected.
// 3) This test must either not have been invoked yet (INACTIVE)
//    or it must be EXECUTING.
#define RFC2544_RUNNABLE(_n_, _N_)                                     \
     (last_status == RFC2544_TEST_STATUS_PASSED                     && \
     (r->profile.common.selected_tests & RFC2544_TEST_TYPE_ ## _N_) && \
     (r->_n_.common.status == RFC2544_TEST_STATUS_INACTIVE          || \
      r->_n_.common.status == RFC2544_TEST_STATUS_EXECUTING))

// This macro runs a step of this test. If the test is about
// to begin, it gets timestamped. If it ended (successfully
// or not), it also gets timestamped.
// Afterwards, it updates last_status, which indicates
// the result of stepping this test case.
#define RFC2544_RUN(_n_) do {                                   \
    if (r->_n_.common.status == RFC2544_TEST_STATUS_INACTIVE) { \
        RFC2544_timestamp(&r->_n_.common, FALSE);               \
    }                                                           \
    next_timeout_ms = RFC2544_ ## _n_ ## _run(idx);             \
    last_status = r->_n_.common.status;                         \
    if (last_status != RFC2544_TEST_STATUS_EXECUTING &&         \
        last_status != RFC2544_TEST_STATUS_PASSED    &&         \
        last_status != RFC2544_TEST_STATUS_FAILED) {            \
        /* The result of a run can only be */                   \
        /* Executing, passed or failed.    */                   \
        T_E("Invalid state coming out of " # _n_);              \
    } else if (last_status == RFC2544_TEST_STATUS_PASSED ||     \
               last_status == RFC2544_TEST_STATUS_FAILED) {     \
        RFC2544_timestamp(&r->_n_.common, TRUE);                \
    }                                                           \
} while (0)

    // First time we exit our mutex (this confuses Lint)
    /*lint --e{455} */
    /*lint -esym(457, RFC2544_crit) */
    RFC2544_CRIT_EXIT();

    while (1) {
        // Currently, this works with at most one test suite running at a time.
#if RFC2544_CONCURRENTLY_EXECUTING_MAX != 1
#error "Change timeout determination"
#endif
        if (next_timeout_ms) {
            (void)cyg_flag_timed_wait(&RFC2544_wakeup_thread_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, cyg_current_time() + VTSS_OS_MSEC2TICK(next_timeout_ms));
        } else {
            (void)cyg_flag_wait(&RFC2544_wakeup_thread_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
        }

        RFC2544_CRIT_ENTER();
        for (idx = 0; idx < ARRSZ(RFC2544_reports); idx++) {
            rfc2544_report_t      *r             = &RFC2544_reports[idx];
            rfc2544_test_status_t overall_status = r->common.status;

            if (RFC2544_REPORT_EMPTY(r)) {
                // Nothing to do.
                continue;
            }

            if (overall_status == RFC2544_TEST_STATUS_CANCELLING) {

                // Check to see if there's anything to be done for the substates.
                if (RFC2544_RUNNING(throughput, THROUGHPUT)) {
                    r->throughput.common.status = RFC2544_TEST_STATUS_CANCELLED;
                } else if (RFC2544_RUNNING(latency, LATENCY)) {
                    r->latency.common.status = RFC2544_TEST_STATUS_CANCELLED;
                } else if (RFC2544_RUNNING(frame_loss, FRAME_LOSS)) {
                    r->frame_loss.common.status = RFC2544_TEST_STATUS_CANCELLED;
                } else if (RFC2544_RUNNING(back_to_back, BACK_TO_BACK)) {
                    r->back_to_back.common.status = RFC2544_TEST_STATUS_CANCELLED;
                }

                (void)RFC2544_mep_delete(r);

                // Here, we've definitely cancelled all activity.
                RFC2544_timestamp(&r->common, TRUE);
                r->common.status = RFC2544_TEST_STATUS_CANCELLED;

                // Time to save report.
                RFC2544_report_flash_save(r);
            } else if (overall_status == RFC2544_TEST_STATUS_EXECUTING) {
                rfc2544_test_status_t last_status = RFC2544_TEST_STATUS_PASSED;

                // Check to see if there's anything to be done for the substates.
                if (RFC2544_RUNNABLE(throughput, THROUGHPUT)) {
                    RFC2544_RUN(throughput);
                }

                if (RFC2544_RUNNABLE(latency, LATENCY)) {
                    RFC2544_RUN(latency);
                }

                if (RFC2544_RUNNABLE(frame_loss, FRAME_LOSS)) {
                    RFC2544_RUN(frame_loss);
                }

                if (RFC2544_RUNNABLE(back_to_back, BACK_TO_BACK)) {
                    RFC2544_RUN(back_to_back);
                }

                // When getting here, last_status is either EXECUTING, PASSED, or FAILED.
                if (last_status == RFC2544_TEST_STATUS_PASSED || last_status == RFC2544_TEST_STATUS_FAILED) {
                    vtss_rc rc = RFC2544_mep_delete(r);

                    if (last_status == RFC2544_TEST_STATUS_PASSED && rc != VTSS_RC_OK) {
                        // Let someone know that we couldn't delete the MEP
                        r->common.rc = rc;
                        r->common.status = RFC2544_TEST_STATUS_FAILED;
                    } else {
                        r->common.rc = VTSS_RC_OK; // Not used in top-level status. If the test failed, it's conveyed in the relevant sub-test.
                        r->common.status = last_status;
                    }
                    RFC2544_timestamp(&r->common, TRUE);
                    RFC2544_report_flash_save(r);
                } else if (last_status != RFC2544_TEST_STATUS_EXECUTING) {
                    T_E("Invalid state");
                }
            }
        }

        RFC2544_CRIT_EXIT();
    }

#undef RFC2544_RUNNING
#undef RFC2544_RUNNABLE
#undef RFC2544_RUN
}

/******************************************************************************/
// RFC2544_profile_throughput_defaults()
/******************************************************************************/
static void RFC2544_profile_common_defaults(rfc2544_profile_common_t *common)
{
    rfc2544_frame_size_t i;

    memset(common, 0, sizeof(*common));

    // Default to include all frame sizes but 9600.
    for (i = 0; i < RFC2544_FRAME_SIZE_CNT; i++) {
        if (i != RFC2544_FRAME_SIZE_9600) {
            common->selected_frame_sizes[i] = TRUE;
        }
    }

    common->meg_level       = RFC2544_COMMON_MEG_LEVEL_DEFAULT;
    common->dmac.addr[5]    = 1; // Make it non-NULL, because the MEP module doesn't support it.
    common->dwell_time_secs = RFC2544_COMMON_DWELL_TIME_DEFAULT;
    common->selected_tests  = RFC2544_TEST_TYPE_THROUGHPUT | RFC2544_TEST_TYPE_LATENCY;
}

/******************************************************************************/
// RFC2544_profile_throughput_defaults()
/******************************************************************************/
static void RFC2544_profile_throughput_defaults(rfc2544_profile_throughput_t *throughput)
{
    memset(throughput, 0, sizeof(*throughput));
    throughput->trial_duration_secs = RFC2544_THROUGHPUT_TRIAL_DURATION_DEFAULT;
    throughput->rate_min_permille   = RFC2544_THROUGHPUT_RATE_MIN_DEFAULT;
    throughput->rate_max_permille   = RFC2544_THROUGHPUT_RATE_MAX_DEFAULT;
    throughput->rate_step_permille  = RFC2544_THROUGHPUT_RATE_STEP_DEFAULT;
}

/******************************************************************************/
// RFC2544_profile_latency_defaults()
/******************************************************************************/
static void RFC2544_profile_latency_defaults(rfc2544_profile_latency_t *latency)
{
    memset(latency, 0, sizeof(*latency));
    latency->trial_duration_secs = RFC2544_LATENCY_TRIAL_DURATION_DEFAULT;
    latency->dmm_interval_secs   = RFC2544_LATENCY_DMM_INTERVAL_DEFAULT;
}

/******************************************************************************/
// RFC2544_profile_frame_loss_defaults()
/******************************************************************************/
static void RFC2544_profile_frame_loss_defaults(rfc2544_profile_frame_loss_t *frame_loss)
{
    memset(frame_loss, 0, sizeof(*frame_loss));
    frame_loss->trial_duration_secs = RFC2544_FRAME_LOSS_TRIAL_DURATION_DEFAULT;
    frame_loss->rate_min_permille   = RFC2544_FRAME_LOSS_RATE_MIN_DEFAULT;
    frame_loss->rate_max_permille   = RFC2544_FRAME_LOSS_RATE_MAX_DEFAULT;
    frame_loss->rate_step_permille  = RFC2544_FRAME_LOSS_RATE_STEP_DEFAULT;
}

/******************************************************************************/
// RFC2544_profile_back_to_back_defaults()
/******************************************************************************/
static void RFC2544_profile_back_to_back_defaults(rfc2544_profile_back_to_back_t *back_to_back)
{
    memset(back_to_back, 0, sizeof(*back_to_back));
    back_to_back->trial_duration_msecs = RFC2544_BACK_TO_BACK_TRIAL_DURATION_DEFAULT;
    back_to_back->trial_cnt            = RFC2544_BACK_TO_BACK_TRIAL_CNT_DEFAULT;
}

/******************************************************************************/
// RFC2544_profile_defaults()
/******************************************************************************/
static void RFC2544_profile_defaults(rfc2544_profile_t *profile)
{
    // Common defaults
    RFC2544_profile_common_defaults(&profile->common);

    // Throughput defaults
    RFC2544_profile_throughput_defaults(&profile->throughput);

    // Latency defaults
    RFC2544_profile_latency_defaults(&profile->latency);

    // Frame Loss defaults
    RFC2544_profile_frame_loss_defaults(&profile->frame_loss);

    // Back-to-back defaults
    RFC2544_profile_back_to_back_defaults(&profile->back_to_back);
}

/******************************************************************************/
// RFC2544_profile_chk()
/******************************************************************************/
vtss_rc RFC2544_profile_chk(rfc2544_profile_t *profile)
{
    rfc2544_frame_size_t frm_size;
    BOOL                 one_specified = FALSE;
    u32                  i, len;

#define RFC2544_MIN_MAX_CHECK(_v_, _m_)                                       \
    if ((_v_) < RFC2544_ ## _m_ ## _MIN || (_v_) > RFC2544_ ## _m_ ## _MAX) { \
        return RFC2544_ERROR_ ## _m_;                                         \
    }

    // Check profile name. It must consist of ASCII chars, only, and be NULL-terminated.
    if ((len = strlen(profile->common.name)) >= RFC2544_PROFILE_NAME_LEN) {
        return RFC2544_ERROR_PROFILE_NAME_TOO_LONG;
    }
    if (len == 0) {
        return RFC2544_ERROR_PROFILE_NAME;
    }
    for (i = 0; i < len; i++) {
        u8 c = profile->common.name[i];
        if (c < 33 || c > 126) {
            return RFC2544_ERROR_PROFILE_NAME;
        }
    }

    // Check common section.
    if (profile->common.egress_port_no >= port_isid_port_count(VTSS_ISID_LOCAL) || port_isid_port_no_is_stack(VTSS_ISID_LOCAL, profile->common.egress_port_no)) {
        return RFC2544_ERROR_EGRESS_PORT;
    }

    if (profile->common.meg_level > RFC2544_COMMON_MEG_LEVEL_MAX) {
        return RFC2544_ERROR_COMMON_MEG_LEVEL;
    }

    // The MEP module doesn't support the NULL MAC address, so neither can we.
    for (i = 0; i < ARRSZ(profile->common.dmac.addr); i++) {
        if (profile->common.dmac.addr[i]) {
            one_specified = TRUE;
        }
    }

    if (!one_specified) {
        return RFC2544_ERROR_COMMON_DMAC;
    }

    one_specified = FALSE;
    for (frm_size = 0; frm_size < RFC2544_FRAME_SIZE_CNT; frm_size++) {
        if (profile->common.selected_frame_sizes[frm_size] != FALSE) {
            one_specified = TRUE;
            break;
        }
    }

    if (!one_specified) {
        return RFC2544_ERROR_COMMON_FRAME_SIZE;
    }

    RFC2544_MIN_MAX_CHECK(profile->common.dwell_time_secs, COMMON_DWELL_TIME);

    if (profile->common.selected_tests == 0) {
        return RFC2544_ERROR_COMMON_SELECTED_TESTS;
    }

    // Check throughput section.
    if (profile->common.selected_tests & RFC2544_TEST_TYPE_THROUGHPUT) {
        RFC2544_MIN_MAX_CHECK(profile->throughput.trial_duration_secs, THROUGHPUT_TRIAL_DURATION);
        RFC2544_MIN_MAX_CHECK(profile->throughput.rate_min_permille,   THROUGHPUT_RATE_MIN);
        RFC2544_MIN_MAX_CHECK(profile->throughput.rate_max_permille,   THROUGHPUT_RATE_MAX);
        RFC2544_MIN_MAX_CHECK(profile->throughput.rate_step_permille,  THROUGHPUT_RATE_STEP);

        if (profile->throughput.rate_min_permille > profile->throughput.rate_max_permille) {
            return RFC2544_ERROR_THROUGHPUT_RATE_MIN_MAX;
        }

        if (profile->throughput.pass_criterion_permille > RFC2544_THROUGHPUT_PASS_CRITERION_MAX) {
            return RFC2544_ERROR_THROUGHPUT_PASS_CRITERION;
        }

    } else {
        RFC2544_profile_throughput_defaults(&profile->throughput);
    }

    // Check latency section.
    if (profile->common.selected_tests & RFC2544_TEST_TYPE_LATENCY) {
        // When latency test is selected, so must the throughput test.
        if ((profile->common.selected_tests & RFC2544_TEST_TYPE_THROUGHPUT) == 0) {
            return RFC2544_ERROR_COMMON_THROUGHPUT_WHEN_LATENCY;
        }

        RFC2544_MIN_MAX_CHECK(profile->latency.trial_duration_secs, LATENCY_TRIAL_DURATION);
        RFC2544_MIN_MAX_CHECK(profile->latency.dmm_interval_secs,   LATENCY_DMM_INTERVAL);

        if (profile->latency.trial_duration_secs <= profile->latency.dmm_interval_secs) {
            return RFC2544_ERROR_LATENCY_TRIAL_DURATION_DMM_INTERVAL;
        }

        if (profile->latency.pass_criterion_permille > RFC2544_LATENCY_PASS_CRITERION_MAX) {
            return RFC2544_ERROR_LATENCY_PASS_CRITERION;
        }
    } else {
        RFC2544_profile_latency_defaults(&profile->latency);
    }

    // Check frame loss section.
    if (profile->common.selected_tests & RFC2544_TEST_TYPE_FRAME_LOSS) {
        RFC2544_MIN_MAX_CHECK(profile->frame_loss.trial_duration_secs, FRAME_LOSS_TRIAL_DURATION);
        RFC2544_MIN_MAX_CHECK(profile->frame_loss.rate_min_permille,   FRAME_LOSS_RATE_MIN);
        RFC2544_MIN_MAX_CHECK(profile->frame_loss.rate_max_permille,   FRAME_LOSS_RATE_MAX);
        RFC2544_MIN_MAX_CHECK(profile->frame_loss.rate_step_permille,  FRAME_LOSS_RATE_STEP);

        // Notice the equal sign
        if (profile->frame_loss.rate_min_permille >= profile->frame_loss.rate_max_permille) {
            return RFC2544_ERROR_FRAME_LOSS_RATE_MIN_MAX;
        }

        if ((profile->frame_loss.rate_max_permille - profile->frame_loss.rate_min_permille) < profile->frame_loss.rate_step_permille) {
            return RFC2544_ERROR_FRAME_LOSS_STEP_VS_MAX_MINUS_MIN;
        }
    } else {
        RFC2544_profile_frame_loss_defaults(&profile->frame_loss);
    }

    // Check back-to-back section.
    if (profile->common.selected_tests & RFC2544_TEST_TYPE_BACK_TO_BACK) {
        RFC2544_MIN_MAX_CHECK(profile->back_to_back.trial_duration_msecs, BACK_TO_BACK_TRIAL_DURATION);
        RFC2544_MIN_MAX_CHECK(profile->back_to_back.trial_cnt,            BACK_TO_BACK_TRIAL_CNT);
    } else {
        RFC2544_profile_back_to_back_defaults(&profile->back_to_back);
    }

    return VTSS_RC_OK;
#undef RFC2544_MIN_MAX_CHECK
}

/******************************************************************************/
// RFC2544_profile_idx_get()
/******************************************************************************/
static u32 RFC2544_profile_idx_get(const i8 *const name)
{
    u32 idx;
    for (idx = 0; idx < ARRSZ(RFC2544_profiles); idx++) {
        if (strcmp(RFC2544_profiles[idx].common.name, name) == 0) {
            return idx;
        }
    }

    return RFC2544_PROFILE_IDX_NONE;
}

/******************************************************************************/
// RFC2544_report_idx_get()
/******************************************************************************/
static u32 RFC2544_report_idx_get(i8 *name)
{
    u32 idx;
    for (idx = 0; idx < ARRSZ(RFC2544_reports); idx++) {
        if (strcmp(RFC2544_reports[idx].info.name, name) == 0) {
            return idx;
        }
    }

    return RFC2544_REPORT_IDX_NONE;
}

/******************************************************************************/
// RFC2544_profile_idx_alloc()
// Returning the first free entry. The invariant is that the RFC2544_profiles[]
// array is filled from index 0, so the first free index also indicates the
// number of profiles defined (i.e. no profiles at or after the returned index
// are in use).
// Only RFC2544_profile_idx_free() needs to be aware of this, and shuffle around
// in case an entry after the freed is deleted. This method is fine as long
// as the management functions are indexed by name rather than by number, and
// as long as a copy of a profile is made when starting executing a profile.
/******************************************************************************/
static u32 RFC2544_profile_idx_alloc(void)
{
    u32 idx;

    RFC2544_CRIT_ASSERT_LOCKED();

    for (idx = 0; idx < ARRSZ(RFC2544_profiles); idx++) {
        if (RFC2544_profiles[idx].common.name[0] == '\0') {
            return idx;
        }
    }

    return RFC2544_PROFILE_IDX_NONE;
}

/******************************************************************************/
// RFC2544_profile_idx_free()
/******************************************************************************/
static u32 RFC2544_profile_idx_free(u32 idx)
{
    u32 iter;

    RFC2544_CRIT_ASSERT_LOCKED();

    // In order to maintain the order by which profiles are added, we move
    // all profiles down one. See also comment in RFC2544_profile_idx_alloc().
    for (iter = idx + 1; iter < ARRSZ(RFC2544_profiles) && RFC2544_profiles[iter].common.name[0] != '\0'; iter++) {
        RFC2544_profiles[idx++] = RFC2544_profiles[iter];
    }

    RFC2544_profiles[iter - 1].common.name[0] = '\0'; // This will be overwritten by caller in just a sec, but better safe than sorry.
    return iter - 1; // Return the new index to overwrite with the deleted profile.
}

/******************************************************************************/
// RFC2544_report_chk()
/******************************************************************************/
vtss_rc RFC2544_report_chk(i8 *report_name, i8 *report_dscr)
{
    u32 i, len;

    // Check report name. It must consist of ASCII chars, only, and be NULL-terminated.
    if ((len = strlen(report_name)) >= RFC2544_REPORT_NAME_LEN) {
        return RFC2544_ERROR_REPORT_NAME_TOO_LONG;
    }
    if (len == 0) {
        return RFC2544_ERROR_REPORT_NAME;
    }
    for (i = 0; i < len; i++) {
        u8 c = report_name[i];
        if (c < 33 || c > 126) {
            return RFC2544_ERROR_REPORT_NAME;
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_report_dyn_list_cp()
/******************************************************************************/
static vtss_rc RFC2544_report_dyn_list_cp(rfc2544_report_frame_loss_and_back_to_back_t *dst, rfc2544_report_frame_loss_and_back_to_back_t *src)
{
    rfc2544_statistics_list_t *iter, *prev_item, *new_item;
    rfc2544_frame_size_t      fs;

    for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
        iter = src->statistics_list[fs];
        prev_item = NULL;
        while (iter) {
            if ((new_item = VTSS_MALLOC(sizeof(*new_item))) == NULL) {
                return RFC2544_ERROR_OUT_OF_MEMORY;
            }
            *new_item = *iter;
            new_item->next = NULL;
            if (prev_item) {
                prev_item->next = new_item;
            } else {
                dst->statistics_list[fs] = new_item;
            }
            prev_item = new_item;
            iter = iter->next;
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// RFC2544_report_idx_alloc()
// The invariant is that the RFC2544_reports[] array is filled from index 0.
// Reports are stored in chronological order (oldest first). When allocating
// a new report, a possible free entry is used first.
// If there are no free entries, all existing reports are moved one down and
// the last entry is returned.
/******************************************************************************/
static u32 RFC2544_report_idx_alloc(void)
{
    u32 idx;

    RFC2544_CRIT_ASSERT_LOCKED();

    for (idx = 0; idx < ARRSZ(RFC2544_reports); idx++) {
        if (RFC2544_REPORT_EMPTY(&RFC2544_reports[idx])) {
            return idx;
        }
    }

    // If we get here, there were no free entries.

    // Get rid of index 0.
    RFC2544_report_dispose(&RFC2544_reports[0]);

    // And move the remaining down one position
    for (idx = 1; idx < ARRSZ(RFC2544_reports); idx++) {
        RFC2544_reports[idx - 1] = RFC2544_reports[idx];
    }

    idx = ARRSZ(RFC2544_reports) - 1;

    // Better safe than sorry
    memset(&RFC2544_reports[idx], 0, sizeof(RFC2544_reports[idx]));

    return idx;
}

/******************************************************************************/
// RFC2544_report_idx_free()
/******************************************************************************/
static void RFC2544_report_idx_free(u32 idx)
{
    RFC2544_CRIT_ASSERT_LOCKED();

    // Clear report.
    RFC2544_report_dispose(&RFC2544_reports[idx]);

    idx++; // Start at the next index.

    // In order to maintain the order by which reports are added, we move
    // all reports down one. See also comment in RFC2544_report_idx_alloc().
    while (idx < ARRSZ(RFC2544_reports) && !RFC2544_REPORT_EMPTY(&RFC2544_reports[idx])) {
        RFC2544_reports[idx - 1] = RFC2544_reports[idx];
        idx++;
    }

    idx--;

    // idx now points to the last one moved down one position (or in case there
    // was only one report or the one we deleted was the last in the list)
    // Clear last entry we copied from (if at all we copied; if we didn't,
    // we simply re-clear the idx that was just cleared by RFC2544_report_dispose()
    memset(&RFC2544_reports[idx], 0, sizeof(RFC2544_reports[idx]));
}

/******************************************************************************/
// RFC2544_profile_flash_write()
/******************************************************************************/
static void RFC2544_profile_flash_write(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    rfc2544_profile_flash_t *flash_cfg;

    RFC2544_CRIT_ASSERT_LOCKED();

    if ((flash_cfg = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_PROFILES, NULL)) == NULL) {
        T_W("Failed to open flash configuration");
    } else {
        flash_cfg->version = RFC2544_FLASH_CFG_VERSION;
        memcpy(flash_cfg->profiles, RFC2544_profiles, sizeof(flash_cfg->profiles));
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_PROFILES);
    }
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/******************************************************************************/
// RFC2544_profile_flash_read()
// Read/create and activate configuration.
/******************************************************************************/
static void RFC2544_profile_flash_read(BOOL create)
{
    rfc2544_profile_flash_t *flash_cfg;
    BOOL                    do_create;
    ulong                   size;
    u32                     idx;

    if (misc_conf_read_use()) {
        // Open or create configuration block
        if ((flash_cfg = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_PROFILES, &size)) == NULL || size != sizeof(*flash_cfg)) {
            T_W("conf_sec_open() failed or size mismatch, creating defaults");
            flash_cfg = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_PROFILES, sizeof(*flash_cfg));
            do_create = TRUE;
        } else if (flash_cfg->version != RFC2544_FLASH_CFG_VERSION) {
            T_W("Version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        flash_cfg = NULL;
        do_create = TRUE;
    }

    RFC2544_CRIT_ENTER();

    if (!do_create && flash_cfg) {
        // Check the current values read from flash.
        BOOL not_in_use_seen = FALSE;

        for (idx = 0; idx < ARRSZ(flash_cfg->profiles); idx++) {
            if (flash_cfg->profiles[idx].common.name[0] != '\0' && not_in_use_seen) {
                // Invariant that all in_use profiles come first does not hold.
                do_create = TRUE;
                break;
            }

            if (flash_cfg->profiles[idx].common.name[0] == '\0') {
                not_in_use_seen = TRUE;
                RFC2544_profile_defaults(&flash_cfg->profiles[idx]);
            } else if (RFC2544_profile_chk(&flash_cfg->profiles[idx]) != VTSS_RC_OK) {
                do_create = TRUE;
                break;
            }
        }

        // Also check that no two profiles have the same name.
        for (idx = 0; do_create == FALSE && idx < ARRSZ(flash_cfg->profiles) - 1; idx++) {
            i8  *name = flash_cfg->profiles[idx].common.name;
            u32 idx2;

            if (name[0] == '\0') {
                break;
            }

            for (idx2 = idx + 1; idx2 < ARRSZ(flash_cfg->profiles); idx2++) {
                i8 *name2 = flash_cfg->profiles[idx2].common.name;
                if (name2[0] == '\0') {
                    break;
                }
                if (strcmp(name, name2) == 0) {
                    do_create = TRUE;
                    break;
                }
            }
        }
    }

    if (do_create) {
        // Gotta create defaults.
        for (idx = 0; idx < ARRSZ(RFC2544_profiles); idx++) {
            RFC2544_profile_defaults(&RFC2544_profiles[idx]);
        }
    } else if (flash_cfg) { // Quiet Lint
        memcpy(RFC2544_profiles, flash_cfg->profiles, sizeof(RFC2544_profiles));
    }

    RFC2544_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash
    if (flash_cfg) {
        flash_cfg->version = RFC2544_FLASH_CFG_VERSION;
        memcpy(flash_cfg->profiles, RFC2544_profiles, sizeof(flash_cfg->profiles));
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_RFC2544_PROFILES);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/******************************************************************************/
// RFC2544_in_progress_cnt()
/******************************************************************************/
static u32 RFC2544_in_progress_cnt(void)
{
    u32 idx, cnt = 0;

    RFC2544_CRIT_ASSERT_LOCKED();

    for (idx = 0; idx < ARRSZ(RFC2544_reports); idx++) {
        if (RFC2544_REPORT_EMPTY(&RFC2544_reports[idx])) {
            // Invariant says that all active reports are first
            // in the RFC2544_reports[] array.
            break;
        }
        if (RFC2544_reports[idx].common.status == RFC2544_TEST_STATUS_EXECUTING) {
            cnt++;
        }
    }

    return cnt;
}

#if defined(VTSS_SW_OPTION_ICFG)
/******************************************************************************/
// RFC2544_icfg_query_func()
// This is the function that synthesizes the running configuration.
/******************************************************************************/
static vtss_rc RFC2544_icfg_query_func(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    rfc2544_profile_t profile, default_profile;
    vtss_rc           rc;

    if ((rc = rfc2544_mgmt_profile_get(req->instance_id.string, &profile)) != VTSS_RC_OK) {
        T_E("Unable to find profile %s", req->instance_id.string);
        return rc;
    }

    RFC2544_profile_defaults(&default_profile);

    // The following function is located together with the rest of the icli stuff in rfc2544.icli.
    extern vtss_rc RFC2544_icfg_synthesize(vtss_icfg_query_result_t *result, BOOL all, rfc2544_profile_t *profile, rfc2544_profile_t *default_profile);
    return RFC2544_icfg_synthesize(result, req->all_defaults, &profile, &default_profile);
}
#endif /* VTSS_SW_OPTION_ICFG */

/******************************************************************************/
//
// PUBLIC FUNCTIONS
//
/******************************************************************************/

/******************************************************************************/
// rfc2544_mgmt_profile_names_get()
/******************************************************************************/
vtss_rc rfc2544_mgmt_profile_names_get(i8 *name)
{
    vtss_rc rc = VTSS_RC_OK;

    if (name == NULL) {
        return RFC2544_ERROR_ARGUMENT;
    }

    RFC2544_CRIT_ENTER();

    if (name[0] == '\0') {
        // User wants to get the first profile name.
        strcpy(name, RFC2544_profiles[0].common.name);
    } else {
        u32 idx = RFC2544_profile_idx_get(name);

        if (idx == RFC2544_PROFILE_IDX_NONE) {
            // No such profile. Stop.
            rc = RFC2544_ERROR_NO_SUCH_PROFILE;
            name[0] = '\0';
        } else if (idx == ARRSZ(RFC2544_profiles) - 1 || RFC2544_profiles[idx + 1].common.name[0] == '\0') {
            // Previous entry was the last entry in the table, or the next entry is not in use.
            name[0] = '\0';
        } else {
            strcpy(name, RFC2544_profiles[idx + 1].common.name);
        }
    }

    RFC2544_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// rfc2544_mgmt_profile_get()
/******************************************************************************/
vtss_rc rfc2544_mgmt_profile_get(const i8 *const name, rfc2544_profile_t *profile)
{
    vtss_rc rc;
    u32     idx;

    if (profile == NULL) {
        return RFC2544_ERROR_ARGUMENT;
    }

    if (name == NULL || name[0] == '\0') {
        RFC2544_profile_defaults(profile);
        return VTSS_RC_OK;
    }

    RFC2544_CRIT_ENTER();

    if ((idx = RFC2544_profile_idx_get(name)) != RFC2544_PROFILE_IDX_NONE) {
        *profile = RFC2544_profiles[idx];
        rc = VTSS_RC_OK;
    } else {
        rc = RFC2544_ERROR_NO_SUCH_PROFILE;
    }

    RFC2544_CRIT_EXIT();

    return rc;
}

/******************************************************************************/
// rfc2544_mgmt_profile_set()
/******************************************************************************/
vtss_rc rfc2544_mgmt_profile_set(i8 *old_name, rfc2544_profile_t *profile)
{
    vtss_rc rc     = VTSS_RC_OK;
    BOOL    create = old_name == NULL || old_name[0] == '\0';
    u32     idx;

    if (profile == NULL) {
        return RFC2544_ERROR_ARGUMENT;
    }

    if (profile->common.name[0] != '\0' && (rc = RFC2544_profile_chk(profile)) != VTSS_RC_OK) {
        return rc;
    }

    RFC2544_CRIT_ENTER();

    if (create) {
        // Creating new profile.
        if (profile->common.name[0] == '\0') {
            // Attempting to create a profile with an empty name
            rc = RFC2544_ERROR_PROFILE_NAME;
            goto do_exit;
        }

        if (RFC2544_profile_idx_get(profile->common.name) != RFC2544_PROFILE_IDX_NONE) {
            rc = RFC2544_ERROR_PROFILE_ALREADY_EXISTS;
            goto do_exit;
        }

        if ((idx = RFC2544_profile_idx_alloc()) == RFC2544_PROFILE_IDX_NONE) {
            rc = RFC2544_ERROR_OUT_OF_PROFILES;
            goto do_exit;
        }
    } else {
        // Updating or deleting an existing.
        if ((idx = RFC2544_profile_idx_get(old_name)) == RFC2544_PROFILE_IDX_NONE) {
            rc = RFC2544_ERROR_NO_SUCH_PROFILE;
            goto do_exit;
        }

        if (profile->common.name[0] == '\0') {
            // Deleting existing profile. Free it's index.
            // The returned index is the one to overwrite below.
            idx = RFC2544_profile_idx_free(idx);

            // Set it to the default profile before saving in just a second.
            RFC2544_profile_defaults(profile);
        } else {
            // Check that the new profile name doesn't exist already.
            u32 idx2;

            if ((idx2 = RFC2544_profile_idx_get(profile->common.name)) != RFC2544_PROFILE_IDX_NONE && idx != idx2) {
                rc = RFC2544_ERROR_PROFILE_ALREADY_EXISTS;
                goto do_exit;
            }
        }
    }

    RFC2544_profiles[idx] = *profile;
    RFC2544_profile_flash_write();

do_exit:
    RFC2544_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// rfc2544_mgmt_test_start()
/******************************************************************************/
vtss_rc rfc2544_mgmt_test_start(i8 *profile_name, i8 *report_name, i8 *report_dscr)
{
    vtss_rc          rc = VTSS_RC_OK;
    u32              profile_idx, report_idx;
    rfc2544_report_t *r;

    if (profile_name == NULL || profile_name[0] == '\0' || report_name == NULL || report_name[0] == '\0') {
        return RFC2544_ERROR_ARGUMENT;
    }

    if ((rc = RFC2544_report_chk(report_name, report_dscr)) != VTSS_RC_OK) {
        return rc;
    }

    RFC2544_CRIT_ENTER();

    if ((profile_idx = RFC2544_profile_idx_get(profile_name)) == RFC2544_PROFILE_IDX_NONE) {
        rc = RFC2544_ERROR_NO_SUCH_PROFILE;
        goto do_exit;
    }

    if (RFC2544_report_idx_get(report_name) != RFC2544_PROFILE_IDX_NONE) {
        rc = RFC2544_ERROR_REPORT_ALREADY_EXISTS;
        goto do_exit;
    }

    if (RFC2544_in_progress_cnt() >= RFC2544_CONCURRENTLY_EXECUTING_MAX) {
        rc = RFC2544_ERROR_TOO_MANY_EXECUTING;
        goto do_exit;
    }

    report_idx = RFC2544_report_idx_alloc();
    r = &RFC2544_reports[report_idx];

    // Copy, timestamp, and start profile
    r->profile = RFC2544_profiles[profile_idx];
    strcpy(r->info.name, report_name);
    strcpy(r->info.dscr, report_dscr);

    // Initialize state and various fields. This may fail, for instance if there is no link
    // on the egress port. This shouldn't cause this function to return an error, but
    // the report itself to be saved to flash with an appropriate error code.
    RFC2544_timestamp(&r->common, FALSE);
    if (RFC2544_test_suite_init(report_idx)) {
        // Success. Kick the thread.
        r->common.status = RFC2544_TEST_STATUS_EXECUTING;
        cyg_flag_setbits(&RFC2544_wakeup_thread_flag, RFC2544_FLAG_EXECUTE);
    } else {
        // Something went wrong.
        r->common.status = RFC2544_TEST_STATUS_FAILED;
        RFC2544_timestamp(&r->common, TRUE);
        RFC2544_report_flash_save(r);
    }

do_exit:
    RFC2544_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// rfc2544_mgmt_test_stop()
/******************************************************************************/
vtss_rc rfc2544_mgmt_test_stop(i8 *report_name)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     idx;

    if (report_name == 0 || report_name[0] == '\0') {
        return RFC2544_ERROR_ARGUMENT;
    }

    RFC2544_CRIT_ENTER();

    if ((idx = RFC2544_report_idx_get(report_name)) == RFC2544_PROFILE_IDX_NONE) {
        rc = RFC2544_ERROR_NO_SUCH_REPORT;
        goto do_exit;
    }

    // Update the status and notify our thread.
    if (RFC2544_reports[idx].common.status == RFC2544_TEST_STATUS_EXECUTING) {
        RFC2544_reports[idx].common.status = RFC2544_TEST_STATUS_CANCELLING;
        cyg_flag_setbits(&RFC2544_wakeup_thread_flag, RFC2544_FLAG_STOP);
    } else {
        rc = RFC2544_ERROR_NOT_EXECUTING;
    }

do_exit:
    RFC2544_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// rfc2544_mgmt_report_info_get()
/******************************************************************************/
vtss_rc rfc2544_mgmt_report_info_get(rfc2544_report_info_t *info)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     idx;

    if (info == NULL) {
        return RFC2544_ERROR_ARGUMENT;
    }

    RFC2544_CRIT_ENTER();

    if (info->name[0] == '\0') {
        // User wants to get the first profile name.
        idx = 0;
    } else {
        idx = RFC2544_report_idx_get(info->name);

        if (idx == RFC2544_REPORT_IDX_NONE) {
            // No such report. Stop.
            rc = RFC2544_ERROR_NO_SUCH_REPORT;
        } else if (idx == ARRSZ(RFC2544_reports) - 1 || RFC2544_REPORT_EMPTY(&RFC2544_reports[idx + 1])) {
            // Previous entry was the last entry in the table, or the next entry is not in use.
            // Exit with VTSS_RC_OK and name[0] = '\0', indicating that we're done.
            idx = RFC2544_REPORT_IDX_NONE;
        } else {
            idx++;
        }
    }

    if (idx != RFC2544_REPORT_IDX_NONE) {
        rfc2544_report_t *r = &RFC2544_reports[idx];
        *info = r->info;
        // When a report is under execution, the creation time and current status
        // is to be found another place in the report:
        strcpy(info->creation_time, r->common.start_time);
        info->status = r->common.status;
    } else {
        info->name[0] = '\0';
    }

    RFC2544_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// rfc2544_mgmt_report_delete()
/******************************************************************************/
vtss_rc rfc2544_mgmt_report_delete(i8 *name)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     idx;

    if (name == NULL || name[0] == '\0') {
        return RFC2544_ERROR_ARGUMENT;
    }

    RFC2544_CRIT_ENTER();

    if ((idx = RFC2544_report_idx_get(name)) != RFC2544_REPORT_IDX_NONE) {
        rfc2544_test_status_t status = RFC2544_reports[idx].common.status;

        if (status == RFC2544_TEST_STATUS_EXECUTING || status == RFC2544_TEST_STATUS_CANCELLING) {
            rc = RFC2544_ERROR_REPORT_DELETE_EXECUTING_OR_CANCELLING;
        } else {
            RFC2544_report_idx_free(idx);
            RFC2544_report_flash_write();
        }
    } else {
        rc = RFC2544_ERROR_NO_SUCH_REPORT;
    }

    RFC2544_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// rfc2544_mgmt_profile_as_txt()
/******************************************************************************/
vtss_rc rfc2544_mgmt_profile_as_txt(i8 *profile_name, i8 **profile_as_txt)
{
    rfc2544_report_get_state_t s;
    vtss_rc                    rc;
    rfc2544_report_t           report;

    if (profile_name == NULL || profile_name[0] == '\0' || profile_as_txt == NULL) {
        return RFC2544_ERROR_ARGUMENT;
    }

    memset(&s,      0, sizeof(s));
    memset(&report, 0, sizeof(report));

    // Get a snapshot of the profile so that we can let go of the crit while printing
    if ((rc = rfc2544_mgmt_profile_get(profile_name, &report.profile)) != VTSS_RC_OK) {
        return rc;
    }

    s.report = &report;

#define RFC2544_DO_EXIT_RC(expr) {rc = (expr); if (rc != VTSS_RC_OK) {goto do_exit;}}

    // From this point on, we must go through the do_exit label below to exit this function,
    // because we possibly need to free the resulting string.
    RFC2544_DO_EXIT_RC(RFC2544_print_ch(&s, '\n'));
    RFC2544_DO_EXIT_RC(RFC2544_print_profile_common(&s));
    if (s.report->profile.common.selected_tests & RFC2544_TEST_TYPE_THROUGHPUT) {
        RFC2544_DO_EXIT_RC(RFC2544_print_ch(&s, '\n'));
        RFC2544_DO_EXIT_RC(RFC2544_print_profile_throughput(&s));
    }
    if (s.report->profile.common.selected_tests & RFC2544_TEST_TYPE_LATENCY) {
        RFC2544_DO_EXIT_RC(RFC2544_print_ch(&s, '\n'));
        RFC2544_DO_EXIT_RC(RFC2544_print_profile_latency(&s));
    }
    if (s.report->profile.common.selected_tests & RFC2544_TEST_TYPE_FRAME_LOSS) {
        RFC2544_DO_EXIT_RC(RFC2544_print_ch(&s, '\n'));
        RFC2544_DO_EXIT_RC(RFC2544_print_profile_frame_loss(&s));
    }
    if (s.report->profile.common.selected_tests & RFC2544_TEST_TYPE_BACK_TO_BACK) {
        RFC2544_DO_EXIT_RC(RFC2544_print_ch(&s, '\n'));
        RFC2544_DO_EXIT_RC(RFC2544_print_profile_back_to_back(&s));
    }
    RFC2544_DO_EXIT_RC(RFC2544_print_ch(&s, '\n'));

#undef RFC2544_DO_EXIT_RC

do_exit:
    if (rc != VTSS_RC_OK) {
        VTSS_FREE(s.txt);
        *profile_as_txt = NULL;
    } else {
        *profile_as_txt = s.txt;
    }
    return rc;
}

/******************************************************************************/
// rfc2544_mgmt_report_as_txt()
/******************************************************************************/
vtss_rc rfc2544_mgmt_report_as_txt(i8 *report_name, i8 **report_as_txt)
{
    rfc2544_report_get_state_t s;
    rfc2544_report_t           report;
    u32                        idx;
    vtss_rc                    rc;
    BOOL                       generate_on_the_fly = FALSE;

    if (report_name == NULL || report_name[0] == '\0' || report_as_txt == NULL) {
        return RFC2544_ERROR_ARGUMENT;
    }

    // Get a snapshot of the report and state so that we can let the state machines run unaffected
    // This must be an atomic snapshot so that report and state are in sync.
    RFC2544_CRIT_ENTER();

    if ((idx = RFC2544_report_idx_get(report_name)) != RFC2544_REPORT_IDX_NONE) {
        // Here, we either return a copy of the already generated text-version of the
        // report, or in case it is under execution, we must generate it on the fly.
        if (RFC2544_reports[idx].report_as_txt != NULL) {
            // It's there already.
            if ((*report_as_txt = VTSS_MALLOC(strlen(RFC2544_reports[idx].report_as_txt) + 1)) == NULL) {
                rc = RFC2544_ERROR_OUT_OF_MEMORY;
            } else {
                strcpy(*report_as_txt, RFC2544_reports[idx].report_as_txt);
                rc = VTSS_RC_OK;
            }
        } else {
            generate_on_the_fly = TRUE;

            // A report-at-hand doesn't exist. So we gotta create one.
            // First take a copy.
            report = RFC2544_reports[idx];

            // Then go over all dynamically allocated entries and create new for the output report.
            memset(report.frame_loss.statistics_list,   0, sizeof(report.frame_loss.statistics_list));
            memset(report.back_to_back.statistics_list, 0, sizeof(report.back_to_back.statistics_list));
            if ((rc = RFC2544_report_dyn_list_cp(&report.frame_loss,   &RFC2544_reports[idx].frame_loss))   != VTSS_RC_OK ||
                (rc = RFC2544_report_dyn_list_cp(&report.back_to_back, &RFC2544_reports[idx].back_to_back)) != VTSS_RC_OK) {
                RFC2544_report_dispose(&report); // Free whatever we really did allocate until now.
            }
        }
    } else {
        rc = RFC2544_ERROR_NO_SUCH_REPORT;
    }

    RFC2544_CRIT_EXIT();

    if (rc != VTSS_RC_OK || !generate_on_the_fly) {
        // Done, either with an error code, or because we copied an already existing report.
        return rc;
    }

    // When we get here, we need to generate a report on the fly.
    memset(&s, 0, sizeof(s));
    s.report = &report;

    rc = RFC2544_report_as_txt(&s);
    *report_as_txt = s.txt;

    (void)RFC2544_report_dispose(&report);
    return rc;
}

/******************************************************************************/
// rfc2544_error_txt()
// Converts RFC2544 error to printable text
/******************************************************************************/
char *rfc2544_error_txt(vtss_rc rc)
{
    switch (rc) {
    case RFC2544_ERROR_EGRESS_PORT:
        return "Invalid egress port number.";

    case RFC2544_ERROR_NO_SUCH_PROFILE:
        return "No such profile.";

    case RFC2544_ERROR_PROFILE_ALREADY_EXISTS:
        return "A profile with that name already exists.";

    case RFC2544_ERROR_OUT_OF_PROFILES:
        return "No vacant profile entries.";

    case RFC2544_ERROR_PROFILE_NAME_TOO_LONG:
        return "Profile name too long.";

    case RFC2544_ERROR_PROFILE_NAME:
        return "Invalid profile name. Name must consist of a non-zero-length ASCII string with characters in range [33; 126].";

    case RFC2544_ERROR_COMMON_DWELL_TIME:
        return "Invalid dwell time.";

    case RFC2544_ERROR_COMMON_MEG_LEVEL:
        return "Invalid MEG level.";

    case RFC2544_ERROR_COMMON_DMAC:
        return "Invalid destination MAC address.";

    case RFC2544_ERROR_COMMON_FRAME_SIZE:
        return "At least one frame size must be selected.";

    case RFC2544_ERROR_COMMON_SELECTED_TESTS:
        return "At least one test must be selected.";

    case RFC2544_ERROR_COMMON_THROUGHPUT_WHEN_LATENCY:
        return "Latency test depends on throughput test.";

    case RFC2544_ERROR_THROUGHPUT_TRIAL_DURATION:
        return "Invalid throughput test trial duration.";

    case RFC2544_ERROR_THROUGHPUT_RATE_MIN:
        return "Invalid throughput test minimum rate.";

    case RFC2544_ERROR_THROUGHPUT_RATE_MAX:
        return "Invalid throughput test maximum rate.";

    case RFC2544_ERROR_THROUGHPUT_RATE_STEP:
        return "Invalid throughput test step rate.";

    case RFC2544_ERROR_THROUGHPUT_RATE_MIN_MAX:
        return "Throughput test maximum rate must be greater than or equal to minimum rate.";

    case RFC2544_ERROR_THROUGHPUT_PASS_CRITERION:
        return "Invalid throughput test allowable frame loss.";

    case RFC2544_ERROR_LATENCY_TRIAL_DURATION:
        return "Invalid latency test trial duration.";

    case RFC2544_ERROR_LATENCY_DMM_INTERVAL:
        return "Invalid latency test delay measurement interval.";

    case RFC2544_ERROR_LATENCY_PASS_CRITERION:
        return "Invalid latency test allowable frame loss.";

    case RFC2544_ERROR_LATENCY_TRIAL_DURATION_DMM_INTERVAL:
        return "Latency test trial duration must be greater than the delay measurement interval.";

    case RFC2544_ERROR_FRAME_LOSS_TRIAL_DURATION:
        return "Invalid frame loss test trial duration.";

    case RFC2544_ERROR_FRAME_LOSS_RATE_MIN:
        return "Invalid frame loss test minimum rate.";

    case RFC2544_ERROR_FRAME_LOSS_RATE_MAX:
        return "Invalid frame loss test maximum rate.";

    case RFC2544_ERROR_FRAME_LOSS_RATE_STEP:
        return "Invalid frame loss test step rate.";

    case RFC2544_ERROR_FRAME_LOSS_RATE_MIN_MAX:
        return "Frame loss test maximum rate must be greater than the minimum rate.";

    case RFC2544_ERROR_FRAME_LOSS_STEP_VS_MAX_MINUS_MIN:
        return "Frame loss step size must be less than or equal to the maximum rate minus the minimum rate";

    case RFC2544_ERROR_BACK_TO_BACK_TRIAL_DURATION:
        return "Invalid back-to-back test trial duration.";

    case RFC2544_ERROR_BACK_TO_BACK_TRIAL_CNT:
        return "Invalid back-to-back test trial count.";

    case RFC2544_ERROR_REPORT_NAME_TOO_LONG:
        return "Report name too long.";

    case RFC2544_ERROR_REPORT_NAME:
        return "Invalid report name. Name must consist of a non-zero-length ASCII string with characters in range [33; 126].";

    case RFC2544_ERROR_NO_SUCH_REPORT:
        return "No such report.";

    case RFC2544_ERROR_REPORT_ALREADY_EXISTS:
        return "A report with that name already exists.";

    case RFC2544_ERROR_TOO_MANY_EXECUTING:
        return "Too many profiles executing simultaneously.";

    case RFC2544_ERROR_NOT_EXECUTING:
        return "Test is not executing.";

    case RFC2544_ERROR_REPORT_DELETE_EXECUTING_OR_CANCELLING:
        return "Cannot delete a report that is currently executing or being cancelled.";

    case RFC2544_ERROR_OUT_OF_MEMORY:
        return "Couldn't allocate memory for the report.";

    case RFC2544_ERROR_REPORT_LINE_TOO_LONG:
        return "Internal error: Report line too long.";

    case RFC2544_ERROR_NO_LINK:
        return "No link on egress port.";

    case RFC2544_ERROR_OUT_OF_MEPS:
        return "No MEP instances available.";

    case RFC2544_ERROR_TST_RX_HIGHER_THAN_TX:
        return "Received more TST frames than were transmitted.";

    case RFC2544_ERROR_TST_NO_FRAMES_SENT:
        return "Internal error: No TST frames were transmitted.";

    case RFC2544_ERROR_TST_OUT_OF_ORDER:
        return "Frames looped back out of order.";

    case RFC2544_ERROR_TST_ALLOWED_LOSS_EXCEEDED:
        return "Allowed TST frame loss exceeded.";

    case RFC2544_ERROR_1DM_RX_HIGHER_THAN_TX:
        return "Received more 1DM frames than were transmitted.";

    case RFC2544_ERROR_1DM_RX_LOWER_THAN_TX:
        return "Received fewer 1DM frames than were transmitted.";

    case RFC2544_ERROR_1DM_NO_FRAMES_SENT:
        return "Internal error: No 1DM frames were transmitted.";

    case RFC2544_ERROR_ARGUMENT:
        // Invalid argument. Most arguments to management functions have their
        // own error code, so this error is for some other types of arguments,
        // e.g. cfg pointers that are NULL, etc.
        return "Invalid argument.";

    default:
        return "Unknown RFC2544 error code";
    }
}

/******************************************************************************/
// rfc2544_mgmt_util_frame_size_enum_to_number()
/******************************************************************************/
u32 rfc2544_mgmt_util_frame_size_enum_to_number(rfc2544_frame_size_t fs)
{
    switch (fs) {
    case RFC2544_FRAME_SIZE_64:
        return 64;

    case RFC2544_FRAME_SIZE_128:
        return 128;

    case RFC2544_FRAME_SIZE_256:
        return 256;

    case RFC2544_FRAME_SIZE_512:
        return 512;

    case RFC2544_FRAME_SIZE_1024:
        return 1024;

    case RFC2544_FRAME_SIZE_1280:
        return 1280;

    case RFC2544_FRAME_SIZE_1518:
        return 1518;

    case RFC2544_FRAME_SIZE_2000:
        return 2000;

    case RFC2544_FRAME_SIZE_9600:
        return 9600;

    default:
        T_E("Why here?");
        break;
    }

    return 0;
}

/******************************************************************************/
// rfc2544_mgmt_util_status_to_str()
/******************************************************************************/
const i8 *rfc2544_mgmt_util_status_to_str(rfc2544_test_status_t status)
{
    switch (status) {
    case  RFC2544_TEST_STATUS_INACTIVE:
        return (const i8 *)"Internal error: Test has never been executed";

    case RFC2544_TEST_STATUS_EXECUTING:
        return (const i8 *)"In progress";

    case RFC2544_TEST_STATUS_CANCELLING:
        return (const i8 *)"Under cancellation";

    case RFC2544_TEST_STATUS_CANCELLED:
        return (const i8 *)"Cancelled by user";

    case RFC2544_TEST_STATUS_PASSED:
        return (const i8 *)"Succeeded";

    case RFC2544_TEST_STATUS_FAILED:
        return (const i8 *)"Failed";

    default:
        return (const i8 *)"Internal error: Unknown status";
    }
}

/******************************************************************************/
// rfc2544_init()
/******************************************************************************/
vtss_rc rfc2544_init(vtss_init_data_t *data)
{
    /*lint --e{454, 456} */
    switch (data->cmd) {
    case INIT_CMD_INIT:
#if defined(VTSS_SW_OPTION_VCLI)
        rfc2544_vcli_init();
#endif

        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        critd_init(&RFC2544_crit,
                   "RFC2544",
                   VTSS_MODULE_ID_RFC2544,
                   VTSS_TRACE_MODULE_ID,
                   CRITD_TYPE_MUTEX);

        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          RFC2544_thread,
                          0,
                          "RFC2544",
                          RFC2544_thread_stack,
                          sizeof(RFC2544_thread_stack),
                          &RFC2544_thread_handle,
                          &RFC2544_thread_block);

        cyg_thread_resume(RFC2544_thread_handle);
        cyg_flag_init(&RFC2544_wakeup_thread_flag);
        break;

    case INIT_CMD_START: {
#if defined(VTSS_SW_OPTION_ICFG)
        vtss_rc rc;
        if ((rc = vtss_icfg_query_register(VTSS_ICFG_RFC2544_PROFILE, "rfc2544", RFC2544_icfg_query_func)) != VTSS_RC_OK) {
            T_E("Eeeh? %s", error_txt(rc));
        }
#endif
        break;
    }

    case INIT_CMD_CONF_DEF:
        if (data->isid == VTSS_ISID_GLOBAL) {
            // Reset configuration. Don't reset reports.
            RFC2544_profile_flash_read(TRUE);
        }
        break;

    case INIT_CMD_MASTER_UP:
        // Read settings from flash
        RFC2544_profile_flash_read(FALSE);
        // Read reports from flash
        RFC2544_report_flash_read();
        break;

    case INIT_CMD_MASTER_DOWN:
        break;

    case INIT_CMD_SWITCH_ADD:
        break;

    default:
        break;
    }
    return VTSS_RC_OK;
}

