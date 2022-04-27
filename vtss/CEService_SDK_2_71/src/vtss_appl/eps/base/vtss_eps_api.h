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

#ifndef _VTSS_EPS_API_H_
#define _VTSS_EPS_API_H_

#include "vtss_types.h"

#define VTSS_EPS_MGMT_CREATED_MAX   100     /* Max number of created EPS */

#define VTSS_EPS_MGMT_WTR_MAX       720     /* Max. 12 * 60 sec. WTR */
#define VTSS_EPS_MGMT_HOFF_OFF        0
#define VTSS_EPS_MGMT_HOFF_MAX      100     /* Max. 100 * 100 ms. HOFF */


#define VTSS_EPS_RC_OK                      0   /* Management operation is ok */
#define VTSS_EPS_RC_NOT_CREATED             1   /* Operating on an instance not created */
#define VTSS_EPS_RC_CREATED                 2   /* Creating an instance already created */
#define VTSS_EPS_RC_INVALID_PARAMETER       3   /* invalid parameter */
#define VTSS_EPS_RC_NOT_CONFIGURED          4   /* Instance is not configured */
#define VTSS_EPS_RC_APS                     5   /* Invalid APS request */
#define VTSS_EPS_RC_ARCHITECTURE            6   /* 1 plus 1 architecture is only for port domain */
#define VTSS_EPS_RC_W_P_EQUAL               7   /* Working and protecting is equal */
#define VTSS_EPS_RC_WORKING_USED            8   /* Working is used by other instance */
#define VTSS_EPS_RC_PROTECTING_USED         9   /* Protecting is used by other instance */

#define VTSS_EPS_APS_DATA_LENGTH     4


/****************************************************************************/
/*  EPS management interface                                                */
/****************************************************************************/

typedef enum
{
    VTSS_EPS_MGMT_ARCHITECTURE_1P1,  /* Architecture is 1+1 */
    VTSS_EPS_MGMT_ARCHITECTURE_1F1   /* Architecture is 1:1 */
} vtss_eps_mgmt_architecture_t;

typedef enum
{
    VTSS_EPS_MGMT_PORT,      /* Domain is Port */
//    VTSS_EPS_MGMT_PATH,      /* Domain is Path */
    VTSS_EPS_MGMT_EVC,       /* Domain is EVC */
//    VTSS_EPS_MGMT_MPLS       /* Domain is Mpls */
} vtss_eps_mgmt_domain_t;

typedef struct
{
    vtss_eps_mgmt_domain_t          domain;             /* Domain.                                     */
    vtss_eps_mgmt_architecture_t    architecture;       /* Architecture.                               */
    u32                             w_flow;             /* Working flow instance number.               */
    u32                             p_flow;             /* Working flow instance number.               */
} vtss_eps_mgmt_create_param_t;

/* instance:        Instance number of EPS              */
/* param:           Parameters for this create          */
/* An EPS instance is created                           */
u32 vtss_eps_mgmt_instance_create(const u32                             instance,
                                  const vtss_eps_mgmt_create_param_t    *const param);

/* instance:		Instance number of EPS.	*/
/* An EPS instance is now deleted.			*/
u32 vtss_eps_mgmt_instance_delete(const u32     instance);




typedef enum
{
    VTSS_EPS_MGMT_UNIDIRECTIONAL,    /* Unidirectional 1+1 */
    VTSS_EPS_MGMT_BIDIRECTIONAL      /* Bidirectional 1+1 */
} vtss_eps_mgmt_directional_t;

typedef struct
{
    vtss_eps_mgmt_directional_t	directional;        /* Directional. Only for 1+1                                     */
    BOOL					    aps;                /* APS enabled if true. Only for 1+1                             */
    BOOL					    revertive;          /* Revertive operation enabled if true.                          */
    u32                         restore_timer;      /* Wait to restore timer in seconds. See VTSS_EPS_MGMT_WTR_XXX   */
    u32                         hold_off_timer;     /* Hold off timer in 100 ms. see VTSS_EPS_MGMT_HOFF_XXX          */
} vtss_eps_mgmt_conf_t;

/* instance:    Instance number of EPS.                 */
/* conf:        Configuration data for this instance    */
u32 vtss_eps_mgmt_conf_set(const u32                    instance,
                           const vtss_eps_mgmt_conf_t   *const conf);

/* instance:        Instance number of EPS.                 */
/* param:           Create parameters for this instance     */
/* conf:            Configuration data for this instance    */
u32 vtss_eps_mgmt_conf_get(const u32                         instance,
                           vtss_eps_mgmt_create_param_t      *const param,
                           vtss_eps_mgmt_conf_t              *const conf);




