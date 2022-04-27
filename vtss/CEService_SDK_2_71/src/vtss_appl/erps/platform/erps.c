/*

 Vitesse Switch Application software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "conf_api.h"
#include "critd_api.h"
#include "vtss_l2_api.h"
#include "interrupt_api.h"
#include "erps.h"
#include "vtss_erps_api.h"
#include "vlan_api.h" 

#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#include "msg_api.h"
#include "topo_api.h"
#include "icli_porting_util.h"
#endif

#ifdef VTSS_SW_OPTION_MEP
#include "mep_api.h"
#endif

#ifdef VTSS_SW_OPTION_VCLI
#include "erps_cli.h"
#endif


/******************************************************************************
                                       #defines
******************************************************************************/
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_ERPS
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_ERPS

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_MGMT         2
#define TRACE_GRP_CTRL         3
#define TRACE_GRP_ERPS_RX      4
#define TRACE_GRP_ERPS_TX      5
#define TRACE_GRP_SWAPI        6
#define TRACE_GRP_STATE_CHANGE 7
#define TRACE_GRP_CNT          8


/*******************************************************************************
  Global variables                                                        
*******************************************************************************/
/*
 * erps_mep_t contains mapping of port to mep_ids, mep_ids are not exposed to
 * base thus will be converted to mep_id to/from erps base
 * 
 * port[0] contains east port and port[1] contains west port
 * ccm_mep_id[0] contains ccm east_mepid and ccm_mep_id[1] contains ccm west_mepid
 * aps_mep_id[0] contains raps east_mepid and aps_mep_id[1] contains raps west_mepid
 */
typedef struct erps_mep
{
#define ERPS_ARRAY_OFFSET_EAST 0
#define ERPS_ARRAY_OFFSET_WEST 1

    vtss_port_no_t  port[2];
    u32             ccm_mep_id[2];
    u32             raps_mep_id[2];
    BOOL            active;
}erps_mep_t;

erps_mep_t   port_to_mep[ERPS_MAX_PROTECTION_GROUPS+1];

/* ERPS thread resources */
static cyg_handle_t        run_thread_handle;
static cyg_thread          run_thread_block;
static char                run_thread_stack[THREAD_DEFAULT_STACK_SIZE];

/* mbox resources */
static cyg_handle_t        erps_mbhandle;
static cyg_mbox            erps_mbox;

/* critical section resources */
static critd_t             crit;
static critd_t             crit_p;
static critd_t             crit_fsm;
static BOOL                erps_init_done = FALSE;

#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg =
{
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "erps",
    .descr     = "Ethernet Ring Protection"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default (ERPS core)",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_MGMT] = {
        .name      = "mgmt",
        .descr     = "MGMT API",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
     },
    [TRACE_GRP_CTRL] = {
        .name      = "Ctrl",
        .descr     = "Ctrl API",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
     },
    [TRACE_GRP_SWAPI] = {
        .name      = "SwAPI",
        .descr     = "Switch API",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
     },
    [TRACE_GRP_ERPS_RX] = {
        .name      = "ERPSRx",
        .descr     = "ERPS Rx frames",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
     },
    [TRACE_GRP_ERPS_TX] = {
        .name      = "ERPSTx",
        .descr     = "ERPS Tx frames",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
     },
    [TRACE_GRP_STATE_CHANGE] = {
        .name      = "ERPSFSM",
        .descr     = "ERPS State Changes",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
     },
};

#define CRIT_ENTER(crit) critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CRIT_EXIT(crit)  critd_exit(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CRIT_ASSERT_LOCKED(crit) critd_assert_locked(&crit, TRACE_GRP_CRIT, __FILE__, __LINE__)
#else

#define CRIT_ENTER(crit) critd_enter(&crit)
#define CRIT_EXIT(crit)  critd_exit(&crit)
#define CRIT_ASSERT_LOCKED(crit) critd_assert_locked(&crit)

#endif /* VTSS_TRACE_ENABLED */

void vtss_erps_crit_lock(void)
{
    CRIT_ENTER(crit);
}

void vtss_erps_crit_unlock(void)
{
    CRIT_EXIT(crit);
}

void vtss_erps_crit_assert_locked(void)
{
    CRIT_ASSERT_LOCKED(crit);
}
/*
 * mutex for ERPS FSM
 */
void vtss_erps_fsm_crit_lock(void)
{
    CRIT_ENTER(crit_fsm);
}

void vtss_erps_fsm_crit_unlock(void)
{
    CRIT_EXIT(crit_fsm);
}


