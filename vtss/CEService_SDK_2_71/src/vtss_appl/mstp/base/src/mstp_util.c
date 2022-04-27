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

#include "mstp_priv.h"
#include "mstp_util.h"
#include "mstp_misc.h"

/**
 * \file mstp_util.c
 * \brief State Machine Procedures
 *
 * This file contain the implementation of the <em> 802.1Q-2005
 * 13.25-26: State machine conditionss, parameters and
 * procedures. </em>
 *
 * \author Lars Povlsen <lpovlsen@vitesse.com>
 *
 * \date 25-03-2009
 */

/**
 * ieee_bridge - IEEE 802.1 bridge group address 
 */
static const u8 ieee_bridge[6] = { (u8) 0x01, (u8) 0x80, (u8) 0xC2, 
                                   (u8) 0x00, (u8) 0x00, (u8) 0x00 };

/**
 * role2flags() - Convert a role into the eqivalent BPDU flag bits
 */
static u8 
role2flags(PortRole_t role) 
{
    unsigned int flags = role;
    switch(role) {
    case RootPort:
    case DesignatedPort:
        /* 1:1 mapping */
        break;
    case AlternatePort:
    case BackupPort:
        flags = AltBackupPort;  /* Common class */
        break;
    default:
        flags = UnknownPort;
    }
    return (u8) (flags << 2);   /* Bits 2-3 */
}

/**
 * vector2s() - Convert a Prriority Vector to a display string
 */
static const char *vector2s(char *str, size_t sz, const PriorityVector_t *vec)
{
    char root[32], regRoot[32], desg[32];
    (void) vtss_mstp_bridge2str(root, sizeof(root), vec->rootBridgeId.bridgeIdPrio);
    (void) vtss_mstp_bridge2str(regRoot, sizeof(regRoot), vec->regRootBridgeId.bridgeIdPrio);
    (void) vtss_mstp_bridge2str(desg, sizeof(desg), vec->DesignatedBridgeId.bridgeIdPrio);
    snprintf(str, sz,
             "%s-[%d]-%s-[%d] desg %s portId %02x-%02x\n",
             root, 
             unal_ntohl(vec->extRootPathCost.bytes),
             regRoot,
             unal_ntohl(vec->intRootPathCost.bytes),
             desg,
             vec->DesignatedPortId.bytes[0], 
             vec->DesignatedPortId.bytes[1]);
    return str;
}

/**
 * pdu_set_times() - Encode designatedTimes into BPDU
 * 
 * \note 802.1Q specify portTimes.HelloTime in favour of
 * designatedTimes.HelloTime.
 */
static void
pdu_set_times(mstp_bpdu_t *pdu, const mstp_port_t *port)
{
    pdu->messageAge[0] = (u8) (port->designatedTimes.messageAge);
    pdu->messageAge[1] = (u8) 0; /* Fraction */
    pdu->maxAge[0] = (u8) (port->designatedTimes.maxAge);
    pdu->maxAge[1] = (u8) 0; /* Fraction */
    pdu->helloTime[0] = (u8) HelloTime(port);
    pdu->helloTime[1] = (u8) 0; /* Fraction */
    pdu->forwardDelay[0] = (u8) (port->designatedTimes.fwdDelay);
    pdu->forwardDelay[1] = (u8) 0; /* Fraction */
}

static void add_path_cost(PathCost_t *pc, u32 cost)
{
    u32 cc = unal_ntohl(pc->bytes) + cost;
    pc->bytes[0] = (u8) (cc >> 24);
    pc->bytes[1] = (u8) (cc >> 16);
    pc->bytes[2] = (u8) (cc >>  8);
    pc->bytes[3] = (u8) (cc      );
}

#if 0
/*lint -e{740} */
static void log_prio(PriorityVector_t const *prio, const char *id, uint port_no, u8 msti)
{
    char root[32], regroot[32];
    (void) vtss_mstp_bridge2str(root, sizeof(root), (u8*) &prio->rootBridgeId);
    (void) vtss_mstp_bridge2str(regroot, sizeof(regroot), (u8*) &prio->regRootBridgeId);
    T_I("%s priority %d[%d]: %s-[%lu]-%s",
        id, 
        port_no, msti, 
        root, 
        unal_ntohl(prio->extRootPathCost.bytes), 
        regroot);
}
#endif

static const mstp_bpdu_mstirec_t *
getMstiRecord(const mstp_cistport_t *cist,
              u16 MSTID)
{
    uint i;
    for(i = 0; i < cist->rcvBpdu.mstp_records; i++) {
        const mstp_bpdu_mstirec_t *pRec = &cist->rcvBpdu.msti_rec[i];
        u16 recMSTID = 
            ((pRec->regional_root_bridge_id.bridgeIdPrio[0] & 0x0F) << 8) +
            pRec->regional_root_bridge_id.bridgeIdPrio[1];
        if(recMSTID == MSTID)
            return pRec;
    }
    return NULL;
}

/** 
 * updateMsgInfo() - record message priority and times for the
 * CIST/MSTI from a received BPDU.
 */
static void
updateMsgInfo(const mstp_cistport_t *cist,
              mstp_port_t *port)
{
    /* Start with CIST info - straight from BPDU (almost) */
    port->msgTimes = cist->rcvBpdu.times;
    if(isCistPort(port)) {
        port->msgPriority = cist->rcvBpdu.priority;
    } else {
        /* CIST BPDU info delta info from MSTI record */
        u16 MSTID = port->tree->MSTID;
        const mstp_bpdu_mstirec_t *pRec = getMstiRecord(cist, MSTID);
        if(pRec) {
            /* Add regional root id */
            port->msgPriority.regRootBridgeId = pRec->regional_root_bridge_id;
            /* Add internal root cost */
            port->msgPriority.intRootPathCost = pRec->internal_root_cost;
            /* Add bridge priority */
            port->msgPriority.DesignatedBridgeId = cist->rcvBpdu.priority.DesignatedBridgeId;
            port->msgPriority.DesignatedBridgeId.bridgeIdPrio[0] = (MSTID >> 8) | (pRec->bridge_priority & 0xF0);
            port->msgPriority.DesignatedBridgeId.bridgeIdPrio[1] = (u8) MSTID;
            /* Add port priority */
            port->msgPriority.DesignatedPortId = cist->rcvBpdu.priority.DesignatedPortId;
            port->msgPriority.DesignatedPortId.bytes[0] &= 0x0F;
            port->msgPriority.DesignatedPortId.bytes[0] |= (pRec->port_priority & 0xF0);
            /* Add remaining hops */
            port->msgTimes.remainingHops = pRec->remaining_hops;
        }
    }
}

static PortRole_t getBpduRole(const mstp_port_t *port)
{
    mstp_cistport_t *cist = port->cistport;
    if(isCistPort(port)) {
        if(cist->rcvBpdu.type == MSTP_BPDU_TYPE_CONFIG) {
            /* 13.26.6 Note: A Configuration BPDU implicitly conveys a
             * Designated Port Role (- for the CIST) */
            return DesignatedPort;
        }
        return cist->rcvBpdu.role;
    } else {
        const mstp_bpdu_mstirec_t *pRec = getMstiRecord(cist, port->tree->MSTID);
        if(pRec)
            return (PortRole_t) ((pRec->flags >> 2) & 0x3);
        else
            return UnknownPort; /* To make rcvInfo return OtherInfo */
    }
}

static u8 getBpduFlags(const mstp_port_t *port)
{
    mstp_cistport_t *cist = port->cistport;
    if(isCistPort(port))
        return cist->rcvBpdu.flags;
    else
        return cist->rcvBpdu.msti_rec[port->msti-1].flags & MSTP_BPDU_FLAG_MASK;
}

/** encode_flags()
 * 
 * Encode a CIST/MSTI BPDU flags for a given port.
 */
static u8
encode_flags(mstp_port_t *port)
{
    u8 flags = role2flags(port->role); /* 14.6 c) */

    /* 14.6 a) */
    if(port->tcWhile != 0)
        flags |= MSTP_BPDU_FLAG_TC;

    /* 14.6 b) */
    if(port->proposing)
        flags |= MSTP_BPDU_FLAG_PROPOSAL;

    /* 14.6 d) */
    if(port->learning)
        flags |= MSTP_BPDU_FLAG_LEARNING;

    /* 14.6 e) */
    if(port->forwarding)
        flags |= MSTP_BPDU_FLAG_FORWARDING;

    /* 14.6 f) */
    if(port->agree)
        flags |= MSTP_BPDU_FLAG_AGREEMENT;

    /* 14.6.1 a) */
    if(!isCistPort(port) && port->master)
        flags |= MSTP_BPDU_FLAG_MASTER;

    /* 14.6 g) Bit 8 of Octet 5 conveys the Topology Change
     * Acknowledge Flag in STP Configuration BPDUs. It is unused in
     * RST and MST BPDUs and shall be transmitted as 0.
     */

    T_N("Encoded flags %d[%d] = 0x%02x", port->port_no, port->msti, flags);

    return flags;
}


/** encode_msti_record()
 * 
 * Encode a MSTI BPDU record for a given tree and port.
 */
