/*

 Vitesse EPS software.

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
 
 $Id$
 $Revision$

*/

#include "vtss_eps_api.h"
#include "vtss_api.h"
#include <string.h>

/*lint -sem( vtss_eps_crit_lock, thread_lock ) */
/*lint -sem( vtss_eps_crit_unlock, thread_unlock ) */

/****************************************************************************/
/*  EPS global variables                                                        */
/****************************************************************************/

/****************************************************************************/
/*  EPS local variables                                                         */
/****************************************************************************/

#define    INPUT_LO_LOCKOUT     0
#define    INPUT_LO_FORCED      1
#define    INPUT_LO_SF_W_ON     2
#define    INPUT_LO_SF_W_OFF    3
#define    INPUT_LO_SF_P_ON     4
#define    INPUT_LO_SF_P_OFF    5
#define    INPUT_LO_MANUAL_P    6
#define    INPUT_LO_MANUAL_W    7
#define    INPUT_LO_CLEAR       8
#define    INPUT_LO_EXERCISE    9
#define    INPUT_LO_WTR         10
#define    INPUT_RM_LO          11
#define    INPUT_RM_SF_P        12
#define    INPUT_RM_FS          13
#define    INPUT_RM_SF_W        14
#define    INPUT_RM_MS_NORMAL   15
#define    INPUT_RM_MS_NULL     16
#define    INPUT_RM_WTR         17
#define    INPUT_RM_EXER_NULL   18
#define    INPUT_RM_EXER_NORMAL 19
#define    INPUT_RM_RR_NULL     20
#define    INPUT_RM_RR_NORMAL   21
#define    INPUT_RM_DNR         22
#define    INPUT_RM_NR_NULL     23
#define    INPUT_RM_NR_NORMAL   24
#define    INPUT_MAX            25


#define    STATE_NO_REQUEST_W       0
#define    STATE_NO_REQUEST_P       1
#define    STATE_LOCKOUT            2
#define    STATE_FORCED_SWITCH      3
#define    STATE_SIGNAL_FAIL_W      4
#define    STATE_SIGNAL_FAIL_P      5
#define    STATE_MANUEL_SWITCH_P    6
#define    STATE_MANUEL_SWITCH_W    7
#define    STATE_WAIT_TO_RESTORE    8
#define    STATE_EXERCISE_W         9
#define    STATE_EXERCISE_P         10
#define    STATE_REVERSE_REQUEST_W  11
#define    STATE_REVERSE_REQUEST_P  12
#define    STATE_DO_NOT_REVERT      13
#define    STATE_MAX                14


#define     EVENT_IN_WTR          0x00000001
#define     EVENT_IN_HOFF_W       0x00000002
#define     EVENT_IN_HOFF_P       0x00000004
#define     EVENT_IN_COMMAND      0x00000008
#define     EVENT_IN_SF_W         0x00000010
#define     EVENT_IN_SF_P         0x00000020
#define     EVENT_IN_APS          0x00000040
#define     EVENT_IN_CONFIG       0x00000080
#define     EVENT_IN_CREATE       0x00000100
#define     EVENT_IN_DELETE       0x00000200
#define     EVENT_IN_DFOP_NR      0x00000400
#define     EVENT_IN_MASK         0x00000FFF
                            
#define     EVENT_OUT_APS         0x80000000
#define     EVENT_OUT_SIGNAL      0x40000000
#define     EVENT_OUT_PORT_PROT   0x20000000
#define     EVENT_OUT_MASK        0xF0000000


typedef struct
{
    BOOL                            created;
    BOOL                            configured;

    vtss_eps_mgmt_state_t           state;          /* State interface to management */

    vtss_eps_mgmt_conf_t            config;         /* Configuration input from management */
    vtss_eps_mgmt_command_t         command;
    vtss_eps_mgmt_create_param_t    param;

    u32                             event_flags;    /* Flags used to indicate what input/output events has activated the 'run' thread */

    BOOL                            sf_w;           /* variables to contain input other than management */
    BOOL                            sf_p;
    BOOL                            sd_w;
    BOOL                            sd_p;
    BOOL                            wtr_event;
    BOOL                            comming_from_sf;
    u8                              rx_aps[VTSS_EPS_APS_DATA_LENGTH];
    u8                              tx_aps[VTSS_EPS_APS_DATA_LENGTH];

    u32                             wtr_timer;      /* Timers */
    u32                             hoff_w_timer;
    u32                             hoff_p_timer;
    u32                             dFop_nr_timer;

    u32                             internal_state; /* Internal state */
    BOOL                            aps_mode;
} eps_instance_data_t;


static eps_instance_data_t   instance_data[VTSS_EPS_MGMT_CREATED_MAX];
static u32                   timer_res;

/* 1+1 transistion tabels */
static u8              nxt_1p1_unidir_rev[INPUT_MAX][STATE_MAX];
static u8              nxt_1p1_unidir_nonrev[INPUT_MAX][STATE_MAX];
static u8              nxt_1p1_bidir_rev[INPUT_MAX][STATE_MAX];
static u8              nxt_1p1_bidir_nonrev[INPUT_MAX][STATE_MAX];

/* 1:1 transistion tabels */
static u8              nxt_1f1_bidir_rev[INPUT_MAX][STATE_MAX];
static u8              nxt_1f1_bidir_nonrev[INPUT_MAX][STATE_MAX];


/****************************************************************************/
/*  EPS local functions                                                     */
/****************************************************************************/
static void instance_data_clear(eps_instance_data_t  *data)
{
    u32                       w_flow_c, p_flow_c, event_flags_c;
    u8                        aps_c[VTSS_EPS_APS_DATA_LENGTH];
    vtss_eps_mgmt_domain_t    domain_c;
    vtss_eps_mgmt_def_conf_t  def_conf;

    vtss_eps_mgmt_def_conf_get(&def_conf);
 
    /* Copy data for call-out - this for the special case where a call-out has to be done after a EPS instance 'Delete' */
    event_flags_c = data->event_flags & EVENT_OUT_MASK;
    domain_c = data->param.domain;
    memcpy(aps_c, data->tx_aps, VTSS_EPS_APS_DATA_LENGTH);
    w_flow_c = data->param.w_flow;
    p_flow_c = data->param.p_flow;

    memset(data, 0, sizeof(eps_instance_data_t));
    data->config = def_conf.config;
    data->param = def_conf.param;
    data->command = def_conf.command;

    /* Copy data for call-out */
    data->event_flags = event_flags_c;
    data->param.domain = domain_c;
    memcpy(data->tx_aps, aps_c, VTSS_EPS_APS_DATA_LENGTH);
    data->param.w_flow = w_flow_c;
    data->param.p_flow = p_flow_c;
}



