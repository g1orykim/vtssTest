/*

 Vitesse Switch Software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef RFC3636_MAU_H
#define RFC3636_MAU_H

#define RFC3636_SUPPORTED_MAU                               1
#define RFC3636_SUPPORTED_rpMAU                             0
#define RFC3636_SUPPORTED_ifMAU                             1
#define RFC3636_SUPPORTED_BROAD_MAU                 0
#define RFC3636_SUPPORTED_ifMAU_AUTONEG         1
/*
 * function declarations
 */
#if RFC3636_SUPPORTED_MAU
void            init_snmpDot3MauMgt(void);
FindVarMethod   var_snmpDot3MauMgt;
#if RFC3636_SUPPORTED_rpMAU
FindVarMethod   var_rpMauTable;
FindVarMethod   var_rpJackTable;
#endif      /*RFC3636_SUPPORTED_rpMAU*/
#if RFC3636_SUPPORTED_ifMAU
FindVarMethod   var_ifMauTable;
FindVarMethod   var_ifJackTable;
#endif      /*RFC3636_SUPPORTED_ifMAU*/
#if RFC3636_SUPPORTED_BROAD_MAU
FindVarMethod   var_broadMauBasicTable;
#endif      /*RFC3636_SUPPORTED_BROAD_MAU*/
#if RFC3636_SUPPORTED_ifMAU_AUTONEG
FindVarMethod   var_ifMauAutoNegTable;
#endif      /*RFC3636_SUPPORTED_ifMAU_AUTONEG*/

#if RFC3636_SUPPORTED_rpMAU
WriteMethod     write_rpMauStatus;
#endif      /*RFC3636_SUPPORTED_rpMAU*/
#if RFC3636_SUPPORTED_ifMAU
#if 0 /* Not support in E-Stax34 */
WriteMethod     write_ifMauStatus;
#endif
WriteMethod     write_ifMauDefaultType;
#endif      /*RFC3636_SUPPORTED_ifMAU*/
#if RFC3636_SUPPORTED_ifMAU_AUTONEG
WriteMethod     write_ifMauAutoNegAdminStatus;
WriteMethod     write_ifMauAutoNegCapAdvertisedBits;
WriteMethod     write_ifMauAutoNegRestart;
#if 0 /* Not support in E-Stax34 */
WriteMethod     write_ifMauAutoNegCapAdvertisedBits;
WriteMethod     write_ifMauAutoNegRemoteFaultAdvertised;
#endif
#endif      /*RFC3636_SUPPORTED_ifMAU_AUTONEG*/

#endif /* RFC3636_SUPPORTED_MAU */

#endif                          /* RFC3636_MAU_H */
