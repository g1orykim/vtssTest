/* Switch API software.

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

/**
 * \file
 * \brief PTP icli functions
 * \details This header file describes ptp control functions
 */

#ifndef VTSS_ICLI_PTP_H
#define VTSS_ICLI_PTP_H

#include "icli_api.h"

/**
 * \brief Function for show div. ptp information
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param clockinst  [IN]  PTP instance number 0..3.
 * \param has_xxx    [IN]  TRUE if corresponding part of PTP data has to be displayed.
 * \return None.
 **/
void ptp_icli_show(i32 session_id, int clockinst, BOOL has_default, BOOL has_current, BOOL has_parent, BOOL has_time_property,
    BOOL has_filter, BOOL has_servo, BOOL has_clk, BOOL has_ho, BOOL has_uni, BOOL has_master_table_unicast,
    BOOL has_slave, BOOL has_port_state, BOOL has_port_ds, BOOL has_wireless, BOOL has_foreign_master_record,
    BOOL has_interface, icli_stack_port_range_t *v_port_type_list);

/**
 * \brief Function for show external clock mode
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \return None.
 **/
void ptp_icli_ext_clock_mode_show(i32 session_id);

#if defined(VTSS_ARCH_SERVAL)
/**
 * \brief Function for show rs422 external clock mode
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \return None.
 **/
void ptp_icli_rs422_clock_mode_show(i32 session_id);
#endif


/**
 * \brief Function for set local clock time or ratio
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param has_update [IN]  TRUE if ptp time has to be loaded with the eCOS time.
 * \param has_ratio  [IN]  TRUE if ptp cloak ration has to be set.
 * \param ratio      [IN]  ratio in units of 0,1 ppb.
 * \return None.
 **/
void ptp_icli_local_clock_set(i32 session_id, int clockinst, BOOL has_update, BOOL has_ratio, i32 ratio);

void ptp_icli_local_clock_show(i32 session_id, int clockinst);


/**
 * \brief Function for set slave lock state configuration
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param clockinst  [IN]  PTP instance number 0..3.
 * \param has_xxx    [IN]  TRUE if parameter is entered. If not entered, default value is used.
 * \param xxx        [IN]  parameter value.
 * \param ratio      [IN]  ratio in units of 0,1 ppb.
 * \return None.
 **/
void ptp_icli_slave_cfg_set(i32 session_id, int clockinst, BOOL has_stable_offset, u32 stable_offset, BOOL has_offset_ok, u32 offset_ok, BOOL has_offset_fail, u32 offset_fail);
void ptp_icli_slave_cfg_show(i32 session_id, int clockinst);

void ptp_icli_slave_table_unicast_show(i32 session_id, int clockinst);


/**
 * \brief Function for PTP wireless mode
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param clockinst  [IN]  PTP instance number 0..3.
 * \param enable     [IN]  True if enable wireless mode.
 * \param priority1  [IN]  port list.
 * \return None.
 **/
