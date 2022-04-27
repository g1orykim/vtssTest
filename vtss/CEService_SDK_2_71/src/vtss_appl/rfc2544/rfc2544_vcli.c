/*

 Vitesse Switch software.

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

#include "cli.h"           /* For all the cli_XXX() functions */
#include "rfc2544_api.h"   /* Management interface            */
#include "rfc2544_trace.h" /* For VTSS_TRACE_MODULE_ID        */
#include "rfc2544_xcli.h"  /* For RFC2544_xcli_XXX()          */

/******************************************************************************/
// This module's special parameters
/******************************************************************************/
typedef struct {
    rfc2544_xcli_req_t x;

    i8 profile_new_name[RFC2544_PROFILE_NAME_LEN];

    i8 report_name[RFC2544_REPORT_NAME_LEN];
    i8 report_dscr[RFC2544_REPORT_DSCR_LEN];

    vtss_ipv4_t tftp_server;

    // Loop enable
    BOOL loop_enable;
} rfc2544_vcli_req_t;

static char RFC2544_vcli_frame_sizes_list_help_txt[300];

/******************************************************************************/
// rfc2544_vcli_init()
/******************************************************************************/
void rfc2544_vcli_init(void)
{
    int                  cnt;
    rfc2544_frame_size_t fs;

    // Register the size required for the rfc2544 CLI request structure
    cli_req_size_register(sizeof(rfc2544_vcli_req_t));

    strcpy(RFC2544_vcli_frame_sizes_list_help_txt, "Comma-separated list of frame sizes. Use 'all' to select all possible. Valid frame sizes are:");
    cnt = strlen(RFC2544_vcli_frame_sizes_list_help_txt);
    for (fs = 0; fs < RFC2544_FRAME_SIZE_CNT; fs++) {
        cnt += snprintf(RFC2544_vcli_frame_sizes_list_help_txt + cnt, sizeof(RFC2544_vcli_frame_sizes_list_help_txt) - cnt - 1, "%s %u", fs == 0 ? "" : ",", rfc2544_mgmt_util_frame_size_enum_to_number(fs));
    }
}

/******************************************************************************/
// RFC2544_vcli_cmd_profile_add()
/******************************************************************************/
static void RFC2544_vcli_cmd_profile_add(cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    (void)RFC2544_xcli_cmd_profile_add(RFC2544_SESSION_ID_NONE, &r->x);
}

/******************************************************************************/
// RFC2544_vcli_cmd_profile_del()
/******************************************************************************/
static void RFC2544_vcli_cmd_profile_del(cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    (void)RFC2544_xcli_cmd_profile_del(RFC2544_SESSION_ID_NONE, r->x.profile.common.name);
}

/******************************************************************************/
// RFC2544_vcli_cmd_profile_rename()
/******************************************************************************/
static void RFC2544_vcli_cmd_profile_rename(cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    (void)RFC2544_xcli_cmd_profile_rename(RFC2544_SESSION_ID_NONE, r->x.profile.common.name, r->profile_new_name);
}

/******************************************************************************/
// RFC2544_vcli_cmd_profile_show()
/******************************************************************************/
static void RFC2544_vcli_cmd_profile_show(cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    (void)RFC2544_xcli_cmd_profile_show(RFC2544_SESSION_ID_NONE, req->set ? r->x.profile.common.name : NULL);
}

/******************************************************************************/
// RFC2544_vcli_cmd_test_start()
/******************************************************************************/
static void RFC2544_vcli_cmd_test_start(cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    (void)RFC2544_xcli_cmd_test_start(RFC2544_SESSION_ID_NONE, r->x.profile.common.name, r->report_name, r->report_dscr);
}

/******************************************************************************/
// RFC2544_vcli_cmd_test_stop()
/******************************************************************************/
static void RFC2544_vcli_cmd_test_stop(cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    (void)RFC2544_xcli_cmd_test_stop(RFC2544_SESSION_ID_NONE, r->report_name);
}

