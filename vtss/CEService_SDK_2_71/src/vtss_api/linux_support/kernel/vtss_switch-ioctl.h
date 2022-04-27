/*

 Vitesse Switch API software.

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


#ifndef _VTSS_SWITCH_IOCTL_H
#define _VTSS_SWITCH_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

#include "vtss_api.h"
#include "vtss_switch-ext.h"

 /************************************
  * port group 
  */


struct vtss_port_map_set_ioc {
    vtss_port_map_t port_map[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_port_map_get_ioc {
    vtss_port_map_t port_map[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_port_mmd_read_ioc {
#ifdef VTSS_FEATURE_10G
    vtss_port_no_t port_no;
    u8 mmd;
    u16 addr;
    u16 value;
#endif				/* VTSS_FEATURE_10G */
};

struct vtss_port_mmd_write_ioc {
#ifdef VTSS_FEATURE_10G
    vtss_port_no_t port_no;
    u8 mmd;
    u16 addr;
    u16 value;
#endif				/* VTSS_FEATURE_10G */
};

struct vtss_port_mmd_masked_write_ioc {
#ifdef VTSS_FEATURE_10G
    vtss_port_no_t port_no;
    u8 mmd;
    u16 addr;
    u16 value;
    u16 mask;
#endif				/* VTSS_FEATURE_10G */
};

struct vtss_mmd_read_ioc {
#ifdef VTSS_FEATURE_10G
    vtss_chip_no_t chip_no;
    vtss_miim_controller_t miim_controller;
    u8 miim_addr;
    u8 mmd;
    u16 addr;
    u16 value;
#endif				/* VTSS_FEATURE_10G */
};

struct vtss_mmd_write_ioc {
#ifdef VTSS_FEATURE_10G
    vtss_chip_no_t chip_no;
    vtss_miim_controller_t miim_controller;
    u8 miim_addr;
    u8 mmd;
    u16 addr;
    u16 value;
#endif				/* VTSS_FEATURE_10G */
};

struct vtss_port_clause_37_control_get_ioc {
#ifdef VTSS_FEATURE_CLAUSE_37
    vtss_port_no_t port_no;
    vtss_port_clause_37_control_t control;
#endif				/* VTSS_FEATURE_CLAUSE_37 */
};

struct vtss_port_clause_37_control_set_ioc {
#ifdef VTSS_FEATURE_CLAUSE_37
    vtss_port_no_t port_no;
    vtss_port_clause_37_control_t control;
#endif				/* VTSS_FEATURE_CLAUSE_37 */
};

struct vtss_port_conf_set_ioc {
    vtss_port_no_t port_no;
    vtss_port_conf_t conf;
};

struct vtss_port_conf_get_ioc {
    vtss_port_no_t port_no;
    vtss_port_conf_t conf;
};

struct vtss_port_status_get_ioc {
    vtss_port_no_t port_no;
    vtss_port_status_t status;
};

struct vtss_port_counters_update_ioc {
    vtss_port_no_t port_no;
};

struct vtss_port_counters_clear_ioc {
    vtss_port_no_t port_no;
};

struct vtss_port_counters_get_ioc {
    vtss_port_no_t port_no;
    vtss_port_counters_t counters;
};

struct vtss_port_basic_counters_get_ioc {
    vtss_port_no_t port_no;
    vtss_basic_counters_t counters;
};

struct vtss_port_forward_state_get_ioc {
    vtss_port_no_t port_no;
    vtss_port_forward_t forward;
};

struct vtss_port_forward_state_set_ioc {
    vtss_port_no_t port_no;
    vtss_port_forward_t forward;
};

struct vtss_miim_read_ioc {
    vtss_chip_no_t chip_no;
    vtss_miim_controller_t miim_controller;
    u8 miim_addr;
    u8 addr;
    u16 value;
};

struct vtss_miim_write_ioc {
    vtss_chip_no_t chip_no;
    vtss_miim_controller_t miim_controller;
    u8 miim_addr;
    u8 addr;
    u16 value;
};



 /************************************
  * misc group 
  */


struct vtss_debug_info_print_ioc {
    vtss_debug_info_t info;
};

struct vtss_reg_read_ioc {
    vtss_chip_no_t chip_no;
    u32 addr;
    u32 value;
};

struct vtss_reg_write_ioc {
    vtss_chip_no_t chip_no;
    u32 addr;
    u32 value;
};

struct vtss_reg_write_masked_ioc {
    vtss_chip_no_t chip_no;
    u32 addr;
    u32 value;
    u32 mask;
};

struct vtss_chip_id_get_ioc {
    vtss_chip_id_t chip_id;
};

struct vtss_ptp_event_poll_ioc {
    vtss_ptp_event_type_t ev_mask;
};

struct vtss_ptp_event_enable_ioc {
    vtss_ptp_event_type_t ev_mask;
    BOOL enable;
};

struct vtss_dev_all_event_poll_ioc {
    vtss_dev_all_event_poll_t poll_type;
    vtss_dev_all_event_type_t ev_mask;
};

struct vtss_dev_all_event_enable_ioc {
    vtss_port_no_t port;
    vtss_dev_all_event_type_t ev_mask;
    BOOL enable;
};

struct vtss_gpio_mode_set_ioc {
    vtss_chip_no_t chip_no;
    vtss_gpio_no_t gpio_no;
    vtss_gpio_mode_t mode;
};

struct vtss_gpio_direction_set_ioc {
    vtss_chip_no_t chip_no;
    vtss_gpio_no_t gpio_no;
    BOOL output;
};

struct vtss_gpio_read_ioc {
    vtss_chip_no_t chip_no;
    vtss_gpio_no_t gpio_no;
    BOOL value;
};

struct vtss_gpio_write_ioc {
    vtss_chip_no_t chip_no;
    vtss_gpio_no_t gpio_no;
    BOOL value;
};

struct vtss_gpio_event_poll_ioc {
    vtss_chip_no_t chip_no;
    BOOL events;
};

struct vtss_gpio_event_enable_ioc {
    vtss_chip_no_t chip_no;
    vtss_gpio_no_t gpio_no;
    BOOL enable;
};

struct vtss_gpio_clk_set_ioc {
#ifdef VTSS_ARCH_B2
    vtss_gpio_no_t gpio_no;
    vtss_port_no_t port_no;
    vtss_recovered_clock_t clk;
#endif				/* VTSS_ARCH_B2 */
};

struct vtss_serial_led_set_ioc {
#ifdef VTSS_FEATURE_SERIAL_LED
    vtss_led_port_t port;
    vtss_led_mode_t mode[3];
#endif				/* VTSS_FEATURE_SERIAL_LED */
};

struct vtss_serial_led_intensity_set_ioc {
#ifdef VTSS_FEATURE_SERIAL_LED
    vtss_led_port_t port;
    i8 intensity;
#endif				/* VTSS_FEATURE_SERIAL_LED */
};

struct vtss_serial_led_intensity_get_ioc {
#ifdef VTSS_FEATURE_SERIAL_LED
    i8 intensity;
#endif				/* VTSS_FEATURE_SERIAL_LED */
};

struct vtss_sgpio_conf_get_ioc {
#ifdef VTSS_FEATURE_SERIAL_GPIO
    vtss_chip_no_t chip_no;
    vtss_sgpio_group_t group;
    vtss_sgpio_conf_t conf;
#endif				/* VTSS_FEATURE_SERIAL_GPIO */
};

struct vtss_sgpio_conf_set_ioc {
#ifdef VTSS_FEATURE_SERIAL_GPIO
    vtss_chip_no_t chip_no;
    vtss_sgpio_group_t group;
    vtss_sgpio_conf_t conf;
#endif				/* VTSS_FEATURE_SERIAL_GPIO */
};

struct vtss_sgpio_read_ioc {
#ifdef VTSS_FEATURE_SERIAL_GPIO
    vtss_chip_no_t chip_no;
    vtss_sgpio_group_t group;
    vtss_sgpio_port_data_t data[VTSS_SGPIO_PORTS];
#endif				/* VTSS_FEATURE_SERIAL_GPIO */
};

struct vtss_sgpio_event_poll_ioc {
#ifdef VTSS_FEATURE_SERIAL_GPIO
    vtss_chip_no_t chip_no;
    vtss_sgpio_group_t group;
    u32 bit;
    BOOL events[VTSS_SGPIO_PORTS];
#endif				/* VTSS_FEATURE_SERIAL_GPIO */
};

struct vtss_sgpio_event_enable_ioc {
#ifdef VTSS_FEATURE_SERIAL_GPIO
    vtss_chip_no_t chip_no;
    vtss_sgpio_group_t group;
    u32 port;
    u32 bit;
    BOOL enable;
#endif				/* VTSS_FEATURE_SERIAL_GPIO */
};

struct vtss_temp_sensor_init_ioc {
#ifdef VTSS_FEATURE_FAN
    BOOL enable;
#endif				/* VTSS_FEATURE_FAN */
};

struct vtss_temp_sensor_get_ioc {
#ifdef VTSS_FEATURE_FAN
    i16 temperature;
#endif				/* VTSS_FEATURE_FAN */
};

struct vtss_fan_rotation_get_ioc {
#ifdef VTSS_FEATURE_FAN
    vtss_fan_conf_t fan_spec;
    u32 rotation_count;
#endif				/* VTSS_FEATURE_FAN */
};

struct vtss_fan_cool_lvl_set_ioc {
#ifdef VTSS_FEATURE_FAN
    u8 lvl;
#endif				/* VTSS_FEATURE_FAN */
};

struct vtss_fan_controller_init_ioc {
#ifdef VTSS_FEATURE_FAN
    vtss_fan_conf_t spec;
#endif				/* VTSS_FEATURE_FAN */
};

struct vtss_fan_cool_lvl_get_ioc {
#ifdef VTSS_FEATURE_FAN
    u8 lvl;
#endif				/* VTSS_FEATURE_FAN */
};

struct vtss_eee_port_conf_set_ioc {
#ifdef VTSS_FEATURE_EEE
    vtss_port_no_t port_no;
    vtss_eee_port_conf_t conf;
#endif				/* VTSS_FEATURE_EEE */
};

struct vtss_eee_port_state_set_ioc {
#ifdef VTSS_FEATURE_EEE
    vtss_port_no_t port_no;
    vtss_eee_port_state_t conf;
#endif				/* VTSS_FEATURE_EEE */
};

struct vtss_eee_port_counter_get_ioc {
#ifdef VTSS_FEATURE_EEE
    vtss_port_no_t port_no;
    vtss_eee_port_counter_t conf;
#endif				/* VTSS_FEATURE_EEE */
};



 /************************************
  * init group 
  */


struct vtss_restart_status_get_ioc {
#ifdef VTSS_FEATURE_WARM_START
    vtss_restart_status_t status;
#endif				/* VTSS_FEATURE_WARM_START */
};

struct vtss_restart_conf_get_ioc {
#ifdef VTSS_FEATURE_WARM_START
    vtss_restart_t restart;
#endif				/* VTSS_FEATURE_WARM_START */
};

struct vtss_restart_conf_set_ioc {
#ifdef VTSS_FEATURE_WARM_START
    vtss_restart_t restart;
#endif				/* VTSS_FEATURE_WARM_START */
};



 /************************************
  * phy group 
  */


struct vtss_phy_pre_reset_ioc {
    vtss_port_no_t port_no;
};

struct vtss_phy_post_reset_ioc {
    vtss_port_no_t port_no;
};

struct vtss_phy_reset_ioc {
    vtss_port_no_t port_no;
    vtss_phy_reset_conf_t conf;
};

struct vtss_phy_chip_temp_get_ioc {
    vtss_port_no_t port_no;
    i16 temp;
};

struct vtss_phy_chip_temp_init_ioc {
    vtss_port_no_t port_no;
};

struct vtss_phy_conf_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_conf_t conf;
};

struct vtss_phy_conf_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_conf_t conf;
};

struct vtss_phy_status_get_ioc {
    vtss_port_no_t port_no;
    vtss_port_status_t status;
};

struct vtss_phy_conf_1g_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_conf_1g_t conf;
};

struct vtss_phy_conf_1g_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_conf_1g_t conf;
};

struct vtss_phy_status_1g_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_status_1g_t status;
};

struct vtss_phy_power_conf_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_power_conf_t conf;
};

struct vtss_phy_power_conf_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_power_conf_t conf;
};

struct vtss_phy_power_status_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_power_status_t status;
};

struct vtss_phy_clock_conf_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_recov_clk_t clock_port;
    vtss_phy_clock_conf_t conf;
};

struct vtss_phy_read_ioc {
    vtss_port_no_t port_no;
    u32 addr;
    u16 value;
};

struct vtss_phy_write_ioc {
    vtss_port_no_t port_no;
    u32 addr;
    u16 value;
};

struct vtss_phy_mmd_read_ioc {
    vtss_port_no_t port_no;
    u16 devad;
    u32 addr;
    u16 value;
};

struct vtss_phy_mmd_write_ioc {
    vtss_port_no_t port_no;
    u16 devad;
    u32 addr;
    u16 value;
};

struct vtss_phy_write_masked_ioc {
    vtss_port_no_t port_no;
    u32 addr;
    u16 value;
    u16 mask;
};

struct vtss_phy_write_masked_page_ioc {
    vtss_port_no_t port_no;
    u16 page;
    u16 addr;
    u16 value;
    u16 mask;
};

struct vtss_phy_veriphy_start_ioc {
    vtss_port_no_t port_no;
    u8 mode;
};

struct vtss_phy_veriphy_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_veriphy_result_t result;
};

struct vtss_phy_led_intensity_set_ioc {
#ifdef VTSS_FEATURE_LED_POW_REDUC
    vtss_port_no_t port_no;
    vtss_phy_led_intensity intensity;
#endif				/* VTSS_FEATURE_LED_POW_REDUC */
};