/******************************************************************************
                              ERPS Control Functions 
******************************************************************************/
static void apply_configuration(const erps_config_blk_t const * blk)
{
    u32                   instance, tmp, tmp_grp_id;
    vtss_erps_mgmt_conf_t conf_req;
    i32                   ret = VTSS_RC_ERROR;
    vtss_erps_base_conf_t pconf;
    u8                    grp_status;

    if (!blk) {
        T_W("ERPS: Cannot apply NULL configuration");
        return;
    }

    for ( instance = 0; instance < ERPS_MAX_PROTECTION_GROUPS; instance++ ) {
        if (blk->groups[instance].enable) {
            /* create a protection group */
            memset(&conf_req, 0, sizeof(conf_req));
            conf_req.req_type              = ERPS_CMD_PROTECTION_GROUP_ADD;
            conf_req.group_id              = blk->groups[instance].group_id;
            conf_req.data.create.east_port = blk->groups[instance].east_port;
            conf_req.data.create.west_port = blk->groups[instance].west_port;
            conf_req.data.create.ring_type = blk->groups[instance].ring_type;
            conf_req.data.create.interconnected = blk->groups[instance].inter_connected_node;
            conf_req.data.create.major_ring_id = blk->groups[instance].major_ring_id;
            conf_req.data.create.virtual_channel = blk->groups[instance].virtual_channel;

            ret = erps_mgmt_set_protection_group_request(&conf_req);

            /* Add protected vlans */
            for (tmp = 0; tmp < PROTECTED_VLANS_MAX; tmp++) {
                if (blk->groups[instance].protected_vlans[tmp]) {
                    memset(&conf_req, 0, sizeof(conf_req));
                    conf_req.req_type              = ERPS_CMD_ADD_VLANS;
                    conf_req.group_id              = blk->groups[instance].group_id;
                    conf_req.data.vid.num_vids     = 1;
                    conf_req.data.vid.p_vid = blk->groups[instance].protected_vlans[tmp];
                    ret += erps_mgmt_set_protection_group_request(&conf_req);
                }
            }

            /* associate MEPs */
            memset(&conf_req, 0, sizeof(conf_req));
            conf_req.req_type              = ERPS_CMD_ADD_MEP_ASSOCIATION;
            conf_req.group_id              = blk->groups[instance].group_id;
            conf_req.data.mep.east_mep_id  = blk->mep_conf[instance].east_mepid;
            conf_req.data.mep.west_mep_id  = blk->mep_conf[instance].west_mepid;
            conf_req.data.mep.raps_eastmep = blk->mep_conf[instance].raps_east_mepid;
            conf_req.data.mep.raps_westmep = blk->mep_conf[instance].raps_west_mepid;

            ret += erps_mgmt_set_protection_group_request(&conf_req);

            /* if RPL Owner, set rpl_block */
            if (blk->groups[instance].rpl_owner && !blk->groups[instance].rpl_neighbour) {
                memset(&conf_req, 0, sizeof(conf_req));
                conf_req.req_type              = ERPS_CMD_SET_RPL_BLOCK;
                conf_req.group_id              = blk->groups[instance].group_id;
                conf_req.data.rpl_block.rpl_port  = blk->groups[instance].rpl_owner_port; 

                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }

            /* if RPL neighbour, set rpl neighbour port */
            if (blk->groups[instance].rpl_neighbour) {
                memset(&conf_req, 0, sizeof(conf_req));
                conf_req.req_type              = ERPS_CMD_SET_RPL_NEIGHBOUR;
                conf_req.group_id              = blk->groups[instance].group_id;
                conf_req.data.rpl_block.rpl_port = blk->groups[instance].rpl_neighbour_port;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }

            /* WTR Timeout */
            memset(&conf_req, 0, sizeof(conf_req));
            conf_req.req_type              = ERPS_CMD_WTR_TIMER;
            conf_req.group_id              = blk->groups[instance].group_id;
            conf_req.data.timer.time       = MIN_TO_MS(blk->groups[instance].wtr_time);

            ret += erps_mgmt_set_protection_group_request(&conf_req);

            /* Hold-Off timeout */
            memset(&conf_req, 0, sizeof(conf_req));
            conf_req.req_type              = ERPS_CMD_HOLD_OFF_TIMER;
            conf_req.group_id              = blk->groups[instance].group_id;
            conf_req.data.timer.time       = blk->groups[instance].hold_off_time;

            ret += erps_mgmt_set_protection_group_request(&conf_req);

            /* Guard Timeout */
            memset(&conf_req, 0, sizeof(conf_req));
            conf_req.req_type              = ERPS_CMD_GUARD_TIMER;
            conf_req.group_id              = blk->groups[instance].group_id;
            conf_req.data.timer.time       = blk->groups[instance].guard_time;

            ret += erps_mgmt_set_protection_group_request(&conf_req);

            /* reversion */
            if (blk->groups[instance].revertive) {
                memset(&conf_req, 0, sizeof(conf_req));
                conf_req.group_id              = blk->groups[instance].group_id;
                conf_req.req_type              = ERPS_CMD_DISABLE_NON_REVERTIVE;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }
            else {
                memset(&conf_req, 0, sizeof(conf_req));
                conf_req.group_id              = blk->groups[instance].group_id;
                conf_req.req_type              = ERPS_CMD_ENABLE_NON_REVERTIVE;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }

            /* ERPS Protocol Version */
            if (blk->groups[instance].version == ERPS_VERSION_V1) {
                memset(&conf_req, 0, sizeof(conf_req));
                conf_req.group_id              = blk->groups[instance].group_id;
                conf_req.req_type              = ERPS_CMD_ENABLE_VERSION_1_COMPATIBLE;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            } 
            else {
                memset(&conf_req, 0, sizeof(conf_req));
                conf_req.group_id              = blk->groups[instance].group_id;
                conf_req.req_type              = ERPS_CMD_DISABLE_VERSION_1_COMPATIBLE;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }

            memset(&conf_req, 0, sizeof(conf_req));
            if (blk->groups[instance].virtual_channel) {
                conf_req.req_type              = ERPS_CMD_SUB_RING_WITH_VIRTUAL_CHANNEL;
                conf_req.group_id              = blk->groups[instance].group_id;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }
            else {
                conf_req.req_type              = ERPS_CMD_SUB_RING_WITHOUT_VIRTUAL_CHANNEL;
                conf_req.group_id              = blk->groups[instance].group_id;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }

            memset(&conf_req, 0, sizeof(conf_req));
            if (blk->groups[instance].topology_change) {
                conf_req.req_type              = ERPS_CMD_TOPOLOGY_CHANGE_PROPAGATE;
                conf_req.group_id              = blk->groups[instance].group_id;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }
            else {
                conf_req.req_type              = ERPS_CMD_TOPOLOGY_CHANGE_NO_PROPAGATE;
                conf_req.group_id              = blk->groups[instance].group_id;
                ret += erps_mgmt_set_protection_group_request(&conf_req);
            }
        } else {
            tmp_grp_id = CONV_MGMTTOERPS_INSTANCE(blk->groups[instance].group_id);
            if (vtss_erps_get_protection_group_status(tmp_grp_id, &grp_status) == VTSS_RC_OK) {
               if (grp_status == ERPS_PROTECTION_GROUP_ACTIVE) { 
                   memset(&pconf, 0, sizeof(pconf));
                   pconf.group_id = tmp_grp_id;
                   if (vtss_erps_get_protection_group_by_id(&pconf) == VTSS_RC_OK) {
                       /* disable R-APS forwarding for this protection group */
                       ret = vtss_erps_raps_forwarding(pconf.erpg.east_port, tmp_grp_id, FALSE);
                       if (ret != VTSS_RC_OK) {
                           T_D("vtss_erps_raps_forwarding failed for group = %u\n", tmp_grp_id);
                       }
                       if (pconf.erpg.west_port || pconf.erpg.virtual_channel) {
                           ret = vtss_erps_raps_forwarding(pconf.erpg.west_port, tmp_grp_id, FALSE);
                           if (ret != VTSS_RC_OK) {
                               T_D("vtss_erps_raps_forwarding failed for group = %u\n", tmp_grp_id);
                           }
                       }
                       /* disable R-APS transmission for this protecion group */
                       ret = vtss_erps_raps_transmission(pconf.erpg.east_port, tmp_grp_id, FALSE);
                       if (ret != VTSS_RC_OK) {
                           T_D("vtss_erps_raps_transmission failed for group = %u\n", tmp_grp_id);
                       }
                       if (pconf.erpg.west_port || pconf.erpg.virtual_channel) {
                           ret = vtss_erps_raps_transmission(pconf.erpg.west_port, tmp_grp_id, FALSE);
                           if (ret != VTSS_RC_OK) {
                               T_D("vtss_erps_raps_transmission failed for group = %u\n", tmp_grp_id);
                           }
                       }
                       /* deregister with mep */
                       ret = erps_mep_aps_sf_register(pconf.erpg.east_port, pconf.erpg.west_port,
                                                      tmp_grp_id, FALSE, pconf.erpg.virtual_channel);
                       if (ret != VTSS_RC_OK) {
                           T_D("vtss_erps_mep_aps_sf_register failed for group = %u\n", tmp_grp_id);
                       }
                       ret = vtss_erps_delete_protection_group (tmp_grp_id);
                       if (ret == ERPS_RC_OK) {
                           erps_platform_clear_port_to_mep(tmp_grp_id);
                       } /* if (ret == ERPS_RC_OK) */
                   } /* if (vtss_erps_get_protection_group_by_id(&pconf) == VTSS_RC_OK) */
               } /* if (grp_status == ERPS_PROTECTION_GROUP_ACTIVE) */
            } /* if (vtss_erps_is_protection_group_active(mgmt_req.group_id) == VTSS_RC_OK) */
        }
    }
    T_D("\n return from apply_configuration = %d \n", ret );
}

void erps_save_configuration(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    erps_config_blk_t *blk;
    vtss_erps_mgmt_conf_t  blk_req;
    ulong size;

    u32 instance, blk_instance = 0;
    blk_req.group_id = 0;

    memset(&blk_req, 0, sizeof(blk_req));

    blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ERPS_CONF, &size);
    bzero(blk, sizeof(erps_config_blk_t));
    while (erps_mgmt_getnext_protection_group_request (&blk_req) == VTSS_RC_OK) {
        bcopy(&blk_req.data.get.erpg, &blk->groups[blk_instance], sizeof(vtss_erps_config_erpg_t));
        blk->mep_conf[blk_instance].east_mepid = blk_req.data.mep.east_mep_id;
        blk->mep_conf[blk_instance].west_mepid = blk_req.data.mep.west_mep_id;
        blk->mep_conf[blk_instance].raps_east_mepid = blk_req.data.mep.raps_eastmep;
        blk->mep_conf[blk_instance].raps_west_mepid = blk_req.data.mep.raps_westmep;
        blk->groups[blk_instance].enable = TRUE;
        if (blk_instance > ERPS_MAX_PROTECTION_GROUPS ) {
            break;
        }
        instance = blk_req.group_id;
        memset(&blk_req, 0, sizeof(blk_req));
        blk_req.group_id = instance;
        blk_instance++;
    }
    blk->count = blk_instance;
    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ERPS_CONF);
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

static void set_conf_to_default(erps_config_blk_t *blk)
{
    u32 i;
    for (i = 0; i < ERPS_MAX_PROTECTION_GROUPS; ++i) {
        blk->groups[i].enable = FALSE;
    }
}

static void erps_restore_to_default(void)
{
    u32                   group_id;
    i32                   ret;
    vtss_erps_base_conf_t pconf;
    u8                    grp_status;

    /* All MEP-related functions are going to fail because the MEPs have already been
     * reset to default -- module 'mep' comes before module 'erps' in the initfun table.
     */

    for (group_id = 0; group_id < ERPS_MAX_PROTECTION_GROUPS; ++group_id) {
        if (vtss_erps_get_protection_group_status(group_id, &grp_status) != VTSS_RC_OK) {
            T_D("Failed getting internal group id status %d", group_id);
            continue;
        }

        // Special case for RESERVED group: no MEP stuff is configured, so just delete it
        if (grp_status == ERPS_PROTECTION_GROUP_RESERVED) {
            ret = vtss_erps_delete_protection_group (group_id);
            continue;
        }

        if (grp_status != ERPS_PROTECTION_GROUP_ACTIVE) {
            continue;
        }

        T_D("Disabling active internal group id %d", group_id);

        memset(&pconf, 0, sizeof(pconf));
        pconf.group_id = group_id;
        if (vtss_erps_get_protection_group_by_id(&pconf) != VTSS_RC_OK) {
            T_D("Failed getting internal group id %d", group_id);
            continue;
        }

        /* disable R-APS forwarding for this protection group */
        ret = vtss_erps_raps_forwarding(pconf.erpg.east_port, group_id, FALSE);
        if (ret != VTSS_RC_OK) {
            T_D("vtss_erps_raps_forwarding failed for group = %u\n", group_id);
        }

        if (pconf.erpg.west_port || pconf.erpg.virtual_channel) {
            ret = vtss_erps_raps_forwarding(pconf.erpg.west_port, group_id, FALSE);
            if (ret != VTSS_RC_OK) {
                T_D("vtss_erps_raps_forwarding failed for group = %u\n", group_id);
            }
        }

        /* disable R-APS transmission for this protection group */
        ret = vtss_erps_raps_transmission(pconf.erpg.east_port, group_id, FALSE);
        if (ret != VTSS_RC_OK) {
            T_D("vtss_erps_raps_transmission failed for group = %u\n", group_id);
        }

        if (pconf.erpg.west_port || pconf.erpg.virtual_channel) {
            ret = vtss_erps_raps_transmission(pconf.erpg.west_port, group_id, FALSE);
            if (ret != VTSS_RC_OK) {
                T_D("vtss_erps_raps_transmission failed for group = %u\n", group_id);
            }
        }

        /* deregister with MEP */
        ret = erps_mep_aps_sf_register(pconf.erpg.east_port, pconf.erpg.west_port,
                                       group_id, FALSE, pconf.erpg.virtual_channel);
        if (ret != VTSS_RC_OK) {
            T_D("vtss_erps_mep_aps_sf_register failed for group = %u\n", group_id);
        }

        ret = vtss_erps_delete_protection_group (group_id);
        if (ret != ERPS_RC_OK) {
            T_D("Failed to delete internal group id status %d", group_id);
            continue;
        }
        erps_platform_clear_port_to_mep(group_id);
    }
}

