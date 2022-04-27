/*

 Vitesse API software.

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

#ifndef _VTSS_PTP_API_H_
#define _VTSS_PTP_API_H_
/**
 * \file vtss_ptp_api.h
 * \brief PTP protocol engine main API header file
 *
 * This file contain the definitions of API functions and associated
 * types.
 *
 */

#include "vtss_ptp_types.h"
#include "vtss_ptp_packet_callout.h"
#include "vtss_ptp_offset_filter.h"
#include "vtss_ptp_delay_filter.h"

/*
 * Trace group numbers
 */

#define VTSS_TRACE_GRP_PTP_BASE_TIMER        1
#define VTSS_TRACE_GRP_PTP_BASE_PACK_UNPACK  2
#define VTSS_TRACE_GRP_PTP_BASE_MASTER       3
#define VTSS_TRACE_GRP_PTP_BASE_SLAVE        4
#define VTSS_TRACE_GRP_PTP_BASE_STATE        5
#define VTSS_TRACE_GRP_PTP_BASE_FILTER       6
#define VTSS_TRACE_GRP_PTP_BASE_PEER_DELAY   7
#define VTSS_TRACE_GRP_PTP_BASE_TC           8
#define VTSS_TRACE_GRP_PTP_BASE_BMCA         9
#define VTSS_TRACE_GRP_PTP_CNT               9  /* must be = number of BASE trace groups */

/**
 * \brief Clock Default Data Set structure
 */
typedef struct {
    /* static */
    UInteger8 deviceType;
    bool twoStepFlag;     /* TRUE if follow up messages are used */
    UInteger8 protocol;   /* Ethernet, IPv4 multicast or IPv4 unicast operation mode */
    BOOL oneWay;          /* TRUE if only one way measurements are done */
    ClockIdentity clockIdentity;
    BOOL tagging_enable;    /* true id VLAN tagging is enabled */
    vtss_vid_t configured_vid; /* vlan id used if VLAN tagging is enabled in the ethernet encapsulation mode */
                               /* in IPv4 encapsulation mode, the management VLAN id is used */
    UInteger8  configured_pcp; /* PCP, CFI/DEI used if VLAN tagging is enabled in the ethernet encapsulation mode */
    Integer32  mep_instance;   /* the mep instance number used when the protocol is OAM */

    UInteger8 numberPorts;
    /* dynamic */
    ClockQuality clockQuality;
    /* configurable */
    UInteger8 priority1;
    UInteger8 priority2;
    UInteger8 domainNumber;
} ptp_clock_default_ds_t;

/**
 * \brief Static part of Clock Default Data Set structure.
 * The contents of this structure is defined at clock init time.
 */
typedef struct ptp_init_clock_ds_t {
    UInteger8 deviceType;
    bool twoStepFlag;   /* TRUE if follow up messages are used */
    UInteger8 protocol; /* Ethernet, IPv4 multicast or IPv4 unicast operation mode */
    BOOL oneWay;        /* TRUE if only one way measurements are done */
    ClockIdentity clockIdentity;
    BOOL tagging_enable;    /* true id VLAN tagging is enabled */
    vtss_vid_t configured_vid; /* vlan id used if VLAN tagging is enabled in the ethernet encapsulation mode */
                               /* in IPv4 encapsulation mode, the management VLAN id is used */
    UInteger8  configured_pcp; /* PCP, CFI/DEI used if VLAN tagging is enabled in the ethernet encapsulation mode */
    UInteger16  portCount;
    Integer16  max_foreign_records;
    Integer16  max_outstanding_records; /* max number of simultaneous outstanding Delay requests in a E2E transparent clock */
    Integer32  mep_instance;            /* the mep instance number used when the protocol is OAM */
} ptp_init_clock_ds_t;



/**
 * \brief Configurable part of Clock Default Data Set structure
 */
typedef struct ptp_set_clock_ds_t {
    UInteger8 priority1;
    UInteger8 priority2;
    UInteger8 domainNumber;
} ptp_set_clock_ds_t;


/**
 * \brief Clock Current Data Set structure
 */
typedef struct {
    /* dynamic */
    UInteger16 stepsRemoved;
    vtss_timeinterval_t offsetFromMaster;
    vtss_timeinterval_t meanPathDelay;
} ptp_clock_current_ds_t;
/**
 * \brief Clock Parent Data Set structure
 */
