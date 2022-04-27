/*

 Vitesse MEP software.

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

 $Id: vtss_mep_api.h,v 1.33 2011/03/16 12:26:55 henrikb Exp $
 $Revision: 1.33 $

*/

#ifndef _VTSS_MEP_API_H_
#define _VTSS_MEP_API_H_

#include "vtss_types.h"

#define VTSS_MEP_INSTANCE_MAX        100     /* Max number of MEP instance */
#define VTSS_MEP_PEER_MAX            5       /* Max number of peer MEP    */
#define VTSS_MEP_TRANSACTION_MAX     5       /* Max number of Link Trace transaction */
#define VTSS_MEP_REPLY_MAX           5       /* Max number of reply in a transaction */
#define VTSS_MEP_DM_MAX              2000    /* Max number of DM counters saved in RAM */
#define VTSS_MEP_CLIENT_FLOWS_MAX    10      /* Max number of client flows */

#define VTSS_MEP_APS_DATA_LENGTH     8
#define VTSS_MEP_MAC_LENGTH          6
#define VTSS_MEP_MEG_CODE_LENGTH     17      /* Both Maintenance Domain Name and MEG-ID can be max 16 charecters plus a NULL termination */

/****************************************************************************/
/*  MEP management interface                                                */
/****************************************************************************/

typedef enum {
    VTSS_MEP_MGMT_PORT,      /* Domain is Port */
//    VTSS_MEP_MGMT_ESP,       /* Domain is ESP Path */
    VTSS_MEP_MGMT_EVC,       /* Domain is EVC Service */
    VTSS_MEP_MGMT_VLAN,      /* Domain is VLAN */
//    VTSS_MEP_MGMT_MPLS       /* Domain is Mpls */
} vtss_mep_mgmt_domain_t;

typedef enum {
    VTSS_MEP_MGMT_MEP,
    VTSS_MEP_MGMT_MIP
} vtss_mep_mgmt_mode_t;

typedef enum {
    VTSS_MEP_MGMT_DOWN,  /* Down MEP/MIP */
    VTSS_MEP_MGMT_UP     /* Up MEP/MIP */
} vtss_mep_mgmt_direction_t;


typedef enum {
    VTSS_MEP_MGMT_ITU_ICC,    /* MEG-ID is ITU ICC format as described in Y.1731 ANNEX A.1 */
    VTSS_MEP_MGMT_IEEE_STR,   /* MEG-ID is IEEE Domain Name format as described in 802.1ag 21.6.5 - Domain format '4' and Short format '2' (Character string) */
    VTSS_MEP_MGMT_ITU_CC_ICC  /* MEG-ID is ITU CC and ICC format as described in Y.1731 ANNEX A.2 */
} vtss_mep_mgmt_format_t;

typedef struct {
    BOOL                        enable;                                           /* Enable/disable                                  */
    vtss_mep_mgmt_mode_t        mode;                                             /* MEP or MIP mode                                 */
    vtss_mep_mgmt_direction_t   direction;                                        /* Ingress or Egress direction                     */
    vtss_mep_mgmt_domain_t      domain;                                           /* Domain                                          */
    u32                         flow;                                             /* Flow instance (Port - EVC - VLAN)               */
    u32                         port;                                             /* Residence port - same as flow if domain is Port */
    u32                         level;                                            /* MEG level                                       */
    u16                         vid;                                              /* VID used for tagged OAM                         */
    BOOL                        voe;                                              /* This should be VOE based if possible            */

    vtss_mep_mgmt_format_t      format;                                           /* MEG-ID format.                                  */
    char                        name[VTSS_MEP_MEG_CODE_LENGTH];                   /* IEEE Maintenance Domain Name.          (string) */
    char                        meg[VTSS_MEP_MEG_CODE_LENGTH];                    /* Unique MEG ID.                         (string) */
    u32                         mep;                                              /* MEP id of this instance                         */
    u32                         peer_count;                                       /* Number of peer MEP’s  (VTSS_MEP_PEER_MAX)       */
    u16                         peer_mep[VTSS_MEP_PEER_MAX];                      /* MEP id of peer MEP’s                            */
    u8                          peer_mac[VTSS_MEP_PEER_MAX][VTSS_MEP_MAC_LENGTH]; /* Peer unicast MAC                                */

    u32                         evc_pag;                                          /* EVC generated policy PAG value. On Jaguar this is used as IS2 key. For Up-MEP (DS1076) this is used when creating MCE (IS1) entries */
    u32                         evc_qos;                                          /* EVC QoS value. Only used on Caracel for getting queue frame counters. For Up-MEP (DS1076) this is used when creating MCE (IS1) entries */
} vtss_mep_mgmt_conf_t;

