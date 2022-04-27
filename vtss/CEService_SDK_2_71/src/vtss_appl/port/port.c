/*

 Vitesse Switch API software.

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
 
 $Id$
 $Revision$

*/

#include "main.h"
#include "misc_api.h"
#include "vtss_api_if_api.h"
#include "conf_api.h"
#include "port_api.h"
#include "port_custom_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "port.h"
#include "control_api.h"
#include "interrupt_api.h"
#ifdef VTSS_SW_OPTION_PHY
#include "phy_api.h"
#endif
#ifdef VTSS_SW_OPTION_VCLI
#include "port_cli.h"
#endif
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif
#include "topo_api.h"

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
#include "ce_max_api.h"
#endif

#ifdef VTSS_SW_OPTION_POE           
#include "poe_api.h"
#endif 

#ifdef VTSS_SW_OPTION_ICFG
#include "port_icli_functions.h" // For port_icfg_init
#if defined(VTSS_SW_OPTION_PORT_POWER_SAVINGS)
#include "port_power_savings_icli_functions.h"  /* For port_power_savings_icfg_init() */
#endif
#endif

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Structure for global variables */
static port_global_t port;
#define PHY_INTERRUPT 0x01
static cyg_flag_t    interrupt_wait_flag;
static BOOL fast_link_int[VTSS_PORTS];    /* this is indicating that a fast link failure interrupt has been received */
static BOOL reset_phy2sgmii[VTSS_PORTS];

