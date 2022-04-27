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

#include "main.h"
#include "conf_api.h"
#include "critd_api.h"
#include "vtss_l2_api.h"
#include "eps_api.h"
#include "mep_api.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_VCLI
#include "eps_cli.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "eps_icli_functions.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_EPS

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_EPS

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_API          2
#define TRACE_GRP_CNT          3

#include <vtss_trace_api.h>


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "EPS",
    .descr     = "EPS module."
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = { 
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_API] = { 
        .name      = "api",
        .descr     = "Switch API printout",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define CRIT_ENTER(crit) critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CRIT_EXIT(crit)  critd_exit( &crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define CRIT_ENTER(crit) critd_enter(&crit)
#define CRIT_EXIT(crit)  critd_exit( &crit)
#endif /* VTSS_TRACE_ENABLED */



/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/
#define FLAG_TIMER     0x0001

typedef struct
{
    ulong                           version;                          /* Block version */
    BOOL                            created[EPS_MGMT_CREATED_MAX];
    BOOL                            configured[EPS_MGMT_CREATED_MAX];
    vtss_eps_mgmt_create_param_t    param[EPS_MGMT_CREATED_MAX];
    vtss_eps_mgmt_conf_t            conf[EPS_MGMT_CREATED_MAX];
    vtss_eps_mgmt_command_t         command[EPS_MGMT_CREATED_MAX];
    eps_mgmt_mep_t                  mep[EPS_MGMT_CREATED_MAX];
} eps_conf_t;

static cyg_handle_t        timer_thread_handle;
static cyg_thread          timer_thread_block;
static u8                  timer_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static cyg_handle_t        run_thread_handle;
static cyg_thread          run_thread_block;
static u8                  run_thread_stack[THREAD_DEFAULT_STACK_SIZE];

static critd_t             crit; 
static critd_t             crit_p;      /* Platform critd */

static cyg_sem_t           run_wait_sem;
static cyg_flag_t          timer_wait_flag;

typedef struct
{
    BOOL         created;
    u32          w_mep;              /* Working MEP instance number.                */
    u32          p_mep;              /* Protecting MEP instance number.             */
    u32          aps_mep;            /* APS MEP instance number.                    */
} eps_data_t;

static eps_data_t  eps_data[EPS_MGMT_CREATED_MAX];


static void set_conf_to_default(eps_conf_t  *blk)
{
    u32 i;

    vtss_eps_mgmt_def_conf_t  def_conf;

    vtss_eps_mgmt_def_conf_get(&def_conf);

    CRIT_ENTER(crit_p);
    for (i=0; i<EPS_MGMT_CREATED_MAX; ++i)
    {
        memset(blk, 0, sizeof(eps_conf_t));
        blk->created[i] = FALSE;
        blk->configured[i] = FALSE;
        blk->conf[i] = def_conf.config;
        blk->param[i] = def_conf.param;
        blk->command[i] = def_conf.command;

        memset(&eps_data[i], 0, sizeof(eps_data_t));
    }
    CRIT_EXIT(crit_p);
}


static void apply_configuration(eps_conf_t  *blk)
{
    u32 i, rc=0, del;
    vtss_eps_mgmt_create_param_t    *param;

    for (i=0; i<EPS_MGMT_CREATED_MAX; ++i)
    {
        if (blk->created[i])
        {
            param = &blk->param[i];
            rc += vtss_eps_mgmt_instance_create(i, param);
            if (param->domain == VTSS_EPS_MGMT_PORT)     /* This in order to in form MEP about port conversion in case of port protection */
                mep_port_protection_create((param->architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1) ? MEP_EPS_ARCHITECTURE_1P1 : MEP_EPS_ARCHITECTURE_1F1,  param->w_flow,  param->p_flow);

            rc += mep_eps_aps_register(blk->mep[i].aps_mep, i, MEP_EPS_TYPE_ELPS, TRUE);
            rc += mep_eps_sf_register(blk->mep[i].w_mep, i, MEP_EPS_TYPE_ELPS, TRUE);
            rc += mep_eps_sf_register(blk->mep[i].p_mep, i, MEP_EPS_TYPE_ELPS, TRUE);
            CRIT_ENTER(crit_p);
            eps_data[i].created = TRUE;
            eps_data[i].w_mep = blk->mep[i].w_mep;
            eps_data[i].p_mep = blk->mep[i].p_mep;
            eps_data[i].aps_mep = blk->mep[i].aps_mep;
            CRIT_EXIT(crit_p);

            if (blk->configured[i])
            {
                rc += vtss_eps_mgmt_conf_set(i, &blk->conf[i]);
                rc += vtss_eps_mgmt_command_set(i, blk->command[i]);
            }
        }
        else
        {
            del = vtss_eps_mgmt_instance_delete(i);
            if (del == VTSS_EPS_RC_NOT_CREATED)      rc += VTSS_EPS_RC_OK;
            else                                     rc += del;
        }
    }
    if (rc)        T_D("Error during configuration");
}


static void eps_restore_to_default(void)
{
    eps_conf_t *blk = VTSS_MALLOC(sizeof(*blk));
    if (!blk) {
        T_W("Out of memory for EPS restore-to-default");
    } else {
        set_conf_to_default(blk);
        apply_configuration(blk);
        VTSS_FREE(blk);
    }
}


static void eps_timer_thread(cyg_addrword_t data)
{
    BOOL               stop;

    for (;;)
    {
        (void) cyg_flag_wait(&timer_wait_flag, FLAG_TIMER, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
        do
        {
            VTSS_OS_MSLEEP(10);
            vtss_eps_timer_thread(&stop);
        } while(!stop);
    }
}


static void eps_run_thread(cyg_addrword_t data)
{
    cyg_bool_t  rc;

    for (;;)
    {
        rc = cyg_semaphore_wait(&run_wait_sem);
        if (!rc)        T_D("Thread was released");
        vtss_eps_run_thread();
    }
}


static u32 rc_conv(u32 rc)
{
    switch (rc)
    {
        case VTSS_EPS_RC_OK:                     return(EPS_RC_OK);
        case VTSS_EPS_RC_NOT_CREATED:            return(EPS_RC_NOT_CREATED);
        case VTSS_EPS_RC_CREATED:                return(EPS_RC_CREATED);
        case VTSS_EPS_RC_INVALID_PARAMETER:      return(EPS_RC_INVALID_PARAMETER);
        case VTSS_EPS_RC_NOT_CONFIGURED:         return(EPS_RC_NOT_CONFIGURED);
        case VTSS_EPS_RC_ARCHITECTURE:           return(EPS_RC_ARCHITECTURE);
        case VTSS_EPS_RC_W_P_EQUAL:              return(EPS_RC_W_P_FLOW_EQUAL);
        case VTSS_EPS_RC_WORKING_USED:           return(EPS_RC_WORKING_USED);
        case VTSS_EPS_RC_PROTECTING_USED:        return(EPS_RC_PROTECTING_USED);
        default:                                 return(EPS_RC_OK);
    }
}



/****************************************************************************/
/*  EPS management interface                                                */
/****************************************************************************/
static void cancel_reg(const u32               instance,
                       const eps_mgmt_mep_t    *const mep)
{
    (void)mep_eps_aps_register(mep->aps_mep, instance, MEP_EPS_TYPE_ELPS, FALSE);
    (void)mep_eps_sf_register(mep->w_mep, instance, MEP_EPS_TYPE_ELPS, FALSE);
    (void)mep_eps_sf_register(mep->p_mep, instance, MEP_EPS_TYPE_ELPS, FALSE);
}

void eps_mgmt_def_conf_get(vtss_eps_mgmt_def_conf_t  *const def_conf)
{
    vtss_eps_mgmt_def_conf_get(def_conf);
}

u32 eps_mgmt_instance_create(const u32                           instance,
                             const vtss_eps_mgmt_create_param_t  *const param)
{
    u32         rc;
    ulong       size;
    eps_conf_t  *blk;

    if (instance >= EPS_MGMT_CREATED_MAX)            return(EPS_RC_INVALID_PARAMETER);

    if ((rc = vtss_eps_mgmt_instance_create(instance, param)) == VTSS_EPS_RC_OK)
    {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE, &size)) != NULL) {
            blk->created[instance] = TRUE;
            blk->param[instance] = *param;
            blk->command[instance] = VTSS_EPS_MGMT_COMMAND_NONE;
            blk->mep[instance].w_mep = EPS_MEP_INST_INVALID;
            blk->mep[instance].p_mep = EPS_MEP_INST_INVALID;
            blk->mep[instance].aps_mep = EPS_MEP_INST_INVALID;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE);
        }

        if (param->domain == VTSS_EPS_MGMT_PORT)     /* This in order to inform MEP about port conversion in case of port protection */
            mep_port_protection_create((param->architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1) ? MEP_EPS_ARCHITECTURE_1P1 : MEP_EPS_ARCHITECTURE_1F1,  param->w_flow,  param->p_flow);

        CRIT_ENTER(crit_p);
        eps_data[instance].created = TRUE;
        eps_data[instance].w_mep = EPS_MEP_INST_INVALID;
        eps_data[instance].p_mep = EPS_MEP_INST_INVALID;
        eps_data[instance].aps_mep = EPS_MEP_INST_INVALID;
        CRIT_EXIT(crit_p);
    }

    return(rc_conv(rc));
}

