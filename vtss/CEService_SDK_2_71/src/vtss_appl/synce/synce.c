/*

 Vitesse Switch SyncE software.

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

#include "critd_api.h"
#include "port_api.h"
#include "conf_api.h"
#include "misc_api.h"
#include "synce.h"
#include "main.h"
#include "vtss_types.h"
#include "interrupt_api.h"
#include "port_custom_api.h"

#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif
#ifdef VTSS_SW_OPTION_VCLI
#include "synce_cli.h"
#endif
#ifdef VTSS_SW_OPTION_PHY
#include "phy_api.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "synce_icli_functions.h" // For synce_icfg_init
#endif
#ifdef VTSS_ARCH_JAGUAR_1
#include "pcb107_cpld.h"
#endif

#include "synce_trace.h"
/****************************************************************************/
/*  Global variables                                                                                                                      */
/****************************************************************************/

#define SSM_QL_INV      0x01      // for test only
#define SSM_QL_PRC      0x02
#define SSM_QL_SSUA     0x04
#define SSM_QL_SSUB     0x08
#define SSM_QL_EEC2     0x0A
#define SSM_QL_EEC1     0x0B
#define SSM_QL_NONE     0x0E      //not standard
#define SSM_QL_DNU      0x0F
#define SSM_QL_FAIL     0xFF

#define SSM_OK          0x00
#define SSM_LINK_DOWN   0x01
#define SSM_FAIL        0x02
#define SSM_INVALID     0x04
#define SSM_NOT_SEEN    0x08

#define WTR_OK          0x00
#define WTR_LINK        0x01
#define WTR_SSM_FAIL    0x02
#define WTR_SSM_INV     0x04
#define WTR_FOS         0x08
#define WTR_LOCS        0x10

#define FLAG_CLOCK_SOURCE_LOCS 0x01
#define FLAG_CLOCK_SOURCE_FOS  0x02
#define FLAG_CLOCK_SOURCE_LOSX 0x04
#define FLAG_CLOCK_SOURCE_LOL  0x08
#define FLAG_PORT_CALL_BACK    0x10
#define FLAG_FRAME_RX          0x20
#define FLAG_ONE_SEC           0x40
#define FLAG_SSM_EVENT         0x80

#define PREFER_MAX  2
#define PREFER_TIMEOUT 6
#define PREFER_RESET  PREFER_TIMEOUT+10

#define SWITCH_AUTO_SQUELCH FALSE              /* Disable auto squelching in the Switch to avoid detection of LOCS before Link down on Serdes ports.*/

uint synce_my_nominated_max;                     /* actual max number of nominated ports */
uint synce_my_priority_max;                      /* actual max number of priorities */

typedef struct
{
    BOOL    new_ssm;
    BOOL    new_link;
    BOOL    new_fiber;
    uint    ssm_frame;
    uint    ssm_rx;
    uint    ssm_tx;
    uint    ssm_count;
    uint    ssm_state;
    vtss_port_speed_t speed;
    BOOL    fiber;
    BOOL    master;
    BOOL    link;
    BOOL    phy;
    BOOL    first_port_callback;
    uint    prefer_timer;
} port_state_t;

typedef struct
{
    BOOL    new_locs;
    BOOL    new_fos;
    uint    wtr_state;
    uint    wtr_timer;
    BOOL    holdoff;
} clock_state_t;

typedef struct
{
    ulong                        version;  /* Block version */
    synce_mgmt_clock_conf_blk_t  clock;
    synce_mgmt_port_conf_blk_t   port;
} conf_blk_t;

typedef struct
{
    BOOL active;
    BOOL master;
    u32  timer;
} port_prefer_state;

#ifdef VTSS_SW_OPTION_PACKET
static u8  ssm_dmac[6] = {0x01,0x80,0xC2,0x00,0x00,0x02};
static u8  ssm_ethertype[2] = {0x88,0x09};
static u8  ssm_standard[6] = {0x0A,0x00,0x19,0xA7,0x00,0x01};
#endif

static cyg_flag_t       func_wait_flag;
static cyg_handle_t     func_thread_handle;
static cyg_thread       func_thread_block;
static char             func_thread_stack[THREAD_DEFAULT_STACK_SIZE];

static critd_t          crit; 

static synce_mgmt_clock_conf_blk_t   clock_conf_blk;
static synce_mgmt_port_conf_blk_t    port_conf_blk;
static synce_mgmt_alarm_state_t      clock_alarm_state;
static clock_selector_state_t        clock_old_selector_state;

static port_state_t                  port_state[SYNCE_PORT_COUNT];
static clock_state_t                 clock_state[SYNCE_NOMINATED_MAX];

static vtss_phy_clock_conf_t         phy_clock_config[SYNCE_NOMINATED_MAX];

static BOOL source_port[SYNCE_NOMINATED_MAX][SYNCE_PORT_COUNT];

static u32 mux_selector[SYNCE_NOMINATED_MAX] [SYNCE_PORT_COUNT];


static BOOL pcb104_synce = FALSE;
//#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
//#include <cyg/io/i2c_vcoreiii.h>
//#define I2C_DEVICE CYG_I2C_VCOREIII_DEVICE
//#else
//#include <cyg/io/i2c_vcoreii.h>
//#define I2C_DEVICE CYG_I2C_VCOREII_DEVICE
//#endif
//I2C_DEVICE(cpld_device, 0x75);
//I2C_DEVICE(si5338_device, 0x71);



#if defined(VTSS_FEATURE_SYNCE)
static BOOL internal[SYNCE_PORT_COUNT];
#endif


#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "SyncE",
    .descr     = "SyncE module."
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = { 
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_PDU_RX] = { 
        .name      = "rx",
        .descr     = "Rx PDU print out ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_PDU_TX] = { 
        .name      = "tx",
        .descr     = "Tx PDU print out ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_API] = { 
        .name      = "api",
        .descr     = "Switch API printout",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CLOCK] = { 
        .name      = "clock",
        .descr     = "Clock API printout ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CLI] = { 
        .name      = "cli",
        .descr     = "CLI",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define CRIT_ENTER() critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CRIT_EXIT()  critd_exit( &crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define CRIT_ENTER() critd_enter(&crit)
#define CRIT_EXIT()  critd_exit( &crit)
#endif /* VTSS_TRACE_ENABLED */


/****************************************************************************/
/*  Various local functions                                                                                                          */
/****************************************************************************/
static void configure_master_slave(void);
static void set_tx_ssm(void);
static void set_clock_source(uint source);
static void set_wtr(uint source);
static BOOL port_is_nominated(uint port, uint *source);

static i8 synce_clk_sel(void)
{
    // the I2C clock selector depends on the board
    int board_type = vtss_board_type();

    if (board_type == VTSS_BOARD_SERVAL_REF || board_type == VTSS_BOARD_SERVAL_PCB106_REF) {
        return 11;    // on serval ref board, use GPIO#11 as clock selector
    } else {
        return NO_I2C_MULTIPLEXER;
    }
}

static void synce_locs_interrupt_function(vtss_interrupt_source_t    source_id,
                                          u32                        clock_input)
{
    /* hook up to the next interrupt */
    if (vtss_interrupt_source_hook_set(synce_locs_interrupt_function,
                                       INTERRUPT_SOURCE_LOCS,
                                       INTERRUPT_PRIORITY_CLOCK) != VTSS_OK)    T_D("error returned source_id %u  clock_input %u", source_id, clock_input);
    T_I("source_id %u  clock_input %u", source_id, clock_input);
    CRIT_ENTER();
    clock_state[clock_input].new_locs = TRUE;

    cyg_flag_setbits(&func_wait_flag, FLAG_CLOCK_SOURCE_LOCS);
    CRIT_EXIT();
}


static void synce_fos_interrupt_function(vtss_interrupt_source_t    source_id,
                                         u32                        clock_input)
{
    /* hook up to the next interrupt */
    if (vtss_interrupt_source_hook_set(synce_fos_interrupt_function,
                                       INTERRUPT_SOURCE_FOS,
                                       INTERRUPT_PRIORITY_CLOCK) != VTSS_OK)    T_D("error returned source_id %u  clock_input %u", source_id, clock_input);

    CRIT_ENTER();
    clock_state[clock_input].new_fos = TRUE;

    cyg_flag_setbits(&func_wait_flag, FLAG_CLOCK_SOURCE_FOS);
    CRIT_EXIT();
}


static void synce_losx_interrupt_function(vtss_interrupt_source_t      source_id,
                                          u32                          clock_input)
{
    /* hook up to the next interrupt */
    if (vtss_interrupt_source_hook_set(synce_losx_interrupt_function,
                                       INTERRUPT_SOURCE_LOSX,
                                       INTERRUPT_PRIORITY_CLOCK) != VTSS_OK)    T_D("error returned source_id %u  clock_input %u", source_id, clock_input);

    T_I("source_id %u  clock_input %u", source_id, clock_input);
    CRIT_ENTER();
    cyg_flag_setbits(&func_wait_flag, FLAG_CLOCK_SOURCE_LOSX);
    CRIT_EXIT();
}


static void synce_lol_interrupt_function(vtss_interrupt_source_t      source_id,
                                         u32                          clock_input)
{
    /* hook up to the next interrupt */
    if (vtss_interrupt_source_hook_set(synce_lol_interrupt_function,
                                       INTERRUPT_SOURCE_LOL,
                                       INTERRUPT_PRIORITY_CLOCK) != VTSS_OK)    T_D("error returned device_id %u  clock_input %u", source_id, clock_input);
    T_I("source_id %u  clock_input %u", source_id, clock_input);

    CRIT_ENTER();
    cyg_flag_setbits(&func_wait_flag, FLAG_CLOCK_SOURCE_LOL);
    CRIT_EXIT();
}


static void port_change_callback(vtss_port_no_t port_no, port_info_t *info)
{
    uint  port;

    port = port_no-VTSS_PORT_NO_START;
        
    if (port >= SYNCE_PORT_COUNT)  return;
    
    CRIT_ENTER();
    port_state[port].first_port_callback = TRUE;
    if (port_state[port].link != info->link)
    {
        port_state[port].new_link = TRUE;
        port_state[port].link = info->link;
    }
    if (port_state[port].fiber != info->fiber)
    {
        port_state[port].new_fiber = TRUE;
        port_state[port].fiber = info->fiber;
    }
    port_state[port].phy = info->phy;
    port_state[port].speed = info->speed;
    //if (!info->phy) {
    //    if (port_10g_phy(port_no)) {
    //        port_state[port].phy = TRUE;
    //        if (port_state[port].fiber != TRUE)
    //        {
    //            port_state[port].new_fiber = TRUE;
    //            port_state[port].fiber = TRUE;
    //        }
    //    }
    //}
    T_W("port_no %u  link %u, fiber %u, phy %d, speed %d", port_no, info->link, port_state[port].fiber, port_state[port].phy, info->speed);

    cyg_flag_setbits(&func_wait_flag, FLAG_PORT_CALL_BACK);
    CRIT_EXIT();
}


static void system_reset(vtss_restart_t restart)
{
    if (clock_selection_mode_set(CLOCK_SELECTION_MODE_FORCED_HOLDOVER, 0) != VTSS_OK)    T_D("error returned");
}


static BOOL port_is_nominated(uint port, uint *source)
{
    for (*source=0; *source<synce_my_nominated_max; ++*source)
    {
        if (clock_conf_blk.nominated[*source] && (clock_conf_blk.port[*source] == port))
        /* This 'port' is a nominated port */
            return (TRUE);
    }
    return (FALSE);
}



static void save_config_blk(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    ulong       size;
    conf_blk_t  *blk;

    if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_SYNCE_CONF_TABLE, &size)) != NULL) {
        blk->clock = clock_conf_blk;
        blk->port = port_conf_blk;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_SYNCE_CONF_TABLE);
    } else {
        T_W("Error opening conf");
    }
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}