/* Port configuration change flags */
#define PORT_CONF_CHANGE_NONE  0x00000000
#define PORT_CONF_CHANGE_PHY   0x00000001
#define PORT_CONF_CHANGE_MAC   0x00000002
#define PORT_CONF_CHANGE_FIBER 0x00000004
#define PORT_CONF_CHANGE_PHY_SGMII 0x00000008
#define PORT_CONF_CHANGE_ALL   0xffffffff

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "port",
    .descr     = "Port module."
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_ITER] = {
        .name      = "iter",
        .descr     = "Iterations ",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_SFP] = {
        .name      = "sfp",
        .descr     = "SFP",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_ICLI] = {
        .name      = "iCLI",
        .descr     = "ICLI",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define PORT_CRIT_ENTER()    critd_enter(&port.crit,    TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PORT_CRIT_EXIT()     critd_exit( &port.crit,    TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PORT_CB_CRIT_ENTER() critd_enter(&port.cb_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PORT_CB_CRIT_EXIT()  critd_exit( &port.cb_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define PORT_CRIT_ENTER()    critd_enter(&port.crit)
#define PORT_CRIT_EXIT()     critd_exit( &port.crit)
#define PORT_CB_CRIT_ENTER() critd_enter(&port.cb_crit)
#define PORT_CB_CRIT_EXIT()  critd_exit( &port.cb_crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS - board dependent functions                   */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

static BOOL port_sfp_sgmii_set(u8 *phy_id, vtss_port_no_t port_no);

/* Determine if port has a PHY */
BOOL is_port_phy(vtss_port_no_t port_no)
{
    return (port_custom_table[port_no].mac_if == VTSS_PORT_INTERFACE_SGMII || 
            port_custom_table[port_no].mac_if == VTSS_PORT_INTERFACE_QSGMII ? 1 : 0);
}

// Returns if a port is PHY (if the PHY is in pass through mode, it shall act as it is not a PHY)
BOOL port_phy(vtss_port_no_t port_no)
{
    return is_port_phy(port_no) && (port.status[VTSS_ISID_LOCAL][port_no].sfp.type != PORT_SFP_1000BASE_T);
}

// Used to skip ports, which has no capabilities when looping through all ports.
#define PORT_HAS_CAP(isid, port_no)  if (port_isid_port_cap(isid, port_no) == 0) {continue;}

port_cap_t port_isid_port_cap(vtss_isid_t isid, vtss_port_no_t port_no)
{
    return (port.cap_valid[isid][port_no] ? port.status[isid][port_no].cap :
            port.isid_added[isid] ? vtss_board_port_cap(port.board_type[isid], port_no) : 0);
}

static BOOL port_isid_phy(vtss_isid_t isid, vtss_port_no_t port_no)
{
    return (port.status[isid][port_no].cap & PORT_CAP_1G_PHY ? 1 : 0);
}

BOOL port_10g_phy(vtss_port_no_t port_no)
{
    return (port_custom_table[port_no].cap & PORT_CAP_10G_FDX ? 1 : 0);
}

// See port_api.h
BOOL port_stack_support_cap(u32 cap) {
  switch_iter_t   sit;
  port_iter_t pit;
  (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID_CFG);
 
  while (switch_iter_getnext(&sit)) {
      // Loop through all ports
      if (port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_STACK  | PORT_ITER_FLAGS_NORMAL | PORT_ITER_FLAGS_NPI) != VTSS_RC_OK) {
          T_E("iter_init error");
      }

      while (port_iter_getnext(&pit)) {
          port_cap_t port_cap = port_isid_port_cap(sit.isid, pit.iport);
          T_D("port_cap:0x%X, port:%d, iport:%d, cap:0x%X, bool:%d", port_cap, pit.uport, pit.iport, cap, port_cap & cap);
          if (port_cap & cap) {
              T_I("port_cap:0x%X, port:%d, iport:%d, cap:0x%X, bool:%d", port_cap, pit.uport, pit.iport, cap, port_cap & cap);
              return TRUE;
          }
      }
  }
  T_I("FALSE");
  return FALSE;
}

/* Determine port MAC interface */
static vtss_port_interface_t port_mac_interface(vtss_port_no_t port_no)
{
    T_DG_PORT(TRACE_GRP_SFP, port_no, "port_custom_table[port_no].mac_if:%d", port_custom_table[port_no].mac_if);
    return port_custom_table[port_no].mac_if;
}

/* Determine  MAC interface to SFP */
static vtss_port_interface_t port_mac_sfp_interface(vtss_port_no_t port_no)
{
    T_DG_PORT(TRACE_GRP_SFP, port_no, "port.mac_sfp_if:%d", port.mac_sfp_if[port_no]);
    return port.mac_sfp_if[port_no];
}

static vtss_rc phy_10g_speed_setup(vtss_port_no_t port_no, vtss_port_speed_t speed, BOOL autoneg, BOOL fc, BOOL ena)
{
#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_mode_t mode;
    BOOL skip_1g_setup = 0;
    vtss_phy_10g_clause_37_control_t ctrl;
    vtss_phy_10g_power_t power;

    if (vtss_phy_10g_mode_get(PHY_INST, port_no, &mode) != VTSS_RC_OK) {
        T_E("vtss_phy_10g_mode_get failed");
        return VTSS_RC_ERROR;
    }

    if (vtss_phy_10g_clause_37_control_get(PHY_INST, port_no, &ctrl) != VTSS_RC_OK) {
        T_E("vtss_phy_10g_mode_get failed");
        return VTSS_RC_ERROR;
    }

    if (vtss_phy_10g_power_get(PHY_INST, port_no, &power) != VTSS_RC_OK) {
        T_E("vtss_phy_10g_power_get failed");
    }

    if ((ena && power != VTSS_PHY_10G_POWER_ENABLE) || (!ena && power == VTSS_PHY_10G_POWER_ENABLE)) {
       power = ena ? VTSS_PHY_10G_POWER_ENABLE : VTSS_PHY_10G_POWER_DISABLE;
        if (vtss_phy_10g_power_set(PHY_INST, port_no, &power) != VTSS_RC_OK) {
            T_E("vtss_phy_10g_power_set failed");
        }
    }

    if ((mode.oper_mode == VTSS_PHY_1G_MODE && speed == VTSS_SPEED_1G && ctrl.enable == autoneg && ctrl.advertisement.symmetric_pause == fc) 
        || (mode.oper_mode == VTSS_PHY_LAN_MODE && speed == VTSS_SPEED_10G)) {
        return VTSS_RC_OK; /* Nothing to do */
    }
    if (speed == VTSS_SPEED_1G) {
        if (mode.oper_mode == VTSS_PHY_1G_MODE) {
            skip_1g_setup = 1;;
        } else {
            mode.oper_mode = VTSS_PHY_1G_MODE;
        }
        mode.xaui_lane_flip = 1;  /* Need to flip the lanes to match JR XAUI-lane-0 and 8487 XAUI-lane-0  */
    } else {
        mode.oper_mode = VTSS_PHY_LAN_MODE; 
        mode.xaui_lane_flip = 0;
    }                

    if (!skip_1g_setup) {
        if (vtss_phy_10g_mode_set(PHY_INST, port_no, &mode) != VTSS_RC_OK) {
            T_E("vtss_phy_10g_mode_set failed");
            return VTSS_RC_ERROR;
        }
    }

    if (speed == VTSS_SPEED_1G) {
        memset(&ctrl, 0, sizeof(vtss_phy_10g_clause_37_control_t));
        ctrl.enable = autoneg;
        ctrl.advertisement.fdx = 1;
        ctrl.advertisement.symmetric_pause = fc;
        ctrl.advertisement.asymmetric_pause = fc;
        ctrl.advertisement.remote_fault = (ena ? VTSS_PHY_10G_CLAUSE_37_RF_LINK_OK : VTSS_PHY_10G_CLAUSE_37_RF_OFFLINE);
        if (vtss_phy_10g_clause_37_control_set(PHY_INST, port_no, &ctrl)) {
            T_E("vtss_phy_10g_clause_37_control_set");
            return VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_CHIP_10G_PHY */
    return VTSS_RC_OK;
}

// Function for determine the auto neg speed and flow control configuration
// In : port_no - port in question 
// In/Out : phy - Pointer to where to put the phy setup.
static void port_phy_setup_aneg_get(vtss_port_no_t port_no, vtss_phy_conf_t *phy) {
    port_conf_t *conf = &port.config[VTSS_ISID_LOCAL][port_no];
    port_cap_t cap = port_custom_table[port_no].cap;

    /* Auto negotiation */
    phy->mode = VTSS_PHY_MODE_ANEG;
    phy->aneg.speed_10m_hdx = conf->autoneg && (cap & PORT_CAP_10M_HDX) && 
                               !(conf->adv_dis & PORT_ADV_DIS_10M_HDX) && !(conf->adv_dis & PORT_ADV_DIS_10M); 
    phy->aneg.speed_10m_fdx = conf->autoneg && (cap & PORT_CAP_10M_FDX) &&
                               !(conf->adv_dis & PORT_ADV_DIS_10M_FDX) && !(conf->adv_dis & PORT_ADV_DIS_10M); 
    phy->aneg.speed_100m_hdx = conf->autoneg && (cap & PORT_CAP_100M_HDX) &&
                               !(conf->adv_dis & PORT_ADV_DIS_100M_HDX) && !(conf->adv_dis & PORT_ADV_DIS_100M); 
    phy->aneg.speed_100m_fdx = conf->autoneg && (cap & PORT_CAP_100M_FDX) &&
                               !(conf->adv_dis & PORT_ADV_DIS_100M_FDX) && !(conf->adv_dis & PORT_ADV_DIS_100M); 
    phy->aneg.speed_1g_fdx = ((cap & PORT_CAP_1G_FDX) && 
                              !(conf->autoneg && 
                                (conf->adv_dis & PORT_ADV_DIS_1G_FDX)));
    phy->aneg.speed_1g_hdx = FALSE; // We don't support 1G half duplex
    phy->aneg.symmetric_pause  = conf->flow_control;
    phy->aneg.asymmetric_pause = conf->flow_control;
}

static vtss_rc port_setup(vtss_port_no_t port_no, u32 change)
{
    vtss_phy_conf_t               phy_setup, *phy=NULL;;
    vtss_port_conf_t              port_setup, *ps;
    vtss_port_clause_37_control_t control;
    vtss_port_clause_37_adv_t     *adv;
    vtss_rc                       rc = VTSS_OK, rc2;
    port_cap_t                    cap;
    int                           i, j;
    port_conf_t                   *conf;
    vtss_port_status_t            *status;
    port_shutdown_reg_t           *reg;
    cyg_tick_count_t              ticks;
    ulong                         msec;
    BOOL                          fc, pcs_force=0;

    
    T_R("enter, port_no: %u", port_no);
    i = (port_no - VTSS_PORT_NO_START);
    cap = port_custom_table[port_no].cap;

    if (cap & PORT_CAP_SFP_DETECT || cap & PORT_CAP_DUAL_SFP_DETECT) {
        T_DG_PORT(TRACE_GRP_SFP, port_no, "port_mac_sfp_interface");
        port.status[VTSS_ISID_LOCAL][i].mac_if = port_mac_sfp_interface(port_no);
    } else {
        T_DG_PORT(TRACE_GRP_SFP, port_no, "port_mac_interface");
        port.status[VTSS_ISID_LOCAL][i].mac_if = port_mac_interface(port_no);
    }              
    T_DG_PORT(TRACE_GRP_SFP, port_no, "mac_if:%d", port.status[VTSS_ISID_LOCAL][i].mac_if);
    conf = &port.config[VTSS_ISID_LOCAL][i];
    status = &port.status[VTSS_ISID_LOCAL][i].status;

    /* Flow control always disabled for stack ports */
    fc = conf->flow_control;
    T_NG_PORT(TRACE_GRP_SFP, port_no, "change:%d", change);
    if (change & PORT_CONF_CHANGE_PHY) {
        /* Configure port */
        if (conf->enable == 0 && status->link) {
            /* Do port shutdown callbacks */
            PORT_CRIT_EXIT(); /* Exit main region before entering CB region */
            PORT_CB_CRIT_ENTER();
            for (j = 0; j < port.shutdown_table.count; j++) {
                reg = &port.shutdown_table.reg[j];
                T_D("callback, j: %d (%s), port_no: %u", 
                    j, vtss_module_names[reg->module_id], port_no);
                ticks = cyg_current_time();

                /* Leave critical region before callback */
                reg->callback(port_no);
                
                ticks = (cyg_current_time() - ticks);
                if (ticks > reg->max_ticks)
                    reg->max_ticks = ticks;
                msec = 1000*ticks/CYGNUM_HAL_RTC_DENOMINATOR;
                T_D("callback done, j: %d (%s), port_no: %u, %u msec", 
                    j, vtss_module_names[reg->module_id], port_no, msec);
            }
            PORT_CB_CRIT_EXIT();
            PORT_CRIT_ENTER();
        }        
        if (port_phy(port_no)) {
            phy = &phy_setup;
            phy->mdi = VTSS_PHY_MDIX_AUTO; // always enable auto detection of crossed/non-crossed cables

            if (conf->enable) {                
                if ((conf->dual_media_fiber_speed == VTSS_SPEED_FIBER_100FX || conf->dual_media_fiber_speed == VTSS_SPEED_FIBER_1000X) &&
                    !conf->autoneg) {
                    /* Forced fiber mode */
                    phy->mode = VTSS_PHY_MODE_FORCED;
                    phy->forced.speed = conf->speed;
                    phy->forced.fdx = conf->fdx;
                
                } else if ((cap & PORT_CAP_AUTONEG) &&
                           (conf->autoneg ||  conf->speed == VTSS_SPEED_1G)) {
                    /* Auto negotiation */
                    port_phy_setup_aneg_get(port_no, phy);
                } else {
                    /* Forced CU mode */
                    phy->mode = VTSS_PHY_MODE_FORCED;
                    phy->forced.speed = conf->speed;
                    phy->forced.fdx = conf->fdx;
                }
            } else {
                /* Power down */
                phy->mode = VTSS_PHY_MODE_POWER_DOWN;
            }
            if (phy->mode != VTSS_PHY_MODE_POWER_DOWN) {
                if ((rc = vtss_phy_conf_set(PHY_INST, port_no, phy)) != VTSS_OK) {
                    T_E("vtss_phy_conf_set failed, port_no: %u", port_no);
                }
            }
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
            {
                vtss_phy_power_conf_t power;

                power.mode = conf->power_mode;
                if ((rc2 = vtss_phy_power_conf_set(PHY_INST, port_no, &power)) != VTSS_OK) {
                    T_E("vtss_phy_power_conf_set failed, port_no: %u", port_no);
                    rc = rc2;
                }
            }
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
        } else if ((cap & PORT_CAP_AUTONEG) && 
                   (conf->speed == VTSS_SPEED_1G) && 
                   port_mac_sfp_interface(port_no) == VTSS_PORT_INTERFACE_SERDES) {
            T_NG_PORT(TRACE_GRP_SFP, port_no, "Clause 37 setup");
            /* PCS auto negotiation */
            pcs_force = 1;
            control.enable = conf->autoneg;
            adv = &control.advertisement;
            adv->fdx = 1;
            adv->hdx = 0;
            adv->symmetric_pause = fc;
            adv->asymmetric_pause = fc;
            adv->remote_fault = (conf->enable ? VTSS_PORT_CLAUSE_37_RF_LINK_OK :
                                 VTSS_PORT_CLAUSE_37_RF_OFFLINE);
            adv->acknowledge = 0;
            adv->next_page = 0;
            
            T_D_PORT(port_no, "set port via cluase_37, %s%s%s", adv->fdx ?"FDX " : "HDX ", fc ? ", Flow control": "", adv->remote_fault ? ", remote_fault":"" );
            if ((rc2 = vtss_port_clause_37_control_set(NULL, port_no, &control)) != VTSS_OK) {
                T_E("vtss_port_clause_37_control_set failed, port_no: %u", port_no);
                rc = rc2;
            }
        }
    }

    /* Port setup */
    ps = &port_setup;
    memset(ps, 0, sizeof(*ps));
    ps->if_type = (cap & PORT_CAP_SFP_DETECT || cap & PORT_CAP_DUAL_SFP_DETECT) ? port_mac_sfp_interface(port_no) : port_mac_interface(port_no);
    ps->power_down = (conf->enable ? 0 : 1);
    conf_mgmt_mac_addr_get(ps->flow_control.smac.addr, i + 1);
    ps->max_frame_length = conf->max_length;
    ps->max_tags = (conf->max_tags == PORT_MAX_TAGS_ONE ? VTSS_PORT_MAX_TAGS_ONE :
                    conf->max_tags == PORT_MAX_TAGS_NONE ? VTSS_PORT_MAX_TAGS_NONE :
                    VTSS_PORT_MAX_TAGS_TWO);
    ps->exc_col_cont = conf->exc_col_cont;
    ps->sd_enable = (cap & PORT_CAP_SD_ENABLE ? 1 : 0);
    ps->sd_active_high = (cap & PORT_CAP_SD_HIGH ? 1 : 0);
    ps->sd_internal = (cap & PORT_CAP_SD_INTERNAL ? 1 : 0);
    ps->xaui_rx_lane_flip = (cap & PORT_CAP_XAUI_LANE_FLIP ? 1 : 0);
    ps->xaui_tx_lane_flip = (cap & PORT_CAP_XAUI_LANE_FLIP ? 1 : 0);

    if (conf->autoneg && status->link && !(cap & PORT_CAP_10G_FDX)) {
        /* If autoneg and link up, status values are used */
        ps->speed = status->speed;
        T_IG_PORT(TRACE_GRP_SFP, port_no, "speed:%d", ps->speed);
        ps->fdx = status->fdx;
        if (ps->if_type == VTSS_PORT_INTERFACE_SGMII_CISCO) {
            ps->flow_control.obey = fc;
            ps->flow_control.generate = fc;        
        } else {
            ps->flow_control.obey =     pcs_force ? fc : status->aneg.obey_pause;
            ps->flow_control.generate = pcs_force ? fc : status->aneg.generate_pause;
        }
    } else {        
        /* If forced mode or link down, configured values are used */
        ps->speed = (conf->autoneg ? VTSS_SPEED_1G : conf->speed);
        ps->fdx = conf->fdx;
        ps->flow_control.obey = fc;
        ps->flow_control.generate = fc;        
    }

    if (!port_phy(port_no) && ps->if_type != VTSS_PORT_INTERFACE_SGMII_CISCO) {
        /* For non-aneg SFPs: Ensure that the speed matches the interface */
        if (!(cap & PORT_CAP_10G_FDX)) {
            if (ps->if_type == VTSS_PORT_INTERFACE_100FX) {
                ps->speed = VTSS_SPEED_100M;
            } else if (ps->if_type == VTSS_PORT_INTERFACE_VAUI) {
                ps->speed = VTSS_SPEED_2500M;
            } else {
                ps->speed = VTSS_SPEED_1G;
            }
        }
        if ((cap & PORT_CAP_10G_FDX) && ((cap & PORT_CAP_1G_FDX) || (cap & PORT_CAP_2_5G_FDX))) {
            if ((cap & PORT_CAP_1G_FDX) && (conf->speed == VTSS_SPEED_1G)) {
                ps->if_type = VTSS_PORT_INTERFACE_SERDES;
                ps->speed = VTSS_SPEED_1G;
            } else if ((cap & PORT_CAP_2_5G_FDX) && (conf->speed == VTSS_SPEED_2500M)) {
                ps->if_type = VTSS_PORT_INTERFACE_VAUI;
                ps->speed = VTSS_SPEED_2500M;
            }
        } else if (ps->if_type == VTSS_PORT_INTERFACE_XAUI && (cap & PORT_CAP_VTSS_10G_PHY) == 0) {
            /* XAUI port without 10G PHY, possibly change to RXAUI (HDMI) */
#if defined(VTSS_SW_OPTION_RXAUI)
            ps->if_type = VTSS_PORT_INTERFACE_RXAUI;
#endif /* VTSS_SW_OPTION_RXAUI */
        }
    }  

    ps->loop = (conf->adv_dis & PORT_ADV_UP_MEP_LOOP) ? VTSS_PORT_LOOP_PCS_HOST : VTSS_PORT_LOOP_DISABLE;

    if (conf->oper_up) {
        /* Force operational state up */
        vtss_port_state_set(NULL, port_no, 1);
    } else if (!status->link) {
        /* Set operational state down if link is down */
        vtss_port_state_set(NULL, port_no, 0);
    }
    
    if (conf->enable) {
        // Do configuration stuff that are board specific - before the port is enabled
        vtss_appl_api_lock();       /* Protect SGIO conf */
        if (ps->loop) {
            /* Disable loop port externally */
            conf->enable = 0;
        }
        port_custom_conf(port_no, conf, status);
        conf->enable = 1;
        vtss_appl_api_unlock();
    }

    T_IG_PORT(TRACE_GRP_SFP, port_no, "if_type = %d, speed:%d %s %s %s", ps->if_type, ps->speed, ps->fdx ? "FDX" : "HDX", ps->flow_control.obey ? "OBEY" : "", ps->flow_control.generate ? "GENERATE" : "" );
    if ((rc2 = vtss_port_conf_set(NULL, port_no, ps)) != VTSS_OK) {
        T_E("vtss_port_conf_set failed, port_no: %u", port_no);
        rc = rc2;
    }
    
    if (phy != NULL && phy->mode == VTSS_PHY_MODE_POWER_DOWN) {
        /* Note that the phy must be power down after the switch port is disabled */
        if ((rc = vtss_phy_conf_set(PHY_INST, port_no, phy)) != VTSS_OK) {
            T_E("vtss_phy_conf_set failed, port_no: %u", port_no);
        }
    }
     
    /* XAUI and VSC848x can run on 1G/10G mode */
    if ((cap & PORT_CAP_VTSS_10G_PHY) && (cap & PORT_CAP_1G_FDX)) {
        if (phy_10g_speed_setup(port_no, (conf->speed == VTSS_SPEED_2500M) ? VTSS_SPEED_1G : conf->speed, 
                                conf->autoneg, conf->flow_control, conf->enable) != VTSS_RC_OK) {
            rc = VTSS_RC_ERROR;
        }
    }

    if (!conf->enable) {
        // Do configuration stuff that are board specific - after the port is disabled.
        vtss_appl_api_lock();       /* Protect SGIO conf */
        port_custom_conf(port_no, conf, status);
        vtss_appl_api_unlock();
    }
   
    if ( (change & PORT_CONF_CHANGE_PHY_SGMII) && VTSS_PORT_INTERFACE_SGMII_CISCO == ps->if_type ) {
        T_D("Set port %d to SGMII mode", port_no);

        if (board_sfp_i2c_lock(1) == VTSS_RC_OK) { 
            port_sfp_sgmii_set(NULL, port_no);
            board_sfp_i2c_lock(0);
        } else {
            reset_phy2sgmii[port_no] = TRUE;
        }
    }

    T_D("exit, port_no: %u", port_no);
    return rc;
}

/* Convert port status to port info */
static void port_status2info(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    port_status_t *status = &port.status[isid][port_no];

    info->link = status->status.link;
    info->speed = status->status.speed;
    info->fdx = status->status.fdx;
    info->fiber = status->fiber;
    info->stack = port_isid_port_no_is_stack(isid, port_no);
    info->phy = port_isid_phy(isid, port_no);
    info->chip_no   = status->chip_no;
    info->chip_port = status->chip_port;
}

/* Port change event callbacks */
static void port_change_event(vtss_port_no_t port_no, port_status_t *status, BOOL changed)
{
    int               i;
    port_change_reg_t *reg;
    port_info_t       info;
    cyg_tick_count_t  ticks;
    ulong             msec;

    /* Leave critical region before doing callbacks */
    port_status2info(VTSS_ISID_LOCAL, port_no, &info);
    PORT_CRIT_EXIT();

    PORT_CB_CRIT_ENTER();
    for (i = 0; i < PORT_CHANGE_REG_MAX; i++) {
        reg = &port.change_table.reg[i];
        if (i < port.change_table.count && (changed || VTSS_PORT_BF_GET(reg->port_done, port_no) == 0)) {
            /* Port state changed or initial event not done */
            VTSS_PORT_BF_SET(reg->port_done, port_no, 1);
            T_D("callback, i: %d (%s), port_no: %u",  i, vtss_module_names[reg->module_id], port_no);
            ticks = cyg_current_time();
            reg->callback(port_no, &info);
            ticks = (cyg_current_time() - ticks);
            if (ticks > reg->max_ticks)
                reg->max_ticks = ticks;
            msec = 1000*ticks/CYGNUM_HAL_RTC_DENOMINATOR;
            T_D("callback done, i: %d (%s), port_no: %u, %u msec", i, vtss_module_names[reg->module_id], port_no, msec);
        }
    }
    PORT_CB_CRIT_EXIT();

    /* Enter critical region again */
    PORT_CRIT_ENTER();
}

/* Port global change event callbacks */
static void port_global_change_event(vtss_isid_t isid, vtss_port_no_t port_no,
                                     port_info_t *info)
{
    int                      i;
    port_global_change_reg_t *reg;
    cyg_tick_count_t         ticks;
    ulong                    msec;
    
#ifdef VTSS_SW_OPTION_SYSLOG
    {
        char buf[80], *p = &buf[0];
        p += sprintf(p, "Link %s on ", info->link ? "up" : "down");
#if VTSS_SWITCH_STACKABLE
        p += sprintf(p, "switch %u, ", topo_isid2usid(isid));
#endif /* VTSS_SWITCH_STACKABLE */
        p += sprintf(p, "port %u", iport2uport(port_no));
        S_I(buf);
    }
#endif /* VTSS_SW_OPTION_SYSLOG */

    PORT_CB_CRIT_ENTER();
    for (i = 0; i < port.global_change_table.count; i++) {
        reg = &port.global_change_table.reg[i];
        T_D("callback, i: %d (%s), isid: %d, port_no: %u", 
            i, vtss_module_names[reg->module_id], isid, port_no);
        ticks = cyg_current_time();
        reg->callback(isid, port_no, info);
        ticks = (cyg_current_time() - ticks);
        if (ticks > reg->max_ticks)
            reg->max_ticks = ticks;
        msec = 1000*ticks/CYGNUM_HAL_RTC_DENOMINATOR;
        T_D("callback done, i: %d (%s), isid: %d, port_no: %u, %u msec", 
            i, vtss_module_names[reg->module_id], isid, port_no, msec);
    }
    PORT_CB_CRIT_EXIT();
}

/* Check and generate global port change events */
static void port_global_change_events(void)
{
    vtss_isid_t    isid;
    vtss_port_no_t port_no;
    port_info_t    info;
    uchar          flags;
    
    PORT_CRIT_ENTER();
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        
        if (!msg_switch_is_master() || !msg_switch_exists(isid))
            continue;

        for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) {
            flags = port.change_flags[isid][port_no];
            if (flags & PORT_CHANGE_ANY) {
                port.change_flags[isid][port_no] = 0;
                port_status2info(isid, port_no, &info);
                
                /* Leave critical region before doing callbacks */
                PORT_CRIT_EXIT();
    
                if (info.link) {
                    /* Link is up now, check for link down event first */
                    if (flags & PORT_CHANGE_DOWN) {
                        info.link = 0;
                        port_global_change_event(isid, port_no, &info);
                        info.link = 1;
                    } 
                    if (flags & PORT_CHANGE_UP)
                        port_global_change_event(isid, port_no, &info);
                } else {
                    /* Link is down now, check for link up event first */
                    if (flags & PORT_CHANGE_UP) {
                        info.link = 1;
                        port_global_change_event(isid, port_no, &info);
                        info.link = 0;
                    } 
                    if (flags & PORT_CHANGE_DOWN)
                        port_global_change_event(isid, port_no, &info);
                }
                
                /* Enter critical region again */
                PORT_CRIT_ENTER();
            }
        }
    }
    PORT_CRIT_EXIT();
}

/* Poll port status */
static void port_status_poll(vtss_port_no_t port_no, BOOL *changed, vtss_event_t *link_down, BOOL fast_link_failure)
{
    port_status_t      *port_status;
    vtss_port_status_t *status, old_status, *old_aneg_status;
    port_conf_t        *conf;
    int                i;
    vtss_rc            rc = VTSS_RC_OK;

    T_N_PORT(port_no, "enter");

    /* Get current status */
    i = (port_no - VTSS_PORT_NO_START);
    port_status = &port.status[VTSS_ISID_LOCAL][i];
    conf = &port.config[VTSS_ISID_LOCAL][i];
    status = &port_status->status;
    old_aneg_status = &port.aneg_status[i];
    old_status = *status;
    
    /* Must break up the port_status_get into a Phy or a Switch call to support the PHY_INST */
    T_N_PORT(port_no, "phy status get, conf->autoneg:%d", conf->autoneg);        
    if (port_phy(port_no) || (is_port_phy(port_no) && !conf->autoneg)) {  // If we are in forced mode, we do not support SFP pass through at the moment             
        if (port.status[VTSS_ISID_LOCAL][port_no].cap != 0) { // Do not poll status for port with no capabilities
            rc = vtss_phy_status_get(PHY_INST, port_no, status);
        }
    } else {
        u8 sfp_phy[2];

        T_RG_PORT(TRACE_GRP_SFP, port_no, "sfp.type:%d", port.status[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START].sfp.type);  

        if (vtss_port_status_get(NULL, port_no, status) != VTSS_OK) {
            T_E_PORT(port_no, "status_get failed");
            return; 
        }
        
        
        // Determine Flow control for CU SFPs
        if (port.status[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START].sfp.type == PORT_SFP_1000BASE_T) {
            // Is the a CU SFP - If it is then we need to read the phy registers in the SFP in order to determine speed and flow control.
            
            /* Must lock the SFP I2C to avoid that another process changes to another SFP */
            if (board_sfp_i2c_lock(1) != VTSS_RC_OK) {
                T_IG_PORT(TRACE_GRP_SFP, port_no, "board_sfp_i2c_lock issue");
                rc = VTSS_RC_ERROR;
            } else if ((rc = board_sfp_i2c_read(port_no, 0x56, 5, &sfp_phy[0], 2)) == VTSS_RC_OK) { // Reading flow control advertisement 
                // Determining flow control based upon SFP registers values.
                u16 reg5 = (sfp_phy[0] << 8) | sfp_phy[1]; // concat to 16 bit register.
                vtss_phy_conf_t phy_setup;
                
                port_phy_setup_aneg_get(port_no, &phy_setup); // Get the current phy setup
                
                vtss_phy_flowcontrol_decode_status(NULL, port_no, reg5, phy_setup, status); // Update Flow control status
                
                T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_data:0x%X 0x%X, reg5:0x%X  phy_setup.aneg.speed_1g_fdx:%d, rc:%d", 
                          sfp_phy[0], sfp_phy[1], reg5, phy_setup.aneg.speed_1g_fdx, rc);
                
            } else {
                T_IG_PORT(TRACE_GRP_SFP, port_no, "i2c");
            }

            // Open of other to access the SFP i2c
            if (board_sfp_i2c_lock(0) != VTSS_RC_OK) {
                T_IG_PORT(TRACE_GRP_SFP, port_no, "board_sfp_i2c unlock issue");
            }

            T_NG_PORT(TRACE_GRP_SFP, port_no, "rc:%d", rc);
        } else {
        }
    }

    /* Force link state to false as long as fast link failure 'hold' timer is running */    
    if (fast_link_failure)   
        status->link = false; 

    /* Detect link down and disable port */
    if ((!status->link || status->link_down) && old_status.link) {
        T_I("link down event on port_no: %u, status->link:%d, status->link_down:%d, old_status.link:%d", port_no, status->link, status->link_down, old_status.link);
        *changed = 1;
        *link_down = 1;
        old_status.link = 0;
        if (!conf->oper_up) {
            /* Set operational state down unless forced up */
            vtss_port_state_set(NULL, port_no, 0);
        }
        port_status->fiber = 0;
        port_change_event(port_no, port_status, 1);
        port_status->port_down_count++;
    }

    /* Detect link up or speed/duplex change and setup port */
    if (status->link && (!old_status.link ||
                         status->speed != old_status.speed ||
                         status->fdx != old_status.fdx)) {
        if (old_status.link) {
            T_D("speed/duplex change event on port_no: %u", port_no);
        } else {
            T_I("link up event on port_no: %u, speed: %u", port_no, status->speed);
            if (conf->autoneg) {
                if (!port_phy(port_no)) {
                    /* A port setup is only performed if there is something new to setup  */
                    if ((status->aneg.generate_pause != old_aneg_status->aneg.generate_pause) ||
                        (status->aneg.obey_pause     != old_aneg_status->aneg.obey_pause) ||
                        (status->speed               != old_aneg_status->speed) ||
                        (status->fdx                 != old_aneg_status->fdx)) {
                        port_setup(port_no, PORT_CONF_CHANGE_MAC);
                        *old_aneg_status = *status;
                    }
                } else {
                    port_setup(port_no, PORT_CONF_CHANGE_MAC);
                }
            }

            vtss_port_state_set(NULL, port_no, 1);
            
            T_D("port_custom_table[%u].cap =%u", port_no, port_custom_table[port_no].cap);
            port_status->fiber = status->fiber;
        }
        *changed = 1;
        port_change_event(port_no, port_status, 1);
        port_status->port_up_count++;
    }


    // Default expecting no power savings and don't expect power capable
    port_status->power.actiphy_capable       = FALSE;
    port_status->power.actiphy_power_savings = FALSE;
    port_status->power.perfectreach_capable  = FALSE;
    port_status->power.perfectreach_power_savings = FALSE;

#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    // When ActiPhy is enabled power is saved when the link is down
    if (port_phy(port_no)) {
        port_status->power.actiphy_capable = TRUE;
        port_status->power.perfectreach_capable = TRUE;
    }    
     
    if (port_phy(port_no) && !status->link && (conf->power_mode == VTSS_PHY_POWER_ACTIPHY || conf->power_mode == VTSS_PHY_POWER_ENABLED)) {
        port_status->power.actiphy_power_savings = TRUE;
    } 

    // When PerfectReach is enabled power is saved when the link is up
    if (status->link && (conf->power_mode == VTSS_PHY_POWER_DYNAMIC || conf->power_mode == VTSS_PHY_POWER_ENABLED)) {
        port_status->power.perfectreach_power_savings = TRUE;
    } 
    T_N_PORT(port_no, "PerSav:%d, PerCap:%d, actiSav:%d, actCap:%d, link:%d, power:%d",
        port_status->power.perfectreach_power_savings, port_status->power.perfectreach_capable, port_status->power.actiphy_power_savings, port_status->power.actiphy_capable, status->link, conf->power_mode);


#endif

    /* Initial port change event */
    port_change_event(port_no, port_status, 0);
    
    T_N_PORT(port_no, "exit");
}

static vtss_rc port_veriphy_start(vtss_port_no_t port_no, port_veriphy_mode_t mode)
{
    port_veriphy_t      *veriphy;
    u8 i;
    veriphy = &port.veriphy[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START];

    // Bugzilla#8911, Sometimes cable length is measured too long (but never too short), so veriphy is done multiple times and the shortest length is found. Start with the longest possible length.   
    for (i = 0; i  < 4; i++) {
        veriphy->result.status[i] = VTSS_VERIPHY_STATUS_UNKNOWN;
        veriphy->result.length[i] = 255;
        veriphy->variate_cnt[port_no] = 0;
        T_D_PORT(port_no, "Len:%d",  veriphy->result.length[i]);
    }
    veriphy->repeat_cnt = VERIPHY_REPEAT_CNT;

    return vtss_phy_veriphy_start(PHY_INST, port_no,
                                  mode == PORT_VERIPHY_MODE_BASIC ? 2 :
                                  mode == PORT_VERIPHY_MODE_NO_LENGTH ? 1 : 0);
}

/* Determine if port configuration has changed */
static u32 port_conf_change(port_conf_t *old, port_conf_t *new_)
{
    u32 change;

    if (new_->autoneg != old->autoneg ||
        new_->dual_media_fiber_speed != old->dual_media_fiber_speed) {
        /* Full change */
        change = PORT_CONF_CHANGE_ALL;
    } else if (new_->enable != old->enable ||
               new_->speed != old->speed ||
               new_->fdx != old->fdx ||
               new_->flow_control != old->flow_control ||
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
               new_->power_mode != old->power_mode ||
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
               new_->adv_dis != old->adv_dis) {
        /* PHY and MAC change */
        change = (PORT_CONF_CHANGE_PHY | PORT_CONF_CHANGE_MAC);
        /* since port disable have power down SFP, once port is configured to enable, SFP needs to be configured again */
        if ( new_->enable != old->enable && new_->enable ) {
            T_D("change PHY to SGMII");
            change |= PORT_CONF_CHANGE_PHY_SGMII;
        }
    } else if (new_->exc_col_cont != old->exc_col_cont ||
               new_->max_tags != old->max_tags ||
               new_->max_length != old->max_length ||
               new_->oper_up != old->oper_up) {
        /* MAC change only */
        change = PORT_CONF_CHANGE_MAC;
    } else {
        /* No change */
        change = PORT_CONF_CHANGE_NONE;
    }
    return change;
}

static BOOL port_vol_conf_changed(const port_vol_conf_t *old, const port_vol_conf_t *new_)
{
    return (old->disable != new_->disable || old->loop != new_->loop || old->oper_up != new_->oper_up ? 1 : 0);
}

/* Clear local port counters */
static vtss_rc port_counters_clear(vtss_port_no_t port_no)
{
    port_status_t *port_status = &port.status[VTSS_ISID_LOCAL][port_no];

    port_status->port_up_count = 0;
    port_status->port_down_count = 0;
    return vtss_port_counters_clear(NULL, port_no);
}

#if defined(VTSS_SW_OPTION_I2C)
/* Try to complete a SGMII-CISCO, to predict if the PCS should be configured to that mode */
static BOOL cisco_aneg_complete(vtss_port_no_t port_no)
{
    vtss_port_conf_t        conf, conf_tmp;
    vtss_port_status_t      status;
    u32                     loop=0;

    memset(&conf, 0, sizeof(vtss_port_conf_t));
    memset(&status, 0, sizeof(vtss_port_status_t));

    conf.if_type = VTSS_PORT_INTERFACE_SGMII_CISCO;
    conf.speed = VTSS_SPEED_1G;
    conf.fdx = 1;
    if ((vtss_port_conf_get(NULL, port_no, &conf_tmp)) != VTSS_OK && 
        (vtss_port_conf_set(NULL, port_no, &conf)) != VTSS_OK) {
        T_E("vtss_port_conf_set failed, port_no: %u", port_no);
    }
    while (loop < 10) {
        if (vtss_port_status_get(NULL, port_no, &status) != VTSS_OK) {
            T_E("vtss_port_status_get, port_no: %u", port_no);
        }
        if (status.aneg_complete) {
            return TRUE;        
        }
        loop++;
        VTSS_OS_MSLEEP(10);            
    }
    return FALSE;
}

static BOOL sfp_sgmii_support( u8 *sfp_phy)
{
    if ( sfp_phy[0] == 0x1 && sfp_phy[1] == 0x41 && sfp_phy[2] == 0xc && (sfp_phy[3] & 0xf0) == 0xc0 ) {
        /* The PHY is Marvell 88E1111 */
        return TRUE;
    }
    return FALSE;
}
static BOOL port_sfp_sgmii_set(u8 *phy_id, vtss_port_no_t port_no)
{
    u8 i2c_data[2];
    u8 sfp_rom[4];
    if ( NULL == phy_id ) {
        if ( board_sfp_i2c_read(port_no, 0x56, 2, &sfp_rom[0], 4) != VTSS_RC_OK || FALSE == sfp_sgmii_support(sfp_rom) ) {
            return FALSE;
        }
    } else if ( FALSE == sfp_sgmii_support(phy_id)) {
        return FALSE;
    }

    /* The SFP's PHY supports SGMII(at present, only Marvell 88E1111 is supported), execute follwoing setting to configure to SGMII mode */

    /* Configure to SGMII mode */
    i2c_data[0]= 0x90;
    i2c_data[1]= 0x84;
    board_sfp_i2c_write (port_no, 0x56, 27, i2c_data);

    /* Advertise 1000BASE-T Full/Half-Duplex */
    i2c_data[0]= 0xf;
    i2c_data[1]= 0x00;
    board_sfp_i2c_write (port_no, 0x56, 9, i2c_data);
    /* Apply Software reset */
    i2c_data[0]= 0x81;
    i2c_data[1]= 0x40;
    board_sfp_i2c_write (port_no, 0x56, 0, i2c_data);

    /* Advertise 10/100BASE-T Full/Half-Duplex */
    i2c_data[0]= 0xd;
    i2c_data[1]= 0xe1;
    board_sfp_i2c_write (port_no, 0x56, 4, i2c_data);

    /* Apply Software reset */
    i2c_data[0]= 0x91;
    i2c_data[1]= 0x40;
    board_sfp_i2c_write (port_no, 0x56, 0, i2c_data);
    return TRUE;
}

/* The function detects the SFP through standard SFP EEPROM    */
/* The detection process is according to the SFP MSA agreement */
vtss_rc sfp_detect(vtss_port_no_t port_no, port_sfp_t *sfp, BOOL *sgmii_cisco, BOOL *approved)
{
    u8     sfp_rom[64], sfp_phy[64];
    
    memset(sfp,0,sizeof(port_sfp_t));

    if (board_sfp_i2c_read(port_no, 0x50, 0, &sfp_rom[0], 60) != VTSS_RC_OK) {
        T_DG_PORT(TRACE_GRP_SFP, port_no, "Could not read from I2C");
        sfp->type = PORT_SFP_NONE;
        return VTSS_RC_OK;
    }    

    /* Get the Vendor codes */
    memcpy(&sfp->vendor_name, &sfp_rom[20], 16);
    sfp->vendor_name[17] = '\0';
    memcpy(&sfp->vendor_pn, &sfp_rom[40], 16);
    sfp->vendor_pn[17] = '\0';
    memcpy(&sfp->vendor_rev, &sfp_rom[56],  4);
    sfp->vendor_rev[5] = '\0';
    
    /* Should we support it? */
    if (!port_custom_sfp_accept(sfp_rom)) {
        *approved = 0; /* This means that this SFP is not on the apporoved list, even though it might work just fine... */
    }    
    /* Is it a SFP module? */
    if (sfp_rom[0] != 0x03) {
        T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_NONE");
        sfp->type = PORT_SFP_NONE;
        return VTSS_RC_OK;
    }

    /* 0x22 indicats RJ45 connector type, */
    if (sfp_rom[2] == 0x22 || sfp_rom[6] == 0x08) {
        /* Is there a phy in there? */
        if (board_sfp_i2c_read(port_no, 0x56, 0, &sfp_phy[0], 10) == VTSS_RC_OK) {
            /* Figure out if the phy supports CISCO-SGMII interface */
            *sgmii_cisco = cisco_aneg_complete(port_no);
            T_DG_PORT(TRACE_GRP_SFP, port_no, "host side Aneg is %s mode", *sgmii_cisco ? "SGMII" : "1000BASE-X");
            if ( FALSE == *sgmii_cisco ) {
                *sgmii_cisco = port_sfp_sgmii_set(&sfp_phy[4], port_no);
            }
        }
    }

    /* Is it a 1000BASE-X module? */
    if (sfp_rom[6] == 0x1 || sfp_rom[6] == 0x2 || sfp_rom[6] == 0x4) {
        if (sfp_rom[6] == 0x1) {
            T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_1000BASE_SX");
            sfp->type = PORT_SFP_1000BASE_SX;
        } else if (sfp_rom[6] == 0x2) {
            T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_1000BASE_LX");
            sfp->type = PORT_SFP_1000BASE_LX;
        } else {
            T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_1000BASE_CX");
            sfp->type = PORT_SFP_1000BASE_CX;  
        }
        return VTSS_RC_OK;
    }

    /* Is it a CU_SFP module? */
    if (sfp_rom[6] == 0x08) {
        T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_1000BASE_T");
        sfp->type = PORT_SFP_1000BASE_T;        

        if  (is_port_phy(port_no)) {
            T_IG_PORT(TRACE_GRP_SFP, port_no, "Dual media port with CU SFP");
            *sgmii_cisco = TRUE;

            // For all the cu SFP we know at the moment register 27 tells, if the cu SFP is running SGMII or BASE-X
            board_sfp_i2c_read(port_no, 0x56, 27, &sfp_phy[0], 1); // MSB of 16 bit cu SFP register
            board_sfp_i2c_read(port_no, 0x56, 27, &sfp_phy[1], 1); // LSB of 16 bit cu SFP register
            
            if ((sfp_phy[1] & 0xF) == 0x8 || (sfp_phy[1] & 0xF) == 0xC) { // Bit 3:0 indicate cu is running BASE-X for the cu SFPs we know. 0x0 & 0x4 = SGMII, 0x8 & 0xC = 1000BASE-X 
                *sgmii_cisco = FALSE;
                sfp->type    = PORT_SFP_1000BASE_X;
            }
                            
            T_IG_PORT(TRACE_GRP_SFP, port_no, "sfp_phy[0]:0x%X, sfp_phy[1]:0x%X, sgmii_cisco:%d", sfp_phy[0], sfp_phy[1], *sgmii_cisco);
        }

        T_RG_PORT(TRACE_GRP_SFP, port_no, "is_port_phy(port_no):%d, sfp_type:%d, sgmii_cisco:%d", is_port_phy(port_no), *sgmii_cisco);
        return VTSS_RC_OK;
    }
    /* Is it a 100FX module? */
    if (sfp_rom[6] == 0x10 || sfp_rom[6] == 0x20 || sfp_rom[6] == 0x40) {
        if (sfp_rom[6] == 0x40) {
            *sgmii_cisco = 1; /* All known BX (0x40) 100FX modules use SGMII-CISCO aneg  */
        }
        T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_100FX");
        sfp->type = PORT_SFP_100FX;  
        return VTSS_RC_OK;
    }
    /* It does not support Ethernet Compliance Codes. Find the max supported signaling rate */
    if (sfp_rom[6] == 0) {
        if (sfp_rom[12] >= 1 && sfp_rom[12] < 10) { 
            T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_100FX");
            sfp->type = PORT_SFP_100FX;
        } else if (sfp_rom[12] >= 10 && sfp_rom[12] < 25) {
            T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_1000BASE_X");
            sfp->type = PORT_SFP_1000BASE_X;
        } else if (sfp_rom[12] >= 25 && sfp_rom[12] < 50) {
            T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_2G5");
            sfp->type = PORT_SFP_2G5;
        } else if (sfp_rom[12] >= 50 && sfp_rom[12] < 100)  {
            T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_5G             ");
            sfp->type = PORT_SFP_5G;
        } else if (sfp_rom[12] >= 100) {
            sfp->type = PORT_SFP_10G;
        }
        return VTSS_RC_OK;
    }    
    T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_rom = PORT_SFP_NONE");

    sfp->type = PORT_SFP_NONE;
    return VTSS_RC_OK;
}

static vtss_port_interface_t port_sfp_vs_conf(vtss_port_no_t port_no, vtss_port_interface_t sfp_if)
{
    port_conf_t  *conf = &port.config[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START];    
    port_cap_t cap = port.status[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START].cap;  

    if (sfp_if != VTSS_PORT_INTERFACE_SGMII_CISCO) {
        if (conf->autoneg) {
            if (cap & PORT_CAP_1G_FDX) {
                return VTSS_PORT_INTERFACE_SERDES;
            }
        } else {
            if (conf->speed == VTSS_SPEED_100M && (cap & PORT_CAP_100M_FDX)) {
                return VTSS_PORT_INTERFACE_100FX;
            } else if (conf->speed == VTSS_SPEED_1G && (cap & PORT_CAP_1G_FDX)) {
                return VTSS_PORT_INTERFACE_SERDES;
            } else if (conf->speed == VTSS_SPEED_2500M && (cap & PORT_CAP_2_5G_FDX)) {
                return VTSS_PORT_INTERFACE_VAUI;
            }
        }
    }
    return sfp_if;
}

/* Detect the SFP and convert to 'vtss_port_interface_t' type */
static vtss_rc sfp_detect_if(vtss_port_no_t port_no, vtss_port_interface_t *sfp_if)  
{  
    port_sfp_t sfp;
    BOOL cisco_aneg=0, approved=1;
    port_cap_t cap;

    /* Must lock the SFP I2C to avoid that another process changes to another SFP */
    if (board_sfp_i2c_lock(1) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }

    /* Determine SFP type. If the port is a PHY port we expect that the PHYs i2c connection is used. */
    if (sfp_detect(port_no, &sfp, &cisco_aneg, &approved) != VTSS_RC_OK) {
        T_E("SFP detect failed port_no:%u",port_no);
    }

    T_IG_PORT(TRACE_GRP_SFP, port_no, "sfp_type:%d, cisco_aneg:%d", sfp.type, cisco_aneg);

    if (board_sfp_i2c_lock(0) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }

    /* Check if the SFP supports CISCO-SGMII-ANEG on the host side */
    if (cisco_aneg) {            
        *sfp_if = VTSS_PORT_INTERFACE_SGMII_CISCO;         
        T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_if:%d", *sfp_if);
    } else if (is_port_phy(port_no)) { // Do not change sfp_if for PHY ports that are not CU-SFPs (Pass through mode)
        *sfp_if = port_mac_interface(port_no);
    } else {
        /* Convert from SFP type to the appropriate vtss_port_interface_t type  */
        switch (sfp.type) {
        case PORT_SFP_NONE:
            *sfp_if = VTSS_PORT_INTERFACE_SERDES; /* Default to Serdes */
            break;
        case PORT_SFP_NOT_SUPPORTED:
            *sfp_if = VTSS_PORT_INTERFACE_NO_CONNECTION; 
            break;
        case PORT_SFP_100FX:
            *sfp_if = VTSS_PORT_INTERFACE_100FX; 
            break;
        case PORT_SFP_2G5:
        case PORT_SFP_5G:
        case PORT_SFP_10G:
            if (port_custom_table[port_no].cap & PORT_CAP_2_5G_FDX) {
                *sfp_if = VTSS_PORT_INTERFACE_VAUI; 
            } else {
                *sfp_if = VTSS_PORT_INTERFACE_SERDES; 
            }
            break;
        case PORT_SFP_1000BASE_T:
        case PORT_SFP_1000BASE_X:
        case PORT_SFP_1000BASE_CX:
        case PORT_SFP_1000BASE_LX:
        case PORT_SFP_1000BASE_SX:
            *sfp_if = VTSS_PORT_INTERFACE_SERDES; 
        }
    }

    T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_if:%d", *sfp_if);
  
    /* Update port capabilities */
    cap = port_custom_table[port_no].cap;
    switch (sfp.type) {
    case PORT_SFP_100FX:
        cap = cap & (0xFFFFFFFF ^ (PORT_CAP_AUTONEG | PORT_CAP_2_5G_FDX | PORT_CAP_1G_FDX | PORT_CAP_10M_FDX));
        break;
    case PORT_SFP_1000BASE_T:
        cap = cap & (0xFFFFFFFF ^ (PORT_CAP_2_5G_FDX | PORT_CAP_1G_FDX | PORT_CAP_100M_FDX | PORT_CAP_10M_FDX));
        break;
    case PORT_SFP_1000BASE_X:
    case PORT_SFP_1000BASE_CX:
    case PORT_SFP_1000BASE_LX:
    case PORT_SFP_1000BASE_SX:
        cap = cap & (0xFFFFFFFF ^ (PORT_CAP_2_5G_FDX));
        break;
    default:
        break;
    }

    if (!port_phy(port_no)) {
        port.status[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START].cap = cap;  
        *sfp_if = port_sfp_vs_conf(port_no, *sfp_if);
    }
    port.status[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START].sfp = sfp;  

    T_IG_PORT(TRACE_GRP_SFP, port_no, "sfp_if:%d, sfp_type:%d", *sfp_if, sfp.type);
    /* Update MAC interface type when the configured mode is considered */


    return VTSS_RC_OK;
} 
#endif /* VTSS_SW_OPTION_I2C */


/****************************************************************************/
/*  Misc support functions                                                  */
/****************************************************************************/
//
// Converts error to printable text
//
// In : rc - The error type
//
// Retrun : Error text
//
char *port_error_txt(vtss_rc rc)
{
    switch (rc) {
    case PORT_ERROR_INCOMPLETE:
        return "VeriPHY still running";

    case PORT_ERROR_PARM:
        return "Illegal parameter";

    case PORT_ERROR_REG_TABLE_FULL:
        return "Registration table full";

    case PORT_ERROR_REQ_TIMEOUT:
        return "Timeout on message request";

    case PORT_ERROR_STACK_STATE:
        return "Illegal MASTER/SLAVE state";

    case PORT_ERROR_MUST_BE_MASTER:
        return "This is not allow at slave switch. Switch must be master";

    case VTSS_RC_OK:
        return "";
    }

    T_I("rc:%d", rc);
    return "Unknown Port error";
}

// Function for converting sfp_tranceiver_t to a printable string
// In : sfp - SFP type to print.
char *sfp_if2txt(sfp_tranceiver_t sfp)
{
    switch (sfp) {
    case PORT_SFP_NONE:          return "None";
    case PORT_SFP_NOT_SUPPORTED: return "Not supported";
    case PORT_SFP_100FX:         return "100FX";
    case PORT_SFP_1000BASE_T:    return "1000BASE_T";
    case PORT_SFP_1000BASE_X:    return "1000BASE_X";
    case PORT_SFP_1000BASE_SX:   return "1000BASE_SX";
    case PORT_SFP_1000BASE_LX:   return "1000BASE_LX";
    case PORT_SFP_1000BASE_CX:   return "1000BASE_CX";
    case PORT_SFP_2G5:           return "2G5";
    case PORT_SFP_5G:            return "5G";
    case PORT_SFP_10G:           return "10G";
    default: return "?";
    }
}

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static const char *port_msg_id_txt(port_msg_id_t msg_id)
{
    const char *txt;

    switch (msg_id) {
    case PORT_MSG_ID_CONF_SET_REQ:
        txt = "CONF_SET_REQ";
        break;
    case PORT_MSG_ID_STATUS_GET_REQ:
        txt = "STATUS_GET_REQ";
        break;
    case PORT_MSG_ID_STATUS_GET_REP:
        txt = "STATUS_GET_REP";
        break;
    case PORT_MSG_ID_COUNTERS_GET_REQ:
        txt = "COUNTERS_GET_REQ";
        break;
    case PORT_MSG_ID_COUNTERS_GET_REP:
        txt = "COUNTERS_GET_REP";
        break;
    case PORT_MSG_ID_COUNTERS_CLEAR_REQ:
        txt = "COUNTERS_CLEAR_REQ";
        break;
    case PORT_MSG_ID_VERIPHY_GET_REQ:
        txt = "VERIPHY_GET_REQ";
        break;
    case PORT_MSG_ID_VERIPHY_GET_REP:
        txt = "VERIPHY_GET_REP";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/* Allocate request buffer */
static port_msg_req_t *port_msg_req_alloc(port_msg_id_t msg_id)
{
    port_msg_req_t *msg = (port_msg_req_t*)msg_buf_pool_get(port.request);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/* Allocate reply buffer */
static port_msg_rep_t *port_msg_rep_alloc(port_msg_id_t msg_id)
{
    port_msg_rep_t *msg = (port_msg_rep_t*)msg_buf_pool_get(port.reply);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/* Release port message buffer */
static void port_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    port_msg_id_t msg_id = *(port_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s", msg_id, port_msg_id_txt(msg_id));

    (void)msg_buf_pool_put(msg);
}

/* Send port message */
static void port_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    port_msg_id_t msg_id = *(port_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_R("msg_id: %d, %s, len: %zu, isid: %d", msg_id, port_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(NULL, port_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_PORT, isid, msg, len + MSG_TX_DATA_HDR_LEN_MAX(port_msg_req_t, req, port_msg_rep_t, rep));
}

// Function for setting the port_media in vtss_phy_reset_conf_t struct, according to configuration (aneg and preferred AMS)
// In: port_no - Port in question
//     gigabit - TRUE is fiber media shall be configured to 1000base-x else meida is configured to 100Fx
//     phy_reset - Pointer to the struct to update
//     conf      - Pointer to current port configuration.
static void phy_fiber_media(const vtss_port_no_t port_no, const BOOL gigabit, vtss_phy_reset_conf_t *phy_reset, const port_conf_t *conf) {

    if (gigabit) {
    //
    // 1000-BASE-X
    //
    if (conf->autoneg) { // AMS mode
      if (port_custom_table[port_no].cap & PORT_CAP_DUAL_COPPER) {
        // Cobber preferred
        phy_reset->media_if = VTSS_PHY_MEDIA_IF_AMS_CU_1000BX;
      } else {
        // Fiber preferred
        phy_reset->media_if = VTSS_PHY_MEDIA_IF_AMS_FI_1000BX;
      }
    } else {
      // Forced Fiber 
      phy_reset->media_if = VTSS_PHY_MEDIA_IF_FI_1000BX;
    }
  } else {
    //
    // 100-BASE-FX
    //
    if (conf->autoneg) { // AMS mode
      if (port_custom_table[port_no].cap & PORT_CAP_DUAL_COPPER_100FX) {
        // Cobber preferred
        phy_reset->media_if = VTSS_PHY_MEDIA_IF_AMS_CU_100FX; 
      } else {
        // Fiber preferred
        phy_reset->media_if = VTSS_PHY_MEDIA_IF_AMS_FI_100FX; 
      }
    }  else {
      // Forced Fiber 
      phy_reset->media_if = VTSS_PHY_MEDIA_IF_FI_100FX;
    }
  }
  T_IG_PORT(TRACE_GRP_SFP, port_no, "phy_reset.media_if:%d", phy_reset->media_if);
}

// Function for setting the SFP fiber mode for a given port (Only PHY ports).
// IN : port_no - The port for which to change fiber mode.
//      fiber_speed - The fiber speed at which the port shall be set to.
static void phy_fiber_speed_update(vtss_port_no_t port_no, vtss_fiber_port_speed_t fiber_speed) {
    vtss_phy_reset_conf_t phy_reset;
    port_conf_t                   *conf;
    phy_reset.mac_if = port_mac_interface(port_no);

    u8 i = (port_no - VTSS_PORT_NO_START);
    conf = &port.config[VTSS_ISID_LOCAL][i];

    T_IG_PORT(TRACE_GRP_SFP, port_no, "fiber_speed:%d, auto:%d", fiber_speed, conf->autoneg);
    if (is_port_phy(port_no)) {
        switch (fiber_speed) {
        case VTSS_SPEED_FIBER_1000X : 
            T_D_PORT(port_no, "phy_reset.media_if = VTSS_SPEED_FIBER_1000X");
            phy_fiber_media(port_no, TRUE, &phy_reset, conf);
            break;
        case VTSS_SPEED_FIBER_100FX:
            phy_fiber_media(port_no, FALSE, &phy_reset, conf);
            T_IG_PORT(TRACE_GRP_SFP, port_no, "phy_reset.media_if: VTSS_SPEED_FIBER_100FX, auto:%d, media:%d", conf->autoneg, phy_reset.media_if);
            break;
            
        case VTSS_SPEED_FIBER_AUTO:
            T_IG_PORT(TRACE_GRP_SFP, port_no, "media_if: VTSS_SPEED_FIBER_AUTO, sfp_type:%d", port.status[VTSS_ISID_LOCAL][port_no].sfp.type);
            if (port.status[VTSS_ISID_LOCAL][port_no].sfp.type == PORT_SFP_100FX) {
              phy_fiber_media(port_no, FALSE, &phy_reset, conf);
            } else if (port.status[VTSS_ISID_LOCAL][port_no].sfp.type == PORT_SFP_1000BASE_T) {
                // CU SFP 
                phy_reset.media_if = VTSS_PHY_MEDIA_IF_SFP_PASSTHRU;
            } else {
              // We treat all others a 1000BASE-X
              phy_fiber_media(port_no, TRUE, &phy_reset, conf);
            } 
            break;
            
        case VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED:
            phy_reset.media_if = VTSS_PHY_MEDIA_IF_CU;
            break;
            
        default:
            T_E_PORT(port_no, "Unknown Fiber mode: %d", fiber_speed) ;
            return;
            break;
        }
        vtss_phy_reset(PHY_INST, port_no, &phy_reset); // PHY reset needed in order to change mode

        T_IG_PORT(TRACE_GRP_SFP, port_no, "cisco_aneg_complete(port_no):%d", cisco_aneg_complete(port_no));
        T_IG_PORT(TRACE_GRP_SFP,port_no, "fiber_speed:%d, auto:%d, media:%d", fiber_speed, conf->autoneg, phy_reset.media_if);
    }
}

/* Receive port message */
static BOOL port_msg_rx(void *contxt, const void * const rx_msg, const size_t len, const vtss_module_id_t modid, const u32 isid)
{
    port_msg_id_t msg_id = *(port_msg_id_t *)rx_msg;
    u32           port_count = port.port_count[VTSS_ISID_LOCAL];

    T_R("msg_id: %d, %s, len: %zu, isid: %u", msg_id, port_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case PORT_MSG_ID_CONF_SET_REQ:
    {
        port_msg_req_t *msg = (port_msg_req_t *)rx_msg;
        vtss_port_no_t port_no;
        port_conf_t    *port_conf, *conf;
        u32            i, change;

        PORT_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (PORT_NO_IS_STACK(port_no))
                continue;

            i = (port_no - VTSS_PORT_NO_START);
            conf = &msg->req.conf_set.conf[i];
            port_conf = &port.config[VTSS_ISID_LOCAL][i];
            change = port_conf_change(port_conf, conf);
            *port_conf = *conf;

            T_D_PORT(port_no, "changed:%x, speed:%d, auto:%d",
                     change, conf->speed, conf->autoneg);

            // Fiber speed changed - update and reset PHY
            if (change & PORT_CONF_CHANGE_FIBER)
                phy_fiber_speed_update(i, conf->dual_media_fiber_speed);

#if defined(VTSS_SW_OPTION_I2C)
            if (!port_phy(port_no)) {
                /* Update MAC interface type when based on the SPF and new conf */
                port.mac_sfp_if[port_no] = port_sfp_vs_conf(port_no, port_mac_sfp_interface(port_no));
                T_DG_PORT(TRACE_GRP_SFP, port_no, "port.mac_sfp_if:%d", port.mac_sfp_if[port_no]);
            }
#endif

            if (change)
                port_setup(port_no, change);
        }

        /* Leave module INIT state */
        if (port.module_state == PORT_MODULE_STATE_INIT) {
            T_I("INIT -> CONF state");
            port.module_state = PORT_MODULE_STATE_CONF;
        }
        PORT_CRIT_EXIT();
        break;
    }
    case PORT_MSG_ID_STATUS_GET_REQ:
    {
        port_msg_rep_t *msg = port_msg_rep_alloc(PORT_MSG_ID_STATUS_GET_REP);
        vtss_port_no_t port_no;
        int            i;

        PORT_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            i = (port_no - VTSS_PORT_NO_START);
            msg->rep.status_get.status[i] = port.status[VTSS_ISID_LOCAL][i];
        }
        PORT_CRIT_EXIT();
        port_msg_tx(msg, isid, sizeof(msg->rep.status_get));
        break;
    }
    case PORT_MSG_ID_STATUS_GET_REP:
    {
        port_msg_rep_t     *msg = (port_msg_rep_t *)rx_msg;
        vtss_port_no_t     port_no;
        int                i;
        port_status_t      *port_status;
        vtss_port_status_t *status, old_status;

        PORT_CRIT_ENTER();
        if (VTSS_ISID_LEGAL(isid) && port.isid_added[isid]) {
            for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) {
                i = (port_no - VTSS_PORT_NO_START);
                port_status = &port.status[isid][i];
                status = &port_status->status;
                old_status = *status;
                *port_status = msg->rep.status_get.status[i];
                port.cap_valid[isid][i] = 1;
                /* Detect link down event */
                if ((!status->link || status->link_down) && old_status.link) {
                    T_D("link down event on isid: %d, port_no: %u", isid, port_no);
                    port.change_flags[isid][port_no] |= PORT_CHANGE_DOWN;
                    old_status.link = 0;
                }

                /* Detect link up or speed/duplex change event */
                if (status->link && (!old_status.link ||
                                     status->speed != old_status.speed ||
                                     status->fdx != old_status.fdx)) {
                    T_D("%s event on isid: %d, port_no: %u",
                        old_status.link ? "speed/duplex change" : "link up", isid, port_no);
                    port.change_flags[isid][port_no] |= PORT_CHANGE_UP;
                }
            }
            VTSS_MTIMER_START(&port.status_timer[isid], PORT_STATUS_TIMER);
            cyg_flag_setbits(&port.status_flags, 1<<isid);
        }
        PORT_CRIT_EXIT();
        break;
    }
    case PORT_MSG_ID_COUNTERS_GET_REQ:
    {
        port_msg_rep_t *msg = port_msg_rep_alloc(PORT_MSG_ID_COUNTERS_GET_REP);
        vtss_port_no_t port_no;
        int            i;

        PORT_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            i = (port_no - VTSS_PORT_NO_START);
            msg->rep.counters_get.counters[i] = port.counters[VTSS_ISID_LOCAL][i];
        }
        PORT_CRIT_EXIT();
        port_msg_tx(msg, isid, sizeof(msg->rep.counters_get));
        break;
    }
    case PORT_MSG_ID_COUNTERS_GET_REP:
    {
        port_msg_rep_t *msg = (port_msg_rep_t *)rx_msg;
        vtss_port_no_t port_no;
        int            i;

        if (VTSS_ISID_LEGAL(isid)) {
            PORT_CRIT_ENTER();
            for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) {
                i = (port_no - VTSS_PORT_NO_START);
                port.counters[isid][i] = msg->rep.counters_get.counters[i];
            }
            VTSS_MTIMER_START(&port.counters_timer[isid], PORT_COUNTERS_TIMER);
            cyg_flag_setbits(&port.counters_flags, 1<<isid);
            PORT_CRIT_EXIT();
        }
        break;
    }
    case PORT_MSG_ID_COUNTERS_CLEAR_REQ:
    {
        port_msg_req_t *msg = (port_msg_req_t *)rx_msg;
        
        port_counters_clear(msg->req.counters_clr.port_no);
        break;
    }
    case PORT_MSG_ID_VERIPHY_GET_REQ:
    {
        port_msg_req_t      *msg = (port_msg_req_t *)rx_msg;
        vtss_port_no_t      port_no;
        port_veriphy_t      *veriphy;
        port_veriphy_mode_t mode;
        int                 i;

        PORT_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            i = (port_no - VTSS_PORT_NO_START);
            veriphy = &port.veriphy[VTSS_ISID_LOCAL][i];
            mode = msg->req.veriphy_get.mode[i];
            if (mode != PORT_VERIPHY_MODE_NONE && !veriphy->running) {
                T_D("starting veriphy on port_no %u", port_no);
                port_veriphy_start(port_no, mode);
                veriphy->mode = mode;
                veriphy->running = 1;
                veriphy->valid = 0;
            }
        }
        PORT_CRIT_EXIT();
        break;
    }
    case PORT_MSG_ID_VERIPHY_GET_REP:
    {
        port_msg_rep_t *msg = (port_msg_rep_t *)rx_msg;;
        vtss_port_no_t port_no;
        int            i;

        if (VTSS_ISID_LEGAL(isid)) {
            PORT_CRIT_ENTER();
            for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) {
                i = (port_no - VTSS_PORT_NO_START);
                port.veriphy[isid][i] = msg->rep.veriphy_get.result[i];
            }
            PORT_CRIT_EXIT();
        }
        break;
    }
    default:
        T_W("Unknown message ID: %d", msg_id);
        break;
    }

    T_R("exit");

    return TRUE;
}

/* Register for port messages */
static vtss_rc port_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = port_msg_rx;
    filter.modid = VTSS_MODULE_ID_PORT;
    return msg_rx_filter_register(&filter);
}

