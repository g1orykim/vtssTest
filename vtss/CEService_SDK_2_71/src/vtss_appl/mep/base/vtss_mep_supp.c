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
 
 $Id: vtss_mep_supp.c,v 1.60 2011/03/22 15:45:14 henrikb Exp $
 $Revision: 1.60 $

*/

#include "vtss_mep_supp_api.h"
#include "vtss_mep_api.h"
#include "vtss_api.h"
#include "vtss_mep.h" /* Internal interface */
#include <string.h>
#include <stdlib.h>

/*lint -sem( vtss_mep_supp_crit_lock, thread_lock ) */
/*lint -sem( vtss_mep_supp_crit_unlock, thread_unlock ) */

/****************************************************************************/
/*  MEP global variables                                                    */
/****************************************************************************/


/****************************************************************************/
/*  MEP local variables                                                     */
/****************************************************************************/
#define DEBUG_PHY_TS      0  //James
#define F_2DM_FOR_1DM       1  /* Use DMM/DMR packets to measure one-way DM if set */

#define OAM_TYPE_CCM      1
#define OAM_TYPE_LBR      2
#define OAM_TYPE_LBM      3
#define OAM_TYPE_LTR      4
#define OAM_TYPE_LTM      5
#define OAM_TYPE_AIS      33
#define OAM_TYPE_LCK      35
#define OAM_TYPE_TST      37
#define OAM_TYPE_LAPS     39
#define OAM_TYPE_RAPS     40
#define OAM_TYPE_LMR      42
#define OAM_TYPE_LMM      43
#define OAM_TYPE_1DM      45
#define OAM_TYPE_DMR      46
#define OAM_TYPE_DMM      47
#define OAM_TYPE_1DM_FUP  0xFE /* Use the reserved code 0xfe as the opcode of follow-up packets */
#define OAM_TYPE_DMR_FUP  0xFF /* Use the reserved code 0xff as the opcode of follow-up packets */

#define HEADER_MAX_SIZE      30

#define CCM_PDU_LENGTH       75
#define LMR_PDU_LENGTH       17
#define LMM_PDU_LENGTH       17
#define LAPS_PDU_LENGTH      9
#define RAPS_PDU_LENGTH      37
#define LTM_PDU_LENGTH       33
#define LTR_PDU_LENGTH       40
#define LBM_PDU_LENGTH       12   /* This is only the size of the 'emty' LBM PDU - without contained data */
#define DM1_PDU_STD_LENGTH   21
#define DM1_PDU_VTSS_LENGTH  DM1_PDU_STD_LENGTH + 4 /* Add "VTSS" after data for remote site to know a follow-up packet is expected */
#define DM1_PDU_LENGTH       DM1_PDU_VTSS_LENGTH
#define DMM_PDU_STD_LENGTH   37
#define DMM_PDU_VTSS_LENGTH  DMM_PDU_STD_LENGTH + 4 /* Add "VTSS" after data for remote site to know a follow-up packet is expected */
#define DMM_PDU_LENGTH       DMM_PDU_VTSS_LENGTH
#define DMR_PDU_LENGTH       37
#define AIS_PDU_LENGTH       5
#define LCK_PDU_LENGTH       5
#define TST_PDU_LENGTH       12   /* This is only the size of the 'emty' TST PDU - without contained data */


#define EVENT_IN_CONFIG           0x0000000000000001LL
#define EVENT_IN_CCM_CONFIG       0x0000000000000002LL
#define EVENT_IN_CCM_GEN          0x0000000000000004LL
#define EVENT_IN_CCM_RDI          0x0000000000000008LL
#define EVENT_IN_LMM_CONFIG       0x0000000000000010LL
#define EVENT_IN_APS_CONFIG       0x0000000000000020LL
#define EVENT_IN_APS_FORWARD      0x0000000000000040LL
#define EVENT_IN_APS_TXDATA       0x0000000000000080LL
#define EVENT_IN_LTM_CONFIG       0x0000000000000100LL
#define EVENT_IN_LBM_CONFIG       0x0000000000000200LL
#define EVENT_IN_DMM_CONFIG       0x0000000000000400LL
#define EVENT_IN_1DM_CONFIG       0x0000000000000800LL
#define EVENT_IN_LCK_CONFIG       0x0000000000001000LL
#define EVENT_IN_TST_CONFIG       0x0000000000002000LL
#define EVENT_IN_AIS_SET          0x0000000000004000LL
#define EVENT_IN_DEFECT_TIMER     0x0000000000008000LL
#define EVENT_IN_RX_CCM_TIMER     0x0000000000010000LL
#define EVENT_IN_TX_CCM_TIMER     0x0000000000020000LL
#define EVENT_IN_TX_APS_TIMER     0x0000000000040000LL
#define EVENT_IN_TX_LMM_TIMER     0x0000000000080000LL
#define EVENT_IN_TX_LBM_TIMER     0x0000000000100000LL
#define EVENT_IN_TX_DMM_TIMER     0x0000000000200000LL
#define EVENT_IN_TX_1DM_TIMER     0x0000000000400000LL
#define EVENT_IN_RX_APS_TIMER     0x0000000000800000LL
#define EVENT_IN_RX_LBM_TIMER     0x0000000001000000LL
#define EVENT_IN_RX_DMR_TIMER     0x0000000002000000LL
#define EVENT_IN_TX_AIS_TIMER     0x0000000004000000LL
#define EVENT_IN_TX_LCK_TIMER     0x0000000008000000LL
#define EVENT_IN_RDI_TIMER        0x0000000010000000LL
#define EVENT_IN_TST_TIMER        0x0000000020000000LL


#define EVENT_IN_MASK             0x000000003FFFFFFFLL

#define EVENT_OUT_NEW_DEFECT      0x8000000000000000LL
#define EVENT_OUT_NEW_APS         0x4000000000000000LL
#define EVENT_OUT_NEW_DMR         0x2000000000000000LL
#define EVENT_OUT_NEW_DM1         0x1000000000000000LL
#define EVENT_OUT_MASK            0xF000000000000000LL

#define APS_DATA_LENGTH ((data->aps_config.type == VTSS_MEP_SUPP_L_APS) ? VTSS_MEP_SUPP_LAPS_DATA_LENGTH : VTSS_MEP_SUPP_RAPS_DATA_LENGTH)
#define APS_PDU_LENGTH ((data->aps_config.type == VTSS_MEP_SUPP_L_APS) ? LAPS_PDU_LENGTH : RAPS_PDU_LENGTH)

#define TX_QOS_NORMAL   7

#define VOE_EVENT_MASK (VTSS_OAM_VOE_EVENT_CCM_PERIOD | VTSS_OAM_VOE_EVENT_CCM_PRIORITY | VTSS_OAM_VOE_EVENT_CCM_ZERO_PERIOD | VTSS_OAM_VOE_EVENT_CCM_RX_RDI | \
                        VTSS_OAM_VOE_EVENT_CCM_LOC | VTSS_OAM_VOE_EVENT_CCM_MEP_ID | VTSS_OAM_VOE_EVENT_CCM_MEG_ID)

#define MEG_ID_LENGTH (2+(VTSS_MEP_SUPP_MEG_CODE_LENGTH-1)+2+(VTSS_MEP_SUPP_MEG_CODE_LENGTH-1))

#define GENERIC_AIS     0
#define GENERIC_LCK     1
#define GENERIC_LAPS    2
#define GENERIC_RAPS    3

typedef struct
{
    i32     near_los_counter;
    i32     far_los_counter;
    u32     near_tx_counter;
    u32     far_tx_counter;

    u32     near_CT1, far_CT1;      /* See Apendix III in Y.1731  */
    u32     near_CR1, far_CR1;

    u32     rx_counter;
    u32     tx_counter;
} lm_counter_t;

typedef struct
{
    u32     timer;
    u32     old_period;
} defect_timer_t;

typedef struct
{
    vtss_mep_supp_ccm_state_t     state;
    u8                            unexp_meg_id[MEG_ID_LENGTH];
} ccm_state_t;

typedef struct
{
    vtss_mep_supp_conf_t           config;                  /* Input from upper logic */
    vtss_mep_supp_ccm_conf_t       ccm_config;
    vtss_mep_supp_gen_conf_t       ccm_gen;
    BOOL                           ccm_rdi;
    vtss_mep_supp_lmm_conf_t       lmm_config;
    vtss_mep_supp_dmm_conf_t       dmm_config;
    vtss_mep_supp_dm1_conf_t       dm1_config;
    vtss_mep_supp_aps_conf_t       aps_config;
    u8                             aps_txdata[VTSS_MEP_SUPP_RAPS_DATA_LENGTH];
    BOOL                           aps_tx;
    BOOL                           aps_event;
    BOOL                           aps_forward;
    BOOL                           ais_enable;
    vtss_mep_supp_ltm_conf_t       ltm_config;
    vtss_mep_supp_lbm_conf_t       lbm_config;
    vtss_mep_supp_ais_conf_t       ais_config;
    vtss_mep_supp_lck_conf_t       lck_config;
    vtss_mep_supp_tst_conf_t       tst_config;

    lm_counter_t                   ccm_lm_counter;          /* Output to upper logic */
    u8                             peer_mac[VTSS_MEP_SUPP_PEER_MAX][VTSS_MEP_SUPP_MAC_LENGTH];
    lm_counter_t                   lmm_lm_counter;
    vtss_mep_supp_defect_state_t   defect_state;
    ccm_state_t                    ccm_state;
    vtss_mep_supp_lb_status_t      lb_state;
    u32                            lbr_cnt;
    vtss_mep_supp_lbr_t            lbr[VTSS_MEP_SUPP_LBR_MAX];
    u32                            ltr_cnt;
    vtss_mep_supp_ltr_t            ltr[VTSS_MEP_SUPP_LTR_MAX];

    u64                            tst_tx_cnt;
    u32                            dmm_tx_cnt;
    u32                            dmr_rx_cnt;
    u32                            dmr_rx_err_cnt;
    u32                            dmr_rx_tout_cnt;
    u32                            dm1_tx_cnt;
    u32                            dm1_rx_cnt_far_to_near;
    u32                            dm1_rx_err_cnt_far_to_near;
    u32                            dm1_rx_cnt_near_to_far;
    u32                            dm1_rx_err_cnt_near_to_far;
    u32                            dmm_late_txtime_cnt;
    u32                            dmr[VTSS_MEP_SUPP_DM_MAX];
    u32                            dm1_far_to_near[VTSS_MEP_SUPP_DM_MAX];
    u32                            dm1_near_to_far[VTSS_MEP_SUPP_DM_MAX];

    u64                            event_flags;                            /* Flags used to indicate what input events has activated the 'run' thread */

    u32                            tx_header_size;                         /* Size of complete transmitted header (index for start of OAM PDU) */
    u32                            ifh_size;
    u32                            rx_tag_cnt;
    u32                            tx_isdx, rx_isdx;                       /* Transmit and receive ISDX for VOE based MEP */
    u8                             exp_meg[MEG_ID_LENGTH];                 /* expected MA-ID/MEG-ID */
    u32                            exp_meg_len;                            /* length of expected MEG */

    BOOL                           defect_timer_active;                    /* Indicating that a defect timer is active */
    BOOL                           ccm_defect_active;                      /* Indicating that a VOE CCM defect is active driving a "fast" poll of CCM PDU */
    BOOL                           dmm_tx_active;                          /* Dont send DMM until previously DMM is done */
    BOOL                           dm1_fup_stdby;                          /* Dont send DM1 until previously DM1 is done */
    BOOL                           dmr_received;                          
    u32                            loc_timer_val;
    u32                            ccm_timer_val;
    u32                            aps_timer_val;
    u32                            lmm_timer_val;
    u32                            lbm_count;
    u32                            aps_count;
    u32                            ais_count, ais_inx;
    u32                            lck_count, lck_inx;
    u32                            dmr_fup_port;
    i32                            dmr_dev_delay;
                               
    u32                            is2_rx_cnt;                             /* This is for Serval to compensate MIP PDU copied by IS2 to CPU and therefor is not counted by Up-MEP VOE */

    defect_timer_t                 dLevel;                                 /* Defect timers */
    defect_timer_t                 dMeg;
    defect_timer_t                 dMep;
    defect_timer_t                 dPeriod[VTSS_MEP_SUPP_PEER_MAX];
    defect_timer_t                 dPrio[VTSS_MEP_SUPP_PEER_MAX];
    defect_timer_t                 dAis;
    defect_timer_t                 dLck;
                               
    u32                            tx_ccm_timer;                           /* Assorted timers */
    u32                            tx_aps_timer;
    u32                            tx_lmm_timer;
    u32                            tx_lbm_timer;
    u32                            tx_dmm_timer;
    u32                            tx_dmm_timeout_timer;
    u32                            tx_dm1_timer;
    u32                            tx_ais_timer;
    u32                            tx_lck_timer;
    u32                            rx_aps_timer;
    u32                            rx_lbm_timer;
    u32                            rx_ais_timer;
    u32                            rx_lck_timer;
    u32                            rx_ccm_timer[VTSS_MEP_SUPP_PEER_MAX];
    u32                            rdi_timer;
    u32                            tst_timer;

    BOOL                           hw_ccm_transmitting;                    /* This is added temporary - I hope - in order to only transmit HW CCM once */
    u32                            voe_idx;                                /* VOE instance index - only relevant for VOE capable HW */
    BOOL                           voe_config;                             /* Indicating that the VOE is configured - to avoid repeating VOE config with same values */
    BOOL                           ccm_start;
    BOOL                           lbm_start;
    BOOL                           tst_start;
    BOOL                           lm_start;

    u8                             *tx_ccm_sw;                             /* HW CCM frame tx buffer */
    u8                             *tx_ccm_hw;                             /* SW CCM frame tx buffer */
    u8                             *tx_aps;                                /* APS frame tx buffer */
    u8                             *tx_lmm;                                /* LMM frame tx buffer */
    u8                             *tx_lmr;                                /* LMR frame tx buffer */
    u8                             *tx_ltm;                                /* LTM frame tx buffer */
    u8                             *tx_ltr;                                /* LTR frame tx buffer */
    u8                             *tx_lbm;                                /* LBM frame tx buffer */
    u8                             *tx_lbr;                                /* LBR frame tx buffer */
    u8                             *tx_dm1;                                /* 1DM frame tx buffer */
    u8                             *tx_dmm;                                /* DMM frame tx buffer */
    u8                             *tx_dmr;                                /* DMR frame tx buffer */
    u8                             *tx_ais;                                /* AIS frame tx buffer */
    u8                             *tx_lck;                                /* LCK frame tx buffer */
    u8                             *tx_tst;                                /* TST frame tx buffer */

    BOOL                           tx_ccm_sw_ongoing;
    BOOL                           tx_dmm_ongoing;
    BOOL                           tx_dm1_ongoing;
    BOOL                           tx_dmr_ongoing;
    BOOL                           tx_lmm_ongoing;
    BOOL                           tx_lmr_ongoing;
    BOOL                           tx_lbm_ongoing;
    BOOL                           tx_lbr_ongoing;
    BOOL                           tx_aps_ongoing;
    BOOL                           tx_ais_ongoing;
    BOOL                           tx_lck_ongoing;

    vtss_timestamp_t               tx_dmm_fup_timestamp;                   /* DMM transmission time (follow-up) */
    vtss_timestamp_t               rx_dmm_timestamp;                       /* DMM receive time      */
    vtss_timestamp_t               rx_dmr_timestamp;                       /* DMR receive time      */
    vtss_timestamp_t               rx_dm1_timestamp;                       /* DM1 receive time      */

    vtss_mep_supp_tx_done_t        done_ccm;                               /* CCM tx done handler */
    vtss_mep_supp_tx_done_t        done_lmm;                               /* LMM tx done handler */
    vtss_mep_supp_tx_done_t        done_lmr;                               /* LMR tx done handler */
    vtss_mep_supp_tx_done_t        done_ltm;                               /* LTM tx done handler */
    vtss_mep_supp_tx_done_t        done_ltr;                               /* LTR tx done handler */
    vtss_mep_supp_tx_done_t        done_lbm;                               /* LBM tx done handler */
    vtss_mep_supp_tx_done_t        done_lbr;                               /* LBR tx done handler */
    vtss_mep_supp_tx_done_t        done_aps;                               /* APS tx done handler */
    vtss_mep_supp_tx_done_t        done_dmm;                               /* DMM tx done handler */
    vtss_mep_supp_tx_done_t        done_dm1;                               /* 1DM tx done handler */
    vtss_mep_supp_tx_done_t        done_dmr;                               /* DMR tx done handler */
    vtss_mep_supp_tx_done_t        done_ais;                               /* AIS tx done handler */
    vtss_mep_supp_tx_done_t        done_lck;                               /* LCK tx done handler */
    vtss_mep_supp_tx_done_t        done_tst;                               /* TST tx done handler */

    vtss_mep_supp_onestep_extra_t  onestep_extra_m;                        /* One step extra buffer for dmm/dm1 */
    vtss_mep_supp_onestep_extra_t  onestep_extra_r;                        /* One step extra buffer for dmr */                                   
    vtss_timestamp_t               rxTimef;                                /* DMM receive time returned in DMR */
    vtss_timestamp_t               txTimeb;                                /* DMR transmit time extracted from DMR */
    BOOL                           dmr_ts_ok;                              /* set when timestamps received, cleared when read */

} supp_instance_data_t;


static supp_instance_data_t    instance_data[VTSS_MEP_SUPP_CREATED_MAX];     /* MEP instance data */
static u32                     timer_res;                                    /* Timer resolution */
static u8                      lm_null_counter[12];
static u32                     pdu_period_to_timer[8];
static vtss_mep_supp_period_t  pdu_period_to_conf[8] = {VTSS_MEP_SUPP_PERIOD_INV,
                                                        VTSS_MEP_SUPP_PERIOD_300S,
                                                        VTSS_MEP_SUPP_PERIOD_100S,
                                                        VTSS_MEP_SUPP_PERIOD_10S,
                                                        VTSS_MEP_SUPP_PERIOD_1S,
                                                        VTSS_MEP_SUPP_PERIOD_6M,
                                                        VTSS_MEP_SUPP_PERIOD_1M,
                                                        VTSS_MEP_SUPP_PERIOD_6H};
#ifdef VTSS_ARCH_SERVAL
static BOOL     voe_idx_used[VTSS_OAM_VOE_CNT];
static u32      voe_to_mep[VTSS_OAM_VOE_CNT];
static BOOL voe_down_mep_present(BOOL port[],  u32 vid,  u32 *voe_idx);
#endif

/* VOE Up-MEP loop port - this is initialized from platform */
static u32      voe_up_mep_loop_port = (VTSS_PORT_ARRAY_SIZE-1);


/****************************************************************************/
/*  MEP local functions                                                     */
/****************************************************************************/

#define var_to_string(string, var) {(string)[0] = (var&0xFF000000)>>24;   (string)[1] = (var&0x00FF0000)>>16;   (string)[2] = (var&0x0000FF00)>>8;   (string)[3] = (var&0x000000FF);}

#define string_to_var(string)  (((string)[0]<<24) | ((string)[1]<<16) | ((string)[2]<<8) | (string)[3])

#define c_s_tag(frame)  (((*(frame) == 0x81) && (*((frame)+1) == 0x00)) || ((*(frame) == 0x88) && (*((frame)+1) == 0xA8)))

#ifdef VTSS_ARCH_SERVAL
#define evc_vlan_up(data) (((data->config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) && (data->config.direction == VTSS_MEP_SUPP_UP)) ? TRUE : FALSE)
#define vlan_down(data)  (((data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID) && (data->config.direction == VTSS_MEP_SUPP_DOWN)) ? TRUE : FALSE)
#else
#define evc_vlan_up(data) ((data->config.voe) ? TRUE : FALSE)   /* This is always FALSE on not Serval - this is only done like this as Lint complains if this is just FALSE */
#define vlan_down(data) ((data->config.voe) ? TRUE : FALSE)   /* This is always FALSE on not Serval - this is only done like this as Lint complains if this is just FALSE */
#endif

#define init_frame_info(frame_info) \
            frame_info.bypass = TRUE;\
            frame_info.maskerade_port = VTSS_PORT_NO_NONE;\
            frame_info.isdx = VTSS_MEP_SUPP_INDX_INVALID;\
            frame_info.vid = 0;\
            frame_info.qos = TX_QOS_NORMAL;\
            frame_info.pcp = 0;\
            frame_info.dp = 0;



static void run_async_ccm_gen(u32 instance);
static void run_async_lmm_config(u32 instance);
static void run_async_aps_config(u32 instance);
static void run_dmm_config(u32 instance);
static void run_1dm_config(u32 instance);
static void run_ltm_config(u32 instance);
static void run_lbm_config(u32 instance);
static void run_lck_config(u32 instance);
static void run_ais_set(u32 instance);
static void run_tst_config(u32 instance);
static void run_tx_aps_timer(u32 instance);
static void voe_config(supp_instance_data_t *data);



static void insert_c_s_tag(u8 *header,   u32 tpid,   u32 vid,   u32 prio,   BOOL dei)
{
    header[0] = (tpid & 0xFF00) >> 8;
    header[1] = (tpid & 0xFF);
    header[2] = ((vid & 0x0F00) >> 8) | (prio << 5) | ((dei) ? 0x10 : 0);
    header[3] = (vid & 0xFF);
}

static u32 init_tx_frame(u32 instance,   u8 dmac[],   u8 smac[],   u32 prio,   BOOL dei,   u8 frame[],   BOOL insert_header)
{
    u32 size=0;
    vtss_mep_supp_conf_t *config = &instance_data[instance].config;
    vtss_vlan_conf_t      vlan_sconf;
    vtss_etype_t          s_etype;
    vtss_vlan_port_type_t port_type;

    instance_data[instance].ifh_size = 0;
#ifdef VTSS_ARCH_SERVAL
    u32 length;
    vtss_packet_tx_info_t   header;

    if (insert_header) {  /* extra injection header inserted */
        (void)vtss_packet_tx_info_init(NULL, &header);

        header.masquerade_port = config->port;
        header.switch_frm = TRUE;
        length = VTSS_PACKET_HDR_SIZE_BYTES;
        (void)vtss_packet_tx_hdr_encode(NULL, &header, frame, &length);
        instance_data[instance].ifh_size = length;
        size = length;
    }
#endif

    memcpy(&frame[size], dmac, VTSS_MEP_SUPP_MAC_LENGTH);
    memcpy(&frame[size+VTSS_MEP_SUPP_MAC_LENGTH], smac, VTSS_MEP_SUPP_MAC_LENGTH);

    size += 2* VTSS_MEP_SUPP_MAC_LENGTH;

    (void)vtss_vlan_conf_get(NULL, &vlan_sconf); /* Get the custom S-Port EtherType */

    s_etype = vlan_sconf.s_etype;
    port_type = ((config->flow.mask & VTSS_MEP_SUPP_FLOW_MASK_CUOUTVID) == VTSS_MEP_SUPP_FLOW_MASK_CUOUTVID) ? VTSS_VLAN_PORT_TYPE_S_CUSTOM :
                (config->flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VLAN_PORT_TYPE_S :
                VTSS_VLAN_PORT_TYPE_C;

#define ether_type() ((port_type == VTSS_VLAN_PORT_TYPE_S_CUSTOM) ? s_etype : (port_type == VTSS_VLAN_PORT_TYPE_S) ? 0x88A8 : 0x8100)

#ifdef VTSS_ARCH_SERVAL
    u32                   prio_in, dei_in;
    vtss_qos_port_conf_t  qos_conf;
    vtss_vlan_port_conf_t vlan_conf;

    (void)vtss_qos_port_conf_get(NULL, config->port, &qos_conf);

    if (config->flow.mask & VTSS_MEP_SUPP_FLOW_MASK_EVC_ID) {  /* This is a EVC MEP/MIP */
        if ((config->mode == VTSS_MEP_SUPP_MEP) && (config->direction == VTSS_MEP_SUPP_UP)) {  /* This is a SW/VOE EVC Up-MEP - 'Dummy tag' must be added */
            vlan_conf.port_type = VTSS_VLAN_PORT_TYPE_UNAWARE;      /* Type of dummy tag is depending on the VLAN port configuration */
            (void)vtss_vlan_port_conf_get(NULL, config->port, &vlan_conf);
            port_type = vlan_conf.port_type;

            dei_in = dei;
            prio_in = prio;
            for (prio=0; prio<VTSS_PRIO_ARRAY_SIZE; ++prio) {   /* Search for the QOS mapping that gives correct classified qos and dp - This is only relevant if the PMF_4 fix is active. If not active the QOS port mapping must be 1:1 */
                dei = 0;
                if ((qos_conf.qos_class_map[prio][dei] == prio_in) && (qos_conf.dp_level_map[prio][dei] == dei_in))     break;
                dei = 1;
                if ((qos_conf.qos_class_map[prio][dei] == prio_in) && (qos_conf.dp_level_map[prio][dei] == dei_in))     break;
            }
            if (prio >= VTSS_PRIO_ARRAY_SIZE)   prio = prio_in; /* This should not happen */

            insert_c_s_tag(&frame[size], ether_type(), config->flow.vid, prio, dei);
            size+=4;
        } else if ((config->mode == VTSS_MEP_SUPP_MIP) && (config->direction == VTSS_MEP_SUPP_UP)) {  /* This is a SW EVC Up-MIP (subscriber MIP) */
            insert_c_s_tag(&frame[size], (config->flow.mask & VTSS_MEP_SUPP_FLOW_MASK_CINVID) ? 0x8100 : 0x88A8, config->flow.in_vid, prio, dei);
            size+=4;
#if defined(VTSS_SW_OPTION_EVC)
        } else if (!config->voe && (config->mode == VTSS_MEP_SUPP_MEP) && (config->direction == VTSS_MEP_SUPP_DOWN)) {  /* This is a SW EVC Down-MEP */
            vtss_mep_supp_evc_mce_outer_tag_t nni_outer_tag[VTSS_MEP_SUPP_COS_ID_SIZE];
            vtss_mep_supp_evc_mce_key_t       nni_key[VTSS_MEP_SUPP_COS_ID_SIZE];
            vtss_mep_supp_evc_nni_type_t      nni_type;

            vtss_mep_supp_evc_mce_info_get(config->flow.evc_id, &nni_type, nni_outer_tag, nni_key);
            if (nni_type == VTSS_MEP_SUPP_EVC_NNI_TYPE_E_NNI) { /* E-NNI - COS-ID is mapped according to ECE configuration */
                prio = nni_outer_tag[prio].pcp;
            } else { /* I-NNI - port based mapping */
                if (qos_conf.tag_remark_mode == VTSS_TAG_REMARK_MODE_MAPPED)
                    prio = qos_conf.tag_pcp_map[prio][dei];
            }
            insert_c_s_tag(&frame[size], ether_type(), config->flow.out_vid, prio, dei);
            size+=4;
#endif
        } else if (config->voe && (config->mode == VTSS_MEP_SUPP_MEP) && (config->direction == VTSS_MEP_SUPP_DOWN)) {
            /* This is a VOE EVC Down-MEP */
        }   /* Do nothing - transmit untagged in any case */ else if ((config->mode == VTSS_MEP_SUPP_MIP) && (config->direction == VTSS_MEP_SUPP_DOWN)) {  /* This is a SW EVC Down-MIP (subscriber MIP) */
            insert_c_s_tag(&frame[size], ether_type(), config->flow.out_vid, prio, dei);
            size+=4;
        }
    } else if (config->flow.mask & VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID) {  /* This is a VLAN MEP */
        if ((config->direction == VTSS_MEP_SUPP_UP) && ((dmac[0] != 0x01) || (dmac[1] != 0x19) || (dmac[2] != 0xA7))) {  /* This is a SW/VOE VLAN Up-MEP that is not transmitting RAPS - 'Dummy tag' must be added */
            vlan_conf.port_type = VTSS_VLAN_PORT_TYPE_UNAWARE;      /* Type of dummy tag is depending on the VLAN port configuration */
            (void)vtss_vlan_port_conf_get(NULL, config->port, &vlan_conf);
            port_type = vlan_conf.port_type;

            insert_c_s_tag(&frame[size], ether_type(), config->flow.vid, prio, dei);
            size+=4;
        }
    }
    else {  /* This is a Port MEP */
        if (config->flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_COUTVID | VTSS_MEP_SUPP_FLOW_MASK_SOUTVID)) {
            insert_c_s_tag(&frame[size], ether_type(), config->flow.out_vid, prio, dei);
            size+=4;
        }
    }
#else
    if (config->flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_COUTVID | VTSS_MEP_SUPP_FLOW_MASK_SOUTVID)) {
        insert_c_s_tag(&frame[size], ether_type(), config->flow.out_vid, prio, dei);
        size+=4;
    }

    if (config->flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_CINVID | VTSS_MEP_SUPP_FLOW_MASK_SINVID)) {
        insert_c_s_tag(&frame[size], (config->flow.mask & VTSS_MEP_SUPP_FLOW_MASK_CINVID) ? 0x8100 : 0x88A8, config->flow.in_vid, prio, dei);
        size+=4;
    }
#endif

    frame[size] = 0x89;
    frame[size+1] = 0x02;
    size+=2;

    memset(&frame[size], 0, 4); /* Clear Common OAM PDU Format */

    return(size);
}

static u32 init_client_tx_frame(u32 instance,   u8 dmac[],   u8 smac[],   u32 prio,   BOOL dei,   u8 frame[])
{
    u32 size=0;
    vtss_mep_supp_conf_t *config = &instance_data[instance].config;
    vtss_mep_supp_flow_t *flow = &config->flow;

    if (frame == instance_data[instance].tx_ais)    /* Check for transmitting AIS */
        flow = &(instance_data[instance].ais_config.flows[instance_data[instance].ais_inx]);
    if (frame == instance_data[instance].tx_lck)    /* Check for transmitting LCK */
        flow = &(instance_data[instance].lck_config.flows[instance_data[instance].lck_inx]);

    memcpy(&frame[size], dmac, VTSS_MEP_SUPP_MAC_LENGTH);
    memcpy(&frame[size+VTSS_MEP_SUPP_MAC_LENGTH], smac, VTSS_MEP_SUPP_MAC_LENGTH);

    size += 2* VTSS_MEP_SUPP_MAC_LENGTH;

#ifdef VTSS_ARCH_SERVAL
    if (flow->mask & VTSS_MEP_SUPP_FLOW_MASK_EVC_ID) { /* The client flow is EVC */
    }
    if (flow->mask & VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID) { /* The client flow is VLAN */
    }
#else
    if (flow->mask & VTSS_MEP_SUPP_FLOW_MASK_COUTVID) {
        insert_c_s_tag(&frame[size], 0x8100, flow->out_vid, prio, dei);
        size+=4;
    }
    else
        if (flow->mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) {
            insert_c_s_tag(&frame[size], 0x88A8, flow->out_vid, prio, dei);
            size+=4;
        }
    if (flow->mask & VTSS_MEP_SUPP_FLOW_MASK_CINVID) {
        insert_c_s_tag(&frame[size], 0x8100, flow->in_vid, prio, dei);
        size+=4;
    }
    else
        if (flow->mask & VTSS_MEP_SUPP_FLOW_MASK_SINVID) {
            insert_c_s_tag(&frame[size], 0x88A8, flow->in_vid, prio, dei);
            size+=4;
        }
#endif
    frame[size] = 0x89;
    frame[size+1] = 0x02;
    size+=2;

    memset(&frame[size], 0, 4); /* Clear Common OAM PDU Format */

    return(size);
}

static u32 rx_tag_cnt_calc(vtss_mep_supp_flow_t *flow)
{
    u32 retval = 0;

    retval += (flow->mask & (VTSS_MEP_SUPP_FLOW_MASK_COUTVID | VTSS_MEP_SUPP_FLOW_MASK_SOUTVID)) ? 1 : 0;
    retval += (flow->mask & (VTSS_MEP_SUPP_FLOW_MASK_CINVID | VTSS_MEP_SUPP_FLOW_MASK_SINVID)) ? 1 : 0;

    return(retval);
}

static void init_pdu_period_to_timer(void)
{
    pdu_period_to_timer[0] = 0;
    pdu_period_to_timer[1] = ((12/timer_res)+1);          /* period 3.33ms -> 12 ms */
    pdu_period_to_timer[2] = ((35/timer_res)+1);          /* period 10ms -> 35 ms */
    pdu_period_to_timer[3] = ((350/timer_res)+1);         /* period 100ms -> 350 ms */
    pdu_period_to_timer[4] = ((3500/timer_res)+1);        /* period 1s. -> 1.000ms -> 35000 ms */
    pdu_period_to_timer[5] = ((35000/timer_res)+1);       /* period 10s. -> 10.000ms -> 35 ms */
    pdu_period_to_timer[6] = ((210000/timer_res)+1);      /* period 1min. -> 60.000ms -> 35 ms */
    pdu_period_to_timer[7] = ((2100000/timer_res)+1);     /* period 10 min. -> 600.000ms -> 35 ms */
}

static u32 tx_timer_calc(vtss_mep_supp_period_t   period)
{
    switch (period)
    {
        case VTSS_MEP_SUPP_PERIOD_INV:   return(0);
        case VTSS_MEP_SUPP_PERIOD_300S:  return(10/timer_res);
        case VTSS_MEP_SUPP_PERIOD_100S:  return(10/timer_res);
        case VTSS_MEP_SUPP_PERIOD_10S:   return(100/timer_res);
        case VTSS_MEP_SUPP_PERIOD_1S:    return(1000/timer_res);
        case VTSS_MEP_SUPP_PERIOD_6M:    return(10*1000/timer_res);
        case VTSS_MEP_SUPP_PERIOD_1M:    return(60*1000/timer_res);
        case VTSS_MEP_SUPP_PERIOD_6H:    return(600*1000/timer_res);
        default:                         return(0);
    }
}

static u8 period_to_pdu_calc(vtss_mep_supp_period_t   period)
{
    switch (period)
    {
        case VTSS_MEP_SUPP_PERIOD_INV:   return(0);
        case VTSS_MEP_SUPP_PERIOD_300S:  return(1);
        case VTSS_MEP_SUPP_PERIOD_100S:  return(2);
        case VTSS_MEP_SUPP_PERIOD_10S:   return(3);
        case VTSS_MEP_SUPP_PERIOD_1S:    return(4);
        case VTSS_MEP_SUPP_PERIOD_6M:    return(5);
        case VTSS_MEP_SUPP_PERIOD_1M:    return(6);
        case VTSS_MEP_SUPP_PERIOD_6H:    return(7);
        default:                         return(4);
    }
}