static void init_nxt_1p1_unidir_rev(void)
{
    u32 i, j;

    for (i=0; i<INPUT_MAX; ++i)
        for (j=0; j<STATE_MAX; ++j)
            nxt_1p1_unidir_rev[i][j] = STATE_NO_REQUEST_W;

    nxt_1p1_unidir_rev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_W] =    STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_LOCKOUT][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_LOCKOUT][STATE_FORCED_SWITCH] =   STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_W] =   STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_P] =   STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_P] = STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_LOCKOUT][STATE_WAIT_TO_RESTORE] = STATE_LOCKOUT;

    nxt_1p1_unidir_rev[INPUT_LO_FORCED][STATE_NO_REQUEST_W] =    STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_FORCED][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_FORCED][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_W] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_P] = STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_FORCED][STATE_WAIT_TO_RESTORE] = STATE_FORCED_SWITCH;

    nxt_1p1_unidir_rev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_W] =    STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_ON][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_ON][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_P] = STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_ON][STATE_WAIT_TO_RESTORE] = STATE_SIGNAL_FAIL_W;

    nxt_1p1_unidir_rev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_OFF][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_OFF][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_W] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_W_OFF][STATE_WAIT_TO_RESTORE] = STATE_WAIT_TO_RESTORE;

    nxt_1p1_unidir_rev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_W] =    STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_ON][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_ON][STATE_FORCED_SWITCH] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_P] = STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_ON][STATE_WAIT_TO_RESTORE] = STATE_SIGNAL_FAIL_P;

    nxt_1p1_unidir_rev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_OFF][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_OFF][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_P] =   STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_rev[INPUT_LO_SF_P_OFF][STATE_WAIT_TO_RESTORE] = STATE_WAIT_TO_RESTORE;

    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_W] =    STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_P][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_P][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_P][STATE_WAIT_TO_RESTORE] = STATE_MANUEL_SWITCH_P;

    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_W][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_W][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_rev[INPUT_LO_MANUAL_W][STATE_WAIT_TO_RESTORE] = STATE_WAIT_TO_RESTORE;

    nxt_1p1_unidir_rev[INPUT_LO_CLEAR][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_CLEAR][STATE_LOCKOUT] =         STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_CLEAR][STATE_FORCED_SWITCH] =   STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_P] = STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_CLEAR][STATE_WAIT_TO_RESTORE] = STATE_NO_REQUEST_W;

    nxt_1p1_unidir_rev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_EXERCISE][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_EXERCISE][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_rev[INPUT_LO_EXERCISE][STATE_WAIT_TO_RESTORE] = STATE_WAIT_TO_RESTORE;

    nxt_1p1_unidir_rev[INPUT_LO_WTR][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_rev[INPUT_LO_WTR][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_rev[INPUT_LO_WTR][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_rev[INPUT_LO_WTR][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_rev[INPUT_LO_WTR][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_rev[INPUT_LO_WTR][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_rev[INPUT_LO_WTR][STATE_WAIT_TO_RESTORE] = STATE_NO_REQUEST_W;
}

static void init_nxt_1p1_unidir_nonrev(void)
{
    u32 i, j;

    for (i=0; i<INPUT_MAX; ++i)
        for (j=0; j<STATE_MAX; ++j)
            nxt_1p1_unidir_nonrev[i][j] = STATE_NO_REQUEST_W;

    nxt_1p1_unidir_nonrev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_W] =    STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_LOCKOUT][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_LOCKOUT][STATE_FORCED_SWITCH] =   STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_W] =   STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_P] =   STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_P] = STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_W] = STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_LOCKOUT][STATE_DO_NOT_REVERT] =   STATE_LOCKOUT;

    nxt_1p1_unidir_nonrev[INPUT_LO_FORCED][STATE_NO_REQUEST_W] =    STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_FORCED][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_FORCED][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_W] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_P] = STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_W] = STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_FORCED][STATE_DO_NOT_REVERT] =   STATE_FORCED_SWITCH;

    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_W] =    STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_ON][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_ON][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_P] = STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_W] = STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_ON][STATE_DO_NOT_REVERT] =   STATE_SIGNAL_FAIL_W;

    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_OFF][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_OFF][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_W] =   STATE_DO_NOT_REVERT;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_W] = STATE_MANUEL_SWITCH_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_W_OFF][STATE_DO_NOT_REVERT] =   STATE_DO_NOT_REVERT;

    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_W] =    STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_ON][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_ON][STATE_FORCED_SWITCH] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_P] = STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_W] = STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_ON][STATE_DO_NOT_REVERT] =   STATE_SIGNAL_FAIL_P;

    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_OFF][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_OFF][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_P] =   STATE_NO_REQUEST_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_W] = STATE_MANUEL_SWITCH_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_SF_P_OFF][STATE_DO_NOT_REVERT] =   STATE_DO_NOT_REVERT;

    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_W] =    STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_P][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_P][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_W] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_P][STATE_DO_NOT_REVERT] =   STATE_MANUEL_SWITCH_P;

    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_W] =    STATE_MANUEL_SWITCH_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_W][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_W][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_W] = STATE_MANUEL_SWITCH_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_MANUAL_W][STATE_DO_NOT_REVERT] =   STATE_MANUEL_SWITCH_W;

    nxt_1p1_unidir_nonrev[INPUT_LO_CLEAR][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_CLEAR][STATE_LOCKOUT] =         STATE_NO_REQUEST_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_CLEAR][STATE_FORCED_SWITCH] =   STATE_DO_NOT_REVERT;
    nxt_1p1_unidir_nonrev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_P] = STATE_DO_NOT_REVERT;
    nxt_1p1_unidir_nonrev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_W] = STATE_NO_REQUEST_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_CLEAR][STATE_DO_NOT_REVERT] =   STATE_DO_NOT_REVERT;

    nxt_1p1_unidir_nonrev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_W] =    STATE_NO_REQUEST_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_EXERCISE][STATE_LOCKOUT] =         STATE_LOCKOUT;
    nxt_1p1_unidir_nonrev[INPUT_LO_EXERCISE][STATE_FORCED_SWITCH] =   STATE_FORCED_SWITCH;
    nxt_1p1_unidir_nonrev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_P] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_unidir_nonrev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_W] = STATE_MANUEL_SWITCH_W;
    nxt_1p1_unidir_nonrev[INPUT_LO_EXERCISE][STATE_DO_NOT_REVERT] =   STATE_DO_NOT_REVERT;
}

static void init_nxt_1p1_bidir_rev(void)
{
    u32 i, j;

    for (i=0; i<INPUT_MAX; ++i)
        for (j=0; j<STATE_MAX; ++j)
            nxt_1p1_bidir_rev[i][j] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_W] =      STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_P] =      STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_FORCED_SWITCH] =     STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_W] =     STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_P] =     STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_P] =   STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_WAIT_TO_RESTORE] =   STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_EXERCISE_W] =        STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_LOCKOUT][STATE_REVERSE_REQUEST_W] = STATE_LOCKOUT;

    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_NO_REQUEST_W] =      STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_NO_REQUEST_P] =      STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_W] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_P] =   STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_WAIT_TO_RESTORE] =   STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_EXERCISE_W] =        STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_FORCED][STATE_REVERSE_REQUEST_W] = STATE_FORCED_SWITCH;

    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_W] =      STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_P] =      STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_P] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_WAIT_TO_RESTORE] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_EXERCISE_W] =        STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_ON][STATE_REVERSE_REQUEST_W] = STATE_SIGNAL_FAIL_W;

    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_W] =     STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_W] =      STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_P] =      STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_FORCED_SWITCH] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_WAIT_TO_RESTORE] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_EXERCISE_W] =        STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_ON][STATE_REVERSE_REQUEST_W] = STATE_SIGNAL_FAIL_P;

    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_P] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_W] =      STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_P] =      STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_WAIT_TO_RESTORE] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_EXERCISE_W] =        STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_P][STATE_REVERSE_REQUEST_W] = STATE_MANUEL_SWITCH_P;

    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_LO_MANUAL_W][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_LOCKOUT] =           STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_CLEAR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_W] =      STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_LO_EXERCISE][STATE_REVERSE_REQUEST_W] = STATE_EXERCISE_W;

    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_LO_WTR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_SIGNAL_FAIL_P] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_LO][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_SF_P][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_FS][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;

    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_SF_W][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;

    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;

    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_RM_WTR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_NO_REQUEST_W] =      STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_RM_EXER_NULL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_RM_RR_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
}

static void init_nxt_1p1_bidir_nonrev(void)
{
    u32 i, j;

    for (i=0; i<INPUT_MAX; ++i)
        for (j=0; j<STATE_MAX; ++j)
            nxt_1p1_bidir_nonrev[i][j] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_W] =      STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_P] =      STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_FORCED_SWITCH] =     STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_W] =     STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_P] =     STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_P] =   STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_W] =   STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_DO_NOT_REVERT] =     STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_EXERCISE_W] =        STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_EXERCISE_P] =        STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_REVERSE_REQUEST_W] = STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_REVERSE_REQUEST_P] = STATE_LOCKOUT;

    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_NO_REQUEST_W] =      STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_NO_REQUEST_P] =      STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_W] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_P] =   STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_W] =   STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_DO_NOT_REVERT] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_EXERCISE_W] =        STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_EXERCISE_P] =        STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_REVERSE_REQUEST_W] = STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_FORCED][STATE_REVERSE_REQUEST_P] = STATE_FORCED_SWITCH;

    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_W] =      STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_P] =      STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_P] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_DO_NOT_REVERT] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_EXERCISE_W] =        STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_EXERCISE_P] =        STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_REVERSE_REQUEST_W] = STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_REVERSE_REQUEST_P] = STATE_SIGNAL_FAIL_W;

    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_W] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_W] =      STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_P] =      STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_FORCED_SWITCH] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_W] =   STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_DO_NOT_REVERT] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_EXERCISE_W] =        STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_EXERCISE_P] =        STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_REVERSE_REQUEST_W] = STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_REVERSE_REQUEST_P] = STATE_SIGNAL_FAIL_P;

    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_P] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_W] =      STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_P] =      STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_DO_NOT_REVERT] =     STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_EXERCISE_W] =        STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_EXERCISE_P] =        STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_REVERSE_REQUEST_W] = STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_REVERSE_REQUEST_P] = STATE_MANUEL_SWITCH_P;

    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_W] =      STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_DO_NOT_REVERT] =     STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_EXERCISE_W] =        STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_EXERCISE_P] =        STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_REVERSE_REQUEST_W] = STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_REVERSE_REQUEST_P] = STATE_MANUEL_SWITCH_W;

    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_LOCKOUT] =           STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_FORCED_SWITCH] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_P] =   STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_EXERCISE_P] =        STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_CLEAR][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_W] =      STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_DO_NOT_REVERT] =     STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_REVERSE_REQUEST_W] = STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_REVERSE_REQUEST_P] = STATE_EXERCISE_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_SIGNAL_FAIL_P] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_EXERCISE_P] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_LO][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_EXERCISE_P] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_P][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_EXERCISE_P] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_FS][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_EXERCISE_P] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_SF_W][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_EXERCISE_P] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_EXERCISE_P] =        STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_W;

    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_EXERCISE_P] =        STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_WTR][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_NO_REQUEST_W] =      STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_DO_NOT_REVERT] =     STATE_REVERSE_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_REVERSE_REQUEST_P] = STATE_DO_NOT_REVERT;

    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_NO_REQUEST_P] =      STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_NO_REQUEST_P] =      STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1p1_bidir_nonrev[INPUT_RM_DNR][STATE_REVERSE_REQUEST_P] = STATE_DO_NOT_REVERT;
}

