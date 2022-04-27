/*

 Vitesse Switch Application software.

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
#include "erps.h"
#include "erps_cli.h"
#include "vtss_erps_api.h"
#include "vtss_module_id.h"

typedef struct erps_module_req {
    u32   group_id;
    u32   east_port;
    u32   west_port;
    u64   timeout;
    u16   rpl_owner;
    u32   rpl_port;
    u16   control_vid;
    u16   revertion;
    u32   east_mep_id;
    u32   west_mep_id;
    u32   raps_eastmep;
    u32   raps_westmep;
    u32   statistics;
    u16   clear;
    vtss_erps_ring_type_t ring_type;
    u32   major_ring_id;
    BOOL  erps_enable;
    vtss_erps_admin_cmd_t cmd;
    BOOL  propagate;
    vtss_erps_version_t   version;
    BOOL  interconnected;
    BOOL  virtual_channel;
} erps_module_req_t;

/*******************************************************************************
  Initialization                                                          
*******************************************************************************/
void erps_cli_req_init(void)
{
    /* register the size required for erps req. structure */
    cli_req_size_register(sizeof(erps_module_req_t));
}

/******************************************************************************
 Helper Functions
******************************************************************************/
void vtss_erps_print_error (vtss_rc error)
{
    switch(error)
    {
        case ERPS_ERROR_PG_CREATION_FAILED:
            CPRINTF("%% Protection group creation failed\n");
            break;
        case  ERPS_ERROR_GROUP_NOT_EXISTS:
            CPRINTF("%% Given protection group does not exist\n");
            break;
        case ERPS_ERROR_GROUP_ALREADY_EXISTS:
            CPRINTF("%% Given protection group already created\n");
            break;
        case ERPS_ERROR_INVALID_PGID:
            CPRINTF("%% Invalid protection group ID\n");
            break;
        case ERPS_ERROR_CANNOT_SET_RPL_OWNER_FOR_ACTIVE_GROUP:
            CPRINTF("%% RPL owner can not be set when a group is active\n");
            break;
        case ERPS_ERROR_NODE_ALREADY_RPL_OWNER:
            CPRINTF("%% This node is RPL owner for given protection group\n");
            break;
        case ERPS_ERROR_PG_IN_INACTIVE_STATE:
            CPRINTF("%% Protection group not in active state\n");
            break;
        case ERPS_ERROR_NODE_NON_RPL:
            CPRINTF("%% This node is not RPL for given protection group\n");
            break;
        case ERPS_ERROR_SETTING_RPL_BLOCK:
            CPRINTF("%% Failed setting RPL block\n");
            break;
        case ERPS_ERROR_VLANS_CANNOT_BE_ADDED:
            CPRINTF("%% VLANs can not be added for this protection group\n");
            break;
        case ERPS_ERROR_VLANS_CANNOT_BE_DELETED:
            CPRINTF("%% VLANs can not be deleted for this protection group\n");
            break;
        case ERPS_ERROR_CONTROL_VID_PART_OF_ANOTHER_GROUP:
            CPRINTF("%% Control VLAN ID is part of another group\n");
            break;
        case ERPS_ERROR_EAST_AND_WEST_ARE_SAME:
            CPRINTF("%% East and west ports are same\n");
            break;
        case ERPS_ERROR_CANNOT_ASSOCIATE_GROUP:
            CPRINTF("%% Group association failed\n");
            break;
        case ERPS_ERROR_ALREADY_NEIGHBOUR_FOR_THISGROUP:
            CPRINTF("%% Node is configured as neighbour for given group, can not set as RPL\n");
            break;
        case ERPS_ERROR_GROUP_IN_ACTIVE_MODE:
            CPRINTF("%% Cannot configure when group is in active mode\n");
            break;
        case ERPS_ERROR_GIVEN_PORT_NOT_EAST_OR_WEST:
            CPRINTF("%% Given port is not configured either east or west for this group\n");
            break;
        case ERPS_ERROR:
            CPRINTF("%% Unknown error occurred\n");
            break;
        case ERPS_ERROR_MAXIMUM_PVIDS_REACHED:
            CPRINTF("%% Maximum number of VLANs already configured for protection group\n");
            break;
        default:
            CPRINTF("%% Unexpected error code: %u\n", error);
            break;
    }
}

static void erps_print_protected_vlans(const vtss_vid_t pvids[])
{
    u8  count = 0, i = 0;

    CPRINTF("\n\n    Protected VLANS: \n");
    for ( i = 0 ; i < PROTECTED_VLANS_MAX; i++ ) {
        if (pvids[i]) {
            CPRINTF("    %d ", pvids[i] );
            if (!(++count%5))
                CPRINTF("\n");
        }
    }

    if (count) {
        CPRINTF("\n");
    }

    if ( !count)
        CPRINTF(" None");
}

static char *req_calc(u16 req)
{
    switch (req) {
        case 14: return("Event "); //Event
        case 13: return("FS "); //FS
        case 11: return("SF "); //SF
        case 07: return("MS "); //MS
        case 00: return("NR "); //NR
        default: return("NR ");
    }
}