static u32 frame_rate_calc(vtss_mep_supp_period_t   period)
{
    switch (period)
    {
        case VTSS_MEP_SUPP_PERIOD_INV:   return(0);
        case VTSS_MEP_SUPP_PERIOD_300S:  return(300);
        case VTSS_MEP_SUPP_PERIOD_100S:  return(100);
        case VTSS_MEP_SUPP_PERIOD_10S:   return(10);
        case VTSS_MEP_SUPP_PERIOD_1S:    return(1);
        default:                         return(0);
    }
}

static BOOL hw_ccm_calc(vtss_mep_supp_period_t  period)
{
    return((period == VTSS_MEP_SUPP_PERIOD_300S) || (period == VTSS_MEP_SUPP_PERIOD_100S));
}

static u32 megid_calc(supp_instance_data_t  *data, u8 *buf)
{
    u32  n_len, m_len, inx=0;

    switch (data->ccm_config.format) {
        case VTSS_MEP_SUPP_ITU_ICC:
            buf[0] = 1;
            buf[1] = 32;
            buf[2] = 13;
            memcpy(&buf[3], data->ccm_config.meg, 13);
            inx = 16;
            break;
        case VTSS_MEP_SUPP_ITU_CC_ICC:
            buf[0] = 1;
            buf[1] = 33;
            buf[2] = 15;
            memcpy(&buf[3], data->ccm_config.meg, 15);
            inx = 18;
            break;
        case VTSS_MEP_SUPP_IEEE_STR:
            n_len = strlen(data->ccm_config.name);
            m_len = strlen(data->ccm_config.meg);
            if (n_len) { /* Maintenance Domain Name present */
                buf[inx++] = 4;
                buf[inx++] = n_len;
                memcpy(&buf[inx], data->ccm_config.name, n_len);
                inx += n_len;
            }
            else
                buf[inx++] = 1; /* No Maintenance Domain Name */

            buf[inx++] = 2;    /* Add Short MA name */
            buf[inx++] = m_len;
            memcpy(&buf[inx], data->ccm_config.meg, m_len);
            inx += m_len;
            break;
    }

    return(inx);    /* Return length of MEG-ID */
}

static BOOL get_dm_tunit(supp_instance_data_t *data)
{
    if (data->dmm_config.enable == TRUE)
        return (data->dmm_config.tunit);
    else
        return (data->dm1_config.tunit);
}

static BOOL handle_dmr(supp_instance_data_t *data)
{
    i32 total_delay, dm_delay;
    BOOL  is_ns;

    if (!data->dmm_config.proprietary)
    {
        
        is_ns =  get_dm_tunit(data);       
        total_delay = vtss_mep_supp_delay_calc(&data->rx_dmr_timestamp,
                                               &data->tx_dmm_fup_timestamp,
                                               is_ns);
        if ((total_delay >= 0) && (data->dmr_dev_delay >= 0))
        {
            /* If delay < 0, it means something wrong. Just skip it */
            dm_delay = total_delay - data->dmr_dev_delay;

            if (dm_delay >= 0 && dm_delay < 1e9 /* timeout is 1 second */)
            {   /* If dm_delay <= 0, it means something wrong. Just skip it. */
                data->dmr[data->dmr_rx_cnt % VTSS_MEP_SUPP_DM_MAX] = dm_delay;
                data->dmr_rx_cnt++;
            }
            else
            {
                data->dmr_rx_err_cnt++;
            }
        }
        else
        {
            data->dmr_rx_err_cnt++;
        }      
    }
    else if (data->dmm_config.proprietary && data->dmm_config.calcway == VTSS_MEP_SUPP_FLOW)    
    {
        /* two-way with followup - flow measurement */
        /* Save the rx timestamp for calculation when flowup packet arrives */ 

        return FALSE;
    }    
    
    data->dmm_tx_active = FALSE;
    data->tx_dmm_timeout_timer = 0;
    return TRUE;
}

static void ccm_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_ccm_sw != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_ccm_sw_ongoing = FALSE;

    vtss_mep_supp_crit_unlock();
}

static void lmm_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_lmm != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_lmm_ongoing = FALSE;

    vtss_mep_supp_crit_unlock();
}

static void lmr_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_lmr != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_lmr_ongoing = FALSE;

    vtss_mep_supp_crit_unlock();
}

static void ltm_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_ltm != NULL)    vtss_mep_supp_packet_tx_free(instance_data[instance].tx_ltm);
    instance_data[instance].tx_ltm = NULL;

    vtss_mep_supp_crit_unlock();
}

static void ltr_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_ltr != NULL)    vtss_mep_supp_packet_tx_free(instance_data[instance].tx_ltr);
    instance_data[instance].tx_ltr = NULL;

    vtss_mep_supp_crit_unlock();
}

static void lbm_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_lbm != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_lbm_ongoing = FALSE;

    if (instance_data[instance].lbm_config.enable)
    {
        if (instance_data[instance].lbm_count)
        {/* Still frames to be send */
            if (!instance_data[instance].tx_lbm_timer)
            {/* TX gap timer is not running - Eihter no gap or TX gap timer run out before LBM TX done - simulate timeout of LBM TX gap timer */
                instance_data[instance].event_flags |= EVENT_IN_TX_LBM_TIMER;
                vtss_mep_supp_run();
            }
            goto unlock;
        }
    }

    unlock:
    vtss_mep_supp_crit_unlock();
}

static void lbr_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_lbr != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_lbr_ongoing = FALSE;

    vtss_mep_supp_crit_unlock();
}


static void aps_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_aps != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_aps_ongoing = FALSE;

    if (instance_data[instance].aps_config.enable)
    {
        if (instance_data[instance].aps_count)
        {
            /* Still APS frames to be send */
            instance_data[instance].aps_count--;
            instance_data[instance].event_flags |= EVENT_IN_TX_APS_TIMER;       /* Simulate run out of TX APS timer */
            vtss_mep_supp_run();
        }
        else
            if (instance_data[instance].aps_event)
            { /* End of ERPS event transmission */
                instance_data[instance].aps_event = FALSE;
                instance_data[instance].tx_aps[instance_data[instance].tx_header_size + 4] = instance_data[instance].aps_txdata[0];     /* Transmission of an 'Normal' ERPS request */
            }
    }

    vtss_mep_supp_crit_unlock();
}

static void ais_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_ais != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_ais_ongoing = FALSE;

    if (instance_data[instance].ais_count < 3)
    {
        /* Still AIS frames to be send fast as possible */
        instance_data[instance].event_flags |= EVENT_IN_TX_AIS_TIMER;       /* Simulate run out of TX AIS timer */
        vtss_mep_supp_run();
    }

    vtss_mep_supp_crit_unlock();
}

static void lck_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_lck != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_lck_ongoing = FALSE;

    if (instance_data[instance].lck_count < 1)
    {
        /* Still LCK frames to be send fast as possible */
        instance_data[instance].event_flags |= EVENT_IN_TX_LCK_TIMER;       /* Simulate run out of TX LCK timer */
        vtss_mep_supp_run();
    }

    vtss_mep_supp_crit_unlock();
}

static void tst_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    /* End of transmitting TST frames - this is the exact transmitted frame counter */
    instance_data[instance].tst_tx_cnt = frame_count;

    vtss_mep_supp_packet_tx_free(frame);

    /* The VOE is configured here as it must happen after transmission has stopped. This is to assure that the TST TX counter is sampled correctly . Only for Serval1 */
    voe_config(&instance_data[instance]);

    vtss_mep_supp_crit_unlock();
}

static void dmm_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    BOOL  new_dmr=FALSE;
    
    vtss_mep_supp_crit_lock();
    
    if (instance_data[instance].tx_dmm != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_dmm_ongoing = FALSE;

#if !DEBUG_PHY_TS
    if (vtss_mep_supp_check_hw_timestamp()) {
        vtss_mep_supp_crit_unlock();
        return;
    }
#endif

    instance_data[instance].tx_dmm_fup_timestamp.seconds      = tx_time.seconds;
    instance_data[instance].tx_dmm_fup_timestamp.nanoseconds  = tx_time.nanoseconds; 
    if (instance_data[instance].dmr_received)
    {
        /* DMR has already arrived. We have enough information to calcualte */
        new_dmr = handle_dmr(&instance_data[instance]);
    }    
    
    vtss_mep_supp_crit_unlock();

    if (new_dmr)
    {
        vtss_mep_supp_new_dmr(instance);  
    }    
}

static void dm1_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();
    
    if (instance_data[instance].tx_dm1 != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_dm1_ongoing = FALSE;

    vtss_mep_supp_crit_unlock();
}

static void dm1_fup_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{ 
    int                             rc;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;
    
    vtss_mep_supp_crit_lock();
    
    /* DM1 is out. Send the follow-up packet now */
        
    instance_data[instance].tx_dm1[instance_data[instance].tx_header_size+1] = OAM_TYPE_1DM_FUP;
    var_to_string(&instance_data[instance].tx_dm1[instance_data[instance].tx_header_size+4], tx_time.seconds);                   
    var_to_string(&instance_data[instance].tx_dm1[instance_data[instance].tx_header_size+8], tx_time.nanoseconds);  
    
    init_frame_info(tx_frame_info)

    instance_data[instance].done_dm1.instance = instance;
    instance_data[instance].done_dm1.cb = dm1_done;
    rc = vtss_mep_supp_packet_tx(instance, &instance_data[instance].done_dm1, instance_data[instance].config.flow.port, (instance_data[instance].tx_header_size + DM1_PDU_LENGTH), instance_data[instance].tx_dm1, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, VTSS_MEP_SUPP_OAM_TYPE_NONE);
    if (!rc)       vtss_mep_supp_trace("dm1_done: packet tx failed", 0, 0, 0, 0);
    if (rc)     instance_data[instance].tx_dm1_ongoing = TRUE;

    instance_data[instance].dm1_fup_stdby = FALSE;
    
    vtss_mep_supp_crit_unlock();
}

static void dmr_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tx_dmr != frame)   /* This buffer is no longer in use - free */
        vtss_mep_supp_packet_tx_free(frame);

    instance_data[instance].tx_dmr_ongoing = FALSE;

    vtss_mep_supp_crit_unlock();
}

#ifndef VTSS_ARCH_SERVAL
static void dmr_fup_done(u32 instance,  u8 *frame,  vtss_timestamp_t tx_time,  u64 frame_count)
{
    int                             rc;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;
    
    vtss_mep_supp_crit_lock();
    
    /* DMR is out. Send the follow-up packet now */
    
    instance_data[instance].tx_dmr[instance_data[instance].tx_header_size+1] = OAM_TYPE_DMR_FUP;
    var_to_string(&instance_data[instance].tx_dmr[instance_data[instance].tx_header_size+20], tx_time.seconds);                   
    var_to_string(&instance_data[instance].tx_dmr[instance_data[instance].tx_header_size+24], tx_time.nanoseconds);  
  
    init_frame_info(tx_frame_info)

    instance_data[instance].done_dmr.instance = instance;
    instance_data[instance].done_dmr.cb = dmr_done;
    rc = vtss_mep_supp_packet_tx_1(instance, &instance_data[instance].done_dmr, instance_data[instance].dmr_fup_port, (instance_data[instance].tx_header_size + DMR_PDU_LENGTH), instance_data[instance].tx_dmr, &tx_frame_info, VTSS_MEP_SUPP_OAM_TYPE_NONE);
    if (!rc)       vtss_mep_supp_trace("fup_done: packet tx failed", 0, 0, 0, 0);
    if (rc)     instance_data[instance].tx_dmr_ongoing = TRUE;

    vtss_mep_supp_crit_unlock();
}
#endif

static void instance_data_clear(u32 instance,  supp_instance_data_t *data)
{
    u32 i;

    memset(data, 0, sizeof(supp_instance_data_t));

#ifdef VTSS_ARCH_SERVAL
    data->voe_idx = VTSS_OAM_VOE_CNT;
#endif
    for (i=0; i<VTSS_MEP_SUPP_PEER_MAX; ++i)
        data->defect_state.dLoc[i] = 0x01;    /* So far we have not seen any valid CCM frame received */

    data->loc_timer_val = pdu_period_to_timer[period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_1S)];   /* Default LOC timer is equal to 1 sec period */

    data->done_ccm.instance = instance;
    data->done_ccm.cb = ccm_done;

    data->done_lmm.instance = instance;
    data->done_lmm.cb = lmm_done;

    data->done_lmr.instance = instance;
    data->done_lmr.cb = lmr_done;

    data->done_dmm.instance = instance;
    data->done_dmm.cb = dmm_done;

    data->done_ltm.instance = instance;
    data->done_ltm.cb = ltm_done;

    data->done_ltr.instance = instance;
    data->done_ltr.cb = ltr_done;

    data->done_lbm.instance = instance;
    data->done_lbm.cb = lbm_done;

    data->done_lbr.instance = instance;
    data->done_lbr.cb = lbr_done;

    data->done_aps.instance = instance;
    data->done_aps.cb = aps_done;

    data->done_ais.instance = instance;
    data->done_ais.cb = ais_done;

    data->done_lck.instance = instance;
    data->done_lck.cb = lck_done;

    data->done_tst.instance = instance;
    data->done_tst.cb = tst_done;

    data->rx_isdx = VTSS_MEP_SUPP_INDX_INVALID;
    data->tx_isdx = VTSS_MEP_SUPP_INDX_INVALID;
}

#ifdef VTSS_ARCH_SERVAL
static vtss_oam_period_t voe_period_calc(vtss_mep_supp_period_t  period)
{
    switch (period) {
        case VTSS_MEP_SUPP_PERIOD_300S: return(VTSS_OAM_PERIOD_3_3_MS);
        case VTSS_MEP_SUPP_PERIOD_100S: return(VTSS_OAM_PERIOD_10_MS);
        case VTSS_MEP_SUPP_PERIOD_10S:  return(VTSS_OAM_PERIOD_100_MS);
        case VTSS_MEP_SUPP_PERIOD_1S:   return(VTSS_OAM_PERIOD_1_SEC);
        default:                        return(VTSS_OAM_PERIOD_INV);
    }
}

static BOOL voe_ccm_enable(supp_instance_data_t *data)
{
    return ((data->config.voe && (data->voe_idx < VTSS_OAM_VOE_CNT) && (data->ccm_config.peer_count == 1) && (voe_period_calc(data->ccm_config.period) != VTSS_OAM_PERIOD_INV)) ? TRUE : FALSE);
}
#else
static BOOL voe_ccm_enable(supp_instance_data_t *_ignored)
{
    return FALSE;
}
#endif

static void voe_config(supp_instance_data_t *data)
{
#ifdef VTSS_ARCH_SERVAL
    u32 voe_idx;
    vtss_oam_voe_conf_t cfg;
    vtss_oam_proc_status_t voe_status;

    if (data->voe_config)   return;
    if (data->voe_idx >= VTSS_OAM_VOE_CNT)   return;

    (void)vtss_oam_voe_conf_get(NULL, data->voe_idx, &cfg);

    cfg.voe_type = (data->config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) ? VTSS_OAM_VOE_SERVICE : VTSS_OAM_VOE_PORT;    /* Calculate if VOE is a Port or a EVC or VLAN VOE */
    memcpy(cfg.unicast_mac.addr, data->config.mac, sizeof(cfg.unicast_mac.addr));
    if (data->config.mode == VTSS_MEP_SUPP_MIP)
        cfg.mep_type = VTSS_OAM_MIP;
    else
    if (data->config.direction == VTSS_MEP_SUPP_DOWN)
        cfg.mep_type = VTSS_OAM_DOWNMEP;
    else
        cfg.mep_type = VTSS_OAM_UPMEP;
    cfg.svc_to_path = FALSE;

    if (data->config.enable && (data->config.direction == VTSS_MEP_SUPP_UP) && voe_down_mep_present(data->config.flow.port, data->config.flow.vid, &voe_idx)) {  /* This EVC Up-MEP enabled - must point to any present VOE Down-MEP */
        cfg.svc_to_path = TRUE;
        cfg.svc_to_path_idx_w = voe_idx;
    }
    if ((data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_PATH_P) && (instance_data[data->config.flow.path_mep_p].voe_idx < VTSS_OAM_VOE_CNT))
        cfg.svc_to_path_idx_p = instance_data[data->config.flow.path_mep_p].voe_idx;

    cfg.loop_isdx_w = data->tx_isdx;
//    cfg.loop_isdx_p = 
    cfg.loop_portidx_p = data->config.flow.port_p;

    cfg.proc.meg_level = data->config.level;
    cfg.proc.dmac_check_type = VTSS_OAM_DMAC_CHECK_BOTH;
    cfg.proc.ccm_check_only = FALSE;
    cfg.proc.copy_next_only = voe_ccm_enable(data);
    cfg.proc.copy_on_ccm_err = TRUE;            /* Hit me once on fail as we want to track failing edge defect values */
    cfg.proc.copy_on_mel_too_low_err = TRUE;    /* Hit me once on low level for same reason */
    cfg.proc.copy_on_ccm_more_than_one_tlv = FALSE;
    cfg.proc.copy_on_dmac_err = FALSE;
    memset(cfg.generic, 0, sizeof(cfg.generic));
    cfg.generic[GENERIC_AIS].enable = TRUE;           /* AIS */
    cfg.generic[GENERIC_AIS].copy_to_cpu = TRUE;
    cfg.generic[GENERIC_AIS].forward = FALSE;
    cfg.generic[GENERIC_AIS].count_as_selected = FALSE;
    cfg.generic[GENERIC_AIS].count_as_data = FALSE;
    cfg.generic[GENERIC_LCK].enable = TRUE;           /* LCK */
    cfg.generic[GENERIC_LCK].copy_to_cpu = TRUE;
    cfg.generic[GENERIC_LCK].forward = FALSE;
    cfg.generic[GENERIC_LCK].count_as_selected = FALSE;
    cfg.generic[GENERIC_LCK].count_as_data = FALSE;
    cfg.generic[GENERIC_LAPS].enable = TRUE;           /* LAPS */
    cfg.generic[GENERIC_LAPS].copy_to_cpu = TRUE;
    cfg.generic[GENERIC_LAPS].forward = FALSE;
    cfg.generic[GENERIC_LAPS].count_as_selected = FALSE;
    cfg.generic[GENERIC_LAPS].count_as_data = TRUE;
    cfg.generic[GENERIC_RAPS].enable = TRUE;           /* RAPS */
    cfg.generic[GENERIC_RAPS].copy_to_cpu = TRUE;
    cfg.generic[GENERIC_RAPS].forward = FALSE;
    cfg.generic[GENERIC_RAPS].count_as_selected = FALSE;
    cfg.generic[GENERIC_RAPS].count_as_data = TRUE;

    cfg.unknown.enable = FALSE;
    cfg.unknown.copy_to_cpu = FALSE;
    cfg.unknown.count_as_selected = FALSE;
    cfg.unknown.count_as_data = FALSE;

    /* CCM */
    cfg.ccm.enable = voe_ccm_enable(data);
    cfg.ccm.copy_to_cpu =  TRUE;
    cfg.ccm.forward = FALSE;
    cfg.ccm.count_as_selected = FALSE;
    cfg.ccm.count_as_data = !(data->ccm_gen.enable && data->ccm_gen.lm_enable);  /* Dual-ended LM is not enabled - count CCM as data */
    cfg.ccm.mepid = data->ccm_config.peer_mep[0];
    memset(cfg.ccm.megid.data, 0, sizeof(cfg.ccm.megid.data));
    (void)megid_calc(data, cfg.ccm.megid.data);
    cfg.ccm.tx_seq_no_auto_upd_op = VTSS_OAM_AUTOSEQ_INCREMENT_AND_UPDATE;
    if (data->ccm_start) {
        data->ccm_start = FALSE;
        cfg.ccm.tx_seq_no = 0;
        cfg.ccm.rx_seq_no = 0;
    }
    cfg.ccm.rx_seq_no_check = TRUE;
    cfg.ccm.rx_priority = data->ccm_config.prio;
    cfg.ccm.rx_period = voe_period_calc(data->ccm_config.period);

    /* CCM LM */
    cfg.ccm_lm.enable = data->ccm_gen.enable && data->ccm_gen.lm_enable;
    cfg.ccm_lm.copy_to_cpu = data->ccm_gen.enable && data->ccm_gen.lm_enable;
    cfg.ccm_lm.forward = FALSE;
    cfg.ccm_lm.count.priority_mask = (cfg.voe_type == VTSS_OAM_VOE_SERVICE) ? 0x1FF : 0x000;
    cfg.ccm_lm.count.yellow = TRUE;
    cfg.ccm_lm.count_as_selected = FALSE;
    cfg.ccm_lm.period = voe_period_calc(data->ccm_gen.lm_period);

    /* LMM LM */
    cfg.single_ended_lm.enable = TRUE;
    cfg.single_ended_lm.copy_lmm_to_cpu = ((cfg.mep_type == VTSS_OAM_UPMEP) && !data->config.out_of_service) ? TRUE : FALSE;
    cfg.single_ended_lm.copy_lmr_to_cpu = TRUE;
    cfg.single_ended_lm.forward_lmm = FALSE;
    cfg.single_ended_lm.forward_lmr = FALSE;
    cfg.single_ended_lm.count.priority_mask = (cfg.voe_type == VTSS_OAM_VOE_SERVICE) ? 0x1FF : 0x000;
    cfg.single_ended_lm.count.yellow = TRUE;
    cfg.single_ended_lm.count_as_selected = FALSE;
    cfg.single_ended_lm.count_as_data = FALSE;

    /* LB */
    cfg.lb.enable = !(data->tst_config.enable_rx || data->tst_config.enable);
    cfg.lb.copy_lbr_to_cpu = (data->lbm_config.enable && (data->lbm_config.to_send != VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE)) ? TRUE : FALSE;
    cfg.lb.copy_lbm_to_cpu = ((cfg.mep_type == VTSS_OAM_UPMEP) && !data->config.out_of_service) ? TRUE : FALSE;
    cfg.lb.forward_lbm = FALSE;
    cfg.lb.forward_lbr = FALSE;
    cfg.lb.count_as_selected = FALSE;
    cfg.lb.count_as_data = FALSE;
    cfg.lb.tx_update_transaction_id = TRUE;
    (void)vtss_oam_proc_status_get(NULL, data->voe_idx, &voe_status);   /* Set TX and RX transaction id to next transmitted */
    cfg.lb.tx_transaction_id = voe_status.tx_next_lbm_transaction_id;
    cfg.lb.rx_transaction_id = voe_status.tx_next_lbm_transaction_id;
    if (data->lbm_start) {
        (void)vtss_oam_voe_counter_clear(NULL, data->voe_idx, VTSS_OAM_CNT_LB);
        data->lbm_start = FALSE;
    }

    /* TST */
    cfg.tst.enable = data->tst_config.enable_rx || data->tst_config.enable;
    cfg.tst.forward = FALSE;
    cfg.tst.count_as_selected = FALSE;
    cfg.tst.count_as_data = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID) ? TRUE : FALSE;
    cfg.tst.tx_seq_no_auto_update = TRUE;
    if (data->tst_start) {
        if (data->tst_config.enable_rx)
            cfg.tst.copy_to_cpu = TRUE;
        data->tst_start = FALSE;
        cfg.tst.tx_seq_no = data->tst_config.transaction_id;
        cfg.tst.rx_seq_no = data->tst_config.transaction_id;
    }

    /* DM */
    cfg.dm.enable_dmm = TRUE;
    cfg.dm.enable_1dm = TRUE;
    cfg.dm.copy_dmm_to_cpu = ((cfg.mep_type == VTSS_OAM_UPMEP) && !data->config.out_of_service) ? TRUE : FALSE;
    cfg.dm.copy_1dm_to_cpu = TRUE;
    cfg.dm.copy_dmr_to_cpu = TRUE;
    cfg.dm.forward_1dm = FALSE;
    cfg.dm.forward_dmm = FALSE;
    cfg.dm.forward_dmr = FALSE;
    cfg.dm.count_as_selected = FALSE;
    cfg.dm.count_as_data = FALSE;

    /* LT */
    cfg.lt.enable = TRUE;
    cfg.lt.copy_ltm_to_cpu = TRUE;
    cfg.lt.copy_ltr_to_cpu = TRUE;
    cfg.lt.forward_ltm = FALSE;
    cfg.lt.forward_ltr = FALSE;
    cfg.lt.count_as_selected = FALSE;
    cfg.lt.count_as_data = FALSE;

    cfg.upmep.discard_rx = ((cfg.mep_type == VTSS_OAM_UPMEP) && data->config.out_of_service) ? TRUE : FALSE;
    cfg.upmep.loopback = cfg.upmep.discard_rx;
    cfg.upmep.port = data->config.port;

    (void)vtss_oam_voe_conf_set(NULL, data->voe_idx, &cfg);

    if (voe_ccm_enable(data))
        (void)vtss_oam_voe_event_enable(NULL, data->voe_idx, VOE_EVENT_MASK, TRUE);

    data->voe_config = TRUE;
#endif
    return;
}

static u32 run_sync_config(u32 instance,   const vtss_mep_supp_conf_t  *const conf)
{
    supp_instance_data_t *data;
#ifdef VTSS_ARCH_SERVAL
    vtss_oam_voe_alloc_cfg_t alloc_cfg;
#endif
    data = &instance_data[instance];    /* Instance data reference */

#ifdef VTSS_ARCH_SERVAL
    if (conf->voe) {   /* VOE based MEP */
        if (data->voe_idx < VTSS_OAM_VOE_CNT)    return(VTSS_MEP_SUPP_RC_OK);    /* VOE is already allocated */

        if (conf->flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) { /* This VOE relates to a EVC or VLAN - Service VOE is required */
            if (vtss_oam_voe_alloc(NULL, VTSS_OAM_VOE_SERVICE, NULL, &data->voe_idx) != VTSS_RC_OK)
                return(VTSS_MEP_SUPP_RC_NO_RESOURCES);
        }
        else {
            /* This requires a VOE port instance */
            alloc_cfg.phys_port = conf->port;
            if (vtss_oam_voe_alloc(NULL, VTSS_OAM_VOE_PORT, &alloc_cfg, &data->voe_idx) != VTSS_RC_OK)
                return(VTSS_MEP_SUPP_RC_NO_RESOURCES);
        }
        voe_to_mep[data->voe_idx] = instance;
    }
#endif
    /* By default must be able to respond on received LMM - transmitting LMR */
    if (data->tx_lmr == NULL)
        if (!(data->tx_lmr = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + LMR_PDU_LENGTH)))
            return(VTSS_MEP_SUPP_RC_NO_RESOURCES);
    
    /* By default must be able to respond on received DMM - transmitting DMR */
    if (data->tx_dmr == NULL)
        if (!(data->tx_dmr = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + DMR_PDU_LENGTH)))
            return(VTSS_MEP_SUPP_RC_NO_RESOURCES);

    return(VTSS_MEP_SUPP_RC_OK);
}

static u8 tx_tag_cnt_calc(supp_instance_data_t  *data)
{
    u32 tag_cnt;

    tag_cnt = (data->tx_header_size - ((2* VTSS_MEP_SUPP_MAC_LENGTH) + 2)) / 4;

    return (tag_cnt);
} 

#ifdef VTSS_ARCH_SERVAL
/* One is added in this calculation in order to avoid MCE id 0 - equal to VTSS_MCE_ID_LAST */
#define MCE_ID_CALC(mep, cos, inject) ((mep*2*VTSS_MEP_SUPP_COS_ID_SIZE) + cos + (inject*VTSS_MEP_SUPP_COS_ID_SIZE) + 1)

static BOOL     mce_idx_used[MCE_ID_CALC(VTSS_MEP_SUPP_CREATED_MAX, 0, 0)];

static u8 level_value_mask_calc(BOOL port[],  u32 vid,  u32 level,  u32 *mask)
{
    u32 i, low_level, retval = 0;
    supp_instance_data_t     *data;

    low_level = 0;

    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; i++) { /* Find lower level on this vid */
        data = &instance_data[i];    /* Instance data reference */

        if (data->config.enable && (data->config.mode == VTSS_MEP_SUPP_MEP) && port[data->config.port] &&
            (data->config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) &&
            (data->config.flow.vid == vid) && (data->config.level < level) && (data->config.level >= low_level))
            low_level = data->config.level+1;
    }

    retval = (0x01 << low_level) - 1;  /* Level value is calculated */
    *mask = (0x01 << (level - low_level)) - 1;  /* "don't care" mask value is calculated */
    *mask <<= low_level;    /* mask is rotated to cover levels */
    *mask = ~*mask;         /* Mask is complimented as "don't care" is '0' */

    return(retval & 0xFF);
}

static BOOL voe_up_mep_present(u32 vid,  u32 *voe_idx)
{
    u32 i;

    for (i=0; i<VTSS_OAM_PATH_SERVICE_VOE_CNT; ++i)
        if ((voe_to_mep[i] < VTSS_MEP_SUPP_CREATED_MAX) && instance_data[voe_to_mep[i]].config.enable &&
            (instance_data[voe_to_mep[i]].config.flow.vid == vid) && (instance_data[voe_to_mep[i]].config.direction == VTSS_MEP_SUPP_UP)) {
            *voe_idx = i;
            return(TRUE);
        }

    *voe_idx = VTSS_OAM_VOE_IDX_NONE;
    return(FALSE);
}

static BOOL voe_down_mep_present(BOOL port[],  u32 vid,  u32 *voe_idx)
{
    u32 i, mep;
    supp_instance_data_t     *data;

    for (i=0; i<VTSS_OAM_PATH_SERVICE_VOE_CNT; ++i) {
        mep = voe_to_mep[i];
        data = &instance_data[mep];    /* Instance data reference */

        if ((mep < VTSS_MEP_SUPP_CREATED_MAX) && data->config.enable && port[data->config.port] &&
            (data->config.flow.vid == vid) && (data->config.direction == VTSS_MEP_SUPP_DOWN)) {
            *voe_idx = i;
            return(TRUE);
        }
    }

    *voe_idx = VTSS_OAM_VOE_IDX_NONE;
    return(FALSE);
}

static BOOL up_mip_present(u32 vid,  u32 *instance)
{
    u32 i;

    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; ++i)
        if (instance_data[i].config.enable && (instance_data[i].config.mode == VTSS_MEP_SUPP_MIP) &&
            (instance_data[i].config.flow.vid == vid) && (instance_data[i].config.direction == VTSS_MEP_SUPP_UP)) {
            *instance = i;
            return(TRUE);
        }

    *instance = 0;
    return(FALSE);
}

static u32 mep_on_highest_level(u32 vid)
{
    u32 i, instance, level;

    level = 0;

    instance = VTSS_MEP_SUPP_CREATED_MAX;
    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; i++) /* Find instance on highest level on this vid */
        if (instance_data[i].config.enable && (instance_data[i].config.mode == VTSS_MEP_SUPP_MEP) && (instance_data[i].config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_EVC_ID) &&
            (instance_data[i].config.flow.vid == vid) && (instance_data[i].config.level >= level)) {
            instance = i;
            level = instance_data[i].config.level;
        }

    return(instance);
}

static void port_on_vlan_id(u32 vid, u8 port[VTSS_PORT_ARRAY_SIZE])
{
    u32 i, j;

    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; i++) {/* Find VLAN MEP instance on this vid */
        if (instance_data[i].config.enable && (instance_data[i].config.mode == VTSS_MEP_SUPP_MEP) && (instance_data[i].config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID) &&
            (instance_data[i].config.flow.vid == vid)) {    /* This is a VLAN MEP on this VID */
            for (j=0; j<VTSS_PORT_ARRAY_SIZE; ++j)
                if (instance_data[i].config.flow.port[j])   /* All flow ports are port on this VLAN */
                    port[j] = TRUE;
            port[instance_data[i].config.port] = TRUE;      /* Residence port is port on this VLAN */
        }
    }
}

