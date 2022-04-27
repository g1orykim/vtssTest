/*

 Vitesse Switch Software.

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
/* vtss_tod_mod_man.c */
/*lint -esym(766, vtss_options.h) */
/*lint -esym(766, vtss_silabs_clk_api.h) */

#include "vtss_options.h"
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)

#include "vtss_tod_mod_man.h"
#include "vtss_tod_api.h"
#include "tod.h"
#include "vtss_tod_phy_engine.h"
#include "interrupt_api.h"
#if defined VTSS_SW_OPTION_PTP
#include "ptp_api.h"
#endif
#include "phy_api.h"
#include "port_api.h"





#include "vtss_silabs_clk_api.h"
/* include pcos interface */
#include "main.h"
#include "critd_api.h"
#include "tod_api.h"


#define API_INST_DEFAULT PHY_INST 
#define LOCK_TRACE_LEVEL VTSS_TRACE_LVL_NOISE
/* Note: The serial TOD feature in Gen2 phy's only works properly if the PHY_CONF_AUTO_CLEAR_LS = TRUE */
#define PHY_CONF_AUTO_CLEAR_LS = TRUE           /* TRUE if auto clear of Load/Save in the gen2 phy's are enabled */


/****************************************************************************
 * PTP Module Manager thread
 ****************************************************************************/

static cyg_handle_t ptp_module_man_thread_handle;
static cyg_thread   ptp_module_man_thread_block;
static char         ptp_module_man_thread_stack[THREAD_DEFAULT_STACK_SIZE];

#define CTLFLAG_PTP_MOD_MAN_1PPS           (1 << 0)
#define CTLFLAG_PTP_MOD_MAN_SETTING_TIME   (1 << 1)

static cyg_flag_t   module_man_thread_flags; /* PTP module manager thread control */

static critd_t mod_man_mutex;          /* clock internal data protection */

#define MOD_MAN_LOCK()        critd_enter(&mod_man_mutex, _C, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define MOD_MAN_UNLOCK()      critd_exit (&mod_man_mutex, _C, LOCK_TRACE_LEVEL, __FILE__, __LINE__)


/**
 * \brief Module manager add a slave timecounter.
 * \note this function is used to manage a list of slave timecounters that are adjusted simultaneously.
 *
 * \param time_cnt [IN]  Timecounter type, currently only VTSS_TS_TIMECOUNTER_PHY is implemented.
 * \param inst     [IN]  API instance parameter (NULL = default API).
 * \param port_no  [IN]  port number that the timecounter is assigned to.
 * \param one_pps_load_latency [IN]  The latency from master 1PPS edge until the internal timer in the PHY is 
 *                              loaded (the internal latency in the Phy is 2 clock cycles.
 * \param one_pps_save_latency [IN]  The latency from master 1PPS edge until the internal timer in the PHY is 
 *                              saved (the internal latency in the Phy is 1 clock cycles.
 *
 * \return Return code.
 **/
static vtss_rc ptp_module_man_time_slave_timecounter_add(vtss_ts_timecounter_type_t time_cnt, 
        vtss_inst_t    inst,
        vtss_port_no_t port_no,
        vtss_timeinterval_t one_pps_load_latency,
        vtss_timeinterval_t one_pps_save_latency);

/**
 * \brief Module manager delete a slave timecounter.
 *
 * \return Return code.
 **/
static vtss_rc ptp_module_man_time_slave_timecounter_delete(vtss_port_no_t port_no);


/****************************************************************************
 * Module Manager port data used to maintain the list of ports that supports
 * PHY timestamping
 ****************************************************************************/
static struct {
    vtss_tod_ts_phy_topo_t topo;                /* Phy timestamp topology info */



    BOOL phy_init;                              /* true when the PHY timestamper for the port has been initialized */
} port_data [VTSS_PORTS];


#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static const i64 clk_mhz [VTSS_PHY_TS_CLOCK_FREQ_MAX] = {125000000LL, 156250000LL, 200000000LL, 250000000LL, 500000000LL};

vtss_rc vtss_tod_phy_ts_clk_info_10g(vtss_phy_ts_clockfreq_t *freq,
                                     vtss_phy_ts_clock_src_t *src,
                                     vtss_timeinterval_t *pps_load_delay,
                                     vtss_timeinterval_t *pps_save_delay)
{
    vtss_rc rc = VTSS_RC_OK;
#if defined(VTSS_ARCH_JAGUAR_1)
    if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
        if (tod_ref_clock_freg_get(freq)) {
            *src = VTSS_PHY_TS_CLOCK_SRC_EXTERNAL;
        } else {
            rc = VTSS_RC_ERROR;
            *freq = VTSS_PHY_TS_CLOCK_FREQ_MAX;
        }
    } else
#endif
    {
#if defined(VTSS_PHY_TS_SILABS_CLK_DLL)
        *freq = VTSS_PHY_TS_CLOCK_FREQ_250M;
        *src = VTSS_PHY_TS_CLOCK_SRC_EXTERNAL;
#else
        *freq = VTSS_PHY_TS_CLOCK_FREQ_15625M;
        *src = VTSS_PHY_TS_CLOCK_SRC_CLIENT_RX;
#endif
    }
    if (*freq < VTSS_PHY_TS_CLOCK_FREQ_MAX) {
        *pps_load_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*4LL)/clk_mhz[*freq];
        *pps_save_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*3LL)/clk_mhz[*freq];
    } else {
        rc = VTSS_RC_ERROR;
    }
    return rc;
}
vtss_rc vtss_tod_phy_ts_clk_info_1g(vtss_phy_ts_clockfreq_t *freq,
                                 vtss_phy_ts_clock_src_t *src,
                                 vtss_timeinterval_t *pps_load_delay,
                                 vtss_timeinterval_t *pps_save_delay)
{
    vtss_rc rc = VTSS_RC_OK;
    *freq = VTSS_PHY_TS_CLOCK_FREQ_MAX;
#if defined(VTSS_ARCH_SERVAL)
    *freq = VTSS_PHY_TS_CLOCK_FREQ_250M;
    *src = VTSS_PHY_TS_CLOCK_SRC_INTERNAL;
    /* default 1pps latency for the phy is 2 clock cycles (in the serval environment the delay is split into output latency from Serval and input latency in the PHY) */
    *pps_load_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*3LL)/clk_mhz[*freq];
    *pps_save_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*2LL)/clk_mhz[*freq];
#else
    if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
        if (tod_ref_clock_freg_get(freq)) {
            //*freq = VTSS_PHY_TS_CLOCK_FREQ_250M;
            *src = VTSS_PHY_TS_CLOCK_SRC_EXTERNAL;
            /* default 1pps latency for the phy is 3 clock cycles */
            if (*freq < VTSS_PHY_TS_CLOCK_FREQ_MAX) {
                *pps_load_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*4LL)/clk_mhz[*freq];
                *pps_save_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*3LL)/clk_mhz[*freq];
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else {
            rc = VTSS_RC_ERROR;
        }
    } else {
#if defined(VTSS_PHY_TS_SILABS_CLK_DLL)
        *freq = VTSS_PHY_TS_CLOCK_FREQ_250M;
        *src = VTSS_PHY_TS_CLOCK_SRC_INTERNAL;
        /* default 1pps latency for the phy is 3 clock cycles */
        *pps_load_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*4LL)/clk_mhz[*freq];
        *pps_save_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*3LL)/clk_mhz[*freq];
#else
        *freq = VTSS_PHY_TS_CLOCK_FREQ_125M;
        *src = VTSS_PHY_TS_CLOCK_SRC_CLIENT_TX;
        /* default 1pps latency for the phy is 3 clock cycles */
        *pps_load_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*4LL)/clk_mhz[*freq];
        *pps_save_delay = (((vtss_timeinterval_t)VTSS_ONE_MIA<<16)*3LL)/clk_mhz[*freq];
#endif /* defined(VTSS_PHY_TS_SILABS_CLK_DLL) */
    }
#endif /* defined(VTSS_ARCH_SERVAL) */
    return rc;
}
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static void phy_ts_internal_mode(vtss_phy_ts_rxtimestamp_len_t *ts_len, vtss_phy_ts_tc_op_mode_t *tc_mode)
{
    vtss_tod_internal_tc_mode_t mode;
    if (tod_tc_mode_get(&mode)) {
        switch (mode) {
            case VTSS_TOD_INTERNAL_TC_MODE_30BIT: 
                *ts_len = VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT;
                *tc_mode = VTSS_PHY_TS_TC_OP_MODE_B;
                break;
            case VTSS_TOD_INTERNAL_TC_MODE_32BIT:
                *ts_len = VTSS_PHY_TS_RX_TIMESTAMP_LEN_32BIT;
                *tc_mode = VTSS_PHY_TS_TC_OP_MODE_B;
                break;
            case VTSS_TOD_INTERNAL_TC_MODE_44BIT:
                *ts_len = VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT;
                *tc_mode = VTSS_PHY_TS_TC_OP_MODE_A;
                break;
            case VTSS_TOD_INTERNAL_TC_MODE_48BIT:
                *ts_len = VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT;
                *tc_mode = VTSS_PHY_TS_TC_OP_MODE_C;
                T_WG(VTSS_TRACE_GRP_PHY_TS, "Mode 48BIT only supported in Gen2 PHY's");
                break;
            default: 
                *ts_len = VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT;
                *tc_mode = VTSS_PHY_TS_TC_OP_MODE_A;
                T_WG(VTSS_TRACE_GRP_PHY_TS, "Mode not supported");
                break;
        }
    }
}
#endif /* (VTSS_FEATURE_PHY_TIMESTAMP) */