static void erps_display_protection_group_info(const vtss_erps_mgmt_conf_t *conf_req)
{
    const vtss_erps_config_erpg_t * erpg = NULL;
    const vtss_erps_fsm_stat_t    * ferpg = NULL;

    erpg = &conf_req->data.get.erpg;
    ferpg = &conf_req->data.get.stats;

    CPRINTF("\n   %d       %d        %d      %s    %d    %s", conf_req->group_id,
		    erpg->west_port, erpg->east_port,
		    (erpg->rpl_owner ? "   RPL Owner   ":(erpg->rpl_neighbour ? " RPL Neighbour ":"    Non RPL    ")),
            ((erpg->rpl_owner || erpg->rpl_neighbour) ? erpg->rpl_owner_port : erpg->rpl_neighbour_port),
		    (erpg->rpl_owner || erpg->rpl_neighbour) ? ((ferpg->rpl_blocked == RAPS_RPL_BLOCKED) ? "        RPL Blocked":"    RPL Not Blocked") : " ");
    erps_print_protected_vlans(erpg->protected_vlans);
    CPRINTF("\n");
    CPRINTF("    Protection Group State             :%s\n", (ferpg->active == TRUE)?"Active":"Not Active" );
    CPRINTF("    Port 0 SF Mep                      :%d\n", conf_req->data.mep.east_mep_id);
    CPRINTF("    Port 1 SF Mep                      :%d\n", conf_req->data.mep.west_mep_id );
    CPRINTF("    Port 0 APS MEP                     :%d\n", conf_req->data.mep.raps_eastmep);
    CPRINTF("    Port 1 APS MEP                     :%d\n", conf_req->data.mep.raps_westmep);
//    if (erpg->rpl_owner || erpg->rpl_neighbour)
//        CPRINTF("    RPL Port                           :%d\n", (erpg->rpl_owner) ? erpg->rpl_owner_port : erpg->rpl_neighbour_port);
    CPRINTF("    WTR Timeout                        :%llu\n",erpg->wtr_time);
    CPRINTF("    WTB Timeout                        :%u\n",  erpg->wtb_time);
    CPRINTF("    Hold-Off Timeout                   :%llu\n",erpg->hold_off_time);
    CPRINTF("    Guard Timeout                      :%llu\n",erpg->guard_time);
    if (erpg->ring_type == ERPS_RING_TYPE_MAJOR) {
        if (erpg->inter_connected_node) {
            CPRINTF("    Node Type                          :Major-Interconnected\n");
        } else {
            CPRINTF("    Node Type                          :Major\n");
        }
    } 
    if (erpg->ring_type == ERPS_RING_TYPE_SUB) {
        if (erpg->inter_connected_node) {
            CPRINTF("    Node Type                          :Sub-Interconnected\n");
            CPRINTF("    Major Ring ID                      :%d\n",erpg->major_ring_id);
            CPRINTF("    Topology change propagation        :%s\n", (erpg->topology_change) ? "Enabled" : "Disabled");
        } else {
            CPRINTF("    Node Type                          :Sub\n");
        }
        CPRINTF("    Virtual Channel                    :%s\n", (erpg->virtual_channel) ? "With" : "Without");
    }
    CPRINTF("    Reversion                          :%s\n", ((erpg->revertive) ?"Revertive":"Non-Revertive"));
    CPRINTF("    Version                            :%s\n",((erpg->version == ERPS_VERSION_V1)?"ERPS-V1 compatible":"ERPS-V2 compatible"));
    CPRINTF("    ERPSv2 Admin Command               :%s\n",(ferpg->admin_cmd == 2)?"Forced Switch":((ferpg->admin_cmd == 1)?"Manual Switch":"None"));

    if (ferpg->state == ERPS_STATE_IDLE ) {
        CPRINTF("\n    FSM State                          :IDLE\n");
    } else if (ferpg->state == ERPS_STATE_PROTECTED ) {
        CPRINTF("\n    FSM State                          :PROTECTED\n");
    } else if (ferpg->state == ERPS_STATE_MANUAL_SWITCH) {
        CPRINTF("\n    FSM State                          :MANUAL_SWITCH\n");
    } else if (ferpg->state == ERPS_STATE_FORCED_SWITCH) {
        CPRINTF("\n    FSM State                          :FORCED_SWITCH\n");
    } else if (ferpg->state == ERPS_STATE_PENDING) {
        CPRINTF("\n    FSM State                          :PENDING\n");
    } else {
        CPRINTF("\n    FSM State                          :Unknown\n");
    }
    if (ferpg->wtr_remaining_time) {
        CPRINTF("    WTR Timeout Remaining              :%llu\n", ferpg->wtr_remaining_time);
    }
    CPRINTF("    Port 0 Link Status                 :%s\n",((ferpg->east_port_state == ERPS_PORT_STATE_OK) ?"Link Up":"Link Down"));
    CPRINTF("    Port 1 Link Status                 :%s\n",((ferpg->west_port_state == ERPS_PORT_STATE_OK) ?"Link Up":"Link Down"));

    CPRINTF("    Port 0 Block Status                :%s\n",((ferpg->east_blocked) ?"BLOCKED":"UNBLOCKED"));
    CPRINTF("    Port 1 Block Status                :%s\n",((ferpg->west_blocked) ?"BLOCKED":"UNBLOCKED"));

    if (!ferpg->tx) {
        CPRINTF("    R-APS Transmission                 :STOPPED\n");
    } else
        CPRINTF("    R-APS Transmission                 :%s%s%s%s\n", req_calc(ferpg->tx_req), ((ferpg->tx_rb) ? "RB " : ""), ((ferpg->tx_dnf) ? "DNF " : ""), ((ferpg->tx_bpr) ? "BPR 1 " : "BPR 0 "));

    if (!ferpg->rx[0]) {
        CPRINTF("    R-APS Port 0 Reception             :NONE\n");
    } else
        CPRINTF("    R-APS Port 0 Reception             :%s%s%s%s%02X-%02X-%02X-%02X-%02X-%02X\n", req_calc(ferpg->rx_req[0]), ((ferpg->rx_rb[0]) ? "RB " : ""), ((ferpg->rx_dnf[0]) ? "DNF " : ""), ((ferpg->rx_bpr[0]) ? "BPR 1 " : "BPR 0 "), ferpg->rx_node_id[0][0], ferpg->rx_node_id[0][1], ferpg->rx_node_id[0][2], ferpg->rx_node_id[0][3], ferpg->rx_node_id[0][4], ferpg->rx_node_id[0][5]);

    if (!ferpg->rx[1]) {
        CPRINTF("    R-APS Port 1 Reception             :NONE\n");
    } else
        CPRINTF("    R-APS Port 1 Reception             :%s%s%s%s%02X-%02X-%02X-%02X-%02X-%02X\n", req_calc(ferpg->rx_req[1]), ((ferpg->rx_rb[1]) ? "RB " : ""), ((ferpg->rx_dnf[1]) ? "DNF " : ""), ((ferpg->rx_bpr[1]) ? "BPR 1 " : "BPR 0 "), ferpg->rx_node_id[1][0], ferpg->rx_node_id[1][1], ferpg->rx_node_id[1][2], ferpg->rx_node_id[1][3], ferpg->rx_node_id[1][4], ferpg->rx_node_id[1][5]);

    CPRINTF("    FOP Alarm                          :%s\n", (ferpg->fop_alarm) ? "ON":"OFF");
}


