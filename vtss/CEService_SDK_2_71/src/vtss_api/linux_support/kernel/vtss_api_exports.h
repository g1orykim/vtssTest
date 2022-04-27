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


 /************************************
  * port group 
  */


EXPORT_SYMBOL( vtss_port_map_set );
EXPORT_SYMBOL( vtss_port_map_get );
#ifdef VTSS_FEATURE_10G
EXPORT_SYMBOL( vtss_port_mmd_read );
#endif /* VTSS_FEATURE_10G */
#ifdef VTSS_FEATURE_10G
EXPORT_SYMBOL( vtss_port_mmd_write );
#endif /* VTSS_FEATURE_10G */
#ifdef VTSS_FEATURE_10G
EXPORT_SYMBOL( vtss_port_mmd_masked_write );
#endif /* VTSS_FEATURE_10G */
#ifdef VTSS_FEATURE_10G
EXPORT_SYMBOL( vtss_mmd_read );
#endif /* VTSS_FEATURE_10G */
#ifdef VTSS_FEATURE_10G
EXPORT_SYMBOL( vtss_mmd_write );
#endif /* VTSS_FEATURE_10G */
#ifdef VTSS_FEATURE_CLAUSE_37
EXPORT_SYMBOL( vtss_port_clause_37_control_get );
#endif /* VTSS_FEATURE_CLAUSE_37 */
#ifdef VTSS_FEATURE_CLAUSE_37
EXPORT_SYMBOL( vtss_port_clause_37_control_set );
#endif /* VTSS_FEATURE_CLAUSE_37 */
EXPORT_SYMBOL( vtss_port_conf_set );
EXPORT_SYMBOL( vtss_port_conf_get );
EXPORT_SYMBOL( vtss_port_status_get );
EXPORT_SYMBOL( vtss_port_counters_update );
EXPORT_SYMBOL( vtss_port_counters_clear );
EXPORT_SYMBOL( vtss_port_counters_get );
EXPORT_SYMBOL( vtss_port_basic_counters_get );
EXPORT_SYMBOL( vtss_port_forward_state_get );
EXPORT_SYMBOL( vtss_port_forward_state_set );
EXPORT_SYMBOL( vtss_miim_read );
EXPORT_SYMBOL( vtss_miim_write );


 /************************************
  * misc group 
  */


EXPORT_SYMBOL( vtss_debug_info_print );
EXPORT_SYMBOL( vtss_reg_read );
EXPORT_SYMBOL( vtss_reg_write );
EXPORT_SYMBOL( vtss_reg_write_masked );
EXPORT_SYMBOL( vtss_chip_id_get );
EXPORT_SYMBOL( vtss_poll_1sec );
EXPORT_SYMBOL( vtss_ptp_event_poll );
EXPORT_SYMBOL( vtss_ptp_event_enable );
EXPORT_SYMBOL( vtss_dev_all_event_poll );
EXPORT_SYMBOL( vtss_dev_all_event_enable );
EXPORT_SYMBOL( vtss_gpio_mode_set );
EXPORT_SYMBOL( vtss_gpio_direction_set );
EXPORT_SYMBOL( vtss_gpio_read );
EXPORT_SYMBOL( vtss_gpio_write );
EXPORT_SYMBOL( vtss_gpio_event_poll );
EXPORT_SYMBOL( vtss_gpio_event_enable );
#ifdef VTSS_ARCH_B2
EXPORT_SYMBOL( vtss_gpio_clk_set );
#endif /* VTSS_ARCH_B2 */
#ifdef VTSS_FEATURE_SERIAL_LED
EXPORT_SYMBOL( vtss_serial_led_set );
#endif /* VTSS_FEATURE_SERIAL_LED */
#ifdef VTSS_FEATURE_SERIAL_LED
EXPORT_SYMBOL( vtss_serial_led_intensity_set );
#endif /* VTSS_FEATURE_SERIAL_LED */
#ifdef VTSS_FEATURE_SERIAL_LED
EXPORT_SYMBOL( vtss_serial_led_intensity_get );
#endif /* VTSS_FEATURE_SERIAL_LED */
#ifdef VTSS_FEATURE_SERIAL_GPIO
EXPORT_SYMBOL( vtss_sgpio_conf_get );
#endif /* VTSS_FEATURE_SERIAL_GPIO */
#ifdef VTSS_FEATURE_SERIAL_GPIO
EXPORT_SYMBOL( vtss_sgpio_conf_set );
#endif /* VTSS_FEATURE_SERIAL_GPIO */
#ifdef VTSS_FEATURE_SERIAL_GPIO
EXPORT_SYMBOL( vtss_sgpio_read );
#endif /* VTSS_FEATURE_SERIAL_GPIO */
#ifdef VTSS_FEATURE_SERIAL_GPIO
EXPORT_SYMBOL( vtss_sgpio_event_poll );
#endif /* VTSS_FEATURE_SERIAL_GPIO */
#ifdef VTSS_FEATURE_SERIAL_GPIO
EXPORT_SYMBOL( vtss_sgpio_event_enable );
#endif /* VTSS_FEATURE_SERIAL_GPIO */
#ifdef VTSS_FEATURE_FAN
EXPORT_SYMBOL( vtss_temp_sensor_init );
#endif /* VTSS_FEATURE_FAN */
#ifdef VTSS_FEATURE_FAN
EXPORT_SYMBOL( vtss_temp_sensor_get );
#endif /* VTSS_FEATURE_FAN */
#ifdef VTSS_FEATURE_FAN
EXPORT_SYMBOL( vtss_fan_rotation_get );
#endif /* VTSS_FEATURE_FAN */
#ifdef VTSS_FEATURE_FAN
EXPORT_SYMBOL( vtss_fan_cool_lvl_set );
#endif /* VTSS_FEATURE_FAN */
#ifdef VTSS_FEATURE_FAN
EXPORT_SYMBOL( vtss_fan_controller_init );
#endif /* VTSS_FEATURE_FAN */
#ifdef VTSS_FEATURE_FAN
EXPORT_SYMBOL( vtss_fan_cool_lvl_get );
#endif /* VTSS_FEATURE_FAN */
#ifdef VTSS_FEATURE_EEE
EXPORT_SYMBOL( vtss_eee_port_conf_set );
#endif /* VTSS_FEATURE_EEE */
#ifdef VTSS_FEATURE_EEE
EXPORT_SYMBOL( vtss_eee_port_state_set );
#endif /* VTSS_FEATURE_EEE */
#ifdef VTSS_FEATURE_EEE
EXPORT_SYMBOL( vtss_eee_port_counter_get );
#endif /* VTSS_FEATURE_EEE */


 /************************************
  * init group 
  */