/******************************************************************************
       File Scope functions
******************************************************************************/
void erps_platform_set_erpg_ports (vtss_port_no_t east, vtss_port_no_t west, 
                                   u32 group_id)
{
    CRIT_ENTER(crit_p);
    port_to_mep[group_id].port[0] = east;
    port_to_mep[group_id].port[1] = west;
    CRIT_EXIT(crit_p);
}

void erps_platform_set_erpg_meps (u32 ccm_east, u32 ccm_west, u32 raps_east,
                                  u32 raps_west, u32 group_id)
{
    CRIT_ENTER(crit_p);
    port_to_mep[group_id].ccm_mep_id[0]  = ccm_east;
    port_to_mep[group_id].ccm_mep_id[1]  = ccm_west;
    port_to_mep[group_id].raps_mep_id[0] = raps_east;
    port_to_mep[group_id].raps_mep_id[1] = raps_west;
    CRIT_EXIT(crit_p);

}

void erps_platform_clear_port_to_mep (u32 group_id)
{
    CRIT_ENTER(crit_p);
    bzero(&port_to_mep[group_id], sizeof(erps_mep_t));
    CRIT_EXIT(crit_p);
}


static erps_mep_t vtss_get_erpg_to_mep (u32 group_id)
{
    erps_mep_t inst;
    CRIT_ENTER(crit_p);
    memcpy(&inst, &port_to_mep[group_id], sizeof(erps_mep_t));
    CRIT_EXIT(crit_p);
    return (inst);
}

/******************************************************************************
                              ERPS Callout Functions
******************************************************************************/
 /* VLAN Module Interface Functions */
vtss_rc vtss_erps_vlan_ingress_filter (vtss_port_no_t l2port, BOOL enable)
{
    vtss_port_no_t switchport;
    vtss_isid_t isid;
    vlan_port_conf_t vlan_pconf;
    vtss_rc rc;

    memset(&vlan_pconf, 0, sizeof(vlan_pconf));

    if (l2port2port(API2L2PORT(l2port), &isid, &switchport)) {
        if (vlan_mgmt_port_conf_get(isid, switchport, &vlan_pconf, VLAN_USER_ERPS) != VTSS_OK ) {
            return (VTSS_RC_ERROR);
        }

        T_NG(TRACE_GRP_CTRL, "VLAN:vlan_mgmt_port_conf_set -- l2port = %u enable = %d", l2port, enable);
        if (enable) {
            vlan_pconf.flags = VLAN_PORT_FLAGS_INGR_FILT;
        } else {
            vlan_pconf.flags = 0;
        }
        vlan_pconf.ingress_filter = enable;

        if ((rc = vlan_mgmt_port_conf_set(isid, switchport, &vlan_pconf, VLAN_USER_ERPS)) != VTSS_RC_OK) {
            T_E("%u:%u: %s", isid, iport2uport(switchport), error_txt(rc));
        }
    } else {
        T_D("Set l2port %u VLAN filtering %d - not a port", l2port, enable);
    }
    return VTSS_RC_OK;
}

/* MEP Module Interface Functions */
/* function for sending out of R-APS Messages */
vtss_rc vtss_erps_raps_tx (vtss_port_no_t port, u32 eps_instance, u8 *aps, BOOL event)
{
    vtss_rc ret = VTSS_RC_ERROR;
    u32    mep_id;

    T_NG(TRACE_GRP_CTRL, "MEP:mep_tx_aps_info_set -- port = %u eps_instance = %u, event = %d", port, eps_instance, event);

    T_NG(TRACE_GRP_ERPS_TX, "MEP:mep_tx_aps_info_set -- port = %u eps_instance = %u", port, eps_instance);

    T_NG_HEX(TRACE_GRP_CTRL, aps, ERPS_PDU_SIZE);

    T_NG_HEX(TRACE_GRP_ERPS_TX, aps, ERPS_PDU_SIZE);


    CRIT_ENTER(crit_p);
    if (port_to_mep[eps_instance].port[ERPS_ARRAY_OFFSET_EAST] == port) {
        mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[eps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_EAST]);

        if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
            CRIT_EXIT(crit_p);
            return (VTSS_RC_OK);
        }

        T_NG(TRACE_GRP_CTRL, "MEP:mep_tx_aps_info_set east mep_id = %u", mep_id);

        ret = mep_tx_aps_info_set (mep_id, eps_instance, aps, event);
        if (ret != VTSS_RC_OK) {
            T_D("\n failed to set tx for east raps_mep_id ");
            CRIT_EXIT(crit_p);
            return (VTSS_RC_ERROR);
        }
    } else if (port_to_mep[eps_instance].port[ERPS_ARRAY_OFFSET_WEST] == port) {
        mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[eps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_WEST]);
        if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
            CRIT_EXIT(crit_p);
            return (VTSS_RC_OK);
        }

        T_NG(TRACE_GRP_CTRL, "MEP:mep_tx_aps_info_set west mep_id = %u", mep_id);

        ret = mep_tx_aps_info_set (mep_id, eps_instance, aps, event);
        if (ret != VTSS_RC_OK) {
            T_D("\n failed to set tx for west raps_mep_id ");
            CRIT_EXIT(crit_p);
            return (VTSS_RC_ERROR);
        }
    } else {
         T_D("\n unable to set aps for tx");
         CRIT_EXIT(crit_p);
         return (VTSS_RC_ERROR);
    }
    CRIT_EXIT(crit_p);
    return (VTSS_RC_OK);
}

/* function for enabling R-APS PDU forwarding to ERPS */
vtss_rc vtss_erps_mep_register (vtss_port_no_t port, u32 erps_instance, BOOL enable)
{
    vtss_rc ret = VTSS_RC_ERROR;
    u32    mep_id = 0;

    CRIT_ENTER(crit_p);
    if (enable) {
        if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_EAST] == port) {
            
            mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_EAST]);
            if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
                CRIT_EXIT(crit_p);
                return (VTSS_RC_OK);
            }

            T_NG(TRACE_GRP_CTRL, "MEP:mep_eps_aps_register mep_id = %u erps_instance = %u \
                                 enable = %d", mep_id, erps_instance, enable);

            ret = mep_eps_aps_register(mep_id, erps_instance,
                                                MEP_EPS_TYPE_ERPS, TRUE);

            T_NG(TRACE_GRP_CTRL, "MEP:mep_eps_aps_register return value = %d", ret);
        } else if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_WEST] == port) {
            mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_WEST]);
            if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
                CRIT_EXIT(crit_p);
                return (VTSS_RC_OK);
            }
            T_NG(TRACE_GRP_CTRL, "MEP:mep_eps_aps_register mep_id = %u erps_instance = %u \
                                 enable = %d", mep_id, erps_instance, enable);

            ret = mep_eps_aps_register(mep_id, erps_instance,
                                                MEP_EPS_TYPE_ERPS, TRUE);
            T_NG(TRACE_GRP_CTRL, "MEP:mep_eps_aps_register return value = %d", ret );
        }
    } else {
        if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_EAST] == port) {
            mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_EAST]);
            if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
                CRIT_EXIT(crit_p);
                return (VTSS_RC_OK);
            }
            T_NG(TRACE_GRP_CTRL, "MEP:mep_eps_aps_register mep_id = %u erps_instance = %u \
                                 enable = %d", mep_id, erps_instance, enable);

            ret = mep_eps_aps_register(mep_id, erps_instance,
                                                MEP_EPS_TYPE_ERPS, FALSE);

            T_NG(TRACE_GRP_CTRL, "MEP:mep_eps_aps_register return value = %d", ret);
        } else if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_WEST] == port) {
            mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_WEST]);
            if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
                CRIT_EXIT(crit_p);
                return (VTSS_RC_OK);
            }
            T_NG(TRACE_GRP_CTRL, "MEP:mep_eps_aps_register mep_id = %u erps_instance = %u \
                                 enable = %d", mep_id, erps_instance, enable);

            ret = mep_eps_aps_register(mep_id, erps_instance,
                                                MEP_EPS_TYPE_ERPS, FALSE);

            T_NG(TRACE_GRP_CTRL, "MEP:mep_eps_aps_register return value = %d", ret );
        }
    }
    CRIT_EXIT(crit_p);
    return (ret);
}