static void init_nxt_1f1_bidir_rev(void)
{
    u32 i, j;

    for (i=0; i<INPUT_MAX; ++i)
        for (j=0; j<STATE_MAX; ++j)
            nxt_1f1_bidir_rev[i][j] = STATE_NO_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_W] =      STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_P] =      STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_FORCED_SWITCH] =     STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_W] =     STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_P] =     STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_P] =   STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_WAIT_TO_RESTORE] =   STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_EXERCISE_W] =        STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_LOCKOUT][STATE_REVERSE_REQUEST_W] = STATE_LOCKOUT;

    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_NO_REQUEST_W] =      STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_NO_REQUEST_P] =      STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_W] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_P] =   STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_WAIT_TO_RESTORE] =   STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_EXERCISE_W] =        STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_FORCED][STATE_REVERSE_REQUEST_W] = STATE_FORCED_SWITCH;

    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_W] =      STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_P] =      STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_P] =   STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_WAIT_TO_RESTORE] =   STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_EXERCISE_W] =        STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_ON][STATE_REVERSE_REQUEST_W] = STATE_SIGNAL_FAIL_W;

    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_W] =     STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_W_OFF][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_W] =      STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_P] =      STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_FORCED_SWITCH] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_WAIT_TO_RESTORE] =   STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_EXERCISE_W] =        STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_ON][STATE_REVERSE_REQUEST_W] = STATE_SIGNAL_FAIL_P;

    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_P] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_LO_SF_P_OFF][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_W] =      STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_P] =      STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_WAIT_TO_RESTORE] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_EXERCISE_W] =        STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_P][STATE_REVERSE_REQUEST_W] = STATE_MANUEL_SWITCH_P;

    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_LO_MANUAL_W][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_LOCKOUT] =           STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_CLEAR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_W] =      STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_LO_EXERCISE][STATE_REVERSE_REQUEST_W] = STATE_EXERCISE_W;

    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_LO_WTR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_SIGNAL_FAIL_P] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_LO][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_SF_P][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_FS][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;

    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_SF_W][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;

    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_WAIT_TO_RESTORE] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_MS_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;

    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_RM_WTR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_NO_REQUEST_W] =      STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_RM_EXER_NULL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_RM_RR_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;

    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_WAIT_TO_RESTORE] =   STATE_WAIT_TO_RESTORE;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_rev[INPUT_RM_NR_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
}

static void init_nxt_1f1_bidir_nonrev(void)
{
    u32 i, j;

    for (i=0; i<INPUT_MAX; ++i)
        for (j=0; j<STATE_MAX; ++j)
            nxt_1f1_bidir_nonrev[i][j] = STATE_NO_REQUEST_W;

    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_W] =      STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_NO_REQUEST_P] =      STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_FORCED_SWITCH] =     STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_W] =     STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_SIGNAL_FAIL_P] =     STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_P] =   STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_MANUEL_SWITCH_W] =   STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_DO_NOT_REVERT] =     STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_EXERCISE_W] =        STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_EXERCISE_P] =        STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_REVERSE_REQUEST_W] = STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_LOCKOUT][STATE_REVERSE_REQUEST_P] = STATE_LOCKOUT;

    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_NO_REQUEST_W] =      STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_NO_REQUEST_P] =      STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_W] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_P] =   STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_MANUEL_SWITCH_W] =   STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_DO_NOT_REVERT] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_EXERCISE_W] =        STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_EXERCISE_P] =        STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_REVERSE_REQUEST_W] = STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_FORCED][STATE_REVERSE_REQUEST_P] = STATE_FORCED_SWITCH;

    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_W] =      STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_NO_REQUEST_P] =      STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_P] =   STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_MANUEL_SWITCH_W] =   STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_DO_NOT_REVERT] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_EXERCISE_W] =        STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_EXERCISE_P] =        STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_REVERSE_REQUEST_W] = STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_ON][STATE_REVERSE_REQUEST_P] = STATE_SIGNAL_FAIL_W;

    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_W] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_W_OFF][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_W] =      STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_NO_REQUEST_P] =      STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_FORCED_SWITCH] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_P] =   STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_MANUEL_SWITCH_W] =   STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_DO_NOT_REVERT] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_EXERCISE_W] =        STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_EXERCISE_P] =        STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_REVERSE_REQUEST_W] = STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_ON][STATE_REVERSE_REQUEST_P] = STATE_SIGNAL_FAIL_P;

    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_SIGNAL_FAIL_P] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_SF_P_OFF][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_W] =      STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_NO_REQUEST_P] =      STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_DO_NOT_REVERT] =     STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_EXERCISE_W] =        STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_EXERCISE_P] =        STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_REVERSE_REQUEST_W] = STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_P][STATE_REVERSE_REQUEST_P] = STATE_MANUEL_SWITCH_P;

    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_W] =      STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_DO_NOT_REVERT] =     STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_EXERCISE_W] =        STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_EXERCISE_P] =        STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_REVERSE_REQUEST_W] = STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_MANUAL_W][STATE_REVERSE_REQUEST_P] = STATE_MANUEL_SWITCH_W;

    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_LOCKOUT] =           STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_FORCED_SWITCH] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_P] =   STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_EXERCISE_P] =        STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_CLEAR][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_W] =      STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_DO_NOT_REVERT] =     STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_REVERSE_REQUEST_W] = STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_LO_EXERCISE][STATE_REVERSE_REQUEST_P] = STATE_EXERCISE_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_SIGNAL_FAIL_P] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_EXERCISE_P] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_LO][STATE_REVERSE_REQUEST_P] = STATE_DO_NOT_REVERT;

    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_FORCED_SWITCH] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_EXERCISE_P] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_P][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_W;

    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_SIGNAL_FAIL_W] =     STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_EXERCISE_P] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_FS][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_MANUEL_SWITCH_P] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_EXERCISE_P] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_SF_W][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_MANUEL_SWITCH_W] =   STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_EXERCISE_P] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NORMAL][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_EXERCISE_W] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_EXERCISE_P] =        STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_MS_NULL][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_W;

    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_DO_NOT_REVERT] =     STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_EXERCISE_W] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_EXERCISE_P] =        STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_WTR][STATE_REVERSE_REQUEST_P] = STATE_NO_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_NO_REQUEST_W] =      STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NULL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_DO_NOT_REVERT] =     STATE_REVERSE_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_EXER_NORMAL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NULL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_RR_NORMAL][STATE_REVERSE_REQUEST_P] = STATE_DO_NOT_REVERT;

    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_NO_REQUEST_P] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_REVERSE_REQUEST_W] = STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NULL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_NO_REQUEST_P] =      STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_NR_NORMAL][STATE_REVERSE_REQUEST_P] = STATE_REVERSE_REQUEST_P;

    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_NO_REQUEST_W] =      STATE_NO_REQUEST_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_NO_REQUEST_P] =      STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_LOCKOUT] =           STATE_LOCKOUT;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_FORCED_SWITCH] =     STATE_FORCED_SWITCH;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_SIGNAL_FAIL_W] =     STATE_SIGNAL_FAIL_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_SIGNAL_FAIL_P] =     STATE_SIGNAL_FAIL_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_MANUEL_SWITCH_P] =   STATE_MANUEL_SWITCH_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_MANUEL_SWITCH_W] =   STATE_MANUEL_SWITCH_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_DO_NOT_REVERT] =     STATE_DO_NOT_REVERT;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_EXERCISE_P] =        STATE_EXERCISE_P;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_EXERCISE_W] =        STATE_EXERCISE_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_REVERSE_REQUEST_W] = STATE_REVERSE_REQUEST_W;
    nxt_1f1_bidir_nonrev[INPUT_RM_DNR][STATE_REVERSE_REQUEST_P] = STATE_DO_NOT_REVERT;
}



static vtss_eps_mgmt_defect_state_t calc_defect_state(BOOL sf,   BOOL sd)
{
    if (sf)   return(VTSS_EPS_MGMT_DEFECT_STATE_SF);
    else
    if (sd)   return(VTSS_EPS_MGMT_DEFECT_STATE_SD);
    else      return(VTSS_EPS_MGMT_DEFECT_STATE_OK);
}



static u8 calc_local_command_input(vtss_eps_mgmt_command_t  command)
{
    switch (command)
    {
        case VTSS_EPS_MGMT_COMMAND_NONE:            return(INPUT_LO_CLEAR);
        case VTSS_EPS_MGMT_COMMAND_CLEAR:           return(INPUT_LO_CLEAR);
        case VTSS_EPS_MGMT_COMMAND_LOCK_OUT:        return(INPUT_LO_LOCKOUT);
        case VTSS_EPS_MGMT_COMMAND_FORCED_SWITCH:   return(INPUT_LO_FORCED);
        case VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_W: return(INPUT_LO_MANUAL_W);
        case VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_P: return(INPUT_LO_MANUAL_P);
        case VTSS_EPS_MGMT_COMMAND_EXERCISE:        return(INPUT_LO_EXERCISE);
        case VTSS_EPS_MGMT_COMMAND_FREEZE:          return(INPUT_LO_CLEAR);
        case VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL:  return(INPUT_LO_CLEAR);
        default:    vtss_eps_trace("calc_local_command_input: invalid command", (u32)command, 0, 0, 0);   return(INPUT_LO_CLEAR);
    }
}



