/*

 Vitesse Interrupt module software.

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

#include "critd_api.h"
#include "interrupt.h"
#include "interrupt_api.h"
#include "port_custom_api.h"
#include "port_api.h"  /* For port_phy_wait_until_ready() */
#include "vtss_api_if_api.h" /* For vtss_api_if_chip_count() */

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Structure for global variables */
static critd_t          crit;
static cyg_flag_t       wait_flag;

static cyg_handle_t     thread_handle;
static cyg_thread       thread_block;
static char             thread_stack[THREAD_DEFAULT_STACK_SIZE];


#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "interrupt",
    .descr     = "interrupt module."
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_IRQ0] = {
        .name      = "IRQ0",
        .descr     = "",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_IRQ1] = {
        .name      = "IRQ1",
        .descr     = "",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define CRIT_ENTER() critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CRIT_EXIT()  critd_exit( &crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define CRIT_ENTER() critd_enter(&crit)
#define CRIT_EXIT()  critd_exit( &crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Module Interface                                                        */
/****************************************************************************/
#define HOOK_MAX 10       /* max number of function_hook on any sources */

typedef struct
{
    int                            hook_no;
    vtss_interrupt_priority_t      priority[HOOK_MAX];
    vtss_interrupt_function_hook_t function_hook[HOOK_MAX];
} source_t;

static source_t *source_get(vtss_interrupt_source_t source_id);

vtss_rc  vtss_interrupt_source_hook_set(vtss_interrupt_function_hook_t function_hook,
                                        vtss_interrupt_source_t        source_id,
                                        vtss_interrupt_priority_t      priority)
{
    vtss_rc  ret_value;
    source_t *source;
    int      i, spot_in;

    if ((function_hook == 0) || (source_id >= INTERRUPT_SOURCE_LAST) || (priority >= INTERRUPT_PRIORITY_LAST)) {
        return VTSS_INVALID_PARAMETER;
    }

    /* get the source struct */
    source = source_get(source_id);
    if (source == 0) {
        return VTSS_INVALID_PARAMETER;
    }
    if (source->hook_no >= HOOK_MAX) {
        return VTSS_UNSPECIFIED_ERROR; /* function_hook list is full */
    }

    /* check if function_hook in prioritized list allready */
    for (i = 0; i < source->hook_no; i++) {
        if (source->function_hook[i] == function_hook) {
            return VTSS_INVALID_PARAMETER;
        }
    }

    CRIT_ENTER();
    /* no return within critical zone */
    ret_value = VTSS_OK;

    /* find spot to put function_hook in prioritized list */
    for (i = 0; i < source->hook_no; i++) {
        if (source->priority[i] < priority) {
            break;
        }
    }

    /* save spot to put function hook in */
    spot_in = i;

    /* move function hooks after spot_in */
    for (i = source->hook_no; i > spot_in; i--) {
        source->function_hook[i] = source->function_hook[i - 1];
    }

    /* insert new function hook */
    source->function_hook[i] = function_hook;
    source->priority[i] = priority;
    source->hook_no++;
    CRIT_EXIT();

    /* enable interrupt */
    interrupt_source_enable(source_id);

    return(ret_value);
}

vtss_rc vtss_interrupt_source_hook_clear(vtss_interrupt_function_hook_t function_hook,
                                         vtss_interrupt_source_t        source_id)
{
    source_t *source;
    u32      i, spot_out;

    if (function_hook == 0) {
        return VTSS_INVALID_PARAMETER;
    }

    /* get the source struct */
    source = source_get(source_id);
    if (source == 0) {
        return VTSS_INVALID_PARAMETER;
    }

    CRIT_ENTER();
    
    /* find function_hook in prioritized list */
    for (i = 0; i < source->hook_no; i++) {
        if (source->function_hook[i] == function_hook) {
            break;
        }
    }

    /* save spot to take function hook out */
    spot_out = i;

    if (spot_out == source->hook_no) {
      // No such function found
      return VTSS_OK;
    }

    /* move function hooks after spot_out */
    for (i = spot_out; i < source->hook_no - 1; i++) {
        source->function_hook[i] = source->function_hook[i + 1];
    }

    source->hook_no--;
    CRIT_EXIT();

    return VTSS_OK;
}

/****************************************************************************/
/*  Internal interface                                                      */
/****************************************************************************/
void interrupt_device_signal(u32 flags)
{
    cyg_flag_setbits(&wait_flag, flags);
}

void interrupt_signal_source(vtss_interrupt_source_t source_id,
                             u32                     instance_no)
{
    source_t                       *source;
    u32                            i, hook_no;
    vtss_interrupt_function_hook_t function_hook[HOOK_MAX];

    /* Empty the  function_hook list */
    CRIT_ENTER();
    source = source_get(source_id);
    if (source != 0) {
        for (i=0; (i<source->hook_no) && (i<HOOK_MAX); i++) {
            function_hook[i] = source->function_hook[i];
        }
        hook_no = source->hook_no;
        source->hook_no = 0;
    }
    else
        hook_no = 0;
    CRIT_EXIT();

    /* call all function hooks for the given source */
    if (hook_no == HOOK_MAX) {
        return;
    }
    for (i = 0; i < hook_no; i++) {
        if (function_hook[i] != 0) {
            function_hook[i](source_id, instance_no);
        }
    }
}

/****************************************************************************/
/*  Various local definitions                                               */
/****************************************************************************/

typedef struct
{
    source_t los;
    source_t flnk;
    source_t ams;
    source_t locs;
    source_t rx_lol;
    source_t lopc;
    source_t receive_fault;
    source_t module;
    source_t tx_lol;
    source_t hi_ber;
    source_t fos;
    source_t losx;
    source_t lol;
    source_t i2c;
    source_t loc;
    source_t ptp_sync;
    source_t ptp_ext_sync;
    source_t ptp_ext_1_sync;
    source_t ptp_clk_adj;
    source_t ptp_tstamp;
    source_t ptp_top_isr;
    source_t ingr_engine;
    source_t ingr_pream;
    source_t ingr_fcs;
    source_t egr_engine;
    source_t egr_fcs;
    source_t egr_timestamp;
    source_t egr_fifo;
    source_t ewis_sef;
    source_t ewis_fplm;
    source_t ewis_fais;
    source_t ewis_lof;
    source_t ewis_rdil;
    source_t ewis_aisl;
    source_t ewis_lcdp;
    source_t ewis_plmp;
    source_t ewis_aisp;
    source_t ewis_lopp;
    source_t ewis_uneqp;
    source_t ewis_ewuneqp;
    source_t ewis_ferdip;
    source_t ewis_reil;
    source_t ewis_reip;
    source_t ewis_b1_nz;
    source_t ewis_b2_nz;
    source_t ewis_b3_nz;
    source_t reil_nz;
    source_t reip_nz;
    source_t b1_thresh;
    source_t b2_thresh;
    source_t b3_thresh;
    source_t reil_thresh;
    source_t reip_thresh;
    source_t sgpio_push_button;
} sources_t;
static sources_t    sources;


static source_t* source_get(vtss_interrupt_source_t     source_id)
{
    switch (source_id)
    {
        case INTERRUPT_SOURCE_LOS:                     return(&sources.los);
        case INTERRUPT_SOURCE_FLNK:                    return(&sources.flnk);
        case INTERRUPT_SOURCE_AMS:                     return(&sources.ams);
        case INTERRUPT_SOURCE_RX_LOL:                  return(&sources.rx_lol);
        case INTERRUPT_SOURCE_LOPC:                    return(&sources.lopc);
        case INTERRUPT_SOURCE_RECEIVE_FAULT:           return(&sources.receive_fault);
        case INTERRUPT_SOURCE_MODULE_STAT:             return(&sources.module);
        case INTERRUPT_SOURCE_TX_LOL:                  return(&sources.tx_lol);
        case INTERRUPT_SOURCE_HI_BER:                  return(&sources.hi_ber);

        case INTERRUPT_SOURCE_LOCS:                    return(&sources.locs);
        case INTERRUPT_SOURCE_FOS:                     return(&sources.fos);
        case INTERRUPT_SOURCE_LOSX:                    return(&sources.losx);
        case INTERRUPT_SOURCE_LOL:                     return(&sources.lol);

        case INTERRUPT_SOURCE_I2C:                     return(&sources.i2c);
        case INTERRUPT_SOURCE_LOC:                     return(&sources.loc);
        case INTERRUPT_SOURCE_SYNC:                    return(&sources.ptp_sync);
        case INTERRUPT_SOURCE_EXT_SYNC:                return(&sources.ptp_ext_sync);
        case INTERRUPT_SOURCE_EXT_1_SYNC:                return(&sources.ptp_ext_1_sync);
        case INTERRUPT_SOURCE_CLK_ADJ:                 return(&sources.ptp_clk_adj);
        case INTERRUPT_SOURCE_CLK_TSTAMP:              return(&sources.ptp_tstamp);
        case INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG:      return(&sources.ptp_top_isr);

        case INTERRUPT_SOURCE_INGR_ENGINE_ERR:         return(&sources.ingr_engine);
        case INTERRUPT_SOURCE_INGR_RW_PREAM_ERR:       return(&sources.ingr_pream);
        case INTERRUPT_SOURCE_INGR_RW_FCS_ERR:         return(&sources.ingr_fcs);
        case INTERRUPT_SOURCE_EGR_ENGINE_ERR:          return(&sources.egr_engine);
        case INTERRUPT_SOURCE_EGR_RW_FCS_ERR:          return(&sources.egr_fcs);
        case INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED:  return(&sources.egr_timestamp);
        case INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW:       return(&sources.egr_fifo);

        case INTERRUPT_SOURCE_EWIS_SEF_EV:             return(&sources.ewis_sef);
        case INTERRUPT_SOURCE_EWIS_FPLM_EV:            return(&sources.ewis_fplm);
        case INTERRUPT_SOURCE_EWIS_FAIS_EV:            return(&sources.ewis_fais);
        case INTERRUPT_SOURCE_EWIS_LOF_EV:             return(&sources.ewis_lof);
        case INTERRUPT_SOURCE_EWIS_RDIL_EV:            return(&sources.ewis_rdil);
        case INTERRUPT_SOURCE_EWIS_AISL_EV:            return(&sources.ewis_aisl);
        case INTERRUPT_SOURCE_EWIS_LCDP_EV:            return(&sources.ewis_lcdp);
        case INTERRUPT_SOURCE_EWIS_PLMP_EV:            return(&sources.ewis_plmp);
        case INTERRUPT_SOURCE_EWIS_AISP_EV:            return(&sources.ewis_aisp);
        case INTERRUPT_SOURCE_EWIS_LOPP_EV:            return(&sources.ewis_lopp);
        case INTERRUPT_SOURCE_EWIS_UNEQP_EV:           return(&sources.ewis_uneqp);
        case INTERRUPT_SOURCE_EWIS_FEUNEQP_EV:         return(&sources.ewis_ewuneqp);
        case INTERRUPT_SOURCE_EWIS_FERDIP_EV:          return(&sources.ewis_ferdip);
        case INTERRUPT_SOURCE_EWIS_REIL_EV:            return(&sources.ewis_reil);
        case INTERRUPT_SOURCE_EWIS_REIP_EV:            return(&sources.ewis_reip);
        case INTERRUPT_SOURCE_EWIS_B1_NZ_EV:           return(&sources.ewis_b1_nz);
        case INTERRUPT_SOURCE_EWIS_B2_NZ_EV:           return(&sources.ewis_b2_nz);
        case INTERRUPT_SOURCE_EWIS_B3_NZ_EV:           return(&sources.ewis_b3_nz);
        case INTERRUPT_SOURCE_EWIS_REIL_NZ_EV:         return(&sources.reil_nz);
        case INTERRUPT_SOURCE_EWIS_REIP_NZ_EV:         return(&sources.reip_nz);
        case INTERRUPT_SOURCE_EWIS_B1_THRESH_EV:       return(&sources.b1_thresh);
        case INTERRUPT_SOURCE_EWIS_B2_THRESH_EV:       return(&sources.b2_thresh);
        case INTERRUPT_SOURCE_EWIS_B3_THRESH_EV:       return(&sources.b3_thresh);
        case INTERRUPT_SOURCE_EWIS_REIL_THRESH_EV:     return(&sources.reil_thresh);
        case INTERRUPT_SOURCE_EWIS_REIP_THRESH_EV:     return(&sources.reip_thresh);

        case INTERRUPT_SOURCE_SGPIO_PUSH_BUTTON:       return(&sources.sgpio_push_button);

        default: return(0);
    }
}

static void interrupt_thread(cyg_addrword_t data)
{
    BOOL              pending, interrupt, onesec;
    cyg_flag_value_t  flag_value;
    vtss_mtimer_t     onesec_timer;

    // This will block this thread from running further until the PHYs are initialized.
    port_phy_wait_until_ready();

    VTSS_MTIMER_START(&onesec_timer, 900);

    while (1) {
        flag_value = 0xFFFFFFFF;
        flag_value = cyg_flag_timed_wait(&wait_flag, flag_value, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, cyg_current_time() + VTSS_OS_MSEC2TICK(1000));

        interrupt = (flag_value != 0);
        onesec = FALSE;
        pending = FALSE;

        if (VTSS_MTIMER_TIMEOUT(&onesec_timer)) {
            /* The 'onesec' indication is calculated to be used for polling of devices */
            onesec = TRUE;
            VTSS_MTIMER_START(&onesec_timer, 900);
        }

        // Figure out exactly which sources caused the interrupt (the physical interrupt may be shared), and call listeners back
        interrupt_device_poll(flag_value, interrupt, onesec, &pending);

        // Re-enable the physical interrupt.
        interrupt_device_enable(flag_value, pending);
    } /* while (1) */
}

/* Initialize module */
vtss_rc interrupt_init(vtss_init_data_t *data)
{
    if (vtss_api_if_chip_count() != 2) {
        // For JR-48 we must have this module included (for Packet Rx) whether or not some other board features are supported
        if(!(vtss_board_features() & (VTSS_BOARD_FEATURE_AMS | VTSS_BOARD_FEATURE_LOS))) {
            return VTSS_RC_OK;
        }
    }

    switch (data->cmd) {
    case INIT_CMD_INIT:
        // Initialize trace...
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        // ...our own mutex
        critd_init(&crit, "Interrupt", VTSS_MODULE_ID_INTERRUPT, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        // ...a thread wait flag
        cyg_flag_init(&wait_flag);

        // ...no user modules are currently hooking interrupts
        memset(&sources, 0, sizeof(sources));

        // ...our thread
        cyg_thread_create(THREAD_HIGHEST_PRIO,
                          interrupt_thread,
                          0,
                          "INTERRUPT THREAD",
                          thread_stack,
                          sizeof(thread_stack),
                          &thread_handle,
                          &thread_block);

        cyg_thread_resume(thread_handle);

        // ...and we're ready to go and accept registrations.
        CRIT_EXIT();
        break;
    case INIT_CMD_START:
        T_D("INIT_CMD_START interrupt");
        // ...hook the required interrupts from the OS.
        interrupt_board_init();
        break;

    case INIT_CMD_SWITCH_ADD:
        break;

    default:
        break;
    }

    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
