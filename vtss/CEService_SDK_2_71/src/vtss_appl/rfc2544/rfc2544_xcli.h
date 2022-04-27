/* Switch API software.

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

#ifndef _RFC2544_XCLI_H_
#define _RFC2544_XCLI_H_

#include "rfc2544_api.h"

#if defined(VTSS_SW_OPTION_ICLI)
#include "icli_api.h" /* For ICLI_SESSION_ID_NONE */
#define RFC2544_SESSION_ID_NONE ICLI_SESSION_ID_NONE
#else
#define RFC2544_SESSION_ID_NONE 0xFFFFFFFF
#endif /* VTSS_SW_OPTION_ICLI */

/******************************************************************************/
// This module's special parameters
/******************************************************************************/
typedef struct {
    // Common
    struct {
        BOOL dscr_seen;
        BOOL meg_level_seen;
        BOOL egress_port_no_seen;
        BOOL sequence_number_check_seen;
        BOOL dwell_time_secs_seen;
        struct {
            BOOL vid_seen;
            BOOL pcp_seen;
            BOOL dei_seen;
        } vlan_tag;
        BOOL dmac_seen;
        BOOL frame_sizes_seen;
    } common;

    // Throughput
    struct {
        BOOL ena_dis_seen;
        BOOL trial_duration_secs_seen;
        BOOL rate_min_permille_seen;
        BOOL rate_max_permille_seen;
        BOOL rate_step_permille_seen;
        BOOL pass_criterion_permille_seen;
    } throughput;

    // Latency
    struct {
        BOOL ena_dis_seen;
        BOOL trial_duration_secs_seen;
        BOOL dmm_interval_secs_seen;
        BOOL pass_criterion_permille_seen;
    } latency;

    // Frame loss
    struct {
        BOOL ena_dis_seen;
        BOOL trial_duration_secs_seen;
        BOOL rate_min_permille_seen;
        BOOL rate_max_permille_seen;
        BOOL rate_step_permille_seen;
    } frame_loss;

    // Back-to-back
    struct {
        BOOL ena_dis_seen;
        BOOL trial_duration_msecs_seen;
        BOOL trial_cnt_seen;
    } back_to_back;

    rfc2544_profile_t profile;

} rfc2544_xcli_req_t;

vtss_rc RFC2544_xcli_cmd_profile_show(u32 session_id, char *profile_name);
vtss_rc RFC2544_xcli_cmd_test_start(u32 session_id, char *profile_name, char *report_name, char *report_dscr);
vtss_rc RFC2544_xcli_cmd_test_stop(u32 session_id, char *report_name);
vtss_rc RFC2544_xcli_cmd_report_show(u32 session_id, char *report_name);
vtss_rc RFC2544_xcli_cmd_report_save(u32 session_id, char *report_name, char *tftp_url);
vtss_rc RFC2544_xcli_cmd_report_del(u32 session_id, char *report_name);
vtss_rc RFC2544_xcli_cmd_profile_del(u32 session_id, char *profile_name);
vtss_rc RFC2544_xcli_cmd_profile_rename(u32 session_id, char *old_profile_name, char *new_profile_name);
vtss_rc RFC2544_xcli_cmd_profile_add(u32 session_id, rfc2544_xcli_req_t *r);

#endif /* _RFC2544_XCLI_H_ */