#ifdef VTSS_FEATURE_WARM_START
EXPORT_SYMBOL( vtss_restart_status_get );
#endif /* VTSS_FEATURE_WARM_START */
#ifdef VTSS_FEATURE_WARM_START
EXPORT_SYMBOL( vtss_restart_conf_get );
#endif /* VTSS_FEATURE_WARM_START */
#ifdef VTSS_FEATURE_WARM_START
EXPORT_SYMBOL( vtss_restart_conf_set );
#endif /* VTSS_FEATURE_WARM_START */


 /************************************
  * phy group 
  */


EXPORT_SYMBOL( vtss_phy_pre_reset );
EXPORT_SYMBOL( vtss_phy_post_reset );
EXPORT_SYMBOL( vtss_phy_reset );
EXPORT_SYMBOL( vtss_phy_chip_temp_get );
EXPORT_SYMBOL( vtss_phy_chip_temp_init );
EXPORT_SYMBOL( vtss_phy_conf_get );
EXPORT_SYMBOL( vtss_phy_conf_set );
EXPORT_SYMBOL( vtss_phy_status_get );
EXPORT_SYMBOL( vtss_phy_conf_1g_get );
EXPORT_SYMBOL( vtss_phy_conf_1g_set );
EXPORT_SYMBOL( vtss_phy_status_1g_get );
EXPORT_SYMBOL( vtss_phy_power_conf_get );
EXPORT_SYMBOL( vtss_phy_power_conf_set );
EXPORT_SYMBOL( vtss_phy_power_status_get );
EXPORT_SYMBOL( vtss_phy_clock_conf_set );
EXPORT_SYMBOL( vtss_phy_read );
EXPORT_SYMBOL( vtss_phy_write );
EXPORT_SYMBOL( vtss_phy_mmd_read );
EXPORT_SYMBOL( vtss_phy_mmd_write );
EXPORT_SYMBOL( vtss_phy_write_masked );
EXPORT_SYMBOL( vtss_phy_write_masked_page );
EXPORT_SYMBOL( vtss_phy_veriphy_start );
EXPORT_SYMBOL( vtss_phy_veriphy_get );
#ifdef VTSS_FEATURE_LED_POW_REDUC
EXPORT_SYMBOL( vtss_phy_led_intensity_set );
#endif /* VTSS_FEATURE_LED_POW_REDUC */
#ifdef VTSS_FEATURE_LED_POW_REDUC
EXPORT_SYMBOL( vtss_phy_enhanced_led_control_init );
#endif /* VTSS_FEATURE_LED_POW_REDUC */
EXPORT_SYMBOL( vtss_phy_coma_mode_disable );
#ifdef VTSS_FEATURE_EEE
EXPORT_SYMBOL( vtss_phy_eee_ena );
#endif /* VTSS_FEATURE_EEE */
#ifdef VTSS_CHIP_CU_PHY
EXPORT_SYMBOL( vtss_phy_event_enable_set );
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_CHIP_CU_PHY
EXPORT_SYMBOL( vtss_phy_event_enable_get );
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_CHIP_CU_PHY
EXPORT_SYMBOL( vtss_phy_event_poll );
#endif /* VTSS_CHIP_CU_PHY */


 /************************************
  * 10gphy group 
  */