/******************************************************************************/
// RFC2544_vcli_cmd_report_show()
/******************************************************************************/
static void RFC2544_vcli_cmd_report_show(cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    (void)RFC2544_xcli_cmd_report_show(RFC2544_SESSION_ID_NONE, req->set ? r->report_name : NULL);
}

/******************************************************************************/
// RFC2544_vcli_cmd_report_save()
/******************************************************************************/
static void RFC2544_vcli_cmd_report_save(cli_req_t *req)
{
    rfc2544_vcli_req_t  *r = req->module_req;
    // req->parm holds the TFTP URL.
    (void)RFC2544_xcli_cmd_report_save(RFC2544_SESSION_ID_NONE, r->report_name, req->parm);
}

/******************************************************************************/
// RFC2544_vcli_cmd_report_del()
/******************************************************************************/
static void RFC2544_vcli_cmd_report_del(cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    (void)RFC2544_xcli_cmd_report_del(RFC2544_SESSION_ID_NONE, r->report_name);
}

#if defined(VTSS_ARCH_SERVAL)
/******************************************************************************/
// RFC2544_vcli_cmd_dbg_loop()
/******************************************************************************/
static void RFC2544_vcli_cmd_dbg_loop(cli_req_t *req)
{
    rfc2544_vcli_req_t   *r = req->module_req;
    port_iter_t          pit;
    BOOL                 first = TRUE;
    vtss_acl_port_conf_t acl_cfg;

    (void)cli_port_iter_init(&pit, VTSS_ISID_LOCAL, PORT_ITER_FLAGS_NORMAL);
    while (cli_port_iter_getnext(&pit, req)) {
        if (vtss_acl_port_conf_get(NULL, pit.iport, &acl_cfg) != VTSS_RC_OK) {
            cli_printf("Error: Unable to obtain ACL port configuration for port %u", pit.uport);
            return;
        }

        if (req->set) {
            acl_cfg.action.mac_swap             = r->loop_enable;
            acl_cfg.action.port_action          = r->loop_enable ? VTSS_ACL_PORT_ACTION_REDIR : VTSS_ACL_PORT_ACTION_NONE;
            memset(acl_cfg.action.port_list, 0, sizeof(acl_cfg.action.port_list));
            acl_cfg.action.port_list[pit.iport] = r->loop_enable;

            if (vtss_acl_port_conf_set(NULL, pit.iport, &acl_cfg) != VTSS_RC_OK) {
                cli_printf("Error: Failed to set loop configuration for port %u", pit.uport);
                return;
            }
        } else {
            if (first) {
                cli_table_header("Port  Looping");
                first = FALSE;
            }

            cli_printf("%4u  %s\n", pit.uport, acl_cfg.action.mac_swap && acl_cfg.action.port_action == VTSS_ACL_PORT_ACTION_REDIR && acl_cfg.action.port_list[pit.iport] ? "Yes" : "No");
        }
    }

    if (!req->set) {
        cli_printf("\n");
    }
}
#endif /* defined(VTSS_ARCH_SERVAL) */

/******************************************************************************/
//
// RFC2544 CLI PARSER FUNCTIONS
//
/******************************************************************************/

/******************************************************************************/
// RFC2544_vcli_parse_keyword()
/******************************************************************************/
static int RFC2544_vcli_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    // The keywords are used as separators, or - you could say - command advancers, to be able
    // to uniquely go to part of the command syntax that you wish to modify.
    // Therefore, no particular req->XXX_seen is required to be set.
    return cli_parse_find(cmd, stx) == NULL ? 1 : 0;
}

/******************************************************************************/
// RFC2544_vcli_parse_profile_name()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_profile_name(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;

    return cli_parse_string(cmd_org, r->x.profile.common.name, 1, sizeof(r->x.profile.common.name) - 1);
}