static void port_data_conf_set(vtss_port_no_t port_no)
{
    BOOL phy_ts_port = FALSE;
    vtss_rc rc = VTSS_RC_OK;
#if defined(VTSS_FEATURE_10G)
    vtss_phy_10g_id_t phy_id;
    vtss_phy_10g_mode_t  phy_10g_mode;
    vtss_gpio_10g_gpio_mode_t  gpio_mode;
#endif /* VTSS_FEATURE_10G */
    vtss_phy_type_t phy_id_1g;
    vtss_phy_ts_init_conf_t  phy_conf;
    char str1 [30];
    vtss_timeinterval_t one_pps_load_latency;
    vtss_timeinterval_t one_pps_save_latency;
    vtss_phy_ts_clockfreq_t clk_freq;
    vtss_phy_ts_clock_src_t clk_src;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)



    vtss_phy_ts_rxtimestamp_len_t ts_len = 0;
    vtss_phy_ts_tc_op_mode_t tc_mode = 0;
    phy_ts_internal_mode(&ts_len, &tc_mode);
    T_NG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, ts_len %d, tc_mode %d", port_no, ts_len, tc_mode);
#endif  /* defined(VTSS_FEATURE_PHY_TIMESTAMP) */

#if defined(VTSS_ARCH_SERVAL)
    vtss_ts_operation_mode_t mode;
    BOOL backplane_ts_port = FALSE;
#endif /* defined(VTSS_ARCH_SERVAL) */

    /* is this port a 10G PHY TS port ? */
#if defined(VTSS_FEATURE_10G)
    rc = vtss_phy_10g_id_get(API_INST_DEFAULT, port_no, &phy_id);
    T_NG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, 10 G PHY test, rc %x, part_number %x, rev %x", port_no, rc, phy_id.part_number, phy_id.revision);
    if (rc == VTSS_RC_OK) {
        T_IG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, this is a 10 G PHY, part_number %x, rev %x", port_no, phy_id.part_number, phy_id.revision);
        T_IG(VTSS_TRACE_GRP_PHY_TS, "device_feature_status %x", phy_id.device_feature_status);
    }
    if (rc == VTSS_RC_OK && (((phy_id.part_number == 0x8488 || phy_id.part_number == 0x8487) &&
            phy_id.revision >= 4) || ((phy_id.part_number == 0x8489 && (phy_id.device_feature_status & VTSS_PHY_10G_TIMESTAMP_DISABLED) == 0)|| 
                                      phy_id.part_number == 0x8490 || phy_id.part_number == 0x8491))) {





        /* Get the operating mode of the Phy */
        TOD_RC(vtss_phy_10g_mode_get (API_INST_DEFAULT, port_no, &phy_10g_mode));
        if (phy_10g_mode.oper_mode != VTSS_PHY_1G_MODE || 
                phy_id.part_number == 0x8489 || phy_id.part_number == 0x8490 || phy_id.part_number == 0x8491) {
            phy_ts_port = TRUE;
            port_data[port_no].topo.port_ts_in_sync = FALSE;
            port_data[port_no].topo.ts_feature = VTSS_PTP_TS_PTS;
            if (phy_id.part_number == 0x8489 || phy_id.part_number == 0x8490 || phy_id.part_number == 0x8491) {
                port_data[port_no].topo.ts_gen = VTSS_PTP_TS_GEN_2;
            } else {
                port_data[port_no].topo.ts_gen = VTSS_PTP_TS_GEN_1;
            }
            port_data[port_no].topo.channel_id = phy_id.channel_id;
            if (phy_id.phy_api_base_no != port_no) {
                port_data[port_no].topo.port_shared = TRUE;
                port_data[port_no].topo.shared_port_no = phy_id.phy_api_base_no;
                port_data[phy_id.phy_api_base_no].topo.port_shared = TRUE;
                port_data[phy_id.phy_api_base_no].topo.shared_port_no = port_no;
            } else {
                port_data[port_no].topo.port_shared = TRUE;
                /* on the old design the port with highest number is the base port */
                port_data[port_no].topo.shared_port_no = port_no - 1; /* this is a hack TBD how the correct solution is */
                port_data[port_no - 1].topo.port_shared = TRUE;
                port_data[port_no - 1].topo.shared_port_no = port_no;
            }
            (void)vtss_tod_phy_ts_clk_info_10g(&clk_freq, &clk_src, &one_pps_load_latency, &one_pps_save_latency);
            if (!port_data[port_no].phy_init) {
                memset(&phy_conf, 0,sizeof(vtss_phy_ts_init_conf_t));
                phy_conf.clk_freq = clk_freq;
                phy_conf.clk_src = clk_src;
                phy_conf.rx_ts_pos = VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP;
                phy_conf.rx_ts_len = ts_len;
                phy_conf.tx_fifo_mode = VTSS_PHY_TS_FIFO_MODE_NORMAL;
                phy_conf.tx_ts_len = VTSS_PHY_TS_FIFO_TIMESTAMP_LEN_4BYTE;
                phy_conf.tc_op_mode = tc_mode;
                if (port_data[port_no].topo.ts_gen == VTSS_PTP_TS_GEN_2) {
                    phy_conf.auto_clear_ls = TRUE;
                }
                if (phy_id.part_number == 0x8488 || phy_id.part_number == 0x8489 || phy_id.part_number == 0x8490) {
                    phy_conf.xaui_sel_8487 = VTSS_PHY_TS_8487_XAUI_SEL_0; /* Required for 8488, 8489, 8490 */
                } else {
                    phy_conf.xaui_sel_8487 = VTSS_PHY_TS_8487_XAUI_SEL_0 | VTSS_PHY_TS_8487_XAUI_SEL_1; /* Required for 8487 and 8491 */
                }
                T_IG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, xaui_SEL_8487 = %X", port_no, phy_conf.xaui_sel_8487);







                rc = vtss_phy_ts_init(API_INST_DEFAULT, port_no, &phy_conf); 

                T_IG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, uses 1588 PHY, channel %d, phy_api_base_no %d, rc = %x", port_no, 
                     phy_id.channel_id, phy_id.phy_api_base_no, rc);
                port_data[port_no].phy_init = TRUE;
            }
            /* default 1pps latency for the phy is 3 clock cycles */
            rc = ptp_module_man_time_slave_timecounter_add(VTSS_TS_TIMECOUNTER_PHY, API_INST_DEFAULT, port_no, one_pps_load_latency, one_pps_save_latency);
            T_IG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, added to slave counter list, latency = %s, rc = %x", port_no, 
                 vtss_tod_TimeInterval_To_String(&one_pps_load_latency,str1,'.'), rc);
            /* GPIO setup: pin 1 = Load/save, if not PCB107: pin 0 = 1PPS_0, pin 11 = 1PPS_1, on PCB107 these two pins are used for SFP control */
            gpio_mode.port = phy_id.phy_api_base_no;
            gpio_mode.mode = VTSS_10G_PHY_GPIO_1588_LOAD_SAVE;
            TOD_RC(vtss_phy_10g_gpio_mode_set(API_INST_DEFAULT, phy_id.phy_api_base_no, 1, &gpio_mode));
            if (vtss_board_type() != VTSS_BOARD_JAG_PCB107_REF) {
                gpio_mode.mode = VTSS_10G_PHY_GPIO_1588_1PPS_0;
                TOD_RC(vtss_phy_10g_gpio_mode_set(API_INST_DEFAULT, phy_id.phy_api_base_no, 0, &gpio_mode));
                gpio_mode.mode = VTSS_10G_PHY_GPIO_1588_1PPS_1;
                TOD_RC(vtss_phy_10g_gpio_mode_set(API_INST_DEFAULT, phy_id.phy_api_base_no, 11, &gpio_mode));
            }
            T_IG(VTSS_TRACE_GRP_PHY_TS, "Set GPIO for port %d", phy_id.phy_api_base_no);
            T_DG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, Phy_id = %x, rev = %d, channel_id = %d, ts_feature = %d, shared = %d, shared port %d, rc = %x", port_no, 
                 phy_id.part_number, 
                 phy_id.revision,
                 port_data[port_no].topo.channel_id, 
                 port_data[port_no].topo.ts_feature, 
                 port_data[port_no].topo.port_shared, 
                 port_data[port_no].topo.shared_port_no, 
                 rc);
        } else { /* 8487/8488 in 1G mode, must be re initialized if it is later changed to 10G mode */
            port_data[port_no].phy_init = FALSE; 
        }
    } else {
#else
    {
#endif /* VTSS_FEATURE_10G */
        rc = vtss_phy_id_get(API_INST_DEFAULT, port_no, &phy_id_1g);
        T_NG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, 1 G PHY test, rc %x, part_number %x, rev %x", port_no, rc, phy_id_1g.part_number, phy_id_1g.revision);
#if defined(VTSS_ARCH_SERVAL)
        if (!tod_port_phy_ts_get(&backplane_ts_port, port_no)) {
            T_WG(VTSS_TRACE_GRP_PHY_TS, "Could not get ts port configuration,Port_no = %d", port_no);
        }
        if (backplane_ts_port) {
            mode.mode = TS_MODE_INTERNAL; /* also set the backplane mode if no TS PHY exists */
            TOD_RC(vtss_ts_operation_mode_set(NULL, port_no, &mode));
        }
#endif
        if (rc == VTSS_RC_OK && (phy_id_1g.part_number == VTSS_PHY_TYPE_8574 || phy_id_1g.part_number == VTSS_PHY_TYPE_8572
                  || phy_id_1g.part_number == VTSS_PHY_TYPE_8582 || phy_id_1g.part_number == VTSS_PHY_TYPE_8584 
                  || phy_id_1g.part_number == VTSS_PHY_TYPE_8575)) {






#if defined(VTSS_ARCH_SERVAL)
            phy_ts_port = backplane_ts_port;
#else
            phy_ts_port = TRUE;
#endif
            if (phy_ts_port) {
                port_data[port_no].topo.port_ts_in_sync = FALSE;
                port_data[port_no].topo.ts_feature = VTSS_PTP_TS_PTS;
                if (phy_id_1g.part_number == VTSS_PHY_TYPE_8582 || phy_id_1g.part_number == VTSS_PHY_TYPE_8584 
                                         || phy_id_1g.part_number == VTSS_PHY_TYPE_8575) {
                    port_data[port_no].topo.ts_gen = VTSS_PTP_TS_GEN_2;
                } else {
                    port_data[port_no].topo.ts_gen = VTSS_PTP_TS_GEN_1;
                }
                // in tesla channel 0 and 1 are channel 0 in the timestamp engines 0 and 1, channel 2,3 are channel 1 in timestamp engines 0 and 1
                port_data[port_no].topo.channel_id = phy_id_1g.channel_id/2; 
                if (phy_id_1g.phy_api_base_no != port_no) {
                    port_data[port_no].topo.port_shared = TRUE;
                    port_data[port_no].topo.shared_port_no = phy_id_1g.phy_api_base_no;
                    port_data[phy_id_1g.phy_api_base_no].topo.port_shared = TRUE;
                    port_data[phy_id_1g.phy_api_base_no].topo.shared_port_no = port_no;
                } else {
                    port_data[port_no].topo.port_shared = TRUE;
                    port_data[port_no].topo.shared_port_no = port_no + 2; /* this is a hack TBD how the correct solution is */
                    port_data[port_no + 2].topo.port_shared = TRUE;
                    port_data[port_no + 2].topo.shared_port_no = port_no;
                }
                T_NG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, 1 G PHY shared %d, shared_port %d, channel %d", port_no, port_data[port_no].topo.port_shared, port_data[port_no].topo.shared_port_no, port_data[port_no].topo.channel_id);

                (void)vtss_tod_phy_ts_clk_info_1g(&clk_freq, &clk_src, &one_pps_load_latency, &one_pps_save_latency);
                if (!port_data[port_no].phy_init) {
                    memset(&phy_conf, 0,sizeof(vtss_phy_ts_init_conf_t));
                    phy_conf.clk_freq = clk_freq;
                    phy_conf.clk_src = clk_src;
                    phy_conf.rx_ts_pos = VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP;
                    phy_conf.rx_ts_len = ts_len;
                    phy_conf.tx_fifo_mode = VTSS_PHY_TS_FIFO_MODE_NORMAL;
                    phy_conf.tx_ts_len = VTSS_PHY_TS_FIFO_TIMESTAMP_LEN_4BYTE;
                    phy_conf.tc_op_mode = tc_mode;
                    if (port_data[port_no].topo.ts_gen == VTSS_PTP_TS_GEN_2) {
                        phy_conf.auto_clear_ls = TRUE;
                    }







                    rc = vtss_phy_ts_init(API_INST_DEFAULT, port_no, &phy_conf); 

                    T_IG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, uses 1588 PHY, channel %d, rc = %x, ", port_no, 
                         port_data[port_no].topo.channel_id, rc);
                    port_data[port_no].phy_init = TRUE;
                }
                rc = ptp_module_man_time_slave_timecounter_add(VTSS_TS_TIMECOUNTER_PHY, API_INST_DEFAULT, port_no, one_pps_load_latency, one_pps_save_latency);
                T_IG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, added to slave counter list, latency = %s, rc = %x", port_no,
                     vtss_tod_TimeInterval_To_String(&one_pps_load_latency,str1,'.'), rc);
                T_DG(VTSS_TRACE_GRP_PHY_TS, "Port_no = %d, uses 1588 , Phy_id = %d, channel %d, phy_api_base_no %d, rc = %x, one_pps %s, ", port_no, 
                     phy_id_1g.part_number, phy_id_1g.channel_id, phy_id_1g.phy_api_base_no, rc, 
                     vtss_tod_TimeInterval_To_String(&one_pps_load_latency,str1,'.'));
            }
        } else {
























































                if (port_data[port_no].topo.ts_feature == VTSS_PTP_TS_PTS) {
                    rc = ptp_module_man_time_slave_timecounter_delete(port_no);
                    T_IG(VTSS_TRACE_GRP_REM_PHY, "Port_no = %d no more remote PHY, i.e.removed from slave counter list, rc = %x", port_no, rc);
                }



        }
    }
    if (!phy_ts_port) {
        port_data[port_no].topo.port_ts_in_sync = TRUE;
#if defined(VTSS_ARCH_JAGUAR_1)
        port_data[port_no].topo.ts_feature = VTSS_PTP_TS_JTS;
#elif  defined(VTSS_ARCH_LUTON26)
        port_data[port_no].topo.ts_feature = VTSS_PTP_TS_CTS;
#else
        port_data[port_no].topo.ts_feature = VTSS_PTP_TS_NONE;
#endif
        port_data[port_no].topo.channel_id = 0;
    }
    
}