u32 eps_mgmt_instance_delete(const u32     instance)
{
    u32                            rc, w_mep, p_mep, aps_mep;
    ulong                          size;
    eps_conf_t                     *blk;
    vtss_eps_mgmt_create_param_t   param;
    vtss_eps_mgmt_conf_t           conf;
    vtss_rc                        mep_rc = VTSS_RC_OK;

    if (instance >= EPS_MGMT_CREATED_MAX)            return(EPS_RC_INVALID_PARAMETER);

    rc = vtss_eps_mgmt_conf_get(instance, &param, &conf);
    if ((rc == VTSS_EPS_RC_OK) || (rc == VTSS_EPS_RC_NOT_CONFIGURED))
    if ((rc = vtss_eps_mgmt_instance_delete(instance)) == VTSS_EPS_RC_OK)
    {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE, &size)) != NULL) {
            blk->created[instance] = FALSE;
            blk->configured[instance] = FALSE;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE);
        }

        if (param.domain == VTSS_EPS_MGMT_PORT)     /* This in order to in form MEP about port conversion in case of port protection */
            mep_port_protection_delete(param.w_flow,  param.p_flow);

        CRIT_ENTER(crit_p);
        w_mep = eps_data[instance].w_mep;
        p_mep = eps_data[instance].p_mep;
        aps_mep = eps_data[instance].aps_mep;
        eps_data[instance].created = FALSE;
        CRIT_EXIT(crit_p);

        if (aps_mep != EPS_MEP_INST_INVALID) mep_rc += mep_eps_aps_register(aps_mep, instance, MEP_EPS_TYPE_ELPS, FALSE);
        if (w_mep != EPS_MEP_INST_INVALID) mep_rc += mep_eps_sf_register(w_mep, instance, MEP_EPS_TYPE_ELPS, FALSE);
        if (p_mep != EPS_MEP_INST_INVALID) mep_rc += mep_eps_sf_register(p_mep, instance, MEP_EPS_TYPE_ELPS, FALSE);
        if (mep_rc != VTSS_RC_OK)   return(EPS_RC_INVALID_PARAMETER);
    }

    return(rc_conv(rc));
}

