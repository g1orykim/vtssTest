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

#include "main.h"
#include "cli.h"
#include "cli_api.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif

#include "mep_cli.h"
#include "vtss_module_id.h"
#include "mep_api.h"
#include "cli_trace_def.h"
#if defined(VTSS_SW_OPTION_UP_MEP)
#include "pvlan_api.h"
#include "vlan_api.h"
#include "mac_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_MEP

#define FLAG_PARAM_INSTANCE          0x1
#define FLAG_PARAM_MEP               0x2
#define FLAG_PARAM_ENABLE            0x4
#define FLAG_PARAM_LEVEL             0x8
#define FLAG_PARAM_DOMAIN            0x10
#define FLAG_PARAM_MODE              0x20
#define FLAG_PARAM_DIRECTION         0x40
#define FLAG_PARAM_CAST              0x80
#define FLAG_PARAM_APS_TYPE          0x100
#define FLAG_PARAM_FORMAT            0x200
#define FLAG_PARAM_FLOW              0x400
#define FLAG_PARAM_CLIENT_FLOWS      0x800
#define FLAG_PARAM_CLIENT_FLOW_COUNT 0x1000
#define FLAG_PARAM_VID               0x2000
#define FLAG_PARAM_TTL               0x4000
#define FLAG_PARAM_MEG_ID            0x8000
#define FLAG_PARAM_TO_SEND           0x10000
#define FLAG_PARAM_SIZE              0x20000
#define FLAG_PARAM_INTERVAL          0x40000
#define FLAG_PARAM_RATE              0x80000
#define FLAG_PARAM_PATTERN           0x100000
#define FLAG_PARAM_PRIO              0x200000
#define FLAG_PARAM_DEI               0x400000
#define FLAG_PARAM_PERIOD            0x800000
#define FLAG_PARAM_ENDED             0x1000000
#define FLAG_PARAM_FLR_INTERVAL      0x2000000
#define FLAG_PARAM_OCTET             0x4000000
#define FLAG_PARAM_WAY               0x8000000
#define FLAG_PARAM_TXWAY             0x10000000
#define FLAG_PARAM_CALCWAY           0x20000000
#define FLAG_PARAM_COUNT             0x40000000
#define FLAG_PARAM_DM_GAP            0x80000000
#define FLAG_PARAM_TUNIT             0x100000000LL
#define FLAG_PARAM_ACT               0x200000000LL
#define FLAG_PARAM_DM2FORDM1         0x400000000LL
#define FLAG_PARAM_PROTECTION        0x800000000LL
#define FLAG_PARAM_PORT              0x1000000000LL
#define FLAG_PARAM_MAC_ADDR          0x2000000000LL
#define FLAG_PARAM_SEQUENCE          0x4000000000LL
#define FLAG_PARAM_VOE               0x8000000000LL
#define FLAG_PARAM_MEG_NAME          0x10000000000LL

typedef struct
{
    u64                        flag;
    u32                        mep;
    u32                        instance;
    BOOL                       enable;
    BOOL                       voe;
    u32                        level;
    vtss_mep_mgmt_domain_t     domain;
    vtss_mep_mgmt_mode_t       mode;
    vtss_mep_mgmt_direction_t  direction;
    vtss_mep_mgmt_cast_t	   cast;
    vtss_mep_mgmt_aps_type_t   aps_type;
    vtss_mep_mgmt_format_t     format;
    u32                        flow;
    u32                        client_flows[VTSS_MEP_CLIENT_FLOWS_MAX];
    u32                        client_flow_count;
    u32                        vid;
    u8                         ttl;
    char                       meg_id[VTSS_MEP_MEG_CODE_LENGTH];
    char                       meg_name[VTSS_MEP_MEG_CODE_LENGTH];
    u32                        to_send;
    u32                        size;
    u32                        interval;
    u32                        rate;
    vtss_mep_mgmt_pattern_t    pattern;

    u32                        prio;
    BOOL                       dei;
    BOOL                       sequence;
    vtss_mep_mgmt_period_t     period;
                             
    vtss_mep_mgmt_ended_t      ended;
    u32                        flr_interval;
    u32                        octet;

    BOOL                       proprietary;
    vtss_mep_mgmt_dm_calcway_t calcway;         
    u32                        count;
    u32                        dm_gap; 
    vtss_mep_mgmt_dm_tunit_t   tunit;
    vtss_mep_mgmt_dm_act_t     act; 
    BOOL                       dm2fordm1;
    BOOL                       protection;
    vtss_uport_no_t            port;
    uchar                      mac_addr[6];
} mep_cli_req_t;

void mep_cli_req_init(void)
{
    /* register the size required for mep req. structure */
    cli_req_size_register(sizeof(mep_cli_req_t));
}

static void mep_print_error(vtss_rc rc)
{
    CPRINTF("Error: %s\n", error_txt(rc));
}

static void cli_cmd_id_mep_test_config(cli_req_t *req)
{
    u32                       i;
    vtss_mep_mgmt_conf_t      config;
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_mep_mgmt_cc_conf_t   cc_config;
    mep_cli_req_t             *mep_req = req->module_req;
    vtss_rc                   rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_COUNT | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_COUNT | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'count' and 'enable|disable' required\n");
            return;
        }

        if (!mep_req->enable)
        {
            config.enable = FALSE;
            for (i=0; i<mep_req->count; ++i)
                if ((rc = mep_mgmt_conf_set(mep_req->instance+i, &config)) != VTSS_RC_OK)
                    mep_print_error(rc);
            return;
        }

        if ((mep_req->flag & (FLAG_PARAM_VID | FLAG_PARAM_PORT | FLAG_PARAM_MAC_ADDR)) != (FLAG_PARAM_VID | FLAG_PARAM_PORT | FLAG_PARAM_MAC_ADDR)) {
            CPRINTF("'\n'vid' and 'port' and 'mac_addr' required\n");
            return; 
        }

        for (i=0; i<mep_req->count; ++i)
        {
            if ((rc = mep_mgmt_conf_get(mep_req->instance, mac, &eps_count, eps_inst, &config)) == MEP_RC_INVALID_PARAMETER) {
                mep_print_error(rc);
                continue;
            }

            config.vid = mep_req->vid+i;
            config.enable = TRUE;
            config.mep = 1;
            config.level = 0;
            config.domain = VTSS_MEP_MGMT_PORT;
            config.flow = mep_req->port-1;
            config.mode = VTSS_MEP_MGMT_MEP;
            config.direction = VTSS_MEP_MGMT_DOWN;
            config.port = mep_req->port-1;
            config.format = VTSS_MEP_MGMT_ITU_ICC;
            strncpy(config.name, "VITESS", VTSS_MEP_MEG_CODE_LENGTH);
            strncpy(config.meg, "meg000", VTSS_MEP_MEG_CODE_LENGTH);
    
            config.peer_mep[0] = 1;
            config.peer_count = 1;
            memcpy(config.peer_mac[0], mep_req->mac_addr, VTSS_MEP_MAC_LENGTH);

            if ((rc = mep_mgmt_conf_set(mep_req->instance+i, &config)) != VTSS_RC_OK)
                mep_print_error(rc);

            cc_config.enable = TRUE;
            cc_config.prio = 0;
            cc_config.period = VTSS_MEP_MGMT_PERIOD_300S;
       
            if ((rc = mep_mgmt_cc_conf_set(mep_req->instance+i, &cc_config)) != VTSS_RC_OK)
                mep_print_error(rc);
        }
    }
}