/******************************************************************************/
// RFC2544_vcli_parse_profile_new_name()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_profile_new_name(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;

    return cli_parse_string(cmd_org, r->profile_new_name, 1, sizeof(r->profile_new_name) - 1);
}

/******************************************************************************/
// RFC2544_vcli_parse_profile_dscr()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_profile_dscr(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    int                error;

    if ((error = cli_parse_quoted_string(cmd_org, r->x.profile.common.dscr, sizeof(r->x.profile.common.dscr) - 1)) == 0) {
        r->x.common.dscr_seen = TRUE;
    }

    return error;
}

/******************************************************************************/
// Macro for simplifying u32 parsing with MIN/MAX values.
/******************************************************************************/
#define RFC2544_VCLI_PARSE_U32(_n_, _N_) do {                                                          \
    rfc2544_vcli_req_t *r = req->module_req;                                                           \
    int                error;                                                                          \
    ulong              val;                                                                            \
    if ((error = cli_parse_ulong(cmd, &val, RFC2544_ ## _N_ ## _MIN, RFC2544_ ## _N_ ## _MAX)) == 0) { \
        r->x.profile._n_ = val;                                                                        \
        r->x._n_ ## _seen = TRUE;                                                                      \
    }                                                                                                  \
    return error;                                                                                      \
} while (0)

/******************************************************************************/
// RFC2544_vcli_parse_mel()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_mel(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(common.meg_level, COMMON_MEG_LEVEL);
}

/******************************************************************************/
// RFC2544_vcli_parse_port()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_port(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;
    int                error;
    vtss_uport_no_t    uport;

    if ((error = cli_parse_ulong(cmd, &uport, 1, port_isid_port_count(VTSS_ISID_LOCAL))) == 0) {
        r->x.profile.common.egress_port_no = uport2iport(uport);
        r->x.common.egress_port_no_seen = TRUE;
    }
    return error;
}

/******************************************************************************/
// RFC2544_vcli_parse_seq_chk()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_seq_chk(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char               *found = cli_parse_find(cmd, stx);
    rfc2544_vcli_req_t *r     = req->module_req;

    req->parm_parsed = 1;
    if (found) {
        if (!strncmp(found, "sc_dis", 6)) {
            r->x.profile.common.sequence_number_check = FALSE;
        } else if (!strncmp(found, "sc_ena", 6)) {
            r->x.profile.common.sequence_number_check = TRUE;
        } else {
            T_E("Ehh?");
        }
        r->x.common.sequence_number_check_seen = TRUE;
    }

    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
// RFC2544_vcli_parse_dwell_time()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_dwell_time(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(common.dwell_time_secs, COMMON_DWELL_TIME);
}

/******************************************************************************/
// RFC2544_vcli_parse_vid()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
#define RFC2544_COMMON_VID_MIN 0
#define RFC2544_COMMON_VID_MAX 4095
    RFC2544_VCLI_PARSE_U32(common.vlan_tag.vid, COMMON_VID);
#undef  RFC2544_COMMON_VID_MIN
#undef  RFC2544_COMMON_VID_MAX
}

/******************************************************************************/
// RFC2544_vcli_parse_pcp()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_pcp(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
#define RFC2544_COMMON_PCP_MIN 0
#define RFC2544_COMMON_PCP_MAX 7
    RFC2544_VCLI_PARSE_U32(common.vlan_tag.pcp, COMMON_PCP);
#undef  RFC2544_COMMON_PCP_MIN
#undef  RFC2544_COMMON_PCP_MAX
}

/******************************************************************************/
// RFC2544_vcli_parse_dei()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_dei(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
#define RFC2544_COMMON_DEI_MIN 0
#define RFC2544_COMMON_DEI_MAX 1
    RFC2544_VCLI_PARSE_U32(common.vlan_tag.dei, COMMON_DEI);
#undef  RFC2544_COMMON_DEI_MIN
#undef  RFC2544_COMMON_DEI_MAX
}