static void port_data_initialize(void)
{
    int i;
    port_iter_t       pit;
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;
        port_data[i].topo.port_ts_in_sync = TRUE;
        port_data[i].topo.ts_gen = VTSS_PTP_TS_GEN_NONE;

#if defined(VTSS_ARCH_JAGUAR_1)
        port_data[i].topo.ts_feature = VTSS_PTP_TS_JTS;
#elif  defined(VTSS_ARCH_LUTON26)
        port_data[i].topo.ts_feature = VTSS_PTP_TS_CTS;
#else
        port_data[i].topo.ts_feature = VTSS_PTP_TS_NONE;
#endif



        port_data[i].topo.channel_id = 0;
        port_data[i].topo.port_shared = FALSE;
        port_data[i].topo.shared_port_no = 0;
        port_data[i].topo.port_ts_in_sync = TRUE;
        port_data[i].phy_init = FALSE;
    }
}

void tod_ts_phy_topo_get(vtss_port_no_t port_no,
                         vtss_tod_ts_phy_topo_t *phy)
{
    MOD_MAN_LOCK();
    *phy = port_data[port_no].topo;
    MOD_MAN_UNLOCK();
}




















/****************************************************************************
 * Module Manager state machine
 ****************************************************************************/

/* Internal slave LTC state */
typedef enum {MOD_MAN_IDLE, MOD_MAN_NORM, MOD_MAN_TSYNC1, MOD_MAN_TSYNC2, MOD_MAN_SKEW1 } mod_man_state_t;



typedef struct {
    vtss_rc (*tod_set)    ( const vtss_inst_t           inst,
                            const vtss_port_no_t        port_no,
                            const vtss_phy_timestamp_t  *const ts) ;

    vtss_rc (*tod_set_done)(const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no);
    vtss_rc (*tod_arm)    ( const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no);
    vtss_rc (*tod_get)    ( const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            vtss_phy_timestamp_t  *const ts);
    vtss_rc (*tod_adj1ns) ( const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            const BOOL            incr);
    vtss_rc (*tod_rateadj)( const vtss_inst_t              inst,
                            const vtss_port_no_t           port_no,
                            const vtss_phy_ts_scaled_ppb_t *const adj);
    vtss_rc (*ts_mode_set)( const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            const BOOL            enable);
    vtss_rc (*tod_fifo_empty)(const vtss_inst_t       inst,
                            const vtss_port_no_t    port_no);

    vtss_inst_t           inst;
    vtss_port_no_t        port_no;
    mod_man_state_t       my_state;
    vtss_phy_timestamp_t  time_at_next_pps;
    vtss_timeinterval_t          one_pps_load_latency;
    vtss_timeinterval_t          one_pps_save_latency;
    BOOL                  in_sync;
    i64                   slave_skew;
} mod_man_slave_table_t;
#if defined(VTSS_ARCH_JAGUAR_1)
#define MOD_MAN_SLAVE_TABLE_SIZE 29
#else
#define MOD_MAN_SLAVE_TABLE_SIZE 8
#endif
#define MOD_MAN_USER_TABLE_SIZE 2
#define SLAVE_MAX_SKEW              400      /* max LTC slave skew that can be corrected by 1nsadj */