void synce_set_conf_to_default(synce_mgmt_clock_conf_blk_t  *clockk,
                               synce_mgmt_port_conf_blk_t   *port)
{
    uint   i;
    clockk->station_clk_in  = SYNCE_MGMT_STATION_CLK_DIS;
    clockk->station_clk_out = SYNCE_MGMT_STATION_CLK_DIS;
    clockk->selection_mode = SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_REVERTIVE;
    clockk->source = 0;
    clockk->wtr_time = 60*5; /* Default 5 min.*/
    clockk->ssm_holdover = QL_NONE;
    clockk->ssm_freerun = QL_NONE;
    clockk->eec_option = EEC_OPTION_1;
    for (i=0; i<synce_my_nominated_max; ++i)
    {
        clockk->nominated[i] = FALSE;
        clockk->port[i] = 0;
        clockk->priority[i] = 0;
        clockk->aneg_mode[i] = SYNCE_MGMT_ANEG_NONE;
        clockk->holdoff_time[i] = 0;
        clockk->ssm_overwrite[i] = QL_NONE;
    }

    for (i=0; i<SYNCE_PORT_COUNT; ++i)
    {
        port->ssm_enabled[i] = FALSE;
    }
}



static void apply_configuration(conf_blk_t  *blk) 
{
    uint  i;
    uint  rc=SYNCE_RC_OK;

    for (i=0; i<synce_my_nominated_max; ++i)
        rc += synce_mgmt_nominated_priority_set(i, blk->clock.priority[i]);

    
    for (i=0; i<synce_my_nominated_max; ++i)
        rc += synce_mgmt_nominated_source_set(i,
                                              blk->clock.nominated[i],
                                              blk->clock.port[i],
                                              blk->clock.aneg_mode[i],
                                              blk->clock.holdoff_time[i],
                                              blk->clock.ssm_overwrite[i]);

    rc += synce_mgmt_nominated_selection_mode_set(blk->clock.selection_mode, blk->clock.source, blk->clock.wtr_time/60, blk->clock.ssm_holdover, blk->clock.ssm_freerun, blk->clock.eec_option);
    rc += synce_mgmt_station_clock_out_set(blk->clock.station_clk_out);
    rc += synce_mgmt_station_clock_in_set(blk->clock.station_clk_in);

    for (i=0; i<SYNCE_PORT_COUNT; ++i)
        rc += synce_mgmt_ssm_set(i+VTSS_PORT_NO_START, blk->port.ssm_enabled[i]);

    if (rc != SYNCE_RC_OK)      T_D("error returned");
}



static uint overwrite_conv(synce_mgmt_quality_level_t overwrite)
{
    switch (overwrite)
    {
        case QL_NONE: return (SSM_QL_NONE);   //This is not standard
        case QL_PRC:  return (SSM_QL_PRC);
        case QL_SSUA: return (SSM_QL_SSUA);
        case QL_SSUB: return (SSM_QL_SSUB);
        case QL_EEC2: return (SSM_QL_EEC2);
        case QL_EEC1: return (SSM_QL_EEC1);
        case QL_DNU:  return (SSM_QL_DNU);
        case QL_INV:  return (SSM_QL_INV);    // for test only
        default:  return (SSM_QL_DNU);
    }
}



static uint get_ssm_rx(uint port, uint source)
{
    if (port_conf_blk.ssm_enabled[port])
        return (clock_conf_blk.ssm_overwrite[source] != QL_NONE) ? overwrite_conv(clock_conf_blk.ssm_overwrite[source]) : port_state[port].ssm_rx;
    else
        return (overwrite_conv(clock_conf_blk.ssm_overwrite[source]));
}



static void set_wtr(uint source)
{
    uint wtr_state, port;

    if ((clock_conf_blk.wtr_time != 0) && clock_conf_blk.nominated[source])
    {
    /* WTR enabled  on a nominated source */
        wtr_state = clock_state[source].wtr_state;
        port = clock_conf_blk.port[source];
        if (!port_state[port].first_port_callback)    return;

//T_D("link %d  fos %d  ssm_state %d", port_state[port].link, clock_alarm_state.fos[source], port_state[port].ssm_state);

        if (clock_alarm_state.fos[source])
            clock_state[source].wtr_state |= WTR_FOS;
        else
            clock_state[source].wtr_state &= ~WTR_FOS;


        if (port_state[port].link)
            clock_state[source].wtr_state &= ~WTR_LINK;
        else
            clock_state[source].wtr_state |= WTR_LINK;


        if (port_conf_blk.ssm_enabled[port] && (clock_conf_blk.ssm_overwrite[source] == QL_NONE) && (port_state[port].ssm_state & SSM_FAIL))
            clock_state[source].wtr_state |= WTR_SSM_FAIL;
        else
            clock_state[source].wtr_state &= ~WTR_SSM_FAIL;

        if (port_conf_blk.ssm_enabled[port] && (clock_conf_blk.ssm_overwrite[source] == QL_NONE) && (port_state[port].ssm_state & SSM_INVALID))
            clock_state[source].wtr_state |= WTR_SSM_INV;
        else
            clock_state[source].wtr_state &= ~WTR_SSM_INV;

        if (clock_state[source].wtr_state != WTR_OK)
        {
            clock_alarm_state.wtr[source] = FALSE;
            clock_state[source].wtr_timer = 0;
        }
        else
        {
            if (wtr_state != WTR_OK) {
                clock_alarm_state.wtr[source] = TRUE;
                clock_state[source].wtr_timer = clock_conf_blk.wtr_time;
            }
        }
    }
    else
    {
        clock_state[source].wtr_state = WTR_OK;
        clock_state[source].wtr_timer = 0;
        clock_alarm_state.wtr[source] = FALSE;
    }
    T_D("source %d  wtr_time %d  wtr_state %d  WTR %d", source, clock_conf_blk.wtr_time, clock_state[source].wtr_state, clock_alarm_state.wtr[source]);
}


static BOOL get_source_disable(uint port, uint source)
{
    if ((get_ssm_rx(port, source) == SSM_QL_DNU) ||         /* DNU is received */
        ((port_state[port].ssm_state & (SSM_FAIL | SSM_INVALID)) && (clock_conf_blk.ssm_overwrite[source] == QL_NONE)))     /* SSM is failing and there is no SSM owerwrite */
        return (TRUE);

    if ((clock_conf_blk.wtr_time != 0) && (clock_alarm_state.wtr[source] || (clock_state[source].wtr_state != WTR_OK)))     /* WTR is enabled and this source is in WTR state */
        return (TRUE);

    if (!port_state[port].phy && !port_state[port].link) /* Link is down and port is NOT a PHY meaning SERDES - need to disable in order to create LOCS */
        return TRUE;
    
    return(FALSE);
}


static void set_clock_source(uint source)
{
    vtss_rc                 rc;
    vtss_phy_recov_clk_t   phy_clk_port;
#if defined(VTSS_FEATURE_SYNCE)
    u32                    i;
    vtss_synce_clk_port_t  switch_clk_port;
    vtss_synce_clock_in_t  clk_in;
#endif
    uint                   port;

    if (source == STATION_CLOCK_SOURCE_NO) return;  /* station clock is neither set up in the Switch nor in the PHY */
    /* This port number is actually allso identifying the PHY - if relevant. */
    /* Requirement is that this function is only called with not nominated source if it previously has been nominated */
    port = clock_conf_blk.port[source];

    if (vtss_board_type() == VTSS_BOARD_SERVAL_REF || vtss_board_type() == VTSS_BOARD_SERVAL_PCB106_REF) {
        phy_clk_port = VTSS_PHY_RECOV_CLK1;
    } else {
        if (source % 2)  phy_clk_port = VTSS_PHY_RECOV_CLK2;
        else             phy_clk_port = VTSS_PHY_RECOV_CLK1;
    }
    
#if defined(VTSS_FEATURE_SYNCE)
    if (source % 2)  switch_clk_port = VTSS_SYNCE_CLK_B;
    else             switch_clk_port = VTSS_SYNCE_CLK_A;
    clk_in.enable = FALSE;
    clk_in.port_no = port;
    clk_in.squelsh = SWITCH_AUTO_SQUELCH;
#endif

    if (port_state[port].phy)
    {/* Port is a PHY */
        /* calculate the clock port */
        if (!clock_conf_blk.nominated[source])
        /* disable clock out from phy if not nominated */
            phy_clock_config[source].src = VTSS_PHY_CLK_DISABLED;
        else
        { /* A PHY port is norminated */
            if (port_state[port].fiber) phy_clock_config[source].src = VTSS_PHY_SERDES_MEDIA;
            else                        phy_clock_config[source].src = VTSS_PHY_COPPER_MEDIA;

            if (get_source_disable(port, source))
                phy_clock_config[source].src = VTSS_PHY_CLK_DISABLED;

#if defined(VTSS_FEATURE_SYNCE)
            if (internal[port])   /* If internal PHY is nominated then disable any SERDES input port selection */
            {
                T_DG(TRACE_GRP_API, "vtss_synce_clock_in_set  clk_port %u  enable %u  port_no %u", switch_clk_port, clk_in.enable, clk_in.port_no);
                if ((rc = vtss_synce_clock_in_set(NULL,  switch_clk_port,  &clk_in)) != VTSS_OK)    T_D("error returned port %u  rc %u", port, rc);
            }
#endif
        }

        /* Configure clock selection in PHY */
        T_DG(TRACE_GRP_API, "vtss_phy_clock_conf_set  port %u  clk_port %u  src %u", port, phy_clk_port, phy_clock_config[source].src);
        if (port_state[port].speed == VTSS_SPEED_10G) {
            T_WG(TRACE_GRP_API, "10G phy port %d nominated", port);
        } else {
            if ((rc = vtss_phy_clock_conf_set(PHY_INST, port, phy_clk_port, &phy_clock_config[source])) != VTSS_OK)    T_D("error returned port %u  rc %u", port, rc);
        }
#ifdef VTSS_ARCH_JAGUAR_1
        if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
            /* set up the multiplexer in the CPLD */
            pcb107_cpld_mux_set(source, mux_selector[source] [port]);
        }
#endif
    }
