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

#ifndef _LLDP_H_
#define _LLDP_H_

#include "lldp_api.h"

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_LLDP
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_STAT         2
#define TRACE_GRP_CONF         3
#define TRACE_GRP_SNMP         4
#define TRACE_GRP_TX           5
#define TRACE_GRP_RX           6
#define TRACE_GRP_POE          7
#define TRACE_GRP_CDP          8
#define TRACE_GRP_CLI          9
#define TRACE_GRP_EEE          10
#define TRACE_GRP_MED_TX       11
#define TRACE_GRP_MED_RX       12
#define TRACE_GRP_CNT          13

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_LLDP

#include <vtss_trace_api.h>
/* ============== */
#define LLDP_CONF_VERSION   3

// Default configuration values.
#define LLDPMED_ALTITUDE_DEFAULT 0
#define LLDPMED_ALTITUDE_TYPE_DEFAULT METERS

#define LLDPMED_LONGITUDE_DEFAULT 0
#define LLDPMED_LONGITUDE_DIR_DEFAULT EAST

#define LLDPMED_LATITUDE_DEFAULT 0
#define LLDPMED_LATITUDE_DIR_DEFAULT NORTH

#define LLDPMED_DATUM_DEFAULT WGS84

// The switch is by default not CDP aware
#define LLDP_CDP_AWARE_DEFAULT FALSE
/************************/
/* functions            */
/************************/
void lldp_system_name(char *sys_name, char get_name);

/************************/
/* misc                 */
/************************/

#endif /* _LLDP_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