static mod_man_slave_table_t mod_man_slave_table[MOD_MAN_SLAVE_TABLE_SIZE];

static  vtss_module_man_in_sync_cb_t my_in_sync_cb [MOD_MAN_USER_TABLE_SIZE];

static vtss_phy_ts_scaled_ppb_t my_adj = 0; /* actual adjustment rate */

static vtss_timeinterval_t my_1pps_latency = 0; /* actual master 1pps latency */

/* forward declarations */

static void my_phy_ts_fifo_read(const vtss_inst_t              inst,
                                const vtss_port_no_t           port_no,
                                const vtss_phy_timestamp_t     *const fifo_ts,
                                const vtss_phy_ts_fifo_sig_t   *const sig,
                                void                           *cntxt,
                                const vtss_phy_ts_fifo_status_t status);


static void phy_timestamp_interrupt_handler(vtss_interrupt_source_t     source_id,
        u32                         instance_id);

static vtss_rc phy_timestamp_age(void);

static void vtss_module_man_tx_timestamp_in_sync_call_out(vtss_port_no_t port_no, BOOL in_sync);

#if defined(VTSS_PHY_TS_SILABS_CLK_DLL)
static BOOL module_man_si5326(void)
{
    vtss_phy_10g_id_t phy_id;
    vtss_rc ec;
    if (vtss_board_type() != VTSS_BOARD_JAG_PCB107_REF) {
    
        /* initialize and configure the Si5326 DLL before 1588 block
           is configured: remember this clock is present only in 10G */
        T_IG(VTSS_TRACE_GRP_MOD_MAN, "test if SILABS Clock 5326 present");
        ec = vtss_phy_10g_id_get(API_INST_DEFAULT, 24, &phy_id);
        if (ec == VTSS_RC_OK && (phy_id.part_number == 0x8488 ||
                                 phy_id.part_number == 0x8487) && phy_id.revision >= 4) {
            T_IG(VTSS_TRACE_GRP_MOD_MAN, "Start initialization of SILABS Clock 5326");
            if (si5326_dll_init() != VTSS_RC_OK) {
                T_EG(VTSS_TRACE_GRP_MOD_MAN, "Error in initialization of SILABS Clock 5326");
            }
            return TRUE;
        }
    }
    return FALSE;
}
#endif /* VTSS_PHY_TS_SILABS_CLK_DLL */


