/*

 Vitesse MEP software.

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

#ifndef _VTSS_MEP_SUPP_API_H_
#define _VTSS_MEP_SUPP_API_H_

#include "vtss_types.h"
#include "vtss_api.h"
#include "vtss_mep_api.h"

#define VTSS_MEP_SUPP_CREATED_MAX      100                              /* Max number of created MEP */
#define VTSS_MEP_SUPP_PEER_MAX         5                                /* Max number of peer MEP */
#define VTSS_MEP_SUPP_SERVICE_VOE_MAX  VTSS_OAM_PATH_SERVICE_VOE_CNT    /* Max number of service VOE */
#define VTSS_MEP_SUPP_PORT_VOE_MAX     VTSS_OAM_PORT_VOE_CNT            /* Max number of port VOE */

#define VTSS_MEP_SUPP_RC_OK                 0   /* Management operation is ok */
#define VTSS_MEP_SUPP_RC_INVALID_PARAMETER  1   /* Invalid parameter */
#define VTSS_MEP_SUPP_RC_NO_RESOURCES       2   /* No resources -  */
#define VTSS_MEP_SUPP_RC_NO_VOE             3   /* VOE not supported -  */
#define VTSS_MEP_SUPP_RC_VOE_ERROR          4   /* VOE returned error  */

#define VTSS_MEP_SUPP_LAPS_DATA_LENGTH    4
#define VTSS_MEP_SUPP_RAPS_DATA_LENGTH    VTSS_MEP_APS_DATA_LENGTH
#define VTSS_MEP_SUPP_MAC_LENGTH          VTSS_MEP_MAC_LENGTH
#define VTSS_MEP_SUPP_MEG_CODE_LENGTH     VTSS_MEP_MEG_CODE_LENGTH
#define VTSS_MEP_SUPP_LBR_MAX             10
#define VTSS_MEP_SUPP_LTR_MAX             10
#define VTSS_MEP_SUPP_DM_MAX              10    /* A risk to cause stack overflow if the value is too large */
#define VTSS_MEP_SUPP_CLIENT_FLOWS_MAX    10    /* Max number of flows to insert AIS/LCK */


/****************************************************************************/
/*  MEP upper logic call in to lower support                                */
/****************************************************************************/

#define  VTSS_MEP_SUPP_FLOW_MASK_PORT     0x0001
#define  VTSS_MEP_SUPP_FLOW_MASK_PORT_P   0x0002
#define  VTSS_MEP_SUPP_FLOW_MASK_CINVID   0x0004
#define  VTSS_MEP_SUPP_FLOW_MASK_SINVID   0x0008
#define  VTSS_MEP_SUPP_FLOW_MASK_COUTVID  0x0010
#define  VTSS_MEP_SUPP_FLOW_MASK_SOUTVID  0x0020
#define  VTSS_MEP_SUPP_FLOW_MASK_CUOUTVID (0x0040 | VTSS_MEP_SUPP_FLOW_MASK_SOUTVID)   /* Custom outer VID - this is two bit's as a Customer VID is also an S-VID */
#define  VTSS_MEP_SUPP_FLOW_MASK_VID      0x0080
#define  VTSS_MEP_SUPP_FLOW_MASK_EVC_ID   0x0100
#define  VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID  0x0200
#define  VTSS_MEP_SUPP_FLOW_MASK_PATH     0x0400
#define  VTSS_MEP_SUPP_FLOW_MASK_PATH_P   0x0800
#define  VTSS_MEP_SUPP_FLOW_MASK_PORT_UNI 0x1000

#define VTSS_MEP_SUPP_MEP_PAG  0x3F
#define VTSS_MEP_SUPP_MIP_PAG  0x3E


typedef enum
{
    VTSS_MEP_SUPP_MEP,
    VTSS_MEP_SUPP_MIP
} vtss_mep_supp_mode_t;

typedef enum
{
    VTSS_MEP_SUPP_DOWN,
    VTSS_MEP_SUPP_UP
} vtss_mep_supp_direction_t;

typedef struct
{
    u32   mask;                                /* Mask to indicate use of following member variables                                                              */
    BOOL  port[VTSS_PORT_ARRAY_SIZE];          /* Port to be used by MEP - that are in front of MEP                                                               */
    u32   in_vid;                              /* Inner VID                                                                                                       */
    u32   out_vid;                             /* Outer VID                                                                                                       */
    u32   vid;                                 /* Classified VID - checked during frame reception                                                                 */
    u32   evc_id;                              /* This is the EVC ID related to this MEP.                                                                         */
                                                                                                                                                                  
    u32   port_p;                              /* Protection port - this is used for a path protected down mep                                  (VOE capability)  */
    u32   path_mep;                            /* Path MEP relation. Used for configuring UP/DOWN MEP relation to a PATH MEP                    (VOE capability)  */
    u32   path_mep_p;                          /* Protection path MEP relation. Used for configuring DOWN-MEP relation to a protection PATH MEP (VOE capability)  */
} vtss_mep_supp_flow_t;

#define VTSS_MEP_SUPP_EVC_ID_NONE	0xFFFFFFFF
typedef struct
{
    BOOL                        enable;                             /* Enable/disable of this MEP                                             		                               */
    BOOL                        voe;                                /* Select the use of an VOE instance for this MEP                         		                               */
    vtss_mep_supp_mode_t        mode;                               /* MEP or MIP mode                                                        		                               */
    vtss_mep_supp_direction_t   direction;                          /* Up or Down MEP direction                                               		                               */
    u8                          mac[VTSS_MEP_SUPP_MAC_LENGTH];      /* MAC of this MEP                                                        		                               */
    u32                         level;                              /* Level  0-7                                                             		                               */
    vtss_mep_supp_flow_t        flow;                               /* MEP is related to this flow                                            		                               */
    u32                         port;                               /* Residence port                                                         		                               */
																																					  							 
    BOOL                        out_of_service;                     /* Set this UP-MEP out of service.     														 (VOE capability)  */
} vtss_mep_supp_conf_t;

