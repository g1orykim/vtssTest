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

*/

/* NOTE: FOR DEBUGGING THE lu26 TRANSPARENT CLOCK FORWARDING */
//#define PTP_LOG_TRANSPARENT_ONESTEP_FORWARDING

//#define TIMEOFDAY_TEST
//#define PHY_DATA_DUMP
// use custom wireless delay filter
//#define PTP_DELAYFILTER_WIRELESS 
#include "main.h"
#include "vtss_types.h"
#include "ip2_api.h"

#include "ptp_api.h"           /* Our module API */
#include "ptp.h"               /* Our private definitions */
#include "ptp_local_clock.h"   /* platform part of local_clock if */
#include "vtss_ptp_os.h"
#include "vtss_ptp_wl_delay_filter.h"
#if defined(VTSS_ARCH_SERVAL)
#include "ptp_1pps_serial.h"
#include "ptp_pim_api.h"
#endif
#include "vtss_ptp_synce_api.h"
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
#include "mep_api.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "ptp_icfg.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PTP

// base PTP
#include "vtss_ptp_types.h"
#include "vtss_ptp_api.h"
#include "vtss_ptp_local_clock.h"
#include "vtss_ptp_offset_filter.h"
#include "vtss_ptp_delay_filter.h"
#include "vtss_ptp_packet_callout.h"
#include "vtss_ptp_sys_timer.h"

/* Used APIs */
#include "l2proto_api.h"
#include "critd_api.h"
#include "packet_api.h"
#include "conf_api.h"
#include "misc_api.h"           /* instantiate MAC */
#include "acl_api.h"            /* set up access rule */
#include "interrupt_api.h"      /* interrupt handling */
#include "msg_api.h"            /* message module */
#if !defined(VTSS_ARCH_LUTON28)
#include "vtss_timer_api.h"     /* timer system with better resolution than the eCOS timer, not supported in L28 */
#endif /* VTSS_ARCH_LUTON28 */
#ifdef VTSS_SW_OPTION_PHY
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "phy_api.h"
#endif
#endif
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"           /* topo_isid2mac() */
#endif
//  TOD
#include "vtss_tod_api.h"
#include "vtss_tod_phy_engine.h"
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
#include "vtss_ts_api.h"
#endif

//#if defined(VTSS_ARCH_JAGUAR_1)
//#define VTSS_MISSING_TIMESTAMP_INTERRUPT
//#endif

#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
#include "zl_3034x_api_pdv_api.h"
#endif

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "vtss_phy_ts_api.h"
#include "vtss_tod_mod_man.h"
#include "tod_api.h"
#include "ptp_1pps_sync.h"     /* platform part: 1pps synchronization */
#include "ptp_1pps_closed_loop.h"     /* platform part: 1pps closed loop delay measurement */
#else
/*
 * Dummy timestamp definitions.
 */
typedef enum {
    VTSS_PTP_TS_PTS,       /* port has PHY timestamp feature */
    VTSS_PTP_TS_JTS,       /* port has Jaguar timestamp feature  */
    VTSS_PTP_TS_CTS,       /* port has Caracal (L26) timestamp feature  */
    VTSS_PTP_TS_NONE,      /* port has no timestamp feature  */
} vtss_ptp_timestamp_feature_t;

typedef struct vtss_tod_ts_phy_topo_t  {
    vtss_ptp_timestamp_feature_t ts_feature;    /* indicates pr port which timestamp feature is available */
    u16 channel_id;                             /* identifies the channel id in the PHY
                                                   (needed to access the timestamping feature) */
    BOOL port_shared;                           /* port shares engine with another port */
    vtss_port_no_t shared_port_no;              /* the port this engine is shared with. */
    BOOL port_ts_in_sync;                       /* false if the port has PHY timestamper, 
                                                    and timestamper is not in sync with master timer, otherwise TRUE */
} vtss_tod_ts_phy_topo_t ;
#endif

#ifdef VTSS_SW_OPTION_VCLI
#include "ptp_cli.h"
#endif

#include <pkgconf/kernel.h>
#include <cyg/kernel/instrmnt.h>
#include <cyg/kernel/kapi.h>

#include <sys/types.h>
#include <sys/socket.h>










//#define PTP_OFFSET_FILTER_CUSTOM
//#define PTP_DELAYFILTER_CUSTOM

#define API_INST_DEFAULT PHY_INST 

static const u16 ptp_ether_type = 0x88f7;


static vtss_ptp_offset_filter_handle_t servo[PTP_CLOCK_PORTS];
static vtss_ptp_delay_filter_handle_t delay_filt[PTP_CLOCK_PORTS];

/**
 * \brief PTP Clock Config Data Set structure
 */
typedef struct ptp_instance_config_t {
    ptp_init_clock_ds_t         clock_init;
    ptp_set_clock_ds_t          clock_ds;
    ptp_clock_timeproperties_ds_t time_prop;
    ptp_set_port_ds_t               port_config [PTP_CLOCK_PORTS];
    vtss_ptp_default_filter_config_t default_of; /* default offset filter config */
    vtss_ptp_default_servo_config_t default_se; /* default servo config */
    vtss_ptp_default_delay_filter_config_t default_df; /* default delay filter  config*/
    vtss_ptp_unicast_slave_config_t unicast_slave[MAX_UNICAST_MASTERS_PR_SLAVE]; /* Unicast slave config, i.e. requested master(s) */
    ptp_clock_slave_cfg_t       slave_cfg;
} ptp_instance_config_t;


typedef struct ptp_config_t {
    i8 version;             /* configuration version, to be changed if this structure is changed */
    ptp_instance_config_t conf [PTP_CLOCK_INSTANCES];
    init_synce_t init_synce;  /* syncE VCXO calibration value */
    vtss_ptp_ext_clock_mode_t init_ext_clock_mode; /* luton26/Jaguar/Serval(Synce) external clock mode */
#if defined (VTSS_ARCH_SERVAL)
    vtss_ptp_rs422_conf_t init_ext_clock_mode_rs422; /* Serval(RS422) external clock mode */
#endif // (VTSS_ARCH_SERVAL)
} ptp_config_t;

static ptp_config_t config_data ;
/* indicates if a transparent clock exists (only one transparent closk may exist in a node)*/
static bool transparent_clock_exists = FALSE;
#if defined (VTSS_ARCH_SERVAL)
/* indicates if the wireless mode is enabled*/
typedef struct {
    BOOL                mode_enabled; /* true is fireless mode enabled */
    BOOL                remote_pre;   /* remote pre_notification => true, remote delay => false */
    BOOL                local_pre;    /* local pre_notification => true, local delay => false */
    vtss_timeinterval_t remote_delay;
    vtss_timeinterval_t local_delay;
    int                 pre_cnt;
} ptp_wireless_status_t;
static ptp_wireless_status_t wireless_status = {FALSE, FALSE, FALSE, 0LL,0LL};

#endif // (VTSS_ARCH_SERVAL)



#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
	static vtss_tod_internal_tc_mode_t phy_ts_mode = VTSS_TOD_INTERNAL_TC_MODE_30BIT;
#endif


/* IPV4 protocol definitions */
#include "network.h"
#define PTP_EVENT_PORT 319
#define PTP_GENERAL_PORT 320
#define IP_HEADER_SIZE 20
#define UDP_HEADER_SIZE 8


static const u16 ip_ether_type = 0x0800;

/* Extra space used in the onestep transmission buffer */
typedef struct {
    u32             corr_offset;
    u32             hw_time;        /* hw counter corresponding to the TX timestamp */
    u32             tx_time;        /* he counter corresponding to the pre_done correction field update time */
    vtss_timeinterval_t    sw_egress_delay;/* set to the calculated min sw egress delay in the tx_event_onestep function */
} onestep_extra_t;

static const mac_addr_t ptp_eth_mcast_adr[2] = {{0x01, 0x1b, 0x19, 0x00, 0x00, 0x00} , {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E}};

static const mac_addr_t ptp_ip_mcast_adr[2] = {{0x01, 0x00, 0x5e, 0, 1, 129}, {0x01, 0x00, 0x5e, 0, 0, 107}};


/* ================================================================= *
 *  Trace definitions
 * ================================================================= */


#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "ptp",
    .descr     = "Precision Time Protocol"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default (PTP core)",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_TIMER] = {
        .name      = "timer",
        .descr     = "PTP Internal timer",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_PACK_UNPACK] = {
        .name      = "pack",
        .descr     = "PTP Pack/Unpack functions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_MASTER] = {
        .name      = "master",
        .descr     = "PTP Master Clock Trace",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_SLAVE] = {
        .name      = "slave",
        .descr     = "PTP Slave Clock Trace",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_STATE] = {
        .name      = "state",
        .descr     = "PTP Port State Trace",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_FILTER] = {
        .name      = "filter",
        .descr     = "PTP filter module Trace",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_PEER_DELAY] = {
        .name      = "peer_delay",
        .descr     = "PTP peer delay module Trace",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_TC] = {
        .name      = "tc",
        .descr     = "PTP Transparent clock module Trace",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_BASE_BMCA] = {
        .name      = "bmca",
        .descr     = "PTP BMCA module Trace",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
   [VTSS_TRACE_GRP_SERVO] = {
        .name      = "servo",
        .descr     = "PTP Local Clock Servo",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_INTERFACE] = {
        .name      = "interface",
        .descr     = "PTP Core interfaces",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_CLOCK] = {
        .name      = "clock",
        .descr     = "PTP Local clock functions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_1_PPS] = {
        .name      = "1_pps",
        .descr     = "PTP 1 pps input functions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_EGR_LAT] = {
        .name      = "egr_lat",
        .descr     = "Jaguar 1 step egress latency",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PHY_TS] = {
        .name      = "phy_ts",
        .descr     = "PHY Timestamp feature",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_REM_PHY] = {
        .name      = "rem_phy",
        .descr     = "Remote PHY Timestamp feature",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_SER] = {
        .name      = "serial_1pps",
        .descr     = "PTP Serial 1pps interface feature",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_PIM] = {
        .name      = "pim_proto",
        .descr     = "PTP PIM protocol for 1pps and Modem modulation",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PTP_ICLI] = {
        .name      = "icli",
        .descr     = "PTP ICLI log function",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PHY_1PPS] = {
        .name      = "phy_1pps",
        .descr     = "PTP PHY 1pps Synchronization",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    
    
    
};
#endif /* VTSS_TRACE_ENABLED */

/* Thread variables */
static cyg_handle_t ptp_thread_handle;
static cyg_thread   ptp_thread_block;
static char         ptp_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static cyg_handle_t ptp_background_thread_handle;
static cyg_thread   ptp_background_thread_block;
static char         ptp_background_thread_stack[THREAD_DEFAULT_STACK_SIZE];

/* PTP global data */
/*lint -esym(457, ptp_global_control_flags) */
static cyg_flag_t ptp_global_control_flags;   /* PTP thread control */
static struct {
    BOOL ready;                 /* PTP Initited  */
    critd_t coremutex;          /* PTP core library serialization */
    mac_addr_t sysmac;          /* Switch system MAC address */
    struct in_addr my_ip [PTP_CLOCK_INSTANCES]; /* one ip pr PTP instance */
    ptp_clock_handle_t ptpi[PTP_CLOCK_INSTANCES];    /* PTP instance handle */
} ptp_global;

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static vtss_phy_ts_fifo_sig_mask_t my_sig_mask = VTSS_PHY_TS_FIFO_SIG_MSG_TYPE | VTSS_PHY_TS_FIFO_SIG_DOMAIN_NUM | 
        VTSS_PHY_TS_FIFO_SIG_SOURCE_PORT_ID | VTSS_PHY_TS_FIFO_SIG_SEQ_ID;    /*  PHY timestamp tx fifo signature */
#endif

static cyg_flag_t bg_thread_flags; /* PTP background thread control */

static const int header_size_ip4 = sizeof(mac_addr_t) + sizeof(ptp_global.sysmac) + sizeof(ptp_ether_type) + IP_HEADER_SIZE + UDP_HEADER_SIZE;

static vtss_rc ptp_phy_ts_update(int inst);

static size_t pack_ip_udp_header(uchar * buf, u32 dest_ip, u32 src_ip, u16 port, u16 len);

/* port ingress and egress data
 * this datastructure is used to hold the port data which depends on the HW
 * architecture.
 */
static struct {
    vtss_tod_ts_phy_topo_t topo;                /* Phy timestamp topology info */
    u32 delay_cnt;                              /* peer delay value */
    u32 asymmetry_cnt;                          /* link asymmetry */
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1)
    u32 ingress_cnt;                            /* ingress latency value */
    u32 egress_cnt;                             /* egress latency value */
#endif
    BOOL link_state;                            /* false if link is down, otherwise TRUE */
    vtss_packet_filter_t vlan_forward[PTP_CLOCK_INSTANCES];     /* packet forwarding filter obtained from vtss_packet_port_filter_get */
    vtss_etype_t         vlan_tpid[PTP_CLOCK_INSTANCES];     /* packet forwarding filter tpid obtained from vtss_packet_port_filter_get */
    BOOL         vlan_forw[PTP_CLOCK_INSTANCES]; /* packet forwarding filter tpid obtained from vtss_packet_port_filter_get */
    BOOL      internal_port_exists;
} port_data [PTP_CLOCK_PORTS];

/*
 * If a TC exists, that forwards all packets in HW, then my_tc_encapsulation is set to the TC encapsulation type
 * If this type of TC exists, and a SlaveOnly instance also exists, this slave is used for syntonization of the TC,
 * and the packets forwarded to the CPU must also be forwarded, i.e. the ACL rules for this slaveOnly instance depends
 * on the TC configuration.
 */
#if defined(VTSS_FEATURE_ACL_V2)
static UInteger8 my_tc_encapsulation = PTP_PROTOCOL_MAX_TYPE;   /* indicates that no forwarding in the SlaveOnly instance is needed */
#endif /*defined(VTSS_FEATURE_ACL_V2)*/
static int my_tc_instance = -1;




#define DEFAULT_1PPS_LATENCY        (12LL<<16)

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static void
ptp_in_sync_callback(vtss_port_no_t port_no,
                     BOOL in_sync);
#endif
/* forward declaration */
static void ptp_port_filter_change_callback(int instance, vtss_port_no_t port_no, BOOL forward);

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static void port_data_conf_set(vtss_port_no_t port_no, BOOL link_up)
{
    vtss_port_no_t shared_port;
    if (link_up) {
        /* is this port a PHY TS port ? */
                                 
        tod_ts_phy_topo_get(port_no, &port_data[port_no].topo);

        T_DG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, ts_feature %d, channel %d, shared_port %d, in_sync %d", port_no, 
             port_data[port_no].topo.ts_feature,
             port_data[port_no].topo.channel_id,
             port_data[port_no].topo.shared_port_no,
             port_data[port_no].topo.port_ts_in_sync);
        if (port_data[port_no].topo.port_shared) {
            shared_port = port_data[port_no].topo.shared_port_no;
            tod_ts_phy_topo_get(shared_port, &port_data[shared_port].topo);
            T_DG(VTSS_TRACE_GRP_PHY_TS, "Shared Port_no = %d, ts_feature %d, channel %d, shared_port %d, in_sync %d", shared_port, 
             port_data[shared_port].topo.ts_feature,
             port_data[shared_port].topo.channel_id,
             port_data[shared_port].topo.shared_port_no,
             port_data[shared_port].topo.port_ts_in_sync);
        }
    }
}
#endif

static void port_data_initialize(void)
{
    int i,j;
    port_iter_t       pit;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    /* add in-sync callback function */
    PTP_RC(vtss_module_man_tx_timestamp_in_sync_cb_add(ptp_in_sync_callback));
#endif
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        port_data[i].topo.port_ts_in_sync = TRUE;
#if defined(VTSS_ARCH_JAGUAR_1)
        port_data[i].topo.ts_feature = VTSS_PTP_TS_JTS;
#elif  defined(VTSS_ARCH_LUTON26)
        port_data[i].topo.ts_feature = VTSS_PTP_TS_CTS;
#else
        port_data[i].topo.ts_feature = VTSS_PTP_TS_NONE;
#endif
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        tod_ts_phy_topo_get(i, &port_data[i].topo);
			 
        T_IG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, ts_feature(gen) %d(%d), channel %d, shared_port %d, in_sync %d", i, 
             port_data[i].topo.ts_feature,
             port_data[i].topo.ts_gen,
             port_data[i].topo.channel_id,
             port_data[i].topo.shared_port_no,
             port_data[i].topo.port_ts_in_sync);
#endif
		
        port_data[i].delay_cnt = 0;
        port_data[i].asymmetry_cnt = 0;
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1)
        port_data[i].ingress_cnt = 0;
        port_data[i].egress_cnt = 0;
#endif
        port_data[i].link_state = FALSE;
        for (j = 0; j < PTP_CLOCK_INSTANCES; j++) {
            port_data[i].vlan_forward[j] =  VTSS_PACKET_FILTER_DISCARD;
            port_data[i].vlan_tpid[j] = 0;
            port_data[i].vlan_forw[j] = FALSE;
        }
        port_data[i].internal_port_exists = FALSE;
    }
}

/* latency observed in onestep tx timestamping */
static observed_egr_lat_t observed_egr_lat = {0,0,0,0};


/*
 * Forward defs
 */
static void
ptp_port_link_state_initial(int instance);
static vtss_rc ptp_ace_update(int i);
static void ext_clock_out_set(const vtss_ptp_ext_clock_mode_t *mode);

#if defined(VTSS_ARCH_SERVAL)
static void ext_clock_rs422_conf_set(const vtss_ptp_rs422_conf_t *mode);
#endif

/*
 * Conversion between l2port numbers and PTP port numbers.
 */
/* Convert from l2port (0-based) to PTP API port (1-based) */
static uint ptp_l2port_to_api(l2_port_no_t l2port)
{
    return (l2port + 1);
}

/* Compute the easy part of the checksum on a range of bytes. */

static u_int32_t checksum (unsigned char *buf,
                           unsigned nbytes,
                           u_int32_t sum)
{
    unsigned i;


    /* Checksum all the pairs of bytes first... */
    for (i = 0; i < (nbytes & ~1U); i += 2) {
        sum += (u_int16_t) ntohs(*((u_int16_t *)(buf + i)));
        /* Add carry. */
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }

    /* If there's a single byte left over, checksum it, too.   Network
       byte order is big-endian, so the remaining byte is the high byte. */
    if (i < nbytes) {
        sum += buf [i] << 8;
        /* Add carry. */
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    return sum;
}

/* Finish computing the checksum, and then put it into network byte order. */

static u_int32_t wrapsum (u_int32_t sum)
{
    sum = ~sum & 0xFFFF;
    return htons(sum);
}


/* Convert from pTP API port (1-based) to l2port (0-based) */
static l2_port_no_t ptp_api_to_l2port(uint port)
{
    return (port - 1);
}

/* Convert from l2port (0-based) to pTP API port (1-based) */
static uint l2port_to_ptp_api(l2_port_no_t port)
{
    return (port + 1);
}

#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
static vtss_ptp_sys_timer_t oam_timer;
#define OAM_TIMER_INIT_VALUE 128
/* 
 * PTP OAM slave timer
 */
/*lint -esym(459, ptp_oam_slave_timer) */
static void ptp_oam_slave_timer(vtss_timer_handle_t timer, void *m)
{
    vtss_mep_mgmt_dm_timestamp_t far_to_near;
    vtss_mep_mgmt_dm_timestamp_t near_to_far;           
    vtss_ptp_timestamps_t ts;
    int instance = 0;
    vtss_rc rc;
    ptp_clock_handle_t ptp = m;
    vtss_mep_mgmt_dm_conf_t   config;
    vtss_mep_mgmt_conf_t  conf;
    u8                    mac[VTSS_MEP_MAC_LENGTH];
    static i32 oam_timer_value = OAM_TIMER_INIT_VALUE;
    u32 mep_inst = 0;

    T_N("ptp %p", ptp);
    if (config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM) {
        if (vtss_non_ptp_slave_check_port(ptp) == NULL) {
            /* check if OAM instance is active */
            oam_timer_value = OAM_TIMER_INIT_VALUE;
            if (mep_mgmt_conf_get(mep_inst, mac, NULL, NULL, &conf) != VTSS_RC_OK) {
                T_W("MISSING MEP instance");
            } else {
                if (conf.enable && conf.mode == VTSS_MEP_MGMT_MEP && conf.direction == VTSS_MEP_MGMT_DOWN && conf.domain == VTSS_MEP_MGMT_PORT && conf.voe) {
                    rc = mep_mgmt_dm_conf_get(mep_inst, &config);
                    if (rc == MEP_RC_INVALID_PARAMETER) {
                        T_W("MISSING OAM monitor");
                    } else {
                        if (!config.enable || config.calcway != VTSS_MEP_MGMT_FLOW || config.tunit != VTSS_MEP_MGMT_NS || !config.syncronized) {
                            T_I("OAM monitor not set up correctly");
                            config.enable = TRUE;
                            config.calcway = VTSS_MEP_MGMT_FLOW;
                            config.tunit = VTSS_MEP_MGMT_NS;
                            config.syncronized = TRUE;
                            rc = mep_mgmt_dm_conf_set(mep_inst, &config);
                            if (rc != VTSS_RC_OK) {
                                T_W("OAM monitor configuration failed");
                            }
                        } else {
                            oam_timer_value = (config.interval*128)/100;
                            T_N("OAM timer rate %d msec pr packet, timer valus %d", config.interval*10, oam_timer_value);
                            
                            (void) vtss_ptp_port_ena(ptp_global.ptpi[instance], l2port_to_ptp_api(conf.port));
                            ptp_port_filter_change_callback(instance, conf.port, TRUE);  /* ignore VLAN discarding function */
                            ptp_port_link_state_initial(instance);
                            vtss_non_ptp_slave_init(ptp_global.ptpi[instance], l2port_to_ptp_api(conf.port));
                        }
                    }
                } else {
                    T_W("Wrong mep configuration");
                }
            }
        } else {
            if (wireless_status.remote_pre || wireless_status.local_pre) {
                T_I("Inst %d, remote_pre %d, local_pre %d", instance, wireless_status.remote_pre, wireless_status.local_pre);
                    if (++wireless_status.pre_cnt > 3) {
                        wireless_status.remote_pre = FALSE;
                        wireless_status.local_pre = FALSE;
                        wireless_status.pre_cnt = 0;
                    }
            } else {
                wireless_status.pre_cnt = 0;
                rc = mep_mgmt_dm_timestamp_get(instance, &far_to_near, &near_to_far);
                if (rc == VTSS_RC_OK) {
                    T_D("Inst %d, far_to_near, tx_time  %12d:%9d, rx_time %12d:%9d", instance, far_to_near.tx_time.seconds, 
                        far_to_near.tx_time.nanoseconds, far_to_near.rx_time.seconds, far_to_near.rx_time.nanoseconds);
                    ts.tx_ts.sec_msb = far_to_near.tx_time.sec_msb; 
                    ts.tx_ts.seconds = far_to_near.tx_time.seconds; 
                    ts.tx_ts.nanoseconds = far_to_near.tx_time.nanoseconds; 
                    ts.rx_ts.sec_msb = far_to_near.rx_time.sec_msb; 
                    ts.rx_ts.seconds = far_to_near.rx_time.seconds; 
                    ts.rx_ts.nanoseconds = far_to_near.rx_time.nanoseconds; 
                    ts.corr = wireless_status.remote_delay;
                    vtss_non_ptp_slave_t1_t2_rx(ptp, &ts);
                    
                    T_D("Inst %d, near_to_far, tx_time  %12d:%9d, rx_time %12d:%9d", instance, near_to_far.tx_time.seconds, 
                        near_to_far.tx_time.nanoseconds, near_to_far.rx_time.seconds, near_to_far.rx_time.nanoseconds);
                    ts.tx_ts.sec_msb = near_to_far.tx_time.sec_msb; 
                    ts.tx_ts.seconds = near_to_far.tx_time.seconds; 
                    ts.tx_ts.nanoseconds = near_to_far.tx_time.nanoseconds; 
                    ts.rx_ts.sec_msb = near_to_far.rx_time.sec_msb; 
                    ts.rx_ts.seconds = near_to_far.rx_time.seconds; 
                    ts.rx_ts.nanoseconds = near_to_far.rx_time.nanoseconds; 
                    ts.corr = wireless_status.local_delay;
                    vtss_non_ptp_slave_t3_t4_rx(ptp, &ts);
                } else if (rc == MEP_RC_NO_TIMESTAMP_DATA) {
                    T_I("Inst %d, no valid timestamp data", instance);
                } else {
                    T_I("Inst %d, OAM monitoring not active", instance);
                }
                if (rc != VTSS_RC_OK) {
                    vtss_non_ptp_slave_timeout_rx(ptp);
                }
            }
        }
        vtss_ptp_timer_start(&oam_timer, oam_timer_value, FALSE);
    }
}
#endif /* defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP) */


static vtss_ptp_sys_timer_t one_pps_sync_timer;
#define ONE_PPS_TIMER_INIT_VALUE 128
/* 
 * PTP one_pps_synchronization timer function
 */
/*lint -esym(459, ptp_one_pps_slave_timer) */
static void ptp_one_pps_slave_timer(vtss_timer_handle_t timer, void *m)
{
    ptp_clock_handle_t ptp = m;
    vtss_non_ptp_slave_timeout_rx(ptp);
    if (0 == vtss_non_ptp_slave_check_port_no(ptp)) {
        vtss_ptp_timer_stop(&one_pps_sync_timer);
    }
}

/* 
 * PTP one_pps_synchronization timer function
 */
void ptp_1pps_ptp_slave_t1_t2_rx(vtss_port_no_t port_no, vtss_ptp_timestamps_t *ts)
{
    ptp_clock_handle_t ptp = ptp_global.ptpi[0];
    u32 slave_port = vtss_non_ptp_slave_check_port_no(ptp);
    int instance = 0;
    char str0 [40];

    if (slave_port == 0) {
        if (config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_1PPS) {
            /* no active slave exists: make this port slave */
            (void) vtss_ptp_port_ena(ptp_global.ptpi[instance], l2port_to_ptp_api(port_no));
            ptp_port_filter_change_callback(instance, port_no, TRUE);  /* ignore VLAN discarding function */
            ptp_port_link_state_initial(instance);
            vtss_non_ptp_slave_init(ptp_global.ptpi[instance], l2port_to_ptp_api(port_no));
            /* start timeout monitoring */
            vtss_ptp_timer_start(&one_pps_sync_timer, ONE_PPS_TIMER_INIT_VALUE, TRUE);
        } else {
            T_W("Port %d, Rx timestamp %s", iport2uport(port_no), TimeStampToString(&ts->rx_ts, str0));
            T_W("         Tx timestamp %s", TimeStampToString(&ts->tx_ts, str0));
        }
    } else {
        /* an active slave port exists, check if ports match */
        if (iport2uport(port_no) == slave_port) {
            /* this is the slave port */
            vtss_non_ptp_slave_t1_t2_rx(ptp, ts);
        } else {
            /* this is not the slaveport, therefore the timestamp is ignored */
        }
        
    }

}

static void
poll_port_filter(int instance)
{
    vtss_packet_port_info_t   info;
    u16 port;
    vtss_packet_port_filter_t filter[VTSS_PORT_ARRAY_SIZE];
    static BOOL first_time[PTP_CLOCK_INSTANCES] = {TRUE, TRUE, TRUE, TRUE};
    port_iter_t       pit;

    vtss_vid_t vid  = config_data.conf[instance].clock_init.configured_vid;
    PTP_RC(vtss_packet_port_info_init(&info));
    info.vid = vid; /* Tx VLAN ID */
    PTP_RC(vtss_packet_port_filter_get(NULL, &info, filter));
    /* dump filter info */
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        port = pit.iport;
        T_I("Inst %d, Port %d, VID %d, filter %d, tpid %x", instance, port, vid, filter[port].filter, filter[port].tpid);
        if (first_time[instance]) {
            ptp_port_filter_change_callback(instance, port, 
                                            filter[port].filter != VTSS_PACKET_FILTER_DISCARD);
        } else {
            if (filter[port].filter != port_data[port].vlan_forward[instance] ||
                    filter[port].tpid != port_data[port].vlan_tpid[instance]) {
                port_data[port].vlan_forward[instance] = filter[port].filter;
                port_data[port].vlan_tpid[instance] = filter[port].tpid;
                ptp_port_filter_change_callback(instance, port, 
                                                filter[port].filter != VTSS_PACKET_FILTER_DISCARD);
            }
        }
    }
    first_time[instance] = FALSE;
}