#ifdef VTSS_FEATURE_10G
EXPORT_SYMBOL( vtss_phy_10g_mode_get );
EXPORT_SYMBOL( vtss_phy_10g_mode_set );
EXPORT_SYMBOL( vtss_phy_10g_synce_clkout_get );
EXPORT_SYMBOL( vtss_phy_10g_synce_clkout_set );
EXPORT_SYMBOL( vtss_phy_10g_xfp_clkout_get );
EXPORT_SYMBOL( vtss_phy_10g_xfp_clkout_set );
EXPORT_SYMBOL( vtss_phy_10g_status_get );
EXPORT_SYMBOL( vtss_phy_10g_reset );
EXPORT_SYMBOL( vtss_phy_10g_loopback_set );
EXPORT_SYMBOL( vtss_phy_10g_loopback_get );
EXPORT_SYMBOL( vtss_phy_10g_cnt_get );
EXPORT_SYMBOL( vtss_phy_10g_power_get );
EXPORT_SYMBOL( vtss_phy_10g_power_set );
EXPORT_SYMBOL( vtss_phy_10g_failover_set );
EXPORT_SYMBOL( vtss_phy_10g_failover_get );
EXPORT_SYMBOL( vtss_phy_10g_id_get );
EXPORT_SYMBOL( vtss_phy_10g_gpio_mode_set );
EXPORT_SYMBOL( vtss_phy_10g_gpio_mode_get );
EXPORT_SYMBOL( vtss_phy_10g_gpio_read );
EXPORT_SYMBOL( vtss_phy_10g_gpio_write );
EXPORT_SYMBOL( vtss_phy_10g_event_enable_set );
EXPORT_SYMBOL( vtss_phy_10g_event_enable_get );
EXPORT_SYMBOL( vtss_phy_10g_event_poll );
EXPORT_SYMBOL( vtss_phy_10g_poll_1sec );
EXPORT_SYMBOL( vtss_phy_10g_edc_fw_status_get );
#endif /* VTSS_FEATURE_10G */

 /************************************
  * qos group 
  */

#ifdef VTSS_FEATURE_QOS
#ifdef VTSS_ARCH_CARACAL
EXPORT_SYMBOL( vtss_mep_policer_conf_get );
#endif /* VTSS_ARCH_CARACAL */
#ifdef VTSS_ARCH_CARACAL
EXPORT_SYMBOL( vtss_mep_policer_conf_set );
#endif /* VTSS_ARCH_CARACAL */
EXPORT_SYMBOL( vtss_qos_conf_get );
EXPORT_SYMBOL( vtss_qos_conf_set );
EXPORT_SYMBOL( vtss_qos_port_conf_get );
EXPORT_SYMBOL( vtss_qos_port_conf_set );
#ifdef VTSS_FEATURE_QCL
EXPORT_SYMBOL( vtss_qce_init );
#endif /* VTSS_FEATURE_QCL */
#ifdef VTSS_FEATURE_QCL
EXPORT_SYMBOL( vtss_qce_add );
#endif /* VTSS_FEATURE_QCL */
#ifdef VTSS_FEATURE_QCL
EXPORT_SYMBOL( vtss_qce_del );
#endif /* VTSS_FEATURE_QCL */
#endif /* VTSS_FEATURE_QOS */

 /************************************
  * packet group 
  */

#ifdef VTSS_FEATURE_PACKET
#ifdef VTSS_FEATURE_NPI
EXPORT_SYMBOL( vtss_npi_conf_get );
#endif /* VTSS_FEATURE_NPI */
#ifdef VTSS_FEATURE_NPI
EXPORT_SYMBOL( vtss_npi_conf_set );
#endif /* VTSS_FEATURE_NPI */
EXPORT_SYMBOL( vtss_packet_rx_conf_get );
EXPORT_SYMBOL( vtss_packet_rx_conf_set );
#ifdef VTSS_FEATURE_PACKET_PORT_REG
EXPORT_SYMBOL( vtss_packet_rx_port_conf_get );
#endif /* VTSS_FEATURE_PACKET_PORT_REG */
#ifdef VTSS_FEATURE_PACKET_PORT_REG
EXPORT_SYMBOL( vtss_packet_rx_port_conf_set );
#endif /* VTSS_FEATURE_PACKET_PORT_REG */
EXPORT_SYMBOL( vtss_packet_rx_frame_get );
EXPORT_SYMBOL( vtss_packet_rx_frame_discard );
EXPORT_SYMBOL( vtss_packet_tx_frame_port );
EXPORT_SYMBOL( vtss_packet_tx_frame_port_vlan );
EXPORT_SYMBOL( vtss_packet_tx_frame_vlan );
EXPORT_SYMBOL( vtss_packet_frame_filter );
EXPORT_SYMBOL( vtss_packet_port_info_init );
EXPORT_SYMBOL( vtss_packet_port_filter_get );
#ifdef VTSS_FEATURE_VSTAX
EXPORT_SYMBOL( vtss_packet_tx_frame_vstax );
#endif /* VTSS_FEATURE_VSTAX */
#ifdef VTSS_FEATURE_VSTAX
EXPORT_SYMBOL( vtss_packet_vstax_header2frame );
#endif /* VTSS_FEATURE_VSTAX */
#ifdef VTSS_FEATURE_VSTAX
EXPORT_SYMBOL( vtss_packet_vstax_frame2header );
#endif /* VTSS_FEATURE_VSTAX */
#ifdef VTSS_FEATURE_AFI_SWC
EXPORT_SYMBOL( vtss_afi_alloc );
#endif /* VTSS_FEATURE_AFI_SWC */
#ifdef VTSS_FEATURE_AFI_SWC
EXPORT_SYMBOL( vtss_afi_free );
#endif /* VTSS_FEATURE_AFI_SWC */
EXPORT_SYMBOL( vtss_packet_rx_hdr_decode );
EXPORT_SYMBOL( vtss_packet_tx_hdr_encode );
EXPORT_SYMBOL( vtss_packet_tx_info_init );
#endif /* VTSS_FEATURE_PACKET */

 /************************************
  * security group 
  */