typedef struct {
    /* dynamic */
    PortIdentity parentPortIdentity;
    bool parentStats;/* FALSE to indicate that the next two fields are not computed. */
    UInteger16 observedParentOffsetScaledLogVariance;   /* optional  */
    Integer32 observedParentClockPhaseChangeRate;       /* optional */
    ClockIdentity grandmasterIdentity;
    ClockQuality grandmasterClockQuality;
    UInteger8 grandmasterPriority1;
    UInteger8 grandmasterPriority2;
} ptp_clock_parent_ds_t;

/**
 * \brief Clock Time Properties Data Set structure
 */
typedef struct ptp_clock_timeproperties_ds_t {
    /* dynamic */
    Integer16 currentUtcOffset;
    bool currentUtcOffsetValid;
    bool leap59;
    bool leap61;
    bool timeTraceable;
    bool frequencyTraceable;
    bool ptpTimescale;
    UInteger8 timeSource;

} ptp_clock_timeproperties_ds_t;

/**
 * \brief Port Data Set structure
 */
typedef struct {
    /* static */
    PortIdentity portIdentity;
    /* dynamic */
    UInteger8 portState;
    Integer8 logMinDelayReqInterval; /* master announces min time between Delay_Req's */
    vtss_timeinterval_t peerMeanPathDelay;  /* P2P delay estimate. = 0 if E2E delay mechanism is used */
    /* configurable */
    Integer8 logAnnounceInterval;    /* interval between announce message transmissions */
    UInteger8 announceReceiptTimeout;
    Integer8 logSyncInterval;        /* interval between sync message transmissions */
    UInteger8 delayMechanism;
    Integer8 logMinPdelayReqInterval;/*only for P2P deley measurements */
    vtss_timeinterval_t    delayAsymmetry;          /* configurable delay asymmetry pr port */
    vtss_timeinterval_t    ingressLatency;          /* configurable ingress delay pr port */
    vtss_timeinterval_t    egressLatency;           /* configurable egress delay pr port */
    UInteger16 versionNumber;
    BOOL syncIntervalError;
    UInteger8       initPortInternal;        /* Internal(TRUE) or normal(FALSE) port */
    BOOL peer_delay_ok;                      /* false if Portmode is P2P and peer delay has not been measured */
} ptp_port_ds_t;

/**
 * \brief Configurable part of Port Data Set structure
 */
typedef struct ptp_set_port_ds_t {
    Integer8        logAnnounceInterval;     /* interval between announce message transmissions */
    UInteger8       announceReceiptTimeout;
    Integer8        logSyncInterval;         /* interval between sync message transmissions */
    UInteger8       delayMechanism;
    Integer8        logMinPdelayReqInterval; /* only for P2P deley measurements */
    vtss_timeinterval_t    delayAsymmetry;   /* configurable delay asymmetry pr port */
    UInteger16      versionNumber;
    UInteger8       initPortState;           /* disabled or enabled */
    UInteger8       initPortInternal;        /* Internal(TRUE) or normal(FALSE) port */
    vtss_timeinterval_t    ingressLatency;   /* configurable ingress delay pr port */
    vtss_timeinterval_t    egressLatency;    /* configurable egress delay pr port */
} ptp_set_port_ds_t;

/**
 * \brief Foreign Master Data Set structure
 */
typedef struct ptp_foreign_ds_t {
    PortIdentity foreignmasterIdentity;
    ClockQuality foreignmasterClockQuality;
    UInteger8 foreignmasterPriority1;
    UInteger8 foreignmasterPriority2;
    BOOL best;              /* true if evaluated as best master on the port */
    BOOL qualified;         /* true if the foreign master is qualified */
} ptp_foreign_ds_t;

/**
 * \brief Unicast slave configuration structure
 */
typedef struct vtss_ptp_unicast_slave_config_t {
    UInteger32 duration;
    UInteger32  ip_addr;
} vtss_ptp_unicast_slave_config_t;

/**
 * \brief Unicast slave configuration/state structure
 */
typedef struct vtss_ptp_unicast_slave_config_state_t {
    UInteger32  duration;
    UInteger32  ip_addr;
    Integer8    log_msg_period; // the granted sync interval
    UInteger8   comm_state;
} vtss_ptp_unicast_slave_config_state_t;


/**
 * \brief Unicast slave table structure
 */