static void
initial_port_filter_get(int instance)
{
    vtss_packet_port_info_t   info;
    u16 port;
    vtss_packet_port_filter_t filter[VTSS_PORT_ARRAY_SIZE];
    port_iter_t       pit;

    vtss_vid_t vid  = config_data.conf[instance].clock_init.configured_vid;
    PTP_RC(vtss_packet_port_info_init(&info));
    info.vid = vid; /* Tx VLAN ID */
    PTP_RC(vtss_packet_port_filter_get(NULL, &info, filter));
    /* dump filter info */
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        port = pit.iport;
        T_I("Inst %d, Port %d, VID %d, filter %d, tpid %x", instance, port, vid, filter[port].filter, filter[port].tpid);
        port_data[port].vlan_forward[instance] = filter[port].filter;
        port_data[port].vlan_tpid[instance] = filter[port].tpid;
    }
}

static size_t
pack_ethernet_header(uchar * buf, const mac_addr_t dest, const mac_addr_t src, u16 ether_type, BOOL tag_enable, vtss_vid_t vid, u16 port, int instance)
{
    int i = 0;
    /* DA MAC */
    memcpy(&buf[i], dest, sizeof(mac_addr_t));
    i += sizeof(mac_addr_t);
    /* SA MAC */
    memcpy(&buf[i], src, sizeof(mac_addr_t));
    i += sizeof(mac_addr_t);

    //if (tag_enable && (port_data[ptp_api_to_l2port(port)].vlan_forward[instance] == VTSS_PACKET_FILTER_TAGGED)) {
    //    vtss_tod_pack16(port_data[ptp_api_to_l2port(port)].vlan_tpid[instance], buf+i);
    //    i += 2;
    //    //vtss_tod_pack16(ptp_global.my_vid, buf+i);
    //    vtss_tod_pack16(vid, buf+i);
    //    i += 2;
    //}
    /* ethertype  */
    buf[i++] = (ether_type>>8) & 0xff;
    buf[i++] = ether_type & 0xff;

    return i;
}

static size_t
unpack_ethernet_header(const uchar *const buf, mac_addr_t src, u16 *ether_type)
{

    int i;
    i = sizeof(mac_addr_t);
    /* SA MAC */
    memcpy(src, &buf[i], sizeof(mac_addr_t));
    i += sizeof(mac_addr_t);
    /* ethertype  */
    *ether_type = (buf[i]<<8) + buf[i+1];
    i += 2;
    if (*ether_type != ptp_ether_type && *ether_type != ip_ether_type) {
        T_W("skipping tag  %d", *ether_type);
        i+=2;
        *ether_type = (buf[i]<<8) + buf[i+1];
        i += 2;
    }
    return i;
}

void
vtss_1588_update_encap_header(uchar *buf, BOOL uni, BOOL event, u16 len, int instance)
{

    int i;
    u16 ether_type;
    mac_addr_t src;
    u32 dest_ip;
    u32 src_ip; 
    u16 port;
    i = sizeof(mac_addr_t);
    if (uni) {  /* swap addresses */
        /* SA MAC */
        memcpy(src, &buf[i], sizeof(mac_addr_t));
        memcpy(&buf[i], &buf[0], sizeof(mac_addr_t));
        memcpy(&buf[0], src, sizeof(mac_addr_t));
    } else {  /* update src addres */
        memcpy(&buf[i], ptp_global.sysmac, sizeof(mac_addr_t));
    }
    
    i += sizeof(mac_addr_t);
    /* ethertype  */
    ether_type = (buf[i]<<8) + buf[i+1];
    i += 2;
    if (ether_type != ptp_ether_type && ether_type != ip_ether_type) {
        T_W("skipping tag  %d", ether_type);
        i+=2;
        ether_type = (buf[i]<<8) + buf[i+1];
        i += 2;
    }
    if (ether_type == ip_ether_type) {
        if (uni) {  /* SWAP IP adresses */
            dest_ip = vtss_tod_unpack32(buf+i+12);
            src_ip = vtss_tod_unpack32(buf+i+16);
        } else {
            dest_ip = vtss_tod_unpack32(buf+i+16);
            src_ip = ptp_global.my_ip[instance].s_addr;
        }
        port = event ? PTP_EVENT_PORT : PTP_GENERAL_PORT;
        (void)pack_ip_udp_header(buf+i, dest_ip, src_ip, port, len);
    }
}

static void update_ptp_ip_address(int instance, vtss_vid_t vid)
{
    vtss_if_status_t status;
    int port;
    struct in_addr ip;
    ip.s_addr = 0;
    BOOL err = FALSE;
    vtss_if_id_vlan_t conf_vid;
    port_iter_t       pit;
    conf_vid = vid;
    if (vtss_ip2_if_status_get_first(VTSS_IF_STATUS_TYPE_IPV4, conf_vid, &status) == VTSS_OK) {
        if(status.type != VTSS_IF_STATUS_TYPE_IPV4) {
            err = TRUE;
        } else {
            ip.s_addr = status.u.ipv4.net.address;
            err = FALSE;
        }
        
        T_D("my IP address= %x", ip.s_addr);
        if (!err) {
            if (ptp_global.my_ip[instance].s_addr != ip.s_addr) {
                ptp_global.my_ip[instance].s_addr = ip.s_addr;
                if (ptp_ace_update(instance) != VTSS_OK) T_EG(0, "ACE update error");
                
            }
        } else T_I("No IPV4 address defined for VID %d", conf_vid);
    } else T_I("Failed to get my IP address for VID %d", conf_vid);
    /* we also must reinitialize the ports, if the IP address has changed */
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        port = pit.iport;
        ptp_port_filter_change_callback(instance, port, 
                                        port_data[port].vlan_forw[instance]);
    }
    T_I("instance: %d, IP address= %x, err %d", instance, ip.s_addr, err);
}

static void notify_ptp_ip_address(vtss_if_id_vlan_t if_id)
{
    int i;
        
    T_D("if_id: %d", if_id);
    for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
        PTP_CORE_LOCK();
        if (config_data.conf[i].clock_init.deviceType != PTP_DEVICE_NONE) {
            if (config_data.conf[i].clock_init.configured_vid == if_id) {
                update_ptp_ip_address(i, if_id);
            }
        }
        PTP_CORE_UNLOCK();
    }
}


/*
 * buf = address of data buffer
 * 
 */
static void
update_udp_checksum(uchar * buf)
{
    u16 ether_type;
    uchar * ip_buf;
    int ip_header_len;
    uchar * udp_buf;
    u16 uh_sum;
    u16 len;
    /* check if IPv4 encapsulation */
    int i = 2*sizeof(mac_addr_t);
    /* ethertype  */
    ether_type = (buf[i]<<8) + buf[i+1];
    i += 2;
    if (ether_type != ptp_ether_type && ether_type != ip_ether_type) {
        T_N("skipping tag  %x", ether_type);
        i+=2;
        ether_type = (buf[i]<<8) + buf[i+1];
        i += 2;
    }
    if (ether_type == ip_ether_type) {
        ip_buf = buf+i;
        ip_header_len = (ip_buf[0] & 0x0f) << 2;
        if (ip_buf[9] == IPPROTO_UDP) { // check if UDP protocol
            i += ip_header_len; /* i = start of UDP protocol header */
            udp_buf = buf+i;
        
            /* if checksum != 0 then recalculate the checksum */
            uh_sum = vtss_tod_unpack16(udp_buf+6);
            if (uh_sum != 0) {
                len = vtss_tod_unpack16(udp_buf+4);
        
                /* Compute UDP checksums, including the ``pseudo-header'', the UDP
                    header and the data. */
                vtss_tod_pack16(0, udp_buf+6); /* clear checksum */
                uh_sum =
                    wrapsum (checksum (udp_buf, UDP_HEADER_SIZE,
                                       checksum (udp_buf+UDP_HEADER_SIZE, len-8,
                                                 checksum (ip_buf + 12, 2 * sizeof(u32),
                                                         IPPROTO_UDP +
                                                         (u_int32_t) len))));
                if (uh_sum == 0) uh_sum = 0xffff;
                T_R("uh_sum after: %x, len = %d", uh_sum, len);
                memcpy(udp_buf+6, &uh_sum, sizeof(uh_sum));
            }
        }
    }
}



static size_t
pack_ip_udp_header(uchar * buf, u32 dest_ip, u32 src_ip, u16 port, u16 len)
{
    u16 ip_sum;
    /* Fill out the IP header */
    buf[0] = (4<<4) | (IP_HEADER_SIZE>>2);
    buf[1] = IPTOS_LOWDELAY;
    vtss_tod_pack16(IP_HEADER_SIZE + UDP_HEADER_SIZE + len, buf+2);
    vtss_tod_pack16(0, buf+4); // identification = 0
    vtss_tod_pack16(0, buf+6); // fragmentation = 0
    buf[8] = 128; // time to live
    buf[9] = IPPROTO_UDP;; // protocol
    vtss_tod_pack16(0, buf+10); // header checksum
    //update_ip_address();
    vtss_tod_pack32(src_ip, buf+12);
    vtss_tod_pack32(dest_ip, buf+16);
    /* Checksum the IP header... */
    ip_sum = wrapsum (checksum (buf, IP_HEADER_SIZE, 0));
    memcpy(buf+10, &ip_sum, sizeof(ip_sum));

    /* Fill out the UDP header */
    vtss_tod_pack16(port, buf+IP_HEADER_SIZE);  /* source port same as dest port */
    vtss_tod_pack16(port, buf+IP_HEADER_SIZE+2);  /* dest port */
    vtss_tod_pack16(UDP_HEADER_SIZE + len, buf+IP_HEADER_SIZE+4);  /* message length */
    vtss_tod_pack16(0, buf+IP_HEADER_SIZE+6); /* 0 checksum is allowed */

    return IP_HEADER_SIZE + UDP_HEADER_SIZE;
}

static size_t
unpack_ip_udp_header(const uchar * buf, u32 *src_ip, u32 *dest_ip, u16 *port, u16 *len)
{

    int             ip_header_len = (buf[0] & 0x0f) << 2;

    /* Decode the IP header */
    *len = vtss_tod_unpack16(buf+2) - ip_header_len - UDP_HEADER_SIZE;
    *src_ip = vtss_tod_unpack32(buf+12);
    *dest_ip = vtss_tod_unpack32(buf+16);

    /* Decode the UDP header */
    if (buf[9] == IPPROTO_UDP) { // protocol
        *port = vtss_tod_unpack16(buf+IP_HEADER_SIZE+2);  /* dest port */
        return ip_header_len + UDP_HEADER_SIZE;
    } else {
        *port = 0;
        return ip_header_len;
    }
}

/*
 * pack the Encapsulation protocol into a frame buffer.
 *
 */
size_t
vtss_1588_pack_encap_header(uchar * buf, Protocol_adr_t *sender, Protocol_adr_t *receiver, u16 data_size, BOOL event, ptp_tag_conf_t *tag_conf, int instance)
{
    size_t i;
    Protocol_adr_t my_sender;
    T_D("Tag_conf: enable %d, vid %d, port %d", tag_conf->enable, tag_conf->vid, tag_conf->port);
    if (sender != 0) {
        memcpy(&my_sender, sender, sizeof(my_sender));
    } else {
        my_sender.ip = ptp_global.my_ip[instance].s_addr;
        memcpy(my_sender.mac, ptp_global.sysmac, sizeof(my_sender.mac));
    }

    if (receiver->ip == 0) {
        i = pack_ethernet_header(buf, receiver->mac, my_sender.mac, ptp_ether_type, tag_conf->enable, tag_conf->vid, tag_conf->port, instance);
    } else {
        i = pack_ethernet_header(buf, receiver->mac, my_sender.mac, ip_ether_type, tag_conf->enable, tag_conf->vid, tag_conf->port, instance);
        i += pack_ip_udp_header(buf+i, receiver->ip, my_sender.ip, event ? PTP_EVENT_PORT : PTP_GENERAL_PORT, data_size);
    }
    return i;
}

void
vtss_1588_pack_eth_header(uchar * buf, mac_addr_t sender, mac_addr_t receiver)
{
    mac_addr_t  my_sender;
    int i = 0;
    if (sender != 0) {
        memcpy(my_sender, sender, sizeof(my_sender));
    } else {
        memcpy(my_sender, ptp_global.sysmac, sizeof(my_sender));
    }
    /* DA MAC */
    memcpy(&buf[i], receiver, sizeof(mac_addr_t));
    i += sizeof(mac_addr_t);
    /* SA MAC */
    memcpy(&buf[i], my_sender, sizeof(my_sender));
}


void
vtss_1588_tag_get(ptp_tag_conf_t *tag_conf, int instance, vtss_ptp_tag_t *tag)
{
    if (tag_conf->enable && (port_data[ptp_api_to_l2port(tag_conf->port)].vlan_forward[instance] == VTSS_PACKET_FILTER_TAGGED)) {
        tag->tpid = port_data[ptp_api_to_l2port(tag_conf->port)].vlan_tpid[instance];
        tag->vid = tag_conf->vid;
    } else {
        tag->tpid = 0;
        tag->vid = 0;
    }
}

/*
 * Propagate the PTP (module) configuration to the PTP core
 * library.
 */
static void
ptp_conf_propagate(void)
{
    int i, j;
    /* Make effective in PTP core */
    for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
        if (ptp_global.ptpi[i]) {
            vtss_ptp_clock_create(ptp_global.ptpi[i]);
            for (j = 0; j < MAX_UNICAST_MASTERS_PR_SLAVE; j++) {
                vtss_ptp_uni_slave_conf_set(ptp_global.ptpi[i], j, &config_data.conf[i].unicast_slave[j]);
            }
            ptp_port_link_state_initial(i);
            vtss_ptp_set_clock_slave_config(ptp_global.ptpi[i], &config_data.conf[i].slave_cfg);
        }
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
        if (i == 0 && config_data.conf[i].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[i].clock_init.protocol == PTP_PROTOCOL_OAM) {
            vtss_ptp_timer_start(&oam_timer, OAM_TIMER_INIT_VALUE, FALSE);
        }
#endif /* defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP) */
        if (ptp_ace_update(i) != VTSS_OK) T_EG(0, "ACE del error");;
    }
}

static void
ptp_conf_save(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    ptp_config_t    *conf;  /* run-time configuration data */
    ulong size;
    if ((conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PTP_CONF, &size)) != NULL) {
        if (size == sizeof(*conf)) {
            T_IG(0, "Saving configuration");
            *conf = config_data;
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PTP_CONF);
    }
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/**
 * initialize run-time options to reasonable values
 * \param create indicates a new default configuration block should be created.
 *
 */
static void
ptp_conf_init(ptp_instance_config_t *conf, UInteger8 defDomain)
{
    int i, j;
    port_iter_t       pit;

    /* use the Encapsulated MAC-48 value as default clockIdentity
     * according to IEEE Guidelines for 64-bit Global Identifier (EUI-64) registration authority
     */
    conf->clock_init.deviceType = PTP_DEVICE_NONE;
    conf->clock_init.twoStepFlag = CLOCK_FOLLOWUP;
    conf->clock_init.protocol = PTP_PROTOCOL_ETHERNET;
    conf->clock_init.oneWay = FALSE;
    conf->clock_init.tagging_enable = FALSE;
    conf->clock_init.configured_vid = 1;
    conf->clock_init.configured_pcp = 0;
    conf->clock_init.portCount = port_isid_port_count(VTSS_ISID_LOCAL);
    conf->clock_init.max_foreign_records = DEFAULT_MAX_FOREIGN_RECORDS;
    conf->clock_init.max_outstanding_records = DEFAULT_MAX_OUTSTANDING_RECORDS;
    memcpy(conf->clock_init.clockIdentity, ptp_global.sysmac, 3);
    memset(conf->clock_init.clockIdentity+3, 0xff, 2);
    memcpy(conf->clock_init.clockIdentity+5, ptp_global.sysmac+3, 3);

    conf->clock_ds.priority1 = 128;
    conf->clock_ds.priority2 = 128;
    conf->clock_ds.domainNumber = defDomain;

    ptp_get_clock_default_timeproperties_ds(&conf->time_prop);
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;
        conf->port_config[i].logSyncInterval = DEFAULT_SYNC_INTERVAL;
        conf->port_config[i].delayMechanism = DELAY_MECH_E2E;
        conf->port_config[i].logAnnounceInterval = DEFAULT_ANNOUNCE_INTERVAL;
        conf->port_config[i].logMinPdelayReqInterval = DEFAULT_DELAY_REQ_INTERVAL;
        conf->port_config[i].announceReceiptTimeout = DEFAULT_ANNOUNCE_RECEIPT_TIMEOUT;
        conf->port_config[i].delayAsymmetry = DEFAULT_DELAY_ASYMMETRY<<16;
        conf->port_config[i].versionNumber = VERSION_PTP;
        conf->port_config[i].initPortState = FALSE;
        conf->port_config[i].initPortInternal = FALSE;
        conf->port_config[i].ingressLatency = DEFAULT_INGRESS_LATENCY<<16;
        conf->port_config[i].egressLatency = DEFAULT_EGRESS_LATENCY<<16;
    }
    /* default offset filter config */
    ptp_default_filter_default_parameters_get(&conf->default_of);

    /* default servo config */
    ptp_default_servo_default_parameters_get(&conf->default_se);

    /* default delay filter  config*/
    ptp_default_delay_filter_default_parameters_get(&conf->default_df);
    for (j = 0; j < MAX_UNICAST_MASTERS_PR_SLAVE; j++) {
        conf->unicast_slave[j].duration = 100;
        conf->unicast_slave[j].ip_addr = 0;
    }
    ptp_get_default_clock_slave_config(&conf->slave_cfg);

}


/**
 * Read the PTP configuration.
 * \param create indicates a new default configuration block should be created.
 *
 */
static void
ptp_conf_read(BOOL create)
{
    ulong           size;
    BOOL            do_create;
    ptp_config_t    *conf;  /* run-time configuration data */
    int i;

    if (misc_conf_read_use()) {
        if ((conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PTP_CONF, &size)) == NULL ||
                size != sizeof(*conf)) {
            conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_PTP_CONF, sizeof(*conf));
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            T_W("actual size = %d,expected size = %d", size, sizeof(*conf));
            do_create = TRUE;
        } else if (conf->version != PTP_CONF_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = create;
        }

        /* initialize run-time options to reasonable values */
        if (conf != NULL) {
            for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
                if (conf->conf[i].clock_init.portCount != port_isid_port_count(VTSS_ISID_LOCAL)) {
                    T_I("number of ports have changed from %d to %d, therefore the configuration is reinitialized",
                        conf->conf[i].clock_init.portCount, port_isid_port_count(VTSS_ISID_LOCAL));
                    do_create = TRUE;
                }
            }
        }
    } else {
        conf      = NULL;
        do_create = TRUE;
    }

    if (do_create) {
        /* initialize run-time options to reasonable values */
        T_I("conf create");
        
        for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
            ptp_conf_init(&config_data.conf[i], i);
        }
        config_data.version = PTP_CONF_VERSION;
        config_data.init_synce.magic_word = 0;
        vtss_ext_clock_out_default_get(&config_data.init_ext_clock_mode);
#if defined (VTSS_ARCH_SERVAL)
        config_data.init_ext_clock_mode_rs422.mode = VTSS_PTP_RS422_DISABLE;
        config_data.init_ext_clock_mode_rs422.delay = 0;
#endif
        if (conf) {
            *conf = config_data;
        }
    } else {
        if (conf) {
            config_data = *conf;
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PTP_CONF);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

BOOL ptp_tc_sw_forwarding (int inst) 
{
    BOOL rv = FALSE;
#if defined (VTSS_ARCH_SERVAL)
    rv = (config_data.conf[inst].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT ||
            config_data.conf[inst].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT) &&
           config_data.conf[inst].clock_init.twoStepFlag;
#elif defined(VTSS_ARCH_LUTON26)
    rv = ((config_data.conf[inst].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT  &&
             (config_data.conf[inst].clock_init.twoStepFlag || port_data[inst].internal_port_exists)) ||
             config_data.conf[inst].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT);
#else
    if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
        rv = (config_data.conf[inst].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT ||
              config_data.conf[inst].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT) &&
             config_data.conf[inst].clock_init.twoStepFlag;
    } else {
        rv =  config_data.conf[inst].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT ||
                config_data.conf[inst].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT;
    }
#endif
    T_D("rv = %d", rv);
    return rv;
}
/****************************************************************************
 * Callbacks
 ****************************************************************************/


/*
 * Local port packet receive indication
 */
static BOOL packet_rx(void *contxt,
                      const u8 *const frm,
                      const vtss_packet_rx_info_t *const rx_info,
                      const void *const fdma_buffers)
{
    int i;
    char str1[20];
    char str2[20];
    u16 etype;
    u16 udp_port = 0;
    u16 len;
    u32 rxTime = 0;
    BOOL timestamp_ok = FALSE;
    Protocol_adr_t sender;
    BOOL release_fdma_buffers = TRUE;
    ptp_tx_buffer_handle_t buf_handle;




#ifdef VTSS_FEATURE_TIMESTAMP
    BOOL ongoing_adjustment = TRUE;
#endif
    
#if defined(VTSS_ARCH_LUTON26)
    vtss_ts_id_t ts_id;
    vtss_ts_timestamp_t ts;
#endif
#if defined(VTSS_ARCH_SERVAL)
    vtss_ts_id_t ts_id;
#endif
#if defined(VTSS_ARCH_JAGUAR_1)
    vtss_ts_timestamp_t ts;
#endif
#if defined(VTSS_ARCH_LUTON26)
    /* No timestamp interrupt, therefore do the timestamp update here */
    T_DG(_I,"update timestamps");
    PTP_RC(vtss_tx_timestamp_update(0));

#endif
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#if defined(VTSS_MISSING_TIMESTAMP_INTERRUPT)
    /* No timestamp interrupt, therefore do the timestamp update here */
    PTP_CORE_LOCK();
    port_iter_t       pit;
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;
        if (port_data[i].topo.ts_feature == VTSS_PTP_TS_PTS && port_data[i].link_state) {
            T_DG(_I,"update timestamps, port %d", i);
            PTP_CORE_UNLOCK();







            PTP_RC(vtss_phy_ts_fifo_empty(API_INST_DEFAULT, i));

            PTP_CORE_LOCK();
        }
    }
    PTP_CORE_UNLOCK();
#endif
#endif


    T_NG(_I, "fdma buffers %p",fdma_buffers);

    ulong j = unpack_ethernet_header(frm, sender.mac, &etype);
    T_RG(_I, "RX_ptp, etype = %04x, port = %d, da= %s, sa = %s", etype, rx_info->port_no, misc_mac_txt(&frm[0],str1), misc_mac_txt(&frm[6], str2));
    T_RG_HEX(_I, frm, rx_info->length);
    sender.ip = 0;

    u32 dest_ip = 0;
    if (etype == ip_ether_type) {
        j += unpack_ip_udp_header(frm+j, &sender.ip, &dest_ip, &udp_port, &len);
        T_NG(_I, "RX_udp, udp_port = %d, port = %d", udp_port, rx_info->port_no);
        if (udp_port != PTP_GENERAL_PORT && udp_port != PTP_EVENT_PORT) {
            T_WG(_I, "This UDP packet is not intended for me, udp_port = %d, port = %d", udp_port, rx_info->port_no);
            return FALSE; /* this IP packet was not intended for me: i.e forward to other subscribers. */
        }
    }
    if (((etype == ptp_ether_type) && (rx_info->length > j)) ||
            ((etype == ip_ether_type) && (rx_info->length > j) && (udp_port == PTP_GENERAL_PORT || udp_port == PTP_EVENT_PORT))) {
        u8 message_type = frm[j] & 0xf;

        u8 domain_number = frm[j+4];
        uint rx_port = ptp_l2port_to_api(rx_info->port_no);
        T_DG(_I, "message type: %d, domain %d, rx_port %d", message_type, domain_number, rx_port);
        PTP_CORE_LOCK();
        if (message_type <= 3) {
#if defined(VTSS_ARCH_LUTON26)
            ts_id.ts_id = rx_info->tstamp_id;
            timestamp_ok = rx_info->tstamp_id_decoded;
            T_IG(_I, "Timestamp_id %d, Timestamp_decoded %d", ts_id.ts_id, timestamp_ok);
            if (VTSS_RC_OK == vtss_rx_timestamp_get(0,&ts_id,&ts) && ts.ts_valid) {
                rxTime = ts.ts;
                /* special case for syntonized TC, i.e. instance 0 is slave_only oneway and instance 1 = onesetp TC */
                /* in L26, the rxtime is added to the correctionField in this case, because L26 does both onestep and twostep on the sync packet */
                if (config_data.conf[0].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[0].clock_init.oneWay &&
                        my_tc_encapsulation == config_data.conf[0].clock_init.protocol && my_tc_instance > 0 && message_type == PTP_MESSAGE_TYPE_SYNC) {
                    vtss_timeinterval_t corr = vtss_tod_unpack64(frm+j+PTP_MESSAGE_CORRECTION_FIELD_OFFSET);
                    corr -= 4*(((vtss_timeinterval_t)rxTime)<<16);
                    /*Update correction Field */
                    vtss_tod_pack64(corr, (u8 *)(frm+j+PTP_MESSAGE_CORRECTION_FIELD_OFFSET));
                }
                
            } else {
                T_IG(_I, "No valid timestamp detected for id: %d",ts_id.ts_id);
                rxTime = 0;
                timestamp_ok = FALSE;
            }
#ifdef PTP_LOG_TRANSPARENT_ONESTEP_FORWARDING
            T_WG(_I,"Forwarded message type %d, ts.id %lu, from port %u, source: %s, dest %s",
                 message_type, ts_id.ts_id, rx_port,
                 (etype == ip_ether_type) ? misc_ipv4_txt(sender.ip,str1) : misc_mac_txt(sender.mac, str1),
                 (etype == ip_ether_type) ? misc_ipv4_txt(dest_ip,str2) : misc_mac_txt(sender.mac, str2));
            uint tx_port_no;
            port_iter_t       pit;
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                tx_port_no = pit.iport;
                if (VTSS_RC_OK == vtss_tx_timestamp_get(0, tx_port_no,&ts_id,&ts) && ts.ts_valid) {
                    T_WG(_I,"Forwarded message, ts.id %u, to port %u", ts_id.ts_id, ptp_l2port_to_api(tx_port_no));
                }
            }
#endif
#endif
#if !defined(VTSS_ARCH_SERVAL)
            if (port_data[rx_info->port_no].topo.ts_feature == VTSS_PTP_TS_PTS) {
                rxTime = vtss_tod_unpack32(frm+j+PTP_MESSAGE_RESERVED_FOR_TS_OFFSET);
                rxTime = vtss_tod_ns_to_ts_cnt(rxTime);
                timestamp_ok = TRUE;
                vtss_tod_pack32(0,(u8 *)frm+j+PTP_MESSAGE_RESERVED_FOR_TS_OFFSET);   /* clear reserved field */
                T_IG(VTSS_TRACE_GRP_PHY_TS, "msgtype %d, rxtime %u, rx_port %u", message_type, rxTime, rx_port);
            } else {
#if defined(VTSS_ARCH_LUTON28)
                /* subtract the inbound latency adjustment  */
                vtss_1588_ts_cnt_sub(&rxTime, rx_info->sw_tstamp.hw_cnt, port_data[rx_info->port_no].ingress_cnt);
                timestamp_ok = TRUE;
#endif
#if defined(VTSS_ARCH_JAGUAR_1)
                ts.ts = rx_info->hw_tstamp;
                timestamp_ok = rx_info->hw_tstamp_decoded;
                T_DG(_I, "Raw timestamp %d, hw_tstamp_decoded %d", ts.ts, timestamp_ok);
                if (VTSS_RC_OK == vtss_rx_master_timestamp_get(0, rx_info->port_no, &ts) && ts.ts_valid) {
                    /* subtract the inbound latency adjustment  */
                    vtss_1588_ts_cnt_sub(&rxTime, ts.ts, port_data[rx_info->port_no].ingress_cnt);
                } else {
                    T_IG(_I, "No valid timestamp for port: %u",rx_port);
                    rxTime = 0;
                    timestamp_ok = FALSE;
                }
#endif
                if (message_type == 0) {
                    /* if Sync message then subtract the p2p delay from rx time  */
                    vtss_1588_ts_cnt_sub(&rxTime, rxTime, port_data[rx_info->port_no].delay_cnt);
                }
                /* link asymmetry compensation for Sync and PdelayResp events */
                if ((message_type == PTP_MESSAGE_TYPE_SYNC || message_type == PTP_MESSAGE_TYPE_P_DELAY_RESP) &&
                port_data[rx_info->port_no].asymmetry_cnt != 0) {
                    vtss_1588_ts_cnt_sub(&rxTime, rxTime, port_data[rx_info->port_no].asymmetry_cnt);
                }
            }
                
#else /* !defined(VTSS_ARCH_SERVAL) */
            rxTime = rx_info->hw_tstamp;
            timestamp_ok = rx_info->hw_tstamp_decoded;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
            if (port_data[rx_info->port_no].topo.ts_feature == VTSS_PTP_TS_PTS) {
				if (phy_ts_mode == VTSS_TOD_INTERNAL_TC_MODE_30BIT) {
					/* convert to 32 bit timestamp */
					rxTime = vtss_tod_ns_to_ts_cnt(rxTime);
					T_DG(_I, "rxTime after %u", rxTime);
				} else if (phy_ts_mode != VTSS_TOD_INTERNAL_TC_MODE_32BIT) {
					T_WG(_I, "PHY timestamp mode %d not supported", phy_ts_mode);
				}
			}
#endif
			
            T_IG(_I, "Raw timestamp %u, hw_tstamp_decoded %u", rxTime, timestamp_ok);
            ts_id.ts_id = rx_info->tstamp_id;
            if (rx_info->tstamp_id_decoded) {
                (void)vtss_rx_timestamp_id_release(0,&ts_id);
                T_DG(_I, "Timestamp %u, ts decoded %d, Ts id %d", rxTime, rx_info->tstamp_id_decoded, ts_id.ts_id);
            }
#endif /* defined(VTSS_ARCH_SERVAL) */

        }