typedef enum
{
    VTSS_EPS_MGMT_COMMAND_NONE,              /* Any active command is cleared                                     */
    VTSS_EPS_MGMT_COMMAND_CLEAR,             /* Any active command is cleared - changed to NONE after CLEAR       */
    VTSS_EPS_MGMT_COMMAND_LOCK_OUT,          /* end-to-end lock out of protection                                 */
    VTSS_EPS_MGMT_COMMAND_FORCED_SWITCH,     /* Forced switch to protection                                       */
    VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_P,   /* Manuel switch to protection                                       */
    VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_W,   /* Manuel switch to working                                          */
    VTSS_EPS_MGMT_COMMAND_EXERCISE,          /* Exercise of APS protocol                                          */
    VTSS_EPS_MGMT_COMMAND_FREEZE,            /* Local Freeze of protection group                                  */
    VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL     /* Local lock out of protection                                      */
} vtss_eps_mgmt_command_t;

/* instance:    Instance number of EPS.                                 */
/* command:	One of the possible management commands to the APS protocol */
u32 vtss_eps_mgmt_command_set(const u32                         instance,
                              const vtss_eps_mgmt_command_t     command);

/* instance:    Instance number of EPS.                                     */
/* command:     One of the possible management commands to the APS protocol */
u32 vtss_eps_mgmt_command_get(const u32                   instance,
                              vtss_eps_mgmt_command_t     *const command);





typedef enum
{
    VTSS_EPS_MGMT_PROT_STATE_DISABLED,          /* The following according to Y.1731 */
    VTSS_EPS_MGMT_PROT_STATE_NO_REQUEST_W,
    VTSS_EPS_MGMT_PROT_STATE_NO_REQUEST_P,
    VTSS_EPS_MGMT_PROT_STATE_LOCKOUT,
    VTSS_EPS_MGMT_PROT_STATE_FORCED_SWITCH,
    VTSS_EPS_MGMT_PROT_STATE_SIGNAL_FAIL_W,
    VTSS_EPS_MGMT_PROT_STATE_SIGNAL_FAIL_P,
    VTSS_EPS_MGMT_PROT_STATE_MANUEL_SWITCH_W,
    VTSS_EPS_MGMT_PROT_STATE_MANUEL_SWITCH_P,
    VTSS_EPS_MGMT_PROT_STATE_WAIT_TO_RESTORE,
    VTSS_EPS_MGMT_PROT_STATE_EXERCISE_W,
    VTSS_EPS_MGMT_PROT_STATE_EXERCISE_P,
    VTSS_EPS_MGMT_PROT_STATE_REVERSE_REQUEST_W,
    VTSS_EPS_MGMT_PROT_STATE_REVERSE_REQUEST_P,
    VTSS_EPS_MGMT_PROT_STATE_DO_NOT_REVERT
} vtss_eps_mgmt_prot_state_t;                /* Protection switching state */

typedef enum
{
    VTSS_EPS_MGMT_DEFECT_STATE_OK,
    VTSS_EPS_MGMT_DEFECT_STATE_SD,
    VTSS_EPS_MGMT_DEFECT_STATE_SF
} vtss_eps_mgmt_defect_state_t;              /* Working/protecting defect state */

typedef enum
{
    VTSS_EPS_MGMT_REQUEST_NR,
    VTSS_EPS_MGMT_REQUEST_DNR,
    VTSS_EPS_MGMT_REQUEST_RR,
    VTSS_EPS_MGMT_REQUEST_EXER,
    VTSS_EPS_MGMT_REQUEST_WTR,
    VTSS_EPS_MGMT_REQUEST_MS,
    VTSS_EPS_MGMT_REQUEST_SD,
    VTSS_EPS_MGMT_REQUEST_SF_W,
    VTSS_EPS_MGMT_REQUEST_FS,
    VTSS_EPS_MGMT_REQUEST_SF_P,
    VTSS_EPS_MGMT_REQUEST_LO
} vtss_eps_mgmt_request_t;               /* Received/transmitted APS request */

typedef struct
{
    vtss_eps_mgmt_request_t     request;    /* Request type */
    u32                         re_signal;  /* Requested signal */
    u32                         br_signal;  /* Bridged signal */
}vtss_eps_mgmt_aps_info_t;

typedef struct
{
    vtss_eps_mgmt_prot_state_t       protection_state;   /* Protection switch state              */
    vtss_eps_mgmt_defect_state_t     w_state;            /* Working state                        */
    vtss_eps_mgmt_defect_state_t     p_state;            /* Protecting state                     */
    vtss_eps_mgmt_aps_info_t         tx_aps;             /* Transmitting APS request             */
    vtss_eps_mgmt_aps_info_t         rx_aps;             /* Receiving APS request                */
    BOOL                             dFop_pm;            /* APS ‘B’ bit mismatch state           */
    BOOL                             dFop_cm;            /* APS configuration mismatch state     */
    BOOL                             dFop_nr;            /* APS protection incomplete state      */
    BOOL                             dFop_NoAps;         /* APS not received                     */
} vtss_eps_mgmt_state_t;