void ptp_icli_wireless_mode_set(i32 session_id, int clockinst, BOOL enable, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_wireless_delay(i32 session_id, int clockinst, i32 base_delay, i32 incr_delay, icli_stack_port_range_t *v_port_type_list);

/**
 * \brief Function for PTP wireless pre notification
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param clockinst  [IN]  PTP instance number 0..3.
 * \param priority1  [IN]  port list.
 * \return None.
 **/
void ptp_icli_wireless_pre_notification(i32 session_id, int clockinst, icli_stack_port_range_t *v_port_type_list);


void ptp_icli_mode(i32 session_id, int clockinst,
              BOOL has_boundary, BOOL has_e2etransparent, BOOL has_p2ptransparent, BOOL has_master, BOOL has_slave,
              BOOL has_onestep, BOOL has_twostep,
              BOOL has_ethernet, BOOL has_ip4multi, BOOL has_ip4unicast, BOOL has_oam, BOOL has_onepps,
              BOOL has_oneway, BOOL has_twoway,
              BOOL has_id, icli_clock_id_t *v_clock_id,
              BOOL has_vid, u32 vid, u32 prio, BOOL has_tag, BOOL has_mep, u32 mep_id);

void ptp_icli_no_mode(i32 session_id, int clockinst,
                 BOOL has_boundary, BOOL has_e2etransparent, BOOL has_p2ptransparent, BOOL has_master, BOOL has_slave);

/**
 * \brief Function for PTP priority1
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param clockinst  [IN]  PTP instance number 0..3.
 * \param priority1  [IN]  PTP priority1.
 * \return None.
 **/
void ptp_icli_priority1_set(i32 session_id, int clockinst, u8 priority1);
void ptp_icli_priority2_set(i32 session_id, int clockinst, u8 priority2);
void ptp_icli_domain_set(i32 session_id, int clockinst, u8 domain);


void ptp_icli_time_property_set(i32 session_id, int clockinst, BOOL has_utc_offset, i32 utc_offset, BOOL has_valid,
                                BOOL has_leapminus_59, BOOL has_leapminus_61, BOOL has_time_traceable,
                                BOOL has_freq_traceable, BOOL has_ptptimescale,
                                BOOL has_time_source, u8 time_source);

void ptp_icli_filter_set(i32 session_id, int clockinst, BOOL has_delay, u32 delay, BOOL has_period, u32 period, BOOL has_dist, u32 dist);

void ptp_icli_filter_default_set(i32 session_id, int clockinst);

void ptp_icli_servo_displaystate_set(i32 session_id, int clockinst, BOOL enable);

void ptp_icli_servo_ap_set(i32 session_id, int clockinst, BOOL enable, u32 ap);

void ptp_icli_servo_ai_set(i32 session_id, int clockinst, BOOL enable, u32 ai);

void ptp_icli_servo_ad_set(i32 session_id, int clockinst, BOOL enable, u32 ad);

void ptp_icli_clock_servo_options_set(i32 session_id, int clockinst, BOOL synce, u32 threshold, u32 ap);

void ptp_icli_clock_slave_holdover_set(i32 session_id, int clockinst, BOOL has_filter, u32 ho_filter, BOOL has_adj_threshold, u32 adj_threshold);

void ptp_icli_clock_unicast_conf_set(i32 session_id, int clockinst, int idx, BOOL has_duration, u32 duration, u32 ip);

/**
 * \brief Function for PTP setting ext clock
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param has_output [IN]  TRUE if 1PPS output is enabled.
 * \param has_input  [IN]  TRUE if 1PPS input is enabled. If both has_output and has_input are false, the 1PPS is disabled.
 * \param has_ext    [IN]  TRUE if 1PPS output is enabled.
 * \param clockfreq  [IN]  External clock output frequency in Hz.
 * \param has_vcxo   [IN]  TRUE if 1PPS output is enabled.
 * \return None.
 **/
void ptp_icli_ext_clock_set(i32 session_id, BOOL has_output, BOOL has_input, BOOL has_ext, u32 clockfreq, BOOL has_vcxo);

/**
 * \brief Function for PTP setting ext clock
 *
 * \param session_id    [IN]  Needed for being able to print error messages
 * \param has_main_auto [IN]  TRUE if main auto mode.
 * \param has_main_man  [IN]  TRUE if main manual mode.
 * \param has_sub       [IN]  TRUE if sub mode.
 * \param has_pps_delay [IN]  TRUE if 1PPS delay is entered.
 * \param pps_delay     [IN]  1pps delay used in main-man mode.
 * \param has_ser       [IN]  TRUE if Serial protocol (UART2) is used.
 * \param has_pim       [IN]  TRUE if PIM protocol is used.
 * \param pim_port      [IN]  TRUE if Switch port used by the PIM protocol.
 * \return None.
 **/
void ptp_icli_rs422_clock_set(i32 session_id, BOOL has_main_auto, BOOL has_main_man, BOOL has_sub, BOOL has_pps_delay, u32 pps_delay, BOOL has_ser, BOOL has_pim, u32 pim_port);

void ptp_icli_debug_log_mode_set(i32 session_id, int clockinst, u32 debug_mode);

void ptp_icli_port_state_set(i32 session_id, int clockinst, BOOL enable, BOOL has_internal, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_port_announce_set(i32 session_id, int clockinst, BOOL has_interval, i8 interval, BOOL has_timeout, u8 timeout, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_port_sync_interval_set(i32 session_id, int clockinst, i8 interval, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_port_delay_mechanism_set(i32 session_id, int clockinst, BOOL has_e2e, BOOL has_p2p, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_port_min_pdelay_interval_set(i32 session_id, int clockinst, i8 interval, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_delay_asymmetry_set(i32 session_id, int clockinst, i32 delay_asymmetry, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_ingress_latency_set(i32 session_id, int clockinst, i32 ingress_latency, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_egress_latency_set(i32 session_id, int clockinst, i32 egress_latency, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_port_1pps_mode_set(i32 session_id, BOOL has_main_auto, BOOL has_main_man, BOOL has_sub, BOOL has_pps_phase, i32 pps_phase,
                            BOOL has_cable_asy, i32 cable_asy, BOOL has_ser_man, BOOL has_ser_auto, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_port_1pps_delay_set(i32 session_id, BOOL has_auto, u32 master_port, BOOL has_man, u32 cable_delay, icli_stack_port_range_t *v_port_type_list);

void ptp_icli_ms_pdv_set(i32 session_id, BOOL has_one_hz, BOOL has_min_phase, u32 min_phase, BOOL has_apr, u32 apr, BOOL enable);

void ptp_icli_tc_internal_set(i32 session_id, BOOL has_mode, u32 mode);

#if defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)
    void ptp_icli_ptp_ref_clock_set(i32 session_id, BOOL has_mhz125, BOOL has_mhz156p25, BOOL has_mhz250);
#endif


#endif /* VTSS_ICLI_PTP_H */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
