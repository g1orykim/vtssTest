/*

 Vitesse sFlow software.

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

#ifndef _SFLOW_API_H_
#define _SFLOW_API_H_

#include "ip2_api.h"
#include "network.h" /* INET6_ADDRSTRLEN */

/** \file sFlow API.
 *
 * Defines the sFlow Agent API.
 *
 * Note: None of the configuration is saved to flash. The reason
 * can be read under #sflow_rcvr_t::timeout.
 *
 * This means that the configuration will be lost if you boot or
 * change master.
 */

#include "vtss_api.h" /* For vtss_sflow_XXX and u8, u16, etc. */

/**
 * Definition of return code errors.
 * See also sflow_error_txt in sflow.c
 */
enum {
    SFLOW_ERROR_ISID = MODULE_ERROR_START(VTSS_MODULE_ID_SFLOW),
    SFLOW_ERROR_NOT_MASTER,
    SFLOW_ERROR_PORT,
    SFLOW_ERROR_INSTANCE,
    SFLOW_ERROR_RECEIVER,
    SFLOW_ERROR_RECEIVER_IDX,
    SFLOW_ERROR_RECEIVER_VS_SAMPLING_DIRECTECTION,
    SFLOW_ERROR_RECEIVER_OWNER,
    SFLOW_ERROR_RECEIVER_TIMEOUT,
    SFLOW_ERROR_RECEIVER_DATAGRAM_SIZE,
    SFLOW_ERROR_RECEIVER_HOSTNAME,
    SFLOW_ERROR_RECEIVER_UDP_PORT,
    SFLOW_ERROR_RECEIVER_SOCKET_CREATE,
    SFLOW_ERROR_RECEIVER_CP_INSTANCE,
    SFLOW_ERROR_RECEIVER_FS_INSTANCE,
    SFLOW_ERROR_DATAGRAM_VERSION,
    SFLOW_ERROR_RECEIVER_ACTIVE,
    SFLOW_ERROR_AGENT_IP,
    SFLOW_ERROR_ARGUMENT,
};
char *sflow_error_txt(vtss_rc rc);

/**
 * Number of Flow Sampler and Counter Poller instances per port (one
 * unified value).
 * For Flow Samplers:
 * Setting this to 1 will cause us to support any flow sample rate
 * from [1; N] that the H/W supports.
 * Setting this to a value > 1 will cause us to support only powers of
 * two of N, i.e. 1, 2, 4, ..., up to the number that the physical
 * hardware supports. In this case, the S/W will configure the H/W
 * to the fastest rate, and perform S/W-based sub-sampling.
 * See also www.sflow.org/sflow_version_5.txt, chapter 4.2.2.
 *
 * For Counters Pollers:
 * Counter polling interval can be set from 1..X regardless of
 * the value of SFLOW_INSTANCE_CNT.
 */
#define SFLOW_INSTANCE_CNT 1

/**
 * Setting the number of receviers to the number of flow samplers/counter
 * pollers (SFLOW_INSTANCE_CNT) is advisable, since that will provide
 * any receiver with the option of sampling a given port even when
 * other receivers are sampling the same port.
 */
#define SFLOW_RECEIVER_CNT SFLOW_INSTANCE_CNT

/**
 * Setting the number of receivers to a value smaller than the number of
 * instances per port doesn't make sense, since noone will be able to
 * take advantage of the additional per-port instances, because the code
 * doesn't allow the same receiver to take control of more than one per-port
 * instance at a time.
 */
#if SFLOW_INSTANCE_CNT > SFLOW_RECEIVER_CNT
#error "Setting SFLOW_INSTANCE_CNT > SFLOW_RECEIVER_CNT doesn't make sense"
#endif

#if SFLOW_INSTANCE_CNT == 1
/**
 * This is the index to use if there's only one instance per port.
 */
#define SFLOW_INSTANCE_IDX 1
#endif