#ifdef VTSS_FEATURE_LAYER2
EXPORT_SYMBOL( vtss_auth_port_state_get );
#endif /* VTSS_FEATURE_LAYER2 */
#ifdef VTSS_FEATURE_LAYER2
EXPORT_SYMBOL( vtss_auth_port_state_set );
#endif /* VTSS_FEATURE_LAYER2 */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_acl_policer_conf_get );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_acl_policer_conf_set );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_acl_port_conf_get );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_acl_port_conf_set );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_acl_port_counter_get );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_acl_port_counter_clear );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_ace_init );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_ace_add );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_ace_del );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_ace_counter_get );
#endif /* VTSS_FEATURE_ACL */
#ifdef VTSS_FEATURE_ACL
EXPORT_SYMBOL( vtss_ace_counter_clear );
#endif /* VTSS_FEATURE_ACL */


 /************************************
  * layer2 group 
  */

#ifdef VTSS_FEATURE_LAYER2
EXPORT_SYMBOL( vtss_mac_table_add );
EXPORT_SYMBOL( vtss_mac_table_del );
EXPORT_SYMBOL( vtss_mac_table_get );
EXPORT_SYMBOL( vtss_mac_table_get_next );
#ifdef VTSS_FEATURE_MAC_AGE_AUTO
EXPORT_SYMBOL( vtss_mac_table_age_time_get );
#endif /* VTSS_FEATURE_MAC_AGE_AUTO */
#ifdef VTSS_FEATURE_MAC_AGE_AUTO
EXPORT_SYMBOL( vtss_mac_table_age_time_set );
#endif /* VTSS_FEATURE_MAC_AGE_AUTO */
EXPORT_SYMBOL( vtss_mac_table_age );
EXPORT_SYMBOL( vtss_mac_table_vlan_age );
EXPORT_SYMBOL( vtss_mac_table_flush );
EXPORT_SYMBOL( vtss_mac_table_port_flush );
EXPORT_SYMBOL( vtss_mac_table_vlan_flush );
EXPORT_SYMBOL( vtss_mac_table_vlan_port_flush );
#ifdef VTSS_FEATURE_VSTAX_V2
EXPORT_SYMBOL( vtss_mac_table_upsid_flush );
#endif /* VTSS_FEATURE_VSTAX_V2 */
#ifdef VTSS_FEATURE_VSTAX_V2
EXPORT_SYMBOL( vtss_mac_table_upsid_upspn_flush );
#endif /* VTSS_FEATURE_VSTAX_V2 */
#ifdef VTSS_FEATURE_AGGR_GLAG
EXPORT_SYMBOL( vtss_mac_table_glag_add );
#endif /* VTSS_FEATURE_AGGR_GLAG */
#ifdef VTSS_FEATURE_AGGR_GLAG
EXPORT_SYMBOL( vtss_mac_table_glag_flush );
#endif /* VTSS_FEATURE_AGGR_GLAG */
#ifdef VTSS_FEATURE_AGGR_GLAG
EXPORT_SYMBOL( vtss_mac_table_vlan_glag_flush );
#endif /* VTSS_FEATURE_AGGR_GLAG */
EXPORT_SYMBOL( vtss_mac_table_status_get );
EXPORT_SYMBOL( vtss_learn_port_mode_get );
EXPORT_SYMBOL( vtss_learn_port_mode_set );
EXPORT_SYMBOL( vtss_port_state_get );
EXPORT_SYMBOL( vtss_port_state_set );
EXPORT_SYMBOL( vtss_stp_port_state_get );
EXPORT_SYMBOL( vtss_stp_port_state_set );
EXPORT_SYMBOL( vtss_mstp_vlan_msti_get );
EXPORT_SYMBOL( vtss_mstp_vlan_msti_set );
EXPORT_SYMBOL( vtss_mstp_port_msti_state_get );
EXPORT_SYMBOL( vtss_mstp_port_msti_state_set );
EXPORT_SYMBOL( vtss_vlan_conf_get );
EXPORT_SYMBOL( vtss_vlan_conf_set );
EXPORT_SYMBOL( vtss_vlan_port_conf_get );
EXPORT_SYMBOL( vtss_vlan_port_conf_set );
EXPORT_SYMBOL( vtss_vlan_port_members_get );
EXPORT_SYMBOL( vtss_vlan_port_members_set );
#ifdef VTSS_FEATURE_VLAN_COUNTERS
EXPORT_SYMBOL( vtss_vlan_counters_get );
#endif /* VTSS_FEATURE_VLAN_COUNTERS */
#ifdef VTSS_FEATURE_VLAN_COUNTERS
EXPORT_SYMBOL( vtss_vlan_counters_clear );
#endif /* VTSS_FEATURE_VLAN_COUNTERS */
EXPORT_SYMBOL( vtss_vce_init );
EXPORT_SYMBOL( vtss_vce_add );
EXPORT_SYMBOL( vtss_vce_del );
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
EXPORT_SYMBOL( vtss_vlan_trans_group_add );
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
EXPORT_SYMBOL( vtss_vlan_trans_group_del );
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
EXPORT_SYMBOL( vtss_vlan_trans_group_get );
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
EXPORT_SYMBOL( vtss_vlan_trans_group_to_port_set );
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */
#ifdef VTSS_FEATURE_VLAN_TRANSLATION
EXPORT_SYMBOL( vtss_vlan_trans_group_to_port_get );
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */
EXPORT_SYMBOL( vtss_isolated_vlan_get );
EXPORT_SYMBOL( vtss_isolated_vlan_set );
EXPORT_SYMBOL( vtss_isolated_port_members_get );
EXPORT_SYMBOL( vtss_isolated_port_members_set );
#ifdef VTSS_FEATURE_PVLAN
EXPORT_SYMBOL( vtss_pvlan_port_members_get );
#endif /* VTSS_FEATURE_PVLAN */
#ifdef VTSS_FEATURE_PVLAN
EXPORT_SYMBOL( vtss_pvlan_port_members_set );
#endif /* VTSS_FEATURE_PVLAN */
#ifdef VTSS_FEATURE_SFLOW
EXPORT_SYMBOL( vtss_sflow_port_conf_get );
#endif /* VTSS_FEATURE_SFLOW */
#ifdef VTSS_FEATURE_SFLOW
EXPORT_SYMBOL( vtss_sflow_port_conf_set );
#endif /* VTSS_FEATURE_SFLOW */
EXPORT_SYMBOL( vtss_aggr_port_members_get );
EXPORT_SYMBOL( vtss_aggr_port_members_set );
EXPORT_SYMBOL( vtss_aggr_mode_get );
EXPORT_SYMBOL( vtss_aggr_mode_set );
#ifdef VTSS_FEATURE_AGGR_GLAG
EXPORT_SYMBOL( vtss_aggr_glag_members_get );
#endif /* VTSS_FEATURE_AGGR_GLAG */
#ifdef VTSS_FEATURE_VSTAX_V1
EXPORT_SYMBOL( vtss_aggr_glag_set );
#endif /* VTSS_FEATURE_VSTAX_V1 */
#ifdef VTSS_FEATURE_VSTAX_V2
EXPORT_SYMBOL( vtss_vstax_glag_get );
#endif /* VTSS_FEATURE_VSTAX_V2 */
#ifdef VTSS_FEATURE_VSTAX_V2
EXPORT_SYMBOL( vtss_vstax_glag_set );
#endif /* VTSS_FEATURE_VSTAX_V2 */
EXPORT_SYMBOL( vtss_mirror_monitor_port_get );
EXPORT_SYMBOL( vtss_mirror_monitor_port_set );
EXPORT_SYMBOL( vtss_mirror_ingress_ports_get );
EXPORT_SYMBOL( vtss_mirror_ingress_ports_set );
EXPORT_SYMBOL( vtss_mirror_egress_ports_get );
EXPORT_SYMBOL( vtss_mirror_egress_ports_set );
EXPORT_SYMBOL( vtss_mirror_cpu_ingress_get );
EXPORT_SYMBOL( vtss_mirror_cpu_ingress_set );
EXPORT_SYMBOL( vtss_mirror_cpu_egress_get );
EXPORT_SYMBOL( vtss_mirror_cpu_egress_set );
EXPORT_SYMBOL( vtss_uc_flood_members_get );
EXPORT_SYMBOL( vtss_uc_flood_members_set );
EXPORT_SYMBOL( vtss_mc_flood_members_get );
EXPORT_SYMBOL( vtss_mc_flood_members_set );
EXPORT_SYMBOL( vtss_ipv4_mc_flood_members_get );
EXPORT_SYMBOL( vtss_ipv4_mc_flood_members_set );
#ifdef VTSS_FEATURE_IPV4_MC_SIP
EXPORT_SYMBOL( vtss_ipv4_mc_add );
#endif /* VTSS_FEATURE_IPV4_MC_SIP */
#ifdef VTSS_FEATURE_IPV4_MC_SIP
EXPORT_SYMBOL( vtss_ipv4_mc_del );
#endif /* VTSS_FEATURE_IPV4_MC_SIP */
EXPORT_SYMBOL( vtss_ipv6_mc_flood_members_get );
EXPORT_SYMBOL( vtss_ipv6_mc_flood_members_set );
EXPORT_SYMBOL( vtss_ipv6_mc_ctrl_flood_get );
EXPORT_SYMBOL( vtss_ipv6_mc_ctrl_flood_set );
#ifdef VTSS_FEATURE_IPV6_MC_SIP
EXPORT_SYMBOL( vtss_ipv6_mc_add );
#endif /* VTSS_FEATURE_IPV6_MC_SIP */
#ifdef VTSS_FEATURE_IPV6_MC_SIP
EXPORT_SYMBOL( vtss_ipv6_mc_del );
#endif /* VTSS_FEATURE_IPV6_MC_SIP */
EXPORT_SYMBOL( vtss_eps_port_conf_get );
EXPORT_SYMBOL( vtss_eps_port_conf_set );
EXPORT_SYMBOL( vtss_eps_port_selector_get );
EXPORT_SYMBOL( vtss_eps_port_selector_set );
EXPORT_SYMBOL( vtss_erps_vlan_member_get );
EXPORT_SYMBOL( vtss_erps_vlan_member_set );
EXPORT_SYMBOL( vtss_erps_port_state_get );
EXPORT_SYMBOL( vtss_erps_port_state_set );
#ifdef VTSS_ARCH_B2
EXPORT_SYMBOL( vtss_vid2port_set );
#endif /* VTSS_ARCH_B2 */
#ifdef VTSS_ARCH_B2
EXPORT_SYMBOL( vtss_vid2lport_get );
#endif /* VTSS_ARCH_B2 */
#ifdef VTSS_ARCH_B2
EXPORT_SYMBOL( vtss_vid2lport_set );
#endif /* VTSS_ARCH_B2 */
#ifdef VTSS_FEATURE_VSTAX
EXPORT_SYMBOL( vtss_vstax_conf_get );
#endif /* VTSS_FEATURE_VSTAX */
#ifdef VTSS_FEATURE_VSTAX
EXPORT_SYMBOL( vtss_vstax_conf_set );
#endif /* VTSS_FEATURE_VSTAX */
#ifdef VTSS_FEATURE_VSTAX
EXPORT_SYMBOL( vtss_vstax_port_conf_get );
#endif /* VTSS_FEATURE_VSTAX */
#ifdef VTSS_FEATURE_VSTAX
EXPORT_SYMBOL( vtss_vstax_port_conf_set );
#endif /* VTSS_FEATURE_VSTAX */
#ifdef VTSS_FEATURE_VSTAX
EXPORT_SYMBOL( vtss_vstax_topology_set );
#endif /* VTSS_FEATURE_VSTAX */
#endif /* VTSS_FEATURE_LAYER2 */

 /************************************
  * evc group 
  */

