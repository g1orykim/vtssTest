/*

 Vitesse Switch Application software.

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

#ifndef __ERPS_H__
#define __ERPS_H__

#include "main.h"
#include "erps_api.h"
#include "vtss_erps_api.h"

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_ERPS

typedef struct erps_mep_conf {
    u32 east_mepid;
    u32 west_mepid;
    u32 raps_east_mepid;
    u32 raps_west_mepid;
} erps_mep_conf_t;

typedef struct erps_config_blk
{
    u32                     count;
    vtss_erps_config_erpg_t groups[ERPS_MAX_PROTECTION_GROUPS];
    erps_mep_conf_t         mep_conf[ERPS_MAX_PROTECTION_GROUPS];
}erps_config_blk_t;

typedef struct raps_pdu_in
{
    u8   pdu[256];
    u8   len;
    u32  mep_id;
    u32  erpg;
}raps_pdu_in_t;

#ifndef API2L2PORT
#define API2L2PORT(p) (p - 1)
#endif

void erps_platform_set_erpg_ports (vtss_port_no_t east, 
                                   vtss_port_no_t west,
                                   u32 group_id);

void erps_platform_set_erpg_meps (u32 ccm_east,
                                  u32 ccm_west, 
                                  u32 raps_east,
                                  u32 raps_west, 
                                  u32 group_id);

void erps_platform_clear_port_to_mep (u32 group_id);

/* erps_instance :     ERPS protection group id                               */
/* registering particular MEP instance, i.e ERPS interested in knowing about
   the events that are occured the given MEP                                  */
void erps_instance_signal_in (u32 erps_instance);

/* east_port      :  east port number related an ERPS group                   */
/* west_port      :  west port number related an ERPS group                   */
/* erps_instance  :  protection group instance number                         */
/* enable         :  associating a given MEP instance to the ERPS instance    */
/* raps_virt_channel : RAPS virtual channel present or not                    */
vtss_rc erps_mep_aps_sf_register (vtss_port_no_t east_port,
                                  vtss_port_no_t west_port,
                                  u32 erps_instance,
                                  BOOL enable,
                                  BOOL raps_virt_channel);

#endif /* __ERPS_H__ */