vtss_rc vtss_erps_raps_forwarding ( vtss_port_no_t port,
                                    u32 erps_instance,
                                    BOOL enable )
{
    u32 mep_id = 0;
    u32 ret; 

    CRIT_ENTER(crit_p); 
    if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_EAST] == port) {
        mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_EAST]);
    } else if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_WEST] == port) {
        mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_WEST]);
    }
    CRIT_EXIT(crit_p); 

    if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
        T_DG(TRACE_GRP_CTRL, "MEP: mep_raps_forwarding returning error VTSS_MEP_INSTANCE_MAX");
        return (VTSS_RC_OK);
    }

    ret = mep_raps_forwarding(mep_id, erps_instance, enable);

    T_NG(TRACE_GRP_CTRL, "MEP: mep_raps_forwarding mep_id = %u erps_instance = %u enable = %d => rc = %d", mep_id, erps_instance, enable, ret);

    return (ret);
}

/* function for registering for SF signals from MEP */
vtss_rc vtss_erps_mep_sf_register (vtss_port_no_t port, u32 erps_instance,
                                   BOOL enable)
{
    vtss_rc ret1, ret2;

    u32 mep_id = 0;
    u32 raps_mep = 0;

    CRIT_ENTER(crit_p);
    if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_EAST] == port) {
        raps_mep = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_EAST]);
        mep_id   = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].ccm_mep_id[ERPS_ARRAY_OFFSET_EAST]);
    } else if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_WEST] == port) {
        raps_mep = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_WEST]);
        mep_id   = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].ccm_mep_id[ERPS_ARRAY_OFFSET_WEST]);
    }
    CRIT_EXIT(crit_p);

    if (mep_id >= VTSS_MEP_INSTANCE_MAX || raps_mep >= VTSS_MEP_INSTANCE_MAX) {
        T_DG(TRACE_GRP_CTRL, "One of MEP %d or RAPS MEP %d > %d - failing", mep_id, raps_mep, VTSS_MEP_INSTANCE_MAX);
        return (VTSS_RC_OK);
    }

    ret1 = mep_eps_sf_register(mep_id,   erps_instance, MEP_EPS_TYPE_ERPS, enable);
    ret2 = mep_eps_sf_register(raps_mep, erps_instance, MEP_EPS_TYPE_ERPS, enable);

    T_NG(TRACE_GRP_CTRL, "MEP: mep_eps_sf_register mep_id   = %u erps_instance = %u enable = %d. rc = %d", mep_id,   erps_instance, enable, ret1);
    T_NG(TRACE_GRP_CTRL, "MEP: mep_eps_sf_register raps_mep = %u erps_instance = %u enable = %d. rc = %d", raps_mep, erps_instance, enable, ret2);

    return (ret1 != VTSS_RC_OK  ||  ret2 != VTSS_RC_OK) ? VTSS_RC_ERROR : VTSS_RC_OK;
}

vtss_rc vtss_erps_raps_transmission ( vtss_port_no_t port, 
                                          u32 erps_instance, BOOL enable )
{
    u32 raps_mep = 0;
    u32 ret;

    CRIT_ENTER(crit_p);
    if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_EAST] == port) {
        raps_mep = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_EAST]);
    } else if (port_to_mep[erps_instance].port[ERPS_ARRAY_OFFSET_WEST] == port) {
        raps_mep = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_WEST]);
    }
    CRIT_EXIT(crit_p);

    if (raps_mep >= VTSS_MEP_INSTANCE_MAX) {
        T_DG(TRACE_GRP_CTRL, "RAPS MEP %d > %d - failing", raps_mep, VTSS_MEP_INSTANCE_MAX);
        return (VTSS_RC_OK);
    }

    ret = mep_raps_transmission(raps_mep, erps_instance, enable);

    T_NG(TRACE_GRP_CTRL, "MEP: vtss_erps_mep_raps_transmission raps_mep = %u erps_instance = %u enable = %d => rc = %d", raps_mep, erps_instance, enable, ret);
    return(ret);
}

vtss_rc erps_mep_aps_sf_register (vtss_port_no_t east_port, vtss_port_no_t west_port, 
                                  u32 erps_instance, BOOL enable, BOOL raps_virt_channel)
{
    vtss_rc  rc = VTSS_RC_OK;
    i32      ret;

    /* deregister with mep for RAPS */
    ret = vtss_erps_mep_register(east_port, erps_instance, enable);
    if (ret != VTSS_RC_OK) {
        rc = VTSS_RC_ERROR;
        T_D("vtss_erps_mep_register failed for group = %u\n", erps_instance);
    }
    if (west_port || raps_virt_channel) {
        ret = vtss_erps_mep_register(west_port, erps_instance, enable);
        if (ret != VTSS_RC_OK) {
            rc = VTSS_RC_ERROR;
            T_D("vtss_erps_mep_register failed for group = %u\n", erps_instance);
        }
    }

    /* need to de-register for SF also */
    ret = vtss_erps_mep_sf_register(east_port, erps_instance, enable);
    if (ret != VTSS_RC_OK) {
        rc = VTSS_RC_ERROR;
        T_D("vtss_erps_mep_sf_register failed for group = %u\n", erps_instance);
    }
    if (west_port || raps_virt_channel) {
        ret = vtss_erps_mep_sf_register(west_port, erps_instance, enable);
        if (ret != VTSS_RC_OK) {
            rc = VTSS_RC_ERROR;
            T_D("vtss_erps_mep_sf_register failed for group = %u\n", erps_instance);
        }
    }
    return rc;
}

vtss_rc erps_sf_sd_state_set(const u32 instance, const u32 mep_instance,
                             const BOOL sf_state, const BOOL sd_state)
{
    u32 mep_id, wmep, emep;
    vtss_port_no_t e_port, w_port;
    mep_id = CONV_ERPSTOMGMT_INSTANCE(mep_instance);

    if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
        return (VTSS_RC_OK);
    }

    CRIT_ENTER(crit_p);
    wmep = port_to_mep[instance].ccm_mep_id[ERPS_ARRAY_OFFSET_WEST];
    emep = port_to_mep[instance].ccm_mep_id[ERPS_ARRAY_OFFSET_EAST];
    e_port = port_to_mep[instance].port[ERPS_ARRAY_OFFSET_EAST];
    w_port = port_to_mep[instance].port[ERPS_ARRAY_OFFSET_WEST];
    CRIT_EXIT(crit_p);

    T_NG(TRACE_GRP_CTRL, "MEP: erps_sf_sd_state_set wmep = %u emep = %u instance = %u ", wmep, emep, instance);
    T_NG(TRACE_GRP_CTRL, "MEP: erps_sf_sd_state_set sf_state = %d sd_state = %d ", sf_state, sd_state);

    if (emep == mep_id) {
        return (vtss_erps_sf_sd_state_set(instance, sf_state, sd_state, e_port) == VTSS_RC_OK) ? VTSS_RC_OK : VTSS_RC_ERROR;
    } else if (wmep == mep_id) {
        return (vtss_erps_sf_sd_state_set(instance, sf_state, sd_state, w_port) == VTSS_RC_OK) ? VTSS_RC_OK : VTSS_RC_ERROR;
    }
    return (VTSS_RC_ERROR);
}