typedef struct vtss_ptp_unicast_slave_table_t {
    Protocol_adr_t  master;
    PortIdentity    sourcePortIdentity;
    UInteger16      port;
    Integer8        log_msg_period; // the granted sync interval
    UInteger8       comm_state;
    UInteger32      conf_master_ip; // copy of the destination ip address, used to cancel request, when the dest. is set to 0
} vtss_ptp_unicast_slave_table_t;

/**
 * \brief Unicast master table structure
 */
typedef struct vtss_ptp_unicast_master_table_t {
    Protocol_adr_t  slave;
    UInteger16      port;
    Integer8        ann_log_msg_period; // the granted Announce interval
    BOOL            ann;                // true if sending announce messages
    Integer8        log_msg_period;     // the granted sync interval
    BOOL            sync;               // true if sending sync messages
} vtss_ptp_unicast_master_table_t;

/**
 * \brief Clock Slave Data Set structure
 */

/**
 * \brief Slave Clock state
 */
typedef enum {
    VTSS_PTP_SLAVE_CLOCK_STATE_FREERUN,       /* Free run state (initial) */
    VTSS_PTP_SLAVE_CLOCK_STATE_FREQ_LOCKING,  /* Frequency Locking */
    VTSS_PTP_SLAVE_CLOCK_STATE_PHASE_LOCKING, /* Phase Locking */
    VTSS_PTP_SLAVE_CLOCK_STATE_LOCKED,        /* Locked run state (final) */
    VTSS_PTP_SLAVE_CLOCK_STATE_HOLDOVER,      /* Holdover state (if reference is lost after the stabilization period) */
    VTSS_PTP_SLAVE_CLOCK_STATE_INVALID,       /* invalid state */
} vtss_ptp_slave_clock_state_t;


typedef struct {
    u16 port_number;         /* 0 => no slave port, 1..n => selected slave port */
    vtss_ptp_slave_clock_state_t slave_state;
    BOOL holdover_stable;   /* true is the stabilization period has expired */
    i64 holdover_adj;       /* the calculated holdover offset (ppb*10) */
} ptp_clock_slave_ds_t;

/**
 * \brief Clock Slave configuration Data structure
 */
typedef struct ptp_clock_slave_cfg_t {
    u32 stable_offset;
    u32 offset_ok;
    u32 offset_fail;
} ptp_clock_slave_cfg_t;





char * vtss_ptp_slave_state_2_text(vtss_ptp_slave_clock_state_t s);
char *vtss_ptp_ho_state_2_text(BOOL stable, i64 adj, char *str);


/**
 * \brief Create PTP instance.
 *
 * The (maximum) number of ports in the clock is defined in the rtOpts
 * structure.
 * This defines the valid port numbers for this instance to lie in.
 * Ports are specifically enabled/disabled with vtss_ptp_port_ena() and
 * vtss_ptp_port_dis() interfaces.
 *
 * \param config [IN]  pointer to a structure containing the default parameters
 * to the clock.
 *
 * \return (opaque) instance data reference or NULL.
 */
ptp_clock_handle_t vtss_ptp_clock_add(ptp_init_clock_ds_t *clock_init,
                                      ptp_set_clock_ds_t   *clock_ds,
                                      ptp_clock_timeproperties_ds_t *time_prop,
                                      ptp_set_port_ds_t    *port_config,
                                      vtss_ptp_offset_filter_handle_t servo,
                                      vtss_ptp_delay_filter_handle_t delay_filt,
                                      int localClockId);


/**
 * \brief Remove PTP instance.
 * The memory allocated for the clock are free'ed
 *
 * \param ptp_clock_handle_t The PTP clock instance handle.
 *
 */
void vtss_ptp_clock_remove(ptp_clock_handle_t ptp);

/**
 * \brief create (initialize) clock .
 * The instance has been created by vtss_ptp_clock_add.
 * the internal clock structures are initialized.
 * Performs the INITIALIZE event (P1588 9.2.6.3)
 *
 * \param ptp The PTP instance data.
 *
 * \return nothing
 */
void
vtss_ptp_clock_create(ptp_clock_handle_t ptp);

/**
 * \brief enable clock port.
 * Performs the DESIGNATED_ENABLED event (P1588 9.2.6.4)
 *
 * \param ptp [IN/OUT] The PTP instance data.
 *
 * \param portnum The clock port to enable
 *
 * \return TRUE if the operation succeeded. The operation will fail
 * if the given port number is < 1 or > the maximum allowed port
 * number (see vtss_ptp_create_instance()).
 */