#if SFLOW_RECEIVER_CNT == 1
/**
 * This is the index to use if there's only one receiver.
 */
#define SFLOW_RECEIVER_IDX 1
#endif

#define SFLOW_AGENT_IP_TYPE_DEFAULT          VTSS_IP_TYPE_IPV4
#define SFLOW_AGENT_IP_ADDR_DEFAULT          0x7f000001 /* loopback */

#define SFLOW_RECEIVER_TIMEOUT_MAX           0x7FFFFFFF

#define SFLOW_RECEIVER_UDP_PORT_MAX          65535
#define SFLOW_RECEIVER_UDP_PORT_DEFAULT      6343

// If you change any of the following ranges or defaults, remember to also update
// the sFlow HTML pages (conf/status/help), because the numbers are hard-coded there.
#define SFLOW_RECEIVER_DATAGRAM_SIZE_MIN     200
#define SFLOW_RECEIVER_DATAGRAM_SIZE_MAX     1468 /* Multiple of 4 and 50 bytes less than an MTU to allow for Ethernet+IP+UDP header */
#define SFLOW_RECEIVER_DATAGRAM_SIZE_DEFAULT 1400

#define SFLOW_FLOW_HEADER_SIZE_MIN           14
#define SFLOW_FLOW_HEADER_SIZE_MAX           200
#define SFLOW_FLOW_HEADER_SIZE_DEFAULT       128

#define SFLOW_FLOW_SAMPLING_RATE_DEFAULT     0

#define SFLOW_POLLING_INTERVAL_MIN           0
#define SFLOW_POLLING_INTERVAL_MAX           3600
#define SFLOW_POLLING_INTERVAL_DEFAULT       SFLOW_POLLING_INTERVAL_MIN

#define SFLOW_DATAGRAM_VERSION               5

#define SFLOW_OWNER_LEN                      128
#define SFLOW_HOSTNAME_LEN                   255 /* To support both IPv4, IPv6, and a valid hostname (for CLI and Web, but not SNMP) */

#define SFLOW_OWNER_LOCAL_MANAGEMENT_STRING  "<Configured through local management>"

/**
 * sflow_rcvr_t
 * Uniquely identifies one sFlow Receiver (a.k.a. collector).
 *
 * Up to SFLOW_RECEIVER_CNT receivers can be configured.
 */
typedef struct {
    /**
     * The entity making use of this receiver table entry.
     * Now owned when empty.
     * According to RFC1757, it's of size 0..127, but
     * I'm not sure whether that includes a trailing '\0'.
     * Better make it 128 bytes long.
     */
    char owner[SFLOW_OWNER_LEN];

    /**
     * The time (in seconds) remaining before the sampler is
     * released and stops sampling.
     * Valid values: [0; SFLOW_RECEIVER_TIMEOUT_MAX].
     *
     * Note that this is the original value configured when
     * read back. The current timeout left is held in
     * sflow_rcvr_info_t::timeout_left.
     *
     * Implementation note:
     *   This attribute is the reason that it doesn't make sense
     *   to save the receiver configuration to flash. How would
     *   you maintain a timeout across boots? One could reserve
     *   0xFFFFFFFF for a receiver that never times out, of course,
     *   but that's beyond the sFlow Spec.
     */
    u32 timeout;

    /**
     * The maximum number of data bytes that can be sent in
     * a single datagram. The manager should set this value
     * to avoid fragmentation of the sFlow datagrams.
     *
     * Valid values: [SFLOW_RECEIVER_DATAGRAM_SIZE_MIN; SFLOW_RECEIVER_DATAGRAM_SIZE_MAX].
     * Default: SFLOW_RECEIVER_DATAGRAM_SIZE_DEFAULT.
     */
    u32 max_datagram_size;

    /**
     * IPv4/IPv6 address or hostname of receiver to which the datagrams are sent.
     */
    u8 hostname[SFLOW_HOSTNAME_LEN];

    /**
     * UDP port number on the receiver to send the datagrams to.
     *
     * Valid values: [1; 65535].
     * Default: 6343.
     */
    u16 udp_port;

    /**
     * The version of sFlow datagrams that we should send.
     * Valid values [5; 5] (we only support v. 5).
     * Default: 5.
     */
    i32 datagram_version;
} sflow_rcvr_t;