#ifdef VTSS_FEATURE_TIMESTAMP
        if (VTSS_RC_OK == vtss_ts_ongoing_adjustment(NULL,&ongoing_adjustment) && !ongoing_adjustment) {
#else
        {
#endif
            for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
                if (config_data.conf[i].port_config[rx_info->port_no].initPortState &&
                            (((config_data.conf[i].clock_init.deviceType == PTP_DEVICE_ORD_BOUND ||
                               config_data.conf[i].clock_init.deviceType == PTP_DEVICE_MASTER_ONLY ||
                               config_data.conf[i].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY) &&
                               config_data.conf[i].clock_ds.domainNumber == domain_number)  ||
                               ptp_tc_sw_forwarding(i) || message_type == PTP_MESSAGE_TYPE_P_DELAY_RESP ||
                               message_type == PTP_MESSAGE_TYPE_P_DELAY_REQ || message_type == PTP_MESSAGE_TYPE_P_DELAY_RESP_FOLLOWUP)) 
                {
                    if (!release_fdma_buffers) {
                        T_DG(_I, "inst %d buffer from domain %d has already been forwarded", i, domain_number);
                    } else {
                        T_NG(_I, "clock instance: %d, domain %d", i, domain_number);
                        if (!config_data.conf[i].clock_init.tagging_enable || rx_info->tag.vid == config_data.conf[i].clock_init.configured_vid) {
                            T_IG(_I,"Accepted Vlan tag: tag %x vid %d,", rx_info->tag_type, rx_info->tag.vid);
                            /* fill in the buffer structure */
                            vtss_1588_tx_handle_init(&buf_handle);
                            buf_handle.handle = (void *)fdma_buffers;
                            buf_handle.frame = (u8 *)frm;                          /* pointer to frame buffer */
                            buf_handle.size = rx_info->length;                    /* length of data received, including encapsulation. */
                            buf_handle.header_length = j;                    /* length of encapsulation header */
                            buf_handle.hw_time = rxTime;                     /* internal HW time value used for correction field update. */
                            if (config_data.conf[i].clock_init.tagging_enable) {
                                buf_handle.tag.vid = rx_info->tag.vid;
                                buf_handle.tag.tpid = port_data[rx_info->port_no].vlan_tpid[i];
                            }
                            T_IG(_I,"Handle Vlan tpid %x vid %d,", buf_handle.tag.tpid, buf_handle.tag.vid);
                            
                            if (message_type <= 3) {
                                // EVENT_PDU:
                                T_IG(_I, "EVENT_PDU: %u bytes, port %u, msg_type %d, timestamp_ok %d", rx_info->length, rx_port, message_type, timestamp_ok);
                                /* if the port has PHY 1588 support, the timestamp is read from the reserved field */
                                if (timestamp_ok) {
                                    release_fdma_buffers = !vtss_ptp_event_rx(ptp_global.ptpi[i], rx_port, &buf_handle, &sender);
                                }
                            } else if (message_type >=8 && message_type <=0xd) {
                                // GENERAL_PDU:
                                T_NG(_I, "GENERAL_PDU: %u bytes, port %u", rx_info->length, rx_info->port_no);
                                release_fdma_buffers = !vtss_ptp_general_rx(ptp_global.ptpi[i], rx_port, &buf_handle, &sender);
                            } else {
                                T_EG(_I, "Invalid message type %d received", message_type);
                            }
                        } else {
                            T_IG(_I,"Not Accepted Vlan tag: tag %x vid %d,", rx_info->tag_type, rx_info->tag.vid);
                        }
                    }
                }
            }
        //} else {
        //    T_DG(_I,"Ignore packets when ongoing clock adjustment");
        }            
        PTP_CORE_UNLOCK();
    } else {
        T_EG(_I, "Invalid message length %u or etype %04x received", rx_info->length, etype);
    }
    if (release_fdma_buffers) {
        T_NG(_I, "free fdma buffers %p",fdma_buffers);
        packet_rx_release_fdma_buffers((void *)fdma_buffers);
    }
    return TRUE; // Don't allow other subscribers to receive the packet
}

/****************************************************************************
 * PTP callouts - vtss_ptp_packet_callout.h
 ****************************************************************************/
// This macro must *not* evaluate to an empty macro, since it will be called an expecting to do useful stuff.
#define PARAMETER_CHECK(x, y) do {if (!(x)) {T_E("Assertion failed: " #x); y}} while (0)
#define EXTRA_SIZE ((sizeof(onestep_extra_t)+3)/sizeof(u32))

size_t
vtss_1588_prepare_general_packet(u8 **frame, Protocol_adr_t *receiver, size_t size, size_t *header_size, ptp_tag_conf_t *tag_conf, int instance)
{
    // Check parameters
    PARAMETER_CHECK(frame != NULL, return 0;);
    *frame = packet_tx_alloc(size + header_size_ip4);
    if (*frame) {
        *header_size = vtss_1588_pack_encap_header(*frame, NULL, receiver,  size, FALSE, tag_conf, instance);
        return size + *header_size;
    } else {
        return 0;
    }
}

size_t
vtss_1588_prepare_general_packet_2(u8 **frame, Protocol_adr_t *sender, Protocol_adr_t *receiver, size_t size, size_t *header_size, ptp_tag_conf_t *tag_conf, int instance)
{
    // Check parameters
    PARAMETER_CHECK(frame != NULL, return 0;);
    *frame = packet_tx_alloc(size + header_size_ip4);
    if (*frame) {
        *header_size = vtss_1588_pack_encap_header(*frame, sender, receiver,  size, FALSE, tag_conf, instance);
        return size + *header_size;
    } else {
        return 0;
    }
}

void vtss_1588_release_general_packet(u8 **handle)
{
    // Check parameters
    PARAMETER_CHECK(handle != NULL, return;);
    PARAMETER_CHECK(*handle != NULL, return;); /* no buffer allocated */
    packet_tx_free(*handle);
    *handle = NULL;
}



size_t
vtss_1588_packet_tx_alloc(void **handle, u8 **frame, size_t size)
{
    // Check parameters
    PARAMETER_CHECK(handle != NULL, return 0;);
    PARAMETER_CHECK(frame != NULL, return 0;);
    PARAMETER_CHECK(*handle == NULL, return 0;); /* buffer already allocated */
    *frame = packet_tx_alloc_extra(size, EXTRA_SIZE, (u8 **)handle);
    if (*frame) {
        return size;
    } else {
        return 0;
    }
}

void vtss_1588_packet_tx_free(void **handle)
{
    // Check parameters
    PARAMETER_CHECK(handle != NULL, return;);
    PARAMETER_CHECK(*handle != NULL, return;); /* no buffer allocated */
    packet_tx_free_extra(*handle);
    *handle = NULL;
}

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#if defined(VTSS_MISSING_TIMESTAMP_INTERRUPT)
static void poll_phy_fifo(void)
{
    /* No timestamp interrupt, therefore do the timestamp update here */
    int i;
    PTP_CORE_LOCK();
    port_iter_t       pit;
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;
        if (port_data[i].topo.ts_feature == VTSS_PTP_TS_PTS && port_data[i].link_state) {
            T_DG(_I,"update timestamps, port %d", i);
            PTP_CORE_UNLOCK();







            PTP_RC(vtss_phy_ts_fifo_empty(API_INST_DEFAULT, i));

            PTP_CORE_LOCK();
        }
    }
    PTP_CORE_UNLOCK();
}
#endif
#endif



/**
 * vtss_1588_tx_event_one_step - Transmit done callback.
 */
static void packet_forw_done(void *correlator, packet_tx_done_props_t *props)
{
    packet_rx_release_fdma_buffers(correlator);
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#if defined(VTSS_MISSING_TIMESTAMP_INTERRUPT)
    poll_phy_fifo();
#endif
#endif
}






/**
 * vtss_1588_tx_general - Transmit a general message.
 */
size_t
vtss_1588_tx_general(u64 port_mask,
                     u8 *frame,
                     size_t size,
                     vtss_ptp_tag_t *tag)
{
    packet_tx_props_t tx_props;
    T_RG_HEX(_I, frame, size);
    T_IG(_I, "Portmask %llx , tx %zd bytes", port_mask, size);
    if (port_mask) {
        packet_tx_props_init(&tx_props);
        tx_props.packet_info.modid     = VTSS_MODULE_ID_PTP;
        tx_props.packet_info.frm[0]    = frame;
        tx_props.packet_info.len[0]    = size;
        tx_props.tx_info.dst_port_mask = port_mask;
        tx_props.tx_info.tag.tpid = tag->tpid;
        tx_props.tx_info.tag.vid =  tag->vid;
        T_IG(_I, "Tag tpid: %x , vid %d", tx_props.tx_info.tag.tpid , tx_props.tx_info.tag.vid);

        update_udp_checksum(frame);
        if (packet_tx(&tx_props) == VTSS_RC_OK) {
            return size;
        } else {
            T_EG(_I,"Transmit general on non-port (%llx)?", port_mask);
        }
    }
    return 0;
}

                        
/**
 * Send a general or event PTP message.
 *
 * \param port_mask switch port mask.
 * \param ptp_buf_handle ptp transmit buffer handle
 *
 */

/**
 * vtss_1588_tx_event_one_step - Transmit done callback.
 */

#if defined(VTSS_ARCH_JAGUAR_1)
#if 0
static void one_step_timestamped (void *context, u32 port_no, vtss_ts_timestamp_t *ts)
{
    onestep_extra_t *onestep_extra = context;
    u32 now = 0;
    u32 lat;
    vtss_timeinterval_t egr_lat;
    char buf1 [25];

    if (ts->ts_valid) {
        now = ts->ts;
        vtss_1588_ts_cnt_sub(&lat, now, onestep_extra->tx_time);
        vtss_1588_ts_cnt_to_timeinterval(&egr_lat, lat);
        T_DG(VTSS_TRACE_GRP_EGR_LAT, "Onestep data: hw_time %ld, tx_time %ld, now %ld, lat %ld, egr_lat %s ",
             onestep_extra->hw_time, onestep_extra->tx_time, now, lat, vtss_tod_TimeInterval_To_String(&egr_lat,buf1,'.'));

        PTP_CORE_LOCK();
        if (observed_egr_lat.cnt == 0) {
            observed_egr_lat.max = egr_lat;
            observed_egr_lat.min = egr_lat;
            observed_egr_lat.mean = egr_lat;
        } else {
            if (observed_egr_lat.max < egr_lat) observed_egr_lat.max = egr_lat;
            if (observed_egr_lat.min > egr_lat) observed_egr_lat.min = egr_lat;
            observed_egr_lat.mean = (observed_egr_lat.mean*observed_egr_lat.cnt + egr_lat)/(observed_egr_lat.cnt+1);
        }
        ++observed_egr_lat.cnt;
        PTP_CORE_UNLOCK();
    } else {
        T_DG(VTSS_TRACE_GRP_EGR_LAT, "Missed Onestep tx time");
    }

}
#endif
#endif



/**
 * vtss_1588_tx_event - Transmit done callback.
 */
static void packet_tx_done_x(void *correlator, packet_tx_done_props_t *props)
{



    ptp_tx_buffer_handle_t *h = correlator;
    
    if (h->msg_type == 1) {  /*2 step transmission */
#if defined(VTSS_ARCH_LUTON26)
        /* No timestamp interrupt, therefore do the timestamp update here */
        T_DG(_I,"update timestamps");
        PTP_RC(vtss_tx_timestamp_update(0));

#endif
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#if defined(VTSS_MISSING_TIMESTAMP_INTERRUPT)
        poll_phy_fifo();
#endif
#endif

    }
    PTP_CORE_LOCK();
    T_DG(_I, "txdone: correlator %p", correlator);
    h->cb(correlator);
    PTP_CORE_UNLOCK();
}


#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
static void timestamp_cbx (void *context, u32 port_no, vtss_ts_timestamp_t *ts)
{
    T_DG(_I, "timestamp: port_no %d, ts %u", port_no, ts->ts);
    if (ts->ts_valid) {
        PTP_CORE_LOCK();
#if defined(VTSS_ARCH_JAGUAR_1)
        if (port_data[port_no].topo.ts_feature != VTSS_PTP_TS_PTS) {
            /* egress latency compensation in SW */
            vtss_1588_ts_cnt_add(&ts->ts, ts->ts, port_data[port_no].egress_cnt);
        }
#endif
        ptp_tx_timestamp_context_t *h = (ptp_tx_timestamp_context_t *)context;
        if (h->cb_ts) {
            h->cb_ts(h->context, l2port_to_ptp_api(port_no), ts->ts);
        } else {
            T_WG(_I, "Missing callback: port_no %d", port_no);
        }
        PTP_CORE_UNLOCK();
    } else {
        T_IG(_I, "Missed tx time: port_no %d", port_no);
    }
}
#endif

static vtss_rc allocate_phy_signaturex(u8 *ptp_frm, void *context, u64 port_mask)
{
    vtss_rc rc = VTSS_RC_OK;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    vtss_ts_timestamp_alloc_t alloc_parm;
    vtss_phy_ts_fifo_sig_t    ts_sig;
    
    ts_sig.sig_mask = my_sig_mask;
    ts_sig.msg_type = ptp_frm[PTP_MESSAGE_MESSAGE_TYPE_OFFSET] & 0xF;
    ts_sig.domain_num = ptp_frm[PTP_MESSAGE_DOMAIN_OFFSET];
    memcpy(ts_sig.src_port_identity, ptp_frm+PTP_MESSAGE_SOURCE_PORT_ID_OFFSET, 10);
    ts_sig.sequence_id = vtss_tod_unpack16(&ptp_frm[PTP_MESSAGE_SEQUENCE_ID_OFFSET]);
    
    alloc_parm.port_mask = port_mask;
    alloc_parm.context = context;
    alloc_parm.cb = timestamp_cbx;
    rc = vtss_module_man_tx_timestamp_sig_alloc(&alloc_parm, &ts_sig);
    T_DG(_I,"Timestamp Signature allocated: mask: %x, type %d, domain %d, seq: %d, rc = %d", 
         ts_sig.sig_mask, ts_sig.msg_type, ts_sig.domain_num, ts_sig.sequence_id, rc);
#endif
    return rc;
}

void vtss_1588_tx_handle_init(ptp_tx_buffer_handle_t *ptp_buf_handle)
{
    memset(ptp_buf_handle, 0, sizeof(ptp_tx_buffer_handle_t));
}


/**
 * vtss_1588_forw_one_step_event.
 */
typedef struct {
    u8 corr [8];
    u8 reserved[4];
} onestep_forw_t;

/**
 * vtss_1588_tx_event_one_step - Transmit just before done callback.
 */
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1)
static void packet_forw_one_step_pre_done(void *tx_pre_contxt, /* Defined by user module                                      */
        unsigned char *frm,  /* Pointer to the frame to be transmitted (points to the DMAC) */
        size_t len)         /* Frame length (excluding IFH, CMD, and FCS).  */

{
    u32 res_raw;
    u32 tx_time;
    u32 res;
    onestep_forw_t *forw = tx_pre_contxt;
    vtss_timeinterval_t corr = vtss_tod_unpack64(forw->corr);
    vtss_timeinterval_t residence;
    corr += observed_egr_lat.min;
    tx_time = vtss_tod_get_ns_cnt();
    memcpy(&res, forw->reserved, sizeof(res));
    vtss_1588_ts_cnt_sub(&res_raw, tx_time, res);
    memset(forw->reserved, 0, sizeof(forw->reserved));
    vtss_1588_ts_cnt_to_timeinterval(&residence, res_raw);
    corr += residence;
    /*Update correction Field */
    vtss_tod_pack64(corr, (forw->corr));
}
#endif




size_t vtss_1588_tx_msg(u64 port_mask,
                        ptp_tx_buffer_handle_t *ptp_buf_handle)
{
    packet_tx_props_t tx_props;
    vtss_rc rc = VTSS_RC_OK;
    u64 pts_port_mask = 0;
    packet_tx_pre_cb_t pre_cb = NULL;
    int portnum = 0;
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1)
    u32 hw_ns = 0;
#endif
    u8 *my_buf = (u8 *)ptp_buf_handle->frame + ptp_buf_handle->header_length;
    onestep_forw_t *forw = (onestep_forw_t *)&my_buf[8];
#if !defined(VTSS_ARCH_SERVAL)
    u8 message_type = my_buf[PTP_MESSAGE_MESSAGE_TYPE_OFFSET] & 0xF;
#endif    
    T_RG_HEX(_I, ptp_buf_handle->frame, ptp_buf_handle->size);
    T_IG(_I, "Portmask %llx , tx %zd bytes", port_mask, ptp_buf_handle->size);
    if (port_mask) {
        packet_tx_props_init(&tx_props);
        tx_props.packet_info.modid     = VTSS_MODULE_ID_PTP;
        tx_props.packet_info.frm[0]    = ptp_buf_handle->frame;
        tx_props.packet_info.len[0]    = ptp_buf_handle->size;
        tx_props.tx_info.dst_port_mask = port_mask;
        if (ptp_buf_handle->cb) {
            tx_props.packet_info.tx_done_cb         = packet_tx_done_x;
            tx_props.packet_info.tx_done_cb_context = ptp_buf_handle; /* User-defined - used in tx done callback */
        } else {
            tx_props.packet_info.tx_done_cb         = packet_forw_done;
            tx_props.packet_info.tx_done_cb_context = ptp_buf_handle->handle; /* If no user callback the fdma buffer must be free'ed */
        }
        tx_props.packet_info.tx_pre_cb_context  = (void*)forw;
        tx_props.tx_info.tag.tpid = ptp_buf_handle->tag.tpid;
        tx_props.tx_info.tag.vid =  ptp_buf_handle->tag.vid;
        T_IG(_I, "Tag tpid: %x , vid %d", tx_props.tx_info.tag.tpid , tx_props.tx_info.tag.vid);
        
        /* split port mask into PHY TS ports and non PHY TS ports */
        port_iter_t       pit;
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            portnum = pit.iport;
            if (port_mask & (1LL<<portnum)) {
                if (port_data[portnum].topo.ts_feature == VTSS_PTP_TS_PTS) {
                    pts_port_mask |= 1LL<<portnum;
                    port_mask &= ~(1LL<<portnum);
                }
            }
        }
        switch (ptp_buf_handle->msg_type) {
            case 0: /*general packet */
                T_IG(_I, "general packet");
                break;
            case 1: /*2-step packet */
                T_IG(_I, "2-step packet");
                
                if (pts_port_mask) {
                    /* packet transmitted to PTS ports */
                    rc = allocate_phy_signaturex(ptp_buf_handle->frame + ptp_buf_handle->header_length, ptp_buf_handle->context, pts_port_mask);
                } 
                if (port_mask) {
                
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
                    vtss_ts_timestamp_alloc_t alloc_parm;
                    alloc_parm.port_mask = port_mask;
                    alloc_parm.context = ptp_buf_handle->context;
                    alloc_parm.cb = timestamp_cbx;
                    vtss_ts_id_t ts_id;
                    rc = vtss_tx_timestamp_idx_alloc(0, &alloc_parm, &ts_id); /* allocate id for transmission*/
                    T_DG(_I,"Timestamp Id (%u)allocated", ts_id.ts_id);
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
                    tx_props.tx_info.ptp_action    = 2; /* twostep action */
                    tx_props.tx_info.ptp_id        = ts_id.ts_id;
                    tx_props.tx_info.ptp_timestamp = 0;
#endif
#if defined(VTSS_ARCH_JAGUAR_1)
                    /* latch register no+1: see Jaguar Datasheet, table 7, fwd.capture_tstamp */
                    tx_props.tx_info.latch_timestamp = ts_id.ts_id+1;
#endif
#endif
                }
                break;
            case 2: /*correction field update */
#if !defined(VTSS_ARCH_SERVAL)
                if (!pts_port_mask) {
                    /* link asymmetry compensation for DelayReq and PDelayReq */
                    portnum = 0;
                    while ((port_mask & (1LL<<portnum)) == 0 && portnum < PTP_CLOCK_PORTS) {
                        portnum++;
                    }
                    if (portnum >= PTP_CLOCK_PORTS)  {
                        portnum = PTP_CLOCK_PORTS-1;
                        T_WG(_I, "invalid portnum");
                    }
                    if ((message_type == PTP_MESSAGE_TYPE_DELAY_REQ || message_type == PTP_MESSAGE_TYPE_P_DELAY_REQ) &&
                    port_data[portnum].asymmetry_cnt != 0) {
                        vtss_1588_ts_cnt_add(&ptp_buf_handle->hw_time, ptp_buf_handle->hw_time, port_data[portnum].asymmetry_cnt);
                    }
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1)
                    /* egress latency compensation in SW */
                    vtss_1588_ts_cnt_sub(&ptp_buf_handle->hw_time, ptp_buf_handle->hw_time, port_data[portnum].egress_cnt);
#endif
                }
#endif
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1)
                if (pts_port_mask) {
                    if ((message_type != PTP_MESSAGE_TYPE_SYNC) ||
                            ptp_buf_handle->cb == NULL) {       /* always insert reserved field if forwarding packet. This is the case if the callback is NULL */
                        hw_ns = vtss_tod_ts_cnt_to_ns(ptp_buf_handle->hw_time);
                        vtss_tod_pack32(hw_ns,forw->reserved);   /* insert hw_time in reserved field for PHY*/
                        T_IG(_I,"HW time %u ns, inserted in reserved field", hw_ns);
                    }
                } else {
                    memcpy(forw->reserved, &ptp_buf_handle->hw_time, sizeof(ptp_buf_handle->hw_time));     /* insert hw_time -egress latehcy in reserved field for pre callback */
                    pre_cb = packet_forw_one_step_pre_done;
                    u32 lat_min = observed_egr_lat.min>>16;
                    T_DG(_I,"HW time %u, observed egr_lat %d", ptp_buf_handle->hw_time, lat_min);
                }
                if (pts_port_mask && port_mask) {
                    T_WG(_I,"onestep forwarding to both PHY_TS ports and Jaguar ports is not supported ");
                }

#endif
#if defined(VTSS_ARCH_JAGUAR_1)
#if 0
                if (port_mask) {
                    vtss_ts_id_t    ts_id;
                    vtss_ts_timestamp_alloc_t alloc_parm;
                    alloc_parm.port_mask = port_mask;
                    alloc_parm.context = ptp_buf_handle->context;
                    alloc_parm.cb = one_step_timestamped;
                    rc = vtss_tx_timestamp_idx_alloc(0, &alloc_parm, &ts_id); /* allocate id for transmission*/
                    T_DG(_I,"Timestamp Id (%lu)allocated", ts_id.ts_id);
                    /* latch register no+1: see Jaguar Datasheet, table 7, fwd.capture_tstamp */
                    tx_props.tx_info.latch_timestamp = ts_id.ts_id+1;
                    //tx_props.tx_info.cos = 8; /* it is not possible to timestamp super priority queue packets */
                }
#endif
                tx_props.tx_info.ptp_action         = 0; /* action not supported in Jaguar */
#else
                T_IG(_I, "one step packet, hw_time %u", ptp_buf_handle->hw_time);
                tx_props.tx_info.ptp_action         = 1; /* onestep action */
                tx_props.tx_info.ptp_timestamp      = ptp_buf_handle->hw_time; /* used for correction field update */
#if defined(VTSS_ARCH_SERVAL)
                tx_props.tx_info.tag.tpid = 0;  /* In Serval: must be 0 otherwise the FDMA inserts a tag, and the so do the Switch */
#endif
#endif
                break;
            case 3: /*org_time */
                tx_props.tx_info.ptp_action         = 4; /* origin PTP action */
#if defined(VTSS_ARCH_LUTON26)
                T_EG(_I, "Org_time is not supported in Caracal");
                rc = VTSS_RC_ERROR;
#else
                tx_props.tx_info.tag.tpid = 0;  /* must be 0 otherwise the FDMA inserts a tag, and the so do the Switch */
                T_IG(_I, "HW sets preciseOriginTimestamp in the packet");
#endif
                break;
            default:
                T_WG(_I, "invalid message type");
                break;
        }

        if ((rc == VTSS_RC_OK)) {
            update_udp_checksum(ptp_buf_handle->frame);
            tx_props.packet_info.tx_pre_cb = pre_cb; /* Pre-Tx callback function */
            if (packet_tx(&tx_props) == VTSS_RC_OK) {
                return ptp_buf_handle->size;
            } else {
                T_EG(_I,"Transmit message on non-port (%llx)?", port_mask);
            }
        } else {
            T_WG(_I,"Could not get a timestamp ID");
        }
    }
    if (ptp_buf_handle->cb) {
        ptp_buf_handle->cb(ptp_buf_handle);
    } else {
        /*free rx FDMA buffer */
        packet_rx_release_fdma_buffers(ptp_buf_handle->handle);
    }
    return 0;
}








static int ptp_sock = 0;

static void ptp_socket_init(void)
{
    struct sockaddr_in sender;
    u16 port = PTP_GENERAL_PORT;
    int length, retry = 0;

    if (ptp_sock > 0) close(ptp_sock);

    ptp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ptp_sock < 0) T_EG(_I, "socket returned %d",ptp_sock);

    length = sizeof(sender);
    bzero(&sender,length);
    sender.sin_family=AF_INET;
    sender.sin_addr.s_addr=INADDR_ANY;
    vtss_tod_pack16(port,(UInteger8*)&sender.sin_port);
    T_IG(_I, "binding socket");
    while (bind(ptp_sock, (struct sockaddr *)&sender, length) < 0 ) {
        if (++retry > 30) {
            T_EG(_I, "binding error, err_no %d",errno);
            break;
        }
        T_IG(_I, "binding problem, err_no %d",errno);
        cyg_thread_delay(1000 / ECOS_MSECS_PER_HWTICK); /* 1 sec */
    }
}

size_t
vtss_1588_tx_unicast_request(u32 dest_ip,
                             const void *buffer,
                             size_t size, 
                             int instance)
{
    struct sockaddr_in receiver;
    char buf1[20];
    int n;
    int length;
    struct sockaddr_in my_addr;
    socklen_t my_addr_len;
    u16 port = PTP_GENERAL_PORT;
    if (ptp_global.my_ip[instance].s_addr == 0) {
        T_IG(_I,"cannot send before my ip address is defined");
        return 0;
    }

    T_IG(_I, "Dest_ip %s, size %d", misc_ipv4_txt(dest_ip, buf1), size);
    //packet_dump(buffer, size);

    memset(&receiver, 0, sizeof(struct sockaddr_in));
    receiver.sin_len = sizeof(in_addr_t);
    receiver.sin_family = AF_INET;
    vtss_tod_pack32(dest_ip,(UInteger8*)&receiver.sin_addr);
    vtss_tod_pack16(port,(UInteger8*)&receiver.sin_port);
    length = sizeof(struct sockaddr_in);

    /* Connect socket */
    if (connect(ptp_sock, (struct sockaddr *)&receiver, length) != 0) {
        T_WG(_I, "Failed to connect: %d - %s", errno, strerror(errno));
        return 0;
    }

    /* Get my address */
    my_addr_len = sizeof(my_addr);
    if (getsockname(ptp_sock, (struct sockaddr *)&my_addr, &my_addr_len) != 0) {
        T_WG(_I, "getsockname failed: %d - %s", errno, strerror(errno));
        return 0;
    }
    if (my_addr.sin_family != AF_INET) {
        T_WG(_I, "Got something else than as an IPv4 address: %d", my_addr.sin_family);
        return 0;
    }
    T_IG(_I, "Source_ip %s, size %d", misc_ipv4_txt(my_addr.sin_addr.s_addr, buf1), size);

    n=write(ptp_sock,buffer,size);
    if (n < 0) {
        T_IG(_I,"write returned %d",n);
        if (errno == EHOSTUNREACH) {
            T_I("Error no: %d, error message: %s", errno, strerror(errno));
        } else {
            T_W("Error no: %d, error message: %s", errno, strerror(errno));
        }
        return 0;
    }
    return n;
}