/*******************************************************************************
  Command Functions
*******************************************************************************/
static void cli_cmd_erps_create_protection_group (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t     *erps_req = NULL;
    vtss_rc               ret = VTSS_RC_ERROR;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type              = ERPS_CMD_PROTECTION_GROUP_ADD;
    conf_req.group_id              = erps_req->group_id;
    conf_req.data.create.east_port = erps_req->east_port;
    conf_req.data.create.west_port = erps_req->west_port;
    conf_req.data.create.ring_type = erps_req->ring_type;
    conf_req.data.create.interconnected = erps_req->interconnected;
    conf_req.data.create.major_ring_id = erps_req->major_ring_id;
    conf_req.data.create.virtual_channel = erps_req->virtual_channel;
    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}
static void cli_cmd_erps_create_revertion (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret = VTSS_RC_ERROR;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    if (erps_req->revertion)
    {
        conf_req.req_type = ERPS_CMD_DISABLE_NON_REVERTIVE;
    } else {
        conf_req.req_type = ERPS_CMD_ENABLE_NON_REVERTIVE;
    }

    conf_req.group_id = erps_req->group_id;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_create_admin_cmd (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret = VTSS_RC_ERROR;

    erps_req = req->module_req;  
    memset(&conf_req,0,sizeof(conf_req));
                                     
    conf_req.req_type = erps_req->cmd;
    conf_req.group_id = erps_req->group_id;
    conf_req.data.create.east_port = req->uport;
   
    ret = erps_mgmt_set_protection_group_request(&conf_req);
           
    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }      
}

static void cli_cmd_erps_topology_change_prop_cmd (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret = VTSS_RC_ERROR;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type = erps_req->cmd;

    if (erps_req->propagate)
        conf_req.req_type = ERPS_CMD_TOPOLOGY_CHANGE_PROPAGATE;
    else
        conf_req.req_type = ERPS_CMD_TOPOLOGY_CHANGE_NO_PROPAGATE;
        
    conf_req.group_id = erps_req->group_id;
        
    ret = erps_mgmt_set_protection_group_request(&conf_req);
        
    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }      
}