/* instance:    Instance number of MEP     */
/* conf:        Configuration              */
/* A MEP is enabled or disabled. The flow relation is set and all OAM PDU sessions on this MEP vill be related to this flow */
u32 vtss_mep_supp_conf_set(const u32                     instance,
                           const vtss_mep_supp_conf_t    *const conf);





typedef enum
{
    VTSS_MEP_SUPP_PERIOD_INV,
    VTSS_MEP_SUPP_PERIOD_300S,      /* This will result in HW supported CCM transmission on all platforms */
    VTSS_MEP_SUPP_PERIOD_100S,      /* This will result in HW supported CCM transmission on all platforms */
    VTSS_MEP_SUPP_PERIOD_10S,
    VTSS_MEP_SUPP_PERIOD_1S,
    VTSS_MEP_SUPP_PERIOD_6M,
    VTSS_MEP_SUPP_PERIOD_1M,
    VTSS_MEP_SUPP_PERIOD_6H,
} vtss_mep_supp_period_t;

typedef enum
{
    VTSS_MEP_SUPP_ITU_ICC,    /* MEG-ID is ITU ICC format as described in Y.1731 ANNEX A         */
    VTSS_MEP_SUPP_IEEE_STR,   /* MEG-ID is IEEE Domain Name format as described in 802.1ag 21.6.5 - Domain format '1' or '4' and Short format '2' (Character string) */
    VTSS_MEP_SUPP_ITU_CC_ICC  /* MEG-ID is ITU CC and ICC format as described in Y.1731 ANNEX A.2 */
} vtss_mep_supp_format_t;

typedef struct
{
    vtss_mep_supp_period_t   period;                                 /* Period expected in received and inserted in transmitted CCM PDU */
    BOOL                     dei;                                    /* DEI                                                             */
    u32                      prio;                                   /* Priority                                                        */
    vtss_mep_supp_format_t   format;                                 /* MEG-ID format.                                                  */
    char                     name[VTSS_MEP_SUPP_MEG_CODE_LENGTH];    /* IEEE Maintenance Domain Name. (string \0 terminated)            */
    char                     meg[VTSS_MEP_SUPP_MEG_CODE_LENGTH];     /* Unique MEG ID Code.           (string \0 terminated)            */
    u32                      mep;                                    /* Id for this MEP                                                 */
    u32                      peer_count;                             /* Number of peer MEPs  (VTSS_MEP_SUPP_PEER_MAX)                   */
    u16                      peer_mep[VTSS_MEP_SUPP_PEER_MAX];       /* MEP id of peer MEPs                                             */
    u8                       dmac[VTSS_MEP_SUPP_MAC_LENGTH];         /* CCM destination MAC                                             */
} vtss_mep_supp_ccm_conf_t;

/* instance:    Instance number of MEP           */
/* conf:        Configuration                    */
/* The CCM parameters are configured. All defect calculation is based on this. Transmitted CCM is based on this if generation is enabled	*/
u32 vtss_mep_supp_ccm_conf_set(const u32                          instance,
                               const vtss_mep_supp_ccm_conf_t     *const conf);




typedef struct
{
    BOOL                         enable;        /* Enable/disable of CCM transmission - CCM will be generated with cc_period                    */
    BOOL                         lm_enable;     /* Enable/disable of LM based on CCM - CCM with loss counters will be generated with lm_period  */
    vtss_mep_supp_period_t       lm_period;     /* This is the LM transmission period                                                           */
    vtss_mep_supp_period_t       cc_period;     /* This is the CC transmission period                                                           */
} vtss_mep_supp_gen_conf_t;

/* instance:	Instance number of MEP                    */
/* This enable/disable the HW or SW CCM generator.        */
u32 vtss_mep_supp_ccm_generator_set(const u32                         instance,
                                    const vtss_mep_supp_gen_conf_t    *const conf);

/* instance:	Instance number of MEP                */
/* This returns if CCM generator is HW based .        */
BOOL vtss_mep_supp_hw_ccm_generator_get(vtss_mep_supp_period_t  period);




/* instance:	Instance number of MEP              */
/* enable:		Enable/disable of CCM RDI           */
/* This controll the RDI bit on a HW/SW based CCM   */
u32 vtss_mep_supp_ccm_rdi_set(const u32        instance,
                              const BOOL       enable);



typedef struct
{
    u64     valid_counter;                                /* Valid CCM received                                                         */
    u64     invalid_counter;                              /* Invalid CCM received                                                       */
    u64     oo_counter;                                   /* Counted received Out of Order sequence numbers (VOE capability)            */

    u32     unexp_level;                                  /* Last received invalid level value                                          */
    u32     unexp_period;                                 /* Last received invalid period value                                         */
    u32     unexp_prio;                                   /* Last received invalid priority value                                       */
    u32     unexp_mep;                                    /* Last received unexpected MEP id                                            */
    char    unexp_name[VTSS_MEP_SUPP_MEG_CODE_LENGTH];    /* Last received unexpected Domain Name or ITU Carrier Code (ICC). (string)   */
    char    unexp_meg[VTSS_MEP_SUPP_MEG_CODE_LENGTH];     /* Last received unexpected Unique MEG ID Code.                    (string)   */
} vtss_mep_supp_ccm_state_t;

/* instance:    Instance number of MEP        */
/* state:       CCM state                     */
/* The CCM state is returned.	*/
u32 vtss_mep_supp_ccm_state_get(const u32                     instance,
                                vtss_mep_supp_ccm_state_t     *const state);




