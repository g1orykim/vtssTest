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
#ifndef SWITCHRMON_H
#define SWITCHRMON_H

/*
 * function declarations
 */
void            init_switchRMON(void);
FindVarMethod   var_switchRMON;
FindVarMethod   var_dataSourceCapsTable;
FindVarMethod   var_smonVlanStatsControlTable;
FindVarMethod   var_smonVlanIdStatsTable;
FindVarMethod   var_smonPrioStatsControlTable;
FindVarMethod   var_smonPrioStatsTable;
FindVarMethod   var_portCopyTable;
WriteMethod     write_smonVlanStatsControlDataSource;
WriteMethod     write_smonVlanStatsControlOwner;
WriteMethod     write_smonVlanStatsControlStatus;
WriteMethod     write_smonPrioStatsControlDataSource;
WriteMethod     write_smonPrioStatsControlOwner;
WriteMethod     write_smonPrioStatsControlStatus;
WriteMethod     write_portCopyDirection;
WriteMethod     write_portCopyStatus;
void smon_create_stat_default_entry(void);
#endif                          /* SWITCHRMON_H */