static void cli_cmd_erps_version_cmd (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret = VTSS_RC_ERROR;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type = erps_req->version;
    conf_req.group_id = erps_req->group_id;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_add_protection_group_vlans (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t     *erps_req = NULL;
    vtss_rc               ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type           = ERPS_CMD_ADD_VLANS;
    conf_req.group_id           = erps_req->group_id;
    conf_req.data.vid.num_vids  = 1;
    conf_req.data.vid.p_vid     = req->vid;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_del_protection_group_vlans (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t     *erps_req = NULL;
    vtss_rc               ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type           = ERPS_CMD_DEL_VLANS;
    conf_req.group_id           = erps_req->group_id;
    conf_req.data.vid.num_vids  = 1;
    conf_req.data.vid.p_vid     = req->vid;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    } 
}

static void cli_cmd_erps_associate_mep (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t     *erps_req = NULL;
    vtss_rc               ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type              = ERPS_CMD_ADD_MEP_ASSOCIATION;
    conf_req.group_id              = erps_req->group_id;
    conf_req.data.mep.east_mep_id  = erps_req->east_mep_id;
    conf_req.data.mep.west_mep_id  = erps_req->west_mep_id;
    conf_req.data.mep.raps_eastmep = erps_req->raps_eastmep;
    conf_req.data.mep.raps_westmep = erps_req->raps_westmep;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_select_rpl_block (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t     *erps_req = NULL;
    vtss_rc               ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type              = ERPS_CMD_SET_RPL_BLOCK;
    conf_req.group_id              = erps_req->group_id;
    conf_req.data.rpl_block.rpl_port  = erps_req->rpl_port;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

#ifdef ERPS_REPLACE_RPL_OPTION
static void cli_cmd_erps_replace_rpl_block (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type                  = ERPS_CMD_REPLACE_RPL_BLOCK;
    conf_req.group_id                  = erps_req->group_id;
    conf_req.data.rpl_block.rpl_port   = erps_req->rpl_port;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}
#endif
static void cli_cmd_erps_deselect_rpl_block (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t     *erps_req = NULL;
    vtss_rc               ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type              = ERPS_CMD_UNSET_RPL_BLOCK;
    conf_req.group_id              = erps_req->group_id;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_select_rpl_neighbour (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type = ERPS_CMD_SET_RPL_NEIGHBOUR;
    conf_req.group_id = erps_req->group_id;
    conf_req.data.rpl_block.rpl_port  = erps_req->rpl_port;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_unselect_rpl_neighbour(cli_req_t * req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type = ERPS_CMD_UNSET_RPL_NEIGHBOUR;
    conf_req.group_id = erps_req->group_id;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }

}

static void cli_cmd_erps_holdoff_timeout (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type        = ERPS_CMD_HOLD_OFF_TIMER;
    conf_req.group_id        = erps_req->group_id;
    conf_req.data.timer.time = erps_req->timeout;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_guard_timeout (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type         = ERPS_CMD_GUARD_TIMER;
    conf_req.group_id         = erps_req->group_id;
    conf_req.data.timer.time  = erps_req->timeout;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_wtr_timeout (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type         = ERPS_CMD_WTR_TIMER;
    conf_req.group_id         = erps_req->group_id;
    conf_req.data.timer.time  = MIN_TO_MS(erps_req->timeout);

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    } 
}

static void cli_cmd_erps_delete_protection_group (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    vtss_rc                    ret;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    conf_req.req_type = ERPS_CMD_PROTECTION_GROUP_DELETE;
    conf_req.group_id = erps_req->group_id;

    ret = erps_mgmt_set_protection_group_request(&conf_req);

    if ( ret != VTSS_RC_OK ) {
        vtss_erps_print_error(ret);
    }
}

static void cli_cmd_erps_dispay_configuration (cli_req_t *req)
{
    vtss_erps_mgmt_conf_t      conf_req;
    erps_module_req_t          *erps_req = NULL;
    u32                        instance = 0;

    erps_req = req->module_req;
    memset(&conf_req,0,sizeof(conf_req));

    if (erps_req->statistics) {
        conf_req.req_type = ERPS_CMD_SHOW_STATISTICS;
    } else if (erps_req->clear) {
        conf_req.req_type = ERPS_CMD_CLEAR_STATISTICS;
    } else {
        conf_req.req_type = ERPS_CMD_SHOW_PROTECTION_GROUPS;
    }

    if (conf_req.req_type == ERPS_CMD_SHOW_STATISTICS) {
        conf_req.group_id = erps_req->group_id-1;
        if (erps_mgmt_getnext_protection_group_request(&conf_req) == VTSS_RC_OK) {
            if (conf_req.group_id != erps_req->group_id) {
                return;
            }
            CPRINTF("\n");
            CPRINTF("    RAPS PDU's Received:%16llu", conf_req.data.get.raps_stats.raps_rcvd);
            CPRINTF("\n");
            CPRINTF("    RAPS PDU's dropped:%17llu", conf_req.data.get.raps_stats.raps_rx_dropped);
            CPRINTF("\n");
            CPRINTF("    local SF Occurred:%19llu", conf_req.data.get.raps_stats.local_sf);
            CPRINTF("\n");
            CPRINTF("    local SF cleared:%19llu", conf_req.data.get.raps_stats.local_sf_cleared);
            CPRINTF("\n");
            CPRINTF("    remote SF received:%17llu", conf_req.data.get.raps_stats.remote_sf);
            CPRINTF("\n");
            CPRINTF("    remote FS received:%17llu", conf_req.data.get.raps_stats.remote_fs);
            CPRINTF("\n");
            CPRINTF("    NR Messages sent:%19llu", conf_req.data.get.raps_stats.event_nr);
            CPRINTF("\n");
        }
        return;
    } else if (conf_req.req_type == ERPS_CMD_SHOW_PROTECTION_GROUPS ) {
        if (!erps_req->group_id) {

            CPRINTF("\n ERPS ID  Port_1   Port_0      Node Role     RPL Port      RPL Block");
            CPRINTF("\n");

            /* display all protection groups information */
            while (erps_mgmt_getnext_protection_group_request (&conf_req) == VTSS_RC_OK) {
                erps_display_protection_group_info(&conf_req);
                instance = conf_req.group_id;
                memset(&conf_req,0,sizeof(conf_req));
                conf_req.group_id = instance;
                if (instance > ERPS_MAX_PROTECTION_GROUPS ) {
                    return;
                }
            }
        } else {
            /* get exact protection group */ 
           CPRINTF("\n ERPS ID  Port_1   Port_0      Node Role         RPL Block         RPL Port");
           CPRINTF("\n");

           conf_req.group_id = CONV_MGMTTOERPS_INSTANCE(erps_req->group_id);
           if (erps_mgmt_getexact_protection_group_by_id (&conf_req) == VTSS_RC_OK) {
               erps_display_protection_group_info(&conf_req);
           }
        }
        return;
    }
}

/*******************************************************************************
  Parameter Functions
*******************************************************************************/
static int32_t cli_parse_west_port (char *cmd, char *cmd2, char *stx, 
                                char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    ulong min = 0, max = VTSS_PORTS;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, min, max);
    erps_req->west_port = value;

    return(error);
}

static int32_t cli_parse_east_port (char *cmd, char *cmd2, char *stx, 
                                char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong min = 1, max = VTSS_PORTS;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, min, max);
    erps_req->east_port = value;
    return(error);
}

static int32_t cli_parse_rpl_port (char *cmd, char *cmd2, char *stx,
                                 char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong min = 1, max = VTSS_PORTS;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, min, max);
    erps_req->rpl_port = value;
    return(error);
}

static int32_t cli_parse_group_id  (char *cmd, char *cmd2, char *stx, 
                                char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, ERPS_MIN_PROTECTION_GROUPS, 
                                         ERPS_MAX_PROTECTION_GROUPS);
    if (!error) {
        erps_req->group_id = value;
    }

    return(error);
}

static int32_t cli_parse_major_ring_id  (char *cmd, char *cmd2, char *stx, 
                                         char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    if (!((erps_req->ring_type == ERPS_RING_TYPE_SUB) && (erps_req->interconnected))) {
        erps_req->major_ring_id = 0;
         return(error);
    }
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, ERPS_MIN_PROTECTION_GROUPS, 
                                         ERPS_MAX_PROTECTION_GROUPS);
    if (!error) {
        erps_req->major_ring_id = value;
    }

    return(error);
}
static int32_t cli_parse_erps_reversion (char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    erps_module_req_t  * erps_req = NULL;
    char *found = cli_parse_find(cmd, stx);

    erps_req = req->module_req;

    req->parm_parsed = 1;

    if(!found)      return 1;
    if(!strncmp(found, "revertive", 9))
        erps_req->revertion = 1;
    else if(!strncmp(found, "nonrevertive", 12))
        erps_req->revertion = 0;
    else return 1;

    return 0;
}