#if defined(VTSS_FEATURE_SYNCE)
    else
    {/* Port is NOT a PHY meaning it is a SERDES */
        if (!clock_conf_blk.nominated[source])
        /* disable clock out from SERDES if not nominated */
            clk_in.enable = FALSE;
        else
        if (clock_conf_blk.nominated[source])
        {
            clk_in.enable = TRUE;
            if (get_source_disable(port, source))
                clk_in.enable = FALSE;

            /* Disable any internal PHY input port selection - any port number on internal PHY will do for identification */
            phy_clock_config[source].src = VTSS_PHY_CLK_DISABLED;
            for (i=0; i<SYNCE_PORT_COUNT; ++i)  if (internal[i])    break;
            if (i < VTSS_FRONT_PORT_COUNT)
            {
                T_DG(TRACE_GRP_API, "vtss_phy_clock_conf_set  port %u  clk_port %u  src %u", i, phy_clk_port, phy_clock_config[source].src);
                if ((rc = vtss_phy_clock_conf_set(PHY_INST, i, phy_clk_port, &phy_clock_config[source])) != VTSS_OK)    T_D("error returned port %u  rc %u", i, rc);
            }
        }
#ifdef VTSS_ARCH_JAGUAR_1
        if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
            /* set up the multiplexer in the CPLD */
            pcb107_cpld_mux_set(source, mux_selector[source] [port]);
        }
#endif

        T_D("SWITCH source %d  port %d  clk_port %d", source, port, switch_clk_port);

        /* Configure clock selection in SWITCH */
        T_DG(TRACE_GRP_API, "vtss_synce_clock_in_set  clk_port %u  enable %u  port_no %u", switch_clk_port, clk_in.enable, clk_in.port_no);
        if ((rc = vtss_synce_clock_in_set(NULL,  switch_clk_port,  &clk_in)) != VTSS_OK)    T_D("error returned port %u  rc %u", port, rc);
    }
#endif
}


static void configure_master_slave(void)
{
    uint                    best_prio, clock_input, port, i, priority;
    vtss_phy_status_1g_t    phy_status;
    vtss_phy_conf_1g_t      phy_setup;
    BOOL                    found, master;
    clock_selector_state_t  selector_state;

    /* get clculated selected clock source */
    clock_input = 0;
    best_prio = synce_my_priority_max;
    found = FALSE;

    if ((clock_conf_blk.selection_mode == SYNCE_MGMT_SELECTOR_MODE_FORCED_HOLDOVER) || (clock_conf_blk.selection_mode == SYNCE_MGMT_SELECTOR_MODE_FORCED_FREE_RUN))
        return;
    else
    if (clock_conf_blk.selection_mode == SYNCE_MGMT_SELECTOR_MODE_MANUEL) {
        clock_input = clock_conf_blk.source;
        found = TRUE;
    }
    else
    if (clock_conf_blk.selection_mode == SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_NONREVERTIVE) {
        if (clock_selector_state_get(&selector_state, &clock_input) != VTSS_OK)    T_D("clock_selector_state_get failed");
        if (selector_state == CLOCK_SELECTOR_STATE_LOCKED)
            found = TRUE;
    }
    else {  /* This is automatic revertive - find the best clock to be selected */
        for (i=0; i<synce_my_nominated_max && i != STATION_CLOCK_SOURCE_NO; ++i) {
            if (clock_conf_blk.nominated[i]) {
                port = clock_conf_blk.port[i];          /* Port number of this clock input */
                if (clock_priority_get(i, &priority) != VTSS_OK)    T_D("Call to clock_priority_get source %u failed", i);
                if ((priority < best_prio) && !(port_state[port].ssm_state & SSM_NOT_SEEN) && (!get_source_disable(port, i) || clock_state[i].holdoff)) {
                    best_prio = priority;
                    clock_input = i;
                    found = TRUE;
                }
            }
        }
    }

    if (found)
    {
    /* A clock source is calculated to be the selected input */
        if (clock_conf_blk.aneg_mode[clock_input] != SYNCE_MGMT_ANEG_FORCED_SLAVE) {
            clock_conf_blk.aneg_mode[clock_input] = SYNCE_MGMT_ANEG_PREFERED_SLAVE;  /* The selected source is preferred slave */
        }
    }

    for (i=0; i<synce_my_nominated_max; ++i) { /* Check for ANEG */
        if (clock_conf_blk.aneg_mode[i] != SYNCE_MGMT_ANEG_NONE) { /* Some ANAG is requested */
            port = clock_conf_blk.port[i];
            if (!(port_state[port].prefer_timer) && port_state[port].phy && (port_state[port].speed == VTSS_SPEED_1G)) { /* ANEG is allowed - not already active and correct port type */
                master = (clock_conf_blk.aneg_mode[i] == SYNCE_MGMT_ANEG_PREFERED_MASTER);
                if (vtss_phy_status_1g_get(PHY_INST, port, &phy_status) != VTSS_OK)    T_D("vtss_phy_status_1g_get port %u failed", port);

                if (phy_status.master != master) {  /* This port has different aneg mode than requested */
                    phy_setup.master.cfg = TRUE;
                    phy_setup.master.val = master;
                    if (vtss_phy_conf_1g_set(PHY_INST, port, &phy_setup) != VTSS_OK)    T_D("vtss_phy_conf_1g_set port %u failed", port);
                    port_state[port].prefer_timer = 1;  /* Activate prefer time out */
                }
            }
            if (clock_conf_blk.aneg_mode[i] != SYNCE_MGMT_ANEG_FORCED_SLAVE)  /* Forced slave mode cannot be cleared - unless from management */
                clock_conf_blk.aneg_mode[i] = SYNCE_MGMT_ANEG_NONE;
        }
    }
}


static void set_clock_priority(void)
{
    uint prio_assigned[SYNCE_NOMINATED_MAX];
    uint ssm, ssm_rx, prio, port, i, prio_count = 0;
    
    for (i=0; i<synce_my_nominated_max; ++i)    prio_assigned[i] = synce_my_prio_disabled;
    
    for (ssm=0; ssm<=SSM_QL_DNU; ++ssm)
    {
    /* Check all qualities */
        if (!((ssm == SSM_QL_PRC)||(ssm == SSM_QL_SSUA)||(ssm == SSM_QL_SSUB)||(ssm == SSM_QL_EEC2)||(ssm == SSM_QL_EEC1)||(ssm == SSM_QL_DNU)||(ssm == SSM_QL_NONE)))
            continue;

        for (prio=0; prio<synce_my_nominated_max; ++prio)
        {
        /* check all priorities */
            for (i=0; i<synce_my_nominated_max; ++i)
            {
            /* check all clock sources */
                if (clock_conf_blk.nominated[i])
                {
                /* a nominated clock */
                    port = clock_conf_blk.port[i];
                    
                    /* check if 'rx_ssm' must be overwrite */
                    ssm_rx = get_ssm_rx(port, i);
                    T_IG(TRACE_GRP_CLOCK, "ssm %d, port %d, rx_ssm %d, prio %u, source %d", ssm, port, ssm_rx, prio, i);
                        
                    if ((ssm_rx == ssm) && (clock_conf_blk.priority[i] == prio))
                    {
                    /* this clock source is nominated and receiving ok SSM that match and got priority match */
                        prio_assigned[i] = prio_count;
                        prio_count++;
                    }
                }
            }
        }
    }
    
    if (prio_count < synce_my_nominated_max)
    {
    /* NOT all possible priorities has been assigned - has to be done */
        for (i=0; i<synce_my_nominated_max; ++i)
        {
            if (prio_assigned[i] == synce_my_nominated_max)
            {
                prio_assigned[i] = prio_count;
                prio_count++;
            }
        }
    }

    if (prio_count != synce_my_nominated_max)
        T_I("invalid numer of priorities assigned %d", prio_count);

    for (i=0; i<synce_my_nominated_max; ++i)
    {
        T_DG(TRACE_GRP_CLOCK, "clock_input_priority_set  source %u  prio %u", i, prio_assigned[i]);
        if (clock_priority_set(i, prio_assigned[i]) != VTSS_OK)    T_D("error returned source %u", i);
    }
}



static void set_tx_ssm(void)
{
    static uint             old_tx_ssm = SSM_QL_EEC1;
    clock_selector_state_t  state;
    uint                    clock_input, port, new_tx_ssm, i;

    if (clock_selector_state_get(&state, &clock_input) != VTSS_OK)    T_D("error returned");
    clock_old_selector_state = state;

    new_tx_ssm = old_tx_ssm;
    port = 0;
    
    switch (state)
    {
        case CLOCK_SELECTOR_STATE_LOCKED:
        {
            port = clock_conf_blk.port[clock_input];
            new_tx_ssm = get_ssm_rx(port, clock_input);
            break;
        }
        case CLOCK_SELECTOR_STATE_HOLDOVER:
        {
            new_tx_ssm = overwrite_conv(clock_conf_blk.ssm_holdover);
            break;
        }
        case CLOCK_SELECTOR_STATE_FREERUN:
        {
            new_tx_ssm = overwrite_conv(clock_conf_blk.ssm_freerun);
            break;
        }
        default: new_tx_ssm = SSM_QL_DNU;;
    }

    /* transmit received ssm on all ports - ofc DNU on selected */
    for (i=0; i<SYNCE_PORT_COUNT; ++i)
            port_state[i].ssm_tx = new_tx_ssm;

    if (state == CLOCK_SELECTOR_STATE_LOCKED)
        port_state[port].ssm_tx = SSM_QL_DNU;

#ifdef VTSS_SW_OPTION_PACKET
    if (new_tx_ssm != old_tx_ssm)
        cyg_flag_setbits(&func_wait_flag, FLAG_SSM_EVENT);
#endif

    old_tx_ssm = new_tx_ssm;
}

#ifdef VTSS_SW_OPTION_PACKET
static void ssm_frame_tx(uint port, BOOL event_flag, uint ssm)
{
    static u8    reserved[3] = {0x00,0x00,0x00};
    static u8    tlv[3] = {0x01,0x00,0x04};
    u8           version = 0x10;
    u32          len = 64;
    u8           *buffer;
    conf_board_t conf;

    if (!port_conf_blk.ssm_enabled[port])  return;

    if (conf_mgmt_board_get(&conf) < 0)
        return;

    if ((buffer = packet_tx_alloc(len))) {
        packet_tx_props_t tx_props;

//T_D("port %d ssm %x", port, ssm);
        memset(buffer, 0xff, len);
        if (event_flag) version |= 0x08;
        ssm &= 0x0F;

        memcpy(buffer, ssm_dmac, 6);
        memcpy(buffer+6, conf.mac_address, 6);
        memcpy(buffer+6+6, ssm_ethertype, 2);
        memcpy(buffer+6+6+2, ssm_standard, 6);
        memcpy(buffer+6+6+2+6, &version, 1);
        memcpy(buffer+6+6+2+6+1, reserved, 3);
        memcpy(buffer+6+6+2+6+1+3, tlv, 3);
        memcpy(buffer+6+6+2+6+1+3+3, &ssm, 1);
        packet_tx_props_init(&tx_props);
        tx_props.packet_info.modid     = VTSS_MODULE_ID_SYNCE;
        tx_props.packet_info.frm[0]    = buffer;
        tx_props.packet_info.len[0]    = len;
        tx_props.tx_info.dst_port_mask = VTSS_BIT64(port);

        T_DG(TRACE_GRP_PDU_TX, "port %u  length %u  dmac %X-%X-%X-%X-%X-%X  smac %X-%X-%X-%X-%X-%X  Ethertype %X-%X", port, len, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13]);
        T_DG(TRACE_GRP_PDU_TX, "frame[14-20] %X-%X-%X-%X-%X-%X-%X  ssm %X-%X-%X-%X", buffer[14], buffer[15], buffer[16], buffer[17], buffer[18], buffer[19], buffer[20], buffer[24], buffer[25], buffer[26], buffer[27]);
        if (packet_tx(&tx_props) != VTSS_RC_OK) {
            T_D("Packet tx failed");
        }
    }
}