typedef struct {
    BOOL    enable;         /* Delivering of PM (LM and DM) data to the PM Data Set is enabled */
} vtss_mep_mgmt_pm_conf_t;

typedef enum {
    VTSS_MEP_MGMT_PERIOD_INV,
    VTSS_MEP_MGMT_PERIOD_300S,
    VTSS_MEP_MGMT_PERIOD_100S,
    VTSS_MEP_MGMT_PERIOD_10S,
    VTSS_MEP_MGMT_PERIOD_1S,
    VTSS_MEP_MGMT_PERIOD_6M,
    VTSS_MEP_MGMT_PERIOD_1M,
    VTSS_MEP_MGMT_PERIOD_6H,
} vtss_mep_mgmt_period_t;

typedef struct {
    BOOL				      enable;         /* Enabled if true              */
    BOOL                      dei;            /* DEI for the CCM              */
    u32                       prio;           /* Priority for the CCM         */
    vtss_mep_mgmt_period_t    period;         /* CCM transmission period      */
} vtss_mep_mgmt_cc_conf_t;

typedef enum {
    VTSS_MEP_MGMT_SINGEL_ENDED,  /* Singel ended LM based on LMM/LMR or DM based on DMM/DMR   */
    VTSS_MEP_MGMT_DUAL_ENDED     /* Dual ended LM based on CCM or DM based on 1DM             */
} vtss_mep_mgmt_ended_t;

typedef enum {
    VTSS_MEP_MGMT_UNICAST,		/* Unicast  destination address */
    VTSS_MEP_MGMT_MULTICAST		/* Multicast  destination address */
} vtss_mep_mgmt_cast_t;

typedef struct {
    BOOL                        enable;         /* Enabled if true                                           */
    BOOL                        dei;            /* DEI for the CCM/LMM                                       */
    u32                         prio;           /* Priority for the CCM/LMM                                  */
    vtss_mep_mgmt_period_t      period;         /* PDU transmission period                                   */
    vtss_mep_mgmt_cast_t	    cast;           /* Uni/multicast selection - only multicast on CCM based LM  */
    vtss_mep_mgmt_ended_t       ended;          /* Dual/single ended selection. CCM or LMM/LMR based         */
    u32                         flr_interval;   /* Frame loss ratio time interval in sec.                    */
} vtss_mep_mgmt_lm_conf_t;

typedef struct {
    u32     tx_counter;         /* Transmitted PDU (LMM - CCM) containing counters     */
    u32     rx_counter;         /* Received PDU (LMM - CCM) containing counters        */
    i32     near_los_counter;   /* Near end loss counter                               */
    i32     far_los_counter;    /* Far end loss counter                                */
    u32     near_los_ratio;     /* Near end frame loss ratio                           */
    u32     far_los_ratio;      /* Near end frame loss ratio                           */
} vtss_mep_mgmt_lm_state_t;

typedef enum {
    VTSS_MEP_MGMT_RDTRP,   /* Use two timestamps to calculate DM */
    VTSS_MEP_MGMT_FLOW      /* Use four timestamps to calculate DM */
} vtss_mep_mgmt_dm_calcway_t;

typedef enum {
    VTSS_MEP_MGMT_US,   /* MicroSecond */
    VTSS_MEP_MGMT_NS    /* NanoSecond */
} vtss_mep_mgmt_dm_tunit_t;

typedef enum {
    VTSS_MEP_MGMT_DISABLE,     /* When overflow, DM is disabled automatically */
    VTSS_MEP_MGMT_CONTINUE     /* When overflow, counter is reset and DM continues running */
} vtss_mep_mgmt_dm_act_t;

