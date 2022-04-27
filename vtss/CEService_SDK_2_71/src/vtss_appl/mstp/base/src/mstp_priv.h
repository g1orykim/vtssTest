 /*

  Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_MSTP_PRIV_H_
#define _VTSS_MSTP_PRIV_H_

#include "vtss_mstp_os.h"
#include "vtss_mstp_api.h"
#include "vtss_mstp_callout.h"

/**
 * \file mstp_priv.h
 * \brief RSTP private main header file
 *
 * This file contain the definitions of most data structures
 * pertaining to the implementation of a RSTP bridge and port object.
 *
 * \author Lars Povlsen <lpovlsen@vitesse.com>
 *
 * \date 09-01-2009
 */

#define N_TREE_STM	1       /* One STM per tree */
#define N_CIST_STM	4       /* Four STMs per bridge per CIST port - PortTimers is native C */
#define N_MSTI_STM	4       /* Four STMs per tree per port */

#define MSTID_MAX	4094    /* MSTIDs are in the range 1..4096 (CIST is zero) */
#define MSTID_CIST	0	/* CIST is MSTI0 - MSTID zero */

#define MSTI_MSTI1	1	/* First MSTI - MSTI1 */

/***********************************************************************
 * 17.13 RSTP performance parameters
 */

/** 802.1D 17.13.2 AgeingTime
 * 
 * The Ageing Time parameter for the Bridge (7.9.2, Table 7-5).
 */
//#define AgeingTime bridge->AgeingTime__

/** 802.1D 17.13.4 ForceProtocolVersion
 * 
 * The Force Protocol Version parameter for the Bridge (17.4,
 * 14.8.1). This can take the value 0 ("STP Compatibility" mode) or 2
 * (the default, normal operation).
 */
#define ForceProtocolVersion(b) ((b)->conf.bridge.forceVersion)

/***********************************************************************
 * 17.20 State machine conditions and parameters
 */

/** 802.1D 17.20.1 AdminEdge
 * 
 * The AdminEdgePort parameter for the Port (14.8.2).
 */
#define AdminEdge(p)		p->adminEdgePort

/** 802.1D 17.20.2 AutoEdge
 * 
 * The AutoEdgePort parameter for the Port (14.8.2).
 */
#define AutoEdge(p)	p->adminAutoEdgePort

/*** 802.1D 17.20.4 EdgeDelay
 * Returns the value of MigrateTime if operPointToPointMAC is TRUE,
 * and the value of MaxAge otherwise.
 */
#define EdgeDelay(t)	((t)->cistport->operPointToPointMAC ? MigrateTime : MaxAge(t))

/** 802.1D 17.20.5 forwardDelay
 * Returns the value of HelloTime if sendRSTP is TRUE, and the value
 * of FwdDelay otherwise.
 */
#define forwardDelay(t)	((t)->cistport->sendRSTP ? HelloTime(t) : FwdDelay(t))

/** 802.1D 17.20.9 Migrate Time
 *
 * The initial value of the mdelayWhile and edgeDelayWhile timers
 * (17.17.4, 17.17.1), fixed for all RSTP implementations conforming
 * to this specification (Table 17-1).
 */
#define MigrateTime	((timer)3)

/* 17.20.10 reRooted - See mstp_util.c */

/** mstpVersion
 *
 * TRUE if Force Protocol Version (17.13.4) is greater than or equal
 * to 3.
 */
#define mstpVersion(b) (ForceProtocolVersion(b) >= 3)

/** 802.1D 17.20.11 rstpVersion
 *
 * TRUE if Force Protocol Version (17.13.4) is greater than or equal
 * to 2.
 */
#define rstpVersion(b) (ForceProtocolVersion(b) >= 2)

/** 802.1D 17.20.12 stpVersion
 *
 * TRUE if Force Protocol Version (17.13.4) is less than 2.
 */
//#define stpVersion(b) (ForceProtocolVersion(b) < 2)

/** 802.1D 17.20.13 TxHoldCount
 * 
 * The Transmit Hold Count (17.13.12, Table 17-1).
 */
#define TxHoldCount(b)	((b)->conf.bridge.txHoldCount)

#define isCistPort(t)   (t->msti == MSTID_CIST)

/** Port/Bridge flag representation 
 */
typedef u32 timer;

 /** 14.2.1 and 13.12
  */
typedef enum PortRole {
    MasterPort = 0,     /*!< a) A value of 0 indicates Master Port */
    AltBackupPort = 1,  /*!< b) A value of 1 indicates Alternate or Backup. */
    RootPort = 2,       /*!< c) A value of 2 indicates Root. */
    DesignatedPort = 3, /*!< d) A value of 3 indicates Designated. */
    AlternatePort,
    BackupPort,
    UnknownPort,
    DisabledPort
} PortRole_t;

/** 
 * Bridge Identifier type 
 */
