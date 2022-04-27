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


/*******************************************************************************
                                       #includes
*******************************************************************************/
#include "vtss_types.h"
#include "vtss_api.h"
#include "vtss_erps_api.h"
#include "vtss_erps.h"

/*lint -sem( vtss_erps_crit_lock, thread_lock ) */
/*lint -sem( vtss_erps_crit_unlock, thread_unlock ) */
/*lint -sem( vtss_erps_fsm_crit_lock, thread_lock ) */
/*lint -sem( vtss_erps_fsm_crit_unlock, thread_unlock ) */

/*******************************************************************************
                                       #defines
*******************************************************************************/
/* 
 * NOTE::: These macros should only be called from event handlers,
 *         event handler are invoked after taking mutual exclusion, thus no need
 *         to worry about ciritical sections inside the macros.In case a need 
 *         persists for inovking these marcors in non-event handler function, 
 *         use corresponding functions directly inorder to avoid problems.
 */

#define VTSS_ERPS_BLOCK_RING_PORT(port) \
    if ((port) != 0) { \
    vtss_erps_trace("<FSM>:block bport =", port);\
    ret = vtss_erps_protection_group_state_set(erpg->group_id, port, VTSS_ERPS_STATE_DISCARDING ); \
    if (ret !=  VTSS_RC_OK) { \
        vtss_erps_trace("<FSM>:erps_protection_group_state_set grp = ", erpg->group_id);\
        return (ERPS_ERROR_MOVING_TO_DISCARDING);\
    }\
    erpg->erps_instance.current_blocked = port; \
    if ((port == erpg->blocked_port) && (erpg->rpl_owner || erpg->rpl_neighbour)) \
        erpg->rpl_blocked = RAPS_RPL_BLOCKED; \
    if (port == erpg->east_port) erpg->erps_instance.east_blocked = port; \
    else                         erpg->erps_instance.west_blocked = port; \
    memset(erpg->erps_instance.stored_fl[0].node_id,0,ERPS_MAX_NODE_ID_LEN); \
    erpg->erps_instance.stored_fl[0].bpr = 0; \
    memset(erpg->erps_instance.stored_fl[1].node_id,0,ERPS_MAX_NODE_ID_LEN); \
        erpg->erps_instance.stored_fl[1].bpr = 0; \
    }

#define VTSS_ERPS_UNBLOCK_RING_PORT(port) \
    if ((port) != 0) { \
    vtss_erps_trace("<FSM>:unblock bport =", port);\
    ret = vtss_erps_protection_group_state_set (erpg->group_id,port,VTSS_ERPS_STATE_FORWARDING); \
    if (ret !=  VTSS_RC_OK) { \
        vtss_erps_trace("<FSM>:erps_protection_group_state_set grp =", erpg->group_id);\
        return (ERPS_ERROR_MOVING_TO_FORWARDING);\
    }\
    if (erpg->erps_instance.current_blocked == port)   erpg->erps_instance.current_blocked = 0; \
    if ((port == erpg->blocked_port) && (erpg->rpl_owner || erpg->rpl_neighbour)) { \
        erpg->rpl_blocked = RAPS_RPL_NON_BLOCKED; \
    } \
    if (port == erpg->east_port)  erpg->erps_instance.east_blocked = 0; \
    else                          erpg->erps_instance.west_blocked = 0; \
    }

#define VTSS_ERPS_STOP_TX_RAPS \
    ret = vtss_erps_raps_transmission (erpg->east_port,erpg->group_id,FALSE); \
    if (ret != VTSS_RC_OK) { \
        vtss_erps_trace("disabing R-APS failed for group =",erpg->group_id); \
        return (ERPS_ERROR_RAPS_DISABLE_RAPS_TX_FAILED); \
    } \
    if ((erpg->west_port != 0) || erpg->raps_virt_channel) { \
    ret = vtss_erps_raps_transmission (erpg->west_port,erpg->group_id,FALSE); \
    if (ret != VTSS_RC_OK) { \
        vtss_erps_trace("disabing R-APS failed for group =",erpg->group_id); \
        return (ERPS_ERROR_RAPS_DISABLE_RAPS_TX_FAILED); \
    } \
    } \
    erpg->erps_instance.raps_tx = ERPS_STOP_RAPS_TX;

#define VTSS_ERPS_START_TX_RAPS \
    ret = vtss_erps_raps_transmission (erpg->east_port,erpg->group_id,TRUE); \
    if (ret != VTSS_RC_OK) { \
        vtss_erps_trace("enabing R-APS failed for group =",erpg->group_id); \
        return (ERPS_ERROR_RAPS_DISABLE_RAPS_TX_FAILED); \
    } \
    if ((erpg->west_port != 0) || erpg->raps_virt_channel) { \
    ret = vtss_erps_raps_transmission (erpg->west_port,erpg->group_id,TRUE); \
    if (ret != VTSS_RC_OK) { \
        vtss_erps_trace("enabling R-APS failed for group =",erpg->group_id); \
        return (ERPS_ERROR_RAPS_DISABLE_RAPS_TX_FAILED); \
    } \
    } \
    erpg->erps_instance.raps_tx = ERPS_START_RAPS_TX;

#define VTSS_ERPS_ENABLE_RAPS_FORWARDING(port) \
    if (((port) != 0) || erpg->raps_virt_channel) { \
    if (vtss_erps_raps_forwarding(port,erpg->group_id,TRUE) != VTSS_RC_OK) { \
        vtss_erps_trace(" Error in enabling forwarding on groupId =",erpg->group_id); \
        return (ERPS_ERROR_RAPS_ENABLE_FORWARDING_FAILED); \
        } \
    }

#define VTSS_ERPS_DISABLE_RAPS_FORWARDING(port) \
    if (((port) != 0) || erpg->raps_virt_channel) { \
        if (vtss_erps_raps_forwarding(port,erpg->group_id,FALSE) != VTSS_RC_OK) { \
            vtss_erps_trace(" Error in enabling forwarding on groupId =",erpg->group_id); \
            return (ERPS_ERROR_RAPS_ENABLE_FORWARDING_FAILED); \
        } \
    }

#define VTSS_ERPS_BUILD_NEW_RAPS_PDU(req_state, sub_code, rb, dnf)  \
    u32 raps_state; \
    raps_state = req_state; \
    if (vtss_erps_build_raps_pkt(erpg, rapspdu, req_state, sub_code, rb, dnf, (erpg->erps_instance.east_blocked) ? 0 : 1) == ERPS_RC_OK) { \
        if (raps_state == ERPS_REQ_EVENT) { \
            ret = vtss_erps_raps_tx(erpg->east_port, erpg->group_id, rapspdu, TRUE); \
        } else { \
            ret = vtss_erps_raps_tx(erpg->east_port, erpg->group_id, rapspdu, FALSE); \
        } \
        if (ret != VTSS_RC_OK) { \
            vtss_erps_trace("Error in setting aps tx info group =", erpg->group_id); \
        } \
    } \
    if ((erpg->west_port != 0) || erpg->raps_virt_channel) { \
        if (vtss_erps_build_raps_pkt(erpg, rapspdu, req_state, sub_code, rb, dnf, (erpg->erps_instance.east_blocked) \
                                     ? 0 : 1) == ERPS_RC_OK) { \
            if (raps_state == ERPS_REQ_EVENT) { \
                ret = vtss_erps_raps_tx(erpg->west_port, erpg->group_id, rapspdu, TRUE); \
            } else { \
                ret = vtss_erps_raps_tx(erpg->west_port, erpg->group_id, rapspdu, FALSE); \
            } \
        if (ret != VTSS_RC_OK) { \
            vtss_erps_trace("Error in setting aps tx info group =", erpg->group_id); \
        } \
        } \
    } 

/* G.8032 Sec: 10.1.12 - When topology change in sub-ring results in FDB flush, topology_change 
   signal for this sub-ring is enabled for the below time after which it will be disabled */
#define VTSS_ERPS_FLUSH_FDB \
       { \
           u32 tmp = 0;\
           for (tmp = 0; tmp < PROTECTED_VLANS_MAX ; tmp++) { \
               if (!erpg->protected_vlans[tmp]) continue; \
               if (vtss_erps_flush_fdb(erpg->blocked_port,erpg->protected_vlans[tmp]) != VTSS_RC_OK) { \
                   vtss_erps_trace("error in fdb flush group=", erpg->group_id); \
               } \
           } \
           vtss_erps_trace("VTSS_ERPS_FLUSH_FDB called", erpg->group_id); \
           if ((erpg->ring_type == ERPS_RING_TYPE_SUB) && (erpg->interconnected_node)) { \
               if (erpg->topology_change == FALSE) { \
                   erpg->topology_change = TRUE; \
                   erpg->tc_running = 1; \
                   erpg->tc_timeout = ERPS_TOPOLOGY_CHANGE_TIMEOUT; \
                   if ((vtss_erps_apply_interconnected_flush_logic(erpg->major_ring_id, erpg->topology_propogate)) != VTSS_RC_OK) { \
                       vtss_erps_trace("Error in flushing FDB for major ring", 0); \
                   } \
               } \
           } \
       }


/*******************************************************************************
                                 File scope globals
*******************************************************************************/
static erps_protection_group_t   erpg_instances[ERPS_MAX_PROTECTION_GROUPS];
static i32 erps_handle_local_event ( i32, u32, vtss_port_no_t );


/******************************************************************************
                                  FINITE STATE MACHINE
*******************************************************************************/
struct {
  u8  event;    // This is not used for anything
  i32 (*fptr)(erps_protection_group_t *);
} ERPS_FSM[ERPS_STATE_MAX][FSM_EVENTS_MAX] =  {

  /* The order is very important. This order should always be same as defined in events enum. 
     Don't change this!!! */
  /* IDLE STATE */
  {
    { FSM_EVENT_LOCAL_CLEAR,           erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_FS,              erps_idle_event_handle_local_fs              },
    { FSM_EVENT_REMOTE_FS,             erps_idle_event_handle_remote_fs             },
    { FSM_EVENT_LOCAL_SF,              erps_idle_event_handle_local_sf              },
    { FSM_EVENT_LOCAL_CLEAR_SF,        erps_idle_event_handle_local_clear_sf        },
    { FSM_EVENT_REMOTE_SF,             erps_idle_event_handle_remote_sf             },
    { FSM_EVENT_REMOTE_MS,             erps_idle_event_handle_remote_ms             },
    { FSM_EVENT_LOCAL_MS,              erps_idle_event_handle_local_ms              },
    { FSM_EVENT_LOCAL_WTR_EXPIRES,     erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_WTR_RUNNING,     erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_WTB_EXPIRES,     erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_WTB_RUNNING,     erps_event_not_applicable                    },
    { FSM_EVENT_REMOTE_NR_RB,          erps_idle_event_handle_remote_nr_rb          },
    { FSM_EVENT_REMOTE_NR,             erps_idle_event_handle_remote_nr             },
    { FSM_EVENT_REMOTE_EVENT,          erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_HOLDOFF_EXPIRES, erps_idle_event_handle_local_holdoff_expires },
  },

  /* PROTECTED STATE */
  {
    { FSM_EVENT_LOCAL_CLEAR,           erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_FS,              erps_prot_event_handle_local_fs              },
    { FSM_EVENT_REMOTE_FS,             erps_prot_event_handle_remote_fs             },
    { FSM_EVENT_LOCAL_SF,              erps_prot_event_handle_local_sf              },
    { FSM_EVENT_LOCAL_CLEAR_SF,        erps_prot_event_handle_local_clear_sf        },
    { FSM_EVENT_REMOTE_SF,             erps_event_not_applicable                    },
    { FSM_EVENT_REMOTE_MS,             erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_MS,              erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_WTR_EXPIRES,     erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_WTR_RUNNING,     erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_WTB_EXPIRES,     erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_WTB_RUNNING,     erps_event_not_applicable                    },
    { FSM_EVENT_REMOTE_NR_RB,          erps_prot_event_handle_remote_nr_rb          },
    { FSM_EVENT_REMOTE_NR,             erps_prot_event_handle_remote_nr             },
    { FSM_EVENT_REMOTE_EVENT,          erps_event_not_applicable                    },
    { FSM_EVENT_LOCAL_HOLDOFF_EXPIRES, erps_prot_event_handle_local_holdoff_expires },
  },
 
  /* FORCED SWITCH STATE */
  {
    { FSM_EVENT_LOCAL_CLEAR,           erps_fs_event_handle_local_clear            },
    { FSM_EVENT_LOCAL_FS,              erps_fs_event_handle_local_fs               },
    { FSM_EVENT_REMOTE_FS,             erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_SF,              erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_CLEAR_SF,        erps_fsm_event_ignore                       },
    { FSM_EVENT_REMOTE_SF,             erps_fsm_event_ignore                       },
    { FSM_EVENT_REMOTE_MS,             erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_MS,              erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_WTR_EXPIRES,     erps_event_not_applicable                   },
    { FSM_EVENT_LOCAL_WTR_RUNNING,     erps_event_not_applicable                   },
    { FSM_EVENT_LOCAL_WTB_EXPIRES,     erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_WTB_RUNNING,     erps_fsm_event_ignore                       },
    { FSM_EVENT_REMOTE_NR_RB,          erps_fs_event_handle_remote_nr_rb           },
    { FSM_EVENT_REMOTE_NR,             erps_fs_event_handle_remote_nr              },
    { FSM_EVENT_REMOTE_EVENT,          erps_event_not_applicable                   },
    { FSM_EVENT_LOCAL_HOLDOFF_EXPIRES, erps_fs_event_handle_local_holdoff_expires  },
  },

  /* MANUAL SWITCH STATE */
  {
    { FSM_EVENT_LOCAL_CLEAR,           erps_ms_event_handle_local_clear            },
    { FSM_EVENT_LOCAL_FS,              erps_ms_event_handle_local_fs               },
    { FSM_EVENT_REMOTE_FS,             erps_ms_event_handle_remote_fs              },
    { FSM_EVENT_LOCAL_SF,              erps_ms_event_handle_local_sf               },
    { FSM_EVENT_LOCAL_CLEAR_SF,        erps_ms_event_handle_local_clear_sf         },
    { FSM_EVENT_REMOTE_SF,             erps_ms_event_handle_remote_sf              },
    { FSM_EVENT_REMOTE_MS,             erps_ms_event_handle_remote_ms              },
    { FSM_EVENT_LOCAL_MS,              erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_WTR_EXPIRES,     erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_WTR_RUNNING,     erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_WTB_EXPIRES,     erps_fsm_event_ignore                       },
    { FSM_EVENT_LOCAL_WTB_RUNNING,     erps_fsm_event_ignore                       },
    { FSM_EVENT_REMOTE_NR_RB,          erps_ms_event_handle_remote_nr_rb           },
    { FSM_EVENT_REMOTE_NR,             erps_ms_event_handle_remote_nr              },
    { FSM_EVENT_REMOTE_EVENT,          erps_event_not_applicable                   },
    { FSM_EVENT_LOCAL_HOLDOFF_EXPIRES, erps_ms_event_handle_local_holdoff_expires  },
  },

  /* PENDING STATE */
  {
    { FSM_EVENT_LOCAL_CLEAR,           erps_pend_event_handle_local_clear          },
    { FSM_EVENT_LOCAL_FS,              erps_pend_event_handle_local_fs             },
    { FSM_EVENT_REMOTE_FS,             erps_pend_event_handle_remote_fs            },
    { FSM_EVENT_LOCAL_SF,              erps_pend_event_handle_local_sf             },
    { FSM_EVENT_LOCAL_CLEAR_SF,        erps_fsm_event_ignore                       },
    { FSM_EVENT_REMOTE_SF,             erps_pend_event_handle_remote_sf            },
    { FSM_EVENT_REMOTE_MS,             erps_pend_event_handle_remote_ms            },
    { FSM_EVENT_LOCAL_MS,              erps_pend_event_handle_local_ms             },
    { FSM_EVENT_LOCAL_WTR_EXPIRES,     erps_pend_event_handle_local_wtr_expires    },
    { FSM_EVENT_LOCAL_WTR_RUNNING,     erps_pend_event_handle_local_wtr_running    },
    { FSM_EVENT_LOCAL_WTB_EXPIRES,     erps_pend_event_handle_local_wtb_expires    },
    { FSM_EVENT_LOCAL_WTB_RUNNING,     erps_pend_event_handle_local_wtb_running    },
    { FSM_EVENT_REMOTE_NR_RB,          erps_pend_event_handle_remote_nr_rb         },
    { FSM_EVENT_REMOTE_NR,             erps_pend_event_handle_remote_nr            },
    { FSM_EVENT_REMOTE_EVENT,          erps_event_not_applicable                   },
    { FSM_EVENT_LOCAL_HOLDOFF_EXPIRES, erps_pend_event_handle_local_holdoff_expires},
  }
};

/*******************************************************************************
                       File scope private functions/static 
*******************************************************************************/