static void mce_rule_config(u32 instance)
{
    supp_instance_data_t     *data;
    u32                      i, nni, mce_id, voe_idx, mask;
    vtss_evc_oam_port_conf_t evc_conf;
    vtss_vlan_port_conf_t    vlan_conf;
    vtss_mce_port_info_t     mce_info;
    vtss_mce_t               mce;

    data = &instance_data[instance];    /* Instance data reference */

    /* Delete all MCE for this instance */
    for (i=0; i<VTSS_MEP_SUPP_COS_ID_SIZE; ++i) {
        mce_id = MCE_ID_CALC(instance, i, 1);
        if (mce_idx_used[mce_id])   (void)vtss_mce_del(NULL, mce_id);  /* DELETE the MCE injection rule */
        mce_idx_used[mce_id] = FALSE;

        mce_id = MCE_ID_CALC(instance, i, 0);
        if (mce_idx_used[mce_id])   (void)vtss_mce_del(NULL, mce_id);  /* DELETE the MCE extraction rule */
        mce_idx_used[mce_id] = FALSE;
    }

    for (nni=0; nni<VTSS_PORT_ARRAY_SIZE; ++nni)  if (data->config.flow.port[nni])    break;    /* Find one port that is a NNI - anyone will do */
    if (nni >= VTSS_PORT_ARRAY_SIZE)    return;

    if (data->config.enable && !data->config.voe && (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_EVC_ID)) { /* EVC SW MEP/MIP NOT based on VOE but MCE only */
        if (data->config.mode == VTSS_MEP_SUPP_MEP) {   /* This is a EVC SW MEP */
            if (data->config.direction == VTSS_MEP_SUPP_UP) {   /* This is an EVC SW Up-MEP */
                (void)vtss_mce_init(NULL, &mce);            /************************* Create injection MCE rule *******************************/
    
                vlan_conf.port_type = VTSS_VLAN_PORT_TYPE_UNAWARE;      /* Type of dummy tag is depending on the VLAN port configuration */
                (void)vtss_vlan_port_conf_get(NULL, data->config.port, &vlan_conf);
    
                mce.id = MCE_ID_CALC(instance, 0, 1);
    
                mce.key.port_list[data->config.port] = TRUE;
                mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer "Dummy" TAG */
                mce.key.tag.s_tagged = (vlan_conf.port_type == VTSS_VLAN_PORT_TYPE_C) ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_1;
                mce.key.tag.vid.value = data->config.flow.vid;
                mce.key.tag.vid.mask = 0xFFFF;
                mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
                mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
                mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
                mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
                mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
                mce.key.mel.mask = mask;
                mce.key.injected = VTSS_VCAP_BIT_1;

                /* mce.action.port_list is empty, we are going to hit the Service ES0 on the NNI */
                mce.action.voe_idx = VTSS_OAM_VOE_IDX_NONE;
                if (voe_up_mep_present(data->config.flow.vid, &voe_idx))    /* This MCE should only point to a VOE if an VOE based Up-MEP exist */
                    mce.action.voe_idx = voe_idx;
                else
                if (voe_down_mep_present(data->config.flow.port, data->config.flow.vid, &voe_idx))  /* This MCE should only point to a VOE if an VOE based Down-MEP exist */
                    mce.action.voe_idx = voe_idx;
                mce.action.vid = data->config.flow.vid;   
                mce.action.pop_cnt = 1;
                mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_VID;
                mce.action.oam_detect = VTSS_MCE_OAM_DETECT_NONE;
                mce.action.isdx = VTSS_MCE_ISDX_NEW;
    
                if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE injection rule */
                mce_idx_used[mce.id] = TRUE;
                if (vtss_mce_port_info_get(NULL, mce.id, data->config.port, &mce_info) != VTSS_RC_OK)  return; /* Get the transmit ISDX */
                data->tx_isdx = mce_info.isdx;
            }

            (void)vtss_mce_init(NULL, &mce);            /********************************* Create extraction MCE rule *******************************/

            mce.id = MCE_ID_CALC(instance, 0, 0);

            memcpy (mce.key.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.key.lookup = 1;                         /* Hit in second lookup. The first lookup is hitting the Service entry */
            mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer EVC TAG */
            mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
            mce.key.tag.vid.value = data->config.flow.out_vid;
            mce.key.tag.vid.mask = 0xFFFF;
            mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
            mce.key.mel.mask = mask;
            mce.key.injected = VTSS_VCAP_BIT_0;

            /* mce.action.port_list is empty */
            mce.action.voe_idx = VTSS_OAM_VOE_IDX_NONE;
            mce.action.pop_cnt = 0;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_VID;
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_NONE;
            mce.action.policy_no = VTSS_MEP_SUPP_MEP_PAG;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;

            if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
            mce_idx_used[mce.id] = TRUE;
            if (vtss_mce_port_info_get(NULL, mce.id, nni, &mce_info) != VTSS_RC_OK)  return; /* Get the receive ISDX */
            data->rx_isdx = mce_info.isdx;
        }
        else {   /* This is a SW MIP */
            (void)vtss_mce_init(NULL, &mce);            /********************************* Create extraction MCE rule *******************************/

            mce.id = MCE_ID_CALC(instance, 0, 0);

            memcpy (mce.key.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.key.lookup = 1;                         /* Hit in second lookup. The first lookup is hitting the Service entry */
            mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer EVC TAG */
            mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
            mce.key.tag.vid.value = data->config.flow.out_vid;
            mce.key.tag.vid.mask = 0xFFFF;
            mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.inner_tag.tagged = (data->config.direction == VTSS_MEP_SUPP_UP) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.vid.value = data->config.flow.in_vid;
            mce.key.inner_tag.vid.mask = (mce.key.inner_tag.tagged == VTSS_VCAP_BIT_1) ? 0xFFFF : 0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.mel.value = (0x1 << data->config.level) - 1;
            mce.key.mel.mask = 0xFF;
            mce.key.injected = VTSS_VCAP_BIT_0;

            /* mce.action.port_list is empty */
            mce.action.voe_idx = VTSS_OAM_VOE_IDX_NONE;
            mce.action.pop_cnt = (data->config.direction == VTSS_MEP_SUPP_UP) ? 1 : 0;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_VID;
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_NONE;
            mce.action.policy_no = VTSS_MEP_SUPP_MIP_PAG;
            mce.action.isdx = VTSS_MCE_ISDX_NONE;

            if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
            mce_idx_used[mce.id] = TRUE;
        }
    }

    if (data->config.enable && !(data->config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID  | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID))) { /* Port MEP is enabled  */
        (void)vtss_mce_init(NULL, &mce);            /******************************* Create extraction MCE rule ***************************/

        mce.id = MCE_ID_CALC(instance, 0, 0);

        mce.key.port_list[data->config.port] = TRUE;
        mce.key.tag.tagged = (data->config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_COUTVID | VTSS_MEP_SUPP_FLOW_MASK_SOUTVID)) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
        mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
        mce.key.tag.vid.value = data->config.flow.out_vid;
        mce.key.tag.vid.mask = (mce.key.tag.tagged == VTSS_VCAP_BIT_1) ? 0xFFFF : 0;
        mce.key.tag.pcp.mask = 0x00;
        mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
        mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
        mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
        mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
        mce.key.mel.mask = 0x00;
        mce.key.injected = VTSS_VCAP_BIT_0;

        /* mce.action.port_list is empty */
        mce.action.voe_idx = VTSS_OAM_VOE_IDX_NONE;
        mce.action.vid = data->config.flow.vid;
        mce.action.pop_cnt = 1;
        mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_VID;
        mce.action.oam_detect = (data->config.voe) ? ((mce.key.tag.tagged == VTSS_VCAP_BIT_1) ? VTSS_MCE_OAM_DETECT_SINGLE_TAGGED : VTSS_MCE_OAM_DETECT_UNTAGGED) : VTSS_MCE_OAM_DETECT_NONE;
        if (!data->config.voe)
            mce.action.policy_no = VTSS_MEP_SUPP_MEP_PAG;
//        mce.action.prio_enable = TRUE;
//        mce.action.prio = 7;
        mce.action.isdx = (data->config.voe) ? VTSS_MCE_ISDX_NEW : VTSS_MCE_ISDX_NONE;

        if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
        mce_idx_used[mce.id] = TRUE;
        if (data->config.voe) {
            if (vtss_mce_port_info_get(NULL, mce.id, data->config.port, &mce_info) != VTSS_RC_OK)  return; /* Get the receive ISDX */
            data->rx_isdx = mce_info.isdx;
        }
    }

    if (data->config.enable && data->config.voe && (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_EVC_ID)) { /* EVC MEP is enabled and is VOE based */
        evc_conf.voe_idx = data->voe_idx;

        /* Create the MCE entries required to handle Service OAM */
        if (data->config.direction == VTSS_MEP_SUPP_UP) {   /* This MEP is an VOE Up-MEP */
            (void)vtss_evc_oam_port_conf_set(NULL, data->config.flow.evc_id, data->config.port, &evc_conf);  /* Hook EVC to VOE - Give VOE index to EVC for performance counting */
            for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i) {    /* All possible NNI ports must be related to VOE */
                if (data->config.flow.port[i])
                    (void)vtss_evc_oam_port_conf_set(NULL, data->config.flow.evc_id, i, &evc_conf);                
            }

            (void)vtss_mce_init(NULL, &mce);            /************************* Create injection MCE rule *******************************/

            vlan_conf.port_type = VTSS_VLAN_PORT_TYPE_UNAWARE;      /* Type of dummy tag is depending on the VLAN port configuration */
            (void)vtss_vlan_port_conf_get(NULL, data->config.port, &vlan_conf);

            mce.id = MCE_ID_CALC(instance, 0, 1);

            mce.key.port_list[data->config.port] = TRUE;
            mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer "Dummy" TAG */
            mce.key.tag.s_tagged = (vlan_conf.port_type == VTSS_VLAN_PORT_TYPE_C) ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_1;
            mce.key.tag.vid.value = data->config.flow.vid;
            mce.key.tag.vid.mask = 0xFFFF;
            mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
            mce.key.mel.mask = mask;
            mce.key.injected = VTSS_VCAP_BIT_1;

            /* mce.action.port_list is empty */
            mce.action.voe_idx = data->voe_idx;
            mce.action.vid = data->config.flow.vid;   
            mce.action.pop_cnt = 1;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_VID;
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_SINGLE_TAGGED;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;

            if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE injection rule */
            mce_idx_used[mce.id] = TRUE;
            if (vtss_mce_port_info_get(NULL, mce.id, data->config.port, &mce_info) != VTSS_RC_OK)  return; /* Get the transmit ISDX */
            data->tx_isdx = mce_info.isdx;

            (void)vtss_mce_init(NULL, &mce);            /********************************* Create extraction MCE rule *******************************/

            mce.id = MCE_ID_CALC(instance, 0, 0);

            memcpy (mce.key.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.key.lookup = 1;                         /* Hit in second lookup. The first lookup is hitting the Service entry */
            mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer EVC TAG */
            mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
            mce.key.tag.vid.value = data->config.flow.out_vid;
            mce.key.tag.vid.mask = 0xFFFF;
            mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
            mce.key.mel.mask = mask;
            mce.key.injected = VTSS_VCAP_BIT_0;

            mce.action.port_list[data->config.port] = TRUE;
            mce.action.voe_idx = data->voe_idx;
            mce.action.pop_cnt = 0;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_ISDX;
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_SINGLE_TAGGED;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;

            if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
            mce_idx_used[mce.id] = TRUE;
            if (vtss_mce_port_info_get(NULL, mce.id, nni, &mce_info) != VTSS_RC_OK)  return; /* Get the receive ISDX */
            data->rx_isdx = mce_info.isdx;
        }
#if defined(VTSS_SW_OPTION_EVC)
        else { /* Down MEP */
            vtss_mep_supp_evc_mce_outer_tag_t nni_outer_tag[VTSS_MEP_SUPP_COS_ID_SIZE];
            vtss_mep_supp_evc_mce_key_t       nni_key[VTSS_MEP_SUPP_COS_ID_SIZE];
            vtss_mep_supp_evc_nni_type_t      nni_type;
            BOOL                              first;

            vtss_mep_supp_evc_mce_info_get(data->config.flow.evc_id, &nni_type, nni_outer_tag, nni_key);

            if (!voe_up_mep_present(data->config.flow.vid, &voe_idx)) {     /* Only hook EVC to this VOE if VOE based Up-MEP do not exist */
                for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i) {    /* All possible NNI ports must be related to VOE */
                    if (data->config.flow.port[i])
                        (void)vtss_evc_oam_port_conf_set(NULL, data->config.flow.evc_id, i, &evc_conf);                
                }
            }

            (void)vtss_mce_init(NULL, &mce);            /******************************** Create injection MCE rule ***************************/

            mce.key.port_cpu = TRUE;
            mce.key.tag.tagged = VTSS_VCAP_BIT_0;       /* NO Outer TAG */
            mce.key.tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.tag.dei = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.mask = 0xFF;
            mce.key.injected = VTSS_VCAP_BIT_0;

            memcpy (mce.action.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.action.voe_idx = data->voe_idx;
            mce.action.pop_cnt = 0;
            mce.action.outer_tag.enable = TRUE;
            mce.action.outer_tag.vid = data->config.flow.out_vid;
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_UNTAGGED;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;

            if (nni_type == VTSS_MEP_SUPP_EVC_NNI_TYPE_E_NNI) { /* E-NNI - MCE must be created for each COS-ID */
                mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_ISDX_PCP;
                for (i=0, first=TRUE; i<VTSS_MEP_SUPP_COS_ID_SIZE; ++i) {
                    if (nni_outer_tag[i].enable) {  /* This COS-ID (prio) is active */
                        mce.id = MCE_ID_CALC(instance, i, 1);
                        mce.action.prio_enable = TRUE;
                        mce.action.prio = i;
                        mce.action.outer_tag.pcp_mode = nni_outer_tag[i].pcp_mode;
                        mce.action.outer_tag.pcp = nni_outer_tag[i].pcp;
                        mce.action.outer_tag.dei_mode = nni_outer_tag[i].dei_mode;
                        mce.action.outer_tag.dei = nni_outer_tag[i].dei;
                        if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE injection rule */
                        mce_idx_used[mce.id] = TRUE;
                        if (first) {   /* Only the first MCE is created to assign ISDX */
                            first = FALSE;
                            if (vtss_mce_port_info_get(NULL, mce.id, VTSS_PORT_NO_CPU, &mce_info) != VTSS_RC_OK)  return; /* Get the transmit ISDX */
                            data->tx_isdx = mce_info.isdx;
                            mce.action.isdx = mce_info.isdx;    /* Now all following ECE is created with same ISDX */
                        }
                    }
                }
            }
            else {  /* I-NNI - Only one MCE is created to used port PCP mapping */
                mce.id = MCE_ID_CALC(instance, 0, 1);
                mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_ISDX;
                mce.action.outer_tag.pcp_mode = VTSS_MCE_PCP_MODE_MAPPED;
                mce.action.outer_tag.dei_mode = VTSS_MCE_DEI_MODE_DP;
                if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE injection rule */
                mce_idx_used[mce.id] = TRUE;
                if (vtss_mce_port_info_get(NULL, mce.id, VTSS_PORT_NO_CPU, &mce_info) != VTSS_RC_OK)  return; /* Get the transmit ISDX */
                data->tx_isdx = mce_info.isdx;
            }

            (void)vtss_mce_init(NULL, &mce);            /******************************* Create extraction MCE rule ***************************/

            memcpy (mce.key.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer EVC TAG */
            mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
            mce.key.tag.vid.value = data->config.flow.out_vid;
            mce.key.tag.vid.mask = 0xFFFF;
            mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
            mce.key.mel.mask = mask;
            mce.key.injected = VTSS_VCAP_BIT_0;

            /* mce.action.port_list is empty */
            mce.action.voe_idx = data->voe_idx;
            mce.action.vid = data->config.flow.vid;   
            mce.action.pop_cnt = 1;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_ISDX;     /* This is to enable VOE loop frames (LMR + DMR + ..) to hit ES0 */
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_SINGLE_TAGGED;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;

            if (nni_type == VTSS_MEP_SUPP_EVC_NNI_TYPE_E_NNI) { /* E-NNI - MCE must be created for each COS-ID */
                mce.key.tag.pcp.mask = 0xFF;
                mce.action.prio_enable = TRUE;
                for (i=0, first=TRUE; i<VTSS_MEP_SUPP_COS_ID_SIZE; ++i) {
                    if (nni_key[i].enable) {      /* This COS-ID (prio) is active */
                        mce.id = MCE_ID_CALC(instance, i, 0);
                        mce.key.tag.pcp.value = nni_key[i].pcp;
                        mce.action.prio = i;
                        if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
                        mce_idx_used[mce.id] = TRUE;
                        if (first) {   /* Only the first MCE is created to assign ISDX */
                            first = FALSE;
                            if (vtss_mce_port_info_get(NULL, mce.id, nni, &mce_info) != VTSS_RC_OK)  return; /* Get the receive ISDX */
                            data->rx_isdx = mce_info.isdx;
                            mce.action.isdx = mce_info.isdx;    /* Now all following ECE is created with same ISDX */
                        }
                    }
                }
            }
            else {  /* I-NNI - Only one MCE is created to used port PCP mapping */
                mce.id = MCE_ID_CALC(instance, 0, 0);
                mce.key.tag.pcp.mask = 0x00;
                mce.action.prio_enable = FALSE;
                if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
                mce_idx_used[mce.id] = TRUE;
                if (vtss_mce_port_info_get(NULL, mce.id, nni, &mce_info) != VTSS_RC_OK)  return; /* Get the receive ISDX */
                data->rx_isdx = mce_info.isdx;
            }
        }
#endif /* VTSS_SW_OPTION_EVC */
    }

    if (data->config.enable && data->config.voe && (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) { /* VLAN MEP is enabled and is VOE based */
        evc_conf.voe_idx = data->voe_idx;

        /* Create the MCE entries required to handle Service OAM */
        if (data->config.direction == VTSS_MEP_SUPP_UP) {   /* This VLAN MEP is an VOE Up-MEP */
            (void)vtss_mce_init(NULL, &mce);            /************************* Create injection MCE rule *******************************/

            vlan_conf.port_type = VTSS_VLAN_PORT_TYPE_UNAWARE;      /* Type of dummy tag is depending on the VLAN port configuration */
            (void)vtss_vlan_port_conf_get(NULL, data->config.port, &vlan_conf);

            mce.id = MCE_ID_CALC(instance, 0, 1);

            mce.key.port_list[data->config.port] = TRUE;
            mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer "Dummy" TAG */
            mce.key.tag.s_tagged = (vlan_conf.port_type == VTSS_VLAN_PORT_TYPE_C) ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_1;
            mce.key.tag.vid.value = data->config.flow.vid;
            mce.key.tag.vid.mask = 0xFFFF;
            mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
            mce.key.mel.mask = mask;
            mce.key.injected = VTSS_VCAP_BIT_1;

            memcpy (mce.action.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.action.voe_idx = data->voe_idx;
            mce.action.vid = data->config.flow.vid;   
            mce.action.pop_cnt = 1;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_VID;
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_SINGLE_TAGGED;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;

            if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE injection rule */
            mce_idx_used[mce.id] = TRUE;
            if (vtss_mce_port_info_get(NULL, mce.id, data->config.port, &mce_info) != VTSS_RC_OK)  return; /* Get the transmit ISDX */
            data->tx_isdx = mce_info.isdx;

            (void)vtss_mce_init(NULL, &mce);            /********************************* Create extraction MCE rule *******************************/

            mce.id = MCE_ID_CALC(instance, 0, 0);

            memcpy (mce.key.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.key.lookup = 0;                         /* Hit in first lookup. */
            mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* VLAN TAG */
            mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
            mce.key.tag.vid.value = data->config.flow.vid;
            mce.key.tag.vid.mask = 0xFFFF;
            mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
            mce.key.mel.mask = mask;
            mce.key.injected = VTSS_VCAP_BIT_0;

            mce.action.port_list[data->config.port] = TRUE;
            mce.action.voe_idx = data->voe_idx;
            mce.action.pop_cnt = 1;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_ISDX;
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_SINGLE_TAGGED;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;

            if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
            mce_idx_used[mce.id] = TRUE;
            if (vtss_mce_port_info_get(NULL, mce.id, nni, &mce_info) != VTSS_RC_OK)  return; /* Get the receive ISDX */
            data->rx_isdx = mce_info.isdx;
        }
        else { /* VLAN MEP is VOE Down MEP */
            (void)vtss_mce_init(NULL, &mce);            /******************************** Create injection MCE rule ***************************/

            mce.id = MCE_ID_CALC(instance, 0, 1);

            mce.key.port_cpu = TRUE;
            mce.key.tag.tagged = VTSS_VCAP_BIT_0;       /* NO Outer TAG */
            mce.key.tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.tag.dei = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.mask = 0xFF;
            mce.key.injected = VTSS_VCAP_BIT_0;

            memcpy (mce.action.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.action.voe_idx = data->voe_idx;
            mce.action.pop_cnt = 0;
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_UNTAGGED;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_ISDX;
            if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE injection rule */
            mce_idx_used[mce.id] = TRUE;
            if (vtss_mce_port_info_get(NULL, mce.id, VTSS_PORT_NO_CPU, &mce_info) != VTSS_RC_OK)  return; /* Get the transmit ISDX */
            data->tx_isdx = mce_info.isdx;

            (void)vtss_mce_init(NULL, &mce);            /******************************* Create extraction MCE rule ***************************/

            mce.id = MCE_ID_CALC(instance, 0, 0);

            memcpy (mce.key.port_list, data->config.flow.port, sizeof(mce.key.port_list));
            mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer EVC TAG */
            mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
            mce.key.tag.vid.value = data->config.flow.out_vid;
            mce.key.tag.vid.mask = 0xFFFF;
            mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
            mce.key.tag.pcp.mask = 0x00;
            mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
            mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
            mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
            mce.key.mel.mask = mask;
            mce.key.injected = VTSS_VCAP_BIT_0;

            /* mce.action.port_list is empty */
            port_on_vlan_id(data->config.flow.vid, mce.action.port_list);   /* THis is only to enable forwarding of any RAPS frame to other ring port */
            mce.action.voe_idx = data->voe_idx;
            mce.action.vid = data->config.flow.vid;   
            mce.action.pop_cnt = 1;
            mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_ISDX;     /* This is to enable VOE loop frames (LMR + DMR + ..) to hit ES0 */
            mce.action.rule = VTSS_MCE_RULE_RX;     /* Only RX rules - no ES0 */
            mce.action.oam_detect = VTSS_MCE_OAM_DETECT_SINGLE_TAGGED;
            mce.action.isdx = VTSS_MCE_ISDX_NEW;
            mce.action.prio_enable = FALSE;
            if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
            mce_idx_used[mce.id] = TRUE;
            if (vtss_mce_port_info_get(NULL, mce.id, nni, &mce_info) != VTSS_RC_OK)  return; /* Get the receive ISDX */
            data->rx_isdx = mce_info.isdx;
        }
    }

    if (data->config.enable && !data->config.voe && (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) { /* VLAN SW MEP NOT based on VOE but MCE only */
        (void)vtss_mce_init(NULL, &mce);            /********************************* Create extraction MCE rule *******************************/

        mce.id = MCE_ID_CALC(instance, 0, 0);

        memcpy (mce.key.port_list, data->config.flow.port, sizeof(mce.key.port_list));
        mce.key.lookup = 0;                         /* Hit in first lookup. */
        mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer VLAN TAG */
        mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
        mce.key.tag.vid.value = data->config.flow.out_vid;
        mce.key.tag.vid.mask = 0xFFFF;
        mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
        mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
        mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
        mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
        mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, data->config.level, &mask);
        mce.key.mel.mask = mask;
        mce.key.injected = VTSS_VCAP_BIT_0;

        /* mce.action.port_list is empty */
        mce.action.voe_idx = VTSS_OAM_VOE_IDX_NONE;
        mce.action.pop_cnt = 1;
        mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_VID;
        mce.action.oam_detect = VTSS_MCE_OAM_DETECT_NONE;
        mce.action.policy_no = VTSS_MEP_SUPP_MEP_PAG;
        mce.action.isdx = VTSS_MCE_ISDX_NONE;

        if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
        mce_idx_used[mce.id] = TRUE;
    }

    if (data->config.enable && (data->config.mode == VTSS_MEP_SUPP_MEP) && (data->config.level != 7) && (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_EVC_ID) && (instance == mep_on_highest_level(data->config.flow.vid))) { /* EVC MEP is enabled and this is highest level */
        (void)vtss_mce_init(NULL, &mce);            /********************************* Create termination MCE rule *******************************/
    
        mce.id = MCE_ID_CALC(instance, 0, 1);
    
        memcpy (mce.key.port_list, data->config.flow.port, sizeof(mce.key.port_list));
        mce.key.lookup = 1;                         /* Hit in second lookup. The first lookup is hitting the Service entry */
        mce.key.tag.tagged = VTSS_VCAP_BIT_1;       /* Outer EVC TAG */
        mce.key.tag.s_tagged = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_SOUTVID) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0;
        mce.key.tag.vid.value = data->config.flow.out_vid;
        mce.key.tag.vid.mask = 0xFFFF;
        mce.key.tag.dei = VTSS_VCAP_BIT_ANY;
        mce.key.inner_tag.tagged = VTSS_VCAP_BIT_0; /* NO Inner TAG */
        mce.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
        mce.key.inner_tag.dei = VTSS_VCAP_BIT_0;
        mce.key.mel.value = level_value_mask_calc(data->config.flow.port, data->config.flow.vid, 7, &mask);
        mce.key.mel.mask = mask;
        mce.key.injected = VTSS_VCAP_BIT_0;
    
        /* mce.action.port_list is empty */
        mce.action.voe_idx = VTSS_OAM_VOE_IDX_NONE;
        mce.action.pop_cnt = 0;
        mce.action.tx_lookup = VTSS_MCE_TX_LOOKUP_VID;
        mce.action.oam_detect = VTSS_MCE_OAM_DETECT_NONE;
        mce.action.policy_no = VTSS_MEP_SUPP_MEP_PAG;
        mce.action.isdx = VTSS_MCE_ISDX_NONE;

// If this is going to work a macro must be defined to calculate MCE id for this - this should be done in IS2 saving IS1 entry
//        if (vtss_mce_add(NULL, VTSS_MCE_ID_LAST, &mce) != VTSS_RC_OK)  return; /* ADD the MCE extraction rule */
//        mce_idx_used[mce.id] = TRUE;
    }

    if ((!data->config.enable || !data->config.voe) && (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_EVC_ID) && (data->voe_idx < VTSS_OAM_VOE_CNT)) { /* EVC MEP is disabled or no longer VOE based, with a VOE reference */
        (void)vtss_evc_oam_port_conf_get(NULL, data->config.flow.evc_id, data->config.port, &evc_conf);
        if (evc_conf.voe_idx == data->voe_idx) {    /* The EVC is hooked to this VOE */
            evc_conf.voe_idx = VTSS_OAM_VOE_IDX_NONE;
            (void)vtss_evc_oam_port_conf_set(NULL, data->config.flow.evc_id, data->config.port, &evc_conf);  /* Un-Hook EVC residence port to VOE */

            for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i) {    /* All possible NNI ports must be Un-Hooked to VOE */
                if ((i != data->config.port) && data->config.flow.port[i])
                    (void)vtss_evc_oam_port_conf_set(NULL, data->config.flow.evc_id, i, &evc_conf);                
            }
        }
    }
}
#endif

static void run_async_config(u32 instance)
{
    supp_instance_data_t  *data;
    u8                    dummy_mac[VTSS_MEP_SUPP_MAC_LENGTH] = {0,0,0,0,0,0};

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_CONFIG;

#ifdef VTSS_ARCH_SERVAL
    u32                   i;
    BOOL                  out_of_service, up_mep_found;

    if ((!data->config.enable || !data->config.voe) && (data->voe_idx < VTSS_OAM_VOE_CNT))    /* This MEP is either disabled or is no longer based on VOE */
        voe_to_mep[data->voe_idx] = VTSS_MEP_SUPP_CREATED_MAX;

    out_of_service = FALSE;
    up_mep_found = FALSE;

    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; i++) {  /* Search for any UP-MEP/MIP */
        if (instance_data[i].config.enable && (instance_data[i].config.direction == VTSS_MEP_SUPP_UP) && instance_data[i].config.voe && (instance_data[i].config.port == data->config.port)) {    /* Enabled VOE based Up-MEP found on this port */
            up_mep_found = TRUE;
            if (instance_data[i].config.out_of_service)
                out_of_service = TRUE;              /* Register an VOE UP-MEP is out of service on this UNI */
        }
    }

    vtss_mep_supp_port_conf(instance, data->config.port, up_mep_found, out_of_service);  /* UNI port must be configured depending on Up-MEP and 'out of service' MEP was found */

    mce_rule_config(instance);    /* Now configure the related OAM IS1 tcam entries */
    voe_config(data);             /* Now configure the VOE */

    if ((!data->config.enable || !data->config.voe) && (data->voe_idx < VTSS_OAM_VOE_CNT)) {    /* This MEP is either disabled or is no longer based on VOE */
        (void)vtss_oam_voe_free(NULL, data->voe_idx);   /* Free the VOE */
        data->voe_idx = VTSS_OAM_VOE_CNT;
    }
#endif

    if (data->config.enable)
    {
        /* Init of LMR transmitter frame */
        data->tx_header_size = init_tx_frame(instance, dummy_mac, data->config.mac, 0, 0, data->tx_lmr, FALSE);
        data->rx_tag_cnt = rx_tag_cnt_calc(&data->config.flow);

        /* Init of DMR transmitter frame */
        (void)init_tx_frame(instance, dummy_mac, data->config.mac, 0, 0, data->tx_dmr, FALSE);

        /* Init of DMR tag count for replying DMM correctly even 2-way DM is not enabled */
        data->onestep_extra_m.tag_cnt = tx_tag_cnt_calc(data);
        data->onestep_extra_r.tag_cnt = tx_tag_cnt_calc(data);

        run_async_ccm_gen(instance);
        run_async_lmm_config(instance);
        run_async_aps_config(instance);
        run_tst_config(instance);
        run_lbm_config(instance);

        data->event_flags |= EVENT_OUT_NEW_DEFECT;     /* Deliver CCM state to upper logic */
    }
    else
    {   /* Disable of this MEP  */
        if ((data->tx_lmr != NULL) && !data->tx_lmr_ongoing)       vtss_mep_supp_packet_tx_free(data->tx_lmr);
        if ((data->tx_dmr != NULL) && !data->tx_dmr_ongoing)       vtss_mep_supp_packet_tx_free(data->tx_dmr);

        data->ccm_gen.enable = FALSE;
        data->lmm_config.enable = FALSE;
        data->dmm_config.enable = FALSE;
        data->dm1_config.enable = FALSE;
        data->aps_config.enable = FALSE;
        data->ltm_config.enable = FALSE;
        data->lbm_config.enable = FALSE;
        data->tst_config.enable = FALSE;
        data->lck_config.enable = FALSE;
        data->ais_enable = FALSE;

        run_async_ccm_gen(instance);
        run_async_lmm_config(instance);
        run_async_aps_config(instance);
        run_dmm_config(instance);
        run_1dm_config(instance);
        run_ltm_config(instance);
        run_lbm_config(instance);
        run_lck_config(instance);
        run_tst_config(instance);
        run_ais_set(instance);

        instance_data_clear(instance,  data);
    }
}

static void run_ccm_config(u32 instance)
{
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_CCM_CONFIG;

    voe_config(data);

    /* LOC timer is based on expected period */
    if (voe_ccm_enable(data)) { /* This is a VOE based CCM - hit me once poll of PDU */
        if (data->ccm_config.period > VTSS_MEP_SUPP_PERIOD_1S)                      /* In this case CCM PDU will be polled with 100ms interval, LOC timer must be min. 100ms */
            data->loc_timer_val = pdu_period_to_timer[period_to_pdu_calc(data->ccm_config.period)];
        else
            data->loc_timer_val = pdu_period_to_timer[period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_1S)];
    }
    else {  /* Either this is not VOE based MEP or there are more peers. In this case either all CCM come to CPU or they are polled with 1s interval */
        if (hw_ccm_calc(data->ccm_config.period))
            data->loc_timer_val = pdu_period_to_timer[period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_1S)];
        else
            data->loc_timer_val = pdu_period_to_timer[period_to_pdu_calc(data->ccm_config.period)];
    }

    run_async_ccm_gen(instance);
}

static u32 run_sync_ccm_sw_gen(u32 instance)
{
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    if (data->tx_ccm_sw == NULL)
        if (!(data->tx_ccm_sw = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + CCM_PDU_LENGTH)))
            return(VTSS_MEP_SUPP_RC_NO_RESOURCES);

    return(VTSS_MEP_SUPP_RC_OK);
}

static u32 run_sync_ccm_hw_gen(u32 instance)
{
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    if (data->tx_ccm_hw == NULL)
    {
        if (!(data->tx_ccm_hw = vtss_mep_supp_packet_tx_alloc((evc_vlan_up(data) ? VTSS_PACKET_HDR_SIZE_BYTES : 0) + HEADER_MAX_SIZE + CCM_PDU_LENGTH)))
            return(VTSS_MEP_SUPP_RC_NO_RESOURCES);
        data->hw_ccm_transmitting = FALSE;
    }

    return(VTSS_MEP_SUPP_RC_OK);
}

static void ccm_pdu_init(supp_instance_data_t  *data, u8 *pdu)
{
    memset(pdu, 0, CCM_PDU_LENGTH);

    pdu[0] = data->config.level << 5;
    pdu[1] = OAM_TYPE_CCM;
    pdu[2] = period_to_pdu_calc(data->ccm_config.period);
    pdu[2] |= ((data->ccm_rdi) ? 0x80 : 0x00);
    pdu[3] = 70;
    pdu[8] = (data->ccm_config.mep & 0xFF00) >> 8;
    pdu[9] = data->ccm_config.mep & 0xFF;

    data->exp_meg_len = megid_calc(data, &pdu[10]);         /* Calculate and save the expected MEG-ID */
    memcpy(data->exp_meg, &pdu[10], data->exp_meg_len);
}