bool
vtss_ptp_port_ena(ptp_clock_handle_t ptp,
                  uint portnum);

/**
 * \brief disable clock port.
 * Performs the DESIGNATED_DISABLED event (P1588 9.2.6.5)
 *
 * \param ptp The PTP instance data.
 *
 * \param portnum The clock port to disable
 *
 * \return TRUE if the operation succeeded. The operation will fail
 * if the given port number is < 1 or > the maximum allowed port
 * number (see vtss_ptp_create_instance()).
 */
bool
vtss_ptp_port_dis(ptp_clock_handle_t ptp,
                  uint portnum);


/**
 * \brief set link state for a clock port.
 *
 * \param ptp [IN/OUT] The PTP instance data.
 *
 * \param portnum The clock port
 *
 * \param enable TRUE = Link Up
 *               FALSE = Link down
 *
 * \return TRUE if the operation succeeded. The operation will fail
 * if the given port number is < 1 or > the maximum allowed port
 * number (see vtss_ptp_create_instance()).
 */
bool
vtss_ptp_port_linkstate(ptp_clock_handle_t ptp, uint portnum, bool enable);

/**
 * \brief set link state for a internal TC clock port.
 *
 * \param ptp [IN/OUT] The PTP instance data.
 *
 * \param portnum The clock port
 *
 * \return TRUE if the operation succeeded. The operation will fail
 * if the given port number is < 1 or > the maximum allowed port
 * number (see vtss_ptp_create_instance()).
 */
bool
vtss_ptp_port_internal_linkstate(ptp_clock_handle_t ptp, uint portnum);

/**
 * \brief  Read clock default data set
 *
 * Purpose: To obtain information regarding the clock's default data set
 *
 * \param ptp The PTP instance data.
 *
 * \param status The clock default data set
 */
void
vtss_ptp_get_clock_default_ds(const ptp_clock_handle_t ptp,
                              ptp_clock_default_ds_t *status);

/**
 * \brief Set Clock Default Data Set
 *
 * Purpose: To set information regarding the Clock's Default data
 *
 * \param ptp The PTP instance data.
 *
 * \param default_ds The Default Data Set
 */
void
vtss_ptp_set_clock_default_ds(ptp_clock_handle_t ptp,
                              const ptp_set_clock_ds_t *default_ds);

/**
* \brief Set Clock Quality
*
* Purpose: To set information regarding the Clock's Quality level
*
* \param ptp The PTP instance data.
*
* \param quality The Quality level
*/
void
    vtss_ptp_set_clock_quality(ptp_clock_handle_t ptp,
                               const ClockQuality *quality);

/**
 * \brief Read clock current data set
 *
 * Purpose: To obtain information regarding the clock's current data set
 *
 * \param ptp The PTP instance data.
 *
 * \param status clock current data set
 */
void
vtss_ptp_get_clock_current_ds(const ptp_clock_handle_t ptp,
                              ptp_clock_current_ds_t *status);

/**
 * \brief Read Clock parent data set.
 *
 * Purpose: To obtain information regarding the Clock's parent data set
 *
 * \param ptp The PTP instance data.
 *
 * \param status clock parent data set
 */
void
vtss_ptp_get_clock_parent_ds(const ptp_clock_handle_t ptp,
                             ptp_clock_parent_ds_t *status);

/**
 * \brief Read Clock slave data set.
 *
 * Purpose: To obtain information regarding the Clock's slave state data set
 *
 * \param ptp The PTP instance data.
 *
 * \param status clock slave data set
 */
void
vtss_ptp_get_clock_slave_ds(const ptp_clock_handle_t ptp,
                            ptp_clock_slave_ds_t *slave_ds);

/**
 * Read Clock Time Properties Data Set
 *
 * Purpose: To obtain information regarding the Clock's Time Properties data
 * Protocol Entity.
 *
 * \param ptp [IN] The PTP instance data.
 *
 * \param timeproperties_ds [OUT] The Time properties Data Set
 */
void
vtss_ptp_get_clock_timeproperties_ds(const ptp_clock_handle_t ptp,
                                     ptp_clock_timeproperties_ds_t *timeproperties_ds);