/****************************************************************************/
/*  Reserved ACEs functions and MAC table setup                             */
/****************************************************************************/
/* On PCB107 all ports have 1588 PHY, therefore no ACL rules are needed for timestamping */
/* instead the forwarding is set up in the MAC table */
#define PTP_MAC_ENTRIES (PTP_CLOCK_INSTANCES*2)
static vtss_mac_table_entry_t mac_entry[PTP_MAC_ENTRIES];
static u32 mac_used = 0;

/* Add reserved ACE */
#define MAX_ACL_RULES_PR_PTP_CLOCK 6

typedef struct {
    UInteger8 domain;
    UInteger8 deviceType;
    UInteger8 protocol;
    BOOL      port_list[PTP_CLOCK_PORTS];
    BOOL      external_port_list[PTP_CLOCK_PORTS];
    BOOL      internal_port_list[PTP_CLOCK_PORTS];
    vtss_ace_id_t id [MAX_ACL_RULES_PR_PTP_CLOCK];
    BOOL      internal_port_exists;
} ptp_ace_rule_t;
static ptp_ace_rule_t rules[PTP_CLOCK_INSTANCES] = {
    {.domain = 0, .deviceType = PTP_DEVICE_NONE, .protocol = PTP_PROTOCOL_ETHERNET, 
     .id = {ACE_ID_NONE, ACE_ID_NONE,ACE_ID_NONE, ACE_ID_NONE, ACE_ID_NONE,ACE_ID_NONE}},
 {.domain = 0, .deviceType = PTP_DEVICE_NONE, .protocol = PTP_PROTOCOL_ETHERNET, 
     .id = {ACE_ID_NONE, ACE_ID_NONE,ACE_ID_NONE, ACE_ID_NONE, ACE_ID_NONE,ACE_ID_NONE}},
 {.domain = 0, .deviceType = PTP_DEVICE_NONE, .protocol = PTP_PROTOCOL_ETHERNET, 
     .id = {ACE_ID_NONE, ACE_ID_NONE,ACE_ID_NONE, ACE_ID_NONE, ACE_ID_NONE,ACE_ID_NONE}},
 {.domain = 0, .deviceType = PTP_DEVICE_NONE, .protocol = PTP_PROTOCOL_ETHERNET, 
     .id = {ACE_ID_NONE, ACE_ID_NONE,ACE_ID_NONE, ACE_ID_NONE, ACE_ID_NONE,ACE_ID_NONE}},
};

/*PTP_PROTOCOL_ETHERNET = 0, PTP_PROTOCOL_IP4MULTI = 1, PTP_PROTOCOL_IP4UNI = 2 */
static vtss_ace_type_t conf_type[] = {VTSS_ACE_TYPE_ETYPE,       VTSS_ACE_TYPE_IPV4,        VTSS_ACE_TYPE_IPV4  };

/*
 * ACL rule_id's are allocated this way:
 * if no TC instance exists, then the rules are inserted as last entries
 * if a tc exists, then the rules are inserted before the TC's rules.
 * the first TC rule id is saved in my_next_id.
 * This is because the BC's are more specific than the TC rules.
 * If a TC exists, that forwards all packets in HW, then my_tc_encapsulation is set to the TC encapsulation type
 * If this type of TC exists, and a SlaveOnly instance also exists, this slave is used for syntonization of the TC,
 * and the packets forwarded to the CPU must also be forwarded, i.e. the ACL rules for this slaveOnly instance depends
 * on the TC configuration. (my_tc_encapsulation is defined and described earlier in this file)
 */
static vtss_ace_id_t my_next_id = ACE_ID_NONE;                  /* indicates that no TC exists */
        
static vtss_rc acl_rule_apply(acl_entry_conf_t    *conf, int instance, int rule_id)
{
    if (rule_id >= MAX_ACL_RULES_PR_PTP_CLOCK) {
        T_W("too many ACL rules");
        return VTSS_RC_ERROR;
    }
    conf->id = rules[instance].id[rule_id];/* if id == ACE_ID_NONE, it is auto assigned, otherwise it is reused */
    T_D("acl_mgmt_ace_add instance: %d, rule idx %d, conf->id %d, my_next_id %d", instance, rule_id, conf->id, my_next_id);
    PTP_RETURN(acl_mgmt_ace_add(ACL_USER_PTP, my_next_id, conf)); 
    if (conf->conflict) {
        T_W("acl_mgmt_ace_add conflict instance %d, rule %d", instance, rule_id);
    } else {
        rules[instance].id[rule_id] = conf->id;
    }
    T_D("ACL rule_id %d, conf->id %d", rule_id, conf->id);
    return VTSS_RC_OK;
}


static vtss_rc ptp_ace_update(int i)
{
    vtss_rc             rc = VTSS_RC_OK;
    acl_entry_conf_t    conf;
    int rule_no = ACE_ID_NONE;
    port_iter_t       pit;
#if defined(VTSS_FEATURE_ACL_V2)
    int portlist_size;
    vtss_ace_ptp_t *pptp = 0;
#endif
    u32 mac_idx;
    u32 inst, idx;
    char dest_txt[100];
    int j;
    BOOL refresh_inst_0 = FALSE;
    
    if ((config_data.conf[i].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY ||
         config_data.conf[i].clock_init.deviceType == PTP_DEVICE_MASTER_ONLY) && 
         config_data.conf[i].clock_init.protocol == PTP_PROTOCOL_OAM) {
        return rc;
    }
    if ((config_data.conf[i].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY) && 
            config_data.conf[i].clock_init.protocol == PTP_PROTOCOL_1PPS) {
        return rc;
    }
    PTP_RETURN(ptp_phy_ts_update(i));
    if (rules[i].deviceType != PTP_DEVICE_NONE) {
        /* Remove rules for this clock instance */
        if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
            /* All ports have 1588 PHY, therefore no ACL rules are needed for timestamping */
            /* instead the forwarding is set up in the MAC table */
            for (mac_idx = 0; mac_idx < PTP_MAC_ENTRIES; mac_idx++) {
                if (mac_idx < mac_used) {
                    PTP_RC(vtss_mac_table_del(NULL,&mac_entry[mac_idx].vid_mac));
                    T_I("deleting mac table entry: mac %s, vid %d, copy to cpu %d", misc_mac2str(mac_entry[mac_idx].vid_mac.mac.addr),
                        mac_entry[mac_idx].vid_mac.vid, mac_entry[mac_idx].copy_to_cpu);
                }
                memset(&mac_entry[mac_idx], 0, sizeof(vtss_mac_table_entry_t));
            }
            mac_used = 0;
        }            
        if (rules[i].deviceType == PTP_DEVICE_E2E_TRANSPARENT || rules[i].deviceType == PTP_DEVICE_P2P_TRANSPARENT) {
            my_next_id = ACE_ID_NONE;
            if (i == my_tc_instance && rules[i].protocol == rules[0].protocol &&
                rules[0].deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[0].clock_init.oneWay) {
                refresh_inst_0 = TRUE;
            }
#if defined(VTSS_FEATURE_ACL_V2)
            my_tc_encapsulation = PTP_PROTOCOL_MAX_TYPE;   /* indicates that no forwarding in the SlaveOnly instance is needed */
#endif /*defined(VTSS_FEATURE_ACL_V2)*/
            my_tc_instance = -1;
        }
        rules[i].deviceType = PTP_DEVICE_NONE;
        for (j=0; j < MAX_ACL_RULES_PR_PTP_CLOCK; j++) {
            if (rules[i].id[j] != ACE_ID_NONE) {
                PTP_RC(acl_mgmt_ace_del(ACL_USER_PTP, rules[i].id[j]));
                rules[i].id[j] = ACE_ID_NONE;
            }
            T_IG(_I,"clear ACL rules for inst %d", i);
        }
    } 

    
    if (config_data.conf[i].clock_init.deviceType != PTP_DEVICE_NONE) {
        /* Set up rules for this instance */
        rules[i].deviceType = config_data.conf[i].clock_init.deviceType;
        rules[i].protocol = config_data.conf[i].clock_init.protocol;
        rules[i].domain = config_data.conf[i].clock_ds.domainNumber;
        if (rules[i].protocol == PTP_PROTOCOL_IP4UNI && (ptp_global.my_ip[i].s_addr == 0)) {
            T_IG(_I,"cannot set up unicast ACL before my ip address is defined");
            return PTP_RC_MISSING_IP_ADDRESS;
        }
        rules[i].internal_port_exists = 0;
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            j = pit.iport;
            rules[i].port_list[j] = config_data.conf[i].port_config[j].initPortState;
            rules[i].external_port_list[j] = config_data.conf[i].port_config[j].initPortState && !config_data.conf[i].port_config[j].initPortInternal;
            rules[i].internal_port_list[j] = config_data.conf[i].port_config[j].initPortInternal;
            rules[i].internal_port_exists |= config_data.conf[i].port_config[j].initPortInternal;
            T_DG(_I,"inst %d, port %d, list %d, xlist %d, ilist %d", i, j, rules[i].port_list[j], rules[i].external_port_list[j], rules[i].internal_port_list[j]);
        }
        T_DG(_I,"inst %d, internal port exists %d", i, rules[i].internal_port_exists);
        port_data[i].internal_port_exists = rules[i].internal_port_exists;

        T_IG(_I, "ace_init protocol: %d", rules[i].protocol);
        if ((rc = acl_mgmt_ace_init(conf_type[rules[i].protocol], &conf)) != VTSS_OK) {
            return rc;
        }
        conf.action.cpu_queue = PACKET_XTR_QU_BPDU;  /* increase extraction priority to the same as bpdu packets */
        VTSS_BF_SET(conf.flags.value, ACE_FLAG_IP_FRAGMENT, 0);  /* ignore IPV4 fragmented packets */
        VTSS_BF_SET(conf.flags.mask, ACE_FLAG_IP_FRAGMENT, 1);
#if defined(VTSS_FEATURE_ACL_V2)
        conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
        memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
#else
        conf.action.permit = FALSE;
#endif /* VTSS_FEATURE_ACL_V2 */
        conf.action.logging = FALSE;
        conf.action.shutdown = FALSE;
        conf.action.cpu_once = FALSE;
        conf.isid = VTSS_ISID_LOCAL;
        if (conf.type == VTSS_ACE_TYPE_ETYPE) {
            conf.frame.etype.etype.value[0] = (ptp_ether_type>>8) & 0xff;
            conf.frame.etype.etype.value[1] = ptp_ether_type & 0xff;
            conf.frame.etype.etype.mask[0] = 0xff;
            conf.frame.etype.etype.mask[1] = 0xff;
        } else if (conf.type == VTSS_ACE_TYPE_IPV4) {
            conf.frame.ipv4.proto.value = 17; //UDP
            conf.frame.ipv4.proto.mask = 0xFF;
            conf.frame.ipv4.sport.in_range = conf.frame.ipv4.dport.in_range = TRUE;
            conf.frame.ipv4.sport.low = 0;
            conf.frame.ipv4.sport.high = 65535;
            conf.frame.ipv4.dport.low = 319;
            conf.frame.ipv4.dport.high = 320;
            if ((PTP_DEVICE_ORD_BOUND == rules[i].deviceType || PTP_DEVICE_MASTER_ONLY  == rules[i].deviceType ||
                    PTP_DEVICE_SLAVE_ONLY == rules[i].deviceType) && (rules[i].protocol == PTP_PROTOCOL_IP4UNI)) {
                /* in unicast mode only terminate PTP messages with my address */
                conf.frame.ipv4.dip.value = ptp_global.my_ip[i].s_addr;
                conf.frame.ipv4.dip.mask = 0xffffffff;
            }
        }

#if defined(VTSS_FEATURE_ACL_V2)
#if (PTP_CLOCK_PORTS != VTSS_PORT_ARRAY_SIZE)
        T_W("inconsistenc port array sizes %d, %d", sizeof(conf.port_list), sizeof(rules[i].port_list));
#endif
        portlist_size = sizeof(conf.port_list);
        memcpy(conf.port_list,  rules[i].port_list, portlist_size);
        conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
        if (conf.type == VTSS_ACE_TYPE_ETYPE) {
            pptp = &conf.frame.etype.ptp;
        } else if (conf.type == VTSS_ACE_TYPE_IPV4) {
            pptp = &conf.frame.ipv4.ptp;
        } else {
            T_W("unsupported ACL frame type %d", conf.type);
            return PTP_RC_UNSUPPORTED_ACL_FRAME_TYPE;
        }
#if defined(VTSS_ARCH_SERVAL)
        switch (rules[i].deviceType) {
            case PTP_DEVICE_ORD_BOUND:
            case PTP_DEVICE_MASTER_ONLY:
            case PTP_DEVICE_SLAVE_ONLY:
                if (i == 0 && rules[i].deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[i].clock_init.oneWay &&
                    my_tc_instance > 0) {
                    /* the Announce packets for this SlaveOnly device are also forwarded to the TC ports */
                    /* also it the TC is a 2-step */  
                    T_D("forward announce in instance %d to TC instance %d", i, my_tc_instance);
                    memcpy(conf.action.port_list,  rules[my_tc_instance].port_list, portlist_size);
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = TRUE;
                    pptp->enable = TRUE;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x0b; /* messageType = [0xb] */
                    pptp->header.mask[1] = 0x0f;
                    pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                    pptp->header.mask[2] = 0xff;
                    pptp->header.value[2] = rules[i].domain; /* domainNumber */
                    pptp->header.mask[3] = 0x00;
                    pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
                    /* ACL rule for Announce */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
                }
                if (i == 0 && rules[i].deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[i].clock_init.oneWay &&
                my_tc_encapsulation == rules[i].protocol && my_tc_instance > 0) {
                    /* the other packets for this SlaveOnly device are also forwarded to the TC ports */
                    /* only if the TC is a 1-step */  
                    memcpy(conf.action.port_list,  rules[my_tc_instance].port_list, portlist_size);
                    T_D("inst %d: packets are forwarded to instance %d TC ports", i, my_tc_instance);
                }
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                conf.action.force_cpu = TRUE;
                pptp->enable = TRUE;
                pptp->header.mask[0] = 0x08;
                pptp->header.value[0] = 0x08; /* messageType = [8..15] */
                pptp->header.mask[1] = 0x0f;
                pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                pptp->header.mask[2] = 0xff;
                pptp->header.value[2] = rules[i].domain; /* domainNumber */
                pptp->header.mask[3] = 0x00;
                pptp->header.value[3] = 0x00; /* flagField[0] = don't care */

                /* ACL rule for generals */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                /* ACL rule for Sync events */
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP_SUB_DELAY_2; /* Asymmetry and p2p delay compensation */
                pptp->header.mask[0] = 0x0f;
                pptp->header.value[0] = 0x00; /* messageType = [0] */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                /* ACL rule for PdelayResp events */
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP_SUB_DELAY_1; /* Asymmetry delay compensation */
                pptp->header.mask[0] = 0x0f;
                pptp->header.value[0] = 0x03; /* messageType = [3] */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                /* ACL rule for DelayReq, PdelayReq events */
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP; /* Rx timestamp no compensation */
                pptp->header.mask[0] = 0x0c;
                pptp->header.value[0] = 0x00; /* messageType = [1..2] (because 0 and 3 are hit by the previous rules */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                break;
            case PTP_DEVICE_E2E_TRANSPARENT:
                if (config_data.conf[i].clock_init.twoStepFlag) {
                    /* two-step E2E transparent clock */
                    /**********************************/
                    memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = TRUE;
                    pptp->enable = TRUE;
                    pptp->header.mask[0] = 0x0e;
                    pptp->header.value[0] = 0x08; /* messageType = [8..9] */
                    pptp->header.mask[1] = 0x0f;
                    pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                    pptp->header.mask[2] = 0x00;
                    pptp->header.value[2] = 0x00; /* domainNumber = d.c */
                    pptp->header.mask[3] = 0x00;
                    pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
        
                    /* ACL rule for Follow_up, Delay_resp */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
        
                    /* ACL rule for Pdelay_resp_follow_up */
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x0a; /* messageType = [10] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
        
                    /* ACL rule for OneStep Sync Events */
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x00; /* messageType = [0] */
                    pptp->header.mask[3] = 0x02;
                    pptp->header.value[3] = 0x00; /* flagField[0] = twostep = 0 */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
        
                    /* ACL rule for 2step Sync, Delay_req and Pdelay_xx */
                    /* forwarded, and 2-step action, i.e. a timestamp id is reserved, and transmit time is saved in fifo */
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
                    pptp->header.mask[0] = 0x0c;
                    pptp->header.value[0] = 0x00; /* messageType = [0..3] */
                    pptp->header.mask[3] = 0x00;
                    pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for All other PTP messages (forward) */
                    memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                    conf.action.force_cpu = FALSE;   /* forwarded for TC. If needed for a BC, then the BC rules must hit first */
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    pptp->header.mask[0] = 0x00;
                    pptp->header.value[0] = 0x00; /* messageType = d.c. */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                } else {
                    /* one-step E2E transparent clock */
                    /**********************************/
                    memcpy(conf.port_list,  rules[i].port_list, portlist_size);
                    memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP_SUB_DELAY_2;
                    conf.action.force_cpu = FALSE;
                    pptp->enable = TRUE;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x00; /* messageType = [0] */
                    pptp->header.mask[1] = 0x0f;
                    pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                    pptp->header.mask[2] = 0x00;
                    pptp->header.value[2] = 0x00; /* domainNumber = d.c */
                    pptp->header.mask[3] = 0x00;
                    pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
                    /* ACL rule for Sync Events:  */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for PDelayResp Events:  */
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP_SUB_DELAY_1;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x03; /* messageType = [3] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for DelayReq, PDelayReq Events:  */
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP_ADD_DELAY;
                    pptp->header.mask[0] = 0x0c;
                    pptp->header.value[0] = 0x00; /* messageType = [1,2]  (because 0 and 3 are hit by the previous rules */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for other general messages */
                    memcpy(conf.port_list,  rules[i].port_list, portlist_size);
                    memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = FALSE;
                    pptp->header.mask[0] = 0x08;
                    pptp->header.value[0] = 0x08; /* messageType = [8..15] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                }
        
                break;
            case PTP_DEVICE_P2P_TRANSPARENT:
                if (config_data.conf[i].clock_init.twoStepFlag) {
                    /* two-step P2P transparent clock */
                    /**********************************/
                    memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = TRUE;
                    pptp->enable = TRUE;
                    pptp->header.mask[0] = 0x0d;
                    pptp->header.value[0] = 0x08; /* messageType = [8,10] */
                    pptp->header.mask[1] = 0x0f;
                    pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                    pptp->header.mask[2] = 0x00;
                    pptp->header.value[2] = 0x00; /* domainNumber = d.c */
                    pptp->header.mask[3] = 0x00;
                    pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
        
                    /* ACL rule for Follow_up, Pdelay_resp_follow_up */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
        
                    /* ACL rule for Delay_Req, Delay_resp */
                    conf.action.force_cpu = FALSE;
                    pptp->header.mask[0] = 0x07;
                    pptp->header.value[0] = 0x01; /* messageType = [1,9] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
        
                    /* ACL rule for TwoStep Sync Events */
                    memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = TRUE;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x00; /* messageType = [0] */
                    pptp->header.mask[3] = 0x02;
                    pptp->header.value[3] = 0x02; /* flagField[0] = twostep = 1 */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                    
					/* ACL rule for PdelayResp events */
					memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
					conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP_SUB_DELAY_1; /* Asymmetry delay compensation */
                    conf.action.force_cpu = TRUE;
					pptp->header.mask[0] = 0x0f;
					pptp->header.value[0] = 0x03; /* messageType = [3] */
                    pptp->header.mask[3] = 0x00;
                    pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
					PTP_RC(acl_rule_apply(&conf, i, rule_no++));
					
                    /* ACL rule for OnesStep Sync, Pdelay_req */
					memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = TRUE;
                    pptp->header.mask[0] = 0x0d;
                    pptp->header.value[0] = 0x00; /* messageType = [0,2] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
        
                    /* ACL rule for All other PTP messages */
                    memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    pptp->header.mask[0] = 0x00;
                    pptp->header.value[0] = 0x00; /* messageType = d.c. */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                } else {
                    /* one-step P2P transparent clock */
                    /**********************************/
                    memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP_SUB_DELAY_2;
                    conf.action.force_cpu = FALSE;
                    pptp->enable = TRUE;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x00; /* messageType = [0] */
                    pptp->header.mask[1] = 0x0f;
                    pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                    pptp->header.mask[2] = 0x00;
                    pptp->header.value[2] = 0x00; /* domainNumber = d.c */
                    pptp->header.mask[3] = 0x00;
                    pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
                    /* ACL rule for Sync Events */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for PdelayReq Events */
                    memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = TRUE;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x02; /* messageType = [2] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for PdelayResp Events */
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP_SUB_DELAY_1;
                    conf.action.force_cpu = TRUE;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x03; /* messageType = [3] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
        
                    /* ACL rule for Pdelay_Resp_follow_Up messages */
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    pptp->header.mask[0] = 0x0f;
                    pptp->header.value[0] = 0x0a; /* messageType = [10] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for Delay_Req and Delay_Resp messages */
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = FALSE;
                    pptp->header.mask[0] = 0x07;
                    pptp->header.value[0] = 0x01; /* messageType = [1,9] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for All other PTP messages */
                    memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                    pptp->header.mask[0] = 0x00;
                    pptp->header.value[0] = 0x00; /* messageType = d.c. */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                }
                break;
        }
#else
        switch (rules[i].deviceType) {
        case PTP_DEVICE_ORD_BOUND:
        case PTP_DEVICE_MASTER_ONLY:
        case PTP_DEVICE_SLAVE_ONLY:
            if (i == 0 && rules[i].deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[i].clock_init.oneWay &&
            my_tc_instance > 0) {
                /* the Announce packets for this SlaveOnly device are also forwarded to the TC ports */
                /* also it the TC is a 2-step */  
                T_D("forward announce in instance %d to TC instance %d", i, my_tc_instance);
                memcpy(conf.action.port_list,  rules[my_tc_instance].port_list, portlist_size);
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                conf.action.force_cpu = TRUE;
                pptp->enable = TRUE;
                pptp->header.mask[0] = 0x0f;
                pptp->header.value[0] = 0x0b; /* messageType = [0xb] */
                pptp->header.mask[1] = 0x0f;
                pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                pptp->header.mask[2] = 0xff;
                pptp->header.value[2] = rules[i].domain; /* domainNumber */
                pptp->header.mask[3] = 0x00;
                pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
                /* ACL rule for Announce */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
            }
            if (i == 0 && rules[i].deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[i].clock_init.oneWay &&
            my_tc_encapsulation == rules[i].protocol && my_tc_instance > 0) {
                /* the other packets for this SlaveOnly device are also forwarded to the TC ports */
                /* only if the TC is a 1-step */  
                memcpy(conf.action.port_list,  rules[my_tc_instance].port_list, portlist_size);
                T_D("inst %d: packets are forwarded to instance %d TC ports", i, my_tc_instance);
            }
            conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
            conf.action.force_cpu = TRUE;
            pptp->enable = TRUE;
            pptp->header.mask[0] = 0x08;
            pptp->header.value[0] = 0x08; /* messageType = [8..15] */
            pptp->header.mask[1] = 0x0f;
            pptp->header.value[1] = 0x02; /* versionPTP = 2 */
            pptp->header.mask[2] = 0xff;
            pptp->header.value[2] = rules[i].domain; /* domainNumber */
            pptp->header.mask[3] = 0x00;
            pptp->header.value[3] = 0x00; /* flagField[0] = don't care */

            /* ACL rule for generals */
            PTP_RC(acl_rule_apply(&conf, i, rule_no++));

            /* ACL rule for Sync */
            conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP; /* always twostep action on received packets */
            if (i == 0 && rules[i].deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[i].clock_init.oneWay &&
            my_tc_encapsulation == rules[i].protocol && my_tc_instance > 0) {
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_AND_TWO_STEP; /* always twostep action on received packets, */
                                                                               /* and also do 1-step because the packet is 'TC' forwarded  */
            }
            pptp->header.mask[0] = 0x0f;
            pptp->header.value[0] = 0x00; /* messageType = [0] */
            PTP_RC(acl_rule_apply(&conf, i, rule_no++));

            /* ACL rule for DelayReq, Pdelayxxx events */
            conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP; /* always twostep action on received packets */
            pptp->header.mask[0] = 0x0c;
            pptp->header.value[0] = 0x00; /* messageType = [1..3] (0 is hit by the previous rule)*/

            PTP_RC(acl_rule_apply(&conf, i, rule_no++));
            break;
        case PTP_DEVICE_E2E_TRANSPARENT:
            if (config_data.conf[i].clock_init.twoStepFlag) {
                /* two-step E2E transparent clock */
                /**********************************/
                memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                conf.action.force_cpu = TRUE;
                pptp->enable = TRUE;
                pptp->header.mask[0] = 0x0e;
                pptp->header.value[0] = 0x08; /* messageType = [8..9] */
                pptp->header.mask[1] = 0x0f;
                pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                pptp->header.mask[2] = 0x00;
                pptp->header.value[2] = 0x00; /* domainNumber = d.c */
                pptp->header.mask[3] = 0x00;
                pptp->header.value[3] = 0x00; /* flagField[0] = don't care */

                /* ACL rule for Follow_up, Delay_resp */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                /* ACL rule for Pdelay_resp_follow_up */
                pptp->header.mask[0] = 0x0f;
                pptp->header.value[0] = 0x0a; /* messageType = [10] */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                /* ACL rule for OneStep Sync Events */
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
                pptp->header.mask[0] = 0x0f;
                pptp->header.value[0] = 0x00; /* messageType = [0] */
                pptp->header.mask[3] = 0x02;
                pptp->header.value[3] = 0x00; /* flagField[0] = twostep = 0 */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                /* ACL rule for 2step Sync, Delay_req and Pdelay_xx */
                //memcpy(conf.action.port_list,  rules[i].port_list, portlist_size); in first release the CPU forwards the messages
                pptp->header.mask[0] = 0x0c;
                pptp->header.value[0] = 0x00; /* messageType = [0..3] */
                pptp->header.mask[3] = 0x00;
                pptp->header.value[3] = 0x00; /* flagField[0] = don't care */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                /* ACL rule for All other PTP messages (forward) */
                memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                conf.action.force_cpu = FALSE;   /* forwarded for TC. If needed for a BC, then the BC rulse must hit first */
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                pptp->header.mask[0] = 0x00;
                pptp->header.value[0] = 0x00; /* messageType = d.c. */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
            } else {
                /* one-step E2E transparent clock */
                /**********************************/
                memcpy(conf.port_list,  rules[i].external_port_list, portlist_size);
#ifdef PTP_LOG_TRANSPARENT_ONESTEP_FORWARDING
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
                conf.action.force_cpu = TRUE;
#else
                /* if no internal ports: forward to other external ports, if internal ports exista: copy to CPU */
                if (rules[i].internal_port_exists) {
                    memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
                    conf.action.force_cpu = TRUE;
                } else {
                    memcpy(conf.action.port_list,  rules[i].external_port_list, portlist_size);
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP;
                    conf.action.force_cpu = FALSE;
                }
#endif
                pptp->enable = TRUE;
                pptp->header.mask[0] = 0x0c;
                pptp->header.value[0] = 0x00; /* messageType = [0..3] */
                pptp->header.mask[1] = 0x0f;
                pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                pptp->header.mask[2] = 0x00;
                pptp->header.value[2] = 0x00; /* domainNumber = d.c */
                pptp->header.mask[3] = 0x00;
                pptp->header.value[3] = 0x00; /* flagField[0] = don't care */

                /* ACL rule for External port Events:  */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                if (rules[i].internal_port_exists) {
                    /* ACL rule for Internal port Events: Send to CPU, SW expects a timestamp on all events, though it is not used */
                    memcpy(conf.port_list,  rules[i].internal_port_list, portlist_size);
                    memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
                    conf.action.force_cpu = TRUE;
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                    /* ACL rule for follow_Up and Delay_Resp messages. Forwarded in SW like Sync and DelayReq, otherwise the packet order may change */
                    memcpy(conf.port_list,  rules[i].port_list, portlist_size);
                    memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                    conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                    conf.action.force_cpu = TRUE;
                    pptp->enable = TRUE;
                    pptp->header.mask[0] = 0x0e;
                    pptp->header.value[0] = 0x08; /* messageType = [8..9] */
                    PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                }                
                /* ACL rule for other general messages */
                memcpy(conf.port_list,  rules[i].port_list, portlist_size);
                memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                conf.action.force_cpu = FALSE;
                pptp->header.mask[0] = 0x08;
                pptp->header.value[0] = 0x08; /* messageType = [8..15] */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
            }

            break;
        case PTP_DEVICE_P2P_TRANSPARENT:
            if (config_data.conf[i].clock_init.twoStepFlag) {
                /* two-step P2P transparent clock */
                /**********************************/
                memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                conf.action.force_cpu = TRUE;
                pptp->enable = TRUE;
                pptp->header.mask[0] = 0x0d;
                pptp->header.value[0] = 0x08; /* messageType = [8,10] */
                pptp->header.mask[1] = 0x0f;
                pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                pptp->header.mask[2] = 0x00;
                pptp->header.value[2] = 0x00; /* domainNumber = d.c */
                pptp->header.mask[3] = 0x00;
                pptp->header.value[3] = 0x00; /* flagField[0] = don't care */

                /* ACL rule for Follow_up, Pdelay_resp_follow_up */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                /* ACL rule for Delay_Req, Delay_resp */
                pptp->header.mask[0] = 0x07;
                pptp->header.value[0] = 0x01; /* messageType = [1,9] */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                /* ACL rule for Sync, Pdelay_req, Pdelay_resp */
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
                pptp->header.mask[0] = 0x0c;
                pptp->header.value[0] = 0x00; /* messageType = [0..3] */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                /* ACL rule for All other PTP messates */
                memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                pptp->header.mask[0] = 0x00;
                pptp->header.value[0] = 0x00; /* messageType = d.c. */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
            } else {
                /* one-step P2P transparent clock */
                /**********************************/
                memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_ONE_STEP;
                conf.action.force_cpu = FALSE;
                pptp->enable = TRUE;
                pptp->header.mask[0] = 0x0f;
                pptp->header.value[0] = 0x00; /* messageType = [0] */
                pptp->header.mask[1] = 0x0f;
                pptp->header.value[1] = 0x02; /* versionPTP = 2 */
                pptp->header.mask[2] = 0x00;
                pptp->header.value[2] = 0x00; /* domainNumber = d.c */
                pptp->header.mask[3] = 0x00;
                pptp->header.value[3] = 0x00; /* flagField[0] = don't care */

                /* ACL rule for Sync Events */

#ifndef VTSS_FEATURE_TIMESTAMP_P2P_DELAY_COMP
                /* as Luton26 does not support path delay correction, the sync is handled in the CPU */
                memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
                conf.action.force_cpu = TRUE;
#endif
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                /* ACL rule for Pdelay Events */
                memset(conf.action.port_list, 0x00, sizeof(conf.action.port_list));
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_TWO_STEP;
                conf.action.force_cpu = TRUE;
                pptp->header.mask[0] = 0x0e;
                pptp->header.value[0] = 0x02; /* messageType = [2..3] */

                PTP_RC(acl_rule_apply(&conf, i, rule_no++));

                /* ACL rule for Pdelay_Resp_follow_Up messages */
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                pptp->header.mask[0] = 0x0f;
                pptp->header.value[0] = 0x0a; /* messageType = [10] */
#ifndef VTSS_FEATURE_TIMESTAMP_P2P_DELAY_COMP
                /* in Luton26 the sync is handled in the CPU, and also the Followup, otherwise the Followup may arrive before the Sync */
                pptp->header.mask[0] = 0x0d;
                pptp->header.value[0] = 0x08; /* messageType = [8, 10] */
#endif
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                /* ACL rule for Delay_Req and Delay_Resp messages */
                conf.action.ptp_action = VTSS_ACL_PTP_ACTION_NONE;
                conf.action.force_cpu = FALSE;
                pptp->header.mask[0] = 0x07;
                pptp->header.value[0] = 0x01; /* messageType = [1,9] */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
                /* ACL rule for All other PTP messages */
                memcpy(conf.action.port_list,  rules[i].port_list, portlist_size);
                pptp->header.mask[0] = 0x00;
                pptp->header.value[0] = 0x00; /* messageType = d.c. */
                PTP_RC(acl_rule_apply(&conf, i, rule_no++));
            }
            break;
        }