static void
ptp_module_man_thread(cyg_addrword_t data)
{
    
    enum { MODULE_MAN_MODE_CENTRAL_CLOCK, MODULE_MAN_MODE_DISTRIBUTED_CLOCK } mode = MODULE_MAN_MODE_CENTRAL_CLOCK;
    vtss_phy_timestamp_t time_at_next_load;
    vtss_phy_timestamp_t slave_time;
    vtss_timestamp_t      ts;
    cyg_tick_count_t wakeup = cyg_current_time() + (6*1000/ECOS_MSECS_PER_HWTICK); /* wait 6 sec before monitoring */
    cyg_flag_value_t flags;
    int slave_idx;
    int tick_cnt = 0;
#if defined VTSS_SW_OPTION_PTP
    vtss_ptp_ext_clock_mode_t ext_clk_mode;
#endif
    vtss_timeinterval_t my_latency ;
    
    port_phy_wait_until_ready(); /* wait until the phy's are initialized */
    /* do initialization of interrupt handling */
    TOD_RC(vtss_interrupt_source_hook_set(phy_timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_INGR_ENGINE_ERR,
                                          INTERRUPT_PRIORITY_NORMAL));
    TOD_RC(vtss_interrupt_source_hook_set(phy_timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_INGR_RW_PREAM_ERR,
                                          INTERRUPT_PRIORITY_NORMAL));
    TOD_RC(vtss_interrupt_source_hook_set(phy_timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_INGR_RW_FCS_ERR,
                                          INTERRUPT_PRIORITY_NORMAL));
    TOD_RC(vtss_interrupt_source_hook_set(phy_timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_EGR_ENGINE_ERR,
                                          INTERRUPT_PRIORITY_NORMAL));
    TOD_RC(vtss_interrupt_source_hook_set(phy_timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_EGR_RW_FCS_ERR,
                                          INTERRUPT_PRIORITY_NORMAL));
    TOD_RC(vtss_interrupt_source_hook_set(phy_timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED,
                                          INTERRUPT_PRIORITY_NORMAL));
    TOD_RC(vtss_interrupt_source_hook_set(phy_timestamp_interrupt_handler,
                                          INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW,
                                          INTERRUPT_PRIORITY_NORMAL));
    T_NG(VTSS_TRACE_GRP_MOD_MAN, "PHY timestamp interrupt installed");
    
#if !defined VTSS_SW_OPTION_PTP
    /* if the PTP module is not included, then always set the 1PPS output, to be able to synchronize the PHY's */
    vtss_ts_ext_clock_mode_t   ext_clock_mode = {TS_EXT_CLOCK_MODE_ONE_PPS_OUTPUT, FALSE, 1};
    TOD_RC(vtss_ts_external_clock_mode_set(NULL, &ext_clock_mode));
    T_IG(VTSS_TRACE_GRP_MOD_MAN, "1PPS out enabled");
#endif
#if defined(VTSS_PHY_TS_SILABS_CLK_DLL)
    BOOL silabs_present = module_man_si5326();
    int tries = 0;
    T_IG(VTSS_TRACE_GRP_MOD_MAN, "PHY 250 MHz LTC timer option");
#else
    T_IG(VTSS_TRACE_GRP_MOD_MAN, "PHY 156/125 MHz LTC timer option");
#endif /* VTSS_PHY_TS_SILABS_CLK_DLL */

    wakeup = cyg_current_time() + (25*1000/ECOS_MSECS_PER_HWTICK); /* wait 20 sec before start monitoring */
    T_IG(VTSS_TRACE_GRP_MOD_MAN, "Wait 20 sec before starting monitor");
    flags = cyg_flag_timed_wait(&module_man_thread_flags, 0xffff, 0, wakeup);
    T_IG(VTSS_TRACE_GRP_MOD_MAN, "done, starting monitor (flags = %x", flags);
    wakeup =  cyg_current_time() + (100/ECOS_MSECS_PER_HWTICK); /* wait 100 ms */
    for (;;) {

        flags = cyg_flag_timed_wait(&module_man_thread_flags, 0xffff,
                                            CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR,wakeup);

        if (flags) {
#if defined VTSS_SW_OPTION_PTP
            vtss_ext_clock_out_get(&ext_clk_mode); /* the monitoring is only done when the 1PPS output is enabled */
            T_IG(VTSS_TRACE_GRP_MOD_MAN, "clock mode 1pps %d", ext_clk_mode.clock_out_enable);
            if (ext_clk_mode.one_pps_mode ==VTSS_PTP_ONE_PPS_OUTPUT) {
#else
            {
#endif /*  VTSS_SW_OPTION_PTP */
#if defined(VTSS_PHY_TS_SILABS_CLK_DLL)
                if (!silabs_present && tries++ < 2) {
                    silabs_present = module_man_si5326();
                }
#endif /* VTSS_PHY_TS_SILABS_CLK_DLL */
                T_DG(VTSS_TRACE_GRP_MOD_MAN, "flag(s) set %x", flags);
                MOD_MAN_LOCK();
                if (flags & CTLFLAG_PTP_MOD_MAN_SETTING_TIME) {
                    T_DG(VTSS_TRACE_GRP_MOD_MAN, "SETTING_TIME signal received");
                    for (slave_idx = 0; slave_idx < MOD_MAN_SLAVE_TABLE_SIZE; slave_idx++) {
                        switch (mod_man_slave_table[slave_idx].my_state) {
                            case MOD_MAN_IDLE:
                                break;
                            case MOD_MAN_NORM:
                            case MOD_MAN_TSYNC2:
                            case MOD_MAN_SKEW1:
                                T_NG(VTSS_TRACE_GRP_MOD_MAN, "ModuleManager state = MOD_MAN_NORM");
                                    /* disable PTP operation  ( removed because it may cause packet loss)*/
                                //TOD_RC(mod_man_slave_table[slave_idx].ts_mode_set(
                                //           mod_man_slave_table[slave_idx].inst,
                                //           mod_man_slave_table[slave_idx].port_no,
                                //           FALSE));
                                vtss_module_man_tx_timestamp_in_sync_call_out(
                                    mod_man_slave_table[slave_idx].port_no, FALSE);
                                mod_man_slave_table[slave_idx].in_sync = FALSE;
                                /* set timeofday in master timecounter ?*/
                                mod_man_slave_table[slave_idx].my_state = MOD_MAN_TSYNC1;
                                break;
                            case MOD_MAN_TSYNC1:
                                T_NG(VTSS_TRACE_GRP_MOD_MAN, "ModuleManager state = MOD_MAN_TSYNC1");
                                /* no operation */
                                break;
                            default:
                                T_WG(VTSS_TRACE_GRP_MOD_MAN, "Undefined ModuleManager state");
                                break;
                        }
                    }
                }
                if (flags & CTLFLAG_PTP_MOD_MAN_1PPS) {
                    T_DG(VTSS_TRACE_GRP_MOD_MAN, "1 PPS signal received");
                    for (slave_idx = 0; slave_idx < MOD_MAN_SLAVE_TABLE_SIZE; slave_idx++) {
                        switch (mod_man_slave_table[slave_idx].my_state) {
                            case MOD_MAN_IDLE:
                                break;
                            case MOD_MAN_NORM:
                                T_NG(VTSS_TRACE_GRP_MOD_MAN, "ModuleManager state = MOD_MAN_NORM");
                                if (mode == MODULE_MAN_MODE_CENTRAL_CLOCK) {
                                    /* start clockskew adjustment */
                                    TOD_RC(vtss_ts_timeofday_next_pps_get(NULL, &ts));
                                    my_latency = mod_man_slave_table[slave_idx].one_pps_save_latency + my_1pps_latency;
                                    vtss_tod_add_TimeInterval(&ts, &ts, &my_latency);
                                    mod_man_slave_table[slave_idx].time_at_next_pps.seconds.high = 0;
                                    mod_man_slave_table[slave_idx].time_at_next_pps.seconds.low = ts.seconds;
                                    mod_man_slave_table[slave_idx].time_at_next_pps.nanoseconds = ts.nanoseconds;
                                    T_DG(VTSS_TRACE_GRP_MOD_MAN, "time_at_next_pps = %d %d:%d", 
                                         mod_man_slave_table[slave_idx].time_at_next_pps.seconds.high, 
                                         mod_man_slave_table[slave_idx].time_at_next_pps.seconds.low, 
                                         mod_man_slave_table[slave_idx].time_at_next_pps.nanoseconds);
                                    if (mod_man_slave_table[slave_idx].tod_arm) {
                                        TOD_RC(mod_man_slave_table[slave_idx].tod_arm(
                                            mod_man_slave_table[slave_idx].inst,
                                            mod_man_slave_table[slave_idx].port_no));
                                    }
                                    mod_man_slave_table[slave_idx].my_state = MOD_MAN_SKEW1;
                                }
                                break;
                            case MOD_MAN_TSYNC1:
                                T_NG(VTSS_TRACE_GRP_MOD_MAN, "ModuleManager state = MOD_MAN_TSYNC1");
                                /* get time of next 1PPS tick */
                                TOD_RC(vtss_ts_timeofday_next_pps_get(NULL, &ts));
                                my_latency = mod_man_slave_table[slave_idx].one_pps_load_latency + my_1pps_latency;
                                vtss_tod_add_TimeInterval(&ts, &ts, &my_latency);
                                time_at_next_load.seconds.high = 0;
                                time_at_next_load.seconds.low = ts.seconds;
                                time_at_next_load.nanoseconds = ts.nanoseconds;
                                /* set timeofday in all slave timecounters */
                                if (mod_man_slave_table[slave_idx].tod_set) {
                                    TOD_RC(mod_man_slave_table[slave_idx].tod_set(
                                        mod_man_slave_table[slave_idx].inst,
                                        mod_man_slave_table[slave_idx].port_no,
                                        &time_at_next_load));
                                }
                                mod_man_slave_table[slave_idx].my_state = MOD_MAN_TSYNC2;
                                
                                break;
                            case MOD_MAN_TSYNC2:
                                T_NG(VTSS_TRACE_GRP_MOD_MAN, "ModuleManager state = MOD_MAN_TSYNC2");
                                /* enable PTP operation */
                                mod_man_slave_table[slave_idx].my_state = MOD_MAN_NORM;
                                TOD_RC(mod_man_slave_table[slave_idx].ts_mode_set( 
                                           mod_man_slave_table[slave_idx].inst,
                                           mod_man_slave_table[slave_idx].port_no,
                                           TRUE));
                                if (mod_man_slave_table[slave_idx].tod_set_done) {
                                    TOD_RC(mod_man_slave_table[slave_idx].tod_set_done(
                                               mod_man_slave_table[slave_idx].inst,
                                               mod_man_slave_table[slave_idx].port_no));
                                }
                                
                                break;
                            case MOD_MAN_SKEW1:
                                T_NG(VTSS_TRACE_GRP_MOD_MAN, "ModuleManager state = MOD_MAN_SKEW1");
                                if (mode == MODULE_MAN_MODE_CENTRAL_CLOCK) {
                                    /* do clockskew adjustment */
                                    if (mod_man_slave_table[slave_idx].tod_get) {
                                        TOD_RC(mod_man_slave_table[slave_idx].tod_get(
                                            mod_man_slave_table[slave_idx].inst,
                                            mod_man_slave_table[slave_idx].port_no,
                                            &slave_time));
                                        mod_man_slave_table[slave_idx].slave_skew = (((i64)mod_man_slave_table[slave_idx].time_at_next_pps.seconds.low - (i64)slave_time.seconds.low))*(i64)VTSS_ONE_MIA +
                                                         (((i64)mod_man_slave_table[slave_idx].time_at_next_pps.nanoseconds - (i64)slave_time.nanoseconds));
                                        T_DG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, mst time: %d %d:%d, slv_time: %d %d:%d, diff %lld", 
                                                mod_man_slave_table[slave_idx].port_no, 
                                                mod_man_slave_table[slave_idx].time_at_next_pps.seconds.high, 
                                                mod_man_slave_table[slave_idx].time_at_next_pps.seconds.low, 
                                                mod_man_slave_table[slave_idx].time_at_next_pps.nanoseconds,
                                                slave_time.seconds.high, slave_time.seconds.low, 
                                                slave_time.nanoseconds, mod_man_slave_table[slave_idx].slave_skew);
                                        mod_man_slave_table[slave_idx].my_state = MOD_MAN_NORM;
                                        if (mod_man_slave_table[slave_idx].slave_skew > SLAVE_MAX_SKEW || mod_man_slave_table[slave_idx].slave_skew < -SLAVE_MAX_SKEW) {
                                            mod_man_slave_table[slave_idx].my_state = MOD_MAN_TSYNC1;
                                            /* disable PTP operation ( removed because it may cause packet loss)*/
                                            //TOD_RC(mod_man_slave_table[slave_idx].ts_mode_set(
                                            //           mod_man_slave_table[slave_idx].inst,
                                            //           mod_man_slave_table[slave_idx].port_no,
                                            //           FALSE));
                                            vtss_module_man_tx_timestamp_in_sync_call_out(
                                                mod_man_slave_table[slave_idx].port_no, FALSE);
                                            T_IG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, skew too high",mod_man_slave_table[slave_idx].port_no);
                                            mod_man_slave_table[slave_idx].in_sync = FALSE;
                                        } else if (mod_man_slave_table[slave_idx].slave_skew > 0) {
                                            /* increment slave timer*/
                                            TOD_RC(mod_man_slave_table[slave_idx].tod_adj1ns(
                                                        mod_man_slave_table[slave_idx].inst,
                                                        mod_man_slave_table[slave_idx].port_no,
                                                        TRUE));
                                            mod_man_slave_table[slave_idx].slave_skew--;
                                            mod_man_slave_table[slave_idx].slave_skew = mod_man_slave_table[slave_idx].slave_skew/2;
                                            T_IG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, increment slave timer",mod_man_slave_table[slave_idx].port_no);
                                        } else if ((mod_man_slave_table[slave_idx].slave_skew < 0)) {
                                            /* decrement slave timer */
                                            TOD_RC(mod_man_slave_table[slave_idx].tod_adj1ns(
                                                        mod_man_slave_table[slave_idx].inst,
                                                        mod_man_slave_table[slave_idx].port_no,
                                                        FALSE));
                                            mod_man_slave_table[slave_idx].slave_skew++;
                                            mod_man_slave_table[slave_idx].slave_skew = mod_man_slave_table[slave_idx].slave_skew/2;
                                            T_IG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, decrement slave timer",mod_man_slave_table[slave_idx].port_no);
                                        } else {
                                            T_DG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, no skew :-)",mod_man_slave_table[slave_idx].port_no);
                                        }
                                    }
                                    T_DG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, in_sync = %d",mod_man_slave_table[slave_idx].port_no, 
                                                mod_man_slave_table[slave_idx].in_sync);
                                    if (mod_man_slave_table[slave_idx].in_sync == FALSE && mod_man_slave_table[slave_idx].my_state == MOD_MAN_NORM) {
                                        vtss_module_man_tx_timestamp_in_sync_call_out(
                                            mod_man_slave_table[slave_idx].port_no, TRUE);
                                        mod_man_slave_table[slave_idx].in_sync = TRUE;
                                        T_DG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, in_sync set to TRUE",mod_man_slave_table[slave_idx].port_no);
                                    }
                                }
                                break;
                            default:
                                T_WG(VTSS_TRACE_GRP_MOD_MAN, "Undefined ModuleManager state");
                                break;
                        }
                    }
                }
                MOD_MAN_UNLOCK();
            }
#if defined VTSS_SW_OPTION_PTP
            else {
                MOD_MAN_LOCK();
                T_DG(VTSS_TRACE_GRP_MOD_MAN, "1PPS disabled");
                for (slave_idx = 0; slave_idx < MOD_MAN_SLAVE_TABLE_SIZE; slave_idx++) {
                    if (mod_man_slave_table[slave_idx].in_sync) {
                        vtss_module_man_tx_timestamp_in_sync_call_out(
                            mod_man_slave_table[slave_idx].port_no, FALSE);
                        mod_man_slave_table[slave_idx].in_sync = FALSE;
                        /* set timeofday in master timecounter ?*/
                        mod_man_slave_table[slave_idx].my_state = MOD_MAN_TSYNC1;
                    }
                }
                MOD_MAN_UNLOCK();
            }
#endif
            tick_cnt = 0;
        } else {
            MOD_MAN_LOCK();
            for (slave_idx = 0; slave_idx < MOD_MAN_SLAVE_TABLE_SIZE; slave_idx++) {
                if ((mod_man_slave_table[slave_idx].my_state == MOD_MAN_NORM || 
                        mod_man_slave_table[slave_idx].my_state == MOD_MAN_SKEW1) &&
                        mod_man_slave_table[slave_idx].slave_skew != 0) {

                    if (mod_man_slave_table[slave_idx].slave_skew > 0) {
                        /* increment slave timer*/
                        TOD_RC(mod_man_slave_table[slave_idx].tod_adj1ns(
                                   mod_man_slave_table[slave_idx].inst,
                                   mod_man_slave_table[slave_idx].port_no,
                                   TRUE));
                        mod_man_slave_table[slave_idx].slave_skew--;
                        T_IG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, increment slave timer, skew = %lld",mod_man_slave_table[slave_idx].port_no, mod_man_slave_table[slave_idx].slave_skew);
                    } else {
                        /* decrement slave timer */
                        TOD_RC(mod_man_slave_table[slave_idx].tod_adj1ns(
                                   mod_man_slave_table[slave_idx].inst,
                                   mod_man_slave_table[slave_idx].port_no,
                                   FALSE));
                        mod_man_slave_table[slave_idx].slave_skew++;
                        T_IG(VTSS_TRACE_GRP_MOD_MAN, "port: %d, decrement slave timer, skew = %lld",mod_man_slave_table[slave_idx].port_no, mod_man_slave_table[slave_idx].slave_skew);
                    }
                } else {
                    mod_man_slave_table[slave_idx].slave_skew = 0;
                }
            }
            MOD_MAN_UNLOCK();
            wakeup += (100/ECOS_MSECS_PER_HWTICK); /* 100 msec */
            /* timeout */
            TOD_RC(phy_timestamp_age());
            if (++tick_cnt > 18) {
                T_IG(VTSS_TRACE_GRP_MOD_MAN, "Missed a 1 PPS tick");
                tick_cnt = 0;
            }
        }


    }
}