typedef struct
{
    u32 tx_counter;         /* Number of LMM/CCM-LM PDU transmitted  */
    u32 rx_counter;         /* Number of LMR/CCM-LM PDU received     */
    i32 near_los_counter;   /* Near end loss measurement counter     */
    i32 far_los_counter;    /* Far end loss measurement counter      */
    u32 near_tx_counter;    /* Near end tx counter                   */
    u32 far_tx_counter;     /* Far end tx counter                    */
} vtss_mep_supp_lm_counters_t;

/* instance:   Instance number of MEP                                  */
/* Accumulated CCM based LM counters are returned - cleared after get  */
u32 vtss_mep_supp_ccm_lm_counters_get(const u32                    instance,
                                      vtss_mep_supp_lm_counters_t  *const counters);



typedef struct
{
    BOOL    dInv;                              /* Invalid period (0) received                 */
    BOOL    dLevel;                            /* Incorrect CCM level received                */
    BOOL    dMeg;                              /* Incorrect CCM MEG id received               */
    BOOL    dMep;                              /* Incorrect CCM MEP id received               */
    BOOL    dLoc[VTSS_MEP_SUPP_PEER_MAX];      /* CCM LOC state from all peer MEP             */
    BOOL    dRdi[VTSS_MEP_SUPP_PEER_MAX];      /* CCM RDI state from all peer MEP             */
    BOOL    dPeriod[VTSS_MEP_SUPP_PEER_MAX];   /* CCM Period state from all peer MEP          */
    BOOL    dPrio[VTSS_MEP_SUPP_PEER_MAX];     /* CCM Priority state from all peer MEP        */
    BOOL    dAis;                              /* AIS received                                */
    BOOL    dLck;                              /* LCK received                                */
} vtss_mep_supp_defect_state_t;

/* instance:    Instance number of MEP                        */
/* The compleate defect state related to CCM is returned      */
u32 vtss_mep_supp_defect_state_get(const u32                      instance,
                                   vtss_mep_supp_defect_state_t   *const state);




/* instance:    Instance number of MEP                */
/* Any learned MAC from expected MEP is returned      */
u32 vtss_mep_supp_learned_mac_get(const u32   instance,
                                  u8          (*mac)[VTSS_MEP_SUPP_MAC_LENGTH]);



typedef struct
{
    BOOL                     enable;                           /* Enable/disable transmission of LMM PDU                                                */
    BOOL                     dei;                              /* DEI                                                                                   */
    u32                      prio;                             /* Priority                                                                              */
    vtss_mep_supp_period_t   period;                           /* Transmission period -  Must be VTSS_MEP_SUPP_PERIOD_1S or VTSS_MEP_SUPP_PERIOD_10S    */
    u8                       dmac[VTSS_MEP_SUPP_MAC_LENGTH];   /* LMM destination MAC                                                                   */
} vtss_mep_supp_lmm_conf_t;

/* instance:    Instance number of MEP         */
/* enable:      Enable/disable of LMM session  */
/* conf:        Configuration                  */
/* A LMM session is created.                   */
u32 vtss_mep_supp_lmm_conf_set(const u32                        instance,
                               const vtss_mep_supp_lmm_conf_t   *const conf);




/* instance:   Instance number of MEP                                  */
/* Accumulated LMM based LM counters are returned - cleared after get  */
u32 vtss_mep_supp_lmm_lm_counters_get(const u32                    instance,
                                      vtss_mep_supp_lm_counters_t  *const counters);



typedef enum
{
    VTSS_MEP_SUPP_RDTRIP,   /* Use two timestamps to calculate DM */
    VTSS_MEP_SUPP_FLOW      /* Use four timestamps to calculate DM - far end residence time substracted */
} vtss_mep_supp_calcway_t;

typedef enum
{
    VTSS_MEP_SUPP_US,  /* MicroSecond */
    VTSS_MEP_SUPP_NS   /* NanoSecond */
} vtss_mep_supp_tunit_t;

typedef enum
{
    VTSS_MEP_SUPP_ACT_DISABLE,     /* When overflow, DM is disabled automatically */
    VTSS_MEP_SUPP_ACT_CONTINUE     /* When overflow, counter is reset and DM continues running */
} vtss_mep_supp_act_t;

typedef struct
{
/* Modify/add this for DMM */
    BOOL                     enable;                            /* Enable/disable transmission of DMM PDU                                                   */
    BOOL                     dei;                               /* DEI                                                                                      */
    u32                      prio;                              /* Priority                                                                                 */
    u8                       dmac[VTSS_MEP_SUPP_MAC_LENGTH];    /* DMM destination MAC                                                                      */
    u32                      interval;                          /* Interval between DMM in 10 ms                                                            */
    vtss_mep_supp_tunit_t    tunit;                             /* Time resolution                                                                          */
    BOOL                     proprietary;                       /* DM based on Vitesse proprietary follow-up PDU                                            */
    vtss_mep_supp_calcway_t  calcway;                           /* Calculation way selection                                                                */
    BOOL                     syncronized;                       /* Near-end and far-end is real time syncronized. One-way DM calculation on receiving DMR   */ 
} vtss_mep_supp_dmm_conf_t;

/* instance:    Instance number of MEP         */
/* enable:      Enable/disable of DMM session  */
/* conf:        Configuration                  */
/* A DMM session is created.                   */
u32 vtss_mep_supp_dmm_conf_set(const u32                        instance,
                               const vtss_mep_supp_dmm_conf_t   *const conf);



typedef struct
{
/* Modify/add this for 1DM */
    BOOL                     enable;                           /* Enable/disable transmission of 1DM PDU         */
    BOOL                     dei;                              /* DEI                                            */
    u32                      prio;                             /* Priority                                       */
    u8                       dmac[VTSS_MEP_SUPP_MAC_LENGTH];   /* DMM destination MAC                            */
    u32                      interval;                         /* Interval between 1DM in 10 ms                  */
    vtss_mep_supp_tunit_t    tunit;                            /* Time resolution                                */
    BOOL                     proprietary;                      /* DM based on Vitesse proprietary follow-up PDU  */
} vtss_mep_supp_dm1_conf_t;