#endif /* VTSS_ARCH_SERVAL */
        if (rules[i].deviceType == PTP_DEVICE_E2E_TRANSPARENT || rules[i].deviceType == PTP_DEVICE_P2P_TRANSPARENT) {
            my_next_id = rules[i].id[0];
#if defined(VTSS_ARCH_SERVAL)
            if ((i != 0) && !config_data.conf[i].clock_init.twoStepFlag) {
#else 
            if ((i != 0) && (rules[i].deviceType == PTP_DEVICE_E2E_TRANSPARENT) && 
                !config_data.conf[i].clock_init.twoStepFlag) {
#endif
                my_tc_encapsulation = rules[i].protocol;   /* indicates that forwarding in the SlaveOnly instance is needed */
                my_tc_instance = i;
                if (rules[0].deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[0].clock_init.oneWay &&
                    my_tc_encapsulation == rules[0].protocol) {
                    /* recalculate instance 0 ACL rules */
                    refresh_inst_0 = TRUE;
                }
            } else {
                my_tc_encapsulation = PTP_PROTOCOL_MAX_TYPE;   /* indicates that no forwarding in the SlaveOnly instance is needed */
                my_tc_instance = i;
                if (rules[0].deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[0].clock_init.oneWay &&
                rules[i].protocol == rules[0].protocol) {
                    /* recalculate instance 0 ACL rules */
                    refresh_inst_0 = TRUE;
                }
            }
        }

        
#else

        if (vtss_board_type() != VTSS_BOARD_JAG_PCB107_REF) {
            conf.action.port_no = VTSS_PORT_NO_NONE;
            conf.action.force_cpu = TRUE;

            PTP_RC(acl_rule_apply(&conf, i, rule_no++));
            T_I("conf.id %d, conf.type %d", conf.id, conf.type);
        }
#endif /* VTSS_FEATURE_ACL_V2 */
        if (rules[i].deviceType == PTP_DEVICE_E2E_TRANSPARENT || rules[i].deviceType == PTP_DEVICE_P2P_TRANSPARENT) {
            my_next_id = rules[i].id[0];
        }

    }
    if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
        /* All ports have 1588 PHY, therefore no ACL rules are needed for timestamping */
        /* instead the forwarding is set up in the MAC table */
        mac_used = 0;
        my_tc_instance = -1;
        for (inst = 0; inst < PTP_CLOCK_INSTANCES; inst++) {
            if (rules[inst].deviceType != PTP_DEVICE_NONE && 
                    (rules[inst].protocol == PTP_PROTOCOL_ETHERNET || rules[inst].protocol == PTP_PROTOCOL_IP4MULTI)) {
                /* check if the configuration matches previous instances */
                for (mac_idx = 0; mac_idx < mac_used; mac_idx++) {
                    if ((0 == memcmp(&mac_entry[mac_idx].vid_mac.mac,  /* match 1588 multicast Dest address */
                                     rules[inst].protocol == PTP_PROTOCOL_IP4MULTI ? ptp_ip_mcast_adr[0] : ptp_eth_mcast_adr[0], 
                                     sizeof(mac_addr_t))) && mac_entry[mac_idx].vid_mac.vid == config_data.conf[inst].clock_init.configured_vid) {
                        break;
                    }
                }
                if (mac_idx >= mac_used) {
                    ++mac_used;
                }
                T_I("mac_used %d, mac_idx %d", mac_used, mac_idx);
                memcpy(&mac_entry[mac_idx].vid_mac.mac,  /* match 1588 multicast Dest address */
                       rules[inst].protocol == PTP_PROTOCOL_IP4MULTI ? ptp_ip_mcast_adr[0] : ptp_eth_mcast_adr[0], 
                       sizeof(mac_addr_t));
                mac_entry[mac_idx].vid_mac.vid = config_data.conf[inst].clock_init.configured_vid;
                
                if (rules[inst].deviceType == PTP_DEVICE_ORD_BOUND || rules[inst].deviceType == PTP_DEVICE_MASTER_ONLY || 
                        rules[inst].deviceType == PTP_DEVICE_SLAVE_ONLY ||
                        ((rules[inst].deviceType == PTP_DEVICE_P2P_TRANSPARENT || rules[inst].deviceType == PTP_DEVICE_E2E_TRANSPARENT) &&
                         config_data.conf[inst].clock_init.twoStepFlag)) {
                    mac_entry[mac_idx].copy_to_cpu = TRUE;
                }
                if ((rules[inst].deviceType == PTP_DEVICE_P2P_TRANSPARENT || rules[inst].deviceType == PTP_DEVICE_E2E_TRANSPARENT) &&
                        !config_data.conf[inst].clock_init.twoStepFlag) {
                    port_iter_init_local(&pit);
                    my_tc_instance = inst;
                    while (port_iter_getnext(&pit)) {
                        mac_entry[mac_idx].destination[pit.iport] |= rules[inst].port_list[pit.iport];
                    }
                }
            }
        }
        /* repeat the same process for the Peer delay multicast addresses (always to sent to CPU) */
        for (inst = 0; inst < PTP_CLOCK_INSTANCES; inst++) {
            if (rules[inst].deviceType != PTP_DEVICE_NONE && 
                    (rules[inst].protocol == PTP_PROTOCOL_ETHERNET || rules[inst].protocol == PTP_PROTOCOL_IP4MULTI)) {
                /* check if the configuration matches previous instances */
                for (mac_idx = 0; mac_idx < mac_used; mac_idx++) {
                    if ((0 == memcmp(&mac_entry[mac_idx].vid_mac.mac,  /* match 1588 multicast Dest address */
                                     rules[inst].protocol == PTP_PROTOCOL_IP4MULTI ? ptp_ip_mcast_adr[1] : ptp_eth_mcast_adr[1], 
                                     sizeof(mac_addr_t))) && mac_entry[mac_idx].vid_mac.vid == config_data.conf[inst].clock_init.configured_vid) {
                        break;
                    }
                }
                if (mac_idx >= mac_used) {
                    ++mac_used;
                }
                T_I("mac_used %d, mac_idx %d", mac_used, mac_idx);
                memcpy(&mac_entry[mac_idx].vid_mac.mac,  /* match 1588 multicast Dest address */
                       rules[inst].protocol == PTP_PROTOCOL_IP4MULTI ? ptp_ip_mcast_adr[1] : ptp_eth_mcast_adr[1], 
                       sizeof(mac_addr_t));
                mac_entry[mac_idx].vid_mac.vid = config_data.conf[inst].clock_init.configured_vid;
                
                mac_entry[mac_idx].copy_to_cpu = TRUE;
            }
        }
        /* now apply the calculated mac teble entries */
        for (mac_idx = 0; mac_idx < mac_used; mac_idx++) {
            port_iter_init_local(&pit);
            idx = 0;
            dest_txt[0] = 0;
            while (port_iter_getnext(&pit)) {
                if (mac_entry[mac_idx].destination[pit.iport]) {
                    idx += snprintf(&dest_txt[idx], sizeof(dest_txt)-idx, "%d, ", pit.uport);
                }
            }
            T_I("Adding mac table entry: mac %s, vid %d, copy to cpu %d, destinations %s", misc_mac2str(mac_entry[mac_idx].vid_mac.mac.addr),
                mac_entry[mac_idx].vid_mac.vid, mac_entry[mac_idx].copy_to_cpu, dest_txt);
            mac_entry[mac_idx].locked = TRUE;
            mac_entry[mac_idx].aged = FALSE;
#if defined(VTSS_FEATURE_MAC_CPU_QUEUE)
            mac_entry[mac_idx].cpu_queue = PACKET_XTR_QU_BPDU;
#endif /* VTSS_FEATURE_MAC_CPU_QUEUE */
#if defined(VTSS_FEATURE_VSTAX_V2)
            mac_entry[mac_idx].vstax2.enable = FALSE;
#endif /* VTSS_FEATURE_VSTAX_V2 */
            PTP_RC(vtss_mac_table_add(NULL, &mac_entry[mac_idx]));
        }
    }
    

    if (refresh_inst_0) {
        PTP_RC(ptp_ace_update(0));
    }
    return rc;
}

/****************************************************************************/
/*  Allocated PHY Timestamp functions                                       */
/****************************************************************************/

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
typedef struct {
    BOOL                        phy_ts_port;  /* TRUE if this port is a PTP port and has the PHY timestamp feature */
    vtss_phy_ts_engine_t        engine_id;   /* VTSS_PHY_TS_ENGINE_ID_INVALID indicates that no engine is allocated */
    u8                          flow_id_low;  /* identifies the flows allocated for this port */
    u8                          flow_id_high; /* identifies the flows allocated for this port */
} ptp_phy_ts_engine_alloc_t;

typedef struct {
    UInteger8 domain;
    UInteger8 deviceType;
    UInteger8 protocol;
    ptp_phy_ts_engine_alloc_t port[PTP_CLOCK_PORTS];
} ptp_phy_ts_rule_t;

/**
 * \brief PHY timestamp configuration for each PTP instance
 */
static ptp_phy_ts_rule_t phy_ts_rules[PTP_CLOCK_INSTANCES];

static void phy_ts_rules_init(void)
{
    int inst;
    vtss_port_no_t j;
    port_iter_t       pit;
    for (inst = 0; inst < PTP_CLOCK_INSTANCES; inst++) {
        phy_ts_rules[inst].domain = 0;
        phy_ts_rules[inst].deviceType = PTP_DEVICE_NONE;
        phy_ts_rules[inst].protocol = PTP_PROTOCOL_ETHERNET;
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            j = pit.iport;
            phy_ts_rules[inst].port[j].phy_ts_port = FALSE;
            phy_ts_rules[inst].port[j].engine_id = VTSS_PHY_TS_ENGINE_ID_INVALID;
            phy_ts_rules[inst].port[j].flow_id_low = 0;
            phy_ts_rules[inst].port[j].flow_id_high = 0;
        }
    }
}

#ifdef PHY_DATA_DUMP
static void phy_ts_dump(void)
{
    int i,p;
    for (i = 0 ; i < PTP_CLOCK_INSTANCES ; i++) {
        printf("inst %d: domain %d, type %d proto: %d\n", i, phy_ts_rules[i].domain, phy_ts_rules[i].deviceType, phy_ts_rules[i].protocol);
        port_iter_t       pit;
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            p = pit.iport;
            if (phy_ts_rules[i].port[p].phy_ts_port) {
                printf("  port %d: eng_id %d, f_lo: %d f_hi: %d\n", p, 
                       phy_ts_rules[i].port[p].engine_id, 
                       phy_ts_rules[i].port[p].flow_id_low,
                       phy_ts_rules[i].port[p].flow_id_high);
            }
        }
        
    }
}

static void dump_conf(vtss_phy_ts_engine_flow_conf_t *flow_conf)
{
    int i;
    printf("flow conf: eng_mode %d ", flow_conf->eng_mode);
    printf("ptp comm: ppb_en %d, etype %x, tpid %x\n", flow_conf->flow_conf.ptp.eth1_opt.comm_opt.pbb_en,
           flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype,
           flow_conf->flow_conf.ptp.eth1_opt.comm_opt.tpid);
    for (i = 0; i < 8; i++) {
        printf(" channel_map[%d] %d ", i, flow_conf->channel_map[i]);
        printf(" ptp flow: flow_en %d, match_mode %d, match_select %d\n", flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].flow_en,
               flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].addr_match_mode,
               flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].addr_match_select);
        printf(" mac address %s\n",misc_mac2str(flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].mac_addr));
        printf(" vlan_check %d, num_tag %d, outer_tag_type %d, inner_tag_type %d, tag_range_mode %d\n", 
               flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].vlan_check,
               flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].num_tag,
               flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].outer_tag_type,
               flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].inner_tag_type,
               flow_conf->flow_conf.ptp.eth1_opt.flow_opt[i].tag_range_mode);
    }
}

static void dump_ptp_action(vtss_phy_ts_engine_action_t *ptp_action)
{
    int i;
    printf("action_ptp %d\n", ptp_action->action_ptp);
    for (i = 0; i < 2; i++) {
        printf(" action[%d]: enable %d, channel_map %d\n", i, ptp_action->action.ptp_conf[i].enable, ptp_action->action.ptp_conf[i].channel_map);
        printf("  ptpconf: range_en %d, val/upper %d, mask/lower %d\n", ptp_action->action.ptp_conf[i].ptp_conf.range_en,
               ptp_action->action.ptp_conf[i].ptp_conf.domain.value.val,
               ptp_action->action.ptp_conf[i].ptp_conf.domain.value.mask);
        printf("   clk__mode %d, delaym_type %d\n",ptp_action->action.ptp_conf[i].clk_mode,
              ptp_action->action.ptp_conf[i].delaym_type);
    }
}

#endif
#endif


/**
 * \brief Update the PHY timestamp analyzers for a PTP engine.
 * \note This function is called whenever the PTP configuration for a ptp
 *  instance change.
 * The function reserves an Analyzer engine pr. PTP instance.
 * If An analyzer is shared between two ports, and both ports are enabled in the same PTP instance,
 * then the Analyzer is shared between the two ports.
 * The number of flows pr analyzer is variable (analyzer 0 and 1 have 8 flows each, while analyser 2a and 2b
 * shares 8 flows). To simplify the model, 4 flows are reserved for each analyser: 2 for channel 0 and 2 for channel 1.
 * I.e. the port that is connected to channel 0 uses flow 0..1, and
 * the port that is connected to channel 1 uses flow 2..3.
 *
 * \param inst        [IN]    PTP instance number [0..x]
 *
 * \return Return code.
 **/

static vtss_rc ptp_phy_ts_update(int inst)
{
    vtss_rc             rc = VTSS_RC_OK;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)



    int j;
    vtss_phy_ts_encap_t encap_type;
    vtss_port_no_t shared_port;
    u8  flow_id;
    BOOL engine_free = FALSE;
    BOOL no_ptp_action = FALSE;
    phy_ts_rules[inst].deviceType = config_data.conf[inst].clock_init.deviceType;
    phy_ts_rules[inst].protocol = config_data.conf[inst].clock_init.protocol;
    phy_ts_rules[inst].domain = config_data.conf[inst].clock_ds.domainNumber;
    if (phy_ts_rules[inst].protocol == PTP_PROTOCOL_IP4UNI && (ptp_global.my_ip[inst].s_addr == 0)) {
        T_IG(VTSS_TRACE_GRP_PHY_TS,"cannot set up unicast analyzer engine before my ip address is defined");
        return PTP_RC_MISSING_IP_ADDRESS;
    }

    switch (phy_ts_rules[inst].protocol) {
    case PTP_PROTOCOL_ETHERNET:
        encap_type = VTSS_PHY_TS_ENCAP_ETH_PTP;
        break;
    case PTP_PROTOCOL_IP4MULTI:
    case PTP_PROTOCOL_IP4UNI:
        encap_type = VTSS_PHY_TS_ENCAP_ETH_IP_PTP;
        break;
    case PTP_PROTOCOL_OAM:
    case PTP_PROTOCOL_1PPS:
        return VTSS_RC_OK;
    default:
        T_WG(VTSS_TRACE_GRP_PHY_TS,"unsupported encapsulation type");
        return PTP_RC_UNSUPPORTED_PTP_ENCAPSULATION_TYPE;
    }
    T_DG(VTSS_TRACE_GRP_PHY_TS,"setting ts rules for instance %d", inst);

    port_iter_t       pit;
    port_iter_init_local(&pit);
    if (inst == 0 && phy_ts_rules[inst].deviceType == PTP_DEVICE_SLAVE_ONLY && 
            (config_data.conf[inst].clock_init.oneWay || vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) &&
            my_tc_instance > 0) {
        /* A oneway slaveonly does not send out events, and it should co-exist with a TC, and may not overwrite the TC Rules */
        no_ptp_action = TRUE;
        T_DG(VTSS_TRACE_GRP_PHY_TS,"no ptp action inst %d", inst);
    }
    while (port_iter_getnext(&pit)) {
        j = pit.iport;
        phy_ts_rules[inst].port[j].phy_ts_port = config_data.conf[inst].port_config[j].initPortState &&
                (port_data[j].topo.ts_feature == VTSS_PTP_TS_PTS && no_ptp_action == FALSE);
        if (phy_ts_rules[inst].port[j].phy_ts_port) {
            T_DG(VTSS_TRACE_GRP_PHY_TS,"allocate engines for port %d", j);
            /* Allocate engines for this instance */
            if (phy_ts_rules[inst].port[j].engine_id == VTSS_PHY_TS_ENGINE_ID_INVALID) { /* if not already allocated */
                phy_ts_rules[inst].port[j].engine_id = tod_phy_eng_alloc(j, encap_type);
                if (phy_ts_rules[inst].port[j].engine_id == VTSS_PHY_TS_ENGINE_ID_INVALID) {
                    T_IG(VTSS_TRACE_GRP_PHY_TS,"No available TS engine for ptp instance %d, port %d", inst, j);
                    phy_ts_rules[inst].port[j].phy_ts_port = FALSE;
                    rc = PTP_RC_MISSING_PHY_TIMESTAMP_RESOURCE;
                    continue;
                }                    


















                PTP_RC(vtss_phy_ts_ingress_engine_init(API_INST_DEFAULT,
                        j,
                        phy_ts_rules[inst].port[j].engine_id,
                        encap_type,
                        0, 3, /* 4 flows are always available (engine 2 can be shared between 2 API engines)  */
                        VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT));

                T_IG(VTSS_TRACE_GRP_PHY_TS,"ing eng init id: %d for port %d, encap_type %d", phy_ts_rules[inst].port[j].engine_id, j, encap_type);
                /* if this engine is shared by an other port, then the other port is also marked as allocated */
                if (port_data[j].topo.port_shared) {
                    shared_port = port_data[j].topo.shared_port_no;
                    phy_ts_rules[inst].port[shared_port].engine_id = phy_ts_rules[inst].port[j].engine_id;
                    T_IG(VTSS_TRACE_GRP_PHY_TS,"shared engine: port %d, sharedPort %d, ing_id %d", 
                         j, shared_port, phy_ts_rules[inst].port[j].engine_id);
                    if (port_data[j].topo.channel_id == 0) {
                        phy_ts_rules[inst].port[j].flow_id_low = 0;
                        phy_ts_rules[inst].port[j].flow_id_high = 1;
                        phy_ts_rules[inst].port[shared_port].flow_id_low = 2;
                        phy_ts_rules[inst].port[shared_port].flow_id_high = 3;
                    } else {
                        phy_ts_rules[inst].port[j].flow_id_low = 2;
                        phy_ts_rules[inst].port[j].flow_id_high = 3;
                        phy_ts_rules[inst].port[shared_port].flow_id_low = 0;
                        phy_ts_rules[inst].port[shared_port].flow_id_high = 1;
                    }
                } else {
                    T_IG(VTSS_TRACE_GRP_PHY_TS,"non shared engine: port %d, ing_id %d", 
                         j, phy_ts_rules[inst].port[j].engine_id);
                    phy_ts_rules[inst].port[j].flow_id_low = 0;
                    phy_ts_rules[inst].port[j].flow_id_high = 3;
                }

















                PTP_RC(vtss_phy_ts_egress_engine_init(API_INST_DEFAULT,
                                                      j,
                                                      phy_ts_rules[inst].port[j].engine_id,
                                                      encap_type,
                                                      0, 3, /* 4 flows are always available (engine 2 can be shared between 2 API engines)  */
                                                      VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT));

                T_IG(VTSS_TRACE_GRP_PHY_TS,"eg eng init id: %d for port %d, encap_type %d", phy_ts_rules[inst].port[j].engine_id, j, encap_type);

            }
            /* Set up flow comparators */
            vtss_phy_ts_engine_flow_conf_t flow_conf;













            PTP_RC(vtss_phy_ts_ingress_engine_conf_get(API_INST_DEFAULT,
                    j,
                    phy_ts_rules[inst].port[j].engine_id,
                    &flow_conf));

#ifdef PHY_DATA_DUMP
            T_WG(VTSS_TRACE_GRP_PHY_TS,"conf dump before:");
            dump_conf(&flow_conf);
#endif
            T_NG(VTSS_TRACE_GRP_PHY_TS,"get ing engine conf: %d", phy_ts_rules[inst].port[j].engine_id);
            /* modify flow configuration */
            flow_conf.eng_mode = TRUE;

            for (flow_id = phy_ts_rules[inst].port[j].flow_id_low; flow_id <= phy_ts_rules[inst].port[j].flow_id_high; flow_id++) {
                flow_conf.channel_map[flow_id] = port_data[j].topo.channel_id == 0 ?
                                              VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1;
                flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].flow_en = TRUE;
                flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].addr_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_MULTICAST;
                flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR;
                memcpy(flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].mac_addr,  /* match 1588 multicast address */
                       ptp_eth_mcast_adr[flow_id-phy_ts_rules[inst].port[j].flow_id_low], sizeof(mac_addr_t));
                flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].vlan_check = FALSE;
                T_DG(VTSS_TRACE_GRP_PHY_TS,"eth: flow_opt[%d]: channel_map %d, flow_en %d, match_mode %d, match:_sel %d", 
                     flow_id, 
                     flow_conf.channel_map[flow_id],
                     flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].flow_en, 
                     flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].addr_match_mode,
                     flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].addr_match_select);

                if (encap_type == VTSS_PHY_TS_ENCAP_ETH_IP_PTP) {
                    memcpy(flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].mac_addr, 
                           ptp_ip_mcast_adr[flow_id-phy_ts_rules[inst].port[j].flow_id_low], sizeof(mac_addr_t));
                    flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].flow_en = TRUE;
                    flow_conf.flow_conf.ptp.ip1_opt.comm_opt.ip_mode = VTSS_PHY_TS_IP_VER_4;
                    flow_conf.flow_conf.ptp.ip1_opt.comm_opt.dport_val = PTP_EVENT_PORT;
                    flow_conf.flow_conf.ptp.ip1_opt.comm_opt.dport_mask = 0xffff;
                    flow_conf.flow_conf.ptp.ip1_opt.comm_opt.sport_val = 0;
                    flow_conf.flow_conf.ptp.ip1_opt.comm_opt.sport_mask = 0;
                    if (phy_ts_rules[inst].protocol == PTP_PROTOCOL_IP4MULTI) {
                        /* multicast configuration */
                        flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_DEST;
                        flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].ip_addr.ipv4.addr = 0;
                        flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].ip_addr.ipv4.mask = 0; /* match any IP address */
                    } else {
                        /* unicast configuration */
                        /* Match any mac address */
                        /* Match src IP address at egress */
                        /* Match dst IP address at ingress */
                        flow_conf.flow_conf.ptp.eth1_opt.flow_opt[flow_id].addr_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST;
                        flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_DEST;
                        flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].ip_addr.ipv4.addr = ptp_global.my_ip[inst].s_addr;
                        flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].ip_addr.ipv4.mask = 0xffffffff; /* match my own IP address */
                    }                    
                    T_DG(VTSS_TRACE_GRP_PHY_TS,"ip: flow_opt[%d]: channel_map %d, flow_en %d, match_mode %d, ip_addr %X(%X)", 
                         flow_id, 
                         flow_conf.channel_map[flow_id],
                         flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].flow_en, 
                         flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].match_mode,
                         flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].ip_addr.ipv4.addr,
                         flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].ip_addr.ipv4.mask);
                }
            }
            /* the same configuration is applied to both ingress and egress, except IP_MATCH in unicast configs  */
#ifdef PHY_DATA_DUMP
            T_WG(VTSS_TRACE_GRP_PHY_TS,"conf dump after:");
            dump_conf(&flow_conf);
