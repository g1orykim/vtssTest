/*

 Vitesse EPS software.

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

#ifndef _EPS_API_H_
#define _EPS_API_H_

#include "main.h"
#include "vtss_types.h"
#include "vtss_eps_api.h"
#include "mep_api.h"

#define EPS_MGMT_CREATED_MAX   VTSS_EPS_MGMT_CREATED_MAX

#define EPS_RC_OK                      0
#define EPS_RC_NOT_CREATED             1
#define EPS_RC_CREATED                 2
#define EPS_RC_INVALID_PARAMETER       3
#define EPS_RC_NOT_CONFIGURED          4
#define EPS_RC_ARCHITECTURE            5
#define EPS_RC_W_P_FLOW_EQUAL          6
#define EPS_RC_W_P_SSF_MEP_EQUAL       7
#define EPS_RC_INVALID_APS_MEP         8
#define EPS_RC_INVALID_W_MEP           9
#define EPS_RC_INVALID_P_MEP           10
#define EPS_RC_WORKING_USED            11
#define EPS_RC_PROTECTING_USED         12


/****************************************************************************/
/*  MEP management interface                                                */
/****************************************************************************/

#define EPS_MEP_INST_INVALID MEP_INSTANCE_MAX
typedef struct
{
    u32                             w_mep;              /* Working MEP instance number.                */
    u32                             p_mep;              /* Protecting MEP instance number.             */
    u32                             aps_mep;            /* APS MEP instance number.                    */
} eps_mgmt_mep_t;

/* instance:        Instance number of EPS              */
/* param:           Parameters for this create          */
/* An EPS instance is created                           */
u32 eps_mgmt_instance_create(const u32                          instance,
                             const vtss_eps_mgmt_create_param_t *const param);

/* instance:		Instance number of EPS.	*/
/* An EPS instance is now deleted.			*/
u32 eps_mgmt_instance_delete(const u32     instance);



/* instance:        Instance number of EPS               */
/* mep:             MEP relation data for this instance  */
/* An EPS instance is related to MEP instances           */
u32 eps_mgmt_mep_set(const u32               instance,
                     const eps_mgmt_mep_t    *const mep);


/* instance:    Instance number of EPS.                 */
/* conf:        Configuration data for this instance    */
u32 eps_mgmt_conf_set(const u32                   instance,
                      const vtss_eps_mgmt_conf_t  *const conf);

/* instance:        Instance number of EPS.                 */
/* param:           Create parameters for this instance     */
/* conf:            Configuration data for this instance    */
/* mep:             MEP relation data for this instance     */
u32 eps_mgmt_conf_get(const u32                      instance,
                      vtss_eps_mgmt_create_param_t   *const param,
                      vtss_eps_mgmt_conf_t           *const conf,
                      eps_mgmt_mep_t                 *mep);




/* instance:    Instance number of EPS.                                 */
/* command:	One of the possible management commands to the APS protocol */
u32 eps_mgmt_command_set(const u32                        instance,
                         const vtss_eps_mgmt_command_t    command);

/* instance:    Instance number of EPS.                                     */
/* command:     One of the possible management commands to the APS protocol */
u32 eps_mgmt_command_get(const u32                  instance,
                         vtss_eps_mgmt_command_t    *const command);





/* instance:    Instance number of EPS. */
/* Get the state of this EPS            */
u32 eps_mgmt_state_get(const u32                instance,
                       vtss_eps_mgmt_state_t    *const state);




void eps_mgmt_def_conf_get(vtss_eps_mgmt_def_conf_t  *const def_conf);


/****************************************************************************/
/*  MEP module interface                                                    */
/****************************************************************************/

/* instance:    Instance number of EPS.                 */
/* mep_inst:    Instance number of MEP                  */
/* aps_info:    Array[4] with the received APS info.	*/
/* MEP is calling this to deliver latest received APS on a flow */
u32 eps_rx_aps_info_set(const u32    instance,
                        const u32    mep_inst,
                        const u8     aps[VTSS_EPS_APS_DATA_LENGTH]);


/* instance:    Instance number of EPS.                               */
/* mep_inst:    Instance number of MEP                                */
/* This is called by MEP to activate EPS calling out transmitting APS */
u32 eps_signal_in(const u32   instance,
                  const u32   mep_inst);


/* instance:    Instance number of EPS.                             */
/* mep_inst:    Instance number of MEP                              */
/* sf_state:    Lates state of SF for this instance.                */
/* sd_state:    Lates state of SD for this instance.                */
/* MEP is calling this to deliver latest SF/SD state on a flow.     */
u32 eps_sf_sd_state_set(const u32   instance,
                        const u32   mep_inst,
                        const BOOL  sf_state,
                        const BOOL  sd_state);




/* Initialize module */
vtss_rc eps_init(vtss_init_data_t *data);

#endif /* _EPS_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