void erps_instance_signal_in (u32 erps_instance)
{
    u32 mep_id;
    vtss_rc ret = VTSS_RC_ERROR;
  
    CRIT_ENTER(crit_p);
    mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_EAST]);
    CRIT_EXIT(crit_p);

    T_NG(TRACE_GRP_CTRL, "MEP: mep_signal_in erps_instance = %u mep_id = %u ", erps_instance, mep_id);

    if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
        return;
    }
    ret = mep_signal_in(mep_id, erps_instance);

    if (ret != VTSS_RC_OK) {
        T_D("\n failed in registering with MEP for raps east \n");
    }

    CRIT_ENTER(crit_p);

    mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].raps_mep_id[ERPS_ARRAY_OFFSET_WEST]);
    CRIT_EXIT(crit_p);
    if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
        return;
    }
    ret = mep_signal_in(mep_id, erps_instance);

    if (ret != VTSS_RC_OK) {
        T_D("\n failed in registering with MEP for raps west \n");
    }

    CRIT_ENTER(crit_p);
    mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].ccm_mep_id[ERPS_ARRAY_OFFSET_EAST]);
    CRIT_EXIT(crit_p);
    if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
        return;
    }
    ret = mep_signal_in(mep_id, erps_instance);

    if (ret != VTSS_RC_OK) {
        T_D("\n failed in registering with MEP for ccm east \n");
    }

    CRIT_ENTER(crit_p);
    mep_id = CONV_MGMTTOERPS_INSTANCE(port_to_mep[erps_instance].ccm_mep_id[ERPS_ARRAY_OFFSET_WEST]);
    CRIT_EXIT(crit_p);
    if (mep_id >= VTSS_MEP_INSTANCE_MAX ) {
        return;
    }
    ret = mep_signal_in(mep_id, erps_instance);

    if (ret != VTSS_RC_OK) {
        T_D("\n failed in registering with MEP for ccm west \n");
    }
}

                            
/* Chip Platform Interface */
vtss_rc vtss_erps_ctrl_set_vlanmap (vtss_vid_t vid, vtss_erpi_t inst, BOOL member)
{
    T_NG(TRACE_GRP_SWAPI, "SWAPI: vtss_erps_vlan_member_set erpi inst = %u vid = %d ", inst, vid);

    if (vtss_erps_vlan_member_set(NULL, API2ERPS_HWINSTANCE(inst), vid, member) != VTSS_RC_OK) {
        T_D("\n setting vtss_erps_vlan_erpi_set failed vtss_erpi_t = %d \n", inst );
        return (VTSS_RC_ERROR);
    }
 
    return (VTSS_RC_OK);
}

vtss_rc vtss_erps_protection_group_state_set ( vtss_erpi_t inst, 
                                               vtss_port_no_t port,
                                               vtss_erps_state_t state )
{
    vtss_rc ret = VTSS_RC_ERROR;

    T_NG(TRACE_GRP_SWAPI, "SWAPI: vtss_erps_port_state_set erpi inst = %u port = %u  state = %d", inst, port, state);

    ret = vtss_erps_port_state_set(NULL, API2ERPS_HWINSTANCE(inst), API2L2PORT(port), state);
    mep_ring_protection_block(API2L2PORT(port),  (state == VTSS_ERPS_STATE_DISCARDING));
    if ( ret != VTSS_RC_OK) {
        T_D("\n error in setting protection group state = %d \n", inst );
        return (VTSS_RC_ERROR);
    }
    return (VTSS_RC_OK);
}

vtss_rc vtss_erps_put_protected_vlans_in_forwarding (vtss_erpi_t inst, vtss_port_no_t rplport)
{
    vtss_port_no_t     iport;
    vtss_rc ret = VTSS_RC_ERROR;
    
    T_NG(TRACE_GRP_SWAPI, "SWAPI: vtss_erps_port_state_set erpi inst = %u port = %u  state = %d", inst, rplport, VTSS_ERPS_STATE_FORWARDING);
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        if (iport == API2L2PORT(rplport)){
            continue;
        }

        ret = vtss_erps_port_state_set(NULL, API2ERPS_HWINSTANCE(inst), iport, VTSS_ERPS_STATE_FORWARDING);
        if (ret != VTSS_RC_OK) {
            T_D("\n error in putting all port+vlan combination in forwarding state \n");
            return (VTSS_RC_ERROR);
        }
    }
    return (VTSS_RC_OK);
}

vtss_rc vtss_erps_flush_fdb (vtss_port_no_t port, vtss_vid_t vid)
{
    T_NG(TRACE_GRP_SWAPI, "SWAPI:vtss_mac_table_vlan_flush vid = %d", vid);

    return(vtss_mac_table_vlan_flush(NULL, vid));
}

vtss_rc vtss_erps_get_port_mac (u32 port, uchar mac[6])
{
    if (conf_mgmt_mac_addr_get(mac, port) < 0) {
        T_D("Error getting own MAC:\n");
        return (VTSS_RC_ERROR);
    }
    return (VTSS_RC_OK);
}

void vtss_erps_trace(const char  *const string, const u32   param)
{
    T_D("%s %u", string, param) ;
}

void vtss_erps_trace_state_change (const char *const string, const u32 parm)
{
    T_NG(TRACE_GRP_STATE_CHANGE, "%s  %u", string, parm);
}

u64 vtss_erps_current_time (void)
{
    return (cyg_current_time());
}

BOOL vtss_erps_is_initialized()
{
    return (erps_init_done);
}

/******************************************************************************
                              ERPS Interface 
******************************************************************************/
/* this function gets called from MEP module soon an R-APS PDU received */
void erps_handover_raps_pdu(const u8 *pdu, u32 mep_id, u8 len, u32 erpg)
{
    raps_pdu_in_t   *req = VTSS_MALLOC(sizeof(raps_pdu_in_t));

    if ( req != NULL ) {
        memcpy(req->pdu, pdu, len);
        req->len    = len;
        req->mep_id = mep_id;
        req->erpg = erpg;
        if (!cyg_mbox_put(erps_mbhandle, req)) {
            T_D("\n error in writing into mbox \n"); 
        }
    }
}