/**
 * sflow_rcvr_info_t
 */
typedef struct {
    /**
     * The IP address of the current receiver, as binary and including type.
     * If there is no current receiver, the type is IPv4 and address 0.0.0.0
     */
    vtss_ip_addr_t ip_addr;

    /**
     * This is an IPv4 or IPv6 representation of #ip_addr.
     */
    char ip_addr_str[INET6_ADDRSTRLEN];

    /**
     * The number of seconds left for this receiver.
     */
    u32  timeout_left;
} sflow_rcvr_info_t;

/**
 * sflow_fs_t
 * Flow sampler entry. There are SFLOW_INSTANCE_CNT such samplers per port.
 *
 * Indexed by <sFlowFsDataSource, sFlowFsInstance>
 */
typedef struct {
    /**
     * Enable or disable this entry.
     * There are several ways to make this entry active, but having a
     * single enable/disable parameter allows for keeping old config
     * and simply disable it.
     */
    BOOL enabled;

    /**
     * One-based index into the sflow_rcvr table. 0 means that this entry is free.
     * Valid values: [0; SFLOW_RECEIVER_CNT].
     * Default: 0.
     */
    u32 receiver;

    /**
     * The statistical sampling rate for packet sampling from this source.
     *
     * See discussion under SFLOW_INSTANCE_CNT for a thorough description
     * of possible adjustments made by the S/W.
     */
    u32 sampling_rate;

    /**
     * The maximum number of bytes that should be copied from a sampled packet.
     * Valid values: [SFLOW_FLOW_HEADER_SIZE_MIN; SFLOW_FLOW_HEADER_SIZE_MAX].
     * Default: SFLOW_FLOW_HEADER_SIZE_DEFAULT.
     */
    u32 max_header_size;

    /**
     * The flow sampler type.
     *
     * The sFlow spec says:
     *   Ideally the sampling entity will perform sampling on all
     *   flows originating from or destined to the specified interface.
     *   However, if the switch architecture only allows input or output
     *   sampling then the sampling agent is permitted to only sample input
     *   flows input or output flows.
     *
     * In the first take, this was set to VTSS_SFLOW_TYPE_ALL, but it appears
     * that sFlow monitors (like InMon's sFlowTrend) report too high ingress
     * rate, because when sending an Rx sample, we're only able to report
     * input interface and not output interface. For Tx samples, we can
     * report both interfaces. Therefore, it has been decided to change
     * the default to VTSS_SFLOW_TYPE_TX. This in turn means that if only
     * one port is sFlow-enabled, and that port never transmits any frames,
     * then no sFlow samples will be sent.
     * But as https://groups.google.com/forum/#!topic/sflow/dhnKOHyGl9I and
     * a later private conversation with Stuart Johnston from InMon state,
     * sFlow is meant to be enabled on all interfaces, which means that this
     * will normally not be a problem. See also Bugzilla#10879.
     *
     * For debug reasons, this field is published but it's only inteded to be
     * changeable via debug CLI commands.
     */
    vtss_sflow_type_t type;
} sflow_fs_t;

/**
 * sflow_cp_t
 * Counter Poller entry. There are SFLOW_INSTANCE_CNT such samplers per port.
 *
 * Indexed by <sFlowCpDataSource, sFlowCpInstance>
 */