/* instance:    Instance number of EPS. */
/* Get the state of this EPS            */
u32 vtss_eps_mgmt_state_get(const u32                 instance,
                            vtss_eps_mgmt_state_t     *const state);


typedef struct
{
    vtss_eps_mgmt_create_param_t   param;
    vtss_eps_mgmt_conf_t           config;
    vtss_eps_mgmt_command_t        command;
} vtss_eps_mgmt_def_conf_t;

void  vtss_eps_mgmt_def_conf_get(vtss_eps_mgmt_def_conf_t  *const def_conf);



/****************************************************************************/
/*  EPS module call in interface                                            */
/****************************************************************************/

typedef enum
{
    VTSS_EPS_FLOW_WORKING,
    VTSS_EPS_FLOW_PROTECTING
} vtss_eps_flow_type_t;

/* instance:    Instance number of EPS.                 */
/* flow:        Flow type                               */
/* aps_info:    Array[4] with the received APS info.	*/
/* External is calling this to deliver latest received APS on a flow */
u32 vtss_eps_rx_aps_info_set(const u32                     instance,
                             const vtss_eps_flow_type_t    flow,
                             const u8                      aps[VTSS_EPS_APS_DATA_LENGTH]);


/* instance:    Instance number of EPS.       */
/* This is called by external to activate EPS calling out transmitting APS */
u32 vtss_eps_signal_in(const u32   instance);


/* instance:    Instance number of EPS.                                    */
/* flow:        Flow type                                                  */
/* sf_state:    Lates state of SF for this instance.                       */
/* sd_state:    Lates state of SD for this instance.                       */
/* External is calling this to deliver latest SF/SD state on a flow.       */
u32 vtss_eps_sf_sd_state_set(const u32                     instance,
                             const vtss_eps_flow_type_t    flow,
                             const BOOL                    sf_state,
                             const BOOL                    sd_state);





/****************************************************************************/
/*  EPS module call out interface                                           */
/****************************************************************************/

/* instance:    Instance number of EPS.                    */
/* domain:      Domain.                                    */
/* aps_info:	Array[4] with the transmitted APS info.	   */
/* This is called by EPS to transmit APS info in a flow    */
void vtss_eps_tx_aps_info_set(const u32                       instance,
                              const vtss_eps_mgmt_domain_t    domain,
                              const u8                        aps[VTSS_EPS_APS_DATA_LENGTH]);



/* instance:    Instance number of EPS.                    */
/* This is called by EPS on change in port protection traffic selector     */
void vtss_eps_port_protection_set(const u32         w_port,
                                  const u32         p_port,
                                  const BOOL        active);



/* instance:    Instance number of EPS.                    */
/* domain:      Domain.                                    */
/* This is called by EPS to activate call in on all in functions     */
void vtss_eps_signal_out(const u32                       instance,
                         const vtss_eps_mgmt_domain_t    domain);





/****************************************************************************/
/*  EPS platform call in interface                                          */
/****************************************************************************/

/* stop:    Returnvalue indicating if calling this thread has to stop.                                                   */
/* This is the thread driving timing in the EPS. Has to be called every 'timer_resolution' ms. by platform until 'stop'  */
/* Initially this thread is not called until EPS callout on vtss_eps_timer_start()                                       */
void vtss_eps_timer_thread(BOOL  *const stop);


/* This is the thread driving the state machine. Has to be call when EPS is calling out on vtss_eps_run() */ 
void vtss_eps_run_thread(void);


/* timer_resolution:    This is the interval of calling vtss_eps_run_thread() in ms.    */
/* This is the initializing of EPS. Has to be called by platform                        */
u32 vtss_eps_init(const u32  timer_resolution);






/****************************************************************************/
/*  EPS platform call out interface                                         */
/****************************************************************************/

/* This is call by EPS to lock/unlock critical code protection */
void vtss_eps_crit_lock(void);

void vtss_eps_crit_unlock(void);

/* This is called by EPS when vtss_eps_run_thread(void) has to be called */
void vtss_eps_run(void);

/* This is called by EPS when vtss_eps_timer_thread(BOOL  *const stop) has to be called until 'stop' is indicated */
void vtss_eps_timer_start(void);

/* This is called by EPS in order to do debug tracing */
void vtss_eps_trace(const char   *const string,
                    const u32    param1,
                    const u32    param2,
                    const u32    param3,
                    const u32    param4);


#endif /* _VTSS_EPS_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