/**
 * Write Clock Time Properties Data Set
 *
 * Purpose: To set information regarding the Clock's Time Properties data
 * Protocol Entity.
 *
 * \param ptp [OUT]The PTP instance data.
 *
 * \param timeproperties_ds [IN] The Time properties Data Set
 */
void
vtss_ptp_set_clock_timeproperties_ds(ptp_clock_handle_t ptp,
                                     const ptp_clock_timeproperties_ds_t *timeproperties_ds);

/**
 * \brief Read port data set
 *
 * Purpose: To obtain information regarding the clock's port
 *
 * \param ptp The PTP instance data.
 *
 * \param portnum The PTP port number.
 *
 * \param status port data set
 *
 * \return TRUE if valid portnum, otherwise FALSE
 */
BOOL
vtss_ptp_get_port_ds(const ptp_clock_handle_t ptp,
                     uint portnum,
                     ptp_port_ds_t *status);

/**
 * \brief Set Port Data Set
 *
 * Purpose: To set information regarding the Port's data
 *
 * \param ptp The PTP instance data.
 *
 * \param portnum The PTP port number.
 *
 * \param port_ds The Port Data Set
 *
 * \return TRUE if valid portnum, otherwise FALSE
 */
BOOL
vtss_ptp_set_port_ds(ptp_clock_handle_t ptp,
                     uint portnum,
                     const ptp_set_port_ds_t *port_ds);

/**
 * \brief Get Port Foreign master Data Set
 *
 * Purpose: To get information regarding the Port's foreign masters
 *
 * \param ptp The PTP instance data.
 * \param portnum The PTP port number.
 * \param ix index in the list of foreign data.
 * \param foreign_ds The Foreign master Data Set
 *
 * \return TRUE if valid portnum and ix, otherwise FALSE
 */
BOOL
vtss_ptp_get_port_foreign_ds(ptp_clock_handle_t ptp,
                     uint portnum,
                     i16 ix,
                     ptp_foreign_ds_t *f_ds);

/**
 * \brief Set Unicast Slave Configuration
 *
 * Purpose: To set information regarding the Unicast slave
 *
 * \param ptp The PTP instance data.
 *
 * \param c The Unicast Slave Configuration
 *
 * \return TRUE if valid portnum, otherwise FALSE
 */
void
vtss_ptp_uni_slave_conf_set(ptp_clock_handle_t ptp, int ix, const vtss_ptp_unicast_slave_config_t *c);

/**
 * \brief Get Unicast Slave Configuration and status
 *
 * Purpose: To get information regarding the Unicast slave
 *
 * \param ptp   The PTP instance data.
 * \param ix    The slave table index.
 * \param c     The Unicast Slave Configuration
 *
 * \return TRUE if valid portnum, otherwise FALSE
 */
void
vtss_ptp_uni_slave_conf_state_get(ptp_clock_handle_t ptp, int ix,  vtss_ptp_unicast_slave_config_state_t *c);


/**
 * \brief Read clock slave-master communication table
 * Purpose: To obtain information regarding the clock's current slave table
 * \param ptp       The PTP instance data.
 * \param uni_slave_table [OUT]  pointer to a structure containing the table for
 *                  the slave-master communication.
 * \param ix        The index in the slave table.
 */
BOOL
vtss_ptp_clock_unicast_table_get(const ptp_clock_handle_t ptp,
                                 vtss_ptp_unicast_slave_table_t *uni_slave_table,
                                 int ix);

/**
 * \brief Read clock master-slave communication table
 * Purpose: To obtain information regarding the clock's current master table
 * \param ptp       The PTP instance data.
 * \param uni_master_table [OUT]  pointer to a structure containing the table for
 *                  the master-slave communication.
 * \param ix        The index in the master table.
 */
BOOL
vtss_ptp_clock_unicast_master_table_get(const ptp_clock_handle_t ptp,
                                 vtss_ptp_unicast_master_table_t *uni_master_table,
                                 int ix);

/**
 * \brief Tick PTP state-event machines.
 * This shall be called every 500msec by the host system.
 *
 * \param ptp The PTP instance data.
 *
 * The function drives state for the PTP instance.
 */

/* note that PTP_SCHEDULER_RATE  = 1000/(2**PTP_LOG_TICKS_PR_SEC) */
#define PTP_SCHEDULER_RATE                  10  /* no of ms pr PTP timer tick*/
#define PTP_LOG_TICKS_PR_SEC                7      /* 0 = 1 tick/s, 1 = 2 tick/s, 2 = 4 ticks/s etc. 7 = 128 ticks pr sec*/
void
vtss_ptp_tick(ptp_clock_handle_t ptp);