typedef struct {
/* Modify/add this for DM */
    BOOL                        enable;                    /* Enabled if true                                                                          */
    BOOL                        dei;                       /* DEI for the DM                                                                           */
    u32                         prio;                      /* Priority for the DM                                                                      */
    vtss_mep_mgmt_cast_t	    cast;                      /* Uni/multicast selection                                                                  */
    u32                         mep;                       /* Peer Mep to receive DMM/1DM - only used if unicast and 'mac' is 'all zero'               */
    vtss_mep_mgmt_ended_t       ended;                     /* Dual/single ended selection. 1DM or DMM/DMR based                                        */
    vtss_mep_mgmt_dm_calcway_t  calcway;                   /* Round trip or flow calculation way selection                                             */
    u32                         interval;                  /* Interval between 1DM/DMM in 10 ms                                                        */
    u32                         lastn;                     /* The number of last N measurements used to calculate delay average and variation          */
    vtss_mep_mgmt_dm_tunit_t    tunit;                     /* Time resolution                                                                          */
    vtss_mep_mgmt_dm_act_t      overflow_act;              /* Action when counter overflow                                                             */
    BOOL                        syncronized;               /* Near-end and far-end is real time syncronized. One-way DM calculation on receiving DMR   */
    BOOL                        proprietary;               /* DM based on Vitesse proprietary follow-up PDU                                            */
} vtss_mep_mgmt_dm_conf_t;

typedef struct {
    u32                         tx_cnt;             /* Transmitted DMM/1DM frames */
    u32                         rx_cnt;             /* Received DMR/1DM frames */
    u32                         rx_tout_cnt;        /* Received DMR timeout. After transmission of DMM, the DMR is expected to be received within 1 sec. */
    u32                         rx_err_cnt;         /* Received error counter. It is considered an error if a delay is negative or above 1 sec. */
    u32                         ovrflw_cnt;         /* The total delay counter overflow counter */
    u32                         late_txtime;        /* debug only */
    u32                         avg_delay;          /* The total average delay */
    u32                         avg_n_delay;        /* The last N average delay */
    u32                         avg_delay_var;      /* The total average delay variation */
    u32                         avg_n_delay_var;    /* The last N average delay variation */
    u32                         min_delay;          /* The minimum delay measured */
    u32                         max_delay;          /* The maximum delay measured */
    u32                         min_delay_var;      /* The minimum delay variation measured */
    u32                         max_delay_var;      /* The maximum delay variation measured */
    vtss_mep_mgmt_dm_tunit_t    tunit;              /* Time resolution */
} vtss_mep_mgmt_dm_state_t;

typedef enum {
    VTSS_MEP_MGMT_INV_APS,  /* Invalid/undefined    */
    VTSS_MEP_MGMT_L_APS,    /* Liniar protection APS    */
    VTSS_MEP_MGMT_R_APS     /* Ring protection APS    */
} vtss_mep_mgmt_aps_type_t;

typedef struct {
    BOOL                        enable;         /* Enabled if true                      */
    BOOL                        dei;            /* DEI for the APS                      */
    u32                         prio;           /* Priority for the APS                 */
    vtss_mep_mgmt_aps_type_t    type;           /* Type of APS                          */
    vtss_mep_mgmt_cast_t	    cast;           /* Uni/multicast selection              */
    u32                         raps_octet;     /* Lats octet in the R-APS multicast DA */
} vtss_mep_mgmt_aps_conf_t;

typedef struct {
    BOOL        enable;                    /* Enabled if true                                                        */
    BOOL        dei;                       /* DEI for the LTM                                                        */
    u32         prio;                      /* Priority for the LTM                                                   */
    u32         mep;                       /* Target Peer Mep to receive LTM - only used if 'mac' is 'all zero'      */
    u8          mac[VTSS_MEP_MAC_LENGTH];  /* Unicast MAC address to receive LTM - has to be used to send LTM to MIP */
    u32         ttl;                       /* Time To Live                                                           */
} vtss_mep_mgmt_lt_conf_t;

typedef enum
{
    VTSS_MEP_MGMT_RELAY_UNKNOWN,    /* Unknown relay action */
    VTSS_MEP_MGMT_RELAY_HIT,        /* Relay based on match of target MAC */
    VTSS_MEP_MGMT_RELAY_FDB,        /* Relay based on hit in FDB */
    VTSS_MEP_MGMT_RELAY_MFDB,       /* Rlay based on hit in MIP CCM database */
} vtss_mep_relay_act_t;

typedef struct {
    vtss_mep_mgmt_mode_t        mode;           /* The reply is done by a MEP or a MIP */
    vtss_mep_mgmt_direction_t   direction;      /* The reply is done by a UP or a Down instance */
    u8                          ttl;            /* The reply TTL value */
    BOOL                        forwarded;      /* The LTM was forwarded */
    vtss_mep_relay_act_t        relay_action;   /* The relay action */
    u8                          last_egress_mac[VTSS_MEP_MAC_LENGTH];
    u8                          next_egress_mac[VTSS_MEP_MAC_LENGTH];
} vtss_mep_mgmt_lt_reply_t;