u32 eps_mgmt_mep_set(const u32               instance,
                     const eps_mgmt_mep_t    *const mep)
{
    u32 i;
    vtss_eps_mgmt_create_param_t   param;
    vtss_eps_mgmt_conf_t           conf;

    if (instance >= EPS_MGMT_CREATED_MAX)                                             return(EPS_RC_INVALID_PARAMETER);
    if ((mep->w_mep >= MEP_INSTANCE_MAX) || (mep->p_mep >= MEP_INSTANCE_MAX) ||
        (mep->aps_mep >= MEP_INSTANCE_MAX))                                           return(EPS_RC_INVALID_PARAMETER);
    if (mep->w_mep == mep->p_mep)                                                     return(EPS_RC_W_P_SSF_MEP_EQUAL);
    if (mep->w_mep == mep->aps_mep)                                                   return(EPS_RC_INVALID_APS_MEP);
    if (vtss_eps_mgmt_conf_get(instance, &param, &conf) == VTSS_EPS_RC_NOT_CREATED)   return(EPS_RC_NOT_CREATED);

    CRIT_ENTER(crit_p);
    for (i=0; i<EPS_MGMT_CREATED_MAX; ++i)
    {
        if ((i != instance) && eps_data[i].created) {
            if ((eps_data[i].w_mep == mep->w_mep) || (eps_data[i].p_mep == mep->w_mep) || (eps_data[i].aps_mep == mep->w_mep))             {CRIT_EXIT(crit_p); return(EPS_RC_INVALID_W_MEP);}
            if (eps_data[i].w_mep == mep->p_mep)                                                                                           {CRIT_EXIT(crit_p); return(EPS_RC_INVALID_P_MEP);}
            if ((param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1) &&
                ((eps_data[i].p_mep == mep->p_mep) || (eps_data[i].aps_mep == mep->p_mep)))                                                {CRIT_EXIT(crit_p); return(EPS_RC_INVALID_P_MEP);}
            if ((eps_data[i].w_mep == mep->aps_mep) || (eps_data[i].p_mep == mep->aps_mep) || (eps_data[i].aps_mep == mep->aps_mep))       {CRIT_EXIT(crit_p); return(EPS_RC_INVALID_APS_MEP);}
        }
    }
    CRIT_EXIT(crit_p);

    if (mep_eps_aps_register(mep->aps_mep, instance, MEP_EPS_TYPE_ELPS, TRUE) != VTSS_RC_OK)   {cancel_reg(instance ,mep); return(EPS_RC_INVALID_APS_MEP);}
    if (mep_eps_sf_register(mep->w_mep, instance, MEP_EPS_TYPE_ELPS, TRUE) != VTSS_RC_OK)      {cancel_reg(instance ,mep); return(EPS_RC_INVALID_W_MEP);}
    if (mep_eps_sf_register(mep->p_mep, instance, MEP_EPS_TYPE_ELPS, TRUE) != VTSS_RC_OK)      {cancel_reg(instance ,mep); return(EPS_RC_INVALID_P_MEP);}

    CRIT_ENTER(crit_p);
    eps_data[instance].w_mep = mep->w_mep;
    eps_data[instance].p_mep = mep->p_mep;
    eps_data[instance].aps_mep = mep->aps_mep;
    CRIT_EXIT(crit_p);

    return(EPS_RC_OK);
}