#endif













            PTP_RC(vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT,
                    j,
                    phy_ts_rules[inst].port[j].engine_id,
                    &flow_conf));

            T_NG(VTSS_TRACE_GRP_PHY_TS,"set ing engine conf: port %d, id %d", j, phy_ts_rules[inst].port[j].engine_id);
            if (phy_ts_rules[inst].protocol == PTP_PROTOCOL_IP4UNI) {
                for (flow_id = phy_ts_rules[inst].port[j].flow_id_low; flow_id <= phy_ts_rules[inst].port[j].flow_id_high; flow_id++) {
                    flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC;
                }
                if (port_data[j].topo.port_shared) {
                    shared_port = port_data[j].topo.shared_port_no;
                    for (flow_id = phy_ts_rules[inst].port[shared_port].flow_id_low; flow_id <= phy_ts_rules[inst].port[shared_port].flow_id_high; flow_id++) {
                        flow_conf.flow_conf.ptp.ip1_opt.flow_opt[flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC;
                        T_DG(VTSS_TRACE_GRP_PHY_TS,"also set egress match mode for shared port, port %d, shared_port %d, flow_id %d", j, shared_port, flow_id);
                    }
                }
            }













            PTP_RC(vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT,
                    j,
                    phy_ts_rules[inst].port[j].engine_id,
                    &flow_conf));

            T_NG(VTSS_TRACE_GRP_PHY_TS,"set eg engine conf: %d", phy_ts_rules[inst].port[j].engine_id);

            /* set up TX FIFO signature */







            PTP_RC(vtss_phy_ts_fifo_sig_set(API_INST_DEFAULT, j, my_sig_mask));

            T_NG(VTSS_TRACE_GRP_PHY_TS,"tx_fifo_signature set, port: %d, mask: %x", j, my_sig_mask);
            
            /* set up actions */
            vtss_phy_ts_engine_action_t ptp_action;













            PTP_RC(vtss_phy_ts_ingress_engine_action_get(API_INST_DEFAULT,
                    j,
                    phy_ts_rules[inst].port[j].engine_id,
                    &ptp_action));

            T_NG(VTSS_TRACE_GRP_PHY_TS,"get ing action: %d", phy_ts_rules[inst].port[j].engine_id);
            ptp_action.action_ptp = TRUE;
            ptp_action.action.ptp_conf[0].enable = TRUE;
            ptp_action.action.ptp_conf[0].channel_map |= port_data[j].topo.channel_id == 0 ?
                                             VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1;
            
            ptp_action.action.ptp_conf[0].ptp_conf.range_en = TRUE;
            if (phy_ts_rules[inst].deviceType == PTP_DEVICE_ORD_BOUND ||
                    phy_ts_rules[inst].deviceType == PTP_DEVICE_MASTER_ONLY ||
                    phy_ts_rules[inst].deviceType == PTP_DEVICE_SLAVE_ONLY) {
                ptp_action.action.ptp_conf[0].ptp_conf.domain.range.upper = phy_ts_rules[inst].domain;
                ptp_action.action.ptp_conf[0].ptp_conf.domain.range.lower = phy_ts_rules[inst].domain;
                ptp_action.action.ptp_conf[0].clk_mode = config_data.conf[inst].clock_init.twoStepFlag ?
                        VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP : VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP;
            } else {
                ptp_action.action.ptp_conf[0].ptp_conf.domain.range.upper = 0xff;
                ptp_action.action.ptp_conf[0].ptp_conf.domain.range.lower = 0x00;
                ptp_action.action.ptp_conf[0].clk_mode = config_data.conf[inst].clock_init.twoStepFlag ?
                        VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP : VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP;
            }

            ptp_action.action.ptp_conf[0].delaym_type = config_data.conf[inst].port_config[j].delayMechanism == DELAY_MECH_E2E ?
                    VTSS_PHY_TS_PTP_DELAYM_E2E : VTSS_PHY_TS_PTP_DELAYM_P2P;
            ptp_action.action.ptp_conf[1].enable = FALSE;

            T_DG(VTSS_TRACE_GRP_PHY_TS,"action[%d]: range_en %d, upper %d, lower %d, clk_mode %d, delaym_type %d",
                 0,
                 ptp_action.action.ptp_conf[0].ptp_conf.range_en,
                 ptp_action.action.ptp_conf[0].ptp_conf.domain.range.upper,
                 ptp_action.action.ptp_conf[0].ptp_conf.domain.range.lower,
                 ptp_action.action.ptp_conf[0].clk_mode,
                 ptp_action.action.ptp_conf[0].delaym_type);
#ifdef PHY_DATA_DUMP
            dump_ptp_action(&ptp_action);
#endif













            PTP_RC(vtss_phy_ts_ingress_engine_action_set(API_INST_DEFAULT,
                    j,
                    phy_ts_rules[inst].port[j].engine_id,
                    &ptp_action));

            T_NG(VTSS_TRACE_GRP_PHY_TS,"set ing action: %d", phy_ts_rules[inst].port[j].engine_id);














            PTP_RC(vtss_phy_ts_egress_engine_action_set(API_INST_DEFAULT,
                    j,
                    phy_ts_rules[inst].port[j].engine_id,
                    &ptp_action));

            T_NG(VTSS_TRACE_GRP_PHY_TS,"set eg action: %d", phy_ts_rules[inst].port[j].engine_id);

        } else {
            /* not used by this port: is it used by the shared port ? */
            engine_free = TRUE;
            if (port_data[j].topo.port_shared) {
                shared_port = port_data[j].topo.shared_port_no;
                if (phy_ts_rules[inst].port[shared_port].phy_ts_port) {
                    engine_free = FALSE;
                }
            } else {
                shared_port = 0;
            }
            if (engine_free && phy_ts_rules[inst].port[j].engine_id != VTSS_PHY_TS_ENGINE_ID_INVALID) { /* if not used any more */









                PTP_RC(vtss_phy_ts_ingress_engine_clear(API_INST_DEFAULT, j, phy_ts_rules[inst].port[j].engine_id));
                PTP_RC(vtss_phy_ts_egress_engine_clear(API_INST_DEFAULT, j, phy_ts_rules[inst].port[j].engine_id));

                tod_phy_eng_free(j,phy_ts_rules[inst].port[j].engine_id);
                T_IG(VTSS_TRACE_GRP_PHY_TS,"eng free id: %d for port %d", phy_ts_rules[inst].port[j].engine_id, j);
                phy_ts_rules[inst].port[j].engine_id = VTSS_PHY_TS_ENGINE_ID_INVALID;
            }
            if (engine_free && port_data[j].topo.port_shared) {
                phy_ts_rules[inst].port[shared_port].engine_id = VTSS_PHY_TS_ENGINE_ID_INVALID;
            }
        }
    }
#ifdef PHY_DATA_DUMP
    phy_ts_dump();
#endif
#endif
    return rc;
}

/****************************************************************************/
/*  port state functions                                                    */
/****************************************************************************/
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
/*
 * Port in-sync state change indication
 */
static void
ptp_in_sync_callback(vtss_port_no_t port_no,
                     BOOL in_sync)
{
    int i;
    PTP_CORE_LOCK();
    if (PTP_READY()) {
        port_data[port_no].topo.port_ts_in_sync = in_sync;
            for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
                if (vtss_ptp_port_linkstate(ptp_global.ptpi[i], ptp_l2port_to_api(port_no), 
                        port_data[port_no].link_state && port_data[port_no].topo.port_ts_in_sync && port_data[port_no].vlan_forw[i])) {
                    T_IG(_I, "port_no: %d %s", ptp_l2port_to_api(port_no), in_sync ? "in_sync" : "out_of_sync");
                } else {
                    T_WG(_I, "invalid port_no: %d", ptp_l2port_to_api(port_no));
                }
            }
    } else {
        T_WG(_I, "PTP not ready");
    }
    PTP_CORE_UNLOCK();
}
#endif
/*
 * Port filter change indication
 */
static void
ptp_port_filter_change_callback(int instance, vtss_port_no_t port_no, BOOL forward)
{
    port_data[port_no].vlan_forw[instance] = forward;
    if (vtss_ptp_port_linkstate(ptp_global.ptpi[instance], ptp_l2port_to_api(port_no), 
            port_data[port_no].link_state && port_data[port_no].topo.port_ts_in_sync && port_data[port_no].vlan_forw[instance])) {
        T_IG(_I, "port_no: %d filter %s", ptp_l2port_to_api(port_no), port_data[port_no].vlan_forw[instance] ? "Forward" : "Discard");
    } else {
        T_WG(_I, "invalid port_no: %d", ptp_l2port_to_api(port_no));
    }
}

/*
 * Port state change indication
 */
static void
ptp_port_state_change_callback(vtss_isid_t isid,
                               vtss_port_no_t port_no,
                               port_info_t *info)
{
    int i;
    PTP_CORE_LOCK();
    if (PTP_READY()) {
        if (!info->stack) {
            port_data[port_no].link_state = info->link;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
            port_data_conf_set(port_no, info->link); /* check if the port has a 1588 PHY */
#endif
            for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
                if (VTSS_RC_OK != ptp_phy_ts_update(i)) {
                    T_IG(_I, "phy ts initialization failed: %d", ptp_l2port_to_api(port_no));
                }
               if (vtss_ptp_port_linkstate(ptp_global.ptpi[i], ptp_l2port_to_api(port_no), 
                        port_data[port_no].link_state && port_data[port_no].topo.port_ts_in_sync && port_data[port_no].vlan_forw[i])) {
                    T_IG(_I, "port_no: %d link %s", ptp_l2port_to_api(port_no), info->link ? "up" : "down");
                } else {
                    T_WG(_I, "invalid port_no: %d", ptp_l2port_to_api(port_no));
                }
            }
        } else {
            T_WG(_I, "PTP does not support stackables (%d)", info->stack);
        }
    } else {
        /* port state may change before PTP process is ready */
        T_IG(_I, "PTP not ready");
    }
    PTP_CORE_UNLOCK();
}

/*
 * Set initial port link down state
 */
static void
ptp_port_link_state_initial(int instance)
{
    vtss_port_no_t portidx;
    port_status_t  ps;
    for (portidx = 0; portidx < config_data.conf[instance].clock_init.portCount; portidx++) {
        if (!(msg_switch_exists(VTSS_ISID_START) && port_mgmt_status_get(VTSS_ISID_START, portidx, &ps) == VTSS_OK)) {
            ps.status.link = FALSE;
            T_NG(_I, "switch %d does not exist", VTSS_ISID_START);
        }
        if (port_data[portidx].link_state != ps.status.link) {
            port_data[portidx].link_state = ps.status.link;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
            port_data_conf_set(portidx, ps.status.link); /* check if the port has a 1588 PHY */
#endif
        }
        if (!vtss_ptp_port_linkstate(ptp_global.ptpi[instance], ptp_l2port_to_api(portidx), 
                port_data[portidx].link_state && port_data[portidx].topo.port_ts_in_sync && port_data[portidx].vlan_forw[instance])) {
            T_EG(_I, "invalid port_no: %d", ptp_l2port_to_api(portidx));
        }
    }
}

static vtss_ptp_clock_mode_t my_mode = VTSS_PTP_CLOCK_FREERUN;

void vtss_local_clock_mode_set(vtss_ptp_clock_mode_t mode)
{
    int i;
    vtss_port_no_t portidx;
    if (mode != my_mode) {
        my_mode = mode;
        
        for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
            for (portidx = 0; portidx < config_data.conf[i].clock_init.portCount; portidx++) {
                if (!vtss_ptp_port_internal_linkstate(ptp_global.ptpi[i], ptp_l2port_to_api(portidx))) {
                    T_EG(_I, "invalid port_no: %d", ptp_l2port_to_api(portidx));
                }
            }
        }
        T_IG(_C, "Clock mode %d", my_mode);
    }
}
void vtss_local_clock_mode_get(vtss_ptp_clock_mode_t *mode)
{
    *mode = my_mode;
}

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
/*
 * PTP Timestamp update handler
 */
static void timestamp_interrupt_handler(vtss_interrupt_source_t     source_id,
                                        u32                         instance_id)
{

    T_DG(_S, "New timestamps detected: source_id %d, instance_id %u", source_id, instance_id);
    PTP_RC(vtss_tx_timestamp_update(0));


    PTP_RC(vtss_interrupt_source_hook_set(timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_CLK_TSTAMP,
                                          INTERRUPT_PRIORITY_NORMAL));
}
#endif


#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)


static i32 one_pps_min, one_pps_max, one_pps_mean;
static i32 one_pps_count;
static BOOL one_pps_los;
static BOOL one_pps_input_active;

#define ONE_SEC_PERIOD  (1000/PTP_SCHEDULER_RATE)

static void one_pps_external_input_servo_init(BOOL start);

static void vtss_one_pps_external_input_timer(BOOL clear)
{
    static int one_pps_timer = 0, one_pps_period = 0;
    if (clear) {
        one_pps_timer = 0;
    } else {
        if (one_pps_input_active && !one_pps_los) {
            one_pps_timer++;
            if (one_pps_timer > 2*ONE_SEC_PERIOD) {
                one_pps_period = 0;
                one_pps_los = TRUE;
            }
        }
        if (one_pps_input_active && one_pps_los) {
            one_pps_timer++;
            one_pps_period++;
            if (one_pps_period > 3*ONE_SEC_PERIOD && one_pps_timer < 1*ONE_SEC_PERIOD) {
                one_pps_timer = 0;
                one_pps_los = FALSE;
                one_pps_external_input_servo_init(TRUE);
            }
        }
    }
}

static void vtss_one_pps_external_input_statistic_clear(void)
{
    static const i32 imax = 0x7fffffff;
    T_IG(VTSS_TRACE_GRP_1_PPS, "Clear One pps statistic");
    one_pps_min = imax;
    one_pps_max = -imax;
    one_pps_mean = 0;
    one_pps_count = 0;
}

void vtss_one_pps_external_input_statistic_get(vtss_ptp_one_pps_statistic_t *one_pps, BOOL clear)
{
    i32 mean = 0;
    PTP_CORE_LOCK();
    if (one_pps != NULL) {
        if (one_pps_count > 0) mean = one_pps_mean/one_pps_count;
        one_pps->min = one_pps_min;
        one_pps->mean = mean;
        one_pps->max = one_pps_max;
        one_pps->dLos = one_pps_los;
    }
    if (clear) {
        vtss_one_pps_external_input_statistic_clear();
    }
    PTP_CORE_UNLOCK();
}

/*
 * 1 pps input servo functions
 */
#define MEASURING 0
#define PAUSING 1
#define PPS_MEASURE_PERIOD 10
#define PPS_PAUSE_PERIOD 3

static i32 one_pps_servo_pps_mean;
static i32 one_pps_servo_count;
static int one_pps_servo_controller_state;
static i32 one_pps_servo_ratio;
static BOOL one_pps_first_time;

static void one_pps_external_input_servo_init(BOOL start)
{
    one_pps_servo_pps_mean = 0;
    one_pps_servo_count = 0;
    one_pps_servo_controller_state = PAUSING;
    one_pps_servo_ratio = 0;
    vtss_local_clock_ratio_clear(0);
    one_pps_first_time = TRUE;
    one_pps_input_active = start;
}

static void one_pps_external_input_servo_action(i32                         freq_offset)
{
    if (one_pps_first_time) {
        one_pps_first_time = FALSE; /* ignore the first reading (also if the master time has changed), as it will have some random value */
        one_pps_servo_pps_mean = 0;
        one_pps_servo_count = 0;
        one_pps_servo_controller_state = PAUSING;
        return;
    } else {
        if (freq_offset < one_pps_min) one_pps_min = freq_offset;
        if (freq_offset > one_pps_max) one_pps_max = freq_offset;
        one_pps_mean += freq_offset;
        ++one_pps_count;
        T_DG(VTSS_TRACE_GRP_1_PPS, "Freq Offset:  %d", freq_offset);
    }
    if (!one_pps_los) {
        switch (one_pps_servo_controller_state) {
        case MEASURING:
            one_pps_servo_pps_mean += freq_offset;
            ++one_pps_servo_count;
            if (one_pps_servo_count == PPS_MEASURE_PERIOD) {
                one_pps_servo_ratio -= one_pps_servo_pps_mean/one_pps_servo_count;
                vtss_local_clock_ratio_set(one_pps_servo_ratio, 0);
                T_DG(VTSS_TRACE_GRP_1_PPS, "pps_mean: %d, ratio: %d", one_pps_servo_pps_mean/one_pps_servo_count, one_pps_servo_ratio);
                one_pps_servo_pps_mean = 0;
                one_pps_servo_count = 0;
                one_pps_servo_controller_state = PAUSING;
            }
            break;
        case PAUSING:
            ++one_pps_servo_count;
            if (one_pps_servo_count == PPS_PAUSE_PERIOD) {
                one_pps_servo_count = 0;
                one_pps_servo_controller_state = MEASURING;
            }
            break;
        default:
            T_WG(VTSS_TRACE_GRP_1_PPS, "invalid controller state: %d", one_pps_servo_controller_state);
            break;

        }
    }
    vtss_one_pps_external_input_timer(TRUE);
}

static void one_pps_external_input_interrupt_handler(vtss_interrupt_source_t     source_id,
        u32                         instance_id)
{
    i32 offset;
#if defined(VTSS_ARCH_SERVAL)
    u32 turnaround_latency;
    vtss_timestamp_t t_next_pps;
#endif /* defined(VTSS_ARCH_SERVAL) */
    T_NG(VTSS_TRACE_GRP_1_PPS, "One sec external clock event: source_id %d, instance_id %u", source_id, instance_id);
    
#if defined(VTSS_ARCH_SERVAL)
    PTP_RC(vtss_ts_alt_clock_saved_get(NULL, &turnaround_latency));
    T_IG(VTSS_TRACE_GRP_1_PPS, "Saved turn around or onesec counter value: %u", turnaround_latency);
#endif /* defined(VTSS_ARCH_SERVAL) */
    PTP_RC(vtss_ts_freq_offset_get(0,&offset));
    PTP_CORE_LOCK();
#if defined(VTSS_ARCH_SERVAL)
    /* main mode */
    if (config_data.init_ext_clock_mode_rs422.mode == VTSS_PTP_RS422_MAIN_AUTO) {
        PTP_RC(vtss_ts_timeofday_next_pps_get(NULL, &t_next_pps));
        T_DG(VTSS_TRACE_GRP_1_PPS, "Fake send time at next PS to the sub module");
        t_next_pps.nanoseconds = turnaround_latency/2 + 19; /* 19 ns is the additional delay in the path to the SUB module */
        if (config_data.init_ext_clock_mode_rs422.proto == VTSS_PTP_RS422_PROTOCOL_SER) {
            ptp_1pps_msg_send(&t_next_pps);
        } else if (config_data.init_ext_clock_mode_rs422.proto == VTSS_PTP_RS422_PROTOCOL_PIM) {
            ptp_pim_1pps_msg_send(config_data.init_ext_clock_mode_rs422.port, &t_next_pps);
        }
    }
    /* sub mode */
    if (config_data.init_ext_clock_mode_rs422.mode == VTSS_PTP_RS422_SUB) {
        T_DG(VTSS_TRACE_GRP_1_PPS, "Make frequency adjustment");
        one_pps_external_input_servo_action(offset);
    }
#else
    one_pps_external_input_servo_action(offset);
#endif /* defined(VTSS_ARCH_SERVAL) */
    PTP_CORE_UNLOCK();
    PTP_RC(vtss_interrupt_source_hook_set(one_pps_external_input_interrupt_handler,
                                          INTERRUPT_SOURCE_EXT_SYNC,
                                          INTERRUPT_PRIORITY_NORMAL));
}
#else
void vtss_one_pps_external_input_statistic_get(vtss_ptp_one_pps_statistic_t *one_pps, BOOL clear)
{
    if (one_pps != NULL) {
        one_pps->min = 0;
        one_pps->mean = 0;
        one_pps->max = 0;
        one_pps->dLos = FALSE;
    }
}

#endif

#if !defined(VTSS_ARCH_LUTON28)
static void ptp_timer_expired(vtss_timer_t *timer)
{
    cyg_flag_setbits(&ptp_global_control_flags, CTLFLAG_PTP_TIMER);
}

static void init_timer(void) {
    // Create ptp timer
    static vtss_timer_t timer;
    vtss_ptp_timer_initialize();
    if (vtss_timer_initialize(&timer) != VTSS_RC_OK) {
        T_EG(VTSS_TRACE_GRP_PTP_BASE_TIMER, "Unable to initialize ptp timer");
    }
    timer.repeat      = TRUE;
    timer.period_us   = 7813;     /* 7813 us = 1/128 sec */
    timer.dsr_context = FALSE;
    timer.callback    = ptp_timer_expired;
    timer.prio        = VTSS_TIMER_PRIO_HIGH;
    timer.modid       = VTSS_MODULE_ID_PTP;
    if (vtss_timer_start(&timer) != VTSS_RC_OK) {
        T_EG(VTSS_TRACE_GRP_PTP_BASE_TIMER, "Unable to start ptp timer");
    }
}
#endif /*!defined(VTSS_ARCH_LUTON28) */


/*
 * TOD Synchronization 1 pps pulse update handler
 */
#if defined(VTSS_ARCH_SERVAL)
static void one_pps_pulse_interrupt_handler(vtss_interrupt_source_t     source_id,
        u32                         instance_id)
{
    vtss_timestamp_t t_next_pps;
    T_NG(VTSS_TRACE_GRP_1_PPS, "One sec internal clock event: source_id %d, instance_id %u", source_id, instance_id);
    PTP_CORE_LOCK();
    if (config_data.init_ext_clock_mode_rs422.mode == VTSS_PTP_RS422_MAIN_MAN) {
        PTP_RC(vtss_ts_timeofday_next_pps_get(NULL, &t_next_pps));
        t_next_pps.nanoseconds = config_data.init_ext_clock_mode_rs422.delay; /* Delay is manually configured */
        if (config_data.init_ext_clock_mode_rs422.proto == VTSS_PTP_RS422_PROTOCOL_SER) {
            ptp_1pps_msg_send(&t_next_pps);
        } else if (config_data.init_ext_clock_mode_rs422.proto == VTSS_PTP_RS422_PROTOCOL_PIM) {
            ptp_pim_1pps_msg_send(config_data.init_ext_clock_mode_rs422.port, &t_next_pps);
        }
    
        PTP_RC(vtss_interrupt_source_hook_set(one_pps_pulse_interrupt_handler,
                                              INTERRUPT_SOURCE_SYNC,
                                              INTERRUPT_PRIORITY_NORMAL));
    }
    PTP_CORE_UNLOCK();
}
#endif



/****************************************************************************
 * Module thread
 ****************************************************************************/

static void
ptp_thread(cyg_addrword_t data)
{
    vtss_rc rc;
    packet_rx_filter_t rx_filter;
    static void *filter_id_ether = NULL;
    static void *filter_id_udp = NULL;
    int i, j;
    vtss_restart_status_t status;
    BOOL cold = TRUE;
	cyg_flag_value_t flags;
    cyg_tick_count_t wakeup = cyg_current_time();

    if (vtss_restart_status_get(NULL, &status) == VTSS_RC_OK) {
        if (status.restart != VTSS_RESTART_COLD) {
            cold = FALSE;
        }
    }
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
	/* wait until TOD module is ready */
    wakeup = cyg_current_time() + (100/ECOS_MSECS_PER_HWTICK);
	while (!tod_ready()) {
		T_I("wait until TOD module is ready");
		flags = cyg_flag_timed_wait(&ptp_global_control_flags, 0xffff, 0, wakeup);
		wakeup += (100/ECOS_MSECS_PER_HWTICK);
	}
#endif
	
    PTP_CORE_LOCK();
    /* initialize PTP timer system */
#if !defined(VTSS_ARCH_LUTON28)
    init_timer();
#endif /* !defined(VTSS_ARCH_LUTON28) */
    
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    /* Initialize PHY ts rules */
    phy_ts_rules_init();
#endif
    /* Initialize port data */
    port_data_initialize();

    /* Read configuration */
    (void) conf_mgmt_mac_addr_get(ptp_global.sysmac, 0);
    T_I("Read sysmac %02x-%02x-%02x-%02x-%02x-%02x",ptp_global.sysmac[0],
        ptp_global.sysmac[1],ptp_global.sysmac[1],ptp_global.sysmac[3],
        ptp_global.sysmac[4],ptp_global.sysmac[5] );

    /* Local clock initialization */
    for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
        vtss_local_clock_initialize(i, &config_data.init_synce, cold, config_data.init_ext_clock_mode.vcxo_enable);
    }
    if (cold) {
        ptp_conf_save(); // save the initial value from clock initialize
    }

    PTP_RC(vtss_ip2_if_callback_add(notify_ptp_ip_address));
    ptp_socket_init();

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    PTP_RC(vtss_interrupt_source_hook_set(timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_CLK_TSTAMP,
                                          INTERRUPT_PRIORITY_NORMAL));
#endif
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    PTP_RC(vtss_interrupt_source_hook_set(one_pps_external_input_interrupt_handler,
                                          INTERRUPT_SOURCE_EXT_SYNC,
                                          INTERRUPT_PRIORITY_NORMAL));

    vtss_one_pps_external_input_statistic_clear(); /* clear statistics counters */
#endif

#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
    vtss_init_ptp_timer(&oam_timer, ptp_oam_slave_timer, ptp_global.ptpi[0]);
#endif /* defined(VTSS_ARCH_SERVAL) */
    vtss_init_ptp_timer(&one_pps_sync_timer, ptp_one_pps_slave_timer, ptp_global.ptpi[0]);

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
	if (!tod_tc_mode_get(&phy_ts_mode)) {
		T_W("Missed tc_mode_get");
	}
#endif
	
    /* Ethernet PTP frames registration */
    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid = VTSS_MODULE_ID_PTP;
    rx_filter.match = PACKET_RX_FILTER_MATCH_ETYPE; // only ethertype filter
    rx_filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
    rx_filter.contxt= ptp_global.ptpi;
    rx_filter.cb_adv= packet_rx;
    rx_filter.etype = ptp_ether_type;
    rc = packet_rx_filter_register(&rx_filter, &filter_id_ether);
    VTSS_ASSERT(rc == VTSS_OK);
    /* UDP PTP frames registration */
    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid = VTSS_MODULE_ID_PTP;
    rx_filter.match = PACKET_RX_FILTER_MATCH_UDP_DST_PORT; // ethertype, IP protocol and UDP port match filter
    rx_filter.udp_dst_port_min = PTP_EVENT_PORT;
    rx_filter.udp_dst_port_max = PTP_GENERAL_PORT;

    rx_filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
    rx_filter.contxt= ptp_global.ptpi;
    rx_filter.cb_adv= packet_rx;
    rx_filter.etype = ip_ether_type;
    rx_filter.ipv4_proto = IPPROTO_UDP;
    rc = packet_rx_filter_register(&rx_filter, &filter_id_udp);
    VTSS_ASSERT(rc == VTSS_OK);

    /* Port change callback */
    (void) port_global_change_register(VTSS_MODULE_ID_PTP, ptp_port_state_change_callback);

    /* Initialize the clock */
    for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
        initial_port_filter_get(i);
        vtss_ptp_clock_create(ptp_global.ptpi[i]);
        if (config_data.conf[i].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT || config_data.conf[i].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT) {
            transparent_clock_exists = TRUE;
        }
        if (config_data.conf[i].clock_init.deviceType != PTP_DEVICE_NONE) {
            update_ptp_ip_address(i, config_data.conf[i].clock_init.configured_vid);
        }

#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
        if (i == 0 && config_data.conf[i].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY && config_data.conf[i].clock_init.protocol == PTP_PROTOCOL_OAM) {
            vtss_ptp_timer_start(&oam_timer, OAM_TIMER_INIT_VALUE, FALSE);
        }
#endif /* defined(VTSS_ARCH_SERVAL) */

        for (j = 0; j < MAX_UNICAST_MASTERS_PR_SLAVE; j++) {
            vtss_ptp_uni_slave_conf_set(ptp_global.ptpi[i], j, &config_data.conf[i].unicast_slave[j]);
        }
        vtss_ptp_set_clock_slave_config(ptp_global.ptpi[i], &config_data.conf[i].slave_cfg);
    }
    /* initial setup of external clock output */
    ext_clock_out_set(&config_data.init_ext_clock_mode);


#if defined (VTSS_ARCH_SERVAL)
    ext_clock_rs422_conf_set(&config_data.init_ext_clock_mode_rs422);