#ifdef VTSS_FEATURE_EVC
EXPORT_SYMBOL( vtss_evc_port_conf_get );
EXPORT_SYMBOL( vtss_evc_port_conf_set );
EXPORT_SYMBOL( vtss_evc_add );
EXPORT_SYMBOL( vtss_evc_del );
EXPORT_SYMBOL( vtss_evc_get );
EXPORT_SYMBOL( vtss_ece_init );
EXPORT_SYMBOL( vtss_ece_add );
EXPORT_SYMBOL( vtss_ece_del );
#ifdef VTSS_ARCH_CARACAL
EXPORT_SYMBOL( vtss_mce_init );
#endif /* VTSS_ARCH_CARACAL */
#ifdef VTSS_ARCH_CARACAL
EXPORT_SYMBOL( vtss_mce_add );
#endif /* VTSS_ARCH_CARACAL */
#ifdef VTSS_ARCH_CARACAL
EXPORT_SYMBOL( vtss_mce_del );
#endif /* VTSS_ARCH_CARACAL */
#ifdef VTSS_ARCH_JAGUAR_1
EXPORT_SYMBOL( vtss_evc_counters_get );
#endif /* VTSS_ARCH_JAGUAR_1 */
#ifdef VTSS_ARCH_JAGUAR_1
EXPORT_SYMBOL( vtss_evc_counters_clear );
#endif /* VTSS_ARCH_JAGUAR_1 */
#ifdef VTSS_ARCH_JAGUAR_1
EXPORT_SYMBOL( vtss_ece_counters_get );
#endif /* VTSS_ARCH_JAGUAR_1 */
#ifdef VTSS_ARCH_JAGUAR_1
EXPORT_SYMBOL( vtss_ece_counters_clear );
#endif /* VTSS_ARCH_JAGUAR_1 */
#endif /* VTSS_FEATURE_EVC */

 /************************************
  * qos_policer_dlb group 
  */