u32 eps_mgmt_conf_set(const u32                    instance,
                      const vtss_eps_mgmt_conf_t   *const conf)
{
    u32         rc;
    ulong       size;
    eps_conf_t  *blk;

    if (instance >= EPS_MGMT_CREATED_MAX)            return(EPS_RC_INVALID_PARAMETER);

    if ((rc = vtss_eps_mgmt_conf_set(instance, conf)) == VTSS_EPS_RC_OK)
    {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE, &size)) != NULL) {
            blk->configured[instance] = TRUE;
            blk->conf[instance] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE);
        }
    }

    return(rc_conv(rc));
}

u32 eps_mgmt_conf_get(const u32                      instance,
                      vtss_eps_mgmt_create_param_t   *const param,
                      vtss_eps_mgmt_conf_t           *const conf,
                      eps_mgmt_mep_t                 *mep)
{
    if (instance >= EPS_MGMT_CREATED_MAX)            return(EPS_RC_INVALID_PARAMETER);

    CRIT_ENTER(crit_p);
    mep->w_mep = eps_data[instance].w_mep;
    mep->p_mep = eps_data[instance].p_mep;
    mep->aps_mep = eps_data[instance].aps_mep;
    CRIT_EXIT(crit_p);

    return(rc_conv(vtss_eps_mgmt_conf_get(instance, param, conf)));
}

u32 eps_mgmt_command_set(const u32                         instance,
                         const vtss_eps_mgmt_command_t     command)
{
    u32         rc;
    ulong       size;
    eps_conf_t  *blk;

    if (instance >= EPS_MGMT_CREATED_MAX)            return(EPS_RC_INVALID_PARAMETER);

    if ((rc = vtss_eps_mgmt_command_set(instance, command)) == VTSS_EPS_RC_OK)
    {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE, &size)) != NULL) {
            if (command == VTSS_EPS_MGMT_COMMAND_CLEAR)     blk->command[instance] = VTSS_EPS_MGMT_COMMAND_NONE;
            else                                            blk->command[instance] = command;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE);
        }
    }

    return(rc_conv(rc));
}

u32 eps_mgmt_command_get(const u32                   instance,
                         vtss_eps_mgmt_command_t     *const command)
{
    if (instance >= EPS_MGMT_CREATED_MAX)            return(EPS_RC_INVALID_PARAMETER);

    return(rc_conv(vtss_eps_mgmt_command_get(instance, command)));
}

u32 eps_mgmt_state_get(const u32                 instance,
                       vtss_eps_mgmt_state_t     *const state)
{
    if (instance >= EPS_MGMT_CREATED_MAX)            return(EPS_RC_INVALID_PARAMETER);

    return(rc_conv(vtss_eps_mgmt_state_get(instance, state)));
}