typedef struct BridgeId {
    u8 bridgeIdPrio[2];         /*!< Priority part */
    u8 bridgeAddress[6];        /*!< MAC address part */
} MSTP_ATTRIBUTE_PACKED BridgeId_t;

/** 
 * Path cost 
 */
typedef struct PathCost {
    u8 bytes[4];
} MSTP_ATTRIBUTE_PACKED PathCost_t;

/** 
 * Port identifier 
 */
typedef struct PortId {
    u8 bytes[2];        /*!< 4 high bits of priority and 12 for id */
} MSTP_ATTRIBUTE_PACKED PortId_t;

/** 802.1Q 13.10: The first 6 components of a port priority vector
 */
typedef struct PriorityVector {
    BridgeId_t rootBridgeId;    /* CIST only */
    PathCost_t extRootPathCost; /* CIST only */
    BridgeId_t regRootBridgeId;
    PathCost_t intRootPathCost;
    BridgeId_t DesignatedBridgeId;
    PortId_t   DesignatedPortId;
} MSTP_ATTRIBUTE_PACKED PriorityVector_t;

/** 13.7 MST Configuration Identification */
typedef struct {
    u8 cfgid_fmt;
    u8 cfgid_name[MSTP_CONFIG_NAME_MAXLEN];
    u8 cfgid_revision[2];
    u8 cfgid_digest[MSTP_DIGEST_LEN];
} MSTP_ATTRIBUTE_PACKED mstp_bpdu_cfgid_t;

/* 14.4 BPDU format - beyond configuration id */
typedef struct {
    PathCost_t intRootPathCost;
    BridgeId_t cistBridgeId;
    u8 cist_remaining_hops;
} MSTP_ATTRIBUTE_PACKED mstp_bpdu_cistinfo_t;

/** 14.6.1 MSTI Configuration Messages
 */
typedef struct {
    u8 flags;
    BridgeId_t regional_root_bridge_id;
    PathCost_t internal_root_cost;
    u8 bridge_priority;
    u8 port_priority;
    u8 remaining_hops;
} MSTP_ATTRIBUTE_PACKED mstp_bpdu_mstirec_t;

/** 802.1Q 13.24.3 - designatedTimes, rootTimes structure */
typedef struct Times {
    timer fwdDelay;             /* CIST */
    timer maxAge;               /* CIST */
    timer messageAge;           /* CIST */
    u8    remainingHops;        /* CIST/MSTI */
} Times_t;

/** 802.1D 17.19.10 infoIs_t
 */
typedef enum InfoIs {
    Received, /*!< The port has received currentinformation from the Designated Bridge */
    Mine,     /*!< Information for the port has been derived from the Root Port for the Bridge */
    Aged,     /*!< Information from the Root Bridge has been aged out */
    Disabled  /*!< If the port is disabled, infoIs is Disabled */
} InfoIs_t;

typedef enum RcvdInfo {
    SuperiorDesignatedInfo,     /*!< Superior Info Received */
    RepeatedDesignatedInfo,     /*!< Same as the Port's port priority vector or timer values */
    InferiorDesignatedInfo,     /*!< Inferior Designated Info Received */
    InferiorRootAlternateInfo,  /*!< Inferior Root Info Received */
    OtherInfo                   /*!< Other Info Received */
} RcvdInfo_t;

/** 802.1Q 13.21,13.24 Per-Port variables (CIST/MSTI common)
 */