#ifdef VTSS_FEATURE_QOS_POLICER_DLB
EXPORT_SYMBOL( vtss_evc_policer_conf_get );
EXPORT_SYMBOL( vtss_evc_policer_conf_set );
#endif /* VTSS_FEATURE_QOS_POLICER_DLB */

 /************************************
  * timestamp group 
  */

#ifdef VTSS_FEATURE_TIMESTAMP
EXPORT_SYMBOL( vtss_ts_timeofday_set );
EXPORT_SYMBOL( vtss_ts_adjtimer_one_sec );
EXPORT_SYMBOL( vtss_ts_timeofday_get );
EXPORT_SYMBOL( vtss_ts_adjtimer_set );
EXPORT_SYMBOL( vtss_ts_adjtimer_get );
EXPORT_SYMBOL( vtss_ts_freq_offset_get );
EXPORT_SYMBOL( vtss_ts_external_clock_mode_set );
EXPORT_SYMBOL( vtss_ts_external_clock_mode_get );
EXPORT_SYMBOL( vtss_ts_ingress_latency_set );
EXPORT_SYMBOL( vtss_ts_ingress_latency_get );
EXPORT_SYMBOL( vtss_ts_p2p_delay_set );
EXPORT_SYMBOL( vtss_ts_p2p_delay_get );
EXPORT_SYMBOL( vtss_ts_egress_latency_set );
EXPORT_SYMBOL( vtss_ts_egress_latency_get );
EXPORT_SYMBOL( vtss_ts_operation_mode_set );
EXPORT_SYMBOL( vtss_ts_operation_mode_get );
EXPORT_SYMBOL( vtss_tx_timestamp_update );
EXPORT_SYMBOL( vtss_rx_timestamp_get );
EXPORT_SYMBOL( vtss_rx_master_timestamp_get );
#ifdef NOTDEF
EXPORT_SYMBOL( vtss_tx_timestamp_idx_alloc );
#endif /* NOTDEF */
EXPORT_SYMBOL( vtss_timestamp_age );
#endif /* VTSS_FEATURE_TIMESTAMP */

 /************************************
  * synce group 
  */