static BOOL ssm_frame_rx(void *contxt, const unsigned char *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    uint port;

    CRIT_ENTER();
    T_DG(TRACE_GRP_PDU_RX, "port %u  length %u  dmac %X-%X-%X-%X-%X-%X  smac %X-%X-%X-%X-%X-%X  ethertype %X-%X", rx_info->port_no, rx_info->length, frm[0], frm[1], frm[2], frm[3], frm[4], frm[5], frm[6], frm[7], frm[8], frm[9], frm[10], frm[11], frm[12], frm[13]);
    T_DG(TRACE_GRP_PDU_RX, "frame[14-20] %X-%X-%X-%X-%X-%X-%X  ssm %X-%X-%X-%X", frm[14], frm[15], frm[16], frm[17], frm[18], frm[19], frm[20], frm[24], frm[25], frm[26], frm[27]);
    if ((rx_info->length < 28) || (memcmp(&frm[14], ssm_standard, 6) == 0))
    {
    /* this is a 'true' ESMC (SSM) slow protocal PDU */
        port = rx_info->port_no;
        if ((port < SYNCE_PORT_COUNT) && port_conf_blk.ssm_enabled[port])
        {
            port_state[port].ssm_frame = frm[27] & 0x0F;
            port_state[port].new_ssm = TRUE;
            
            cyg_flag_setbits(&func_wait_flag, FLAG_FRAME_RX);
        }
        CRIT_EXIT();
        return(TRUE);
    }

    CRIT_EXIT();
    return(FALSE);
}
#endif


static void change_state_action(uint source)
{
    /* do the WTR stuff */
    set_wtr(source);
    
    /* calculate and change priority in clock controller */
    set_clock_priority();

    /* calculate and change clock source in phy */
    set_clock_source(source);
    
    /* check for change in transmission of SSM */
    set_tx_ssm();

    /* Configure master/slave */
    configure_master_slave();
}

void set_eec_option(synce_eec_option_t eec_option)
{
    (void)clock_eec_option_set(eec_option == EEC_OPTION_1 ? CLOCK_EEC_OPTION_1 : CLOCK_EEC_OPTION_2);
}

static void func_thread(cyg_addrword_t data)
{
    uint                  i, source, new_ssm, old_ssm, old_state, port;
    BOOL                  state, holdoff_active;
    cyg_flag_value_t      flag_value;
    vtss_phy_status_1g_t  phy_status;
    vtss_phy_conf_1g_t    phy_setup;
    conf_blk_t            save_blk;
    u8                    rx_buf[16], tx_buf[2];
    vtss_restart_status_t status;
    ulong                 size;
    vtss_mtimer_t         onesec_timer;
    clock_selector_state_t new_selector_state;
    uint new_source;
    
    if (vtss_restart_status_get(NULL, &status) != VTSS_RC_OK)    T_D("error returned");
    /* This is checkking for Vitesse SyncE module (PCB104). If it is the CPLD must be initialized and the 5338 must be reset */
    tx_buf[0] = 1;
    if (vtss_i2c_wr_rd(NULL, 0x75, tx_buf, 1, rx_buf, 2, 0xFF, synce_clk_sel()) == VTSS_RC_OK) {
        T_D("received correctly PCB104 CPLD Version %X  Device %x", rx_buf[0], rx_buf[1]);
        if ((rx_buf[0] >= 0x03) && (rx_buf[1] == 0x33)) {   /* This PCB104 SyncE */
            T_D("PCB104 CPLD Version accepted");
            pcb104_synce = TRUE;
            if (rx_buf[0] < 0x0A) {   /* This PCB104 SyncE has an old Firmware thes causes reboot problems on the PCB107*/
                T_E("Please upgrade PCB104 CPLD Version to 10 or greater, older versions causes reboot problems");
            }    
            if (status.restart==VTSS_RESTART_COLD) {
                tx_buf[0] = 16; /* Set CPLD SPI selector to Si5326 */
                if (vtss_i2c_wr_rd(NULL, 0x75, tx_buf, 1, rx_buf, 1, 0xFF, synce_clk_sel()) == VTSS_RC_OK) {
                    rx_buf[0] &= ~0x1C;
                    rx_buf[0] |= 0x18;
                    tx_buf[1] = rx_buf[0];
                    if (vtss_i2c_wr(NULL, 0x75, tx_buf, 2, 0xFF, synce_clk_sel()) != VTSS_RC_OK)   T_D("Failed SPI selector set to SI5326  %X", tx_buf[1]);
                }
                else    T_D("Failed CPLD SPI selector get");

                tx_buf[0] = 0xF6;  /* This is reset of Silabs 5338 */
                tx_buf[1] = 2;
                if (vtss_i2c_wr(NULL, 0x71, tx_buf, 2, 50, synce_clk_sel()) != VTSS_RC_OK)   T_D("Failed reset of SI5338");
            }
        }
    }
    else    T_D("faild read of PCB104 CPLD Version %X  Device %x", rx_buf[0], rx_buf[1]);

    if (pcb104_synce) {
        /* Enable Station clock input for PCB104 */
        source_port[0][SYNCE_STATION_CLOCK_PORT] = 1;
        source_port[1][SYNCE_STATION_CLOCK_PORT] = 1;
        
    }
    if (clock_startup((status.restart==VTSS_RESTART_COLD), pcb104_synce) != VTSS_OK)    T_D("error returned");

    /*lint -e{455} */
    CRIT_EXIT();

    /* Register on all interrupt from clock controller */
    synce_locs_interrupt_function(INTERRUPT_SOURCE_LOCS, 0);
    synce_locs_interrupt_function(INTERRUPT_SOURCE_LOCS, 1);
    synce_locs_interrupt_function(INTERRUPT_SOURCE_LOCS, 2);
    synce_fos_interrupt_function(INTERRUPT_SOURCE_FOS, 0);
    synce_fos_interrupt_function(INTERRUPT_SOURCE_FOS, 1);
    synce_fos_interrupt_function(INTERRUPT_SOURCE_FOS, 2);
    synce_losx_interrupt_function(INTERRUPT_SOURCE_LOSX, 0);
    synce_lol_interrupt_function(INTERRUPT_SOURCE_LOL, 0);

    if (misc_conf_read_use()) {
        conf_blk_t *blk;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_SYNCE_CONF_TABLE, &size)) == NULL || size != sizeof(*blk))
        {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            if ((blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_SYNCE_CONF_TABLE, sizeof(*blk))) != NULL) {
                blk->version = 1;
                synce_set_conf_to_default(&blk->clock, &blk->port);
            } else {
                T_W("conf_sec_create failed");
                return;
            }
        }

        /* Save and close configuration */
        save_blk = *blk;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_SYNCE_CONF_TABLE);
    } else {
        save_blk.version = 1;
        synce_set_conf_to_default(&save_blk.clock, &save_blk.port);
    }

    /* Apply configuration */
    apply_configuration(&save_blk);

#if defined(VTSS_FEATURE_SYNCE)
    vtss_synce_clock_out_t clk_out;

    /* Enable for clock output from switch */
    clk_out.divider = VTSS_SYNCE_DIVIDER_1;
    clk_out.enable = TRUE;
    if (vtss_synce_clock_out_set(NULL, VTSS_SYNCE_CLK_A, &clk_out) != VTSS_OK)    T_D("vtss_synce_clock_out_set returned error");
    if (vtss_synce_clock_out_set(NULL, VTSS_SYNCE_CLK_B, &clk_out) != VTSS_OK)    T_D("vtss_synce_clock_out_set returned error");