struct vtss_phy_enhanced_led_control_init_ioc {
#ifdef VTSS_FEATURE_LED_POW_REDUC
    vtss_port_no_t port_no;
    vtss_phy_enhanced_led_control_t conf;
#endif				/* VTSS_FEATURE_LED_POW_REDUC */
};

struct vtss_phy_coma_mode_disable_ioc {
    vtss_port_no_t port_no;
};

struct vtss_phy_eee_ena_ioc {
#ifdef VTSS_FEATURE_EEE
    vtss_port_no_t port_no;
    BOOL enable;
#endif				/* VTSS_FEATURE_EEE */
};

struct vtss_phy_event_enable_set_ioc {
#ifdef VTSS_CHIP_CU_PHY
    vtss_port_no_t port_no;
    vtss_phy_event_t ev_mask;
    BOOL enable;
#endif				/* VTSS_CHIP_CU_PHY */
};

struct vtss_phy_event_enable_get_ioc {
#ifdef VTSS_CHIP_CU_PHY
    vtss_port_no_t port_no;
    vtss_phy_event_t ev_mask;
#endif				/* VTSS_CHIP_CU_PHY */
};

struct vtss_phy_event_poll_ioc {
#ifdef VTSS_CHIP_CU_PHY
    vtss_port_no_t port_no;
    vtss_phy_event_t ev_mask;
#endif				/* VTSS_CHIP_CU_PHY */
};



 /************************************
  * 10gphy group 
  */

#ifdef VTSS_FEATURE_10G
struct vtss_phy_10g_mode_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_mode_t mode;
};

struct vtss_phy_10g_mode_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_mode_t mode;
};

struct vtss_phy_10g_synce_clkout_get_ioc {
    vtss_port_no_t port_no;
    BOOL synce_clkout;
};

struct vtss_phy_10g_synce_clkout_set_ioc {
    vtss_port_no_t port_no;
    BOOL synce_clkout;
};

struct vtss_phy_10g_xfp_clkout_get_ioc {
    vtss_port_no_t port_no;
    BOOL xfp_clkout;
};

struct vtss_phy_10g_xfp_clkout_set_ioc {
    vtss_port_no_t port_no;
    BOOL xfp_clkout;
};

struct vtss_phy_10g_status_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_status_t status;
};

struct vtss_phy_10g_reset_ioc {
    vtss_port_no_t port_no;
};

struct vtss_phy_10g_loopback_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_loopback_t loopback;
};

struct vtss_phy_10g_loopback_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_loopback_t loopback;
};

struct vtss_phy_10g_cnt_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_cnt_t cnt;
};

struct vtss_phy_10g_power_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_power_t power;
};

struct vtss_phy_10g_power_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_power_t power;
};

struct vtss_phy_10g_failover_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_failover_mode_t mode;
};

struct vtss_phy_10g_failover_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_failover_mode_t mode;
};

struct vtss_phy_10g_id_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_id_t phy_id;
};

struct vtss_phy_10g_gpio_mode_set_ioc {
    vtss_port_no_t port_no;
    vtss_gpio_10g_no_t gpio_no;
    vtss_gpio_10g_gpio_mode_t mode;
};

struct vtss_phy_10g_gpio_mode_get_ioc {
    vtss_port_no_t port_no;
    vtss_gpio_10g_no_t gpio_no;
    vtss_gpio_10g_gpio_mode_t mode;
};

struct vtss_phy_10g_gpio_read_ioc {
    vtss_port_no_t port_no;
    vtss_gpio_10g_no_t gpio_no;
    BOOL value;
};

struct vtss_phy_10g_gpio_write_ioc {
    vtss_port_no_t port_no;
    vtss_gpio_10g_no_t gpio_no;
    BOOL value;
};

struct vtss_phy_10g_event_enable_set_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_event_t ev_mask;
    BOOL enable;
};

struct vtss_phy_10g_event_enable_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_event_t ev_mask;
};

struct vtss_phy_10g_event_poll_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_event_t ev_mask;
};

struct vtss_phy_10g_edc_fw_status_get_ioc {
    vtss_port_no_t port_no;
    vtss_phy_10g_fw_status_t status;
};

#endif				/* VTSS_FEATURE_10G */

 /************************************
  * qos group 
  */

#ifdef VTSS_FEATURE_QOS
struct vtss_mep_policer_conf_get_ioc {
#ifdef VTSS_ARCH_CARACAL
    vtss_port_no_t port_no;
    vtss_prio_t prio;
    vtss_dlb_policer_conf_t conf;
#endif				/* VTSS_ARCH_CARACAL */
};

struct vtss_mep_policer_conf_set_ioc {
#ifdef VTSS_ARCH_CARACAL
    vtss_port_no_t port_no;
    vtss_prio_t prio;
    vtss_dlb_policer_conf_t conf;
#endif				/* VTSS_ARCH_CARACAL */
};

struct vtss_qos_conf_get_ioc {
    vtss_qos_conf_t conf;
};

struct vtss_qos_conf_set_ioc {
    vtss_qos_conf_t conf;
};

struct vtss_qos_port_conf_get_ioc {
    vtss_port_no_t port_no;
    vtss_qos_port_conf_t conf;
};

struct vtss_qos_port_conf_set_ioc {
    vtss_port_no_t port_no;
    vtss_qos_port_conf_t conf;
};

struct vtss_qce_init_ioc {
#ifdef VTSS_FEATURE_QCL
    vtss_qce_type_t type;
    vtss_qce_t qce;
#endif				/* VTSS_FEATURE_QCL */
};

struct vtss_qce_add_ioc {
#ifdef VTSS_FEATURE_QCL
    vtss_qcl_id_t qcl_id;
    vtss_qce_id_t qce_id;
    vtss_qce_t qce;
#endif				/* VTSS_FEATURE_QCL */
};

struct vtss_qce_del_ioc {
#ifdef VTSS_FEATURE_QCL
    vtss_qcl_id_t qcl_id;
    vtss_qce_id_t qce_id;
#endif				/* VTSS_FEATURE_QCL */
};

#endif				/* VTSS_FEATURE_QOS */

 /************************************
  * packet group 
  */

#ifdef VTSS_FEATURE_PACKET
struct vtss_npi_conf_get_ioc {
#ifdef VTSS_FEATURE_NPI
    vtss_npi_conf_t conf;
#endif				/* VTSS_FEATURE_NPI */
};

struct vtss_npi_conf_set_ioc {
#ifdef VTSS_FEATURE_NPI
    vtss_npi_conf_t conf;
#endif				/* VTSS_FEATURE_NPI */
};

struct vtss_packet_rx_conf_get_ioc {
    vtss_packet_rx_conf_t conf;
};

struct vtss_packet_rx_conf_set_ioc {
    vtss_packet_rx_conf_t conf;
};

struct vtss_packet_rx_port_conf_get_ioc {
#ifdef VTSS_FEATURE_PACKET_PORT_REG
    vtss_port_no_t port_no;
    vtss_packet_rx_port_conf_t conf;
#endif				/* VTSS_FEATURE_PACKET_PORT_REG */
};

struct vtss_packet_rx_port_conf_set_ioc {
#ifdef VTSS_FEATURE_PACKET_PORT_REG
    vtss_port_no_t port_no;
    vtss_packet_rx_port_conf_t conf;
#endif				/* VTSS_FEATURE_PACKET_PORT_REG */
};

struct vtss_packet_frame_filter_ioc {
    vtss_packet_frame_info_t info;
    vtss_packet_filter_t filter;
};

struct vtss_packet_port_info_init_ioc {
    vtss_packet_port_info_t info;
};

struct vtss_packet_port_filter_get_ioc {
    vtss_packet_port_info_t info;
    vtss_packet_port_filter_t filter[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_afi_alloc_ioc {
#ifdef VTSS_FEATURE_AFI_SWC
    vtss_afi_frm_dscr_t dscr;
    vtss_afi_id_t id;
#endif				/* VTSS_FEATURE_AFI_SWC */
};

struct vtss_afi_free_ioc {
#ifdef VTSS_FEATURE_AFI_SWC
    vtss_afi_id_t id;
#endif				/* VTSS_FEATURE_AFI_SWC */
};

struct vtss_packet_rx_hdr_decode_ioc {
    vtss_packet_rx_meta_t meta;
    vtss_packet_rx_info_t info;
};

struct vtss_packet_tx_info_init_ioc {
    vtss_packet_tx_info_t info;
};

#endif				/* VTSS_FEATURE_PACKET */

 /************************************
  * security group 
  */


struct vtss_auth_port_state_get_ioc {
#ifdef VTSS_FEATURE_LAYER2
    vtss_port_no_t port_no;
    vtss_auth_state_t state;
#endif				/* VTSS_FEATURE_LAYER2 */
};

struct vtss_auth_port_state_set_ioc {
#ifdef VTSS_FEATURE_LAYER2
    vtss_port_no_t port_no;
    vtss_auth_state_t state;
#endif				/* VTSS_FEATURE_LAYER2 */
};

struct vtss_acl_policer_conf_get_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_acl_policer_no_t policer_no;
    vtss_acl_policer_conf_t conf;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_acl_policer_conf_set_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_acl_policer_no_t policer_no;
    vtss_acl_policer_conf_t conf;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_acl_port_conf_get_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_port_no_t port_no;
    vtss_acl_port_conf_t conf;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_acl_port_conf_set_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_port_no_t port_no;
    vtss_acl_port_conf_t conf;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_acl_port_counter_get_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_port_no_t port_no;
    vtss_acl_port_counter_t counter;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_acl_port_counter_clear_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_port_no_t port_no;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_ace_init_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_ace_type_t type;
    vtss_ace_t ace;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_ace_add_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_ace_id_t ace_id;
    vtss_ace_t ace;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_ace_del_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_ace_id_t ace_id;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_ace_counter_get_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_ace_id_t ace_id;
    vtss_ace_counter_t counter;
#endif				/* VTSS_FEATURE_ACL */
};

struct vtss_ace_counter_clear_ioc {
#ifdef VTSS_FEATURE_ACL
    vtss_ace_id_t ace_id;
#endif				/* VTSS_FEATURE_ACL */
};