static void cli_cmd_id_mep_config(cli_req_t *req)
{
    u32                    i, j;
    vtss_mep_mgmt_conf_t   config;
    u32                    eps_count;
    u16                    eps_inst[MEP_EPS_MAX];
    mep_cli_req_t          *mep_req = req->module_req;
    u8                     mac[VTSS_MEP_MAC_LENGTH];
    char                   string[100];
    vtss_rc                rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_conf_get(mep_req->instance, mac, &eps_count, eps_inst, &config)) == MEP_RC_INVALID_PARAMETER)
            mep_print_error(rc);
        else
        {
            config.enable = mep_req->enable;
            if (mep_req->flag & FLAG_PARAM_VID)         config.vid = mep_req->vid;
            if (mep_req->flag & FLAG_PARAM_MEP)         config.mep = mep_req->mep;
            if (mep_req->flag & FLAG_PARAM_LEVEL)       config.level = mep_req->level;
            if (mep_req->flag & FLAG_PARAM_DOMAIN)      config.domain = mep_req->domain;
            if (mep_req->flag & FLAG_PARAM_FLOW)        config.flow = mep_req->flow;
            if (mep_req->flag & FLAG_PARAM_MODE)        config.mode = mep_req->mode;
            if (mep_req->flag & FLAG_PARAM_DIRECTION)   config.direction = mep_req->direction;
            if (mep_req->flag & FLAG_PARAM_PORT)        config.port = mep_req->port-1;
            if (mep_req->flag & FLAG_PARAM_FORMAT)      config.format = mep_req->format;
            if (mep_req->flag & FLAG_PARAM_MEG_ID)      strncpy(config.meg, mep_req->meg_id, VTSS_MEP_MEG_CODE_LENGTH-1);
            if (mep_req->flag & FLAG_PARAM_MEG_NAME)    strncpy(config.name, mep_req->meg_name, VTSS_MEP_MEG_CODE_LENGTH-1);
            if (mep_req->flag & FLAG_PARAM_VOE)         config.voe = TRUE;
            else                                        config.voe = FALSE;
    
            if ((rc = mep_mgmt_conf_set(mep_req->instance, &config)) != VTSS_RC_OK)
                mep_print_error(rc);
        }
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%9s", "Mode");
        CPRINTF("%8s", "Voe");
        CPRINTF("%14s", "Direction");
        CPRINTF("%9s", "Port");
        CPRINTF("%8s", "Dom");
        CPRINTF("%10s", "Level");
        CPRINTF("%13s", "Format");
        CPRINTF("%18s", "Name");
        CPRINTF("%18s", "Meg id");
        CPRINTF("%8s", "Mep");
        CPRINTF("%8s", "Vid");
        CPRINTF("%9s", "Flow");
        CPRINTF("%9s", "Eps");
        CPRINTF("%22s", "MAC");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_conf_get(i, mac, &eps_count, eps_inst, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    switch (config.mode)
                    {
                        case VTSS_MEP_MGMT_MEP:       CPRINTF("%9s", "Mep");     break;
                        case VTSS_MEP_MGMT_MIP:       CPRINTF("%9s", "Mip");     break;
                    }
                    if (config.voe)     CPRINTF("%8s", "Voe");
                    else                CPRINTF("%8s", "        ");
                    switch (config.direction)
                    {
                        case VTSS_MEP_MGMT_DOWN:   CPRINTF("%14s", "Ingress");    break;
                        case VTSS_MEP_MGMT_UP:    CPRINTF("%14s", "Egress");      break;
                    }
                    CPRINTF("%9u", config.port+1);
                    switch (config.domain)
                    {
                        case VTSS_MEP_MGMT_PORT:       CPRINTF("%8s", "Port");     break;
//                        case VTSS_MEP_MGMT_ESP:        CPRINTF("%8s", "Eps");      break;
                        case VTSS_MEP_MGMT_EVC:        CPRINTF("%8s", "Evc");      break;
                        case VTSS_MEP_MGMT_VLAN:       CPRINTF("%8s", "Vlan");     break;
//                        case VTSS_MEP_MGMT_MPLS:       CPRINTF("%8s", "Mpls");     break;
                    }
                    CPRINTF("%10u", config.level);
                    if (config.mode == VTSS_MEP_MGMT_MEP)
                    {
                        switch (config.format)
                        {
                            case VTSS_MEP_MGMT_ITU_ICC:    CPRINTF("%13s", "ITU ICC");     break;
                            case VTSS_MEP_MGMT_IEEE_STR:   CPRINTF("%13s", "IEEE String");   break;
                            case VTSS_MEP_MGMT_ITU_CC_ICC: CPRINTF("%13s", "ITU CC ICC");     break;
                        }
                        CPRINTF("%18s", config.name);
                        CPRINTF("%18s", config.meg);
                        CPRINTF("%8u", config.mep);
                    }
                    else
                    {
                        CPRINTF("             ");
                        CPRINTF("                  ");
                        CPRINTF("                  ");
                        CPRINTF("        ");
                    }
                    CPRINTF("%8u", config.vid);
                    CPRINTF("%9u", config.flow+1);
                    if (config.mode == VTSS_MEP_MGMT_MEP)
                    {
                        if (eps_count)
                        {
                            string[0] = '\0';
                            for (j=0; j<3 && j<eps_count; ++j)   sprintf(string, "%s%u-", string, eps_inst[j]+1);
                            string[strlen(string)-1] = '\0';
                            CPRINTF("%9s", string);
                        }
                        else                    CPRINTF("%9u", 0);
                    }
                    else
                        CPRINTF("         ");
                    CPRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_peer_config(cli_req_t *req)
{
    u32                    i, j, idx;
    vtss_mep_mgmt_conf_t   config;
    u32                    eps_count;
    u16                    eps_inst[MEP_EPS_MAX];
    u16                    peer_mep[VTSS_MEP_PEER_MAX];
    u8                     peer_mac[VTSS_MEP_PEER_MAX][VTSS_MEP_MAC_LENGTH];
    u8                     mac[VTSS_MEP_MAC_LENGTH];
    mep_cli_req_t          *mep_req = req->module_req;
    vtss_rc                rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE | FLAG_PARAM_MEP)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE | FLAG_PARAM_MEP)) {
            CPRINTF("\n'instance' and 'mep' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_conf_get(mep_req->instance, mac, &eps_count, eps_inst, &config)) == MEP_RC_INVALID_PARAMETER) {
            mep_print_error(rc);
        } else {
            if (config.enable)
            {
                if (mep_req->enable)
                {   /* Add or modify peer MEP */
                    for (i=0; i<config.peer_count; ++i)     if (config.peer_mep[i] == mep_req->mep)     break;
                    config.peer_mep[i] = mep_req->mep;
                    memcpy(config.peer_mac[i], mep_req->mac_addr, VTSS_MEP_MAC_LENGTH);
                    if (i == config.peer_count)     config.peer_count++;
                    if ((rc = mep_mgmt_conf_set(mep_req->instance, &config)) != VTSS_RC_OK) {
                        mep_print_error(rc);
                    }
                }
                else
                {   /* Remove peer MEP */
                    memset(peer_mep, 0, VTSS_MEP_PEER_MAX*sizeof(u16));
                    memset(peer_mac, 0, VTSS_MEP_PEER_MAX*VTSS_MEP_MAC_LENGTH);
                    for (i=0, idx=0; i<config.peer_count; ++i)
                        if (config.peer_mep[i] != mep_req->mep)
                        {
                            peer_mep[idx] = config.peer_mep[i];
                            memcpy(peer_mac[idx], config.peer_mac[i], VTSS_MEP_MAC_LENGTH);
                            idx++;
                        }
                    memcpy(config.peer_mep, peer_mep, idx*sizeof(u16));
                    memcpy(config.peer_mac, peer_mac, idx*VTSS_MEP_MAC_LENGTH);
                    config.peer_count = idx;
                    if ((rc = mep_mgmt_conf_set(mep_req->instance, &config)) != VTSS_RC_OK) {
                        mep_print_error(rc);
                    }
                }
            }
            else    CPRINTF("This MEP is not enabled\n");
        }
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP Peer MEP Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%12s", "Peer id");
        CPRINTF("%22s", "Peer MAC");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_conf_get(i, mac, &eps_count, eps_inst, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable && (config.peer_count != 0))
                {
                    CPRINTF("%9u", i+1);
                    for (j=0; j<config.peer_count; ++j)
                    {
                        if (j != 0)     CPRINTF("         ");
                        CPRINTF("%12u", config.peer_mep[j]);
                        CPRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", config.peer_mac[j][0], config.peer_mac[j][1], config.peer_mac[j][2],
                                                                      config.peer_mac[j][3], config.peer_mac[j][4], config.peer_mac[j][5]);
                        CPRINTF("\n");
                    }
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_cc_config(cli_req_t *req)
{
    u32                       i;
    vtss_mep_mgmt_cc_conf_t   config;
    mep_cli_req_t             *mep_req = req->module_req;
    vtss_rc                   rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_cc_conf_get(mep_req->instance, &config)) == MEP_RC_INVALID_PARAMETER) {
            mep_print_error(rc);
            return; 
        }

        config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PRIO)    config.prio = mep_req->prio;
        if (mep_req->flag & FLAG_PARAM_PERIOD)  config.period = mep_req->period;

        if ((rc = mep_mgmt_cc_conf_set(mep_req->instance, &config)) != VTSS_RC_OK)
            mep_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP CC Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%9s", "Prio");
        CPRINTF("%11s", "Period");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_cc_conf_get(i, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%9u", config.prio);
                    switch (config.period)
                    {
                        case VTSS_MEP_MGMT_PERIOD_300S:       CPRINTF("%11s", "300s");   break;
                        case VTSS_MEP_MGMT_PERIOD_100S:       CPRINTF("%11s", "100s");   break;
                        case VTSS_MEP_MGMT_PERIOD_10S:        CPRINTF("%11s", "10s");    break;
                        case VTSS_MEP_MGMT_PERIOD_1S:         CPRINTF("%11s", "1s");     break;
                        case VTSS_MEP_MGMT_PERIOD_6M:         CPRINTF("%11s", "6m");     break;
                        case VTSS_MEP_MGMT_PERIOD_1M:         CPRINTF("%11s", "1m");     break;
                        case VTSS_MEP_MGMT_PERIOD_6H:         CPRINTF("%11s", "6h");     break;
                        default:                              CPRINTF("%11s", "Unknown");
                    }
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_dm_config(cli_req_t *req)
{
    u32                       i;
    vtss_mep_mgmt_dm_conf_t   config;
    mep_cli_req_t             *mep_req = req->module_req;
    vtss_rc                   rc;
    
    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }


        if ((rc = mep_mgmt_dm_conf_get(mep_req->instance, &config)) == MEP_RC_INVALID_PARAMETER) {
            mep_print_error(rc);
            return; 
        }

        config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PRIO)        config.prio = mep_req->prio;
        if (mep_req->flag & FLAG_PARAM_CAST)        config.cast = mep_req->cast;
        if (mep_req->flag & FLAG_PARAM_MEP)         config.mep = mep_req->mep;
        if (mep_req->flag & FLAG_PARAM_WAY)         config.ended = mep_req->ended;
        if (mep_req->flag & FLAG_PARAM_DM_GAP)      config.interval = mep_req->dm_gap;
        if (mep_req->flag & FLAG_PARAM_TXWAY)       config.proprietary = mep_req->proprietary;
        if (mep_req->flag & FLAG_PARAM_CALCWAY)     config.calcway = mep_req->calcway;
        if (mep_req->flag & FLAG_PARAM_COUNT)       config.lastn = mep_req->count;
        if (mep_req->flag & FLAG_PARAM_TUNIT)       config.tunit = mep_req->tunit;
        if (mep_req->flag & FLAG_PARAM_ACT)         config.overflow_act = mep_req->act;
        if (mep_req->flag & FLAG_PARAM_DM2FORDM1)   config.syncronized = mep_req->dm2fordm1;

        if ((rc = mep_mgmt_dm_conf_set(mep_req->instance, &config)) != VTSS_RC_OK)
            mep_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP DM Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%9s", "Prio");
        CPRINTF("%9s", "Cast");
        CPRINTF("%8s", "Mep");
        CPRINTF("%10s", "Way");
        CPRINTF("%10s", "Prtcl");
        CPRINTF("%14s", "2-way Calc");
        CPRINTF("%8s", "Gap");
        CPRINTF("%8s", "Count");
        CPRINTF("%7s", "Unit");
        CPRINTF("%10s", "D2forD1");
        CPRINTF("%11s", "CNT OV ACT");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_dm_conf_get(i, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%9u", config.prio);
                    switch (config.cast)
                    {
                        case VTSS_MEP_MGMT_UNICAST:     CPRINTF("%9s", "Uni");     break;
                        case VTSS_MEP_MGMT_MULTICAST:   CPRINTF("%9s", "Multi");   break;
                    }
                    CPRINTF("%8u", config.mep);
                    switch (config.ended)
                    {
                        case VTSS_MEP_MGMT_DUAL_ENDED:     CPRINTF("%10s", "One-way");   break;
                        case VTSS_MEP_MGMT_SINGEL_ENDED:   CPRINTF("%10s", "Two-way");   break;
                        default:                           CPRINTF("%10s", "Unknown");
                    }
                    if (config.proprietary)         CPRINTF("%10s", "Prop");
                    else                            CPRINTF("%10s", "Std");
                    switch (config.calcway)
                    {
                        case VTSS_MEP_MGMT_RDTRP:      CPRINTF("%14s", "Rdtrp");   break;
                        case VTSS_MEP_MGMT_FLOW:        CPRINTF("%14s", "Flow");   break;
                        default:                        CPRINTF("%14s", "Unknown");
                    }
                    CPRINTF("%8u", config.interval);
                    CPRINTF("%8u", config.lastn);
                    switch (config.tunit)
                    {
                        case VTSS_MEP_MGMT_US:          CPRINTF("%7s", "us");   break;
                        case VTSS_MEP_MGMT_NS:          CPRINTF("%7s", "ns");   break;
                        default:                        CPRINTF("%7s", "Unknown");
                    }
                    if (config.syncronized == TRUE)
                    {
                        CPRINTF("%10s", "Enabled");       
                    } else {
                        CPRINTF("%10s", "Disabled");       
                    }            
                    switch (config.overflow_act)
                    {
                        case VTSS_MEP_MGMT_DISABLE:     CPRINTF("%11s", "keep");    break;
                        case VTSS_MEP_MGMT_CONTINUE:    CPRINTF("%11s", "reset");    break;
                        default:                        CPRINTF("%11s", "Unknown");
                    }
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }    
}