static void
encode_msti_record(mstp_tree_t *tree, 
                   mstp_port_t *port, 
                   mstp_bpdu_mstirec_t *rec)
{
    /* 14.6.1 a) */
    rec->flags = encode_flags(port);

    /* 14.6.1 b) */
    rec->regional_root_bridge_id = port->designatedPriority.regRootBridgeId;
    /* - add MSTID - what about the existing MSTID - is that always zero ? */
    rec->regional_root_bridge_id.bridgeIdPrio[0] &= 0xF0;
    rec->regional_root_bridge_id.bridgeIdPrio[0] |= (u8) (tree->MSTID >> 8);
    rec->regional_root_bridge_id.bridgeIdPrio[1] = (u8) tree->MSTID;

    /* 14.6.1 c) */
    rec->internal_root_cost = port->designatedPriority.intRootPathCost;

    /* 14.6.1 d) */
    rec->bridge_priority = 
        port->portPriority.DesignatedBridgeId.bridgeIdPrio[0] & 0xF0;

    /* 14.6.1 e) */
    rec->port_priority = 
        port->portPriority.DesignatedPortId.bytes[0] & 0xF0;

    /* 14.6.1 f) */
    rec->remaining_hops = port->designatedTimes.remainingHops;
}

/** calculateMasterFlag()
 *
 * Helper function for calculating the \e master flag (13.24.5).
 *
 * \return TRUE if the Port Role for the MSTI and Port is Root Port or
 * Designated Port, and the Bridge has selected one of its Ports as
 * the Master Port for this MSTI or the mastered flag is set for this
 * MSTI for any other Bridge Port with a Root Port or Designated Port
 * Role. Return FALSE otherwise.
*/
static bool
calculateMasterFlag(const mstp_tree_t *tree, 
                    const mstp_port_t *msti)
{
    if((msti->role == RootPort || msti->role == DesignatedPort)) {
        const mstp_port_t *port;
        ForAllPorts(tree, port) {
            PortRole_t role = port->role;
            if(role == MasterPort ||
               (msti != port &&
                (role == RootPort || role == DesignatedPort) &&
                port->mastered))
                return TRUE;
        }
    }
    return FALSE;
}

/**
 * priorityIsBetter() - Implement 13.10 priority comparison.
 * 
 * \param prio the priority vector to compare towards (whether it is
 * \e superior compared to the port pririty)
 *
 * \param port the port priority vector to compare against
 *
 * \return \e TRUE if prio is better
 */
static bool priorityIsBetter(PriorityVector_t const *prio,
                             mstp_port_t const *port)
{
    return memcmp(prio, &port->portPriority, sizeof(PriorityVector_t)) < 0;
}

/**
 * priorityIsBetterEq() - Implement 13.10 priority comparison.
 * 
 * \param prio the priority vector to compare towards (whether it is
 * \e superior compared to the port pririty)
 *
 * \param port the port priority vector to compare against
 *
 * \return \e TRUE if prio is better or equal
 */
static bool priorityIsBetterEq(PriorityVector_t const *prio,
                               mstp_port_t const *port)
{
    return memcmp(prio, &port->portPriority, sizeof(PriorityVector_t)) <= 0;
}

/**
 * priorityIsEqual() - Implement 13.10 priority comparison.
 * 
 * \param prio the priority vector to compare towards (whether it is
 * \e equal compared to the port pririty)
 *
 * \param port the port priority vector to compare against
 *
 * \return \e TRUE if prio is equal
 */
static bool priorityIsEqual(PriorityVector_t const *prio,
                            mstp_port_t const *port)
{
    return memcmp(prio, &port->portPriority, sizeof(PriorityVector_t)) == 0;
}

/**
 * priorityIsWorseEq() - Implement 13.10 priority comparison.
 * 
 * \param prio the priority vector to compare towards (whether it is
 * <em>inferior or equal to </em>)
 *
 * \param port the port priority vector to compare against
 *
 * \return \e TRUE if prio is worse or equal
 */
static bool priorityIsWorseEq(PriorityVector_t const *prio,
                                 mstp_port_t const *port)
{
    return memcmp(prio, &port->portPriority, sizeof(PriorityVector_t)) >= 0;
}

/**
 * priorityIsWorse() - Implement 13.10 priority comparison.
 * 
 * \param prio the priority vector to compare towards (whether it is
 * \e inferior compared to the port pririty)
 *
 * \param port the port priority vector to compare against
 *
 * \return \e TRUE if prio is worse
 */
static bool priorityIsWorse(PriorityVector_t const *prio,
                               mstp_port_t const *port)
{
    return memcmp(prio, &port->portPriority, sizeof(PriorityVector_t)) > 0;
}

/**
 * priorityIsSuperior() - Implement 13.10 priority comparison.
 * 
 * \param prio the priority vector to compare towards (whether it is
 * \e superior compared to the port pririty)
 *
 * \param port the port priority vector to compare against
 *
 * \return \e TRUE if prio is superior
 */
static bool priorityIsSuperior(PriorityVector_t const *prio,
                               mstp_port_t const *port)
{
    bool superior = priorityIsBetter(prio, port);
    if(!superior) {
        /* Muy importante - 13.10, the last comparison:
         *
         * || ((D.BridgeAddress == DesignatedBridgeID.BridgeAddress) &&
         *     (PD.PortNumber == DesignatedPortID.PortNumber))
         *
         * Ie: Is it from the former port designated bridge (and same
         * port).
         */
        if(memcmp(prio->DesignatedBridgeId.bridgeAddress, 
                  port->portPriority.DesignatedBridgeId.bridgeAddress, 6) == 0 &&
           memcmp(prio->DesignatedPortId.bytes, 
                  port->portPriority.DesignatedPortId.bytes, sizeof(PortId_t)) == 0) {
            T_D("Inferior priority from designated bridge - is SUPERIOR");
            superior = TRUE;
        }
    }
    return superior;
}

/**
 * send_bpdu() - MAC encapsulate and transmit BPDU on port
 *
 * \param port to tranmit the BPDU on
 *
 * \param pdu BPDU buffer structure pointer
 *
 * \param len number of bytes valid in BPDU buffer
 *
 * \note Transmitting port must fill in source MAC address of port.
 */
static void
send_bpdu(const mstp_port_t *port,
          mstp_bpdu_t *pdu,
          size_t len)
{
    size_t len8023 = len - 14;   /* 802.3 len is excl. MAC hdr */

    memcpy(pdu->dst_mac, ieee_bridge, sizeof(pdu->dst_mac));

    pdu->len8023[0] = (u8) (len8023 >> 8);  /* Hi first */
    pdu->len8023[1] = (u8) (len8023 & 0xFF); /* Then Lo */
    pdu->ssap = pdu->dsap = BPDU_L_SAP;
    pdu->llc = LLC_UI;
    
    vtss_mstp_tx(port->port_no, pdu, len);
}

static u32
TotalXmits(const mstp_cistport_t *cist)
{
    return cist->stat.stp_frame_xmits + 
        cist->stat.tcn_frame_xmits + 
        cist->stat.rstp_frame_xmits + 
        cist->stat.mstp_frame_xmits;
}

/** doBpduFiltering()
 */
static bool
doBpduFiltering(const mstp_cistport_t *cist, const mstp_port_t *port)
{
    if(!portEnabled(cist)) /* This os *non*-standard - but makes sense */
        return TRUE;

    if(bpduFiltering(cist) && 
       TotalXmits(cist) >= 3) {
        T_D("Port %d - filtering BPDU xmit", port->port_no);
        return TRUE;
    }
    return FALSE;
}

/************************************************************************
 * The following procedures perform the functions specified in 17.21
 * of IEEE Std 802.1D for the CIST state machines:
 *
 * a) txTcn()
 *
 */

/* 802.1D-2004 17.21.21 txTcn() */
void
txTcn(mstp_port_t *port)
{
    mstp_cistport_t *cist = port->cistport;
    mstp_bpdu_t pdu;

    VTSS_ASSERT(isCistPort(port));

    if(doBpduFiltering(cist, port))
        return;

    T_D("Port %d", port->port_no);

    memset(&pdu, 0, sizeof(pdu));

    /* Each transmitted Topology Change Notification BPDU shall
     * contain the following parameters and no others. Where a
     * specific parameter value is indicated in this subclause, that
     * parameter value shall be encoded in all transmitted Topology
     * Change Notification BPDUs:
     *
     * a) The Protocol Identifier is encoded in Octets 1 and 2 . It
     * takes the value 0000 0000 0000 0000.
     */
    pdu.protocolIdentifier[0] = pdu.protocolIdentifier[1] = (u8) 0;

    /* b) The Protocol Version Identifier is encoded in Octet 3 . It
     * takes the value 0000 0000.
     */
    pdu.protocolVersionIdentifier = (u8) 0;
    
    /* c) The BPDU Type is encoded in Octet 4 . This field takes the
     * value 1000 0000 (where bit 8 is shown at the left of the
     * sequence). This denotes a Topology Change Notification BPDU.
     */
    pdu.bpduType = MSTP_BPDU_TYPE_TCN;

    port->cistport->stat.tcn_frame_xmits++; /* Count stats */

    /* TCN PDU skips the flags field and forward */
    send_bpdu(port, &pdu, offsetof(mstp_bpdu_t, __tcn_marker));
}

/************************************************************************
 * The following procedures perform the functions specified in 17.21
 * of IEEE Std 802.1D for the CIST or any given MSTI instance:
 *
 * b) disableForwarding()
 * c) disableLearning()
 * d) enableForwarding()
 * e) enableLearning()
 * f) recordPriority()
 */