 /************************************
  * layer2 group 
  */

#ifdef VTSS_FEATURE_LAYER2
struct vtss_mac_table_add_ioc {
    vtss_mac_table_entry_t entry;
};

struct vtss_mac_table_del_ioc {
    vtss_vid_mac_t vid_mac;
};

struct vtss_mac_table_get_ioc {
    vtss_vid_mac_t vid_mac;
    vtss_mac_table_entry_t entry;
};

struct vtss_mac_table_get_next_ioc {
    vtss_vid_mac_t vid_mac;
    vtss_mac_table_entry_t entry;
};

struct vtss_mac_table_age_time_get_ioc {
#ifdef VTSS_FEATURE_MAC_AGE_AUTO
    vtss_mac_table_age_time_t age_time;
#endif				/* VTSS_FEATURE_MAC_AGE_AUTO */
};

struct vtss_mac_table_age_time_set_ioc {
#ifdef VTSS_FEATURE_MAC_AGE_AUTO
    vtss_mac_table_age_time_t age_time;
#endif				/* VTSS_FEATURE_MAC_AGE_AUTO */
};

struct vtss_mac_table_vlan_age_ioc {
    vtss_vid_t vid;
};

struct vtss_mac_table_port_flush_ioc {
    vtss_port_no_t port_no;
};

struct vtss_mac_table_vlan_flush_ioc {
    vtss_vid_t vid;
};

struct vtss_mac_table_vlan_port_flush_ioc {
    vtss_port_no_t port_no;
    vtss_vid_t vid;
};

struct vtss_mac_table_upsid_flush_ioc {
#ifdef VTSS_FEATURE_VSTAX_V2
    vtss_vstax_upsid_t upsid;
#endif				/* VTSS_FEATURE_VSTAX_V2 */
};

struct vtss_mac_table_upsid_upspn_flush_ioc {
#ifdef VTSS_FEATURE_VSTAX_V2
    vtss_vstax_upsid_t upsid;
    vtss_vstax_upspn_t upspn;
#endif				/* VTSS_FEATURE_VSTAX_V2 */
};

struct vtss_mac_table_glag_add_ioc {
#ifdef VTSS_FEATURE_AGGR_GLAG
    vtss_mac_table_entry_t entry;
    vtss_glag_no_t glag_no;
#endif				/* VTSS_FEATURE_AGGR_GLAG */
};

struct vtss_mac_table_glag_flush_ioc {
#ifdef VTSS_FEATURE_AGGR_GLAG
    vtss_glag_no_t glag_no;
#endif				/* VTSS_FEATURE_AGGR_GLAG */
};

struct vtss_mac_table_vlan_glag_flush_ioc {
#ifdef VTSS_FEATURE_AGGR_GLAG
    vtss_glag_no_t glag_no;
    vtss_vid_t vid;
#endif				/* VTSS_FEATURE_AGGR_GLAG */
};

struct vtss_mac_table_status_get_ioc {
    vtss_mac_table_status_t status;
};

struct vtss_learn_port_mode_get_ioc {
    vtss_port_no_t port_no;
    vtss_learn_mode_t mode;
};

struct vtss_learn_port_mode_set_ioc {
    vtss_port_no_t port_no;
    vtss_learn_mode_t mode;
};

struct vtss_port_state_get_ioc {
    vtss_port_no_t port_no;
    BOOL state;
};

struct vtss_port_state_set_ioc {
    vtss_port_no_t port_no;
    BOOL state;
};

struct vtss_stp_port_state_get_ioc {
    vtss_port_no_t port_no;
    vtss_stp_state_t state;
};

struct vtss_stp_port_state_set_ioc {
    vtss_port_no_t port_no;
    vtss_stp_state_t state;
};

struct vtss_mstp_vlan_msti_get_ioc {
    vtss_vid_t vid;
    vtss_msti_t msti;
};

struct vtss_mstp_vlan_msti_set_ioc {
    vtss_vid_t vid;
    vtss_msti_t msti;
};

struct vtss_mstp_port_msti_state_get_ioc {
    vtss_port_no_t port_no;
    vtss_msti_t msti;
    vtss_stp_state_t state;
};

struct vtss_mstp_port_msti_state_set_ioc {
    vtss_port_no_t port_no;
    vtss_msti_t msti;
    vtss_stp_state_t state;
};

struct vtss_vlan_conf_get_ioc {
    vtss_vlan_conf_t conf;
};

struct vtss_vlan_conf_set_ioc {
    vtss_vlan_conf_t conf;
};

struct vtss_vlan_port_conf_get_ioc {
    vtss_port_no_t port_no;
    vtss_vlan_port_conf_t conf;
};

struct vtss_vlan_port_conf_set_ioc {
    vtss_port_no_t port_no;
    vtss_vlan_port_conf_t conf;
};

struct vtss_vlan_port_members_get_ioc {
    vtss_vid_t vid;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_vlan_port_members_set_ioc {
    vtss_vid_t vid;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_vlan_counters_get_ioc {
#ifdef VTSS_FEATURE_VLAN_COUNTERS
    vtss_vid_t vid;
    vtss_vlan_counters_t counters;
#endif				/* VTSS_FEATURE_VLAN_COUNTERS */
};

struct vtss_vlan_counters_clear_ioc {
#ifdef VTSS_FEATURE_VLAN_COUNTERS
    vtss_vid_t vid;
#endif				/* VTSS_FEATURE_VLAN_COUNTERS */
};

struct vtss_vce_init_ioc {
    vtss_vce_type_t type;
    vtss_vce_t vce;
};

struct vtss_vce_add_ioc {
    vtss_vce_id_t vce_id;
    vtss_vce_t vce;
};

struct vtss_vce_del_ioc {
    vtss_vce_id_t vce_id;
};

struct vtss_vlan_trans_group_add_ioc {
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    u16 group_id;
    vtss_vid_t vid;
    vtss_vid_t trans_vid;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */
};

struct vtss_vlan_trans_group_del_ioc {
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    u16 group_id;
    vtss_vid_t vid;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */
};

struct vtss_vlan_trans_group_get_ioc {
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    vtss_vlan_trans_grp2vlan_conf_t conf;
    BOOL next;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */
};

struct vtss_vlan_trans_group_to_port_set_ioc {
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    vtss_vlan_trans_port2grp_conf_t conf;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */
};

struct vtss_vlan_trans_group_to_port_get_ioc {
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    vtss_vlan_trans_port2grp_conf_t conf;
    BOOL next;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */
};

struct vtss_isolated_vlan_get_ioc {
    vtss_vid_t vid;
    BOOL isolated;
};

struct vtss_isolated_vlan_set_ioc {
    vtss_vid_t vid;
    BOOL isolated;
};

struct vtss_isolated_port_members_get_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_isolated_port_members_set_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_pvlan_port_members_get_ioc {
#ifdef VTSS_FEATURE_PVLAN
    vtss_pvlan_no_t pvlan_no;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
#endif				/* VTSS_FEATURE_PVLAN */
};

struct vtss_pvlan_port_members_set_ioc {
#ifdef VTSS_FEATURE_PVLAN
    vtss_pvlan_no_t pvlan_no;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
#endif				/* VTSS_FEATURE_PVLAN */
};

struct vtss_sflow_port_conf_get_ioc {
#ifdef VTSS_FEATURE_SFLOW
    u32 port_no;
    vtss_sflow_port_conf_t conf;
#endif				/* VTSS_FEATURE_SFLOW */
};

struct vtss_sflow_port_conf_set_ioc {
#ifdef VTSS_FEATURE_SFLOW
    u32 port_no;
    vtss_sflow_port_conf_t conf;
#endif				/* VTSS_FEATURE_SFLOW */
};

struct vtss_aggr_port_members_get_ioc {
    vtss_aggr_no_t aggr_no;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_aggr_port_members_set_ioc {
    vtss_aggr_no_t aggr_no;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_aggr_mode_get_ioc {
    vtss_aggr_mode_t mode;
};

struct vtss_aggr_mode_set_ioc {
    vtss_aggr_mode_t mode;
};

struct vtss_aggr_glag_members_get_ioc {
#ifdef VTSS_FEATURE_AGGR_GLAG
    vtss_glag_no_t glag_no;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
#endif				/* VTSS_FEATURE_AGGR_GLAG */
};

struct vtss_aggr_glag_set_ioc {
#ifdef VTSS_FEATURE_VSTAX_V1
    vtss_glag_no_t glag_no;
    vtss_port_no_t member[VTSS_GLAG_PORT_ARRAY_SIZE];
#endif				/* VTSS_FEATURE_VSTAX_V1 */
};

struct vtss_vstax_glag_get_ioc {
#ifdef VTSS_FEATURE_VSTAX_V2
    vtss_glag_no_t glag_no;
    vtss_vstax_glag_entry_t entry[VTSS_GLAG_PORT_ARRAY_SIZE];
#endif				/* VTSS_FEATURE_VSTAX_V2 */
};

struct vtss_vstax_glag_set_ioc {
#ifdef VTSS_FEATURE_VSTAX_V2
    vtss_glag_no_t glag_no;
    vtss_vstax_glag_entry_t entry[VTSS_GLAG_PORT_ARRAY_SIZE];
#endif				/* VTSS_FEATURE_VSTAX_V2 */
};

struct vtss_mirror_monitor_port_get_ioc {
    vtss_port_no_t port_no;
};

struct vtss_mirror_monitor_port_set_ioc {
    vtss_port_no_t port_no;
};

struct vtss_mirror_ingress_ports_get_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_mirror_ingress_ports_set_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_mirror_egress_ports_get_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_mirror_egress_ports_set_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_mirror_cpu_ingress_get_ioc {
    BOOL member;
};

struct vtss_mirror_cpu_ingress_set_ioc {
    BOOL member;
};

struct vtss_mirror_cpu_egress_get_ioc {
    BOOL member;
};

struct vtss_mirror_cpu_egress_set_ioc {
    BOOL member;
};

struct vtss_uc_flood_members_get_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_uc_flood_members_set_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_mc_flood_members_get_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_mc_flood_members_set_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_ipv4_mc_flood_members_get_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_ipv4_mc_flood_members_set_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_ipv4_mc_add_ioc {
#ifdef VTSS_FEATURE_IPV4_MC_SIP
    vtss_vid_t vid;
    vtss_ip_t sip;
    vtss_ip_t dip;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
#endif				/* VTSS_FEATURE_IPV4_MC_SIP */
};

struct vtss_ipv4_mc_del_ioc {
#ifdef VTSS_FEATURE_IPV4_MC_SIP
    vtss_vid_t vid;
    vtss_ip_t sip;
    vtss_ip_t dip;
#endif				/* VTSS_FEATURE_IPV4_MC_SIP */
};

struct vtss_ipv6_mc_flood_members_get_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_ipv6_mc_flood_members_set_ioc {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
};

struct vtss_ipv6_mc_ctrl_flood_get_ioc {
    BOOL scope;
};

struct vtss_ipv6_mc_ctrl_flood_set_ioc {
    BOOL scope;
};

struct vtss_ipv6_mc_add_ioc {
#ifdef VTSS_FEATURE_IPV6_MC_SIP
    vtss_vid_t vid;
    vtss_ipv6_t sip;
    vtss_ipv6_t dip;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
#endif				/* VTSS_FEATURE_IPV6_MC_SIP */
};

struct vtss_ipv6_mc_del_ioc {
#ifdef VTSS_FEATURE_IPV6_MC_SIP
    vtss_vid_t vid;
    vtss_ipv6_t sip;
    vtss_ipv6_t dip;
#endif				/* VTSS_FEATURE_IPV6_MC_SIP */
};

struct vtss_eps_port_conf_get_ioc {
    vtss_port_no_t port_no;
    vtss_eps_port_conf_t conf;
};

struct vtss_eps_port_conf_set_ioc {
    vtss_port_no_t port_no;
    vtss_eps_port_conf_t conf;
};

struct vtss_eps_port_selector_get_ioc {
    vtss_port_no_t port_no;
    vtss_eps_selector_t selector;
};

struct vtss_eps_port_selector_set_ioc {
    vtss_port_no_t port_no;
    vtss_eps_selector_t selector;
};

struct vtss_erps_vlan_member_get_ioc {
    vtss_erpi_t erpi;
    vtss_vid_t vid;
    BOOL member;
};

struct vtss_erps_vlan_member_set_ioc {
    vtss_erpi_t erpi;
    vtss_vid_t vid;
    BOOL member;
};

struct vtss_erps_port_state_get_ioc {
    vtss_erpi_t erpi;
    vtss_port_no_t port_no;
    vtss_erps_state_t state;
};

struct vtss_erps_port_state_set_ioc {
    vtss_erpi_t erpi;
    vtss_port_no_t port_no;
    vtss_erps_state_t state;
};

struct vtss_vid2port_set_ioc {
#ifdef VTSS_ARCH_B2
    vtss_vid_t vid;
    vtss_port_no_t port_no;
#endif				/* VTSS_ARCH_B2 */
};

struct vtss_vid2lport_get_ioc {
#ifdef VTSS_ARCH_B2
    vtss_vid_t vid;
    vtss_lport_no_t lport_no;
#endif				/* VTSS_ARCH_B2 */
};

struct vtss_vid2lport_set_ioc {
#ifdef VTSS_ARCH_B2
    vtss_vid_t vid;
    vtss_lport_no_t lport_no;
#endif				/* VTSS_ARCH_B2 */
};

struct vtss_vstax_conf_get_ioc {
#ifdef VTSS_FEATURE_VSTAX
    vtss_vstax_conf_t conf;
#endif				/* VTSS_FEATURE_VSTAX */
};

struct vtss_vstax_conf_set_ioc {
#ifdef VTSS_FEATURE_VSTAX
    vtss_vstax_conf_t conf;
#endif				/* VTSS_FEATURE_VSTAX */
};

struct vtss_vstax_port_conf_get_ioc {
#ifdef VTSS_FEATURE_VSTAX
    vtss_chip_no_t chip_no;
    BOOL stack_port_a;
    vtss_vstax_port_conf_t conf;
#endif				/* VTSS_FEATURE_VSTAX */
};

struct vtss_vstax_port_conf_set_ioc {
#ifdef VTSS_FEATURE_VSTAX
    vtss_chip_no_t chip_no;
    BOOL stack_port_a;
    vtss_vstax_port_conf_t conf;
#endif				/* VTSS_FEATURE_VSTAX */
};

struct vtss_vstax_topology_set_ioc {
#ifdef VTSS_FEATURE_VSTAX
    vtss_chip_no_t chip_no;
    vtss_vstax_route_table_t table;
#endif				/* VTSS_FEATURE_VSTAX */
};

#endif				/* VTSS_FEATURE_LAYER2 */

 /************************************
  * evc group 
  */

#ifdef VTSS_FEATURE_EVC
struct vtss_evc_port_conf_get_ioc {
    vtss_port_no_t port_no;
    vtss_evc_port_conf_t conf;
};

struct vtss_evc_port_conf_set_ioc {
    vtss_port_no_t port_no;
    vtss_evc_port_conf_t conf;
};

struct vtss_evc_add_ioc {
    vtss_evc_id_t evc_id;
    vtss_evc_conf_t conf;
};

struct vtss_evc_del_ioc {
    vtss_evc_id_t evc_id;
};

struct vtss_evc_get_ioc {
    vtss_evc_id_t evc_id;
    vtss_evc_conf_t conf;
};

struct vtss_ece_init_ioc {
    vtss_ece_type_t type;
    vtss_ece_t ece;
};

struct vtss_ece_add_ioc {
    vtss_ece_id_t ece_id;
    vtss_ece_t ece;
};

struct vtss_ece_del_ioc {
    vtss_ece_id_t ece_id;
};

struct vtss_mce_init_ioc {
#ifdef VTSS_ARCH_CARACAL
    vtss_mce_t mce;
#endif				/* VTSS_ARCH_CARACAL */
};

struct vtss_mce_add_ioc {
#ifdef VTSS_ARCH_CARACAL
    vtss_mce_id_t mce_id;
    vtss_mce_t mce;
#endif				/* VTSS_ARCH_CARACAL */
};

struct vtss_mce_del_ioc {
#ifdef VTSS_ARCH_CARACAL
    vtss_mce_id_t mce_id;
#endif				/* VTSS_ARCH_CARACAL */
};

struct vtss_evc_counters_get_ioc {
#ifdef VTSS_ARCH_JAGUAR_1
    vtss_evc_id_t evc_id;
    vtss_port_no_t port_no;
    vtss_evc_counters_t counters;
#endif				/* VTSS_ARCH_JAGUAR_1 */
};

struct vtss_evc_counters_clear_ioc {
#ifdef VTSS_ARCH_JAGUAR_1
    vtss_evc_id_t evc_id;
    vtss_port_no_t port_no;
#endif				/* VTSS_ARCH_JAGUAR_1 */
};

struct vtss_ece_counters_get_ioc {
#ifdef VTSS_ARCH_JAGUAR_1
    vtss_ece_id_t ece_id;
    vtss_port_no_t port_no;
    vtss_evc_counters_t counters;
#endif				/* VTSS_ARCH_JAGUAR_1 */
};

struct vtss_ece_counters_clear_ioc {
#ifdef VTSS_ARCH_JAGUAR_1
    vtss_ece_id_t ece_id;
    vtss_port_no_t port_no;
#endif				/* VTSS_ARCH_JAGUAR_1 */
};

#endif				/* VTSS_FEATURE_EVC */

 /************************************
  * qos_policer_dlb group 
  */

#ifdef VTSS_FEATURE_QOS_POLICER_DLB
struct vtss_evc_policer_conf_get_ioc {
    vtss_evc_policer_id_t policer_id;
    vtss_evc_policer_conf_t conf;
};

struct vtss_evc_policer_conf_set_ioc {
    vtss_evc_policer_id_t policer_id;
    vtss_evc_policer_conf_t conf;
};

#endif				/* VTSS_FEATURE_QOS_POLICER_DLB */

 /************************************
  * timestamp group 
  */

#ifdef VTSS_FEATURE_TIMESTAMP
struct vtss_ts_timeofday_set_ioc {
    vtss_timestamp_t ts;
};

struct vtss_ts_adjtimer_one_sec_ioc {
    BOOL ongoing_adjustment;
};

struct vtss_ts_timeofday_get_ioc {
    vtss_timestamp_t ts;
    u32 tc;
};

struct vtss_ts_adjtimer_set_ioc {
    i32 adj;
};

struct vtss_ts_adjtimer_get_ioc {
    i32 adj;
};

struct vtss_ts_freq_offset_get_ioc {
    i32 adj;
};

struct vtss_ts_external_clock_mode_set_ioc {
    vtss_ts_ext_clock_mode_t ext_clock_mode;
};

struct vtss_ts_external_clock_mode_get_ioc {
    vtss_ts_ext_clock_mode_t ext_clock_mode;
};

struct vtss_ts_ingress_latency_set_ioc {
    vtss_port_no_t port_no;
    vtss_timeinterval_t ingress_latency;
};

struct vtss_ts_ingress_latency_get_ioc {
    vtss_port_no_t port_no;
    vtss_timeinterval_t ingress_latency;
};

struct vtss_ts_p2p_delay_set_ioc {
    vtss_port_no_t port_no;
    vtss_timeinterval_t p2p_delay;
};

struct vtss_ts_p2p_delay_get_ioc {
    vtss_port_no_t port_no;
    vtss_timeinterval_t p2p_delay;
};

struct vtss_ts_egress_latency_set_ioc {
    vtss_port_no_t port_no;
    vtss_timeinterval_t egress_latency;
};

struct vtss_ts_egress_latency_get_ioc {
    vtss_port_no_t port_no;
    vtss_timeinterval_t egress_latency;
};

struct vtss_ts_operation_mode_set_ioc {
    vtss_port_no_t port_no;
    vtss_ts_operation_mode_t mode;
};

struct vtss_ts_operation_mode_get_ioc {
    vtss_port_no_t port_no;
    vtss_ts_operation_mode_t mode;
};

struct vtss_rx_timestamp_get_ioc {
    vtss_ts_id_t ts_id;
    vtss_ts_timestamp_t ts;
};

struct vtss_rx_master_timestamp_get_ioc {
    vtss_port_no_t port_no;
    vtss_ts_timestamp_t ts;
};

struct vtss_tx_timestamp_idx_alloc_ioc {
#ifdef NOTDEF
    vtss_ts_timestamp_alloc_t alloc_parm;
    vtss_ts_id_t ts_id;
#endif				/* NOTDEF */
};

#endif				/* VTSS_FEATURE_TIMESTAMP */

 /************************************
  * synce group 
  */

#ifdef VTSS_FEATURE_SYNCE
struct vtss_synce_clock_out_set_ioc {
    vtss_synce_clk_port_t clk_port;
    vtss_synce_clock_out_t conf;
};

struct vtss_synce_clock_out_get_ioc {
    vtss_synce_clk_port_t clk_port;
    vtss_synce_clock_out_t conf;
};

struct vtss_synce_clock_in_set_ioc {
    vtss_synce_clk_port_t clk_port;
    vtss_synce_clock_in_t conf;
};

struct vtss_synce_clock_in_get_ioc {
    vtss_synce_clk_port_t clk_port;
    vtss_synce_clock_in_t conf;
};

#endif				/* VTSS_FEATURE_SYNCE */

 /************************************
  * ccm group 
  */


struct vtssx_ccm_start_ioc {
    const void *frame;
    size_t length;
    vtssx_ccm_opt_t opt;
    vtssx_ccm_session_t sess;
};

struct vtssx_ccm_status_get_ioc {
    vtssx_ccm_session_t sess;
    vtssx_ccm_status_t status;
};

struct vtssx_ccm_cancel_ioc {
    vtssx_ccm_session_t sess;
};



 /************************************
  * l3 group 
  */

#ifdef VTSS_SW_OPTION_L3RT
struct vtss_l3_rleg_common_get_ioc {
    vtss_l3_rleg_common_conf_t conf;
};

struct vtss_l3_rleg_common_set_ioc {
    vtss_l3_rleg_common_conf_t conf;
};

struct vtss_l3_rleg_get_ioc {
    u32 cnt;
    vtss_l3_rleg_conf_t *buf;	/* Separate copy-in/out */
};

struct vtss_l3_rleg_add_ioc {
    vtss_l3_rleg_conf_t conf;
};

struct vtss_l3_rleg_del_ioc {
    vtss_vid_t vlan;
};

struct vtss_l3_route_get_ioc {
    u32 cnt;
    vtss_routing_entry_t *buf;	/* Separate copy-in/out */
};

struct vtss_l3_route_add_ioc {
    vtss_routing_entry_t entry;
};

struct vtss_l3_route_del_ioc {
    vtss_routing_entry_t entry;
};

struct vtss_l3_neighbour_get_ioc {
    u32 cnt;
    vtss_l3_neighbour_t *buf;	/* Separate copy-in/out */
};

struct vtss_l3_neighbour_add_ioc {
    vtss_l3_neighbour_t entry;
};

struct vtss_l3_neighbour_del_ioc {
    vtss_l3_neighbour_t entry;
};

struct vtss_l3_counters_system_get_ioc {
    vtss_l3_counters_t counters;
};

struct vtss_l3_counters_rleg_get_ioc {
    vtss_l3_rleg_id_t rleg;
    vtss_l3_counters_t counters;
};

#endif				/* VTSS_SW_OPTION_L3RT */

 /************************************
  * mpls group 
  */

#ifdef VTSS_FEATURE_MPLS
struct vtss_mpls_l2_alloc_ioc {
    vtss_mpls_l2_idx_t idx;
};

struct vtss_mpls_l2_free_ioc {
    vtss_mpls_l2_idx_t idx;
};

struct vtss_mpls_l2_get_ioc {
    vtss_mpls_l2_idx_t idx;
    vtss_mpls_l2_t l2;
};

struct vtss_mpls_l2_set_ioc {
    vtss_mpls_l2_idx_t idx;
    vtss_mpls_l2_t l2;
};

struct vtss_mpls_l2_segment_attach_ioc {
    vtss_mpls_l2_idx_t idx;
    vtss_mpls_segment_idx_t seg_idx;
};

struct vtss_mpls_l2_segment_detach_ioc {
    vtss_mpls_segment_idx_t seg_idx;
};

struct vtss_mpls_segment_alloc_ioc {
    BOOL is_in;
    vtss_mpls_segment_idx_t idx;
};

struct vtss_mpls_segment_free_ioc {
    vtss_mpls_segment_idx_t idx;
};

struct vtss_mpls_segment_get_ioc {
    vtss_mpls_segment_idx_t idx;
    vtss_mpls_segment_t seg;
};

struct vtss_mpls_segment_set_ioc {
    vtss_mpls_segment_idx_t idx;
    vtss_mpls_segment_t seg;
};

struct vtss_mpls_segment_state_get_ioc {
    vtss_mpls_segment_idx_t idx;
    vtss_mpls_segment_state_t state;
};

struct vtss_mpls_segment_server_attach_ioc {
    vtss_mpls_segment_idx_t idx;
    vtss_mpls_segment_idx_t srv_idx;
};

struct vtss_mpls_segment_server_detach_ioc {
    vtss_mpls_segment_idx_t idx;
};

struct vtss_mpls_xc_alloc_ioc {
    vtss_mpls_xc_type_t type;
    vtss_mpls_xc_idx_t idx;
};

struct vtss_mpls_xc_free_ioc {
    vtss_mpls_xc_idx_t idx;
};

struct vtss_mpls_xc_get_ioc {
    vtss_mpls_xc_idx_t idx;
    vtss_mpls_xc_t xc;
};

struct vtss_mpls_xc_set_ioc {
    vtss_mpls_xc_idx_t idx;
    vtss_mpls_xc_t xc;
};

struct vtss_mpls_xc_segment_attach_ioc {
    vtss_mpls_xc_idx_t xc_idx;
    vtss_mpls_segment_idx_t seg_idx;
};

struct vtss_mpls_xc_segment_detach_ioc {
    vtss_mpls_segment_idx_t seg_idx;
};

struct vtss_mpls_xc_mc_segment_attach_ioc {
    vtss_mpls_xc_idx_t xc_idx;
    vtss_mpls_segment_idx_t seg_idx;
};

struct vtss_mpls_xc_mc_segment_detach_ioc {
    vtss_mpls_xc_idx_t xc_idx;
    vtss_mpls_segment_idx_t seg_idx;
};

struct vtss_mpls_tc_conf_get_ioc {
    vtss_mpls_tc_conf_t conf;
};

struct vtss_mpls_tc_conf_set_ioc {
    vtss_mpls_tc_conf_t conf;
};

#endif				/* VTSS_FEATURE_MPLS */

 /************************************
  * macsec group 
  */

#ifdef VTSS_FEATURE_MACSEC
struct vtss_macsec_init_set_ioc {
    vtss_port_no_t port_no;
    vtss_macsec_init_t init;
};

struct vtss_macsec_init_get_ioc {
    vtss_port_no_t port_no;
    vtss_macsec_init_t init;
};

struct vtss_macsec_secy_conf_add_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_secy_conf_t conf;
};

struct vtss_macsec_secy_conf_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_secy_conf_t conf;
};

struct vtss_macsec_secy_conf_del_ioc {
    vtss_macsec_port_t port;
};

struct vtss_macsec_secy_controlled_set_ioc {
    vtss_macsec_port_t port;
    BOOL enable;
};

struct vtss_macsec_secy_controlled_get_ioc {
    vtss_macsec_port_t port;
    BOOL enable;
};

struct vtss_macsec_secy_controlled_get_ioc {
    vtss_macsec_port_t port;
    BOOL enable;
};

struct vtss_macsec_secy_port_status_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_secy_port_status_t status;
};

struct vtss_macsec_rx_sc_add_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
};

struct vtss_macsec_rx_sc_get_next_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t search_sci;
    vtss_macsec_sci_t found_sci;
};