vtss_rc ptp_module_man_trig(BOOL ongoing_adj)
{
/* trig ModuleManager */
    T_DG(VTSS_TRACE_GRP_MOD_MAN, "Ongoing adj = %d",ongoing_adj);

    cyg_flag_setbits(&module_man_thread_flags, ongoing_adj ? CTLFLAG_PTP_MOD_MAN_SETTING_TIME : CTLFLAG_PTP_MOD_MAN_1PPS);
    return VTSS_RC_OK;
}

vtss_rc ptp_module_man_init(void)
{
    vtss_rc rc = VTSS_OK;
    int ix;
    critd_init(&mod_man_mutex, "mod_man_mutex", VTSS_MODULE_ID_TOD, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      ptp_module_man_thread,
                      0,
                      "PTP_module_man",
                      ptp_module_man_thread_stack,
                      sizeof(ptp_module_man_thread_stack),
                      &ptp_module_man_thread_handle,
                      &ptp_module_man_thread_block);
    cyg_flag_init( &module_man_thread_flags );
    /* initialize moduleman table */
    for (ix = 0; ix < MOD_MAN_SLAVE_TABLE_SIZE; ++ix) {
        mod_man_slave_table[ix].tod_set = NULL;
        mod_man_slave_table[ix].tod_arm = NULL;
        mod_man_slave_table[ix].tod_set_done = NULL;
        mod_man_slave_table[ix].tod_get = NULL;
        mod_man_slave_table[ix].tod_adj1ns = NULL;
        mod_man_slave_table[ix].tod_rateadj = NULL;
        mod_man_slave_table[ix].tod_fifo_empty = NULL;
        mod_man_slave_table[ix].inst = 0;
        mod_man_slave_table[ix].port_no = 0;
        mod_man_slave_table[ix].my_state = MOD_MAN_IDLE;
        mod_man_slave_table[ix].in_sync = FALSE;
        mod_man_slave_table[ix].one_pps_load_latency = 0;
        mod_man_slave_table[ix].one_pps_save_latency = 0;
    }
    T_RG(VTSS_TRACE_GRP_MOD_MAN, "ModuleManager initialized");
    MOD_MAN_UNLOCK();
    tod_phy_eng_alloc_init();
    return rc;
}


vtss_rc ptp_module_man_resume(void)
{
    vtss_port_no_t port_no;
    port_iter_t       pit;
    MOD_MAN_LOCK();
    port_data_initialize();
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        port_no = pit.iport;
        port_data_conf_set(port_no);
    }
    MOD_MAN_UNLOCK();

    cyg_thread_resume(ptp_module_man_thread_handle);
    return VTSS_RC_OK;
}

vtss_rc ptp_module_man_rateadj(vtss_phy_ts_scaled_ppb_t *adj)
{
    vtss_rc rc = VTSS_OK;
    int ix;
    /* set adjustment on all active timecounters */
    MOD_MAN_LOCK();
    my_adj = *adj;
    for (ix = 0; ix < MOD_MAN_SLAVE_TABLE_SIZE; ++ix) {
        if (mod_man_slave_table[ix].tod_rateadj) {
            rc = mod_man_slave_table[ix].tod_rateadj(
                        mod_man_slave_table[ix].inst,
                        mod_man_slave_table[ix].port_no,
                        adj);
            T_IG(VTSS_TRACE_GRP_MOD_MAN, "rate adj port=%d, rc = %x, adj = %lld", 
                        mod_man_slave_table[ix].port_no, rc, (*adj)>>16);
        }
    }
    MOD_MAN_UNLOCK();
    return rc;
}

//ptp_module_man_time_offset_set(vtss_timestamp_t *ts, BOOL add)
//{
//    /* Add/subtract ts to/from actual PTP time */
//    
//}

//ptp_module_man_time_master_timecounter_set(Timecounter_id time_cnt)
//{
//    /* Set master clock id (type) */
//    
//}

static vtss_rc ptp_module_man_time_slave_timecounter_add(vtss_ts_timecounter_type_t time_cnt, 
                                                    vtss_inst_t    inst,
                                                    vtss_port_no_t port_no,
                                                    vtss_timeinterval_t one_pps_load_latency,
                                                    vtss_timeinterval_t one_pps_save_latency)
{



    vtss_rc my_rc = VTSS_OK;
    /* Add a slaveclock to the system */
    if (time_cnt == VTSS_TS_TIMECOUNTER_PHY) {
        int ix = 0;
        while (ix < MOD_MAN_SLAVE_TABLE_SIZE) {
            if (mod_man_slave_table[ix].port_no == port_no) {
                T_IG(VTSS_TRACE_GRP_MOD_MAN, "port already included in moduleman table, port = %d", port_no);
                break;
            }
            ++ix;
        }
        if (ix >= MOD_MAN_SLAVE_TABLE_SIZE) { /* id not already in table then find a free entry */
            ix = 0;
            while (ix < MOD_MAN_SLAVE_TABLE_SIZE && mod_man_slave_table[ix].tod_set != NULL) {
                ++ix;
            }
        }
        if (ix < MOD_MAN_SLAVE_TABLE_SIZE) {























            mod_man_slave_table[ix].tod_set = vtss_phy_ts_ptptime_set;
            mod_man_slave_table[ix].tod_arm = vtss_phy_ts_ptptime_arm;
            mod_man_slave_table[ix].tod_set_done = vtss_phy_ts_ptptime_set_done;
            mod_man_slave_table[ix].tod_get = vtss_phy_ts_ptptime_get;
            mod_man_slave_table[ix].tod_adj1ns = vtss_phy_ts_ptptime_adj1ns;
            mod_man_slave_table[ix].tod_rateadj = vtss_phy_ts_clock_rateadj_set;
            mod_man_slave_table[ix].ts_mode_set = vtss_phy_ts_mode_set;
            mod_man_slave_table[ix].tod_fifo_empty = vtss_phy_ts_fifo_empty;
            TOD_RC(vtss_phy_ts_fifo_read_install(inst, my_phy_ts_fifo_read, NULL));

            T_IG(VTSS_TRACE_GRP_MOD_MAN, "installed callback, f = %p", my_phy_ts_fifo_read);
            mod_man_slave_table[ix].inst = inst;
            mod_man_slave_table[ix].port_no = port_no;
            mod_man_slave_table[ix].my_state = MOD_MAN_TSYNC1;
            mod_man_slave_table[ix].in_sync = FALSE;
            mod_man_slave_table[ix].one_pps_load_latency = one_pps_load_latency;
            mod_man_slave_table[ix].one_pps_save_latency = one_pps_save_latency;
            /* initial setting of adjustment rate */
            TOD_RC(mod_man_slave_table[ix].tod_rateadj(mod_man_slave_table[ix].inst,
                                                mod_man_slave_table[ix].port_no,
                                                &my_adj));
            T_DG(VTSS_TRACE_GRP_MOD_MAN, "added port = %d, index = %d", port_no, ix);
        } else {
            my_rc = VTSS_RC_ERROR;
            T_WG(VTSS_TRACE_GRP_MOD_MAN, "missing space in moduleman table, port = %d", port_no);
        }
    } else {
        my_rc = VTSS_RC_ERROR;
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "no timecounter support for port = %d", port_no);
    }
    return my_rc;
}