/******************************************************************************/
// RFC2544_vcli_parse_dmac()
/******************************************************************************/
static int RFC2544_vcli_parse_dmac(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    cli_spec_t         spec;
    rfc2544_vcli_req_t *r    = req->module_req;
    int                error;

    if ((error = cli_parse_mac(cmd, r->x.profile.common.dmac.addr, &spec, FALSE)) == 0) {
        r->x.common.dmac_seen = TRUE;
    }

    return error;
}

/******************************************************************************/
// RFC2544_vcli_frame_size_index()
// Given a numeric frame size, convert to an enumeration.
// The function returns RFC2544_FRAME_SIZE_CNT if input frame size is illegal.
/******************************************************************************/
static rfc2544_frame_size_t RFC2544_vcli_frame_size_index(u32 frame_size_bytes)
{
    rfc2544_frame_size_t fs_enum;

    for (fs_enum = 0; fs_enum < RFC2544_FRAME_SIZE_CNT; fs_enum++) {
        if (rfc2544_mgmt_util_frame_size_enum_to_number(fs_enum) == frame_size_bytes) {
            return fs_enum;
        }
    }

    return fs_enum;
}

/******************************************************************************/
// RFC2544_vcli_parse_frame_size_list()
/******************************************************************************/
static int RFC2544_vcli_parse_frame_size_list(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    u32                   n;
    char                  *p, *end;
    BOOL                  error, comma = 0;
    rfc2544_vcli_req_t    *r = req->module_req;
    rfc2544_frame_size_t  fs;

    p = cmd;
    error = p == NULL;
    while (p != NULL && *p != '\0') {
        // Read integer.
        n = strtoul(p, &end, 0);
        if (end == p) {
            error = 1;
            break;
        }
        p = end;

        // Check value.
        if ((fs = RFC2544_vcli_frame_size_index(n)) == RFC2544_FRAME_SIZE_CNT) {
            error = 1;
            break;
        }

        r->x.profile.common.selected_frame_sizes[fs] = TRUE;

        comma = 0;
        if (*p == ',') {
            comma = 1;
            p++;
        }
    }

    // Check for trailing comma
    if (comma) {
        error = 1;
    }

    if (!error) {
        r->x.common.frame_sizes_seen = TRUE;
    }

    return error;
}

/******************************************************************************/
// RFC2544_vcli_parse_tp_ena_dis()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_tp_ena_dis(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char               *found = cli_parse_find(cmd, stx);
    rfc2544_vcli_req_t *r     = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "tp_dis", 6)) {
            r->x.profile.common.selected_tests &= ~RFC2544_TEST_TYPE_THROUGHPUT;
        } else if (!strncmp(found, "tp_ena", 6)) {
            r->x.profile.common.selected_tests |=  RFC2544_TEST_TYPE_THROUGHPUT;
        } else {
            T_E("Ehh?");
        }
        r->x.throughput.ena_dis_seen = TRUE;
    }

    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
// RFC2544_vcli_parse_tp_trial_dur()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_tp_trial_dur(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(throughput.trial_duration_secs, THROUGHPUT_TRIAL_DURATION);
}

/******************************************************************************/
// RFC2544_vcli_parse_tp_rate_min()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_tp_rate_min(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(throughput.rate_min_permille, THROUGHPUT_RATE_MIN);
}

/******************************************************************************/
// RFC2544_vcli_parse_tp_rate_max()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_tp_rate_max(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(throughput.rate_max_permille, THROUGHPUT_RATE_MAX);
}

/******************************************************************************/
// RFC2544_vcli_parse_tp_rate_step()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_tp_rate_step(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(throughput.rate_step_permille, THROUGHPUT_RATE_STEP);
}

/******************************************************************************/
// RFC2544_vcli_parse_tp_allowed_frame_loss()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_tp_allowed_frame_loss(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(throughput.pass_criterion_permille, THROUGHPUT_PASS_CRITERION);
}