#endif

    VTSS_MTIMER_START(&onesec_timer, 950);
    holdoff_active = FALSE;

    for (;;)
    {
        flag_value = FLAG_CLOCK_SOURCE_LOCS | FLAG_CLOCK_SOURCE_FOS | FLAG_CLOCK_SOURCE_LOSX | FLAG_CLOCK_SOURCE_LOL | FLAG_PORT_CALL_BACK | FLAG_FRAME_RX | FLAG_SSM_EVENT;
        flag_value = cyg_flag_timed_wait(&func_wait_flag, flag_value, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, cyg_current_time() + VTSS_OS_MSEC2TICK(holdoff_active ? 50 : 1000));

        CRIT_ENTER();
        /* no return within critical zone */
#ifdef VTSS_SW_OPTION_PACKET
        if (VTSS_MTIMER_TIMEOUT(&onesec_timer) || (flag_value & FLAG_SSM_EVENT)) {
            /* SSM is transmitted onece a sec or when an event occur */
            for (i=0; i<SYNCE_PORT_COUNT; ++i)
            { /* Transmit SSM */
                if (!port_conf_blk.ssm_enabled[i])  continue;
                
                ssm_frame_tx(i, (flag_value & FLAG_SSM_EVENT), port_state[i].ssm_tx);
            }
        }
#endif
        if (VTSS_MTIMER_TIMEOUT(&onesec_timer)) {
            /* One second time out */
            VTSS_MTIMER_START(&onesec_timer, 950);
            /* check if selector state has changed (this check is needed because the ZL30343 may shortly enter holdover mode before locked mode */
            if (clock_selector_state_get(&new_selector_state, &new_source) != VTSS_OK)    T_D("clock_selector_state_get failed");
            if (clock_old_selector_state == CLOCK_SELECTOR_STATE_FREERUN || clock_old_selector_state != new_selector_state)
            {
            /* wait for selector out of free run mode during system startup */
                /* check for change in transmission of SSM */
                set_tx_ssm();

                /* check and configure master/slave mode of the nominated sources */
                configure_master_slave();
            }
            
            for (source=0; source<synce_my_nominated_max; ++source)
            { /* Run the WTR timer */
                if (clock_alarm_state.wtr[source])
                {
                /* WTR state - run timer */
                    if (clock_state[source].wtr_timer > 0)  clock_state[source].wtr_timer--;
                    if (clock_state[source].wtr_timer == 0)
                    {
                    /* WTR timeout */
                        T_D("WTR timeout  source %d", source);
                        clock_alarm_state.wtr[source] = FALSE;
                        change_state_action(source);
                    }
                }
            }

            for (i=0; i<SYNCE_PORT_COUNT; ++i)
            { /* Run the prefer master/slave timer */
                if (port_state[i].prefer_timer) {   /* Prefer timer is active */
                    port_state[i].prefer_timer++;
                    if (port_state[i].prefer_timer >= PREFER_TIMEOUT) {   /* Prefer time out */
                        port_state[i].prefer_timer = 0;  /* Passive prefer timer */
                        /* don't call the phy, for the station clock 'port' */
                        if (i < SYNCE_STATION_CLOCK_PORT && port_is_nominated(i, &source) && (clock_conf_blk.aneg_mode[source] != SYNCE_MGMT_ANEG_FORCED_SLAVE)) {
                            phy_setup.master.cfg = FALSE;
                            if (vtss_phy_conf_1g_set(PHY_INST, i, &phy_setup) != VTSS_OK)    T_D("vtss_phy_conf_1g_set port %u failed", i);
                        }
                    }
                }
            }

            for (i=0; i<SYNCE_PORT_COUNT; ++i)
            { /* Check for no SSM received - declare SSM failed */
                if (port_conf_blk.ssm_enabled[i] && !(port_state[i].ssm_state & (SSM_LINK_DOWN | SSM_FAIL)))
                {
                /* SSM enabled and link is active - check for SSM received */
                    port_state[i].ssm_count++;
                    if (port_state[i].ssm_count >= 5)
                    {
                    /* SSM on this port is now failed */
                        port_state[i].ssm_count = 5;
                        port_state[i].ssm_state |= SSM_FAIL;
                        port_state[i].ssm_rx = SSM_QL_FAIL;
                        T_D("SSM fail  port %d  ssm_state %d", i, port_state[i].ssm_state);

                        if (port_is_nominated(i, &source))
                        {
                            clock_alarm_state.ssm[source] = (port_state[i].ssm_state != SSM_OK) ? TRUE : FALSE;

                            change_state_action(source);
                        }
                    }
                }
            }

            for (i=0; i<SYNCE_PORT_COUNT; ++i)
            { /* Get PHY master/slave mode */
                if (port_state[i].phy && port_state[i].speed == VTSS_SPEED_1G)
                {
                    if (vtss_phy_status_1g_get(NULL, i, &phy_status) != VTSS_OK)    T_D("vtss_phy_status_1g_get port %u failed", i);
                    port_state[i].master = phy_status.master;
                }
            }

            for (i=0; i<synce_my_nominated_max; ++i)
            { /* Get digital hold valid state */
                if (clock_dhold_state_get(&state) != VTSS_OK)    T_D("error returned");

                clock_alarm_state.dhold = !state;
            }
        } /* One second time out */

        if (holdoff_active) { /* Run the hold off timer */
            BOOL holdoff;
            if (clock_holdoff_run(&holdoff_active) != VTSS_OK)    T_D("error returned");
            for (source=0; source<synce_my_nominated_max; ++source) { /* Check for holdoff time out on any clock source */
                if (clock_holdoff_active_get(source, &holdoff) != VTSS_OK)    T_D("error returned");
                if (clock_state[source].holdoff && !holdoff) { /* Hold off timer runnig out */
                    clock_state[source].holdoff = FALSE;
                    if (clock_locs_state_get(source, &state) != VTSS_OK)    T_D("error returned");
                    if (state)  change_state_action(source);
                }
            }
        }

        if (flag_value & FLAG_CLOCK_SOURCE_LOCS)
        { /* Loss Of Clock Source */
            for (source=0; source<synce_my_nominated_max; ++source)
            {
                if (clock_state[source].new_locs)
                {
                    T_D("new los source %d", source);
                    clock_state[source].new_locs = FALSE;
                    
                    if (clock_locs_state_get(source, &state) != VTSS_OK)    T_D("error returned");
                    T_I("locs source %d, value %d", source, state);
                    old_state = clock_alarm_state.locs[source];
                    clock_alarm_state.locs[source] = state;

                    if (clock_holdoff_event(source) != VTSS_OK)    T_D("error returned");   /* Signal Hold Off event to clock API */
                    if (clock_holdoff_active_get(source, &clock_state[source].holdoff) != VTSS_OK)    T_D("error returned");    /* Check if hold of active on any clock source */
                    holdoff_active = holdoff_active || clock_state[source].holdoff;

                    port = clock_conf_blk.port[source];
                    if (!clock_state[source].holdoff && (state || port_state[port].link))  /* Holdoff is not active and state is active or link is up */
                        change_state_action(source);
                }
            }
        }
        if (flag_value & FLAG_CLOCK_SOURCE_FOS)
        {
            for (i=0; i<synce_my_nominated_max; ++i)
            {
                if (clock_state[i].new_fos)
                {
                    T_D("new fos source %d", i);
                    clock_state[i].new_fos = FALSE;
                }
            }
        }
        if (flag_value & FLAG_CLOCK_SOURCE_LOSX)
        {
            T_D("new losx");
            if (clock_losx_state_get(&state) != VTSS_OK)    T_D("error returned");

            clock_alarm_state.losx = state;

            /* check for change in transmission of SSM */
            set_tx_ssm();
        }
        if (flag_value & FLAG_CLOCK_SOURCE_LOL)
        {
            T_D("new lol");
            if (clock_lol_state_get(&state) != VTSS_OK)    T_D("error returned");
            T_I("new lol %d", state);

            clock_alarm_state.lol = state;

            /* check for change in transmission of SSM */
            set_tx_ssm();
        }
        if (flag_value & FLAG_PORT_CALL_BACK)
        {   /* Change in port state */
            for (i=0; i<SYNCE_PORT_COUNT; ++i)
            {
                T_D("new-link port %d link %d", i, port_state[i].new_link);
                if (!port_state[i].link && port_conf_blk.ssm_enabled[i])   port_state[i].ssm_state |= (SSM_LINK_DOWN | SSM_NOT_SEEN);
                else                                                       port_state[i].ssm_state &= ~SSM_LINK_DOWN;

                if (port_state[i].new_link)
                {
                    T_D("new-link port %d link %d", i, port_state[i].link);
                    port_state[i].new_link = FALSE;
                    if (port_is_nominated(i, &source))
                    {
                    /* the port is norminated */
                        clock_alarm_state.ssm[source] = (port_state[i].ssm_state != SSM_OK) ? TRUE : FALSE;
                        if (port_state[i].link && !clock_state[source].holdoff)   /* Link is comming back up without holdoff */
                            change_state_action(source);
                        if (!port_state[i].link && clock_state[source].wtr_timer) /* Link is going down and WTR is running - stop it */
                            change_state_action(source);
#if defined(VTSS_FEATURE_SYNCE)
                        vtss_synce_clock_in_t  clk_in;
                        if (!port_state[i].phy && !port_state[i].link) { /* Link is going down and port is NOT a PHY meaning SERDES - need to disable in order to create LOCS */
                            clk_in.enable = FALSE;
                            clk_in.port_no = i;
                            clk_in.squelsh = SWITCH_AUTO_SQUELCH;
                            T_D("vtss_synce_clock_in_set source %d", source);
                            if (vtss_synce_clock_in_set(NULL,  (source % 2) ? VTSS_SYNCE_CLK_B : VTSS_SYNCE_CLK_A,  &clk_in) != VTSS_OK)    T_D("vtss_synce_clock_in_set returned error  port %u", i);
                        }
#endif
                    }
                }
                if (port_state[i].new_fiber)
                {
                    T_D("new-fiber port %d", i);
                    port_state[i].new_fiber = FALSE;
                    if (port_is_nominated(i, &source))
                        set_clock_source(source);
                }
            }
        }
        if (flag_value & FLAG_FRAME_RX)
        {
            for (i=0; i<SYNCE_PORT_COUNT; ++i)
            {
                if (port_state[i].new_ssm)
                {
                    port_state[i].new_ssm = FALSE;

                    new_ssm = port_state[i].ssm_frame;
                    old_ssm = port_state[i].ssm_rx;
                    old_state = port_state[i].ssm_state;

                    port_state[i].ssm_rx = new_ssm;
                    
                    /* has to count to 5 again to be SSM_FAIL */
                    port_state[i].ssm_count = 0;
                    port_state[i].ssm_state &= ~(SSM_FAIL | SSM_NOT_SEEN);

                    if ((new_ssm == SSM_QL_PRC)||(new_ssm == SSM_QL_SSUA)||(new_ssm == SSM_QL_SSUB)||(new_ssm == SSM_QL_EEC2)||
                        (new_ssm == SSM_QL_EEC1)||(new_ssm == SSM_QL_DNU)||(new_ssm == SSM_QL_NONE))
                    /* received a valid SSM */
                        port_state[i].ssm_state &= ~SSM_INVALID;
                    else
                        port_state[i].ssm_state |= SSM_INVALID;

                    if (((port_state[i].ssm_state != old_state) || (new_ssm != old_ssm)) && port_is_nominated(i, &source))
                    {
                        clock_alarm_state.ssm[source] = (port_state[i].ssm_state != SSM_OK) ? TRUE : FALSE;
                        T_D("frame rx  port %d  ssm %x  ssm_state %d", i, new_ssm, port_state[i].ssm_state);

                        if (port_state[i].link)    change_state_action(source);
                    }

                }
            }
        }

        CRIT_EXIT();
    }
}

/****************************************************************************/
/*  MISC. Help functions                                                    */
/****************************************************************************/

char* ssm_string(synce_mgmt_quality_level_t ssm)
{
    switch(ssm)
    {
        case QL_NONE: return("   QL_NONE");
        case QL_PRC:  return("    QL_PRC");
        case QL_SSUA: return("   QL_SSUA");
        case QL_SSUB: return("   QL_SSUB");
        case QL_DNU:  return("    QL_DNU");
        case QL_EEC2: return("   QL_EEC2");
        case QL_EEC1: return("   QL_EEC1");
        case QL_INV:  return("    QL_INV");
        case QL_FAIL: return("   QL_FAIL");
        case QL_LINK: return("   QL_LINK");
        default:      return("   QL_NONE");
    }
}

// Convert error code to text
// In : rc - error return code
char *synce_error_txt(vtss_rc rc)
{
    switch (rc)
    {
    case SYNCE_RC_OK:                return("SYNCE_RC_OK");
    case SYNCE_RC_INVALID_PARAMETER: return("Invalid parameter error returned from SYNCE\n");
    case SYNCE_RC_NOM_PORT:          return("Port nominated to a clock source is already nominated\n");
    case SYNCE_RC_SELECTION:         return("NOT possible to make Manuel To Selected if not in locked mode\n");
    case SYNCE_RC_INVALID_PORT:      return("The selected port is not valid\n");
    default:                         return("Unknown error returned from SYNCE\n");
    }
}


// Function for converting from user clock source (starting from 1) to clock source index (starting from 0) 
// In - uclk - user clock source
// Return - Clock source index 
u8 synce_uclk2iclk(u8 uclk) {
  return uclk - 1;
}

// Function for converting from clock source index (starting from 0) to user clock source (starting from 1) 
// In - iclk - Index clock source
// Return - Clock source index 
u8 synce_iclk2uclk(u8 iclk) {
  return iclk + 1;
}