static void cli_cmd_id_mep_lm_config(cli_req_t *req)
{
    u32                       i;
    vtss_mep_mgmt_lm_conf_t   config;
    mep_cli_req_t             *mep_req = req->module_req;
    vtss_rc                   rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_lm_conf_get(mep_req->instance, &config)) == MEP_RC_INVALID_PARAMETER) {
            mep_print_error(rc);
            return; 
        }

        config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PRIO)            config.prio = mep_req->prio;
        if (mep_req->flag & FLAG_PARAM_CAST)            config.cast = mep_req->cast;
        if (mep_req->flag & FLAG_PARAM_FLR_INTERVAL)    config.flr_interval = mep_req->flr_interval;
        if (mep_req->flag & FLAG_PARAM_ENDED)           config.ended = mep_req->ended;
        if (mep_req->flag & FLAG_PARAM_PERIOD)          config.period = mep_req->period;

        if ((rc = mep_mgmt_lm_conf_set(mep_req->instance, &config)) != VTSS_RC_OK)
            mep_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP LM Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%9s", "Prio");
        CPRINTF("%9s", "Cast");
        CPRINTF("%11s", "Ended");
        CPRINTF("%11s", "Period");
        CPRINTF("%8s", "Flr");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_lm_conf_get(i, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%9u", config.prio);
                    switch (config.cast)
                    {
                        case VTSS_MEP_MGMT_UNICAST:       CPRINTF("%9s", "Uni");     break;
                        case VTSS_MEP_MGMT_MULTICAST:     CPRINTF("%9s", "Multi");   break;
                    }
                    switch (config.ended)
                    {
                        case VTSS_MEP_MGMT_SINGEL_ENDED:      CPRINTF("%10s", "Single");   break;
                        case VTSS_MEP_MGMT_DUAL_ENDED:        CPRINTF("%10s", "Dual");   break;
                        default:                              CPRINTF("%10s", "Unknown");
                    }
                    switch (config.period)
                    {
                        case VTSS_MEP_MGMT_PERIOD_300S:       CPRINTF("%11s", "300s");   break;
                        case VTSS_MEP_MGMT_PERIOD_100S:       CPRINTF("%11s", "100s");   break;
                        case VTSS_MEP_MGMT_PERIOD_10S:        CPRINTF("%11s", "10s");    break;
                        case VTSS_MEP_MGMT_PERIOD_1S:         CPRINTF("%11s", "1s");     break;
                        case VTSS_MEP_MGMT_PERIOD_6M:         CPRINTF("%11s", "6m");     break;
                        case VTSS_MEP_MGMT_PERIOD_1M:         CPRINTF("%11s", "1m");     break;
                        case VTSS_MEP_MGMT_PERIOD_6H:         CPRINTF("%11s", "6h");     break;
                        default:                              CPRINTF("%11s", "Unknown");
                    }
                    CPRINTF("%8u", config.flr_interval);
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_aps_config(cli_req_t *req)
{
    u32                        i;
    vtss_mep_mgmt_aps_conf_t   config;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_aps_conf_get(mep_req->instance, &config)) == MEP_RC_INVALID_PARAMETER) {
            mep_print_error(rc);
            return; 
        }

        config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PRIO)        config.prio = mep_req->prio;
        if (mep_req->flag & FLAG_PARAM_CAST)        config.cast = mep_req->cast;
        if (mep_req->flag & FLAG_PARAM_APS_TYPE)    config.type = mep_req->aps_type;
        if (mep_req->flag & FLAG_PARAM_OCTET)       config.raps_octet = mep_req->octet;

        if ((rc = mep_mgmt_aps_conf_set(mep_req->instance, &config)) != VTSS_RC_OK)
            mep_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP APS Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%9s", "Prio");
        CPRINTF("%9s", "Cast");
        CPRINTF("%9s", "Type");
        CPRINTF("%9s", "Octet");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_aps_conf_get(i, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%9u", config.prio);
                    switch (config.cast)
                    {
                        case VTSS_MEP_MGMT_UNICAST:       CPRINTF("%9s", "Uni");     break;
                        case VTSS_MEP_MGMT_MULTICAST:     CPRINTF("%9s", "Multi");   break;
                    }
                    switch (config.type)
                    {
                        case VTSS_MEP_MGMT_INV_APS:   CPRINTF("%9s", "none");   break;
                        case VTSS_MEP_MGMT_L_APS:     CPRINTF("%9s", "laps");   break;
                        case VTSS_MEP_MGMT_R_APS:     CPRINTF("%9s", "raps");   break;
                    }
                    CPRINTF("%9X", config.raps_octet);
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_client_config (cli_req_t * req)
{
    vtss_rc                      rc;
    u32                          eps_count;
    u16                          eps_inst[MEP_EPS_MAX];
    u8                           mac[VTSS_MEP_MAC_LENGTH];
    vtss_mep_mgmt_conf_t         config;
    vtss_mep_mgmt_client_conf_t  client_config;
    mep_cli_req_t                *mep_req = req->module_req;
    u32                          j, i;

    memset(&config,0,sizeof(config));

    if (req->set) {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE)) != (FLAG_PARAM_INSTANCE)) {
            CPRINTF("\n'instance' required\n");
            return; 
        }

        if ((rc = mep_mgmt_client_conf_get(mep_req->instance, &client_config)) != VTSS_RC_OK) {
            mep_print_error(rc);
            return; 
        }

        if (mep_req->flag & FLAG_PARAM_DOMAIN)   client_config.domain = mep_req->domain;
        if (mep_req->flag & FLAG_PARAM_LEVEL) {
            for (i=0; i<VTSS_MEP_CLIENT_FLOWS_MAX; i++)
                client_config.level[i] = mep_req->level;
        }
        if (mep_req->flag & FLAG_PARAM_CLIENT_FLOWS) {
            for (i=0; i<mep_req->client_flow_count; ++i)
                client_config.flows[i] = mep_req->client_flows[i];
            client_config.flow_count = mep_req->client_flow_count;
        }

        if ((rc = mep_mgmt_client_conf_set(mep_req->instance, &client_config)) != VTSS_RC_OK) {
            mep_print_error(rc);
        }
    } else {
        CPRINTF("\n");
        CPRINTF("MEP Client Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%11s", "Domain");
        CPRINTF("%10s", "Level");
        CPRINTF("%25s", "Flows");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_conf_get(i, mac, &eps_count, eps_inst, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            if ((rc = mep_mgmt_client_conf_get(i, &client_config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    switch (client_config.domain)
                    {
                        case VTSS_MEP_MGMT_PORT:       CPRINTF("%11s", "Port");  break;
//                        case VTSS_MEP_MGMT_ESP:        CPRINTF("%11s", "ESP");   break;
                        case VTSS_MEP_MGMT_EVC:        CPRINTF("%11s", "Evc");   break;
                        case VTSS_MEP_MGMT_VLAN:       CPRINTF("%11s", "Vlan");   break;
//                        case VTSS_MEP_MGMT_MPLS:       CPRINTF("%11s", "Mpls");  break;
                    }
                    CPRINTF("%10d", client_config.level[0]);
                    CPRINTF("     ");
                    for (j=0; j<(10-client_config.flow_count); ++j)
                        CPRINTF("  ");
                    for (j=0; j<client_config.flow_count; ++j)
                        CPRINTF("%u-", client_config.flows[j]+1);
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_ais_config (cli_req_t * req)
{
    vtss_rc                      rc;
    vtss_mep_mgmt_ais_conf_t     ais_config;
    vtss_mep_mgmt_client_conf_t  client_config;
    mep_cli_req_t                *mep_req = req->module_req;
    u32                          i;

    memset(&ais_config,0,sizeof(ais_config));
    memset(&client_config,0,sizeof(ais_config));

    if (req->set) {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }


        if ((rc = mep_mgmt_ais_conf_get(mep_req->instance, &ais_config)) != VTSS_RC_OK) {
            mep_print_error(rc);
            return;
        }


        if ((rc = mep_mgmt_client_conf_get(mep_req->instance, &client_config)) != VTSS_RC_OK) {
            mep_print_error(rc);
            return;
        }


        ais_config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PERIOD)      ais_config.period        = mep_req->period;
        if (mep_req->flag & FLAG_PARAM_PROTECTION)  ais_config.protection    = mep_req->protection;
        if (mep_req->flag & FLAG_PARAM_PRIO) {
            for (i=0; i<VTSS_MEP_CLIENT_FLOWS_MAX; i++)
                client_config.ais_prio[i] = mep_req->prio;
        }


        if ((rc = mep_mgmt_ais_conf_set(mep_req->instance, &ais_config)) != VTSS_RC_OK)
            mep_print_error(rc);
        if ((rc = mep_mgmt_client_conf_set(mep_req->instance, &client_config)) != VTSS_RC_OK)
            mep_print_error(rc);
    } else {
        CPRINTF("\n");
        CPRINTF("MEP AIS Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%9s", "Prio");
        CPRINTF("%11s", "Period");
        CPRINTF("%15s","Protection");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_ais_conf_get(i, &ais_config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            if ((rc = mep_mgmt_client_conf_get(i, &client_config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (ais_config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%9d", client_config.ais_prio[0]);
                    switch (ais_config.period)
                    {
                        case VTSS_MEP_MGMT_PERIOD_1S:         CPRINTF("%11s", "1s");     break;
                        case VTSS_MEP_MGMT_PERIOD_1M:         CPRINTF("%11s", "1m");     break;
                        default:                              CPRINTF("%11s", "Unknown");
                    }
                    if (ais_config.protection)      CPRINTF("%15s", "Enabled");
                    else                            CPRINTF("%15s", "Disabled");
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_lck_config (cli_req_t * req)
{
    u32                          i;
    vtss_mep_mgmt_lck_conf_t     lck_config;
    vtss_mep_mgmt_client_conf_t  client_config;
    mep_cli_req_t                *mep_req = req->module_req;
    vtss_rc                      rc;

    if (req->set) {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_lck_conf_get(mep_req->instance, &lck_config)) != VTSS_RC_OK) {
            mep_print_error(rc);
            return;
        }


        if ((rc = mep_mgmt_client_conf_get(mep_req->instance, &client_config)) != VTSS_RC_OK) {
            mep_print_error(rc);
            return;
        }

        lck_config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PERIOD)  lck_config.period        = mep_req->period;
        if (mep_req->flag & FLAG_PARAM_PRIO) {
            for (i=0; i<VTSS_MEP_CLIENT_FLOWS_MAX; i++)
                client_config.lck_prio[i] = mep_req->prio;
        }

        if ((rc = mep_mgmt_lck_conf_set(mep_req->instance, &lck_config)) != VTSS_RC_OK)
            mep_print_error(rc);
        if ((rc = mep_mgmt_client_conf_set(mep_req->instance, &client_config)) != VTSS_RC_OK)
            mep_print_error(rc);
    } else {
        CPRINTF("\n");
        CPRINTF("MEP LCK Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%9s", "Prio");
        CPRINTF("%11s", "Period");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_lck_conf_get(i, &lck_config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            if ((rc = mep_mgmt_client_conf_get(i, &client_config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (lck_config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%9d", client_config.lck_prio[0]);
                    switch (lck_config.period)
                    {
                        case VTSS_MEP_MGMT_PERIOD_1S:         CPRINTF("%11s", "1s");     break;
                        case VTSS_MEP_MGMT_PERIOD_1M:         CPRINTF("%11s", "1m");     break;
                        default:                              CPRINTF("%11s", "Unknown");
                    }
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_lt_config(cli_req_t *req)
{
    u32                        i;
    vtss_mep_mgmt_lt_conf_t    config;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_lt_conf_get(mep_req->instance, &config)) != VTSS_RC_OK) {
            mep_print_error(rc);
            return;
        }

        config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PRIO)        config.prio = mep_req->prio;
        if (mep_req->flag & FLAG_PARAM_MEP)         config.mep = mep_req->mep;
        if (mep_req->flag & FLAG_PARAM_MAC_ADDR)    memcpy(config.mac, mep_req->mac_addr, VTSS_MEP_MAC_LENGTH);
        if (mep_req->flag & FLAG_PARAM_TTL)         config.ttl = mep_req->ttl;
//why this !!        memset(config.mac, 0, sizeof(config.mac));

        if ((rc = mep_mgmt_lt_conf_set(mep_req->instance, &config)) != VTSS_RC_OK)
            mep_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP LT Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%9s", "Prio");
        CPRINTF("%8s", "Mep");
        CPRINTF("%22s", "MAC");
        CPRINTF("%8s", "Ttl");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_lt_conf_get(i, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%9u", config.prio);
                    CPRINTF("%8u", config.mep);
                    CPRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", config.mac[0], config.mac[1], config.mac[2], config.mac[3], config.mac[4], config.mac[5]);
                    CPRINTF("%8u", config.ttl);
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_lb_config(cli_req_t *req)
{
    u32                        i;
    vtss_mep_mgmt_lb_conf_t    config;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_lb_conf_get(mep_req->instance, &config)) != VTSS_RC_OK) {
            mep_print_error(rc);
            return;
        }


        config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PRIO)        config.prio = mep_req->prio;
        if (mep_req->flag & FLAG_PARAM_DEI)         config.dei = mep_req->dei;
        if (mep_req->flag & FLAG_PARAM_CAST)        config.cast = mep_req->cast;
        if (mep_req->flag & FLAG_PARAM_MEP)         config.mep = mep_req->mep;
        if (mep_req->flag & FLAG_PARAM_MAC_ADDR)    memcpy(config.mac, mep_req->mac_addr, VTSS_MEP_MAC_LENGTH);
        if (mep_req->flag & FLAG_PARAM_TO_SEND)     config.to_send = mep_req->to_send;
        if (mep_req->flag & FLAG_PARAM_SIZE)        config.size = mep_req->size;
        if (mep_req->flag & FLAG_PARAM_INTERVAL)    config.interval = mep_req->interval;

        if ((rc = mep_mgmt_lb_conf_set(mep_req->instance, &config)) != VTSS_RC_OK)
            mep_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP LB Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%8s", "Dei");
        CPRINTF("%9s", "Prio");
        CPRINTF("%9s", "Cast");
        CPRINTF("%8s", "Mep");
        CPRINTF("%22s", "MAC");
        CPRINTF("%11s", "ToSend");
        CPRINTF("%9s", "Size");
        CPRINTF("%13s", "Interval");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_lb_conf_get(i, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%8u", config.dei);
                    CPRINTF("%9u", config.prio);
                    switch (config.cast)
                    {
                        case VTSS_MEP_MGMT_UNICAST:       CPRINTF("%9s", "Uni");     break;
                        case VTSS_MEP_MGMT_MULTICAST:     CPRINTF("%9s", "Multi");   break;
                    }
                    CPRINTF("%8u", config.mep);
                    CPRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", config.mac[0], config.mac[1], config.mac[2], config.mac[3], config.mac[4], config.mac[5]);
                    CPRINTF("%11u", config.to_send);
                    CPRINTF("%9u", config.size);
                    CPRINTF("%13u", config.interval);
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_tst_config(cli_req_t *req)
{
    u32                        i;
    vtss_mep_mgmt_tst_conf_t   config;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    if (req->set)
    {
        if ((mep_req->flag & (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) != (FLAG_PARAM_INSTANCE | FLAG_PARAM_ENABLE)) {
            CPRINTF("\n'instance' and 'enable|disable' required\n");
            return; 
        }

        if ((rc = mep_mgmt_tst_conf_get(mep_req->instance, &config)) != VTSS_RC_OK) {
            mep_print_error(rc);
            return;
        }

        config.enable = mep_req->enable;
        if (mep_req->flag & FLAG_PARAM_PRIO)        config.prio = mep_req->prio;
        if (mep_req->flag & FLAG_PARAM_DEI)         config.dei = mep_req->dei;
        if (mep_req->flag & FLAG_PARAM_MEP)         config.mep = mep_req->mep;
        if (mep_req->flag & FLAG_PARAM_RATE)        config.rate = mep_req->rate * 1000;
        if (mep_req->flag & FLAG_PARAM_SIZE)        config.size = mep_req->size;
        if (mep_req->flag & FLAG_PARAM_PATTERN)     config.pattern = mep_req->pattern;
        if (mep_req->flag & FLAG_PARAM_SEQUENCE)    config.sequence = mep_req->sequence;

        if ((rc = mep_mgmt_tst_conf_set(mep_req->instance, &config)) != VTSS_RC_OK)
            mep_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("MEP TST Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%8s", "Dei");
        CPRINTF("%9s", "Prio");
        CPRINTF("%8s", "Mep");
        CPRINTF("%9s", "rate");
        CPRINTF("%9s", "Size");
        CPRINTF("%13s", "Pattern");
        CPRINTF("%13s", "Sequence");
        CPRINTF("\n");
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_tst_conf_get(i, &config)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (config.enable)
                {
                    CPRINTF("%9u", i+1);
                    CPRINTF("%8u", config.dei);
                    CPRINTF("%9u", config.prio);
                    CPRINTF("%8u", config.mep);
                    CPRINTF("%9u", config.rate/1000);
                    CPRINTF("%9u", config.size);
                    switch (config.pattern)
                    {
                        case VTSS_MEP_MGMT_PATTERN_ALL_ZERO:    CPRINTF("%13s", "All zero");     break;
                        case VTSS_MEP_MGMT_PATTERN_ALL_ONE:     CPRINTF("%13s", "All one");   break;
                        case VTSS_MEP_MGMT_PATTERN_0XAA:        CPRINTF("%13s", "0xAA");   break;
                    }
                    if (config.sequence)
                        CPRINTF("%13s", "seq");
                    else
                        CPRINTF("%13s", "no_seq");
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_mep_state(cli_req_t *req)
{
    u32                     i, j;
    vtss_mep_mgmt_state_t   state;
    vtss_mep_mgmt_conf_t    config;
    u32                     eps_count;
    u16                     eps_inst[MEP_EPS_MAX];
    u8                      mac[VTSS_MEP_MAC_LENGTH];
    mep_cli_req_t           *mep_req = req->module_req;
    vtss_rc                 rc;

    CPRINTF("MEP state is:\n");
    CPRINTF("%9s", "Inst");
    CPRINTF("%11s", "cLevel");
    CPRINTF("%11s", "cMeg");
    CPRINTF("%11s", "cMep");
    CPRINTF("%11s", "cAis");
    CPRINTF("%11s", "cLck");
    CPRINTF("%11s", "cSsf");
    CPRINTF("%11s", "aBlk");
    CPRINTF("%11s", "aTsf");
    CPRINTF("%11s", "Peer MEP");
    CPRINTF("%11s", "cLoc");
    CPRINTF("%11s", "cRdi");
    CPRINTF("%11s", "cPeriod");
    CPRINTF("%11s", "cPrio");

    CPRINTF("\n");
    for (i=0; i<MEP_INSTANCE_MAX; ++i) {
        if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i)) {
            continue;
        }
        
        if ((rc = mep_mgmt_conf_get(i, mac, &eps_count, eps_inst, &config)) == MEP_RC_INVALID_PARAMETER) {
            mep_print_error(rc);
        }
        
        if (config.mode == VTSS_MEP_MGMT_MIP) {
            continue;
        }
        
        if ((rc = mep_mgmt_state_get(i, &state)) == MEP_RC_INVALID_PARAMETER) {
            mep_print_error(rc);
        } else {
            if (rc != MEP_RC_NOT_ENABLED) {
                CPRINTF("%9u", i+1);
                CPRINTF("%11s",  (state.cLevel) ? "True" : "False");
                CPRINTF("%11s",  (state.cMeg) ? "True" : "False");
                CPRINTF("%11s",  (state.cMep) ? "True" : "False");
                CPRINTF("%11s",  (state.cAis) ? "True" : "False");
                CPRINTF("%11s",  (state.cLck) ? "True" : "False");
                CPRINTF("%11s",  (state.cSsf) ? "True" : "False");
                CPRINTF("%11s",  (state.aBlk) ? "True" : "False");
                CPRINTF("%11s",  (state.aTsf) ? "True" : "False");
                
                for (j=0; j<config.peer_count; ++j) {
                    if (j != 0)     CPRINTF("%97s", " ");
                    CPRINTF("%11u", config.peer_mep[j]);
                    CPRINTF("%11s",  (state.cLoc[j]) ? "True" : "False");
                    CPRINTF("%11s",  (state.cRdi[j]) ? "True" : "False");
                    CPRINTF("%11s",  (state.cPeriod[j]) ? "True" : "False");
                    CPRINTF("%11s",  (state.cPrio[j]) ? "True" : "False");
                    CPRINTF("\n");
                }
                
                if (config.peer_count == 0) {
                    CPRINTF("\n");
                }
            }
        }
    }
    CPRINTF("\n");
}

static void cli_cmd_id_mep_lm_state(cli_req_t *req)
{
    u32                        i;
    vtss_mep_mgmt_lm_state_t   state;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    CPRINTF("MEP LM state is:\n");
    CPRINTF("%9s", "Inst");
    CPRINTF("%7s", "Tx");
    CPRINTF("%7s", "Rx");
    CPRINTF("%15s", "Near Count");
    CPRINTF("%14s", "Far Count");
    CPRINTF("%15s", "Near Ratio");
    CPRINTF("%14s", "Far Ratio");
    CPRINTF("\n");
    for (i=0; i<MEP_INSTANCE_MAX; ++i)
    {
        if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
        if ((rc = mep_mgmt_lm_state_get(i, &state)) == MEP_RC_INVALID_PARAMETER)
            mep_print_error(rc);
        else
        {
            if (rc != MEP_RC_NOT_ENABLED)
            {
                CPRINTF("%9u", i+1);
                CPRINTF("%7u", state.tx_counter);
                CPRINTF("%7u", state.rx_counter);
                CPRINTF("%15u", state.near_los_counter);
                CPRINTF("%14u", state.far_los_counter);
                CPRINTF("%15u", state.near_los_ratio);
                CPRINTF("%14u", state.far_los_ratio);
                CPRINTF("\n");
            }
        }
    }
    CPRINTF("\n");
}

static void cli_cmd_id_mep_lm_state_clear(cli_req_t *req)
{
    vtss_rc          rc;
    mep_cli_req_t    *mep_req = req->module_req;

    if ((rc = mep_mgmt_lm_state_clear_set(mep_req->instance)) == MEP_RC_INVALID_PARAMETER) {
        mep_print_error(rc);
    }
}

static void cli_cmd_id_mep_lt_state(cli_req_t *req)
{
    u32                        i, j, t;
    vtss_mep_mgmt_lt_state_t   state;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    CPRINTF("MEP LT state is:\n");
    CPRINTF("%9s", "Inst");
    CPRINTF("%19s", "Transaction ID");
    CPRINTF("%8s", "Ttl");
    CPRINTF("%9s", "Mode");
    CPRINTF("%14s", "Direction");
    CPRINTF("%12s", "Forwarded");
    CPRINTF("%10s", "relay");
    CPRINTF("%22s", "Last MAC");
    CPRINTF("%22s", "Next MAC");
    CPRINTF("\n");
    for (i=0; i<MEP_INSTANCE_MAX; ++i)
    {
        if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
        if ((rc = mep_mgmt_lt_state_get(i, &state)) == MEP_RC_INVALID_PARAMETER)
            mep_print_error(rc);
        else
        {
            if ((rc != MEP_RC_NOT_ENABLED) && (state.transaction_cnt != 0))
            {
                CPRINTF("%9u", i+1);
                for (t=0; t<state.transaction_cnt; ++t)
                {
                    if (t != 0)     CPRINTF("         ");
                    CPRINTF("%19u", state.transaction[t].transaction_id);
                    
                    for (j=0; j<state.transaction[t].reply_cnt; ++j)
                    {
                        if (j != 0)     CPRINTF("                            ");
                        CPRINTF("%8u", state.transaction[t].reply[j].ttl);
                        switch (state.transaction[t].reply[j].mode)
                        {
                            case VTSS_MEP_MGMT_MEP:       CPRINTF("%9s", "Mep");     break;
                            case VTSS_MEP_MGMT_MIP:       CPRINTF("%9s", "Mip");     break;
                        }
                        switch (state.transaction[t].reply[j].direction)
                        {
                            case VTSS_MEP_MGMT_DOWN:   CPRINTF("%14s", "Ingress");     break;
                            case VTSS_MEP_MGMT_UP:    CPRINTF("%14s", "Egress");      break;
                        }
                        CPRINTF("%12s", state.transaction[t].reply[j].forwarded ? "Yes" : "No");
                        switch (state.transaction[t].reply[j].relay_action) {
                            case VTSS_MEP_MGMT_RELAY_UNKNOWN:   CPRINTF("%10s", "Unknown");     break;
                            case VTSS_MEP_MGMT_RELAY_HIT:       CPRINTF("%10s", "MAC");         break;
                            case VTSS_MEP_MGMT_RELAY_FDB:       CPRINTF("%10s", "FDB");         break;
                            case VTSS_MEP_MGMT_RELAY_MFDB:      CPRINTF("%10s", "CCM FDB");     break;
                        }
                        CPRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", state.transaction[t].reply[j].last_egress_mac[0], state.transaction[t].reply[j].last_egress_mac[1], state.transaction[t].reply[j].last_egress_mac[2], state.transaction[t].reply[j].last_egress_mac[3], state.transaction[t].reply[j].last_egress_mac[4], state.transaction[t].reply[j].last_egress_mac[5]);
                        CPRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", state.transaction[t].reply[j].next_egress_mac[0], state.transaction[t].reply[j].next_egress_mac[1], state.transaction[t].reply[j].next_egress_mac[2], state.transaction[t].reply[j].next_egress_mac[3], state.transaction[t].reply[j].next_egress_mac[4], state.transaction[t].reply[j].next_egress_mac[5]);
                        CPRINTF("\n");
                    }
                    if (state.transaction[t].reply_cnt == 0)    CPRINTF("\n");
                }
            }
        }
    }
    CPRINTF("\n");
}

static void cli_cmd_id_mep_lb_state(cli_req_t *req)
{
    u32                        i, j;
    vtss_mep_mgmt_lb_state_t   state;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    CPRINTF("MEP LB state is:\n");
    CPRINTF("%9s", "Inst");
    CPRINTF("%19s", "Transaction ID");
    CPRINTF("%11s", "TX LBM");
    CPRINTF("%22s", "MAC");
    CPRINTF("%13s", "Received");
    CPRINTF("%17s", "Out Of Order");
    CPRINTF("\n");
    for (i=0; i<MEP_INSTANCE_MAX; ++i)
    {
        if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
        if ((rc = mep_mgmt_lb_state_get(i, &state)) == MEP_RC_INVALID_PARAMETER)
            mep_print_error(rc);
        else
        {
            if ((rc != MEP_RC_NOT_ENABLED) && (state.reply_cnt != 0))
            {
                CPRINTF("%9u", i+1);
                CPRINTF("%19u", state.transaction_id);
                CPRINTF("%11llu", state.lbm_transmitted);
                for (j=0; j<state.reply_cnt; ++j)
                {
                    if (j != 0)     CPRINTF("                                       ");
                    
                    CPRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", state.reply[j].mac[0], state.reply[j].mac[1], state.reply[j].mac[2], state.reply[j].mac[3], state.reply[j].mac[4], state.reply[j].mac[5]);
                    CPRINTF("%12llu", state.reply[j].lbr_received);
                    CPRINTF("%14llu\n", state.reply[j].out_of_order);
                }
            }
        }
    }
    CPRINTF("\n");
}

static void cli_cmd_id_mep_tst_state(cli_req_t *req)
{
    u32                        i;
    vtss_mep_mgmt_tst_state_t  state;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    CPRINTF("MEP TST state is:\n");
    CPRINTF("%9s", "Inst");
    CPRINTF("%19s", "TX frame count");
    CPRINTF("%19s", "RX frame count");
    CPRINTF("%12s", "RX rate");
    CPRINTF("%14s", "Test time");
    CPRINTF("\n");
    for (i=0; i<MEP_INSTANCE_MAX; ++i)
    {
        if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
        if ((rc = mep_mgmt_tst_state_get(i, &state)) == MEP_RC_INVALID_PARAMETER)
            mep_print_error(rc);
        else
        {
            if (rc != MEP_RC_NOT_ENABLED)
            {
                CPRINTF("%9u", i+1);
                CPRINTF("%19llu", state.tx_counter);
                CPRINTF("%19llu", state.rx_counter);
                CPRINTF("%12u", state.rx_rate);
                CPRINTF("%14u\n", state.time);
            }
        }
    }
    CPRINTF("\n");
}

static void cli_cmd_id_mep_tst_state_clear(cli_req_t *req)
{
    vtss_rc          rc;
    mep_cli_req_t    *mep_req = req->module_req;

    if ((rc = mep_mgmt_tst_state_clear_set(mep_req->instance)) == MEP_RC_INVALID_PARAMETER)
        mep_print_error(rc);
}

static void cli_cmd_id_mep_dm_state(cli_req_t *req)
{
    u32                        i, j;
    vtss_mep_mgmt_dm_state_t   dmr_state, dm1_state_far_to_near, dm1_state_near_to_far;
    mep_cli_req_t              *mep_req = req->module_req;
    vtss_rc                    rc;

    CPRINTF("MEP DM state is:\n");
    
    CPRINTF("RxT : Rx Timeout\n");
    CPRINTF("RxE : Rx Error\n");
    CPRINTF("AT  : Average Total\n");
    CPRINTF("AN  : Average last N\n");
    CPRINTF("AVT : Average Variation Total\n");
    CPRINTF("AVN : Average Variation last N\n");
    CPRINTF("OV  : Overflow. The number of statistics overflow.\n");
    
    CPRINTF("\n");
    CPRINTF("%4s", "Inst");
    CPRINTF("%9s", "Tx");
    CPRINTF("%9s", "RxT");
    CPRINTF("%9s", "Rx");
    CPRINTF("%9s", "RxE");
    CPRINTF("%10s", "AT");
    CPRINTF("%10s", "AN");
    CPRINTF("%10s", "AVT");
    CPRINTF("%9s", "AVN");
    CPRINTF("%9s", "Max ");
    CPRINTF("%10s", "Min  ");
    CPRINTF("%4s", "OV");
    
    for (j=VTSS_MEP_MGMT_US; j<= VTSS_MEP_MGMT_NS; j++)
    {
        CPRINTF("\n");
        if (j == VTSS_MEP_MGMT_US)
            CPRINTF("%8s", "One-way(time unit: us)");
        else
            CPRINTF("%8s", "One-way(time unit: ns)");    
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_dm_state_get(i, &dmr_state, &dm1_state_far_to_near, &dm1_state_near_to_far)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (rc != MEP_RC_NOT_ENABLED)
                {   
                    if (dm1_state_far_to_near.tunit != j)   
                        continue;
                    CPRINTF("\n");
                    CPRINTF("Far-end-to-near-end\n");
                    CPRINTF("%4u", i+1);
                    CPRINTF("%9u", dm1_state_far_to_near.tx_cnt);
                    CPRINTF("%9u", dm1_state_far_to_near.rx_tout_cnt);
                    CPRINTF("%9u", dm1_state_far_to_near.rx_cnt);
                    CPRINTF("%9u", dm1_state_far_to_near.rx_err_cnt);
                    CPRINTF(" %9u", dm1_state_far_to_near.avg_delay);
                    CPRINTF(" %9u", dm1_state_far_to_near.avg_n_delay);
                    CPRINTF(" %9u", dm1_state_far_to_near.avg_delay_var);
                    CPRINTF(" %8u", dm1_state_far_to_near.avg_n_delay_var);
                    CPRINTF(" %8u", dm1_state_far_to_near.min_delay);
                    CPRINTF(" %9u", dm1_state_far_to_near.max_delay);
                    CPRINTF(" %3u", dm1_state_far_to_near.ovrflw_cnt);
                    CPRINTF("\n");
                    CPRINTF("Near-end-to-far-end\n");
                    CPRINTF("%4u", i+1);
                    CPRINTF("%9u", dm1_state_near_to_far.tx_cnt);
                    CPRINTF("%9u", dm1_state_near_to_far.rx_tout_cnt);
                    CPRINTF("%9u", dm1_state_near_to_far.rx_cnt);
                    CPRINTF("%9u", dm1_state_near_to_far.rx_err_cnt);
                    CPRINTF(" %9u", dm1_state_near_to_far.avg_delay);
                    CPRINTF(" %9u", dm1_state_near_to_far.avg_n_delay);
                    CPRINTF(" %9u", dm1_state_near_to_far.avg_delay_var);
                    CPRINTF(" %8u", dm1_state_near_to_far.avg_n_delay_var);
                    CPRINTF(" %8u", dm1_state_near_to_far.min_delay);
                    CPRINTF(" %9u", dm1_state_near_to_far.max_delay);
                    CPRINTF(" %3u", dm1_state_near_to_far.ovrflw_cnt);
                    CPRINTF("\n");
                    
                    
                }
            }
        }
        CPRINTF("\n");

        if (j == VTSS_MEP_MGMT_US)
            CPRINTF("%8s", "Two-way(time unit: us)");
        else
            CPRINTF("%8s", "Two-way(time unit: ns)");
        
        for (i=0; i<MEP_INSTANCE_MAX; ++i)
        {
            if ((mep_req->instance != 0xFFFFFFFF) && (mep_req->instance != i))      continue;
            if ((rc = mep_mgmt_dm_state_get(i, &dmr_state, &dm1_state_far_to_near, &dm1_state_near_to_far)) == MEP_RC_INVALID_PARAMETER)
                mep_print_error(rc);
            else
            {
                if (rc != MEP_RC_NOT_ENABLED)
                {
                    if (dmr_state.tunit != j)   
                        continue;
                    CPRINTF("\n");
                    CPRINTF("%4u", i+1);
                    CPRINTF("%9u", dmr_state.tx_cnt);
                    CPRINTF("%9u", dmr_state.rx_tout_cnt);
                    CPRINTF("%9u", dmr_state.rx_cnt);
                    CPRINTF("%9u", dmr_state.rx_err_cnt);
                    CPRINTF(" %9u", dmr_state.avg_delay);
                    CPRINTF(" %9u", dmr_state.avg_n_delay);
                    CPRINTF(" %9u", dmr_state.avg_delay_var);
                    CPRINTF(" %8u", dmr_state.avg_n_delay_var);
                    CPRINTF(" %8u", dmr_state.min_delay);
                    CPRINTF(" %9u", dmr_state.max_delay);
                    CPRINTF(" %3u", dmr_state.ovrflw_cnt);
                }
            }
        }
    }        
    CPRINTF("\n");
    
}

#if defined(VTSS_SW_OPTION_UP_MEP)
static void cli_cmd_id_debug_mep_up_mep(cli_req_t *req)
{
    mep_cli_req_t      *mep_req = req->module_req;
    port_conf_t        port_conf;
    pvlan_mgmt_entry_t pvlan_conf;
    vtss_learn_mode_t  learn_mode;
    vlan_port_conf_t   vlan_conf;
    u32                i;
    vtss_rc            rc;

    /* Enable Up-MEP */
    mep_mgmt_up_mep_enable(mep_req->enable);

    /* Enable loop on all loop UNI */
    for (i=16; i<23; ++i) {
        if(port_mgmt_conf_get(VTSS_ISID_LOCAL, i, &port_conf) != VTSS_OK)   continue;

        if (!mep_req->enable)     port_conf.adv_dis &= ~PORT_ADV_UP_MEP_LOOP;
        else                      port_conf.adv_dis |= PORT_ADV_UP_MEP_LOOP;

        if (port_mgmt_conf_set(VTSS_ISID_START, i, &port_conf) == PORT_ERROR_PARM)
            CPRINTF("Port %u does not support this mode\n", iport2uport(i));
    }


    /* Create the PVLAN groups that seperate UNI in three groups */
    pvlan_conf.privatevid = 0;
    memset(pvlan_conf.ports, 0, sizeof(pvlan_conf.ports));
    for (i=0; i<16; ++i)
        pvlan_conf.ports[i] = TRUE;
    if (pvlan_mgmt_pvlan_add(VTSS_ISID_START, &pvlan_conf) != VTSS_OK)    CPRINTF("PVLAN create %u failed\n", 1);

    pvlan_conf.privatevid = 1;
    memset(pvlan_conf.ports, 0, sizeof(pvlan_conf.ports));
    for (i=0; i<12; ++i)
        pvlan_conf.ports[i] = TRUE;
    for (i=16; i<20; ++i)
        pvlan_conf.ports[i] = TRUE;
    if (pvlan_mgmt_pvlan_add(VTSS_ISID_START, &pvlan_conf) != VTSS_OK)    CPRINTF("PVLAN create %u failed\n", 2);

    pvlan_conf.privatevid = 2;
    memset(pvlan_conf.ports, 0, sizeof(pvlan_conf.ports));
    for (i=0; i<12; ++i)
        pvlan_conf.ports[i] = TRUE;
    for (i=20; i<24; ++i)
        pvlan_conf.ports[i] = TRUE;
    if (pvlan_mgmt_pvlan_add(VTSS_ISID_START, &pvlan_conf) != VTSS_OK)    CPRINTF("PVLAN create %u failed\n", 3);

    /* Learning must be disabled on all loop UNI */
    learn_mode.automatic = FALSE;
    learn_mode.cpu = FALSE;
    learn_mode.discard = FALSE;
    for (i=16; i<24; ++i)
        if (mac_mgmt_learn_mode_set(VTSS_ISID_START, i, &learn_mode) == MAC_ERROR_LEARN_FORCE_SECURE)
            CPRINTF("The learn mode can not be changed on port %d while the learn mode is forced to 'secure' (probably by 802.1X module)\n", iport2uport(i));



    /* Customer ports are C-Tagged and TX tag type all */
    for (i=12; i<16; ++i) {
        if (vlan_mgmt_port_conf_get(VTSS_ISID_START, i, &vlan_conf, VLAN_USER_STATIC) != VTSS_OK)
            continue;
        vlan_conf.flags = VLAN_PORT_FLAGS_ALL;
        vlan_conf.tx_tag_type = VLAN_TX_TAG_TYPE_TAG_ALL;
        vlan_conf.port_type = VLAN_PORT_TYPE_C;
        if ((rc = vlan_mgmt_port_conf_set(VTSS_ISID_START, i, &vlan_conf, VLAN_USER_STATIC)) != VTSS_RC_OK) {
            T_E("%u: %s", iport2uport(i), error_txt(rc));
            continue;
        }
    }


    /* Customer ports are C-Tagged and Ingress Filtering disabled */
    for (i=16; i<24; ++i) {
        if (vlan_mgmt_port_conf_get(VTSS_ISID_START, i, &vlan_conf, VLAN_USER_STATIC) != VTSS_OK)
            continue;
        vlan_conf.flags = VLAN_PORT_FLAGS_ALL;
        vlan_conf.ingress_filter = FALSE;
        vlan_conf.port_type = VLAN_PORT_TYPE_C;
        if ((rc = vlan_mgmt_port_conf_set(VTSS_ISID_START, i, &vlan_conf, VLAN_USER_STATIC)) != VTSS_OK) {
            T_E("%u: %s", iport2uport(i), error_txt(rc));
            continue;
        }
    }
}
#endif

static void cli_cmd_id_debug_mep_dm_state(cli_req_t *req)
{
    u32                        i, j, count;
    vtss_mep_mgmt_dm_state_t   dmr_state, dm1_state_far_to_near, dm1_state_near_to_far;
    vtss_mep_mgmt_dm_conf_t    config;
    u32                        *delay = NULL;
    u32                        *delay_var = NULL;
    vtss_rc                    rc;

    CPRINTF("MEP DM state is:\n");
    CPRINTF("%9s", "Packet no.");
    CPRINTF("%19s", "Delay(ns)");
    CPRINTF("%22s", "Delay variation(ns)");
    CPRINTF("\n");

    /* Use malloc to avoid stackoverflow when VTSS_MEP_DM_MAX is too large */
    delay = VTSS_MALLOC(sizeof(u32) * VTSS_MEP_DM_MAX);
    if (delay == NULL )
        return; 
    delay_var = VTSS_MALLOC(sizeof(u32) * VTSS_MEP_DM_MAX);
    if (delay_var == NULL )
    {
        VTSS_FREE(delay);
        return;  
    } 
     
    for (i=0; i<MEP_INSTANCE_MAX; ++i)
    {
        if ((rc = mep_mgmt_dm_conf_get(i, &config)) != VTSS_RC_OK)
            continue;
        if ((rc = mep_mgmt_dm_db_state_get(i, delay, delay_var)) != VTSS_RC_OK)
            continue;
        else
        {
            rc = mep_mgmt_dm_state_get(i, &dmr_state, &dm1_state_far_to_near, &dm1_state_near_to_far);
            if (rc == VTSS_RC_OK)
            {    
                count = (dmr_state.rx_cnt > config.lastn) ? config.lastn : dmr_state.rx_cnt;
                
                for (j=0; j<count; j++)
                {
                    CPRINTF("%9u", j+1);
                    CPRINTF("%19u", *(delay + j));
                    CPRINTF("%22d", *(delay_var + j));
                    CPRINTF("\n");
                }
                
                CPRINTF("The late txtime count: %u\n", dmr_state.late_txtime);    
            }
        }
    }


    VTSS_FREE(delay);
    VTSS_FREE(delay_var);
    
    CPRINTF("\n");
    return;
}

static void cli_cmd_id_mep_dm_state_clear(cli_req_t *req)
{
    vtss_rc          rc;
    mep_cli_req_t    *mep_req = req->module_req;

    if ((rc = mep_mgmt_dm_state_clear_set(mep_req->instance)) == MEP_RC_INVALID_PARAMETER)
        mep_print_error(rc);
}

static int cli_mep_level_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 0, 7);
    mep_req->level = value;
    if (!error) mep_req->flag |= FLAG_PARAM_LEVEL;

    return (error);
}

static int cli_mep_meg_id_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    uint len;

    if (!(mep_req->flag & FLAG_PARAM_FORMAT))      return(TRUE);    /* Only parse MEG-ID if format is passed */

    if (!(error = cli_parse_text(cmd_org, req->parm, VTSS_MEP_MEG_CODE_LENGTH)))
    {
        len = strlen(req->parm);

        if (mep_req->format == VTSS_MEP_MGMT_ITU_ICC)       error = (len != 13);
        if (mep_req->format == VTSS_MEP_MGMT_ITU_CC_ICC)    error = (len != 15);
        if (len > VTSS_MEP_MEG_CODE_LENGTH-1)    error = TRUE;

        strncpy(&mep_req->meg_id[0], req->parm, VTSS_MEP_MEG_CODE_LENGTH-1);
        if (!error) mep_req->flag |= FLAG_PARAM_MEG_ID;
    }


    return (error);
}

static int cli_mep_meg_name_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    uint len;

    if (!(mep_req->flag & FLAG_PARAM_FORMAT))        return(TRUE);    /* Only parse Name if format is passed */
    if (mep_req->format != VTSS_MEP_MGMT_IEEE_STR)   return(TRUE);    /* Only parse Name if format is IEEE */

    if (!(error = cli_parse_text(cmd_org, req->parm, VTSS_MEP_MEG_CODE_LENGTH)))
    {
        len = strlen(req->parm);

        if (len > VTSS_MEP_MEG_CODE_LENGTH-1)    error = TRUE;

        strncpy(&mep_req->meg_name[0], req->parm, VTSS_MEP_MEG_CODE_LENGTH-1);
        if (!error) mep_req->flag |= FLAG_PARAM_MEG_NAME;
    }

    return (error);
}


static int cli_mep_mep_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 0, 0x1FFF);
    mep_req->mep = value;
    if (!error) mep_req->flag |= FLAG_PARAM_MEP;

    return (error);
}

static int cli_mep_ttl_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 1, 0xFF);
    mep_req->ttl = value;
    if (!error) mep_req->flag |= FLAG_PARAM_TTL;

    return (error);
}

static int cli_mep_flow_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value = 0;

    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    mep_req->flow = value-1;
    if (!error) mep_req->flag |= FLAG_PARAM_FLOW;

    return (error);
}

static int cli_mep_cflow_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value = 0;

    if (mep_req->client_flow_count < VTSS_MEP_CLIENT_FLOWS_MAX)
    {
        error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
        if (error)  return (error);
        mep_req->client_flows[mep_req->client_flow_count] = value-1;
        mep_req->client_flow_count++;
        mep_req->flag |= FLAG_PARAM_CLIENT_FLOWS;
    }
    else return(1);

    return(0);
}

static int cli_mep_vid_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value = 0;

    error = cli_parse_ulong(cmd, &value, 0, 0x0FFF);
    mep_req->vid = value;
    if (!error) mep_req->flag |= FLAG_PARAM_VID;

    return (error);
}

static int cli_mep_instance_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    mep_req->instance = value-1;
    if (!error) mep_req->flag |= FLAG_PARAM_INSTANCE;

    return (error);
}