vtss_rc
vtss_erps_getnext_protection_group_by_id (vtss_erps_base_conf_t * erpg)
{

    u32 group_id = erpg->group_id;
    u32 count,i;

    if (group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (VTSS_RC_ERROR);
    }

    ERPS_CRITD_ENTER();
    for (count = group_id ; count <ERPS_MAX_PROTECTION_GROUPS; count++) {
        if (erpg_instances[count].erps_status != 
                               ERPS_PROTECTION_GROUP_INACTIVE) {
            erpg->group_id = erpg_instances[count].group_id;
            erpg->erpg.east_port = erpg_instances[count].east_port;
            erpg->erpg.west_port = erpg_instances[count].west_port;
            erpg->erpg.hold_off_time = erpg_instances[count].holdoff_time;
            erpg->erpg.wtr_time = MS_TO_MIN(erpg_instances[count].wtr_time);
            erpg->erpg.guard_time = erpg_instances[count].guard_time;
            erpg->erpg.rpl_owner = erpg_instances[count].rpl_owner;
            erpg->erpg.rpl_owner_port = erpg_instances[count].blocked_port;
            erpg->erpg.group_id = erpg_instances[count].group_id;
            erpg->erpg.rpl_neighbour = erpg_instances[count].rpl_neighbour;
            erpg->erpg.rpl_neighbour_port = erpg_instances[count].rpl_neighbour_port;
            for (i = 0 ; i < PROTECTED_VLANS_MAX ; i++) {
                if (erpg_instances[count].protected_vlans[i]) {
                    erpg->erpg.protected_vlans[i]=erpg_instances[count].protected_vlans[i];
                }
            }

            /* ERPS V2 related information */
            erpg->erpg.wtb_time  = erpg_instances[count].wtb_time; 
            erpg->erpg.ring_type = erpg_instances[count].ring_type; 
            erpg->erpg.inter_connected_node = erpg_instances[count].interconnected_node; 
            erpg->erpg.major_ring_id = erpg_instances[count].major_ring_id; 
            erpg->erpg.revertive = erpg_instances[count].revertive; 
            erpg->erpg.version = erpg_instances[count].erps_version; 
            erpg->erpg.topology_change = erpg_instances[count].topology_propogate; 
            erpg->erpg.virtual_channel = erpg_instances[count].raps_virt_channel; 

            /* copy erpg FSM related information here */
            erpg->stats.wtr_remaining_time = erpg_instances[count].erps_instance.wtr_timeout;
            erpg->stats.state = erpg_instances[count].erps_instance.current_state;
            if (erpg_instances[count].erps_status == ERPS_PROTECTION_GROUP_ACTIVE ) {
                erpg->stats.active = TRUE;
            } else {
                erpg->stats.active = FALSE;
            }

            if (erpg_instances[count].erps_instance.lsf_port_east) {
                erpg->stats.east_port_state = ERPS_PORT_STATE_SF;
            } else {
                erpg->stats.east_port_state = ERPS_PORT_STATE_OK;
            }

            if (erpg_instances[count].erps_instance.lsf_port_west) {
                erpg->stats.west_port_state = ERPS_PORT_STATE_SF;
            } else {
                erpg->stats.west_port_state = ERPS_PORT_STATE_OK;
            }

            erpg->stats.east_blocked = erpg_instances[count].erps_instance.east_blocked;
            erpg->stats.west_blocked = erpg_instances[count].erps_instance.west_blocked;
            erpg->stats.rpl_blocked = (erpg_instances[count].rpl_blocked == RAPS_RPL_BLOCKED);
            if (erpg_instances[count].erps_instance.admin_cmd == FSM_EVENT_LOCAL_FS) {
                erpg->stats.admin_cmd = 2;
            } else if (erpg_instances[count].erps_instance.admin_cmd == FSM_EVENT_LOCAL_MS) {
                erpg->stats.admin_cmd = 1;
            } else {
                erpg->stats.admin_cmd = 0;
            }
            erpg->stats.fop_alarm = erpg_instances[count].erps_instance.fop_alarm;

            erpg->stats.tx     = erpg_instances[count].erps_instance.raps_tx;
            erpg->stats.tx_req = erpg_instances[count].erps_instance.tx_req;
            erpg->stats.tx_rb  = erpg_instances[count].erps_instance.tx_rb;
            erpg->stats.tx_dnf = erpg_instances[count].erps_instance.tx_dnf;
            erpg->stats.tx_bpr = erpg_instances[count].erps_instance.tx_bpr;

            erpg->stats.rx[0]     = erpg_instances[count].erps_instance.raps_rx[0];
            erpg->stats.rx_rb[0]  = erpg_instances[count].erps_instance.rb[0];
            erpg->stats.rx_dnf[0] = erpg_instances[count].erps_instance.dnf[0];
            erpg->stats.rx_bpr[0] = erpg_instances[count].erps_instance.bpr[0];
            erpg->stats.rx_req[0] = erpg_instances[count].erps_instance.req[0];
            memcpy(&erpg->stats.rx_node_id[0][0], &erpg_instances[count].erps_instance.node_id[0][0], ERPS_MAX_NODE_ID_LEN);

            erpg->stats.rx[1]     = erpg_instances[count].erps_instance.raps_rx[1];
            erpg->stats.rx_rb[1]  = erpg_instances[count].erps_instance.rb[1];
            erpg->stats.rx_dnf[1] = erpg_instances[count].erps_instance.dnf[1];
            erpg->stats.rx_bpr[1] = erpg_instances[count].erps_instance.bpr[1];
            erpg->stats.rx_req[1] = erpg_instances[count].erps_instance.req[1];
            memcpy(&erpg->stats.rx_node_id[1][0], &erpg_instances[count].erps_instance.node_id[1][0], ERPS_MAX_NODE_ID_LEN);

            /* copy R-APS statisticts */
            memcpy(&erpg->raps_stats,
                    &erpg_instances[count].erps_instance.erps_stats,
                    sizeof(vtss_erps_statistics_t));

            ERPS_CRITD_EXIT();
            return (VTSS_RC_OK);
        }
    }
    ERPS_CRITD_EXIT();
    return (VTSS_RC_ERROR);
}

vtss_rc
vtss_erps_get_protection_group_by_id (vtss_erps_base_conf_t * erpg)
{
    u32 group_id = erpg->group_id;
    u32 count = erpg->group_id,i;

    if (group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (VTSS_RC_ERROR);
    }

    ERPS_CRITD_ENTER();
    if (erpg_instances[count].erps_status != ERPS_PROTECTION_GROUP_INACTIVE) {
        erpg->group_id = erpg_instances[count].group_id;
        erpg->erpg.east_port = erpg_instances[count].east_port;
        erpg->erpg.west_port = erpg_instances[count].west_port;
        erpg->erpg.hold_off_time = erpg_instances[count].holdoff_time;
        erpg->erpg.wtr_time = MS_TO_MIN(erpg_instances[count].wtr_time);
        erpg->erpg.guard_time = erpg_instances[count].guard_time;
        erpg->erpg.rpl_owner = erpg_instances[count].rpl_owner;
        erpg->erpg.rpl_owner_port = erpg_instances[count].blocked_port;
        erpg->erpg.group_id = erpg_instances[count].group_id;
        erpg->erpg.rpl_neighbour = erpg_instances[count].rpl_neighbour;
        erpg->erpg.rpl_neighbour_port = erpg_instances[count].rpl_neighbour_port;
        for (i = 0 ; i < PROTECTED_VLANS_MAX ; i++) {
            if (erpg_instances[count].protected_vlans[i]) {
                erpg->erpg.protected_vlans[i]=erpg_instances[count].protected_vlans[i];
            }
        }

        /* ERPS V2 related information */
        erpg->erpg.wtb_time  = erpg_instances[count].wtb_time; 
        erpg->erpg.ring_type = erpg_instances[count].ring_type; 
        erpg->erpg.inter_connected_node = erpg_instances[count].interconnected_node; 
        erpg->erpg.major_ring_id = erpg_instances[count].major_ring_id; 
        erpg->erpg.revertive = erpg_instances[count].revertive; 
        erpg->erpg.version = erpg_instances[count].erps_version; 
        erpg->erpg.topology_change = erpg_instances[count].topology_propogate; 
        erpg->erpg.virtual_channel = erpg_instances[count].raps_virt_channel; 

        /* copy erpg FSM related information here */
        erpg->stats.wtr_remaining_time = erpg_instances[count].erps_instance.wtr_timeout;
        erpg->stats.state = erpg_instances[count].erps_instance.current_state;
        if (erpg_instances[count].erps_status == ERPS_PROTECTION_GROUP_ACTIVE ) {
            erpg->stats.active = TRUE;
        } else {
            erpg->stats.active = FALSE;
        }

        if (erpg_instances[count].erps_instance.lsf_port_east) {
            erpg->stats.east_port_state = ERPS_PORT_STATE_SF;
        } else {
            erpg->stats.east_port_state = ERPS_PORT_STATE_OK;
        }

        if (erpg_instances[count].erps_instance.lsf_port_west) {
            erpg->stats.west_port_state = ERPS_PORT_STATE_SF;
        } else {
            erpg->stats.west_port_state = ERPS_PORT_STATE_OK;
        }

        erpg->stats.east_blocked = erpg_instances[count].erps_instance.east_blocked;
        erpg->stats.west_blocked = erpg_instances[count].erps_instance.west_blocked;
        erpg->stats.rpl_blocked = (erpg_instances[count].rpl_blocked == RAPS_RPL_BLOCKED);

        erpg->stats.tx     = erpg_instances[count].erps_instance.raps_tx;
        erpg->stats.tx_req = erpg_instances[count].erps_instance.tx_req;
        erpg->stats.tx_rb  = erpg_instances[count].erps_instance.tx_rb;
        erpg->stats.tx_dnf = erpg_instances[count].erps_instance.tx_dnf;

        erpg->stats.rx[0]     = erpg_instances[count].erps_instance.raps_rx[0];
        erpg->stats.rx_rb[0]  = erpg_instances[count].erps_instance.rb[0];
        erpg->stats.rx_dnf[0] = erpg_instances[count].erps_instance.dnf[0];
        erpg->stats.rx_bpr[0] = erpg_instances[count].erps_instance.bpr[0];
        erpg->stats.rx_req[0] = erpg_instances[count].erps_instance.req[0];
        memcpy(&erpg->stats.rx_node_id[0][0], &erpg_instances[count].erps_instance.node_id[0][0], ERPS_MAX_NODE_ID_LEN);

        erpg->stats.rx[1]     = erpg_instances[count].erps_instance.raps_rx[1];
        erpg->stats.rx_rb[1]  = erpg_instances[count].erps_instance.rb[1];
        erpg->stats.rx_dnf[1] = erpg_instances[count].erps_instance.dnf[1];
        erpg->stats.rx_bpr[1] = erpg_instances[count].erps_instance.bpr[1];
        erpg->stats.rx_req[1] = erpg_instances[count].erps_instance.req[1];
        memcpy(&erpg->stats.rx_node_id[1][0], &erpg_instances[count].erps_instance.node_id[1][0], ERPS_MAX_NODE_ID_LEN);

        /* copy R-APS statisticts */
        memcpy(&erpg->raps_stats,
               &erpg_instances[count].erps_instance.erps_stats,
               sizeof(vtss_erps_statistics_t));

        ERPS_CRITD_EXIT();
        return (VTSS_RC_OK);
    }
    ERPS_CRITD_EXIT();
    return (VTSS_RC_ERROR);
}

vtss_rc vtss_erps_sf_sd_state_set (const u32 instance, 
                                   const BOOL sf_state, 
                                   const BOOL sd_state,
                                   const vtss_port_no_t lport)
{
    vtss_rc ret = erps_handle_local_event(sf_state ? FSM_EVENT_LOCAL_SF : FSM_EVENT_LOCAL_CLEAR_SF,
                                          instance, lport);
    return ret == VTSS_RC_OK ? VTSS_RC_OK : VTSS_RC_ERROR;
}