#ifdef VTSS_FEATURE_SYNCE
EXPORT_SYMBOL( vtss_synce_clock_out_set );
EXPORT_SYMBOL( vtss_synce_clock_out_get );
EXPORT_SYMBOL( vtss_synce_clock_in_set );
EXPORT_SYMBOL( vtss_synce_clock_in_get );
#endif /* VTSS_FEATURE_SYNCE */

 /************************************
  * l3 group 
  */

#ifdef VTSS_SW_OPTION_L3RT
EXPORT_SYMBOL( vtss_l3_flush );
EXPORT_SYMBOL( vtss_l3_rleg_common_get );
EXPORT_SYMBOL( vtss_l3_rleg_common_set );
EXPORT_SYMBOL( vtss_l3_rleg_get );
EXPORT_SYMBOL( vtss_l3_rleg_add );
EXPORT_SYMBOL( vtss_l3_rleg_del );
EXPORT_SYMBOL( vtss_l3_route_get );
EXPORT_SYMBOL( vtss_l3_route_add );
EXPORT_SYMBOL( vtss_l3_route_del );
EXPORT_SYMBOL( vtss_l3_neighbour_get );
EXPORT_SYMBOL( vtss_l3_neighbour_add );
EXPORT_SYMBOL( vtss_l3_neighbour_del );
EXPORT_SYMBOL( vtss_l3_counters_reset );
EXPORT_SYMBOL( vtss_l3_counters_system_get );
EXPORT_SYMBOL( vtss_l3_counters_rleg_get );
#endif /* VTSS_SW_OPTION_L3RT */

 /************************************
  * mpls group 
  */

#ifdef VTSS_FEATURE_MPLS
EXPORT_SYMBOL( vtss_mpls_l2_alloc );
EXPORT_SYMBOL( vtss_mpls_l2_free );
EXPORT_SYMBOL( vtss_mpls_l2_get );
EXPORT_SYMBOL( vtss_mpls_l2_set );
EXPORT_SYMBOL( vtss_mpls_l2_segment_attach );
EXPORT_SYMBOL( vtss_mpls_l2_segment_detach );
EXPORT_SYMBOL( vtss_mpls_segment_alloc );
EXPORT_SYMBOL( vtss_mpls_segment_free );
EXPORT_SYMBOL( vtss_mpls_segment_get );
EXPORT_SYMBOL( vtss_mpls_segment_set );
EXPORT_SYMBOL( vtss_mpls_segment_state_get );
EXPORT_SYMBOL( vtss_mpls_segment_server_attach );
EXPORT_SYMBOL( vtss_mpls_segment_server_detach );
EXPORT_SYMBOL( vtss_mpls_xc_alloc );
EXPORT_SYMBOL( vtss_mpls_xc_free );
EXPORT_SYMBOL( vtss_mpls_xc_get );
EXPORT_SYMBOL( vtss_mpls_xc_set );
EXPORT_SYMBOL( vtss_mpls_xc_segment_attach );
EXPORT_SYMBOL( vtss_mpls_xc_segment_detach );
EXPORT_SYMBOL( vtss_mpls_xc_mc_segment_attach );
EXPORT_SYMBOL( vtss_mpls_xc_mc_segment_detach );
EXPORT_SYMBOL( vtss_mpls_tc_conf_get );
EXPORT_SYMBOL( vtss_mpls_tc_conf_set );
#endif /* VTSS_FEATURE_MPLS */

 /************************************
  * macsec group 
  */