static int cli_mep_priority_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 0, 0x07);
    mep_req->prio = value;
    if (!error) mep_req->flag |= FLAG_PARAM_PRIO;

    return (error);
}

static int cli_mep_flr_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    mep_req->flr_interval = value;
    if (!error) mep_req->flag |= FLAG_PARAM_FLR_INTERVAL;

    return (error);
}

static int cli_mep_octet_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 0, 0xFF);
    mep_req->octet = value;
    if (!error) mep_req->flag |= FLAG_PARAM_OCTET;

    return (error);
}

static int cli_mep_tosend_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    mep_req->to_send = value;
    if (!error) mep_req->flag |= FLAG_PARAM_TO_SEND;

    return (error);
}

static int cli_mep_tst_size_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 1, 1518);
    mep_req->size = value;
    if (!error) mep_req->flag |= FLAG_PARAM_SIZE;

    return (error);
}

static int cli_mep_rate_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 1, 400);
    mep_req->rate = value;
    if (!error) mep_req->flag |= FLAG_PARAM_RATE;

    return (error);
}

static int cli_mep_lb_size_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 1, 1400);
    mep_req->size = value;
    if (!error) mep_req->flag |= FLAG_PARAM_SIZE;

    return (error);
}

static int cli_mep_interval_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    if (mep_req->to_send != 0) error = cli_parse_ulong(cmd, &value, 0, 100);
    else                       error = cli_parse_ulong(cmd, &value, 1, 10000);
    mep_req->interval = value;
    if (!error) mep_req->flag |= FLAG_PARAM_INTERVAL;

    return (error);
}