/* Set port configuration */
static vtss_rc port_stack_conf_set(vtss_isid_t isid)
{
    port_msg_req_t  *msg;
    vtss_port_no_t  port_no;
    int             i;
    port_conf_t     *conf;
    port_user_t     user;
    port_vol_conf_t *vol_conf;

    T_D("isid: %d", isid);

    if (msg_switch_exists(isid) && port.isid_added[isid]) {
        msg = port_msg_req_alloc(PORT_MSG_ID_CONF_SET_REQ);
        PORT_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) {
            i = (port_no - VTSS_PORT_NO_START);
            conf = &msg->req.conf_set.conf[i];
            *conf = port.config[isid][i];

            /* Take ISID volatile configuration into account */
            for (user = PORT_USER_STATIC; user < PORT_USER_CNT; user++) {
                vol_conf = &port.vol_conf[user][isid][port_no];
                if (vol_conf->disable)
                    conf->enable = 0;
                if (vol_conf->loop == VTSS_PORT_LOOP_PCS_HOST)
                    conf->adv_dis |= PORT_ADV_UP_MEP_LOOP;
                if (vol_conf->oper_up)
                    conf->oper_up = 1;
            }
        }
        PORT_CRIT_EXIT();
        port_msg_tx(msg, isid, sizeof(msg->req.conf_set));
    }
    return VTSS_OK;
}