static void update_clk_in_selector(uint source)
{
    clock_frequency_t    frq;
    u8 tx_buf[2];
    if (clock_conf_blk.nominated[source]) {
        frq = (clock_conf_blk.port[source] == SYNCE_STATION_CLOCK_PORT) ? 
              (clock_conf_blk.station_clk_in != SYNCE_MGMT_STATION_CLK_DIS ? CLOCK_FREQ_10MHZ : CLOCK_FREQ_INVALID) : CLOCK_FREQ_125MHZ;
    } else {
        frq = CLOCK_FREQ_INVALID;
    }
    T_D("Set freq, source %d, freq %d", source,frq);
    if (clock_frequency_set(source, frq) != VTSS_OK)    T_D("error returned");
    if (pcb104_synce) {
        if (source == 0) {
            tx_buf[0] = 9; /* Set CPLD Clock input selector: CKIN1 connected to CLKIN_0 or SMA_1 */
            tx_buf[1] = frq == CLOCK_FREQ_125MHZ ? 0x22 : frq == CLOCK_FREQ_10MHZ ? 0x09 : 0x00;
        } else {
            tx_buf[0] = 8 ; /* Set CPLD Clock input selector: CKIN2 connected to CLKIN_1 or SMA_1*/
            tx_buf[1] = frq == CLOCK_FREQ_125MHZ ? 0x21 : frq == CLOCK_FREQ_10MHZ ? 0x09 : 0x00;
        }
        if (vtss_i2c_wr(NULL, 0x75, tx_buf, 2, 50, synce_clk_sel()) != VTSS_RC_OK)    T_D("Clock selector set failed  %u", 1);
    }
}



/****************************************************************************/
/*  SyncE interface                                                         */
/****************************************************************************/
vtss_rc synce_mgmt_nominated_source_set(const uint                           source,
                                        const BOOL                           enable,
                                        const uint                           port_no,
                                        const synce_mgmt_aneg_mode_t         aneg_mode,
                                        const uint                           holdoff_time,
                                        const synce_mgmt_quality_level_t     ssm_overwrite)
{
    uint  i;
    clock_selector_state_t  state = CLOCK_SELECTOR_STATE_FREERUN;
    uint clock_input = 0;
    CRIT_ENTER();    
    T_D("source = %d, port_no = %d, aneg_mode = %d, ssm_overwrite %d, enable:%d, source_port[source][port_no]:%d", source, port_no, aneg_mode, ssm_overwrite, enable, source_port[source][port_no]);
    if (source >= synce_my_nominated_max)        {CRIT_EXIT(); return (SYNCE_RC_INVALID_PARAMETER);}
    if (port_no >= SYNCE_PORT_COUNT)             {CRIT_EXIT(); return (SYNCE_RC_INVALID_PARAMETER);}
    if (enable && !source_port[source][port_no]) {CRIT_EXIT(); return (SYNCE_RC_INVALID_PORT);}
    

    if (!enable) {
        /* This is a de-nomination of the clock source */
        clock_state[source].wtr_state = WTR_OK;
        clock_alarm_state.wtr[source] = FALSE;
        clock_alarm_state.ssm[source] = FALSE;

        if (clock_selector_state_get(&state, &clock_input) != VTSS_OK)    T_D("error returned");
        if (state == CLOCK_SELECTOR_STATE_LOCKED && clock_input == source) {
            T_D("Force to holdover");
            /* node is locked to the nominated source, therefore switch to holdover */
            if (clock_selection_mode_set(CLOCK_SELECTION_MODE_FORCED_HOLDOVER, clock_input) != VTSS_OK)    T_D("error returned");
            
        }
        /* save this de-normination in config block */
        clock_conf_blk.nominated[source] = FALSE;
    } else {
      clock_conf_blk.nominated[source] = TRUE;
    }

    if ((ssm_overwrite != QL_NONE) && (ssm_overwrite != QL_PRC) && (ssm_overwrite != QL_SSUA) &&
        (ssm_overwrite != QL_SSUB) && (ssm_overwrite != QL_EEC2) && (ssm_overwrite != QL_EEC1) && (ssm_overwrite != QL_DNU))                {CRIT_EXIT(); return(SYNCE_RC_INVALID_PARAMETER);}
    if (holdoff_time && (holdoff_time != SYNCE_HOLDOFF_TEST) && ((holdoff_time < SYNCE_HOLDOFF_MIN) || (holdoff_time > SYNCE_HOLDOFF_MAX))) {CRIT_EXIT(); return(SYNCE_RC_INVALID_PARAMETER);}
    /* check if other clock sources is nominated to the same port    */
    for (i=0; i<synce_my_nominated_max && i != STATION_CLOCK_SOURCE_NO; ++i)
      {
        if (clock_conf_blk.nominated[source] && clock_conf_blk.nominated[i] && (clock_conf_blk.port[i] == port_no) && (i != source))                       {CRIT_EXIT(); return(SYNCE_RC_NOM_PORT);}
      }
    if ((aneg_mode == SYNCE_MGMT_ANEG_NONE) && (clock_conf_blk.aneg_mode[source] == SYNCE_MGMT_ANEG_FORCED_SLAVE))     {CRIT_EXIT(); return (SYNCE_RC_INVALID_PARAMETER);}
    
    /* save this normination in config block */
    clock_conf_blk.port[source] = port_no;

    clock_conf_blk.ssm_overwrite[source] = ssm_overwrite;
    clock_conf_blk.aneg_mode[source] = aneg_mode;
    clock_conf_blk.holdoff_time[source] = holdoff_time;
    
    update_clk_in_selector(source);
    if (clock_holdoff_time_set(source, holdoff_time*100) != VTSS_OK)    T_D("error returned");
    
    save_config_blk();
    
    if (!clock_conf_blk.nominated[source])  {CRIT_EXIT(); return (SYNCE_RC_OK);} // Skip updating hardware, since the clock source is not nominated.

    /* calculate and change priority in clock controller */
    set_clock_priority();

    /* check for change in transmission of SSM */
    set_tx_ssm();

    /* do the WTR stuff */
    set_wtr(source);

    /* calculate and change clock source in phy */
    set_clock_source(source);

    /* check and configure master/slave mode of the nominated sources */
    configure_master_slave();

    CRIT_EXIT();

    return (SYNCE_RC_OK);
}


vtss_rc synce_mgmt_nominated_selection_mode_set(synce_mgmt_selection_mode_t       mode,
                                                const uint                        source,
                                                const uint                        wtr_time,
                                                const synce_mgmt_quality_level_t  ssm_holdover,
                                                const synce_mgmt_quality_level_t  ssm_freerun,
                                                const synce_eec_option_t          eec_option)
{
    clock_selection_mode_t  selection_mode;
    clock_selector_state_t  state;
    uint                    clock_input, input, i;
    uint                    clock_type;
    synce_eec_option_t      my_eec_option;
    
    T_D("mode = %d, source = %d, wtr_time = %d", mode, source, wtr_time);
    
    if (source >= synce_my_nominated_max)    return(SYNCE_RC_INVALID_PARAMETER);
    if ((clock_conf_blk.selection_mode == SYNCE_MGMT_SELECTOR_MODE_FORCED_FREE_RUN) && (mode == SYNCE_MGMT_SELECTOR_MODE_FORCED_HOLDOVER))    return(SYNCE_RC_INVALID_PARAMETER);
    if ((mode == SYNCE_MGMT_SELECTOR_MODE_MANUEL_TO_SELECTED) &&
        (clock_conf_blk.selection_mode != SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_NONREVERTIVE) &&
        (clock_conf_blk.selection_mode != SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_REVERTIVE))    return(SYNCE_RC_INVALID_PARAMETER);
    if (wtr_time > 12)    return(SYNCE_RC_INVALID_PARAMETER);
    if ((ssm_holdover != QL_NONE) && (ssm_holdover != QL_PRC) && (ssm_holdover != QL_SSUA) && (ssm_holdover != QL_SSUB) &&
        (ssm_holdover != QL_DNU) && (ssm_holdover != QL_EEC2) && (ssm_holdover != QL_EEC1) && (ssm_holdover != QL_INV))
        return (SYNCE_RC_INVALID_PARAMETER);
    if ((ssm_freerun != QL_NONE) && (ssm_freerun != QL_PRC) && (ssm_freerun != QL_SSUA) && (ssm_freerun != QL_SSUB) &&
            (ssm_freerun != QL_DNU) && (ssm_freerun != QL_EEC2) && (ssm_freerun != QL_EEC1) && (ssm_freerun != QL_INV))
        return (SYNCE_RC_INVALID_PARAMETER);
    if ((eec_option != EEC_OPTION_1) && (eec_option != EEC_OPTION_2))
        return (SYNCE_RC_INVALID_PARAMETER);
    (void)synce_mgmt_station_clock_type_get(&clock_type);
    my_eec_option = eec_option;
    if ((my_eec_option != EEC_OPTION_1) && (clock_type != 0)) my_eec_option = EEC_OPTION_1;

    clock_input = source;
    
    if (clock_selector_state_get(&state, &input) != VTSS_OK)    T_D("error returned");
    
    switch (mode)
    {
        case SYNCE_MGMT_SELECTOR_MODE_MANUEL:                 selection_mode = CLOCK_SELECTION_MODE_MANUEL; break;
        case SYNCE_MGMT_SELECTOR_MODE_MANUEL_TO_SELECTED:
        {
            if (state != SYNCE_MGMT_SELECTOR_STATE_LOCKED)
                return (SYNCE_RC_SELECTION);   /* NOT possible to make Manuel To Selected if not in locked mode */
    
            clock_input = input;
            mode = SYNCE_MGMT_SELECTOR_MODE_MANUEL;
            selection_mode = CLOCK_SELECTION_MODE_MANUEL;
            break;
        }
        case SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_NONREVERTIVE: selection_mode = CLOCK_SELECTION_MODE_AUTOMATIC_NONREVERTIVE; break;
        case SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_REVERTIVE:    selection_mode = CLOCK_SELECTION_MODE_AUTOMATIC_REVERTIVE; break;
        case SYNCE_MGMT_SELECTOR_MODE_FORCED_HOLDOVER:
            if (state == CLOCK_SELECTOR_STATE_FREERUN) {
                selection_mode = CLOCK_SELECTION_MODE_FORCED_FREE_RUN;   /* NOT possible to make forced hold over in free run selector state */
                mode = SYNCE_MGMT_SELECTOR_MODE_FORCED_FREE_RUN;
            }
            else
                selection_mode = CLOCK_SELECTION_MODE_FORCED_HOLDOVER;
            break;
        case SYNCE_MGMT_SELECTOR_MODE_FORCED_FREE_RUN:        selection_mode = CLOCK_SELECTION_MODE_FORCED_FREE_RUN; break;
        default: return(SYNCE_RC_INVALID_PARAMETER);
    }

    T_DG(TRACE_GRP_CLOCK, "clock_selection_mode_set  selection_mode %u  clock_input %u", selection_mode, clock_input);
    if (clock_selection_mode_set(selection_mode, clock_input) != VTSS_OK)    T_D("error returned");

    CRIT_ENTER();
    clock_conf_blk.selection_mode = mode;
    clock_conf_blk.source = clock_input;
    clock_conf_blk.wtr_time = wtr_time * 60;
    clock_conf_blk.ssm_holdover = ssm_holdover;
    clock_conf_blk.ssm_freerun = ssm_freerun;
    clock_conf_blk.eec_option = my_eec_option;
    save_config_blk();
    set_eec_option(my_eec_option);

    for (i=0; i<synce_my_nominated_max; ++i)
    {
        set_wtr(i);                 /* do the WTR stuff */
        set_clock_source(i);    /* calculate and change clock source in phy */
    }

    /* check and configure master/slave mode of the nominated sources */
    configure_master_slave();

    /* check for change in transmission of SSM */
    set_tx_ssm();
    CRIT_EXIT();

    return(SYNCE_RC_OK);
}