/******************************************************************************
    Functions that are exposed to Platform for ERPS configuration
******************************************************************************/
i32 vtss_erps_create_protection_group(u32 erps_group_id,
                                      vtss_port_no_t east_port,
                                      vtss_port_no_t west_port,
                                      vtss_erps_ring_type_t ring_type,
                                      BOOL interconnected,
                                      u32 erps_major_group_id,
                                      BOOL virtual_channel)
{
    erps_protection_group_t         *erpg = NULL;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        vtss_erps_trace("erps_group_id is out of range for group",0);
        return (ERPS_ERROR_INVALID_PGID);
    }

    if (east_port == west_port) {
        vtss_erps_trace("East and West ports are same =",0);
        return ( ERPS_ERROR_EAST_AND_WEST_ARE_SAME );
    }

    /* west port can be 0 only for interconnected sub-ring */
    if (!west_port) {
        if (!(ring_type == ERPS_RING_TYPE_SUB && interconnected)) {
            vtss_erps_trace("West port can be zero only for interconnected sub-ring", 0 );
            return (ERPS_ERROR_PG_CREATION_FAILED);
        }
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_ACTIVE) {
        vtss_erps_trace("protection group is already active =", erps_group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_ALREADY_EXISTS);
    }

    erpg->east_port          = east_port;
    erpg->west_port          = west_port;
    erpg->erps_status        = ERPS_PROTECTION_GROUP_RESERVED;

    /* initializing with default values */
    erpg->rpl_owner          = ERPS_NODE_NON_RPL_OWNER;
    erpg->rpl_blocked        = RAPS_RPL_NON_BLOCKED;
    erpg->rpl_neighbour      = FALSE;

    erpg->erps_instance.current_blocked = RING_BLOCKED_NONE;
    erpg->erps_instance.current_state   = ERPS_STATE_NONE;

    erpg->erps_instance.lsf_port_east     = 0;
    erpg->erps_instance.lsf_port_west     = 0;

    erpg->erps_instance.east_blocked      = erpg->east_port;
    erpg->erps_instance.west_blocked      = erpg->west_port;

    erpg->erps_instance.dnf[0]             = ERPS_UNSET_DNF;
    erpg->erps_instance.dnf[1]             = ERPS_UNSET_DNF;

    /* need to initialize default values of holdoff/wtr/guard timer */
    erpg->wtr_time     = MIN_TO_MS(RAPS_WTR_TIMEOUT_MIN_MINUTES); 
    erpg->guard_time   = RAPS_GUARDTIMEOUT_DEFAULT_MILLISECONDS;
    erpg->holdoff_time = RAPS_HOLD_OFF_DEFAULT_TIMEOUT;
    erpg->wtb_time     = RAPS_WTBTIMEOUT_DEFAULT_MILLISECONDS;
    
    /* initialization of default values for revertion */
    erpg->revertive = TRUE; 

    /* initialization of default protocol version */
    erpg->erps_version = ERPS_VERSION_V2;

    /* updating ring-type */
    erpg->ring_type = ring_type;

    erpg->raps_virt_channel = virtual_channel;

    erpg->major_ring_id = erps_major_group_id;
    erpg->interconnected_node = interconnected;

    /* initializing flush logic default values */
    erpg->erps_instance.rcvd_fl[0].bpr = 0;
    erpg->erps_instance.rcvd_fl[1].bpr = 0;
    bzero(erpg->erps_instance.rcvd_fl[0].node_id,ERPS_MAX_NODE_ID_LEN);
    bzero(erpg->erps_instance.rcvd_fl[1].node_id,ERPS_MAX_NODE_ID_LEN);

    /* initialize FSM wtr & hold-off timeout values to ZERO */
    erpg->erps_instance.wtr_timeout         = 0;
    erpg->erps_instance.wtr_running         = 0;
    erpg->erps_instance.hold_off_timeout    = 0;
    erpg->erps_instance.hold_off_running    = 0;
    erpg->erps_instance.guard_timeout       = 0;
    erpg->erps_instance.guard_timer_running = 0;
    erpg->erps_instance.wtb_timeout         = 0;
    erpg->erps_instance.wtb_running         = 0;

    /*
     * updating group_id, the same is used as hw instance for setting protection
     * group state i.e discarding/forwarding 
     */
    erpg->group_id = erps_group_id;

    /* initialize number of configured protected vids to zero */
    erpg->p_vids_configured = 0;

    /* get port mac address and store it as node id */
    if (vtss_erps_get_port_mac(erpg->east_port,&erpg->node_id_e[0]) != VTSS_RC_OK) {
        vtss_erps_trace("error in getting east port mac address",0);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR);
    }

    /* incase of sub-ring interconnected node, west port can be 0 */
    if (erpg->west_port) {
        if (vtss_erps_get_port_mac(erpg->west_port,&erpg->node_id_w[0]) != VTSS_RC_OK) {
            vtss_erps_trace("error in getting east port mac address",0);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR);
        }
    }

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32 vtss_erps_remove_protected_vlan (u32 erps_group_id, u8  num_vids, 
                                     u16 protected_vid)
{
    erps_protection_group_t         *erpg;
    u16                         i;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if ( erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return ( ERPS_ERROR_VLANS_CANNOT_BE_DELETED );
    }

    for (i = 0; i < PROTECTED_VLANS_MAX; i++) {
        if (erpg->protected_vlans[i] == protected_vid) {
            erpg->protected_vlans[i] = 0;
            /* decrement number of protected vlans counter */
            erpg->p_vids_configured--;
        }
    }

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_ACTIVE) {
        /* need to remove the vlan from protected vlan set */
        if(vtss_erps_ctrl_set_vlanmap(protected_vid, erpg->group_id,FALSE) != VTSS_RC_OK) {
            vtss_erps_trace("erps_ctrl_set_vlanmap failed.. putting protection group status back to INACTIVE",0);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_FAILED_IN_SETTING_VLANMAP);
        }
    }
    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32 vtss_erps_get_protection_group_status(u32 erps_group_id, u8 *grp_status)
{
    erps_protection_group_t         *erpg;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];
    *grp_status = erpg->erps_status;
    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32 vtss_erps_delete_protection_group(u32 erps_group_id)
{
    erps_protection_group_t         *erpg;
    vtss_rc   ret = VTSS_RC_ERROR;
    u16       vid;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if ( erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE ) {
        ERPS_CRITD_EXIT();
        return ( ERPS_ERROR_GROUP_NOT_EXISTS );
    }

    erpg->erps_instance.raps_tx = ERPS_STOP_RAPS_TX;
    
    /* keep all vlans of this protection group in forwarding */
    ret = vtss_erps_protection_group_state_set (erpg->group_id,
                                           erpg->east_port,
                                           VTSS_ERPS_STATE_FORWARDING);

    if (ret != VTSS_RC_OK) {
        vtss_erps_trace("vtss_erps_protection_group_state_set failed for group =", erpg->group_id);
    }

    if (erpg->west_port) {
    ret = vtss_erps_protection_group_state_set (erpg->group_id,
                                                erpg->west_port,
                                                VTSS_ERPS_STATE_FORWARDING);
    }
    if (ret != VTSS_RC_OK) {
        vtss_erps_trace("vtss_erps_protection_group_state_set failed for group =", erpg->group_id);
    }

    /*
     * incase there are no protection groups on the given east and west port,
     * remove ingress filtering
     */
    if (erpg->east_port) {
        ret = vtss_erps_vlan_ingress_filter (erpg->east_port,FALSE);
        if (ret !=  VTSS_RC_OK) {
            vtss_erps_trace("<FSM>: error in erps_vlan_ingress_filter grp =",erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR);
        }
    }

    /* incase of interconnected sub-ring node, west_port is 0 */
    if (erpg->west_port) {
        ret = vtss_erps_vlan_ingress_filter (erpg->west_port,FALSE);
        if (ret !=  VTSS_RC_OK) {
            vtss_erps_trace("<FSM>: error in erps_vlan_ingress_filter grp =",erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR);
        }
    }

    /* Delete protection group vlan map  */
    for (vid = 0; vid < PROTECTED_VLANS_MAX ; vid++ ) {
        if (erpg->protected_vlans[vid]) {
            if(vtss_erps_ctrl_set_vlanmap(erpg->protected_vlans[vid], erpg->group_id, FALSE) != VTSS_RC_OK) {
                vtss_erps_trace("erps_ctrl_set_vlanmap failed",0);
                ERPS_CRITD_EXIT();
                return (ERPS_ERROR_FAILED_IN_SETTING_VLANMAP);
            }
        } /* if */
    } /* for */

    /* 
     * need to kept port forwarding status as FORWARDING, 
     * Since ERPS can block any of the ring ports which need to be unblocked here 
     */
    if (erpg->erps_instance.east_blocked) {
        ret = vtss_erps_protection_group_state_set (erpg->group_id,erpg->east_port,
                                                VTSS_ERPS_STATE_FORWARDING);

        if (ret !=  VTSS_RC_OK) {
            vtss_erps_trace("failing in putting blocked port in discarding state group =",erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_MOVING_TO_DISCARDING);
        }
    }

    if (erpg->erps_instance.west_blocked) {
        if (erpg->west_port) {
            ret = vtss_erps_protection_group_state_set (erpg->group_id,erpg->west_port,
                    VTSS_ERPS_STATE_FORWARDING);

            if (ret !=  VTSS_RC_OK) {
                vtss_erps_trace("failing in putting blocked port in discarding state group =",erpg->group_id);
                ERPS_CRITD_EXIT();
                return (ERPS_ERROR_MOVING_TO_DISCARDING);
            }
        }
    }

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_RESERVED) {
       /* 
        * This is just a reserved group, no resources are associated
        * with this, just change erps_status to 
        * ERPS_PROTECTION_GROUP_INACTIVE 
        */

         erpg->erps_status = ERPS_PROTECTION_GROUP_INACTIVE; 
         /* then delete the entire group */
         memset(&erpg_instances[erps_group_id],0,sizeof(erps_protection_group_t));
         ERPS_CRITD_EXIT();
         return (ERPS_RC_OK);
    }

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_ACTIVE) {
       /* 
        * This is in active state, some resources are associated
        * with this, need to release the resources first then 
        * delete all the data in that instance, resources can be
        * memory or any Operating System resources 
        */
         erpg->erps_status = ERPS_PROTECTION_GROUP_INACTIVE; 

         /* delete the entire group */
         memset(&erpg_instances[erps_group_id],0,sizeof(erps_protection_group_t));
    }


    vtss_erps_trace("return from vtss_erps_delete_protection_group =", ret);
    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32  vtss_erps_associate_protected_vlans (u32 erps_group_id, u16 protected_vid)
{
    erps_protection_group_t         *erpg;
    u16                             i;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return ( ERPS_ERROR_VLANS_CANNOT_BE_ADDED );
    }

    if (erpg->p_vids_configured >= PROTECTED_VLANS_MAX) {
        vtss_erps_trace("maximum number of vlans are configured for this group =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_MAXIMUM_PVIDS_REACHED);
    }

    /* check given vlan is already available in the protected vlan list */
    for ( i = 0; i < PROTECTED_VLANS_MAX; i++ ) {
        if ( erpg->protected_vlans[i] == protected_vid ) {
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_VLAN_ALREADY_PARTOF_GROUP);
        }
    }

    /* now add vlans to the given protection group */
    for ( i = 0; i < PROTECTED_VLANS_MAX; i++ ) {
        if (!erpg->protected_vlans[i]) {
            erpg->protected_vlans[i] = protected_vid; 
            
            /* increment number of protected vlans counter */
            erpg->p_vids_configured++;
            break;
        }
    }

    /* adding protected vlans while the group is in ACTIVE state */
    if (erpg->erps_status == ERPS_PROTECTION_GROUP_ACTIVE) {
        vtss_erps_trace("adding a vlan in PG ACTIVE State",0);
        if(vtss_erps_ctrl_set_vlanmap(protected_vid, erpg->group_id,TRUE) != VTSS_RC_OK) {
            vtss_erps_trace("erps_ctrl_set_vlanmap failed..",0);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_FAILED_IN_SETTING_VLANMAP);
        }
    }

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32 vtss_erps_set_rpl_owner(u32 erps_group_id, vtss_port_no_t port, u16 rpl_replace)
{
    erps_protection_group_t         *erpg;
    u16                              vid;
    vtss_rc                          ret;
    u8                               rapspdu[ERPS_PDU_SIZE];

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        vtss_erps_trace("errpr in setting rpl block for group =",erps_group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    if (port == 0) {
        vtss_erps_trace("RPL block port cannot be zero", 0);
        ERPS_CRITD_EXIT();
        return ( ERPS_ERROR_SETTING_RPL_BLOCK );
    }

    if (erpg->east_port != port && erpg->west_port != port) {
        vtss_erps_trace("RPL block is not part of east or west port =",port);
        ERPS_CRITD_EXIT();
        return ( ERPS_ERROR_SETTING_RPL_BLOCK );
    }

    if (erpg->rpl_owner) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_NODE_ALREADY_RPL_OWNER);
    }

    if (erpg->rpl_neighbour) {
        vtss_erps_trace("This node is aleady rpl neighbour for this group =",0);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_ALREADY_NEIGHBOUR_FOR_THISGROUP);
    }

    if (rpl_replace == TRUE) {
        if( erpg->erps_instance.current_state == ERPS_STATE_PROTECTED) {
            if (erpg->erps_instance.lsf_port_east || erpg->erps_instance.lsf_port_west) {
                erpg->rpl_owner    = ERPS_NODE_RPL_OWNER;
                erpg->rpl_blocked  = RAPS_RPL_BLOCKED;

                if (erpg->erps_instance.lsf_port_east == port) {
                    erpg->blocked_port = port;
                    erpg->erps_instance.current_blocked = erpg->blocked_port;
                } 
                if (erpg->erps_instance.lsf_port_west == port) {
                    erpg->blocked_port = port;
                    erpg->erps_instance.current_blocked = erpg->blocked_port;
                } 

                if (erpg->erps_instance.lsf_port_east != port) {
                    erpg->blocked_port = erpg->erps_instance.lsf_port_east;
                    erpg->erps_instance.current_blocked = erpg->blocked_port;
                }
                if (erpg->erps_instance.lsf_port_west != port) {
                    erpg->blocked_port = erpg->erps_instance.lsf_port_west;
                    erpg->erps_instance.current_blocked = erpg->blocked_port;
                }
            } else {
                erpg->rpl_owner    = ERPS_NODE_RPL_OWNER;
                erpg->rpl_blocked  = RAPS_RPL_BLOCKED;
                erpg->blocked_port = port;
                erpg->erps_instance.current_blocked = erpg->blocked_port;
            }

            /* if  WTR is running, cancel them */
            if (erpg->erps_instance.wtr_running) {
                erpg->erps_instance.wtr_running = 0;
                erpg->erps_instance.wtr_timeout = 0;
            }
            ERPS_CRITD_EXIT();
            return (ERPS_RC_OK);
        }
    }

    /* stop guard, WTR and WTB Timers */
    erpg->erps_instance.wtr_running = 0;
    erpg->erps_instance.wtr_timeout = 0;
    erpg->erps_instance.wtb_running = 0;
    erpg->erps_instance.wtb_timeout = 0;
    erpg->erps_instance.guard_timeout = 0;
    erpg->erps_instance.guard_timer_running = 0;

    erpg->rpl_owner    = ERPS_NODE_RPL_OWNER;
    erpg->rpl_blocked  = RAPS_RPL_BLOCKED;
    erpg->blocked_port = port;
    erpg->erps_instance.current_blocked = erpg->blocked_port;

    /* move protection group into IDLE state */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING; 

    /* create protection group vlan map  */
    for ( vid = 0; vid < PROTECTED_VLANS_MAX ; vid++ ) {
        if (erpg->protected_vlans[vid]) {
            if(vtss_erps_ctrl_set_vlanmap(erpg->protected_vlans[vid], erpg->group_id,TRUE) != VTSS_RC_OK) {
                /*
                 * some problem in setting vlan map, don't move Protection
                 * group into active state, i.e this group is not usable
                 */
                erpg->erps_status = ERPS_PROTECTION_GROUP_INACTIVE;
                vtss_erps_trace("erps_ctrl_set_vlanmap failed.. putting protection"
                        "group status back to INACTIVE",0);
                ERPS_CRITD_EXIT();
                return (ERPS_ERROR_FAILED_IN_SETTING_VLANMAP);
            }
        } /* if */
    } /* for */

    /* put blocked port into discarding state */
    ret = vtss_erps_protection_group_state_set (erpg->group_id,erpg->blocked_port,
                                          VTSS_ERPS_STATE_DISCARDING);
   
    if (ret !=  VTSS_RC_OK) {
       vtss_erps_trace("failing in putting blocked port in discarding \
                                       state group =",erpg->group_id);
       ERPS_CRITD_EXIT();
       return (ERPS_ERROR_MOVING_TO_DISCARDING);
    }

    erpg->rpl_blocked = RAPS_RPL_BLOCKED;
    (erpg->blocked_port == erpg->west_port)? \
    (erpg->erps_instance.west_blocked=erpg->west_port): \
    (erpg->erps_instance.east_blocked=erpg->east_port);

    /* 
     *  The default configuration is that all VLANs are disabled and all ports 
     *  are discarding for all ERPS instances. If the application (ERPS module) 
     *  enables a VLAN for an ERPS instance, it must setup the forwarding state 
     *  for all ports for the instance. 
     */
    ret = vtss_erps_put_protected_vlans_in_forwarding(erpg->group_id,erpg->blocked_port);
    if (ret != VTSS_RC_OK ) {
        vtss_erps_trace(" error in putting all port+vlan in forwarding mode",0);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR);
    }
    if (erpg->west_port) {
    /* put non-blocked port into forwarding state */  
    ret = vtss_erps_protection_group_state_set (erpg->group_id,\
               ((erpg->blocked_port == erpg->west_port) ? \
               erpg->east_port : erpg->west_port), VTSS_ERPS_STATE_FORWARDING);
    if (ret !=  VTSS_RC_OK) {
        vtss_erps_trace("failing in putting blocked port in forwarding state", 0);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_MOVING_TO_FORWARDING);
    }
    }

    if (erpg->west_port || erpg->raps_virt_channel) {
    (erpg->blocked_port == erpg->west_port)? \
    (erpg->erps_instance.east_blocked=0):(erpg->erps_instance.west_blocked=0);

    /* enable R-APS forwarding */
    if (vtss_erps_raps_forwarding(((erpg->blocked_port == erpg->west_port) \
                                  ? erpg->east_port : erpg->west_port),
                                  erpg->group_id,FALSE) != VTSS_RC_OK) {
        vtss_erps_trace(" Error in enabling forwarding for group =" , erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_RAPS_DISABLE_FORWARDING_FAILED);
    }
    }

    /* disable R-APS forwarding on rpl block */
    ret = vtss_erps_raps_forwarding(erpg->blocked_port,erpg->group_id,FALSE);
    if (ret != VTSS_RC_OK) {
        vtss_erps_trace("vtss_erps_raps_forwarding failed for group =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_RAPS_DISABLE_FORWARDING_FAILED);
    }

    /* enable R-APS transmission on both the ports */
    /* Enable transmission of R-APS before actually transmitting */
    ret = vtss_erps_raps_transmission(erpg->east_port,erpg->group_id,TRUE);
    if (ret != VTSS_RC_OK) {
        vtss_erps_trace("vtss_erps_raps_transmission failed for group =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_RAPS_ENABLE_RAPS_TX_FAILED);
    }

    if (erpg->west_port || erpg->raps_virt_channel) {
    ret = vtss_erps_raps_transmission(erpg->west_port,erpg->group_id,TRUE);
    if (ret != VTSS_RC_OK) {
        vtss_erps_trace("vtss_erps_raps_transmission failed for group =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_RAPS_ENABLE_RAPS_TX_FAILED);
    }
    }

    if (erpg->erps_instance.raps_tx == ERPS_STOP_RAPS_TX) {
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR, 0, FALSE, FALSE);
        erpg->erps_instance.raps_tx = ERPS_START_RAPS_TX;
    }

    /* incase of Revertive, start WTR Timer */
    if (erpg->revertive) {
        erpg->erps_instance.wtr_timeout = erpg->wtr_time;
        erpg->erps_instance.wtr_running = 1;
    }

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

static i32 vtss_erps_apply_interconnected_flush_logic(u32 major_ring_id, BOOL topology_propogate)
{
    u32                      count;
    erps_protection_group_t  *erpg;
    vtss_rc                  ret = VTSS_RC_ERROR;
    u8                       rapspdu[ERPS_PDU_SIZE];
    for (count = 0 ; count < ERPS_MAX_PROTECTION_GROUPS; count++) {
        if (erpg_instances[count].erps_status != ERPS_PROTECTION_GROUP_INACTIVE) {
            /* Apply flush logic for interconnected major ring */
            if ((erpg_instances[count].ring_type == ERPS_RING_TYPE_MAJOR) && 
                (erpg_instances[count].interconnected_node) &&
                (major_ring_id) &&
                (erpg_instances[count].group_id == (major_ring_id - 1))) {
                erpg = &erpg_instances[count];
                /* Flush major ring's FDB */
                VTSS_ERPS_FLUSH_FDB;
                /* Check whether the major ring is configured to propagate TC from this sub-ring */
                if (topology_propogate == TRUE) {
                    /* Tx R-APS (EVENT) */ 
                    /* VTSS_ERPS_START_TX_RAPS Cannot be used directly as mutex need to be released on error */
                    ret = vtss_erps_raps_transmission (erpg->east_port, erpg->group_id, TRUE);
                    if (ret != VTSS_RC_OK) {
                        vtss_erps_trace("enabing R-APS failed for group =",erpg->group_id);
                        return (ERPS_ERROR_RAPS_DISABLE_RAPS_TX_FAILED);
                    }
                    if (erpg->west_port || erpg->raps_virt_channel) {
                        ret = vtss_erps_raps_transmission (erpg->west_port, erpg->group_id, TRUE);
                        if (ret != VTSS_RC_OK) {
                            vtss_erps_trace("enabling R-APS failed for group =",erpg->group_id);
                            return (ERPS_ERROR_RAPS_DISABLE_RAPS_TX_FAILED);
                        }
                    }
                    erpg->erps_instance.raps_tx = ERPS_START_RAPS_TX;
                    /* Equivalent code for VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_EVENT, 0, FALSE, FALSE) */
                    if (vtss_erps_build_raps_pkt(erpg, rapspdu, ERPS_REQ_EVENT, 0, FALSE, FALSE, 
                                                 (erpg->erps_instance.east_blocked) ? 0 : 1) == ERPS_RC_OK) {
                        ret = vtss_erps_raps_tx(erpg->east_port, erpg->group_id, rapspdu, TRUE);
                        if (ret != VTSS_RC_OK) {
                            vtss_erps_trace("Error in setting aps tx info group =", erpg->group_id);
                        }
                    }
                    if (erpg->west_port || erpg->raps_virt_channel) {
                        if (vtss_erps_build_raps_pkt(erpg, rapspdu, ERPS_REQ_EVENT, 0, FALSE, FALSE, 
                                    (erpg->erps_instance.east_blocked) ? 0 : 1) == ERPS_RC_OK) {
                            ret = vtss_erps_raps_tx(erpg->west_port, erpg->group_id, rapspdu, TRUE);
                            if (ret != VTSS_RC_OK) {
                                vtss_erps_trace("Error in setting aps tx info group =", erpg->group_id);
                            }
                        } /* if (vtss_erps_build_raps_pktif */ 
                    } /* if (erpg->west_port) */
                } /* if (topology_propogate == TRUE) */
                break;
            } /* if (erpg_instances[count].ring_type == ERPS_RING_TYPE_MAJOR) */
        } /* if (erpg_instances[count].erps_status != ERPS_PROTECTION_GROUP_INACTIVE) */
    } /* for (count = 0 ; count < ERPS_MAX_PROTECTION_GROUPS; count++) */
    return (ERPS_RC_OK);
}

i32 vtss_erps_unset_rpl_owner(u32 erps_group_id)
{
    erps_protection_group_t         *erpg;
    vtss_rc                          ret;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }


    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    /*
     * This operation is only allowed on rpl owner and if the group is in active state
     */
    if (erpg->erps_status == ERPS_PROTECTION_GROUP_ACTIVE &&
        erpg->rpl_owner) {

        erpg->rpl_owner    = ERPS_NODE_NON_RPL_OWNER;
        erpg->rpl_blocked  = RAPS_RPL_NON_BLOCKED;
        erpg->blocked_port = 0;
        erpg->erps_instance.current_blocked = 0;

       /* STOP WTR if running */
        if (erpg->erps_instance.wtr_running) {
            erpg->erps_instance.wtr_timeout = 0;
            erpg->erps_instance.wtr_running = 0;
        }

        if (!erpg->erps_instance.lsf_port_east) {
            ret = vtss_erps_protection_group_state_set (erpg->group_id, 
                    erpg->east_port, VTSS_ERPS_STATE_FORWARDING);
            if (ret !=  VTSS_RC_OK) {
                vtss_erps_trace("failing in putting blocked port in forwarding state", 0);
                ERPS_CRITD_EXIT();
                return (ERPS_ERROR_MOVING_TO_FORWARDING);
            }
        }

        if (!erpg->erps_instance.lsf_port_west) {
            if (erpg->west_port) {
                ret = vtss_erps_protection_group_state_set (erpg->group_id, 
                        erpg->west_port, VTSS_ERPS_STATE_FORWARDING);
                if (ret !=  VTSS_RC_OK) {
                    vtss_erps_trace("failing in putting blocked port in forwarding state", 0);
                    ERPS_CRITD_EXIT();
                    return (ERPS_ERROR_MOVING_TO_FORWARDING);
                }
            } 
        } 

        /* 
         * enable R-APS forwarding as this node is no more acting as RPL Owner for given group 
         */
        ret = vtss_erps_raps_forwarding(erpg->east_port,erpg->group_id,TRUE);
        if (ret != VTSS_RC_OK) {
            vtss_erps_trace("vtss_erps_raps_forwarding failed for group =", erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_RAPS_DISABLE_FORWARDING_FAILED);
        }

        if (erpg->west_port || erpg->raps_virt_channel) {
        ret = vtss_erps_raps_forwarding(erpg->west_port,erpg->group_id,TRUE);
        if (ret != VTSS_RC_OK) {
            vtss_erps_trace("vtss_erps_raps_forwarding failed for group =", erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_RAPS_DISABLE_FORWARDING_FAILED);
        }
        }

        /* 
         * disable R-APS PDU transmission as this node is not more acting as RPL Owner for given group 
         */
        ret = vtss_erps_raps_transmission(erpg->east_port,erpg->group_id,FALSE);
        if (ret != VTSS_RC_OK) {
            vtss_erps_trace("vtss_erps_raps_transmission failed for group =", erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_RAPS_ENABLE_RAPS_TX_FAILED);
        }

        if (erpg->west_port || erpg->raps_virt_channel) {
        ret = vtss_erps_raps_transmission(erpg->west_port,erpg->group_id,FALSE);
        if (ret != VTSS_RC_OK) {
            vtss_erps_trace("vtss_erps_raps_transmission failed for group =", erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_RAPS_ENABLE_RAPS_TX_FAILED);
        }
        }

        /*
         * flush fdb table
         */
        VTSS_ERPS_FLUSH_FDB;

        /* inorder to GUI know, which Request is being transmitted currently */
        erpg->erps_instance.raps_tx = ERPS_STOP_RAPS_TX;
        erpg->erps_instance.tx_req = 0;
        erpg->erps_instance.tx_rb = 0;
        erpg->erps_instance.tx_dnf = 0;
    } 

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32  vtss_erps_set_guard_timeout(u32 erps_group_id, u16 timeout)
{
    erps_protection_group_t         *erpg;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    erpg->guard_time = timeout;
    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32  vtss_erps_set_wtr_timeout(u32 erps_group_id, u64 timeout)
{
    erps_protection_group_t         *erpg;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    erpg->wtr_time = timeout;
    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32  vtss_erps_set_holdoff_timeout(u32 erps_group_id, u64 timeout)
{
    erps_protection_group_t         *erpg;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    erpg->holdoff_time = timeout;

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32 vtss_erps_set_rpl_neighbour (u32 erps_group_id, vtss_port_no_t port)
{
    erps_protection_group_t   *erpg;
    vtss_port_no_t            unblocked_port;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    if (erpg->rpl_neighbour) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_ALREADY_NEIGHBOUR_FOR_THISGROUP);
    }

    /* rpl owner can not be neighbour for himself */
    if (erpg->rpl_owner) {
        vtss_erps_trace("This node is aleady rpl owner for this group",0);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_NODE_ALREADY_RPL_OWNER);
    }

    if (port == 0) {
        vtss_erps_trace("neighbour port cannot be zero", 0);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GIVEN_PORT_NOT_EAST_OR_WEST);
    }

    if (port != erpg->east_port && port != erpg->west_port) {
        vtss_erps_trace("neighbour port should be one among east or west port group=", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GIVEN_PORT_NOT_EAST_OR_WEST);
    }

    erpg->rpl_neighbour = TRUE;
    erpg->rpl_neighbour_port = port;
    erpg->blocked_port = erpg->rpl_neighbour_port;
    erpg->rpl_blocked  = RAPS_RPL_BLOCKED;

    erpg->erps_instance.current_blocked = erpg->blocked_port;
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;

    if (port == erpg->east_port) {
        erpg->erps_instance.east_blocked = erpg->blocked_port;
        erpg->erps_instance.west_blocked = 0;
        unblocked_port = erpg->west_port;
    }
    else {
        erpg->erps_instance.west_blocked = erpg->blocked_port;
        erpg->erps_instance.east_blocked = 0;
        unblocked_port = erpg->east_port;
    }

    /* block rpl port */
    if (vtss_erps_protection_group_state_set (erpg->group_id, erpg->blocked_port, VTSS_ERPS_STATE_DISCARDING) !=  VTSS_RC_OK) {
        vtss_erps_trace("<FSM>:erps_protection_group_state_set grp =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_MOVING_TO_DISCARDING);
    }

    if (erpg->west_port) {
    /* unblock non-rpl port */
    if (vtss_erps_protection_group_state_set (erpg->group_id, unblocked_port, VTSS_ERPS_STATE_FORWARDING) !=  VTSS_RC_OK) {
        vtss_erps_trace("<FSM>:erps_protection_group_state_set grp =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_MOVING_TO_DISCARDING);
    }
    }

    /* disable R-APS forwarding on blocked port */
    if (vtss_erps_raps_forwarding(erpg->blocked_port, erpg->group_id, FALSE) != VTSS_RC_OK) {
        vtss_erps_trace("Error in enabling forwarding on group =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_MOVING_TO_FORWARDING);
    }

    if (erpg->west_port || erpg->raps_virt_channel) {
    /* enable R-APS forwarding non blocked port*/
    if (vtss_erps_raps_forwarding(unblocked_port, erpg->group_id, TRUE) != VTSS_RC_OK) {
        vtss_erps_trace("Error in enabling forwarding on group =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_MOVING_TO_FORWARDING);
    }
    }

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32 vtss_erps_unset_rpl_neighbour (u32 erps_group_id)
{
    erps_protection_group_t         *erpg;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    if (erpg->rpl_neighbour) {
        VTSS_ERPS_FLUSH_FDB;

        erpg->rpl_neighbour = !erpg->rpl_neighbour;
        erpg->rpl_neighbour_port = 0;
        erpg->blocked_port = 0;
    }

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32 vtss_erps_associate_group (u32 erps_group_id)
{
    erps_protection_group_t         *erpg;
    vtss_rc                         ret;
    u16                             vid = 0;
    u8                              rapspdu[ERPS_PDU_SIZE];

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER(); 
    erpg = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_ACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_CANNOT_ASSOCIATE_GROUP);
    }

    /* State Machine Initialization */

    /* stop guard, WTR and WTB Timers */
    erpg->erps_instance.wtr_running = 0;
    erpg->erps_instance.wtr_timeout = 0;
    erpg->erps_instance.wtb_running = 0;
    erpg->erps_instance.wtb_timeout = 0;
    erpg->erps_instance.guard_timeout = 0;
    erpg->erps_instance.guard_timer_running = 0;

    /* move protection group to ACTIVE mode */
    erpg->erps_status = ERPS_PROTECTION_GROUP_ACTIVE;

    /* move protection group to PENDING state */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;
    erpg->erps_instance.admin_cmd = FSM_EVENT_INVALID;
    /* If RPL owner and Revertive, start WTR Timer */
    if (erpg->rpl_owner && erpg->revertive) {
        erpg->erps_instance.wtr_timeout = erpg->wtr_time;
        erpg->erps_instance.wtr_running = 1;
    }

    if (!erpg->rpl_owner) {
        if (erpg->rpl_neighbour) {
           erpg->erps_instance.current_blocked = erpg->blocked_port; 
        }
        else {
           /* for normal ring nodes, initially east port is made as blocking */
           erpg->erps_instance.current_blocked = erpg->east_port;
        }

       /* enabling ingress filter EAST Port */
        ret = vtss_erps_vlan_ingress_filter (erpg->east_port,TRUE);
        if (ret !=  VTSS_RC_OK) {
            vtss_erps_trace("<FSM>: error in erps_vlan_ingress_filter grp =",erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR);
        }

        /* enabling ingress filter WEST Port */
        if (erpg->west_port) {
        ret = vtss_erps_vlan_ingress_filter (erpg->west_port,TRUE);
        if (ret !=  VTSS_RC_OK) {
            vtss_erps_trace("<FSM>: error in erps_vlan_ingress_filter grp =",erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR);
        }
        }

        /* create protection group vlan map  */
        for ( vid = 0; vid < PROTECTED_VLANS_MAX ; vid++ ) {
            if (erpg->protected_vlans[vid]) {
                if(vtss_erps_ctrl_set_vlanmap(erpg->protected_vlans[vid],
                     erpg->group_id,TRUE) != VTSS_RC_OK) {
                     /*
                      * some problem in setting vlan map, don't move Protection
                      * group into active state, i.e this group is not usable
                      */
                      erpg->erps_status = ERPS_PROTECTION_GROUP_INACTIVE;
                      vtss_erps_trace("<FSM>: erps_ctrl_set_vlanmap failed grp =",erpg->group_id);
                      ERPS_CRITD_EXIT();
                      return (ERPS_ERROR_FAILED_IN_SETTING_VLANMAP);
                } /* if */
            } /* if */
         } /* for */


        /*
         * The default configuration is that all VLANs are disabled and all ports
         *  are discarding for all ERPS instances. If the application (ERPS module)
         *  enables a VLAN for an ERPS instance, it must setup the forwarding state
         *  for all ports for the instance.
         */
        ret = vtss_erps_put_protected_vlans_in_forwarding(erpg->group_id, erpg->erps_instance.current_blocked);
        if (ret != VTSS_RC_OK ) {
            vtss_erps_trace(" error in putting all port+vlan in forwarding mode",0);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR);
        }

        /*
         * block one ring port , i guess this function call is not required, 
         * as blocking is being handled by the above function, as we are 
         * passing erpg->erps_instance.current_blocked as parameter.
         */
        ret = vtss_erps_protection_group_state_set (erpg->group_id, 
                                                    erpg->erps_instance.current_blocked,
			                                        VTSS_ERPS_STATE_DISCARDING);
        if (ret !=  VTSS_RC_OK) {
            vtss_erps_trace("<FSM>:erps_protection_group_state_set grp =", erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_MOVING_TO_DISCARDING);
        }

        (erpg->erps_instance.current_blocked == erpg->west_port) ? \
        (erpg->erps_instance.west_blocked=erpg->west_port): \
        (erpg->erps_instance.east_blocked=erpg->east_port);


        /* disable R-APS forwarding on blocked port */
        ret = vtss_erps_raps_forwarding(erpg->erps_instance.current_blocked,
                                        erpg->group_id,FALSE);
        if (ret != VTSS_RC_OK) {
            vtss_erps_trace("vtss_erps_raps_forwarding failed for group =", erpg->group_id );
            ERPS_CRITD_EXIT();
            return (ERPS_ERROR_RAPS_ENABLE_FORWARDING_FAILED);
        }

        if (erpg->west_port || erpg->raps_virt_channel) {
            /* enable R-APS forwarding on non blocked port */
            ret = vtss_erps_raps_forwarding(((erpg->erps_instance.current_blocked == erpg->east_port) ? \
                        erpg->west_port : erpg->east_port),erpg->group_id,TRUE);
            if (ret != VTSS_RC_OK) {
                vtss_erps_trace("vtss_erps_raps_forwarding failed for group =", erpg->group_id );
                ERPS_CRITD_EXIT();
                return (ERPS_ERROR_RAPS_ENABLE_FORWARDING_FAILED);
            }

            if (erpg->erps_instance.current_blocked == erpg->east_port) {
                if (erpg->west_port) {
                    /* unblock other ring port */
                    ret = vtss_erps_protection_group_state_set (erpg->group_id, erpg->west_port,
                            VTSS_ERPS_STATE_FORWARDING);

                    if (ret !=  VTSS_RC_OK) {
                        vtss_erps_trace("<FSM>:erps_protection_group_state_set grp =", erpg->group_id);
                        ERPS_CRITD_EXIT();
                        return (ERPS_ERROR_MOVING_TO_DISCARDING);
                    }
                }
            }
        }

        /* making other ring port as non-blokced */
        ((erpg->erps_instance.current_blocked == erpg->east_port) ? \
         ( erpg->erps_instance.west_blocked = 0):\
         ( erpg->erps_instance.east_blocked = 0));

        /* enable R-APS transmission on both the ports */
        /* Enable transmission of R-APS before actually transmitting */
        if (erpg->erps_instance.raps_tx == ERPS_STOP_RAPS_TX) {
            ret = vtss_erps_raps_transmission(erpg->east_port,erpg->group_id,TRUE);
            if (ret != VTSS_RC_OK) {
                vtss_erps_trace("vtss_erps_raps_transmission failed for group =", erpg->group_id);
                ERPS_CRITD_EXIT();
                return (ERPS_ERROR_RAPS_ENABLE_RAPS_TX_FAILED);
            }
            if (erpg->west_port || erpg->raps_virt_channel) {
                ret = vtss_erps_raps_transmission(erpg->west_port,erpg->group_id,TRUE);
                if (ret != VTSS_RC_OK) {
                    vtss_erps_trace("vtss_erps_raps_transmission failed for group =", erpg->group_id);
                    ERPS_CRITD_EXIT();
                    return (ERPS_ERROR_RAPS_ENABLE_RAPS_TX_FAILED);
                }
            }
        }

        if (erpg->erps_instance.raps_tx == ERPS_STOP_RAPS_TX) {
            VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR, 0, FALSE, FALSE);
            erpg->erps_instance.raps_tx = ERPS_START_RAPS_TX;
        }
    }

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

i32 vtss_erps_enable_interconnected(u32 erps_group_id, u32 major_ring_id)
{
    erps_protection_group_t         *erpg = NULL;
    erps_protection_group_t         *p_erpg = NULL;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ||
         major_ring_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];
    p_erpg = &erpg_instances[major_ring_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    if (p_erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    /* if a sub-ring node has west_port, it cannot be an interconnected node */
    if (erpg->west_port) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_CANNOTBE_INTERCONNECTED_SUB_NODE);
    }

    erpg->interconnected_node = p_erpg->interconnected_node = TRUE;
    erpg->major_ring_id       = major_ring_id;

    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32 vtss_erps_disable_interconnected(u32 erps_group_id, u32 major_ring_id)
{
    erps_protection_group_t         *erpg = NULL;
    erps_protection_group_t         *p_erpg = NULL;

    if ( erps_group_id >= ERPS_MAX_PROTECTION_GROUPS ||
         major_ring_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];
    p_erpg = &erpg_instances[major_ring_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    if (p_erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    if (!erpg->interconnected_node || !p_erpg->interconnected_node) {
        vtss_erps_trace("one of these two groups are not part of interconnection =", major_ring_id );
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_INVALID_PGID);
    }

    if (erpg->major_ring_id != major_ring_id) {
        vtss_erps_trace("given major-ring-id is not the exact pair of give sub-ring =", major_ring_id );
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_INVALID_PGID);
    }

    erpg->interconnected_node = p_erpg->interconnected_node = FALSE;

    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32 vtss_erps_enable_non_reversion(u32 erps_group_id)
{
    erps_protection_group_t         *erpg = NULL;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];
    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }
    
    if (erpg->erps_version == ERPS_VERSION_V1) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR);
    }

    erpg->revertive =  FALSE;
    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32 vtss_erps_disable_non_reversion(u32 erps_group_id)
{
    erps_protection_group_t         *erpg = NULL;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];
    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }
    erpg->revertive =  TRUE;
    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

static i32 vtss_erps_convert_admin_cmd_to_event (vtss_erps_admin_cmd_t cmd)
{
    switch (cmd) {
        case ERPS_ADMIN_CMD_MANUAL_SWITCH:
            return (FSM_EVENT_LOCAL_MS);
        case ERPS_ADMIN_CMD_FORCED_SWITCH:
            return (FSM_EVENT_LOCAL_FS);
        case ERPS_ADMIN_CMD_CLEAR:
            return (FSM_EVENT_LOCAL_CLEAR);
        default:
            return 0;
    }
}

i32 vtss_erps_admin_command(u32 erps_group_id, 
                            vtss_erps_admin_cmd_t cmd,
                            vtss_port_no_t  port)
{
    erps_protection_group_t         *erpg = NULL;
    i32 event = vtss_erps_convert_admin_cmd_to_event(cmd);

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }      

    if (erpg->erps_version == ERPS_VERSION_V1) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR);
    }      

    ERPS_CRITD_EXIT();

    if (erps_handle_local_event(event, erps_group_id, port) != ERPS_RC_OK) {
        return (VTSS_RC_ERROR);
    }

    return (ERPS_RC_OK);
}