static u8 calc_remote_request_input(const u8  *const aps)
{
    switch (aps[0] & 0xF0)
    {
        case 0xF0:                                return(INPUT_RM_LO);
        case 0xE0:                                return(INPUT_RM_SF_P);
        case 0xD0:                                return(INPUT_RM_FS);
        case 0xB0:                                return(INPUT_RM_SF_W);
        case 0x70:
        {
            if ((aps[1] == 0) || (aps[2] == 0))   return(INPUT_RM_MS_NULL);
            else                                  return(INPUT_RM_MS_NORMAL);
        }
        case 0x50:                                return(INPUT_RM_WTR);
        case 0x40:
        {
            if ((aps[1] == 0) || (aps[2] == 0))   return(INPUT_RM_EXER_NULL);
            else                                  return(INPUT_RM_EXER_NORMAL);
        }
        case 0x20:
        {
            if ((aps[1] == 0) || (aps[2] == 0))   return(INPUT_RM_RR_NULL);
            else                                  return(INPUT_RM_RR_NORMAL);
        }
        case 0x10:                                return(INPUT_RM_DNR);
        case 0x00:
        {
            if ((aps[1] == 0) || (aps[2] == 0))   return(INPUT_RM_NR_NULL);
            else                                  return(INPUT_RM_NR_NORMAL);
        }
        default:    vtss_eps_trace("calc_remote_request_input: invalid APS", aps[0], 0, 0, 0);  return(INPUT_RM_NR_NORMAL);
    }
}



static void calc_tx_aps(u32 instance,    u32 state,   u8 *aps)
{
    eps_instance_data_t *data;

    data = &instance_data[instance];    /* Instance data reference */

    aps[0] = 0;
    if (data->aps_mode)                                               aps[0] |= 0x08;
    if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1F1)   aps[0] |= 0x04;
    if (data->config.directional == VTSS_EPS_MGMT_BIDIRECTIONAL)      aps[0] |= 0x02;
    if (data->config.revertive)                                       aps[0] |= 0x01;

    switch (state)
    {
        case STATE_NO_REQUEST_W:
            aps[1] = 0; aps[2] = 0;
            if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)          aps[2] = 1;
            break;
        case STATE_NO_REQUEST_P:   aps[1] = 1; aps[2] = 1;   break;
        case STATE_LOCKOUT:
            aps[0] |= 0xF0; aps[1] = 0; aps[2] = 0;
            if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)          aps[2] = 1;
            break;
        case STATE_FORCED_SWITCH:   aps[0] |= 0xD0; aps[1] = 1; aps[2] = 1;  break;
        case STATE_SIGNAL_FAIL_W:   aps[0] |= 0xB0; aps[1] = 1; aps[2] = 1;  break;
        case STATE_SIGNAL_FAIL_P:
            aps[0] |= 0xE0; aps[1] = 0; aps[2] = 0;
            if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)          aps[2] = 1;
            break;
        case STATE_MANUEL_SWITCH_P:   aps[0] |= 0x70; aps[1] = 1; aps[2] = 1;  break;
        case STATE_MANUEL_SWITCH_W:
            aps[0] |= 0x70; aps[1] = 0; aps[2] = 0;
            if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)          aps[2] = 1;
            break;
        case STATE_WAIT_TO_RESTORE: aps[0] |= 0x50; aps[1] = 1; aps[2] = 1;  break;
        case STATE_EXERCISE_W:
            aps[0] |= 0x40; aps[1] = 0; aps[2] = 0;
            if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)          aps[2] = 1;
            break;
        case STATE_EXERCISE_P:      aps[0] |= 0x40; aps[1] = 1; aps[2] = 1;  break;
        case STATE_REVERSE_REQUEST_W:
            aps[0] |= 0x20; aps[1] = 0; aps[2] = 0;
            if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)          aps[2] = 1;
            break;
        case STATE_REVERSE_REQUEST_P: aps[0] |= 0x20; aps[1] = 1; aps[2] = 1;  break;
        case STATE_DO_NOT_REVERT:   aps[0] |= 0x10; aps[1] = 1; aps[2] = 1;  break;
        default:    vtss_eps_trace("calc_tx_aps: invalid state", instance, state, 0, 0);
    }
}



static void calc_protection_state(u32 state,    vtss_eps_mgmt_prot_state_t *pr_state)
{
    switch (state)
    {
        case STATE_NO_REQUEST_W:      *pr_state = VTSS_EPS_MGMT_PROT_STATE_NO_REQUEST_W;       break;
        case STATE_NO_REQUEST_P:      *pr_state = VTSS_EPS_MGMT_PROT_STATE_NO_REQUEST_P;       break;
        case STATE_LOCKOUT:           *pr_state = VTSS_EPS_MGMT_PROT_STATE_LOCKOUT;            break;
        case STATE_FORCED_SWITCH:     *pr_state = VTSS_EPS_MGMT_PROT_STATE_FORCED_SWITCH;      break;
        case STATE_SIGNAL_FAIL_W:     *pr_state = VTSS_EPS_MGMT_PROT_STATE_SIGNAL_FAIL_W;      break;
        case STATE_SIGNAL_FAIL_P:     *pr_state = VTSS_EPS_MGMT_PROT_STATE_SIGNAL_FAIL_P;      break;
        case STATE_MANUEL_SWITCH_W:   *pr_state = VTSS_EPS_MGMT_PROT_STATE_MANUEL_SWITCH_W;    break;
        case STATE_MANUEL_SWITCH_P:   *pr_state = VTSS_EPS_MGMT_PROT_STATE_MANUEL_SWITCH_P;    break;
        case STATE_WAIT_TO_RESTORE:   *pr_state = VTSS_EPS_MGMT_PROT_STATE_WAIT_TO_RESTORE;    break;
        case STATE_EXERCISE_W:        *pr_state = VTSS_EPS_MGMT_PROT_STATE_EXERCISE_W;         break;
        case STATE_EXERCISE_P:        *pr_state = VTSS_EPS_MGMT_PROT_STATE_EXERCISE_P;         break;
        case STATE_REVERSE_REQUEST_W: *pr_state = VTSS_EPS_MGMT_PROT_STATE_REVERSE_REQUEST_W;  break;
        case STATE_REVERSE_REQUEST_P: *pr_state = VTSS_EPS_MGMT_PROT_STATE_REVERSE_REQUEST_P;  break;
        case STATE_DO_NOT_REVERT:     *pr_state = VTSS_EPS_MGMT_PROT_STATE_DO_NOT_REVERT;      break;
        default:    vtss_eps_trace("calc_protection_state: invalid state", state, 0, 0, 0);
    }
}



static void calc_aps_request(const u8 *const aps,    vtss_eps_mgmt_aps_info_t *request)
{
    request->re_signal = aps[1];
    request->br_signal = aps[2];

    switch (aps[0] & 0xF0)
    {
        case 0xF0:   request->request = VTSS_EPS_MGMT_REQUEST_LO;     break;
        case 0xE0:   request->request = VTSS_EPS_MGMT_REQUEST_SF_P;   break;
        case 0xD0:   request->request = VTSS_EPS_MGMT_REQUEST_FS;     break;
        case 0xB0:   request->request = VTSS_EPS_MGMT_REQUEST_SF_W;   break;
        case 0x90:   request->request = VTSS_EPS_MGMT_REQUEST_SD;     break;
        case 0x70:   request->request = VTSS_EPS_MGMT_REQUEST_MS;     break;
        case 0x50:   request->request = VTSS_EPS_MGMT_REQUEST_WTR;    break;
        case 0x40:   request->request = VTSS_EPS_MGMT_REQUEST_EXER;   break;
        case 0x20:   request->request = VTSS_EPS_MGMT_REQUEST_RR;     break;
        case 0x10:   request->request = VTSS_EPS_MGMT_REQUEST_DNR;    break;
        case 0x00:   request->request = VTSS_EPS_MGMT_REQUEST_NR;     break;
        default:    vtss_eps_trace("calc_aps_request: invalid APS", aps[0], 0, 0, 0);
    }
}



static vtss_eps_selector_t calc_selector_state(u32 state)
{
    switch (state)
    {
        case STATE_NO_REQUEST_W:      return(VTSS_EPS_SELECTOR_WORKING);
        case STATE_NO_REQUEST_P:      return(VTSS_EPS_SELECTOR_PROTECTION);
        case STATE_LOCKOUT:           return(VTSS_EPS_SELECTOR_WORKING);
        case STATE_FORCED_SWITCH:     return(VTSS_EPS_SELECTOR_PROTECTION);
        case STATE_SIGNAL_FAIL_W:     return(VTSS_EPS_SELECTOR_PROTECTION);
        case STATE_SIGNAL_FAIL_P:     return(VTSS_EPS_SELECTOR_WORKING);
        case STATE_MANUEL_SWITCH_W:   return(VTSS_EPS_SELECTOR_WORKING);
        case STATE_MANUEL_SWITCH_P:   return(VTSS_EPS_SELECTOR_PROTECTION);
        case STATE_WAIT_TO_RESTORE:   return(VTSS_EPS_SELECTOR_PROTECTION);
        case STATE_EXERCISE_W:        return(VTSS_EPS_SELECTOR_WORKING);
        case STATE_EXERCISE_P:        return(VTSS_EPS_SELECTOR_PROTECTION);
        case STATE_REVERSE_REQUEST_W: return(VTSS_EPS_SELECTOR_WORKING);
        case STATE_REVERSE_REQUEST_P: return(VTSS_EPS_SELECTOR_PROTECTION);
        case STATE_DO_NOT_REVERT:     return(VTSS_EPS_SELECTOR_PROTECTION);
        default:
            vtss_eps_trace("calc_selector_state: invalid state", state, 0, 0, 0);
            return(VTSS_EPS_SELECTOR_WORKING);
    }
}