static vtss_rc ptp_module_man_time_slave_timecounter_delete(vtss_port_no_t port_no)
{
    vtss_rc rc = VTSS_OK;
    /* Remove a slaveclock from the system */
    int ix = 0;
    MOD_MAN_LOCK();
    while (ix < MOD_MAN_SLAVE_TABLE_SIZE && mod_man_slave_table[ix].port_no != port_no) {
        ++ix;
    }
    if (ix < MOD_MAN_SLAVE_TABLE_SIZE) {
        mod_man_slave_table[ix].tod_set = NULL;
        mod_man_slave_table[ix].tod_set_done = NULL;
        mod_man_slave_table[ix].tod_arm = NULL;
        mod_man_slave_table[ix].tod_get = NULL;
        mod_man_slave_table[ix].tod_adj1ns = NULL;
        mod_man_slave_table[ix].tod_rateadj = NULL;
        mod_man_slave_table[ix].inst = 0;
        mod_man_slave_table[ix].port_no = 0;
        mod_man_slave_table[ix].my_state = MOD_MAN_IDLE;
        T_DG(VTSS_TRACE_GRP_MOD_MAN, "deleted port = %d, index = %d", port_no, ix);
    } else {
        rc = VTSS_RC_ERROR;
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "no active timecounter for port = %d", port_no);
    }
    MOD_MAN_UNLOCK();
    return rc;
}

vtss_rc ptp_module_man_time_slave_timecounter_enable_disable(vtss_port_no_t port_no, BOOL enable)
{
    vtss_rc rc = VTSS_OK;
    /* change the state for a slaveclock from the system */
    int ix = 0;
    MOD_MAN_LOCK();
    while (ix < MOD_MAN_SLAVE_TABLE_SIZE && mod_man_slave_table[ix].port_no != port_no) {
        ++ix;
    }
    if (ix < MOD_MAN_SLAVE_TABLE_SIZE) {
        if (enable && (mod_man_slave_table[ix].my_state == MOD_MAN_IDLE)) {
            mod_man_slave_table[ix].my_state = MOD_MAN_TSYNC1;
        } 
        if (!enable) {
            mod_man_slave_table[ix].my_state = MOD_MAN_IDLE;
        }
        T_DG(VTSS_TRACE_GRP_MOD_MAN, "enable/disable port = %d, index = %d, enable = %d", port_no, ix, enable);
    } else {
        rc = VTSS_RC_ERROR;
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "no active timecounter for port = %d", port_no);
    }
    MOD_MAN_UNLOCK();
    return rc;
}


/***************************************************************************************************
 * PHY tx timestamp fifo handling.
 ***************************************************************************************************/

/* PHY timestamp table structure */
typedef struct {
    u64 reserved_mask;                      /* port mask indicating which ports this tx idx is reserved for */
    u64 valid_mask;                         /* indication pr. port if there is a valid timestamp in the table  */
    u32 age;                                /* ageing counter */
    u32 tx_tc [VTSS_PORT_ARRAY_SIZE];       /* actual transmit time counter for the [idx][port] */
    vtss_phy_ts_fifo_sig_t    ts_sig;       /* actual transmit time stamp signature */
    void * context;                         /* context aligned to the  [idx] */
    void (*cb)(void *context, u32 port_no, vtss_ts_timestamp_t *ts); /* timestamp callback function */
} vtss_phy_ts_timestamp_status_t;
#define PHY_TS_TABLE_SIZE VTSS_PORTS

static vtss_phy_ts_timestamp_status_t phy_ts_status [PHY_TS_TABLE_SIZE];


static BOOL signature_match(const vtss_phy_ts_fifo_sig_t   *a, const vtss_phy_ts_fifo_sig_t   *b)
{
    if (a->sig_mask != b->sig_mask) return FALSE;
    if ((a->sig_mask & VTSS_PHY_TS_FIFO_SIG_MSG_TYPE) && (a->msg_type != b->msg_type)) return FALSE;
    if ((a->sig_mask & VTSS_PHY_TS_FIFO_SIG_DOMAIN_NUM) && (a->domain_num != b->domain_num)) return FALSE;
    if ((a->sig_mask & VTSS_PHY_TS_FIFO_SIG_SOURCE_PORT_ID) && 
            (0 != memcmp(a->src_port_identity,b->src_port_identity,10))) return FALSE;
    if ((a->sig_mask & VTSS_PHY_TS_FIFO_SIG_SEQ_ID) && (a->sequence_id != b->sequence_id)) return FALSE;
    if ((a->sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) && (a->dest_ip != b->dest_ip)) return FALSE;
    if ((a->sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) && (a->src_ip != b->src_ip)) return FALSE;
    return TRUE;
    
}

/* Update the internal timestamp table, from HW */








static void my_phy_ts_fifo_read(      const vtss_inst_t              inst,
                                      const vtss_port_no_t           port_no,
                                      const vtss_phy_timestamp_t     *const fifo_ts,
                                      const vtss_phy_ts_fifo_sig_t   *const sig,
                                      void                           *cntxt,
                                      const vtss_phy_ts_fifo_status_t status)

{
    u64 port_mask;
    int ts_idx;
    vtss_ts_timestamp_t ts;
    void * cx;                                                       /* context aligned to the  [idx] */
    void (*cb)(void *context, u32 port_no, vtss_ts_timestamp_t *ts); /* timestamp callback function */ 
    port_mask = 1LL<<port_no;
    T_IG(VTSS_TRACE_GRP_MOD_MAN, "port_no %d, tx signature: mask %x, type %d, domain %d, seq %d", 
         port_no, 
         sig->sig_mask, sig->msg_type, sig->domain_num, sig->sequence_id);
    T_IG_HEX(VTSS_TRACE_GRP_MOD_MAN, sig->src_port_identity, 10);
    T_IG(VTSS_TRACE_GRP_MOD_MAN, "tx time:  %d, %d:%d, hex ns %x", 
         fifo_ts->seconds.high, fifo_ts->seconds.low, fifo_ts->nanoseconds, fifo_ts->nanoseconds);
    
    MOD_MAN_LOCK();
    for (ts_idx = 0; ts_idx < PHY_TS_TABLE_SIZE; ++ts_idx) {
        T_IG(VTSS_TRACE_GRP_MOD_MAN, "ts_status[%d]: reserved %llx, signature: mask %x, type %d, domain %d, seq %d", 
             ts_idx, phy_ts_status[ts_idx].reserved_mask, 
             phy_ts_status[ts_idx].ts_sig.sig_mask, phy_ts_status[ts_idx].ts_sig.msg_type, 
             phy_ts_status[ts_idx].ts_sig.domain_num, phy_ts_status[ts_idx].ts_sig.sequence_id);
        T_IG_HEX(VTSS_TRACE_GRP_MOD_MAN, phy_ts_status[ts_idx].ts_sig.src_port_identity, 10);

        if ((phy_ts_status[ts_idx].reserved_mask & port_mask) &&
                    signature_match(sig, &phy_ts_status[ts_idx].ts_sig)) {
            phy_ts_status[ts_idx].valid_mask &= ~port_mask;
            phy_ts_status[ts_idx].reserved_mask &= ~port_mask;
            ts.id = 0;
            ts.ts = vtss_tod_ns_to_ts_cnt(fifo_ts->nanoseconds);
            ts.ts_valid = TRUE;
            if (phy_ts_status[ts_idx].cb && phy_ts_status[ts_idx].context) {
                /* call out to the application, make local copy to avoid unprotected access to phy_ts_status */
                cb = phy_ts_status[ts_idx].cb;
                cx = phy_ts_status[ts_idx].context;
                MOD_MAN_UNLOCK();
                cb(cx, port_no, &ts);
                MOD_MAN_LOCK();
            } else {
                T_EG(VTSS_TRACE_GRP_MOD_MAN, "undefined TS callback port_idx %d, ts_idx %d", port_no, ts_idx);
            }
            T_IG(VTSS_TRACE_GRP_MOD_MAN, "port_no %d, ts_id %d, ts %d(%d)", port_no, ts.id, ts.ts, ts.ts_valid);
        }
        if (phy_ts_status[ts_idx].reserved_mask == 0LL) {
            phy_ts_status[ts_idx].cb = NULL;
            phy_ts_status[ts_idx].context = NULL;
        }
    }
    MOD_MAN_UNLOCK();
}