typedef struct {
    u32                        transaction_id;
    u32                        reply_cnt;
    vtss_mep_mgmt_lt_reply_t   reply[VTSS_MEP_REPLY_MAX];
} vtss_mep_mgmt_lt_transaction_t;

typedef struct {
    u32                              transaction_cnt;
    vtss_mep_mgmt_lt_transaction_t   transaction[VTSS_MEP_TRANSACTION_MAX];
} vtss_mep_mgmt_lt_state_t;

#define VTSS_MEP_LBM_SIZE_MAX  9600    /* The maximum LBM frame size - includung DMAC + SMAC + T/L + FCS = 18 bytes - excluding any tags that might be added */
#define VTSS_MEP_LBM_SIZE_MIN  64      /* The minimum LBM frame size - includung DMAC + SMAC + T/L + FCS = 18 bytes - excluding any tags that might be added  */
typedef struct {
    BOOL                     enable;                    /* Enabled if true                                                                          */
    BOOL                     dei;                       /* DEI for the LBM                                                                          */
    u32                      prio;                      /* Priority for the LBM                                                                     */
    vtss_mep_mgmt_cast_t	 cast;                      /* Uni/multicast selection                                                                  */
    u32                      mep;                       /* Peer Mep to receive LBM - only used if unicast and 'mac' is 'all zero'                   */
    u8                       mac[VTSS_MEP_MAC_LENGTH];  /* Unicast MAC address to receive LBM - has to be used to send LBM to MIP                   */
    u32                      to_send;                   /* Number of LBM to send. VTSS_MEP_LB_MGMT_TO_SEND_INFINITE => test behaviour. Requires VOE */
    u32                      size;                      /* Size of LBM frame to send - max. VTSS_MEP_LBM_SIZE_MAX bytes - min. VTSS_MEP_LBM_SIZE_MIN */
    u32                      interval;                  /* Frame interval. In 10 ms. if to_send != VTSS_MEP_LB_MGMT_TO_SEND_INFINITE                */
                                                        /*                 In 1 us. if to_send == VTSS_MEP_LB_MGMT_TO_SEND_INFINITE                 */
} vtss_mep_mgmt_lb_conf_t;

typedef struct {
    u8      mac[VTSS_MEP_MAC_LENGTH];
    u64     lbr_received;
    u64     out_of_order;
} vtss_mep_mgmt_lb_reply_t;

typedef struct {
    u32                        transaction_id;
    u64                        lbm_transmitted;
    u32                        reply_cnt;
    vtss_mep_mgmt_lb_reply_t   reply[VTSS_MEP_PEER_MAX];
} vtss_mep_mgmt_lb_state_t;

typedef struct {
    BOOL                     enable;
    BOOL                     protection;
    vtss_mep_mgmt_period_t   period;
} vtss_mep_mgmt_ais_conf_t;

typedef struct {
    BOOL                     enable;
    vtss_mep_mgmt_period_t   period;
} vtss_mep_mgmt_lck_conf_t;

typedef enum {
    VTSS_MEP_MGMT_PATTERN_ALL_ZERO,  /* All zero data pattern    */
    VTSS_MEP_MGMT_PATTERN_ALL_ONE,   /* All zero data pattern    */
    VTSS_MEP_MGMT_PATTERN_0XAA,      /* 10101010 data pattern    */
} vtss_mep_mgmt_pattern_t;