static void calc_request_fop(u32 instance, const u8 rx_aps[VTSS_EPS_APS_DATA_LENGTH])
{
    if (instance_data[instance].config.directional == VTSS_EPS_MGMT_UNIDIRECTIONAL)
    {
        instance_data[instance].state.dFop_nr = FALSE;
        return;
    }
    /* This is bidirectional */
    if (instance_data[instance].tx_aps[1] != rx_aps[1])
    {
    /*  Mismatch in transmitted and received requested signal - dFop_nr calculation */
        if (!instance_data[instance].state.dFop_nr)
        {
            instance_data[instance].dFop_nr_timer = (50/timer_res)+1;
            vtss_eps_timer_start();
        }
    }
    else
    {
        instance_data[instance].state.dFop_nr = FALSE;
        instance_data[instance].dFop_nr_timer = 0;
    }
}



static void check_1_for_N_port(eps_instance_data_t *data, u32 p_port, BOOL active)
{
    u32                       i;
    vtss_eps_mgmt_command_t   command;

    if (!active && (data->command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT))    return; /* The insatnce that is in locked out command should be ignored */
    for (i=0; i<VTSS_EPS_MGMT_CREATED_MAX; ++i)
    {
        if ((data != &instance_data[i]) && instance_data[i].created && instance_data[i].configured &&
            (instance_data[i].param.domain == VTSS_EPS_MGMT_PORT) && (instance_data[i].param.p_flow == p_port))
        {/* This EPS is not the calling EPS and it is created to have the same protection port - must be 1:N then */
            /* This EPS is hindered to do protection switch by a 'Lock out' command */
            command = (active) ? VTSS_EPS_MGMT_COMMAND_LOCK_OUT : VTSS_EPS_MGMT_COMMAND_CLEAR;
            if (((command == VTSS_EPS_MGMT_COMMAND_CLEAR) && (instance_data[i].command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT)) ||
                ((command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT) && (instance_data[i].command != VTSS_EPS_MGMT_COMMAND_LOCK_OUT)))
            {/* Only apply command if relevant - othervise it will create a "command loop" */
                instance_data[i].command = command;
                instance_data[i].event_flags |= EVENT_IN_COMMAND;
                vtss_eps_run();
            }
        }
    }
}

static void selector_state_set(eps_instance_data_t *data,    vtss_eps_selector_t selector)
{
    vtss_rc rc = VTSS_RC_OK;

    if (data->param.domain == VTSS_EPS_MGMT_PORT)
    {
/*vtss_eps_trace("selector_state_set - port - selector", data->param.w_flow, selector, 0, 0);*/
        vtss_eps_trace("selector_state_set: vtss_eps_port_selector_set  w_flow  selector ", data->param.w_flow | 0xF0000000, selector, 0, 0);
        rc = vtss_eps_port_selector_set(NULL, data->param.w_flow, selector);
        data->event_flags |= EVENT_OUT_PORT_PROT;     /* Activate callout of port protection state */
        check_1_for_N_port(data, data->param.p_flow, (selector == VTSS_EPS_SELECTOR_PROTECTION));
    }
#if defined(VTSS_SW_OPTION_EVC)
#if 0
    if ((data->param.domain == VTSS_EPS_MGMT_PATH) || (data->param.domain == VTSS_EPS_MGMT_SERVICE))
    {
/*vtss_eps_trace("selector_state_set - EVC - selector", data->param.w_flow, selector, 0, 0);*/
        vtss_eps_trace("selector_state_set: vtss_eps_evc_selector_set  w_flow  selector ", data->param.w_flow | 0xF0000000, selector, 0, 0);
        rc = vtss_eps_evc_selector_set(NULL, data->param.w_flow, selector);
    }
#endif
#endif
    if (rc != VTSS_RC_OK)       vtss_eps_trace("selector_state_set: selector set failed", data->param.w_flow, 0, 0, 0);
}



static void run_state(u32 instance)
{
    eps_instance_data_t  *data;
    u8                   (*nxt)[STATE_MAX];
    u8                   state, from_state, nxt_state, input_w, input_p, input_l, input_r, state_cnt;

    data = &instance_data[instance];    /* Instance data reference */

    if (!data->configured)
    {
        vtss_eps_trace("run_state: not configured", 0, 0, 0, 0);
        return;
    }

    if (data->command == VTSS_EPS_MGMT_COMMAND_FREEZE)      return; /* Local Freeze - no change in state */

    nxt = 0;
    /* Find state transition table for this configuration */
    if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)
        if (data->config.directional == VTSS_EPS_MGMT_UNIDIRECTIONAL)
            nxt = (data->config.revertive) ? nxt_1p1_unidir_rev : nxt_1p1_unidir_nonrev;

    if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)
        if (data->config.directional == VTSS_EPS_MGMT_BIDIRECTIONAL)
            nxt = (data->config.revertive) ? nxt_1p1_bidir_rev : nxt_1p1_bidir_nonrev;

    if (data->param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1F1)
        if (data->config.directional == VTSS_EPS_MGMT_BIDIRECTIONAL)
            nxt = (data->config.revertive) ? nxt_1f1_bidir_rev : nxt_1f1_bidir_nonrev;

    /* Check if any state transition table was found for this configuration */
    if (nxt == 0)
    {
        vtss_eps_trace("run_state: wrong configuration", 0, 0, 0, 0);
        return;
    }

    /* calculate all static input */
    input_w = (data->state.w_state != VTSS_EPS_MGMT_DEFECT_STATE_OK) ? INPUT_LO_SF_W_ON : INPUT_LO_SF_W_OFF;
    input_p = (data->state.p_state != VTSS_EPS_MGMT_DEFECT_STATE_OK) ? INPUT_LO_SF_P_ON : INPUT_LO_SF_P_OFF;
    input_l = calc_local_command_input(data->command);
    input_r = calc_remote_request_input(data->rx_aps);
    if (data->command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL)      input_w = INPUT_LO_SF_W_OFF;         /* Ignore SF on working state when Local Lockout is active */
vtss_eps_trace("run_state - instance", instance, 0, 0, 0);
vtss_eps_trace("run_state 1 - input_w - input_p - input_l - input_r", input_w, input_p, input_l, input_r);

    /* Repeat until state does not change with the current input */
    from_state = data->internal_state;
    nxt_state = data->internal_state;
    state_cnt = 0;
/*vtss_eps_trace("run_state 2 - nxt_state", nxt_state, 0, 0, 0);*/
    do
    {
        state = nxt_state;      /* Save the state */
        if (data->wtr_event)
        {
/*vtss_eps_trace("run_state 3 - WTR", 0, 0, 0, 0);*/
            data->wtr_event = FALSE;                    /* WTR is an event - not an static input */
            nxt_state = nxt[INPUT_LO_WTR][nxt_state];
        }
        nxt_state = nxt[input_w][nxt_state];
        nxt_state = nxt[input_p][nxt_state];
        if ((data->command != VTSS_EPS_MGMT_COMMAND_NONE) && (data->command != VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL))    /* Only use local command if NOT NONE or LOCK_OUT_LOCAL */
            nxt_state = nxt[input_l][nxt_state];
        if (data->config.directional == VTSS_EPS_MGMT_BIDIRECTIONAL)    /* Only use remote request if bidirectional protection type */
        {
            if ((data->config.revertive) && (input_r == INPUT_RM_NR_NORMAL) && data->comming_from_sf)   nxt_state = STATE_WAIT_TO_RESTORE;  /* This handle the case where both endes detect SF working OFF at the same time - 8031 clause 11.13 */
            else                                                                                        nxt_state = nxt[input_r][nxt_state];
        }
        state_cnt++;
    }
    while ((nxt_state != state) && (state_cnt < 10));
/*vtss_eps_trace("run_state 4 - nxt_state - state_cnt", nxt_state, state_cnt, 0, 0);*/

    if (data->command != VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL) {
        /* A local end to end command must be cleard when "overridden" (G.8031 11.11) */
        switch (nxt_state)
        {
            case STATE_LOCKOUT:         data->command = VTSS_EPS_MGMT_COMMAND_LOCK_OUT; break;
            case STATE_FORCED_SWITCH:   data->command = VTSS_EPS_MGMT_COMMAND_FORCED_SWITCH; break;
            case STATE_MANUEL_SWITCH_W: data->command = VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_W; break;
            case STATE_MANUEL_SWITCH_P: data->command = VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_P; break;
            case STATE_EXERCISE_W:      data->command = VTSS_EPS_MGMT_COMMAND_EXERCISE; break;
            case STATE_EXERCISE_P:      data->command = VTSS_EPS_MGMT_COMMAND_EXERCISE; break;
            default:                    data->command = VTSS_EPS_MGMT_COMMAND_NONE;             /* Clear if not in a local command state */
        }
    }

    /* This handle the case where both endes detect SF working OFF at the same time - 8031 clause 11.13 */
    if (nxt_state == STATE_NO_REQUEST_P)
    {
        if (from_state == STATE_SIGNAL_FAIL_W)  data->comming_from_sf = TRUE;   /* Comming from SF on working */
    }
    else    data->comming_from_sf = FALSE;

    /* Check for loop exit on max loop counter */
    if (state_cnt == 10)
        vtss_eps_trace("run_state: Repeat until max", 0, 0, 0, 0);

    /* If we are entering WTR state the wtr timer must start */
    if ((nxt_state == STATE_WAIT_TO_RESTORE) && (data->internal_state != STATE_WAIT_TO_RESTORE))
    {
        data->wtr_timer = data->config.restore_timer * (1000/timer_res);
        if (data->wtr_timer == 0)     data->wtr_timer = 1;  /* Must be one count to change state on time-out */
        vtss_eps_timer_start();
    }

    /* If we are not in WTR state the wtr timer must stop */
    if (nxt_state != STATE_WAIT_TO_RESTORE)
        data->wtr_timer = 0;

    /* Internal state is now 'next_state' */
    data->internal_state = nxt_state;

    /* Control the traffic selector */
    selector_state_set(data, calc_selector_state(nxt_state));

    if (data->aps_mode)
    {
        /* calculate transmitted APS for this state */
        calc_tx_aps(instance, nxt_state, data->tx_aps);
        data->event_flags |= EVENT_OUT_APS;
        /* Calculate request FOP (fop_nr) */
        calc_request_fop(instance, data->rx_aps);
        /* calculate transmitting APS request for management */
        calc_aps_request(data->tx_aps, &data->state.tx_aps);
    }

    /* calculate protection state for management */
    calc_protection_state(nxt_state, &data->state.protection_state);
}