typedef struct mstp_port {

    /** Port number for port object */
    uint port_no;                

    /** Instance number - zero for CIST */
    u8 msti;

    /** list pointer */
    struct mstp_port *next;

    /** Ancestor Tree */
    struct mstp_tree *tree;

    /** Base CIST port */
    struct mstp_cistport *cistport;

    /** 12.8.2.2.3 b) Uptime - count in seconds of the time elapsed
     * since the Port was last reset or initialized (BEGIN, 13.23).
     */
    u32 resetTime;

    /*******************************************************************
     * 13.21 (=> 802.1D 17.17) State machine timers 
     *
     * One instance per-Port of the following shall be implemented for
     * the CIST and one per-Port for each MSTI:
     */

    /** 802.1D 17.17.2 fdWhile
     * 
     * The Forward Delay timer. Used to delay Port State transitions
     * until other Bridges have received spanning tree information.
     */
    timer fdWhile;

    /** 802.1D 17.17.7 rrWhile
     * The Recent Root timer.
     */
    timer rrWhile;

    /** 802.1D 17.17.5 rbWhile
     * 
     * The Recent Backup timer. Maintained at its initial value,
     * twice HelloTime, while the Port is a Backup Port.
     */
    timer rbWhile;

    /** 802.1D 17.17.8 tcWhile
     * 
     * The Topology Change timer. TCN Messages are sent while this
     * timer is running.
     */
    timer tcWhile;

    /** 802.1D 17.17.6 rcvdInfoWhile
     * 
     * The Received Info timer. The time remaining before the
     * spanning tree information received by this Port [portPriority
      * (17.19.21) and portTimes (17.19.22)] is aged out if not
      * refreshed by the receipt of a further Configuration Message.
      */
    timer rcvdInfoWhile;

    /*******************************************************************
     *
     * 13.24 Per-Port variables 
     *
     * The following variables are as specified in 17.19 of IEEE Std
     * 802.1D. There is one instance per-Port of each variable for the
     * CIST and one per-Port for each MSTI:
     */

    /** 802.1Q 13.24.2 designatedPriority 
     */
    PriorityVector_t designatedPriority;

    /** 802.1Q 13.24.3 designatedTimes
     */
    Times_t designatedTimes;

    /** 802.1Q 13.24.9 msgPriority
     */
    PriorityVector_t msgPriority;

    /** 802.1Q 13.24.10 msgTimes
     */
    Times_t msgTimes;

    /** 802.1D 17.19.2 */
    bool agree;

    /** 802.1D 17.19.6 disputed
     * A boolean. See 17.21.10.
     */
    bool disputed;

    /* 17.19.7 fdbFlush
     *
     * A boolean. Set by the topology change state machine to instruct
     * the filtering database to remove all entries for this Port,
     * immediately if rstpVersion (17.20.11) is TRUE, or by rapid
     * ageing (17.19.1) if \a stpVersion (17.20.12) is TRUE. Reset by
     * the filtering database once the entries are removed if
     * rstpVersion is TRUE, and immediately if \a stpVersion is TRUE.
     */
    //bool fdbFlush;

    /** 802.1D 17.19.8 forward 
     */
    bool forward;

    /** 802.1D 17.19.9 forwarding
     */
    bool forwarding;

    /** 802.1D 17.19.10 infoIs
     * 
     * A variable that takes the values Mine, Aged, Received, or
     * Disabled, to indicate the origin/state of the Port's Spanning
     * Tree information (portInfo) held for the Port.
     */
     InfoIs_t infoIs;

    /** 802.1D 17.19.11 learn
     */
    bool learn;

    /** 802.1D 17.19.12 learning
     */
    bool learning;

    /** 802.1D 17.19.23 proposed
     * See 17.10.
     */
    bool proposed;
    
    /** 802.1D 17.19.24 proposing
     * See 17.10.
     */
    bool proposing;
    
    /** 802.1D 17.19.26 rcvdInfo
     * 
     * Set to the result of the rcvInfo() procedure (17.21.8).
     */
    RcvdInfo_t rcvdInfo;
    
    /** 802.1D 17.19.27 rcvdMsg
     * See 17.23.
     */
    bool rcvdMsg;
    
    /** 802.1D 17.19.30 rcvdTc
     * See 17.21.17 and 17.31.
     */
    bool rcvdTc;
    
    /** 802.1D 17.19.33 reRoot
     * See 17.29.2.
     */
    bool reRoot;
    
    /** 802.1D 17.19.34 reselect
     * See 17.28.
     */
    bool reselect;
    
    /** 802.1D 17.19.36 selected
     * See 17.28, 17.21.16.
     */
    bool selected;
    
    /** 802.1D 17.19.42 tcProp
     * 
     * Set by the Topology Change state machine of any
     * other Port, to indicate that a topology change should be
     * propagated through this Port.
     */
    bool tcProp;
    
    /** 802.1D 17.19.45 updtInfo
     * 
     * Set by the Port Role Selection state machine (17.28, 17.21.25)
     * to tell the Port Information state machine that it should copy
     * designatedPriority to portPriority and designatedTimes to
     * portTimes.
     */
    bool updtInfo;
    
    /*
     * The following variables perform the functions described in
     * 17.19 of IEEE Std 802.1D but have enhanced or extended
     * specifications or considerations. There is one instance
     * per-Port of each variable for the CIST, and one per-Port for
     * each MSTI:
     */

    /** 802.1D 17.19.3 + 802.1Q 13.24.1 */
    bool agreed;

    /** 802.1Q 13.24.11 portId
     *
     * The Port Identifier for this Port. This variable forms a
     * component of the port priority and designated priority vectors
     * (13.10,13.11).
     *
     * The four most significant bits of the Port Identifier (the
     * settable Priority component) for the CIST and for each MSTI can
     * be modified independently of the setting of those bits for all
     * other trees, as a part of allowing full and independent
     * configuration control to be exerted over each Spanning Tree
     * instance.
     */
    PortId_t portId;

    /** 802.1Q 13.24.12 portPriority  
     */
    PriorityVector_t portPriority;

    /** 802.1D 17.19.22 + 802.1Q 13.24.13 portTimes
     *
     * The portTimes variable comprises the Port's timer parameter
     * values (Message Age, Max Age, Forward Delay, and Hello
     * Time). These timer values are used in BPDUs transmitted from
     * the Port.
     */
    Times_t portTimes;

    /** 802.1D 17.19.35 + 802.1Q 13.24.15 role
     * The assigned Port Role (17.7).
     */
    PortRole_t role;
    
    /** 802.1D 17.19.37 + 802.1Q 13.24.16 selectedRole
     * 
     * The newly computed role for the Port (17.7, 17.28, 17.21.25, 17.19.35).
     */
    PortRole_t selectedRole;
    
    /** 802.1D 17.19.39 + 802.1Q 13.24.17 sync
     *
     * Set TRUE to force the Port State to be compatible with the loop
     * free active topology determined by the priority vectors held by
     * this Bridge (13.16,13.19) for this tree (CIST, or MSTI), by
     * transitioning the Port State to Discarding and soliciting an
     * Agreement if possible, if the Port is not already synchronized
     * (13.24.18).
     */
    bool sync;
    
    /** 802.1D 17.19.40 + 802.1Q 13.24.18 synced
     * 
     * TRUE only if the Port State is compatible with the loop free
     * active topology determined by the priority vectors held by this
     * Bridge for this tree (13.16,13.19).
     */
    bool synced;
    
    /*
     * The following variable(s) are additional to those specified in
     * 17.19 of IEEE Std 802.1D. There is one instance per-Port of
     * each variable for each MSTI (InternalPostPathCost) and one for
     * the CIST (ExternalPortPathCost):
     */

    /** 802.1Q 13.22 q)
     */
    u32 portPathCost;

    /* 802.1Q 13.24.5 master  
     *
     * A Boolean variable used to determine the value of the Master
     * flag for this MSTI and Port in transmitted MST BPDUs.
     *
     * Set TRUE if the Port Role for the MSTI and Port is Root Port or
     * Designated Port, and the Bridge has selected one of its Ports
     * as the Master Port for this MSTI or the mastered flag is set
     * for this MSTI for any other Bridge Port with a Root Port or
     * Designated Port Role. Set FALSE otherwise.
     */
    bool master;

    /** 13.24.6 mastered
     * 
     * A Boolean variable used to record the value of the Master flag
     * for this MSTI and Port in MST BPDUs received from the attached LAN.
     * 
     * \note master and mastered signal the connection of the MSTI to
     * the CST via the Master Port throughout the MSTI. These
     * variables and their supporting procedures do not affect the
     * connectivity provided by this revision of this standard but
     * permit future enhancements to MSTP providing increased
     * flexibility in the choice of Master Port without abandoning
     * plug-and-play network migration.
     */
    bool mastered;

    /** Configured Path Cost - 802.1Q 12.8.2.4. 
     */
    u32 adminPathCost;  

    /** Configured Port Priority - 802.1Q 12.8.2.4. 
     */
    u8  adminPortPriority;

    /** MSTI STM state */
    int state[N_MSTI_STM];

} mstp_port_t;