#endif

    T_I("Clock initialized");
    
    ptp_global.ready = TRUE; /* Ready to rock */
    /* start background task */
	cyg_thread_resume(ptp_background_thread_handle);
	/* wait 500 ms, otherwise the ACL setup may fail ! */
    wakeup = cyg_current_time() + (500/ECOS_MSECS_PER_HWTICK);
    flags = cyg_flag_timed_wait(&ptp_global_control_flags, 0xffff, 0, wakeup);
    
    /* Register Initialport state (portstate changes before setting ptp_global.ready are ignored) */
    for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
        ptp_port_link_state_initial(i);
    }
    PTP_CORE_UNLOCK();
    
    for (;;) {
        PTP_CORE_LOCK();
        while (ptp_global.ptpi[0]) {
            T_R( "%s: tick()", __FUNCTION__);
            wakeup += (PTP_SCHEDULER_RATE/ECOS_MSECS_PER_HWTICK);
            PTP_CORE_UNLOCK();
            while ((flags = cyg_flag_timed_wait(&ptp_global_control_flags, 0xffff,
                                                CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, wakeup))) {
                T_R("PTP thread event, flags 0x%x", flags);

                PTP_CORE_LOCK();
                if (flags & CTLFLAG_PTP_DEFCONFIG) {
                    /* no existing trandsparent clock after reset */
                    transparent_clock_exists = FALSE;
                    ptp_conf_read(TRUE); /* Reset configuration */
                    /* Make PTP configuration effective in PTP core */
                    ptp_conf_propagate();
                }
                if (flags & CTLFLAG_PTP_SET_ACL) {
                    T_I("set ACL rules");
                    for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
                        if (ptp_ace_update(i) != VTSS_OK) T_IG(0, "ACE update error");
                    }
                }
#if !defined(VTSS_ARCH_LUTON28)
                if (flags & CTLFLAG_PTP_TIMER) {
                    vtss_ptp_timer_tick(1);
                    T_NG(VTSS_TRACE_GRP_PTP_BASE_TIMER, "ptp timer");
                }
#endif /* !defined(VTSS_ARCH_LUTON28) */
                PTP_CORE_UNLOCK();
            }
            /* timeout */
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            /* timestamp_age calls back to ptp, therefore it is outside the locked region */
            PTP_RC(vtss_timestamp_age(NULL));
#endif
            PTP_CORE_LOCK();

#if defined(VTSS_ARCH_LUTON28)
            vtss_ptp_timer_tick(1);
#endif /* defined(VTSS_ARCH_LUTON28) */
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            vtss_one_pps_external_input_timer(FALSE);
#endif
            for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
                if (config_data.conf[i].clock_init.deviceType != PTP_DEVICE_NONE) {
                    vtss_ptp_tick(ptp_global.ptpi[i]);
                    if (config_data.conf[i].clock_init.protocol != PTP_PROTOCOL_OAM && config_data.conf[i].clock_init.protocol != PTP_PROTOCOL_1PPS) {
                        if (wakeup %(400*(PTP_SCHEDULER_RATE/ECOS_MSECS_PER_HWTICK)) == i*100LL) {
                            T_I("Poll port filter");
                            poll_port_filter(i);
                        }
                    }
                }
            }
            if (wakeup %(2100*(PTP_SCHEDULER_RATE/ECOS_MSECS_PER_HWTICK)) == 0) {
                T_I("socket_init");
                ptp_socket_init(); /* reinitialize socket each 21 sec, to reconnect if the vlan has been down */
            }
        }
        ptp_global.ready = FALSE; /* Done rocking */
        PTP_CORE_UNLOCK();

        /* De-Initialize PTP core */
        for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {

            PTP_CORE_LOCK();
            if (ptp_global.ptpi[i]) {
                vtss_ptp_clock_remove(ptp_global.ptpi[i]);
                vtss_ptp_default_filter_delete(servo[i]);
                vtss_ptp_default_delay_filter_delete(delay_filt[i]);
                if (ptp_ace_update(i) != VTSS_OK) T_EG(0, "ACE update error");
                T_I("ptp_ace_update returned %d", rc);
            }
            PTP_CORE_UNLOCK();
        }
    }
}

/****************************************************************************
 * PTP Clock Background thread
 ****************************************************************************/


static void
ptp_background_thread(cyg_addrword_t data)
{
    cyg_tick_count_t wakeup = cyg_current_time() + (500/ECOS_MSECS_PER_HWTICK);
    cyg_flag_value_t flags;

#if defined TIMEOFDAY_TEST
    tod_test();
#endif
    for (;;) {

        while ((flags = cyg_flag_timed_wait(&bg_thread_flags, 0xffff,
                                            CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR,wakeup))) {
            if (flags) {
                T_D("flag(s) set %x", flags);
            }
        }
        /* timeout */
        T_I("tbd");
        wakeup += (15*60*1000/ECOS_MSECS_PER_HWTICK); /* 15 min */

    }
}



/****************************************************************************/
/*  PTP callout functions                                                  */
/****************************************************************************/
/*
 * calculate difference between two ts counters.
 */
void vtss_1588_ts_cnt_sub(u32 *r, u32 x, u32 y)
{
    vtss_tod_ts_cnt_sub(r, x, y);
}

/*
 * calculate sum of two ts counters.
 */
void vtss_1588_ts_cnt_add(u32 *r, u32 x, u32 y)
{
    vtss_tod_ts_cnt_add(r, x, y);
}

void vtss_1588_timeinterval_to_ts_cnt(u32 *r, vtss_timeinterval_t x)
{
    vtss_tod_timeinterval_to_ts_cnt(r, x);
}

void vtss_1588_ts_cnt_to_timeinterval(vtss_timeinterval_t *r, u32 x)
{
    vtss_tod_ts_cnt_to_timeinterval(r, x);
}


void vtss_1588_ingress_latency_set(u16 portnum,
                                   vtss_timeinterval_t ingress_latency)
{




    if (port_data[ptp_api_to_l2port(portnum)].topo.ts_feature == VTSS_PTP_TS_PTS) {
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)







        PTP_RC(vtss_phy_ts_ingress_latency_set(API_INST_DEFAULT, ptp_api_to_l2port(portnum), &ingress_latency));

        T_DG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, ingress_latency = %lld ns", portnum, ingress_latency>>16);
#else
        T_WG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, PHY timestamping not supported", portnum);
#endif
    } else {
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1)
        u32 ingress_cnt;
        vtss_1588_timeinterval_to_ts_cnt(&ingress_cnt, ingress_latency);
        port_data[ptp_api_to_l2port(portnum)].ingress_cnt = ingress_cnt;
        T_IG(_I, "port %d, ingress_cnt = %d", portnum, ingress_cnt);

#endif
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
        PTP_RC(vtss_ts_ingress_latency_set( 0,
                                            ptp_api_to_l2port(portnum),
                                            &ingress_latency));
#endif
    }
}

void vtss_1588_egress_latency_set(u16 portnum,
                                  vtss_timeinterval_t egress_latency)
{




    if (port_data[ptp_api_to_l2port(portnum)].topo.ts_feature == VTSS_PTP_TS_PTS) {
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)







        PTP_RC(vtss_phy_ts_egress_latency_set(API_INST_DEFAULT, ptp_api_to_l2port(portnum), &egress_latency));

        T_DG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, egress_latency = %lld ns", portnum, egress_latency>>16);
#else
        T_WG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, PHY timestamping not supported", portnum);
#endif
    } else {

#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_JAGUAR_1)
        u32 egress_cnt;
        vtss_1588_timeinterval_to_ts_cnt(&egress_cnt, egress_latency);
        port_data[ptp_api_to_l2port(portnum)].egress_cnt = egress_cnt;
        T_IG(_I, "port %d, egress_cnt = %d", portnum, egress_cnt);
#endif
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
        PTP_RC(vtss_ts_egress_latency_set( 0,
                                           ptp_api_to_l2port(portnum),
                                           &egress_latency));
#endif
    }
}

void vtss_1588_p2p_delay_set(u16 portnum,
                             vtss_timeinterval_t p2p_delay)
{




    if (port_data[ptp_api_to_l2port(portnum)].topo.ts_feature == VTSS_PTP_TS_PTS) {
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)







        PTP_RC(vtss_phy_ts_path_delay_set(API_INST_DEFAULT, ptp_api_to_l2port(portnum), &p2p_delay));

        T_DG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, p2p_delay = %lld ns", portnum, p2p_delay>>16);
#else
        T_WG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, PHY timestamping not supported", portnum);
#endif
    } else {

#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
    u32 delay_cnt;
    vtss_1588_timeinterval_to_ts_cnt(&delay_cnt, p2p_delay);
    port_data[ptp_api_to_l2port(portnum)].delay_cnt = delay_cnt;
    T_IG(_I, "port %d, delay_cnt = %d", portnum, delay_cnt);

#endif
#if defined(VTSS_ARCH_SERVAL)
    PTP_RC(vtss_ts_p2p_delay_set(0, ptp_api_to_l2port(portnum), &p2p_delay));
#endif
    }
}

void vtss_1588_asymmetry_set(u16 portnum,
                             vtss_timeinterval_t asymmetry)
{




    if (port_data[ptp_api_to_l2port(portnum)].topo.ts_feature == VTSS_PTP_TS_PTS) {
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)







        PTP_RC(vtss_phy_ts_delay_asymmetry_set(API_INST_DEFAULT, ptp_api_to_l2port(portnum), &asymmetry));

        T_DG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, asymmetry = %lld ns", portnum, asymmetry>>16);
#else
        T_WG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, PHY timestamping not supported", portnum);
#endif
    } else {

        u32 asy_cnt;
        vtss_1588_timeinterval_to_ts_cnt(&asy_cnt, asymmetry);
        port_data[ptp_api_to_l2port(portnum)].asymmetry_cnt = asy_cnt;
        T_IG(_I, "port %d, asymmetry = %d", portnum, asy_cnt);

#if defined(VTSS_ARCH_SERVAL)
        PTP_RC(vtss_ts_delay_asymmetry_set(0, ptp_api_to_l2port(portnum), &asymmetry));
#endif
    
    }
}

u16 vtss_ptp_get_rand(u32 *seed)
{
    return rand_r((unsigned int*)seed);
}

void *vtss_ptp_malloc(size_t sz)
{
    return VTSS_MALLOC(sz);
}

void *vtss_ptp_calloc(size_t nmemb, size_t sz)
{
    return VTSS_CALLOC(nmemb, sz);
}

void vtss_ptp_free(void *ptr)
{
    VTSS_FREE(ptr);
}

/****************************************************************************/
/*  API functions                                                           */
/****************************************************************************/

BOOL
ptp_clock_create(const ptp_init_clock_ds_t *initData, uint instance)
{
    BOOL ok = FALSE;
    int j;
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
    vtss_chip_id_t  chip_id;
#endif /* defined(VTSS_ARCH_SERVAL) */
    /* after change to icfg, this function is initially called before the PTP application is ready */
    /* in this case we only save the configuration, and does not update the HW */
    PTP_CORE_LOCK();
    T_D("PTP_READY %d, deviceType %d, transparent_clock_exists %d", PTP_READY(), initData->deviceType, transparent_clock_exists);
    if (instance < PTP_CLOCK_INSTANCES) {
        config_data.conf[instance].clock_init.deviceType = initData->deviceType;
        config_data.conf[instance].clock_init.twoStepFlag = initData->twoStepFlag;
        config_data.conf[instance].clock_init.oneWay = initData->oneWay;
        config_data.conf[instance].clock_init.protocol = initData->protocol;
        config_data.conf[instance].clock_init.tagging_enable = initData->tagging_enable;
        config_data.conf[instance].clock_init.configured_vid = initData->configured_vid;
        config_data.conf[instance].clock_init.configured_pcp = initData->configured_pcp;
        memcpy(config_data.conf[instance].clock_init.clockIdentity, initData->clockIdentity, sizeof(config_data.conf[instance].clock_init.clockIdentity));
        T_D("clock init set: portcount = %d, deviceType = %d",config_data.conf[instance].clock_init.portCount, config_data.conf[instance].clock_init.deviceType);
        if (initData->deviceType == PTP_DEVICE_SLAVE_ONLY && initData->protocol == PTP_PROTOCOL_OAM) {
            /* check if OAM instance is active */
            if (instance == 0) {
                config_data.conf[instance].clock_init.mep_instance = initData->mep_instance;
                config_data.conf[instance].clock_ds.priority1 = 255;
            }
        }
        if (PTP_READY()) {
            if (initData->deviceType == PTP_DEVICE_P2P_TRANSPARENT || initData->deviceType == PTP_DEVICE_E2E_TRANSPARENT) {
                if (transparent_clock_exists) {
                    PTP_CORE_UNLOCK();
                    return ok;
                } else
                    transparent_clock_exists = TRUE;
            }
            (void)update_ptp_ip_address(instance, initData->configured_vid);
            if (initData->deviceType == PTP_DEVICE_SLAVE_ONLY && initData->protocol == PTP_PROTOCOL_OAM) {
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
                /* check if chip revision is >= rev B */
                PTP_RC(vtss_chip_id_get(NULL, &chip_id));
                T_D("Chip ID = %x, rev = %d",chip_id.part_number, chip_id.revision);
                if (chip_id.revision < 1) {
                    T_W("OAM protocol only supported for Serval rev B or later");
                } else {
                    /* check if OAM instance is active */
                    if (instance == 0) {
                        vtss_ptp_clock_create(ptp_global.ptpi[instance]);
                        /* start timer for reading OAM timestamps */
                        vtss_ptp_timer_start(&oam_timer, 128, FALSE);
                        ok = TRUE;
                        ptp_conf_save();
                    }
                }
#else
                T_W("OAM protocol only supported for Serval rev B or later, and it depends on the MEP module");
#endif /* defined(VTSS_ARCH_SERVAL) */
            } else if (initData->deviceType == PTP_DEVICE_MASTER_ONLY && initData->protocol == PTP_PROTOCOL_OAM) {
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
                /* check if chip revision is >= rev B */
                PTP_RC(vtss_chip_id_get(NULL, &chip_id));
                T_I("Chip ID = %x, rev = %d",chip_id.part_number, chip_id.revision);
                if (chip_id.revision < 1) {
                    T_W("OAM protocol only supported for Serval rev B or later");
                } else {
                    /* check if OAM instance is active */
                    if (instance == 0) {
                        config_data.conf[instance].clock_init.mep_instance = initData->mep_instance;
                        vtss_ptp_clock_create(ptp_global.ptpi[instance]);
                        ok = TRUE;
                        ptp_conf_save();
                    }
                }
#else
                T_W("OAM protocol only supported for Serval rev B or later, and it depends on the MEP module");
#endif /* defined(VTSS_ARCH_SERVAL) */
            } else {
                if (ptp_ace_update(instance) != VTSS_OK) T_IG(0, "ACE add error");
                if (initData->deviceType == PTP_DEVICE_SLAVE_ONLY) {
                    config_data.conf[instance].clock_ds.priority1 = 255;
                }
                vtss_ptp_clock_create(ptp_global.ptpi[instance]);
                for (j = 0; j < MAX_UNICAST_MASTERS_PR_SLAVE; j++) {
                    vtss_ptp_uni_slave_conf_set(ptp_global.ptpi[instance], j, &config_data.conf[instance].unicast_slave[j]);
                }
                ptp_port_link_state_initial(instance);
            }
            vtss_ptp_set_clock_slave_config(ptp_global.ptpi[instance], &config_data.conf[instance].slave_cfg);
            ok = TRUE;
            ptp_conf_save();
        } else {
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}


BOOL
ptp_clock_delete(uint instance)
{
    BOOL ok = FALSE;
#if defined(VTSS_FEATURE_TIMESTAMP) && defined(VTSS_ARCH_SERVAL)
    vtss_ts_operation_mode_t mode;
    int i;
    port_iter_t       pit;
#endif
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT ||
                config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT) {
            transparent_clock_exists = FALSE;
#if defined(VTSS_FEATURE_TIMESTAMP) && defined(VTSS_ARCH_SERVAL)
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;
                if (config_data.conf[instance].port_config[i].initPortInternal) {
                    mode.mode = TS_MODE_EXTERNAL;
                    PTP_RC(vtss_ts_operation_mode_set(NULL,i,&mode));
                    T_D("set ts_operation mode to %d", mode.mode);
                }
            }
#endif            
        }
        ptp_conf_init(&config_data.conf[instance], instance);
        T_D("clock delete");

        vtss_ptp_clock_create(ptp_global.ptpi[instance]);
        if (ptp_ace_update(instance) != VTSS_OK) T_EG(0, "ACE update error");;
        ok = TRUE;
        ptp_conf_save();
    }
    PTP_CORE_UNLOCK();
    return ok;
}