#define VTSS_MEP_TST_SIZE_MAX  9600    /* The maximum TST frame size - includung DMAC + SMAC + T/L + FCS = 18 bytes - excluding any tags that might be added */
#define VTSS_MEP_TST_SIZE_MIN  64      /* The minimum TST frame size - includung DMAC + SMAC + T/L + FCS = 18 bytes - excluding any tags that might be added  */
#if defined(VTSS_ARCH_SERVAL)
#define VTSS_MEP_TST_RATE_MAX  2500000     /* 250000 Kbps = 2,5 Gbps */
#else
#define VTSS_MEP_TST_RATE_MAX  400000      /* 400000 Kbps = 400 Mbps */
#endif
typedef struct {
    BOOL                     enable;                    /* Enable/disable transmission of TST PDU	                                                 */
    BOOL                     enable_rx;                 /* Enable/disable reception and analyze of TST PDU                                           */
    u32                      prio;                      /* Priority for the TST frame                                                                */
    BOOL                     dei;                       /* DEI for the TST frame                                                                     */
    u32                      mep;                       /* Peer Mep to receive TST - only used if unicast and 'mac' is 'all zero'                    */
    u32                      rate;                      /* The transmission rate in Kbps - max VTSS_MEP_TST_RATE_MAX - min 1                         */
    u32                      size;                      /* Size of TST frame to send - max. VTSS_MEP_TST_SIZE_MAX bytes - min. VTSS_MEP_TST_SIZE_MIN */
    vtss_mep_mgmt_pattern_t  pattern;                   /* Pattern in TST frame                                                                      */
    BOOL                     sequence;                  /* Sequence number will be inserted - not checked in receiver on Caracal and Jaguar          */
} vtss_mep_mgmt_tst_conf_t;

typedef struct {
    u64    tx_counter;  /* Transmitted TST frames counter */
    u64    rx_counter;  /* Received TST frames counter */
    u64    oo_counter;  /* Out of Order counter */
    u32    rx_rate;     /* Receive bit rate in Kbit/s */
    u32    time;        /* Test time in seconds. The elapsed time since last clear */
} vtss_mep_mgmt_tst_state_t;

#define VTSS_MEP_CLIENT_PRIO_HIGHEST  0xFF      /* The highest possible priority is requested */
typedef struct {
    vtss_mep_mgmt_domain_t   domain;
    u32                      flow_count;
    u32                      flows[VTSS_MEP_CLIENT_FLOWS_MAX];
    u8                       ais_prio[VTSS_MEP_CLIENT_FLOWS_MAX];   /* AIS Priority (EVC COS-ID) 0-7. VTSS_MEP_CLIENT_PRIO_HIGHEST indicate highest possible is requested */
    u8                       lck_prio[VTSS_MEP_CLIENT_FLOWS_MAX];   /* LCK Priority (EVC COS-ID) 0-7. VTSS_MEP_CLIENT_PRIO_HIGHEST indicate highest possible is requested */
    u8                       level[VTSS_MEP_CLIENT_FLOWS_MAX];
} vtss_mep_mgmt_client_conf_t;

typedef struct
{
    BOOL    cLevel;                        /* Incorrect CCM level received             */
    BOOL    cMeg;                          /* Incorrect CCM MEG id received            */
    BOOL    cMep;                          /* Incorrect CCM MEP id received            */
    BOOL    cSsf;                          /* SSF state                                */
    BOOL    cLoc[VTSS_MEP_PEER_MAX];       /* CCM LOC state from all peer MEP          */
    BOOL    cRdi[VTSS_MEP_PEER_MAX];       /* CCM RDI state from all peer MEP          */
    BOOL    cPeriod[VTSS_MEP_PEER_MAX];    /* CCM Period state from all peer MEP       */
    BOOL    cPrio[VTSS_MEP_PEER_MAX];      /* CCM Priority state from all peer MEP     */
    BOOL    cAis;                          /* AIS state                                */
    BOOL    cLck;                          /* Locked State                             */

    BOOL    aTsf;
    BOOL    aTsd;
    BOOL    aBlk;
} vtss_mep_mgmt_state_t;

typedef struct {
    vtss_timestamp_t tx_time;                           /* DMM transmit time  */
    vtss_timestamp_t rx_time;                           /* DMM receive time returned in DMR */
} vtss_mep_mgmt_dm_timestamp_t;

typedef struct
{
    vtss_mep_mgmt_conf_t         config;
    vtss_mep_mgmt_pm_conf_t      pm_conf;
    vtss_mep_mgmt_cc_conf_t      cc_conf;
    vtss_mep_mgmt_lm_conf_t      lm_conf;
    vtss_mep_mgmt_dm_conf_t      dm_conf;
    vtss_mep_mgmt_aps_conf_t     aps_conf;
    vtss_mep_mgmt_lt_conf_t      lt_conf;
    vtss_mep_mgmt_lb_conf_t      lb_conf;
    vtss_mep_mgmt_ais_conf_t     ais_conf;
    vtss_mep_mgmt_lck_conf_t     lck_conf;
    vtss_mep_mgmt_tst_conf_t     tst_conf;
    vtss_mep_mgmt_client_conf_t  client_conf;
} vtss_mep_mgmt_def_conf_t;

#endif /* _VTSS_MEP_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