/* Send request and wait for response */
static BOOL port_stack_req_timeout(vtss_isid_t isid, port_msg_id_t msg_id,
                                   vtss_mtimer_t *timer, cyg_flag_t *flags)
{
    port_msg_req_t   *msg;
    BOOL             timeout;
    cyg_flag_value_t flag;
    cyg_tick_count_t time;

    T_D("enter, isid: %d", isid);

    PORT_CRIT_ENTER();
    timeout = VTSS_MTIMER_TIMEOUT(timer);
    PORT_CRIT_EXIT();

    if (timeout) {
        T_I("info old, sending GET_REQ(isid=%d)", isid);
        msg = port_msg_req_alloc(msg_id);
        flag = (1<<isid);
        cyg_flag_maskbits(flags, ~flag);
        port_msg_tx(msg, isid, 0);
        time = cyg_current_time() + VTSS_OS_MSEC2TICK(PORT_REQ_TIMEOUT * 1000);
        return (cyg_flag_timed_wait(flags, flag, CYG_FLAG_WAITMODE_OR, time) & flag ? 0 : 1);
    }
    return 0;
}

/* Get port status */
static vtss_rc port_stack_status_get(vtss_isid_t isid, BOOL wait)
{
    port_msg_req_t *msg;

    T_D("enter, isid: %d, wait: %d", isid, wait);
    if (wait) {
        if (port_stack_req_timeout(isid, PORT_MSG_ID_STATUS_GET_REQ,
                                   &port.status_timer[isid], &port.status_flags)) {
            T_W("timeout, STATUS_GET_REQ(isid=%d)", isid);
            return PORT_ERROR_REQ_TIMEOUT;
        }
    } else {
        msg = port_msg_req_alloc(PORT_MSG_ID_STATUS_GET_REQ);
        port_msg_tx(msg, isid, 0);
    }
    T_D("exit, isid: %d", isid);

    return VTSS_OK;
}

/* Send port status reply to master */
static vtss_rc port_stack_status_reply(vtss_event_t link_down[VTSS_PORTS])
{
    port_msg_rep_t *msg;
    vtss_port_no_t port_no;
    int            i;

    T_D("enter");

    msg = port_msg_rep_alloc(PORT_MSG_ID_STATUS_GET_REP);
    PORT_CRIT_ENTER();
    for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[VTSS_ISID_LOCAL]; port_no++) {
        i = (port_no - VTSS_PORT_NO_START);
        msg->rep.status_get.status[i] = port.status[VTSS_ISID_LOCAL][i];
        msg->rep.status_get.status[i].status.link_down = link_down[i];
    }
    PORT_CRIT_EXIT();
    port_msg_tx(msg, 0, sizeof(msg->rep.status_get));

    T_D("exit");
    return VTSS_OK;
}

/* Get port counters */
static vtss_rc port_stack_counters_get(vtss_isid_t isid,
                                       vtss_port_no_t port_no, vtss_port_counters_t *counters)
{
    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (port_stack_req_timeout(isid, PORT_MSG_ID_COUNTERS_GET_REQ,
                               &port.counters_timer[isid], &port.counters_flags)) {
        T_W("timeout, COUNTERS_GET_REQ(isid=%d)", isid);
        return PORT_ERROR_REQ_TIMEOUT;
    }

    PORT_CRIT_ENTER();
    *counters = port.counters[isid][port_no - VTSS_PORT_NO_START];
    PORT_CRIT_EXIT();

    T_D("exit, isid: %d, port_no: %u", isid, port_no);
    return VTSS_OK;
}

/* Clear port counters */
static vtss_rc port_stack_counters_clear(vtss_isid_t isid, vtss_port_no_t port_no)
{
    port_msg_req_t *msg;

    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    /* Timeout current counters */
    PORT_CRIT_ENTER();
    VTSS_MTIMER_START(&port.counters_timer[isid], 0);
    PORT_CRIT_EXIT();

    msg = port_msg_req_alloc(PORT_MSG_ID_COUNTERS_CLEAR_REQ);
    msg->req.counters_clr.port_no = port_no;
    port_msg_tx(msg, isid, sizeof(msg->req.counters_clr));

    T_D("exit, isid: %d, port_no: %u", isid, port_no);

    return VTSS_OK;
}

/* Start VeriPHY on ports */
static vtss_rc port_stack_veriphy_get(vtss_isid_t isid,
                                      port_veriphy_mode_t mode[VTSS_PORT_ARRAY_SIZE])
{
    port_msg_req_t *msg;
    vtss_port_no_t port_no;

    T_D("enter, isid: %d", isid);
    msg = port_msg_req_alloc(PORT_MSG_ID_VERIPHY_GET_REQ);
    for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) 
        msg->req.veriphy_get.mode[port_no - VTSS_PORT_NO_START] = mode[port_no];
    
    port_msg_tx(msg, isid, sizeof(msg->req.veriphy_get));

    T_D("exit, isid: %d", isid);

    return VTSS_OK;
}

/* Send VeriPHY reply to master */
static vtss_rc port_stack_veriphy_reply(void)
{
    port_msg_rep_t *msg;
    vtss_port_no_t port_no;
    int            i;

    T_D("enter");

    msg = port_msg_rep_alloc(PORT_MSG_ID_VERIPHY_GET_REP);
    PORT_CRIT_ENTER();
    for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[VTSS_ISID_LOCAL]; port_no++) {
        i = (port_no - VTSS_PORT_NO_START);
        msg->rep.veriphy_get.result[i] = port.veriphy[VTSS_ISID_LOCAL][i];
    }
    PORT_CRIT_EXIT();
    port_msg_tx(msg, 0, sizeof(msg->rep.veriphy_get));

    T_D("exit");
    return VTSS_OK;
}


static void port_los_interrupt_function(vtss_interrupt_source_t        source_id,
                                        u32                            port_no)
{
    /* hook up to the next interrupt */
    /* to achieve backpressure this should be done in the thread just before taken action (reading the actual status) on the interrupt */
    if (vtss_interrupt_source_hook_set(port_los_interrupt_function, source_id, INTERRUPT_PRIORITY_NORMAL) != VTSS_OK)
        T_E("vtss_interrupt_source_hook_set failed");

    /* Fast link interrupt only applies if the source is INTERRUPT_SOURCE_FLNK */
    if (source_id == INTERRUPT_SOURCE_FLNK) {
        /* rising edge of fast link failure detected  */
        fast_link_int[port_no-VTSS_PORT_NO_START] = true;
    }

    /* wake up thread */
    cyg_flag_setbits(&interrupt_wait_flag, PHY_INTERRUPT);
}


static void port_ams_interrupt_function(vtss_interrupt_source_t        source_id,
                                        u32                            port_no)
{
    /* hook up to the next interrupt */
    /* to achieve backpressure this should be done in the thread just before taken action (reading the actual status) on the interrupt */
    if (vtss_interrupt_source_hook_set(port_ams_interrupt_function, INTERRUPT_SOURCE_AMS, INTERRUPT_PRIORITY_NORMAL) != VTSS_OK)
        T_E("vtss_interrupt_source_hook_set failed");

    /* wake up thread */
    cyg_flag_setbits(&interrupt_wait_flag, PHY_INTERRUPT);
}

/****************************************************************************/
/*  Port interface                                                          */
/****************************************************************************/

void port_phy_wait_until_ready(void)
{
    /* All PHYs have been reset when the port module is ready */
    PORT_CRIT_ENTER();
    PORT_CRIT_EXIT();
}

/* Get port capability */
vtss_rc port_cap_get(vtss_port_no_t port_no, port_cap_t *cap)
{
    /* Check port number */
    if (!VTSS_PORT_IS_PORT(port_no)) {
        return PORT_ERROR_PARM;
    }

    *cap = port_custom_table[port_no].cap;

    return VTSS_OK;
}

/* Get local port information */
vtss_rc port_info_get(vtss_port_no_t port_no, port_info_t *info)
{
    T_N("enter, port_no: %u", port_no);

    /* Check port number */
    if (!VTSS_PORT_IS_PORT(port_no)) {
        T_E("illegal port_no: %u", port_no);
        return PORT_ERROR_PARM;
    }

    PORT_CRIT_ENTER();
    port_status2info(VTSS_ISID_LOCAL, port_no, info);
    PORT_CRIT_EXIT();
    T_N("exit");
    return VTSS_OK;
}

/* Determine if port and ISID are valid */
static BOOL port_isid_port_no_invalid(vtss_isid_t isid, vtss_port_no_t port_no)
{
    /* Check ISID */
    if (isid >= VTSS_ISID_END){
        T_E("illegal isid: %u", isid);
        return 1;
    }
    
    /* Check port number */
    if (port_no >= port.port_count[isid]) {
        T_E("illegal port_no: %u, isid: %u", port_no, isid);
        return 1;
    }

    return 0;
}

// Function returning the board type for a given isid
// In : isid - The switch id for which to return the board type.
// Return - Board type.
vtss_board_type_t port_isid_info_board_type_get(vtss_isid_t isid) {
    return (vtss_board_type_t)port.board_type[isid];
}

vtss_rc port_isid_info_get(vtss_isid_t isid, port_isid_info_t *info)
{
    vtss_port_no_t port_no;

    T_N("enter, isid: %u", isid);

    if (port_isid_port_no_invalid(isid, VTSS_PORT_NO_START))
        return PORT_ERROR_PARM;

    info->port_count = port.port_count[isid];
    info->board_type = port.board_type[isid];
    info->stack_port_0 = port.stack_port_0[isid];
    info->stack_port_1 = port.stack_port_1[isid];

    info->cap = 0;
    for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) {
        info->cap |= port_isid_port_cap(isid, port_no);
    }
    return VTSS_RC_OK;
}

vtss_rc port_isid_port_info_get(vtss_isid_t isid, vtss_port_no_t port_no, 
                                port_isid_port_info_t *info)
{
    port_status_t *status;

    if (port_isid_port_no_invalid(isid, VTSS_PORT_NO_START))
        return PORT_ERROR_PARM;

    status = &port.status[isid][port_no];
    info->cap = status->cap;
    info->chip_no = status->chip_no;
    info->chip_port = status->chip_port;
    return VTSS_OK;
}

u32 port_isid_port_count(vtss_isid_t isid)
{
    return (isid < VTSS_ISID_END ? port.port_count[isid] : 0);
}

BOOL port_isid_port_no_is_stack(vtss_isid_t isid, vtss_port_no_t port_no)
{
    return (port_no < port_isid_port_count(isid) && port.isid_added[isid] &&
            (port_no == port.stack_port_0[isid] || port_no == port.stack_port_1[isid]) ? 1 : 0);
}


// Function for determine is a port is a front port.
// In : isid - The switch id for the switch to check.
//    : port_no - Port number to check.
// Return : TRUE if the port at the given switch is a front port else FALSE
BOOL port_isid_port_no_is_front_port(vtss_isid_t isid, vtss_port_no_t port_no)
{
    // Check if port is within the number of ports for the switch and that it isn't a stack port.
    return ((port_isid_port_count(isid) > port_no) && 
            port.isid_added[isid] && 
            !port_isid_port_no_is_stack(isid, port_no));
}

u32 port_count_max(void) 
{
    u32         port_count = 0, count;
    vtss_isid_t isid;

    if (msg_switch_is_master()) {
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            count = port.port_count[isid];
            if (msg_switch_exists(isid) && count > port_count)
                port_count = count;
        }
    } else {
        port_count = port.port_count[VTSS_ISID_LOCAL];
    }
    return port_count;
}

vtss_rc switch_iter_init(switch_iter_t *sit, vtss_isid_t isid, switch_iter_sort_order_t sort_order)
{
    T_NG(TRACE_GRP_ITER, "enter: isid: %u, sort_order: %d", isid, sort_order);

    if (sit == NULL) {
        T_E("Invalid sit");
        return VTSS_INVALID_PARAMETER;
    }

    memset(sit, 0, sizeof(*sit)); // Initialize this before checking for errors. This will enable getnext() to work as expected.

    if ((isid != VTSS_ISID_GLOBAL) && !VTSS_ISID_LEGAL(isid)) {
        T_E("Invalid isid:%d", isid);
        return VTSS_INVALID_PARAMETER;
    }

    if (sort_order > SWITCH_ITER_SORT_ORDER_END) {
        T_E("Invalid sort_order");
        return VTSS_INVALID_PARAMETER;
    }

    sit->m_order = sort_order;

    if (isid == VTSS_ISID_GLOBAL) {
        switch (sort_order) {
        case SWITCH_ITER_SORT_ORDER_USID:
        case SWITCH_ITER_SORT_ORDER_USID_CFG:
            {
                u32 exists_mask = sort_order == SWITCH_ITER_SORT_ORDER_USID_CFG ? msg_configurable_switches() : msg_existing_switches();
                vtss_usid_t s;
                for (s = VTSS_USID_START; s < VTSS_USID_END; s++) {
                    if (exists_mask & (1LU << topo_usid2isid(s))) {
                        sit->remaining++;
                        sit->m_switch_mask |= 1LU << s;
                        sit->m_exists_mask |= 1LU << s;
                        T_DG(TRACE_GRP_ITER, "add usid %u to m_xxxxx_mask", s);
                    }
                }
                break;
            }
        case SWITCH_ITER_SORT_ORDER_ISID:
        case SWITCH_ITER_SORT_ORDER_ISID_CFG:
        case SWITCH_ITER_SORT_ORDER_ISID_ALL:
            {
                u32 exists_mask = sort_order == SWITCH_ITER_SORT_ORDER_ISID_CFG ? msg_configurable_switches() : msg_existing_switches();
                vtss_isid_t s;
                for (s = VTSS_ISID_START; s < VTSS_ISID_END; s++) {
                    BOOL exists = (exists_mask & (1LU << s)) != 0;
                    if ((sort_order == SWITCH_ITER_SORT_ORDER_ISID_ALL) || exists) {
                        sit->remaining++;
                        sit->m_switch_mask |= 1LU << s;
                        T_DG(TRACE_GRP_ITER, "add isid %u to m_switch_mask", s);
                        if (exists) {
                            sit->m_exists_mask |= 1LU << s;
                            T_DG(TRACE_GRP_ITER, "add isid %u to m_exists_mask", s);
                        }
                    }
                }
                break;
            }
        } /* switch */
    } else {
        BOOL exists = FALSE;
        switch (sort_order) {
        case SWITCH_ITER_SORT_ORDER_USID:
        case SWITCH_ITER_SORT_ORDER_ISID:
            exists = msg_switch_exists(isid);
            break;
        case SWITCH_ITER_SORT_ORDER_USID_CFG:
        case SWITCH_ITER_SORT_ORDER_ISID_CFG:
            exists = msg_switch_configurable(isid);
            break;
        case SWITCH_ITER_SORT_ORDER_ISID_ALL:
            exists = TRUE;
        }
        if (exists) {
            sit->remaining = 1;
            sit->m_switch_mask = 1LU << isid;
            T_DG(TRACE_GRP_ITER, "add isid %u to m_switch_mask", isid);
        }
    }
    T_NG(TRACE_GRP_ITER, "exit");
    return VTSS_OK;
}