typedef struct {
    /**
     * Enable or disable this entry.
     * There are several ways to make this entry active, but having a
     * single enable/disable parameter allows for keeping old config
     * and simply disable it.
     */
    BOOL enabled;

    /**
     * One-based index into the sflow_rcvr table. 0 means that this entry is free.
     * Valid values: [0; SFLOW_RECEIVER_CNT] (where 0 frees it).
     * Default: 0
     */
    u32 receiver;

    /**
     * The maximum number of seconds between sampling of counters for this port. 0 disables sampling.
     * Valid values: [SFLOW_POLLING_INTERVAL_MIN; SFLOW_POLLING_INTERVAL_MAX].
     * Default: 0
     */
    u32 interval;
} sflow_cp_t;

/**
 * sflow_rcvr_statistics_t
 * Per-receiver statistics.
 * The statistics get cleared automatically when
 * a new receiver is configured for this receiver
 * index or a receiver unregisters itself or
 * times out.
 */
typedef struct {
    /**
     * Counting number of times datagrams were sent successfully.
     */
    u64 dgrams_ok;

    /**
     * Counting number of times datagram transmission failed
     * (for instance because the receiver could not be reached).
     */
    u64 dgrams_err;

    /**
     * Counting number of attempted transmitted flow samples.
     * If #dgrams_err is 0, this corresponds to the actual number
     * of transmitted flow samples.
     */
    u64 fs;

    /**
     * Counting number of attempted transmitted counter samples.
     * If #dgrams_err is 0, this corresponds to the actual number
     * of transmitted counter samples.
     */
    u64 cp;
} sflow_rcvr_statistics_t;

/**
 * sflow_port_statistics_t
 * Per-port statistics.
 * The statistics get cleared automatically for
 * a given instance on the port, when that instance
 * changes configuration (also when just the interval
 * is changed). fs_rx/tx and cp get cleared separately.
 */
typedef struct {
    /**
     * Number of flow samples received on the master that
     * were Rx sampled.
     */
    u64 fs_rx[SFLOW_INSTANCE_CNT];

    /**
     * Number of flow samples received on the master that
     * were Tx sampled.
     */
    u64 fs_tx[SFLOW_INSTANCE_CNT];

    /**
     * Number of counter samples received on the master.
     */
    u64 cp[SFLOW_INSTANCE_CNT];
} sflow_port_statistics_t;

/**
 * sflow_switch_statistics_t
 * Per-switch statistics.
 * Bundles together all port statistics.
 */
typedef struct {
    /**
     * One port statistics set of counters per port.
     */
    sflow_port_statistics_t port[VTSS_PORTS];
} sflow_switch_statistics_t;

/**
 * sflow_agent_t
 *
 * Holds part of the agent configuration, particularly
 * for use by SNMP.
 */
typedef struct {
    /**
     * Uniquely identifies the version and implementation of the sFlow MIB.
     * Format: <MIB Version>;<Organization>;<Software Revision>
     */
    const i8 *version;

    /**
     * This switch's IP address, or rather, by default the IPv4 loopback address,
     * but it can be changed from CLI and Web. Will not get persisted.
     * The aggregate type allows for holding either an IPv4 and IPv6 address.
     */
    vtss_ip_addr_t agent_ip_addr;
} sflow_agent_t;

/**
 * \brief Get general agent information.
 *
 * \param agent_cfg [OUT] Current configuration.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to a string.
 */
vtss_rc sflow_mgmt_agent_cfg_get(sflow_agent_t *agent_cfg);

/**
 * \brief Set general agent information.
 *
 * \param agent_cfg [IN] Current configuration.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to a string.
 */
vtss_rc sflow_mgmt_agent_cfg_set(sflow_agent_t *agent_cfg);

/**
 * \brief Get receiver configuration.
 *
 * Along with the configuration, we also pass the resolved IP address
 * of the receiver - in case the user has inserted a hostname in a
 * previous call to sflow_mgmt_rcvr_cfg_set().
 *
 * \param rcvr_idx [IN]  Receiver index ([1; SFLOW_RECEIVER_CNT]).
 * \param rcvr_cfg [OUT] Current configuration.
 * \param rcvr_info [OUT] Current receiver info. May be NULL.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to a string.
 */