struct vtss_macsec_rx_sc_del_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
};

struct vtss_macsec_rx_sc_status_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    vtss_macsec_rx_sc_status_t status;
};

struct vtss_macsec_tx_sc_set_ioc {
    vtss_macsec_port_t port;
};

struct vtss_macsec_tx_sc_del_ioc {
    vtss_macsec_port_t port;
};

struct vtss_macsec_tx_sc_status_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_tx_sc_status_t status;
};

struct vtss_macsec_rx_sa_set_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    u16 an;
    u32 lowest_pn;
    vtss_macsec_sak_t sak;
};

struct vtss_macsec_rx_sa_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    u16 an;
    u32 lowest_pn;
    vtss_macsec_sak_t sak;
    BOOL active;
};

struct vtss_macsec_rx_sa_activate_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    u16 an;
};

struct vtss_macsec_rx_sa_disable_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    u16 an;
};

struct vtss_macsec_rx_sa_del_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    u16 an;
};

struct vtss_macsec_rx_sa_lowest_pn_update_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    u16 an;
    u32 lowest_pn;
};

struct vtss_macsec_rx_sa_status_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    u16 an;
    vtss_macsec_rx_sa_status_t status;
};

struct vtss_macsec_tx_sa_set_ioc {
    vtss_macsec_port_t port;
    u16 an;
    u32 next_pn;
    BOOL confidentiality;
    vtss_macsec_sak_t sak;
};

struct vtss_macsec_tx_sa_get_ioc {
    vtss_macsec_port_t port;
    u16 an;
    u32 next_pn;
    BOOL confidentiality;
    vtss_macsec_sak_t sak;
    BOOL active;
};

struct vtss_macsec_tx_sa_activate_ioc {
    vtss_macsec_port_t port;
    u16 an;
};

struct vtss_macsec_tx_sa_disable_ioc {
    vtss_macsec_port_t port;
    u16 an;
};

struct vtss_macsec_tx_sa_del_ioc {
    vtss_macsec_port_t port;
    u16 an;
};

struct vtss_macsec_tx_sa_status_get_ioc {
    vtss_macsec_port_t port;
    u16 an;
    vtss_macsec_tx_sa_status_t status;
};

struct vtss_macsec_secy_port_counters_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_secy_port_counters_t counters;
};

struct vtss_macsec_secy_cap_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_secy_cap_t cap;
};

struct vtss_macsec_secy_counters_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_secy_counters_t counters;
};

struct vtss_macsec_counters_update_ioc {
    vtss_port_no_t port_no;
};

struct vtss_macsec_counters_clear_ioc {
    vtss_port_no_t port_no;
};

struct vtss_macsec_rx_sc_counters_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    vtss_macsec_rx_sc_counters_t counters;
};

struct vtss_macsec_tx_sc_counters_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_tx_sc_counters_t counters;
};

struct vtss_macsec_tx_sa_counters_get_ioc {
    vtss_macsec_port_t port;
    u16 an;
    vtss_macsec_tx_sa_counters_t counters;
};

struct vtss_macsec_rx_sa_counters_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_sci_t sci;
    u16 an;
    vtss_macsec_rx_sa_counters_t counters;
};