#ifdef VTSS_FEATURE_MACSEC
EXPORT_SYMBOL( vtss_macsec_init_set );
EXPORT_SYMBOL( vtss_macsec_init_get );
EXPORT_SYMBOL( vtss_macsec_secy_conf_add );
EXPORT_SYMBOL( vtss_macsec_secy_conf_get );
EXPORT_SYMBOL( vtss_macsec_secy_conf_del );
EXPORT_SYMBOL( vtss_macsec_secy_controlled_set );
EXPORT_SYMBOL( vtss_macsec_secy_controlled_get );
EXPORT_SYMBOL( vtss_macsec_secy_controlled_get );
EXPORT_SYMBOL( vtss_macsec_secy_port_status_get );
EXPORT_SYMBOL( vtss_macsec_rx_sc_add );
EXPORT_SYMBOL( vtss_macsec_rx_sc_get_next );
EXPORT_SYMBOL( vtss_macsec_rx_sc_del );
EXPORT_SYMBOL( vtss_macsec_rx_sc_status_get );
EXPORT_SYMBOL( vtss_macsec_tx_sc_set );
EXPORT_SYMBOL( vtss_macsec_tx_sc_del );
EXPORT_SYMBOL( vtss_macsec_tx_sc_status_get );
EXPORT_SYMBOL( vtss_macsec_rx_sa_set );
EXPORT_SYMBOL( vtss_macsec_rx_sa_get );
EXPORT_SYMBOL( vtss_macsec_rx_sa_activate );
EXPORT_SYMBOL( vtss_macsec_rx_sa_disable );
EXPORT_SYMBOL( vtss_macsec_rx_sa_del );
EXPORT_SYMBOL( vtss_macsec_rx_sa_lowest_pn_update );
EXPORT_SYMBOL( vtss_macsec_rx_sa_status_get );
EXPORT_SYMBOL( vtss_macsec_tx_sa_set );
EXPORT_SYMBOL( vtss_macsec_tx_sa_get );
EXPORT_SYMBOL( vtss_macsec_tx_sa_activate );
EXPORT_SYMBOL( vtss_macsec_tx_sa_disable );
EXPORT_SYMBOL( vtss_macsec_tx_sa_del );
EXPORT_SYMBOL( vtss_macsec_tx_sa_status_get );
EXPORT_SYMBOL( vtss_macsec_secy_port_counters_get );
EXPORT_SYMBOL( vtss_macsec_secy_cap_get );
EXPORT_SYMBOL( vtss_macsec_secy_counters_get );
EXPORT_SYMBOL( vtss_macsec_counters_update );
EXPORT_SYMBOL( vtss_macsec_counters_clear );
EXPORT_SYMBOL( vtss_macsec_rx_sc_counters_get );
EXPORT_SYMBOL( vtss_macsec_tx_sc_counters_get );
EXPORT_SYMBOL( vtss_macsec_tx_sa_counters_get );
EXPORT_SYMBOL( vtss_macsec_rx_sa_counters_get );
EXPORT_SYMBOL( vtss_macsec_control_frame_match_conf_set );
EXPORT_SYMBOL( vtss_macsec_control_frame_match_conf_get );
EXPORT_SYMBOL( vtss_macsec_pattern_set );
EXPORT_SYMBOL( vtss_macsec_pattern_del );
EXPORT_SYMBOL( vtss_macsec_pattern_get );
EXPORT_SYMBOL( vtss_macsec_default_action_set );
EXPORT_SYMBOL( vtss_macsec_default_action_get );
EXPORT_SYMBOL( vtss_macsec_bypass_mode_set );
EXPORT_SYMBOL( vtss_macsec_bypass_mode_get );
EXPORT_SYMBOL( vtss_macsec_bypass_tag_set );
EXPORT_SYMBOL( vtss_macsec_bypass_tag_get );
EXPORT_SYMBOL( vtss_macsec_frame_capture_set );
EXPORT_SYMBOL( vtss_macsec_port_loopback_set );
EXPORT_SYMBOL( vtss_macsec_frame_get );
#endif /* VTSS_FEATURE_MACSEC */


/************************************
 * Misc kernel-mode only exports
 */
#if defined(VTSS_FEATURE_INTERRUPTS)
EXPORT_SYMBOL( vtss_intr_cfg );
#endif /* VTSS_FEATURE_INTERRUPTS */

/************************************
 * FDMA group
 */

#if defined(VTSS_FEATURE_FDMA)
EXPORT_SYMBOL( vtss_fdma_cfg );
EXPORT_SYMBOL( vtss_fdma_ch_cfg );
EXPORT_SYMBOL( vtss_fdma_init );
EXPORT_SYMBOL( vtss_fdma_inj );
EXPORT_SYMBOL( vtss_fdma_irq_handler );
EXPORT_SYMBOL( vtss_fdma_stats_clr );
EXPORT_SYMBOL( vtss_fdma_uninit );
EXPORT_SYMBOL( vtss_fdma_xtr_add_dcbs );
EXPORT_SYMBOL( vtss_fdma_xtr_ch_from_list );
EXPORT_SYMBOL( vtss_fdma_xtr_hdr_decode );
#endif  /* VTSS_FEATURE_FDMA */