BOOL switch_iter_getnext(switch_iter_t *sit)
{
    if (sit == NULL) {
        T_E("Invalid sit");
        return FALSE;
    }

    T_NG(TRACE_GRP_ITER, "enter %d", sit->m_state);

    switch (sit->m_state) {
    case SWITCH_ITER_STATE_FIRST:
    case SWITCH_ITER_STATE_NEXT:
        if (sit->m_switch_mask) {
            // Handle the first call
            if (sit->m_state == SWITCH_ITER_STATE_FIRST) {
                sit->first = TRUE;
                sit->last = FALSE;
                sit->m_state = SWITCH_ITER_STATE_NEXT;
            } else {
                sit->first = FALSE;
            }

            // Skip non-existing switches
            while (!(sit->m_switch_mask & 1)) {
                sit->m_switch_mask >>= 1;
                sit->m_exists_mask >>= 1;
                sit->m_sid++;
            }

            // Update isid, usid and exists with info about the found switch
            if (sit->m_order == SWITCH_ITER_SORT_ORDER_USID || sit->m_order == SWITCH_ITER_SORT_ORDER_USID_CFG) {
                sit->isid = topo_usid2isid(sit->m_sid);
                sit->usid = sit->m_sid;
            } else {
                sit->isid = sit->m_sid;
                sit->usid = (sit->m_order == SWITCH_ITER_SORT_ORDER_ISID_ALL) ? 0 : topo_isid2usid(sit->m_sid);
            }
            sit->exists = (sit->m_exists_mask & 1);

            // Skip this switch
            sit->m_switch_mask >>= 1;
            sit->m_exists_mask >>= 1;
            sit->m_sid++;

            // Update the last flag
            if (!sit->m_switch_mask) {
                sit->last = TRUE;
                sit->m_state = SWITCH_ITER_STATE_LAST;
            }

            // Update the remaining counter
            if (sit->remaining) {
                sit->remaining--;
            } else {
                T_E("Internal error in remaining counter");
            }

            T_DG(TRACE_GRP_ITER, "isid %u, usid %u, first %u, last %u, exists %u, remaining %u", sit->isid, sit->usid, sit->first, sit->last, sit->exists, sit->remaining);
            return TRUE; // We have a switch
        } else {
            sit->m_state = SWITCH_ITER_STATE_DONE;
        }
        break;
    case SWITCH_ITER_STATE_LAST:
        sit->m_state = SWITCH_ITER_STATE_DONE;
        break;
    case SWITCH_ITER_STATE_DONE:
        // Fall through 
    default:
        T_E("Invalid state");
    }

    T_NG(TRACE_GRP_ITER, "exit FALSE");
    return FALSE;
}

#if VTSS_PORTS > 64
#error "Port iterator functions support at most 64 ports"
#endif

static vtss_rc port_iter_init_internal(port_iter_t *pit, switch_iter_t *sit, vtss_isid_t isid, port_iter_sort_order_t sort_order, port_iter_flags_t flags)
{
    u32            port_count_max;
    u32            port_count_real;
    vtss_port_no_t iport;

    T_NG(TRACE_GRP_ITER, "enter: isid: %u, sort_order: %d, flags: %02x", isid, sort_order, flags);

    if (pit == NULL) {
        T_E("Invalid pit");
        return VTSS_INVALID_PARAMETER;
    }

    memset(pit, 0, sizeof(*pit)); // Initialize this before checking for errors. This will enable getnext() to work as expected.

    if (sit) {
        if (!msg_switch_is_master()) {
            T_DG(TRACE_GRP_ITER, "Not master");
            return VTSS_UNSPECIFIED_ERROR;
        }

        if ((isid != VTSS_ISID_GLOBAL) && !VTSS_ISID_LEGAL(isid)) {
            T_E("Invalid isid");
            return VTSS_INVALID_PARAMETER;
        }
    } else {
        if ((isid != VTSS_ISID_LOCAL) && !msg_switch_is_master()) {
            T_DG(TRACE_GRP_ITER, "Not master");
            return VTSS_UNSPECIFIED_ERROR;
        }

        if ((isid != VTSS_ISID_LOCAL) && !VTSS_ISID_LEGAL(isid)) {
            T_E("Invalid isid (%u)", isid);
            return VTSS_INVALID_PARAMETER;
        }
    }

    if (sort_order > PORT_ITER_SORT_ORDER_IPORT_ALL) {
        T_E("Invalid sort_order");
        return VTSS_INVALID_PARAMETER;
    }

    pit->m_sit = sit;
    pit->m_isid = isid;
    pit->m_order = sort_order;
    pit->m_flags = flags;

    if (sit && (isid == VTSS_ISID_GLOBAL)) {
        pit->m_state = PORT_ITER_STATE_INIT;
        T_NG(TRACE_GRP_ITER, "exit - has sit and global");
        return VTSS_OK; // We will be called again with a 'legal' isid
    }

    port_count_max  = (sort_order == PORT_ITER_SORT_ORDER_IPORT_ALL) ? VTSS_PORTS : port_isid_port_count(isid);
    port_count_real = port_isid_port_count(isid);

    // There is currently no difference in sorting order for iport and uport. Use same algorithm.
    for (iport = VTSS_PORT_NO_START; iport < port_count_max; iport++) {

        // The following must be modified when we add support for other types than front and stack:
        port_iter_type_t port_type = port_isid_port_no_is_stack(isid, iport) ? PORT_ITER_TYPE_STACK : PORT_ITER_TYPE_FRONT;

        if ((flags & (1 << port_type)) == 0) {
            continue; // Port type not present in flags - skip it
        }
        if (port.status[isid][iport].status.link) { // Port is up
            if (flags & PORT_ITER_FLAGS_DOWN) {
                continue; // We only want ports that are down - skip it
            }
        } else { // Port is down
            if (flags & PORT_ITER_FLAGS_UP) {
                continue; // We only want ports that are up - skip it
            }
        }
        pit->m_port_mask |= 1LLU << iport;
        T_DG(TRACE_GRP_ITER, "add iport %u type %d to m_port_mask", iport, port_type);
        if (iport < port_count_real) {
            pit->m_exists_mask |= 1LLU << iport;
            T_DG(TRACE_GRP_ITER, "add iport %u type %d to m_exists_mask", iport, port_type);
        }
    }
    T_NG(TRACE_GRP_ITER, "exit");
    return VTSS_OK;
}

vtss_rc port_iter_init(port_iter_t *pit, switch_iter_t *sit, vtss_isid_t isid, port_iter_sort_order_t sort_order, port_iter_flags_t flags)
{
    // If sit != NULL then use isid = VTSS_ISID_GLOBAL no matter what isid was.
    return port_iter_init_internal(pit, sit, (sit) ? VTSS_ISID_GLOBAL : isid, sort_order, flags);
}

void port_iter_init_local(port_iter_t *pit)
{
    (void)port_iter_init(pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
}

void port_iter_init_local_all(port_iter_t *pit)
{
    (void)port_iter_init(pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_NORMAL);
}

BOOL port_iter_getnext(port_iter_t *pit)
{
    if (pit == NULL) {
        T_E("Invalid pit");
        return FALSE;
    }

    T_NG(TRACE_GRP_ITER, "enter %d", pit->m_state);

    while (TRUE) {
        T_DG(TRACE_GRP_ITER, "state %d", pit->m_state);
        switch (pit->m_state) {
        case PORT_ITER_STATE_INIT:
            if (switch_iter_getnext(pit->m_sit)) {
                // Reinitialize the port iterator with isid from current switch
                (void) port_iter_init_internal(pit, pit->m_sit, pit->m_sit->isid, pit->m_order, pit->m_flags);
                // No return here. Take another round
            } else {
                pit->m_state = PORT_ITER_STATE_DONE;
                return FALSE;
            }
            break;
        case PORT_ITER_STATE_FIRST:
        case PORT_ITER_STATE_NEXT:
            if (pit->m_port_mask) {

                // Handle the first call
                if (pit->m_state == PORT_ITER_STATE_FIRST) {
                    pit->first = TRUE;
                    pit->last = FALSE;
                    pit->m_state = PORT_ITER_STATE_NEXT;
                } else {
                    pit->first = FALSE;
                }

                // Skip non-existing ports
                while (!(pit->m_port_mask & 1)) {
                    pit->m_port_mask >>= 1;
                    pit->m_exists_mask >>= 1;
                    pit->m_port++;
                }

                // Update iport, uport and exists with info about the found port
                pit->iport  = pit->m_port;
                pit->uport  = iport2uport(pit->m_port);
                pit->exists = (pit->m_exists_mask & 1);
                pit->link   = port.status[pit->m_isid][pit->m_port].status.link;

                // The following must be modified when we add support for other types than front and stack:
                pit->type   = port_isid_port_no_is_stack(pit->m_isid, pit->m_port) ? PORT_ITER_TYPE_STACK : PORT_ITER_TYPE_FRONT;

                // Skip this port
                pit->m_port_mask >>= 1;
                pit->m_exists_mask >>= 1;
                pit->m_port++;

                // Update the last flag
                if (!pit->m_port_mask) {
                    pit->last = TRUE;
                    pit->m_state = PORT_ITER_STATE_LAST;
                }

                T_DG(TRACE_GRP_ITER, "iport %u, uport %u, first %u, last %u, exists %u", pit->iport, pit->uport, pit->first, pit->last, pit->exists);
                return TRUE; // We have a port
            } else {
                if (pit->m_sit) {
                    pit->m_state = PORT_ITER_STATE_INIT; // Try next switch
                    // No return here. Take another round
                } else {
                    pit->m_state = PORT_ITER_STATE_DONE;
                    return FALSE;
                }
            }
            break;
        case PORT_ITER_STATE_LAST:
            if (pit->m_sit) {
                pit->m_state = PORT_ITER_STATE_INIT; // Try next switch
                // No return here. Take another round
            } else {
                pit->m_state = PORT_ITER_STATE_DONE;
                return FALSE;
            }
            break;
        case PORT_ITER_STATE_DONE:
        default:
            T_E("Invalid state:%d", pit->m_state);
            return FALSE;
        }
    }
}

/* Convert chip port to logical port */
vtss_port_no_t port_physical2logical(vtss_chip_no_t chip_no, uint chip_port, vtss_glag_no_t *glag_no)
{
    vtss_port_no_t port_no;
    *glag_no = VTSS_GLAG_NO_NONE; // Undefined aggregation number

#if defined(VTSS_ARCH_LUTON28)
    if (chip_port == 30 || chip_port == 31) {
        vtss_rc rc;
        BOOL    member[VTSS_PORT_ARRAY_SIZE];

        *glag_no = (chip_port - 30 + VTSS_GLAG_NO_START);

        // Several front ports may be members of the same GLAG. We pick the first
        // of them since there is no way to exactly figure out which front port it
        // came from.
        rc = vtss_aggr_glag_members_get(NULL, *glag_no, member);
        if (rc == VTSS_OK) {
            for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[VTSS_ISID_LOCAL]; port_no++)
                if (member[port_no])
                    return port_no;
        }

        // No members found. Return whatever.
        T_W("No port members of GLAG #%u (Switch API GLAG numbered)", *glag_no);
        return VTSS_PORT_NO_START;
    }
#endif /* VTSS_ARCH_LUTON28 */

#if VTSS_OPT_INT_AGGR
    /* Chip port 24/25 and 26/27 map to the same logical port */
    if (chip_port == 25 || chip_port == 27)
        chip_port--;
#endif /* VTSS_OPT_INT_AGGR */

    for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[VTSS_ISID_LOCAL]; port_no++)
        if (port_custom_table[port_no].map.chip_no == chip_no && port_custom_table[port_no].map.chip_port == (i32)chip_port)
            return port_no;

    T_E("illegal chip_port: %d", chip_port);
    return 0;
}

static vtss_rc port_module_invalid(vtss_module_id_t module_id)
{
    if (module_id < VTSS_MODULE_ID_NONE)
        return 0;

    T_E("invalid module_id: %d", module_id);
    return 1;
}

/* Port change registration */
vtss_rc port_change_register(vtss_module_id_t module_id, port_change_callback_t callback)
{
    vtss_rc             rc = VTSS_OK;
    port_change_table_t *table;
    port_change_reg_t   *reg;

    if (port_module_invalid(module_id))
        return PORT_ERROR_PARM;
    
    PORT_CB_CRIT_ENTER();
    table = &port.change_table;
    if (table->count < PORT_CHANGE_REG_MAX) {
        reg = &port.change_table.reg[table->count];
        reg->callback = callback;
        reg->module_id = module_id;
        VTSS_PORT_BF_CLR(reg->port_done);
        table->count++;
    } else {
        T_E("port change table full");
        rc = PORT_ERROR_REG_TABLE_FULL;
    }
    PORT_CB_CRIT_EXIT();

    return rc;
}

vtss_rc port_change_reg_get(port_change_reg_t *entry, BOOL clear)
{
    vtss_rc           rc = PORT_ERROR_GEN;
    int               i;
    port_change_reg_t *reg;
    
    PORT_CB_CRIT_ENTER();
    for (i = 0; i < port.change_table.count; i++) {
        reg = &port.change_table.reg[i];
        if (clear) {
            /* Clear all entries */
            reg->max_ticks = 0;
        } else if (entry->module_id == VTSS_MODULE_ID_NONE) {
            /* Get first */
            *entry = *reg;
            rc = VTSS_RC_OK;
            break;
        } else if (entry->module_id == reg->module_id && entry->callback == reg->callback) {
            /* Get next */
            entry->module_id = VTSS_MODULE_ID_NONE;
        }
    }
    PORT_CB_CRIT_EXIT();
    
    return rc;
}

/* Port global change registration */
vtss_rc port_global_change_register(vtss_module_id_t module_id, 
                                    port_global_change_callback_t callback)
{
    vtss_rc                    rc = VTSS_OK;
    port_global_change_table_t *table;
    port_global_change_reg_t   *reg;

    if (port_module_invalid(module_id))
        return PORT_ERROR_PARM;

    PORT_CB_CRIT_ENTER();
    table = &port.global_change_table;
    if (table->count < PORT_CHANGE_REG_MAX) {
        reg = &port.global_change_table.reg[table->count];
        reg->callback = callback;
        reg->module_id = module_id;
        table->count++;
    } else {
        T_E("port change table full");
        rc = PORT_ERROR_REG_TABLE_FULL;
    }
    PORT_CB_CRIT_EXIT();

    return rc;
}

vtss_rc port_global_change_reg_get(port_global_change_reg_t *entry, BOOL clear)
{
    vtss_rc                  rc = PORT_ERROR_GEN;
    int                      i;
    port_global_change_reg_t *reg;
    
    PORT_CB_CRIT_ENTER();
    for (i = 0; i < port.global_change_table.count; i++) {
        reg = &port.global_change_table.reg[i];
        if (clear) {
            /* Clear all entries */
            reg->max_ticks = 0;
        } else if (entry->module_id == VTSS_MODULE_ID_NONE) {
            /* Get first */
            *entry = *reg;
            rc = VTSS_RC_OK;
            break;
        } else if (entry->module_id == reg->module_id && entry->callback == reg->callback) {
            /* Get next */
            entry->module_id = VTSS_MODULE_ID_NONE;
        }
    }
    PORT_CB_CRIT_EXIT();
    
    return rc;
}

/* Port shutdown registration */
vtss_rc port_shutdown_register(vtss_module_id_t module_id, port_shutdown_callback_t callback)
{
    vtss_rc               rc = VTSS_OK;
    port_shutdown_table_t *table;
    port_shutdown_reg_t   *reg;

    if (port_module_invalid(module_id))
        return PORT_ERROR_PARM;

    PORT_CB_CRIT_ENTER();
    table = &port.shutdown_table;
    if (table->count < PORT_SHUTDOWN_REG_MAX) {
        reg = &port.shutdown_table.reg[table->count];
        reg->callback = callback;
        reg->module_id = module_id;
        table->count++;
    } else {
        T_E("shutdown table full");
        rc = PORT_ERROR_REG_TABLE_FULL;
    }
    PORT_CB_CRIT_EXIT();

    return rc;
}

vtss_rc port_shutdown_reg_get(port_shutdown_reg_t *entry, BOOL clear)
{
    vtss_rc             rc = PORT_ERROR_GEN;
    int                 i;
    port_shutdown_reg_t *reg;
    
    PORT_CB_CRIT_ENTER();
    for (i = 0; i < port.shutdown_table.count; i++) {
        reg = &port.shutdown_table.reg[i];
        if (clear) {
            /* Clear all entries */
            reg->max_ticks = 0;
        } else if (entry->module_id == VTSS_MODULE_ID_NONE) {
            /* Get first */
            *entry = *reg;
            rc = VTSS_RC_OK;
            break;
        } else if (entry->module_id == reg->module_id && entry->callback == reg->callback) {
            /* Get next */
            entry->module_id = VTSS_MODULE_ID_NONE;
        }
    }
    PORT_CB_CRIT_EXIT();
    
    return rc;
}

/****************************************************************************/
/*  Volatile port configuration interface                                   */
/****************************************************************************/

static const char *port_vol_user_txt(port_user_t user)
{
    return (user == PORT_USER_STATIC ? "Static" :
#ifdef VTSS_SW_OPTION_ACL
            user == PORT_USER_ACL ? "ACL" :
#endif /* VTSS_SW_OPTION_ACL */
#ifdef VTSS_SW_OPTION_THERMAL_PROTECT
            user == PORT_USER_THERMAL_PROTECT ? "Thermal" :
#endif /* VTSS_SW_OPTION_THERMAL_PROTECT */
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
            user == PORT_USER_LOOP_PROTECT ? "Loop" :
#endif /* VTSS_SW_OPTION_LOOP_PROTECT */
#ifdef VTSS_SW_OPTION_MEP
            user == PORT_USER_MEP ? "MEP" :
#endif /* VTSS_SW_OPTION_LOOP_PROTECT */
#ifdef VTSS_SW_OPTION_EFF
            user == PORT_USER_EFF ? "EFF" :
#endif /* VTSS_SW_OPTION_LOOP_PROTECT */
            "Unknown");
}

static vtss_rc port_vol_invalid(port_user_t user, vtss_isid_t isid, 
                                vtss_port_no_t port_no, BOOL config)
{
    /* Check user */
    if (user > PORT_USER_CNT || (config && user == PORT_USER_CNT)) {
        T_E("illegal user: %d", user);
        return 1;
    }
    
    /* Check ISID and port */
    if (isid == VTSS_ISID_LOCAL || port_isid_port_no_invalid(isid, port_no)) {
        T_E("illegal isid: %u or port_no: %u", isid, port_no);
        return 1;
    }
        
    /* Check that we are master and ISID exists */
    if (!msg_switch_is_master()) {
        T_W("not master");
        return 1;
    }
    if (!msg_switch_exists(isid)) {
        T_W("isid %d not active", isid);
        return 1;
    }

    return 0;
}

/* Get volatile port configuration */
vtss_rc port_vol_conf_get(port_user_t user, vtss_isid_t isid, 
                          vtss_port_no_t port_no, port_vol_conf_t *conf)
{
    if (port_vol_invalid(user, isid, port_no, 1))
        return PORT_ERROR_PARM;

    PORT_CRIT_ENTER();
    *conf = port.vol_conf[user][isid][port_no];
    PORT_CRIT_EXIT();

    return VTSS_RC_OK;
}