struct vtss_macsec_control_frame_match_conf_set_ioc {
    vtss_port_no_t port;
    vtss_macsec_control_frame_match_conf_t conf;
};

struct vtss_macsec_control_frame_match_conf_get_ioc {
    vtss_port_no_t port;
    vtss_macsec_control_frame_match_conf_t conf;
};

struct vtss_macsec_pattern_set_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_direction_t direction;
    vtss_macsec_match_action_t action;
    vtss_macsec_match_pattern_t pattern;
};

struct vtss_macsec_pattern_del_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_direction_t direction;
    vtss_macsec_match_action_t action;
};

struct vtss_macsec_pattern_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_direction_t direction;
    vtss_macsec_match_action_t action;
    vtss_macsec_match_pattern_t pattern;
};

struct vtss_macsec_default_action_set_ioc {
    vtss_port_no_t port;
    vtss_macsec_default_action_policy_t pattern;
};

struct vtss_macsec_default_action_get_ioc {
    vtss_port_no_t port;
    vtss_macsec_default_action_policy_t pattern;
};

struct vtss_macsec_bypass_mode_set_ioc {
    vtss_port_no_t port_no;
    vtss_macsec_bypass_mode_t mode;
};

struct vtss_macsec_bypass_mode_get_ioc {
    vtss_port_no_t port_no;
    vtss_macsec_bypass_mode_t mode;
};

struct vtss_macsec_bypass_tag_set_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_tag_bypass_t tag;
};

struct vtss_macsec_bypass_tag_get_ioc {
    vtss_macsec_port_t port;
    vtss_macsec_tag_bypass_t tag;
};

struct vtss_macsec_frame_capture_set_ioc {
    vtss_port_no_t port_no;
    vtss_macsec_frame_capture_t capture;
};

struct vtss_macsec_port_loopback_set_ioc {
    vtss_port_no_t port_no;
    BOOL enable;
};

struct vtss_macsec_frame_get_ioc {
    vtss_port_no_t port_no;
    u32 buf_length;
    u32 return_length;
    u8 frame;
};

#endif				/* VTSS_FEATURE_MACSEC */



 /************************************
  * port group 
  */


#define SWIOC_vtss_port_map_set                  _IOR ('A',  1, struct vtss_port_map_set_ioc)
#define SWIOC_vtss_port_map_get                  _IOW ('A',  2, struct vtss_port_map_get_ioc)
#define SWIOC_vtss_port_mmd_read                 _IOWR('A',  3, struct vtss_port_mmd_read_ioc)
#define SWIOC_vtss_port_mmd_write                _IOR ('A',  4, struct vtss_port_mmd_write_ioc)
#define SWIOC_vtss_port_mmd_masked_write         _IOR ('A',  5, struct vtss_port_mmd_masked_write_ioc)
#define SWIOC_vtss_mmd_read                      _IOWR('A',  6, struct vtss_mmd_read_ioc)
#define SWIOC_vtss_mmd_write                     _IOR ('A',  7, struct vtss_mmd_write_ioc)
#define SWIOC_vtss_port_clause_37_control_get    _IOWR('A',  8, struct vtss_port_clause_37_control_get_ioc)
#define SWIOC_vtss_port_clause_37_control_set    _IOR ('A',  9, struct vtss_port_clause_37_control_set_ioc)
#define SWIOC_vtss_port_conf_set                 _IOR ('A', 10, struct vtss_port_conf_set_ioc)
#define SWIOC_vtss_port_conf_get                 _IOWR('A', 11, struct vtss_port_conf_get_ioc)
#define SWIOC_vtss_port_status_get               _IOWR('A', 12, struct vtss_port_status_get_ioc)
#define SWIOC_vtss_port_counters_update          _IOR ('A', 13, struct vtss_port_counters_update_ioc)
#define SWIOC_vtss_port_counters_clear           _IOR ('A', 14, struct vtss_port_counters_clear_ioc)
#define SWIOC_vtss_port_counters_get             _IOWR('A', 15, struct vtss_port_counters_get_ioc)
#define SWIOC_vtss_port_basic_counters_get       _IOWR('A', 16, struct vtss_port_basic_counters_get_ioc)
#define SWIOC_vtss_port_forward_state_get        _IOWR('A', 17, struct vtss_port_forward_state_get_ioc)
#define SWIOC_vtss_port_forward_state_set        _IOR ('A', 18, struct vtss_port_forward_state_set_ioc)
#define SWIOC_vtss_miim_read                     _IOWR('A', 19, struct vtss_miim_read_ioc)
#define SWIOC_vtss_miim_write                    _IOR ('A', 20, struct vtss_miim_write_ioc)


 /************************************
  * misc group 
  */


#define SWIOC_vtss_debug_info_print              _IOR ('B',  1, struct vtss_debug_info_print_ioc)
#define SWIOC_vtss_reg_read                      _IOWR('B',  2, struct vtss_reg_read_ioc)
#define SWIOC_vtss_reg_write                     _IOR ('B',  3, struct vtss_reg_write_ioc)
#define SWIOC_vtss_reg_write_masked              _IOR ('B',  4, struct vtss_reg_write_masked_ioc)
#define SWIOC_vtss_chip_id_get                   _IOW ('B',  5, struct vtss_chip_id_get_ioc)
#define SWIOC_vtss_poll_1sec                     _IO  ('B',  6)
#define SWIOC_vtss_ptp_event_poll                _IOW ('B',  7, struct vtss_ptp_event_poll_ioc)
#define SWIOC_vtss_ptp_event_enable              _IOR ('B',  8, struct vtss_ptp_event_enable_ioc)
#define SWIOC_vtss_dev_all_event_poll            _IOWR('B',  9, struct vtss_dev_all_event_poll_ioc)
#define SWIOC_vtss_dev_all_event_enable          _IOR ('B', 10, struct vtss_dev_all_event_enable_ioc)
#define SWIOC_vtss_gpio_mode_set                 _IOR ('B', 11, struct vtss_gpio_mode_set_ioc)
#define SWIOC_vtss_gpio_direction_set            _IOR ('B', 12, struct vtss_gpio_direction_set_ioc)
#define SWIOC_vtss_gpio_read                     _IOWR('B', 13, struct vtss_gpio_read_ioc)
#define SWIOC_vtss_gpio_write                    _IOR ('B', 14, struct vtss_gpio_write_ioc)
#define SWIOC_vtss_gpio_event_poll               _IOWR('B', 15, struct vtss_gpio_event_poll_ioc)
#define SWIOC_vtss_gpio_event_enable             _IOR ('B', 16, struct vtss_gpio_event_enable_ioc)
#define SWIOC_vtss_gpio_clk_set                  _IOR ('B', 17, struct vtss_gpio_clk_set_ioc)
#define SWIOC_vtss_serial_led_set                _IOR ('B', 18, struct vtss_serial_led_set_ioc)
#define SWIOC_vtss_serial_led_intensity_set      _IOR ('B', 19, struct vtss_serial_led_intensity_set_ioc)
#define SWIOC_vtss_serial_led_intensity_get      _IOW ('B', 20, struct vtss_serial_led_intensity_get_ioc)
#define SWIOC_vtss_sgpio_conf_get                _IOWR('B', 21, struct vtss_sgpio_conf_get_ioc)
#define SWIOC_vtss_sgpio_conf_set                _IOR ('B', 22, struct vtss_sgpio_conf_set_ioc)
#define SWIOC_vtss_sgpio_read                    _IOWR('B', 23, struct vtss_sgpio_read_ioc)
#define SWIOC_vtss_sgpio_event_poll              _IOWR('B', 24, struct vtss_sgpio_event_poll_ioc)
#define SWIOC_vtss_sgpio_event_enable            _IOR ('B', 25, struct vtss_sgpio_event_enable_ioc)
#define SWIOC_vtss_temp_sensor_init              _IOR ('B', 26, struct vtss_temp_sensor_init_ioc)
#define SWIOC_vtss_temp_sensor_get               _IOW ('B', 27, struct vtss_temp_sensor_get_ioc)
#define SWIOC_vtss_fan_rotation_get              _IOW ('B', 28, struct vtss_fan_rotation_get_ioc)
#define SWIOC_vtss_fan_cool_lvl_set              _IOR ('B', 29, struct vtss_fan_cool_lvl_set_ioc)
#define SWIOC_vtss_fan_controller_init           _IOR ('B', 30, struct vtss_fan_controller_init_ioc)
#define SWIOC_vtss_fan_cool_lvl_get              _IOW ('B', 31, struct vtss_fan_cool_lvl_get_ioc)
#define SWIOC_vtss_eee_port_conf_set             _IOR ('B', 32, struct vtss_eee_port_conf_set_ioc)
#define SWIOC_vtss_eee_port_state_set            _IOR ('B', 33, struct vtss_eee_port_state_set_ioc)
#define SWIOC_vtss_eee_port_counter_get          _IOWR('B', 34, struct vtss_eee_port_counter_get_ioc)


 /************************************
  * init group 
  */


#define SWIOC_vtss_restart_status_get            _IOW ('C',  1, struct vtss_restart_status_get_ioc)
#define SWIOC_vtss_restart_conf_get              _IOW ('C',  2, struct vtss_restart_conf_get_ioc)
#define SWIOC_vtss_restart_conf_set              _IOR ('C',  3, struct vtss_restart_conf_set_ioc)


 /************************************
  * phy group 
  */


#define SWIOC_vtss_phy_pre_reset                 _IOR ('D',  1, struct vtss_phy_pre_reset_ioc)
#define SWIOC_vtss_phy_post_reset                _IOR ('D',  2, struct vtss_phy_post_reset_ioc)
#define SWIOC_vtss_phy_reset                     _IOR ('D',  3, struct vtss_phy_reset_ioc)
#define SWIOC_vtss_phy_chip_temp_get             _IOWR('D',  4, struct vtss_phy_chip_temp_get_ioc)
#define SWIOC_vtss_phy_chip_temp_init            _IOR ('D',  5, struct vtss_phy_chip_temp_init_ioc)
#define SWIOC_vtss_phy_conf_get                  _IOWR('D',  6, struct vtss_phy_conf_get_ioc)
#define SWIOC_vtss_phy_conf_set                  _IOR ('D',  7, struct vtss_phy_conf_set_ioc)
#define SWIOC_vtss_phy_status_get                _IOWR('D',  8, struct vtss_phy_status_get_ioc)
#define SWIOC_vtss_phy_conf_1g_get               _IOWR('D',  9, struct vtss_phy_conf_1g_get_ioc)
#define SWIOC_vtss_phy_conf_1g_set               _IOR ('D', 10, struct vtss_phy_conf_1g_set_ioc)
#define SWIOC_vtss_phy_status_1g_get             _IOWR('D', 11, struct vtss_phy_status_1g_get_ioc)
#define SWIOC_vtss_phy_power_conf_get            _IOWR('D', 12, struct vtss_phy_power_conf_get_ioc)
#define SWIOC_vtss_phy_power_conf_set            _IOR ('D', 13, struct vtss_phy_power_conf_set_ioc)
#define SWIOC_vtss_phy_power_status_get          _IOWR('D', 14, struct vtss_phy_power_status_get_ioc)
#define SWIOC_vtss_phy_clock_conf_set            _IOR ('D', 15, struct vtss_phy_clock_conf_set_ioc)
#define SWIOC_vtss_phy_read                      _IOWR('D', 16, struct vtss_phy_read_ioc)
#define SWIOC_vtss_phy_write                     _IOR ('D', 17, struct vtss_phy_write_ioc)
#define SWIOC_vtss_phy_mmd_read                  _IOWR('D', 18, struct vtss_phy_mmd_read_ioc)
#define SWIOC_vtss_phy_mmd_write                 _IOR ('D', 19, struct vtss_phy_mmd_write_ioc)
#define SWIOC_vtss_phy_write_masked              _IOR ('D', 20, struct vtss_phy_write_masked_ioc)
#define SWIOC_vtss_phy_write_masked_page         _IOR ('D', 21, struct vtss_phy_write_masked_page_ioc)
#define SWIOC_vtss_phy_veriphy_start             _IOR ('D', 22, struct vtss_phy_veriphy_start_ioc)
#define SWIOC_vtss_phy_veriphy_get               _IOWR('D', 23, struct vtss_phy_veriphy_get_ioc)
#define SWIOC_vtss_phy_led_intensity_set         _IOR ('D', 24, struct vtss_phy_led_intensity_set_ioc)
#define SWIOC_vtss_phy_enhanced_led_control_init _IOR ('D', 25, struct vtss_phy_enhanced_led_control_init_ioc)
#define SWIOC_vtss_phy_coma_mode_disable         _IOR ('D', 26, struct vtss_phy_coma_mode_disable_ioc)
#define SWIOC_vtss_phy_eee_ena                   _IOR ('D', 27, struct vtss_phy_eee_ena_ioc)
#define SWIOC_vtss_phy_event_enable_set          _IOR ('D', 28, struct vtss_phy_event_enable_set_ioc)
#define SWIOC_vtss_phy_event_enable_get          _IOWR('D', 29, struct vtss_phy_event_enable_get_ioc)
#define SWIOC_vtss_phy_event_poll                _IOWR('D', 30, struct vtss_phy_event_poll_ioc)


 /************************************
  * 10gphy group 
  */