/** 802.1D-2004 17.21.12 recordPriority() */
void
recordPriority(mstp_port_t *port)
{
    T_D("Port %d[%d]", port->port_no, port->msti);
    /* Sets the components of the portPriority variable to the values
     * of the corresponding msgPriority components. */
    port->portPriority = port->msgPriority;
}

/** 802.1D-2004 17.21.24 updtRoleDisabledTree() */
void
updtRoleDisabledTree(mstp_tree_t *tree)
{
    mstp_port_t *port;
    T_N("Called");
    /* Sets selectedRole to DisabledPort for all Ports of the
     * Bridge. */
    ForAllPorts(tree, port) {
        port->selectedRole = DisabledPort;
    }
}

/** 802.1D-2004 17.20.10 reRooted */
bool
reRooted(const mstp_port_t *iport)
{
    struct mstp_tree *tree = iport->tree;
    mstp_port_t *port;
    T_N("Port %d[%d]", iport->port_no, tree->msti);
    ForAllPorts(tree, port) {
        if(port != iport &&     /* Skip the given port */
           port->rrWhile != 0)
            return FALSE;       /* Non-zero port spoils the lot */
    }
    T_D("**** REROOTED - port %d[%d] ***", iport->port_no, tree->msti);
    return TRUE;
}

/************************************************************************
 * The following procedures perform the general functions described in
 * 17.21 of IEEE Std 802.1D for both the CIST and the MSTI state
 * machines or specifically for the CIST or a given MSTI but have
 * enhanced or extended specifications or considerations:
 *
 * g) betterorsameInfo(newInfoIs) (13.26.1)
 * h) clearReselectTree() (13.26.3)
 * i) newTcWhile() (13.26.5)
 * j) rcvInfo() (13.26.6)
 * k) recordAgreement() (13.26.7)
 * l) recordDispute() (13.26.8)
 * m) recordProposal() (13.26.10)
 * n) recordTimes() (13.26.11)
 * o) setReRootTree(13.26.13)
 * p) setSelectedTree() (13.26.14)
 * q) setSyncTree() (13.26.15)
 * r) setTcFlags() (13.26.16)
 * s) setTcPropTree() (13.26.17)
 * t) txConfig() (13.26.19)
 * u) txMstp() (13.26.20)
 * v) updtBPDUVersion() (13.26.21)
 * w) updtRcvdInfoWhile() (13.26.22)
 * x) updtRolesTree() (13.26.23)
 * y) updtRolesDisabledTree() (13.26.24)
 * 
 */
 
/** 13.26.1 betterorsameInfo() */
bool
betterorsameInfo(const mstp_port_t *port, InfoIs_t newInfoIs)
{
    /* Returns TRUE if, for a given Port and Tree (CIST, or MSTI),
     * either
     *
     * a) The procedure.s parameter newInfoIs is Received, and infoIs
     * is Received and the msgPriority vector is better than or the
     * same as (13.10) the portPriority vector; or,
     *
     * b) The procedure.s parameter newInfoIs is Mine, and infoIs is
     * Mine and the designatedPriority vector is better than or the
     * same as (13.10) the portPriority vector.
     *
     * Returns False otherwise.
     */
    if((newInfoIs == Received &&
        port->infoIs == Received &&
        priorityIsBetterEq(&port->msgPriority, port))
       ||
       (newInfoIs == Mine &&
        port->infoIs == Mine &&
        priorityIsBetterEq(&port->designatedPriority, port)))
        return TRUE;
    return FALSE;
}

/** 13.26.3 clearReselectTree() */
void
clearReselectTree(const struct mstp_tree *tree)
{
    mstp_port_t *port;
    T_N("Called");
    /* Clears reselect for the tree (the CIST or a given MSTI) for all
     * Ports of the Bridge. 
     */
    ForAllPorts(tree, port) {
        port->reselect = FALSE;
    }
}

/** 13.26.5 newTcWhile() */
void
newTcWhile(mstp_port_t *port)
{
    T_N("Port %d[%d]", port->port_no, port->msti);
    /* If the value of tcWhile is zero and sendRSTP is TRUE, this
     * procedure sets the value of tcWhile to HelloTime plus one
     * second and sets either newInfo TRUE for the CIST or newInfoMsti
     * TRUE for a given MSTI. The value of HelloTime is taken from the
     * CIST.s portTimes parameter (13.24.13) for this Port.
     *
     * If the value of tcWhile is zero and sendRSTP is FALSE, this
     * procedure sets the value of tcWhile to the sum of the Max Age
     * and Forward Delay components of rootTimes and does not change
     * the value of either newInfoCist or newInfoMsti.
     *
     * Otherwise the procedure takes no action.
     */
    if(port->tcWhile == 0) {
        if(port->cistport->sendRSTP) {
            port->tcWhile = HelloTime(port) + 1; /* Round up */
            newInfoXst(port, TRUE);
        } else {
            mstp_tree_t *bridge = port->tree;
            port->tcWhile = bridge->rootTimes.maxAge + bridge->rootTimes.fwdDelay;
        }
    }
}

/** 13.26.6 rcvInfo() 
 *
 * Decodes received BPDUs. 
 */
RcvdInfo_t
rcvInfo(mstp_port_t *port)
{
    char str[132];
    mstp_cistport_t *cist = port->cistport;
    RcvdInfo_t info;
    PortRole_t role;
    
    /* 
     * Sets rcvdTcn and sets rcvdTc for each and every MSTI if a TCN
     * BPDU has been received,
     *
     * NB: The *function* as a whole is called for 'each and every
     * MSTI', so the wording is misleading...!
     */
    if(cist->rcvBpdu.type == MSTP_BPDU_TYPE_TCN) {
        /* This is from 802.1Q (MSTP) */
        if(isCistPort(port))
            /* NB: Only do this once - for the CIST */
            cist->rcvdTcn = TRUE;
        port->rcvdTc = TRUE;
        role = UnknownPort;
    } else {

        /* ... and extracts the message priority and timer values from the
         * received BPDU storing them in the msgPriority and msgTimes
         * variables.
         */
        updateMsgInfo(cist, port);

        /* Extract conveyed port role */
        role = getBpduRole(port);
    }


    /* Returns SuperiorDesignatedInfo if, for a given Port and Tree
     * (CIST, or MSTI):
     *
     * a) The received CIST or MSTI message conveys a Designated Port
     * Role, and
     *
     * 1) The message priority (msgPriority.13.24.9) is superior
     * (13.10 or 13.11) to the Port.s port priority vector, or
     *
     * 2) The message priority is the same as the Port.s port priority
     * vector, and any of the received timer parameter values
     * (msgTimes.13.24.10) differ from those already held for the Port
     * (portTimes.13.24.13).
     */
    if(role == DesignatedPort &&
       (priorityIsSuperior(&port->msgPriority, port) ||
        (priorityIsEqual(&port->msgPriority, port) &&
         struct_cmp(port->msgTimes, !=, port->portTimes)))) /* XXX struct_cmp of times CIS/MSTI ??? */
        info = SuperiorDesignatedInfo;

    /* Otherwise, returns RepeatedDesignatedInfo if, for a given Port
     * and Tree (CIST, or MSTI):
     *
     * b) The received CIST or MSTI message conveys a Designated Port
     * Role, and
     *
     * 1) A message priority vector and timer parameters that are the
     * same as the Port.s port priority vector and timer values; and
     *
     * 2) infoIs is Received.
     */
    else if(role == DesignatedPort &&
            priorityIsEqual(&port->msgPriority, port) && 
            struct_cmp(port->msgTimes, ==, port->portTimes) && /* XXX compare for CIST/MSTI ??? */
            port->infoIs == Received)
        info = RepeatedDesignatedInfo;

    /* Otherwise, returns InferiorDesignatedInfo if, for a given Port
     * and Tree (CIST, or MSTI):
     *
     * c) The received message conveys a Designated Port Role, and a
     * message priority vector that is worse than the Port.s port
     * priority vector.
     */
    else if(role == DesignatedPort &&
            priorityIsWorse(&port->msgPriority, port))
            info = InferiorDesignatedInfo;

    /* Otherwise, returns InferiorRootAlternateInfo if, for a given
     * Port and Tree (CIST, or MSTI):
     *
     * d) The received message conveys a Root Port, Alternate Port, or
     * Backup Port Role and a message priority that is the same as or
     * worse than the port priority vector.
     */
    else if((role == RootPort || 
             role == AltBackupPort) &&
            priorityIsWorseEq(&port->msgPriority, port))
        info = InferiorRootAlternateInfo; 

    /* Otherwise, returns OtherInfo. */
    else
        info = OtherInfo;

    T_N("Port %d[%d] - role %s, return %s", port->port_no, port->msti, 
        rstp_portrole2str(role), rstp_rcvinfo2str(info));
    T_N("Port %d[%d] - portPrio %s", port->port_no, port->msti,
        vector2s(str, sizeof(str), &port->portPriority));
    T_N("Port %d[%d] - msgPrio  %s", port->port_no, port->msti,
        vector2s(str, sizeof(str), &port->msgPriority));

    return info;
}

/** sameRootAsCist() - a recordAgreement() helper
 *
 * \return TRUE iff
 *
 * The message priority vector of the CIST Message accompanying the
 * received MSTI Message (i.e., received in the same BPDU) has the
 * same CIST Root Identifier, CIST External Root Path Cost, and
 * Regional Root Identifier as the CIST port priority vector
 *
 * FALSE otherwise.
 */
static bool
sameRootAsCist(const mstp_port_t *port)
{
    bool ret = FALSE;
    mstp_port_t *cport = get_port(port->cistport->bridge, port->port_no);

    VTSS_ASSERT(cport != NULL);

    const PriorityVector_t 
        *msgPrio = &port->cistport->rcvBpdu.priority,
        *prtPrio = &cport->portPriority;

    //log_prio(msgPrio, "CIST rcvPrio", port->port_no, port->msti);
    //log_prio(prtPrio, "CIST prtPrio", port->port_no, port->msti);

    if(struct_cmp(msgPrio->rootBridgeId, ==, prtPrio->rootBridgeId) &&
       struct_cmp(msgPrio->extRootPathCost, ==, prtPrio->extRootPathCost) &&
       struct_cmp(msgPrio->regRootBridgeId, ==, prtPrio->regRootBridgeId))
        ret = TRUE;

    T_D("Port %d[%d] - return %d", port->port_no, port->msti, ret);

    return ret;
}

/** 13.26.7 recordAgreement() */
void
recordAgreement(mstp_port_t *port)
{
    mstp_tree_t *tree = port->tree;
    mstp_cistport_t *cist = port->cistport;
    T_D("Port %d[%d]", port->port_no, port->msti);
    if(isCistPort(port)) {
        /*
         * For the CIST and a given Port, if rstpVersion is TRUE,
         * operPointToPointMAC (6.4.3) is TRUE, and the received CIST
         * Message has the Agreement flag set, ...
         */
        if(rstpVersion(tree->bridge) && 
           cist->operPointToPointMAC && 
           (getBpduFlags(port) & MSTP_BPDU_FLAG_AGREEMENT)) {
            /* the CIST agreed flag is set, and the CIST proposing
             * flag is cleared.
             */
            port->agreed = TRUE;
            port->proposing = FALSE;
        } else {
            /* Otherwise the CIST agreed flag is cleared. */
            port->agreed = FALSE;
        }
        /* Additionally, if the CIST message was received from a
         * Bridge in a different MST Region, i.e., the rcvdInternal
         * flag is clear, the agreed and proposing flags for this Port
         * for all MSTIs are set or cleared to the same value as the
         * CIST agreed and proposing flags. If the CIST message was
         * received from a Bridge in the same MST Region, the MSTI
         * agreed and proposing flags are not changed.
         */
        if(!cist->rcvdInternal) {
            ForAllPortNo(cist->bridge, port->port_no, { 
                    _tp_->agreed = port->agreed;
                    _tp_->proposing = port->proposing;
                });
        }
    } else {
        /* For a given MSTI and Port, if operPointToPointMAC (6.4.3)
         * is TRUE, and
         *
         * a) The message priority vector of the CIST Message
         * accompanying the received MSTI Message (i.e., received in
         * the same BPDU) has the same CIST Root Identifier, CIST
         * External Root Path Cost, and Regional Root Identifier as
         * the CIST port priority vector, and
         *
         * b) The received MSTI Message has the Agreement flag set,
         *
         * the MSTI agreed flag is set and the MSTI proposing flag is
         * cleared. Otherwise the MSTI agreed flag is cleared.
         */
        if(cist->operPointToPointMAC &&
           sameRootAsCist(port) && /* (a) */
           (getBpduFlags(port) & MSTP_BPDU_FLAG_AGREEMENT)) { /* (b) */
            port->agreed = TRUE;
            port->proposing = FALSE;
        } else {
            port->agreed = FALSE;
        }
    }
}

/** 13.26.8 recordDispute() */
void
recordDispute(mstp_port_t *port)
{
    char str[132];
    u8 flags = getBpduFlags(port);
    T_I("Port %d[%d] - learning %s", port->port_no, port->msti, 
        (flags & MSTP_BPDU_FLAG_LEARNING) ? "ON" : "OFF");
    T_I("Port %d[%d] - Disputed:Port %s", port->port_no, port->msti, 
        vector2s(str, sizeof(str), &port->portPriority));
    T_I("Port %d[%d] - Disputed:Mesg %s", port->port_no, port->msti,
        vector2s(str, sizeof(str), &port->msgPriority));
    if(isCistPort(port)) {
        /* For the CIST and a given port, if the CIST message has the
         * learning flag set:
         * 
         * a) The disputed variable is set; and
         * b) The agreed variable is cleared.
         */
        if((flags & MSTP_BPDU_FLAG_LEARNING)) {
            port->disputed = TRUE;
            port->agreed = FALSE;
        }
        /* Additionally, if the CIST message was received from a
         * Bridge in a different MST region (i.e., if the rcvdInternal
         * flag is clear), then for all the MSTIs:
         * 
         * c) The disputed variable is set; and
         * d) The agreed variable is cleared.
         */
        mstp_cistport_t *cist = port->cistport;
        if(!cist->rcvdInternal) {
            ForAllPortNo(cist->bridge, port->port_no, { 
                    _tp_->disputed = TRUE;
                    _tp_->agreed = FALSE;
                });
        }
    } else {
        /* For a given MSTI and port, if the received MSTI message has
         * the learning flag set:
         *
         * e) The disputed variable is set; and
         * f) The agreed variable is cleared.
         */
        if((flags & MSTP_BPDU_FLAG_LEARNING)) {
            port->disputed = TRUE;
            port->agreed = FALSE;
        }
    }
}

/** 13.26.9 recordMastered() 
 *
 * \note master and mastered signal the connection of the MSTI to the
 * CST via the Master Port throughout the MSTI.  These variables and
 * their supporting procedures do not affect the connectivity provided
 * by this revision of this standard but permit future enhancements to
 * MSTP providing increased flexibility in the choice of Master Port
 * without abandoning plug-and-play network migration. They are,
 * therefore, omitted from the overviews of protocol operation,
 * including Figure 13-9.
 */
void recordMastered(mstp_port_t *port)
{
    mstp_cistport_t *cist = port->cistport;
    if(isCistPort(port)) {
        /* For the CIST and a given Port, if the CIST message was
         * received from a Bridge in a different MST Region, i.e. the
         * rcvdInternal flag is clear, the mastered variable for this
         * Port is cleared for all MSTIs.
         */
        if(!cist->rcvdInternal) {
            ForAllPortNo(cist->bridge, port->port_no, { 
                    _tp_->mastered = FALSE;
                });
        }
    } else {
        /* For a given MSTI and Port, if the MSTI message was received
         * on a point-to-point link and the MSTI Message has the
         * Master flag set, set the mastered variable for this
         * MSTI. Otherwise reset the mastered variable.
         */
        port->mastered = 
            (cist->operPointToPointMAC &&
             (getBpduFlags(port) & MSTP_BPDU_FLAG_MASTER));
    }
}

/** 13.26.10 recordProposal() */
void recordProposal(mstp_port_t *port)
{
    u8 flags = getBpduFlags(port);
    PortRole_t role = getBpduRole(port);
    T_D("Port %d[%d]", port->port_no, port->msti);
    if(isCistPort(port)) {
        mstp_cistport_t *cist = port->cistport;
        /* For the CIST and a given Port, if the received CIST Message
         * conveys a Designated Port Role, and has the Proposal flag
         * set, the CIST proposed flag is set. Otherwise the CIST
         * proposed flag is not changed.
         */
        if(role == DesignatedPort && 
           (flags & MSTP_BPDU_FLAG_PROPOSAL))
            port->proposed = TRUE;
        /* Additionally, if the CIST Message was received from a
         * Bridge in a different MST Region, i.e., the
         * rcvdInternal flag is clear, the proposed flags for this
         * Port for all MSTIs are set or cleared to the same value
         * as the CIST proposed flag. If the CIST message was
         * received from a Bridge in the same MST Region, the MSTI
         * proposed flags are not changed.
         */
        if(!cist->rcvdInternal) {
            ForAllPortNo(cist->bridge, port->port_no, { 
                    _tp_->proposed = port->proposed; 
                });
        }
    } else {
        /* For a given MSTI and Port, if the received MSTI Message conveys
         * a Designated Port Role, and has the Proposal flag set, the MSTI
         * proposed flag is set. Otherwise the MSTI proposed flag is not
         * changed.
         */
        if(role == DesignatedPort && 
           (flags & MSTP_BPDU_FLAG_PROPOSAL))
            port->proposed = TRUE;
    }
}

/** 13.26.11 recordTimes() */
void
recordTimes(mstp_port_t *port)
{
    T_D("Port %d[%d]", port->port_no, port->msti);
    /* For the CIST and a given Port, sets portTimes. Message Age, Max
     * Age, Forward Delay, and remainingHops to the received values
     * held in msgTimes ...
     */
    port->portTimes = port->msgTimes;
    /* ... and portTimes.HelloTime to msgTimes.HelloTime if that is
     * greater than the minimum specified in the Compatibility Range
     * column of Table 17-1 of IEEE Std 802.1D, and to that minimum
     * otherwise.
     */
    // Note: As per UNH, msgTimes.helloTime does not seem to be needed.
    //if(port->msgTimes.helloTime < MSTP_HELLOTIME_MINIMUM_COMPAT)
    //port->portTimes.helloTime = MSTP_HELLOTIME_MINIMUM_COMPAT;
}

/** 13.26.13 setReRootTree() */
void
setReRootTree(const mstp_tree_t *tree)
{
    mstp_port_t *port;
    T_I("Called");
    /*
     * Sets reRoot TRUE for this tree (the CIST or a given MSTI) for
     * all Ports of the Bridge.
     */
    ForAllPorts(tree, port) {
        port->reRoot = TRUE;
    }
}

/** 13.26.14 setSelectedTree() */
void
setSelectedTree(const mstp_tree_t *tree)
{
    mstp_port_t *port;
    T_N("Called");
    /* If reselect is TRUE for any Port, this procedure takes no
     * action.
     */
    if(anyReSelect(tree))
        return;
    T_D("SetSelect MST%d", tree->msti);
    /* Sets selected TRUE for this tree (the CIST or a given MSTI) for
     * all Ports of the Bridge if reselect is FALSE for all Ports in
     * this tree.
     */
    ForAllPorts(tree, port) {
        port->selected = TRUE;
    }
}

/** 13.26.15 setSyncTree() */
void
setSyncTree(const mstp_tree_t *tree)
{
    mstp_port_t *port;
    T_D("Called");
    /* Sets sync TRUE for this tree (the CIST or a given MSTI) for all
     * Ports of the Bridge. 
     */
    ForAllPorts(tree, port) {
        port->sync = TRUE;
    }
}

/** 13.26.16 setTcFlags() */
void
setTcFlags(mstp_port_t *port)
{
    u8 flags = getBpduFlags(port);
    T_D("Port %d[%d] - TC %d Flags 0x%02x", port->port_no, port->msti,
        !!(flags & MSTP_BPDU_FLAG_TC), flags);
    if(isCistPort(port)) {
        mstp_cistport_t *cist = port->cistport;
        /* 
         * For the CIST and a given Port:
         * 
         * a) If the Topology Change Acknowledgment flag is set for
         * the CIST in the received BPDU, sets rcvdTcAck TRUE.
         */
        if(flags & MSTP_BPDU_FLAG_TC_ACK)
            cist->rcvdTcAck = TRUE;
        if(!cist->rcvdInternal) {
            /* b) If rcvdInternal is clear and the Topology Change flag is
             * set for the CIST in the received BPDU, sets rcvdTc TRUE for
             * the CIST and for each and every MSTI.
             */
            if(flags & MSTP_BPDU_FLAG_TC) {
                ForAllPortNo(cist->bridge, port->port_no, { 
                        _tp_->rcvdTc = TRUE; 
                    });
            }
        } else {
            /* c) If rcvdInternal is set, sets rcvdTc for the CIST if the
             * Topology Change flag is set for the CIST in the received
             * BPDU.
             */
            if(flags & MSTP_BPDU_FLAG_TC)
                port->rcvdTc = TRUE;
        }
    } else {
        /* For a given MSTI and Port, sets rcvdTc for this MSTI if the
         * Topology Change flag is set in the corresponding MSTI
         * message.
         */
        if(flags & MSTP_BPDU_FLAG_TC)
            port->rcvdTc = TRUE;
    }
}

/* 13.26.17 setTcPropTree() */
void
setTcPropTree(const mstp_port_t *iport)
{
    T_D("Port %d[%d] restrictedTcn %d", iport->port_no, iport->msti, 
        iport->cistport->restrictedTcn);
    /* If and only if restrictedTcn is FALSE for the Port that invoked
     * the procedure, sets tcProp TRUE for the given tree (the CIST or
     * a given MSTI) for all other Ports.
     */
    if(!iport->cistport->restrictedTcn) {
        mstp_tree_t *tree = iport->tree;
        mstp_port_t *port;
        ForAllPorts(tree, port) {
            if(iport != port)
                port->tcProp = TRUE;
        }
    }
}

/** 13.26.18 syncMaster()
 *
 * For all MSTIs, for each Port that has infoInternal set:
 *
 * a) Clears the agree, agreed, and synced variables; and
 *
 * b) Sets the sync variable.
 */
static void
syncMaster(mstp_bridge_t *mstp)
{
    T_I("Initiating sync");
    mstp_port_t *port;
    ForAllCistPorts(mstp, port) {
        mstp_cistport_t *cist = getCist(port);
        if(cist->infoInternal) {
            ForAllPortNo(mstp, port->port_no, { 
                    _tp_->agree = _tp_->agreed = _tp_->synced = FALSE;
                    _tp_->sync = TRUE;
                });
        }
    }
}

/** 13.26.19 txConfig()
 *
 *  Transmits a Configuration BPDU. The first four components of the
 *  message priority vector (13.24.9) conveyed in the BPDU are set to
 *  the value of the CIST Root Identifier, External Root Path Cost,
 *  Bridge Identifier, and Port Identifier components of the CIST.s
 *  designatedPriority parameter (13.24.2) for this Port.  The
 *  topology change flag is set if (tcWhile != 0) for the Port. The
 *  topology change acknowledgment flag is set to the value of TcAck
 *  for the Port. The remaining flags are set to zero. The value of
 *  the Message Age, Max Age, and Fwd Delay parameters conveyed in the
 *  BPDU are set to the values held in the CIST.s designatedTimes
 *  parameter (13.24.3) for the Port. The value of the Hello Time
 *  parameter conveyed in the BPDU is set to the value held in the
 *  CIST.s portTimes parameter (13.24.13) for the Port.
 */
void
txConfig(mstp_port_t *port)
{
    mstp_cistport_t *cist = port->cistport;
    mstp_bpdu_t pdu;

    VTSS_ASSERT(isCistPort(port));

    if(doBpduFiltering(cist, port))
        return;

    memset(&pdu, 0, sizeof(pdu));

    T_N("Port %d", port->port_no);

    /* a) The Protocol Identifier is encoded in Octets 1 and 2. It
     * takes the value 0000 0000 0000 0000, which identifies the Rapid
     * Spanning Tree Protocol as specified in Clause 17.
     *
     * NOTE: This value of the Protocol Identifier also identifies the
     * Spanning Tree Algorithm and Protocol specified in previous
     * editions of this standard.
     */
    pdu.protocolIdentifier[0] = pdu.protocolIdentifier[1] = (u8) 0;

    /* b) The Protocol Version Identifier is encoded in Octet 3. It
     * takes the value 0000 0000. */
    pdu.protocolVersionIdentifier = (u8) 0;

    /* c) The BPDU Type is encoded in Octet 4. This field takes the
     * value 0000 0000. This denotes a Configuration BPDU. */
    pdu.bpduType = MSTP_BPDU_TYPE_CONFIG;

    /* f) The remaining flags, Bits 2 through 7 of Octet 5, are unused
     * and take the value 0. */
    pdu.flags = (u8) 0;

    /* d) The Topology Change Acknowledgment flag is encoded in Bit 8
     * of Octet 5. */
    if(cist->tcAck)
        pdu.flags |= MSTP_BPDU_FLAG_TC_ACK;

    /* e) The Topology Change flag is encoded in Bit 1 of Octet 5. */
    if(port->tcWhile != 0)
        pdu.flags |= MSTP_BPDU_FLAG_TC;

    /* g) The Root Identifier is encoded in Octets 6 through 13.
     * h) The Root Path Cost is encoded in Octets 14 through 17.
     * i) The Bridge Identifier is encoded in Octets 18 through 25.
     * j) The Port Identifier is encoded in Octets 26 and 27. 
     */
    pdu.rootBridgeId = port->designatedPriority.rootBridgeId;
    pdu.rootPathCost = port->designatedPriority.extRootPathCost;
    pdu.DesignatedBridgeId = port->designatedPriority.DesignatedBridgeId;
    pdu.DesignatedPortId = port->designatedPriority.DesignatedPortId;

    /* k) The Message Age timer value is encoded in Octets 28 and 29.
     * l) The Max Age timer value is encoded in Octets 30 and 31.
     * m) The Hello Time timer value is encoded in Octets 32 and 33. 
     * n) The Forward Delay timer value is encoded in Octets 34 and 35. 
     */
    pdu_set_times(&pdu, port);

    /* The Message Age (Octets 28 and 29) shall be less than Max Age
     * (Octets 30 and 31). (???)
     */

    cist->stat.stp_frame_xmits++; /* Count stats */

    /* STP Config PDU skips the version1Length field */
    send_bpdu(port, &pdu, offsetof(mstp_bpdu_t, __config_marker));
}

/** 13.26.20 txMstp() 
 * 
 * (was txRstp in 802.1D)
 * 
 * Transmits a MST BPDU (14.3.3), encoded according to the
 * specification contained in 14.6. The first six components of the
 * CIST message priority vector (13.24.9) conveyed in the BPDU are set
 * to the value of the CIST.s designatedPriority parameter (13.24.2)
 * for this Port. The Port Role in the BPDU (14.2.1) is set to the
 * current value of the role variable for the transmitting port
 * (13.24.15). The Agreement and Proposal flags in the BPDU are set to
 * the values of the agree (13.24 of this standard, 17.19 of IEEE Std
 * 802.1D) and proposing (13.24 of this standard, 17.19.24 of IEEE Std
 * 802.1D) variables for the transmitting Port, respectively. The CIST
 * topology change flag is set if (tcWhile != 0) for the Port. The
 * topology change acknowledge flag in the BPDU is never used and is
 * set to zero. The learning and forwarding flags in the BPDU are set
 * to the values of the learning (13.24 of this standard, 17.19.12 of
 * IEEE Std 802.1D) and forwarding (13.24 of this standard, 17.19.9 of
 * IEEE Std 802.1D) variables for the CIST, respectively. The value of
 * the Message Age, Max Age, and Fwd Delay parameters conveyed in the
 * BPDU are set to the values held in the CIST.s designatedTimes
 * parameter (13.24.3) for the Port. The value of the Hello Time
 * parameter conveyed in the BPDU is set to the value held in the
 * CIST.s portTimes parameter (13.24.13) for the Port.
 */
void
txMstp(mstp_port_t *port)
{
    mstp_cistport_t *cist = port->cistport;
    mstp_bridge_t *mstp = cist->bridge;
    mstp_bpdu_t pdu;

    VTSS_ASSERT(isCistPort(port));

    if(doBpduFiltering(cist, port))
        return;

    T_N("Port %d", port->port_no);

    memset(&pdu, 0, sizeof(pdu));

    /* 14.5 */
    pdu.protocolIdentifier[0] = pdu.protocolIdentifier[1] = (u8) 0;

    /* 14.5 d) */
    pdu.protocolVersionIdentifier = ForceProtocolVersion(mstp);

    /* 14.5 d) */
    pdu.bpduType = MSTP_BPDU_TYPE_RSTP;

    /* 14.6 a-g) */
    pdu.flags = encode_flags(port);

    /* 14.6 h,i,j,k) */
    pdu.rootBridgeId = port->designatedPriority.rootBridgeId;
    pdu.rootPathCost = port->designatedPriority.extRootPathCost;
    pdu.DesignatedBridgeId = port->designatedPriority.regRootBridgeId; /* NB! */
    pdu.DesignatedPortId = port->portId; /* NB! */

    /* 14.6 l,m,n,o) */
    pdu_set_times(&pdu, port);

    /* 14.6 p) Octet 36 conveys the Version 1 Length. This shall be
     * transmitted as 0. It is checked on receipt by the validation
     * procedure (14.4).
     */
    pdu.version1Length = (u8) 0;

    size_t pdulen;
    if(pdu.protocolVersionIdentifier == MSTP_PROTOCOL_VERSION_MSTP) {

        /* 14.6 r) + 13.26.20 b) */
        pdu.mstp.cfgid = mstp->cfgid;

        /* 14.6 s) + 13.26.20 c) */
        pdu.mstp.cistinfo.intRootPathCost = port->designatedPriority.intRootPathCost;
        
        /* 14.6 t) + 13.26.20 d) */
        pdu.mstp.cistinfo.cistBridgeId = port->designatedPriority.DesignatedBridgeId;

        /* 14.6 u) + 13.26.20 e) + 13.24.3 */
        pdu.mstp.cistinfo.cist_remaining_hops = port->designatedTimes.remainingHops;

        /* 14.6 v) + 13.26.20 f) */
        uint nrecords = 0;
        ForAllMstiTrees(mstp, tree, {
                mstp_port_t *msti = get_tport(tree, port->port_no);
                if(msti) {
                    msti->master = /* 13.24.5 master */
                        calculateMasterFlag(tree, port);
                    encode_msti_record(tree, msti, &pdu.mstpcfg[nrecords++]);
                }
            });

        uint dynparts = nrecords * sizeof(mstp_bpdu_mstirec_t);
        uint v3len = /* Static parts */ sizeof(pdu.mstp) + dynparts;

        /* 14.6 q) + 13.26.20 a) */
        pdu.version3Length[0] = (u8) (v3len >> 8);
        pdu.version3Length[1] = (u8) v3len;

        pdulen = offsetof(mstp_bpdu_t, __mstp_marker) + dynparts;
        cist->stat.mstp_frame_xmits++; /* Count stats */
    } else {
        pdulen = offsetof(mstp_bpdu_t, __rstp_marker);
        cist->stat.rstp_frame_xmits++; /* Count stats */
    }
    send_bpdu(port, &pdu, pdulen);
}

/** 13.26.21 updtBPDUVersion() */
void
updtBPDUVersion(mstp_port_t *port)
{
    T_D("Port %d", port->port_no);
    mstp_cistport_t *cist = port->cistport;
    /* Sets rcvdSTP TRUE if the BPDU received is a version 0 or
     * version 1 TCN or a Config BPDU. Sets rcvdRSTP TRUE if the
     * received BPDU is a RST BPDU or a MST BPDU.
    */
    if(((cist->rcvBpdu.version == 0 || cist->rcvBpdu.version == 1) && 
        cist->rcvBpdu.type == MSTP_BPDU_TYPE_TCN) ||
       cist->rcvBpdu.type == MSTP_BPDU_TYPE_CONFIG)
        cist->rcvdSTP = TRUE;
    if(cist->rcvBpdu.type == MSTP_BPDU_TYPE_RSTP) /* MST BPDU has also RST type */
        cist->rcvdRSTP = TRUE;
}

/** 13.26.22 updtRcvdInfoWhile() */
void
updtRcvdInfoWhile(mstp_port_t *port)
{
    T_D("Port %d", port->port_no);
    /* Updates rcvdInfoWhile (13.21). The value assigned to
     * rcvdInfoWhile is three times the Hello Time, if either:
     *
     * a) Message Age, incremented by 1 second and rounded to the
     * nearest whole second, does not exceed Max Age and the
     * information was received from a Bridge external to the MST
     * Region (rcvdInternal FALSE);
     *
     * or
     *
     * b) remainingHops, decremented by one, is greater than zero and
     * the information was received from a Bridge internal to the MST
     * Region (rcvdInternal TRUE); 
     *
     * and is zero otherwise.  
     *
     * The values of Message Age, Max Age, remainingHops, and Hello
     * Time used in these calculations are taken from the CIST's
     * portTimes parameter (13.24.13) and are not changed by this
     * procedure.
     */
    if((!port->cistport->rcvdInternal && 
        /* external */ (port->portTimes.messageAge + 1 <= port->portTimes.maxAge)) ||
       (port->cistport->rcvdInternal && 
        /* internal */ (port->portTimes.remainingHops > 1)))
        port->rcvdInfoWhile = (timer) (3 * HelloTime(port));
    else
        port->rcvdInfoWhile = 0;
}

/* 13.26.23 updtRolesTree() 
 *
 * This procedure calculates the following Spanning Tree priority
 * vectors (13.9, 13.10 for the CIST, 13.11 for a MSTI) and timer
 * values, for the CIST or a given MSTI:
 *
 */
void
updtRolesTree(mstp_tree_t *tree)
{
    PriorityVector_t rootPriority = tree->BridgePriority; /* Default to myself */
    PortId_t rootPortId;
    mstp_port_t *port, *root = NULL;

    memset(&rootPortId, 0, sizeof(rootPortId));

    ForAllPorts(tree, port) {
        mstp_cistport_t *cist = port->cistport;
        if(/* a) The root path priority vector for each Bridge Port
            * that is not Disabled */
            portEnabled(cist) &&
            /*.. and has a port priority vector (portPriority plus
            * portId - see 13.24.12 and 13.24.11) that has been
            * recorded from a received message and not aged out
            * (infoIs == Received); and
            */
            port->infoIs == Received &&
            /* b1) DesignatedBridgeID Bridge Address component is not
             * equal to that component of the Bridge's own bridge
             * priority vector (13.10) and,
             */
            struct_cmp(port->portPriority.DesignatedBridgeId, !=, 
                       tree->BridgeIdentifier) &&
            /* b2) Port's restrictedRole parameter is FALSE; */
            !cist->restrictedRole) {
            /* b) The Bridge's root priority vector (rootPortId,
             * rootPriority - 13.23.5, 13.23.6), chosen as the best of
             * the set of priority vectors comprising the Bridge's own
             * bridge priority vector (BridgePriority - 13.23.3) plus
             * all calculated root path priority vectors whose:
             */
            PriorityVector_t portPriority = port->portPriority;
            /* What if this port was the root port ? */
            if(isCistPort(port) && !cist->rcvdInternal) {
                /*
                 * Boundary port to other region, bump external cost
                 */
                add_path_cost(&portPriority.extRootPathCost, port->portPathCost);
                T_D("External port port %d, external cost %d", port->port_no, port->portPathCost);
                /*
                 * Declare myself as the regional root. This
                 * priority vector will then "battle" it out with
                 * other vectors linking to the same or another
                 * external root - choosing both the best external
                 * and regional root.
                 */
                portPriority.regRootBridgeId = tree->BridgeIdentifier;
            } else {
                /*
                 * Inside a region, just bump internal cost
                 */
                add_path_cost(&portPriority.intRootPathCost, port->portPathCost);
                T_D("Internal port port %d, internal cost %d", port->port_no, port->portPathCost);
            }
            if(struct_cmp(portPriority, <, rootPriority) || /* Better ? */
               (struct_cmp(portPriority, ==, rootPriority) && /* - or same */
                struct_cmp(port->portId, <, rootPortId))) { /* - and better port */
                rootPriority = portPriority;
                root = port;    /* This is now chosen as root port */
                rootPortId = root->portId; /* Update the better portid */
                T_D("Port port %d has best prio so far", port->port_no);
            }
        } else {
            T_D("Skipped port %d", port->port_no);
        }
    }

    /* If the root priority vector for the CIST is recalculated, and
     * has a different Regional Root Identifier than that previously
     * selected, and has or had a non-zero CIST External Root Path
     * Cost, the syncMaster() procedure (13.26.18) is invoked.
     */
    if(tree->msti == MSTID_CIST &&
       struct_cmp(tree->rootPriority.regRootBridgeId, !=, 
                  rootPriority.regRootBridgeId) &&
       (unal_ntohl(tree->rootPriority.extRootPathCost.bytes) != 0 || 
        unal_ntohl(rootPriority.extRootPathCost.bytes) != 0))
        syncMaster(tree->bridge);

    /* Trap support - Did we select a new root ? */
    if(struct_cmp(rootPriority, !=, tree->rootPriority)) {
        //log_prio(&rootPriority, "newRoot", root ? root->port_no : -1, tree->msti);
        //log_prio(&tree->rootPriority, "oldRoot", -1, tree->msti);
        T_I("MSTI%d: New root on port %d", tree->msti, root ? root->port_no : -1);
        vtss_mstp_trap(tree->msti, MSTP_TRAP_NEW_ROOT);
    }

    /* b) - final parts */
    tree->rootPriority = rootPriority;
    tree->rootPortId = rootPortId;

    /* 
     * c) The Bridge's "root times", (rootTimes - 13.23.7), set equal
     * to:
     */
    if(root == NULL) {
        /* 1) BridgeTimes (13.23.4), if the chosen root priority
         * vector is the bridge priority vector; otherwise,
         */
        tree->rootTimes = tree->bridge->BridgeTimes;
        T_D("MST%d **** I'm root! ****", tree->msti);
    } else {
        /* 2) portTimes (13.24.13) for the port associated with the
         * selected root priority vector, with the Message Age
         * component incremented by 1 second and rounded to the
         * nearest whole second if the information was received from a
         * Bridge external to the MST Region (rcvdInternal FALSE), and
         * with remainingHops decremented by one if the information
         * was received from a Bridge internal to the MST Region
         * (rcvdInternal TRUE).
         */
        tree->rootTimes = root->portTimes;
        if(!root->cistport->rcvdInternal)
            tree->rootTimes.messageAge += 1;
        else
            tree->rootTimes.remainingHops--;
        T_D("MST%d **** Chose port %d as root! ****", tree->msti, root->port_no);
    }

    /* (For each port) */
    ForAllPorts(tree, port) {

        /* d) The designated priority vector
         * (designatedPriority - 13.24.2) for each port
         */

        /* 13.10+13.11 ... CIST/MST designatedPriority
         *
         * The designated priority vector for a port Q on Bridge B is
         * the root priority vector with B's Bridge Identifier B
         * substituted for the DesignatedBridgeID and Q.s Port
         * Identifier QB substituted for the DesignatedPortID and
         * RcvPortID components.
         */
        port->designatedPriority = rootPriority;
        port->designatedPriority.DesignatedBridgeId = tree->BridgeIdentifier;
        port->designatedPriority.DesignatedPortId = port->portId;

        /* 13.10 ... CIST designatedPriority
         *
         * ... If Q is attached to a LAN that has one or more STP
         * Bridges attached (as determined by the Port Protocol
         * Migration state machine), B.s Bridge Identifier B is also
         * substituted for the the RRootID component.
         */
        if(tree->msti == MSTID_CIST) {
            if(!port->cistport->sendRSTP)
                port->designatedPriority.regRootBridgeId = 
                    tree->BridgeIdentifier;
        }

        /* The designated times (designatedTimes - 13.24.3) for each
         * Port set equal to the value of "root times".
         */
        port->designatedTimes = tree->rootTimes;
    }

    /* The port role for each Port is assigned, and its port priority
     * vector and Spanning Tree timer information are updated as
     * follows:
     */
    ForAllPorts(tree, port) {
        PortRole_t oldrole = port->selectedRole;

        if(port->infoIs == Disabled) {
            /* f) If the Port is Disabled (infoIs = Disabled),
             * selectedRole is set to DisabledPort. */
            port->selectedRole = DisabledPort;
        } else {
            /* g) Otherwise, if this procedure is invoked for a given
             * MSTI:
             *
             * 1) If the Port is not Disabled, the selected CIST Port
             * Role (calculated for the CIST prior to invoking this
             * procedure for a given MSTI) is RootPort, and the CIST
             * port priority information was received from a Bridge
             * external to the MST Region (infoIs == Received and
             * infoInternal == FALSE), selectedRole is set to
             * MasterPort. Additionally, updtInfo is set if the port
             * priority vector differs from the designated priority
             * vector or the Port's associated timer parameter differs
             * from the one for the Root Port; 
             *
             * 2) If the Port is not Disabled, the selected CIST Port
             * Role (calculated for the CIST prior to invoking this
             * procedure for a given MSTI) is AlternatePort, and the
             * CIST port priority information was received from a
             * Bridge external to the MST Region (infoIs == Received
             * and infoInternal == FALSE), selectedRole is set to
             * AlternatePort. Additionally, updtInfo is set if the
             * port priority vector differs from the designated
             * priority vector or the Port's associated timer
             * parameter differs from the one for the Root Port.
             */
            mstp_port_t *cport;
            if(tree->msti != MSTID_CIST &&
               (cport = get_port(tree->bridge, port->port_no)) != NULL &&
               cport->infoIs == Received &&
               !cport->cistport->infoInternal &&
               (cport->selectedRole == RootPort || 
                cport->selectedRole == AlternatePort)) {
                port->selectedRole = 
                    (cport->selectedRole == RootPort ? MasterPort : AlternatePort);
                if(struct_cmp(port->designatedPriority, !=, port->portPriority) || 
                   struct_cmp(port->designatedTimes, !=, port->portTimes))
                    port->updtInfo = TRUE;
            } else
                /* 
                 * Otherwise ... 
                 *
                 * the CIST or MSTI port role for each Port is
                 * assigned, and its port priority vector and Spanning
                 * Tree timer information are updated as follows:
                 */
                switch(port->infoIs) {

                case Aged:
                    /* h) If the port priority vector information was
                     * aged (infoIs = Aged), updtInfo is set and
                     * selectedRole is set to DesignatedPort; */
                    port->updtInfo = TRUE;
                    port->selectedRole = DesignatedPort;
                    break;

                case Mine:
                    /* i) If the port priority vector was derived from
                     * another port on the Bridge or from the Bridge
                     * itself as the Root Bridge (infoIs = Mine),
                     * selectedRole is set to
                     * DesignatedPort. Additionally, updtInfo is set
                     * if the port priority vector differs from the
                     * designated priority vector or the Port's
                     * associated timer parameter(s) differ(s) from
                     * the Root Port's associated timer parameters; */
                    port->selectedRole = DesignatedPort;
                    if(struct_cmp(port->designatedPriority, !=, port->portPriority) || 
                       struct_cmp(port->designatedTimes, !=, port->portTimes))
                        port->updtInfo = TRUE;
                    break;

                case Received:
                    /* j)-m) If the port priority vector was received
                     * in a Configuration Message and is not aged
                     * (infoIs == Received), ... */
                    
                    /* j) ..., and the root priority vector is now
                     * derived from it, selectedRole is set to
                     * RootPort, and updtInfo is reset; */
                    if(root == port) {
                        port->selectedRole = RootPort;
                        port->updtInfo = FALSE;
                        break;
                    }

                    /* k)-m) ..., the root priority vector is not now derived
                     * from it, ... */
                    /* Previous conditonal 'break's, causing the above to be
                     * true */
                    
                    /* k,l) ..., the designated priority vector is
                     * not better than the port priority vector, ... */
                    if(!priorityIsBetter(&port->designatedPriority, port)) {
                
                        /* k) ..., and the designated bridge and
                         * designated port components of the port
                         * priority vector reflect another port on
                         * this bridge, selectedRole is set to
                         * AlternatePort, and updtInfo is reset; */
                        if(struct_cmp(port->portPriority.DesignatedBridgeId, !=, 
                                      tree->BridgeIdentifier)) {
                            /* About the 'designated port' part - We don't
                             * check this explicitly - we assume if the bridge
                             * id matches, its our port */
                            port->selectedRole = AlternatePort;
                            port->updtInfo = FALSE;
                        } else {
                            /* l) ..., and the designated bridge and
                             * designated port components of the port
                             * priority vector reflect another port on
                             * this bridge, selectedRole is set to
                             * BackupPort, and updtInfo is reset; */
                            port->selectedRole = BackupPort;
                            port->updtInfo = FALSE;
                        }
                    } else {
                        /* m) ..., the designated priority vector is
                         * better than the port priority vector,
                         * selectedRole is set to DesignatedPort, and
                         * updtInfo is set. */
                        port->selectedRole = DesignatedPort;
                        port->updtInfo = TRUE;
                    }
                    break;
                    
                default:
                    VTSS_ABORT();
                }
        }

        T_D("Port %d[%d] infoIs %s updtInfo %d role %s => %s", 
            port->port_no, 
            port->msti,
            rstp_info2str(port->infoIs),
            port->updtInfo,
            rstp_portrole2str(oldrole),
            rstp_portrole2str(port->selectedRole));
    }
}