vtss_rc synce_mgmt_nominated_priority_set(const uint     source,
                                          const uint     priority)
{
    uint  rc=SYNCE_RC_OK;
    
    T_D("source = %d, priority = %d", source, priority);
    
    if ((source >= synce_my_nominated_max) || (priority >= synce_my_priority_max))    return (SYNCE_RC_INVALID_PARAMETER);

    CRIT_ENTER();
    /* save this priority in config block */
    clock_conf_blk.priority[source] = priority;
    save_config_blk();

    /* calculate and change priority in clock controller */
    set_clock_priority();
    
    /* check and configure master/slave mode of the nominated sources */
    configure_master_slave();

    /* check for change in transmission of SSM */
    set_tx_ssm();
    CRIT_EXIT();

    return(rc);
}



vtss_rc synce_mgmt_ssm_set(const uint      port_no,
                           const BOOL      ssm_enabled)
{
    uint     source;
    
    T_D("port_no = %d, ssm_enabled = %d", port_no, ssm_enabled);
    
    if (port_no >= SYNCE_PORT_COUNT)    return (SYNCE_RC_INVALID_PARAMETER);

    CRIT_ENTER();
    port_conf_blk.ssm_enabled[port_no] = ssm_enabled;
    
    if (ssm_enabled)
        port_state[port_no].ssm_state = port_state[port_no].link ? SSM_OK : (SSM_LINK_DOWN | SSM_NOT_SEEN);
    else
        port_state[port_no].ssm_state = SSM_OK;

    if (port_is_nominated(port_no, &source))
    {
        clock_alarm_state.ssm[source] = (port_state[port_no].ssm_state != SSM_OK) ? TRUE : FALSE;

        /* calculate and change priority in clock controller */
        set_clock_priority();
        
        /* check for change in transmission of SSM */
        set_tx_ssm();

        /* do the WTR stuff */
        set_wtr(source);

        /* calculate and change clock source in phy */
        set_clock_source(source);

        /* check and configure master/slave mode of the nominated sources */
        configure_master_slave();
    }

    save_config_blk();
    CRIT_EXIT();

    return(SYNCE_RC_OK);
}



vtss_rc synce_mgmt_wtr_clear_set(const uint     source)     /* nominated source - range is 0 - synce_my_nominated_max */
{
    if (source >= synce_my_nominated_max)    return(SYNCE_RC_INVALID_PARAMETER);

    CRIT_ENTER();
    if (clock_state[source].wtr_timer != 0)
    {
    /* WTR timer running */
        clock_state[source].wtr_timer = 1;
    }
    CRIT_EXIT();

    return(SYNCE_RC_OK);
}

static u32 freq_khz(synce_mgmt_frequency_t f)
{
    switch(f)
    {
        case SYNCE_MGMT_STATION_CLK_DIS: return 0;
        case SYNCE_MGMT_STATION_CLK_1544_KHZ: return 1544;
        case SYNCE_MGMT_STATION_CLK_2048_KHZ: return 2048;
        case SYNCE_MGMT_STATION_CLK_10_MHZ: return 10000;
        default:      return 0;
    }
}

BOOL clock_out_range_check(const synce_mgmt_frequency_t freq)
{
    BOOL valid [3][SYNCE_MGMT_STATION_CLK_MAX] = {{TRUE,TRUE,TRUE,TRUE}, {TRUE, FALSE, TRUE,TRUE}, {TRUE, FALSE, FALSE, FALSE}};
    uint clock_type;
    (void)clock_station_clock_type_get(&clock_type);
    return valid[clock_type][freq];
}

BOOL clock_in_range_check(const synce_mgmt_frequency_t freq)
{
    BOOL valid [3][SYNCE_MGMT_STATION_CLK_MAX] = {{TRUE,TRUE,TRUE,TRUE}, {TRUE, FALSE, FALSE, TRUE}, {TRUE, FALSE, FALSE, FALSE}};
    uint clock_type;
    (void)clock_station_clock_type_get(&clock_type);
    return valid[clock_type][freq];
}

uint synce_mgmt_station_clock_out_set(const synce_mgmt_frequency_t freq) /* set the station clock output frequency */
{
    vtss_rc rc = SYNCE_RC_OK;
    CRIT_ENTER();
    if (clock_out_range_check(freq)) {
        /* save this parameter in config block */
        clock_conf_blk.station_clk_out = freq;
        save_config_blk();
        /* apply configuration */
        rc = clock_station_clk_out_freq_set(freq_khz(freq));
        if (rc != SYNCE_RC_OK) rc = SYNCE_RC_INVALID_PARAMETER;
        
    } else {
        rc = SYNCE_RC_INVALID_PARAMETER;
    }
    CRIT_EXIT();
    return(rc);
}

uint synce_mgmt_station_clock_out_get(synce_mgmt_frequency_t *const freq)
{
    CRIT_ENTER();
    /* get this parameter from config block */
    *freq = clock_conf_blk.station_clk_out;
    CRIT_EXIT();
    return(SYNCE_RC_OK);
}

uint synce_mgmt_station_clock_in_set(const synce_mgmt_frequency_t freq) /* set the station clock input frequency */
{
    vtss_rc rc = SYNCE_RC_OK;
    uint source;
    CRIT_ENTER();
    if (clock_in_range_check(freq)) {
        /* save this parameter in config block */
        clock_conf_blk.station_clk_in = freq;
        save_config_blk();
        /* apply configuration */
        rc = clock_station_clk_in_freq_set(freq_khz(freq));
        if (rc != SYNCE_RC_OK) rc = SYNCE_RC_INVALID_PARAMETER;
    } else {
        rc = SYNCE_RC_INVALID_PARAMETER;
    }
    for (source = 0; source < synce_my_nominated_max; source++) {
        update_clk_in_selector(source);
    }
    CRIT_EXIT();
    return(rc);
}

uint synce_mgmt_station_clock_in_get(synce_mgmt_frequency_t *const freq)
{
    CRIT_ENTER();
    /* get this parameter from config block */
    *freq = clock_conf_blk.station_clk_in;
    CRIT_EXIT();
    return(SYNCE_RC_OK);
}

uint synce_mgmt_station_clock_type_get(uint *const clock_type)
{
    vtss_rc rc = SYNCE_RC_OK;
    CRIT_ENTER();
    rc = clock_station_clock_type_get(clock_type);
    CRIT_EXIT();
    return (rc);
}


vtss_rc synce_mgmt_clock_state_get(synce_mgmt_selector_state_t  *const selector_state,
                                   uint                         *const clock_input,
                                   synce_mgmt_alarm_state_t     *const alarm_state)     /* clock_input range is 0 - SYNCE_CLOCK_MAX */
{
    clock_selector_state_t  state;

    if (clock_selector_state_get(&state, clock_input) != VTSS_OK)    T_D("error returned");
    
    switch (state)
    {
        case CLOCK_SELECTOR_STATE_LOCKED:   *selector_state = SYNCE_MGMT_SELECTOR_STATE_LOCKED;   break;
        case CLOCK_SELECTOR_STATE_HOLDOVER: *selector_state = SYNCE_MGMT_SELECTOR_STATE_HOLDOVER; break;
        case CLOCK_SELECTOR_STATE_FREERUN:  *selector_state = SYNCE_MGMT_SELECTOR_STATE_FREERUN;  break;
        default: return(SYNCE_RC_SELECTION);
    }
    
    CRIT_ENTER();
    *alarm_state = clock_alarm_state;
    CRIT_EXIT();

    return(SYNCE_RC_OK);
}



vtss_rc synce_mgmt_port_state_get(const uint                     port_no,
                                  synce_mgmt_quality_level_t     *const ssm_rx,
                                  synce_mgmt_quality_level_t     *const ssm_tx,
                                  BOOL                           *const master)
{
    uint ssm_state;
    
    T_D("port_no = %d", port_no);

    if (port_no >= SYNCE_PORT_COUNT)    return (SYNCE_RC_INVALID_PARAMETER);

    CRIT_ENTER();
    ssm_state = port_state[port_no].ssm_state;
    
    if (ssm_state == SSM_OK)
    {
        switch (port_state[port_no].ssm_rx)
        {
            case SSM_QL_PRC:  *ssm_rx = QL_PRC; break;
            case SSM_QL_SSUA: *ssm_rx = QL_SSUA; break;
            case SSM_QL_SSUB: *ssm_rx = QL_SSUB; break;
            case SSM_QL_EEC2: *ssm_rx = QL_EEC2; break;
            case SSM_QL_EEC1: *ssm_rx = QL_EEC1; break;
            case SSM_QL_NONE: *ssm_rx = QL_NONE; break;
            case SSM_QL_DNU:  *ssm_rx = QL_DNU; break;
            default:  *ssm_rx = QL_DNU; break;
        }
    }
    else
    {
        if (ssm_state & SSM_LINK_DOWN)              *ssm_rx = QL_LINK;
        else
        if (ssm_state & (SSM_FAIL | SSM_NOT_SEEN))  *ssm_rx = QL_FAIL;
        else
        if (ssm_state & SSM_INVALID)                *ssm_rx = QL_INV;
    }    
        
    switch (port_state[port_no].ssm_tx)
    {
        case SSM_QL_PRC:  *ssm_tx = QL_PRC; break;
        case SSM_QL_SSUA: *ssm_tx = QL_SSUA; break;
        case SSM_QL_SSUB: *ssm_tx = QL_SSUB; break;
        case SSM_QL_EEC2: *ssm_tx = QL_EEC2; break;
        case SSM_QL_EEC1: *ssm_tx = QL_EEC1; break;
        case SSM_QL_NONE: *ssm_tx = QL_NONE; break;
        case SSM_QL_DNU:  *ssm_tx = QL_DNU; break;
        case SSM_QL_INV:  *ssm_tx = QL_INV; break;
        default:  *ssm_tx = QL_DNU; break;
    }

    *master = port_state[port_no].master;
    CRIT_EXIT();

    return(SYNCE_RC_OK);
}



uint synce_mgmt_clock_config_get(synce_mgmt_clock_conf_blk_t     *const config)
{
    CRIT_ENTER();
    *config = clock_conf_blk;

    if (config->wtr_time != 0)
        config->wtr_time /= 60;
    CRIT_EXIT();
        
    return(SYNCE_RC_OK);
}


uint synce_mgmt_port_config_get(synce_mgmt_port_conf_blk_t     *const config)
{
    CRIT_ENTER();
    *config = port_conf_blk;
    CRIT_EXIT();
    
    return(SYNCE_RC_OK);
}


//uint synce_mgmt_source_port_get(const BOOL   **src_port)
synce_src_port synce_mgmt_source_port_get(void)
{
    return(source_port);
}
                              


uint synce_mgmt_register_get(const uint   reg,
                             uint *const  value)
{
    if (clock_read(reg, value) != VTSS_OK)    T_D("error returned");
    return(SYNCE_RC_OK);
}



uint synce_mgmt_register_set(const uint reg,
                             const uint value)
{
    if (clock_write(reg, value) != VTSS_OK)    T_D("error returned");
    return(SYNCE_RC_OK);
}