/* instance:    Instance number of MEP            */
/* enable:      Enable/disable TX of 1DM session  */
/* conf:        Configuration                     */
/* A 1DM session is created.                      */
u32 vtss_mep_supp_dm1_conf_set(const u32                        instance,
                               const vtss_mep_supp_dm1_conf_t   *const conf);

typedef struct
{
    u32  tx_counter;                           /* Number of DMM PDU transmitted                                                                     */
    u32  rx_counter;                           /* Number of DMR PDU received - delays in 'xx_delays'                                                */
    u32  rx_tout_counter;                      /* Number of timeout on recieved DMR PDU                                                             */
    u32  late_txtime_counter;                  /* Number of late TX timestamp detected when DMR received - no TX timestamp when DMR received        */

    u32  tw_rx_err_counter;                    /* Number of invalid Two-way delays calculated when DMR received - 0 < valid delay < 1s              */
    u32  ow_ftn_rx_err_counter;                /* Number of invalid one-way far-to-near delays caculated when DMR received - 0 < valid delay < 1s   */
    u32  ow_ntf_rx_err_counter;                /* Number of invalid one-way near-to-fardelays caculated when DMR received - 0 < valid delay < 1s    */
    u32  tw_delays[VTSS_MEP_SUPP_DM_MAX];      /* Calculated two-way delays based on 'rx_counter' number of DMR received                            */
    u32  ow_ftn_delays[VTSS_MEP_SUPP_DM_MAX];  /* Calculated one-way far-to-near delays based on 'rx_counter' number of DMR received                */
    u32  ow_ntf_delays[VTSS_MEP_SUPP_DM_MAX];  /* Calculated one-way near-to-far delays based on 'rx_counter' number of DMR received                */
} vtss_mep_supp_dmr_status;

/* instance:   Instance number of MEP                         */
/* Accumulated DMM counters are returned - cleared after get  */
u32 vtss_mep_supp_dmr_get(const u32                        instance,
                          vtss_mep_supp_dmr_status *const  status);


typedef struct
{
    u32  tx_counter;                    /* Number of 1DM PDU transmitted                                                        */
    u32  rx_counter;                    /* Number of 1DM PDU received - delays in 'delays'                                      */
    u32  rx_err_counter;                /* Number of invalid delays caculated when 1DM/DMM received - 0 < valid delay < 1s      */
    u32  delays[VTSS_MEP_SUPP_DM_MAX];  /* Calculated one-way far-to-near delays based on 'rx_counter' number of 1DM received   */
} vtss_mep_supp_dm1_status;

/* instance:   Instance number of MEP                         */
/* Accumulated DM1 counters are returned - cleared after get  */
u32 vtss_mep_supp_dm1_get(const u32                        instance,
                          vtss_mep_supp_dm1_status *const  status);


typedef enum
{
    VTSS_MEP_SUPP_L_APS,    /* Liniar protection APS    */
    VTSS_MEP_SUPP_R_APS     /* Ring protection APS    */
} vtss_mep_supp_aps_type_t;

typedef struct
{
    BOOL                      enable;                           /* Enable/disable of APS       */
    vtss_mep_supp_aps_type_t  type;                             /* Type of APS                 */
    BOOL                      dei;                              /* DEI                         */
    u32                       prio;                             /* Priority                    */
    u8                        dmac[VTSS_MEP_SUPP_MAC_LENGTH];   /* APS destination MAC         */
} vtss_mep_supp_aps_conf_t;

/* instance:    Instance number of MEP         */
/* enable:      Enable/disable of APS session  */
/* conf:        Configuration                  */
/* A APS session is created. An APS is transmitted every 5 sec. Received APS info is delivered to upper logic   */
u32 vtss_mep_supp_aps_conf_set(const u32                        instance,
                               const vtss_mep_supp_aps_conf_t   *const conf);



/* instance:	Instance number of MEP	         */
/* txdata:      Transmitted APS specific info    */
/* event:       transmit APS as an ERPS event.   */
/* First three APS PDU's are transmitted with 3.3 ms (approx) interval */
u32 vtss_mep_supp_aps_txdata_set(const u32     instance,
                                 const u8      *txdata,
                                 const BOOL    event);


/* instance:    Instance number of MEP.                 */
/* enable:      TRUE means that R-APS is forwarded      */
/* R-APS forwarding from the MEP related port, can be enabled/disabled (VOE capability) */
u32 vtss_mep_supp_raps_forwarding(const u32    instance,
                                  const BOOL   enable);


/* instance:    Instance number of MEP.                                    */
/* enable:      TRUE means that R-APS is transmitted at 5 sec. interval    */
/* R-APS transmission from the MEP related port, can be enabled/disabled   */
u32 vtss_mep_supp_raps_transmission(const u32    instance,
                                    const BOOL   enable);





typedef struct
{
    BOOL         enable;                           /* Enable/disable of LT        */
    u32          transaction_id;                   /* Transaction id              */
    BOOL         dei;                              /* DEI                         */
    u32          prio;                             /* Priority                    */
    u8           ttl;                              /* Time To Live                */
    u8           dmac[VTSS_MEP_SUPP_MAC_LENGTH];   /* LTM destination MAC         */
    u8           tmac[VTSS_MEP_SUPP_MAC_LENGTH];   /* LTM target MAC              */
} vtss_mep_supp_ltm_conf_t;

/* instance:    Instance number of MEP         */
/* enable:      Enable/disable of LT session   */
/* conf:        Configuration                  */
/* A LT session is created. One LTM is transmitted. Received LTR is delivered to upper logic   */
u32 vtss_mep_supp_ltm_conf_set(const u32                         instance,
                               const vtss_mep_supp_ltm_conf_t    *const conf);