/****************************************************************************/
/*  EPS module interface - call out                                         */
/****************************************************************************/
void vtss_eps_tx_aps_info_set(const u32                       instance,
                              const vtss_eps_mgmt_domain_t    domain,
                              const u8                        *const aps_info)
{
    u32 rc, mep;
    u8  aps[VTSS_MEP_APS_DATA_LENGTH];

    CRIT_ENTER(crit_p);
    mep = eps_data[instance].aps_mep;
    CRIT_EXIT(crit_p);

    memcpy(aps, aps_info, VTSS_EPS_APS_DATA_LENGTH);    /* This is only to satify LINT as a copy of VTSS_EPS_APS_DATA_LENGTH bytes will be copied from 'aps' array */
    rc = mep_tx_aps_info_set(mep, instance, aps, FALSE);
    if (rc)        T_D("Error during APS tx set %u", instance);
}

void vtss_eps_signal_out(const u32                       instance,
                         const vtss_eps_mgmt_domain_t    domain)
{
    u32 rc, w_mep, p_mep, aps_mep;

    CRIT_ENTER(crit_p);
    w_mep = eps_data[instance].w_mep;
    p_mep = eps_data[instance].p_mep;
    aps_mep = eps_data[instance].aps_mep;
    CRIT_EXIT(crit_p);

    rc = mep_signal_in(w_mep, instance);
    rc += mep_signal_in(p_mep, instance);
    if (aps_mep != p_mep)   rc += mep_signal_in(aps_mep, instance);
    if (rc)        T_D("Error during MEP signal %u", instance);
}

void vtss_eps_port_protection_set(const u32         w_port,
                                  const u32         p_port,
                                  const BOOL        active)
{
    /* Change in port protection selector state */
    mep_port_protection_change(w_port, p_port, active);
}




/****************************************************************************/
/*  EPS module interface - call in                                          */
/****************************************************************************/
u32 eps_rx_aps_info_set(const u32    instance,
                        const u32    mep_inst,
                        const u8     aps[VTSS_EPS_APS_DATA_LENGTH])
{
    u32                     rc=EPS_RC_OK;
    vtss_eps_flow_type_t    flow=VTSS_EPS_FLOW_WORKING;

    CRIT_ENTER(crit_p);
    if (eps_data[instance].aps_mep == mep_inst)    flow = VTSS_EPS_FLOW_PROTECTING;
    else
    if (eps_data[instance].w_mep == mep_inst)      flow = VTSS_EPS_FLOW_WORKING;
    else
        rc = EPS_RC_INVALID_PARAMETER;
    CRIT_EXIT(crit_p);

    if ((rc==EPS_RC_OK) && (vtss_eps_rx_aps_info_set(instance, flow, aps) != VTSS_EPS_RC_OK))
        rc = EPS_RC_INVALID_PARAMETER;

    return(rc);
}


u32 eps_signal_in(const u32   instance,
                  const u32   mep_inst)
{
    u32 rc=EPS_RC_OK;

    CRIT_ENTER(crit_p);
    if ((eps_data[instance].aps_mep != mep_inst) && (eps_data[instance].w_mep != mep_inst) && (eps_data[instance].p_mep != mep_inst))
        rc = EPS_RC_INVALID_PARAMETER;
    CRIT_EXIT(crit_p);

    if ((rc==EPS_RC_OK) && (vtss_eps_signal_in(instance) != VTSS_EPS_RC_OK))
        rc = EPS_RC_INVALID_PARAMETER;

    return(rc);
}



u32 eps_sf_sd_state_set(const u32   instance,
                        const u32   mep_inst,
                        const BOOL  sf_state,
                        const BOOL  sd_state)
{
    u32                     rc=EPS_RC_OK;
    vtss_eps_flow_type_t    flow=VTSS_EPS_FLOW_WORKING;

    CRIT_ENTER(crit_p);
    if (eps_data[instance].p_mep == mep_inst)      flow = VTSS_EPS_FLOW_PROTECTING;
    else
    if (eps_data[instance].w_mep == mep_inst)      flow = VTSS_EPS_FLOW_WORKING;
    else
        rc = EPS_RC_INVALID_PARAMETER;
    CRIT_EXIT(crit_p);

    if ((rc==EPS_RC_OK) && (vtss_eps_sf_sd_state_set(instance, flow, sf_state, sd_state) != VTSS_EPS_RC_OK))
        rc = EPS_RC_INVALID_PARAMETER;

    return(rc);
}




/****************************************************************************/
/*  EPS platform interface                                                  */
/****************************************************************************/

void vtss_eps_crit_lock(void)
{
    CRIT_ENTER(crit);
}