/* Initialize module */
vtss_rc synce_init(vtss_init_data_t *data)
{
    int                board_type;
    u32                i;
    vtss_rc            rc;
    vtss_isid_t        isid = data->isid;
    conf_blk_t         save_blk;
#ifdef VTSS_SW_OPTION_PACKET
    packet_rx_filter_t filter;
    void               *filter_id; 
#endif    
    vtss_restart_status_t status;

    /*lint --e{454,456} */

    if (data->cmd == INIT_CMD_INIT)
    {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);
    
    switch (data->cmd)
    {
        case INIT_CMD_INIT:
            T_D("INIT");
            /* initialize critd */
            critd_init(&crit, "SyncE Crit", VTSS_MODULE_ID_SYNCE, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
            
            if (vtss_restart_status_get(NULL, &status) != VTSS_RC_OK)    T_D("vtss_restart_status_get() error returned");
            if (clock_init((status.restart==VTSS_RESTART_COLD)) != VTSS_OK)    T_D("clock_init()  error returned");
            synce_my_nominated_max = clock_my_input_max;
            synce_my_priority_max = clock_my_input_max;

            for (i=0; i<synce_my_nominated_max; ++i)
            {
                if (vtss_board_type() == VTSS_BOARD_SERVAL_PCB106_REF) {
                    phy_clock_config[i].freq = VTSS_PHY_FREQ_25M;
                } else {
                    phy_clock_config[i].freq = VTSS_PHY_FREQ_125M;
                }
                phy_clock_config[i].squelch = VTSS_PHY_CLK_SQUELCH_MAX;
                phy_clock_config[i].src = VTSS_PHY_CLK_DISABLED;
                clock_state[i].holdoff = FALSE;
                clock_state[i].new_locs = FALSE;
                clock_state[i].new_fos = FALSE;
                clock_state[i].wtr_state = WTR_OK;
                clock_state[i].wtr_timer = 0;
                clock_alarm_state.locs[i] = FALSE;
                clock_alarm_state.fos[i] = FALSE;
                clock_alarm_state.ssm[i] = FALSE;
                clock_alarm_state.wtr[i] = FALSE;
            }
            clock_alarm_state.losx = FALSE;
            clock_alarm_state.lol = FALSE;

            for (i=0; i<SYNCE_PORT_COUNT; ++i)
            {
                port_state[i].new_ssm = FALSE;
                port_state[i].new_link = FALSE;
                port_state[i].new_fiber = FALSE;
                port_state[i].ssm_rx = SSM_QL_DNU;
                port_state[i].ssm_tx = SSM_QL_EEC1;
                port_state[i].ssm_count = 0;
                port_state[i].ssm_state = SSM_OK;
                port_state[i].fiber = FALSE;
                port_state[i].master =TRUE;
                port_state[i].link = FALSE;
                port_state[i].first_port_callback = FALSE;
                port_state[i].prefer_timer = 0;;
            }

            board_type = vtss_board_type();
#ifdef VTSS_ARCH_JAGUAR_1
            if (board_type == VTSS_BOARD_JAG_PCB107_REF) {
                pcb107_cpld_init();
            }
#endif
            memset(source_port, 0, sizeof(source_port));
            if ((board_type == VTSS_BOARD_ESTAX_34_ENZO) || (board_type == VTSS_BOARD_ESTAX_34_ENZO_SFP))
            {
#if defined(VTSS_CHIP_SPARX_II_16)
                for (i=12; i<=15; ++i)
                {
                    source_port[0][i] = 1;
                    source_port[1][i] = 1;
                }
#endif
#if defined(VTSS_CHIP_SPARX_II_24) || defined(VTSS_CHIP_E_STAX_34)
                for (i=20; i<=23; ++i)
                {
                    source_port[0][i] = 1;
                    source_port[1][i] = 1;
                }
#endif
            }

            if ((board_type == VTSS_BOARD_LUTON10_REF) || (board_type == VTSS_BOARD_LUTON26_REF))
            {
#if defined(VTSS_CHIP_CARACAL_1)
                for (i=0; i<=9; ++i)
                {
                    source_port[0][i] = 1;
                    source_port[1][i] = 1;
                }
#endif
#if defined(VTSS_CHIP_CARACAL_2) /* This is only valid in case on a two port clock controller */
                for (i=0; i<=11; ++i)
                {
                    source_port[0][i] = 1;
                    source_port[1][i] = 1;
                }
                source_port[0][24] =source_port[1][24] = 1;
                source_port[0][25] =source_port[1][25] = 1;
#endif
#if defined(VTSS_CHIP_SPARX_III_26) /* This is only valid for bringup system */
                for (i=0; i<=13; ++i)
                {
                    source_port[0][i] = 1;
                    source_port[1][i] = 1;
                }
#endif
            }

#ifdef VTSS_ARCH_JAGUAR_1
            if (board_type == VTSS_BOARD_JAG_CU24_REF)
            {
            }
            if (board_type == VTSS_BOARD_JAG_PCB107_REF)
            {
                port_iter_t       pit;
                port_iter_init_local(&pit);
                while (port_iter_getnext(&pit)) {
                    i = pit.iport;
                    if (i < 28) {
                        source_port[0][i] = 1;
                        source_port[1][i] = 1;
                        if (i == 24 || i == 25) {
                            source_port[0][i] = 0;  /* currently we don't support Synce from a Plugin module */
                            source_port[1][i] = 0;  /* currently we don't support Synce from a Plugin module */
                            mux_selector[0] [i] = 0;
                            mux_selector[1] [i] = 1;
                        } else {
                            mux_selector[0] [i] = (i/4)*2 + 2;
                            mux_selector[1] [i] = (i/4)*2 + 3;
                            if (i >= 26) {
                                source_port[0][i] = 0;  /* currently we don't support Synce from a Plugin module */
                                source_port[1][i] = 0;  /* currently we don't support Synce from a Plugin module */
                                mux_selector[0] [i] += 2;
                                mux_selector[1] [i] += 2;
                            }
                        }
                    } else {
                        source_port[0][i] = 0;
                        source_port[1][i] = 0;
                        mux_selector[0] [i] = 20; /* no clock */
                        mux_selector[1] [i] = 20; /* no clock */
                    }
                }
                                      
            }
#endif
            if (board_type == VTSS_BOARD_JAG_SFP24_REF)
            {
                for (i=0; ((i<=23) && (i<SYNCE_PORT_COUNT)); ++i)
                {
                    source_port[0][i] = 1;
                    source_port[1][i] = 1;
                }
            }
            if (board_type == VTSS_BOARD_SERVAL_REF || board_type == VTSS_BOARD_SERVAL_PCB106_REF)
            {
                port_iter_t       pit;
                port_iter_init_local(&pit);
                while (port_iter_getnext(&pit)) {
                    i = pit.iport;
                    if (port_phy(i)) {
                        if (i <=4) {
                            source_port[1][i] = 1;
                        }
                    } else {
                        source_port[0][i] = 1;
                    }
                }
            }
            source_port[STATION_CLOCK_SOURCE_NO][SYNCE_STATION_CLOCK_PORT] = 1;

#if defined(VTSS_FEATURE_SYNCE)
            memset(internal, 0, sizeof(internal));
#if defined(VTSS_CHIP_CARACAL_1)
            for (i=0; i<=7; ++i)
                internal[i] = 1;
#endif
#if defined(VTSS_CHIP_CARACAL_2)
            for (i=0; i<=11; ++i)
                internal[i] = 1;
#endif
#if defined(VTSS_CHIP_SPARX_III_26)  /* This is only valid for bringup system */
            for (i=0; i<=11; ++i)
                internal[i] = 1;
#endif
#endif
            clock_old_selector_state = 0xFF;

            synce_set_conf_to_default(&clock_conf_blk, &port_conf_blk);

#ifdef VTSS_SW_OPTION_VCLI
            synce_cli_init();
#endif

            cyg_thread_create(THREAD_DEFAULT_PRIO, 
                              func_thread, 
                              0, 
                              "SYNCE Function", 
                              func_thread_stack, 
                              sizeof(func_thread_stack),
                              &func_thread_handle,
                              &func_thread_block);

#ifdef VTSS_SW_OPTION_ICFG
        // Initialize ICLI "show running" configuration
        VTSS_RC(synce_icfg_init());
#endif
            break;
        case INIT_CMD_START:
            T_D("START");
            break;
        case INIT_CMD_CONF_DEF:
            T_D("CONF_DEF");
            if (isid == VTSS_ISID_GLOBAL) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
                conf_blk_t *blk;
                ulong      size;
                if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_SYNCE_CONF_TABLE, &size)) != NULL) {
                    synce_set_conf_to_default(&blk->clock, &blk->port);
                    /* Save and close configuration */
                    save_blk = *blk;
                    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_SYNCE_CONF_TABLE);
                    /* Apply configuration */
                    apply_configuration(&save_blk);
                } else {
                    T_W("Error opening conf");
                }
#else
                save_blk.version = 1;
                synce_set_conf_to_default(&save_blk.clock, &save_blk.port);
                apply_configuration(&save_blk);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            }
            break;
        case INIT_CMD_MASTER_UP:
            T_D("MASTER_UP");

            port_iter_t       pit;
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;
                T_W("port %d sel[0] = %d, sel[1] = %d",i, mux_selector[0] [i], mux_selector[1] [i]);
            }


            for (i=0; i<SYNCE_PORT_COUNT; ++i)
                port_state[i].phy = port_phy(i);

            // This will wait until the PHYs are initialized.
            port_phy_wait_until_ready();

            /* hook up on port change */
            if ((rc = port_change_register(VTSS_MODULE_ID_SYNCE, port_change_callback)) != VTSS_OK)    T_D("error returned rc %u", rc);

#ifdef VTSS_SW_OPTION_PACKET
            /* hook up on SSM frame */
            memset(&filter, 0, sizeof(filter));
            filter.modid = VTSS_MODULE_ID_SYNCE;
            filter.match = PACKET_RX_FILTER_MATCH_DMAC | PACKET_RX_FILTER_MATCH_ETYPE;
            filter.cb    = ssm_frame_rx;
            filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
            filter.etype = 0x8809; // slow protocol ethertype
            memcpy(filter.dmac, ssm_dmac, sizeof(filter.dmac));

            if ((rc = packet_rx_filter_register(&filter, &filter_id)) != VTSS_OK)    T_D("error returned rc %u", rc);
#endif
            control_system_reset_register(system_reset);

            cyg_thread_resume(func_thread_handle);
            break;
        case INIT_CMD_MASTER_DOWN:
            T_D("MASTER_DOWN");
            break;
        case INIT_CMD_SWITCH_ADD:
            T_D("SWITCH_ADD");
            break;
        case INIT_CMD_SWITCH_DEL:
            T_D("SWITCH_DEL");
            break;
        case INIT_CMD_SUSPEND_RESUME:
            T_D("SUSPEND_RESUME");
            break;
        default:
            break;
    }

    T_D("exit");
    return 0;
}
/****************************************************************************/
/*                                                                                                                                                    */
/*  End of file.                                                                                                                              */
/*                                                                                                                                                   */
/****************************************************************************/