/*****************************************************************************p
                              MGMT Interface 
******************************************************************************/
vtss_rc erps_mgmt_set_protection_group_request (const vtss_erps_mgmt_conf_t *mgmt_req)
{
    vtss_rc                ret = ERPS_RC_OK;
    vtss_erps_base_conf_t  pconf;
    u8                     grp_status;
    u32                    group_id = CONV_MGMTTOERPS_INSTANCE(mgmt_req->group_id);

    switch (mgmt_req->req_type) {
        case ERPS_CMD_PROTECTION_GROUP_ADD:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_create_protection_group group_id = %u east_port = %u west_port = %u",
                 group_id, mgmt_req->data.create.east_port, mgmt_req->data.create.west_port);
            ret = vtss_erps_create_protection_group(group_id,
                                               mgmt_req->data.create.east_port,
                                               mgmt_req->data.create.west_port,
                                               mgmt_req->data.create.ring_type,
                                               mgmt_req->data.create.interconnected,
                                               mgmt_req->data.create.major_ring_id,
                                               mgmt_req->data.create.virtual_channel);
            if (ret == ERPS_RC_OK) {
                /* update port numbers in port_to_mep global strcture */
                erps_platform_set_erpg_ports(mgmt_req->data.create.east_port,
                                             mgmt_req->data.create.west_port,
                                             group_id);
            }
            break;
        case ERPS_CMD_PROTECTION_GROUP_DELETE:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_delete_protection_group group_id = %u", group_id);
            if (vtss_erps_get_protection_group_status(group_id, &grp_status) == VTSS_RC_OK) {
                if (grp_status == ERPS_PROTECTION_GROUP_RESERVED) {
                    ret = vtss_erps_delete_protection_group (group_id);
                } else if (grp_status == ERPS_PROTECTION_GROUP_ACTIVE) {
                    memset(&pconf, 0, sizeof(pconf));
                    pconf.group_id = group_id;
                    if (vtss_erps_get_protection_group_by_id(&pconf) == VTSS_RC_OK) {
                        /* disable R-APS forwarding for this protection group */
                        ret = vtss_erps_raps_forwarding(pconf.erpg.east_port, pconf.group_id, FALSE);
                        if (ret != VTSS_RC_OK) {
                            T_D("vtss_erps_raps_forwarding failed for group = %u\n", pconf.group_id);
                        }
                        if (pconf.erpg.west_port || pconf.erpg.virtual_channel) {
                            ret = vtss_erps_raps_forwarding(pconf.erpg.west_port, pconf.group_id, FALSE);
                            if (ret != VTSS_RC_OK) {
                                T_D("vtss_erps_raps_forwarding failed for group = %u\n", pconf.group_id);
                            }
                        }
                        /* disable R-APS transmission for this protecion group */
                        ret = vtss_erps_raps_transmission(pconf.erpg.east_port, pconf.group_id, FALSE);
                        if (ret != VTSS_RC_OK) {
                            T_D("vtss_erps_raps_transmission failed for group = %u\n", pconf.group_id);
                        }
                        if (pconf.erpg.west_port || pconf.erpg.virtual_channel) {
                            ret = vtss_erps_raps_transmission(pconf.erpg.west_port, pconf.group_id, FALSE);
                            if (ret != VTSS_RC_OK) {
                                T_D("vtss_erps_raps_transmission failed for group = %u\n", pconf.group_id);
                            }
                        }
                        /* deregister with mep */
                        ret = erps_mep_aps_sf_register(pconf.erpg.east_port, pconf.erpg.west_port,
                                pconf.group_id, FALSE, pconf.erpg.virtual_channel);
                        if (ret != VTSS_RC_OK) {
                            T_D("vtss_erps_mep_aps_sf_register failed for group = %u\n", pconf.group_id);
                        }
                        ret = vtss_erps_delete_protection_group (group_id);
                        if (ret == ERPS_RC_OK) {
                            erps_platform_clear_port_to_mep(group_id);
                        }
                    }
                }
            }
            break;
        case ERPS_CMD_HOLD_OFF_TIMER:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_set_holdoff_timeout group_id = %u hold timeout = %llu", \
                                  group_id, mgmt_req->data.timer.time);
            ret = vtss_erps_set_holdoff_timeout (group_id,
                                          mgmt_req->data.timer.time);
            break;
        case ERPS_CMD_WTR_TIMER:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_set_wtr_timeout group_id = %u wtr timeout = %llu", \
                                  group_id, mgmt_req->data.timer.time);
            ret = vtss_erps_set_wtr_timeout (group_id,
                                             mgmt_req->data.timer.time);
            break;
        case ERPS_CMD_GUARD_TIMER:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_set_guard_timeout group_id = %u guard timeout = %llu", \
                                  group_id, mgmt_req->data.timer.time);
            ret = vtss_erps_set_guard_timeout (group_id,
                                          mgmt_req->data.timer.time);
            break;
        case ERPS_CMD_ADD_VLANS:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_associate_protected_vlan group_id = %u vid = %d", \
                                  group_id, mgmt_req->data.vid.p_vid);
            ret = vtss_erps_associate_protected_vlans (group_id,
                                                  mgmt_req->data.vid.p_vid);
            break;
        case ERPS_CMD_DEL_VLANS:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_remove_protected_vlan group_id = %u vid = %d", \
                                  group_id, mgmt_req->data.vid.p_vid);
            ret = vtss_erps_remove_protected_vlan (group_id,
                                              mgmt_req->data.vid.num_vids,
                                              mgmt_req->data.vid.p_vid);
            break;
        case ERPS_CMD_SET_RPL_BLOCK:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_set_rpl_block group_id = %u block port = %u ", \
                                  group_id, mgmt_req->data.rpl_block.rpl_port);
            ret = vtss_erps_set_rpl_owner (group_id,
                                           mgmt_req->data.rpl_block.rpl_port, FALSE );
            break;
        case ERPS_CMD_UNSET_RPL_BLOCK:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_unset_rpl_owner group_id = %u ", group_id);
            ret = vtss_erps_unset_rpl_owner (group_id);
            break;
        case ERPS_CMD_ADD_MEP_ASSOCIATION:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_associate_group group_id = %u eastmep = %u westmep = %u east rapsmep = %u  \
                                  west rapsmep = %u", group_id, mgmt_req->data.mep.east_mep_id, mgmt_req->data.mep.west_mep_id, \
                                  mgmt_req->data.mep.raps_eastmep, mgmt_req->data.mep.raps_westmep);
            /* update mep_ids in port_to_mep global strcture */
            CRIT_ENTER(crit_p);
            if (!port_to_mep[group_id].active) {
            CRIT_EXIT(crit_p);
                erps_platform_set_erpg_meps(mgmt_req->data.mep.east_mep_id,
                                        mgmt_req->data.mep.west_mep_id,
                                        mgmt_req->data.mep.raps_eastmep,
                                        mgmt_req->data.mep.raps_westmep,
                                        group_id);
            CRIT_ENTER(crit_p);
                port_to_mep[group_id].active = TRUE;
            CRIT_EXIT(crit_p);
            } else {
                CRIT_EXIT(crit_p);
            }

            if (vtss_erps_get_protection_group_status(group_id, &grp_status) == VTSS_RC_OK) {
                if ((grp_status == ERPS_PROTECTION_GROUP_ACTIVE) || (grp_status == ERPS_PROTECTION_GROUP_INACTIVE)){
                    ret = (grp_status == ERPS_PROTECTION_GROUP_ACTIVE) ? ERPS_ERROR_CANNOT_ASSOCIATE_GROUP :
                                                                         ERPS_ERROR_GROUP_NOT_EXISTS;
                } else {
                    memset(&pconf, 0, sizeof(pconf));
                    pconf.group_id = group_id;
                    if (vtss_erps_get_protection_group_by_id(&pconf) == VTSS_RC_OK) {
                        /* register with mep */
                        ret = erps_mep_aps_sf_register(pconf.erpg.east_port, pconf.erpg.west_port,
                                                       pconf.group_id, TRUE, pconf.erpg.virtual_channel);
                        if (ret != VTSS_RC_OK) {
                            T_D("vtss_erps_mep_aps_sf_register failed for group = %u\n", pconf.group_id);
                        }
                        ret = vtss_erps_associate_group (group_id);
                        if (ret == VTSS_RC_OK) {
                            /* register with MEP for incoming signal */
                            erps_instance_signal_in(group_id);
                        }
                    }
                }
            }
            CRIT_ENTER(crit_p);
            if (port_to_mep[group_id].active == TRUE && ret == ERPS_RC_OK) {
            CRIT_EXIT(crit_p);
                erps_platform_set_erpg_meps(mgmt_req->data.mep.east_mep_id,
                                        mgmt_req->data.mep.west_mep_id,
                                        mgmt_req->data.mep.raps_eastmep,
                                        mgmt_req->data.mep.raps_westmep,
                                        group_id);

            } else {
                CRIT_EXIT(crit_p);
            }
            break;
        case ERPS_CMD_DEL_MEP_ASSOCIATION:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_associate_group group_id = %u eastmep = %u westmep = %u east rapsmep = %u  \
                                  west rapsmep = %u", group_id, mgmt_req->data.mep.east_mep_id, mgmt_req->data.mep.west_mep_id, \
                                  mgmt_req->data.mep.raps_eastmep, mgmt_req->data.mep.raps_westmep);
            if (vtss_erps_get_protection_group_status(group_id, &grp_status) == VTSS_RC_OK) {
                if ((grp_status == ERPS_PROTECTION_GROUP_ACTIVE) || (grp_status == ERPS_PROTECTION_GROUP_INACTIVE)){
                    ret = (grp_status == ERPS_PROTECTION_GROUP_ACTIVE) ? ERPS_ERROR_CANNOT_ASSOCIATE_GROUP :
                                                                         ERPS_ERROR_GROUP_NOT_EXISTS;
                } else {
                    memset(&pconf, 0, sizeof(pconf));
                    pconf.group_id = group_id;
                    if (vtss_erps_get_protection_group_by_id(&pconf) == VTSS_RC_OK) {
                        /* register with mep */
                        ret = erps_mep_aps_sf_register(pconf.erpg.east_port, pconf.erpg.west_port,
                                                       pconf.group_id, TRUE, pconf.erpg.virtual_channel);
                        if (ret != VTSS_RC_OK) {
                            T_D("vtss_erps_mep_aps_sf_register failed for group = %u\n", pconf.group_id);
                        }
                        ret = vtss_erps_associate_group (group_id);
                        if (ret == VTSS_RC_OK) {
                            /* register with MEP for incoming signal */
                            erps_instance_signal_in(group_id);
                        }
                    }
                }
            }
            break;
        case ERPS_CMD_SET_RPL_NEIGHBOUR:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_set_rpl_neighbour group_id = %u neighbour port = %u ", \
                                  group_id, mgmt_req->data.rpl_block.rpl_port);
            ret = vtss_erps_set_rpl_neighbour(group_id,
                                              mgmt_req->data.rpl_block.rpl_port);
            break;
        case ERPS_CMD_UNSET_RPL_NEIGHBOUR:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_unset_rpl_neighbour group_id = %u", group_id);
            ret = vtss_erps_unset_rpl_neighbour(group_id);
            break;
        case ERPS_CMD_REPLACE_RPL_BLOCK:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_set_rpl_block group_id = %u neighbour port = %u ", \
                                  group_id, mgmt_req->data.rpl_block.rpl_port);
            ret = vtss_erps_set_rpl_owner (group_id,
                                           mgmt_req->data.rpl_block.rpl_port, TRUE );
            break;
        case ERPS_CMD_ENABLE_INTERCONNECTED_NODE:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_set_rpl_block group_id = %u major_ring_id = %u ", \
                                  group_id, mgmt_req->data.inter_connected.major_ring_id);
            ret = vtss_erps_enable_interconnected(group_id,
                                                   CONV_MGMTTOERPS_INSTANCE(mgmt_req->data.inter_connected.major_ring_id));
            break;
        case ERPS_CMD_DISABLE_INTERCONNECTED_NODE:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_set_rpl_block group_id = %u major_ring_id = %u ", \
                                  group_id, mgmt_req->data.inter_connected.major_ring_id);
            ret = vtss_erps_disable_interconnected(group_id,
                                                   CONV_MGMTTOERPS_INSTANCE(mgmt_req->data.inter_connected.major_ring_id));
            break;
        case ERPS_CMD_ENABLE_NON_REVERTIVE:
            ret = vtss_erps_enable_non_reversion(group_id);
            break;
        case ERPS_CMD_DISABLE_NON_REVERTIVE:
            ret = vtss_erps_disable_non_reversion(group_id);
            break;
        case ERPS_CMD_FORCED_SWITCH:
            ret = vtss_erps_admin_command(group_id, ERPS_ADMIN_CMD_FORCED_SWITCH,
                                          mgmt_req->data.create.east_port);
            break;
        case ERPS_CMD_MANUAL_SWITCH:
            ret = vtss_erps_admin_command(group_id, ERPS_ADMIN_CMD_MANUAL_SWITCH,
                                          mgmt_req->data.create.east_port);
            break;
        case ERPS_CMD_CLEAR:
            ret = vtss_erps_admin_command(group_id, ERPS_ADMIN_CMD_CLEAR, 0);
            break;
        case ERPS_CMD_TOPOLOGY_CHANGE_PROPAGATE:
            ret = vtss_erps_set_topology_change_propogation(group_id, TRUE);
            break;
        case ERPS_CMD_TOPOLOGY_CHANGE_NO_PROPAGATE:
            ret = vtss_erps_set_topology_change_propogation(group_id, FALSE);
            break;
        case ERPS_CMD_SUB_RING_WITH_VIRTUAL_CHANNEL:
            ret = vtss_erps_enable_virtual_channel(group_id);
            break;
        case ERPS_CMD_SUB_RING_WITHOUT_VIRTUAL_CHANNEL:
            ret = vtss_erps_disable_virtual_channel(group_id);
            break;
        case ERPS_CMD_ENABLE_VERSION_1_COMPATIBLE:
            ret = vtss_erps_set_protocol_version(group_id, ERPS_VERSION_V1);
            break;
        case ERPS_CMD_DISABLE_VERSION_1_COMPATIBLE:
            ret = vtss_erps_set_protocol_version(group_id, ERPS_VERSION_V2);
            break;
        case ERPS_CMD_CLEAR_STATISTICS:
            T_NG(TRACE_GRP_MGMT, "vtss_erps_clear_statistics group_id = %u", group_id);
            ret = vtss_erps_clear_statistics(group_id);
            break;
        default:
            break;
    }

    erps_save_configuration();
    return (ret);
}