#ifdef VTSS_FEATURE_10G
#define SWIOC_vtss_phy_10g_mode_get              _IOWR('E',  1, struct vtss_phy_10g_mode_get_ioc)
#define SWIOC_vtss_phy_10g_mode_set              _IOR ('E',  2, struct vtss_phy_10g_mode_set_ioc)
#define SWIOC_vtss_phy_10g_synce_clkout_get      _IOWR('E',  3, struct vtss_phy_10g_synce_clkout_get_ioc)
#define SWIOC_vtss_phy_10g_synce_clkout_set      _IOR ('E',  4, struct vtss_phy_10g_synce_clkout_set_ioc)
#define SWIOC_vtss_phy_10g_xfp_clkout_get        _IOWR('E',  5, struct vtss_phy_10g_xfp_clkout_get_ioc)
#define SWIOC_vtss_phy_10g_xfp_clkout_set        _IOR ('E',  6, struct vtss_phy_10g_xfp_clkout_set_ioc)
#define SWIOC_vtss_phy_10g_status_get            _IOWR('E',  7, struct vtss_phy_10g_status_get_ioc)
#define SWIOC_vtss_phy_10g_reset                 _IOR ('E',  8, struct vtss_phy_10g_reset_ioc)
#define SWIOC_vtss_phy_10g_loopback_set          _IOR ('E',  9, struct vtss_phy_10g_loopback_set_ioc)
#define SWIOC_vtss_phy_10g_loopback_get          _IOWR('E', 10, struct vtss_phy_10g_loopback_get_ioc)
#define SWIOC_vtss_phy_10g_cnt_get               _IOWR('E', 11, struct vtss_phy_10g_cnt_get_ioc)
#define SWIOC_vtss_phy_10g_power_get             _IOWR('E', 12, struct vtss_phy_10g_power_get_ioc)
#define SWIOC_vtss_phy_10g_power_set             _IOR ('E', 13, struct vtss_phy_10g_power_set_ioc)
#define SWIOC_vtss_phy_10g_failover_set          _IOWR('E', 14, struct vtss_phy_10g_failover_set_ioc)
#define SWIOC_vtss_phy_10g_failover_get          _IOWR('E', 15, struct vtss_phy_10g_failover_get_ioc)
#define SWIOC_vtss_phy_10g_id_get                _IOWR('E', 16, struct vtss_phy_10g_id_get_ioc)
#define SWIOC_vtss_phy_10g_gpio_mode_set         _IOR ('E', 17, struct vtss_phy_10g_gpio_mode_set_ioc)
#define SWIOC_vtss_phy_10g_gpio_mode_get         _IOWR('E', 18, struct vtss_phy_10g_gpio_mode_get_ioc)
#define SWIOC_vtss_phy_10g_gpio_read             _IOWR('E', 19, struct vtss_phy_10g_gpio_read_ioc)
#define SWIOC_vtss_phy_10g_gpio_write            _IOR ('E', 20, struct vtss_phy_10g_gpio_write_ioc)
#define SWIOC_vtss_phy_10g_event_enable_set      _IOR ('E', 21, struct vtss_phy_10g_event_enable_set_ioc)
#define SWIOC_vtss_phy_10g_event_enable_get      _IOWR('E', 22, struct vtss_phy_10g_event_enable_get_ioc)
#define SWIOC_vtss_phy_10g_event_poll            _IOWR('E', 23, struct vtss_phy_10g_event_poll_ioc)
#define SWIOC_vtss_phy_10g_poll_1sec             _IO  ('E', 24)
#define SWIOC_vtss_phy_10g_edc_fw_status_get     _IOWR('E', 25, struct vtss_phy_10g_edc_fw_status_get_ioc)
#endif				/* VTSS_FEATURE_10G */

 /************************************
  * qos group 
  */

#ifdef VTSS_FEATURE_QOS
#define SWIOC_vtss_mep_policer_conf_get          _IOWR('F',  1, struct vtss_mep_policer_conf_get_ioc)
#define SWIOC_vtss_mep_policer_conf_set          _IOR ('F',  2, struct vtss_mep_policer_conf_set_ioc)
#define SWIOC_vtss_qos_conf_get                  _IOW ('F',  3, struct vtss_qos_conf_get_ioc)
#define SWIOC_vtss_qos_conf_set                  _IOR ('F',  4, struct vtss_qos_conf_set_ioc)
#define SWIOC_vtss_qos_port_conf_get             _IOWR('F',  5, struct vtss_qos_port_conf_get_ioc)
#define SWIOC_vtss_qos_port_conf_set             _IOR ('F',  6, struct vtss_qos_port_conf_set_ioc)
#define SWIOC_vtss_qce_init                      _IOWR('F',  7, struct vtss_qce_init_ioc)
#define SWIOC_vtss_qce_add                       _IOR ('F',  8, struct vtss_qce_add_ioc)
#define SWIOC_vtss_qce_del                       _IOR ('F',  9, struct vtss_qce_del_ioc)
#endif				/* VTSS_FEATURE_QOS */

 /************************************
  * packet group 
  */

#ifdef VTSS_FEATURE_PACKET
#define SWIOC_vtss_npi_conf_get                  _IOW ('G',  1, struct vtss_npi_conf_get_ioc)
#define SWIOC_vtss_npi_conf_set                  _IOR ('G',  2, struct vtss_npi_conf_set_ioc)
#define SWIOC_vtss_packet_rx_conf_get            _IOW ('G',  3, struct vtss_packet_rx_conf_get_ioc)
#define SWIOC_vtss_packet_rx_conf_set            _IOR ('G',  4, struct vtss_packet_rx_conf_set_ioc)
#define SWIOC_vtss_packet_rx_port_conf_get       _IOWR('G',  5, struct vtss_packet_rx_port_conf_get_ioc)
#define SWIOC_vtss_packet_rx_port_conf_set       _IOR ('G',  6, struct vtss_packet_rx_port_conf_set_ioc)
#define SWIOC_vtss_packet_frame_filter           _IOWR('G',  7, struct vtss_packet_frame_filter_ioc)
#define SWIOC_vtss_packet_port_info_init         _IOW ('G',  8, struct vtss_packet_port_info_init_ioc)
#define SWIOC_vtss_packet_port_filter_get        _IOWR('G',  9, struct vtss_packet_port_filter_get_ioc)
#define SWIOC_vtss_afi_alloc                     _IOWR('G', 10, struct vtss_afi_alloc_ioc)
#define SWIOC_vtss_afi_free                      _IOR ('G', 11, struct vtss_afi_free_ioc)
#define SWIOC_vtss_packet_rx_hdr_decode          _IOWR('G', 12, struct vtss_packet_rx_hdr_decode_ioc)
#define SWIOC_vtss_packet_tx_info_init           _IOW ('G', 13, struct vtss_packet_tx_info_init_ioc)
#endif				/* VTSS_FEATURE_PACKET */

 /************************************
  * security group 
  */


#define SWIOC_vtss_auth_port_state_get           _IOWR('H',  1, struct vtss_auth_port_state_get_ioc)
#define SWIOC_vtss_auth_port_state_set           _IOR ('H',  2, struct vtss_auth_port_state_set_ioc)
#define SWIOC_vtss_acl_policer_conf_get          _IOWR('H',  3, struct vtss_acl_policer_conf_get_ioc)
#define SWIOC_vtss_acl_policer_conf_set          _IOR ('H',  4, struct vtss_acl_policer_conf_set_ioc)
#define SWIOC_vtss_acl_port_conf_get             _IOWR('H',  5, struct vtss_acl_port_conf_get_ioc)
#define SWIOC_vtss_acl_port_conf_set             _IOR ('H',  6, struct vtss_acl_port_conf_set_ioc)
#define SWIOC_vtss_acl_port_counter_get          _IOWR('H',  7, struct vtss_acl_port_counter_get_ioc)
#define SWIOC_vtss_acl_port_counter_clear        _IOR ('H',  8, struct vtss_acl_port_counter_clear_ioc)
#define SWIOC_vtss_ace_init                      _IOWR('H',  9, struct vtss_ace_init_ioc)
#define SWIOC_vtss_ace_add                       _IOR ('H', 10, struct vtss_ace_add_ioc)
#define SWIOC_vtss_ace_del                       _IOR ('H', 11, struct vtss_ace_del_ioc)
#define SWIOC_vtss_ace_counter_get               _IOWR('H', 12, struct vtss_ace_counter_get_ioc)
#define SWIOC_vtss_ace_counter_clear             _IOR ('H', 13, struct vtss_ace_counter_clear_ioc)


 /************************************
  * layer2 group 
  */