static int32_t cli_parse_erps_admin_command (char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    erps_module_req_t  * erps_req = NULL;
    char *found = cli_parse_find(cmd, stx);

    erps_req = req->module_req;

    req->parm_parsed = 1;

    if(!found)      return 1;
    if(!strncmp(found, "fs", 2)) {
        erps_req->cmd = ERPS_CMD_FORCED_SWITCH;
    }
    else if(!strncmp(found, "ms", 2)) {
        erps_req->cmd = ERPS_CMD_MANUAL_SWITCH;
    }
    else if(!strncmp(found, "clear", 5)) {
        erps_req->cmd = ERPS_CMD_CLEAR;
    }
    else return 1;

    return 0;
}

static int32_t cli_parse_erps_topology_change_propagation (char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    erps_module_req_t  * erps_req = NULL;
    char *found = cli_parse_find(cmd, stx);

    erps_req = req->module_req;

    req->parm_parsed = 1;

    if(!found)      return 1;
    if(!strncmp(found, "propagate", 9))
        erps_req->propagate = 1;
    else if(!strncmp(found, "nopropagate", 11))
        erps_req->propagate = 0;
    else return 1;

    return 0;
}

static int32_t cli_parse_erps_version (char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    erps_module_req_t  * erps_req = NULL;
    char *found = cli_parse_find(cmd, stx);

    erps_req = req->module_req;

    req->parm_parsed = 1;

    if(!found)      return 1;
    if(!strncmp(found, "v1", 2))
        erps_req->version = ERPS_CMD_ENABLE_VERSION_1_COMPATIBLE;
    else if(!strncmp(found, "v2", 2))
        erps_req->version = ERPS_CMD_DISABLE_VERSION_1_COMPATIBLE;
    else return 1;

    return 0;

}

static int32_t cli_parse_east_mepid    (char *cmd, char *cmd2, char *stx, 
                                    char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 1, VTSS_MEP_INSTANCE_MAX);
    if (!error) {
        erps_req->east_mep_id = value;
        return (VTSS_RC_OK);
    }

    return (VTSS_RC_ERROR);
}

static int32_t cli_parse_west_mepid    (char *cmd, char *cmd2, char *stx,
                                    char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 0, VTSS_MEP_INSTANCE_MAX);
    if (!error) {
        erps_req->west_mep_id = value;
        return (VTSS_RC_OK);
    }

    return (VTSS_RC_ERROR);
}

static int32_t cli_parse_raps_west_mepid (char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 0, VTSS_MEP_INSTANCE_MAX);
    if (!error) {
        erps_req->raps_westmep = value;
        return (VTSS_RC_OK);
    }

    return (VTSS_RC_ERROR);
}

static int32_t cli_parse_raps_east_mepid (char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, 1, VTSS_MEP_INSTANCE_MAX);
    if (!error) {
        erps_req->raps_eastmep = value;
        return (VTSS_RC_OK);
    }

    return (VTSS_RC_ERROR);
}