/** 802.1Q 13.21,13.24 Per-Port variables (CIST specific)
 */
typedef struct mstp_cistport {

    /** The tree (MSTP) / bridge/(RSTP) that this port belongs to */
    struct mstp_bridge *bridge;

    /*******************************************************************
     * 6.4 MAC attributes
     */

    /** 6.4.3 Point-to-Point MAC parameters - operPointToPointMAC */
    bool operPointToPointMAC;

    /*******************************************************************
     * 13.21 (=> 802.1D 17.17) State machine timers 
     * 
     * One instance of the following shall be implemented per-Port:
     */

    /** 802.1D 17.17.1 edgeDelayWhile
     * 
     * The Edge Delay timer. The time remaining, in the absence of a
     * received BPDU, before this port is identified as an
     * operEdgePort.
     */
    timer edgeDelayWhile;

    /** 802.1D 17.17.3 helloWhen
     * 
     * The Hello timer. Used to ensure that at least one BPDU is
     * transmitted by a Designated Port in each HelloTime period.
     */
    timer helloWhen;

    /** 802.1D 17.17.4 mdelayWhile
     * 
     * The Migration Delay timer. Used by the Port Protocol Migration
     * state machine to allow time for another RSTP Bridge on the
     * same LAN to synchronize its migration state with this Port
     * before the receipt of a BPDU can cause this Port to change the
     * BPDU types it transmits. Initialized to MigrateTime (17.13.9).
     */
    timer mdelayWhile;

    /*******************************************************************
     * The following variables perform the function specified in 17.19
     * of IEEE Std 802.1D. A single per-Port instance applies to the
     * CIST and to all MSTIs:
     */

    /** 802.1D 17.19.17 operEdge
     * 
     * The value of the operEdgePort parameter, as determined by the
     * operation of the Bridge Detection state machine (17.25).
     */
    bool operEdge;

    /** 802.1D 17.19.18 portEnabled contributor.
     * 
     * Set if the Bridge's MAC Relay Entity and Spanning Tree Protocol
     * Entity can use the MAC Service provided by the Port's MAC
     * entity to transmit and receive frames to and from the attached
     * LAN, i.e., portEnabled is TRUE if and only if:
     *
     * -# MAC_Operational (6.4.2) is TRUE; and 
     * -# Administrative Bridge Port State (14.8.2.2) for the Port is Enabled; 
     * -# AuthControlledPortStatus is Authorized [if the port is a
     *    network access port (IEEE Std 802.1X)].
     */
    bool linkEnabled;

    /** 802.1D 17.19.44 txCount
     * 
     * A counter. Incremented by the Port Transmission (17.26) state
     * machine on every BPDU transmission, and decremented used by
     * the Port Timers state machine (17.22) once a
     * second. Transmissions are delayed if txCount reaches
     * TxHoldCount (17.13.12).
     */
    timer txCount;
    
    /*******************************************************************
     * A single per-Port instance of the following variable(s) not
     * specified in IEEE Std 802.1D applies to the CIST and to all
     * MSTIs
     *
     * f) infoInternal (13.24.4)
     * g) rcvdInternal (13.24.14)
     * h) restrictedRole (13.25.14)
     * i) restrictedTcn (13.25.15)
     */

    /** 802.1Q 13.24.4 infoInternal
     * 
     * If infoIs is Received, indicating that the port has received
     * current information from the Designated Bridge for the attached
     * LAN, infoInternal is set if that Designated Bridge is in the
     * same MST Region as the receiving Bridge and reset otherwise.
     */
    bool infoInternal;

    /** 802.1Q 13.24.14 rcvdInternal
     * 
     * A Boolean variable set TRUE by the Receive Machine if the BPDU
     * received was transmitted by a Bridge in the same MST Region as
     * the receiving Bridge.
     */
    bool rcvdInternal;

    /** 13.25.14 restrictedRole
     * 
     * A Boolean value set by management. If TRUE causes the Port not
     * to be selected as Root Port for the CIST or any MSTI, even it
     * has the best spanning tree priority vector. Such a Port will be
     * selected as an Alternate Port after the Root Port has been
     * selected. This parameter should be FALSE by default. If set, it
     * can cause lack of spanning tree connectivity. It is set by a
     * network administrator to prevent bridges external to a core
     * region of the network influencing the spanning tree active
     * topology, possibly because those bridges are not under the full
     * control of the administrator.
     */
    bool restrictedRole;

    /** 13.25.15 restrictedTcn
     * 
     * A Boolean value set by management. If TRUE causes the Port not
     * to propagate received topology change notifications and
     * topology changes to other Ports. This parameter should be FALSE
     * by default. If set it can cause temporary loss of connectivity
     * after changes in a spanning trees active topology as a result
     * of persistent incorrectly learned station location
     * information. It is set by a network administrator to prevent
     * bridges external to a core region of the network, causing
     * address flushing in that region, possibly because those bridges
     * are not under the full control of the administrator or
     * MAC_Operational for the attached LANs transitions frequently.
     */
    bool restrictedTcn;

    /* 
     * The following variables perform the related functions described
     * in 17.19 of IEEE Std 802.1D but have extended
     * specifications. There is one instance per-Port of each variable
     * for the CIST:
     */

    /** 802.1D 17.19.16 + 802.1Q 13.24.7 newInfo
     * 
     * Set if a BPDU is to be transmitted. Reset by the Port Transmit
     * state machine.
     */
    bool newInfo;

    /** 802.1Q 13.24.8 newInfoMsti
     *
     * A Boolean variable set TRUE if a BPDU conveying changed MSTI
     * information is to be transmitted. It is set FALSE by the Port
     * Transmit state machine.
     */
    bool newInfoMsti;

    /** 802.1D 17.19.13 mcheck
     * 
     * May be set by management to force the Port Protocol Migration
     * state machine to transmit RST BPDUs for a MigrateTime (17.13.9)
     * period, to test whether all STP Bridges (17.4) on the attached
     * LAN have been removed and the Port can continue to transmit
     * RSTP BPDUs. Setting mcheck has no effect if stpVersion
     * (17.20.12) is TRUE, i.e., the Bridge is operating in "STP
     * Compatibility" mode.
     */
    bool mcheck;

    /* 802.1D 17.19.20 PortPathCost (Renamed ExternalPortPathCost in 802.1Q)
     *
     * The Port's contribution, when it is the Root Port, to the Root
     * Path Cost (17.3.1, 17.5, 17.6) for the Bridge.
     *
     * Merged into common, per tree port, internal/external
     * portPathCost.
     */
    //u32 ExternalPortPathCost;

    /** 802.1D 17.19.25 rcvdBpdu
     * 
     * Set by system dependent processes, this variable
     * notifies the Port Receive state machine (17.23) when a valid
     * (9.3.4) Configuration, TCN, or RST BPDU (9.3.1, 9.3.2, 9.3.3)
     * is received on the Port. Reset by the Port Receive state
     * machine.
     */
    bool rcvdBpdu;
    
    /** 802.1D 17.19.28 rcvdRSTP
     * See 17.23.
     */
    bool rcvdRSTP;
    
    /** 802.1D 17.19.29 rcvdSTP
     * See 17.23.
     */
    bool rcvdSTP;
    
    /** 802.1D 17.19.31 rcvdTcAck
     * See 17.21.17 and 17.31.
     */
    bool rcvdTcAck;
    
    /** 802.1D 17.19.32 rcvdTcn
     * See 17.21.17 and 17.31.
     */
    bool rcvdTcn;
    
    /** 802.1D 17.19.38 sendRSTP
     * See 17.24, 17.26.
     */
    bool sendRSTP;
    
    /** 802.1D 17.19.41 tcAck
     * 
     * Set if a Configuration Message with a topology
     * change acknowledge flag set is to be transmitted.
     */
    bool tcAck;
    
    /* 17.19.43 tick
     * See 17.22.
     * Note: PortTimer STM implemented "by hand". tick obsoleted.
     */
    //bool tick;
    
    /** 802.1D 17.13.1 AdminEdge
     * 
     * The AdminEdgePort parameter for the Port (14.8.2).
     */
    bool adminEdgePort;

    /** 802.1D 17.13.3 AutoEdge
     * 
     * The AutoEdgePort parameter for the Port (14.8.2).
     */
    bool adminAutoEdgePort;

    /** 802.1D 17.20.3 allSynced
     * TRUE if and only if, for all Ports for the given Tree, selected
     * is true and the port.s role is the same as its selectedRole and
     * either:
     *
     * -# synced is true; or
     * -# The port is the Root Port.
     *
     */

    /** bpduGuard - is BPDU guard enabled specifically on the port?
     */
    u8 bpduGuard;
    
    /**
     * received BPDU (parsed)
     */
    struct {
        int version;
        PortRole_t role;
        u8 type;
        u8 flags;
        PriorityVector_t priority;
        Times_t times;
        u8 mstp_records;
        bool sameRegion;
        mstp_bpdu_mstirec_t msti_rec[N_MSTI_MAX];
    } rcvBpdu;

    /** CIST Port STMs current state */ 
    int state[N_CIST_STM];

    /* Management interface */

    /** Current link speed (MB/s) */
    u32 linkspeed;

    /** Current link Duplex state */
    bool fdx;

    /** Port configuration */

    /** g) adminPointToPointMAC - the new value of the
     * adminPointToPointMAC parameter (6.4.3).
     */
    mstp_p2p_t adminPointToPointMAC;

    /** Port statistics */
    mstp_port_statistics_t stat;

    /** Non-standard property - stpInconsistent */
    bool stpInconsistent;

    /** Non-standard timer - errorRecoveryWhile
     * 
     * The Error Recovery timer. The time remaining, before a
     * stpInconsistent recovers automatically. The timer is optionally
     * enabled.
     */
    timer errorRecoveryWhile;

} mstp_cistport_t;