static void run_async_ccm_gen(u32 instance)
{
    static u32 first_timer = 2;
    supp_instance_data_t            *data;
    u8                              *packet;
    u32                             i, header_size;
    BOOL                            hw_port[VTSS_PORT_ARRAY_SIZE];
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;
    vtss_mep_supp_period_t          sw_ccm_period;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_CCM_GEN;

    voe_config(data);

    if (data->ccm_gen.enable) {  /* CC or LM is enabled */
        if (hw_ccm_calc(data->ccm_gen.cc_period)) { /* HW CCM generation */
#if defined(VTSS_ARCH_SERVAL) || defined(VTSS_ARCH_JAGUAR_1)
            if (!data->ccm_gen.lm_enable && (data->tx_ccm_sw != NULL))    {vtss_mep_supp_packet_tx_free(data->tx_ccm_sw); data->tx_ccm_sw = NULL;}  /* On Jaguar and Serval without LM - No SW based CCM-LM */
#endif
#if defined(VTSS_ARCH_SERVAL)
            if (data->config.voe && (data->config.direction == VTSS_MEP_SUPP_DOWN) && (data->tx_ccm_sw != NULL))    {vtss_mep_supp_packet_tx_free(data->tx_ccm_sw); data->tx_ccm_sw = NULL;}  /* On Serval VOE down mep - No SW based CCM-LM */
#endif
#if defined(VTSS_ARCH_LUTON26)
            if (data->tx_ccm_sw != NULL)    {vtss_mep_supp_packet_tx_free(data->tx_ccm_sw); data->tx_ccm_sw = NULL;}  /* On Luton26 SW CCM-LM is not possible */
#endif
            if (data->hw_ccm_transmitting)
            {   /* HW CCM transmitting is on-going. This must be stopped before starting the new one */
                if (!(packet = vtss_mep_supp_packet_tx_alloc((evc_vlan_up(data) ? VTSS_PACKET_HDR_SIZE_BYTES : 0) + HEADER_MAX_SIZE + CCM_PDU_LENGTH)))    {vtss_mep_supp_trace("run_async_ccm_gen: packet tx alloc", 0, 0, 0, 0);   return;}
                memcpy(packet, data->tx_ccm_hw, ((evc_vlan_up(data) ? VTSS_PACKET_HDR_SIZE_BYTES : 0) + HEADER_MAX_SIZE + CCM_PDU_LENGTH));
                vtss_mep_supp_packet_tx_cancel(instance, data->tx_ccm_hw);
                data->tx_ccm_hw = packet;
            }

            header_size = init_tx_frame(instance, data->ccm_config.dmac, data->config.mac, data->ccm_config.prio, data->ccm_config.dei, data->tx_ccm_hw, evc_vlan_up(data));

            ccm_pdu_init(data, &data->tx_ccm_hw[header_size]);

            tx_frame_info.bypass = TRUE;
            tx_frame_info.maskerade_port = VTSS_PORT_NO_NONE;
            tx_frame_info.isdx = data->tx_isdx;
            tx_frame_info.vid = data->config.flow.vid;
            tx_frame_info.vid_inj = vlan_down(data);
            tx_frame_info.qos = data->ccm_config.prio;
            tx_frame_info.pcp = data->ccm_config.prio;
            tx_frame_info.dp = data->ccm_config.dei;

            if (evc_vlan_up(data)) { /* EVC/VLAN Up-MEP must send hw generated frames to loop port */
                memset(hw_port, FALSE, sizeof(hw_port));
                hw_port[voe_up_mep_loop_port] = TRUE;
            } else
                memcpy(hw_port, data->config.flow.port, sizeof(hw_port));

            if (!vtss_mep_supp_packet_tx(instance, NULL, hw_port, (header_size + CCM_PDU_LENGTH), data->tx_ccm_hw, &tx_frame_info, frame_rate_calc(data->ccm_gen.cc_period), FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_CCM : VTSS_MEP_SUPP_OAM_TYPE_NONE)))
                vtss_mep_supp_trace("run_async_ccm_gen: packet tx failed", 0, 0, 0, 0);

            data->hw_ccm_transmitting = TRUE;   /* HW based CCM is now on-going */
            data->tx_ccm_timer = 0;

            sw_ccm_period = data->ccm_gen.lm_period;    /* In case of any CCM-LM the period is LM period */
        }
        else {  /* SW CCM generation */
            if (data->tx_ccm_hw != NULL)    {vtss_mep_supp_packet_tx_cancel(instance, data->tx_ccm_hw);  data->tx_ccm_hw = NULL;}
            data->hw_ccm_transmitting = FALSE;

            for (i=0; i<data->ccm_config.peer_count; ++i)       /* All peer LOC timers are initialized with this LOC timer value */
                data->rx_ccm_timer[i] = data->loc_timer_val;
            
            data->defect_state.dLoc[0] = 0;
    
            sw_ccm_period = data->ccm_gen.cc_period;    /* Both CC and LM is based on SW CCM - the period is CC period */
        }

        if (data->tx_ccm_sw != NULL) {  /* SW based CCM must be transmitted with the calculated period */
            header_size = init_tx_frame(instance, data->ccm_config.dmac, data->config.mac, data->ccm_config.prio, data->ccm_config.dei, data->tx_ccm_sw, FALSE);
            ccm_pdu_init(data, &data->tx_ccm_sw[header_size]);
            
            data->ccm_timer_val = tx_timer_calc(sw_ccm_period);
//            data->tx_ccm_timer = data->ccm_timer_val;   /* Start timer transmitting CCM */
            // Now CCM is send the first time with 20 ms interval within one sec - this is only temporary
            data->tx_ccm_timer = first_timer;
            first_timer += 2;
            if (first_timer >= 100) first_timer = 2;

            vtss_mep_supp_timer_start();
        }
    }
    else {  /* Transmission of CCM PDU is disabled */
        if ((data->tx_ccm_sw != NULL) && !data->tx_ccm_sw_ongoing)   vtss_mep_supp_packet_tx_free(data->tx_ccm_sw);
        if (data->tx_ccm_hw != NULL)                                 vtss_mep_supp_packet_tx_cancel(instance, data->tx_ccm_hw);

        data->tx_ccm_hw = NULL;
        data->tx_ccm_sw = NULL;
        data->hw_ccm_transmitting = FALSE;
        data->tx_ccm_timer = 0;
        data->defect_state.dLoc[0] = 0;
    }

    data->event_flags |= EVENT_OUT_NEW_DEFECT;     /* Deliver CCM state to upper logic */
}

static void run_ccm_rdi(u32 instance)
{
    supp_instance_data_t  *data;
    u8                    flags;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_CCM_RDI;

#ifdef VTSS_ARCH_SERVAL
    if (data->config.voe)   /* VOE is assigned */
        (void)vtss_oam_voe_ccm_set_rdi_flag(NULL, data->voe_idx, data->ccm_rdi);
#endif
    if (data->tx_ccm_sw)
    {   /* SW CCM */
        flags = data->tx_ccm_sw[data->tx_header_size + 2];
        data->tx_ccm_sw[data->tx_header_size + 2] = ((flags & ~0x80) | ((data->ccm_rdi) ? 0x80 : 0x00));
    }

    if (!data->config.voe && hw_ccm_calc(data->ccm_gen.cc_period))
    {   /* This is a non VOE hw based CCM transmission */
        data->rdi_timer = 1000/timer_res;   /* Start timer for detecting stable RDI */
        vtss_mep_supp_timer_start();
    }
}

static u32 run_sync_lmm_config(u32 instance)
{
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    if (instance_data[instance].tx_lmm == NULL)
        if (!(data->tx_lmm = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + LMM_PDU_LENGTH)))
            return(VTSS_MEP_SUPP_RC_NO_RESOURCES);

    return(VTSS_MEP_SUPP_RC_OK);
}

static void run_async_lmm_config(u32 instance)
{
    static u32 first_timer = 1;
    u32                    header_size;
    supp_instance_data_t   *data;
    u8                     *pdu;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_LMM_CONFIG;

    voe_config(data);

    if (data->lmm_config.enable)
    {
        header_size = init_tx_frame(instance, data->lmm_config.dmac, data->config.mac, data->lmm_config.prio, data->lmm_config.dei, data->tx_lmm, FALSE);

        pdu = &data->tx_lmm[header_size];
        pdu[0] = data->config.level << 5;
        pdu[1] = OAM_TYPE_LMM;
        pdu[3] = 12;
        memset(&pdu[4], 0, LMM_PDU_LENGTH-4);
        data->lmm_timer_val = tx_timer_calc(data->lmm_config.period);
//        data->tx_lmm_timer = data->lmm_timer_val;   /* Start timer transmitting LMM */
        // Now LMM is send the first time with 20 ms interval within one sec - this is only temporary
        data->tx_lmm_timer = first_timer;
        first_timer += 2;
        if (first_timer >= 100) first_timer = 1;

        vtss_mep_supp_timer_start();
    }
    else
    {
        if ((data->tx_lmm != NULL) && !data->tx_lmm_ongoing)    vtss_mep_supp_packet_tx_free(data->tx_lmm);

        data->tx_lmm = NULL;
        data->tx_lmm_timer = 0;
    }
}


static void run_dmm_config(u32 instance)
{
    supp_instance_data_t       *data;
    u8                         *pdu;
    u32                        header_size;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_DMM_CONFIG;

    if (data->dmm_config.enable)
    {
        if (data->tx_dmm == NULL) 
            data->tx_dmm = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + DMM_PDU_LENGTH);

        if (data->tx_dmm != NULL)
        {/* DMM is ready and a tx buffer has been allocated */
            /* Init of DMM transmitter frame */
            header_size = init_tx_frame(instance, data->dmm_config.dmac, data->config.mac, data->dmm_config.prio, data->dmm_config.dei, data->tx_dmm, FALSE);
            pdu = &data->tx_dmm[header_size];
            pdu[0] = data->config.level << 5;
            pdu[1] = OAM_TYPE_DMM;
            pdu[2] = 0;
            pdu[3] = 32;
        } else
            return;

        data->dmm_tx_active = FALSE;        
        data->dmm_tx_cnt = 0;
        data->dmr_rx_cnt = 0;
        data->dmm_late_txtime_cnt = 0;            
        data->tx_dmm_fup_timestamp.seconds = 0; 
        data->tx_dmm_fup_timestamp.nanoseconds = 0;

        if (data->dmm_config.proprietary) {
            /* Expect to get a follow-up packet for DMR and add "VTSS" after "End TLV" to inform the remote site to send a follow-up packet. */
            int pdu_len = DMM_PDU_LENGTH;
            
            if (pdu_len == DMM_PDU_VTSS_LENGTH) {
                memcpy(&pdu[DMM_PDU_STD_LENGTH], "VTSS", 4);
            }      
        }
        else 
            memset(&pdu[DMM_PDU_STD_LENGTH], 0, 4);

        data->tx_dmm_timer = (data->dmm_config.interval * 10) / timer_res;

        vtss_mep_supp_timer_start();
    }
    else {
        if ((data->tx_dmm != NULL) && !data->tx_dmm_ongoing)    vtss_mep_supp_packet_tx_free(data->tx_dmm);

        data->tx_dmm = NULL;
        data->tx_dmm_timer = 0;
        data->tx_dmm_timeout_timer = 0;
    }
}


static void run_1dm_config(u32 instance)
{
    supp_instance_data_t            *data;
    u8                              *pdu;
    u32                             header_size;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_1DM_CONFIG;

    if (data->dm1_config.enable)
    {
        if (data->tx_dm1 == NULL)
            data->tx_dm1 = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + DM1_PDU_LENGTH);

        if (data->tx_dm1 != NULL)
        {/* 1DM is ready and a tx buffer has been allocated */
            /* Init of 1DM transmitter frame */
            header_size = init_tx_frame(instance, data->dm1_config.dmac, data->config.mac, data->dm1_config.prio, data->dm1_config.dei, data->tx_dm1, FALSE);
            pdu = &data->tx_dm1[header_size];
            pdu[0] = data->config.level << 5;
            pdu[1] = OAM_TYPE_1DM;
            pdu[2] = 0;
            pdu[3] = 16;
        } else
            return;
       
        data->dm1_tx_cnt = 0;
        data->dm1_fup_stdby = FALSE;            

        if (data->dm1_config.proprietary)
        {
            /* Inform the remote site that a follow-up packet will send out */
            int pdu_len = DM1_PDU_LENGTH;
            
            pdu[1] = OAM_TYPE_1DM;
            if (pdu_len == DM1_PDU_VTSS_LENGTH) 
            {
                data->done_dm1.instance = instance;
                data->done_dm1.cb = dm1_fup_done;
                memcpy(&pdu[DM1_PDU_STD_LENGTH], "VTSS", 4);
            }      
        }
        else 
        {
            pdu[1] = OAM_TYPE_1DM;
            memset(&pdu[DM1_PDU_STD_LENGTH], 0, 4);
            data->done_dm1.instance = instance;
            data->done_dm1.cb = dm1_done;   
        }
        
        data->tx_dm1_timer = (data->dm1_config.interval * 10) / timer_res;

        vtss_mep_supp_timer_start();
    }
    else {
        if ((data->tx_dm1 != NULL) && !data->tx_dm1_ongoing)    vtss_mep_supp_packet_tx_free(data->tx_dm1);

        data->tx_dm1 = NULL;
        data->tx_dm1_timer = 0;
    }
}

static u32 run_sync_aps_config(u32 instance)
{
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    if (instance_data[instance].tx_aps == NULL)
        if (!(data->tx_aps = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + RAPS_PDU_LENGTH)))
            return(VTSS_MEP_SUPP_RC_NO_RESOURCES);

    return(VTSS_MEP_SUPP_RC_OK);
}

static void run_async_aps_config(u32 instance)
{
    supp_instance_data_t  *data;
    u8                    *pdu;
    u32                   header_size;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_APS_CONFIG;

    if (data->aps_config.enable)
    {
        header_size = init_tx_frame(instance, data->aps_config.dmac, data->config.mac, data->aps_config.prio, data->aps_config.dei, data->tx_aps, FALSE);

        pdu = &data->tx_aps[header_size];
        pdu[0] = data->config.level << 5;
        pdu[0] |= (data->aps_config.type == VTSS_MEP_SUPP_L_APS) ? 0x00 : 0x01;     /* This is new version number for Version 2 of ERPS */
        pdu[1] = (data->aps_config.type == VTSS_MEP_SUPP_L_APS) ? OAM_TYPE_LAPS : OAM_TYPE_RAPS;
        pdu[3] = (data->aps_config.type == VTSS_MEP_SUPP_L_APS) ? 4 : 32;
        memcpy(&pdu[4], data->aps_txdata, APS_DATA_LENGTH);

        data->aps_timer_val = ((5000/timer_res)+1);

        data->aps_count = 0;
        data->event_flags |= EVENT_IN_TX_APS_TIMER;       /* Simulate run out of TX APS timer */
        vtss_mep_supp_run();
    }
    else
    {
        if ((data->tx_aps != NULL) && !data->tx_aps_ongoing)       vtss_mep_supp_packet_tx_free(data->tx_aps);

        data->tx_aps = NULL;
        data->tx_aps_timer = 0;
    }
}

static void run_aps_txdata(u32 instance)
{
    supp_instance_data_t  *data;
    u32                   tx_header_size;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_APS_TXDATA;

    if (data->aps_config.enable)
    {
        tx_header_size = evc_vlan_up(data) ? (data->tx_header_size - 4) : data->tx_header_size; /* The transmission of RAPS in a VLAN Up-MEP is not done masqueraded but directly on port without TAG */
        if (!data->aps_event)   memcpy(&data->tx_aps[tx_header_size + 4], data->aps_txdata, APS_DATA_LENGTH);
        else                    data->tx_aps[tx_header_size + 4] = 0xE0;     /* Transmission of an ERPS event */
        data->aps_count = 2;
        instance_data[instance].event_flags |= EVENT_IN_TX_APS_TIMER;       /* Simulate run out of TX APS timer */
//        vtss_mep_supp_run();         No longer asynchronious
        run_tx_aps_timer(instance);
    }
}

#ifdef VTSS_ARCH_SERVAL
static void run_aps_forward(u32 instance)
{
    vtss_oam_voe_conf_t   cfg;
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_APS_FORWARD;

    if (data->aps_config.enable)
    {
        if (data->voe_idx >= VTSS_OAM_VOE_CNT)   return;

        (void)vtss_oam_voe_conf_get(NULL, data->voe_idx, &cfg);
        cfg.generic[GENERIC_RAPS].forward = data->aps_forward;
        (void)vtss_oam_voe_conf_set(NULL, data->voe_idx, &cfg);
    }
}
#endif

static void run_ltm_config(u32 instance)
{
    u32                             rc, header_size;
    supp_instance_data_t            *data;
    u8                              *pdu;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_LTM_CONFIG;

    if (data->ltm_config.enable)
    {
        if ((data->tx_ltm == NULL) && (data->tx_ltm = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + LTM_PDU_LENGTH)))
        {/* LTM is ready and a tx buffer has been allocated */
            /* Init of LTM transmitter frame */
            header_size = init_tx_frame(instance, data->ltm_config.dmac, data->config.mac, data->ltm_config.prio, data->ltm_config.dei, data->tx_ltm, FALSE);
            pdu = &data->tx_ltm[header_size];
            pdu[0] = data->config.level << 5;
            pdu[1] = OAM_TYPE_LTM;
            pdu[2] = 0x80;
            pdu[3] = 17;
            var_to_string(&pdu[4], data->ltm_config.transaction_id);
            pdu[8] = data->ltm_config.ttl;
            memcpy(&pdu[9], data->config.mac, VTSS_MEP_SUPP_MAC_LENGTH);
            memcpy(&pdu[15], data->ltm_config.tmac, VTSS_MEP_SUPP_MAC_LENGTH);
            pdu[21] = 7;
            pdu[22] = 0;
            pdu[23] = 8;
            pdu[24] = 0;
            pdu[25] = 0;
            memcpy(&pdu[26], data->config.mac, VTSS_MEP_SUPP_MAC_LENGTH);
            pdu[32] = 0;

            data->ltr_cnt = 0;

            tx_frame_info.bypass = !evc_vlan_up(data);
            tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
            tx_frame_info.isdx = data->tx_isdx;
            tx_frame_info.vid = data->config.flow.vid;
            tx_frame_info.vid_inj = vlan_down(data);
            tx_frame_info.qos = data->ltm_config.prio;
            tx_frame_info.pcp = data->ltm_config.prio;
            tx_frame_info.dp = data->ltm_config.dei;

            rc = vtss_mep_supp_packet_tx(instance, &data->done_ltm, data->config.flow.port, (header_size + LTM_PDU_LENGTH), data->tx_ltm, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LTM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
            if (!rc)       vtss_mep_supp_trace("run_ltm_config: packet tx failed", instance, 0, 0, 0);
        }
    }
    else
        if (data->tx_ltm != NULL)    {vtss_mep_supp_packet_tx_free(data->tx_ltm);     data->tx_ltm = NULL;}
}