i32 vtss_erps_set_topology_change_propogation(u32 erps_group_id, BOOL enable)
{
    erps_protection_group_t         *erpg = NULL;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];
    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }
    erpg->topology_propogate =  enable;
    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32 vtss_erps_disable_virtual_channel(u32 erps_group_id)
{
    erps_protection_group_t         *erpg = NULL;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    erpg->raps_virt_channel =  FALSE;
    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32 vtss_erps_enable_virtual_channel(u32 erps_group_id)
{
    erps_protection_group_t         *erpg = NULL;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];
    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    if (erpg->ring_type == ERPS_RING_TYPE_MAJOR) {
        vtss_erps_trace("virtual channel not required for major ring =", erps_group_id );
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_INVALID_PGID);
    }

    erpg->raps_virt_channel =  TRUE;
    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32 vtss_erps_set_protocol_version(u32 erps_group_id, vtss_erps_version_t version)
{
    erps_protection_group_t         *erpg = NULL;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg   = &erpg_instances[erps_group_id];
    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    erpg->erps_version =  version;

    /* change revertion to revertive mode incase of ERPS V1*/
    if (version == ERPS_VERSION_V1) {
        erpg->revertive = TRUE;
    }
    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}

i32 vtss_erps_set_wtb_timeout(u32 erps_group_id, u32 timeout )
{
    return (ERPS_RC_OK);
}