void vtss_eps_crit_unlock(void)
{
    CRIT_EXIT(crit);
}

void vtss_eps_run(void)
{
    cyg_semaphore_post(&run_wait_sem);
}

void vtss_eps_timer_start(void)
{
    cyg_flag_setbits(&timer_wait_flag, FLAG_TIMER);
}

void vtss_eps_trace(const char  *const string,
                    const u32   param1,
                    const u32   param2,
                    const u32   param3,
                    const u32   param4)
{
    if ((param1 & 0xF0000000) == 0xF0000000)
        T_DG(TRACE_GRP_API, "%s - %u, %u, %u, %u", string, param1&~0xF0000000, param2, param3, param4);
    else
        T_D("%s - %u, %u, %u, %u", string, param1, param2, param3, param4);
}





/****************************************************************************/
/*  EPS Initialize module                                                   */
/****************************************************************************/
vtss_rc eps_init(vtss_init_data_t *data)
{
    u32           size, rc, i;
    vtss_isid_t   isid = data->isid;
    eps_conf_t    *blk;
    
    if (data->cmd == INIT_CMD_INIT)
    {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);
    switch (data->cmd)
    {
        case INIT_CMD_INIT:
            T_D("INIT");

            /* initialize critd */
            critd_init(&crit, "EPS Crit", VTSS_MODULE_ID_EPS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
            critd_init(&crit_p, "EPS Platform Crit", VTSS_MODULE_ID_EPS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

            cyg_semaphore_init(&run_wait_sem, 0);
            cyg_flag_init(&timer_wait_flag);
#ifdef VTSS_SW_OPTION_VCLI
            eps_cli_req_init();    
#endif

            cyg_thread_create(THREAD_HIGHEST_PRIO, 
                              eps_timer_thread, 
                              0, 
                              "EPS Timer", 
                              timer_thread_stack, 
                              THREAD_DEFAULT_STACK_SIZE,
                              &timer_thread_handle,
                              &timer_thread_block);
            cyg_thread_resume(timer_thread_handle);

            cyg_thread_create(THREAD_HIGHEST_PRIO,
                              eps_run_thread, 
                              0, 
                              "EPS State Machine", 
                              run_thread_stack, 
                              THREAD_DEFAULT_STACK_SIZE,
                              &run_thread_handle,
                              &run_thread_block);
            cyg_thread_resume(run_thread_handle);

            rc = vtss_eps_init(10);
            if (rc)        T_D("Error during init");

            for (i=0; i<EPS_MGMT_CREATED_MAX; ++i)
                memset(&eps_data[i], 0, sizeof(eps_data_t));

            CRIT_EXIT(crit);
            CRIT_EXIT(crit_p);

#ifdef VTSS_SW_OPTION_ICFG
            // Initialize ICLI "show running" configuration
            VTSS_RC(eps_icfg_init());
#endif
            break;
        case INIT_CMD_START:
            T_D("START");
            break;
        case INIT_CMD_CONF_DEF:
            T_D("CONF_DEF");
            if (isid == VTSS_ISID_LOCAL)
            {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
                if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE, &size)) != NULL) {
                    set_conf_to_default(blk);
                    apply_configuration(blk);
                    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE);
                }
#else
                eps_restore_to_default();
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            }
            break;
        case INIT_CMD_MASTER_UP:
            T_D("MASTER_UP");

            if (misc_conf_read_use()) {
                T_D("New size %zu\n", sizeof(*blk));
                if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE, &size)) == NULL || size != sizeof(*blk)) {
                    T_W("conf_sec_open failed or size mismatch, creating defaults");
                    blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE, sizeof(*blk));
                    if (blk != NULL) {
                        blk->version = 0;
                        set_conf_to_default(blk);
                    } else {
                        T_W("conf_sec_create failed");
                        break;
                    }
                }
                apply_configuration(blk);
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_EPS_CONF_TABLE);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            } else {
                eps_restore_to_default();
            }
            break;
        case INIT_CMD_MASTER_DOWN:
            T_D("MASTER_DOWN");
            break;
        case INIT_CMD_SWITCH_ADD:
            T_D("SWITCH_ADD");
            break;
        case INIT_CMD_SWITCH_DEL:
            T_D("SWITCH_DEL");
            break;
        case INIT_CMD_SUSPEND_RESUME:
            T_D("SUSPEND_RESUME");
            break;
        default:
            break;
    }

    T_D("exit");
    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