typedef enum
{
    VTSS_MEP_SUPP_RELAY_UNKNOWN,    /* Unknown relay action */
    VTSS_MEP_SUPP_RELAY_HIT,        /* Relay based on match of target MAC */
    VTSS_MEP_SUPP_RELAY_FDB,        /* Relay based on hit in FDB */
    VTSS_MEP_SUPP_RELAY_MFDB,       /* Rlay based on hit in MIP CCM database */
} vtss_mep_supp_relay_act_t;

typedef struct
{
    u32                         transaction_id;
    vtss_mep_supp_mode_t        mode;               /* The reply is done by a MEP or a MIP */
    vtss_mep_supp_direction_t   direction;          /* The reply is done by a UP or a Down instance */
    u8                          ttl;                /* The reply TTL value */
    BOOL                        forwarded;          /* The LTM was forwarded */
    vtss_mep_supp_relay_act_t   relay_action;       /* The relay action */
    u8                          last_egress_mac[VTSS_MEP_SUPP_MAC_LENGTH];
    u8                          next_egress_mac[VTSS_MEP_SUPP_MAC_LENGTH];
} vtss_mep_supp_ltr_t;

u32 vtss_mep_supp_ltr_get(const u32            instance,
                          u32                  *const count,
                          vtss_mep_supp_ltr_t  ltr[VTSS_MEP_SUPP_LTR_MAX]);



#define VTSS_MEP_SUPP_LBM_SIZE_MAX  9600    /* The maximum LBM frame size - includung DMAC + SMAC + T/L + FCS = 18 bytes - excluding any tags that might be added */
#define VTSS_MEP_SUPP_LBM_SIZE_MIN  64      /* The minimum LBM frame size - includung DMAC + SMAC + T/L + FCS = 18 bytes - excluding any tags that might be added  */
#define VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE  0
typedef struct
{
    BOOL         enable;                           /* Enable/disable of LB                                                                               */
    BOOL         dei;                              /* DEI                                                                                                */
    u32          prio;                             /* Priority                                                                                           */
    u32          to_send;                          /* Number of LBM to send. != VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE is only for SW based LB.              */
                                                   /*                        == VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE is only for HW based (VOE capability) */
    u32          size;                             /* Size of LBM frame to send - max. VTSS_MEP_SUPP_LBM_SIZE_MAX bytes - min. VTSS_MEP_SUPP_LBM_SIZE_MIN*/
    u32          interval;                         /* Frame interval. In 10 ms. if to_send != VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE    (SW based)           */
                                                   /*                 In 1 us. if to_send == VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE    (HW baseed)           */
    u8           dmac[VTSS_MEP_SUPP_MAC_LENGTH];   /* LBM destination MAC                                                                                */
} vtss_mep_supp_lbm_conf_t;

/* instance:    Instance number of MEP         */
/* enable:      Enable/disable of LB session   */
/* conf:        Configuration                  */
/* A LB session is created. A to_send number of LBM is transmitted. Received LBR is delivered to upper logic   */
u32 vtss_mep_supp_lbm_conf_set(const u32                         instance,
                               const vtss_mep_supp_lbm_conf_t    *const conf);

typedef struct
{
    u32     transaction_id;
    u8      mac[VTSS_MEP_SUPP_MAC_LENGTH];
} vtss_mep_supp_lbr_t;

/* instance:    Instance number of MEP   */
/* count:       Number of LBR received   */
/* lbr:         LBR array                */
/* Received MAC and transaction ID for each LBR - this is only supported for SW based Discovery-like LB */
u32 vtss_mep_supp_lbr_get(const u32            instance,
                          u32                  *const count,
                          vtss_mep_supp_lbr_t  lbr[VTSS_MEP_SUPP_LBR_MAX]);


typedef struct
{
    u64     lbr_counter;      /* Received LBR PDU count                                          */
    u64     lbm_counter;      /* Transmitted LBM PDU count                                       */
    u64     oo_counter;       /* Counted received Out of Order sequence numbers (VOE capability) */
    u32     trans_id;         /* The next transmitted transaction id                             */
} vtss_mep_supp_lb_status_t;

/* instance:    Instance number of MEP   */
/* status:      LB status   */
/* The LB status counters - this is only supported for HW based Series-like LB */
u32 vtss_mep_supp_lb_status_get(const u32                  instance,
                                vtss_mep_supp_lb_status_t  *const status);

typedef enum
{
    VTSS_MEP_SUPP_PATTERN_ALL_ZERO,  /* All zero data pattern    */
    VTSS_MEP_SUPP_PATTERN_ALL_ONE,   /* All zero data pattern    */
    VTSS_MEP_SUPP_PATTERN_0XAA,      /* 10101010 data pattern    */
} vtss_mep_supp_pattern_t;