#ifdef VTSS_FEATURE_LAYER2
#define SWIOC_vtss_mac_table_add                 _IOR ('I',  1, struct vtss_mac_table_add_ioc)
#define SWIOC_vtss_mac_table_del                 _IOR ('I',  2, struct vtss_mac_table_del_ioc)
#define SWIOC_vtss_mac_table_get                 _IOWR('I',  3, struct vtss_mac_table_get_ioc)
#define SWIOC_vtss_mac_table_get_next            _IOWR('I',  4, struct vtss_mac_table_get_next_ioc)
#define SWIOC_vtss_mac_table_age_time_get        _IOW ('I',  5, struct vtss_mac_table_age_time_get_ioc)
#define SWIOC_vtss_mac_table_age_time_set        _IOR ('I',  6, struct vtss_mac_table_age_time_set_ioc)
#define SWIOC_vtss_mac_table_age                 _IO  ('I',  7)
#define SWIOC_vtss_mac_table_vlan_age            _IOR ('I',  8, struct vtss_mac_table_vlan_age_ioc)
#define SWIOC_vtss_mac_table_flush               _IO  ('I',  9)
#define SWIOC_vtss_mac_table_port_flush          _IOR ('I', 10, struct vtss_mac_table_port_flush_ioc)
#define SWIOC_vtss_mac_table_vlan_flush          _IOR ('I', 11, struct vtss_mac_table_vlan_flush_ioc)
#define SWIOC_vtss_mac_table_vlan_port_flush     _IOR ('I', 12, struct vtss_mac_table_vlan_port_flush_ioc)
#define SWIOC_vtss_mac_table_upsid_flush         _IOR ('I', 13, struct vtss_mac_table_upsid_flush_ioc)
#define SWIOC_vtss_mac_table_upsid_upspn_flush   _IOR ('I', 14, struct vtss_mac_table_upsid_upspn_flush_ioc)
#define SWIOC_vtss_mac_table_glag_add            _IOR ('I', 15, struct vtss_mac_table_glag_add_ioc)
#define SWIOC_vtss_mac_table_glag_flush          _IOR ('I', 16, struct vtss_mac_table_glag_flush_ioc)
#define SWIOC_vtss_mac_table_vlan_glag_flush     _IOR ('I', 17, struct vtss_mac_table_vlan_glag_flush_ioc)
#define SWIOC_vtss_mac_table_status_get          _IOW ('I', 18, struct vtss_mac_table_status_get_ioc)
#define SWIOC_vtss_learn_port_mode_get           _IOWR('I', 19, struct vtss_learn_port_mode_get_ioc)
#define SWIOC_vtss_learn_port_mode_set           _IOR ('I', 20, struct vtss_learn_port_mode_set_ioc)
#define SWIOC_vtss_port_state_get                _IOWR('I', 21, struct vtss_port_state_get_ioc)
#define SWIOC_vtss_port_state_set                _IOR ('I', 22, struct vtss_port_state_set_ioc)
#define SWIOC_vtss_stp_port_state_get            _IOWR('I', 23, struct vtss_stp_port_state_get_ioc)
#define SWIOC_vtss_stp_port_state_set            _IOR ('I', 24, struct vtss_stp_port_state_set_ioc)
#define SWIOC_vtss_mstp_vlan_msti_get            _IOWR('I', 25, struct vtss_mstp_vlan_msti_get_ioc)
#define SWIOC_vtss_mstp_vlan_msti_set            _IOR ('I', 26, struct vtss_mstp_vlan_msti_set_ioc)
#define SWIOC_vtss_mstp_port_msti_state_get      _IOWR('I', 27, struct vtss_mstp_port_msti_state_get_ioc)
#define SWIOC_vtss_mstp_port_msti_state_set      _IOR ('I', 28, struct vtss_mstp_port_msti_state_set_ioc)
#define SWIOC_vtss_vlan_conf_get                 _IOW ('I', 29, struct vtss_vlan_conf_get_ioc)
#define SWIOC_vtss_vlan_conf_set                 _IOR ('I', 30, struct vtss_vlan_conf_set_ioc)
#define SWIOC_vtss_vlan_port_conf_get            _IOWR('I', 31, struct vtss_vlan_port_conf_get_ioc)
#define SWIOC_vtss_vlan_port_conf_set            _IOR ('I', 32, struct vtss_vlan_port_conf_set_ioc)
#define SWIOC_vtss_vlan_port_members_get         _IOWR('I', 33, struct vtss_vlan_port_members_get_ioc)
#define SWIOC_vtss_vlan_port_members_set         _IOR ('I', 34, struct vtss_vlan_port_members_set_ioc)
#define SWIOC_vtss_vlan_counters_get             _IOWR('I', 35, struct vtss_vlan_counters_get_ioc)
#define SWIOC_vtss_vlan_counters_clear           _IOR ('I', 36, struct vtss_vlan_counters_clear_ioc)
#define SWIOC_vtss_vce_init                      _IOWR('I', 37, struct vtss_vce_init_ioc)
#define SWIOC_vtss_vce_add                       _IOR ('I', 38, struct vtss_vce_add_ioc)
#define SWIOC_vtss_vce_del                       _IOR ('I', 39, struct vtss_vce_del_ioc)
#define SWIOC_vtss_vlan_trans_group_add          _IOR ('I', 40, struct vtss_vlan_trans_group_add_ioc)
#define SWIOC_vtss_vlan_trans_group_del          _IOR ('I', 41, struct vtss_vlan_trans_group_del_ioc)
#define SWIOC_vtss_vlan_trans_group_get          _IOWR('I', 42, struct vtss_vlan_trans_group_get_ioc)
#define SWIOC_vtss_vlan_trans_group_to_port_set  _IOR ('I', 43, struct vtss_vlan_trans_group_to_port_set_ioc)
#define SWIOC_vtss_vlan_trans_group_to_port_get  _IOWR('I', 44, struct vtss_vlan_trans_group_to_port_get_ioc)
#define SWIOC_vtss_isolated_vlan_get             _IOWR('I', 45, struct vtss_isolated_vlan_get_ioc)
#define SWIOC_vtss_isolated_vlan_set             _IOR ('I', 46, struct vtss_isolated_vlan_set_ioc)
#define SWIOC_vtss_isolated_port_members_get     _IOW ('I', 47, struct vtss_isolated_port_members_get_ioc)
#define SWIOC_vtss_isolated_port_members_set     _IOR ('I', 48, struct vtss_isolated_port_members_set_ioc)
#define SWIOC_vtss_pvlan_port_members_get        _IOWR('I', 49, struct vtss_pvlan_port_members_get_ioc)
#define SWIOC_vtss_pvlan_port_members_set        _IOR ('I', 50, struct vtss_pvlan_port_members_set_ioc)
#define SWIOC_vtss_sflow_port_conf_get           _IOWR('I', 51, struct vtss_sflow_port_conf_get_ioc)
#define SWIOC_vtss_sflow_port_conf_set           _IOR ('I', 52, struct vtss_sflow_port_conf_set_ioc)
#define SWIOC_vtss_aggr_port_members_get         _IOWR('I', 53, struct vtss_aggr_port_members_get_ioc)
#define SWIOC_vtss_aggr_port_members_set         _IOR ('I', 54, struct vtss_aggr_port_members_set_ioc)
#define SWIOC_vtss_aggr_mode_get                 _IOW ('I', 55, struct vtss_aggr_mode_get_ioc)
#define SWIOC_vtss_aggr_mode_set                 _IOR ('I', 56, struct vtss_aggr_mode_set_ioc)
#define SWIOC_vtss_aggr_glag_members_get         _IOWR('I', 57, struct vtss_aggr_glag_members_get_ioc)
#define SWIOC_vtss_aggr_glag_set                 _IOR ('I', 58, struct vtss_aggr_glag_set_ioc)
#define SWIOC_vtss_vstax_glag_get                _IOWR('I', 59, struct vtss_vstax_glag_get_ioc)
#define SWIOC_vtss_vstax_glag_set                _IOR ('I', 60, struct vtss_vstax_glag_set_ioc)
#define SWIOC_vtss_mirror_monitor_port_get       _IOW ('I', 61, struct vtss_mirror_monitor_port_get_ioc)
#define SWIOC_vtss_mirror_monitor_port_set       _IOR ('I', 62, struct vtss_mirror_monitor_port_set_ioc)
#define SWIOC_vtss_mirror_ingress_ports_get      _IOW ('I', 63, struct vtss_mirror_ingress_ports_get_ioc)
#define SWIOC_vtss_mirror_ingress_ports_set      _IOR ('I', 64, struct vtss_mirror_ingress_ports_set_ioc)
#define SWIOC_vtss_mirror_egress_ports_get       _IOW ('I', 65, struct vtss_mirror_egress_ports_get_ioc)
#define SWIOC_vtss_mirror_egress_ports_set       _IOR ('I', 66, struct vtss_mirror_egress_ports_set_ioc)
#define SWIOC_vtss_mirror_cpu_ingress_get        _IOW ('I', 67, struct vtss_mirror_cpu_ingress_get_ioc)
#define SWIOC_vtss_mirror_cpu_ingress_set        _IOR ('I', 68, struct vtss_mirror_cpu_ingress_set_ioc)
#define SWIOC_vtss_mirror_cpu_egress_get         _IOW ('I', 69, struct vtss_mirror_cpu_egress_get_ioc)
#define SWIOC_vtss_mirror_cpu_egress_set         _IOR ('I', 70, struct vtss_mirror_cpu_egress_set_ioc)
#define SWIOC_vtss_uc_flood_members_get          _IOW ('I', 71, struct vtss_uc_flood_members_get_ioc)
#define SWIOC_vtss_uc_flood_members_set          _IOR ('I', 72, struct vtss_uc_flood_members_set_ioc)
#define SWIOC_vtss_mc_flood_members_get          _IOW ('I', 73, struct vtss_mc_flood_members_get_ioc)
#define SWIOC_vtss_mc_flood_members_set          _IOR ('I', 74, struct vtss_mc_flood_members_set_ioc)
#define SWIOC_vtss_ipv4_mc_flood_members_get     _IOW ('I', 75, struct vtss_ipv4_mc_flood_members_get_ioc)
#define SWIOC_vtss_ipv4_mc_flood_members_set     _IOR ('I', 76, struct vtss_ipv4_mc_flood_members_set_ioc)
#define SWIOC_vtss_ipv4_mc_add                   _IOR ('I', 77, struct vtss_ipv4_mc_add_ioc)
#define SWIOC_vtss_ipv4_mc_del                   _IOR ('I', 78, struct vtss_ipv4_mc_del_ioc)
#define SWIOC_vtss_ipv6_mc_flood_members_get     _IOW ('I', 79, struct vtss_ipv6_mc_flood_members_get_ioc)
#define SWIOC_vtss_ipv6_mc_flood_members_set     _IOR ('I', 80, struct vtss_ipv6_mc_flood_members_set_ioc)
#define SWIOC_vtss_ipv6_mc_ctrl_flood_get        _IOW ('I', 81, struct vtss_ipv6_mc_ctrl_flood_get_ioc)
#define SWIOC_vtss_ipv6_mc_ctrl_flood_set        _IOR ('I', 82, struct vtss_ipv6_mc_ctrl_flood_set_ioc)
#define SWIOC_vtss_ipv6_mc_add                   _IOR ('I', 83, struct vtss_ipv6_mc_add_ioc)
#define SWIOC_vtss_ipv6_mc_del                   _IOR ('I', 84, struct vtss_ipv6_mc_del_ioc)
#define SWIOC_vtss_eps_port_conf_get             _IOWR('I', 85, struct vtss_eps_port_conf_get_ioc)
#define SWIOC_vtss_eps_port_conf_set             _IOR ('I', 86, struct vtss_eps_port_conf_set_ioc)
#define SWIOC_vtss_eps_port_selector_get         _IOWR('I', 87, struct vtss_eps_port_selector_get_ioc)
#define SWIOC_vtss_eps_port_selector_set         _IOR ('I', 88, struct vtss_eps_port_selector_set_ioc)
#define SWIOC_vtss_erps_vlan_member_get          _IOWR('I', 89, struct vtss_erps_vlan_member_get_ioc)
#define SWIOC_vtss_erps_vlan_member_set          _IOR ('I', 90, struct vtss_erps_vlan_member_set_ioc)
#define SWIOC_vtss_erps_port_state_get           _IOWR('I', 91, struct vtss_erps_port_state_get_ioc)
#define SWIOC_vtss_erps_port_state_set           _IOR ('I', 92, struct vtss_erps_port_state_set_ioc)
#define SWIOC_vtss_vid2port_set                  _IOR ('I', 93, struct vtss_vid2port_set_ioc)
#define SWIOC_vtss_vid2lport_get                 _IOWR('I', 94, struct vtss_vid2lport_get_ioc)
#define SWIOC_vtss_vid2lport_set                 _IOR ('I', 95, struct vtss_vid2lport_set_ioc)
#define SWIOC_vtss_vstax_conf_get                _IOW ('I', 96, struct vtss_vstax_conf_get_ioc)
#define SWIOC_vtss_vstax_conf_set                _IOR ('I', 97, struct vtss_vstax_conf_set_ioc)
#define SWIOC_vtss_vstax_port_conf_get           _IOWR('I', 98, struct vtss_vstax_port_conf_get_ioc)
#define SWIOC_vtss_vstax_port_conf_set           _IOR ('I', 99, struct vtss_vstax_port_conf_set_ioc)
#define SWIOC_vtss_vstax_topology_set            _IOR ('I', 100, struct vtss_vstax_topology_set_ioc)
#endif				/* VTSS_FEATURE_LAYER2 */

 /************************************
  * evc group 
  */

#ifdef VTSS_FEATURE_EVC
#define SWIOC_vtss_evc_port_conf_get             _IOWR('J',  1, struct vtss_evc_port_conf_get_ioc)
#define SWIOC_vtss_evc_port_conf_set             _IOR ('J',  2, struct vtss_evc_port_conf_set_ioc)
#define SWIOC_vtss_evc_add                       _IOR ('J',  3, struct vtss_evc_add_ioc)
#define SWIOC_vtss_evc_del                       _IOR ('J',  4, struct vtss_evc_del_ioc)
#define SWIOC_vtss_evc_get                       _IOWR('J',  5, struct vtss_evc_get_ioc)
#define SWIOC_vtss_ece_init                      _IOWR('J',  6, struct vtss_ece_init_ioc)
#define SWIOC_vtss_ece_add                       _IOR ('J',  7, struct vtss_ece_add_ioc)
#define SWIOC_vtss_ece_del                       _IOR ('J',  8, struct vtss_ece_del_ioc)
#define SWIOC_vtss_mce_init                      _IOW ('J',  9, struct vtss_mce_init_ioc)
#define SWIOC_vtss_mce_add                       _IOR ('J', 10, struct vtss_mce_add_ioc)
#define SWIOC_vtss_mce_del                       _IOR ('J', 11, struct vtss_mce_del_ioc)
#define SWIOC_vtss_evc_counters_get              _IOWR('J', 12, struct vtss_evc_counters_get_ioc)
#define SWIOC_vtss_evc_counters_clear            _IOR ('J', 13, struct vtss_evc_counters_clear_ioc)
#define SWIOC_vtss_ece_counters_get              _IOWR('J', 14, struct vtss_ece_counters_get_ioc)
#define SWIOC_vtss_ece_counters_clear            _IOR ('J', 15, struct vtss_ece_counters_clear_ioc)
#endif				/* VTSS_FEATURE_EVC */

 /************************************
  * qos_policer_dlb group 
  */

#ifdef VTSS_FEATURE_QOS_POLICER_DLB
#define SWIOC_vtss_evc_policer_conf_get          _IOWR('K',  1, struct vtss_evc_policer_conf_get_ioc)
#define SWIOC_vtss_evc_policer_conf_set          _IOR ('K',  2, struct vtss_evc_policer_conf_set_ioc)
#endif				/* VTSS_FEATURE_QOS_POLICER_DLB */

 /************************************
  * timestamp group 
  */

#ifdef VTSS_FEATURE_TIMESTAMP
#define SWIOC_vtss_ts_timeofday_set              _IOR ('L',  1, struct vtss_ts_timeofday_set_ioc)
#define SWIOC_vtss_ts_adjtimer_one_sec           _IOW ('L',  2, struct vtss_ts_adjtimer_one_sec_ioc)
#define SWIOC_vtss_ts_timeofday_get              _IOW ('L',  3, struct vtss_ts_timeofday_get_ioc)
#define SWIOC_vtss_ts_adjtimer_set               _IOR ('L',  4, struct vtss_ts_adjtimer_set_ioc)
#define SWIOC_vtss_ts_adjtimer_get               _IOW ('L',  5, struct vtss_ts_adjtimer_get_ioc)
#define SWIOC_vtss_ts_freq_offset_get            _IOW ('L',  6, struct vtss_ts_freq_offset_get_ioc)
#define SWIOC_vtss_ts_external_clock_mode_set    _IOR ('L',  7, struct vtss_ts_external_clock_mode_set_ioc)
#define SWIOC_vtss_ts_external_clock_mode_get    _IOW ('L',  8, struct vtss_ts_external_clock_mode_get_ioc)
#define SWIOC_vtss_ts_ingress_latency_set        _IOR ('L',  9, struct vtss_ts_ingress_latency_set_ioc)
#define SWIOC_vtss_ts_ingress_latency_get        _IOWR('L', 10, struct vtss_ts_ingress_latency_get_ioc)
#define SWIOC_vtss_ts_p2p_delay_set              _IOR ('L', 11, struct vtss_ts_p2p_delay_set_ioc)
#define SWIOC_vtss_ts_p2p_delay_get              _IOWR('L', 12, struct vtss_ts_p2p_delay_get_ioc)
#define SWIOC_vtss_ts_egress_latency_set         _IOR ('L', 13, struct vtss_ts_egress_latency_set_ioc)
#define SWIOC_vtss_ts_egress_latency_get         _IOWR('L', 14, struct vtss_ts_egress_latency_get_ioc)
#define SWIOC_vtss_ts_operation_mode_set         _IOR ('L', 15, struct vtss_ts_operation_mode_set_ioc)
#define SWIOC_vtss_ts_operation_mode_get         _IOWR('L', 16, struct vtss_ts_operation_mode_get_ioc)
#define SWIOC_vtss_tx_timestamp_update           _IO  ('L', 17)
#define SWIOC_vtss_rx_timestamp_get              _IOWR('L', 18, struct vtss_rx_timestamp_get_ioc)
#define SWIOC_vtss_rx_master_timestamp_get       _IOWR('L', 19, struct vtss_rx_master_timestamp_get_ioc)
#define SWIOC_vtss_tx_timestamp_idx_alloc        _IOWR('L', 20, struct vtss_tx_timestamp_idx_alloc_ioc)
#define SWIOC_vtss_timestamp_age                 _IO  ('L', 21)
#endif				/* VTSS_FEATURE_TIMESTAMP */

 /************************************
  * synce group 
  */

#ifdef VTSS_FEATURE_SYNCE
#define SWIOC_vtss_synce_clock_out_set           _IOR ('M',  1, struct vtss_synce_clock_out_set_ioc)
#define SWIOC_vtss_synce_clock_out_get           _IOWR('M',  2, struct vtss_synce_clock_out_get_ioc)
#define SWIOC_vtss_synce_clock_in_set            _IOR ('M',  3, struct vtss_synce_clock_in_set_ioc)
#define SWIOC_vtss_synce_clock_in_get            _IOWR('M',  4, struct vtss_synce_clock_in_get_ioc)
#endif				/* VTSS_FEATURE_SYNCE */

 /************************************
  * ccm group 
  */