static void run_lbm_config(u32 instance)
{
    u32                             rc, i, frame_rate, header_size, pattern_size, payload_size;
    supp_instance_data_t            *data;
    u8                              *pdu;
    BOOL                            inj_hdr, hw_port[VTSS_PORT_ARRAY_SIZE];
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_LBM_CONFIG;

    voe_config(data);

    if (data->lbm_config.enable)
    {
        inj_hdr = ((data->lbm_config.to_send == VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE) && evc_vlan_up(data)) ? TRUE : FALSE;

        pattern_size = 0;
        payload_size = data->lbm_config.size - (12+2+4); /* Payload size is configured frame size subtracted untagged overhead and CRC */
    
        if (payload_size > LBM_PDU_LENGTH)   /* Check if pattern must be added to frame - requested payload is bigger than empty LBM PDU */
            pattern_size = payload_size - LBM_PDU_LENGTH;

        if ((data->tx_lbm == NULL) && (data->tx_lbm = vtss_mep_supp_packet_tx_alloc((inj_hdr ? VTSS_PACKET_HDR_SIZE_BYTES : 0) + HEADER_MAX_SIZE + LBM_PDU_LENGTH + pattern_size)))
        {/* LBM is ready and a tx buffer has been allocated */
            /* Init of LBM transmitter frame */
            header_size = init_tx_frame(instance, data->lbm_config.dmac, data->config.mac, data->lbm_config.prio, data->lbm_config.dei, data->tx_lbm, inj_hdr);
            pdu = &data->tx_lbm[header_size];
            pdu[0] = data->config.level << 5;
            pdu[1] = OAM_TYPE_LBM;
            pdu[2] = 0;
            pdu[3] = 4;
            data->lb_state.trans_id++;
            var_to_string(&pdu[4], data->lb_state.trans_id);
            pdu[8] = 3;
            pdu[9] = (pattern_size & 0xFF00)>>8;
            pdu[10] = (pattern_size & 0xFF);
            for (i=0; i<pattern_size; ++i)     pdu[11+i] = 0xAA;
            pdu[11+i] = 0;
    
            frame_rate = 0;     /* Calculation of frame rate */
            if (data->lbm_config.to_send == VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE)  /* This is HW based generation of LBM */
                frame_rate = 1000000/data->lbm_config.interval; /* 'interval' is in 1 us. One second is equal to 1000000 us. This is the calculation of how many frame should be send in one sec. */

            tx_frame_info.bypass = (frame_rate || !evc_vlan_up(data)) ? TRUE : FALSE;
            tx_frame_info.maskerade_port = (!frame_rate && evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
            tx_frame_info.isdx = data->tx_isdx;
            tx_frame_info.vid = data->config.flow.vid;
            tx_frame_info.vid_inj = vlan_down(data);
            tx_frame_info.qos = data->lbm_config.prio;
            tx_frame_info.pcp = data->lbm_config.prio;
            tx_frame_info.dp = data->lbm_config.dei;

            if (frame_rate && evc_vlan_up(data)) { /* EVC Up-MEP must send hw generated frames to loop port */
                memset(hw_port, FALSE, sizeof(hw_port));
                hw_port[voe_up_mep_loop_port] = TRUE;
            } else
                memcpy(hw_port, data->config.flow.port, sizeof(hw_port));

            if (data->lbm_config.to_send != VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE) {
                rc = vtss_mep_supp_packet_tx(instance, &data->done_lbm, hw_port, (header_size + LBM_PDU_LENGTH + pattern_size), data->tx_lbm, &tx_frame_info, frame_rate, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LBM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
                if (rc)     data->tx_lbm_ongoing = TRUE;
            } else { /* Do not give CB pointer in case of AFI */
                rc = vtss_mep_supp_packet_tx(instance, NULL, hw_port, (header_size + LBM_PDU_LENGTH + pattern_size), data->tx_lbm, &tx_frame_info, frame_rate, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LBM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
            }
            if (!rc)       vtss_mep_supp_trace("run_lbm_config: packet tx failed", instance, 0, 0, 0);

            data->lbm_count = (data->lbm_config.to_send != VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE) ? (data->lbm_config.to_send - 1) : 0;
            data->lbr_cnt = 0;
            data->lb_state.oo_counter = 0;
            data->lb_state.lbr_counter = 0;
            data->lb_state.lbm_counter = 1;

            if ((data->lbm_config.to_send != VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE) && (data->lbm_config.interval))  /* This is SW based generation of LBM - tx time is required */
            {/* Start LBM tx timer if any - othervise send next on tx done */
                data->tx_lbm_timer = (data->lbm_config.interval*10)/timer_res;
                vtss_mep_supp_timer_start();
            }
        }
    }
    else
    {
        data->tx_lbm_timer = 0;
        if (data->tx_lbm != NULL) {
            if (data->lbm_config.to_send != VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE) {  /* This is SW based generation of LBM - buffer must be freed */
                if (!data->tx_lbm_ongoing)  vtss_mep_supp_packet_tx_free(data->tx_lbm);
            } else
                vtss_mep_supp_packet_tx_cancel(instance, data->tx_lbm);

            data->tx_lbm = NULL;
        }
    }
}

static void run_lck_config (u32 instance)
{
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_LCK_CONFIG;

    if (data->lck_config.enable)
    {
        /* ETH-LCK packet to be constructed here */
        if ((data->tx_lck == NULL) && (data->tx_lck = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + LCK_PDU_LENGTH)))
        {/* LCK is ready and a tx buffer has been allocated */
            data->lck_count = 0;
            data->lck_inx = 0;
            data->event_flags |= EVENT_IN_TX_LCK_TIMER;       /* Simulate run out of TX LCK timer */
            vtss_mep_supp_run();
        }
    }
    else
    {
        if ((data->tx_lck != NULL) && !data->tx_lck_ongoing)     vtss_mep_supp_packet_tx_free(data->tx_lck);

        data->tx_lck = NULL;
        data->tx_lck_timer = 0;
    }
}

static void run_ais_set (u32 instance)
{
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_AIS_SET;

    if (data->ais_enable)
    {
        if ((data->tx_ais == NULL) && (data->tx_ais = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + AIS_PDU_LENGTH)))
        {
            data->ais_count = (data->ais_config.protection) ? 0 : 2;
            data->ais_inx = 0;
            data->event_flags |= EVENT_IN_TX_AIS_TIMER;       /* Simulate run out of TX AIS timer */
            vtss_mep_supp_run();
        }
    }
    else
    {
        if ((data->tx_ais != NULL) && !data->tx_ais_ongoing)     vtss_mep_supp_packet_tx_free(data->tx_ais);

        data->tx_ais = NULL;
        data->tx_ais_timer = 0;
    }
}

static void run_tst_config (u32 instance)
{
    supp_instance_data_t            *data;
    u8                              *pdu;
    u32                             i, rc, pattern_size, pattern, payload_size, frame_size, frame_rate, header_size;
    BOOL                            hw_port[VTSS_PORT_ARRAY_SIZE];
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TST_CONFIG;

    if (!data->tst_config.enable)  {    /* TX is not enabled */
        if (data->tx_tst != NULL)  {    /* TX is ongoing */
            vtss_mep_supp_packet_tx_cancel(instance, data->tx_tst); /* Cancel TST transmission */
            data->tx_tst = NULL;
            return; /* As Serval1 do not have separate LB and TST counters, disable of TST in VOE must be done after cancel of TST transmission - in tst_done() */
        }
    }

    voe_config(data);

    if (data->tst_config.enable)
    {
        if (data->tx_tst != NULL)    {vtss_mep_supp_packet_tx_cancel(instance, data->tx_tst);        data->tx_tst = NULL;}
        pattern_size = 0;
        payload_size = data->tst_config.size - (12+2+4); /* Payload size is configured frame size subtracted untagged overhead and CRC */

        if (payload_size > TST_PDU_LENGTH)   /* Check if pattern must be added to frame - requested payload is bigger than empty TST PDU */
            pattern_size = payload_size - TST_PDU_LENGTH;

        if ((data->tx_tst == NULL) && (data->tx_tst = vtss_mep_supp_packet_tx_alloc(((evc_vlan_up(data) ? VTSS_PACKET_HDR_SIZE_BYTES : 0) + HEADER_MAX_SIZE + TST_PDU_LENGTH + pattern_size))))
        {/* TST is ready and a tx buffer has been allocated */
            /* Init of TST transmitter frame - remember that the CRC is inserted by packet transmitter */
            header_size = init_tx_frame(instance, data->tst_config.dmac, data->config.mac, data->tst_config.prio, data->tst_config.dei, data->tx_tst, evc_vlan_up(data));
            pdu = &data->tx_tst[header_size];
            pdu[0] = data->config.level << 5;
            pdu[1] = OAM_TYPE_TST;
            pdu[2] = 0;
            pdu[3] = 4;
            pdu[4] = 0;
            pdu[5] = 0;
            pdu[6] = 0;
            pdu[7] = 0;
            pdu[8] = 3;
            pdu[9] = (pattern_size & 0xFF00)>>8;
            pdu[10] = (pattern_size & 0xFF);
            switch (data->tst_config.pattern)
            {
                case VTSS_MEP_SUPP_PATTERN_ALL_ZERO: pattern = 0; break;
                case VTSS_MEP_SUPP_PATTERN_ALL_ONE:  pattern = 0xFF; break;
                case VTSS_MEP_SUPP_PATTERN_0XAA:     pattern = 0xAA; break;
                default:                             pattern = 0; break;
            }
            for (i=0; i<pattern_size; ++i)     pdu[11+i] = pattern;
            pdu[11+i] = 0;

            frame_size = data->tst_config.size + 8 + 12; /* Calculate frame size - Preamble + Inter fame gap */
            frame_rate = ((data->tst_config.rate * 1000) / (frame_size * 8)) + 1;     /* Configured TST rate is in Kbps - now converted to fps. One frame is added to achive bit rate not less than requested */
//printf("payload_size %u  pattern_size %u  header_size %u  frame_size %u  frame_rate %u\n",payload_size,pattern_size,header_size,frame_size,frame_rate);

            data->tst_tx_cnt = 0;

            tx_frame_info.bypass = TRUE;
            tx_frame_info.maskerade_port = VTSS_PORT_NO_NONE;
            tx_frame_info.isdx = data->tx_isdx;
            tx_frame_info.vid = data->config.flow.vid;
            tx_frame_info.vid_inj = vlan_down(data);
            tx_frame_info.qos = data->tst_config.prio;
            tx_frame_info.pcp = data->tst_config.prio;
            tx_frame_info.dp = data->tst_config.dei;

            if (evc_vlan_up(data)) { /* EVC Up-MEP must send hw generated frames to loop port */
                memset(hw_port, FALSE, sizeof(hw_port));
                hw_port[voe_up_mep_loop_port] = TRUE;
            } else
                memcpy(hw_port, data->config.flow.port, sizeof(hw_port));

            rc = vtss_mep_supp_packet_tx(instance, &data->done_tst, hw_port, (header_size + TST_PDU_LENGTH + pattern_size), data->tx_tst, &tx_frame_info, frame_rate, TRUE, (data->tst_config.sequence) ? (header_size + 4) : 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LBM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
            if (!rc)       vtss_mep_supp_trace("run_tst_config: packet tx failed", instance, 0, 0, 0);
        }
    }
}


static void run_defect_timer(u32 instance)
{
    supp_instance_data_t  *data;
    BOOL                  new_, ccm_active, ais_lck_active;
    u32                   i;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_DEFECT_TIMER;

    new_ = ccm_active = ais_lck_active = FALSE;

    if (!data->dLevel.timer)
    {
        if (data->defect_state.dLevel)
        {
            data->dLevel.old_period = 0;
            data->defect_state.dLevel = FALSE;
            new_ = TRUE;
        }
    } else ccm_active = TRUE;
    if (!data->dMeg.timer)
    {
        if (data->defect_state.dMeg)
        {
            data->dMeg.old_period = 0;
            data->defect_state.dMeg = FALSE;
            new_ = TRUE;
        }
    } else ccm_active = TRUE;
    if (!data->dMep.timer)
    {
        if (data->defect_state.dMep)
        {
            data->dMep.old_period = 0;
            data->defect_state.dMep = FALSE;
            new_ = TRUE;
        }
    } else ccm_active = TRUE;
    for (i=0; i<data->ccm_config.peer_count; ++i)
    {
        if (!data->dPeriod[i].timer)
        {
            if (data->defect_state.dPeriod[i])
            {
                data->dPeriod[i].old_period = 0;
                data->defect_state.dPeriod[i] = FALSE;
                new_ = TRUE;
            }
        } else ccm_active = TRUE;
        if (!data->dPrio[i].timer)
        {
            if (data->defect_state.dPrio[i])
            {
                data->dPrio[i].old_period = 0;
                data->defect_state.dPrio[i] = FALSE;
                new_ = TRUE;
            }
        } else ccm_active = TRUE;
    }
    if (!data->dAis.timer)
    {
        if (data->defect_state.dAis)
        {
            data->dAis.old_period = 0;
            data->defect_state.dAis = FALSE;
            new_ = TRUE;
        }
    } else ais_lck_active = TRUE;
    if (!data->dLck.timer)
    {
        if (data->defect_state.dLck)
        {
            data->dLck.old_period = 0;
            data->defect_state.dLck = FALSE;
            new_ = TRUE;
        }
    } else ais_lck_active = TRUE;

    data->defect_timer_active = ccm_active || ais_lck_active;
    data->ccm_defect_active = ccm_active;

    if (new_) {
        data->event_flags |= EVENT_OUT_NEW_DEFECT;
    }
}

static void run_rx_ccm_timer(u32 instance)
{
    supp_instance_data_t  *data;
    BOOL                  new_;
    u32                   i;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_RX_CCM_TIMER;

    new_ = FALSE;

    for (i=0; i<data->ccm_config.peer_count; ++i)
    {
        if (!data->rx_ccm_timer[i])
        {
            if (!(data->defect_state.dLoc[i] & 0x01))
            { /* sw detected LOC is active */
                data->defect_state.dLoc[i] |= 0x01;
                data->defect_state.dRdi[i] = FALSE;
                new_ = TRUE;
            }
        }
    }

    if (new_) {
        data->event_flags |= EVENT_OUT_NEW_DEFECT;
    }
}

/*lint -e{454, 455, 456} ... The mutex is locked so it is ok to unlock */
static void run_tx_ccm_timer(u32 instance)
{
    u32                             rc, tx_frames, rx_frames;
    supp_instance_data_t            *data;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TX_CCM_TIMER;

    data->tx_ccm_timer = data->ccm_timer_val;
    vtss_mep_supp_timer_start();

    if (data->tx_ccm_sw_ongoing)   return;

    if (data->ccm_gen.lm_enable)
    {   /* LM carried in CCM PDU is enabled */
        data->ccm_lm_counter.tx_counter++;

        if (!data->config.voe) { /* This is not a VOE based instance */
            vtss_mep_supp_crit_unlock();
            vtss_mep_supp_counters_get(instance, &rx_frames, &tx_frames);
            vtss_mep_supp_crit_lock();
            var_to_string(&data->tx_ccm_sw[data->tx_header_size+58], tx_frames);
        }
        else { /* This is VOE basede - in case of UP-MEP the counters to get updated must be non zero */
            memset(&data->tx_ccm_sw[data->tx_header_size+58], 1, 4);
        }
    }

    tx_frame_info.bypass = !evc_vlan_up(data);
    tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
    tx_frame_info.isdx = data->tx_isdx;
    tx_frame_info.vid = data->config.flow.vid;
    tx_frame_info.vid_inj = vlan_down(data);
    tx_frame_info.qos = data->ccm_config.prio;
    tx_frame_info.pcp = data->ccm_config.prio;
    tx_frame_info.dp = data->ccm_config.dei;

    rc = vtss_mep_supp_packet_tx(instance, &data->done_ccm, data->config.flow.port, (data->tx_header_size + CCM_PDU_LENGTH), data->tx_ccm_sw, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_CCM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
    if (!rc)    vtss_mep_supp_trace("run_tx_ccm_timer: packet tx failed", instance, 0, 0, 0);
    if (rc)     data->tx_ccm_sw_ongoing = TRUE;
}

static void run_tx_aps_timer(u32 instance)
{
    u32                             rc, tx_header_size;
    supp_instance_data_t            *data;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TX_APS_TIMER;
    data->tx_aps_timer = data->aps_timer_val;
    vtss_mep_supp_timer_start();

    if (data->tx_aps_ongoing)   return;

    if ((data->aps_config.type != VTSS_MEP_SUPP_R_APS) || data->aps_tx)
    {/* R-APS PDU must only be transmitted when 'active' is enabled from ERPS */
        tx_frame_info.bypass = TRUE;
        tx_frame_info.maskerade_port = VTSS_PORT_NO_NONE;
        tx_frame_info.isdx = data->tx_isdx;
        tx_frame_info.vid = data->config.flow.vid;
        tx_frame_info.vid_inj = (data->config.flow.mask & VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID) ? TRUE : FALSE;
        tx_frame_info.qos = data->aps_config.prio;
        tx_frame_info.pcp = data->aps_config.prio;
        tx_frame_info.dp = data->aps_config.dei;

        tx_header_size = evc_vlan_up(data) ? (data->tx_header_size - 4) : data->tx_header_size; /* The transmission of RAPS in a VLAN Up-MEP is not done masqueraded but directly on port without TAG */
        rc = vtss_mep_supp_packet_tx(instance, &data->done_aps, data->config.flow.port, (tx_header_size + APS_PDU_LENGTH), data->tx_aps, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_GENERIC : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        if (!rc)       vtss_mep_supp_trace("run_tx_aps_timer: packet tx failed", instance, 0, 0, 0);
        if (rc)     data->tx_aps_ongoing = TRUE;
    }
}

/*lint -e{454, 455, 456} ... The mutex is locked so it is ok to unlock */
static void run_tx_lmm_timer(u32 instance)
{
    u32                             rc, rx_frames, tx_frames;
    supp_instance_data_t            *data;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TX_LMM_TIMER;

    if (!data->lmm_config.enable)   return;

    data->tx_lmm_timer = data->lmm_timer_val;
    vtss_mep_supp_timer_start();

    if (data->tx_lmm_ongoing)   return;

    data->lmm_lm_counter.tx_counter++;
    if (!instance_data[instance].config.voe) { /* This is not a VOE based instance */
        vtss_mep_supp_crit_unlock();
        vtss_mep_supp_counters_get(instance, &rx_frames, &tx_frames);
        vtss_mep_supp_crit_lock();
    }
    else
        tx_frames = 1;   /* I think this must be != 0 in or to be overwritten by VOE */
    var_to_string(&data->tx_lmm[data->tx_header_size+4], tx_frames);

    tx_frame_info.bypass = !evc_vlan_up(data);
    tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
    tx_frame_info.isdx = data->tx_isdx;
    tx_frame_info.vid = data->config.flow.vid;
    tx_frame_info.vid_inj = vlan_down(data);
    tx_frame_info.qos = data->lmm_config.prio;
    tx_frame_info.pcp = data->lmm_config.prio;
    tx_frame_info.dp = data->lmm_config.dei;

    rc = vtss_mep_supp_packet_tx(instance, &data->done_lmm, data->config.flow.port, (data->tx_header_size + LMM_PDU_LENGTH), data->tx_lmm, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LMM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
    if (!rc)       vtss_mep_supp_trace("run_tx_lmm_timer: packet tx failed", instance, 0, 0, 0);
    if (rc)     data->tx_lmm_ongoing = TRUE;
}

static void run_rx_aps_timer(u32 instance)
{
    instance_data[instance].event_flags &= ~EVENT_IN_RX_APS_TIMER;

    instance_data[instance].event_flags |= EVENT_OUT_NEW_APS;
}

static void run_tx_lbm_timer(u32 instance)
{
    u32                             rc, pattern_size, payload_size;
    supp_instance_data_t            *data;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TX_LBM_TIMER;

    if (!data->lbm_config.enable)   return;
    if (data->lbm_config.to_send == VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE) return;   /* This HW transmission - there should be no LBM tx timer !!!! */

    if (data->lbm_count)
    {
        if (data->lbm_config.interval)
        {/* Start LBM tx timer if any - othervise send next on tx done */
            data->tx_lbm_timer = (data->lbm_config.interval*10)/timer_res;
            vtss_mep_supp_timer_start();
        }

        if (data->tx_lbm_ongoing)   return;

        data->lbm_count--;
        data->lb_state.trans_id++;   /* Increment Transaction ID */
        var_to_string(&data->tx_lbm[data->tx_header_size+4], data->lb_state.trans_id);

        tx_frame_info.bypass = !evc_vlan_up(data);
        tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
        tx_frame_info.isdx = data->tx_isdx;
        tx_frame_info.vid = data->config.flow.vid;
        tx_frame_info.vid_inj = vlan_down(data);
        tx_frame_info.qos = data->lbm_config.prio;
        tx_frame_info.pcp = data->lbm_config.prio;
        tx_frame_info.dp = data->lbm_config.dei;

        pattern_size = 0;
        payload_size = data->lbm_config.size - (12+2+4); /* Payload size is configured frame size subtracted untagged overhead and CRC */
    
        if (payload_size > LBM_PDU_LENGTH)   /* Check if pattern must be added to frame - requested payload is bigger than empty LBM PDU */
            pattern_size = payload_size - LBM_PDU_LENGTH;

        rc = vtss_mep_supp_packet_tx(instance, &data->done_lbm, data->config.flow.port, (data->tx_header_size + LBM_PDU_LENGTH + pattern_size), data->tx_lbm, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LBM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        if (!rc)       vtss_mep_supp_trace("run_tx_lbm_timer: packet tx failed", instance, 0, 0, 0);
        if (rc)     data->tx_lbm_ongoing = TRUE;

        data->lb_state.lbm_counter++;
    }
}

static void run_rx_dmr_timer(u32 instance)
{
    supp_instance_data_t        *data;
    
    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_RX_DMR_TIMER;
    /* The expected DMR not coming. Give it up */
    data->dmm_tx_active = FALSE;
    data->dmr_rx_tout_cnt++;
    data->event_flags |= EVENT_OUT_NEW_DMR;
    if (!data->tx_dmm_timer)
    {
        data->event_flags |= EVENT_IN_TX_DMM_TIMER;
    }    
}    

static void run_tx_dmm_timer(u32 instance)
{
    u32                           rc;
    supp_instance_data_t          *data;
    vtss_timestamp_t              timestamp;
    u32                           tc=0;
    vtss_mep_supp_tx_frame_info_t tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TX_DMM_TIMER;

    if (!data->dmm_config.enable)   return;

    data->tx_dmm_timer = (data->dmm_config.interval * 10) / timer_res;
    vtss_mep_supp_timer_start();

    if (data->tx_dmm_ongoing)   return;

    if (!data->dmm_tx_active)
    {
        vtss_mep_supp_timestamp_get(&timestamp, &tc);
        var_to_string(&data->tx_dmm[data->tx_header_size + 4], timestamp.seconds);
        var_to_string(&data->tx_dmm[data->tx_header_size + 8], timestamp.nanoseconds);

        data->tx_dmm_fup_timestamp.seconds = 0; 
        data->tx_dmm_fup_timestamp.nanoseconds = 0;
        data->dmr_received = 0;
        
        /* If DMR not received in one second, treat it as timeout */
        data->tx_dmm_timeout_timer = 1000 / timer_res;
        
        tx_frame_info.bypass = !evc_vlan_up(data);
        tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
        tx_frame_info.isdx = data->tx_isdx;
        tx_frame_info.vid = data->config.flow.vid;
        tx_frame_info.vid_inj = vlan_down(data);
        tx_frame_info.qos = data->dmm_config.prio;
        tx_frame_info.pcp = data->dmm_config.prio;
        tx_frame_info.dp = data->dmm_config.dei;

        if (!vtss_mep_supp_check_hw_timestamp())
        {    
            /* no h/w timestamp */
            rc = vtss_mep_supp_packet_tx(instance, &data->done_dmm, data->config.flow.port, (data->tx_header_size + DMM_PDU_LENGTH), data->tx_dmm, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_DMM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        }
        else
        {         
            /* one step */
#if DEBUG_PHY_TS
            u32 zero = 0;
            var_to_string(&data->tx_dmm[data->tx_header_size + 4], zero);
            var_to_string(&data->tx_dmm[data->tx_header_size + 8], zero);           
            rc = vtss_mep_supp_packet_tx(instance, &data->done_dmm, data->config.flow.port, (data->tx_header_size + DMM_PDU_LENGTH), data->tx_dmm, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_DMM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
#else 
            rc = vtss_mep_supp_packet_tx_one_step_ts(instance, &data->done_dmm, data->config.flow.port, (data->tx_header_size + DMM_PDU_LENGTH), data->tx_dmm, &tx_frame_info, tc, &data->onestep_extra_m, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_DMM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
#endif       
        }
        if (!rc)    vtss_mep_supp_trace("run_tx_dmm_timer: packet tx failed", instance, 0, 0, 0);
        if (rc)     data->tx_dmm_ongoing = TRUE;

        data->dmm_tx_active = TRUE;
        data->dmm_tx_cnt++;
    }
}

static void run_tx_dm1_timer(u32 instance)
{
    u32                           rc;
    vtss_timestamp_t              timestamp;
    supp_instance_data_t          *data;
    u32                           tc=0;
    vtss_mep_supp_tx_frame_info_t tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TX_1DM_TIMER;

    if (!data->dm1_config.enable)   return;

    data->tx_dm1_timer = (data->dm1_config.interval * 10) / timer_res;
    vtss_mep_supp_timer_start();

    if (data->tx_dm1_ongoing)   return;

    vtss_mep_supp_timestamp_get(&timestamp, &tc);
    var_to_string(&data->tx_dm1[data->tx_header_size + 4], timestamp.seconds);
    var_to_string(&data->tx_dm1[data->tx_header_size + 8], timestamp.nanoseconds);

    tx_frame_info.bypass = !evc_vlan_up(data);
    tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
    tx_frame_info.isdx = data->tx_isdx;
    tx_frame_info.vid = data->config.flow.vid;
    tx_frame_info.vid_inj = vlan_down(data);
    tx_frame_info.qos = data->dm1_config.prio;
    tx_frame_info.pcp = data->dm1_config.prio;
    tx_frame_info.dp = data->dm1_config.dei;

    if (data->dm1_config.proprietary)
    {
        /* The opcode was modified because of follow-up packets */
        data->tx_dm1[data->tx_header_size + 1] = OAM_TYPE_1DM;
        if (!vtss_mep_supp_check_hw_timestamp())
        {    
            /* no h/w timestamp */           
            rc = vtss_mep_supp_packet_tx(instance, &data->done_dm1, data->config.flow.port, (data->tx_header_size + DM1_PDU_LENGTH), data->tx_dm1, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_1DM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        }
        else
        {                
            /* two step*/
            rc = vtss_mep_supp_packet_tx_two_step_ts(instance, &data->done_dm1, data->config.flow.port, (data->tx_header_size + DM1_PDU_LENGTH), data->tx_dm1);
        }
        data->dm1_fup_stdby = TRUE;
    }
    else
    {
        if (!vtss_mep_supp_check_hw_timestamp())
        {
            /* no h/w timestamp*/               
            rc = vtss_mep_supp_packet_tx(instance, &data->done_dm1, data->config.flow.port, (data->tx_header_size + DM1_PDU_LENGTH), data->tx_dm1, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_1DM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        }
        else
        {    
            /* one step */
#if DEBUG_PHY_TS
            u32 zero = 0;
            var_to_string(&data->tx_dm1[data->tx_header_size + 4], zero);
            var_to_string(&data->tx_dm1[data->tx_header_size + 8], zero);           
            rc = vtss_mep_supp_packet_tx(instance, &data->done_dm1, data->config.flow.port, (data->tx_header_size + DM1_PDU_LENGTH), data->tx_dm1, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_1DM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
#else                
            rc = vtss_mep_supp_packet_tx_one_step_ts(instance, &data->done_dm1, data->config.flow.port, (data->tx_header_size + DM1_PDU_LENGTH), data->tx_dm1, &tx_frame_info, tc, &data->onestep_extra_m, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_1DM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
#endif                       
        }
    }
    if (!rc)       vtss_mep_supp_trace("run_tx_dm1_timer: packet tx failed", instance, 0, 0, 0);
    if (rc)     data->tx_dm1_ongoing = TRUE;

    data->dm1_tx_cnt++;
    data->event_flags |= EVENT_OUT_NEW_DM1;
}

static void run_rx_lbm_timer(u32 instance)
{
    supp_instance_data_t  *data;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_RX_LBM_TIMER;

    if ((data->tx_lbr != NULL) && !data->tx_lbr_ongoing)     vtss_mep_supp_packet_tx_free(data->tx_lbr);

    data->tx_lbr = NULL;
}

static void run_tx_ais_timer(u32 instance)
{
    u32                             rc, header_size;;
    supp_instance_data_t            *data;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TX_AIS_TIMER;

    data->tx_ais_timer = (tx_timer_calc(data->ais_config.period) / data->ais_config.flow_count) + 1;

    vtss_mep_supp_timer_start();

    if (data->tx_ais_ongoing)   return;

    /* Init of AIS transmitter frame */
    header_size = init_client_tx_frame(instance, data->ais_config.dmac, data->config.mac, data->ais_config.prio[data->ais_inx], data->ais_config.dei, data->tx_ais);
    data->tx_ais[header_size] = data->ais_config.level[data->ais_inx] << 5;
    data->tx_ais[header_size + 1] = OAM_TYPE_AIS;
    data->tx_ais[header_size + 2] = period_to_pdu_calc(data->ais_config.period);
    data->tx_ais[header_size + 3] = 0;
    data->tx_ais[header_size + 4] = 0;

    tx_frame_info.bypass = TRUE;
    tx_frame_info.maskerade_port = VTSS_PORT_NO_NONE;
    tx_frame_info.isdx = 0;    /* The ISDX for AIS in EVC can be fetched from EVC API */
    tx_frame_info.vid = data->ais_config.flows[data->ais_inx].vid;
    tx_frame_info.vid_inj = TRUE;
    tx_frame_info.qos = data->ais_config.prio[data->ais_inx];
    tx_frame_info.pcp = data->ais_config.prio[data->ais_inx];
    tx_frame_info.dp = data->ais_config.dei;

    rc = vtss_mep_supp_packet_tx(instance, &data->done_ais, data->ais_config.flows[data->ais_inx].port, (header_size + AIS_PDU_LENGTH), data->tx_ais, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_GENERIC : VTSS_MEP_SUPP_OAM_TYPE_NONE));
    if (!rc)    vtss_mep_supp_trace("run_tx_ais_timer: packet tx failed", instance, 0, 0, 0);
    if (rc)     data->tx_ais_ongoing = TRUE;

    data->ais_inx++;
    if (data->ais_inx >= data->ais_config.flow_count)   /* Transmit AIS in all configured flows */
    {
        data->ais_inx = 0;
        if (data->ais_count < 3)    data->ais_count++;  /* This counter is used for transmitting 3 frames as fast as possible in case 'Protection' is selected */
    }
}

static void run_tx_lck_timer(u32 instance)
{
    u32                             rc, header_size;;
    supp_instance_data_t            *data;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TX_LCK_TIMER;

    if (!data->lck_config.enable)   return;

    data->tx_lck_timer = (tx_timer_calc(data->lck_config.period) / data->lck_config.flow_count) + 1;

    vtss_mep_supp_timer_start();

    if (data->tx_lck_ongoing)   return;

    /* Init of LCK transmitter frame */
    header_size = init_client_tx_frame(instance, data->lck_config.dmac, data->config.mac, data->lck_config.prio[data->lck_inx], data->lck_config.dei, data->tx_lck);
    data->tx_lck[header_size] = data->lck_config.level[data->lck_inx] << 5;
    data->tx_lck[header_size + 1] = OAM_TYPE_LCK;
    data->tx_lck[header_size + 2] = period_to_pdu_calc(data->lck_config.period);
    data->tx_lck[header_size + 3] = 0;
    data->tx_lck[header_size + 4] = 0;

    tx_frame_info.bypass = TRUE;
    tx_frame_info.maskerade_port = VTSS_PORT_NO_NONE;
    tx_frame_info.isdx = data->tx_isdx;
    tx_frame_info.vid = data->lck_config.flows[data->lck_inx].vid;
    tx_frame_info.vid_inj = TRUE;
    tx_frame_info.qos = data->lck_config.prio[data->lck_inx];
    tx_frame_info.pcp = data->lck_config.prio[data->lck_inx];
    tx_frame_info.dp = data->lck_config.dei;

    rc = vtss_mep_supp_packet_tx(instance, &data->done_lck, data->lck_config.flows[data->lck_inx].port, (header_size + LCK_PDU_LENGTH), data->tx_lck, &tx_frame_info, VTSS_MEP_SUPP_PERIOD_INV, FALSE, 0, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_GENERIC : VTSS_MEP_SUPP_OAM_TYPE_NONE));

    if (!rc)    vtss_mep_supp_trace("run_tx_lck_timer: packet tx failed", instance, 0, 0, 0);
    if (rc)     data->tx_lck_ongoing = TRUE;

    data->lck_inx++;
    if (data->lck_inx >= data->lck_config.flow_count)   /* Transmit LCK in all configured flows */
    {
        data->lck_inx = 0;
        if (data->lck_count < 1)    data->lck_count++;   /* This counter could be used for transmitting 3 frames as fast as possible - currently unused */
    }
}

static void run_rdi_timer (u32 instance)
{
    supp_instance_data_t   *data;
    BOOL                   rdi;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_RDI_TIMER;

    if ((data->ccm_gen.enable) && hw_ccm_calc(data->ccm_gen.cc_period))
    {   /* HW generated CCM */
        rdi = ((data->tx_ccm_hw[data->ifh_size + data->tx_header_size + 2] & 0x80) != 0) ? TRUE : FALSE;
        if (rdi != data->ccm_rdi)  run_async_ccm_gen(instance);
    }
}

#ifdef VTSS_ARCH_SERVAL
static void run_tst_timer (u32 instance)
{
    supp_instance_data_t  *data;
    vtss_oam_voe_conf_t   cfg;

    data = &instance_data[instance];    /* Instance data reference */

    data->event_flags &= ~EVENT_IN_TST_TIMER;

    if (data->voe_idx < VTSS_OAM_VOE_CNT) { /* VOE based TST */
        (void)vtss_oam_voe_conf_get(NULL, data->voe_idx, &cfg);
        cfg.tst.copy_to_cpu = TRUE;
        (void)vtss_oam_voe_conf_set(NULL, data->voe_idx, &cfg);
    }
}
#endif

static void lm_counter_calc(lm_counter_t    *lm,
                            BOOL            *lm_start,
                            u32             near_CT2,
                            u32             far_CT2,
                            u32             near_CR2,
                            u32             far_CR2)
{
    u32 farTxC, nearTxC, farRxC, nearRxC;
    int nearLos, farLos;

    if (!*lm_start) { /* The first measurement is only used to initialize the 'previously frame counters */
        /* See Apendix III in Y.1731  */
        farTxC = (far_CT2 - lm->far_CT1);
        nearTxC = (near_CT2 - lm->near_CT1);
        nearRxC = (near_CR2 - lm->near_CR1);
        farRxC = (far_CR2 - lm->far_CR1);
    
        nearLos = (farTxC - nearRxC);
        farLos = (nearTxC - farRxC);
/*vtss_mep_supp_trace("lm_counter_calc - nearLos - farLos", nearLos, farLos, 0, 0);*/

        lm->near_los_counter += nearLos;
        lm->far_los_counter += farLos;
        lm->near_tx_counter += nearTxC;
        lm->far_tx_counter += farTxC;
    }
    *lm_start = FALSE;

    lm->near_CT1 = near_CT2;
    lm->near_CR1 = near_CR2;
    lm->far_CT1 = far_CT2;
    lm->far_CR1 = far_CR2;

    return;
}

static void defect_timer_calc(supp_instance_data_t  *data,   defect_timer_t *defect,   u32 period)
{
    if (period > defect->old_period)       defect->old_period = period;
    defect->timer = pdu_period_to_timer[defect->old_period];

    data->defect_timer_active = TRUE;
    vtss_mep_supp_timer_start();
}

static BOOL ccm_defect_calc(u32 instance,   supp_instance_data_t *data,   u8 smac[],   u8 pdu[],   u32 prio,   BOOL *defect,   BOOL *state)
{
    u32   i;
	u32   mep, level, period, rx_period;
    BOOL  rdi, unexp;

    /* Extract PDU information */
    level = pdu[0]>>5;
    mep = (pdu[8]<<8) + pdu[9];
    period = rx_period = pdu[2] & 0x07;
    rdi = (pdu[2] & 0x80);

    *defect = *state = unexp = FALSE;

    if (period == 0)
    {
        unexp = TRUE;
        if (!data->defect_state.dInv)                       *defect = TRUE;
        if (data->ccm_state.state.unexp_period != period)   *state = TRUE;
        data->defect_state.dInv = TRUE;
        data->ccm_state.state.unexp_period = period;
        goto return_;
    }
    else
    {
        if (data->defect_state.dInv)    *defect = TRUE;
        data->defect_state.dInv = FALSE;
    }

    if (voe_ccm_enable(data)) { /* This is a VOE based CCM - hit me once poll of PDU */
        if (period < period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_10S))  period = period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_10S);
    }
    else {  /* Either this is not VOE based MEP or there are more peers. In this case either all CCM come to CPU or they are polled with 1s interval */
        if (hw_ccm_calc(data->ccm_config.period))  period = period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_1S);      /* HW CCM is enabled - the period must be 1s. */
    }

    /* Calculate all defects related to this CCM PDU */
    if (level != data->config.level)
    {
        unexp = TRUE;
        if (!data->defect_state.dLevel)                  *defect = TRUE;
        if (data->ccm_state.state.unexp_level != level)  *state = TRUE;
        data->defect_state.dLevel = TRUE;
        data->ccm_state.state.unexp_level = level;
        defect_timer_calc(data, &data->dLevel, period);
        data->ccm_defect_active = TRUE;
        goto return_;
    }
    if (memcmp(&pdu[10], data->exp_meg, data->exp_meg_len))
    {
        unexp = TRUE;
        if (!data->defect_state.dMeg)                                       *defect = TRUE;
        if (memcmp(&pdu[10], data->ccm_state.unexp_meg_id, MEG_ID_LENGTH))  *state = TRUE;
        data->defect_state.dMeg = TRUE;
        memcpy(data->ccm_state.unexp_meg_id, &pdu[10], MEG_ID_LENGTH);
        defect_timer_calc(data, &data->dMeg, period);
        data->ccm_defect_active = TRUE;
        goto return_;
    }
    for (i=0; i<data->ccm_config.peer_count; ++i)   if (data->ccm_config.peer_mep[i] == mep)   break;
    if (i == data->ccm_config.peer_count)
    {
        unexp = TRUE;
        if (!data->defect_state.dMep)               *defect = TRUE;
        if (data->ccm_state.state.unexp_mep != mep) *state = TRUE;
        data->defect_state.dMep = TRUE;
        data->ccm_state.state.unexp_mep = mep;
        defect_timer_calc(data, &data->dMep, period);
        data->ccm_defect_active = TRUE;
        goto return_;
    }
    else
        memcpy(data->peer_mac[i], smac, VTSS_MEP_SUPP_MAC_LENGTH);

    if (pdu_period_to_conf[rx_period] != data->ccm_config.period)
    {
        if (!data->defect_state.dPeriod[i])                                      *defect = TRUE;
        if (data->ccm_state.state.unexp_period != pdu_period_to_conf[rx_period]) *state = TRUE;
        data->defect_state.dPeriod[i] = TRUE;
        data->ccm_state.state.unexp_period = pdu_period_to_conf[rx_period];
        defect_timer_calc(data, &data->dPeriod[i], period);
        data->ccm_defect_active = TRUE;
    }
    if ((prio != 0xFFFFFFFF) && (prio != data->ccm_config.prio))
    {
        if (!data->defect_state.dPrio[i])               *defect = TRUE;
        if (data->ccm_state.state.unexp_prio != prio)   *state = TRUE;
        data->defect_state.dPrio[i] = TRUE;
        data->ccm_state.state.unexp_prio = prio;
        defect_timer_calc(data, &data->dPrio[i], period);
        data->ccm_defect_active = TRUE;
    }
    if (data->defect_state.dRdi[i] != rdi)
    {
        *defect = TRUE;
        data->defect_state.dRdi[i] = rdi;
    }

    if (data->defect_state.dLoc[i] & 0x01)
        *defect = TRUE;

    data->defect_state.dLoc[i] &= ~0x01;   /* sw detected LOC is passive */
    data->rx_ccm_timer[i] = data->loc_timer_val;
    vtss_mep_supp_timer_start();

    return_:

    return(unexp);
}

#ifdef VTSS_ARCH_SERVAL
#define insert_prio(data, frame, frame_info)                                                                \
{                                                                                                           \
    if (!(data->config.voe && (data->config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) && (data->config.direction == VTSS_MEP_SUPP_DOWN))) \
        frame[12+2] = (frame[12+2] & ~0xE0) | (frame_info->pcp << 5);                                       \
}
#else
#define insert_prio(data, frame, frame_info)                                                                \
{                                                                                                           \
    if (data->config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_COUTVID | VTSS_MEP_SUPP_FLOW_MASK_SOUTVID))       \
        frame[12+2] = (frame[12+2] & ~0xE0) | (frame_info->pcp << 5);                                       \
    if (data->config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_CINVID | VTSS_MEP_SUPP_FLOW_MASK_SINVID))         \
        frame[12+4+2] = (frame[12+4+2] & ~0xE0) | (frame_info->pcp << 5);                                   \
}
#endif

/*lint -e{454, 455, 456} ... The mutex is locked so it is ok to unlock */
static void lmr_to_peer(u32 instance,   supp_instance_data_t *data,   u8 smac[],   u8 lmm[],   vtss_mep_supp_frame_info_t *frame_info,   u32 port)
{
    int                             rc=TRUE;
    u32                             rx_frames=0, tx_frames=0;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    if (data->tx_lmr_ongoing)   return;

    /* Generate LMR frame */
    /* DMAC */
    memcpy(data->tx_lmr, smac, VTSS_MEP_SUPP_MAC_LENGTH);
    /* Prio */
    insert_prio(data, data->tx_lmr, frame_info)
    /* PDU */
    memcpy(&data->tx_lmr[data->tx_header_size], lmm, LMM_PDU_LENGTH);
    data->tx_lmr[data->tx_header_size+1] = OAM_TYPE_LMR;
    /* Counters */
    if (!data->config.voe) { /* This is not a VOE based instance */
        vtss_mep_supp_crit_unlock();
        vtss_mep_supp_counters_get(instance, &rx_frames, &tx_frames);
        vtss_mep_supp_crit_lock();
    }
    else {  /* This is a VOE based instance - Up-MEP only */
#ifdef VTSS_ARCH_SERVAL
        u32 mip;
        vtss_oam_voe_counter_t voe_counter;
        (void)vtss_oam_voe_counter_get(NULL, data->voe_idx, &voe_counter);
        if ((frame_info->oam_info>>27) != voe_counter.lm.up_mep.rx_lmm_sample_seq_no) {
            vtss_mep_supp_trace("lmr_to_peer: Unexpected LMM LM sequence number", instance, frame_info->oam_info>>27, voe_counter.lm.up_mep.rx_lmm_sample_seq_no, 0);
            return;
        }
        rx_frames = voe_counter.lm.up_mep.lmm;
        if (up_mip_present(data->config.flow.vid, &mip)) { /* This is a VOE based Up-MEP with a Up-MIP behind. Any LBM or LTM received by this MIP has to be added to 'rx_frames' */
            data->is2_rx_cnt += instance_data[mip].is2_rx_cnt;
            rx_frames = rx_frames + data->is2_rx_cnt;
            instance_data[mip].is2_rx_cnt = 0;
        }
        tx_frames = 1;   /* I think this must be != 0 in order to be overwritten by VOE */
#endif
    }

    var_to_string(&data->tx_lmr[data->tx_header_size+8], rx_frames);                    /* Rx counter in LMR */
    var_to_string(&data->tx_lmr[data->tx_header_size+12], tx_frames);                   /* Tx counter in LMR */

    tx_frame_info.bypass = !evc_vlan_up(data);
    tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
    tx_frame_info.isdx = data->tx_isdx;
    tx_frame_info.vid = data->config.flow.vid;
    tx_frame_info.vid_inj = vlan_down(data);
    tx_frame_info.qos = frame_info->qos;
    tx_frame_info.pcp = frame_info->pcp;
    tx_frame_info.dp = frame_info->dei;

    rc = vtss_mep_supp_packet_tx_1(instance, &data->done_lmr, port, (data->tx_header_size + LMR_PDU_LENGTH), data->tx_lmr, &tx_frame_info, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LMR : VTSS_MEP_SUPP_OAM_TYPE_NONE));
    if (!rc)       vtss_mep_supp_trace("lmr_to_peer: packet tx failed", 0, 0, 0, 0);
    if (rc)     data->tx_lmr_ongoing = TRUE;
}

static void ltr_to_peer(u32 instance,   supp_instance_data_t *data,   u8 ltm[],   vtss_mep_supp_frame_info_t *frame_info,   u32 port,   BOOL forward,   BOOL mep,  u32 action)
{
    int                             rc;
    u8                              *tx_pdu;
    u32                             header_size;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    /* Generate LTR frame */
    if ((data->tx_ltr == NULL) && (data->tx_ltr = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + LTR_PDU_LENGTH)))
    {/* LTR is ready and a tx buffer has been allocated */
        /* Init of LTR transmitter frame */
        header_size = init_tx_frame(instance, &ltm[9], data->config.mac, frame_info->pcp, frame_info->dei, data->tx_ltr, FALSE);
        /* Prio */
        insert_prio(data, data->tx_ltr, frame_info)
        /* PDU */
        tx_pdu = &data->tx_ltr[header_size];
        tx_pdu[0] = ltm[0];
        tx_pdu[1] = OAM_TYPE_LTR;
        tx_pdu[2] = (ltm[2] & 0x80) | ((mep) ? 0x20 : 0x00) | ((forward) ? 0x40 : 0x00);
        tx_pdu[3] = 6;                     
        memcpy(&tx_pdu[4], &ltm[4], 4);    /* Transaction ID */
        tx_pdu[8] = ltm[8]-1;              /* Decrement TTL */
        tx_pdu[9] = action;                /* Relay action */
        tx_pdu[10] = 8;
        tx_pdu[11] = 0;
        tx_pdu[12] = 16;
        memcpy(&tx_pdu[13], &ltm[24], 8);  /* Last Egress Identifier */
        tx_pdu[21] = 0;
        tx_pdu[22] = 0;
        memcpy(&tx_pdu[23], data->config.mac, VTSS_MEP_SUPP_MAC_LENGTH);  /* Next Egress Identifier */
        tx_pdu[29] = (data->config.direction == VTSS_MEP_SUPP_DOWN) ? 5 : 6; /* down/up reply */
        tx_pdu[30] = 0;
        tx_pdu[31] = 7;
        tx_pdu[32] = 1;     /* down/up action is OK */
        memcpy(&tx_pdu[33], data->config.mac, VTSS_MEP_SUPP_MAC_LENGTH);  /* down/up Identifier */
        tx_pdu[39] = 0;

        tx_frame_info.bypass = !evc_vlan_up(data);
        tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
        tx_frame_info.isdx = data->tx_isdx;
        tx_frame_info.vid = data->config.flow.vid;
        tx_frame_info.vid_inj = vlan_down(data);
        tx_frame_info.qos = frame_info->qos;
        tx_frame_info.pcp = frame_info->pcp;
        tx_frame_info.dp = frame_info->dei;

        rc = vtss_mep_supp_packet_tx_1(instance, &data->done_ltr, port, (header_size + LTR_PDU_LENGTH), data->tx_ltr, &tx_frame_info, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LTR : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        if (!rc)       vtss_mep_supp_trace("ltr_to_peer: packet tx failed", 0, 0, 0, 0);
    }
}

static void lbr_to_peer(u32 instance,   supp_instance_data_t *data,   u8 smac[],   u8 lbm[],   u32 lbm_len,   vtss_mep_supp_frame_info_t *frame_info,   u32 port)
{
    int                             rc;
    u32                             header_size;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    /* Generate LBR frame */
    if (!data->tx_lbr_ongoing && ((data->tx_lbr != NULL) || (data->tx_lbr = vtss_mep_supp_packet_tx_alloc((data->tx_header_size + lbm_len)))))
    {/* LBR is ready and a tx buffer is/has been allocated */
        /* Init of LRR transmitter frame */
        header_size = init_tx_frame(instance, smac, data->config.mac, frame_info->pcp, frame_info->dei, data->tx_lbr, FALSE);
        /* Prio */
        insert_prio(data, data->tx_lbr, frame_info)
        /* PDU */
        memcpy(&data->tx_lbr[header_size], lbm, lbm_len);
        data->tx_lbr[header_size+1] = OAM_TYPE_LBR;

        tx_frame_info.bypass = !evc_vlan_up(data);
        tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
        tx_frame_info.isdx = data->tx_isdx;
        tx_frame_info.vid = data->config.flow.vid;
        tx_frame_info.vid_inj = vlan_down(data);
        tx_frame_info.qos = frame_info->qos;
        tx_frame_info.pcp = frame_info->pcp;
        tx_frame_info.dp = frame_info->dei;

        rc = vtss_mep_supp_packet_tx_1(instance, &data->done_lbr, port, (header_size + lbm_len), data->tx_lbr, &tx_frame_info, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LBR : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        if (!rc)       vtss_mep_supp_trace("lbr_to_peer: packet tx failed", 0, 0, 0, 0);
        if (rc)     data->tx_lbr_ongoing = TRUE;

        data->rx_lbm_timer = 2000/timer_res;   /* If no LBM is received in two sec. then free the TX buffer */
        vtss_mep_supp_timer_start();
    }
}

static void ltm_to_forward(u32 instance,   supp_instance_data_t *data,   u8 dmac[],   u8 ltm[],   vtss_mep_supp_frame_info_t *frame_info,   u32 port)
{
    int                             rc;
    u8                              *tx_pdu;
    u32                             header_size;
    vtss_mep_supp_tx_frame_info_t   tx_frame_info;

    if ((data->tx_ltm == NULL) && (data->tx_ltm = vtss_mep_supp_packet_tx_alloc(HEADER_MAX_SIZE + LTM_PDU_LENGTH)))
    {/* LTM forwarding is ongoing atm. and a tx buffer has been allocated */
        /* Init of LTM transmitter frame */
        header_size = init_tx_frame(instance, dmac, data->config.mac, frame_info->pcp, frame_info->dei, data->tx_ltm, FALSE);
        /* Prio */
        insert_prio(data, data->tx_ltm, frame_info)
        /* PDU */
        memcpy(&data->tx_ltm[header_size], ltm, LTM_PDU_LENGTH);
        tx_pdu = &data->tx_ltm[data->tx_header_size];
        tx_pdu[8] -= 1;             /* Decrement TTL */
        tx_pdu[21] = 7;             /* Egress identifier */
        tx_pdu[22] = 0;
        tx_pdu[23] = 8;
        tx_pdu[24] = 0;
        tx_pdu[25] = 0;
        memcpy(&tx_pdu[26], data->config.mac, VTSS_MEP_SUPP_MAC_LENGTH);
        tx_pdu[32] = 0;

#ifdef VTSS_ARCH_SERVAL
        tx_frame_info.bypass = evc_vlan_up(data);
        tx_frame_info.maskerade_port = (!evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
#else
        tx_frame_info.bypass = TRUE;
        tx_frame_info.maskerade_port = VTSS_PORT_NO_NONE;
#endif
        tx_frame_info.vid = data->config.flow.vid;
        tx_frame_info.vid_inj = FALSE;
        tx_frame_info.isdx = data->tx_isdx;
        tx_frame_info.qos = frame_info->qos;
        tx_frame_info.pcp = frame_info->pcp;
        tx_frame_info.dp = frame_info->dei;

        rc = vtss_mep_supp_packet_tx_1(instance, &data->done_ltm, port, (header_size + LTM_PDU_LENGTH), data->tx_ltm, &tx_frame_info, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_LTM : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        if (!rc)       vtss_mep_supp_trace("ltm_to_forward: packet tx failed", 0, 0, 0, 0);
    }
}

static void dmr_to_peer(u32 instance,   supp_instance_data_t *data,   u8 smac[],   u8 dmm[],   vtss_mep_supp_frame_info_t *frame_info,   u32 port)
{
    int                           rc;
    vtss_timestamp_t              timestamp;
    u32                           tc = 0;
    vtss_mep_supp_tx_frame_info_t tx_frame_info;

    if (data->tx_dmr_ongoing)   return;

    /* Generate DMR frame */
    /* DMAC */
    memcpy(data->tx_dmr, smac, VTSS_MEP_SUPP_MAC_LENGTH);
    /* Prio */
    insert_prio(data, data->tx_dmr, frame_info)
    /* PDU */
    memcpy(&data->tx_dmr[data->tx_header_size], dmm, 12);
    data->tx_dmr[data->tx_header_size+1] = OAM_TYPE_DMR;

    /* RxTimeStampf */
    var_to_string(&data->tx_dmr[data->tx_header_size+12], data->rx_dmm_timestamp.seconds);                    
    var_to_string(&data->tx_dmr[data->tx_header_size+16], data->rx_dmm_timestamp.nanoseconds);                  

    /* TxTimeStampb */
    vtss_mep_supp_timestamp_get(&timestamp, &tc);
    var_to_string(&data->tx_dmr[data->tx_header_size+20], timestamp.seconds);                   
    var_to_string(&data->tx_dmr[data->tx_header_size+24], timestamp.nanoseconds);                   
    
    tx_frame_info.bypass = !evc_vlan_up(data);
    tx_frame_info.maskerade_port = (evc_vlan_up(data)) ? data->config.port : VTSS_PORT_NO_NONE;
    tx_frame_info.isdx = data->tx_isdx;
    tx_frame_info.vid = data->config.flow.vid;
    tx_frame_info.vid_inj = vlan_down(data);
    tx_frame_info.qos = frame_info->qos;
    tx_frame_info.pcp = frame_info->pcp;
    tx_frame_info.dp = frame_info->dei;

#ifndef VTSS_ARCH_SERVAL
    if (memcmp(&dmm[DMM_PDU_STD_LENGTH], "VTSS", 4) == 0) {
        /* Need to send a follow-up packet */
        data->done_dmr.instance = instance;
        data->done_dmr.cb = dmr_fup_done;
        if (!vtss_mep_supp_check_hw_timestamp()) 
        {  
            /* no h/w timestamp */     
            rc = vtss_mep_supp_packet_tx_1(instance, &data->done_dmr, port, (data->tx_header_size + DMR_PDU_LENGTH), data->tx_dmr, &tx_frame_info, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_DMR : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        }
        else
        {         
            /* two step */
            rc = vtss_mep_supp_packet_tx_1_two_step_ts(instance, &data->done_dmr, port, (data->tx_header_size + DMR_PDU_LENGTH), data->tx_dmr);
        }     
        data->dmr_fup_port = port;
    }
    else
#endif
    {
        data->done_dmr.instance = instance;
        data->done_dmr.cb = dmr_done;
        if (!vtss_mep_supp_check_hw_timestamp()) {    
            /* no h/w timestamp */   
            rc = vtss_mep_supp_packet_tx_1(instance, &data->done_dmr, port, (data->tx_header_size + DMR_PDU_LENGTH), data->tx_dmr, &tx_frame_info, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_DMR : VTSS_MEP_SUPP_OAM_TYPE_NONE));
        }
        else {    
            /* one step */
#if DEBUG_PHY_TS
            u32 zero = 0;
            var_to_string(&data->tx_dmr[data->tx_header_size+20], zero);
            var_to_string(&data->tx_dmr[data->tx_header_size+24], zero);           
            rc = vtss_mep_supp_packet_tx_1(instance, &data->done_dmr, port, (data->tx_header_size + DMR_PDU_LENGTH), data->tx_dmr, &tx_frame_info, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_DMR : VTSS_MEP_SUPP_OAM_TYPE_NONE));
#else       
            rc = vtss_mep_supp_packet_tx_1_one_step_ts(instance, &data->done_dmr, port, (data->tx_header_size + DMR_PDU_LENGTH), data->tx_dmr, &tx_frame_info, tc, &data->onestep_extra_r, ((data->config.voe) ? VTSS_MEP_SUPP_OAM_TYPE_DMR : VTSS_MEP_SUPP_OAM_TYPE_NONE));
#endif        
        }
    }
    if (!rc)       vtss_mep_supp_trace("dmr_to_peer: packet tx failed", 0, 0, 0, 0);
    if (rc)     data->tx_dmr_ongoing = TRUE;
}

static BOOL find_lowest_mep(u32 port, vtss_mep_supp_direction_t direct, u16 *meps, u32 meps_idx, u32 *mep)
{
    u32  mep_level, i;
    BOOL found = FALSE;

    mep_level = 8;
    for (i=0; i<meps_idx; ++i)      /* Find the MEP with the lowest level on this port */
        if ((instance_data[meps[i]].config.port == port) && (instance_data[meps[i]].config.direction == direct) && (instance_data[meps[i]].config.level < mep_level))
        {
            mep_level = instance_data[meps[i]].config.level;
            *mep = meps[i];
            found = TRUE;
        }
    return(found);
}

static u32 reply_port_calc(supp_instance_data_t *data,  u32 port)
{
    u32 i;

    if (!data->config.flow.port[port])      /* A PDU is received on a port that is not a flow port - this is possible in case of Up-MEP according to DS1076 */
    {
        for (i=0; i<VTSS_PORT_ARRAY_SIZE; ++i)  if (data->config.flow.port[i])  break;     /* This is assuming that this Up-Mep has one flow port only */
        return(i);
    }
    else
        return (port);
}

void vtss_mep_supp_rx_frame(u32 port,  vtss_mep_supp_frame_info_t *frame_info,  u8 frame[],  u32 len, vtss_timestamp_t *rx_time, u32 rx_count)
{
/* This is called by API when a OAM frame is received */
    supp_instance_data_t *data;
    vtss_mep_supp_ltr_t  *ltr;
    BOOL                 check_lm, new_lbr, new_dmr, new_ltr, new_aps, new_dm1, new_defect, new_state, new_tst;
    u8                   *pdu;
    u32                  mep, mep_lm, near_CT2, near_CR2, far_CT2, far_CR2, priority;
    u32                  tag_idx, tl_idx, tag_cnt, f_port;
    u32                  level, meps_idx, mips_idx, i, tx_count, tst_frame_size, rx_tag_cnt;
    i32                  dm_delay, total_delay, device_delay;
    vtss_timestamp_t     rxTimef, txTimeb;
    BOOL                 egress_ports[VTSS_PORT_ARRAY_SIZE];
    BOOL                 is_ns;

    #define MEPS_MIPS_MAX      20
    u16                        meps[MEPS_MIPS_MAX], mipss[MEPS_MIPS_MAX];

    vtss_mep_supp_crit_lock();

    new_lbr = new_ltr = new_aps = new_dmr = new_dm1 = new_defect = new_state = new_tst = FALSE;
    pdu = frame;
    mep = tst_frame_size = 0;
    priority = (frame_info->tagged) ? frame_info->pcp : 0xFFFFFFFF;

    /* Calculate number of tags */
    tag_idx = 12;
    if (c_s_tag(&frame[tag_idx]))    tag_cnt = ((c_s_tag(&frame[tag_idx+4])) ? 2 : 1);
    else                             tag_cnt = 0;

    /* Check for type of frame - has to be OAM */
    tl_idx = tag_idx+(tag_cnt*4);
    if ((frame[tl_idx] != 0x89) || (frame[tl_idx+1] != 0x02))     goto unlock;

    pdu = &frame[tl_idx + 2];
    level = pdu[0]>>5;
    rx_tag_cnt = tag_cnt + ((frame_info->tagged) ? 1 : 0);

//printf("vtss_mep_supp_rx_frame  port %u  vid %u  isdx %u  tl_idx %u  level %u  rx_tag_cnt %u  pdu[1+8+9] %X-%X-%X  sec %u sec %u nano %u\n", port, frame_info->vid, frame_info->isdx, tl_idx, level, rx_tag_cnt, pdu[1], pdu[8], pdu[9], rx_time->sec_msb, rx_time->seconds, rx_time->nanoseconds);

    meps_idx = 0;
    memset(meps, 0, sizeof(meps));
    mips_idx = 0;
    mep = 0xFFFFFFFF;
    memset(mipss, 0, sizeof(mipss));
    memset(egress_ports, 1, sizeof(egress_ports));

    /* Find all MEP/MIP in this flow */
    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; ++i)
    {   /* Find the MEP with the received frame header as expected */
        data = &instance_data[i];    /* Instance data reference */
        /* Check if this MEP/MIP is configured with this expected header */
        if (!data->config.enable)                                                                  continue;         /* Check if enabled */
#ifdef VTSS_ARCH_SERVAL
        if (data->config.voe && (data->rx_isdx == frame_info->isdx))                               mep = i;          /* VOE based MEP is found */
#endif
        if (data->rx_tag_cnt != rx_tag_cnt)                                                        continue;         /* Check number of tags */
        if (data->config.level < level)                                                            continue;         /* Check level */
        if (data->config.flow.vid != frame_info->vid)                                              continue;         /* Check classified VID */
#ifdef VTSS_ARCH_SERVAL
        if ((data->config.mode == VTSS_MEP_SUPP_MIP) && (data->config.flow.port[port]))            mep = i;          /* Subscriber MIP is found */
#endif
        if ((data->config.mode == VTSS_MEP_SUPP_MEP) && (meps_idx < MEPS_MIPS_MAX))                                    meps[meps_idx++] = i;  /* All MEP are saved for later analyze */
        if ((data->config.mode == VTSS_MEP_SUPP_MIP) && (data->config.flow.port[port]) && (mips_idx < MEPS_MIPS_MAX))  mipss[mips_idx++] = i; /* All MIP are saved for later analyze */
    }

    /* Check for discard */
    if ((mips_idx == 0) && (meps_idx == 0))   goto unlock;     /* No MEP or MIP was found */
    if (mep == 0xFFFFFFFF) { /* this frame is not related to a VOE based MEP or a Serval Subscriber MIP */
        if (pdu[1] != OAM_TYPE_CCM)
        {/* This is not a CCM - check for discard */
            for (i=0; i<meps_idx; ++i)  /* search for Ingress MEP on this level on this port */
                if ((instance_data[meps[i]].config.direction == VTSS_MEP_SUPP_DOWN) && (instance_data[meps[i]].config.port == port) && (instance_data[meps[i]].config.level == level))
                    {mep = meps[i]; break;}
            if (mep == 0xFFFFFFFF)
            {/* Ingress MEP on this level was not found - search for ingress/egress MEP on this port,level or higher to discard */
                for (i=0; i<meps_idx; ++i)    if ((instance_data[meps[i]].config.port == port) && (instance_data[meps[i]].config.level >= level))    goto unlock;
                if (pdu[1] != OAM_TYPE_LTM)
                {/* This is not a LTM - find target MEP/MIP */
                    if ((pdu[1] == OAM_TYPE_LBM) && !(frame[0] & 0x01))
                    {/* This is a unicast LBM PDU - search for MIP with this MAC */
                        for (i=0; i<mips_idx; ++i)
                            if ((instance_data[mipss[i]].config.level == level) && !memcmp(&frame[0], instance_data[mipss[i]].config.mac, VTSS_MEP_SUPP_MAC_LENGTH))   {mep = mipss[i]; break;}
                    }
                    if (mep == 0xFFFFFFFF) /* No MEP/MIP was found - search for egress MEP on this level */
                        for (i=0; i<meps_idx; ++i)    if ((instance_data[meps[i]].config.direction == VTSS_MEP_SUPP_UP) && (instance_data[meps[i]].config.level == level)) {mep = meps[i]; break;}
                    if (mep == 0xFFFFFFFF)  goto unlock;
                }
            }
            if (mep != 0xFFFFFFFF)  data = &instance_data[mep];    /* Instance data reference - not used in case of CCM and LTM */
        }
    }
    else { /* This frame was related to a VOE based MEP */
#ifdef VTSS_ARCH_SERVAL
        data = &instance_data[mep];    /* Instance data reference */
        if (mips_idx)   /* If Subscriber MIP is found then any MEP found is not relevant */
            meps_idx = 0;
#endif
    }


    /* No reason was found to discard this PDU - analyze */
    switch (pdu[1])
    {
        case OAM_TYPE_CCM:
            if (meps_idx == 0)     goto unlock;
            check_lm = FALSE;
            mep_lm = 0;

            if ((mep != 0xFFFFFFFF) || find_lowest_mep(port, VTSS_MEP_SUPP_DOWN, meps, meps_idx, &mep)) { /* Either a VOE based MEP was found or the ingress MEP with the lowest level on this port was found */
                check_lm = !ccm_defect_calc(mep, &instance_data[mep], &frame[VTSS_MEP_SUPP_MAC_LENGTH], pdu, priority, &new_defect, &new_state);
                mep_lm = mep;
                if (check_lm)   instance_data[mep].ccm_state.state.valid_counter++;
                else            instance_data[mep].ccm_state.state.invalid_counter++;
            }
            else
            { /* No ingress MEP was found on this port */
                for (i=0; i<meps_idx; ++i)    if ((instance_data[meps[i]].config.port == port) && (instance_data[meps[i]].config.level >= level))    goto unlock; /* Discard if egress MEP on this level or higher on this port */
                egress_ports[port] = FALSE;     /* Don't check for egress on this port */
                for (i=0; i<meps_idx; ++i) { /* Find all Egress MEP on this level or higher */
                    if ((instance_data[meps[i]].config.direction == VTSS_MEP_SUPP_UP) && egress_ports[instance_data[meps[i]].config.port]) { /* Egress on a port not already checked */
                        egress_ports[instance_data[meps[i]].config.port] = FALSE;   /* Now this port is checked */
                        if (find_lowest_mep(instance_data[meps[i]].config.port, VTSS_MEP_SUPP_UP, meps, meps_idx, &mep)) { /* The egress MEP with the lowest level on this port was found */
                            if (!ccm_defect_calc(mep, &instance_data[mep], &frame[VTSS_MEP_SUPP_MAC_LENGTH], pdu, priority, &new_defect, &new_state)) {
                                check_lm = TRUE;  /* No defects and on right level - check for LM */
                                mep_lm = mep;
                                instance_data[mep].ccm_state.state.valid_counter++;
                            }
                            else
                                instance_data[mep].ccm_state.state.invalid_counter++;
                        }
                    }
                }
            }
            if (!check_lm)      break;
            if (!instance_data[mep_lm].ccm_gen.enable || !instance_data[mep_lm].ccm_gen.lm_enable)        break;  /* CCM based LM is not enabled */
            if (!memcmp(lm_null_counter, &pdu[58], 12))                  break;  /* No counters in this CCM */

            instance_data[mep_lm].ccm_lm_counter.rx_counter++;

            if (!instance_data[mep_lm].config.voe) { /* This is not a VOE based instance */
                vtss_mep_supp_crit_unlock();    /* Get RX frame counter from platform */
                vtss_mep_supp_counters_get(mep_lm, &rx_count, &tx_count);
                vtss_mep_supp_crit_lock();
            }
            else {  /* This is a VOE based instance */
#ifdef VTSS_ARCH_SERVAL
                vtss_oam_voe_counter_t     voe_counter;
                if (instance_data[mep_lm].config.direction == VTSS_MEP_SUPP_UP) { /* In case of UP-MEP the RX frame counter is fetched in the VOE */
                    (void)vtss_oam_voe_counter_get(NULL, instance_data[mep_lm].voe_idx, &voe_counter);
                    if (frame_info->oam_info>>27 != voe_counter.lm.up_mep.rx_ccm_lm_sample_seq_no) {
                        vtss_mep_supp_trace("vtss_mep_supp_rx_frame CCM: Unexpected CCM LM sequence number", mep_lm, frame_info->oam_info>>27, voe_counter.lm.up_mep.rx_ccm_lm_sample_seq_no, 0);
                        break;
                    }
                    rx_count = voe_counter.lm.up_mep.ccm_lm;
                }
#endif
            }

            if (instance_data[mep_lm].tx_ccm_sw) {
                var_to_string(&instance_data[mep_lm].tx_ccm_sw[instance_data[mep_lm].tx_header_size + 62], rx_count);   /* Rx counter back to far end */
                memcpy(&instance_data[mep_lm].tx_ccm_sw[instance_data[mep_lm].tx_header_size + 66], &pdu[58], 4);        /* far end TX counter back to far end */
            }

            near_CT2 = string_to_var(&pdu[66]);          /* Calculate LM information */
            far_CT2 = string_to_var(&pdu[58]);
            near_CR2 = rx_count;
            far_CR2 = string_to_var(&pdu[62]);

            lm_counter_calc(&(instance_data[mep_lm].ccm_lm_counter),  &(instance_data[mep_lm].lm_start),  near_CT2,  far_CT2,  near_CR2,  far_CR2);   /* Calculate LM counters */
            break;

        case OAM_TYPE_LMM:
            if (level != data->config.level)          break;  /* Wrong level */
            lmr_to_peer(mep, data, &frame[VTSS_MEP_SUPP_MAC_LENGTH], pdu, frame_info, reply_port_calc(data, port));
            break;

        case OAM_TYPE_LMR:
            if (level != data->config.level)          break;  /* Wrong level */
            if (!data->lmm_config.enable)             break;  /* LMM is not enabled */

            data->lmm_lm_counter.rx_counter++;
            if (!data->config.voe) { /* This is not a VOE based instance */
                vtss_mep_supp_crit_unlock();
                vtss_mep_supp_counters_get(mep, &rx_count, &tx_count);
                vtss_mep_supp_crit_lock();
            }
            else {  /* This is a VOE based instance */
#ifdef VTSS_ARCH_SERVAL
                vtss_oam_voe_counter_t     voe_counter;
                if (data->config.direction == VTSS_MEP_SUPP_UP) { /* In case of UP-MEP the RX frame counter is fetched in the VOE */
                    (void)vtss_oam_voe_counter_get(NULL, data->voe_idx, &voe_counter);
                    if (frame_info->oam_info>>27 != voe_counter.lm.up_mep.rx_lmr_sample_seq_no) {
                        vtss_mep_supp_trace("vtss_mep_supp_rx_frame LMR: Unexpected LMR LM sequence number", mep, frame_info->oam_info>>27, voe_counter.lm.up_mep.rx_lmr_sample_seq_no, 0);
                        break;
                    }
                    rx_count = voe_counter.lm.up_mep.lmr;
                }
#endif
            }

            near_CT2 = string_to_var(&pdu[4]);          /* Calculate LM information */
            far_CT2 = string_to_var(&pdu[12]);
            near_CR2 = rx_count;
            far_CR2 = string_to_var(&pdu[8]);
            lm_counter_calc(&data->lmm_lm_counter,  &data->lm_start,  near_CT2,  far_CT2,  near_CR2,  far_CR2);   /* Calculate LM counters */
            break;

        case OAM_TYPE_LAPS:
        case OAM_TYPE_RAPS:
            if (level != data->config.level)          break;  /* Wrong level */
            if (!data->aps_config.enable)             break;  /* APS is not enabled */

            data->rx_aps_timer = data->aps_timer_val*3;

            new_aps = TRUE;
            break;

        case OAM_TYPE_LTM:
            if (pdu[8] == 0)                          break;  /* TTL is "dead" */
            if (pdu[21] != 7)                         break;  /* No Egress identifier present */

            if (meps_idx != 0)
            {   /* One or more MEP's got this expected frame header */
                for (i=0; i<meps_idx; ++i)
                    if ((instance_data[meps[i]].config.level == level) && (instance_data[meps[i]].config.port == port) && (instance_data[meps[i]].config.direction == VTSS_MEP_SUPP_DOWN))
                        break;
                if (i < meps_idx)
                {/* Ingress MEP on this level,port was found */
                    instance_data[meps[i]].is2_rx_cnt++;
                    if (!memcmp(&pdu[15], instance_data[meps[i]].config.mac, VTSS_MEP_SUPP_MAC_LENGTH)) {/* Reply if this is the target MAC */
                        ltr_to_peer(meps[i], &instance_data[meps[i]], pdu, frame_info, reply_port_calc(&instance_data[meps[i]], port), FALSE, TRUE, 1);
                        break;
                    }
                    if (vtss_mep_supp_check_forwarding(port, &f_port, &pdu[15], frame_info->vid)) {/* Reply if forwarding is possible */
                        ltr_to_peer(meps[i], &instance_data[meps[i]], pdu, frame_info, reply_port_calc(&instance_data[meps[i]], port), FALSE, TRUE, 2);
                        break;
                    }
                    break;
                }
                for (i=0; i<meps_idx; ++i)
                    if ((instance_data[meps[i]].config.level == level) && (instance_data[meps[i]].config.direction == VTSS_MEP_SUPP_UP))
                        break;
                if (i < meps_idx)
                {/* Egress MEP on this level was found */
                    instance_data[meps[i]].is2_rx_cnt++;
                    if (!memcmp(&pdu[15], instance_data[meps[i]].config.mac, VTSS_MEP_SUPP_MAC_LENGTH)) {/* Reply if this is the target MAC */
                        ltr_to_peer(meps[i], &instance_data[meps[i]], pdu, frame_info, reply_port_calc(&instance_data[meps[i]], port), FALSE, TRUE, 1);
                        break;
                    }
                    if (vtss_mep_supp_check_forwarding(port, &f_port, &pdu[15], frame_info->vid)) /* Reply if forwarding is possible on residence port */
                        if (instance_data[meps[i]].config.port == f_port) {
                            ltr_to_peer(meps[i], &instance_data[meps[i]], pdu, frame_info, reply_port_calc(&instance_data[meps[i]], port), FALSE, TRUE, 2);
                            break;
                        }
                }
                if (vtss_mep_supp_check_forwarding(port, &f_port, &pdu[15], frame_info->vid))
                    for (i=0; i<meps_idx; ++i)  /* Search for ingress/egress MEP on this level or higher, on forwarding port - must not leak */
                        if ((instance_data[meps[i]].config.port == f_port) && (instance_data[meps[i]].config.level >= level))
                            goto unlock;
            }

            /* No terminating ingress/egress MEP was found */
            if (mips_idx != 0)
            {   /* One or more MIP's got this expected frame header */
                for (i=0; i<mips_idx; ++i) /* Check for MIP with target MAC */
                    if ((instance_data[mipss[i]].config.level == level) && !memcmp(&pdu[15], instance_data[mipss[i]].config.mac, VTSS_MEP_SUPP_MAC_LENGTH)) {/* Reply if this is the target MAC */
                        instance_data[mipss[i]].is2_rx_cnt++;
                        ltr_to_peer(mipss[i], &instance_data[mipss[i]], pdu, frame_info, port, FALSE, FALSE, 1);
                        break;
                    }
                if (i < mips_idx) break;  /* MIP with target MAC was found - done */

                /* No MIP with target MAC was found */
                if (vtss_mep_supp_check_forwarding(port, &f_port, &pdu[15], frame_info->vid))
                {/* Forwarding is possible - means that we will reply but not necessarily forward (depending on TTL) */
                    for (i=0; i<mips_idx; ++i)    /* Check if an egress MIP on forwarding port exists */
                        if ((instance_data[mipss[i]].config.level == level) && (instance_data[mipss[i]].config.direction == VTSS_MEP_SUPP_UP) && (instance_data[mipss[i]].config.port == f_port))
                            break;
                    if (i < mips_idx)
                    {/* Egress MIP was found on forwarding port */
                        instance_data[mipss[i]].is2_rx_cnt++;
                        ltr_to_peer(mipss[i], &instance_data[mipss[i]], pdu, frame_info, port, (pdu[8] > 1), FALSE, 2);
                        if (pdu[8] > 1)    ltm_to_forward(mipss[i], &instance_data[mipss[i]], frame, pdu, frame_info, f_port);  /* Don't forwared with a 'zero' TTL */
                        break;
                    }
                    /* No egress MIP was found */
                    for (i=0; i<mips_idx; ++i)    /* Check if an ingress MIP exists */
                        if ((instance_data[mipss[i]].config.level == level) && instance_data[mipss[i]].config.direction == VTSS_MEP_SUPP_DOWN)
                            break;
                    if (i < mips_idx)
                    {/* Ingress MIP was found */
                        instance_data[mipss[i]].is2_rx_cnt++;
                        ltr_to_peer(mipss[i], &instance_data[mipss[i]], pdu, frame_info, port, (pdu[8] > 1), FALSE, 2);
                        if (pdu[8] > 1)
                            ltm_to_forward(mipss[i], &instance_data[mipss[i]], frame, pdu, frame_info, f_port);  /* Don't forwared with a 'zero' TTL */
                    }
                }
            }

            break;

        case OAM_TYPE_LTR:
            if (level != data->config.level)      break;  /* Wrong level */
            if (!data->ltm_config.enable)         break;  /* LTM is not enabled */

            if (data->ltr_cnt < VTSS_MEP_SUPP_LTR_MAX)
            {
                ltr = &data->ltr[data->ltr_cnt];

                ltr->mode = (pdu[2] & 0x20) ? VTSS_MEP_SUPP_MEP : VTSS_MEP_SUPP_MIP;
                ltr->forwarded = (pdu[2] & 0x40);
                ltr->transaction_id = string_to_var(&pdu[4]);
                ltr->ttl = pdu[8];
                ltr->relay_action = (pdu[9] == 1) ? VTSS_MEP_SUPP_RELAY_HIT : (pdu[9] == 2) ? VTSS_MEP_SUPP_RELAY_FDB : (pdu[9] == 3) ? VTSS_MEP_SUPP_RELAY_MFDB : VTSS_MEP_SUPP_RELAY_UNKNOWN;
                memcpy(ltr->last_egress_mac, &pdu[15], VTSS_MEP_SUPP_MAC_LENGTH);
                memcpy(ltr->next_egress_mac, &pdu[23], VTSS_MEP_SUPP_MAC_LENGTH);
                ltr->direction = (pdu[29] == 5) ? VTSS_MEP_SUPP_DOWN : VTSS_MEP_SUPP_UP;

                data->ltr_cnt++;

                new_ltr = TRUE;
            }
            break;

        case OAM_TYPE_LBM:
            if (level != data->config.level)      break;  /* Wrong level */
            if (!(frame[0] & 0x01) && memcmp(&frame[0], data->config.mac, VTSS_MEP_SUPP_MAC_LENGTH))      break;  /* Wrong unicast MAC */

            data->is2_rx_cnt++;

            lbr_to_peer(mep, data, &frame[VTSS_MEP_SUPP_MAC_LENGTH], pdu, len-(tl_idx + 2), frame_info, reply_port_calc(data, port));
            break;

        case OAM_TYPE_LBR:
            if (level != data->config.level)      break;  /* Wrong level */
            if (!data->lbm_config.enable)         break;  /* LBM is not enabled */

            data->lb_state.lbr_counter++;
            if (data->lbr_cnt < VTSS_MEP_SUPP_LBR_MAX)
            {
                data->lbr[data->lbr_cnt].transaction_id = string_to_var(&pdu[4]);
                memcpy(data->lbr[data->lbr_cnt].mac, &frame[VTSS_MEP_SUPP_MAC_LENGTH], VTSS_MEP_SUPP_MAC_LENGTH);
                data->lbr_cnt++;
                new_lbr = TRUE;
            }
            break;

        case OAM_TYPE_DMR:
            if (level != data->config.level)                                        break;  /* Wrong level */
            if (!data->dmm_config.enable)                                           break;  /* DMM is not enabled */

            /* Make sure the DMR is expected */
            if (!data->dmm_config.proprietary && data->dmm_config.calcway == VTSS_MEP_SUPP_FLOW) {
                data->rxTimef.seconds = string_to_var(&pdu[12]);
                data->rxTimef.nanoseconds = string_to_var(&pdu[16]);
                data->txTimeb.seconds = string_to_var(&pdu[20]);    
                data->txTimeb.nanoseconds = string_to_var(&pdu[24]);
                data->dmr_dev_delay = vtss_mep_supp_delay_calc(&data->txTimeb, &data->rxTimef, get_dm_tunit(data));
            }
            else {
                data->dmr_dev_delay = 0;
            }

            data->rx_dmr_timestamp = *rx_time;          /* Save RX time stamp */
            
            if (!vtss_mep_supp_check_hw_timestamp())
            {    
                /* Check if the TX time is calculated */
                if (data->tx_dmm_fup_timestamp.seconds == 0 && data->tx_dmm_fup_timestamp.nanoseconds == 0) {
                    /* For some reasons, the DMR packet receiving is earlier than the DMM tx done callback. Save the related information for calculating the delay when tx time available. */
                    /* The reason could be the priority of the thread to receive packets is higher than that of the thread for tx done call back. */  
                    vtss_mep_supp_trace("Real tx time is not returned yet", 0, 0, 0, 0);
                    data->dmm_late_txtime_cnt++;
                    data->dmr_received = 1;
                    break; /* Real tx time is not returned yet */
                }   
            }
            else
            {
                /* Get tx time from packets. To simplify code to handle normal/fup dmr, always use fup timestamp to calculate DM. Therefore copy tx_time on packets to fup_timestamp */
                data->tx_dmm_fup_timestamp.seconds = string_to_var(&pdu[4]);
                data->tx_dmm_fup_timestamp.nanoseconds = string_to_var(&pdu[8]);      
                data->dmr_ts_ok = TRUE;
            }                    

            new_dmr = handle_dmr(data);
#if F_2DM_FOR_1DM
            if (data->dmm_config.syncronized == TRUE) {            
                if (data->dmm_config.proprietary && data->dmm_config.calcway == VTSS_MEP_SUPP_FLOW) {
                    /* One-way follow-up. Save the rx timestamp for calculation when follow-up packet arrives */
                    data->rx_dm1_timestamp = *rx_time;
                    break;
                }    
                else {
                    /* One-way Standard (far-to-near)*/
                    txTimeb.seconds = string_to_var(&pdu[20]);    
                    txTimeb.nanoseconds = string_to_var(&pdu[24]);
                    dm_delay = vtss_mep_supp_delay_calc(rx_time, &txTimeb, get_dm_tunit(data));

                    if (dm_delay >= 0 && dm_delay < 1e9) {   /* Check for valid delay */
                        data->dm1_far_to_near[data->dm1_rx_cnt_far_to_near % VTSS_MEP_SUPP_DM_MAX] = dm_delay;
                        data->dm1_rx_cnt_far_to_near++;
                    } 
                    else {  /* If dm_delay <= 0 or above 1 sec, it means something wrong. Just skip it. */
                        vtss_mep_supp_trace("vtss_mep_supp_rx_frame: dm_delay invalid", 0, 0, 0, 0);
                        data->dm1_rx_err_cnt_far_to_near++;
                    } 

                    /* One-way Standard (near-to-far)*/
                    rxTimef.seconds = string_to_var(&pdu[12]);    
                    rxTimef.nanoseconds = string_to_var(&pdu[16]);
                    dm_delay = vtss_mep_supp_delay_calc(&rxTimef, &data->tx_dmm_fup_timestamp, get_dm_tunit(data));

                    if (dm_delay >= 0 && dm_delay < 1e9) {   /* Check for valid delay */
                        data->dm1_near_to_far[data->dm1_rx_cnt_near_to_far % VTSS_MEP_SUPP_DM_MAX] = dm_delay;
                        data->dm1_rx_cnt_near_to_far++;
                    } 
                    else {
                        vtss_mep_supp_trace("vtss_mep_supp_rx_frame: dm_delay invalid", 0, 0, 0, 0);
                        data->dm1_rx_err_cnt_near_to_far++;
                    }
                }
            }                
#endif
            break;

        case OAM_TYPE_DMR_FUP:
            if (level != data->config.level)                                        break;  /* Wrong level */
            if (!data->dmm_config.enable)                                           break;  /* DMM is not enabled */

            /* Make sure the real tx time is gotten */
            if (data->tx_dmm_fup_timestamp.seconds == 0 && data->tx_dmm_fup_timestamp.nanoseconds == 0) {
                vtss_mep_supp_trace("Real tx time is not returned yet", 0, 0, 0, 0);
                break; /* Real tx time is not returned yet */
            }

            if (data->dmm_config.proprietary && data->dmm_config.calcway == VTSS_MEP_SUPP_FLOW) {
                /* two-way with followup - flow measurement */
                rxTimef.seconds = string_to_var(&pdu[12]);
                rxTimef.nanoseconds = string_to_var(&pdu[16]);
                txTimeb.seconds = string_to_var(&pdu[20]);    
                txTimeb.nanoseconds = string_to_var(&pdu[24]);
                /* the timestamp of followup packets seems not to be correct */                
                is_ns =  get_dm_tunit(data);
                total_delay = vtss_mep_supp_delay_calc(&data->rx_dmr_timestamp, &data->tx_dmm_fup_timestamp, is_ns);
                if (total_delay < 0)
                    vtss_mep_supp_trace("total_delay < 0", 0, 0, 0, 0);
                
                device_delay = vtss_mep_supp_delay_calc(&txTimeb, &rxTimef, is_ns);
                if (device_delay < 0)
                    vtss_mep_supp_trace("device_delay < 0", 0, 0, 0, 0);
                if ((total_delay >= 0) && (device_delay >= 0)) {
                    /* If delay < 0, it means something wrong. Just skip it */
                    dm_delay = total_delay - device_delay;
                    if (dm_delay >= 0 && dm_delay < 1e9 /* timeout is 1 second */) {   /* If dm_delay <= 0, it means something wrong. Just skip it. */
                        data->dmr[data->dmr_rx_cnt % VTSS_MEP_SUPP_DM_MAX] = dm_delay;
                        data->dmr_rx_cnt++;
                    }
                    else {
                        if (dm_delay < 0)
                            vtss_mep_supp_trace("dm_delay < 0", 0, 0, 0, 0); 
                        data->dmr_rx_err_cnt++;
                    } 
                }
                else {
                    data->dmr_rx_err_cnt++;
                }   
            }
           
            new_dmr = TRUE;
            data->dmm_tx_active = FALSE;
            data->tx_dmm_timeout_timer = 0;

#if F_2DM_FOR_1DM
            if (data->dmm_config.syncronized == TRUE) {             
                /* one-way with followup (far-to-near)*/
                txTimeb.seconds = string_to_var(&pdu[20]);    
                txTimeb.nanoseconds = string_to_var(&pdu[24]);
                
                if (data->rx_dm1_timestamp.seconds == 0) {
                    /* Unexpected 1DM FUP packet */
                    break;    
                }
                
                dm_delay = vtss_mep_supp_delay_calc(&data->rx_dm1_timestamp, &txTimeb, get_dm_tunit(data));
                
                if (dm_delay >= 0 && dm_delay < 1e9 /* delay over 1 second  */) {   /* If dm_delay <= 0, it means something wrong. Just skip it. */
                    data->dm1_far_to_near[data->dm1_rx_cnt_far_to_near % VTSS_MEP_SUPP_DM_MAX] = dm_delay;
                    data->dm1_rx_cnt_far_to_near++;
                } 
                else {
                    data->dm1_rx_err_cnt_far_to_near++;
                }
                
                /* One-way Standard (near-to-far)*/
                rxTimef.seconds = string_to_var(&pdu[12]);    
                rxTimef.nanoseconds = string_to_var(&pdu[16]);
                dm_delay = vtss_mep_supp_delay_calc(&rxTimef, &data->tx_dmm_fup_timestamp, get_dm_tunit(data));

                if (dm_delay >= 0 && dm_delay < 1e9) {   /* Check for valid delay */
                    data->dm1_near_to_far[data->dm1_rx_cnt_near_to_far % VTSS_MEP_SUPP_DM_MAX] = dm_delay;
                    data->dm1_rx_cnt_near_to_far++;
                } 
                else {
                    vtss_mep_supp_trace("vtss_mep_supp_rx_frame: dm_delay invalid", 0, 0, 0, 0);
                    data->dm1_rx_err_cnt_near_to_far++;
                }
                
                data->rx_dm1_timestamp.seconds = 0;
                data->rx_dm1_timestamp.nanoseconds = 0;
            }                
#endif
            break;

        case OAM_TYPE_DMM:
            if (level != data->config.level)      break;  /* Wrong level */
            if (data->config.voe && (data->config.direction == VTSS_MEP_SUPP_DOWN))   break;  /* Not expected to receive this */

            memcpy(&data->rx_dmm_timestamp, rx_time, sizeof(data->rx_dmm_timestamp));
            dmr_to_peer(mep, data, &frame[VTSS_MEP_SUPP_MAC_LENGTH], pdu, frame_info, reply_port_calc(data, port));
            break;

        case OAM_TYPE_1DM:
            if (level != data->config.level)      break;  /* Wrong level */

            if (memcmp(&pdu[DM1_PDU_STD_LENGTH], "VTSS", 4)) {
                /* One-way Standard */
                txTimeb.seconds = string_to_var(&pdu[4]);    
                txTimeb.nanoseconds = string_to_var(&pdu[8]);
                
                dm_delay = vtss_mep_supp_delay_calc(rx_time, &txTimeb, get_dm_tunit(data));     

                if (dm_delay >= 0 && dm_delay < 1e9 /* delay over 1 second  */) {   /* If dm_delay <= 0, it means something wrong. Just skip it. */
                    data->dm1_far_to_near[data->dm1_rx_cnt_far_to_near % VTSS_MEP_SUPP_DM_MAX] = dm_delay;
                    data->dm1_rx_cnt_far_to_near++;
                } 
                else {
                    vtss_mep_supp_trace("xxx vtss_mep_supp_rx_frame: dm_delay < 0", 0, 0, 0, 0);
                    data->dm1_rx_err_cnt_far_to_near++;
                }                 
            }
            else {
                /* One-way follow-up */
                /* Save the rx timestamp for calculation when follow-up packet arrives */ 
                data->rx_dm1_timestamp.seconds = rx_time->seconds;
                data->rx_dm1_timestamp.nanoseconds = rx_time->nanoseconds;
                break;
            }            
            new_dm1 = TRUE;
            break;
        
        case OAM_TYPE_1DM_FUP:
            if (level != data->config.level)                                        break;  /* Wrong level */

            /* one-way with followup */
            txTimeb.seconds = string_to_var(&pdu[4]);    
            txTimeb.nanoseconds = string_to_var(&pdu[8]);
            
            if (data->rx_dm1_timestamp.seconds == 0) {
                /* Unexpected 1DM FUP packet */
                break;    
            }
            
            dm_delay = vtss_mep_supp_delay_calc(&data->rx_dm1_timestamp, &txTimeb, get_dm_tunit(data));     
            
            if (dm_delay >= 0 && dm_delay < 1e9 /* delay over 1 second  */) {   /* If dm_delay <= 0, it means something wrong. Just skip it. */
                data->dm1_far_to_near[data->dm1_rx_cnt_far_to_near % VTSS_MEP_SUPP_DM_MAX] = dm_delay;
                data->dm1_rx_cnt_far_to_near++;
            } 
            else {
                data->dm1_rx_err_cnt_far_to_near++;
            }                  
            
            data->rx_dm1_timestamp.seconds = 0;
            data->rx_dm1_timestamp.nanoseconds = 0;
            
            new_dm1 = TRUE;
            break;

        case OAM_TYPE_AIS:
            if (level != data->config.level)      break;  /* Wrong level */

            if (!data->defect_state.dAis)    new_defect = TRUE;
            data->defect_state.dAis = TRUE;
            defect_timer_calc(data, &data->dAis, pdu[2] & 0x07);
            break;

        case OAM_TYPE_LCK:
            if (level != data->config.level)      break;  /* Wrong level */

            if (!data->defect_state.dLck)    new_defect = TRUE;
            data->defect_state.dLck = TRUE;
            defect_timer_calc(data, &data->dLck, pdu[2] & 0x07);
            break;

        case OAM_TYPE_TST:
            if (level != data->config.level)      break;  /* Wrong level */

            tst_frame_size = len + ((frame_info->tagged) ? 4 : 0) + data->tst_config.add_cnt + 4 + 8 + 12;  /* Added CRC and Preamble and Inter frame gap */
            new_tst = TRUE;

#ifdef VTSS_ARCH_SERVAL
            vtss_oam_voe_conf_t cfg;

            if (data->voe_idx < VTSS_OAM_VOE_CNT) { /* VOE based TST */
                (void)vtss_oam_voe_conf_get(NULL, data->voe_idx, &cfg);
                cfg.tst.copy_to_cpu = FALSE;
                (void)vtss_oam_voe_conf_set(NULL, data->voe_idx, &cfg);

#if 0
                if (data->tst_config.enable_rx) {   /* Get another TST PDU sample in 1 sec - if TST analyze is still enabled */
                    data->tst_timer = 1000/timer_res;
                    vtss_mep_supp_timer_start();
                }
#endif
            }
#endif
            break;

        default:            break;
    }

    unlock:
    vtss_mep_supp_crit_unlock();

    /* The following call-out is done here where crit is unlocked, in order to avoid deadlock */
    if (new_ltr)        vtss_mep_supp_new_ltr(mep);
    if (new_lbr)        vtss_mep_supp_new_lbr(mep);
    if (new_dm1)        vtss_mep_supp_new_dm1(mep);
    if (new_dmr)        vtss_mep_supp_new_dmr(mep);
    if (new_aps)        vtss_mep_supp_new_aps(mep, &pdu[4]);
    if (new_tst)        vtss_mep_supp_new_tst(mep, tst_frame_size);
    if (new_defect)     vtss_mep_supp_new_defect_state(mep);
    if (new_state)      vtss_mep_supp_new_ccm_state(mep);
}


#ifdef VTSS_ARCH_SERVAL
void vtss_mep_supp_voe_interrupt(void)
{
    supp_instance_data_t        *data;
    vtss_oam_event_mask_t       voe_mask;
    vtss_oam_voe_event_mask_t   voe_event;
    vtss_oam_ccm_status_t       ccm_status;
    u32                         size, voe, indexx, mask, mep;
    BOOL                        new_defect;

    (void)vtss_oam_event_poll(NULL, &voe_mask);
    size = sizeof(voe_mask.voe_mask[0]) * 8;

    vtss_mep_supp_crit_lock();
    for (voe=0, indexx=0, mask =0x01; voe<VTSS_OAM_VOE_CNT; ++voe, indexx+=(!(voe%size) ? 1 : 0), mask = (!(voe%size) ? 0x01 : (mask<<1))) {
        if (!(voe_mask.voe_mask[indexx] & mask))     continue;                                         /* Check for new event on this VOE instance */
        if (vtss_oam_voe_event_poll(NULL, voe, &voe_event) != VTSS_RC_OK)   continue;                 /* Poll the VOE */
        (void)vtss_oam_voe_event_enable(NULL, voe, voe_event, FALSE);

        mep = voe_to_mep[voe];
        data = &instance_data[mep];    /* Instance data reference */

        if (!voe_ccm_enable(data))    continue;   /* Check if this MEP instance has valid VOE and only one peer */

        new_defect = FALSE;

        (void)vtss_oam_ccm_status_get(NULL, data->voe_idx, &ccm_status);

        /* Set defects active if new state */
        if (voe_event & VTSS_OAM_VOE_EVENT_CCM_ZERO_PERIOD)     new_defect = TRUE;      /* Status is fetched directly from VOE */
        if (voe_event & VTSS_OAM_VOE_EVENT_CCM_RX_RDI)          new_defect = TRUE;
        if (voe_event & VTSS_OAM_VOE_EVENT_CCM_LOC)             new_defect = TRUE;
        if (voe_event & VTSS_OAM_VOE_EVENT_CCM_MEG_ID) {
            if (!data->defect_state.dMeg && ccm_status.meg_id_err) { /* Only Raising edge */
                new_defect = TRUE;
                data->defect_state.dMeg = TRUE;
                defect_timer_calc(data, &data->dMeg, period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_10S));     /* This is to activate defect timer in case this detect is not caught by CCM poll */
                data->ccm_defect_active = TRUE;
            }
        }
        if (voe_event & VTSS_OAM_VOE_EVENT_CCM_MEP_ID) {
            if (!data->defect_state.dMep && ccm_status.mep_id_err) { /* Only Raising edge */
                new_defect = TRUE;
                data->defect_state.dMep = TRUE;
                defect_timer_calc(data, &data->dMep, period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_10S));     /* This is to activate defect timer in case this detect is not caught by CCM poll */
                data->ccm_defect_active = TRUE;
            }
        }
        if (voe_event & VTSS_OAM_VOE_EVENT_CCM_PERIOD) {
            if (!data->defect_state.dPeriod[0] && ccm_status.period_err) { /* Only Raising edge */
                new_defect = TRUE;
                data->defect_state.dPeriod[0] = TRUE;
                defect_timer_calc(data, &data->dPeriod[0], period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_10S));     /* This is to activate defect timer in case this detect is not caught by CCM poll */
                data->ccm_defect_active = TRUE;
            }
        }
        if (voe_event & VTSS_OAM_VOE_EVENT_CCM_PRIORITY) {
            if (!data->defect_state.dPrio[0] && ccm_status.priority_err) { /* Only Raising edge */
                new_defect = TRUE;
                data->defect_state.dPrio[0] = TRUE;
                defect_timer_calc(data, &data->dPrio[0], period_to_pdu_calc(VTSS_MEP_SUPP_PERIOD_10S));     /* This is to activate defect timer in case this detect is not caught by CCM poll */
                data->ccm_defect_active = TRUE;
            }
        }

        if (new_defect) {
            vtss_mep_supp_crit_unlock();
            vtss_mep_supp_new_defect_state(mep);
            vtss_mep_supp_crit_lock();
        }
    }
    vtss_mep_supp_crit_unlock();
}
#endif

BOOL vtss_mep_supp_voe_up(u32 isdx, u32 *voe)
{   /* Check if this ISDX relates to a VOE UP-MEP */
#ifdef VTSS_ARCH_SERVAL
    u32 i;
    supp_instance_data_t  *data;

    vtss_mep_supp_crit_lock();
    for (i=0; i<VTSS_OAM_PATH_SERVICE_VOE_CNT; ++i) {
        if (voe_to_mep[i] < VTSS_MEP_SUPP_CREATED_MAX) { /* This VOE index is in use */
            data = &instance_data[voe_to_mep[i]];        /* Instance data reference */
            if (data->rx_isdx == isdx) {         /* Check if this ISDX belong to this MEP */
                *voe = i;
                vtss_mep_supp_crit_unlock();
                return (data->config.direction == VTSS_MEP_SUPP_UP);
            }
        }
    }
    vtss_mep_supp_crit_unlock();
#endif
    return(FALSE);
}



u32 vtss_mep_supp_rx_isdx_get(const u32  instance)
{
    return(instance_data[instance].rx_isdx);
}

/****************************************************************************/
/*  MEP configuration interface                                             */
/****************************************************************************/

u32 vtss_mep_supp_conf_set(const u32                     instance,
                           const vtss_mep_supp_conf_t    *const conf)
{
    u32     retval;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                             					                  return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].config.enable && !conf->enable)                                           					                  return(VTSS_MEP_SUPP_RC_OK);
//    if (!memcmp(&instance_data[instance].config, conf, sizeof(vtss_mep_supp_conf_t)))                      					                  return(VTSS_MEP_SUPP_RC_OK);
    if (instance_data[instance].config.enable && conf->enable &&
        ((instance_data[instance].config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) !=
         (conf->flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID))))                                             return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (instance_data[instance].config.enable && conf->enable && (instance_data[instance].config.direction != conf->direction))               return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
#ifndef VTSS_ARCH_SERVAL                                                                                                                      
    if (conf->voe)                                                                                                                            return(VTSS_MEP_SUPP_RC_NO_VOE);
#endif

    vtss_mep_supp_crit_lock();

    retval = VTSS_MEP_SUPP_RC_OK;
    if (conf->enable)      retval = run_sync_config(instance, conf);
    if (retval == VTSS_MEP_SUPP_RC_OK)
    {
        instance_data[instance].event_flags |= EVENT_IN_CONFIG;
        if (conf->enable)   instance_data[instance].config = *conf;
        else                instance_data[instance].config.enable = FALSE;

#ifdef VTSS_ARCH_SERVAL
        u32 i;
        if ((instance_data[instance].config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) && (instance_data[instance].config.mode == VTSS_MEP_SUPP_MEP)) { /* This is a EVC or VLAN MEP - all MEP in this VID must be re-configured to assure correct TCAM rules and VOE configuration */
            for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; i++) /* Find instance on this vid */
                if (instance_data[i].config.enable && (instance_data[i].config.mode == VTSS_MEP_SUPP_MEP) && (instance_data[i].config.flow.mask & (VTSS_MEP_SUPP_FLOW_MASK_EVC_ID | VTSS_MEP_SUPP_FLOW_MASK_VLAN_ID)) &&
                    (instance_data[i].config.flow.vid == instance_data[instance].config.flow.vid)) {
                    instance_data[i].event_flags |= EVENT_IN_CONFIG;
                }
        }
#endif
    }

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(retval);
}