/******************************************************************************/
// RFC2544_vcli_parse_la_ena_dis()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_la_ena_dis(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char               *found = cli_parse_find(cmd, stx);
    rfc2544_vcli_req_t *r     = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "la_dis", 6)) {
            r->x.profile.common.selected_tests &= ~RFC2544_TEST_TYPE_LATENCY;
        } else if (!strncmp(found, "la_ena", 6)) {
            r->x.profile.common.selected_tests |=  RFC2544_TEST_TYPE_LATENCY;
        } else {
            T_E("Ehh?");
        }
        r->x.latency.ena_dis_seen = TRUE;
    }

    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
// RFC2544_vcli_parse_la_trial_dur()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_la_trial_dur(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(latency.trial_duration_secs, LATENCY_TRIAL_DURATION);
}

/******************************************************************************/
// RFC2544_vcli_parse_la_dmm_interval()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_la_dmm_interval(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(latency.dmm_interval_secs, LATENCY_DMM_INTERVAL);
}

/******************************************************************************/
// RFC2544_vcli_parse_la_allowed_frame_loss()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_la_allowed_frame_loss(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(latency.pass_criterion_permille, LATENCY_PASS_CRITERION);
}

/******************************************************************************/
// RFC2544_vcli_parse_fl_ena_dis()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_fl_ena_dis(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char               *found = cli_parse_find(cmd, stx);
    rfc2544_vcli_req_t *r     = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "fl_dis", 6)) {
            r->x.profile.common.selected_tests &= ~RFC2544_TEST_TYPE_FRAME_LOSS;
        } else if (!strncmp(found, "fl_ena", 6)) {
            r->x.profile.common.selected_tests |=  RFC2544_TEST_TYPE_FRAME_LOSS;
        } else {
            T_E("Ehh?");
        }
        r->x.frame_loss.ena_dis_seen = TRUE;
    }

    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
// RFC2544_vcli_parse_fl_trial_dur()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_fl_trial_dur(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(frame_loss.trial_duration_secs, FRAME_LOSS_TRIAL_DURATION);
}

/******************************************************************************/
// RFC2544_vcli_parse_fl_rate_min()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_fl_rate_min(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(frame_loss.rate_min_permille, FRAME_LOSS_RATE_MIN);
}

/******************************************************************************/
// RFC2544_vcli_parse_fl_rate_max()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_fl_rate_max(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(frame_loss.rate_max_permille, FRAME_LOSS_RATE_MAX);
}

/******************************************************************************/
// RFC2544_vcli_parse_fl_rate_step()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_fl_rate_step(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(frame_loss.rate_step_permille, FRAME_LOSS_RATE_STEP);
}

/******************************************************************************/
// RFC2544_vcli_parse_bb_ena_dis()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_bb_ena_dis(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char               *found = cli_parse_find(cmd, stx);
    rfc2544_vcli_req_t *r     = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "bb_dis", 6)) {
            r->x.profile.common.selected_tests &= ~RFC2544_TEST_TYPE_BACK_TO_BACK;
        } else if (!strncmp(found, "bb_ena", 6)) {
            r->x.profile.common.selected_tests |=  RFC2544_TEST_TYPE_BACK_TO_BACK;
        } else {
            T_E("Ehh?");
        }
        r->x.back_to_back.ena_dis_seen = TRUE;
    }

    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
// RFC2544_vcli_parse_bb_trial_dur()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_bb_trial_dur(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(back_to_back.trial_duration_msecs, BACK_TO_BACK_TRIAL_DURATION);
}

/******************************************************************************/
// RFC2544_vcli_parse_bb_trial_cnt()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_bb_trial_cnt(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    RFC2544_VCLI_PARSE_U32(back_to_back.trial_cnt, BACK_TO_BACK_TRIAL_CNT);
}