#define VTSS_MEP_SUPP_TST_SIZE_MAX  9600    /* The maximum TST frame size - includung DMAC + SMAC + T/L + FCS = 18 bytes - excluding any tags that might be added */
#define VTSS_MEP_SUPP_TST_SIZE_MIN  64      /* The minimum TST frame size - includung DMAC + SMAC + T/L + FCS = 18 bytes - excluding any tags that might be added  */
#if defined(VTSS_ARCH_SERVAL)
#define VTSS_MEP_SUPP_TST_RATE_MAX  2500000     /* 250000 Kbps = 2,5 Gbps */
#else
#define VTSS_MEP_SUPP_TST_RATE_MAX  400000      /* 400000 Kbps = 400 Mbps */
#endif
typedef struct
{
    BOOL                    enable;                           /* Enable/disable transmission of TST PDU                                                                 */
    BOOL                    enable_rx;                        /* Enable/disable reception and analyze of TST PDU                                                        */
    BOOL                    dei;                              /* DEI                                                                                                    */
    u32                     prio;                             /* Priority                                                                                               */
    u32                     size;                             /* Size of TST frame to send - max. VTSS_MEP_SUPP_TST_SIZE_MAX bytes - min. VTSS_MEP_SUPP_TST_SIZE_MIN    */
    u32                     add_cnt;                          /* Number of bytes that will be added when leaving port. A tunnel EVC adds a tag on NNI                   */
    u32                     rate;                             /* The transmission rate in Kbps - max VTSS_MEP_SUPP_TST_RATE_MAX - min 1                                 */
    u8                      dmac[VTSS_MEP_SUPP_MAC_LENGTH];   /* TST destination MAC                                                                                    */
    u32                     transaction_id;                   /* Transaction id start value                                                                             */
    vtss_mep_supp_pattern_t pattern;                          /* Data pattern in Test TLV                                                                               */
    BOOL                    sequence;                         /* Sequence number will be inserted                                                                       */
} vtss_mep_supp_tst_conf_t;

/* instance:    Instance number of MEP               */
/* enable:      Enable/disable of TST transmission   */
/* conf:        Configuration                        */
/* A TST transmission is enabled or disabled against a unicast MAC   */
u32 vtss_mep_supp_tst_conf_set(const u32                         instance,
                               const vtss_mep_supp_tst_conf_t    *const conf);



typedef struct
{
    u64     tx_counter;   /* Transmitted TST PDU count                                       */
    u64     rx_counter;   /* Received TST PDU count (VOE capability)                         */
    u64     oo_counter;   /* Counted received Out of Order sequence numbers (VOE capability) */
} vtss_mep_supp_tst_status_t;

/* instance:    Instance number of MEP   */
/* status:      TST status   */
/* The TST status counters */
u32 vtss_mep_supp_tst_status_get(const u32                   instance,
                                 vtss_mep_supp_tst_status_t  *const status);


/* instance:    Instance number of MEP   */
/* Clear the TST status counters */
u32 vtss_mep_supp_tst_clear(const u32    instance);


typedef struct 
{
    vtss_mep_supp_period_t            period;                                   /* Transmission period                                           */
    BOOL                              dei;                                      /* DEI                                                           */
    BOOL                              protection;                               /* TRUE will transmit the first three frames as fast as possible */
    u8                                dmac[VTSS_MEP_SUPP_MAC_LENGTH];           /* AIS destination MAC                                           */
    u32                               flow_count;                               /* Number of client flows                                        */
    u8                                level[VTSS_MEP_SUPP_CLIENT_FLOWS_MAX];    /* Client MEG level                                              */
    u32                               prio[VTSS_MEP_SUPP_CLIENT_FLOWS_MAX];     /* Client Piority                                                */
    vtss_mep_supp_flow_t              flows[VTSS_MEP_SUPP_CLIENT_FLOWS_MAX];    /* Client flows                                                  */
} vtss_mep_supp_ais_conf_t;

u32 vtss_mep_supp_ais_conf_set(const u32  instance,  
                               const vtss_mep_supp_ais_conf_t   *const conf);

u32 vtss_mep_supp_ais_set(const u32  instance,  
                          const BOOL enable);


typedef struct 
{
    BOOL                              enable;
    vtss_mep_supp_period_t            period;                                   /* Transmission period                                           */
    BOOL                              dei;                                      /* DEI                     */
    u8                                dmac[VTSS_MEP_SUPP_MAC_LENGTH];           /* LCK destination MAC     */
    u32                               flow_count;                               /* Number of client flows  */
    u8                                level[VTSS_MEP_SUPP_CLIENT_FLOWS_MAX];    /* Client MEG level        */
    u32                               prio[VTSS_MEP_SUPP_CLIENT_FLOWS_MAX];     /* Client Piority          */
    vtss_mep_supp_flow_t              flows[VTSS_MEP_SUPP_CLIENT_FLOWS_MAX];    /* Client flows            */
} vtss_mep_supp_lck_conf_t;

u32 vtss_mep_supp_lck_conf_set(const u32  instance,
                               const vtss_mep_supp_lck_conf_t   *const conf);





u32 vtss_mep_supp_loc_state(const u32   instance,
                            const BOOL  state);



u32 vtss_mep_supp_rx_isdx_get(const u32  instance);

/****************************************************************************/
/*  MEP lower support call out to upper logic                               */
/****************************************************************************/

/* instance:    Instance number of MEP                               */
/* This is called by lower support when new defect state is detected */
void vtss_mep_supp_new_defect_state(const u32  instance);

/* instance:    Instance number of MEP                                                   */
/* This is called by lower support when new ccm state (unexpected values) is detected    */
void vtss_mep_supp_new_ccm_state(const u32  instance);

/* instance:    Instance number of MEP                           */
/* This is called by lower support when new APS PDU is received  */
void vtss_mep_supp_new_aps(const u32  instance,
                           const u8   *aps);

/* instance:    Instance number of MEP                          */
/* This is called by lower support when new LTR PDU is received */
void vtss_mep_supp_new_ltr(const u32  instance);

/* instance:    Instance number of MEP                          */
/* This is called by lower support when new LBR PDU is received */
void vtss_mep_supp_new_lbr(const u32  instance);

/* instance:    Instance number of MEP                          */
/* This is called by lower support when new DMR PDU is received */
void vtss_mep_supp_new_dmr(const u32  instance);

/* instance:    Instance number of MEP                          */
/* This is called by lower support when new DMR PDU is received */
void vtss_mep_supp_new_dm1(const u32  instance);

/* instance:    Instance number of MEP                          */
/* This is called by lower support when new TST PDU is received. The size is al inclusive - Preamble + TAG + CRC + IFG */
void vtss_mep_supp_new_tst(const u32   instance,
                           const u32   frame_size);