/* Set volatile port configuration */
vtss_rc port_vol_conf_set(port_user_t user, vtss_isid_t isid, 
                          vtss_port_no_t port_no, const port_vol_conf_t *conf)
{
    port_vol_conf_t *vol_conf;
    BOOL            changed = 0;

    if (port_vol_invalid(user, isid, port_no, 1))
        return PORT_ERROR_PARM;

    T_I("isid: %u, port_no: %u, user: %s, disable: %u, loop: %s, oper_up: %u",
        isid, port_no, port_vol_user_txt(user),
        conf->disable, conf->loop == VTSS_PORT_LOOP_DISABLE ? "disabled" : "enabled", conf->oper_up);

    PORT_CRIT_ENTER();
    vol_conf = &port.vol_conf[user][isid][port_no];
    if (port_vol_conf_changed(vol_conf, conf)) {
        /* Configuration changed */
        changed = 1;
        *vol_conf = *conf;
    }
    PORT_CRIT_EXIT();

    return (changed ? port_stack_conf_set(isid) : VTSS_OK);
}

vtss_rc vtss_port_vol_status_get(port_user_t user, vtss_isid_t isid,
                                 vtss_port_no_t port_no, port_vol_status_t *status)
{
    port_user_t      usr;
    port_conf_t      *conf;
    BOOL             disable, oper_up;
    vtss_port_loop_t loop;
    port_vol_conf_t  *vol_conf;

    if (port_vol_invalid(user, isid, port_no, 0))
        return PORT_ERROR_PARM;
    
    memset(&status->conf, 0, sizeof(status->conf));
    status->user = PORT_USER_STATIC;
    strcpy(status->name, "Static");

    PORT_CRIT_ENTER();
    for (usr = PORT_USER_STATIC; usr < PORT_USER_CNT; usr++) {
        if (usr != user && user != PORT_USER_CNT)
            continue;
        
        vol_conf = &port.vol_conf[usr][isid][port_no];
        if (usr == PORT_USER_STATIC) {
            conf = &port.config[isid][port_no];
            disable = (conf->enable ? 0 : 1);
            loop = ((conf->adv_dis & PORT_ADV_UP_MEP_LOOP) ? VTSS_PORT_LOOP_PCS_HOST : 
                    VTSS_PORT_LOOP_DISABLE);
            oper_up = conf->oper_up;
        } else {
            disable = vol_conf->disable;
            loop = vol_conf->loop;
            oper_up = vol_conf->oper_up;
        }
            
        vol_conf = &status->conf;

        /* If user matches or port has been disabled, use admin status */
        if (usr == user || (vol_conf->disable == 0 && disable)) {
            vol_conf->disable = disable;
            status->user = usr;
            strcpy(status->name, port_vol_user_txt(usr));
        }
        
        /* If user matches or loop is enabled, use loop */
        if (usr == user || (vol_conf->loop == VTSS_PORT_LOOP_DISABLE && vol_conf->loop != loop)) {
            vol_conf->loop = loop;
        }

        /* If user matches or operational mode is forced, use operational mode */
        if (usr == user || oper_up) {
            vol_conf->oper_up = oper_up;
        }
    }
    PORT_CRIT_EXIT();

    return VTSS_RC_OK;
}

/****************************************************************************/
/*  Management functions                                                                                                           */
/****************************************************************************/

/* Port mode text string */
char *vtss_port_mgmt_mode_txt(vtss_port_no_t port_no, vtss_port_speed_t speed, BOOL fdx, BOOL fiber)
{
    switch (speed) {
    case VTSS_SPEED_10M:
        if (fiber) {
            if (port.status[VTSS_ISID_LOCAL][port_no].sfp.type == PORT_SFP_1000BASE_T) {
                if (fdx) {
                    return ("10fdx (Cu SFP)"); 
                } 
                return ("10hdx (Cu SFP)"); 
            } 
        }
        return (fdx ? "10fdx" : "10hdx");
    case VTSS_SPEED_100M:
        if (fiber) {
            if (port.status[VTSS_ISID_LOCAL][port_no].sfp.type == PORT_SFP_1000BASE_T) {
                if (fdx) {
                    return ("100fdx (Cu SFP)"); 
                }
                return ("100hdx (Cu SFP)"); 
            }
            return ("100fdx Fiber"); 
        } else {
            return (fdx ? "100fdx" : "100hdx");
        }
    case VTSS_SPEED_1G:
        if (fiber) {
            if (port.status[VTSS_ISID_LOCAL][port_no].sfp.type == PORT_SFP_1000BASE_T) {
                return ("1Gfdx (Cu SFP)"); 
            }
            return ("1Gfdx Fiber"); 
        } else {
            return (fdx ? "1Gfdx" : "1Ghdx");
        }
    case VTSS_SPEED_2500M:
        return (fdx ? "2.5Gfdx" : "2.5Ghdx");
    case VTSS_SPEED_5G:
        return (fdx ? "5Gfdx" : "5Ghdx");
    case VTSS_SPEED_10G:
        return (fdx ? "10Gfdx" : "10Ghdx");
    default:
        T_I("Speed:%d", speed);
        return "?";
    }
}

/* Fiber Port mode text string */
char *port_fiber_mgmt_mode_txt(vtss_fiber_port_speed_t speed, BOOL auto_neg)
{
    
    switch (speed) {
    case VTSS_SPEED_FIBER_1000X:
        if (auto_neg) {
            return ("1000x_ams");
        } else {
            return ("1000x");
        }
    case VTSS_SPEED_FIBER_100FX:
        if (auto_neg) {
            return ("100fx_ams");
        } else {
            return ("100fx");
        }
    case VTSS_SPEED_FIBER_AUTO:
        return ("sfp_auto_ams");

    default:
        return ("Not supported");
    }
}


/* Get port configuration */
vtss_rc vtss_port_mgmt_conf_get(vtss_isid_t isid, vtss_port_no_t port_no, port_conf_t *conf)
{
    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (port_isid_port_no_invalid(isid, port_no))
        return PORT_ERROR_PARM;

    PORT_CRIT_ENTER();
    *conf = port.config[isid][port_no - VTSS_PORT_NO_START];
    PORT_CRIT_EXIT();
    T_D("exit, isid: %d, port_no: %u", isid, port_no);
    return VTSS_OK;
}

vtss_rc do_phy_reset(vtss_port_no_t port_no) {

    vtss_rc rc = VTSS_OK;
    vtss_phy_reset_conf_t phy_reset;
    
/* Reset PHY */
    phy_reset.mac_if = port_mac_interface(port_no);
    if (port_custom_table[port_no].cap & PORT_CAP_COPPER)
        phy_reset.media_if = VTSS_PHY_MEDIA_IF_CU;
    else if (port_custom_table[port_no].cap & PORT_CAP_FIBER) {
        T_D_PORT(port_no, "phy_reset.media_if = VTSS_PHY_MEDIA_IF_FI_1000BX");
        phy_reset.media_if = VTSS_PHY_MEDIA_IF_FI_1000BX;
    } else if (port_custom_table[port_no].cap & PORT_CAP_DUAL_COPPER) {
        phy_reset.media_if = VTSS_PHY_MEDIA_IF_AMS_CU_1000BX;
    } else if (port_custom_table[port_no].cap & PORT_CAP_DUAL_FIBER) {
        phy_reset.media_if = VTSS_PHY_MEDIA_IF_AMS_FI_1000BX;
    }   else if (port_custom_table[port_no].cap & PORT_CAP_DUAL_FIBER_100FX) {
        T_D_PORT(port_no, "phy_reset.media_if = VTSS_PHY_MEDIA_IF_AMS_FI_100FX");
        phy_reset.media_if = VTSS_PHY_MEDIA_IF_AMS_FI_100FX;
    } else if (port_custom_table[port_no].cap & PORT_CAP_DUAL_COPPER_100FX) {
        phy_reset.media_if = VTSS_PHY_MEDIA_IF_AMS_CU_100FX;
    } else {
        T_E("unknown media interface on port_no %u", port_no);
    }

    phy_reset.i_cpu_en = false;
    
    rc = vtss_phy_reset(PHY_INST, port_no, &phy_reset);
    if (rc != VTSS_OK) {
        T_E("vtss_phy_reset failed, port_no: %u, rc:%d", port_no, rc);
    }
    return rc;
}

/* Set port configuration */
vtss_rc vtss_port_mgmt_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, port_conf_t *conf)
{
    vtss_rc           rc = VTSS_OK;
    u32               i, change;
    port_conf_t       *port_conf;
    port_cap_t        cap;
    vtss_port_speed_t speed = conf->speed;
    vtss_fiber_port_speed_t dual_media_fiber_speed = conf->dual_media_fiber_speed;
    BOOL              mode_ok, fdx = conf->fdx;

    T_D("enter, isid: %d, port_no: %u, %s, %s mode, %d",
        isid,
        port_no,
        conf->enable ? (conf->autoneg ? "auto" :
                        vtss_port_mgmt_mode_txt(speed, fdx, FALSE, port_no)) : "disabled",
        conf->flow_control ? "fc" : "drop",
        conf->max_length);

    if (port_isid_port_no_invalid(isid, port_no))
        return PORT_ERROR_PARM;

    /* Check port capabilities */
    cap = port_isid_port_cap(isid, port_no);

    // Checking Speed for fiber
    T_D_PORT(port_no, "fiber_speed:%d, cap:0x%X, PORT_CAP_FIBER_1000X:0x%X, cap & PORT_CAP_FIBER_100F:0x%X", dual_media_fiber_speed, cap, PORT_CAP_FIBER_1000X, cap & PORT_CAP_FIBER_100FX);
    switch (dual_media_fiber_speed) {

    case VTSS_SPEED_FIBER_100FX:
        mode_ok = ((cap & (PORT_CAP_DUAL_FIBER_100FX | PORT_CAP_DUAL_COPPER_100FX)) != 0);
        T_D_PORT(port_no, "mode_ok:%d, VTSS_SPEED_FIBER_100FX", mode_ok);
        break;

    case VTSS_SPEED_FIBER_1000X:
        mode_ok = ((cap & (PORT_CAP_DUAL_COPPER | PORT_CAP_DUAL_FIBER)) != 0);
        T_D_PORT(port_no, "mode_ok:%d, VTSS_SPEED_FIBER_1000X", mode_ok);
        break;

    case VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED:
        mode_ok = TRUE;
        T_D_PORT(port_no, "mode_ok:%d, VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED", mode_ok);
        break;

    case VTSS_SPEED_FIBER_AUTO:
        mode_ok = ((cap & PORT_CAP_DUAL_SFP_DETECT) != 0);
        T_D_PORT(port_no, "mode_ok:%d, VTSS_SPEED_FIBER_AUTO", mode_ok);
        break;
        
    default:
        mode_ok = FALSE;
        T_D_PORT(port_no, "mode_ok:%d", mode_ok);
        break;
    }

    if ((cap & PORT_CAP_10G_FDX) && (cap & PORT_CAP_1G_FDX)) {
        if (conf->autoneg && (conf->speed == VTSS_SPEED_10G)) {
            conf->speed = VTSS_SPEED_1G; /* autoneg works only with 1G mode */
        }
    }

    if (!mode_ok) {
        T_D_PORT(port_no, "Fiber mode %d not supported", dual_media_fiber_speed);
        return PORT_ERROR_PARM;
    }

    /* Ignore Aneg capability check for SFP ports (only 100fx actually) */
    if (!((cap & PORT_CAP_SFP_DETECT) && !(cap & PORT_CAP_AUTONEG))) {
        if (conf->autoneg && !(cap & PORT_CAP_AUTONEG)) {
            T_D("aneg not supported on port_no %u", port_no);
            return PORT_ERROR_PARM;
        }
    }

    if (conf->flow_control && !(cap & PORT_CAP_FLOW_CTRL)) {
        T_D("flow control not supported on port_no %u", port_no);
        return PORT_ERROR_PARM;
    }

    // Checking Speed
    if (!conf->autoneg) {
        switch (speed) {
        case VTSS_SPEED_10G:
            mode_ok = (fdx && (cap & PORT_CAP_10G_FDX));
            break;
        case VTSS_SPEED_5G:
            mode_ok = (fdx && (cap & PORT_CAP_5G_FDX));
            break;
        case VTSS_SPEED_2500M:
            mode_ok = (fdx && (cap & PORT_CAP_2_5G_FDX));
            break;
        case VTSS_SPEED_1G:
            mode_ok = (fdx && (cap & PORT_CAP_1G_FDX));
            break;
        case VTSS_SPEED_100M:
            mode_ok = ((!fdx && (cap & PORT_CAP_100M_HDX)) || (fdx && (cap & PORT_CAP_100M_FDX)));
            break;
        case VTSS_SPEED_10M:
            mode_ok = ((!fdx && (cap & PORT_CAP_10M_HDX)) || (fdx && (cap & PORT_CAP_10M_FDX)));
            break;
        default:
            mode_ok = 0;
            break;
        }

        if (!mode_ok) {
	    T_D("mode %s not supported on port_no %u", vtss_port_mgmt_mode_txt(port_no, speed, fdx, FALSE), port_no);
            return PORT_ERROR_PARM;
        }
    }

    if (conf->exc_col_cont && !(cap & PORT_CAP_HDX)) {
        T_D("exc col not supported on port_no %u", port_no);
        return PORT_ERROR_PARM;
    }

#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    if (conf->power_mode != VTSS_PHY_POWER_NOMINAL) {
        if (!port_isid_phy(isid, port_no) && (msg_switch_exists(isid))) { // If the switch doesn't exist in the stack we assume that the PHY for the switch is green Ethernet capable.
            T_D("PHY power control not supported on port_no %u", port_no);
            return PORT_ERROR_PARM;
        }
    }
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */

    i = (port_no - VTSS_PORT_NO_START);
    if (isid == VTSS_ISID_LOCAL) {
        if (vtss_switch_mgd()) {
            T_W("SET not allowed, isid: %d", isid);
            return PORT_ERROR_PARM;
        }
    } else {
        if (!msg_switch_is_master()) {
            T_W("not master");
            return PORT_ERROR_MUST_BE_MASTER;
        }
        if (!msg_switch_configurable(isid)) {
            T_W("isid %d not active", isid);
            return PORT_ERROR_STACK_STATE;
        }
    }

    PORT_CRIT_ENTER();
    port_conf = &port.config[isid][i];
    change = port_conf_change(port_conf, conf);
    *port_conf = *conf;
    PORT_CRIT_EXIT();

    if (change) {
        if (isid == VTSS_ISID_LOCAL) {
            /* Unmanaged */
            PORT_CRIT_ENTER();
            port_setup(port_no, change);
            PORT_CRIT_EXIT();
        } else {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            /* Save changed configuration */
            port_conf_blk_t *blk;
            if ((blk = (port_conf_blk_t*)conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE, NULL)) == NULL) {
                T_W("failed to open port config table");
            } else {
                blk->conf[(isid - VTSS_ISID_START)*VTSS_PORTS + i] = *conf;
                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE);
            }
#endif
            /* Activate changed configuration */
            rc = port_stack_conf_set(isid);
        }
    }
    T_D("exit, isid: %d, port_no: %u", isid, port_no);
    return rc;
}

/* Get port status */
static vtss_rc port_status_get(vtss_isid_t isid, vtss_port_no_t port_no,
                               port_status_t *status, BOOL all)
{
    vtss_rc rc;

    T_N("enter, isid: %d, port_no: %u", isid, port_no);

    if (port_isid_port_no_invalid(isid, port_no))
        return PORT_ERROR_PARM;

    if (isid != VTSS_ISID_LOCAL) {
        if (!msg_switch_is_master()) {
            T_W("not master");
            return PORT_ERROR_MUST_BE_MASTER;
        } else if (!msg_switch_exists(isid)) {
            T_W("isid %d not active", isid);
            return PORT_ERROR_STACK_STATE;
        }
    }

    rc = (all && isid != VTSS_ISID_LOCAL ? port_stack_status_get(isid, 1) : VTSS_OK);

    PORT_CRIT_ENTER();
    *status = port.status[isid][port_no - VTSS_PORT_NO_START];

    T_DG_PORT(TRACE_GRP_SFP, port_no, "port.mac_sfp_if:%d", port.mac_sfp_if[port_no]);
    PORT_CRIT_EXIT();

    T_N("exit, isid: %d, port_no: %u", isid, port_no);
    return rc;
}

/* Get port status */
vtss_rc port_mgmt_status_get(vtss_isid_t isid, vtss_port_no_t port_no, port_status_t *status)
{
    return port_status_get(isid, port_no, status, 0);
}

/* Get all port status (including power status) */
vtss_rc vtss_port_mgmt_status_get_all(vtss_isid_t isid, vtss_port_no_t port_no, port_status_t *status)
{
    return port_status_get(isid, port_no, status, 1);
}

/* Get port counters */
vtss_rc vtss_port_mgmt_counters_get(vtss_isid_t isid,
                                    vtss_port_no_t port_no, vtss_port_counters_t *counters)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (port_isid_port_no_invalid(isid, port_no))
        return PORT_ERROR_PARM;

    if (isid == VTSS_ISID_LOCAL) {
        PORT_CRIT_ENTER();
        *counters = port.counters[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START];
        PORT_CRIT_EXIT();
    } else {
        if (!msg_switch_is_master()) {
            T_W("not master");
            return PORT_ERROR_MUST_BE_MASTER;
        } else if (!msg_switch_exists(isid)) {
            T_W("isid %d not active", isid);
            return PORT_ERROR_STACK_STATE;
        }
        rc = port_stack_counters_get(isid, port_no, counters);
    }

    T_D("exit, isid: %d, port_no: %u", isid, port_no);

    return rc;
}

/* Clear port counters */
vtss_rc vtss_port_mgmt_counters_clear(vtss_isid_t isid, vtss_port_no_t port_no)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (port_isid_port_no_invalid(isid, port_no))
        return PORT_ERROR_PARM;

    if (isid == VTSS_ISID_LOCAL) {
        rc = port_counters_clear(port_no);
    } else {
        if (!msg_switch_is_master()) {
            T_W("not master");
            return PORT_ERROR_MUST_BE_MASTER;
        } else if (!msg_switch_exists(isid)) {
            T_W("isid %d not active", isid);
            return PORT_ERROR_STACK_STATE;
        }
        rc = port_stack_counters_clear(isid, port_no);
    }

    T_D("exit, isid: %d, port_no: %u", isid, port_no);

    return rc;
}

/* VeriPHY state text string */
const char *vtss_port_mgmt_veriphy_txt(vtss_phy_veriphy_status_t status)
{
    const char *txt;

    switch (status) {
    case VTSS_VERIPHY_STATUS_OK:
        txt = "OK";
        break;
    case VTSS_VERIPHY_STATUS_OPEN:
        txt = "Open";
        break;
    case VTSS_VERIPHY_STATUS_SHORT:
        txt = "Short";
        break;
    case VTSS_VERIPHY_STATUS_ABNORM:
        txt = "Abnormal";
        break;
    case VTSS_VERIPHY_STATUS_SHORT_A:
        txt = "Short A";
        break;
    case VTSS_VERIPHY_STATUS_SHORT_B:
        txt = "Short B";
        break;
    case VTSS_VERIPHY_STATUS_SHORT_C:
        txt = "Short C";
        break;
    case VTSS_VERIPHY_STATUS_SHORT_D:
        txt = "Short D";
        break;
    case VTSS_VERIPHY_STATUS_COUPL_A:
        txt = "Cross A";
        break;
    case VTSS_VERIPHY_STATUS_COUPL_B:
        txt = "Cross B";
        break;
    case VTSS_VERIPHY_STATUS_COUPL_C:
        txt = "Cross C";
        break;
    case VTSS_VERIPHY_STATUS_COUPL_D:
        txt = "Cross D";
        break;
    case VTSS_VERIPHY_STATUS_UNKNOWN:
        txt = "N/A";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}


/* Start VeriPHY */
vtss_rc port_mgmt_veriphy_start(vtss_isid_t isid,
                                port_veriphy_mode_t mode[VTSS_PORT_ARRAY_SIZE])
{
    vtss_port_no_t      port_no;
    port_veriphy_t      *veriphy;
    port_veriphy_mode_t port_mode[VTSS_PORT_ARRAY_SIZE];
    BOOL start = FALSE;

    T_D("enter, isid: %d", isid);

    if (!msg_switch_is_master()) {
        return PORT_ERROR_MUST_BE_MASTER;
    }

    if (port_isid_port_no_invalid(isid, VTSS_PORT_NO_START))
        return PORT_ERROR_PARM;

    PORT_CRIT_ENTER();
    for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) {
        PORT_HAS_CAP(isid, port_no);

        veriphy = &port.veriphy[isid][port_no - VTSS_PORT_NO_START];
        

        port_mode[port_no] = PORT_VERIPHY_MODE_NONE;
        if (mode[port_no] != PORT_VERIPHY_MODE_NONE && port_isid_phy(isid, port_no) && 
            !veriphy->running) {
            T_I("starting veriPHY on isid %d, port_no %u", isid, port_no);
            veriphy->running = 1;
            veriphy->valid = 0;
            if (isid == VTSS_ISID_LOCAL) {
                port_veriphy_start(port_no, mode[port_no]);
            } else {
                
                port_mode[port_no] = mode[port_no];
                start = TRUE;
            }
        }
    }
    PORT_CRIT_EXIT();

    if (start)
        port_stack_veriphy_get(isid, port_mode);

    T_D("exit");

    return VTSS_OK;
}