/* this function converts incoming R-APS event to FSM event */
static i32 erps_map_incoming_event_to_fsm_event (u8 in_event, u8 rb)
{
    switch (in_event) 
    {
        case ERPS_REQ_NR:
            return ((rb) ? FSM_EVENT_REMOTE_NR_RB : FSM_EVENT_REMOTE_NR);
        case ERPS_REQ_SF:
            return (FSM_EVENT_REMOTE_SF);
        case ERPS_REQ_FS:
            return (FSM_EVENT_REMOTE_FS);
        case ERPS_REQ_MS:
            return (FSM_EVENT_REMOTE_MS);
        case ERPS_REQ_EVENT:
            return (FSM_EVENT_REMOTE_EVENT);
        default:
            break;
    }
    return (ERPS_ERROR_INVALID_REMOTE_EVENT);
}

static i32
vtss_erps_remote_node_is_higher (u8 * nodeid, u8 * remote_nodeid)
{
    u8 offset = 0;

    while (offset < ERPS_MAX_NODE_ID_LEN) {
        if (nodeid[offset] != remote_nodeid[offset])
            break;
        offset++;
    }

    if (offset != ERPS_MAX_NODE_ID_LEN)
        return ((nodeid[offset] < remote_nodeid[offset]) ? ERPS_RC_OK : ERPS_ERROR);
    else
        return (ERPS_ERROR);
}

i32 vtss_erps_clear_statistics(u32 erps_group_id)
{
    erps_protection_group_t *erpg;

    if (erps_group_id >= ERPS_MAX_PROTECTION_GROUPS) {
        return (ERPS_ERROR_INVALID_PGID);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erps_group_id];
    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    memset(&erpg->erps_instance.erps_stats, 0, sizeof(erpg->erps_instance.erps_stats));
    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}
/******************************************************************************
                             R-APS PDU Manipulation Functions 
******************************************************************************/
static i32 erps_validate_raps_pdu (erps_pdu_t pdu,
                                   erps_protection_group_t * pg)
{
    if (!(pdu.req_state == ERPS_REQ_NR) && !(pdu.req_state == ERPS_REQ_SF) && !(pdu.req_state == ERPS_REQ_FS) &&
        !(pdu.req_state == ERPS_REQ_MS) && !(pdu.req_state == ERPS_REQ_EVENT)) {
        return (ERPS_ERROR_INVALID_REMOTE_EVENT);
    }

    return (ERPS_RC_OK);
}

static i32 erps_parse_raps_pdu (erps_protection_group_t *pg, 
                                u8 *pdu, 
                                u16 pdu_len, 
                                erps_pdu_t *r_pdu,
                                vtss_port_no_t port)
{
    u8     byte   = 0;
    u8     offset = 0;
    u8     port_reference = 0;

    /* for flush logic, east port information is getting stored in offset 0 or west on offset 1 */
    port_reference = ((port == pg->east_port) ? 0 : 1);

    /* get first byte containing req_state and reserved fields */
    memcpy(&byte, pdu+offset, sizeof(u8));

    /* extracting req_state and reserved fields */
    r_pdu->req_state = GET_REQUEST_STATE (byte);
    r_pdu->reserved  = GET_RESERVED(byte);
    offset++;

    /* get second byte containing rb/dnf and status reserved fields */
    byte = 0;
    memcpy(&byte,pdu+offset,sizeof(u8));

    /* extracting RB - DNF - BPR Flags and node ID*/
    r_pdu->rb  = ERPS_CHECK_FLAG(byte,ERPS_PDU_NOREQUEST_RB_MASK);
    r_pdu->dnf = ERPS_CHECK_FLAG(byte,ERPS_PDU_DNF_MASK);
    r_pdu->bpr = ERPS_CHECK_FLAG(byte,ERPS_PDU_BPR_MASK);
    offset++;
    memcpy(r_pdu->node_id, pdu+offset, ERPS_MAX_NODE_ID_LEN);

    pg->erps_instance.raps_rx[port_reference] = 1;
    pg->erps_instance.rb[port_reference] = r_pdu->rb;
    pg->erps_instance.dnf[port_reference] =  r_pdu->dnf;
    pg->erps_instance.bpr[port_reference] = r_pdu->bpr;
    pg->erps_instance.req[port_reference] =  r_pdu->req_state;
    memcpy(pg->erps_instance.node_id[port_reference], r_pdu->node_id, ERPS_MAX_NODE_ID_LEN);

    /* findout if we are receiving R-APS frames from higher node_id's */
    pg->erps_instance.remote_higher_nodeid = vtss_erps_remote_node_is_higher(pg->node_id_e,r_pdu->node_id); 
    /* 
     * copy information related to flush logic, these are required for logic specified in 
     * Section 10.1.10 ITUT.G8032(V2)
     */
    pg->erps_instance.rcvd_fl[port_reference].bpr = r_pdu->bpr;
    memcpy(pg->erps_instance.rcvd_fl[port_reference].node_id, r_pdu->node_id, ERPS_MAX_NODE_ID_LEN);

    return (ERPS_RC_OK);
}


/* function to construct R-APS PDU's */
static i32 vtss_erps_build_raps_pkt (erps_protection_group_t *erpg,
                                     u8      *pdu,
                                     u8      req_state,
                                     u8      sub_code,
                                     BOOL    rb,
                                     BOOL    dnf,
                                     BOOL    bpr)
{
    u8   rapspdu[ERPS_PDU_SIZE];
    u8   offset = 0;

    memset(rapspdu,0,ERPS_PDU_SIZE);

    rapspdu[offset]=SET_REQUEST_STATE(req_state, ERPS_PDU_RESERVED);

    /* 
     * for EVENT PDU's sub-code is 0000, which is the same by default and flags
     * are also 00000000
     */

    offset++;

    if (rb)
        ERPS_SET_FLAG(rapspdu[offset],ERPS_PDU_NOREQUEST_RB_MASK);
    if (dnf)    
        ERPS_SET_FLAG(rapspdu[offset],ERPS_PDU_DNF_MASK);
    if (bpr)
        ERPS_SET_FLAG(rapspdu[offset],ERPS_PDU_BPR_MASK);

    offset++;

    erpg->erps_instance.tx_req = GET_REQUEST_STATE(rapspdu[0]);
    erpg->erps_instance.tx_rb = ERPS_CHECK_FLAG(rapspdu[1],ERPS_PDU_NOREQUEST_RB_MASK);
    erpg->erps_instance.tx_dnf = ERPS_CHECK_FLAG(rapspdu[1],ERPS_PDU_DNF_MASK);
    erpg->erps_instance.tx_bpr = bpr;

    memcpy(&rapspdu[offset],erpg->node_id_e,ERPS_MAX_NODE_ID_LEN);

    offset+=ERPS_MAX_NODE_ID_LEN;
    memcpy(pdu,rapspdu,offset);

    return (ERPS_RC_OK);
}

/******************************************************************************
                                    FSM Handlers
******************************************************************************/
static i32
erps_fsm_event_ignore (erps_protection_group_t *erpg)
{
    return (ERPS_RC_OK);
}

static i32 
erps_event_not_applicable (erps_protection_group_t *erpg)
{
    vtss_erps_trace("calling from FSM Handler erps_event_not_applicable",0 );
    return (ERPS_RC_OK);
}

static i32 
erps_idle_event_handle_remote_sf (erps_protection_group_t *erpg)
{
    vtss_rc  ret = VTSS_RC_ERROR;

    /* unblock non-failed ring port */
    if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    }

    if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
    }

    /* stop tx r-aps */
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
        VTSS_ERPS_STOP_TX_RAPS;
    }

    /* move to PROTECTED STATE */
    erpg->erps_instance.current_state = ERPS_STATE_PROTECTED;
    erpg->erps_instance.erps_stats.remote_sf++;
    vtss_erps_trace_state_change("moving from IDLE to PROTECTED group = ",erpg->group_id);
    

    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_idle_event_handle_local_holdoff_expires) */
static i32 
erps_idle_event_handle_local_holdoff_expires (erps_protection_group_t *erpg)
{
    vtss_rc   ret = VTSS_RC_ERROR;
    u8   rapspdu[ERPS_PDU_SIZE];

    vtss_erps_trace("erps_idle_event_handle_local_holdoff_expires", 0);
    /* by the time hold-off expires, if SF disappeared, no need to take action */
    if (!erpg->erps_instance.lsf_port_east && !erpg->erps_instance.lsf_port_west) {
         return (ERPS_RC_OK);
    }

    /* it may so happend that both the ring ports are down */
    if (erpg->erps_instance.lsf_port_east && erpg->erps_instance.lsf_port_west) {
         /* such cases, block both ring ports,disable forwarding and stop tx */
         VTSS_ERPS_BLOCK_RING_PORT(erpg->east_port);
         VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->east_port);

         VTSS_ERPS_BLOCK_RING_PORT(erpg->east_port);
         VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->west_port);

         if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
             VTSS_ERPS_STOP_TX_RAPS;
         }
         
         /* move to PROTECTED STATE */
         erpg->erps_instance.current_state = ERPS_STATE_PROTECTED;
         vtss_erps_trace_state_change("moving from IDLE to PROTECTED group = ",erpg->group_id);
         return (ERPS_RC_OK);
    }

    /* if failed ring port is already blocked */
    if (((erpg->erps_instance.l_event_port == erpg->east_port) && erpg->erps_instance.east_blocked) ||
       ((erpg->erps_instance.l_event_port == erpg->west_port) && erpg->erps_instance.west_blocked)) {

        /* unblock non-failed ring port */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }

        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

    	vtss_erps_trace("erps_idle_event_handle_local_holdoff_expires already blocked", 0);
        /* Tx R-APS (SF,DNF) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_SF,0,FALSE,TRUE);
    
    } else {
        /* block failed ring port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* unblock non-failed ring port */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }

        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

    	vtss_erps_trace("erps_idle_event_handle_local_holdoff_expires blocking...", 0);
        /* Tx R-APS (SF) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_SF,0,FALSE,FALSE);

        /* Flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    /* move to PROTECTED STATE */
    erpg->erps_instance.current_state = ERPS_STATE_PROTECTED;
    vtss_erps_trace_state_change("moving from IDLE to PROTECTED group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_idle_event_handle_local_clear_sf(erps_protection_group_t *erpg)
{
    return (ERPS_RC_OK);
}

/* This function should even handle for NR messages */
static i32 
erps_idle_event_handle_remote_nr (erps_protection_group_t *erpg)
{
    vtss_rc    ret = VTSS_RC_ERROR;

    if (!erpg->rpl_owner && !erpg->rpl_neighbour) {
        if (erpg->erps_instance.remote_higher_nodeid == ERPS_RC_OK) {
            /* unblock non-failed ring port */
            if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
                VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
                VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
            }

            if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
                VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
                VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
            }

            /* Stop Tx R-APS */
            if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
                VTSS_ERPS_STOP_TX_RAPS;
            }
        }
    }

    /* move to IDLE State */
    erpg->erps_instance.current_state = ERPS_STATE_IDLE;
    erpg->erps_instance.erps_stats.event_nr++;

    return (ERPS_RC_OK);
}

/* This function should even handle for NR_RB messages */
static i32 
erps_idle_event_handle_remote_nr_rb (erps_protection_group_t *erpg)
{
    vtss_rc    ret = VTSS_RC_ERROR;
    vtss_port_no_t non_rpl_port;

    if (erpg->rpl_owner || erpg->rpl_neighbour) {
        non_rpl_port = ((erpg->blocked_port) == erpg->east_port ? erpg->west_port : erpg->east_port);

        if ((non_rpl_port == erpg->east_port) && erpg->erps_instance.east_blocked) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        } else if ((non_rpl_port == erpg->west_port) && erpg->erps_instance.west_blocked) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }
    }
    else {
        if (erpg->erps_instance.east_blocked) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }
        if (erpg->erps_instance.west_blocked) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }
    }

    if (!erpg->rpl_owner) {
        if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
            VTSS_ERPS_STOP_TX_RAPS;
        }
    }

    /* move to IDLE State */
    erpg->erps_instance.current_state = ERPS_STATE_IDLE;
    erpg->erps_instance.erps_stats.event_nr++;

    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_idle_event_handle_local_sf) */
static i32
erps_idle_event_handle_local_sf (erps_protection_group_t *erpg)
{
    i32 fsm_ret = ERPS_RC_OK;

    if (erpg->holdoff_time == RAPS_HOLD_OFF_TIMEOUT_MIN_SECONDS ) {
        vtss_erps_trace("erps_idle_event_handle_local_sf RAPS_HOLD_OFF_TIMEOUT_MIN_SECONDS is 0", 0);
        /*  
         * as holdoff timeout is 0, no need to honor holdoff_timeout,
         * do state change immediately 
         */
        fsm_ret = ERPS_FSM[erpg->erps_instance.current_state-1]\
                           [FSM_EVENT_LOCAL_HOLDOFF_EXPIRES].fptr(erpg);

        if (fsm_ret != ERPS_RC_OK) {
           vtss_erps_trace("<FSM>: error in moving to hold_off_expiry state grp =",erpg->group_id);
            return (ERPS_ERROR_MOVING_TO_NEXT_STATE);
        }
        erpg->erps_instance.erps_stats.local_sf++;
        return (ERPS_RC_OK);
    } 

    /* 
     * holdoff is configured, initialized holdoff time and wait for
     * it's expiry 
     */
    if (!erpg->erps_instance.hold_off_running) {
        erpg->erps_instance.hold_off_timeout = erpg->holdoff_time;
        erpg->erps_instance.hold_off_running = 1;
        erpg->erps_instance.erps_stats.local_sf++;
        return (ERPS_RC_OK);
    }
    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_idle_event_handle_local_fs) */