u32 vtss_mep_supp_ccm_conf_set(const u32                          instance,
                               const vtss_mep_supp_ccm_conf_t     *const conf)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                   return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!memcmp(&instance_data[instance].ccm_config, conf, sizeof(vtss_mep_supp_ccm_conf_t)))    return(VTSS_MEP_SUPP_RC_OK);
    if ((conf->peer_count > 1) && hw_ccm_calc(conf->period))                                     return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_CCM_CONFIG;

    /* Check if the learned peer MAC must be deleted */
    if (conf->peer_count >= instance_data[instance].ccm_config.peer_count) { /* Number of peer MEP is increased */
        if (memcmp(conf->peer_mep, instance_data[instance].ccm_config.peer_mep, instance_data[instance].ccm_config.peer_count*sizeof(u16))) /* Clear if the existing peer MEP is changed */
            memset(instance_data[instance].peer_mac, 0, (VTSS_MEP_SUPP_PEER_MAX*VTSS_MEP_SUPP_MAC_LENGTH));
    }
    else {   /* Number of peer MEP is the same or less */
        if (memcmp(conf->peer_mep, instance_data[instance].ccm_config.peer_mep, conf->peer_count*sizeof(u16)))  /* Clear if the remaining peer MEP is changed */
            memset(instance_data[instance].peer_mac, 0, (VTSS_MEP_SUPP_PEER_MAX*VTSS_MEP_SUPP_MAC_LENGTH));
        memset(instance_data[instance].peer_mac[conf->peer_count], 0, ((VTSS_MEP_SUPP_PEER_MAX-conf->peer_count)*VTSS_MEP_SUPP_MAC_LENGTH));
    }

    instance_data[instance].ccm_config = *conf;

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();
    
    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_ccm_generator_set(const u32                         instance,
                                    const vtss_mep_supp_gen_conf_t    *const conf)
{
    u32 retval;

    retval = VTSS_MEP_SUPP_RC_OK;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!memcmp(&instance_data[instance].ccm_gen, conf, sizeof(vtss_mep_supp_gen_conf_t)))    return(VTSS_MEP_SUPP_RC_OK);
    if ((instance_data[instance].ccm_config.peer_count > 1) && hw_ccm_calc(conf->cc_period))  return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    if (hw_ccm_calc(conf->cc_period))
    {   /* HW generation requested */
#ifdef VTSS_ARCH_LUTON26
        if (conf->lm_enable)    {vtss_mep_supp_crit_unlock();  return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);}   /* On Luton26 SW CCM-LM is not possible besides HW CCM */