/* Get VeriPHY result */
vtss_rc vtss_port_mgmt_veriphy_get(vtss_isid_t isid, vtss_port_no_t port_no,
                                   vtss_phy_veriphy_result_t *result, uint timeout)
{
    vtss_rc        rc = PORT_ERROR_INCOMPLETE;
    uint           timer;
    port_veriphy_t *veriphy;


    T_N_PORT(port_no, "enter, isid: %d", isid);

    if (port_isid_port_no_invalid(isid, port_no))
        return PORT_ERROR_PARM;

    for (timer = 0; ; timer++) {
        veriphy = &port.veriphy[isid][port_no - VTSS_PORT_NO_START];
        PORT_CRIT_ENTER();
        if (!veriphy->running) {
            *result = veriphy->result;
            T_D_PORT(port_no, "veriphy->valid =%d",veriphy->valid);
            rc = (veriphy->valid ? (vtss_rc)VTSS_OK : (vtss_rc)PORT_ERROR_GEN);
        }
        PORT_CRIT_EXIT();
        if (rc != PORT_ERROR_INCOMPLETE || timer >= timeout)
            break;
        VTSS_OS_MSLEEP(1000);
    }

    T_D("exit");

    return rc;
}

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
static vtss_rc port_oobfc_conf_set(vtss_port_no_t port_no, const port_oobfc_conf_t *const conf)
{
    vtss_host_conf_t host_conf;
    vtss_rc          rc;
    vtss_port_no_t   hmda = ce_mac_mgmt_hmdx_get(TRUE);


    rc = vtss_host_conf_get(NULL, port_no, &host_conf);
    if (rc != VTSS_RC_OK) {
        return rc;
    }
    host_conf.xaui.fc.extended_reach = conf->xtend_reach;
    host_conf.xaui.fc.channel_enable = conf->channel_status;
    rc = vtss_host_conf_set(NULL, port_no, &host_conf);
    if (rc != VTSS_RC_OK) {
        return rc;
    }
    port.oobfc_conf[port_no-hmda].xtend_reach =  conf->xtend_reach;
    port.oobfc_conf[port_no-hmda].channel_status = conf->channel_status;

    return rc;
}

vtss_rc port_mgmt_oobfc_conf_set(vtss_port_no_t port_no, const port_oobfc_conf_t *const conf)
{
    vtss_rc         rc;
    vtss_port_no_t  hmda = ce_mac_mgmt_hmdx_get(TRUE);

    T_D("enter, port_no: %u", port_no);
    if (vtss_port_is_host(NULL, port_no) == FALSE) {
        return VTSS_RC_ERROR;
    }
    PORT_CRIT_ENTER();
    rc = port_oobfc_conf_set(port_no, conf);
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK) {
        port_conf_blk_t *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE, NULL)) == NULL) {
            T_W("failed to open port config table");
        } else {
            blk->oobfc_conf[port_no - hmda] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE);
        }
    }
#else
    (void) hmda;   // Quiet lint
#endif
    PORT_CRIT_EXIT();

    T_D("exit");

    return rc;

}

vtss_rc port_mgmt_oobfc_conf_get(vtss_port_no_t port_no, port_oobfc_conf_t *const conf)
{
    vtss_port_no_t hmda = ce_mac_mgmt_hmdx_get(TRUE);

    T_D("enter, port_no: %u", port_no);
    if (vtss_port_is_host(NULL, port_no) == FALSE) {
        return VTSS_RC_ERROR;
    }
    PORT_CRIT_ENTER();
    *conf = port.oobfc_conf[port_no-hmda];
    PORT_CRIT_EXIT();

    T_D("exit");

    return VTSS_RC_OK;
}

static void ce_max_event_cb(const ce_max_cb_events_t events, void *data)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    port_conf_blk_t *blk = NULL;
    BOOL            mode_change = TRUE;

    if (events & CE_MAX_CB_EVENT_MODE_CHANGE) {
        mode_change = TRUE;
    } else if (events & CE_MAX_CB_EVENT_NULLIFY_MODE_CHANGE) {
        mode_change = FALSE;
    }
    PORT_CRIT_ENTER();
    if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE, NULL)) == NULL) {
        T_W("failed to open port config table");
    } else {
        blk->mode_change = mode_change;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE);
    }
    PORT_CRIT_EXIT();
#else
    T_D("No conf save due to silent upgrade");
#endif
}
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/
/* Set default volatile port configuration */
static BOOL port_vol_conf_default(vtss_isid_t isid, vtss_port_no_t port_no)
{
    port_user_t     user;
    BOOL            changed = FALSE;
    port_vol_conf_t *vol_conf, def_conf;

    memset(&def_conf, 0, sizeof(def_conf));

    /* Clear volatile port configuration */
    for (user = PORT_USER_STATIC; user < PORT_USER_CNT; user++) {
        vol_conf = &port.vol_conf[user][isid][port_no];
        if (port_vol_conf_changed(vol_conf, &def_conf)) {
            *vol_conf = def_conf;
            changed = TRUE;
        }
    }
    return changed;
}

/* Get defaults for local port */
vtss_rc vtss_port_conf_default(vtss_isid_t isid, vtss_port_no_t port_no, port_conf_t *conf)
{
    port_cap_t cap = port_isid_port_cap(isid, port_no);
    
    memset(conf, 0, sizeof(*conf));
    conf->enable = 1;
    if (cap & PORT_CAP_AUTONEG)
        conf->autoneg = 1;
    if (cap & PORT_CAP_10G_FDX) {
        conf->speed = VTSS_SPEED_10G;
        conf->fdx = 1;
        conf->autoneg = 0;
    } else if (cap & PORT_CAP_5G_FDX) {
        conf->speed = VTSS_SPEED_5G;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_1G_FDX) {
        /* 1G is preferred over 2.5G, as 1G is standardized and supports Autoneg */
        conf->speed = VTSS_SPEED_1G; 
        conf->fdx = 1;
    } else if (cap & PORT_CAP_2_5G_FDX) {
        conf->speed = VTSS_SPEED_2500M;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_100M_FDX) {
        conf->speed = VTSS_SPEED_100M;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_100M_HDX) {
        conf->speed = VTSS_SPEED_100M;
    } else if (cap & PORT_CAP_10M_FDX) {
        conf->speed = VTSS_SPEED_10M;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_10M_HDX) {
        conf->speed = VTSS_SPEED_10M;
    }

    if (cap & PORT_CAP_SPEED_DUAL_ANY_FIBER) {
        conf->dual_media_fiber_speed = VTSS_SPEED_FIBER_AUTO;
    } else {
      conf->dual_media_fiber_speed = VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED;
    }

    conf->max_length = VTSS_MAX_FRAME_LENGTH_MAX;
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    conf->power_mode = CONF_POWER_MODE_DEFAULT;
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
    return VTSS_RC_OK;
}

/* Check and update port configuration */
static void port_conf_check(vtss_isid_t isid, vtss_port_no_t port_no, port_conf_t *conf)
{
    port_cap_t cap;

    /* Check maximum frame length (does not require defaults) */
    if (conf->max_length > VTSS_MAX_FRAME_LENGTH_MAX)
        conf->max_length = VTSS_MAX_FRAME_LENGTH_MAX;
    
    /* Check speed/duplex */
    switch (conf->speed) {
    case VTSS_SPEED_10G:
        cap = PORT_CAP_10G_FDX;
        break;
    case VTSS_SPEED_5G:
        cap = PORT_CAP_5G_FDX;
        break;
    case VTSS_SPEED_2500M:
        cap = PORT_CAP_2_5G_FDX;
        break;
    case VTSS_SPEED_1G:        
        cap = PORT_CAP_1G_FDX;
        break;
    case VTSS_SPEED_100M:
        cap = (conf->fdx ? PORT_CAP_100M_FDX : PORT_CAP_100M_HDX);
        break;
    case VTSS_SPEED_10M:
        cap = (conf->fdx ? PORT_CAP_10M_FDX : PORT_CAP_10M_HDX);
        break;
    default:
        cap = 0;
        break;
    }

    /* Check auto negotiation */
    if (conf->autoneg)
        cap |= PORT_CAP_AUTONEG;
    
    /* Create defaults, if the port capabilities are insufficient */
    if ((port_isid_port_cap(isid, port_no) & cap) == 0) {
        vtss_port_conf_default(isid, port_no, conf);
    }
}

void port_10g_phy_setup(vtss_port_no_t port_no, BOOL *port_skip)
{
#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_mode_t oper_mode;
    u16                 model,tmp;
    
    /* Skip ports already setup and ports without 10G PHYs */
    if (*port_skip || 
        !(port_custom_table[port_no].cap & PORT_CAP_VTSS_10G_PHY) ||
        vtss_port_mmd_read(NULL, port_no, 30, 0, &model) != VTSS_RC_OK) {
        *port_skip = 0;
        return;
    }

    if (model == 0) {
        if (port_custom_table[port_no+1].cap & PORT_CAP_VTSS_10G_PHY) {
            if (vtss_port_mmd_read(NULL, port_no+1, 30, 0, &tmp) == VTSS_RC_OK) {
                if (tmp == 0x8489 || tmp == 0x8490) {
                    model = tmp;
                }
            }
        }
    }
    /* Initialize setup */
    memset(&oper_mode, 0, sizeof(oper_mode));
    /* Note that WAN mode must be also be supported on the board (currently only Phy-ESTAX boards) */
    oper_mode.oper_mode = VTSS_PHY_LAN_MODE;

    if (model == 0x8484 || model == 0x8487 || model == 0x8488) {
        oper_mode.xfi_pol_invert = 0;
    } else {
        oper_mode.xfi_pol_invert = 1;
    }

    /* If PHY with two ports is found, the last port (channel 0) is setup first */
    if (model == 0x8484 || model == 0x8487 || model == 0x8488 || model == 0x8489 || model == 0x8490) {
        *port_skip = 1;
        if (vtss_phy_10g_mode_set(PHY_INST, port_no + 1, &oper_mode) != VTSS_RC_OK) {
            T_E("vtss_phy_10g_mode_set failed, port_no %u", port_no + 1);
        }
    }
    
    /* Setup port */
    if (vtss_phy_10g_mode_set(PHY_INST, port_no, &oper_mode) != VTSS_RC_OK) {
        T_E("vtss_phy_10g_mode_set failed, port_no %u", port_no);
    }
#endif /* VTSS_CHIP_10G_PHY */
}

static void port_fiber_speed_check(vtss_fiber_port_speed_t *fiber_speed)
{
    // VTSS_SPEED_FIBER_DISABLED is obsolete, use VTSS_SPEED_FIBER_NOT_SUPPORTED (backward compatibility)
    if (*fiber_speed == VTSS_SPEED_FIBER_DISABLED) {
        *fiber_speed = VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED;
    }
}

/* Read/create and activate port configuration */
static void port_conf_read(vtss_isid_t isid, BOOL do_create, BOOL *changed)
{
    vtss_port_no_t  port_no;
    port_conf_blk_t *blk;
    port_conf_t     *port_conf, conf;
    int             i;
    ulong           size;

    T_I("enter, isid: %d, do_create: %u", isid, do_create);

    /* Open or create configuration block */
    if (misc_conf_read_use()) {
        if ((blk = (port_conf_blk_t*)conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE, &size)) == NULL ||
            size != sizeof(*blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            blk = (port_conf_blk_t*)conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE, sizeof(*blk));
            do_create = 1;
        } else if (blk->version != PORT_CONF_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        }
    }
    else {
        T_N("no silent upgrade; creating defaults");
        do_create = 1;
        blk       = NULL;
    }

    *changed = 0;
    PORT_CRIT_ENTER();
    for (port_no = VTSS_PORT_NO_START; port_no < port.port_count[isid]; port_no++) {
        PORT_HAS_CAP(isid, port_no);

        i = (port_no - VTSS_PORT_NO_START);
        if (do_create) {
            /* Use default values */
            vtss_port_conf_default(isid, port_no, &conf);
            if (port_vol_conf_default(isid, port_no)) {
                *changed = 1;
            }
            if (blk != NULL)
                blk->conf[(isid - VTSS_ISID_START)*VTSS_PORTS + i] = conf;
        } else {
            /* Use new configuration */
            port_conf = &blk->conf[(isid - VTSS_ISID_START)*VTSS_PORTS + i];
            port_conf_check(isid, port_no, port_conf);
            conf = *port_conf;
        }
        port_conf = &port.config[isid][i];
        
        /* Check/update fiber speed fields */
        port_fiber_speed_check(&port_conf->dual_media_fiber_speed);
        port_fiber_speed_check(&conf.dual_media_fiber_speed);

        if (port_conf_change(port_conf, &conf))
            *changed = 1;
        *port_conf = conf;
    }
    PORT_CRIT_EXIT();
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    {
        vtss_port_no_t hmda = ce_mac_mgmt_hmdx_get(TRUE), hmdb = ce_mac_mgmt_hmdx_get(FALSE);
        vtss_rc        rc;

        if (blk) {
            do {
                if (do_create) {
                    if (blk != NULL) {
                        blk->oobfc_conf[0].xtend_reach = FALSE;
                        blk->oobfc_conf[0].channel_status = FALSE;
                        blk->oobfc_conf[1].xtend_reach = TRUE;
                        blk->oobfc_conf[1].channel_status = TRUE;
                    }
                }
                if (blk->mode_change == TRUE) { /* There is a mode change, so set the configuration to defaults */
                    blk->oobfc_conf[0].xtend_reach = FALSE;
                    blk->oobfc_conf[0].channel_status = FALSE;
                    blk->oobfc_conf[1].xtend_reach = FALSE;
                    blk->oobfc_conf[1].channel_status = FALSE;
                }

                rc = port_oobfc_conf_set(hmda, &blk->oobfc_conf[0]);
                if (rc != VTSS_RC_OK) {
                    break;

                }
                if (hmdb == 0) {
                    break;

                }
                rc = port_oobfc_conf_set(hmdb, &blk->oobfc_conf[1]);
                if (rc != VTSS_RC_OK) {
                    break;
                }
                blk->mode_change = FALSE;
            } while(0);
        }
    }
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (blk == NULL) {
        T_W("failed to open port config table");
    } else {
        blk->version = PORT_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE);
    }
#endif

    T_D("exit");
}

/* Initialize ports */
static void port_init_ports(void)
{
    vtss_port_no_t        port_no;
    uint                  i;
    vtss_isid_t           isid;
    port_status_t         *status;
    port_conf_t           conf;
    BOOL                  reset_phys = 1;
    BOOL                  port_skip = 0;
    u32                   port_count;

    T_D("enter");












    /* Release ports from reset */
    if (reset_phys)
        port_custom_reset();

    /* Initialize all ports */
    port_count = port.port_count[VTSS_ISID_LOCAL];
#if defined(VTSS_SW_OPTION_MEP_LOOP_PORT)
    /* Include loop port */
    port_count++;
#endif /* VTSS_SW_OPTION_MEP_LOOP_PORT */
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        PORT_HAS_CAP(VTSS_ISID_LOCAL, port_no);

        if (port_phy(port_no)) {
            (void)do_phy_reset(port_no);
        } else 
            port_10g_phy_setup(port_no, &port_skip);

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
        {
            vtss_host_conf_t      conf;            
            if (vtss_port_is_host(NULL, port_no)) {
                /* Get Host defaults */
                if (vtss_host_conf_get(NULL, port_no, &conf) == VTSS_RC_ERROR) {
                    T_E("Could not get Host config(port %u)", port_no);
                }
                conf.xaui.fc.extended_reach = TRUE;
                conf.xaui.fc.channel_enable = TRUE;
                /* Apply the defaults */
                if (vtss_host_conf_set(NULL, port_no, &conf) != VTSS_RC_OK) {
                    T_E("Could not configure XAUI port %u", port_no);                
                }
            }
        }

#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC  */

        /* Set port status down */
        i = (port_no - VTSS_PORT_NO_START);
        status = &port.status[VTSS_ISID_LOCAL][i];
        status->status.link_down = 0;
        status->status.link = 0;
        status->fiber = 0;
        status->cap = port_custom_table[port_no].cap;
        status->chip_no = port_custom_table[i].map.chip_no;
        status->chip_port = port_custom_table[i].map.chip_port;
        memset(&port.aneg_status[i], 0, sizeof(vtss_port_status_t));

        /* Initialize VeriPHY status */
        port.veriphy[VTSS_ISID_LOCAL][i].running = 0;
        port.veriphy[VTSS_ISID_LOCAL][i].valid = 0;

       /* Update sfp interface with the default mac interface */
        port.mac_sfp_if[port_no] = port_mac_interface(port_no);
        T_DG_PORT(TRACE_GRP_SFP, port_no, "port.mac_sfp_if:%d", port.mac_sfp_if[port_no]);



        /* Setup port with default configuration */
        vtss_port_conf_default(VTSS_ISID_LOCAL, port_no, &conf);
#if defined(VTSS_SW_OPTION_MEP_LOOP_PORT)
        if (port_no == (port_count - 1)) {
            /* Enable loopback for loop port */
            conf.adv_dis |= PORT_ADV_UP_MEP_LOOP;
            conf.oper_up = 1;

            if (conf.max_length <= (9600 + VTSS_PACKET_HDR_SIZE_BYTES + 4))   /* On the loop port the max frame size must be at least 9600 + extra injection header + dummy tag */
                conf.max_length = 9600 + VTSS_PACKET_HDR_SIZE_BYTES + 4;

#if defined(VTSS_ARCH_SERVAL)
            {
                vtss_port_ifh_t ifh_conf;
                ifh_conf.ena_inj_header = TRUE;        /* VOE UP-MEP loop port must have injection header capability enabled */
                ifh_conf.ena_xtr_header = FALSE;
                (void)vtss_port_ifh_conf_set(NULL, port_no, &ifh_conf);
            }
#endif /* VTSS_ARCH_SERVAL */
        }
#endif /* VTSS_SW_OPTION_MEP_LOOP_PORT */
        (void) port_vol_conf_default(VTSS_ISID_LOCAL, port_no);
        port.config[VTSS_ISID_LOCAL][i] = conf;
        port_setup(port_no, PORT_CONF_CHANGE_ALL);
    } /* Port loop */

    // Do post reset 
    post_port_custom_reset();
    
#if defined(VTSS_ARCH_LUTON28) && defined(VTSS_FEATURE_NPI)
    {
        /* Set NPI port configuration */
        vtss_npi_conf_t npi;

        npi.enable = 1;
        vtss_npi_conf_set(NULL, &npi);
    }
#endif /* VTSS_ARCH_LUTON28 && VTSS_FEATURE_NPI */

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        /* Initialize config, status and counter timers */
        VTSS_MTIMER_START(&port.config_timer[isid], 1);
        VTSS_MTIMER_START(&port.status_timer[isid], 1);
        VTSS_MTIMER_START(&port.counters_timer[isid], 1);
    }

    cyg_flag_init(&port.config_flags);
    cyg_flag_init(&port.status_flags);
    cyg_flag_init(&port.counters_flags);

    /* Register for stack messages */
    port_stack_register();

    /* Initialize port LEDs */
    port_custom_led_init();

    /* Open up port API only after init */
    PORT_CRIT_EXIT();

    T_D("exit");
}