static void run_wtr(u32 instance)
{
    instance_data[instance].event_flags &= ~EVENT_IN_WTR;
    instance_data[instance].wtr_event = TRUE;

    run_state(instance);
}



static void run_hold(u32 instance)
{
    vtss_eps_mgmt_defect_state_t state;

    if (instance_data[instance].event_flags & EVENT_IN_HOFF_W)
    {
        instance_data[instance].event_flags &= ~EVENT_IN_HOFF_W;

        state = calc_defect_state(instance_data[instance].sf_w, instance_data[instance].sd_w);

        if (state != VTSS_EPS_MGMT_DEFECT_STATE_OK)
        {
            instance_data[instance].state.w_state = state;
            instance_data[instance].hoff_w_timer = 0;

            run_state(instance);
        }
    }

    if (instance_data[instance].event_flags & EVENT_IN_HOFF_P)
    {
        instance_data[instance].event_flags &= ~EVENT_IN_HOFF_P;

        state = calc_defect_state(instance_data[instance].sf_p, instance_data[instance].sd_p);

        if (state != VTSS_EPS_MGMT_DEFECT_STATE_OK)
        {
            instance_data[instance].state.p_state = state;
            instance_data[instance].hoff_p_timer = 0;

            run_state(instance);
        }
    }
}



static void run_command(u32 instance)
{
    instance_data[instance].event_flags &= ~EVENT_IN_COMMAND;

    if (instance_data[instance].command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL)
    /* Lock Out Local requires recalculation of internal state - where SF/SD on working is ignored. (See run_state()) */
        instance_data[instance].internal_state = STATE_NO_REQUEST_W;

    run_state(instance);

    if (instance_data[instance].command == VTSS_EPS_MGMT_COMMAND_CLEAR)
        instance_data[instance].command = VTSS_EPS_MGMT_COMMAND_NONE;
}



static void run_sf(u32 instance)
{
    vtss_eps_mgmt_defect_state_t state;

    if (instance_data[instance].event_flags & EVENT_IN_SF_W)
    {
        instance_data[instance].event_flags &= ~EVENT_IN_SF_W;

        state = calc_defect_state(instance_data[instance].sf_w, instance_data[instance].sd_w);

        if ((state != VTSS_EPS_MGMT_DEFECT_STATE_OK) && (instance_data[instance].config.hold_off_timer != VTSS_EPS_MGMT_HOFF_OFF))
        {
        /* Working state is not OK and hold off is required - timer must run */
            instance_data[instance].hoff_w_timer = instance_data[instance].config.hold_off_timer * (100/timer_res);
            vtss_eps_timer_start();
        }
        else
        {
        /* No hold off timing - do the job */
            instance_data[instance].state.w_state = state;
            instance_data[instance].hoff_w_timer = 0;

            run_state(instance);
        }
    }

    if (instance_data[instance].event_flags & EVENT_IN_SF_P)
    {
        instance_data[instance].event_flags &= ~EVENT_IN_SF_P;

        state = calc_defect_state(instance_data[instance].sf_p, instance_data[instance].sd_p);

        if ((state != VTSS_EPS_MGMT_DEFECT_STATE_OK) && (instance_data[instance].config.hold_off_timer != VTSS_EPS_MGMT_HOFF_OFF))
        {
        /* Protection state is not OK and hold off is required - timer must run */
            instance_data[instance].hoff_p_timer = instance_data[instance].config.hold_off_timer * (100/timer_res);
            vtss_eps_timer_start();
        }
        else
        {
        /* No hold off timing - do the job */
            instance_data[instance].state.p_state = state;
            instance_data[instance].hoff_p_timer = 0;

            run_state(instance);
        }
    }
}



static void run_async_aps(u32 instance)
{
    instance_data[instance].event_flags &= ~EVENT_IN_APS;

    if (instance_data[instance].aps_mode)
    {
        /* calculate receivinging APS request for management */
        calc_aps_request(instance_data[instance].rx_aps, &instance_data[instance].state.rx_aps);

        /* Run state machine */
        run_state(instance);
    }
}




static void run_sync_aps(u32 instance,   vtss_eps_flow_type_t flow,   const u8 aps[VTSS_EPS_APS_DATA_LENGTH])
{
    if (instance_data[instance].aps_mode)
    {
        /* Do dFOP calculation */
        if (flow == VTSS_EPS_FLOW_WORKING)
        {
        /*  APS received from working MEP - dFop_cm calculation */
            instance_data[instance].state.dFop_cm = (aps[0]&0x08);   /* Check for "no APS channel" */
        }
        else
        {
        /*  APS received from protecting MEP */
            if (!(aps[0]&0x08))   /* Check for "no APS channel" */
            {
                instance_data[instance].state.dFop_NoAps = TRUE;
                instance_data[instance].state.dFop_pm = FALSE;
                instance_data[instance].state.dFop_nr = FALSE;
            }
            else
            {
                instance_data[instance].state.dFop_NoAps = FALSE;
                /* Calculate request FOP (fop_nr) */
                calc_request_fop(instance, aps);
                /*  Architecture mismatch - dFop_pm calculation */
                instance_data[instance].state.dFop_pm = ((aps[0]&0x04) != (instance_data[instance].tx_aps[0]&0x04));
            }
        }
    }
}



static void run_config(u32 instance)
{
    BOOL old_aps;
    u32  i;

    instance_data[instance].event_flags &= ~EVENT_IN_CONFIG;
    instance_data[instance].configured = TRUE;

    memset(&instance_data[instance].state, 0, sizeof(vtss_eps_mgmt_state_t));
    memset(instance_data[instance].rx_aps, 0, VTSS_EPS_APS_DATA_LENGTH);

    old_aps = instance_data[instance].aps_mode;
    instance_data[instance].aps_mode = (instance_data[instance].param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1F1) ||
                                       (instance_data[instance].config.directional == VTSS_EPS_MGMT_BIDIRECTIONAL) ||
                                       instance_data[instance].config.aps;

    if (!old_aps && instance_data[instance].aps_mode)
        instance_data[instance].state.dFop_NoAps = TRUE;            // No APS received yet */

    /* new configuration - get all relevant input */
    instance_data[instance].event_flags |= EVENT_OUT_SIGNAL;

    for (i=0; i<VTSS_EPS_MGMT_CREATED_MAX; ++i)
    {
        if ((i != instance) && instance_data[i].created && instance_data[i].configured &&
            (instance_data[i].param.domain == VTSS_EPS_MGMT_PORT) &&
            (instance_data[i].param.p_flow == instance_data[instance].param.p_flow) &&
            (calc_selector_state(instance_data[i].internal_state) == VTSS_EPS_SELECTOR_PROTECTION))
        /* This EPS is not the configured EPS and it is created to have the same protection port (must be 1:N then) and the selector is on protection */
            /* The configured EPS is hindered to do protection switch by a 'Lock out' command */
            instance_data[instance].command = VTSS_EPS_MGMT_COMMAND_LOCK_OUT;
    }
}



static void run_create(u32 instance)
{
    vtss_rc                   rc = VTSS_RC_OK;
    vtss_eps_mgmt_def_conf_t  def_conf;
    vtss_eps_port_conf_t      port_conf;

    instance_data[instance].event_flags &= ~EVENT_IN_CREATE;

    vtss_eps_mgmt_def_conf_get(&def_conf);
    instance_data[instance].config = def_conf.config;   /* When created the configuration is set to default */

    if (instance_data[instance].param.domain == VTSS_EPS_MGMT_PORT)
    {
        if (instance_data[instance].param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)    port_conf.type = VTSS_EPS_PORT_1_PLUS_1;
        if (instance_data[instance].param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1F1)    port_conf.type = VTSS_EPS_PORT_1_FOR_1;
        port_conf.port_no = instance_data[instance].param.p_flow;
        vtss_eps_trace("run_create: vtss_eps_port_conf_set  w_flow  type  port_no", instance_data[instance].param.w_flow | 0xF0000000, port_conf.type, port_conf.port_no, 0);
        rc = vtss_eps_port_conf_set(NULL, instance_data[instance].param.w_flow, &port_conf);
    }

#if defined(VTSS_SW_OPTION_EVC)
#if 0
    vtss_eps_evc_conf_t     evc_conf;

    if ((instance_data[instance].param.domain == VTSS_EPS_MGMT_PATH) || (instance_data[instance].param.domain == VTSS_EPS_MGMT_SERVICE))
    {
        evc_conf.evc_id = instance_data[instance].param.p_flow;
        vtss_eps_trace("run_create: vtss_eps_evc_conf_set  w_flow  evc_id", instance_data[instance].param.w_flow | 0xF0000000, evc_conf.evc_id, 0, 0);
        rc = vtss_eps_evc_conf_set(NULL, instance_data[instance].param.w_flow, &evc_conf);
    }
#endif
#endif
    if (rc != VTSS_RC_OK)       vtss_eps_trace("run_create: protection create failed", instance_data[instance].param.w_flow, 0, 0, 0);

    selector_state_set(&instance_data[instance],    VTSS_EPS_SELECTOR_WORKING);
}