/* Return the CIST-specific data, given a (common) port
 */
#define getCist(p) ((mstp_cistport_t*) (void*) (&p[1]))

/** MSTP instance structure */
typedef struct mstp_tree {

    /** Parent pointer */
    struct mstp_bridge *bridge;

    /** MSTI index */
    u8 msti;

    /** MSTID (0..4094 - CIST is 0) */
    u16 MSTID;

    /** 14.8.1.1 b) Time Since Topology Change - the count in seconds of the
     * time since the tcWhile timer (17.17.8) for any Port was
     * non-zero.
     */
    u32 lastTopologyChange;

    /** 14.8.1.1 c) Topology Change Count - the count of times that there has
     * been at least one non-zero tcWhile timer (17.17.8).
     */
    u32 topologyChangeCount;

    /** 14.8.1.1 d) Topology Change. Asserted if the tcWhile timer (17.17.8)
     * for any Port is non-zero.
     */
    bool topologyChange;

    /* 17.18 Per-Bridge variables */
    
    /** 802.1D 17.18.2 BridgeIdentifier
     * 
     * The unique Bridge Identifier assigned to this Bridge,
     * comprising two components - the Bridge Identifier Priority, which
     * may be modified by management (see 9.2.5 and 14.8.1.2) and is
     * the more significant when Bridge Identifiers are compared, and
     * a component derived from the Bridge Address (7.12.5), which
     * guarantees uniqueness of the Bridge Identifiers of different
     * Bridges.
     */
    BridgeId_t BridgeIdentifier;

    /** 802.1Q 13.23.3 BridgePriority
     */
    PriorityVector_t BridgePriority;

    /** 802.1D 17.18.5 rootPortId
     *
     * The Port Identifier of the Root Port - this is the fifth
     * component of the root priority vector, as defined in 17.6.
     */
    PortId_t rootPortId;

    /** 802.1D 17.18.6 rootPriority
     *
     * The first four components of the Bridge's root priority vector,
     * as defined in 17.6.
     */
    PriorityVector_t rootPriority;

    /** 802.1Q 13.24.7 rootTimes
     *
     * For the CIST, the Bridge's timer parameter values (Message Age,
     * Max Age, Forward Delay, and remainingHops). The values of these
     * timers are derived (see 13.26.23) from the values stored in the
     * CIST's portTimes parameter (13.24.13) for the Root Port or from
     * BridgeTimes (13.23.4).
     *
     * For a given MSTI, the value of remainingHops derived (13.26.23)
     * from the value stored in the MSTI's portTimes parameter
     * (13.24.13) for the Root Port or from BridgeTimes (13.23.4).
     */
    Times_t rootTimes;

    /** Port map (array) - NULL entries for unmapped ports */
    mstp_port_t **portmap;

    /** Port list - skips unmapped entries */
    mstp_port_t *portlist;

    /** MSTI STM state(s) */
    int state[N_TREE_STM];

} mstp_tree_t;