/* Module thread */
static void port_thread(cyg_addrword_t data)
{
    vtss_rc               rc;
    vtss_port_no_t        port_no, p, fast_port_no;
    vtss_mtimer_t         optimize_timer, loop_timer, aneg_timer = 0;
    vtss_port_counters_t  *counters;
    port_veriphy_t        *veriphy;
    int                   i, i2, veriphy_running, veriphy_done;
    BOOL                  changed;
    vtss_event_t          link_down[VTSS_PORTS];
    cyg_flag_value_t      flag_value;
    uint                  fast_link_failure[VTSS_PORTS]; /* fast link failure 'hold' timer */
    u32                   port_count = port.port_count[VTSS_ISID_LOCAL];
#if defined(VTSS_SW_OPTION_I2C)
    BOOL                  sfp_status_new[VTSS_PORTS], sfp_status_old[VTSS_PORTS], first_time=1;
    vtss_port_interface_t sfp_if = VTSS_PORT_INTERFACE_SERDES;
    vtss_port_no_t        decection_port = VTSS_PORT_NO_START;
#endif

    T_D("enter, data: %d", data);
    memset(sfp_status_new, 0, sizeof(BOOL)*VTSS_PORTS);
    /* The switch API has been set up by the vtss_api module */
    VTSS_ASSERT(vtss_board_type() != VTSS_BOARD_UNKNOWN);
    T_I("Detected %s board", vtss_board_name());
    port_custom_init();
    port_init_ports();
    memset(reset_phy2sgmii, 0, sizeof(reset_phy2sgmii));

#ifdef VTSS_SW_OPTION_VCLI
    /* Initialize CLI */
    port_cli_init();
#endif

    VTSS_MTIMER_START(&optimize_timer, 1000);
    
    /* This is used to indicate that a fast link failure interrupt has 
       requested a full run through all ports */ 
    fast_port_no = VTSS_PORT_NO_NONE;   
    
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++)
        fast_link_failure[port_no - VTSS_PORT_NO_START] = 0; /* No fast link failure detected */

    if(vtss_board_features() & VTSS_BOARD_FEATURE_LOS) {
#ifndef VTSS_SW_OPTION_PORT_LOS_WITHOUT_FLNK
        if (vtss_interrupt_source_hook_set(port_los_interrupt_function, INTERRUPT_SOURCE_FLNK, INTERRUPT_PRIORITY_NORMAL) != VTSS_OK)
            T_E("vtss_interrupt_source_hook_set failed");
#endif
        if (vtss_interrupt_source_hook_set(port_los_interrupt_function, INTERRUPT_SOURCE_LOS, INTERRUPT_PRIORITY_NORMAL) != VTSS_OK)
            T_E("vtss_interrupt_source_hook_set failed");
    }
    if(vtss_board_features() & VTSS_BOARD_FEATURE_AMS) {
        if (vtss_interrupt_source_hook_set(port_ams_interrupt_function, INTERRUPT_SOURCE_AMS, INTERRUPT_PRIORITY_NORMAL) != VTSS_OK)
            T_E("vtss_interrupt_source_hook_set failed");
    }

    for (;;) {
        changed = 0;

        /* Port module state machine used to detect if ready for warm start */
        PORT_CRIT_ENTER();
        switch (port.module_state) {
        case PORT_MODULE_STATE_CONF:
            T_I("CONF -> ANEG state");
            port.module_state = PORT_MODULE_STATE_ANEG;
            VTSS_MTIMER_START(&aneg_timer, 5000);
            break;
        case PORT_MODULE_STATE_ANEG:
            if (VTSS_MTIMER_TIMEOUT(&aneg_timer)) {
                T_I("ANEG -> POLL state");
                port.module_state = PORT_MODULE_STATE_POLL;
            }
            break;
        case PORT_MODULE_STATE_POLL:
            T_I("POLL -> READY state");
            port.module_state = PORT_MODULE_STATE_READY;
            break;
        case PORT_MODULE_STATE_READY:
        default:
            break;
        }
        PORT_CRIT_EXIT();

        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++)
            link_down[port_no - VTSS_PORT_NO_START] = 0;
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            PORT_HAS_CAP(VTSS_ISID_LOCAL, port_no);

            i = (port_no - VTSS_PORT_NO_START);
            if (fast_port_no == VTSS_PORT_NO_NONE || fast_link_int[i]) {
                /* NOT a fast link fail detection run or rising edge is detected on this port */
                if (fast_link_int[i]) {
                    /* Rising edge of fast link failure detected */
                    fast_link_int[i] = false;            /* Clear rising edge */
                    fast_link_failure[i] = (port_count - 2); /* Now initialize fast link failure 'hold' timer counter to 'allmost' 1 s. */
                }

                /* Poll port status */
                PORT_CRIT_ENTER();
                port_status_poll(port_no, &changed, &link_down[i], fast_link_failure[i]);

                if (fast_port_no == VTSS_PORT_NO_NONE) {
#if VTSS_SWITCH_STACKABLE
                    /* Poll stack ports frequently */
                    p = port_no_stack(FALSE);
                    if (p < port_count && port_no != p)
                        port_status_poll(p, &changed, &link_down[p - VTSS_PORT_NO_START],0);
                    p = port_no_stack(TRUE);
                    if (p < port_count && port_no != p)
                        port_status_poll(p, &changed, &link_down[p - VTSS_PORT_NO_START],0);
#endif /* VTSS_SWITCH_STACKABLE */

                    /* Read port counters */
                    counters = &port.counters[VTSS_ISID_LOCAL][i];
                    rc = vtss_port_counters_get(NULL, port_no, counters);

#if defined(VTSS_SW_OPTION_I2C) /* SFP auto detection through Ecos/I2C */
                    T_RG_PORT(TRACE_GRP_SFP, port_no, "port_custom_table[port_no].cap:0x%X", port_custom_table[port_no].cap);     
                    if (port_custom_table[port_no].cap & PORT_CAP_SFP_DETECT || port_custom_table[port_no].cap & PORT_CAP_DUAL_SFP_DETECT) {
                        if (first_time) {
                            decection_port = port_no;
                            first_time = 0;
                        }

                        if ( reset_phy2sgmii[port_no] && board_sfp_i2c_lock(1) == VTSS_RC_OK ) {
                            T_D("reset to SGMII");
                            port_sfp_sgmii_set(NULL, port_no);
                            reset_phy2sgmii[port_no] = FALSE;
                            board_sfp_i2c_lock(0);
                        }
                        /* Read SFP slot status once each port poll round */
                        if (port_no == decection_port) {  
                            memcpy(sfp_status_old, sfp_status_new, sizeof(BOOL)*VTSS_PORTS);
                            if (port_custom_sfp_mod_detect(sfp_status_new) != VTSS_RC_OK) {
                                T_DG_PORT(TRACE_GRP_SFP, port_no, "Could not perform a SFP module detect");
                            }
                            T_RG_PORT(TRACE_GRP_SFP, port_no, "New SFP :%d, old:%d, new%d", 
                                      sfp_status_old[port_no] != sfp_status_new[port_no], sfp_status_old[port_no], sfp_status_new[port_no]);
                            
                        }       
                        T_RG_PORT(TRACE_GRP_SFP, port_no, "New SFP :%d, old:%d, new%d", 
                                  sfp_status_old[port_no] != sfp_status_new[port_no], sfp_status_old[port_no], sfp_status_new[port_no]);

                        if (sfp_status_old[port_no] != sfp_status_new[port_no]) {
                            if (sfp_status_new[port_no]) {
                                /* New SFP module is inserted. Figure out the type */
                                if (sfp_detect_if(port_no, &sfp_if) != VTSS_RC_OK) {
                                    T_DG_PORT(TRACE_GRP_SFP, port_no, "Could not detect sfp if");     
                                }                       
                            } else {
                                /* Module is removed, default to 1000-Base-X serdes */
                                port.status[VTSS_ISID_LOCAL][port_no].cap = port_custom_table[port_no].cap;  
                                memset (&port.status[VTSS_ISID_LOCAL][port_no].sfp, 0, sizeof(port_sfp_t));
                                sfp_if = port_mac_interface(port_no);
                            }

                            // Change MAC interface if SFP module is connected directly to a MAC interface (Not through a PHY unless it is in pass-through mode)
                            /* Update the board file with the new if and run port_setup()*/
                            port.mac_sfp_if[port_no] = sfp_if;
                            if (port_setup(port_no, PORT_CONF_CHANGE_ALL) != VTSS_RC_OK) {
                                T_DG_PORT(TRACE_GRP_SFP, port_no, "Could not perform port_setup");
                            }
                                                    
                            T_DG_PORT(TRACE_GRP_SFP, port_no, "sfp_if:%d, port.mac_sfp_if[port_no]:%d", sfp_if, port.mac_sfp_if[port_no]);
                            if (is_port_phy(port_no)) { // Update ports with PHY (including ports in pass-through mode)
                                phy_fiber_speed_update(port_no, port.config[VTSS_ISID_LOCAL][port_no].dual_media_fiber_speed);
                            }
                        }
                    }                    
#endif /* VTSS_SW_OPTION_I2C */

                    /* Update port LED */
                    port_custom_led_update(port_no, &port.status[VTSS_ISID_LOCAL][i].status, counters, &port.config[VTSS_ISID_LOCAL][port_no]);
                    /* VeriPHY processing for all ports */
                    veriphy_running = 0;
                    veriphy_done = 0;

#ifdef VTSS_SW_OPTION_POE           
                    // Needed due to bugzilla#8911, see the other comments regarding this.
                    poe_status_t poe_port_status;
                    poe_mgmt_get_status(VTSS_ISID_LOCAL, &poe_port_status);
#endif
                    for (p = VTSS_PORT_NO_START; p < port_count; p++) {
                        veriphy = &port.veriphy[VTSS_ISID_LOCAL][p - VTSS_PORT_NO_START];
                        if (veriphy->running) {
                            veriphy_running++;
                            vtss_phy_veriphy_result_t temp_result;  /* Temp Result */
                            rc = vtss_phy_veriphy_get(PHY_INST, p, &temp_result);
                            if (rc != VTSS_RC_INCOMPLETE) {
                             
                                veriphy->result.link = temp_result.link;
                                for (i = 0; i  < 4; i++) {
                                    T_I_PORT(p, "status[%d]:0x%X, temp_status:0x%X, length:0x%X, temp_length:0x%X", 
                                             i, veriphy->result.status[i], temp_result.status[i], veriphy->result.length[i], temp_result.length[i]);


                                    // Bugzilla#8911, Sometimes cable length is measured too long (but never too short), so veriphy is done multiple times and the shortest length is stored.   

                                    // First time we simply use the result we got from the PHY
                                    if (veriphy->repeat_cnt < VERIPHY_REPEAT_CNT) {
                                        // We have seen that the results variates when no cable is plugged in so if that 
                                        // happens we say that the port is unconnected. (adding 3 because the resolution is 3 m, so that variation is OK).
                                        if ((veriphy->result.length[i] + 3) < temp_result.length[i]) {
                                            veriphy->variate_cnt[p]++;
                                        } else {
                                            veriphy->result.length[i] = temp_result.length[i];
                                            veriphy->result.status[i] = temp_result.status[i];
                                        }
                                    } else {
                                        veriphy->result.length[i] = temp_result.length[i];
                                        veriphy->result.status[i] = temp_result.status[i];
                                    }

                                    T_I_PORT(p, "status[%d]:0x%X, temp_status:0x%X, length:0x%X, temp_length:0x%X", 
                                             i, veriphy->result.status[i], temp_result.status[i], veriphy->result.length[i], temp_result.length[i]);


#ifdef VTSS_SW_OPTION_POE           
                                    // Bugzilla#8911, we have that PoE boards can "confuse" VeriPhy, to wrongly report ABNORMAL, SHORT and OPEN, so when we have a PD, we only report OK (else it is set as unknown)
                                    if (poe_port_status.port_status[p] != NO_PD_DETECTED && 
                                        poe_port_status.port_status[p] != POE_NOT_SUPPORTED && 
                                        poe_port_status.port_status[p] != POE_DISABLED) {
                                        T_I_PORT(p, "PoE Port status:%d", poe_port_status.port_status[p]);
                                        if (temp_result.status[i] != VTSS_VERIPHY_STATUS_OK) {
                                            T_I_PORT(p, "temp_result.status[%d]:%d", i, temp_result.status[i]);
                                            veriphy->result.status[i] = VTSS_VERIPHY_STATUS_UNKNOWN;
                                        }
                                    }
#endif                                                                         
                                    T_I_PORT(p, "status[%d]:0x%X, length:0x%X variate_cnt:%d", i, veriphy->result.status[i], veriphy->result.length[i], veriphy->variate_cnt[p]);
                                }
                                T_N_PORT(p, "veriPHY done, rc = %d", rc);
                                
                                // Work-around of bugzilla#8911 - If the result variates it is an indication of that the port in not connected
                                if (veriphy->variate_cnt[p] > 1) {
                                    for (i2 = 0; i2  < 4; i2++) {
                                        if (veriphy->result.status[i2] != VTSS_VERIPHY_STATUS_UNKNOWN) {
                                            veriphy->result.status[i2] = VTSS_VERIPHY_STATUS_OPEN;
                                            veriphy->result.length[i2] = 0;
                                        }
                                    }
                                    T_D_PORT(p, "Forcing status to open, vaiate_cnt:%d", veriphy->variate_cnt[p]);
                                }
                                
                                if (veriphy->repeat_cnt > 1) {
                                    // Bugzilla#8911- Repeat veriphy a number of times in order to get correct result.
                                    veriphy->repeat_cnt--;
                                    vtss_phy_veriphy_start(PHY_INST, p,
                                                           veriphy->mode == PORT_VERIPHY_MODE_BASIC ? 2 :
                                                           veriphy->mode == PORT_VERIPHY_MODE_NO_LENGTH ? 1 : 0);
                                } else {
                                    veriphy->repeat_cnt = 0;
                                    veriphy->running = 0;
                                    veriphy->valid = (rc == VTSS_OK ? 1 : 0);
                                    veriphy_done++;
                                }
                            }
                        }
                    }
                }

                PORT_CRIT_EXIT();
                if (fast_port_no == VTSS_PORT_NO_NONE) {
                    if (veriphy_running && veriphy_running == veriphy_done)
                        port_stack_veriphy_reply();
                }
            }

            if (fast_port_no == VTSS_PORT_NO_NONE) {
                /* Only do this decrement of fast link fail 'hold' timer counter 
                   if it is a 'normal' run (not initiated by interrupt) */
                for (p = VTSS_PORT_NO_START; p < port_count; p++)
                    if (fast_link_failure[p - VTSS_PORT_NO_START])   /* If fast link failure is active, the counter is decremented to run the 'hold' timer */
                        fast_link_failure[p - VTSS_PORT_NO_START]--;
            }

            if (fast_port_no == port_no || fast_port_no == VTSS_PORT_NO_NONE) {
                /* Either a fast link fail run is not activated or it has been done */
                fast_port_no = VTSS_PORT_NO_NONE; /* Fast link fail run has been done */
                VTSS_MTIMER_START(&loop_timer, 1000/port_count);
                flag_value = cyg_flag_timed_wait(&interrupt_wait_flag, PHY_INTERRUPT, 
                                                 CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, 
                                                 loop_timer);
                if (flag_value == PHY_INTERRUPT) {
                    fast_port_no = port_no;  /* When interrupt received, loop has to take a fast link failure run, to take action on rising edge  */
                }
            }

            if (VTSS_MTIMER_TIMEOUT(&optimize_timer)) {
                rc = vtss_poll_1sec(NULL);
                VTSS_MTIMER_START(&optimize_timer, 1000);
            }

            if (fast_port_no == VTSS_PORT_NO_NONE) {
                /* Global port change events */
                port_global_change_events();
            }
        } /* Port loop */

        if (changed) {
            port_stack_status_reply(link_down);
        }
    } /* for (;;) */
}

/****************************************************************************/
// port_pre_reset_callback()
// Called when system is reset.
/****************************************************************************/
static void port_pre_reset_callback(vtss_restart_t restart)
{
    port_custom_pre_reset();
}

static void port_isid_info_update(vtss_isid_t isid, vtss_init_data_t *data, BOOL add)
{
    init_switch_info_t *info = &data->switch_info[isid];

    if (info->configurable) {
        T_I("%s isid: %u, board_type: %u, port_cnt: %u, stack_0: %u, stack_1: %u",
            add ? "added" : "config",
            isid, info->board_type, info->port_cnt, info->stack_ports[0], info->stack_ports[1]);
        port.board_type[isid] = info->board_type;
        port.port_count[isid] = info->port_cnt;
        port.stack_port_0[isid] = info->stack_ports[0];
        port.stack_port_1[isid] = info->stack_ports[1];
        port.isid_added[isid] = 1;
    } else if (add) {
        T_E("isid: %u added, but not configurable", isid);
    }
}

/* Initialize module */
vtss_rc port_init(vtss_init_data_t *data)
{
    vtss_isid_t           isid = data->isid;
    BOOL                  changed, new_;
    vtss_board_info_t     info;
    vtss_port_no_t        port_no;
    port_vol_conf_t       *vol_conf;
    port_user_t           user;
    vtss_port_status_t    *status;
    port_veriphy_t        *veriphy;
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    ce_max_cb_event_context_t cb_context;
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_I("INIT");

        /* Initialize message buffer pools */
        port.request = msg_buf_pool_create(VTSS_MODULE_ID_PORT, "Request", VTSS_ISID_CNT + 1, sizeof(port_msg_req_t));
        port.reply   = msg_buf_pool_create(VTSS_MODULE_ID_PORT, "Reply",   1,                 sizeof(port_msg_rep_t));

        /* Initialize port change table */
        port.change_table.count = 0;
        port.global_change_table.count = 0;
        port.shutdown_table.count = 0;

        /* Create critical region variables */
        critd_init(&port.crit, "port.crit", VTSS_MODULE_ID_PORT, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        /* Create critical region protecting callbacks and their registrations  */
        critd_init(&port.cb_crit, "port.cb_crit", VTSS_MODULE_ID_PORT, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        /* Let go of the callback crit immediately, so that modules can start registering ASAP */
        PORT_CB_CRIT_EXIT();

        /* Create event flag for interrupt to signal and port_thread to wait for */
        cyg_flag_init(&interrupt_wait_flag);

        /* Create thread */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          port_thread,
                          0,
                          "Port Control",
                          port.thread_stack,
                          sizeof(port.thread_stack),
                          &port.thread_handle,
                          &port.thread_block);
        /* Resume thread */
        cyg_thread_resume(port.thread_handle);

        // Register system reset callback
        control_system_reset_register(port_pre_reset_callback);

        port.thread_suspended = 0;

        /* Initialize ISID information for all switches */
        vtss_board_info_get(&info);
        for (isid = VTSS_ISID_LOCAL; isid < VTSS_ISID_END; isid++) {
            /* By default, the local information is used for all switches */
            port.board_type[isid] = info.board_type;
            port.port_count[isid] = info.port_count;
            port.stack_port_0[isid] = port_no_stack(0);
            port.stack_port_1[isid] = port_no_stack(1);
        }
        port.isid_added[VTSS_ISID_LOCAL] = 1; /* Local always there */

#ifdef VTSS_SW_OPTION_ICFG
        // Initialize ICLI "show running" configuration
        VTSS_RC(port_icfg_init());

#if defined(VTSS_SW_OPTION_PORT_POWER_SAVINGS)
        VTSS_RC(port_power_savings_icfg_init());
#endif
#endif
        break;
    case INIT_CMD_START:
        break;
    case INIT_CMD_CONF_DEF:
        T_I("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset global configuration */
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
            if (!data->switch_info[isid].configurable) {
                /* Switch no longer configurable */
                port.isid_added[isid] = 0;
            }
            if (port.isid_added[isid]) {
                port_conf_read(isid, 1, &changed);
                if (changed)
                    port_stack_conf_set(isid);
            }
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_I("MASTER_UP");

        PORT_CRIT_ENTER();
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            /* Update switch information for configurable switches */
            port_isid_info_update(isid, data, 0);

            /* Initialize port states to link down */
            for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
                port.status[isid][port_no - VTSS_PORT_NO_START].status.link = 0;
        }
        PORT_CRIT_EXIT();

        /* Read configuration */
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (port.isid_added[isid]) {
                port_conf_read(isid, 0, &changed);
            }
        }
        T_I("MASTER_UP EXIT");
        break;
    case INIT_CMD_MASTER_DOWN:
        T_I("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_I("SWITCH_ADD, isid: %d", isid);

        PORT_CRIT_ENTER();

        /* Determine if new switch and store ISID information */
        new_ = (port.isid_added[isid] == 0);
        port_isid_info_update(isid, data, 1);

        /* Initialize VeriPHY state */
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            veriphy = &port.veriphy[isid][port_no - VTSS_PORT_NO_START];
            veriphy->running = 0;
            veriphy->valid = 0;
        }

        PORT_CRIT_EXIT();

        if (misc_conf_read_use() || new_) {
            /* Read/create configuration if in silent upgrade mode or new switch */
            port_conf_read(isid, 0, &changed);
        }
        port_stack_conf_set(isid);
        port_stack_status_get(isid, 0);
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
        /* Register with CE_MAC module for host mode change event */
        cb_context.module_id = VTSS_MODULE_ID_PORT;
        cb_context.cb        = ce_max_event_cb;
        if (ce_max_mgmt_mode_change_event_register(&cb_context) != VTSS_RC_OK) {
            T_E("Unable to register with CE-MAX");
        }
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
        break;
    case INIT_CMD_SWITCH_DEL:
        T_I("SWITCH_DEL, isid: %d", isid);

        PORT_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            /* Generate port change events */
            status = &port.status[isid][port_no - VTSS_PORT_NO_START].status;
            if (status->link) {
                status->link = 0;
                port.change_flags[isid][port_no] |= PORT_CHANGE_DOWN;
            }
            port.cap_valid[isid][port_no] = 0;

            /* Delete volatile port configuration */
            for (user = PORT_USER_STATIC; user < PORT_USER_CNT; user++) {
                vol_conf = &port.vol_conf[user][isid][port_no];
                memset(vol_conf, 0, sizeof(*vol_conf));
            }
        }
        PORT_CRIT_EXIT();
        break;
    case INIT_CMD_SUSPEND_RESUME:
        PORT_CRIT_ENTER();
        if (data->resume) {
            if (port.thread_suspended) {
                cyg_thread_resume(port.thread_handle);
                port.thread_suspended = 0;
            }
        } else {
            if (!port.thread_suspended) {
                cyg_thread_suspend(port.thread_handle);
                port.thread_suspended = 1;
            }
        }
        PORT_CRIT_EXIT();
        break;
    case INIT_CMD_WARMSTART_QUERY:
        /* Check if ready for warm start */
        if (port.module_state != PORT_MODULE_STATE_READY)
            data->warmstart = 0;
        break;
    default:
        break;
    }

    T_D("exit");
    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