static void run_delete(u32 instance)
{
    vtss_rc                 rc = VTSS_RC_OK;
    vtss_eps_port_conf_t    port_conf;

    instance_data[instance].event_flags &= ~EVENT_IN_DELETE;

    selector_state_set(&instance_data[instance],    VTSS_EPS_SELECTOR_WORKING);

    if (instance_data[instance].param.domain == VTSS_EPS_MGMT_PORT)
    {
        if (instance_data[instance].param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1)    port_conf.type = VTSS_EPS_PORT_1_PLUS_1;
        if (instance_data[instance].param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1F1)    port_conf.type = VTSS_EPS_PORT_1_FOR_1;
        port_conf.port_no = VTSS_PORT_NO_NONE;
        vtss_eps_trace("run_delete: vtss_eps_port_conf_set  w_flow  type  port_no", instance_data[instance].param.w_flow | 0xF0000000, port_conf.type, port_conf.port_no, 0);
        rc = vtss_eps_port_conf_set(NULL, instance_data[instance].param.w_flow, &port_conf);
    }

#if defined(VTSS_SW_OPTION_EVC)
#if 0
    vtss_eps_evc_conf_t     evc_conf;

    if ((instance_data[instance].param.domain == VTSS_EPS_MGMT_PATH) || (instance_data[instance].param.domain == VTSS_EPS_MGMT_SERVICE))
    {
        evc_conf.evc_id = VTSS_EVC_ID_NONE;
        vtss_eps_trace("run_delete: vtss_eps_evc_conf_set  w_flow  evc_id", instance_data[instance].param.w_flow | 0xF0000000, evc_conf.evc_id, 0, 0);
        rc = vtss_eps_evc_conf_set(NULL, instance_data[instance].param.w_flow, &evc_conf);
    }
#endif
#endif

    if (rc != VTSS_RC_OK)       vtss_eps_trace("run_delete: protection config failed", instance_data[instance].param.w_flow, 0, 0, 0);

    if (instance_data[instance].configured)
    {
        /* No more transmitting of APS */
        instance_data[instance].config.aps = FALSE;

        /* calculate transmitted APS for this state */
        calc_tx_aps(instance, STATE_NO_REQUEST_W, instance_data[instance].tx_aps);

        /* Activate transmit the APS */
        instance_data[instance].event_flags |= EVENT_OUT_APS;
    }

    instance_data_clear(&instance_data[instance]);
}



static void run_dFop_nr(u32 instance)
{
    instance_data[instance].event_flags &= ~EVENT_IN_DFOP_NR;
    instance_data[instance].state.dFop_nr = TRUE;
}






/****************************************************************************/
/*  EPS management interface                                                */
/****************************************************************************/

u32 vtss_eps_mgmt_instance_create(const u32                             instance,
                                  const vtss_eps_mgmt_create_param_t    *const param)
{
    u32 i;

    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)                 return(VTSS_EPS_RC_INVALID_PARAMETER);

    vtss_eps_crit_lock();

    if (instance_data[instance].created)                       {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_CREATED);}
    if (param->domain != VTSS_EPS_MGMT_PORT)                   {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if ((param->architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1) && 
        (param->domain != VTSS_EPS_MGMT_PORT))                 {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_ARCHITECTURE);}
    if (param->w_flow == param->p_flow)                        {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_W_P_EQUAL);}

    for (i=0; i<VTSS_EPS_MGMT_CREATED_MAX; ++i)
    {
        if ((i != instance) && instance_data[i].created)
        {
            if (param->domain == instance_data[i].param.domain)
            {
                if (param->w_flow == instance_data[i].param.w_flow)       {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_WORKING_USED);}
                if ((instance_data[i].param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1) || (param->architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1))
                    if (param->p_flow == instance_data[i].param.p_flow)   {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_PROTECTING_USED);}
            }
        }
    }

    instance_data[instance].event_flags |= EVENT_IN_CREATE;

    instance_data[instance].param = *param;
    instance_data[instance].created = TRUE;
    instance_data[instance].configured = FALSE;

    vtss_eps_run();

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}



u32 vtss_eps_mgmt_instance_delete(const u32     instance)
{
    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)          return(VTSS_EPS_RC_INVALID_PARAMETER);

    vtss_eps_crit_lock();

    if (!instance_data[instance].created)               {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}

    instance_data[instance].event_flags |= EVENT_IN_DELETE;

    vtss_eps_run();

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}



u32 vtss_eps_mgmt_conf_set(const u32                    instance,
                           const vtss_eps_mgmt_conf_t   *const conf)
{
    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)                                            return(VTSS_EPS_RC_INVALID_PARAMETER);

    vtss_eps_crit_lock();

    if (!instance_data[instance].created)                                                 {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}
    if (instance_data[instance].configured &&
        !memcmp(&instance_data[instance].config, conf, sizeof(vtss_eps_mgmt_conf_t)))     {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_OK);}
    if (conf->restore_timer > VTSS_EPS_MGMT_WTR_MAX)                                      {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if (conf->hold_off_timer > VTSS_EPS_MGMT_HOFF_MAX)                                    {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}

    instance_data[instance].event_flags |= EVENT_IN_CONFIG;

    instance_data[instance].config = *conf;
    instance_data[instance].configured = TRUE;
    memset(instance_data[instance].tx_aps, 0, VTSS_EPS_APS_DATA_LENGTH);

    if (instance_data[instance].param.architecture ==  VTSS_EPS_MGMT_ARCHITECTURE_1F1) {
        instance_data[instance].config.directional = VTSS_EPS_MGMT_BIDIRECTIONAL;   /* 1:1 must be bidirectional with APS */
        instance_data[instance].config.aps = TRUE;
    }
    if ((instance_data[instance].param.architecture ==  VTSS_EPS_MGMT_ARCHITECTURE_1P1) &&
        (conf->directional == VTSS_EPS_MGMT_BIDIRECTIONAL)) /* 1+1 bidirectional must be with APS */
        instance_data[instance].config.aps = TRUE;

    vtss_eps_run();

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}



u32 vtss_eps_mgmt_conf_get(const u32                         instance,
                           vtss_eps_mgmt_create_param_t      *const param,
                           vtss_eps_mgmt_conf_t              *const conf)
{
    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)          return(VTSS_EPS_RC_INVALID_PARAMETER);

    vtss_eps_crit_lock();

    if (!instance_data[instance].created)               {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}

    *param = instance_data[instance].param;
    *conf = instance_data[instance].config;

    if (!instance_data[instance].configured)            {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CONFIGURED);}

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}



u32 vtss_eps_mgmt_command_set(const u32                         instance,
                              const vtss_eps_mgmt_command_t     command)
{
    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)                                                                                  return(VTSS_EPS_RC_INVALID_PARAMETER);

    vtss_eps_crit_lock();

    if (!instance_data[instance].created)                                                                                       {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}
    if (!instance_data[instance].configured)                                                                                    {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CONFIGURED);}
    if (command == instance_data[instance].command)                                                                             {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_OK);}
    if ((instance_data[instance].command != VTSS_EPS_MGMT_COMMAND_NONE) && (command == VTSS_EPS_MGMT_COMMAND_NONE))             {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if (command == VTSS_EPS_MGMT_COMMAND_NONE)                                                                                  {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_OK);}
    if ((instance_data[instance].command == VTSS_EPS_MGMT_COMMAND_FREEZE) && (command != VTSS_EPS_MGMT_COMMAND_CLEAR))          {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if ((instance_data[instance].command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL) && (command != VTSS_EPS_MGMT_COMMAND_CLEAR))  {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if ((command == VTSS_EPS_MGMT_COMMAND_FREEZE) && (instance_data[instance].command != VTSS_EPS_MGMT_COMMAND_NONE))           {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if ((command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL) && (instance_data[instance].command != VTSS_EPS_MGMT_COMMAND_NONE))   {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}

    instance_data[instance].command = command;
    instance_data[instance].event_flags |= EVENT_IN_COMMAND;

    vtss_eps_run();

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}




u32 vtss_eps_mgmt_command_get(const u32                   instance,
                              vtss_eps_mgmt_command_t     *const command)
{
    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)           return(VTSS_EPS_RC_INVALID_PARAMETER);

    vtss_eps_crit_lock();

    if (!instance_data[instance].created)                {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}

    *command = instance_data[instance].command;

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}



u32 vtss_eps_mgmt_state_get(const u32                 instance,
                            vtss_eps_mgmt_state_t     *const state)
{
    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)           return(VTSS_EPS_RC_INVALID_PARAMETER);

    vtss_eps_crit_lock();

    if (!instance_data[instance].created)                {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}

    *state = instance_data[instance].state;

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}