static int cli_mep_dm_count_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 10, 2000);
    mep_req->count = value;
    if (!error) mep_req->flag |= FLAG_PARAM_COUNT;

    return (error);
}

static int cli_mep_port_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 1, VTSS_PORTS);
    mep_req->port = value;
    if (!error) mep_req->flag |= FLAG_PARAM_PORT;

    return (error);
}

static int cli_mep_mac_addr_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    cli_spec_t spec;

    error = cli_parse_mac(cmd, mep_req->mac_addr, &spec, 0);
    if (!error) mep_req->flag |= FLAG_PARAM_MAC_ADDR;

    return (error);
}

static int cli_mep_dm_gap_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;
    ulong value;

    error = cli_parse_ulong(cmd, &value, 10, 0xffff);
    mep_req->dm_gap = value;
    if (!error) mep_req->flag |= FLAG_PARAM_DM_GAP;

    return (error);
}

static int cli_mep_dei_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;

    if(!found)      return 1;
    else if(!strncmp(found, "set", 3))       mep_req->dei = TRUE;
    else if(!strncmp(found, "clear", 5))     mep_req->dei = FALSE;
    else return 1;
    mep_req->flag |= FLAG_PARAM_DEI;

    return (error);
}

static int cli_mep_seq_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;
    ulong error = 0;

    if(!found)      return 1;
    else if(!strncmp(found, "seq", 3))     mep_req->sequence = TRUE;
    else if(!strncmp(found, "no_seq", 6))  mep_req->sequence = FALSE;
    else return 1;
    mep_req->flag |= FLAG_PARAM_SEQUENCE;

    return (error);
}