vtss_rc erps_mgmt_getnext_protection_group_request (vtss_erps_mgmt_conf_t *mgmt_req)
{
   vtss_rc ret = VTSS_RC_ERROR;
   vtss_erps_base_conf_t  base_conf;
   erps_mep_t  ermep;

   memset(&base_conf, 0, sizeof(base_conf));
//   ERPS_CRITD_ENTER();
   base_conf.group_id = mgmt_req->group_id;
   ret = vtss_erps_getnext_protection_group_by_id(&base_conf);
   mgmt_req->group_id = base_conf.group_id;
   memcpy(&mgmt_req->data.get.erpg, &base_conf.erpg, sizeof(vtss_erps_config_erpg_t));
   memcpy(&mgmt_req->data.get.stats, &base_conf.stats, sizeof(vtss_erps_fsm_stat_t));
   memcpy(&mgmt_req->data.get.raps_stats, &base_conf.raps_stats, sizeof(vtss_erps_statistics_t));
//   ERPS_CRITD_EXIT();

   ermep = vtss_get_erpg_to_mep(mgmt_req->group_id);

   mgmt_req->group_id = CONV_ERPSTOMGMT_INSTANCE(mgmt_req->group_id);
   mgmt_req->data.get.erpg.group_id = CONV_ERPSTOMGMT_INSTANCE (mgmt_req->data.get.erpg.group_id);

   mgmt_req->data.mep.east_mep_id = ermep.ccm_mep_id[ERPS_ARRAY_OFFSET_EAST];
   mgmt_req->data.mep.west_mep_id = ermep.ccm_mep_id[ERPS_ARRAY_OFFSET_WEST];
   mgmt_req->data.mep.raps_eastmep = ermep.raps_mep_id[ERPS_ARRAY_OFFSET_EAST];
   mgmt_req->data.mep.raps_westmep = ermep.raps_mep_id[ERPS_ARRAY_OFFSET_WEST];

   return (ret);
}

vtss_rc erps_mgmt_getexact_protection_group_by_id (vtss_erps_mgmt_conf_t *mgmt_req)
{
   vtss_rc ret = VTSS_RC_ERROR;
   vtss_erps_base_conf_t  base_conf;
   erps_mep_t  ermep;

   memset(&base_conf, 0, sizeof(base_conf));

//   ERPS_CRITD_ENTER();
   base_conf.group_id = mgmt_req->group_id;
   ret = vtss_erps_get_protection_group_by_id(&base_conf);
   mgmt_req->group_id = base_conf.group_id;
   memcpy(&mgmt_req->data.get.erpg, &base_conf.erpg, sizeof(vtss_erps_config_erpg_t));
   memcpy(&mgmt_req->data.get.stats, &base_conf.stats, sizeof(vtss_erps_fsm_stat_t));
   memcpy(&mgmt_req->data.get.raps_stats, &base_conf.raps_stats, sizeof(vtss_erps_statistics_t));
//   ERPS_CRITD_EXIT();

   ermep = vtss_get_erpg_to_mep(mgmt_req->group_id);

   mgmt_req->group_id = CONV_ERPSTOMGMT_INSTANCE(mgmt_req->group_id);
   mgmt_req->data.get.erpg.group_id = CONV_ERPSTOMGMT_INSTANCE (mgmt_req->data.get.erpg.group_id);

   mgmt_req->data.mep.east_mep_id = ermep.ccm_mep_id[ERPS_ARRAY_OFFSET_EAST];
   mgmt_req->data.mep.west_mep_id = ermep.ccm_mep_id[ERPS_ARRAY_OFFSET_WEST];
   mgmt_req->data.mep.raps_eastmep = ermep.raps_mep_id[ERPS_ARRAY_OFFSET_EAST];
   mgmt_req->data.mep.raps_westmep = ermep.raps_mep_id[ERPS_ARRAY_OFFSET_WEST];

   return (ret);
}


/******************************************************************************
   Misc functions
******************************************************************************/

#ifdef VTSS_SW_OPTION_ICFG