/******************************************************************************/
// RFC2544_vcli_parse_report_name()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_report_name(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;

    return cli_parse_string(cmd_org, r->report_name, 1, sizeof(r->report_name) - 1);
}

/******************************************************************************/
// RFC2544_vcli_parse_report_dscr()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_report_dscr(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    rfc2544_vcli_req_t *r = req->module_req;

    return cli_parse_quoted_string(cmd_org, r->report_dscr, sizeof(r->report_dscr) - 1);
}

/******************************************************************************/
// RFC2544_vcli_parse_tftp_url()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_tftp_url(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    // This puts the filename into req->parm.
    return cli_parse_raw(cmd_org, req);
}

#if defined(VTSS_ARCH_SERVAL)
/******************************************************************************/
// RFC2544_vcli_parse_loop_ena_dis()
/******************************************************************************/
static int32_t RFC2544_vcli_parse_loop_ena_dis(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char               *found = cli_parse_find(cmd, stx);
    rfc2544_vcli_req_t *r     = req->module_req;

    req->parm_parsed = 1;
    if (found) {
        if (!strncmp(found, "enable", 6)) {
            r->loop_enable = TRUE;
        } else if (!strncmp(found, "disable", 7)) {
            r->loop_enable = FALSE;
        } else {
            T_E("Ehh?");
        }
    }

    return (found == NULL ? 1 : 0);
}
#endif /* defined(VTSS_ARCH_SERVAL) */

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/
static cli_parm_t RFC2544_vcli_parm_table[] = {
    {
        "<profile_name>",
        "Name of profile to add, modify, show, or delete. Restricted to the ASCII character set (33 - 126)",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_profile_name,
        NULL
    },
    {
        "<new_profile_name>",
        "When renaming, this is the new name of the profile",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_profile_new_name,
        NULL
    },
    {
        "mel",
        "MEG Level keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "<mel>",
        "MEG Level (0 - 7)",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_mel,
        NULL
    },
    {
        "port",
        "Port keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "<port>",
        "Egress port number",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_port,
        NULL
    },
    {
        "seq_chk",
        "Sequence number check keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "sc_ena|sc_dis",
        "Enable or disable sequence number check",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_seq_chk,
        NULL
    },
    {
        "dwell_time",
        "Dwell time keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "<dwell_time>",
        "Time (in seconds) to wait after each trial before getting statistics from hardware",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_dwell_time,
        NULL
    },
    {
        "vid",
        "VLAN ID keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "<vid>",
        "If VLAN Down MEP, a value from 1 - 4095. If port DOWN MEP set to 0",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_vid,
        NULL
    },
    {
        "<pcp>",
        "PCP (0 - 7) held in VLAN tag on VLAN DOWN MEPs. Unused if Port Down MEP",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_pcp,
        NULL
    },
    {
        "<dei>",
        "DEI (0 - 1) held in VLAN tag on VLAN DOWN MEPs. Unused if Port Down MEP",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_dei,
        NULL
    },
    {
        "dmac",
        "Destination MAC keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "<dmac>",
        "Destination MAC address.",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_dmac,
        NULL
    },
    {
        "frame_sizes",
        "Frame sizes keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "<frame_size_list>",
        RFC2544_vcli_frame_sizes_list_help_txt,
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_frame_size_list,
        NULL
    },
    {
        "throughput",
        "Throughput keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "tp_ena|tp_dis",
        "Enable or disable throughput test",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_tp_ena_dis,
        NULL
    },
    {
        "<tp_trial_dur>",
        "Throughput test trial duration in seconds",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_tp_trial_dur,
        NULL
    },
    {
        "<tp_min_rate>",
        "Throughput test's minimum rate in permille of link speed",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_tp_rate_min,
        NULL
    },
    {
        "<tp_max_rate>",
        "Throughput test's maximum rate in permille of link speed",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_tp_rate_max,
        NULL
    },
    {
        "<tp_rate_accuracy>",
        "Throughput test's rate accuracy in permille of link speed",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_tp_rate_step,
        NULL
    },
    {
        "<tp_allowed_frame_loss>",
        "Throughput test's allowed frame loss in permille",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_tp_allowed_frame_loss,
        NULL
    },
    {
        "latency",
        "Latency keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "la_ena|la_dis",
        "Enable or disable latency test",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_la_ena_dis,
        NULL
    },
    {
        "<la_trial_dur>",
        "Latency test trial duration in seconds",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_la_trial_dur,
        NULL
    },
    {
        "<la_dmm_interval>",
        "Latency test's interval (in seconds) between sending delay measurement frames",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_la_dmm_interval,
        NULL
    },
    {
        "<la_allowed_frame_loss>",
        "Latency test's allowed frame loss in permille",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_la_allowed_frame_loss,
        NULL
    },
    {
        "frame_loss",
        "Frame Loss keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "fl_ena|fl_dis",
        "Enable or disable frame loss test",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_fl_ena_dis,
        NULL
    },
    {
        "<fl_trial_dur>",
        "Frame loss test trial duration in seconds",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_fl_trial_dur,
        NULL
    },
    {
        "<fl_min_rate>",
        "Frame loss test's minimum rate in permille of link speed",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_fl_rate_min,
        NULL
    },
    {
        "<fl_max_rate>",
        "Frame loss test's maximum rate in permille of link speed",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_fl_rate_max,
        NULL
    },
    {
        "<fl_rate_step>",
        "Frame loss test's rate step in permille of link speed",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_fl_rate_step,
        NULL
    },
    {
        "back_to_back",
        "Back-to-back keyword",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_keyword,
        NULL
    },
    {
        "bb_ena|bb_dis",
        "Enable or disable back-to-back test",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_bb_ena_dis,
        NULL
    },
    {
        "<bb_trial_dur>",
        "Back-to-back test trial duration in milliseconds",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_bb_trial_dur,
        NULL
    },
    {
        "<bb_trial_cnt>",
        "Back-to-back test's number of trials",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_bb_trial_cnt,
        NULL
    },
    {
        "<dscr>",
        "Profile description",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_profile_dscr,
        NULL
    },
    {
        "<report_name>",
        "Name of report. Restricted to the ASCII character set (33 - 126)",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_report_name,
        NULL
    },
    {
        "<report_dscr>",
        "Report description",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_report_dscr,
        NULL
    },
    {
        "<tftp_url>",
        "TFTP server URL on the form tftp://server[:port]/path-to-file",
        CLI_PARM_FLAG_NONE,
        RFC2544_vcli_parse_tftp_url,
        NULL
    },
#if defined(VTSS_ARCH_SERVAL)
    {
        "<port_list>",
        "Port(s) on which to enable or disable looping",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_port_list,
        RFC2544_vcli_cmd_dbg_loop
    },
    {
        "enable|disable",
        "Enable or disable looping of frames on a port",
        CLI_PARM_FLAG_SET,
        RFC2544_vcli_parse_loop_ena_dis,
        RFC2544_vcli_cmd_dbg_loop
    },
#endif /* defined(VTSS_ARCH_SERVAL) */
    {NULL, NULL, 0, 0, NULL}
};