static int cli_mep_period_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "300s", 4))    mep_req->period = VTSS_MEP_MGMT_PERIOD_300S;
    else if(!strncmp(found, "100s", 4))    mep_req->period = VTSS_MEP_MGMT_PERIOD_100S;
    else if(!strncmp(found, "10s", 3))     mep_req->period = VTSS_MEP_MGMT_PERIOD_10S;
    else if(!strncmp(found, "1s", 2))      mep_req->period = VTSS_MEP_MGMT_PERIOD_1S;
    else if(!strncmp(found, "6m", 2))      mep_req->period = VTSS_MEP_MGMT_PERIOD_6M;
    else if(!strncmp(found, "1m", 2))      mep_req->period = VTSS_MEP_MGMT_PERIOD_1M;
    else if(!strncmp(found, "6h", 2))      mep_req->period = VTSS_MEP_MGMT_PERIOD_6H;
    else return 1;
    mep_req->flag |= FLAG_PARAM_PERIOD;

    return 0;
}

static int cli_mep_lm_period_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "10s", 3))     mep_req->period = VTSS_MEP_MGMT_PERIOD_10S;
    else if(!strncmp(found, "1s", 2))      mep_req->period = VTSS_MEP_MGMT_PERIOD_1S;
    else if(!strncmp(found, "6m", 2))      mep_req->period = VTSS_MEP_MGMT_PERIOD_6M;
    else if(!strncmp(found, "1m", 2))      mep_req->period = VTSS_MEP_MGMT_PERIOD_1M;
    else if(!strncmp(found, "6h", 2))      mep_req->period = VTSS_MEP_MGMT_PERIOD_6H;
    else return 1;
    mep_req->flag |= FLAG_PARAM_PERIOD;

    return 0;
}

static int cli_mep_ais_lck_period_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "1s", 2))    mep_req->period = VTSS_MEP_MGMT_PERIOD_1S;
    else if(!strncmp(found, "1m", 2))    mep_req->period = VTSS_MEP_MGMT_PERIOD_1M;
    else return 1;
    mep_req->flag |= FLAG_PARAM_PERIOD;

    return 0;
}

static int cli_mep_domain_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "domport", 7))       mep_req->domain = VTSS_MEP_MGMT_PORT;
//    else if(!strncmp(found, "domesp", 6))        mep_req->domain = VTSS_MEP_MGMT_ESP;
    else if(!strncmp(found, "domevc", 6))        mep_req->domain = VTSS_MEP_MGMT_EVC;
    else if(!strncmp(found, "domvlan", 7))       mep_req->domain = VTSS_MEP_MGMT_VLAN;
//    else if(!strncmp(found, "dommpls", 7))       mep_req->domain = VTSS_MEP_MGMT_MPLS;
    else return 1;
    mep_req->flag |= FLAG_PARAM_DOMAIN;

    return 0;
}

static int cli_mep_mode_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "mep", 3))       mep_req->mode = VTSS_MEP_MGMT_MEP;
    else if(!strncmp(found, "mip", 3))       mep_req->mode = VTSS_MEP_MGMT_MIP;
    else return 1;
    mep_req->flag |= FLAG_PARAM_MODE;

    return 0;
}

static int cli_mep_direction_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "ingress", 7))      mep_req->direction = VTSS_MEP_MGMT_DOWN;
    else if(!strncmp(found, "egress", 6))       mep_req->direction = VTSS_MEP_MGMT_UP;
    else return 1;
    mep_req->flag |= FLAG_PARAM_DIRECTION;

    return 0;
}

static int cli_mep_format_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "itu", 3))      mep_req->format = VTSS_MEP_MGMT_ITU_ICC;
    else if(!strncmp(found, "ieee", 4))     mep_req->format = VTSS_MEP_MGMT_IEEE_STR;
    else if(!strncmp(found, "itucc", 5))    mep_req->format = VTSS_MEP_MGMT_ITU_CC_ICC;
    else return 1;
    mep_req->flag |= FLAG_PARAM_FORMAT;

    return 0;
}

static int cli_mep_enable_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "enable", 6))       mep_req->enable = TRUE;
    else if(!strncmp(found, "disable", 7))      mep_req->enable = FALSE;
    else return 1;
    mep_req->flag |= FLAG_PARAM_ENABLE;

    return 0;
}

static int cli_mep_voe_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "voe", 3))       mep_req->voe = TRUE;
    else return 1;
    mep_req->flag |= FLAG_PARAM_VOE;

    return 0;
}

static int cli_mep_ended_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "single", 6))    mep_req->ended = VTSS_MEP_MGMT_SINGEL_ENDED;
    else if(!strncmp(found, "dual", 4))      mep_req->ended = VTSS_MEP_MGMT_DUAL_ENDED;
    else return 1;
    mep_req->flag |= FLAG_PARAM_ENDED;

    return 0;

}


static int cli_mep_way_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "oneway", 6))    mep_req->ended = VTSS_MEP_MGMT_DUAL_ENDED;
    else if(!strncmp(found, "twoway", 6))    mep_req->ended = VTSS_MEP_MGMT_SINGEL_ENDED;
    else return 1;
    mep_req->flag |= FLAG_PARAM_WAY;

    return 0;

}

static int cli_mep_txway_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "std", 3))    mep_req->proprietary = FALSE;
    else if(!strncmp(found, "prop", 4))   mep_req->proprietary = TRUE;
    else return 1;
    mep_req->flag |= FLAG_PARAM_TXWAY;

    return 0;

}

static int cli_mep_calcway_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "rdtrp", 5))    mep_req->calcway = VTSS_MEP_MGMT_RDTRP;
    else if(!strncmp(found, "flow", 4))     mep_req->calcway = VTSS_MEP_MGMT_FLOW;
    else return 1;
    mep_req->flag |= FLAG_PARAM_CALCWAY;

    return 0;

}

static int cli_mep_tunit_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "us", 2))   mep_req->tunit = VTSS_MEP_MGMT_US;
    else if(!strncmp(found, "ns", 2))   mep_req->tunit = VTSS_MEP_MGMT_NS;
    else return 1;
    mep_req->flag |= FLAG_PARAM_TUNIT;

    return 0;

}

static int cli_mep_act_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "keep", 4))  mep_req->act = VTSS_MEP_MGMT_DISABLE;
    else if(!strncmp(found, "reset", 5)) mep_req->act = VTSS_MEP_MGMT_CONTINUE;
    else return 1;
    mep_req->flag |= FLAG_PARAM_ACT;

    return 0;

}