static int32_t cli_parse_guard_timeout (char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, RAPS_GUARD_TIMEOUT_MIN_MILLISECONDS,
                                         SECS_TO_MS(RAPS_GUARD_TIMEOUT_MAX_SECONDS));
    if (!error) {
        /* guard timeout should be in the increments of 10ms */
        if (value%10 == 0) {
            erps_req->timeout = value;
            return (VTSS_RC_OK);
        } else {
            CPRINTF("\n guard timeout should be configured in multiples of 10 ms \n");
        }
    }

    return (VTSS_RC_ERROR);
}

static int32_t cli_parse_wtrtimeout   (char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, RAPS_WTR_TIMEOUT_MIN_MINUTES,
                                         RAPS_WTR_TIMEOUT_MAX_MINUTES);
    if (!error) {
        erps_req->timeout = value;
        return (VTSS_RC_OK);
    }

    return (VTSS_RC_ERROR);
}

static int32_t cli_parse_holdtimeout   (char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, SECS_TO_MS(RAPS_HOLD_OFF_TIMEOUT_MIN_SECONDS),
                                         SECS_TO_MS(RAPS_HOLD_OFF_TIMEOUT_MAX_SECONDS));
    if (!error) {
        /* hold-off timeout should be in the increments of 100ms */
        if (value%100 == 0) {
            erps_req->timeout = value;
            return (VTSS_RC_OK);
        } else {
            CPRINTF("\n hold off should be configured in multiples of 100 ms \n");
        }
    }

    return (VTSS_RC_ERROR);
}

static int32_t cli_parse_ring_type (char *cmd, char *cmd2, char *stx,
                                    char *cmd_org, cli_req_t *req)
{
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;

    char *found = cli_parse_find(cmd, stx);

    erps_req = req->module_req;

    if(!found)      return 1;
    if(!strncmp(found, "major", 5))       
        erps_req->ring_type = ERPS_RING_TYPE_MAJOR;
    else if(!strncmp(found, "sub", 3))      
        erps_req->ring_type = ERPS_RING_TYPE_SUB;
    else return 1;

    return 0;
}

static int32_t cli_parse_interconnected (char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;

    char *found = cli_parse_find(cmd, stx);
    if(!found)      return 1;
    if(!strncmp(found, "interconnected", 14)) {
        erps_req->interconnected = 1;
    }
    return 0;
}

static int32_t cli_parse_virtual_channel (char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    erps_module_req_t  *erps_req = NULL;

    erps_req = req->module_req;

    char *found = cli_parse_find(cmd, stx);
    if(!found)      return 1;
    if(!strncmp(found, "virtual_channel", 15)) {
        erps_req->virtual_channel = 1;
    }
    return 0;
}