static vtss_rc erps_icfg_conf_group(const vtss_icfg_query_request_t  *req,
                                    vtss_icfg_query_result_t         *result,
                                    const vtss_erps_mgmt_conf_t      *conf)
{
    const vtss_erps_config_erpg_t *erpg = &conf->data.get.erpg;
    u32                           group = conf->group_id;
    vtss_usid_t                   usid  = topo_isid2usid(msg_master_isid());
    char                          port0[40], port1[40];
    u32                           i;
    BOOL                          sub_interconnected;

    sub_interconnected = erpg->ring_type == ERPS_RING_TYPE_SUB  &&  erpg->inter_connected_node;

    (void) icli_port_info_txt(usid, erpg->east_port, port0);
    (void) icli_port_info_txt(usid, erpg->west_port, port1);

    if (erpg->ring_type == ERPS_RING_TYPE_MAJOR) {
        VTSS_RC(vtss_icfg_printf(result, "erps %d major port0 interface %s port1 interface %s%s\n",
                                 group, port0, port1, (erpg->inter_connected_node ? " interconnect" : "")));
        
    } else {
        if (erpg->inter_connected_node) {
            VTSS_RC(vtss_icfg_printf(result, "erps %d sub port0 interface %s interconnect %d%s\n",
                                     group, port0, erpg->major_ring_id, (erpg->virtual_channel ? " virtual-channel" : "")));
        } else {
            VTSS_RC(vtss_icfg_printf(result, "erps %d sub port0 interface %s port1 interface %s\n",
                                     group, port0, port1));
        }
    }

    BOOL have_meps = conf->data.mep.east_mep_id && conf->data.mep.raps_eastmep;

    if (req->all_defaults && !have_meps) {
        VTSS_RC(vtss_icfg_printf(result, "no erps %d mep\n", group));
    } else if (have_meps) {
        if (sub_interconnected) {
            if (erpg->virtual_channel) {
                // Interconnected sub-ring with virtual channel has an APS MEP "on port1"
                VTSS_RC(vtss_icfg_printf(result, "erps %d mep port0 sf %d aps %d port1 aps %d\n",
                                         group,
                                         conf->data.mep.east_mep_id, conf->data.mep.raps_eastmep, conf->data.mep.raps_westmep));
            } else {
                // Interconnected sub-ring with virtual channel does not have any MEPs on port 1
                VTSS_RC(vtss_icfg_printf(result, "erps %d mep port0 sf %d aps %d\n",
                                         group,
                                         conf->data.mep.east_mep_id, conf->data.mep.raps_eastmep));
            }
        } else {
            VTSS_RC(vtss_icfg_printf(result, "erps %d mep port0 sf %d aps %d port1 sf %d aps %d\n",
                                     group,
                                     conf->data.mep.east_mep_id, conf->data.mep.raps_eastmep,
                                     conf->data.mep.west_mep_id, conf->data.mep.raps_westmep));
        }
    }

    if (req->all_defaults || erpg->version != ERPS_VERSION_V2) {
        VTSS_RC(vtss_icfg_printf(result, "erps %d version %d\n", group,
                                 (erpg->version == ERPS_VERSION_V1 ? 1 : 2)));
    }

    if (req->all_defaults && !erpg->rpl_owner && !erpg->rpl_neighbour) {
        VTSS_RC(vtss_icfg_printf(result, "no erps %d rpl\n", group));
    } else {
        if (erpg->rpl_owner) {
            VTSS_RC(vtss_icfg_printf(result, "erps %d rpl owner %s\n", group,
                                     (erpg->rpl_owner_port == erpg->east_port ? "port0" : "port1")));
        } else if (erpg->rpl_neighbour) {
            VTSS_RC(vtss_icfg_printf(result, "erps %d rpl neighbor %s\n", group,
                                     (erpg->rpl_neighbour_port == erpg->east_port ? "port0" : "port1")));
        }
    }

    if (req->all_defaults || erpg->hold_off_time != 0) {
        VTSS_RC(vtss_icfg_printf(result, "erps %d holdoff %" PRIu64 "\n", group, erpg->hold_off_time));
    }

    if (req->all_defaults || erpg->guard_time != RAPS_GUARDTIMEOUT_DEFAULT_MILLISECONDS) {
        VTSS_RC(vtss_icfg_printf(result, "erps %d guard %" PRIu64 "\n", group, erpg->guard_time));
    }

    if (erpg->revertive) {
        if (req->all_defaults  ||  erpg->wtr_time != 1) {
            VTSS_RC(vtss_icfg_printf(result, "erps %d revertive %" PRIu64 "\n", group, erpg->wtr_time));
        }
    } else {
        VTSS_RC(vtss_icfg_printf(result, "no erps %d revertive\n", group));
    }

    if (req->all_defaults && !erpg->topology_change) {
        VTSS_RC(vtss_icfg_printf(result, "no erps %d topology-change propagate\n", group));
    } else if (erpg->topology_change) {
        VTSS_RC(vtss_icfg_printf(result, "erps %d topology-change propagate\n", group));
    }

    // VLANs

    for (i = 0; i < PROTECTED_VLANS_MAX  &&  !erpg->protected_vlans[i]; i++) {
        // loop
    }
    BOOL has_vlans = i < PROTECTED_VLANS_MAX;

    if (req->all_defaults  &&  !has_vlans) {
        VTSS_RC(vtss_icfg_printf(result, "no erps %d vlan\n", group));
    } else if (has_vlans) {
        const char *separator = "";
        VTSS_RC(vtss_icfg_printf(result, "erps %d vlan ", group));
        for (i = 0; i < PROTECTED_VLANS_MAX; i++) {
            if (erpg->protected_vlans[i]) {
                VTSS_RC(vtss_icfg_printf(result, "%s%d", separator, erpg->protected_vlans[i]));
                separator = ",";
            }
        }
        VTSS_RC(vtss_icfg_printf(result, "\n"));
    }

    return VTSS_RC_OK;
}

static vtss_rc erps_icfg_conf(const vtss_icfg_query_request_t  *req,
                              vtss_icfg_query_result_t         *result)
{
    vtss_erps_mgmt_conf_t conf_req;
    u32                   instance = 0;
    vtss_rc               rc;

    memset(&conf_req, 0, sizeof(conf_req));

    while (instance <= ERPS_MAX_PROTECTION_GROUPS  &&  erps_mgmt_getnext_protection_group_request(&conf_req) == VTSS_RC_OK) {
        rc = erps_icfg_conf_group(req, result, &conf_req);
        if (rc != VTSS_RC_OK) {
            return rc;
        }
        instance = conf_req.group_id;
        memset(&conf_req, 0, sizeof(conf_req));
        conf_req.group_id = instance;
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_ICFG */

/******************************************************************************
                               OS Resources 
******************************************************************************/
/* ERPS main thread */
static void erps_run_thread(cyg_addrword_t data)
{
    vtss_rc   ret;
    vtss_port_no_t  port = 0;
    for (;;) {
        cyg_tick_count_t wakeup = cyg_current_time() + (10/ECOS_MSECS_PER_HWTICK);
        raps_pdu_in_t *req;
        while((req = cyg_mbox_timed_get(erps_mbhandle, wakeup))) {

            req->mep_id = CONV_ERPSTOMGMT_INSTANCE(req->mep_id);

            T_NG(TRACE_GRP_ERPS_RX, "ERPS PDU received on mep_id = %u", req->mep_id);
            T_NG_HEX(TRACE_GRP_ERPS_RX, req->pdu, ERPS_PDU_SIZE);

            if (port_to_mep[req->erpg].raps_mep_id[ERPS_ARRAY_OFFSET_EAST] == req->mep_id) {
                port = port_to_mep[req->erpg].port[ERPS_ARRAY_OFFSET_EAST];
                ret = vtss_erps_rx(port, req->len, req->pdu, req->erpg);
                if ( ret != ERPS_RC_OK ) {
                    T_D("error in erps_rx = %d", ret);
                }
            } else if (port_to_mep[req->erpg].raps_mep_id[ERPS_ARRAY_OFFSET_WEST] == req->mep_id) {
                port = port_to_mep[req->erpg].port[ERPS_ARRAY_OFFSET_WEST];
                ret = vtss_erps_rx(port, req->len, req->pdu, req->erpg);
                if ( ret != ERPS_RC_OK ) {
                    T_D("error in erps_rx = %d", ret);
                }
            } 
            VTSS_FREE(req);
        }
        /* Upon timeout - run timer vtss_erps_timer_thread() */
        vtss_erps_timer_thread();
    }
}

/******************************************************************************
 				Module Init Function 
******************************************************************************/
vtss_rc erps_init(vtss_init_data_t *data)
{
    vtss_isid_t     isid = data->isid; 
    erps_config_blk_t  *blk;
    u32           size;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }


    switch (data->cmd)
    {
        case INIT_CMD_INIT:
            T_D("INIT ERPS");
            T_D("initializing ERPS module");

            /* initialize critd */
            critd_init(&crit, "ERPS Crit", VTSS_MODULE_ID_ERPS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
            critd_init(&crit_p, "ERPS supp Crit", VTSS_MODULE_ID_ERPS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
            critd_init(&crit_fsm, "ERPS FSM Crit", VTSS_MODULE_ID_ERPS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

            /* semaphore initialization go here */
#ifdef VTSS_SW_OPTION_VCLI
            erps_cli_req_init();
#endif
            cyg_mbox_create(&erps_mbhandle, &erps_mbox);

            cyg_thread_create(THREAD_HIGHEST_PRIO,
                              erps_run_thread,
                              0,
                              "ERPS State Machine",
                              run_thread_stack,
                              sizeof(run_thread_stack),
                              &run_thread_handle,
                              &run_thread_block);
            cyg_thread_resume(run_thread_handle);

            CRIT_EXIT(crit);
            CRIT_EXIT(crit_p);
            CRIT_EXIT(crit_fsm);
            erps_init_done = FALSE;
#ifdef VTSS_SW_OPTION_ICFG
            VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_ERPS_GLOBAL_CONF, "erps", erps_icfg_conf));
#endif
            break;
        case INIT_CMD_CONF_DEF:
            T_D("CONF_DEF");
            if (isid == VTSS_ISID_LOCAL)
            {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
                blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ERPS_CONF, &size);
                set_conf_to_default(blk);
                apply_configuration(blk);
                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ERPS_CONF);
#else
                erps_restore_to_default();
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            }
            break;
        case INIT_CMD_MASTER_UP:
            T_D("MASTER_UP");
            if (misc_conf_read_use()) {
                if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ERPS_CONF, &size)) == NULL || size != sizeof(*blk)) {
                    T_W("conf_sec_open failed or size mismatch, creating defaults");
                    blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_ERPS_CONF, sizeof(*blk));
                    if (blk) {
                        set_conf_to_default(blk);
                    }
                }
                if (blk) {
                    apply_configuration(blk);
                }
                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ERPS_CONF);
            } else {
                erps_restore_to_default();
            }
            erps_init_done = TRUE;
            break;
        default:
            break;
    }

    T_D("exit");
    return 0;
}