#endif
        if (conf->enable)
            retval = run_sync_ccm_hw_gen(instance);
#ifdef VTSS_ARCH_JAGUAR_1
        if ((conf->lm_enable) && (retval == VTSS_MEP_SUPP_RC_OK))
            retval = run_sync_ccm_sw_gen(instance);  /* On Jaguar CCM-LM must be transmitted by SW */
#endif
#ifdef VTSS_ARCH_SERVAL
        if ((conf->lm_enable) && (retval == VTSS_MEP_SUPP_RC_OK) && instance_data[instance].config.voe && (instance_data[instance].config.direction == VTSS_MEP_SUPP_UP))
            retval = run_sync_ccm_sw_gen(instance); /* On Serval CCM-LM must be transmitted by SW in case of UP-MEP */
#endif
    }
    else
    {   /* SW generator requested */
        if (conf->enable)     retval = run_sync_ccm_sw_gen(instance);
    }

    if (retval == VTSS_MEP_SUPP_RC_OK)
    {
        if (conf->lm_enable && !instance_data[instance].ccm_gen.lm_enable)
            instance_data[instance].lm_start = TRUE;  /* This is to know when the first LM counters are received. The first counters only used to get 'previous' frame counters initialized */

        instance_data[instance].event_flags |= EVENT_IN_CCM_GEN;
        instance_data[instance].ccm_start = ((conf->enable) && (!instance_data[instance].ccm_gen.enable)) ? TRUE : FALSE;
        if (conf->enable)   instance_data[instance].ccm_gen = *conf;
        else                instance_data[instance].ccm_gen.enable = FALSE;

        vtss_mep_supp_run();
    }

    vtss_mep_supp_crit_unlock();

    return(retval);
}

u32 vtss_mep_supp_ccm_rdi_set(const u32       instance,
                              const BOOL      state)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)              return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (instance_data[instance].ccm_rdi == state)           return(VTSS_MEP_SUPP_RC_OK);

    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_CCM_RDI;
    instance_data[instance].ccm_rdi = state;

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_ccm_state_get(const u32                     instance,
                                vtss_mep_supp_ccm_state_t     *const state)
{
    supp_instance_data_t        *data;
    u32 offset, length;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)              return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    data = &instance_data[instance];    /* Instance data reference */
    *state = data->ccm_state.state;

    if (data->ccm_state.unexp_meg_id[0] == 1) {
        if (data->ccm_state.unexp_meg_id[1] == 32) {   /* ITU ICC based MEG-ID - short MA name */
            memcpy(state->unexp_name, &data->ccm_state.unexp_meg_id[3], 6);
            state->unexp_name[6] = '\0';
            memcpy(state->unexp_meg, &data->ccm_state.unexp_meg_id[9], 7);
            state->unexp_meg[7] = '\0';
        }
    }
    else {
        if (data->ccm_state.unexp_meg_id[0] == 4) {   /* IEEE string */
            offset = 2; /* Name offset and length */
            length = data->ccm_state.unexp_meg_id[1];
            length = (length >= VTSS_MEP_SUPP_MEG_CODE_LENGTH-1) ? VTSS_MEP_SUPP_MEG_CODE_LENGTH-1 : length;
            memcpy(state->unexp_name, &data->ccm_state.unexp_meg_id[offset], length);
            state->unexp_name[length] = '\0';

            offset = 4 + data->ccm_state.unexp_meg_id[1];   /* Short name offset and length */
            if (offset < MEG_ID_LENGTH) { /* The short name is in the buffer */
                if (data->ccm_state.unexp_meg_id[2+data->ccm_state.unexp_meg_id[1]] == 2) {   /* IEEE string */
                    length = data->ccm_state.unexp_meg_id[3+data->ccm_state.unexp_meg_id[1]];
                    length = (length >= VTSS_MEP_SUPP_MEG_CODE_LENGTH-1) ? VTSS_MEP_SUPP_MEG_CODE_LENGTH-1 : length;
                    length = ((offset+length) > MEG_ID_LENGTH) ? MEG_ID_LENGTH-offset : length;
                    memcpy(state->unexp_meg, &data->ccm_state.unexp_meg_id[offset], length);
                    state->unexp_meg[length] = '\0';
                }
            }
        }
    }

#ifdef VTSS_ARCH_SERVAL
    vtss_oam_voe_counter_t voe_counter;
    if (voe_ccm_enable(data)) {  /* This VOE based CCM - Check for VOE based status */
        if (vtss_oam_voe_counter_get(NULL, data->voe_idx, &voe_counter) != VTSS_RC_OK) {
            vtss_mep_supp_crit_unlock();
            return(VTSS_MEP_SUPP_RC_VOE_ERROR);
        }

        state->valid_counter = voe_counter.ccm.rx_valid_count;
        state->invalid_counter = voe_counter.ccm.rx_invalid_count;
        state->oo_counter = voe_counter.ccm.rx_invalid_seq_no;
    }
#endif
    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_ccm_lm_counters_get(const u32                    instance,
                                      vtss_mep_supp_lm_counters_t  *const counters)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)        return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    counters->rx_counter = instance_data[instance].ccm_lm_counter.rx_counter;
    counters->tx_counter = instance_data[instance].ccm_lm_counter.tx_counter;
    counters->near_los_counter = instance_data[instance].ccm_lm_counter.near_los_counter;
    counters->far_los_counter = instance_data[instance].ccm_lm_counter.far_los_counter;
    counters->near_tx_counter = instance_data[instance].ccm_lm_counter.near_tx_counter;
    counters->far_tx_counter = instance_data[instance].ccm_lm_counter.far_tx_counter;

    instance_data[instance].ccm_lm_counter.tx_counter = 0;
    instance_data[instance].ccm_lm_counter.rx_counter = 0;
    instance_data[instance].ccm_lm_counter.near_los_counter = 0;
    instance_data[instance].ccm_lm_counter.far_los_counter = 0;
    instance_data[instance].ccm_lm_counter.near_tx_counter = 0;
    instance_data[instance].ccm_lm_counter.far_tx_counter = 0;

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_defect_state_get(const u32                      instance,
                                   vtss_mep_supp_defect_state_t   *const state)
{
    u32 i;
    vtss_rc rc = VTSS_RC_OK;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)        return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    *state = instance_data[instance].defect_state;

    /* sw/hw detected LOC is converted to a TRUE/FALSE */
    for (i=0; i<instance_data[instance].ccm_config.peer_count; ++i)
        state->dLoc[i] = (state->dLoc[i] != 0) ? TRUE : FALSE;

#ifdef VTSS_ARCH_SERVAL
    vtss_oam_ccm_status_t       ccm_status;
    if (voe_ccm_enable(&instance_data[instance])) {  /* This is VOE based CCM - Check for VOE based status */
        rc += vtss_oam_ccm_status_get(NULL, instance_data[instance].voe_idx, &ccm_status);
        state->dInv = ccm_status.zero_period_err;
        state->dLoc[0] = ccm_status.loc;
        state->dRdi[0] = ccm_status.rx_rdi;
        if (state->dLoc[0])    state->dRdi[0] = FALSE;
    }
#endif
    vtss_mep_supp_crit_unlock();

    return((rc == VTSS_RC_OK) ? VTSS_MEP_SUPP_RC_OK : VTSS_MEP_SUPP_RC_VOE_ERROR);
}

u32 vtss_mep_supp_learned_mac_get(const u32   instance,
                                  u8          (*mac)[VTSS_MEP_SUPP_MAC_LENGTH])
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)        return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    memcpy(mac, instance_data[instance].peer_mac, (VTSS_MEP_SUPP_PEER_MAX*VTSS_MEP_SUPP_MAC_LENGTH));

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_lmm_conf_set(const u32                        instance,
                               const vtss_mep_supp_lmm_conf_t   *const conf)
{
    u32 retval;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                   return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].lmm_config.enable && !conf->enable)                             return(VTSS_MEP_SUPP_RC_OK);
    if (!memcmp(&instance_data[instance].lmm_config, conf, sizeof(vtss_mep_supp_lmm_conf_t)))    return(VTSS_MEP_SUPP_RC_OK);

    vtss_mep_supp_crit_lock();

    retval = VTSS_MEP_SUPP_RC_OK;
    if (conf->enable)      retval = run_sync_lmm_config(instance);
    if (retval == VTSS_MEP_SUPP_RC_OK)
    {
        if (conf->enable && !instance_data[instance].lmm_config.enable)
            instance_data[instance].lm_start = TRUE;  /* This is to know when the first LM counters are received. The first counters only used to get 'previous' frame counters initialized */
        instance_data[instance].event_flags |= EVENT_IN_LMM_CONFIG;
        if (conf->enable)   instance_data[instance].lmm_config = *conf;
        else                instance_data[instance].lmm_config.enable = FALSE;
    }

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_lmm_lm_counters_get(const u32                    instance,
                                      vtss_mep_supp_lm_counters_t  *const counters)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)         return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    counters->rx_counter = instance_data[instance].lmm_lm_counter.rx_counter;
    counters->tx_counter = instance_data[instance].lmm_lm_counter.tx_counter;
    counters->near_los_counter = instance_data[instance].lmm_lm_counter.near_los_counter;
    counters->far_los_counter = instance_data[instance].lmm_lm_counter.far_los_counter;
    counters->near_tx_counter = instance_data[instance].lmm_lm_counter.near_tx_counter;
    counters->far_tx_counter = instance_data[instance].lmm_lm_counter.far_tx_counter;

    instance_data[instance].lmm_lm_counter.tx_counter = 0;
    instance_data[instance].lmm_lm_counter.rx_counter = 0;
    instance_data[instance].lmm_lm_counter.near_los_counter = 0;
    instance_data[instance].lmm_lm_counter.far_los_counter = 0;
    instance_data[instance].lmm_lm_counter.near_tx_counter = 0;
    instance_data[instance].lmm_lm_counter.far_tx_counter = 0;

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_aps_conf_set(const u32                         instance,
                               const vtss_mep_supp_aps_conf_t    *const conf)
{
    u32 retval;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                 return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].aps_config.enable && !conf->enable)                           return(VTSS_MEP_SUPP_RC_OK);
    if (!memcmp(&instance_data[instance].aps_config, conf, sizeof(vtss_mep_supp_aps_conf_t)))  return(VTSS_MEP_SUPP_RC_OK);

    vtss_mep_supp_crit_lock();

    retval = VTSS_MEP_SUPP_RC_OK;
    if (conf->enable)      retval = run_sync_aps_config(instance);
    if (retval == VTSS_MEP_SUPP_RC_OK)
    {
        instance_data[instance].event_flags |= EVENT_IN_APS_CONFIG;
        if (conf->enable)   instance_data[instance].aps_config = *conf;
        else                instance_data[instance].aps_config.enable = FALSE;
    }

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(retval);
}

u32 vtss_mep_supp_aps_txdata_set(const u32     instance,
                                 const u8      *txdata,
                                 const BOOL    event)
{
    vtss_mep_supp_crit_lock();

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                 {vtss_mep_supp_crit_unlock(); return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);}

    instance_data[instance].event_flags |= EVENT_IN_APS_TXDATA;
    instance_data[instance].aps_event = event;
    if (!event)     memcpy(instance_data[instance].aps_txdata, txdata, VTSS_MEP_SUPP_RAPS_DATA_LENGTH);

//    vtss_mep_supp_run();         No longer asynchronious
    run_aps_txdata(instance);

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_raps_transmission(const u32    instance,
                                    const BOOL   enable)
{
    vtss_mep_supp_crit_lock();

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)        {vtss_mep_supp_crit_unlock(); return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);}
    if (instance_data[instance].aps_tx == enable)     {vtss_mep_supp_crit_unlock(); return(VTSS_MEP_SUPP_RC_OK);}

    instance_data[instance].aps_tx = enable;

    if (enable)     run_aps_txdata(instance);

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_raps_forwarding(const u32    instance,
                                  const BOOL   enable)
{
    vtss_mep_supp_crit_lock();

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)             {vtss_mep_supp_crit_unlock(); return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);}
    if (!instance_data[instance].config.voe)               {vtss_mep_supp_crit_unlock(); return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);}
    if (instance_data[instance].aps_forward == enable)     {vtss_mep_supp_crit_unlock(); return(VTSS_MEP_SUPP_RC_OK);}

    instance_data[instance].event_flags |= EVENT_IN_APS_FORWARD;
    instance_data[instance].aps_forward = enable;

    vtss_mep_supp_crit_unlock();

    vtss_mep_supp_run();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_ltm_conf_set(const u32                         instance,
                               const vtss_mep_supp_ltm_conf_t    *const conf)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].ltm_config.enable && !conf->enable)          return(VTSS_MEP_SUPP_RC_OK);

    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_LTM_CONFIG;
    if (conf->enable)   instance_data[instance].ltm_config = *conf;
    else                instance_data[instance].ltm_config.enable = FALSE;

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_ltr_get(const u32            instance,
                          u32                  *const count,
                          vtss_mep_supp_ltr_t  ltr[VTSS_MEP_SUPP_LTR_MAX])
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)               return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].ltm_config.enable)          return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    *count = instance_data[instance].ltr_cnt;
    memcpy(ltr, instance_data[instance].ltr, sizeof(vtss_mep_supp_ltr_t) * instance_data[instance].ltr_cnt);
    instance_data[instance].ltr_cnt = 0;

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_lbm_conf_set(const u32                         instance,
                               const vtss_mep_supp_lbm_conf_t    *const conf)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (instance_data[instance].lbm_config.enable && conf->enable)            return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].lbm_config.enable && !conf->enable)          return(VTSS_MEP_SUPP_RC_OK);
    if (conf->enable) {
        if (conf->size > VTSS_MEP_SUPP_LBM_SIZE_MAX)          return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
        if (conf->size < VTSS_MEP_SUPP_LBM_SIZE_MIN)          return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    }

    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_LBM_CONFIG;
    instance_data[instance].lbm_start = ((conf->enable) && (!instance_data[instance].lbm_config.enable)) ? TRUE : FALSE;

    if (conf->enable)   instance_data[instance].lbm_config = *conf;
    else                instance_data[instance].lbm_config.enable = FALSE;

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_lbr_get(const u32            instance,
                          u32                  *const count,
                          vtss_mep_supp_lbr_t  lbr[VTSS_MEP_SUPP_LBR_MAX])
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)               return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].lbm_config.enable)          return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    *count = instance_data[instance].lbr_cnt;
    memcpy(lbr, instance_data[instance].lbr, sizeof(vtss_mep_supp_lbr_t) * instance_data[instance].lbr_cnt);
    instance_data[instance].lbr_cnt = 0;

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_lb_status_get(const u32                  instance,
                                vtss_mep_supp_lb_status_t  *const status)
{
    supp_instance_data_t  *data;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)               return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    data = &instance_data[instance];    /* Instance data reference */

    *status = data->lb_state;
    status->trans_id += 1;

#ifdef VTSS_ARCH_SERVAL
    vtss_oam_voe_counter_t voe_counter;
    vtss_oam_proc_status_t voe_status;

    if (data->voe_idx < VTSS_OAM_VOE_CNT) {
        if (vtss_oam_voe_counter_get(NULL, data->voe_idx, &voe_counter) != VTSS_RC_OK) {
            vtss_mep_supp_crit_unlock();
            return(VTSS_MEP_SUPP_RC_VOE_ERROR);
        }
        status->lbr_counter = voe_counter.lb.rx_lbr;
        status->lbm_counter = voe_counter.lb.tx_lbm;
        status->oo_counter = voe_counter.lb.rx_lbr_trans_id_err;

        (void)vtss_oam_proc_status_get(NULL, data->voe_idx, &voe_status);
        status->trans_id = voe_status.tx_next_lbm_transaction_id + 1;
    }