static int32_t cli_parse_erps_statistics (char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    erps_module_req_t *erps_req = req->module_req;
    char          *found = cli_parse_find(cmd, stx);

    T_I("ALL %s",found);

    req->parm_parsed = 1;

    if (found != NULL) {
        if (!strncmp(found, "statistics",10 )) {
            erps_req->statistics = 1;
        } else if (!strncmp(found, "clear",5 )) {
            erps_req->clear = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

/*******************************************************************************
 Parameter Table
*******************************************************************************/
static cli_parm_t erps_cli_parm_table[] = {

    {
        "<west_port>",
        "Port 1 of a protection group",
        CLI_PARM_FLAG_SET,
        cli_parse_west_port,
        cli_cmd_erps_create_protection_group,
    },
    {
        "<east_port>",
        "Port 0 of a protection group",
        CLI_PARM_FLAG_SET,
        cli_parse_east_port,
        cli_cmd_erps_create_protection_group,
    },
    {
        "<group-id>",
        "protection group id 1 - 64",
        CLI_PARM_FLAG_SET,
        cli_parse_group_id,
        NULL,
    },
    {
        "<east_sf_mep>",
        "SF mep id for Port 0",
        CLI_PARM_FLAG_SET,
        cli_parse_east_mepid,
        cli_cmd_erps_associate_mep,
    },
    {
        "<west_sf_mep>",
        "SF mep id for Port 1",
        CLI_PARM_FLAG_SET,
        cli_parse_west_mepid,
        cli_cmd_erps_associate_mep,
    },
    {
        "<east_raps_mep>",
        "CC/RAPS mep id for Port 0",
        CLI_PARM_FLAG_SET,
        cli_parse_raps_east_mepid,
        cli_cmd_erps_associate_mep,
    },
    {
        "<west_raps_mep>",
        "CC/RAPS mep id for Port 1",
        CLI_PARM_FLAG_SET,
        cli_parse_raps_west_mepid,
        cli_cmd_erps_associate_mep,
    },
    {
        "<rpl_port>",
        "RPL Block ",
        CLI_PARM_FLAG_SET,
        cli_parse_rpl_port,
        NULL,
    },
    {
        "<guard_timeout>",
        "timer timeout values ",
        CLI_PARM_FLAG_SET,
        cli_parse_guard_timeout,
        cli_cmd_erps_guard_timeout,
    },
    {
        "<wtr_timeout>",
        "timer timeout values ",
        CLI_PARM_FLAG_SET,
        cli_parse_wtrtimeout,
        cli_cmd_erps_wtr_timeout,
    },
    {
        "<hold_timeout>",
        "timer timeout values ",
        CLI_PARM_FLAG_SET,
        cli_parse_holdtimeout,
        cli_cmd_erps_holdoff_timeout,
    },
    {
        "statistics|clear",
        "ERPS statistics ",
        CLI_PARM_FLAG_SET,
        cli_parse_erps_statistics,
        cli_cmd_erps_dispay_configuration,
    },
    {
        "major|sub",
        "ring type",
        CLI_PARM_FLAG_SET,
        cli_parse_ring_type,
        cli_cmd_erps_create_protection_group,
    },
    {
        "interconnected",
        "Set for interconnected node",
        CLI_PARM_FLAG_SET,
        cli_parse_interconnected,
        cli_cmd_erps_create_protection_group
    },
    {
        "virtual_channel",
        "Set for virtual channel",
        CLI_PARM_FLAG_SET,
        cli_parse_virtual_channel,
        cli_cmd_erps_create_protection_group
    },
    {
        "<major-ring-id>",
        "major ring of a sub-ring, when configuring as an interconnected node ",
        CLI_PARM_FLAG_SET,
        cli_parse_major_ring_id,
        cli_cmd_erps_create_protection_group,
    },
    {
        "revertive|nonrevertive",
        "specifying reversion parameters\n",
        CLI_PARM_FLAG_SET,
        cli_parse_erps_reversion,
        cli_cmd_erps_create_revertion,
    },
    {
        "fs|ms|clear",
        "administrative commands \n",
        CLI_PARM_FLAG_SET,
        cli_parse_erps_admin_command,
        cli_cmd_erps_create_admin_cmd,
    },
    {
        "propagate|nopropagate",
        "topology change propagation configuration \n",
        CLI_PARM_FLAG_SET,
        cli_parse_erps_topology_change_propagation,
        cli_cmd_erps_topology_change_prop_cmd, 
    },
    {
        "v1|v2",
        "ERPS protocol version to be supported \n",
        CLI_PARM_FLAG_SET,
        cli_parse_erps_version,
        cli_cmd_erps_version_cmd, 
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

/*******************************************************************************
  Command Table
*******************************************************************************/
/* ERPS CLI Command Sorting Order */
enum {
    CLI_CMD_ERPS_CONF_PRIO = 0,
    CLI_CMD_CREATE_ADMIN_COMMAND_PRIO,
    CLI_CMD_CREATE_PROTECTION_GROUP_PRIO,
    CLI_CMD_CREATE_INTERCONNECTED_NODE_PRIO,
    CLI_CMD_CREATE_REVERTIVE_PRIO,
    CLI_CMD_ADD_VLAN_PRIO,
    CLI_CMD_DEL_VLAN_PRIO,
    CLI_CMD_ASSOCIATE_MEP_PRIO,
    CLI_CMD_RPL_BLOCK_PRIO,
    CLI_CMD_REPLACE_RPL_BLOCK_PRIO,
    CLI_CMD_RPL_UNBLOCK_PRIO,
    CLI_CMD_SET_HOLDOFF_TIMEOUT_PRIO,
    CLI_CMD_SET_GUARD_TIMEOUT_PRIO,
    CLI_CMD_SET_WTR_TIMEOUT_PRIO,
    CLI_CMD_DELETE_PROTECTION_GROUP_PRIO,
    CLI_CMD_CREATE_TC_COMMAND_PRIO,
    CLI_CMD_ERPS_CONFIGURATION_PRIO,
    CLI_CMD_ERPS_UNSET_CONFIGURATION_PRIO
};

cli_cmd_tab_entry(
    "NULL",
    "Erps add <group-id> <east_port> <west_port> [major|sub] [interconnected] [virtual_channel] [<major-ring-id>]",
    "create a new ethernet ring protection group \n"
    "<group-id>  : protection group id \n"
    "<east_port> : protection group Port 0 \n"
    "<west_port> : protection group Port 1, Port 1 can be 0 for sub-rings \n"
    "[major|sub] : ring type i.e major-ring or sub-ring\n"
    "[interconnected]  : interconnection node or not\n"
    "[[virtual_channel] : Virtual channel present or not\n"
    "[<major-ring-id>] : major ring group Id for interconnected sub-ring\n",
    CLI_CMD_CREATE_PROTECTION_GROUP_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_create_protection_group,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    "NULL",
    "Erps reversion [revertive|nonrevertive] <group-id>",
    "configuring reversion characteristics for a given node \n"
    "[revertive|nonrevertive] : enabling or disabling reversion for a given group \n"
    "<group_id>               : protection group id \n",
    CLI_CMD_CREATE_REVERTIVE_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_create_revertion,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps command [fs|ms|clear] <port> <group-id>",
    "invoking an administrative command for a given protection group \n"
    "[fs|ms|clear]  : setting or clearing an administrative command for a given group \n"
    "<port>         : forced a block on the ring port where this command is issued \n"
    "<group_id>     : protection group id \n",
    CLI_CMD_CREATE_ADMIN_COMMAND_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_create_admin_cmd,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps topologychange [propagate|nopropagate] <group-id>",
    "specifying topology change propagation parameters for a given protection group \n"
    "[propagate|nopropagate]  : enabling or disabling topology change propagation for a given group \n"
    "<group_id>               : protection group id \n",
    CLI_CMD_CREATE_TC_COMMAND_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_topology_change_prop_cmd,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps version [v1|v2] <group-id>",
    "specifying protocol version for a given protection group \n"
    "[v1|v2]     : specifying protocol version for a given protection group \n"
    "<group_id>  : protection group id \n",
    CLI_CMD_CREATE_ADMIN_COMMAND_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_version_cmd,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry(
    "NULL",
    "Erps vlan add <vid> <group-id>",
    "associating a given vlan to a protection group \n"
    "<vid>       : vlan to be protected \n"
    "<group-id>  : protection group-id for which vid belongs to",
    CLI_CMD_ADD_VLAN_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_add_protection_group_vlans,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps vlan delete <vid> <group-id>",
    "disassociating a given vlan to a protection group \n"
    "<vid>       : protected vlan to be deleted \n"
    "<group-id>  : protection group-id for which vid belongs to",
    CLI_CMD_DEL_VLAN_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_del_protection_group_vlans,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps mep <east_sf_mep> <west_sf_mep> <east_raps_mep> <west_raps_mep> <group-id>",
    "associating Port 0/1 MEP to a protection group \n"
    " <east_sf_mep>        : Mep_ID for finding out Continuity Check errors on Port 0 \n"
    " <west_sf_mep>        : Mep_ID for finding out Continuity Check errors on Port 1 \n"
    " <east_raps_mep>      : Mep_ID for transmitting R-APS frames on Port 0 \n"
    " <west_raps_mep>      : Mep_ID for transmitting R-APS frames on Port 1 \n"
    " <group_id>          : protection group id for which mep is associating",
    CLI_CMD_ASSOCIATE_MEP_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_associate_mep,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps rpl owner <rpl_port> <group-id>",
    "selection of RPL Block for a protection group \n"
    "by default this node is considered as RPL Owner \n"
    "(east|west)   : select east(Port 0) or west(Port 1) as RPL Block \n"
    "<group-id>    : protection group id for selecting RPL Block",
    CLI_CMD_RPL_BLOCK_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_select_rpl_block,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps rpl owner clear <group-id>",
    "making a node as Non-RPL Block for a protection group \n"
    "After clear, this node is nore an rpl owner for the given group \n"
    "(east|west)   : selected east(Port 0) or west(Port 1) as RPL Block \n"
    "<group-id>    : protection group id for selecting RPL Block",
    CLI_CMD_RPL_UNBLOCK_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_deselect_rpl_block,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#ifdef ERPS_REPLACE_RPL_OPTION
cli_cmd_tab_entry(
    "NULL",
    "Erps rpl owner replace <rpl_port> <group-id>",
    "replacing of RPL Block for a protection group \n"
    "(east|west)   : selected east(Port 0) or west(Port 1) as RPL Block \n"
    "<group-id>    : protection group id for selecting RPL Block",
    CLI_CMD_REPLACE_RPL_BLOCK_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_replace_rpl_block,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif
cli_cmd_tab_entry(
    "NULL",
    "Erps rpl neighbour <rpl_port> <group-id>",
    "selection of RPL neighbour for a protection group \n"
    "(east|west)   : selected east(Port 0) or west(Port 1) as RPL neighbour \n"
    "<group-id>    : protection group id for selecting RPL Block",
    CLI_CMD_RPL_BLOCK_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_select_rpl_neighbour,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps rpl neighbour clear <group-id>",
    "make this node as non-neighbour for a protection group \n"
    "<group-id>    : protection group id for selecting RPL Block",
    CLI_CMD_RPL_UNBLOCK_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_unselect_rpl_neighbour,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry(
    "NULL",
    "Erps hold off timeout <hold_timeout> <group-id>",
    "configuring hold off timeout for a protection group \n"
    "in milliseconds 0-10000 in the increments of 100ms \n"
    "<hold_timeout>    : hold-off timeout \n"
    "<group-id>        : protection group id for configuring hold-off time",
    CLI_CMD_SET_HOLDOFF_TIMEOUT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_holdoff_timeout,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps guard-timeout <guard_timeout> <group-id>",
    "configuring guard timeout for a protection group \n"
    "guard timeout should be configured in the increments of 10 milliseconds \n"
    "minimum guard timeout 10ms and maximum 2 seconds \n"
    "<guard_timeout>    : guard timeout \n"
    "<group-id>   : protection group id for configuring guard time",
    CLI_CMD_SET_GUARD_TIMEOUT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_guard_timeout,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps wtr-timeout <wtr_timeout> <group-id>",
    "configuring wait to restore timeout for a protection group \n"
    "in minutes in the range of 1 to 12 minutes \n"
    "<wtr_timeout>    : configuring wtr timeout \n"
    "<group-id>   : protection group id for configuring wtr time",
    CLI_CMD_SET_WTR_TIMEOUT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_wtr_timeout,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "NULL",
    "Erps delete <group-id>",
    "deletion of a protection group \n"
    "<group-id>   : protection group id for deletion ",
    CLI_CMD_DELETE_PROTECTION_GROUP_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_delete_protection_group,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Erps configuration [<group-id>] [statistics]",
    "Erps configuration [<group-id>] [statistics|clear]",
    "deletion of a protection group \n"
    "<group-id>     : protection group id \n"
    "[statistics]   : for displaying R-APS statistics \n"
    "[clear]        : for clearing R-APS statistics",
    CLI_CMD_ERPS_CONFIGURATION_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ERPS,
    cli_cmd_erps_dispay_configuration,
    NULL,
    erps_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