vtss_rc
ptp_port_ena(BOOL internal, uint portnum, uint instance)
{
    vtss_rc ok = PTP_ERROR_INV_PARAM;
#if defined(VTSS_FEATURE_TIMESTAMP) && defined(VTSS_ARCH_SERVAL)
    vtss_ts_operation_mode_t mode;
#endif
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        if (0 < portnum && portnum <= config_data.conf[instance].clock_init.portCount) {
            config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortInternal = internal;
            config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortState = TRUE;
        }
        if (PTP_READY()) {
            if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
                if (0 < portnum && portnum <= config_data.conf[instance].clock_init.portCount) {
#if defined(VTSS_FEATURE_TIMESTAMP) && defined(VTSS_ARCH_SERVAL)
                    if (config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT ||
                        config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT) {
                        if (internal) {
                            mode.mode = TS_MODE_INTERNAL;
                            PTP_RC(vtss_ts_operation_mode_set(NULL,ptp_api_to_l2port(portnum),&mode));
                            T_D("set ts_operation mode to %d", mode.mode);
                        } else if (config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortInternal) {
                            mode.mode = TS_MODE_EXTERNAL;
                            PTP_RC(vtss_ts_operation_mode_set(NULL,ptp_api_to_l2port(portnum),&mode));
                            T_D("set ts_operation mode to %d", mode.mode);
                        }
                    }
#endif
                    if (config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT ||
                            config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT ||
                        !internal) {
                        config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortInternal = internal;
                        ok = vtss_ptp_port_ena(ptp_global.ptpi[instance], portnum) ? VTSS_RC_OK : PTP_RC_INVALID_PORT_NUMBER;
                        config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortState = TRUE;
                    }
                }
            }
            if (ok == VTSS_RC_OK) {
                if ((ok = ptp_ace_update(instance)) != VTSS_RC_OK) {
                    if (ok != PTP_RC_MISSING_IP_ADDRESS) {
                        if(!vtss_ptp_port_dis(ptp_global.ptpi[instance], portnum)) T_W("port disable failed");
                        config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortState = FALSE;
                        config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortInternal = FALSE;
                    } else {
                        ok = VTSS_RC_OK;
                    }
                }
            }
            if (ok == VTSS_RC_OK) {
                ptp_conf_save();
            }
        } else {
            ok = VTSS_RC_OK;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_port_dis( uint portnum, uint instance)
{
    BOOL ok = FALSE;
#if defined(VTSS_FEATURE_TIMESTAMP) && defined(VTSS_ARCH_SERVAL)
    vtss_ts_operation_mode_t mode;
#endif
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            if (0 < portnum && portnum <= config_data.conf[instance].clock_init.portCount) {
                ok = vtss_ptp_port_dis(ptp_global.ptpi[instance], portnum);
                config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortState = FALSE;
                config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortInternal = FALSE;
#if defined(VTSS_FEATURE_TIMESTAMP) && defined(VTSS_ARCH_SERVAL)
                if (config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_P2P_TRANSPARENT ||
                        config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_E2E_TRANSPARENT) {
                    if (config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortInternal) {
                        mode.mode = TS_MODE_EXTERNAL;
                        PTP_RC(vtss_ts_operation_mode_set(NULL,ptp_api_to_l2port(portnum),&mode));
                        T_D("set ts_operation mode to %d", mode.mode);
                    }
                }
#endif
            }
        }
        if (ok) {
            ptp_conf_save();
            PTP_RC(ptp_ace_update(instance));
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_get_clock_default_ds(ptp_clock_default_ds_t *default_ds, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            vtss_ptp_get_clock_default_ds(ptp_global.ptpi[instance], default_ds);
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_get_clock_set_default_ds(ptp_set_clock_ds_t *default_ds, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            *default_ds = config_data.conf[instance].clock_ds;
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}


void
ptp_get_default_clock_default_ds(ptp_clock_default_ds_t *default_ds)
{
    default_ds->deviceType= PTP_DEVICE_NONE;
    default_ds->twoStepFlag = CLOCK_FOLLOWUP;
    default_ds->protocol = PTP_PROTOCOL_ETHERNET;
    default_ds->oneWay = FALSE;
    PTP_CORE_LOCK();
    memcpy(default_ds->clockIdentity, ptp_global.sysmac, 3);
    default_ds->clockIdentity[3] = 0xff;
    default_ds->clockIdentity[4] = 0xfe;
    memcpy(default_ds->clockIdentity+5, ptp_global.sysmac+3, 3);
    PTP_CORE_UNLOCK();

    default_ds->tagging_enable = FALSE;
    default_ds->configured_vid = 1;
    /* in IPv4 encapsulation mode, the management VLAN id is used */
    default_ds->configured_pcp = 0;
    default_ds->mep_instance = 0;
    
    default_ds->numberPorts = port_isid_port_count(VTSS_ISID_LOCAL);;
    /* dynamic */
    default_ds->clockQuality.clockAccuracy = 0;
    default_ds->clockQuality.clockClass = 0;
    default_ds->clockQuality.offsetScaledLogVariance = 0;
    /* configurable */
    default_ds->priority1 = 128;
    default_ds->priority2 = 128;
    default_ds->domainNumber = 0;
    
}

BOOL
ptp_set_clock_default_ds(const ptp_set_clock_ds_t *default_ds, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        config_data.conf[instance].clock_ds = *default_ds;
        if (PTP_READY()) {
            if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
                vtss_ptp_set_clock_default_ds(ptp_global.ptpi[instance], default_ds);
                ok = TRUE;
            }
            if (ok) {
                ptp_conf_save();
                if (VTSS_RC_OK != ptp_ace_update(instance)) {
                    T_IG(_I,"ptp_ace_update failed");
                }
            }
        } else {
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}


BOOL
ptp_get_clock_slave_config(ptp_clock_slave_cfg_t *cfg, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            *cfg = config_data.conf[instance].slave_cfg;
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_set_clock_slave_config(const ptp_clock_slave_cfg_t *cfg, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            config_data.conf[instance].slave_cfg = *cfg;
            vtss_ptp_set_clock_slave_config(ptp_global.ptpi[instance], cfg);
            ok = TRUE;
        }
        if (ok) {
            ptp_conf_save();
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

void
ptp_get_default_clock_slave_config(ptp_clock_slave_cfg_t *cfg)
{
    cfg->offset_fail = 3000;
    cfg->offset_ok = 1000;
    cfg->stable_offset = 1000;
}

BOOL
ptp_get_clock_current_ds(ptp_clock_current_ds_t *status, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            vtss_ptp_get_clock_current_ds(ptp_global.ptpi[instance], status);
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_get_clock_parent_ds(ptp_clock_parent_ds_t *status, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            vtss_ptp_get_clock_parent_ds(ptp_global.ptpi[instance], status);
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_get_clock_timeproperties_ds(ptp_clock_timeproperties_ds_t *timeproperties_ds, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            vtss_ptp_get_clock_timeproperties_ds(ptp_global.ptpi[instance], timeproperties_ds);
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_get_clock_cfg_timeproperties_ds(ptp_clock_timeproperties_ds_t *timeproperties_ds, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        *timeproperties_ds = config_data.conf[instance].time_prop;
        ok = TRUE;
    }
    PTP_CORE_UNLOCK();
    return ok;
}

void
ptp_get_clock_default_timeproperties_ds(ptp_clock_timeproperties_ds_t *timeproperties_ds)
{
    timeproperties_ds->currentUtcOffset = DEFAULT_UTC_OFFSET;
    timeproperties_ds->currentUtcOffsetValid = FALSE;
    timeproperties_ds->leap59 = FALSE;
    timeproperties_ds->leap61 = FALSE;
    timeproperties_ds->timeTraceable = FALSE;
    timeproperties_ds->frequencyTraceable = FALSE;
    timeproperties_ds->ptpTimescale = TRUE; /* default in ITU profile */
    timeproperties_ds->timeSource = 0xa0; /* indicates internal oscillator */
}

BOOL
ptp_set_clock_timeproperties_ds(const ptp_clock_timeproperties_ds_t *timeproperties_ds, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        config_data.conf[instance].time_prop = *timeproperties_ds;
        if (PTP_READY()) {
            if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
                vtss_ptp_set_clock_timeproperties_ds(ptp_global.ptpi[instance], timeproperties_ds);
                ok = TRUE;
            }
            if (ok) {
                ptp_conf_save();
            }
        } else {
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_get_port_ds(ptp_port_ds_t *ds, int portnum, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            ok = vtss_ptp_get_port_ds(ptp_global.ptpi[instance], portnum, ds);
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_set_port_ds(uint portnum,
                const ptp_set_port_ds_t *port_ds, uint instance)
{
    BOOL ok = FALSE;
    BOOL temp_int = FALSE;
    UInteger8 temp;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    u32 sync_pr_sec;
#endif
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        /* preserve initportState, and portInternal state */
        temp_int = config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortInternal;
        temp = config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortState;
        config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)] = *port_ds;
        config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortState = temp;
        config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)].initPortInternal = temp_int;
        if (PTP_READY()) {
            if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
                if ((ok = vtss_ptp_set_port_ds(ptp_global.ptpi[instance], portnum, port_ds))) {
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
                    /* Currently we do not support PHY LTC synchronization, if the clockrate is adjusted more than once pr sec */
                    if (port_ds->logSyncInterval < 0) {
                        sync_pr_sec = 1<<(abs(port_ds->logSyncInterval));
                        if (config_data.conf[instance].default_of.period*config_data.conf[instance].default_of.dist < sync_pr_sec) {
                            config_data.conf[instance].default_of.period = sync_pr_sec/config_data.conf[instance].default_of.dist;
                            T_IG(_I,"Increasing instance(%d)filter parameter: period to %d", instance, config_data.conf[instance].default_of.period);
                        }
                    }
#endif
                }
            }
            if (ok) {
                ptp_conf_save();
                if (VTSS_RC_OK != ptp_ace_update(instance)) {
                    T_IG(_I,"ptp_ace_update failed");
                }
            }
        } else {
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_get_port_cfg_ds(uint portnum,
                    ptp_set_port_ds_t *port_ds, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        *port_ds = config_data.conf[instance].port_config[ptp_api_to_l2port(portnum)];
        ok = TRUE;
    }
    PTP_CORE_UNLOCK();
    return ok;
}


BOOL
ptp_get_port_foreign_ds(ptp_foreign_ds_t *f_ds, int portnum, i16 ix, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            ok = vtss_ptp_get_port_foreign_ds(ptp_global.ptpi[instance], portnum, ix, f_ds);
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}


/**
 * \brief Set filter parameters for a Default PTP filter instance.
 *
 */
BOOL
ptp_default_filter_parameters_set(const vtss_ptp_default_filter_config_t *c, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        config_data.conf[instance].default_of = *c;
        if (PTP_READY()) {
            if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
                ok = TRUE;
            }
            if (ok) {
                ptp_conf_save();
            }
        } else {
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get filter parameters for a Default PTP filter instance.
 *
 */
BOOL
ptp_default_filter_parameters_get(vtss_ptp_default_filter_config_t *c, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            *c = config_data.conf[instance].default_of;
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get default filter parameters for a Default PTP filter instance.
 *
 */
void
ptp_default_filter_default_parameters_get(vtss_ptp_default_filter_config_t *c)
{
    c->period = 1;
    c->dist = 2;
}

/**
 * \brief Set filter parameters for a Default PTP servo instance.
 *
 */
BOOL
ptp_default_servo_parameters_set(const vtss_ptp_default_servo_config_t *c, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        config_data.conf[instance].default_se = *c;
        if (PTP_READY()) {
            if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
                ok = TRUE;
            }
            if (ok)
                ptp_conf_save();
        } else {
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief SGt filter parameters for a Default PTP servo instance.
 *
 */
BOOL
ptp_default_servo_parameters_get(vtss_ptp_default_servo_config_t *c, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            *c = config_data.conf[instance].default_se;
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get default filter parameters for a Default PTP servo
 *
 */
void
ptp_default_servo_default_parameters_get(vtss_ptp_default_servo_config_t *c)
{
    c->display_stats = FALSE;
    c->p_reg = TRUE;
    c->i_reg = TRUE;
    c->d_reg = TRUE;
    c->ap = DEFAULT_AP;
    c->ai = DEFAULT_AI;
    c->ad = DEFAULT_AD;
    c->srv_option = VTSS_PTP_CLOCK_FREE;
    c->synce_threshold = 0;
    c->synce_ap = 0;
    c->ho_filter = 60;
    c->stable_adj_threshold = 300;  /* = 30 ppb */
    
}

BOOL
ptp_default_servo_status_get(vtss_ptp_servo_status_t *s, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            servo[instance]->clock_servo_status(servo[instance], s);
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

BOOL
ptp_get_clock_slave_ds(ptp_clock_slave_ds_t *status, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            vtss_ptp_get_clock_slave_ds(ptp_global.ptpi[instance], status);
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Set filter parameters for a Default PTP filter instance.
 *
 */
BOOL
ptp_default_delay_filter_parameters_set(const vtss_ptp_default_delay_filter_config_t *c, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        config_data.conf[instance].default_df = *c;
        if (PTP_READY()) {
            if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
                ok = TRUE;
            }
            if (ok)
                ptp_conf_save();
        } else {
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get filter parameters for a Default PTP filter instance.
 *
 */
BOOL
ptp_default_delay_filter_parameters_get(vtss_ptp_default_delay_filter_config_t *c, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (instance < PTP_CLOCK_INSTANCES) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            *c = config_data.conf[instance].default_df;
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get default filter parameters for a Default PTP delay filter.
 *
 */
void
ptp_default_delay_filter_default_parameters_get(vtss_ptp_default_delay_filter_config_t *c)
{
    c->delay_filter = DEFAULT_DELAY_S;
}

/**
 * \brief Set unicast slave configuration parameters.
 *
 */
BOOL
ptp_uni_slave_conf_set(const vtss_ptp_unicast_slave_config_t *c, uint ix, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if ((instance < PTP_CLOCK_INSTANCES) && ix < MAX_UNICAST_MASTERS_PR_SLAVE) {
        config_data.conf[instance].unicast_slave[ix] = *c;
        if (PTP_READY()) {
            if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
                vtss_ptp_uni_slave_conf_set(ptp_global.ptpi[instance], ix, c);

                ok = TRUE;
            }
            if (ok)
                ptp_conf_save();
        } else {
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get unicast slave configuration parameters.
 *
 */
BOOL
ptp_uni_slave_conf_state_get(vtss_ptp_unicast_slave_config_state_t *c, uint ix, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if ((instance < PTP_CLOCK_INSTANCES) && ix < MAX_UNICAST_MASTERS_PR_SLAVE) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            vtss_ptp_uni_slave_conf_state_get(ptp_global.ptpi[instance], ix, c);
            ok = TRUE;
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get unicast slave table data.
 *
 */
BOOL
ptp_clock_unicast_table_get(vtss_ptp_unicast_slave_table_t *uni_slave_table, int ix, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            ok = vtss_ptp_clock_unicast_table_get(ptp_global.ptpi[instance], uni_slave_table, ix);
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get unicast master table data.
 *
 */
BOOL
ptp_clock_unicast_master_table_get(vtss_ptp_unicast_master_table_t *uni_master_table, int ix, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (config_data.conf[instance].clock_init.deviceType != PTP_DEVICE_NONE) {
            ok = vtss_ptp_clock_unicast_master_table_get(ptp_global.ptpi[instance], uni_master_table, ix);
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/**
 * \brief Get observed egress latency.
 *
 */
void ptp_clock_egress_latency_get(observed_egr_lat_t *lat)
{
    PTP_CORE_LOCK();
    *lat = observed_egr_lat;
    PTP_CORE_UNLOCK();
}

/**
 * \brief Clear observer egress latency.
 *
 */
void ptp_clock_egress_latency_clear(void)
{
    PTP_CORE_LOCK();
    observed_egr_lat.cnt = 0;
    observed_egr_lat.min = 0;
    observed_egr_lat.mean = 0;
    observed_egr_lat.max = 0;
    PTP_CORE_UNLOCK();
}

/* Get external clock output configuration. */
void vtss_ext_clock_out_get(vtss_ptp_ext_clock_mode_t *mode)
{

    PTP_CORE_LOCK();
    *mode = config_data.init_ext_clock_mode;
    PTP_CORE_UNLOCK();
}

/* Get default external clock output configuration. */
void vtss_ext_clock_out_default_get(vtss_ptp_ext_clock_mode_t *mode)
{
    mode->one_pps_mode = VTSS_PTP_ONE_PPS_DISABLE;
    mode->clock_out_enable = FALSE;
    mode->vcxo_enable = FALSE;
    mode->freq = 1;
    
}

/* Get debug_mode. */
BOOL ptp_debug_mode_get(int *debug_mode, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        ok =  vtss_ptp_debug_mode_get(ptp_global.ptpi[instance], debug_mode);
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/* Set debug_mode. */
BOOL ptp_debug_mode_set(int debug_mode, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        ok =  vtss_ptp_debug_mode_set(ptp_global.ptpi[instance], debug_mode);
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/* Set external clock output configuration. */
static void ext_clock_out_set(const vtss_ptp_ext_clock_mode_t *mode)
{
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_ts_ext_clock_one_pps_mode_t m;
#if !defined (VTSS_ARCH_SERVAL)
    ptp_set_clock_ds_t default_set;
    ptp_clock_default_ds_t default_get;
#endif /* !defined (VTSS_ARCH_SERVAL) */
    vtss_ts_ext_clock_mode_t clock_mode;
    m = (mode->one_pps_mode == VTSS_PTP_ONE_PPS_DISABLE) ? TS_EXT_CLOCK_MODE_ONE_PPS_DISABLE :
        (mode->one_pps_mode == VTSS_PTP_ONE_PPS_OUTPUT ? TS_EXT_CLOCK_MODE_ONE_PPS_OUTPUT : TS_EXT_CLOCK_MODE_ONE_PPS_INPUT);
    clock_mode.one_pps_mode = m;
    clock_mode.enable = mode->clock_out_enable;
    clock_mode.freq = mode->freq;
#if !defined (VTSS_ARCH_SERVAL)
    if (mode->one_pps_mode == VTSS_PTP_ONE_PPS_INPUT) {
        one_pps_external_input_servo_init(TRUE);
        /* set ClockQuality.ClockClass to a low value (this may be made configurable in a later release */
        vtss_ptp_get_clock_default_ds(ptp_global.ptpi[0], &default_get);
        default_set.priority1 = 0;
        default_set.priority2 = default_get.priority2;
        default_set.domainNumber = default_get.domainNumber;
        vtss_ptp_set_clock_default_ds(ptp_global.ptpi[0], &default_set);
        default_get.clockQuality.clockClass = 6;
        vtss_ptp_set_clock_quality(ptp_global.ptpi[0], &default_get.clockQuality);
    } else {
        one_pps_external_input_servo_init(FALSE);
        vtss_ptp_set_clock_default_ds(ptp_global.ptpi[0], &config_data.conf[0].clock_ds);
        vtss_ptp_get_clock_default_ds(ptp_global.ptpi[0], &default_get);
        default_get.clockQuality.clockClass = DEFAULT_CLOCK_CLASS;
        vtss_ptp_set_clock_quality(ptp_global.ptpi[0], &default_get.clockQuality);
    }
#endif /* !defined (VTSS_ARCH_SERVAL) */


    PTP_RC(vtss_ts_external_clock_mode_set( NULL, &clock_mode));
#else
    T_D("Clock mode set not supported, mode = %d, freq = %u",mode->clock_out_enable, mode->freq);

#endif
}
/* Set external clock output configuration. */
void vtss_ext_clock_out_set(const vtss_ptp_ext_clock_mode_t *mode)
{
    PTP_CORE_LOCK();
    config_data.init_ext_clock_mode = *mode;
    ptp_conf_save();
    if (PTP_READY()) {
        ext_clock_out_set(mode);
    }
    PTP_CORE_UNLOCK();

}

#if defined (VTSS_ARCH_SERVAL)
static void my_co_1pps(vtss_port_no_t port_no, const vtss_timestamp_t *ts)
{
    char str [40];
    T_DG(VTSS_TRACE_GRP_PTP_PIM,"got 1pps: %s",TimeStampToString (ts, str));
    if (config_data.init_ext_clock_mode_rs422.proto == VTSS_PTP_RS422_PROTOCOL_PIM) {
        vtss_ext_clock_rs422_time_set(ts);
    }
}

static void my_co_delay(vtss_port_no_t port_no, const vtss_timeinterval_t *ts)
{
    char str [40];
    T_DG(VTSS_TRACE_GRP_PTP_PIM,"got delay: %s",vtss_tod_TimeInterval_To_String (ts, str, '.'));
    PTP_CORE_LOCK();
    wireless_status.remote_pre = FALSE;
    wireless_status.remote_delay = *ts;
    PTP_CORE_UNLOCK();
}

static void my_co_pre_delay(vtss_port_no_t port_no)
{
    T_DG(VTSS_TRACE_GRP_PTP_PIM,"got pre delay");
    PTP_CORE_LOCK();
    wireless_status.remote_pre = TRUE;
    PTP_CORE_UNLOCK();
}

static void pim_proto_init(void)
{
    ptp_pim_init_t pim_ini;
    pim_ini.co_1pps = my_co_1pps;
    pim_ini.co_delay = my_co_delay;
    pim_ini.co_pre_delay = my_co_pre_delay;
    pim_ini.tg = VTSS_TRACE_GRP_PTP_PIM;
    ptp_pim_init(&pim_ini, config_data.init_ext_clock_mode_rs422.proto == VTSS_PTP_RS422_PROTOCOL_PIM || wireless_status.mode_enabled);
}


/* Get serval rs422 external clock configuration. */
void vtss_ext_clock_rs422_conf_get(vtss_ptp_rs422_conf_t *mode)
{

    PTP_CORE_LOCK();
    *mode = config_data.init_ext_clock_mode_rs422;
    if (mode->mode == VTSS_PTP_RS422_MAIN_AUTO) {
        /* in main auto mode the delay is read from HW */
        PTP_RC(vtss_ts_alt_clock_saved_get(NULL, &mode->delay));
    }
    PTP_CORE_UNLOCK();
}

/* Get serval default rs4224 external clock configuration. */
void vtss_ext_clock_rs422_default_conf_get(vtss_ptp_rs422_conf_t *mode)
{
    mode->mode = VTSS_PTP_RS422_DISABLE;
    mode->delay = 0;
    mode->proto = VTSS_PTP_RS422_PROTOCOL_SER;
    mode->port = 0;
}

#define GPIO_RS422_1588_MSTOEN 23   /* gpio pin to tristate the Tx connector feedback signal */
/* Set serval rs422 external clock configuration. */
static void ext_clock_rs422_conf_set(const vtss_ptp_rs422_conf_t *mode)
{
    vtss_ts_alt_clock_mode_t clock_mode;
    vtss_timestamp_t ts;
    u32 tc;
    vtss_rc rc;
    vtss_timeinterval_t one_pps_latency = (vtss_timeinterval_t)8LL<<16; /* default 8 ns including GPIO delay */
    /* if mode is VTSS_PTP_RS422_SUB, then GPIO23 must be set active high.
     * this is done to tristate the driver connected to the TX connector feedback signal */
    PTP_RC(vtss_gpio_mode_set(NULL, 0,   GPIO_RS422_1588_MSTOEN, VTSS_GPIO_OUT));
    if (mode->mode == VTSS_PTP_RS422_SUB) {
        /* set slave mode */
        PTP_RC(vtss_gpio_write(NULL, 0, GPIO_RS422_1588_MSTOEN, TRUE));
        one_pps_latency += (((vtss_timeinterval_t)mode->delay)<<16); /* default one clock cycle */
    } else {
        /* clear slave mode */
        PTP_RC(vtss_gpio_write(NULL, 0, GPIO_RS422_1588_MSTOEN, FALSE));
    }
    PTP_RC(vtss_module_man_master_1pps_latency(one_pps_latency));

    clock_mode.one_pps_out = (mode->mode == VTSS_PTP_RS422_DISABLE) ? FALSE :
                             ((mode->mode == VTSS_PTP_RS422_MAIN_AUTO || mode->mode == VTSS_PTP_RS422_MAIN_MAN) ? TRUE : FALSE);
    clock_mode.one_pps_in  = (mode->mode == VTSS_PTP_RS422_DISABLE) ? FALSE :
                             ((mode->mode == VTSS_PTP_RS422_MAIN_AUTO || mode->mode == VTSS_PTP_RS422_MAIN_MAN) ? TRUE : TRUE);
    clock_mode.save        = (mode->mode == VTSS_PTP_RS422_DISABLE) ? FALSE :
                             ((mode->mode == VTSS_PTP_RS422_MAIN_AUTO || mode->mode == VTSS_PTP_RS422_MAIN_MAN) ? TRUE : TRUE);
    clock_mode.load        = (mode->mode == VTSS_PTP_RS422_DISABLE) ? FALSE :
                             ((mode->mode == VTSS_PTP_RS422_MAIN_AUTO || mode->mode == VTSS_PTP_RS422_MAIN_MAN) ? FALSE : TRUE);
    PTP_RC(vtss_ts_alt_clock_mode_set( NULL, &clock_mode));
    if (mode->mode == VTSS_PTP_RS422_SUB) {
        one_pps_external_input_servo_init(TRUE);
        /* temporary set time */
        PTP_RC(vtss_ts_timeofday_get(NULL, &ts, &tc));
        ts.nanoseconds = mode->delay;
        PTP_RC(vtss_ts_timeofday_next_pps_set(NULL, &ts));
    }
    pim_proto_init();
    if (mode->mode == VTSS_PTP_RS422_MAIN_MAN) {
        /* in manual mode there is no External input, therefore the transmission is triggered by the internal 1PPS */
        rc = vtss_interrupt_source_hook_set(one_pps_pulse_interrupt_handler,
                                              INTERRUPT_SOURCE_SYNC,
                                              INTERRUPT_PRIORITY_NORMAL);
        if (rc != VTSS_RC_OK && rc != VTSS_UNSPECIFIED_ERROR) {
            T_W("Error code: %x", rc);
        }
    }
    
}
void vtss_ext_clock_rs422_conf_set(const vtss_ptp_rs422_conf_t *mode)
{
    PTP_CORE_LOCK();
    config_data.init_ext_clock_mode_rs422 = *mode;
    ptp_conf_save();
    if (PTP_READY()) {
        ext_clock_rs422_conf_set(mode);
    }
    PTP_CORE_UNLOCK();
}
void vtss_ext_clock_rs422_time_set(const vtss_timestamp_t *t)
{
    vtss_timestamp_t ts;
    u32 tc;
    BOOL ongoing_adjustment;
    vtss_timeinterval_t one_pps_latency = (vtss_timeinterval_t)8LL<<16; /* default 8 ns including GPIO delay */
    PTP_CORE_LOCK();
    if (config_data.init_ext_clock_mode_rs422.mode == VTSS_PTP_RS422_SUB &&
            (VTSS_RC_OK == vtss_ts_ongoing_adjustment(NULL,&ongoing_adjustment) && !ongoing_adjustment)) {
        /* set time if needed*/
        PTP_RC(vtss_ts_timeofday_get(NULL, &ts, &tc));
        if (t->seconds != ts.seconds+1 || t->nanoseconds != config_data.init_ext_clock_mode_rs422.delay) {
            PTP_RC(vtss_ts_timeofday_next_pps_set(NULL, t));
            config_data.init_ext_clock_mode_rs422.delay = t->nanoseconds;
            one_pps_latency += (((vtss_timeinterval_t)t->nanoseconds)<<16); /* add actual delay */
            PTP_RC(vtss_module_man_master_1pps_latency(one_pps_latency));
            one_pps_first_time = TRUE;
            T_DG(VTSS_TRACE_GRP_PTP_SER,"Set time of next pps, t->s = %u, ts.s = %u",t->seconds, ts.seconds);
        } else {
            T_DG(VTSS_TRACE_GRP_PTP_SER,"time uptodate");
        }
    }
    PTP_CORE_UNLOCK();
}
#endif

BOOL
ptp_get_port_link_state(vtss_ptp_port_link_state_t *ds, int portnum, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY()) {
        ds->link_state = port_data[portnum-1].link_state;
        ds->in_sync_state = port_data[portnum-1].topo.port_ts_in_sync;
        ds->forw_state = port_data[portnum-1].vlan_forw[instance];
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        ds->phy_timestamper = phy_ts_rules[instance].port[portnum-1].phy_ts_port;
#else
        ds->phy_timestamper = FALSE;
#endif
        ok = TRUE;
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/*
 * Enable/disable the wireless variable tx delay feature for a port.
 */
BOOL ptp_port_wireless_delay_mode_set(BOOL enable, int portnum, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_ORD_BOUND) && 
              config_data.conf[instance].clock_init.twoStepFlag) ||
            (config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM)) {
            ok = vtss_ptp_port_wireless_delay_mode_set(ptp_global.ptpi[instance], enable, portnum);
#if defined (VTSS_ARCH_SERVAL)
            if (config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM) {
                wireless_status.mode_enabled = TRUE;
                pim_proto_init();
            }
#endif /* defined (VTSS_ARCH_SERVAL) */
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}
                
/*
 * Get the Enable/disable mode for the wireless variable tx delay feature for a port.
 */
BOOL ptp_port_wireless_delay_mode_get(BOOL *enable, int portnum, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_ORD_BOUND) && 
                config_data.conf[instance].clock_init.twoStepFlag) ||
                (config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM)) {
            ok = vtss_ptp_port_wireless_delay_mode_get(ptp_global.ptpi[instance], enable, portnum);
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}
                
/*
 * Pre notification sent from the wireless modem transmitter before the delay is changed.
 */
BOOL ptp_port_wireless_delay_pre_notif(int portnum, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if ((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_ORD_BOUND) && 
                config_data.conf[instance].clock_init.twoStepFlag) {
            ok = vtss_ptp_port_wireless_delay_pre_notif(ptp_global.ptpi[instance], portnum);
#if defined(VTSS_ARCH_SERVAL)
        } else if ((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_MASTER_ONLY) && 
                 config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM) {
            ptp_pim_modem_pre_delay_msg_send(uport2iport(portnum));
            ok = TRUE;
        } else if ((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY) && 
                    config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM) {
            wireless_status.local_pre = TRUE;
            ok = TRUE;
#endif /* defined(VTSS_ARCH_SERVAL) */
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}
                
/*
 * Set the delay configuration, sent from the wireless modem transmitter whenever the delay is changed.
 */
BOOL ptp_port_wireless_delay_set(const vtss_ptp_delay_cfg_t *delay_cfg, int portnum, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_ORD_BOUND) && 
                config_data.conf[instance].clock_init.twoStepFlag) ||
        (config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM)) {
            ok = vtss_ptp_port_wireless_delay_set(ptp_global.ptpi[instance], delay_cfg, portnum);
        } 
#if defined(VTSS_ARCH_SERVAL)
        if ((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_MASTER_ONLY) && 
                    config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM) {
            ptp_pim_modem_delay_msg_send(uport2iport(portnum), &delay_cfg->base_delay);
            ok = TRUE;
        } else if ((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_SLAVE_ONLY) && 
                    config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM) {
            wireless_status.local_pre = FALSE;
            wireless_status.local_delay = delay_cfg->base_delay;
            ok = TRUE;
        }
#endif /* defined(VTSS_ARCH_SERVAL) */
    }
    PTP_CORE_UNLOCK();
    return ok;
}

/*
 * Get the delay configuration.
 */
BOOL ptp_port_wireless_delay_get(vtss_ptp_delay_cfg_t *delay_cfg, int portnum, uint instance)
{
    BOOL ok = FALSE;
    PTP_CORE_LOCK();
    if (PTP_READY() && (instance < PTP_CLOCK_INSTANCES)) {
        if (((config_data.conf[instance].clock_init.deviceType == PTP_DEVICE_ORD_BOUND) && 
                config_data.conf[instance].clock_init.twoStepFlag) ||
                (config_data.conf[instance].clock_init.protocol == PTP_PROTOCOL_OAM)) {
            ok = vtss_ptp_port_wireless_delay_get(ptp_global.ptpi[instance], delay_cfg, portnum);
        }
    }
    PTP_CORE_UNLOCK();
    return ok;
}

static BOOL ext_pdv;
vtss_rc ptp_set_ext_pdv_config(BOOL enable)
{
#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
    PTP_CORE_LOCK();
    ext_pdv = enable;
    PTP_RC(zl_3034x_pdv_create(enable));
    PTP_CORE_UNLOCK();
    T_I("set external PDV %s",enable ? "TRUE" : "FALSE");
#else
    T_W("External PDV filter function not defined");
#endif /* VTSS_SW_OPTION_ZL_3034X_PDV */
    return VTSS_RC_OK;
}
vtss_rc ptp_get_ext_pdv_config(BOOL *enable)
{
#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
    PTP_CORE_LOCK();
    *enable = ext_pdv;
    PTP_CORE_UNLOCK();
    T_I("get external PDV %s",ext_pdv ? "TRUE" : "FALSE");
#else
    *enable = FALSE;
    T_W("External PDV filter function not defined");
#endif /* VTSS_SW_OPTION_ZL_3034X_PDV */
    return VTSS_RC_OK;
}

BOOL vtss_1588_external_pdv(u32 clock_id)
{
    return (clock_id == 0 && ext_pdv);
}

void vtss_1588_process_timestamp(const vtss_timestamp_t *send_time, 
                                 const vtss_timestamp_t *recv_time, 
                                 vtss_timeinterval_t correction,
                                 Integer8 logMsgIntv, BOOL fwd_path)
{
#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
    vtss_zl_3034x_process_timestamp_t ts;
    if(!zl_3034x_packet_rate_set(logMsgIntv, fwd_path)) {
        T_W("zl_3034x_packet_rate_set failed");
    }
    ts.tx_ts = *send_time;
    ts.rx_ts = *recv_time;
    ts.corr = correction;
    ts.fwd_path = fwd_path;
    if(!zl_3034x_process_timestamp(&ts)) {
        T_W("zl_3034x_process_timestamp failed");
    }
#else
    T_W("External PDV filter function not defined");
#endif    
}

void vtss_1588_pdv_status_get(u32 *pdv_clock_state, i32 *freq_offset)
{
#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
    if(!zl_3034x_pdv_status_get(pdv_clock_state, freq_offset)) {
        T_W("zl_3034x_packet_rate_set failed");
    }
#else
    T_W("External PDV filter function not defined");
#endif    
    
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc
ptp_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    int i;
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
#ifdef VTSS_SW_OPTION_VCLI
        ptp_cli_req_init();
#endif
        ptp_global.ready = FALSE;
        critd_init(&ptp_global.coremutex, "ptp_core", VTSS_MODULE_ID_PTP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        PTP_CORE_UNLOCK();
        cyg_flag_init( &ptp_global_control_flags );
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          ptp_thread,
                          0,
                          "PTP",
                          ptp_thread_stack,
                          sizeof(ptp_thread_stack),
                          &ptp_thread_handle,
                          &ptp_thread_block);
        cyg_flag_init( &bg_thread_flags );
        cyg_thread_create(THREAD_HIGH_PRIO,
                          ptp_background_thread,
                          0,
                          "ptp_background",
                          ptp_background_thread_stack,
                          sizeof(ptp_background_thread_stack),
                          &ptp_background_thread_handle,
                          &ptp_background_thread_block);
#if defined(VTSS_ARCH_SERVAL)
        PTP_RC(ptp_1pps_serial_init());
#endif
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        PTP_RC(ptp_1pps_sync_init());
        PTP_RC(ptp_1pps_closed_loop_init());
#endif
        
#ifdef VTSS_SW_OPTION_ICFG
        PTP_RC(ptp_icfg_init());
#endif
        T_IG(0, "INIT_CMD_INIT PTP" );
        break;
    case INIT_CMD_START:
        T_IG(0, "INIT_CMD_START PTP");
        break;
    case INIT_CMD_CONF_DEF:
        T_IG(0, "INIT_CMD_CONF_DEF PTP" );
        if (isid == VTSS_ISID_GLOBAL)
            cyg_flag_setbits(&ptp_global_control_flags, CTLFLAG_PTP_DEFCONFIG);
        break;
    case INIT_CMD_MASTER_UP:
        ptp_conf_read(FALSE);

        for (i = 0; i < PTP_CLOCK_INSTANCES; i++) {
            /* Create a clock servo instance */
            servo[i] = vtss_ptp_default_filter_create(&config_data.conf[i].default_of,&config_data.conf[i].default_se);
#ifdef  PTP_DELAYFILTER_WIRELESS
            delay_filt[i] = vtss_ptp_wl_delay_filter_create(&config_data.conf[i].default_of, PTP_CLOCK_PORTS);
#else
            delay_filt[i] = vtss_ptp_default_delay_filter_create(&config_data.conf[i].default_df, PTP_CLOCK_PORTS);
#endif
            /* Create a PTP engine */
            ptp_global.ptpi[i] = vtss_ptp_clock_add(&config_data.conf[i].clock_init,
                                                    &config_data.conf[i].clock_ds,
                                                    &config_data.conf[i].time_prop,
                                                    &config_data.conf[i].port_config[0],
                                                    servo[i],
                                                    delay_filt[i],
                                                    i);
            VTSS_ASSERT(ptp_global.ptpi[i] != NULL);
        }
        
        T_IG(0, "INIT_CMD_MASTER_UP - size global = %u bytes", sizeof(ptp_global));
        break;
    case INIT_CMD_SWITCH_ADD:
        T_IG(0, "INIT_CMD_SWITCH_ADD resuming thread- ISID %u", isid);
        if (isid == VTSS_ISID_START) {
            cyg_thread_resume(ptp_thread_handle);
            cyg_flag_setbits(&ptp_global_control_flags, CTLFLAG_PTP_SET_ACL);
        } else {
            T_EG(0, "INIT_CMD_SWITCH_ADD - unknown ISID %u", isid);
        }
        break;
    case INIT_CMD_MASTER_DOWN:
        T_IG(0, "INIT_CMD_MASTER_DOWN - ISID %u", isid);
        break;
    default:
        break;
    }

    return 0;
}

// Convert error code to text
// In : rc - error return code
char *ptp_error_txt(vtss_rc rc)
{
    switch (rc)
    {
    case PTP_ERROR_INV_PARAM:                   return("Invalid parameter error returned from PTP");
    case PTP_RC_INVALID_PORT_NUMBER:            return("Invalid port number");
    case PTP_RC_INTERNAL_PORT_NOT_ALLOWED:      return("Enabling internal mode is only valid for a Transparent clock");
    case PTP_RC_MISSING_PHY_TIMESTAMP_RESOURCE: return("No timestamp engine is available in the PHY");
    case PTP_RC_MISSING_IP_ADDRESS:             return("cannot set up unicast ACL before my ip address is defined");
    case PTP_RC_UNSUPPORTED_ACL_FRAME_TYPE:      return ("unsupported ACL frame type");
    case PTP_RC_UNSUPPORTED_PTP_ENCAPSULATION_TYPE: return ("unsupported PTP ancapsulation type");
    case PTP_RC_UNSUPPORTED_1PPS_OPERATION_MODE:  return ("unsupported 1PPS operation mode");
    default:                                    return("Unknown error returned from PTP");
    }
}

#if defined TIMEOFDAY_TEST
static BOOL first_time = TRUE;
static void tod_test(void)
{
    vtss_timestamp_t ts;
    vtss_timestamp_t prev_ts;

    vtss_timeinterval_t diff;
    u32 tc;
    u32 prev_tc = 0;
    u32 diff_tc;
    int i;
    char str1[40];
    char str2[40];

    vtss_tod_gettimeofday(&ts, &tc);
    T_IG(_C,"Testing TOD: Now= %s, tc = %lu", TimeStampToString (&ts, str1),tc);
    for (i = 0; i < 250; i++) {
        vtss_tod_gettimeofday(&ts, &tc);
        if (!first_time) {
            first_time = FALSE;
            subTimeInterval(&diff, &ts, &prev_ts);
            if (diff < 0) {
                T_WG(_C,"Bad time reading: Now= %s, Prev = %s, diff = %lld",
                     TimeStampToString (&ts, str1),
                     TimeStampToString (&prev_ts, str2), diff);
            }
            prev_ts = ts;
            diff_tc = tc-prev_tc;
            if (tc < prev_tc) { /* time counter has wrapped */
                diff_tc += VTSS_HW_TIME_WRAP_LIMIT;
                T_WG(_C,"counter wrapped: tc = %lu,  hw_time = %lu, diff = %lu",tc,prev_tc, diff_tc);
            }
            if (diff_tc > VTSS_HW_TIME_NSEC_PR_CNT) {
                T_WG(_C,"Bad TC reading: tc = %lu,  prev_tc = %lu, diff = %lu",tc, prev_tc, diff_tc);
            }
            prev_tc = tc;
        }
    }
    vtss_tod_gettimeofday(&ts, &tc);
    T_IG(_C,"End Testing TOD: Now= %s, tc = %lu", TimeStampToString (&ts, str1),tc);
    
}

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