static int cli_mep_dm2fordm1_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "d2ford1", 7))  mep_req->dm2fordm1 = 1;
    else return 1;
    mep_req->flag |= FLAG_PARAM_DM2FORDM1;

    return 0;
}





static int cli_mep_cast_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "uni", 3))    mep_req->cast = VTSS_MEP_MGMT_UNICAST;
    else if(!strncmp(found, "multi", 5))  mep_req->cast = VTSS_MEP_MGMT_MULTICAST;
    else return 1;
    mep_req->flag |= FLAG_PARAM_CAST;

    return 0;

}

static int cli_mep_aps_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "laps", 4))  mep_req->aps_type = VTSS_MEP_MGMT_L_APS;
    else if(!strncmp(found, "raps", 4))  mep_req->aps_type = VTSS_MEP_MGMT_R_APS;
    else return 1;
    mep_req->flag |= FLAG_PARAM_APS_TYPE;

    return 0;

}

static int cli_mep_protection_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "set", 3))    mep_req->protection = TRUE;
    else if(!strncmp(found, "clear", 5))  mep_req->protection = FALSE;
    else return 1;
    mep_req->flag |= FLAG_PARAM_PROTECTION;

    return 0;
}

static int cli_mep_pattern_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mep_cli_req_t *mep_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "allzero", 7))  mep_req->pattern = VTSS_MEP_MGMT_PATTERN_ALL_ZERO;
    else if(!strncmp(found, "allone", 6))   mep_req->pattern = VTSS_MEP_MGMT_PATTERN_ALL_ONE;
    else if(!strncmp(found, "onezero", 4))  mep_req->pattern = VTSS_MEP_MGMT_PATTERN_0XAA;
    else return 1;
    mep_req->flag |= FLAG_PARAM_PATTERN;

    return 0;

}

void mep_cli_def_req ( cli_req_t * req)
{
   mep_cli_req_t * mep_req = NULL;
  
   mep_req = req->module_req;

   mep_req->flag = 0;
   mep_req->instance = 0xFFFFFFFF;
   mep_req->enable = FALSE;
   mep_req->voe = FALSE;
   mep_req->level = 0;
   mep_req->mep = 0;
   mep_req->flow = 0;
   mep_req->client_flow_count = 0;
   mep_req->protection = FALSE;
   mep_req->vid = 0;
   mep_req->prio = 0;
   mep_req->dei = FALSE;
   mep_req->flr_interval = 0;
   mep_req->domain = VTSS_MEP_MGMT_PORT;
   mep_req->format = VTSS_MEP_MGMT_ITU_ICC;
   mep_req->period = VTSS_MEP_MGMT_PERIOD_10S;
   mep_req->ended = VTSS_MEP_MGMT_SINGEL_ENDED;
   mep_req->proprietary = FALSE;
   mep_req->calcway = VTSS_MEP_MGMT_RDTRP;
   mep_req->dm_gap = 100;
   mep_req->count = 2000;
   memset(mep_req->mac_addr, 0, sizeof(mep_req->mac_addr));
}

static cli_parm_t mep_cli_parm_table[] = {
    {
        "enable|disable",
        "enable/disable",
        CLI_PARM_FLAG_SET,
        cli_mep_enable_parse,
        NULL,
    },
    {
        "voe",
        "The instance will be VOE based if possible",
        CLI_PARM_FLAG_SET,
        cli_mep_voe_parse,
        NULL,
    },
    {
        "<level>",
        "MEP level (0-7)",
        CLI_PARM_FLAG_SET,
        cli_mep_level_parse,
        NULL,
    },
    {
        "<inst>",
        "Instance number",
        CLI_PARM_FLAG_NONE,
        cli_mep_instance_parse,
        NULL,
    },
    {
        "itu|ieee|itucc",
        "MEG format\n"
        "       ITU: ICC + UMC format as defined in Y.1731 ANNEX A\n"
        "       IEEE: String format Domain Name and Short Name as defined in 802.1ag\n"
        "       ITU CC: CC + ICC + UMC format as defined in Y.1731 ANNEX A\n",
        CLI_PARM_FLAG_SET,
        cli_mep_format_parse,
        NULL,
    },
    {
        "<meg_id>",
        "IEEE short name or ITU UMC\n"
        "       IEEE short name must be max. 16 chars\n"
        "       ITU ICC + UMC must be 13 chars\n"
        "       ITU CC + ICC + UMC must be 15 chars",
        CLI_PARM_FLAG_SET,
        cli_mep_meg_id_parse,
        NULL,
    },
    {
        "<name>",
        "IEEE Maintenance Domain name\n"
        "       IEEE Maintenance Domain name must be max. 16 chars\n",
        CLI_PARM_FLAG_SET,
        cli_mep_meg_name_parse,
        NULL,
    },
    {
        "<mep>",
        "This MEP id (0-0x1FFF)",
        CLI_PARM_FLAG_SET,
        cli_mep_mep_parse,
        NULL,
    },
    {
        "<flow>",
        "Flow instance number (Port/EVC)",
        CLI_PARM_FLAG_SET,
        cli_mep_flow_parse,
        NULL,
    },
    {
        "<cflow>",
        "Client flow instance number (EVC)",
        CLI_PARM_FLAG_SET,
        cli_mep_cflow_parse,
        NULL,
    },
    {
        "<flr>",
        "Frame loss ratio (in sec.)",
        CLI_PARM_FLAG_SET,
        cli_mep_flr_parse,
        cli_cmd_id_mep_lm_config,
    },
    {
        "<octet>",
        "The last octet in RAPS multicast MAC",
        CLI_PARM_FLAG_SET,
        cli_mep_octet_parse,
        cli_cmd_id_mep_aps_config,
    },
    {
        "single|dual",
        "LM is single or dual ended",
        CLI_PARM_FLAG_SET,
        cli_mep_ended_parse,
        cli_cmd_id_mep_lm_config,
    },
    {
        "<tosend>",
        "Number of LBM to send. 0 indicate infinite transmission (test behaviour). Requires VOE",
        CLI_PARM_FLAG_SET,
        cli_mep_tosend_parse,
        cli_cmd_id_mep_lb_config,
    },
    {
        "<size>",
        "Size of LBM data field in bytes (max 1400)",
        CLI_PARM_FLAG_SET,
        cli_mep_lb_size_parse,
        cli_cmd_id_mep_lb_config,
    },
    {
        "<size>",
        "Size of TST data field in bytes (max 1518)",
        CLI_PARM_FLAG_SET,
        cli_mep_tst_size_parse,
        cli_cmd_id_mep_tst_config,
    },
    {
        "<rate>",
        "Transmission bit rate of TST frames - in Mbps",
        CLI_PARM_FLAG_SET,
        cli_mep_rate_parse,
        cli_cmd_id_mep_tst_config,
    },
    {
        "<interval>",
        "Interval between LBM to send. In 10ms. in case 'tosend' is != 0 (max 100 - '0' is as fast as possible)\n"
        "                              In 1us. in case 'tosend' is == 0 (max 10.000)",
        CLI_PARM_FLAG_SET,
        cli_mep_interval_parse,
        cli_cmd_id_mep_lb_config,
    },
    {
        "uni|multi",
        "Destination address is unicast or multicast",
        CLI_PARM_FLAG_SET,
        cli_mep_cast_parse,
        NULL,
    },
    {
        "laps|raps",
        "Selection of Linear or Ring APS type",
        CLI_PARM_FLAG_SET,
        cli_mep_aps_parse,
        NULL,
    },
    {
        "1s|1m",
        "Transmit period for AIS\n"
        "       1s - to send OAM Frames in the rate of 1 per second\n"
        "       1m - to send OAM frames in the rate of 1 per minute",
        CLI_PARM_FLAG_SET,
        cli_mep_ais_lck_period_parse,
        cli_cmd_id_mep_ais_config,
    },
    {
        "1s|1m",
        "Transmit period for LCK\n"
        "       1s - to send OAM Frames in the rate of 1 per second\n"
        "       1m - to send OAM frames in the rate of 1 per minute",
        CLI_PARM_FLAG_SET,
        cli_mep_ais_lck_period_parse,
        cli_cmd_id_mep_lck_config,
    },
    {
        "10s|1s|6m|1m|6h",
        "LM period (10s -> 10 PDU pr. second)",
        CLI_PARM_FLAG_SET,
        cli_mep_lm_period_parse,
        cli_cmd_id_mep_lm_config,
    },
    {
        "300s|100s|10s|1s|6m|1m|6h",
        "OAM period (100s -> 100 PDU pr. second)",
        CLI_PARM_FLAG_SET,
        cli_mep_period_parse,
        NULL,
    },
    {
        "set|clear",
        "Protection usability set/clear",
        CLI_PARM_FLAG_SET,
        cli_mep_protection_parse,
        cli_cmd_id_mep_ais_config,
    },
    {
        "set|clear",
        "OAM DEI set/clear",
        CLI_PARM_FLAG_SET,
        cli_mep_dei_parse,
        NULL,
    },
    {
        "no_seq|seq",
        "TST sequence number transmission",
        CLI_PARM_FLAG_SET,
        cli_mep_seq_parse,
        NULL,
    },
    {
        "<prio>",
        "OAM PDU priority",
        CLI_PARM_FLAG_SET,
        cli_mep_priority_parse,
        NULL,
    },
    {
        "<ttl>",
        "LT - Time To Live",
        CLI_PARM_FLAG_SET,
        cli_mep_ttl_parse,
        cli_cmd_id_mep_lt_config,
    },
    {
        "domport|domevc|domvlan",
        "Flow domain",
        CLI_PARM_FLAG_SET,
        cli_mep_domain_parse,
        NULL,
    },
    {
        "mep|mip",
        "Mode of the MEP instance",
        CLI_PARM_FLAG_SET,
        cli_mep_mode_parse,
        cli_cmd_id_mep_config,
    },
    {
        "ingress|egress",
        "Direction of the MEP instance",
        CLI_PARM_FLAG_SET,
        cli_mep_direction_parse,
        cli_cmd_id_mep_config,
    },
    {
        "<vid>",
        "C-TAG only applicable for Port MEP",
        CLI_PARM_FLAG_SET,
        cli_mep_vid_parse,
        NULL,
    },
    {
        "oneway|twoway",
        "DM is one-way or two-way",
        CLI_PARM_FLAG_SET,
        cli_mep_way_parse,
        cli_cmd_id_mep_dm_config,
    },
    {
        "<gap>",
        "Gap between 1DM/DMM to send in 10ms(10-65535).",
        CLI_PARM_FLAG_SET,
        cli_mep_dm_gap_parse,
        cli_cmd_id_mep_dm_config,
    },
    {
        "std|prop",
        "Standard or Vitesse proprietary way(w/ follow-up packets) to send DM",
        CLI_PARM_FLAG_SET,
        cli_mep_txway_parse,
        cli_cmd_id_mep_dm_config,
    },
    {
        "rdtrp|flow",
        "2/4 timestamps selection",
        CLI_PARM_FLAG_SET,
        cli_mep_calcway_parse,
        cli_cmd_id_mep_dm_config,
    },
    {
        "<count>",
        "The number of last records to calculate(10 - 2000)",
        CLI_PARM_FLAG_SET,
        cli_mep_dm_count_parse,
        NULL,
    },
    {
        "us|ns",
        "Time resolution",
        CLI_PARM_FLAG_SET,
        cli_mep_tunit_parse,
        cli_cmd_id_mep_dm_config,
    },
    {
        "keep|reset",
        "The action to counter when overflow happens",
        CLI_PARM_FLAG_SET,
        cli_mep_act_parse,
        cli_cmd_id_mep_dm_config,
    },
    {
        "d2ford1",
        "Enable to use DMM/DMR packets to calculate one-way DM",
        CLI_PARM_FLAG_SET,
        cli_mep_dm2fordm1_parse,
        cli_cmd_id_mep_dm_config,
    },
    {
        "allzero|allone|onezero",
        "Data pattern to be filled in TST PDU",
        CLI_PARM_FLAG_SET,
        cli_mep_pattern_parse,
        cli_cmd_id_mep_tst_config,
    },
    {
        "<port>",
        "Port number.",
        CLI_PARM_FLAG_SET,
        cli_mep_port_parse,
        NULL,
    },
    {
        "<mac_addr>",
        "MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit)",
        CLI_PARM_FLAG_SET,
        cli_mep_mac_addr_parse,
        NULL
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    }
};