#endif
    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_tst_conf_set(const u32                         instance,
                               const vtss_mep_supp_tst_conf_t    *const conf)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                 return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].tst_config.enable && !conf->enable &&
        !instance_data[instance].tst_config.enable_rx && !conf->enable_rx)                     return(VTSS_MEP_SUPP_RC_OK);
    if (!memcmp(&instance_data[instance].tst_config, conf, sizeof(vtss_mep_supp_tst_conf_t)))  return(VTSS_MEP_SUPP_RC_OK);
    if (conf->enable) {
        if (conf->size > VTSS_MEP_SUPP_TST_SIZE_MAX)          return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
        if (conf->size < VTSS_MEP_SUPP_TST_SIZE_MIN)          return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
        if (conf->rate > VTSS_MEP_SUPP_TST_RATE_MAX)          return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
        if (conf->rate < 1)                                   return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    }

    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_TST_CONFIG;
    instance_data[instance].tst_start = (((conf->enable) && (!instance_data[instance].tst_config.enable)) || ((conf->enable_rx) && (!instance_data[instance].tst_config.enable_rx))) ? TRUE : FALSE;
    if (conf->enable || conf->enable_rx)   instance_data[instance].tst_config = *conf;
    else {
        instance_data[instance].tst_config.enable = FALSE;
        instance_data[instance].tst_config.enable_rx = FALSE;
    }
    
    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_tst_status_get(const u32                   instance,
                                 vtss_mep_supp_tst_status_t  *const status)
{
    u64 tx_counter;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                 return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    memset(status, 0, sizeof(*status));

    vtss_mep_supp_crit_lock();

    if (instance_data[instance].tst_config.enable && instance_data[instance].tx_tst) {  /* If test is ongonig we return currently transmitted frame count - this is not accurate. The accurate value is obtained when test is stopped */
        vtss_mep_supp_packet_tx_frm_cnt(instance_data[instance].tx_tst, &tx_counter);
        instance_data[instance].tst_tx_cnt = tx_counter;
    }

    status->tx_counter = instance_data[instance].tst_tx_cnt;

#ifdef VTSS_ARCH_SERVAL
    vtss_oam_voe_counter_t voe_counter;
    if (instance_data[instance].voe_idx < VTSS_OAM_VOE_CNT) {
        if (vtss_oam_voe_counter_get(NULL, instance_data[instance].voe_idx, &voe_counter) == VTSS_RC_OK) {
            status->tx_counter = voe_counter.tst.tx_tst;
            status->rx_counter = voe_counter.tst.rx_tst;
            status->oo_counter = voe_counter.tst.rx_tst_trans_id_err;
        }
    }
#endif

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}


u32 vtss_mep_supp_tst_clear(const u32    instance)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                 return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    instance_data[instance].tst_tx_cnt = 0;

#ifdef VTSS_ARCH_SERVAL
    vtss_oam_voe_conf_t   cfg;

    if (instance_data[instance].voe_idx < VTSS_OAM_VOE_CNT) { /* On a VOE the counters need to be cleared and arm to receive the next TST PDU */
        (void)vtss_oam_voe_counter_clear(NULL, instance_data[instance].voe_idx, VTSS_OAM_CNT_TST);
        if (instance_data[instance].tst_config.enable_rx && vtss_oam_voe_conf_get(NULL, instance_data[instance].voe_idx, &cfg) == VTSS_RC_OK) {
            cfg.tst.copy_to_cpu = TRUE;
            (void)vtss_oam_voe_conf_set(NULL, instance_data[instance].voe_idx, &cfg);
        }
    }
#endif

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}


u32 vtss_mep_supp_dmm_conf_set(const u32                        instance,
                               const vtss_mep_supp_dmm_conf_t   *const conf)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                   return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].dmm_config.enable && !conf->enable)                             return(VTSS_MEP_SUPP_RC_OK);
    if (!memcmp(&instance_data[instance].dmm_config, conf, sizeof(vtss_mep_supp_dmm_conf_t)))    return(VTSS_MEP_SUPP_RC_OK);

    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_DMM_CONFIG;
    if (conf->enable)   instance_data[instance].dmm_config = *conf;
    else                instance_data[instance].dmm_config.enable = FALSE;

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_dm1_conf_set(const u32                        instance,
                               const vtss_mep_supp_dm1_conf_t   *const conf)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                             return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!memcmp(&instance_data[instance].dm1_config, conf, sizeof(vtss_mep_supp_dm1_conf_t)))              return(VTSS_MEP_SUPP_RC_OK);
#ifdef VTSS_ARCH_SERVAL
    if (conf->enable && conf->proprietary)                                                                 return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
#endif
    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_1DM_CONFIG;
    if (conf->enable)   instance_data[instance].dm1_config = *conf;
    else {
        instance_data[instance].dm1_config.enable = FALSE;
        instance_data[instance].dm1_config.tunit = conf->tunit;    /* This is only in order to change the time unit for 1DM reception - 1DM is not enabled  */
    }

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}



static void dm1_far_to_near_get(const u32    instance,
                                u32          *const txcount,  
                                u32          *const rxcount,
                                u32          *const rx_err_count,
                                u32          dm1[VTSS_MEP_SUPP_DM_MAX])
{
    u32 postition, num;

    if (instance_data[instance].dm1_rx_cnt_far_to_near > VTSS_MEP_SUPP_DM_MAX)
    {
        /*
         * A little tricky here. Only the latest "VTSS_MEP_SUPP_DM_MAX" etries
         * are saved when the buffer is full. The implementation treats the
         * buffer as a ring. The oldest one is overwritten when full. See 
         * comments in vtss_mep_supp_dmr_get(). 
         */
        *rxcount = VTSS_MEP_SUPP_DM_MAX;
        postition = (instance_data[instance].dm1_rx_cnt_far_to_near % VTSS_MEP_SUPP_DM_MAX); 
        num = VTSS_MEP_SUPP_DM_MAX - postition; 
        
        memcpy(dm1, &instance_data[instance].dm1_far_to_near[postition], sizeof(u32) * num);
        memcpy(&dm1[num], &instance_data[instance].dm1_far_to_near[0], sizeof(u32) * postition);
    }
    else
    {    
        *rxcount = instance_data[instance].dm1_rx_cnt_far_to_near;
        memcpy(dm1, instance_data[instance].dm1_far_to_near, sizeof(u32) * instance_data[instance].dm1_rx_cnt_far_to_near);
    }
    *txcount = 0; /* always 0 */
    *rx_err_count = instance_data[instance].dm1_rx_err_cnt_far_to_near;
    instance_data[instance].dm1_rx_cnt_far_to_near = 0;
    instance_data[instance].dm1_rx_err_cnt_far_to_near = 0;
}

static void dm1_near_to_far_get(const u32    instance,
                                u32          *const txcount,  
                                u32          *const rxcount,
                                u32          *const rx_err_count,
                                u32          dm1[VTSS_MEP_SUPP_DM_MAX])
{
    u32 postition, num;

    if (instance_data[instance].dm1_rx_cnt_near_to_far > VTSS_MEP_SUPP_DM_MAX)
    {
        /*
         * A little tricky here. Only the latest "VTSS_MEP_SUPP_DM_MAX" etries
         * are saved when the buffer is full. The implementation treats the
         * buffer as a ring. The oldest one is overwritten when full. See 
         * comments in vtss_mep_supp_dmr_get(). 
         */
        *rxcount = VTSS_MEP_SUPP_DM_MAX;
        postition = (instance_data[instance].dm1_rx_cnt_near_to_far % VTSS_MEP_SUPP_DM_MAX); 
        num = VTSS_MEP_SUPP_DM_MAX - postition; 
        
        memcpy(dm1, &instance_data[instance].dm1_near_to_far[postition], sizeof(u32) * num);
        memcpy(&dm1[num], &instance_data[instance].dm1_near_to_far[0], sizeof(u32) * postition);
    }
    else
    {    
        *rxcount = instance_data[instance].dm1_rx_cnt_near_to_far;
        memcpy(dm1, instance_data[instance].dm1_near_to_far, sizeof(u32) * instance_data[instance].dm1_rx_cnt_near_to_far);
    }
    *txcount = instance_data[instance].dm1_tx_cnt;
    *rx_err_count = instance_data[instance].dm1_rx_err_cnt_near_to_far;
    instance_data[instance].dm1_tx_cnt = 0;
    instance_data[instance].dm1_rx_cnt_near_to_far = 0;
    instance_data[instance].dm1_rx_err_cnt_near_to_far = 0;
}



u32 vtss_mep_supp_dmr_timestamps_get(const u32                        instance,
                                vtss_mep_mgmt_dm_timestamp_t *const dm1_timestamp_far_to_near,
                                vtss_mep_mgmt_dm_timestamp_t *const dm1_timestamp_near_to_far)
{
    u32 ret_val = VTSS_MEP_SUPP_RC_OK;
    
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)               return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].dmm_config.enable)          return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();
    if (instance_data[instance].dmm_config.syncronized == TRUE && instance_data[instance].dmr_ts_ok == TRUE &&
            instance_data[instance].tx_dmm_fup_timestamp.seconds != 0) {            
        dm1_timestamp_near_to_far->tx_time = instance_data[instance].tx_dmm_fup_timestamp;
        dm1_timestamp_near_to_far->rx_time = instance_data[instance].rxTimef;
        dm1_timestamp_far_to_near->tx_time = instance_data[instance].txTimeb;
        dm1_timestamp_far_to_near->rx_time = instance_data[instance].rx_dmr_timestamp;
    } else {
        ret_val = VTSS_MEP_SUPP_RC_INVALID_PARAMETER;
    }
    instance_data[instance].dmr_ts_ok = FALSE;
    vtss_mep_supp_crit_unlock();
    
    return(ret_val);
}



u32 vtss_mep_supp_dmr_get(const u32                        instance,
                          vtss_mep_supp_dmr_status *const  status)
{
    u32 postition, num, dummy;
    
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)               return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].dmm_config.enable)          return(VTSS_MEP_SUPP_RC_OK);

    vtss_mep_supp_crit_lock();

    memset(status, 0, sizeof(vtss_mep_supp_dmr_status));

    if (instance_data[instance].dmr_rx_cnt > VTSS_MEP_SUPP_DM_MAX)
    {
        /*
         * A little tricky here. Only the latest "VTSS_MEP_SUPP_DM_MAX" etries
         * are saved when the buffer is full. The implementation treats the
         * buffer as a ring. The oldest one is overwritten when full.  
         */
        status->rx_counter = VTSS_MEP_SUPP_DM_MAX;
        /* The index of the first entry to get */
        postition = (instance_data[instance].dmr_rx_cnt % VTSS_MEP_SUPP_DM_MAX); 
        /* The number of entry from the first positon to the end of the buffer */
        num = VTSS_MEP_SUPP_DM_MAX - postition; 
        
        /*
         *        1th   2nd     3rd     4th     5th <- The no.of entery before table full
         *      ----------------------------------
         *        0     1       2       3       4 <- Array index    
         *      -----------------------------------
         *        6th   7th     8th     9th    10th <- The no.of entery before table full
         *       11th  12th    13th
         *  If there are 13 erntries in a 5-entry table, the last five entries in the
         *  table is 9th(arry[3]->dmr[0]), 10th(array[4]->dmr[1]), 11th(array[0]->dmr[2]),
         *  12th(arrary[1]->dmr[3])and 13th(array[2]->dmr[4]). Then
         *     
         *  position = 13 % 5 = 3 (instance_data[instance].dmr_rx_cnt % VTSS_MEP_SUPP_DM_MAX)
         *  num = 5 - 3 = 2 (VTSS_MEP_SUPP_DM_MAX - postition)
         *  
         *  memcpy(dm, &instance_data[instance].dmr[postition], sizeof(vtss_mep_supp_dm_t) * num);
         *  memcpy(dm, &instance_data[instance].dmr[3], sizeof(u32) * 2);
         *  memcpy(dmr[num], &instance_data[instance].dmr[0], sizeof(u32) * postition);
         *  memcpy(dmr[2], &instance_data[instance].dmr[0], sizeof(u32) * 3);
         *
         *      
         */
        memcpy(status->tw_delays, &instance_data[instance].dmr[postition], sizeof(u32) * num);
        memcpy(&status->tw_delays[num], &instance_data[instance].dmr[0], sizeof(u32) * postition);
    }
    else
    {    
        status->rx_counter = instance_data[instance].dmr_rx_cnt;
        memcpy(status->tw_delays, instance_data[instance].dmr, sizeof(u32) * instance_data[instance].dmr_rx_cnt);
    }
    
    status->tx_counter = instance_data[instance].dmm_tx_cnt;
    status->rx_tout_counter = instance_data[instance].dmr_rx_tout_cnt; 
    status->tw_rx_err_counter = instance_data[instance].dmr_rx_err_cnt;
    status->late_txtime_counter = instance_data[instance].dmm_late_txtime_cnt;
    
    instance_data[instance].dmm_tx_cnt = 0;
    instance_data[instance].dmr_rx_cnt = 0;
    instance_data[instance].dmr_rx_tout_cnt = 0; 
    instance_data[instance].dmr_rx_err_cnt = 0;
    instance_data[instance].dmm_late_txtime_cnt = 0;

    if (instance_data[instance].dmm_config.syncronized == TRUE) {            
        dm1_far_to_near_get(instance, &dummy, &dummy, &status->ow_ftn_rx_err_counter, status->ow_ftn_delays);
        dm1_near_to_far_get(instance, &dummy, &dummy, &status->ow_ntf_rx_err_counter, status->ow_ntf_delays);
    }

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);





}

u32 vtss_mep_supp_dm1_get(const u32                        instance,
                          vtss_mep_supp_dm1_status *const  status)
{
    u32 dummy;

    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)      return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    dm1_near_to_far_get(instance, &status->tx_counter, &dummy, &dummy, status->delays);
    dm1_far_to_near_get(instance, &dummy, &status->rx_counter, &status->rx_err_counter, status->delays);

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}


u32 vtss_mep_supp_ais_conf_set(const u32                        instance,
                               const vtss_mep_supp_ais_conf_t   *const conf)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)      return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    instance_data[instance].ais_config = *conf;

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_ais_set(const u32  instance,
                          const BOOL enable)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                           return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (enable && instance_data[instance].ais_enable)                    return(VTSS_MEP_SUPP_RC_OK);
    if (enable && !instance_data[instance].ais_config.flow_count)        return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_AIS_SET;
    instance_data[instance].ais_enable = enable;

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_lck_conf_set(const u32                        instance,
                               const vtss_mep_supp_lck_conf_t   *const conf)
{
    if (instance >= VTSS_MEP_SUPP_CREATED_MAX)                                                      return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);
    if (!instance_data[instance].lck_config.enable && !conf->enable)                                return(VTSS_MEP_SUPP_RC_OK);
    if (!memcmp(&instance_data[instance].lck_config, conf, sizeof(vtss_mep_supp_lck_conf_t)))       return(VTSS_MEP_SUPP_RC_OK);
    if (conf->enable && (conf->flow_count > VTSS_MEP_SUPP_CLIENT_FLOWS_MAX))                        return(VTSS_MEP_SUPP_RC_OK);

    vtss_mep_supp_crit_lock();

    instance_data[instance].event_flags |= EVENT_IN_LCK_CONFIG;
    if (conf->enable)   instance_data[instance].lck_config = *conf;
    else                instance_data[instance].lck_config.enable = FALSE;

    vtss_mep_supp_run();

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}

u32 vtss_mep_supp_loc_state(u32 instance,   BOOL state)
{
/* This is called when LOC state has changed by SW poll of ACL */
    BOOL new_state;

    supp_instance_data_t  *data;

    vtss_mep_supp_crit_lock();

    data = &instance_data[instance];    /* Instance data reference */

    if ((data->ccm_gen.enable) && hw_ccm_calc(data->ccm_config.period))
    {
        new_state = (state) ? 0x02 : 0x00;
        if (new_state != (data->defect_state.dLoc[0] & 0x02))
        { /* Change in hw detected LOC */
            if (new_state)  data->defect_state.dLoc[0] |= 0x2;
            else            data->defect_state.dLoc[0] &= ~0x2;

            if (data->defect_state.dLoc[0])    data->defect_state.dRdi[0] = FALSE;
            data->event_flags |= EVENT_OUT_NEW_DEFECT;

            vtss_mep_supp_run();
        }
    }

    vtss_mep_supp_crit_unlock();

    return(VTSS_MEP_SUPP_RC_OK);
}


BOOL vtss_mep_supp_hw_ccm_generator_get(vtss_mep_supp_period_t  period)
{
    BOOL hw_gen;

    vtss_mep_supp_crit_lock();

    hw_gen = hw_ccm_calc(period);

    vtss_mep_supp_crit_unlock();

    return (hw_gen);
}

void vtss_mep_supp_protection_change(u32 instance)
{
    supp_instance_data_t  *data;

    /* Port protection has changed meaning AFI generation might be moved to other port - This is only Serval down MEP */

    vtss_mep_supp_crit_lock();

    data = &instance_data[instance];    /* Instance data reference */

    if (!data->config.direction == VTSS_MEP_SUPP_DOWN) {
        vtss_mep_supp_crit_unlock();
        return;
    }

    if ((data->ccm_gen.enable) && hw_ccm_calc(data->ccm_gen.cc_period)) {  /* AFI based CCM is enabled - restart */
        data->event_flags |= EVENT_IN_CCM_GEN;
        vtss_mep_supp_run();
    }
    if ((data->lbm_config.enable) && (data->lbm_config.to_send == VTSS_MEP_SUPP_LBM_TO_SEND_INFINITE))  /* AFI based LBM is enabled - restart */
    {
        if (data->tx_lbm != NULL) {
            vtss_mep_supp_packet_tx_cancel(instance, data->tx_lbm);
            data->tx_lbm = NULL;
        }
        data->event_flags |= EVENT_IN_LBM_CONFIG;
        vtss_mep_supp_run();
    }
    if (data->tst_config.enable)  {    /* AFI based TST is enabled - restart */
        data->event_flags |= EVENT_IN_TST_CONFIG;
        vtss_mep_supp_run();
    }

    vtss_mep_supp_crit_unlock();
}

/****************************************************************************/
/*  MEP platform interface                                                  */
/****************************************************************************/

void vtss_mep_supp_timer_thread(BOOL  *const stop)
{
    u32  i, j;
    BOOL run;
    supp_instance_data_t  *data;

    run = FALSE;
    *stop = TRUE;

    vtss_mep_supp_crit_lock();

    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; ++i)
    {
        if (!instance_data[i].config.enable)    continue;

        data = &instance_data[i];    /* Instance data reference */

        if (data->defect_timer_active)
        {
            if (data->dLevel.timer)
            {
                data->dLevel.timer--;
                if (!data->dLevel.timer)
                {
                    data->event_flags |= EVENT_IN_DEFECT_TIMER;
                    run = TRUE;
                }
                else  *stop = FALSE;
            }
            if (data->dMeg.timer)
            {
                data->dMeg.timer--;
                if (!data->dMeg.timer)
                {
                    data->event_flags |= EVENT_IN_DEFECT_TIMER;
                    run = TRUE;
                }
                else  *stop = FALSE;
            }
            if (data->dMep.timer)
            {
                data->dMep.timer--;
                if (!data->dMep.timer)
                {
                    data->event_flags |= EVENT_IN_DEFECT_TIMER;
                    run = TRUE;
                }
                else  *stop = FALSE;
            }
            for (j=0; j<data->ccm_config.peer_count; ++j)
            {
                if (data->dPeriod[j].timer)
                {
                    data->dPeriod[j].timer--;
                    if (!data->dPeriod[j].timer)
                    {
                        data->event_flags |= EVENT_IN_DEFECT_TIMER;
                        run = TRUE;
                    }
                    else  *stop = FALSE;
                }
                if (data->dPrio[j].timer)
                {
                    data->dPrio[j].timer--;
                    if (!data->dPrio[j].timer)
                    {
                        data->event_flags |= EVENT_IN_DEFECT_TIMER;
                        run = TRUE;
                    }
                    else  *stop = FALSE;
                }
            }
            if (data->dAis.timer)
            {
                data->dAis.timer--;
                if (!data->dAis.timer)
                {
                    data->event_flags |= EVENT_IN_DEFECT_TIMER;
                    run = TRUE;
                }
                else  *stop = FALSE;
            }
            if (data->dLck.timer)
            {
                data->dLck.timer--;
                if (!data->dLck.timer)
                {
                    data->event_flags |= EVENT_IN_DEFECT_TIMER;
                    run = TRUE;
                }
                else  *stop = FALSE;
            }
        }/* Defect active */
        for (j=0; j<data->ccm_config.peer_count; ++j) {
            if (data->rx_ccm_timer[j]) {
                data->rx_ccm_timer[j]--;
                if (!data->rx_ccm_timer[j]) {
                    data->event_flags |= EVENT_IN_RX_CCM_TIMER;
                    run = TRUE;
                }
                else  *stop = FALSE;
            }
        }
        if (data->tx_ccm_timer)
        {
            data->tx_ccm_timer--;
            if (!data->tx_ccm_timer)
            {
                data->event_flags |= EVENT_IN_TX_CCM_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->tx_aps_timer)
        {
            data->tx_aps_timer--;
            if (!data->tx_aps_timer)
            {
                data->event_flags |= EVENT_IN_TX_APS_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->tx_lmm_timer)
        {
            data->tx_lmm_timer--;
            if (!data->tx_lmm_timer)
            {
                data->event_flags |= EVENT_IN_TX_LMM_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->tx_lbm_timer)
        {
            data->tx_lbm_timer--;
            if (!data->tx_lbm_timer)
            {
                data->event_flags |= EVENT_IN_TX_LBM_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->tx_dmm_timer)
        {
            data->tx_dmm_timer--;
            if (!data->tx_dmm_timer)
            {
                data->event_flags |= EVENT_IN_TX_DMM_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->tx_dmm_timeout_timer)
        {
            data->tx_dmm_timeout_timer--;
            if (!data->tx_dmm_timeout_timer)
            {
                data->event_flags |= EVENT_IN_RX_DMR_TIMER;
                run = TRUE;
            }       
            else  *stop = FALSE;
        }
        if (data->tx_dm1_timer)
        {
            data->tx_dm1_timer--;
            if (!data->tx_dm1_timer)
            {
                data->event_flags |= EVENT_IN_TX_1DM_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->tx_ais_timer)
        {
            data->tx_ais_timer--;
            if (!data->tx_ais_timer)
            {
                data->event_flags |= EVENT_IN_TX_AIS_TIMER;
                run = TRUE;
            }
            else *stop = FALSE;
        }
        if (data->tx_lck_timer)
        {
            data->tx_lck_timer--;
            if (!data->tx_lck_timer)
            {
                data->event_flags |= EVENT_IN_TX_LCK_TIMER;
                run = TRUE;
            }
            else *stop = FALSE;
        }
        if (data->rx_aps_timer)
        {
            data->rx_aps_timer--;
            if (!data->rx_aps_timer)
            {
                data->event_flags |= EVENT_IN_RX_APS_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->rx_lbm_timer)
        {
            data->rx_lbm_timer--;
            if (!data->rx_lbm_timer)
            {
                data->event_flags |= EVENT_IN_RX_LBM_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->rdi_timer)
        {
            data->rdi_timer--;
            if (!data->rdi_timer)
            {
                data->event_flags |= EVENT_IN_RDI_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
        if (data->tst_timer)
        {
            data->tst_timer--;
            if (!data->tst_timer)
            {
                data->event_flags |= EVENT_IN_TST_TIMER;
                run = TRUE;
            }
            else  *stop = FALSE;
        }
    }

    if (run)    vtss_mep_supp_run();

#ifdef VTSS_ARCH_SERVAL
    static u32 voe_fast_poll_timer = 0;
    static u32 voe_slow_poll_timer = 0;
    if (++voe_fast_poll_timer >= (100/timer_res)) {  /* VOE "fast" polling */
        voe_fast_poll_timer = 0;
        for (i=0; i<VTSS_OAM_VOE_CNT; ++i) {
            if ((voe_to_mep[i] < VTSS_MEP_SUPP_CREATED_MAX) && voe_ccm_enable(&instance_data[voe_to_mep[i]])) { /* This is a VOE based CCM - VOE event is utilized */
                if (instance_data[voe_to_mep[i]].ccm_defect_active)
                    (void)vtss_oam_voe_ccm_arm_hitme(NULL, i, TRUE);    /* CCM defect is active - CCM PDU is polled "fast" in order to check for defect going passive */
            }
            if (voe_to_mep[i] < VTSS_MEP_SUPP_CREATED_MAX) /* This is a active VOE - do the one-bit counter accumulation on Serval-1 */
                (void)vtss_oam_voe_counter_update_serval(NULL, i);
        }
    }

    if (++voe_slow_poll_timer >= (1000/timer_res)) {  /* VOE "slow" polling */
        voe_slow_poll_timer = 0;
        for (i=0; i<VTSS_OAM_VOE_CNT; ++i) {
            if ((voe_to_mep[i] < VTSS_MEP_SUPP_CREATED_MAX) && voe_ccm_enable(&instance_data[voe_to_mep[i]])) { /* This is a VOE based CCM - VOE event is utilized */
                if (!instance_data[voe_to_mep[i]].ccm_defect_active)
                    (void)vtss_oam_voe_ccm_arm_hitme(NULL, i, TRUE);   /* No CCM defect is active - CCM PDU is polled "slow" in order to learn Peer MAC */
                (void)vtss_oam_voe_event_enable(NULL, i, VOE_EVENT_MASK, TRUE); /* Enable all VOE CCM events */
            }
        }
    }

    *stop = FALSE;
#endif
    vtss_mep_supp_crit_unlock();
}

static u8  null_aps[VTSS_MEP_SUPP_RAPS_DATA_LENGTH] = {0,0,0,0,0,0,0,0};

void vtss_mep_supp_run_thread(void)
{
    u32 i;

    vtss_mep_supp_crit_lock();

    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; ++i)
    {
        if (instance_data[i].event_flags & EVENT_IN_MASK)
        {/* New input */
            if (instance_data[i].event_flags & EVENT_IN_CONFIG)         run_async_config(i);
            if (instance_data[i].event_flags & EVENT_IN_CCM_CONFIG)     run_ccm_config(i);
            if (instance_data[i].event_flags & EVENT_IN_CCM_GEN)        run_async_ccm_gen(i);
            if (instance_data[i].event_flags & EVENT_IN_CCM_RDI)        run_ccm_rdi(i);
            if (instance_data[i].event_flags & EVENT_IN_LMM_CONFIG)     run_async_lmm_config(i);
            if (instance_data[i].event_flags & EVENT_IN_DMM_CONFIG)     run_dmm_config(i);
            if (instance_data[i].event_flags & EVENT_IN_1DM_CONFIG)     run_1dm_config(i);
            if (instance_data[i].event_flags & EVENT_IN_APS_CONFIG)     run_async_aps_config(i);
            if (instance_data[i].event_flags & EVENT_IN_APS_TXDATA)     run_aps_txdata(i);
#ifdef VTSS_ARCH_SERVAL
            if (instance_data[i].event_flags & EVENT_IN_APS_FORWARD)    run_aps_forward(i);
#endif
            if (instance_data[i].event_flags & EVENT_IN_LTM_CONFIG)     run_ltm_config(i);
            if (instance_data[i].event_flags & EVENT_IN_LBM_CONFIG)     run_lbm_config(i);
            if (instance_data[i].event_flags & EVENT_IN_LCK_CONFIG)     run_lck_config(i);
            if (instance_data[i].event_flags & EVENT_IN_TST_CONFIG)     run_tst_config(i);
            if (instance_data[i].event_flags & EVENT_IN_AIS_SET)        run_ais_set(i);
            if (instance_data[i].event_flags & EVENT_IN_DEFECT_TIMER)   run_defect_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_RX_CCM_TIMER)   run_rx_ccm_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_TX_CCM_TIMER)   run_tx_ccm_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_TX_APS_TIMER)   run_tx_aps_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_TX_LMM_TIMER)   run_tx_lmm_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_TX_LBM_TIMER)   run_tx_lbm_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_RX_DMR_TIMER)   run_rx_dmr_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_TX_DMM_TIMER)   run_tx_dmm_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_TX_1DM_TIMER)   run_tx_dm1_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_RX_APS_TIMER)   run_rx_aps_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_RX_LBM_TIMER)   run_rx_lbm_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_TX_AIS_TIMER)   run_tx_ais_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_TX_LCK_TIMER)   run_tx_lck_timer(i);
            if (instance_data[i].event_flags & EVENT_IN_RDI_TIMER)      run_rdi_timer(i);
#ifdef VTSS_ARCH_SERVAL
            if (instance_data[i].event_flags & EVENT_IN_TST_TIMER)      run_tst_timer(i);
#endif
        }

        instance_data[i].voe_config = FALSE;

        if (instance_data[i].event_flags & EVENT_OUT_MASK)
        {/* New output */
            /* Now do unlocked call-out to avoid any deadlock */
            if (instance_data[i].event_flags & EVENT_OUT_NEW_DEFECT)
            {   /* Deliver new CCM state */
                instance_data[i].event_flags &= ~EVENT_OUT_NEW_DEFECT;

                vtss_mep_supp_crit_unlock();
                vtss_mep_supp_new_defect_state(i);
                vtss_mep_supp_crit_lock();
            }
            if (instance_data[i].event_flags & EVENT_OUT_NEW_APS)
            {   /* Deliver new APS */
                instance_data[i].event_flags &= ~EVENT_OUT_NEW_APS;

                vtss_mep_supp_crit_unlock();
                vtss_mep_supp_new_aps(i, null_aps);
                vtss_mep_supp_crit_lock();
            }
            if (instance_data[i].event_flags & EVENT_OUT_NEW_DMR)
            {   /* Deliver new DMR */
                instance_data[i].event_flags &= ~EVENT_OUT_NEW_DMR;

                vtss_mep_supp_crit_unlock();
                vtss_mep_supp_new_dmr(i);
                vtss_mep_supp_crit_lock();
            }
            if (instance_data[i].event_flags & EVENT_OUT_NEW_DM1)
            {   /* Deliver new DM1 */
                instance_data[i].event_flags &= ~EVENT_OUT_NEW_DM1;

                vtss_mep_supp_crit_unlock();
                vtss_mep_supp_new_dm1(i);
                vtss_mep_supp_crit_lock();
            }
        }
    }

    vtss_mep_supp_crit_unlock();
}

u32 vtss_mep_supp_init(const u32 timer_resolution, u32 up_mep_loop_port)
{
    u32  i;

#ifdef VTSS_ARCH_SERVAL
    vtss_mac_t   multicast_addr = {{0x01,0x80,0xC2,0x00,0x00,0x30}};
    vtss_packet_rx_queue_t rx_queue = vtss_mep_oam_cpu_queue_get();
    vtss_oam_vop_conf_t cfg;
#endif

    voe_up_mep_loop_port = up_mep_loop_port;
    if ((timer_resolution == 0) || (timer_resolution > 100))     return(VTSS_MEP_SUPP_RC_INVALID_PARAMETER);

    timer_res = timer_resolution;
    memset(lm_null_counter, 0, 12);
    init_pdu_period_to_timer();

    /* Initialize all instance data */
    for (i=0; i<VTSS_MEP_SUPP_CREATED_MAX; ++i)
        instance_data_clear(i,  &instance_data[i]);

#ifdef VTSS_ARCH_SERVAL
    memset(voe_idx_used, 0, sizeof(voe_idx_used));
    memset(mce_idx_used, 0, sizeof(mce_idx_used));

    for (i=0; i<VTSS_OAM_VOE_CNT; ++i)
        voe_to_mep[i] = VTSS_MEP_SUPP_CREATED_MAX;

    (void)vtss_oam_vop_conf_get(NULL, &cfg);

    cfg.enable_all_voe = TRUE;
    cfg.external_cpu_portmask = 1<<voe_up_mep_loop_port;
    memcpy(cfg.common_multicast_dmac.addr, multicast_addr.addr, sizeof(cfg.common_multicast_dmac.addr));
    cfg.ccm_lm_enable_rx_fcf_in_reserved_field = FALSE;
    cfg.down_mep_lmr_proprietary_fcf_use = TRUE;
    cfg.sdlb_cpy_copy_idx = 0;

    cfg.pdu_type.generic[GENERIC_AIS].opcode = VTSS_OAM_OPCODE_AIS;
    cfg.pdu_type.generic[GENERIC_AIS].check_dmac = TRUE;
    cfg.pdu_type.generic[GENERIC_AIS].extract.to_front = FALSE;
    cfg.pdu_type.generic[GENERIC_AIS].extract.rx_queue = rx_queue;
    cfg.pdu_type.generic[GENERIC_LCK].opcode = VTSS_OAM_OPCODE_LCK;
    cfg.pdu_type.generic[GENERIC_LCK].check_dmac = TRUE;
    cfg.pdu_type.generic[GENERIC_LCK].extract.to_front = FALSE;
    cfg.pdu_type.generic[GENERIC_LCK].extract.rx_queue = rx_queue;
    cfg.pdu_type.generic[GENERIC_LAPS].opcode = VTSS_OAM_OPCODE_LINEAR_APS;
    cfg.pdu_type.generic[GENERIC_LAPS].check_dmac = TRUE;
    cfg.pdu_type.generic[GENERIC_LAPS].extract.to_front = FALSE;
    cfg.pdu_type.generic[GENERIC_LAPS].extract.rx_queue = rx_queue;
    cfg.pdu_type.generic[GENERIC_RAPS].opcode = VTSS_OAM_OPCODE_RING_APS;
    cfg.pdu_type.generic[GENERIC_RAPS].check_dmac = FALSE;
    cfg.pdu_type.generic[GENERIC_RAPS].extract.to_front = FALSE;
    cfg.pdu_type.generic[GENERIC_RAPS].extract.rx_queue = rx_queue;
    cfg.pdu_type.ccm.to_front = FALSE;
    cfg.pdu_type.ccm.rx_queue = rx_queue;
    cfg.pdu_type.ccm_lm.to_front = FALSE;
    cfg.pdu_type.ccm_lm.rx_queue = rx_queue;
    cfg.pdu_type.lt.to_front = FALSE;
    cfg.pdu_type.lt.rx_queue = rx_queue;
    cfg.pdu_type.dmm.to_front = FALSE;
    cfg.pdu_type.dmm.rx_queue = rx_queue;
    cfg.pdu_type.dmr.to_front = FALSE;
    cfg.pdu_type.dmr.rx_queue = rx_queue;
    cfg.pdu_type.lmm.to_front = FALSE;
    cfg.pdu_type.lmm.rx_queue = rx_queue;
    cfg.pdu_type.lmr.to_front = FALSE;
    cfg.pdu_type.lmr.rx_queue = rx_queue;
    cfg.pdu_type.lbm.to_front = FALSE;
    cfg.pdu_type.lbm.rx_queue = rx_queue;
    cfg.pdu_type.lbr.to_front = FALSE;
    cfg.pdu_type.lbr.rx_queue = rx_queue - 1; // TST and LBR frames go into a lower prioritized queue in order not to disturb 1DM/DMR frames.
    cfg.pdu_type.err.to_front = FALSE;
    cfg.pdu_type.err.rx_queue = rx_queue;
    cfg.pdu_type.other.to_front = FALSE;
    cfg.pdu_type.other.rx_queue = rx_queue;

    (void)vtss_oam_vop_conf_set(NULL,  &cfg);
#endif

    return(VTSS_MEP_SUPP_RC_OK);
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