static i32
erps_idle_event_handle_local_fs (erps_protection_group_t *erpg)
{
    vtss_rc      ret = VTSS_RC_ERROR;
    u8           rapspdu[ERPS_PDU_SIZE];
    vtss_port_no_t  port;

    /* if requested ring port is already blocked */
    if (erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked ||
        erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked) {
        port = ((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port : erpg->east_port);
        VTSS_ERPS_UNBLOCK_RING_PORT(port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(port);

        /* Tx R-APS (FS,DNF) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,TRUE);
    } else {
        /* block requested ring port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);
        port = ((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port : erpg->east_port);
        VTSS_ERPS_UNBLOCK_RING_PORT(port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(port);

        /* Tx R-APS (FS) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,FALSE);

        /* Flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    /* move to FORCED_SWITCH State */
    erpg->erps_instance.current_state = ERPS_STATE_FORCED_SWITCH;
    erpg->erps_instance.erps_stats.local_fs++;
    vtss_erps_trace_state_change("moving from IDLE to FORCED_SWITCH State ::: group = ",erpg->group_id);

    /* start guard timer */
    if (!erpg->erps_instance.guard_timer_running ) {
        erpg->erps_instance.guard_timeout       = erpg->guard_time;
        erpg->erps_instance.guard_timer_running = 1;
    }

    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_idle_event_handle_local_ms) */
static i32
erps_idle_event_handle_local_ms (erps_protection_group_t *erpg)
{
    vtss_rc      ret = VTSS_RC_ERROR;
    u8           rapspdu[ERPS_PDU_SIZE];

    /* if requested ring port is already blocked */
    if (erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked ||
        erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked) {
        /* unblock non-requested ring port */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port:erpg->east_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port:erpg->east_port));

        /* Tx R-APS (MS,DNF) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_MS,0,FALSE,TRUE);
    } else {
        /* blocked requested ring port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* unblock non-requested ring port */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port:erpg->east_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port:erpg->east_port));

        /* Tx R-APS (MS) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_MS,0,FALSE,FALSE);

        /* Flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    /* move to MANUAL_SWITCH State */
    erpg->erps_instance.current_state = ERPS_STATE_MANUAL_SWITCH;
    erpg->erps_instance.erps_stats.local_ms++;
    vtss_erps_trace_state_change("moving from IDLE to MANUAL_SWITCH State ::: group = ",erpg->group_id);

    /* start guard timer */
    if (!erpg->erps_instance.guard_timer_running ) {
        erpg->erps_instance.guard_timeout       = erpg->guard_time;
        erpg->erps_instance.guard_timer_running = 1;
    }

    return (ERPS_RC_OK);
}

static i32
erps_idle_event_handle_remote_fs (erps_protection_group_t *erpg)
{
    vtss_rc     ret = VTSS_RC_ERROR;

    /* unblock both ports, unblocking only if they are blocked */
    if (erpg->erps_instance.east_blocked) {
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    }
    if (erpg->erps_instance.west_blocked) {
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
    }

    /* stop Tx R-APS */
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
        VTSS_ERPS_STOP_TX_RAPS;
    }

    /* move to FORCED_SWITCH State */
    erpg->erps_instance.current_state = ERPS_STATE_FORCED_SWITCH;
    erpg->erps_instance.erps_stats.remote_fs++;
    vtss_erps_trace_state_change("moving from IDLE to FORCED_SWITCH(R-APS) State ::: group = ",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_idle_event_handle_remote_ms (erps_protection_group_t *erpg)
{
    vtss_rc     ret = VTSS_RC_ERROR;

    /* unblock non-failed ring port */
    if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east){
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    }
    if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west){
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
    }

    /* stop Tx R-APS */
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
        VTSS_ERPS_STOP_TX_RAPS;
    }

    /* move to MANUAL_SWITCH State */
    erpg->erps_instance.current_state = ERPS_STATE_MANUAL_SWITCH;
    erpg->erps_instance.erps_stats.remote_ms++;
    vtss_erps_trace_state_change("moving from IDLE to MANUAL_SWITCH(R-APS) State ::: group = ",erpg->group_id);

    return (ERPS_RC_OK);
}

#if 0
static i32 erps_prot_event_handle_local_clear (erps_protection_group_t *erpg)
{
    vtss_rc    ret = VTSS_RC_ERROR;
    u8         rapspdu[ERPS_PDU_SIZE];

    if (erpg->rpl_owner && !erpg->revertive) {
        /* blocked rpl port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->blocked_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->blocked_port);

        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->blocked_port == erpg->west_port)?erpg->east_port : erpg->west_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->blocked_port == erpg->west_port)?erpg->east_port : erpg->west_port));

        /* Tx R-APS (NR RB) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,TRUE,FALSE);

        erpg->erps_instance.current_state = ERPS_STATE_IDLE;
        vtss_erps_trace_state_change("moving from PROTECTED  to IDLE State ::: group = ",erpg->group_id);
    }
    return (ERPS_RC_OK);
}
#endif

/*lint -esym(459, erps_prot_event_handle_local_fs) */
static i32
erps_prot_event_handle_local_fs (erps_protection_group_t *erpg)
{
    vtss_rc      ret = VTSS_RC_ERROR;
    u8           rapspdu[ERPS_PDU_SIZE];

    /* if requested ring port is already blocked */
    if (erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked ||
        erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked) {
        /* unblock non-requested ring port, don't care about health of the link */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port : erpg->east_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port : erpg->east_port));

        /* Tx R-APS (FS,DNF) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,TRUE);
    } else {
        /* blocked requested ring port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* unblock non-requested ring port, don't care about health of the link */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port : erpg->east_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->east_port) ? erpg->west_port : erpg->east_port));

        /* Tx R-APS (FS) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,FALSE);

        /* Flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    /* move to FORCED_SWITCH State */
    erpg->erps_instance.current_state = ERPS_STATE_FORCED_SWITCH;
    erpg->erps_instance.erps_stats.remote_fs++;
    vtss_erps_trace_state_change("moving from PROTECTED to FORCED_SWITCH(Local) State ::: group = ",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_prot_event_handle_remote_fs (erps_protection_group_t *erpg)
{
    vtss_rc     ret = VTSS_RC_ERROR;

    /* unblock both ports */
    VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
    VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
    VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);

    /* stop Tx R-APS */
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
        VTSS_ERPS_STOP_TX_RAPS;
    }

    /* move to PROTECTED State */
    erpg->erps_instance.current_state = ERPS_STATE_FORCED_SWITCH;
    erpg->erps_instance.erps_stats.remote_fs++;
    vtss_erps_trace_state_change("moving from PROTECTED to FORCED_SWITCH(R-APS) State ::: group = ",erpg->group_id);
   
    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_prot_event_handle_local_holdoff_expires) */
static i32 
erps_prot_event_handle_local_holdoff_expires (erps_protection_group_t *erpg)
{
    vtss_rc      ret = VTSS_RC_ERROR;
    u8           rapspdu[ERPS_PDU_SIZE];

    /* if failed ring port is already blocked */
    if (erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked ||
        erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked) {
        /* unblock non-failed ring port */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }

        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

        /* Tx R-APS (SF,DNF) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_SF,0,FALSE,TRUE);
    } else {
        /* blocked failed ring port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* unblock non-failed ring port */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }

        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

        /* Tx R-APS (SF) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_SF,0,FALSE,FALSE);

        /* Flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    /* remain in the same state */
    erpg->erps_instance.current_state = ERPS_STATE_PROTECTED;
    erpg->erps_instance.erps_stats.local_sf++;
    vtss_erps_trace_state_change("no state change is required, staying in PROTECTED State ::: group = ",erpg->group_id);

    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_prot_event_handle_local_sf) */
static i32
erps_prot_event_handle_local_sf (erps_protection_group_t *erpg)
{
    i32 fsm_ret = ERPS_RC_OK;

   /* when you are in protection no need to honor hold-off timeout */
   fsm_ret = ERPS_FSM[erpg->erps_instance.current_state-1]\
                      [FSM_EVENT_LOCAL_HOLDOFF_EXPIRES].fptr(erpg);

   if (fsm_ret != ERPS_RC_OK) {
        vtss_erps_trace("<FSM>: error in moving to hold_off_expiry state grp =",erpg->group_id);
        return (ERPS_ERROR_MOVING_TO_NEXT_STATE);
   }
   erpg->erps_instance.erps_stats.local_sf++;
   return (ERPS_RC_OK);
}

static i32 
erps_prot_event_handle_local_clear_sf (erps_protection_group_t *erpg)
{
    u8                          rapspdu[ERPS_PDU_SIZE];
    vtss_rc                     ret = VTSS_RC_ERROR;
    
    if (!erpg->erps_instance.guard_timer_running ) {
        erpg->erps_instance.guard_timeout       = erpg->guard_time;
        erpg->erps_instance.guard_timer_running = 1;
    }

    /* TX R-APS(NR) */
    VTSS_ERPS_START_TX_RAPS;
    VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,FALSE,FALSE);

    if (erpg->rpl_owner & erpg->revertive) {
         if (!erpg->erps_instance.wtr_running) {
             erpg->erps_instance.wtr_timeout = erpg->wtr_time;
             erpg->erps_instance.wtr_running = 1;
         }
    }
    
    /* increment statistics */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;
    erpg->erps_instance.erps_stats.local_sf_cleared++;
    vtss_erps_trace_state_change("moving from PROTECTED to PENDING State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

/* This function should even handle for NR messages */
static i32 
erps_prot_event_handle_remote_nr (erps_protection_group_t *erpg)
{
    /* we recived NO_REQUEST and RB is 0 */
    if (erpg->rpl_owner && erpg->revertive) {
       /* start WTR only when it is not running */
        if (!erpg->erps_instance.wtr_running) {
            erpg->erps_instance.wtr_timeout = erpg->wtr_time;
            erpg->erps_instance.wtr_running = 1;
            erpg->erps_instance.wtr_sf_on_rpl = FALSE;     /* Remember if SF was on RPL */
        }
    }
    /* move to PENDING State */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;
    erpg->erps_instance.erps_stats.event_nr++;
    vtss_erps_trace_state_change("moving from PROTECTED to PENDING State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

/* This function should even handle for NR_RB messages */
static i32 
erps_prot_event_handle_remote_nr_rb (erps_protection_group_t *erpg)
{
    /* we recived NO_REQUEST and RB is 1 */
    /* move to PENDING State */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;
    erpg->erps_instance.erps_stats.event_nr++;
    vtss_erps_trace_state_change("moving from PROTECTED to PENDING State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_fs_event_handle_remote_nr (erps_protection_group_t *erpg)
{
    /* incoming R-APS(NR) */
    if (erpg->rpl_owner && erpg->revertive) {
        if (!erpg->erps_instance.wtb_running) {
            erpg->erps_instance.wtb_timeout = erpg->wtb_time;
            erpg->erps_instance.wtb_running = 1;
        }
    }

    /* move to PENDING State */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;
    erpg->erps_instance.erps_stats.event_nr++;
    vtss_erps_trace_state_change("moving from FORCED_SWITCH to PENDING State ::: group =",erpg->group_id);
   
    return (ERPS_RC_OK);
}

static i32
erps_fs_event_handle_remote_nr_rb (erps_protection_group_t *erpg)
{
    /* incoming R-APS(NR-RB) */
    /* move to PENDING State */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;
    erpg->erps_instance.erps_stats.event_nr++;
    vtss_erps_trace_state_change("moving from FORCED_SWITCH to PENDING State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_fs_event_handle_local_holdoff_expires (erps_protection_group_t *erpg)
{
   return (ERPS_RC_OK);
}

/*lint -esym(459, erps_fs_event_handle_local_fs) */
static i32
erps_fs_event_handle_local_fs (erps_protection_group_t *erpg)
{
    u8         rapspdu[ERPS_PDU_SIZE];
    vtss_rc    ret = VTSS_RC_ERROR;

    /* block requested ring port */ 
    VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
    VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

    /* Tx R-APS(FS) */
    VTSS_ERPS_START_TX_RAPS;
    VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,FALSE);

    /* Flush FDB */
    VTSS_ERPS_FLUSH_FDB;

    /* no need state change, as the current state is FORCED_SWITCH */

    return (ERPS_RC_OK);
}

static i32
erps_fs_event_handle_local_clear (erps_protection_group_t *erpg)
{
    vtss_rc   ret = VTSS_RC_ERROR;
    u8        rapspdu[ERPS_PDU_SIZE];

    if (erpg->erps_instance.east_blocked || erpg->erps_instance.west_blocked) {
        /*: start guard timer */
        if (!erpg->erps_instance.guard_timer_running ) {
            erpg->erps_instance.guard_timeout       = erpg->guard_time;
            erpg->erps_instance.guard_timer_running = 1;
        }

        /*TX-R-APS(NR) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,FALSE,FALSE);

        if (erpg->rpl_owner && erpg->revertive) {
            /* Start WTB */
            erpg->erps_instance.wtb_timeout = erpg->wtb_time;
            erpg->erps_instance.wtb_running = 1;
        }

        /* move to PENDING State */
        erpg->erps_instance.current_state = ERPS_STATE_PENDING;
        vtss_erps_trace_state_change("moving from FORCED_SWITCH to PENDING State ::: group =",erpg->group_id);
    }

    return (ERPS_RC_OK);
}

static i32
erps_ms_event_handle_remote_nr (erps_protection_group_t *erpg)
{
    if (erpg->rpl_owner && erpg->revertive) {
        /* Start WTB */
        erpg->erps_instance.wtb_timeout = erpg->wtb_time;
        erpg->erps_instance.wtb_running = 1;
    }

    /* move to PENDING State */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;
    erpg->erps_instance.erps_stats.event_nr++;
    vtss_erps_trace_state_change("moving from MANUAL_SWITCH to PENDING State ::: group =",erpg->group_id);
  
    return (ERPS_RC_OK);
}

static i32
erps_ms_event_handle_remote_nr_rb (erps_protection_group_t *erpg)
{
    /* just move to PENDING State, no actions are required */
    erpg->erps_instance.current_state = ERPS_STATE_PENDING;
    erpg->erps_instance.erps_stats.event_nr++;
    vtss_erps_trace_state_change("moving from MANUAL_SWITCH to PENDING State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_ms_event_handle_remote_sf (erps_protection_group_t *erpg)
{
    vtss_rc     ret = VTSS_RC_ERROR;

    /* unblock non-failed ring port */
    if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east){
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    }
    if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west){
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
    }

    /* stop Tx R-APS */ 
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
       VTSS_ERPS_STOP_TX_RAPS;
    }

    /* move to PROTECTED State */
    erpg->erps_instance.current_state = ERPS_STATE_PROTECTED;
    erpg->erps_instance.erps_stats.remote_ms++;
    vtss_erps_trace_state_change("moving from MANUAL_SWITCH to PROTECTED State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_ms_event_handle_remote_fs (erps_protection_group_t *erpg)
{
    vtss_rc       ret = VTSS_RC_ERROR;

    VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port)
    VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port)

    VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);

    /* stop Tx R-APS */
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
        VTSS_ERPS_STOP_TX_RAPS;
    }

    /* move to FORCED_SWITCH State */
    erpg->erps_instance.current_state = ERPS_STATE_FORCED_SWITCH;
    erpg->erps_instance.erps_stats.remote_fs++;
    vtss_erps_trace_state_change("moving from MANUAL_SWITCH to FORCED_SWITCH State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_ms_event_handle_remote_ms (erps_protection_group_t *erpg)
{
    vtss_rc    ret = VTSS_RC_ERROR;
    u8         rapspdu[ERPS_PDU_SIZE];

    if (erpg->erps_instance.east_blocked || erpg->erps_instance.west_blocked) {
        if (!erpg->erps_instance.guard_timer_running ) {
            erpg->erps_instance.guard_timeout       = erpg->guard_time;
            erpg->erps_instance.guard_timer_running = 1;
        }

        /* Tx R-APS (NR) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,FALSE,FALSE);

        /* if rpl owner and revertive, start WTB Timer */
        if (erpg->rpl_owner && erpg->revertive) {
            if (!erpg->erps_instance.wtb_running ) {
                erpg->erps_instance.wtb_timeout       = erpg->wtb_time;
                erpg->erps_instance.wtb_running = 1;
            }
        }

        /* move to PENDING State */
        erpg->erps_instance.current_state = ERPS_STATE_PENDING;
        erpg->erps_instance.erps_stats.remote_ms++;
        vtss_erps_trace_state_change("moving from MANUAL_SWITCH to PENDING State ::: group =",erpg->group_id);
    }
    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_ms_event_handle_local_sf) */
static i32
erps_ms_event_handle_local_sf (erps_protection_group_t *erpg)
{
    vtss_rc      ret = VTSS_RC_ERROR;
    u8           rapspdu[ERPS_PDU_SIZE];

    /* if failed ring port is already blocked */
    if (erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked ||
        erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked) {
        /* unblock non-failed ring port */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }

        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

        /* Tx R-APS (SF,DNF) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_SF,0,FALSE,TRUE);
    } else {
        /* blocked failed ring port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* unblock non-failed ring port */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }

        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

        /* Tx R-APS (SF) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_SF,0,FALSE,FALSE);

        /*Flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    /* move to PROTECTED State */
    erpg->erps_instance.current_state = ERPS_STATE_PROTECTED;
    erpg->erps_instance.erps_stats.local_sf++;
    vtss_erps_trace_state_change("moving from MANUAL_SWITCH to PROTECTED State ::: group =",erpg->group_id);
  
    return (ERPS_RC_OK);
}

static i32
erps_ms_event_handle_local_clear_sf (erps_protection_group_t *erpg)
{
   return (ERPS_RC_OK);
}

static i32
erps_ms_event_handle_local_holdoff_expires (erps_protection_group_t *erpg)
{
   return (ERPS_RC_OK);
}

/*lint -esym(459, erps_ms_event_handle_local_fs) */
static i32
erps_ms_event_handle_local_fs (erps_protection_group_t *erpg)
{
    vtss_rc      ret = VTSS_RC_ERROR;
    u8           rapspdu[ERPS_PDU_SIZE];

    /* if requested ring port is already blocked */
    if (erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked ||
        erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked) {
        /* unblock non-requested ring port */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

        /* Tx R-APS (FS,DNF) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,TRUE);
    } else {
        /* blocked requested ring port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* unblock non-requested ring port */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

        /* Tx R-APS (FS) */ 
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,FALSE);

        /*Flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    /* move to FORCED_SWITCH State */
    erpg->erps_instance.current_state = ERPS_STATE_FORCED_SWITCH;
    erpg->erps_instance.erps_stats.local_fs++;
    vtss_erps_trace_state_change("moving from MANUAL_SWITCH to FORCED_SWITCH State ::: group =",erpg->group_id);
  
    return (ERPS_RC_OK);
}

static i32
erps_ms_event_handle_local_clear (erps_protection_group_t *erpg)
{
    vtss_rc   ret = VTSS_RC_ERROR;
    u8        rapspdu[ERPS_PDU_SIZE];

    if (erpg->erps_instance.east_blocked || erpg->erps_instance.west_blocked) {
        if (!erpg->erps_instance.guard_timer_running ) {
            erpg->erps_instance.guard_timeout = erpg->guard_time;
            erpg->erps_instance.guard_timer_running = 1;
        }

        /* Tx R-APS (NR) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,FALSE,FALSE);

        /* if rpl owner and revertive, start WTB Timer */
        if (erpg->rpl_owner && erpg->revertive) {
            if (!erpg->erps_instance.wtb_running ) {
                erpg->erps_instance.wtb_timeout = erpg->wtb_time;
                erpg->erps_instance.wtb_running = 1;
            }
        }

        /* move to PENDING State */
        erpg->erps_instance.current_state = ERPS_STATE_PENDING;
        erpg->erps_instance.erps_stats.admin_cleared++;
        vtss_erps_trace_state_change("moving from MANUAL_SWITCH to PENDING State ::: group =",erpg->group_id);
    }

    return (ERPS_RC_OK);
}

static i32
erps_pend_event_handle_remote_nr (erps_protection_group_t *erpg)
{
    vtss_rc ret = VTSS_RC_ERROR;

    /*
     * if remote node is higher than own node-id, 
     * unblock non-failed port and stop R-APS TX
     */
    if (erpg->erps_instance.remote_higher_nodeid == ERPS_RC_OK) {
        /* unblock non-failed ring ports - but only if the port is not RPL */
        if (erpg->erps_instance.east_blocked && (erpg->blocked_port != erpg->east_port) && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }
        if (erpg->erps_instance.west_blocked && (erpg->blocked_port != erpg->east_port) && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

        if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
            VTSS_ERPS_STOP_TX_RAPS;
        }

        erpg->erps_instance.current_state = ERPS_STATE_PENDING;
        erpg->erps_instance.erps_stats.event_nr++;
        vtss_erps_trace_state_change("no state change is required, staying in PENDING State ::: group =",erpg->group_id);
    }

    return (ERPS_RC_OK);
}

static i32
erps_pend_event_handle_remote_nr_rb (erps_protection_group_t *erpg)
{
    vtss_rc ret = VTSS_RC_ERROR;

    if (erpg->rpl_owner) {
        /* stop WTR & WTB */
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;
    } else if (erpg->rpl_neighbour) {
        /* block RPL Port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->blocked_port) \
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->blocked_port);

        /* unblock non rpl port */ 
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->blocked_port == erpg->east_port) ? erpg->west_port : erpg->east_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->blocked_port == erpg->west_port) ?erpg->east_port : erpg->west_port));

        /* stop R-APS Tx */
        if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
            VTSS_ERPS_STOP_TX_RAPS;
        }
    } else {
        /* unblock rings ports, stop R-APS tx */
        /* This condiction arises even for Non-Revertive mode */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }
        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

        if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
            VTSS_ERPS_STOP_TX_RAPS;
        }
    }

    /* move to IDLE State */
    erpg->erps_instance.current_state = ERPS_STATE_IDLE;
    erpg->erps_instance.erps_stats.event_nr++;
    vtss_erps_trace_state_change("moving from PENDING to IDLE State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_pend_event_handle_remote_sf (erps_protection_group_t *erpg)
{
    vtss_rc      ret = VTSS_RC_ERROR;

    /* unblock non-failed ring ports */
    if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    }
    if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
    }

    /* stop tx r-aps */
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
        VTSS_ERPS_STOP_TX_RAPS;
    }

    /* STOP WTR & WTB Timers */
    if (erpg->rpl_owner) {
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;
    }

    /* move to PROTECTED  State */
    erpg->erps_instance.current_state = ERPS_STATE_PROTECTED;
    vtss_erps_trace_state_change("moving from PENDING to PROTECTED State ::: group =",erpg->group_id);
    return (ERPS_RC_OK);
}