/**
 * \brief BPPDU event message receive.
 * This is called when a port receives an event PDU for the clock.
 *
 * \param ptp The PTP instance data.
 * \param portnum The physical port on which the frames was received.
 * \param buf_handle Handle to the received PTP PDU.
 * \param sender sender protocol address.
 *
 * \return TRUE if buffers is used for reply or forwarding
 *         FALSE if buffers is returned to platform
 */
BOOL vtss_ptp_event_rx(struct ptp_clock_t *ptpClock, uint portnum, ptp_tx_buffer_handle_t *buf_handle, Protocol_adr_t *sender);

/**
 * \brief BPPDU general message receive.
 *
 * This is called when a port receives a general PDU for the clock.
 * \param ptp The PTP instance data.
 * \param portnum The physical port on which the frames was received.
 * \param buf_handle Handle to the received PTP PDU.
 * \param sender sender protocol address.
 *
 * \return TRUE if buffers is used for reply or forwarding
 *         FALSE if buffers is returned to platform
 */
BOOL vtss_ptp_general_rx(struct ptp_clock_t *ptpClock, uint portnum, ptp_tx_buffer_handle_t *buf_handle, Protocol_adr_t *sender);


BOOL vtss_ptp_debug_mode_get(const ptp_clock_handle_t ptp, int *debug_mode);

/* Set debug_mode. */
BOOL vtss_ptp_debug_mode_set(ptp_clock_handle_t ptp, int debug_mode);

/*
 * Enable/disable the wireless variable tx delay feature for a port.
 */
BOOL vtss_ptp_port_wireless_delay_mode_set(ptp_clock_handle_t ptp, BOOL enable, int portnum);
BOOL vtss_ptp_port_wireless_delay_mode_get(ptp_clock_handle_t ptp, BOOL *enable, int portnum);

typedef struct vtss_ptp_delay_cfg_s {
    vtss_timeinterval_t base_delay;      /* wireless base delay in scaled ns */
    vtss_timeinterval_t incr_delay;      /* wireless incremental delay pr packet byte in scaled ns */
} vtss_ptp_delay_cfg_t;

/*
 * Pre notification sent from the wireless modem transmitter before the delay is changed.
 */
BOOL vtss_ptp_port_wireless_delay_pre_notif(ptp_clock_handle_t ptp, int portnum);

/*
 * Set the delay configuration, sent from the wireless modem transmitter whenever the delay is changed.
 */
BOOL vtss_ptp_port_wireless_delay_set(ptp_clock_handle_t ptp, const vtss_ptp_delay_cfg_t *delay_cfg, int portnum);

/*
 * Get the delay configuration.
 */
BOOL vtss_ptp_port_wireless_delay_get(ptp_clock_handle_t ptp, vtss_ptp_delay_cfg_t *delay_cfg, int portnum);

void vtss_ptp_set_clock_slave_config(ptp_clock_handle_t ptp, const ptp_clock_slave_cfg_t *cfg);

/**
 * \brief Other protocol timestamp receive.
 * This is called when a port receives forwarding timestamps from a non PTP protocol.
 *
 * \param ptp The PTP instance data.
 * \param portnum The physical port on which the frames was received.
 * \param ts timestamps.
 *
 * \return void
 */

typedef struct {
    vtss_timestamp_t tx_ts;
    vtss_timestamp_t rx_ts;
    vtss_timeinterval_t corr;
} vtss_ptp_timestamps_t;

void vtss_non_ptp_slave_t1_t2_rx(ptp_clock_handle_t ptp, vtss_ptp_timestamps_t *ts);
void vtss_non_ptp_slave_t3_t4_rx(ptp_clock_handle_t ptp, vtss_ptp_timestamps_t *ts);
void vtss_non_ptp_slave_timeout_rx(ptp_clock_handle_t ptp);
void *vtss_non_ptp_slave_check_port(ptp_clock_handle_t ptp);
u32 vtss_non_ptp_slave_check_port_no(ptp_clock_handle_t ptp);
void vtss_non_ptp_slave_init(ptp_clock_handle_t ptp, int portnum);




#endif /* _VTSS_PTP_API_H_ */