typedef struct {
    u16 map[MSTP_MAX_VID];
} mstp_mstidmap_t;

/** Main MSTP structure */
struct mstp_bridge {

    /** The number of ports in this bridge. Valid port numbers are
     * [0; n_ports-1]. */
    uint n_ports;

    /** The bridge identifier for CIST & MSTIs */
    mstp_macaddr_t bridge_id;

    /**
     * lock state - nesting allowed.
     */
    uint lock;


    /************************************************************************
     * 
     * MSTI instance data
     *
     */

    /**
     *
     */
    mstp_tree_t *trees[N_MSTI_MAX];

    /** 
     * 8.9.1 MST Configuration table 
     *
     * \note Only 1 byte per entry
     */
    mstp_map_t mst_config;

    /** 
     * 8.9.2 Configuration Identification (As seen in a BPDU)
     */
    mstp_bpdu_cfgid_t cfgid;

    /** 802.1Q 13.23.4 BridgeTimes
     * 
     * For the CIST, BridgeTimes comprises:
     *
     * a) The current values of Bridge Forward Delay and Bridge Max
     * Age (see Table 17-1 IEEE Std 802.1D). These parameter values
     * are determined only by management;
     *
     * b) A Message Age value of zero.
     *
     * c) The current value of MaxHops (13.22). This parameter value
     * is determined only by management.
     *
     * For a given MSTI, BridgeTimes comprises:
     *
     * d) The current value of MaxHops (13.22). This parameter value
     * is determined only by management;
     *
     * BridgeTimes is used by updtRolesTree() in determining the value
     * of the RootTimes variable (13.23.7).
     *
     * BridgeTimes is shared by all MSTIs (and the CIST).
     */
    Times_t BridgeTimes;