enum
{
  PRIO_MEP_CONFIG,
  PRIO_MEP_PEER_CONFIG,
  PRIO_MEP_CC_CONFIG,
  PRIO_MEP_LM_CONFIG,
  PRIO_MEP_APS_CONFIG,
  PRIO_MEP_CLIENT_CONFIG,
  PRIO_MEP_AIS_CONFIG,
  PRIO_MEP_LCK_CONFIG,
  PRIO_MEP_LT_CONFIG,
  PRIO_MEP_LB_CONFIG,
  PRIO_MEP_DM_CONFIG,
  PRIO_MEP_TST_CONFIG,
  PRIO_MEP_STATE,
  PRIO_MEP_LM_STATE,
  PRIO_MEP_LM_STATE_CLEAR,
  PRIO_MEP_LT_STATE,
  PRIO_MEP_LB_STATE,
  PRIO_MEP_DM_STATE,
  PRIO_MEP_DM_STATE_CLEAR,
  PRIO_MEP_TST_STATE,
  PRIO_MEP_TST_STATE_CLEAR,
  PRIO_DEBUG_MEP_TEST     = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_MEP_UP_MEP   = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_MEP_DM_STATE = CLI_CMD_SORT_KEY_DEFAULT,
};

/* Command table entries */
cli_cmd_tab_entry (
  "MEP config",
  "MEP config [<inst>] [mep|mip] [voe] [ingress|egress] [<port>] [domport|domevc|domvlan] [<level>] [itu|ieee|itucc] [<meg_id>] [<name>] [<mep>] [<vid>] [<flow>] [enable|disable]",
  "MEP instance configuration\n"
  "'mep|mip' this entity is either a MEP or a MIP - end point or intermediate point\n"
  "'voe' this entity will be based on HW VOE if possible\n"
  "'ingress|egress' this entity is either a Ingress (down) or Egress (up) type of MEP/MIP\n"
  "'domport|domevc|domvlan' the domain is either Port, EVC or VLAN\n"
  "'level' is the MEG level\n"
  "'port' is the residence port\n"
  "'flow' is the related flow instance number - Port number in Port domain - EVC number in EVC domain\n"
  "'vid' is used for TAGGED OAM in port domain\n"
  "'itu|ieee|itucc' is the MEG ID format\n"
  "'meg_id' is the MEG ID - max. 8 char in case of 'ieee' - 6 or 7 char in case of 'itu'\n"
  "'name' is the IEEE domain name or ITU ICC - max. 8 char in case of 'ieee' - 6 char in case of 'itu'\n"
  "'mep' is the MEP ID",
  PRIO_MEP_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP peer MEP",
  "MEP peer MEP [<inst>] [<mep>] [<mac_addr>] [enable|disable]",
  "MEP Peer MEP id configuration",
  PRIO_MEP_PEER_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_peer_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP cc config",
  "MEP cc config [<inst>] [<prio>] [300s|100s|10s|1s|6m|1m|6h] [enable|disable]",
  "MEP Continuity Check configuration\n"
  "'prio' is the priority (PCP) of transmitted CCM frame\n"
  "'300s|100s|10s|1s|6m|1m|6h' is the number of CCM frame pr. second",
  PRIO_MEP_CC_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_cc_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP aps config",
  "MEP aps config [<inst>] [<prio>] [uni|multi] [laps|raps] [<octet>] [enable|disable]",
  "MEP APS configuration\n"
  "'prio' is the priority (PCP) of transmitted APS frame\n"
  "'uni|multi' is selecting uni-cast or multi-cast transmission of APS frame\n"
  "'laps|raps' is selecting ELPS or ERPS protocol\n"
  "'octet' is the last octet in RAPS multicast MAC",
  PRIO_MEP_APS_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_aps_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP lm config",
  "MEP lm config [<inst>] [<prio>] [uni|multi] [single|dual] [10s|1s|6m|1m|6h] [<flr>] [enable|disable]",
  "MEP Loss Measurement configuration\n"
  "'prio' is the priority (PCP) of transmitted LM frame\n"
  "'uni|multi' is selecting uni-cast or multi-cast transmission of LM frame\n"
  "'single|dual' is selecting single-ended (LMM) or dual-ended (CCM) LM\n"
  "'10s|1s|6m|1m|6h' is the number of LM frame pr. second\n"
  "'flr' is the Frame Loss Ratio time interval",
  PRIO_MEP_LM_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_lm_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP dm config",
  "MEP dm config [<inst>] [<prio>] [uni|multi] [<mep>] [oneway|twoway] [std|prop] [rdtrp|flow] [<gap>] [<count>] [us|ns] [keep|reset] [d2ford1] [enable|disable]",
  "MEP Delay Measurement configuration\n"
  "'prio' is the priority (PCP) of transmitted DM frame\n"
  "'uni|multi' is selecting uni-cast or multi-cast transmission of DM frame\n"
  "'mep' is the peer MEP-ID of target MEP - only used if 'uni'\n"
  "'oneway|twoway' is selecting one-way (1DM) or two-way (DMM) DM\n"
  "'std|prop' is selecting standadized or proprietary DM. the latest is using off-standard follow-up message carrying the exact HW transmit timestamp\n"
  "'rdtrp|flow' is selecting round-trip or flow delay calculation. Round-trip is not using the far-end timestamps to calculate the far-end residence time\n"
  "'gap' Gap between transmitting 1DM/DMM PDU - in 10 ms.\n"
  "'count' number of frames used for average calculation on the latest 'count' frames received\n"
  "'us|ns' calculation results are shown in micro or nano seconds\n"
  "'keep|reset' the action in case of total delay counter overflow - either 'keep' all results or 'reset' all results\n"
  "'d2ford1' this is selecting to used two-way DMM for calculate one-way delay",
  PRIO_MEP_DM_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_dm_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);



cli_cmd_tab_entry (
  "MEP lt config",
  "MEP lt config [<inst>] [<prio>] [<mac_addr>] [<mep>] [<ttl>] [enable|disable]",
  "MEP Link Trace configuration\n"
  "'prio' is the priority (PCP) of transmitted LTM frame\n"
  "'mac_addr' is the unicast MAC of target MEP/MIP\n"
  "'mep' is the peer MEP-ID of target MEP - only used if 'mac_addr is 'all zero'\n"
  "'tll' is the TLL in the transmitted LTM",
  PRIO_MEP_LT_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_lt_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP lb config",
  "MEP lb config [<inst>] [set|clear] [<prio>] [uni|multi] [<mac_addr>] [<mep>] [<tosend>] [<size>] [<interval>] [enable|disable]",
  "MEP Loop Back configuration\n"
  "'set|clear' is set or clear of DEI of transmitted LBM frame\n"
  "'prio' is the priority (PCP) of transmitted LBM frame\n"
  "'uni|multi' is selecting uni-cast or multi-cast transmission of LBM frame\n"
  "'mac_addr' is the unicast MAC of target MEP/MIP\n"
  "'mep' is the peer MEP-ID of target MEP - only used if 'mac_addr is 'all zero'\n"
  "'tosend' is the number of LBM to send\n"
  "'size' is the size of the LBM data field\n"
  "'interval' is the interval between LBM",
  PRIO_MEP_LB_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_lb_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP tst config",
  "MEP tst config [<inst>] [set|clear] [<prio>] [<mep>] [no_seq|seq] [<rate>] [<size>] [allzero|allone|onezero] [enable|disable]",
  "MEP Test Signal configuration\n"
  "'set|clear' is set or clear of DEI of transmitted LBM frame\n"
  "'prio' is the priority (PCP) of transmitted TST frame\n"
  "'mep' is the peer MEP-ID of target MEP - only used if 'mac_addr is 'all zero'\n"
  "'no_seq|seq' is without and with transmitted sequence numbers\n"
  "'rate' is the TST frame transmission bit rate in Mbps'\n"
  "'size' is the size of the un-tagged TST frame - four bytes will be added for each tag\n"
  "'allzero|allone|onezero' is pattern contained in the TST frame data TLV",
  PRIO_MEP_TST_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_tst_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP ais config",
  "MEP ais config [<inst>] [<prio>] [1s|1m] [set|clear] [enable|disable]",
  "MEP AIS configuration\n"
  "'prio' is the priority (PCP) of transmitted AIS frame\n"
  "'1s|1m' is the number of AIS frame pr. second\n"
  "'set|clear' is set or clear of protection usability. If set, the first 3 AIS frames are transmitted as fast as possible - this gives protection reliability in the path end-point",
  PRIO_MEP_AIS_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_ais_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP lck config",
  "MEP lck config [<inst>] [<prio>] [1s|1m] [enable|disable]",
  "MEP LCK configuration\n"
  "'prio' is the priority (PCP) of transmitted AIS frame\n"
  "'1s|1m' is the number of AIS frame pr. second",
  PRIO_MEP_LCK_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_lck_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "MEP client config",
  "MEP client config [<inst>] [domport|domevc] [<level>] [<cflow>] [<cflow>] [<cflow>] [<cflow>] [<cflow>] [<cflow>] [<cflow>] [<cflow>] [<cflow>] [<cflow>]",
  "MEP Client configuration\n"
  "'domport|domevc' is the client domain - must be EVC\n"
  "'level' is the client MEG level - the contained level in the AIS and LCK frames\n"
  "'cflow' is the client flow instance - up to 10 possible client flows (EVC)",
  PRIO_MEP_CLIENT_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_client_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
 "MEP state",
 "MEP state [<inst>]",
 "MEP state get",
  PRIO_MEP_STATE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_state,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "MEP lm state",
  "MEP lm state [<inst>]",
  "MEP Loss Measurement state get",
  PRIO_MEP_LM_STATE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_lm_state,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
 "MEP lm clear",
 "MEP lm clear <inst>",
 "MEP Loss Measurement state clear",
 PRIO_MEP_LM_STATE_CLEAR,
 CLI_CMD_TYPE_CONF,
 VTSS_MODULE_ID_MEP,
 cli_cmd_id_mep_lm_state_clear,
 mep_cli_def_req,
 mep_cli_parm_table,
 CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "MEP lt state",
  "MEP lt state [<inst>]",
  "MEP Link Trace state get",
  PRIO_MEP_LT_STATE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_lt_state,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "MEP lb state",
  "MEP lb state [<inst>]",
  "MEP Loop Back state get",
  PRIO_MEP_LB_STATE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_lb_state,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "MEP dm state",
  "MEP dm state [<inst>]",
  "MEP Delay Measurement state get",
  PRIO_MEP_DM_STATE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_dm_state,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
 "MEP dm clear",
 "MEP dm clear <inst>",
 "MEP Delay Measurement state clear",
 PRIO_MEP_DM_STATE_CLEAR,
 CLI_CMD_TYPE_CONF,
 VTSS_MODULE_ID_MEP,
 cli_cmd_id_mep_dm_state_clear,
 mep_cli_def_req,
 mep_cli_parm_table,
 CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "MEP tst state",
  "MEP tst state [<inst>]",
  "MEP Test Signal state get\n"
  "RX rate is shown in Kbps",
  PRIO_MEP_TST_STATE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_MEP,
  cli_cmd_id_mep_tst_state,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
 "MEP tst clear",
 "MEP tst clear <inst>",
 "MEP Test Signal state clear",
 PRIO_MEP_TST_STATE_CLEAR,
 CLI_CMD_TYPE_CONF,
 VTSS_MODULE_ID_MEP,
 cli_cmd_id_mep_tst_state_clear,
 mep_cli_def_req,
 mep_cli_parm_table,
 CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug MEP Test Config",
  "Debug MEP Test Config <inst> [<vid>] <count> [<port>] [<mac_addr>] [enable|disable]",
  "MEP Test configuration that creates a 'count' of INGRESS Port MEP with 3.3ms CCM active - each on incrementing 'vid'",
  PRIO_DEBUG_MEP_TEST,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_id_mep_test_config,
  mep_cli_def_req,
  mep_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "Debug MEP DM State",
  NULL,
  "Show DM status",
  PRIO_DEBUG_MEP_DM_STATE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_id_debug_mep_dm_state,
  NULL,
  mep_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#if defined(VTSS_SW_OPTION_UP_MEP)
cli_cmd_tab_entry (
  "Debug MEP Up-MEP",
  "Debug MEP Up-MEP [enable|disable]",
  "Enable the DS1076 Up-MEP to be active\n"
  "Port 13-16 is customer UNI\n"
  "Port 17-24 is loop UNI\n"
  "This require host loop back on the loop UNI ports - this is done automatically\n"
  "This require 3 PVLAN groups: (1-16) + (1-12,17-20) + (1-12,21-24) - this is done automatically\n"
  "This require filtering disabled on loop UNI - this is done automatically\n"
  "This require learning disabled on loop UNI - this is done automatically\n"
  "This require loop ports and customer ports to have 'vlan port type' set to 'C-port' - this is done automatically\n"
  "This require customer ports to have 'vlan Tx Tag' set to 'Tag_all' - this is done automatically\n"
  "This is only active after a system re-boot",
  PRIO_DEBUG_MEP_UP_MEP,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  cli_cmd_id_debug_mep_up_mep,
  NULL,
  mep_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif

