/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
/*
 * Note: this file originally auto-generated by mib2c using
 *        $
 */
#ifndef QBRIDGEMIB_H
#define QBRIDGEMIB_H

#define RFC2674_SUPPORTED_Q_BRIDGE  1

/* function declarations */
void init_rfc2674_q_bridge(void);
FindVarMethod var_qBridgeMIB;
FindVarMethod var_dot1qFdbTable;
FindVarMethod var_dot1qTpFdbTable;
FindVarMethod var_dot1qTpGroupTable;
FindVarMethod var_dot1qForwardAllTable;
FindVarMethod var_dot1qForwardUnregisteredTable;
FindVarMethod var_dot1qStaticUnicastTable;
FindVarMethod var_dot1qStaticMulticastTable;
FindVarMethod var_dot1qVlanCurrentTable;
FindVarMethod var_dot1qVlanStaticTable;
FindVarMethod var_dot1qPortVlanTable;
FindVarMethod var_dot1qPortVlanStatisticsTable;
FindVarMethod var_dot1qPortVlanHCStatisticsTable;
FindVarMethod var_dot1qLearningConstraintsTable;
FindVarMethod var_dot1vProtocolGroupTable;
FindVarMethod var_dot1vProtocolPortTable;
WriteMethod write_dot1qGvrpStatus;
WriteMethod write_dot1qConstraintSetDefault;
WriteMethod write_dot1qConstraintTypeDefault;
WriteMethod write_dot1qForwardAllStaticPorts;
WriteMethod write_dot1qForwardAllForbiddenPorts;
WriteMethod write_dot1qForwardUnregisteredStaticPorts;
WriteMethod write_dot1qForwardUnregisteredForbiddenPorts;
WriteMethod write_dot1qStaticUnicastAllowedToGoTo;
WriteMethod write_dot1qStaticUnicastStatus;
WriteMethod write_dot1qStaticMulticastStaticEgressPorts;
WriteMethod write_dot1qStaticMulticastForbiddenEgressPorts;
WriteMethod write_dot1qStaticMulticastStatus;
WriteMethod write_dot1qVlanStaticName;
WriteMethod write_dot1qVlanStaticEgressPorts;
WriteMethod write_dot1qVlanForbiddenEgressPorts;
WriteMethod write_dot1qVlanStaticUntaggedPorts;
WriteMethod write_dot1qVlanStaticRowStatus;
WriteMethod write_dot1qPvid;
WriteMethod write_dot1qPortAcceptableFrameTypes;
WriteMethod write_dot1qPortIngressFiltering;
WriteMethod write_dot1qPortGvrpStatus;
WriteMethod write_dot1qPortRestrictedVlanRegistration;
WriteMethod write_dot1qConstraintType;
WriteMethod write_dot1qConstraintStatus;
WriteMethod write_dot1vProtocolGroupId;
WriteMethod write_dot1vProtocolGroupRowStatus;
WriteMethod write_dot1vProtocolPortGroupVid;
WriteMethod write_dot1vProtocolPortRowStatus;

#endif /* QBRIDGEMIB_H */