static i32
erps_pend_event_handle_remote_fs (erps_protection_group_t *erpg)
{
    vtss_rc         ret = VTSS_RC_ERROR;

    /* unblock both ring ports */
    VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port)
    VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port)
    VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);

    /* stop tx r-aps */
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
        VTSS_ERPS_STOP_TX_RAPS;
    }

    /* STOP WTR & WTB Timers */
    if (erpg->rpl_owner) {
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;
    }

    /* move to FORCED_SWITCH  State */
    erpg->erps_instance.current_state = ERPS_STATE_FORCED_SWITCH;
    vtss_erps_trace_state_change("moving from PENDING to FORCED_SWITCH State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_pend_event_handle_remote_ms (erps_protection_group_t *erpg)
{
    vtss_rc      ret = VTSS_RC_ERROR;

    /* unblock non-failed ring ports */
    if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
    }
    if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
        VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
    }

    /* stop tx r-aps */
    if (erpg->erps_instance.raps_tx == ERPS_START_RAPS_TX) {
        VTSS_ERPS_STOP_TX_RAPS;
    }

    /* STOP WTR & WTB Timers */
    if (erpg->rpl_owner) {
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;
    }

    /* move to MANUAL_SWITCH  State */
    erpg->erps_instance.current_state = ERPS_STATE_MANUAL_SWITCH;
    vtss_erps_trace_state_change("moving from PENDING to MANUAL_SWITCH State ::: group =",erpg->group_id);
    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_pend_event_handle_local_sf) */
static i32
erps_pend_event_handle_local_sf (erps_protection_group_t *erpg)
{
    vtss_rc  ret = VTSS_RC_ERROR;
    u8       rapspdu[ERPS_PDU_SIZE];

    if ((erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked) ||
        (erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked)) {
        /* TODO:: to look once unblock non-failed ring port */
        /* unblock non-failed ring port */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }

        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

        /* Tx R-APS(SF,DNF) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_SF,0,FALSE,TRUE);
    } else {
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* TODO:: to look once  unblock non-failed ring port */
        /* unblock non-failed ring port */
        if (erpg->erps_instance.east_blocked && !erpg->erps_instance.lsf_port_east) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->east_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->east_port);
        }

        if (erpg->erps_instance.west_blocked && !erpg->erps_instance.lsf_port_west) {
            VTSS_ERPS_UNBLOCK_RING_PORT(erpg->west_port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(erpg->west_port);
        }

        /* TX R-APS (SF) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_SF,0,FALSE,FALSE);

        /* flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    if (erpg->rpl_owner) {
        /* STOP WTR & WTB Timers */
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;
    }

    /* move to PROTECTED State */
    erpg->erps_instance.current_state = ERPS_STATE_PROTECTED;
    erpg->erps_instance.erps_stats.local_sf++;
    vtss_erps_trace_state_change("moving from PENDING to PROTECTED State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_pend_event_handle_local_holdoff_expires (erps_protection_group_t *erpg)
{
    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_pend_event_handle_local_fs) */
static i32
erps_pend_event_handle_local_fs (erps_protection_group_t *erpg)
{
    vtss_rc          ret = VTSS_RC_ERROR;
    u8               rapspdu[ERPS_PDU_SIZE];

    /* if requested ring port is already blocked */
    if ((erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked) ||
        (erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked)) {
        /* unblock non-requested ring port */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

        /* Tx R-APS(FS,DNF) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,TRUE);
    } else {
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* unblock non-requested ring port */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

        /* TX R-APS (FS) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_FS,0,FALSE,FALSE);

        /* flush fdb */
        VTSS_ERPS_FLUSH_FDB;
    }

    if (erpg->rpl_owner) {
        /* STOP WTR & WTB Timers */
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;
    }

    /* move to FORCED_SWITCH State */
    erpg->erps_instance.current_state = ERPS_STATE_FORCED_SWITCH;
    erpg->erps_instance.erps_stats.local_fs++;
    vtss_erps_trace_state_change("moving from PENDING to FORCED_SWITCH State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_pend_event_handle_local_ms) */
static i32
erps_pend_event_handle_local_ms (erps_protection_group_t *erpg)
{
    u8       rapspdu[ERPS_PDU_SIZE];
    vtss_rc  ret = VTSS_RC_ERROR;

    /* STOP WTB */
    if (erpg->rpl_owner) {
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;
    }

    if (erpg->erps_instance.l_event_port == erpg->erps_instance.east_blocked ||
        erpg->erps_instance.l_event_port == erpg->erps_instance.west_blocked) {
        /* unblock non-requested ring port */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

        /* Tx R-APS(MS,DNF) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_MS,0,FALSE,TRUE);
    } else {
        /* block requested ring port */
        VTSS_ERPS_BLOCK_RING_PORT(erpg->erps_instance.l_event_port);
        VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->erps_instance.l_event_port);

        /* unblock non-requested ring port */
        VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
        VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->erps_instance.l_event_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

        /* Tx R-APS (MS) */
        VTSS_ERPS_START_TX_RAPS;
        VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_MS,0,FALSE,FALSE);

        /* flush FDB */
        VTSS_ERPS_FLUSH_FDB;
    }

    /* move to MANUAL_SWITCH  State */
    erpg->erps_instance.current_state = ERPS_STATE_MANUAL_SWITCH;
    vtss_erps_trace_state_change("moving from PENDING to MANUAL_SWITCH State ::: group =",erpg->group_id);

    return (ERPS_RC_OK);
}

static i32
erps_pend_event_handle_local_wtr_running (erps_protection_group_t *erpg)
{
    /* NO ACTION */
    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_pend_event_handle_local_wtr_expires) */
static i32
erps_pend_event_handle_local_wtr_expires (erps_protection_group_t *erpg)
{
    u8       rapspdu[ERPS_PDU_SIZE];
    vtss_rc  ret = VTSS_RC_ERROR;

    if (erpg->rpl_owner) {

        /* STOP WTB */
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;

        if (erpg->rpl_blocked == RAPS_RPL_BLOCKED) {
            /* unblock non-rpl port */
            VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

            /* TX R-APS (NR,RB,DNF) */
            VTSS_ERPS_START_TX_RAPS;
            VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,TRUE,TRUE);
        } else {
            /* block RPL Port */
            VTSS_ERPS_BLOCK_RING_PORT(erpg->blocked_port);
            VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->blocked_port);

            /* unblock non-rpl port */
            VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

            /* TX R-APS (NR,RB) */
            VTSS_ERPS_START_TX_RAPS;
            VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,TRUE,FALSE);

            /* flush FDB */
            VTSS_ERPS_FLUSH_FDB;
        }

        /* move to IDLE State */
        erpg->erps_instance.current_state = ERPS_STATE_IDLE;
        vtss_erps_trace_state_change("moving from PENDING to IDLE State ::: group =",erpg->group_id);
    }

    return (ERPS_RC_OK);
}

static i32
erps_pend_event_handle_local_wtb_running (erps_protection_group_t *erpg)
{

    /* NO ACTION */
    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_pend_event_handle_local_wtb_expires) */
static i32
erps_pend_event_handle_local_wtb_expires (erps_protection_group_t *erpg)
{
    u8            rapspdu[ERPS_PDU_SIZE];
    vtss_rc       ret = VTSS_RC_ERROR;                   

    if (erpg->rpl_owner) {
        /* STOP WTR */
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;

        if (erpg->rpl_blocked == RAPS_RPL_BLOCKED) {
            /* unblock non-rpl port */
            VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

            /* TX R-APS (NR,RB,DNF) */
            VTSS_ERPS_START_TX_RAPS;
            VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,TRUE,TRUE);
        } else {
            /* block RPL Port */
            VTSS_ERPS_BLOCK_RING_PORT(erpg->blocked_port);
            VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->blocked_port);

            /* unblock non-rpl port */
            VTSS_ERPS_UNBLOCK_RING_PORT(((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port));
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port));

            /* TX R-APS (NR,RB) */
            VTSS_ERPS_START_TX_RAPS;
            VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,TRUE,FALSE);

            /* flush FDB */
            VTSS_ERPS_FLUSH_FDB;
        }

        /* move to IDLE State */
        erpg->erps_instance.current_state = ERPS_STATE_IDLE;
        vtss_erps_trace_state_change("moving from PENDING to IDLE State ::: group =",erpg->group_id);
    }

    return (ERPS_RC_OK);
}

/*lint -esym(459, erps_pend_event_handle_local_clear) */
static i32
erps_pend_event_handle_local_clear (erps_protection_group_t *erpg)
{
    vtss_rc        ret = VTSS_RC_ERROR;
    u8             rapspdu[ERPS_PDU_SIZE];
    vtss_port_no_t port;

    if (erpg->rpl_owner) {
        /* STOP WTR & WTB */
        erpg->erps_instance.wtr_running = 0;
        erpg->erps_instance.wtr_timeout = 0;
        erpg->erps_instance.wtb_running = 0;
        erpg->erps_instance.wtb_timeout = 0;

        port = ((erpg->blocked_port == erpg->west_port) ? erpg->east_port : erpg->west_port);
        if (erpg->rpl_blocked == RAPS_RPL_BLOCKED) {
            /* unblock non-rpl port */
            VTSS_ERPS_UNBLOCK_RING_PORT(port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(port);

            /* TX R-APS (NR,RB,DNF) */
            VTSS_ERPS_START_TX_RAPS;
            VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,TRUE,TRUE);
        } else {
            /* block rpl port */
            VTSS_ERPS_BLOCK_RING_PORT(erpg->blocked_port);
            VTSS_ERPS_DISABLE_RAPS_FORWARDING(erpg->blocked_port);

            /* unblock non-rpl port */
            VTSS_ERPS_UNBLOCK_RING_PORT(port);
            VTSS_ERPS_ENABLE_RAPS_FORWARDING(port);

            /* TX R-APS (NR,RB) */
            VTSS_ERPS_START_TX_RAPS;
            VTSS_ERPS_BUILD_NEW_RAPS_PDU(ERPS_REQ_NR,0,TRUE,FALSE);

            /* flush FDB */
            VTSS_ERPS_FLUSH_FDB;
        }

        /* move to IDLE State */
        erpg->erps_instance.current_state = ERPS_STATE_IDLE;
        vtss_erps_trace_state_change("moving from PENDING to IDLE State ::: group =",erpg->group_id);
    }
    return (ERPS_RC_OK);
}

/******************************************************************************
                               ERPS Protocol related functions
******************************************************************************/
#if 0 // TODO:: commenting because of lint errors.. need to have this function
/* 
 * Interconnection flush logic as per ITUT-T G.8032(V2) Section 10.1.11, it receives
 * topology change notification information from other connected entities, such
 * as sub-ring's ERP control process and MI_RAPS_Propogate_TC management 
 * information.  Based on this information, it may initiate flysing of the FDB 
 * for the local ring ports and may trigger transmission of R-APS event requests
 * to both ring ports.
 *
 * This logic is included on the ERP control processes of the interconnection
 * nodes of Ethernet Rings that sub-rings are connected to.  This logic is not
 * present on Ethernet Ring nodes that are not Interconnected nodes.
 */
static i32 vtss_erps_apply_interconnection_flush_logic (u32 erpg_instance)
{
    erps_protection_group_t   *erpg = NULL;

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erpg_instance];

    if (!erpg->interconnected_node) {
        vtss_erps_trace("\n Ring node is not an interconnected node, thus not acting on it"
                "associated with it",0);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_NOT_AN_INTERCONNECTED_NODE);
    }

    /* Flush FDB */
    VTSS_ERPS_FLUSH_FDB;

    /* TODO::: Need to generate EVENT message to other ring nodes */

    ERPS_CRITD_EXIT();

    return (ERPS_RC_OK);
}
#endif

BOOL vtss_comapre_node_id (u8 *stored, u8 *received)
{
    u8 i;

    for (i = 0; i < ERPS_MAX_NODE_ID_LEN ; i++ ) {
        if (stored[i] != received[i]) {
            return (FALSE); 
        }
    }
    return (TRUE); 
}
/* 
 * flush logic as per ITUT-T G.8032 Section 10.1.10, this is only 
 * applicable for incoming R-APS frames
 */