/******************************************************************************/
// Command table
/******************************************************************************/
enum {
    RFC2544_VCLI_PRIO_PROFILE_ADD = 0,
    RFC2544_VCLI_PRIO_PROFILE_DEL,
    RFC2544_VCLI_PRIO_PROFILE_RENAME,
    RFC2544_VCLI_PRIO_PROFILE_SHOW,
    RFC2544_VCLI_PRIO_TEST_START,
    RFC2544_VCLI_PRIO_TEST_STOP,
    RFC2544_VCLI_PRIO_REPORT_SHOW,
    RFC2544_VCLI_PRIO_REPORT_SAVE,
    RFC2544_VCLI_PRIO_REPORT_DEL,
};

cli_cmd_tab_entry (
    NULL, // Requires R/W access.
    "RFC2544 Profile Add <profile_name>\n"
    "                    [mel]          [<mel>]\n"
    "                    [port]         [<port>]\n"
    "                    [seq_chk]      [sc_ena|sc_dis]\n"
    "                    [dwell_time]   [<dwell_time>]\n"
    "                    [vid]          [<vid>] [<pcp>] [<dei>]\n"
    "                    [dmac]         [<dmac>]\n"
    "                    [frame_sizes]  [<frame_size_list>]\n"
    "                    [throughput]   [tp_ena|tp_dis] [<tp_trial_dur>] [<tp_min_rate>] [<tp_max_rate>] [<tp_rate_accuracy>] [<tp_allowed_frame_loss>]\n"
    "                    [latency]      [la_ena|la_dis] [<la_trial_dur>] [<la_dmm_interval>] [<la_allowed_frame_loss>]\n"
    "                    [frame_loss]   [fl_ena|fl_dis] [<fl_trial_dur>] [<fl_min_rate>] [<fl_max_rate>] [<fl_rate_step>]\n"
    "                    [back_to_back] [bb_ena|bb_dis] [<bb_trial_dur>] [<bb_trial_cnt>]\n"
    "                                   [<dscr>]", /* Description must come last (and not be accompanied by a keyword, because that would prevent us from using the keyword as actual description),
                                                    * because it's a string, which - if it didn't come last - would capture the keywords (such as mel, port, seq_chk above) */
    "Add or modify profile.\nFields not specified will obtain default values if adding, or retain their value if modifying",
    RFC2544_VCLI_PRIO_PROFILE_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_profile_add,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "RFC2544 Profile Delete <profile_name>",
    "Delete profile",
    RFC2544_VCLI_PRIO_PROFILE_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_profile_del,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "RFC2544 Profile Rename <profile_name> <new_profile_name>",
    "Rename <profile_name> to <new_profile_name>",
    RFC2544_VCLI_PRIO_PROFILE_RENAME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_profile_rename,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "RFC2544 Profile Show [<profile_name>]",
    NULL,
    "If <profile_name> is not specified, show all profile names along with their descriptions. If <profile_name> is specified show details for the profile",
    RFC2544_VCLI_PRIO_PROFILE_SHOW,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_profile_show,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "RFC2544 Start <report_name> <profile_name> [<report_dscr>]",
    "Start execution of <profile_name> and save the report under <report_name>. Optionally give it a description",
    RFC2544_VCLI_PRIO_TEST_START,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_test_start,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "RFC2544 Stop <report_name>",
    "Stop execution of test given by <report_name>",
    RFC2544_VCLI_PRIO_TEST_STOP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_test_stop,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "RFC2544 Report Show [<report_name>]",
    NULL,
    "If <report_name> is not specified, show all report names along with their descriptions, creation time, and status. If <report_name> is specified show details for the report",
    RFC2544_VCLI_PRIO_REPORT_SHOW,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_report_show,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "RFC2544 Report Save <report_name> <tftp_url>",
    "Save <report_name> to <tftp_url> using TFTP",
    RFC2544_VCLI_PRIO_REPORT_SAVE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_report_save,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "RFC2544 Report Delete <report_name>",
    "Delete report",
    RFC2544_VCLI_PRIO_REPORT_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_RFC2544,
    RFC2544_vcli_cmd_report_del,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_ARCH_SERVAL)
cli_cmd_tab_entry (
    NULL,
    "Debug RFC2544 Loop [<port_list>] [enable|disable]",
    "Enable or disable looping of all frames received on a list of ports.\n"
    "Notice: All ACL configuration made by other modules on the ports will get lost",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    RFC2544_vcli_cmd_dbg_loop,
    NULL,
    RFC2544_vcli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* defined(VTSS_ARCH_SERVAL) */