#define SWIOC_vtssx_ccm_start                    _IOWR('N',  1, struct vtssx_ccm_start_ioc)
#define SWIOC_vtssx_ccm_status_get               _IOWR('N',  2, struct vtssx_ccm_status_get_ioc)
#define SWIOC_vtssx_ccm_cancel                   _IOR ('N',  3, struct vtssx_ccm_cancel_ioc)


 /************************************
  * l3 group 
  */

#ifdef VTSS_SW_OPTION_L3RT
#define SWIOC_vtss_l3_flush                      _IO  ('O',  1)
#define SWIOC_vtss_l3_rleg_common_get            _IOW ('O',  2, struct vtss_l3_rleg_common_get_ioc)
#define SWIOC_vtss_l3_rleg_common_set            _IOR ('O',  3, struct vtss_l3_rleg_common_set_ioc)
#define SWIOC_vtss_l3_rleg_get                   _IOW ('O',  4, struct vtss_l3_rleg_get_ioc)
#define SWIOC_vtss_l3_rleg_add                   _IOR ('O',  5, struct vtss_l3_rleg_add_ioc)
#define SWIOC_vtss_l3_rleg_del                   _IOR ('O',  6, struct vtss_l3_rleg_del_ioc)
#define SWIOC_vtss_l3_route_get                  _IOW ('O',  7, struct vtss_l3_route_get_ioc)
#define SWIOC_vtss_l3_route_add                  _IOR ('O',  8, struct vtss_l3_route_add_ioc)
#define SWIOC_vtss_l3_route_del                  _IOR ('O',  9, struct vtss_l3_route_del_ioc)
#define SWIOC_vtss_l3_neighbour_get              _IOW ('O', 10, struct vtss_l3_neighbour_get_ioc)
#define SWIOC_vtss_l3_neighbour_add              _IOR ('O', 11, struct vtss_l3_neighbour_add_ioc)
#define SWIOC_vtss_l3_neighbour_del              _IOR ('O', 12, struct vtss_l3_neighbour_del_ioc)
#define SWIOC_vtss_l3_counters_reset             _IO  ('O', 13)
#define SWIOC_vtss_l3_counters_system_get        _IOW ('O', 14, struct vtss_l3_counters_system_get_ioc)
#define SWIOC_vtss_l3_counters_rleg_get          _IOWR('O', 15, struct vtss_l3_counters_rleg_get_ioc)
#endif				/* VTSS_SW_OPTION_L3RT */

 /************************************
  * mpls group 
  */

#ifdef VTSS_FEATURE_MPLS
#define SWIOC_vtss_mpls_l2_alloc                 _IOW ('P',  1, struct vtss_mpls_l2_alloc_ioc)
#define SWIOC_vtss_mpls_l2_free                  _IOR ('P',  2, struct vtss_mpls_l2_free_ioc)
#define SWIOC_vtss_mpls_l2_get                   _IOWR('P',  3, struct vtss_mpls_l2_get_ioc)
#define SWIOC_vtss_mpls_l2_set                   _IOR ('P',  4, struct vtss_mpls_l2_set_ioc)
#define SWIOC_vtss_mpls_l2_segment_attach        _IOR ('P',  5, struct vtss_mpls_l2_segment_attach_ioc)
#define SWIOC_vtss_mpls_l2_segment_detach        _IOR ('P',  6, struct vtss_mpls_l2_segment_detach_ioc)
#define SWIOC_vtss_mpls_segment_alloc            _IOWR('P',  7, struct vtss_mpls_segment_alloc_ioc)
#define SWIOC_vtss_mpls_segment_free             _IOR ('P',  8, struct vtss_mpls_segment_free_ioc)
#define SWIOC_vtss_mpls_segment_get              _IOWR('P',  9, struct vtss_mpls_segment_get_ioc)
#define SWIOC_vtss_mpls_segment_set              _IOR ('P', 10, struct vtss_mpls_segment_set_ioc)
#define SWIOC_vtss_mpls_segment_state_get        _IOWR('P', 11, struct vtss_mpls_segment_state_get_ioc)
#define SWIOC_vtss_mpls_segment_server_attach    _IOR ('P', 12, struct vtss_mpls_segment_server_attach_ioc)
#define SWIOC_vtss_mpls_segment_server_detach    _IOR ('P', 13, struct vtss_mpls_segment_server_detach_ioc)
#define SWIOC_vtss_mpls_xc_alloc                 _IOWR('P', 14, struct vtss_mpls_xc_alloc_ioc)
#define SWIOC_vtss_mpls_xc_free                  _IOR ('P', 15, struct vtss_mpls_xc_free_ioc)
#define SWIOC_vtss_mpls_xc_get                   _IOWR('P', 16, struct vtss_mpls_xc_get_ioc)
#define SWIOC_vtss_mpls_xc_set                   _IOR ('P', 17, struct vtss_mpls_xc_set_ioc)
#define SWIOC_vtss_mpls_xc_segment_attach        _IOR ('P', 18, struct vtss_mpls_xc_segment_attach_ioc)
#define SWIOC_vtss_mpls_xc_segment_detach        _IOR ('P', 19, struct vtss_mpls_xc_segment_detach_ioc)
#define SWIOC_vtss_mpls_xc_mc_segment_attach     _IOR ('P', 20, struct vtss_mpls_xc_mc_segment_attach_ioc)
#define SWIOC_vtss_mpls_xc_mc_segment_detach     _IOR ('P', 21, struct vtss_mpls_xc_mc_segment_detach_ioc)
#define SWIOC_vtss_mpls_tc_conf_get              _IOW ('P', 22, struct vtss_mpls_tc_conf_get_ioc)
#define SWIOC_vtss_mpls_tc_conf_set              _IOR ('P', 23, struct vtss_mpls_tc_conf_set_ioc)
#endif				/* VTSS_FEATURE_MPLS */

 /************************************
  * macsec group 
  */

#ifdef VTSS_FEATURE_MACSEC
#define SWIOC_vtss_macsec_init_set               _IOR ('Q',  1, struct vtss_macsec_init_set_ioc)
#define SWIOC_vtss_macsec_init_get               _IOWR('Q',  2, struct vtss_macsec_init_get_ioc)
#define SWIOC_vtss_macsec_secy_conf_add          _IOR ('Q',  3, struct vtss_macsec_secy_conf_add_ioc)
#define SWIOC_vtss_macsec_secy_conf_get          _IOWR('Q',  4, struct vtss_macsec_secy_conf_get_ioc)
#define SWIOC_vtss_macsec_secy_conf_del          _IOR ('Q',  5, struct vtss_macsec_secy_conf_del_ioc)
#define SWIOC_vtss_macsec_secy_controlled_set    _IOR ('Q',  6, struct vtss_macsec_secy_controlled_set_ioc)
#define SWIOC_vtss_macsec_secy_controlled_get    _IOWR('Q',  7, struct vtss_macsec_secy_controlled_get_ioc)
#define SWIOC_vtss_macsec_secy_controlled_get    _IOWR('Q',  8, struct vtss_macsec_secy_controlled_get_ioc)
#define SWIOC_vtss_macsec_secy_port_status_get   _IOWR('Q',  9, struct vtss_macsec_secy_port_status_get_ioc)
#define SWIOC_vtss_macsec_rx_sc_add              _IOR ('Q', 10, struct vtss_macsec_rx_sc_add_ioc)
#define SWIOC_vtss_macsec_rx_sc_get_next         _IOWR('Q', 11, struct vtss_macsec_rx_sc_get_next_ioc)
#define SWIOC_vtss_macsec_rx_sc_del              _IOR ('Q', 12, struct vtss_macsec_rx_sc_del_ioc)
#define SWIOC_vtss_macsec_rx_sc_status_get       _IOWR('Q', 13, struct vtss_macsec_rx_sc_status_get_ioc)
#define SWIOC_vtss_macsec_tx_sc_set              _IOR ('Q', 14, struct vtss_macsec_tx_sc_set_ioc)
#define SWIOC_vtss_macsec_tx_sc_del              _IOR ('Q', 15, struct vtss_macsec_tx_sc_del_ioc)
#define SWIOC_vtss_macsec_tx_sc_status_get       _IOWR('Q', 16, struct vtss_macsec_tx_sc_status_get_ioc)
#define SWIOC_vtss_macsec_rx_sa_set              _IOR ('Q', 17, struct vtss_macsec_rx_sa_set_ioc)
#define SWIOC_vtss_macsec_rx_sa_get              _IOWR('Q', 18, struct vtss_macsec_rx_sa_get_ioc)
#define SWIOC_vtss_macsec_rx_sa_activate         _IOR ('Q', 19, struct vtss_macsec_rx_sa_activate_ioc)
#define SWIOC_vtss_macsec_rx_sa_disable          _IOR ('Q', 20, struct vtss_macsec_rx_sa_disable_ioc)
#define SWIOC_vtss_macsec_rx_sa_del              _IOR ('Q', 21, struct vtss_macsec_rx_sa_del_ioc)
#define SWIOC_vtss_macsec_rx_sa_lowest_pn_update _IOR ('Q', 22, struct vtss_macsec_rx_sa_lowest_pn_update_ioc)
#define SWIOC_vtss_macsec_rx_sa_status_get       _IOWR('Q', 23, struct vtss_macsec_rx_sa_status_get_ioc)
#define SWIOC_vtss_macsec_tx_sa_set              _IOR ('Q', 24, struct vtss_macsec_tx_sa_set_ioc)
#define SWIOC_vtss_macsec_tx_sa_get              _IOWR('Q', 25, struct vtss_macsec_tx_sa_get_ioc)
#define SWIOC_vtss_macsec_tx_sa_activate         _IOR ('Q', 26, struct vtss_macsec_tx_sa_activate_ioc)
#define SWIOC_vtss_macsec_tx_sa_disable          _IOR ('Q', 27, struct vtss_macsec_tx_sa_disable_ioc)
#define SWIOC_vtss_macsec_tx_sa_del              _IOR ('Q', 28, struct vtss_macsec_tx_sa_del_ioc)
#define SWIOC_vtss_macsec_tx_sa_status_get       _IOWR('Q', 29, struct vtss_macsec_tx_sa_status_get_ioc)
#define SWIOC_vtss_macsec_secy_port_counters_get _IOWR('Q', 30, struct vtss_macsec_secy_port_counters_get_ioc)
#define SWIOC_vtss_macsec_secy_cap_get           _IOWR('Q', 31, struct vtss_macsec_secy_cap_get_ioc)
#define SWIOC_vtss_macsec_secy_counters_get      _IOWR('Q', 32, struct vtss_macsec_secy_counters_get_ioc)
#define SWIOC_vtss_macsec_counters_update        _IOR ('Q', 33, struct vtss_macsec_counters_update_ioc)
#define SWIOC_vtss_macsec_counters_clear         _IOR ('Q', 34, struct vtss_macsec_counters_clear_ioc)
#define SWIOC_vtss_macsec_rx_sc_counters_get     _IOWR('Q', 35, struct vtss_macsec_rx_sc_counters_get_ioc)
#define SWIOC_vtss_macsec_tx_sc_counters_get     _IOWR('Q', 36, struct vtss_macsec_tx_sc_counters_get_ioc)
#define SWIOC_vtss_macsec_tx_sa_counters_get     _IOWR('Q', 37, struct vtss_macsec_tx_sa_counters_get_ioc)
#define SWIOC_vtss_macsec_rx_sa_counters_get     _IOWR('Q', 38, struct vtss_macsec_rx_sa_counters_get_ioc)
#define SWIOC_vtss_macsec_control_frame_match_conf_set _IOR ('Q', 39, struct vtss_macsec_control_frame_match_conf_set_ioc)
#define SWIOC_vtss_macsec_control_frame_match_conf_get _IOWR('Q', 40, struct vtss_macsec_control_frame_match_conf_get_ioc)
#define SWIOC_vtss_macsec_pattern_set            _IOR ('Q', 41, struct vtss_macsec_pattern_set_ioc)
#define SWIOC_vtss_macsec_pattern_del            _IOR ('Q', 42, struct vtss_macsec_pattern_del_ioc)
#define SWIOC_vtss_macsec_pattern_get            _IOWR('Q', 43, struct vtss_macsec_pattern_get_ioc)
#define SWIOC_vtss_macsec_default_action_set     _IOR ('Q', 44, struct vtss_macsec_default_action_set_ioc)
#define SWIOC_vtss_macsec_default_action_get     _IOWR('Q', 45, struct vtss_macsec_default_action_get_ioc)
#define SWIOC_vtss_macsec_bypass_mode_set        _IOR ('Q', 46, struct vtss_macsec_bypass_mode_set_ioc)
#define SWIOC_vtss_macsec_bypass_mode_get        _IOWR('Q', 47, struct vtss_macsec_bypass_mode_get_ioc)
#define SWIOC_vtss_macsec_bypass_tag_set         _IOR ('Q', 48, struct vtss_macsec_bypass_tag_set_ioc)
#define SWIOC_vtss_macsec_bypass_tag_get         _IOWR('Q', 49, struct vtss_macsec_bypass_tag_get_ioc)
#define SWIOC_vtss_macsec_frame_capture_set      _IOR ('Q', 50, struct vtss_macsec_frame_capture_set_ioc)
#define SWIOC_vtss_macsec_port_loopback_set      _IOR ('Q', 51, struct vtss_macsec_port_loopback_set_ioc)
#define SWIOC_vtss_macsec_frame_get              _IOWR('Q', 52, struct vtss_macsec_frame_get_ioc)
#endif				/* VTSS_FEATURE_MACSEC */



#endif				/* ifndef _VTSS_SWITCH_IOCTL_H */