vtss_rc sflow_mgmt_rcvr_cfg_get(u32 rcvr_idx, sflow_rcvr_t *rcvr_cfg, sflow_rcvr_info_t *rcvr_info);

/**
 * \brief Set receiver configuration.
 *
 * \param rcvr_idx [IN] Receiver index ([1; SFLOW_RECEIVER_CNT]).
 * \param rcvr_cfg [IN] New configuration.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to get error string.
 */
vtss_rc sflow_mgmt_rcvr_cfg_set(u32 rcvr_idx, sflow_rcvr_t *rcvr_cfg);

/**
 * \brief Get flow sampler configuration.
 *
 * \param isid     [IN]  Switch ID
 * \param port     [IN]  Port number
 * \param instance [IN]  Instance (1..SFLOW_INSTANCE_CNT)
 * \param cfg      [OUT] Configuration.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc sflow_mgmt_flow_sampler_cfg_get(vtss_isid_t isid, vtss_port_no_t port, u16 instance, sflow_fs_t *cfg);

/**
 * \brief Set flow sampler configuration.
 *
 * \param isid     [IN] Switch ID
 * \param port     [IN] Port number
 * \param instance [IN] Instance (1..SFLOW_INSTANCE_CNT)
 * \param cfg      [IN] Configuration.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc sflow_mgmt_flow_sampler_cfg_set(vtss_isid_t isid, vtss_port_no_t port, u16 instance, sflow_fs_t *cfg);

/**
 * \brief Get counter polling configuration.
 *
 * \param isid     [IN]  Switch ID
 * \param port     [IN]  Port number
 * \param instance [IN]  Instance (1..SFLOW_INSTANCE_CNT)
 * \param cfg      [OUT] Configuration.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc sflow_mgmt_counter_poller_cfg_get(vtss_isid_t isid, vtss_port_no_t port, u16 instance, sflow_cp_t *cfg);

/**
 * \brief Set counter polling configuration.
 *
 * \param isid     [IN] Switch ID
 * \param port     [IN] Port number
 * \param instance [IN] Instance (1..SFLOW_INSTANCE_CNT)
 * \param cfg      [IN] Configuration.
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc sflow_mgmt_counter_poller_cfg_set(vtss_isid_t isid, vtss_port_no_t port, u16 instance, sflow_cp_t *cfg);

/**
 * \brief Get receiver statistics
 *
 * \param isid       [IN]  Switch ID
 * \param statistics [OUT] Statistics. If NULL, #clear must be TRUE.
 * \param clear      [IN]  TRUE => Clears statistics before returning them (if #statistics is non-NULL)
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc sflow_mgmt_rcvr_statistics_get(u32 rcvr_idx, sflow_rcvr_statistics_t *statistics, BOOL clear);

/**
 * \brief Get switch counter polling and flow sampling statistics
 *
 * Notice that the intance indices in sflow_port_statistics_t are 0-based.
 *
 * \param isid       [IN]  Switch ID
 * \param statistics [OUT] Statistics
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc sflow_mgmt_switch_statistics_get(vtss_isid_t isid, sflow_switch_statistics_t *statistics);

/**
 * \brief Clear per-instance per-port counter polling and flow sampling statistics
 *
 * \param isid       [IN] Switch ID
 * \param port       [IN] Port number
 * \param instance   [IN] Instance (1..N)
 *
 * \return VTSS_RC_OK on success. Anything else on error. Use error_txt() to convert to string.
 */
vtss_rc sflow_mgmt_instance_statistics_clear(vtss_isid_t isid, vtss_port_no_t port, u16 instance);

/**
 * \brief sFlow module initialization function.
 *
 * \param data [IN]  Initialization state.
 *
 * \return VTSS_RC_OK always.
 */
vtss_rc sflow_init(vtss_init_data_t *data);

#endif /* _SFLOW_API_H_ */