/* Allocate a timestamp entry for a two step transmission */
vtss_rc vtss_module_man_tx_timestamp_sig_alloc(const vtss_ts_timestamp_alloc_t *const alloc_parm,
                                        const vtss_phy_ts_fifo_sig_t    *const ts_sig)
{
    vtss_rc rc = VTSS_RC_ERROR;
    u32 id;
    MOD_MAN_LOCK();
    /* Find a free entry */
    for (id = 0; id < PHY_TS_TABLE_SIZE; id++) {
        if (phy_ts_status[id].reserved_mask == 0) {
            phy_ts_status[id].reserved_mask = alloc_parm->port_mask;
            phy_ts_status[id].context = alloc_parm->context;
            phy_ts_status[id].cb = alloc_parm->cb;
            phy_ts_status[id].age = 0;
            memcpy(&phy_ts_status[id].ts_sig, ts_sig, sizeof(vtss_phy_ts_fifo_sig_t));
            T_DG(VTSS_TRACE_GRP_MOD_MAN, "portmask = %llx, idx = %d", alloc_parm->port_mask, id);
            rc = VTSS_RC_OK;
            break;
        }
    }
    MOD_MAN_UNLOCK();
    return rc;
}


/*
 * PHY Timestamp feature interrupt handler
 * \param instance_id   IN  PHY port number that caused the interrupt.
 */
static void phy_timestamp_interrupt_handler(vtss_interrupt_source_t     source_id,
                                        u32                         instance_id)
{

    T_NG(VTSS_TRACE_GRP_MOD_MAN, "PHY timestamp interrupt detected: source_id %d, instance_id %u", source_id, instance_id);
    if (source_id == INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED) {
        TOD_RC(vtss_phy_ts_fifo_empty(API_INST_DEFAULT, instance_id));
    }
    if (source_id == INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW) {
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "PHY timestamp FIFO overflow: instance_id %u", instance_id);
        TOD_RC(vtss_phy_ts_fifo_empty(API_INST_DEFAULT, instance_id));
    }
    if (source_id == INTERRUPT_SOURCE_INGR_ENGINE_ERR) {
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "TS More than one engine find match: instance_id %u", instance_id);
    }
    if (source_id == INTERRUPT_SOURCE_INGR_RW_PREAM_ERR) {
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "TS Preamble too short to append timestamp: instance_id %u", instance_id);
    }
    if (source_id == INTERRUPT_SOURCE_INGR_RW_FCS_ERR) {
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "TS FCS error in ingress: instance_id %u", instance_id);
    }
    if (source_id == INTERRUPT_SOURCE_EGR_ENGINE_ERR) {
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "TS More than one engine find match: instance_id %u", instance_id);
    }
    if (source_id == INTERRUPT_SOURCE_EGR_RW_FCS_ERR) {
        T_WG(VTSS_TRACE_GRP_MOD_MAN, "TS FCS error in egress: instance_id %u", instance_id);
    }

    TOD_RC(vtss_interrupt_source_hook_set(phy_timestamp_interrupt_handler,
                                          source_id,
                                          INTERRUPT_PRIORITY_NORMAL));
}


#define TOD_TX_MAX_TIMETICKS 3 /* 300 msec timeout */
/* Age the TX FIFO timestamps */
static vtss_rc phy_timestamp_age(void)
{
    int id;
    u64 port_mask;
    vtss_ts_timestamp_t ts;
    int port_idx = 0;
    vtss_rc rc = VTSS_RC_OK;
    port_iter_t       pit;
    void * cx;                                                       /* context aligned to the  [idx] */
    void (*cb)(void *context, u32 port_no, vtss_ts_timestamp_t *ts); /* timestamp callback function */ 
    MOD_MAN_LOCK();
    /* traverse all entries and check if aged out */
    for (id = 0; id < PHY_TS_TABLE_SIZE; id++) {
        if (++phy_ts_status[id].age > TOD_TX_MAX_TIMETICKS) {
            port_idx = 0;
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                port_idx = pit.iport;
                port_mask = 1LL<<port_idx;
                if ((phy_ts_status[id].reserved_mask & port_mask)) {
                    phy_ts_status[id].reserved_mask  &= ~port_mask;
                    ts.id = id;
                    ts.ts = 0;
                    ts.ts_valid = FALSE;
                    if (phy_ts_status[id].cb && phy_ts_status[id].context) {
                        /* call out to the application, make local copy to avoid unprotected access to phy_ts_status */
                        cb = phy_ts_status[id].cb;
                        cx = phy_ts_status[id].context;
                        MOD_MAN_UNLOCK();
                        cb(cx, port_idx, &ts);
                        MOD_MAN_LOCK();
                    } else {
                        T_NG(VTSS_TRACE_GRP_MOD_MAN, "undefined TS callback port_idx %d, ts_idx %d", port_idx, id);
                    }
                    T_IG(VTSS_TRACE_GRP_MOD_MAN, "port_no %d, ts_id %d, ts %d(%d)", port_idx, ts.id, ts.ts, ts.ts_valid);
                }
                ++ port_idx;
            }
            phy_ts_status[id].reserved_mask = 0LL;
            phy_ts_status[id].valid_mask = 0LL;
            phy_ts_status[id].age = 0;
            phy_ts_status[id].cb = NULL;
            phy_ts_status[id].context = NULL;
        }
    }
    MOD_MAN_UNLOCK();
    return rc;
}


/* to avoid lint warnings for the MOD_MAN_UNLOCK() ; MOD_MAN_LOCK when calling out from the module_man */
/*lint -e{456} */
/*lint -e{455} */
/*lint -e{454} */
static void vtss_module_man_tx_timestamp_in_sync_call_out(vtss_port_no_t port_no, BOOL in_sync)
{
   int id;
   port_data[port_no].topo.port_ts_in_sync = in_sync;

   for (id = 0; id < MOD_MAN_USER_TABLE_SIZE; id++) {
        if (my_in_sync_cb[id] != NULL) {
            /* as the applications may call back to module_man, we unlock here to avoid deadlock */
            MOD_MAN_UNLOCK();
            my_in_sync_cb[id](port_no, in_sync);
            MOD_MAN_LOCK();
        }
    }
}

vtss_rc vtss_module_man_tx_timestamp_in_sync_cb_add(const vtss_module_man_in_sync_cb_t in_sync_cb)
{
    int id;
    vtss_rc rc = VTSS_RC_ERROR;
    MOD_MAN_LOCK();
    for (id = 0; id < MOD_MAN_USER_TABLE_SIZE; id++) {
        if (my_in_sync_cb[id] == in_sync_cb) {
            /* only insert same pointer once */
            rc = VTSS_RC_OK;
            break;
        }
    }
    if (rc != VTSS_RC_OK) {
        /* if not already in list, then find a free entry */
        for (id = 0; id < MOD_MAN_USER_TABLE_SIZE; id++) {
            if (my_in_sync_cb[id] == NULL) {
                my_in_sync_cb[id] = in_sync_cb;
                rc = VTSS_RC_OK;
                break;
            }
        }
    }
    MOD_MAN_UNLOCK();
    return rc;
}

vtss_rc vtss_module_man_tx_timestamp_in_sync_cb_delete(const vtss_module_man_in_sync_cb_t in_sync_cb)
{
    int id;
    vtss_rc rc = VTSS_RC_ERROR;
    MOD_MAN_LOCK();
    for (id = 0; id < MOD_MAN_USER_TABLE_SIZE; id++) {
        if (my_in_sync_cb[id] == in_sync_cb) {
            my_in_sync_cb[id] = NULL;
            rc = VTSS_RC_OK;
        }
    }
    MOD_MAN_UNLOCK();
    return rc;
}

/**
 * Module manager set 1PPS output latency.
 **/
vtss_rc vtss_module_man_master_1pps_latency(const vtss_timeinterval_t latency)
{
    vtss_rc rc = VTSS_RC_OK;
    MOD_MAN_LOCK();
    my_1pps_latency = latency;
    MOD_MAN_UNLOCK();
    return rc;
}

#endif