    /* 802.1Q 13.22
     *
     * The following parameters are as specified in 17.13 of IEEE Std
     * 802.1D for RSTP. A single value of each parameter applies to
     * the MST Bridge as a whole, including all Ports and all CIST and
     * MSTI state machines.
     *
     * e) Force Protocol Version
     * f) Bridge Forward Delay
     * g) Transmit Hold Count
     * h) Migrate Time
     * i) Bridge Max Age
     *
     * (See mstp->conf.bridge)
     */

    struct {
        mstp_bridge_param_t bridge;
        u8 bridgePriority[N_MSTI_MAX];
        mstp_port_param_t *cist;
        mstp_msti_port_param_t *msti;
    } conf;

    /** 
     * The CIST tree - always present (within bridge instance)
     */
    mstp_tree_t ist;

};

/** BPDU decode variants */
typedef enum {
    BPDU_ILL,
    BPDU_CONFIG,
    BPDU_TCN,
    BPDU_RSTP,
    BPDU_MSTP
} mstp_bpdutype_t;

#define LLC_UI				((u8) 0x03)
#define BPDU_L_SAP			((u8) 0x42)

#define MSTP_BPDU_TYPE_CONFIG		((u8) 0x00) /* STP - Config */
#define MSTP_BPDU_TYPE_TCN		((u8) 0x80) /* STP - TCN */
#define MSTP_BPDU_TYPE_RSTP		((u8) 0x02)

#define MSTP_BPDU_FLAG_TC               ((u8)(1 << 0)) /* Bit 0 */
#define MSTP_BPDU_FLAG_PROPOSAL         ((u8)(1 << 1)) /* Bit 1 */
//#define MSTP_BPDU_FLAG_ROLE_MASK	((u8)(0x3 << 2)) * Bit 2-3 */
//#define MSTP_BPDU_FLAG_ROLE_GET(flags)	((u8)((flags & MSTP_BPDU_FLAG_ROLE_MASK)>>2))
#define MSTP_BPDU_FLAG_LEARNING		((u8)(1 << 4)) /* Bit 4 */
#define MSTP_BPDU_FLAG_FORWARDING	((u8)(1 << 5)) /* Bit 5 */
#define MSTP_BPDU_FLAG_AGREEMENT	((u8)(1 << 6)) /* Bit 6 */
#define MSTP_BPDU_FLAG_TC_ACK		((u8)(1 << 7)) /* Bit 7 */
#define MSTP_BPDU_FLAG_MASTER		((u8)(1 << 7)) /* Bit 7 - only in MSTI record */