/****************************************************************************/
/*  MEP platform call in to lower support                                   */
/****************************************************************************/

/* stop:    Return value indicating if calling this thread has to stop.                                                 */
/* This is the thread driving timing . Has to be called every 'timer_resolution' ms. by upper logic until 'stop'        */
/* Initially this thread is not called until callout on vtss_mep_supp_timer_start()                                     */
void vtss_mep_supp_timer_thread(BOOL  *const stop);

/* This is the thread driving the state machine. Has to be call by upper logic when calling out on vtss_mep_supp_run() */ 
void vtss_mep_supp_run_thread(void);

/* timer_resolution:    This is the interval of calling vtss_mep_supp_run_thread() in ms.    */
/* This is the initializing of lower support. Has to be called by upper logic                */
u32 vtss_mep_supp_init(const u32 timer_resolution, u32 up_mep_loop_port);


typedef struct
{
    u32 vid;
    u32 qos;
    u32 pcp;
    u32 oam_info;
    u32 isdx;
    BOOL dei;
    BOOL tagged;
} vtss_mep_supp_frame_info_t;

/* This function is called with received OAM PDU frames */
void vtss_mep_supp_rx_frame(u32 port, vtss_mep_supp_frame_info_t *frame_info, u8 frame[], u32 len, vtss_timestamp_t *rx_time, u32 rx_count);

#define VTSS_MEP_SUPP_VOE_CCM_PERIOD_EV       (1<<7)
#define VTSS_MEP_SUPP_VOE_CCM_PRIORITY_EV     (1<<6)
#define VTSS_MEP_SUPP_VOE_CCM_ZERO_PERIOD_EV  (1<<5)
#define VTSS_MEP_SUPP_VOE_CCM_RX_RDI_EV       (1<<4)
#define VTSS_MEP_SUPP_VOE_CCM_LOC_EV          (1<<3)
#define VTSS_MEP_SUPP_VOE_CCM_MEP_ID_EV       (1<<2)
#define VTSS_MEP_SUPP_VOE_CCM_MEG_ID_EV       (1<<1)
#define VTSS_MEP_SUPP_VOE_MEG_LEVEL_EV        (1<<0)
/* This function is called on detected VOE interrupt - ev_mask is a return value containing indication of detected events */
void vtss_mep_supp_voe_interrupt(void);

/* This function is called to determine if this ISDX relates to a VOE based UP-MEP */
BOOL vtss_mep_supp_voe_up(u32 isdx, u32 *voe);

/* This function is called to inform that protection has changed for this instance */
void vtss_mep_supp_protection_change(u32 instance);

/****************************************************************************/
/*  MEP lower support call out to platform                                  */
/****************************************************************************/

/* This is call by MEP lower support to lock/unlock critical code protection */
void vtss_mep_supp_crit_lock(void);

void vtss_mep_supp_crit_unlock(void);

/* This is called by MEP lower support when vtss_mep_supp_run_thread(void) has to be called */
void vtss_mep_supp_run(void);

/* This is called by MEP lower support when vtss_mep_supp_timer_thread(BOOL  *const stop) has to be called until 'stop' is indicated */
void vtss_mep_supp_timer_start(void);

/* This is called by MEP lower support in order to do debug tracing */
void vtss_mep_supp_trace(const char   *string,
                         const u32    param1,
                         const u32    param2,
                         const u32    param3,
                         const u32    param4);



u8 *vtss_mep_supp_packet_tx_alloc(u32 size);

void vtss_mep_supp_packet_tx_free(u8 *buffer);

void vtss_mep_supp_packet_tx_cancel(u32 instance,  u8 *buffer);

typedef void (*vtss_mep_supp_cb_t)(u32 instance, u8 *frame, vtss_timestamp_t tx_time,  u64 frame_count);
typedef struct
{
    u32                     instance;
    vtss_mep_supp_cb_t      cb;
} vtss_mep_supp_tx_done_t;

typedef struct
{
    u32                       corr_offset;
    u32                       tc;             /* hw counter corresponding to the TX timestamp */
    vtss_timestamp_t          ts;
    u32                       tx_time;        /* hw counter corresponding to the pre_done correction field update time */
    vtss_timeinterval_t       sw_egress_delay;/* set to the calculated min sw egress delay in the tx_event_onestep function */
    vtss_mep_supp_tx_done_t   *ts_done;
    u8                        tag_cnt;
} vtss_mep_supp_onestep_extra_t;


/* latency observed in onestep tx timestamping */
typedef struct
{
    vtss_timeinterval_t max;
    vtss_timeinterval_t min;
    vtss_timeinterval_t mean;
    u32 cnt;
} vtss_mep_supp_observed_egr_lat_t;

#define VTSS_MEP_SUPP_INDX_INVALID  0xFFFFFFFF
typedef struct
{
    BOOL  bypass;         /* Frame is bypassing the analyzer */
    BOOL  vid_inj;        /* Injection into classified VID */
    u32   maskerade_port; /* Maskerading is disabled if VTSS_PORT_NO_NONE */
    u32   isdx;           /* ISDX */
    u32   vid;            /* Classified VID */
    u32   qos;            /* Classified QOS */
    u32   pcp;            /* Classified PCP - EVC COS ID */
    u32   dp;             /* Claasified Drop Precedence */
} vtss_mep_supp_tx_frame_info_t;

