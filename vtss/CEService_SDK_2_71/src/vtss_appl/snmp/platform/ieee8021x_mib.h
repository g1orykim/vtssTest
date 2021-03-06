/*

 Vitesse Switch Software.

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
/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.old-api.conf
 */

#ifndef IEEE8021PAEMIB_H
#define IEEE8021PAEMIB_H

#define IEEE8021X_SUPPORTED_MIB     1

/* function declarations */
void init_ieee8021paeMIB(void);
FindVarMethod var_ieee8021paeMIB;
FindVarMethod var_dot1xPaePortTable;
FindVarMethod var_dot1xAuthConfigTable;
FindVarMethod var_dot1xAuthStatsTable;
FindVarMethod var_dot1xAuthDiagTable;
FindVarMethod var_dot1xAuthSessionStatsTable;
FindVarMethod var_dot1xSuppConfigTable;
FindVarMethod var_dot1xSuppStatsTable;
WriteMethod write_dot1xPaeSystemAuthControl;
WriteMethod write_dot1xPaePortInitialize;
WriteMethod write_dot1xPaePortReauthenticate;
WriteMethod write_dot1xAuthAdminControlledDirections;
WriteMethod write_dot1xAuthAuthControlledPortControl;
WriteMethod write_dot1xAuthQuietPeriod;
WriteMethod write_dot1xAuthTxPeriod;
WriteMethod write_dot1xAuthSuppTimeout;
WriteMethod write_dot1xAuthServerTimeout;
WriteMethod write_dot1xAuthMaxReq;
WriteMethod write_dot1xAuthReAuthPeriod;
WriteMethod write_dot1xAuthReAuthEnabled;
WriteMethod write_dot1xAuthKeyTxEnabled;
WriteMethod write_dot1xSuppHeldPeriod;
WriteMethod write_dot1xSuppAuthPeriod;
WriteMethod write_dot1xSuppStartPeriod;
WriteMethod write_dot1xSuppMaxStart;
WriteMethod write_dot1xSuppAccessCtrlWithAuth;

#endif /* IEEE8021PAEMIB_H */