#define MSTP_BPDU_FLAG_MASK		(MSTP_BPDU_FLAG_TC|             \
                                         MSTP_BPDU_FLAG_PROPOSAL|       \
                                         MSTP_BPDU_FLAG_LEARNING|       \
                                         MSTP_BPDU_FLAG_FORWARDING|     \
                                         MSTP_BPDU_FLAG_AGREEMENT)

/*** BPDU format - 14.4.
 */

/* 14.4 BPDU format */
typedef struct {
    mstp_bpdu_cfgid_t cfgid;
    mstp_bpdu_cistinfo_t cistinfo;
} MSTP_ATTRIBUTE_PACKED mstp_bpdu_mstpstatic_t;

/*** BPDU format - 9.3
 *
 */
/*lint -esym(768,src_mac) ... must be filled in by xmitter */
typedef struct {

    /* Ethernet encapsulation - in PDU for simplicity*/
    u8 dst_mac[6];
    u8 src_mac[6];              
    u8 len8023[2];
    u8 dsap;
    u8 ssap;
    u8 llc;

    /* BPDU starts here */
    u8 protocolIdentifier[2];
    u8 protocolVersionIdentifier;
    u8 bpduType;
    u8 __tcn_marker[0];         /* TCN PDU ends here */

    u8 flags;
    
    /* Priority vector */
    BridgeId_t rootBridgeId;    /* CIST rootBridgeId */
    PathCost_t rootPathCost;    /* CIST extRootPathCost */
    BridgeId_t DesignatedBridgeId; /* CIST regRootBridgeId */
    PortId_t   DesignatedPortId;   /* CIST portId */

    u8 messageAge[2];
    u8 maxAge[2];
    u8 helloTime[2];
    u8 forwardDelay[2];

    u8 __config_marker[0];      /* STP Config PDU ends here */

    u8 version1Length;          /* Only for RSTP */

    u8 __rstp_marker[0];        /* RSTP PDU ends here */

    u8 version3Length[2];       /* Only for MSTP */

    /* MSTP config ID + CIST onfo*/
    mstp_bpdu_mstpstatic_t mstp;

    u8 __mstp_marker[0];        /* MSTP static PDU ends here */

    /* 0-64 records (up to N_MSTI_MAX in this version) */
    mstp_bpdu_mstirec_t mstpcfg[N_MSTI_MAX];

} MSTP_ATTRIBUTE_PACKED mstp_bpdu_t;

/** Enumerate across all MSTI ports for a tree.
 * { statements; } block is supposed to follow
 */
#define ForAllPorts(t, p) for(p = (t)->portlist; p ; p = p->next)

/** Enumerate across all CIST ports for a tree.
 * { statements; } block is supposed to follow
 */
#define ForAllCistPorts(m, p) ForAllPorts(&m->ist, p)

/** Enumerate Trees For a Bridge (with start index)
 */
#define _ForAllTrees(_m, _start, _t_, _s)                                \
    do {  mstp_bridge_t *__fora_b = _m; uint __fora_ix;                 \
        for(__fora_ix = _start; __fora_ix < N_MSTI_MAX; __fora_ix++) { \
            mstp_tree_t *_t_ = __fora_b->trees[__fora_ix];              \
            if(_t_) { _s }                                              \
        }                                                               \
    } while(0)

/** Enumerate Trees For a Bridge
 */
#define ForAllTrees(_m, _t_, _s)                \
    _ForAllTrees(_m, 0, _t_, _s)

/** Enumerate MSTI Trees For a Bridge
 */
#define ForAllMstiTrees(_m, _t_, _s)            \
    _ForAllTrees(_m, 1, _t_, _s)

/** Enumerate across CIST and MSTI ports for a port number.
 */
#define ForAllPortNo(_m, _pno, _s)                                      \
    ForAllTrees(_m, _tree_, {                                           \
            mstp_port_t *_tp_ = get_tport(_tree_, _pno);                \
            if(_tp_) { _s }                                             \
        })

/** Enumerate across MSTI ports for a port number.
 */
#define ForAllMsti(_m, _pno, _s)                                        \
    ForAllMstiTrees(_m, _tree_, {                                       \
            mstp_port_t *_tp_ = get_tport(_tree_, _pno);                \
            if(_tp_) { _s }                                             \
        })

#endif /* _VTSS_MSTP_PRIV_H_ */