typedef enum {
  VTSS_MEP_SUPP_OAM_TYPE_NONE = 0,
  VTSS_MEP_SUPP_OAM_TYPE_CCM,
  VTSS_MEP_SUPP_OAM_TYPE_CCM_LM,
  VTSS_MEP_SUPP_OAM_TYPE_LBM,
  VTSS_MEP_SUPP_OAM_TYPE_LBR,
  VTSS_MEP_SUPP_OAM_TYPE_LMM,
  VTSS_MEP_SUPP_OAM_TYPE_LMR,
  VTSS_MEP_SUPP_OAM_TYPE_DMM,
  VTSS_MEP_SUPP_OAM_TYPE_DMR,
  VTSS_MEP_SUPP_OAM_TYPE_1DM,
  VTSS_MEP_SUPP_OAM_TYPE_LTM,
  VTSS_MEP_SUPP_OAM_TYPE_LTR,
  VTSS_MEP_SUPP_OAM_TYPE_GENERIC,
} vtss_mep_supp_oam_type_t;

u32 vtss_mep_supp_packet_tx(u32                            instance,
                            vtss_mep_supp_tx_done_t        *done,
                            BOOL                           port[VTSS_PORT_ARRAY_SIZE],
                            u32                            len,
                            unsigned char                  *frm,
                            vtss_mep_supp_tx_frame_info_t  *frame_info,
                            u32                            rate,
                            BOOL                           count_enable,
                            u32                            seq_offset,
                            vtss_mep_supp_oam_type_t       oam_type);

u32 vtss_mep_supp_packet_tx_1(u32                            instance,
                              vtss_mep_supp_tx_done_t        *done,
                              u32                            port,
                              u32                            len,
                              unsigned char                  *frm,
                              vtss_mep_supp_tx_frame_info_t  *frame_info,
                              vtss_mep_supp_oam_type_t       oam_type);

u32 vtss_mep_supp_packet_tx_two_step_ts(u32             instance,
                                        vtss_mep_supp_tx_done_t     *done,
                                        BOOL                        port[VTSS_PORT_ARRAY_SIZE],
                                        u32                         len,
                                        unsigned char               *frm);

u32 vtss_mep_supp_packet_tx_1_two_step_ts(u32           instance,
                                          vtss_mep_supp_tx_done_t   *done,
                                          u32                       port,
                                          u32                       len,
                                          unsigned char             *frm);
                              
u32 vtss_mep_supp_packet_tx_1_one_step_ts(u32                            instance,
                                          vtss_mep_supp_tx_done_t        *done,
                                          u32                            port,
                                          u32                            len,
                                          unsigned char                  *frm,
                                          vtss_mep_supp_tx_frame_info_t  *frame_info,
                                          u32                            hw_time,
                                          vtss_mep_supp_onestep_extra_t  *onestep_extra,
                                          vtss_mep_supp_oam_type_t       oam_type);

u32 vtss_mep_supp_packet_tx_one_step_ts(u32                           instance,
                                        vtss_mep_supp_tx_done_t       *done,
                                        BOOL                          port[VTSS_PORT_ARRAY_SIZE],
                                        u32                           len,
                                        unsigned char                 *frm,
                                        vtss_mep_supp_tx_frame_info_t *frame_info,
                                        u32                           hw_time,
                                        vtss_mep_supp_onestep_extra_t *buf_handle,
                                        vtss_mep_supp_oam_type_t      oam_type);

BOOL vtss_mep_supp_check_hw_timestamp(void);

i32 vtss_mep_supp_delay_calc(const vtss_timestamp_t *x,
                             const vtss_timestamp_t *y,
                             const BOOL             is_ns);

BOOL vtss_mep_supp_check_phy_timestamp(u32 port, u8 *mac);

void vtss_mep_supp_timestamp_get(vtss_timestamp_t *timestamp, u32 *tc);

BOOL vtss_mep_supp_check_forwarding(u32   i_port,
                                    u32   *f_port,
                                    u8    mac[VTSS_MEP_SUPP_MAC_LENGTH],
                                    u32   vid);

/* This function should return number of transmitted frames related to the pointer 'frm' */
void vtss_mep_supp_packet_tx_frm_cnt(u8 *frm, u64 *frm_cnt);

/* instance:    Instance number of MEP                                         */
/* This is called by lower support to retrive frame counters for this instance */
void vtss_mep_supp_counters_get(const u32       instance,
                                u32             *const rx_frames,
                                u32             *const tx_frames);

/* Control the UNI port configuration*/
void vtss_mep_supp_port_conf(const u32       instance,
                             const u32       port,
                             const BOOL      up_mep,
                             const BOOL      out_of_service);/* Control the host loop on the port */

u32 vtss_mep_supp_dmr_timestamps_get(const u32                    instance,
                                     vtss_mep_mgmt_dm_timestamp_t *const dm1_timestamp_far_to_near,
                                     vtss_mep_mgmt_dm_timestamp_t *const dm1_timestamp_near_to_far);

#if defined(VTSS_ARCH_SERVAL)
typedef struct {
    BOOL                enable;
    vtss_mce_pcp_mode_t pcp_mode;
    vtss_tagprio_t      pcp;
    vtss_mce_dei_mode_t dei_mode;
    vtss_dei_t          dei;
} vtss_mep_supp_evc_mce_outer_tag_t;

typedef struct {
    BOOL    enable;
    u32     pcp;
} vtss_mep_supp_evc_mce_key_t;

typedef enum {
    VTSS_MEP_SUPP_EVC_NNI_TYPE_E_NNI,
    VTSS_MEP_SUPP_EVC_NNI_TYPE_I_NNI
} vtss_mep_supp_evc_nni_type_t;

#define VTSS_MEP_SUPP_COS_ID_SIZE 8
void vtss_mep_supp_evc_mce_info_get(const u32                          evc_inst,
                                    vtss_mep_supp_evc_nni_type_t       *nni_type,
                                    vtss_mep_supp_evc_mce_outer_tag_t  nni_outer_tag[VTSS_MEP_SUPP_COS_ID_SIZE],
                                    vtss_mep_supp_evc_mce_key_t        nni_key[VTSS_MEP_SUPP_COS_ID_SIZE]);
#endif
#endif /* _VTSS_MEP_SUPP_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