/************************************************************************
 * The following procedures perform functions additional to those
 * described in 17.21 of IEEE Std 802.1D for both the CIST and the
 * MSTI state machines or for the CIST or a given MSTI specifically:
 *
 * z) clearAllRcvdMsgs() (13.26.2)
 * aa) fromSameRegion() (13.26.4)
 * ab) recordMastered() (13.26.9)
 * ac) setRcvdMsgs() (13.26.12)
 * ad) syncMaster() (13.26.18)
 * 
 */

/** 13.26.2 clearAllRcvdMsgs / 13.26.12 setRcvdMsgs
 *
 * Set CIST and MSTI ports rcvdMsg to given value.
 */
void
setAllRcvdMsgs(mstp_port_t *port, bool v)
{
    ForAllPortNo(port->tree->bridge, port->port_no, { _tp_->rcvdMsg = v; });
}

/** 13.26.4 fromSameRegion()
 *
 * Returns TRUE if rcvdRSTP is TRUE, and the received BPDU conveys a
 * MST Configuration Identifier that matches that held for the
 * Bridge. Returns FALSE otherwise.
 * 
 */
bool
fromSameRegion(mstp_port_t *port)
{
    mstp_cistport_t *cist = port->cistport;
    if(cist->rcvdRSTP && cist->rcvBpdu.sameRegion)
        return TRUE;
    return FALSE;
}

/************************************************************************
 * 13.25 State machine conditions and parameters  
 * 
 * The following boolean variable evaluations are defined for
 * notational convenience in the state machines. These definitions
 * also serve to highlight those cases where a state transition for
 * one tree (CIST or MSTI) depends on the state of the variables of
 * one or more other trees.
 */

/************************************************************************
 * The following conditions and parameters are similar to those
 * specified in 17.20 of IEEE Std 802.1D but have enhanced or extended
 * specifications or considerations:
 *
 * j) allSynced (13.25.1)
 * k) FwdDelay (13.25.6)
 * l) HelloTime (13.25.7)
 * m) MaxAge (13.25.8)
 */

/** 13.25.6 FwdDelay
 * 
 * The Forward Delay component of the CIST's designatedTimes parameter
 * (13.24.3).
 */
timer
FwdDelay(const mstp_port_t *port)
{
    if(isCistPort(port)) {
        return port->designatedTimes.fwdDelay;
    }
    mstp_port_t *cport = get_port(port->cistport->bridge, port->port_no); 
    return cport->designatedTimes.fwdDelay;
}

/** 13.25.7 HelloTime
 * 
 * The Hello Time component of the CIST's portTimes parameter
 * (13.24.13) with the recommended default value given in 13.37.2.
 *
 * \note portTimes is derived from rootTimes and designatedTimes which
 * holds *no* HelloTime. This implies (by UNH) that it should be
 * sourced from the administratively setting, which in our case is
 * bridge global (default 2). To simplify, we managed helloTime
 * outside Times_t.
 */
timer
HelloTime(const mstp_port_t *port)
{
    return port->tree->bridge->conf.bridge.bridgeHelloTime;
}

/** 13.25.8 MaxAge
 * 
 * The Max Age component of the CIST's designatedTimes parameter
 * (13.24.3).
 */
timer
MaxAge(const mstp_port_t *port)
{
    if(isCistPort(port)) {
        return port->designatedTimes.maxAge;
    }
    mstp_port_t *cport = get_port(port->cistport->bridge, port->port_no); 
    return cport->designatedTimes.maxAge;
}

/** 13.25.1 allSynced()
 *
 * The condition allSynced is TRUE for a given Port, for a given Tree,
 * if and only if
 *
 * a) For all Ports for the given Tree, selected is TRUE, the Port's
 * role is the same as its selectedRole, and updtInfo is FALSE; and
 *
 * b) The role of the given Port is
 *
 *    1) Root Port or Alternate Port and synced is TRUE for all Ports
 *    for the given Tree other than the Root Port; or
 *
 *    2) Designated Port and synced is TRUE for all Ports for the
 *    given Tree other than the given Port; or
 *
 *    3) Master Port and synced is TRUE for all Ports for the given
 *    Tree other than the given Port.
 */
bool
allSynced(const struct mstp_tree *tree, const mstp_port_t *iport)
{
    mstp_port_t const *port;

    switch(iport->role) {
    case RootPort:
    case AlternatePort:
        ForAllPorts(tree, port) {
            if(port->selected && 
               port->role == port->selectedRole &&
               !port->updtInfo &&
               (port->role == RootPort || port->synced))
                continue;           /* Good so far */
            return FALSE;
        }
        break;

    case MasterPort:
    case DesignatedPort:
        ForAllPorts(tree, port) {
            if(port->selected && 
               port->role == port->selectedRole &&
               !port->updtInfo &&
               (port == iport || port->synced))
                continue;           /* Good so far */
            return FALSE;
        }
        break;
        
    default:
        return FALSE;
    }

    return TRUE;
}

/************************************************************************
 * The following conditions and parameters are additional to those
 * described in 17.20 of IEEE Std 802.1D:
 *
 * n) allTransmitReady (13.25.2)
 * o) cist (13.25.3) => isCistPort()
 * p) cistRootPort (13.25.4) (not implemented)
 * q) cistDesignatedPort (13.25.5) (not implemented)
 * r) mstiDesignatedOrTCpropagatingRootPort (13.25.9)
 * s) mstiMasterPort (13.25.10)
 * t) rcvdAnyMsg (13.25.11)
 * u) rcvdCistMsg (13.25.12)
 * v) rcvdMstiMsg (13.25.13)
 * w) restrictedRole (13.25.14)
 * x) restrictedTcn (13.25.15)
 * y) updtCistInfo (13.25.16)
 * z) updtMstiInfo (13.25.17)
 *
 */

/** 13.25.2 allTransmitReady()
 *
 * TRUE, if and only if, for the given Port for all Trees
 *
 * a) selected is TRUE; and
 *
 * b) updtInfo is FALSE.
 */
bool
allTransmitReady(mstp_port_t *port)
{
    VTSS_ASSERT(isCistPort(port)); /* Called from PortTransmit.stm */
    ForAllPortNo(port->tree->bridge, port->port_no, {
            if(!(_tp_->selected && !_tp_->updtInfo))
                return FALSE;
        });
    return TRUE;
}

/** 13.25.9 mstiDesignatedOrTCpropagatingRootPort() 
 *
 * TRUE if the role for any MSTI for the given Port is either:
 *
 * a) DesignatedPort; or
 *
 * b) RootPort, and the instance for the given MSTI and Port of
 * the tcWhile timer is not zero.
 */
bool
mstiDesignatedOrTCpropagatingRootPort(mstp_port_t *port)
{
    VTSS_ASSERT(isCistPort(port)); /* Called from PortTransmit.stm */
    ForAllMsti(port->tree->bridge, port->port_no, { 
            if(_tp_->selectedRole == DesignatedPort ||
               (_tp_->selectedRole == RootPort && _tp_->tcWhile == 0)) {
                T_N("Port %d: return TRUE", port->port_no);
                return TRUE;
            }
        });
    T_N("Port %d: return FALSE", port->port_no);
    return FALSE;
}

/** 13.25.10 mstiMasterPort()
 *
 * TRUE if the role for any MSTI for the given Port is MasterPort.
 */
bool
mstiMasterPort(mstp_port_t *port)
{
    VTSS_ASSERT(isCistPort(port)); /* Called from PortTransmit.stm */
    ForAllMsti(port->tree->bridge, port->port_no, { 
            if(_tp_->selectedRole == MasterPort)
                return TRUE;
        });
    return FALSE;
}

/** 13.25.11 rcvdAnyMsg
 * 
 * TRUE for a given Port if rcvdMsg is TRUE for the CIST or any MSTI
 * for that Port.
 */
bool
rcvdAnyMsg(mstp_port_t *port)
{
    ForAllPortNo(port->tree->bridge, port->port_no, { 
            if(_tp_->rcvdMsg) 
                return TRUE;
        });
    return FALSE;
}

/** 13.25.12 rcvdCistMsg()
 *
 * TRUE for a given Port if and only if rcvdMsg is TRUE for the CIST
 * for that Port.
 */
bool
rcvdCistMsg(const mstp_port_t *port)
{
    return port->rcvdMsg;
}

/** 13.25.13 rcvdMstiMsg()
 *
 * TRUE for a given Port and MSTI if and only if rcvdMsg is FALSE for
 * the CIST for that Port and rcvdMsg is TRUE for the MSTI for that
 * Port.
 */
bool
rcvdMstiMsg(const mstp_port_t *port)
{
    const mstp_port_t *cport = get_port(port->cistport->bridge, port->port_no); 
    return !cport->rcvdMsg && port->rcvdMsg;
}

/** 13.25.16 updtCistInfo()
 *
 * TRUE for a given Port if and only if updtInfo is TRUE for the CIST
 * for that Port.
*/
bool 
updtCistMsg(const mstp_port_t *port)
{
    return port->updtInfo;
}

/** 13.25.17 updtMstiInfo()
 *
 * TRUE for a given Port and MSTI if and only if updtInfo is TRUE for
 * the MSTI for that Port or updtInfo is TRUE for the CIST for that
 * Port.
 */
bool 
updtMstiMsg(const mstp_port_t *port)
{
    const mstp_port_t *cport = get_port(port->cistport->bridge, port->port_no); 
    return port->updtInfo || cport->updtInfo;
}