static i32 vtss_erps_apply_flush_logic (erps_protection_group_t *erpg, u32 port)
{
    u32 other = (port == 0) ? 1 : 0;

    if ((erpg->erps_instance.req[port] == ERPS_REQ_NR) && !erpg->erps_instance.rb[port]) {
        /* clear bpr & node_id pair */
        memset(erpg->erps_instance.stored_fl[port].node_id,0,ERPS_MAX_NODE_ID_LEN);
        erpg->erps_instance.stored_fl[port].bpr = 0;
    } else {
        if ((vtss_comapre_node_id(erpg->erps_instance.stored_fl[port].node_id, erpg->erps_instance.rcvd_fl[port].node_id) == FALSE) ||
             (erpg->erps_instance.stored_fl[port].bpr != erpg->erps_instance.rcvd_fl[port].bpr)) {
            memcpy(erpg->erps_instance.stored_fl[port].node_id, erpg->erps_instance.rcvd_fl[port].node_id, ERPS_MAX_NODE_ID_LEN);
            erpg->erps_instance.stored_fl[port].bpr = erpg->erps_instance.rcvd_fl[port].bpr;

            if ((vtss_comapre_node_id(erpg->erps_instance.stored_fl[other].node_id, erpg->erps_instance.stored_fl[port].node_id) == FALSE) ||
                (erpg->erps_instance.stored_fl[other].bpr != erpg->erps_instance.stored_fl[port].bpr)) {
                if (!erpg->erps_instance.dnf[port]) {
                    VTSS_ERPS_FLUSH_FDB;
                }
            }
        }
    }
    return (ERPS_RC_OK);
}

/* based upon Section 10.1.1 ITUT.G8032/Y.1344 (03/2010) */
static i32 vtss_erps_apply_priority_logic(erps_protection_group_t *erpg,  i32 *request)
{
    i32 tmp, prio_req = 0;
    i32 erps_events[FSM_EVENTS_MAX];

    /* Clear the events array */
    memset(erps_events, 0, sizeof(erps_events));
    /* request event may be any one of the following: wtb_expire, wtr_expire, local_sf, 
       local_clear_sf, local FS, local MS, local Clear or R-APS event. Remember that
       local FS, local MS or local SF may be previously enforced and will be handled
       in this priority logic.
     */
    erps_events[*request] = 1;
    /* Ignore Local SF in FS state */
    if ((*request == FSM_EVENT_LOCAL_SF) && 
        (erpg->erps_instance.current_state == ERPS_STATE_FORCED_SWITCH)) {
        erps_events[*request] = 0;
    }
    /* Check for SF condition when FS is removed */
    if ((erpg->erps_instance.lsf_port_east || erpg->erps_instance.lsf_port_west) && 
        (erpg->erps_instance.current_state != ERPS_STATE_FORCED_SWITCH)){
            vtss_erps_trace("Local SF condition exists",0);
            erps_events[FSM_EVENT_LOCAL_SF] = 1;
    }
    /* If one of Local FS or Local MS are already enforced previously, it will be 
       present in admin_cmd */
    if ((erpg->erps_instance.admin_cmd == FSM_EVENT_LOCAL_FS) || 
        (erpg->erps_instance.admin_cmd == FSM_EVENT_LOCAL_MS)) {
        vtss_erps_trace("local FS or MS in effect previously",0);
        erps_events[erpg->erps_instance.admin_cmd] = 1;
    }
    /* Get the Top most priority event. Lower the priority value, higher the priority */
    for (tmp = 0; tmp < FSM_EVENTS_MAX; tmp++) {
        if (erps_events[tmp] == 1) {
            vtss_erps_trace("Top most priority event is =", tmp);
            prio_req = tmp;
            break;
        }
    }
    /* if highest priority request event is FS or MS, admin_cmd needs to be updated */
    if ((prio_req == FSM_EVENT_LOCAL_FS) || (prio_req == FSM_EVENT_LOCAL_MS)) {
        vtss_erps_trace("admin_cmd will be updated for FS or MS",0);
        erpg->erps_instance.admin_cmd = *request;
    }
    /* Forget local FS or MS if there is higher priority event */
    if ((erpg->erps_instance.admin_cmd == FSM_EVENT_LOCAL_FS) || 
            (erpg->erps_instance.admin_cmd == FSM_EVENT_LOCAL_MS)) {
        if (prio_req < erpg->erps_instance.admin_cmd) {
            erpg->erps_instance.admin_cmd = FSM_EVENT_INVALID;
        }
    }
    /* If highest priority event is FSM_EVENT_LOCAL_SF, FSM needs to be executed. This 
       is because SF can exist after clearing FS */
    if (prio_req == FSM_EVENT_LOCAL_SF){
        *request = FSM_EVENT_LOCAL_SF;
    }
    if (*request == prio_req) {
        return ERPS_RC_OK;
    } else {
        return ERPS_PRIORITY_LOGIC_FAILED;
    }
}

/******************************************************************************
                                  ERPS Platform Interface 
*******************************************************************************/
/* R-APS PDU rx function */
i32 vtss_erps_rx (vtss_port_no_t port, u16 pdu_len, u8 *raps_pdu,u32 erpg_instance)
{
    erps_pdu_t                r_pdu;
    i32                       fsm_event;
    erps_protection_group_t   *erpg;
    i32                       fsm_ret;
    u8                        null_mac[ERPS_MAX_NODE_ID_LEN];

    memset(null_mac, 0, ERPS_MAX_NODE_ID_LEN);

    if (erpg_instance >= ERPS_MAX_PROTECTION_GROUPS) {
        vtss_erps_trace("protection group id not with in valid range",0);
        return (ERPS_ERROR_GROUP_NOT_EXISTS);
    }

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erpg_instance];

    /* 
     * incase of RPL Owner Node, don't process R-APS PDU's unless the
     * protection group state is active, for Non-RPL nodes, only when
     * they receive NR_RB they moves into ACTIVE State 
     */
     
    if (erpg->rpl_owner && erpg->erps_status != ERPS_PROTECTION_GROUP_ACTIVE) {
	erpg->erps_instance.erps_stats.raps_rx_dropped++;
        vtss_erps_trace("protection group not moved to active state at RPL Owner node",0);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_PG_IN_INACTIVE_STATE);
    }

    if (!memcmp(raps_pdu+2, null_mac, ERPS_MAX_NODE_ID_LEN)) { 
        erpg->erps_instance.erps_stats.raps_rx_dropped++;
            vtss_erps_trace("No pdu received - 5 sec. timeout .. discarding grp =", erpg->group_id);
            if (port == erpg->east_port)   erpg->erps_instance.raps_rx[0] = 0;
            else                           erpg->erps_instance.raps_rx[1] = 0;
            ERPS_CRITD_EXIT();
            return (ERPS_RC_OK);
    }

    /* parsing incoming R-APS PDU */
    if (erps_parse_raps_pdu (erpg, raps_pdu, pdu_len, &r_pdu, port) != ERPS_RC_OK) {
        erpg->erps_instance.erps_stats.raps_rx_dropped++;
        vtss_erps_trace("error in erps_parse_raps_pdu group =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return -100;
    }

    /* 
     * update timestamp for received R-APS pdu, updating timestamp for all the
     * received R-APS PDU's except the ones that are not self generated 
     */
    erpg->erps_instance.rx_timestamp = vtss_erps_current_time();
     
    /* 
     * check whether packet is originated by this node, if so don't process
     * this one i.e this packet originated by this node, travelled all over the ring
     * and again reached again this us, such packets need not to be processed 
     */
    if ((!memcmp(r_pdu.node_id,erpg->node_id_w,ERPS_MAX_NODE_ID_LEN)) ||
        (!memcmp(r_pdu.node_id,erpg->node_id_e,ERPS_MAX_NODE_ID_LEN))) { 
            vtss_erps_trace("pdu received sent by this switch .. discarding grp =", erpg->group_id);
            ERPS_CRITD_EXIT();
            return (ERPS_RC_OK);
    }

    /* erps_validate_raps_pdu function should detect "event" request according to 10.1.6*/
    if (erps_validate_raps_pdu (r_pdu, erpg) != ERPS_RC_OK) {
        vtss_erps_trace("R-APS pdu validation failed, discarding pdu grp =", erpg->group_id); 
        erpg->erps_instance.erps_stats.raps_rx_dropped++;
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_RAPS_PARSING_FAILED);
    }

    /*
     * If received raps frame with NR-RB and node_id differs from self
     * node_id, this need to be informed to the management....
     * ITUT-G.8032 Section 10.4 
     */
    if (erpg->rpl_owner) {
        if ((r_pdu.req_state == ERPS_REQ_NR) && (r_pdu.rb)) {
            erpg->erps_instance.fop_alarm = TRUE;
            erpg->erps_instance.fop_timeout = ERPS_FOP_TIMEOUT;
            erpg->erps_instance.fop_running = 1;
        }
    }

    /* Handle event message from interconnection node */
    if (r_pdu.req_state == ERPS_REQ_EVENT) {
        vtss_erps_trace("R-APS Event message received..flushing FDB", 0);
        VTSS_ERPS_FLUSH_FDB;
        ERPS_CRITD_EXIT();
        return (ERPS_RC_OK);
    }

    if (erpg->west_port) {
    /* after validity check, it's time to apply flush logic */
    if (vtss_erps_apply_flush_logic(erpg, ((port==erpg->east_port) ? 0 : 1)) != ERPS_RC_OK) {
        vtss_erps_trace("Apply flush logic failed, discarding pdu grp =", erpg->group_id); 
        erpg->erps_instance.erps_stats.raps_rx_dropped++;
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR);
    }
    }

    /*
     * when guard timer is running no need to process R-APS PDU's destined to
     * a protection group. guard-timer should not discard EVENT messages as 
     * per ITU-T G.8032/Y.1344 (03/2010) Section 10.1.6
     */
    if (erpg->erps_instance.guard_timer_running && r_pdu.req_state != ERPS_REQ_EVENT ) {
        vtss_erps_trace("Gurad Timer running thus dropping R-APS PDU's grp =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_RC_OK);
    }

    /* 
     * each incoming R-APS frame need to undergo priority logic as per
     * section 10.1.1 as per ITUT.G.8032/Y.1344 (03/2010) 
     */
    fsm_event = erps_map_incoming_event_to_fsm_event(r_pdu.req_state, r_pdu.rb);
    if (fsm_event == ERPS_ERROR_INVALID_REMOTE_EVENT) {
       /* received invalid event, discard the R-APS PDU */
       erpg->erps_instance.erps_stats.raps_rx_dropped++;
       ERPS_CRITD_EXIT();
       return (fsm_event);
    }

    vtss_erps_trace("vtss_erps_rx called  port =", port);
    if (vtss_erps_apply_priority_logic(erpg, &fsm_event) == ERPS_PRIORITY_LOGIC_FAILED) {
        ERPS_CRITD_EXIT();
        return (ERPS_PRIORITY_LOGIC_FAILED);
    }

    /* If we now have state LOCAL_SF, it's because the other port has failed earlier -- we
     * just RX'd a PDU on this port, so we know it's OK. Thus, we're about to process the local
     * event handler, and it relies on l_event_port to be set to the failed port. So that's
     * what we'll do here.
     */
    if (fsm_event == FSM_EVENT_LOCAL_SF) {
        erpg->erps_instance.l_event_port = (port == erpg->east_port) ? erpg->erps_instance.lsf_port_west : erpg->erps_instance.lsf_port_east;
        vtss_erps_trace("Local SF, setting l_event_port =", erpg->erps_instance.l_event_port);
    }

    /* invocation of appropriate event handler */
    vtss_erps_trace("invoking event handler for event =", fsm_event);

    /* invocation of appropriate event handler */
    fsm_ret = ERPS_FSM[erpg->erps_instance.current_state-1][fsm_event].fptr(erpg);
    vtss_erps_trace("event handler returning  return =", fsm_ret);

    /* increment counters for received valid R-APS pdu */
    erpg->erps_instance.erps_stats.raps_rcvd++;

    ERPS_CRITD_EXIT();
    return (ERPS_RC_OK);
}

static i32 erps_handle_local_event (i32 event_in, u32 erpg_instance, vtss_port_no_t lport)
{
    erps_protection_group_t   *erpg;
    i32 fsm_ret = ERPS_ERROR_INVALID_LOCAL_EVENT;
    i32 l_event = event_in;

    ERPS_CRITD_ENTER();
    erpg = &erpg_instances[erpg_instance];

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_INACTIVE) {
        vtss_erps_trace("<FSM>: PG not in active state group = ", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_PG_IN_INACTIVE_STATE);
    }

    if (l_event == FSM_EVENT_LOCAL_SF)
        ((lport == erpg->east_port) ? (erpg->erps_instance.lsf_port_east = lport) : (erpg->erps_instance.lsf_port_west = lport));

    if (l_event == FSM_EVENT_LOCAL_CLEAR_SF)
        ((lport == erpg->east_port) ? (erpg->erps_instance.lsf_port_east = 0) : (erpg->erps_instance.lsf_port_west = 0));

    if (erpg->erps_status == ERPS_PROTECTION_GROUP_RESERVED) {
        vtss_erps_trace("<FSM>: PG not in active state group =", erpg->group_id);
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR_PG_IN_INACTIVE_STATE);
    }

    vtss_erps_trace("erps_handle_local_event called", 0);
    if (vtss_erps_apply_priority_logic(erpg, &l_event) != ERPS_RC_OK)
    {
        ERPS_CRITD_EXIT();
        return (ERPS_ERROR);
    }

    erpg->erps_instance.l_event_port = lport;
	        fsm_ret = ERPS_FSM[erpg->erps_instance.current_state-1][l_event].fptr(erpg);
    ERPS_CRITD_EXIT();
    return (fsm_ret);
}

void vtss_erps_timer_thread(void)
{
    u16  cnt;
    erps_protection_group_t * erpg;
    i32 fsm_ret, req;

    ERPS_CRITD_ENTER();
    for ( cnt = 0; cnt < ERPS_MAX_PROTECTION_GROUPS ; cnt++ ) {
        erpg =  &erpg_instances[cnt];
        if (erpg->erps_status == ERPS_PROTECTION_GROUP_ACTIVE ) {
            if (erpg->erps_instance.wtr_timeout && erpg->erps_instance.wtr_running) {
	            erpg->erps_instance.wtr_timeout=erpg->erps_instance.wtr_timeout-10;
            }
            if (erpg->erps_instance.wtb_timeout && erpg->erps_instance.wtb_running) {
	            erpg->erps_instance.wtb_timeout=erpg->erps_instance.wtb_timeout-10;
            } 
            if (erpg->erps_instance.hold_off_timeout && erpg->erps_instance.hold_off_running ) {
	            erpg->erps_instance.hold_off_timeout-=10;
            } 
            if (erpg->erps_instance.guard_timeout && erpg->erps_instance.guard_timer_running) {
                erpg->erps_instance.guard_timeout -= 10;
            }
            if (erpg->tc_timeout && erpg->tc_running) {
                erpg->tc_timeout -= 10;
            }
            /* Failure of Protocol timeout */
            if (erpg->erps_instance.fop_timeout && erpg->erps_instance.fop_running) {
                erpg->erps_instance.fop_timeout -= 10;
            }
            /* If topology change time expires, change the topology_change to FALSE */
            if (!erpg->tc_timeout && erpg->tc_running) {
                vtss_erps_trace("Topology_Change timeout for the group =", erpg->group_id);
                erpg->tc_running = 0;
                erpg->topology_change = FALSE;
            }
            if (!erpg->erps_instance.fop_timeout && erpg->erps_instance.fop_running) {
                vtss_erps_trace("FOP alarm timeout for the group =", erpg->group_id);
                erpg->erps_instance.fop_running = 0;
                erpg->erps_instance.fop_alarm = FALSE;
            }
            if (!erpg->erps_instance.wtr_timeout && erpg->erps_instance.wtr_running ) {
                /* looks like WTR is timedout.. need to inform the same to the protection group  */
                erpg->erps_instance.wtr_running = 0;
                req = FSM_EVENT_LOCAL_WTR_EXPIRES;
                vtss_erps_trace("FSM_EVENT_LOCAL_WTR_EXPIRES called", 0);
                if (vtss_erps_apply_priority_logic(erpg, &req) == ERPS_RC_OK) {
                    fsm_ret = ERPS_FSM[erpg->erps_instance.current_state-1][req].fptr(erpg);
                    if (fsm_ret != ERPS_RC_OK) {
                        vtss_erps_trace("error in executing FSM handler",0);
                    }
                }
            }
            if (!erpg->erps_instance.wtb_timeout && erpg->erps_instance.wtb_running ) {
            /* looks like WTB is timedout.. need to inform the same to the protection group */
                req = FSM_EVENT_LOCAL_WTB_EXPIRES;
                vtss_erps_trace("FSM_EVENT_LOCAL_WTB_EXPIRES called", 0);
                if (vtss_erps_apply_priority_logic(erpg, &req) == ERPS_RC_OK) {
                erpg->erps_instance.wtb_running = 0;
                    fsm_ret = ERPS_FSM[erpg->erps_instance.current_state-1][req].fptr(erpg);
                if (fsm_ret != ERPS_RC_OK) {
                    vtss_erps_trace("error in executing FSM handler",0);
                    }
                }
            }
            if (!erpg->erps_instance.hold_off_timeout && erpg->erps_instance.hold_off_running ) {
	        /* looks like HOLD off is timeout.. need to inform the same to the protection group */
	            erpg->erps_instance.hold_off_running = 0;
                fsm_ret = ERPS_FSM[erpg->erps_instance.current_state-1][FSM_EVENT_LOCAL_HOLDOFF_EXPIRES].fptr(erpg);
                if (fsm_ret != ERPS_RC_OK) {
                    vtss_erps_trace("error in executing FSM handler",0);
                }
            }
            if (erpg->erps_instance.guard_timeout <= 0 && erpg->erps_instance.guard_timer_running) {
                erpg->erps_instance.guard_timer_running = 0; 
            }
        }
    }
    ERPS_CRITD_EXIT();
}