void  vtss_eps_mgmt_def_conf_get(vtss_eps_mgmt_def_conf_t  *const def_conf)
{
    memset(def_conf, 0, sizeof(*def_conf));

    def_conf->param.domain = VTSS_EPS_MGMT_PORT;
    def_conf->param.architecture = VTSS_EPS_MGMT_ARCHITECTURE_1F1;
    def_conf->param.w_flow = 1;
    def_conf->param.p_flow = 1;

    def_conf->config.directional = VTSS_EPS_MGMT_UNIDIRECTIONAL;
    def_conf->config.aps = FALSE;
    def_conf->config.revertive = FALSE;
    def_conf->config.restore_timer = 60*5;
    def_conf->config.hold_off_timer = VTSS_EPS_MGMT_HOFF_OFF;

    def_conf->command = VTSS_EPS_MGMT_COMMAND_NONE;
}



/****************************************************************************/
/*  EPS module interface                                                    */
/****************************************************************************/

u32 vtss_eps_rx_aps_info_set(const u32                     instance,
                             const vtss_eps_flow_type_t    flow,
                             const u8                      aps[VTSS_EPS_APS_DATA_LENGTH])
{
    vtss_eps_crit_lock();

    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)                         {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if (!instance_data[instance].created)                              {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}
    if (!instance_data[instance].configured)                           {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CONFIGURED);}
    if (((aps[0]&0xF0) == 0x30) || ((aps[0]&0xF0) == 0x60) || ((aps[0]&0xF0) == 0x80) || ((aps[0]&0xF0) == 0xA0) || ((aps[0]&0xF0) == 0xC0))
                                                                       {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_APS);}
    run_sync_aps(instance, flow, aps);

    if (!instance_data[instance].state.dFop_NoAps &&
        (flow == VTSS_EPS_FLOW_PROTECTING) &&
        memcmp(instance_data[instance].rx_aps, aps, VTSS_EPS_APS_DATA_LENGTH))
    {
        memcpy(instance_data[instance].rx_aps, aps, VTSS_EPS_APS_DATA_LENGTH);
        instance_data[instance].event_flags |= EVENT_IN_APS;
    }

    vtss_eps_run();

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}



u32 vtss_eps_sf_sd_state_set(const u32                     instance,
                             const vtss_eps_flow_type_t    flow,
                             const BOOL                    sf_state,
                             const BOOL                    sd_state)
{
    vtss_eps_crit_lock();

    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)                       {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if (!instance_data[instance].created)                            {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}
    if (!instance_data[instance].configured)                         {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CONFIGURED);}

    if (flow == VTSS_EPS_FLOW_WORKING)
    {
        instance_data[instance].event_flags |= EVENT_IN_SF_W;
        instance_data[instance].sf_w = sf_state;
        instance_data[instance].sd_w = sd_state;
    }
    if (flow == VTSS_EPS_FLOW_PROTECTING)
    {
        instance_data[instance].event_flags |= EVENT_IN_SF_P;
        instance_data[instance].sf_p = sf_state;
        instance_data[instance].sd_p = sd_state;
    }

    vtss_eps_run();

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}

u32 vtss_eps_signal_in(const u32   instance)
{
    vtss_eps_crit_lock();

    if (instance >= VTSS_EPS_MGMT_CREATED_MAX)               {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_INVALID_PARAMETER);}
    if (!instance_data[instance].created)                    {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CREATED);}
    if (!instance_data[instance].configured)                 {vtss_eps_crit_unlock(); return(VTSS_EPS_RC_NOT_CONFIGURED);}

    instance_data[instance].event_flags |= EVENT_OUT_APS;

    vtss_eps_run();

    vtss_eps_crit_unlock();

    return(VTSS_EPS_RC_OK);
}




/****************************************************************************/
/*  EPS platform interface                                                  */
/****************************************************************************/

void vtss_eps_timer_thread(BOOL  *const stop)
{
    u32  i;
    BOOL run;

    run = FALSE;
    *stop = TRUE;

    vtss_eps_crit_lock();

    for (i=0; i<VTSS_EPS_MGMT_CREATED_MAX; ++i)
    {
        if (instance_data[i].wtr_timer)
        {
            instance_data[i].wtr_timer--;
            if (!instance_data[i].wtr_timer)
            {
                instance_data[i].event_flags |= EVENT_IN_WTR;
                run = TRUE;
            }
            else
                *stop = FALSE;
        }
        if (instance_data[i].hoff_w_timer)
        {
            instance_data[i].hoff_w_timer--;
            if (!instance_data[i].hoff_w_timer)
            {
                instance_data[i].event_flags |= EVENT_IN_HOFF_W;
                run = TRUE;
            }
            else
                *stop = FALSE;
        }
        if (instance_data[i].hoff_p_timer)
        {
            instance_data[i].hoff_p_timer--;
            if (!instance_data[i].hoff_p_timer)
            {
                instance_data[i].event_flags |= EVENT_IN_HOFF_P;
                run = TRUE;
            }
            else
                *stop = FALSE;
        }
        if (instance_data[i].dFop_nr_timer)
        {
            instance_data[i].dFop_nr_timer--;
            if (!instance_data[i].dFop_nr_timer)
            {
                instance_data[i].event_flags |= EVENT_IN_DFOP_NR;
                run = TRUE;
            }
            else
                *stop = FALSE;
        }
    }

    if (run)    vtss_eps_run();

    vtss_eps_crit_unlock();
}



void vtss_eps_run_thread(void)
{
    u32                     i, w_flow_c, p_flow_c;
    u8                      aps_c[VTSS_EPS_APS_DATA_LENGTH];
    vtss_eps_mgmt_domain_t  domain_c;
    vtss_eps_selector_t     selector_c;

    vtss_eps_crit_lock();

    for (i=0; i<VTSS_EPS_MGMT_CREATED_MAX; ++i)
    {
        if (instance_data[i].event_flags & EVENT_IN_MASK)
        {/* New input */
            if (instance_data[i].event_flags & EVENT_IN_CREATE)         run_create(i);
            if (instance_data[i].event_flags & EVENT_IN_DELETE)         run_delete(i);
            if (instance_data[i].event_flags & EVENT_IN_CONFIG)         run_config(i);
            if (instance_data[i].event_flags & EVENT_IN_SF_W)           run_sf(i);
            if (instance_data[i].event_flags & EVENT_IN_SF_P)           run_sf(i);
            if (instance_data[i].event_flags & EVENT_IN_APS)            run_async_aps(i);
            if (instance_data[i].event_flags & EVENT_IN_WTR)            run_wtr(i);
            if (instance_data[i].event_flags & EVENT_IN_HOFF_W)         run_hold(i);
            if (instance_data[i].event_flags & EVENT_IN_HOFF_P)         run_hold(i);
            if (instance_data[i].event_flags & EVENT_IN_COMMAND)        run_command(i);
            if (instance_data[i].event_flags & EVENT_IN_DFOP_NR)        run_dFop_nr(i);
        }

        if (instance_data[i].event_flags & EVENT_OUT_MASK)
        {/* New output */
            if (instance_data[i].event_flags & EVENT_OUT_APS)
            {   /* Transmit APS info to flow */
                instance_data[i].event_flags &= ~EVENT_OUT_APS;
    
                /* Copy data for call-out */
                domain_c = instance_data[i].param.domain;
                memcpy(aps_c, instance_data[i].tx_aps, VTSS_EPS_APS_DATA_LENGTH);
    
                vtss_eps_crit_unlock();
                vtss_eps_tx_aps_info_set(i, domain_c, aps_c);
                vtss_eps_crit_lock();
            }
            if (instance_data[i].event_flags & EVENT_OUT_SIGNAL)
            {   /* Send signal */
                instance_data[i].event_flags &= ~EVENT_OUT_SIGNAL;
    
                /* Copy data for call-out */
                domain_c = instance_data[i].param.domain;
    
                vtss_eps_crit_unlock();
                vtss_eps_signal_out(i, domain_c);
                vtss_eps_crit_lock();
            }
            if (instance_data[i].event_flags & EVENT_OUT_PORT_PROT)
            {   /* Send out port protection state */
                instance_data[i].event_flags &= ~EVENT_OUT_PORT_PROT;
    
                /* Copy data for call-out */
                w_flow_c = instance_data[i].param.w_flow;
                p_flow_c = instance_data[i].param.p_flow;
                selector_c = calc_selector_state(instance_data[i].internal_state);
    
                vtss_eps_crit_unlock();
                vtss_eps_port_protection_set(w_flow_c,  p_flow_c, (selector_c == VTSS_EPS_SELECTOR_PROTECTION));
                vtss_eps_crit_lock();
            }
        }
    }

    vtss_eps_crit_unlock();
}


u32 vtss_eps_init(const u32  timer_resolution)
{
    u32 i;

    if ((timer_resolution == 0) || (timer_resolution > 100))     return(VTSS_EPS_RC_INVALID_PARAMETER);

    timer_res = timer_resolution;

    /* Initialize all instance data */
    for (i=0; i<VTSS_EPS_MGMT_CREATED_MAX; ++i)
        instance_data_clear(&instance_data[i]);

    /* initialize state transition tables */
    init_nxt_1p1_unidir_rev();      /* 1+1 */
    init_nxt_1p1_unidir_nonrev();
    init_nxt_1p1_bidir_rev();
    init_nxt_1p1_bidir_nonrev();

    init_nxt_1f1_bidir_rev();       /* 1:1 */
    init_nxt_1f1_bidir_nonrev();

    return(VTSS_EPS_RC_OK);
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
