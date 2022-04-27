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
 * \brief sFlow icli functions
 * \details This header file describes sFlow functions
 */

/**
 * \brief Function for printing sFlow statistics.
 *
 * \param session_id [IN]  Needed for being able to print
 * \param has_receiver [IN]  TRUE to print receiver statistics
 * \param has_samplers [IN]  TRUE to print samplers statistics
 * \param rcvr_idx_list [IN] List of receiver indexes
 * \param plist [IN] List of interfaces to print statistics for.
 * \param has_clear [IN]  TRUE to clear statistics.
 * \return error code
 **/
vtss_rc sflow_icli_statistics(i32 session_id, BOOL has_receiver, BOOL has_samplers, icli_range_t *rcvr_idx_list, icli_range_t *samplers_list, icli_stack_port_range_t *plist, BOOL has_clear);

/**
 * \brief Function to determine the number of receiver instances
 *
 * \param session_id [IN]  Needed for being able to print
 * \param ask [IN]  Asking
 * \param runtime [IN]  Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL sflow_icli_run_time_receiver_instances(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);

/**
 * \brief Function to determine the number of samplers instances
 *
 * \param session_id [IN]  Needed for being able to print
 * \param ask [IN]  Asking
 * \param runtime [IN]  Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL sflow_icli_run_time_sampler_instances(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);

/**
 * \brief Function to determine if IPv6 is supported
 *
 * \param session_id [IN]  Needed for being able to print
 * \param ask [IN]  Asking
 * \param runtime [IN]  Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL sflow_icli_run_time_collector_address(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);

/**
 * \brief Function to configure collector ip/hostname
 *
 * \param session_id [IN]  Needed for being able to print
 * \param rcvr_idx_list [IN] List of receiver indexes
 * \param hostname [IN]  Hostname for the collector
 * \param no [IN]  For the no command
 * \return Error code.
 **/
vtss_rc sflow_icli_collector_address(i32 session_id, icli_range_t *rcvr_idx_list, char *hostname, BOOL no);

/**
 * \brief Function for printing current sflow configuration.
 *
 * \param session_id [IN]  Needed for being able to print
 * \return Error code.
 **/
vtss_rc sflow_icli_show_flow_conf(i32 session_id);

/**
 * \brief Function for configuring receiver timeout
 *
 * \param session_id [IN]  Needed for being able to print
 * \param timeout [IN] New timeout value
 * \param rcvr_idx_list [IN] List of receiver indexes
 * \return Error code.
 **/
vtss_rc sflow_icli_timeout(i32 session_id, u32 timeout, icli_range_t *rcvr_idx_list);

/**
 * \brief Function for configuring receiver collector port
 *
 * \param session_id [IN]  Needed for being able to print
 * \param collector_port [IN] New UDP collector port
 * \param rcvr_idx_list [IN] List of receiver indexes
 * \param no [IN]  For the no command
 * \return Error code.
 **/
vtss_rc sflow_icli_collector_port(i32 session_id, u32 collector_port, icli_range_t *rcvr_idx_list, BOOL no);

/**
 * \brief Function for configuring receiver max datagram size
 *
 * \param session_id [IN]  Needed for being able to print
 * \param max_datagram_size [IN] New maximum datagram size
 * \param rcvr_idx_list [IN] List of receiver indexes
 * \param no [IN]  For the no command
 * \return Error code.
 **/
vtss_rc sflow_icli_max_datagram_size(i32 session_id, u32 max_datagram_size, icli_range_t *rcvr_idx_list, BOOL no);

/**
 * \brief Function for setting receiver to default
 *
 * \param session_id [IN]  Needed for being able to print
 * \param rcvr_idx_list [IN] List of receiver indexes
 * \return Error code.
 **/
vtss_rc sflow_icli_release(i32 session_id, icli_range_t *rcvr_idx_list);

/**
 * \brief Function for setting sampling rate
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to configure.
 * \param sampling_rate [IN] New sampling rate
 * \param sampler_idx_list [IN] List of samples instances to configure
 * \param no [IN]  For the no command
 * \return Error code.
 **/
vtss_rc sflow_icli_sampling_rate(i32 session_id, icli_stack_port_range_t *plist, u32 sampling_rate, icli_range_t *sampler_idx_list, BOOL no);

/**
 * \brief Function for enable sampler
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to configure.
 * \param no [IN]  For the no command
 * \return Error code.
 **/
vtss_rc sflow_icli_enable(i32 session_id, icli_stack_port_range_t *plist, icli_range_t *sampler_idx_list, BOOL no);

/**
 * \brief Function for setting max header size
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to configure.
 * \param max_hdr_size [IN] New max header size
 * \param sampler_idx_list [IN] List of samples instances to configure
 * \param no [IN]  For the no command
 * \return Error code.
 **/
vtss_rc sflow_icli_max_hdr_size(i32 session_id, icli_stack_port_range_t *plist, u32 max_hdr_size, icli_range_t *sampler_idx_list, BOOL no);

/**
 * \brief Function for setting counter poll insterval
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to configure.
 * \param poll_interval [IN] New counter poll interval
 * \param sampler_idx_list [IN] List of samples instances to configure
 * \param no [IN]  For the no command
 * \return Error code.
 **/
vtss_rc sflow_icli_counter_poll_interval(i32 session_id, icli_stack_port_range_t *plist, u32 poll_interval, icli_range_t *sampler_idx_list, BOOL no);

/**
 * \brief Function for setting counter poll insterval
 *
 * \param session_id  [IN] Needed for being able to print
 * \param has_ipv4    [IN] True if v_ipv4_addr is valid
 * \param v_ipv4_addr [IN] IPv4 address of agent (ourselves)
 * \param has_ipv4    [IN] True if v_ipv6_addr is valid
 * \param v_ipv6_addr [IN] IPv6 address of agent (ourselves)
 * \param no [IN]  For the no command
 * \return Error code.
 **/
vtss_rc sflow_icli_agent_ip(i32 session_id, BOOL has_ipv4, vtss_ip_t *v_ipv4_addr, BOOL has_ipv6, vtss_ipv6_t *v_ipv6_addr, BOOL no);

