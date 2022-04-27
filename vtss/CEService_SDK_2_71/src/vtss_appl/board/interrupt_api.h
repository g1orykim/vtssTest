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

#ifndef _VTSS_INTERRUPT_API_H_
#define _VTSS_INTERRUPT_API_H_

#include "vtss_types.h"
#include "main_types.h"


/****************************************************************************/
/*  Module Interface                                                        */
/****************************************************************************/

/* Initialize module */
extern vtss_rc interrupt_init(vtss_init_data_t *data);


typedef enum
{
    INTERRUPT_SOURCE_LOS,                    /* Loss of signal - link down on PHY - loss of SFP optical signal */
    INTERRUPT_SOURCE_FLNK,                   /* Fast link failure detect on PHY (Enzo - Atom) */
    INTERRUPT_SOURCE_AMS,                    /* Automatic Media-Sence on PHY */

    INTERRUPT_SOURCE_RX_LOL,                 /* RX_LOL on 10G PHY 8484 8487 8488 */
    INTERRUPT_SOURCE_LOPC,                   /* LOPC on 10G PHY 8484 8487 8488 */
    INTERRUPT_SOURCE_RECEIVE_FAULT,          /* RECEIVE_FAULT on 10G PHY 8484 8487 8488 */
    INTERRUPT_SOURCE_MODULE_STAT,            /* GPIO pin state being driven by optics module on 10G PHY 8484 8487 8488 */
    INTERRUPT_SOURCE_TX_LOL,                 /* TX_LOL on 10G PHY 8484 8487 8488 */
    INTERRUPT_SOURCE_HI_BER,                 /* HI_BER on 10G PHY 8484 8487 8488 */

    INTERRUPT_SOURCE_LOCS,                   /* Loss of clock source on SYNCE Clock controller */
    INTERRUPT_SOURCE_FOS,                    /* Frequency offset alarm on SYNCE Clock controller*/
    INTERRUPT_SOURCE_LOSX,                   /* Loss of signal on XA/XB SYNCE Clock controller */
    INTERRUPT_SOURCE_LOL,                    /* PLL Loss Of Lock SYNCE Clock controller */

    INTERRUPT_SOURCE_I2C,                    /* I2C interrupt from POE controlller */
    INTERRUPT_SOURCE_LOC,                    /* OAM Loss of continuity */
                                    
    INTERRUPT_SOURCE_SYNC,                   /* PTP Synchronization pulse update */
    INTERRUPT_SOURCE_EXT_SYNC,               /* PTP External Synchronization input */
    INTERRUPT_SOURCE_EXT_1_SYNC,             /* PTP External Synchronization 1 input (Serval only) */
    INTERRUPT_SOURCE_CLK_ADJ,                /* PTP Clock adjustment updated */
    INTERRUPT_SOURCE_CLK_TSTAMP,             /* PTP Clock timestamp is updated */

    INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG,     /* Zarlink DCO Phase Word and Local System Time have been sampled */

    INTERRUPT_SOURCE_INGR_ENGINE_ERR,        /* TS More than one engine find match */
    INTERRUPT_SOURCE_INGR_RW_PREAM_ERR,      /* TS Preamble too short to append timestamp */
    INTERRUPT_SOURCE_INGR_RW_FCS_ERR,        /* TS FCS error in ingress */
    INTERRUPT_SOURCE_EGR_ENGINE_ERR,         /* TS More than one engine find match */
    INTERRUPT_SOURCE_EGR_RW_FCS_ERR,         /* TS FCS error in egress */
    INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED, /* TS Timestamp captured in Tx TSFIFO */
    INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW,      /* TS Tx TSFIFO overflow */

    INTERRUPT_SOURCE_EWIS_SEF_EV,            /**< SEF has changed state */
    INTERRUPT_SOURCE_EWIS_FPLM_EV,           /**< far-end (PLM-P) / (LCDP) */
    INTERRUPT_SOURCE_EWIS_FAIS_EV,           /**< far-end (AIS-P) / (LOP) */
    INTERRUPT_SOURCE_EWIS_LOF_EV,            /**< Loss of Frame (LOF) */
    INTERRUPT_SOURCE_EWIS_RDIL_EV,           /**< Line Remote Defect Indication (RDI-L) */
    INTERRUPT_SOURCE_EWIS_AISL_EV,           /**< Line Alarm Indication Signal (AIS-L) */
    INTERRUPT_SOURCE_EWIS_LCDP_EV,           /**< Loss of Code-group Delineation (LCD-P) */
    INTERRUPT_SOURCE_EWIS_PLMP_EV,           /**< Path Label Mismatch (PLMP) */
    INTERRUPT_SOURCE_EWIS_AISP_EV,           /**< Path Alarm Indication Signal (AIS-P) */
    INTERRUPT_SOURCE_EWIS_LOPP_EV,           /**< Path Loss of Pointer (LOP-P) */

    INTERRUPT_SOURCE_EWIS_UNEQP_EV,          /**< Unequiped Path (UNEQ-P) */
    INTERRUPT_SOURCE_EWIS_FEUNEQP_EV,        /**< Far-end Unequiped Path (UNEQ-P) */
    INTERRUPT_SOURCE_EWIS_FERDIP_EV,         /**< Far-end Path Remote Defect Identifier (RDI-P) */
    INTERRUPT_SOURCE_EWIS_REIL_EV,           /**< Line Remote Error Indication (REI-L) */
    INTERRUPT_SOURCE_EWIS_REIP_EV,           /**< Path Remote Error Indication (REI-P) */

    INTERRUPT_SOURCE_EWIS_B1_NZ_EV,          /**< PMTICK B1 BIP (B1_ERR_CNT) not zero */
    INTERRUPT_SOURCE_EWIS_B2_NZ_EV,          /**< PMTICK B2 BIP (B1_ERR_CNT) not zero */
    INTERRUPT_SOURCE_EWIS_B3_NZ_EV,          /**< PMTICK B3 BIP (B1_ERR_CNT) not zero */
    INTERRUPT_SOURCE_EWIS_REIL_NZ_EV,        /**< PMTICK REI-L (REIL_ERR_CNT) not zero */
    INTERRUPT_SOURCE_EWIS_REIP_NZ_EV,        /**< PMTICK REI-P (REIP_ERR_CNT) not zero */
    INTERRUPT_SOURCE_EWIS_B1_THRESH_EV,      /**< B1_THRESH_ERR */
    INTERRUPT_SOURCE_EWIS_B2_THRESH_EV,      /**< B2_THRESH_ERR */
    INTERRUPT_SOURCE_EWIS_B3_THRESH_EV,      /**< B3_THRESH_ERR */
    INTERRUPT_SOURCE_EWIS_REIL_THRESH_EV,    /**< REIL_THRESH_ERR */
    INTERRUPT_SOURCE_EWIS_REIP_THRESH_EV,    /**< REIp_THRESH_ERR */

    INTERRUPT_SOURCE_SGPIO_PUSH_BUTTON,      /**< Serial General Purpose Input Output Push Button*/

    INTERRUPT_SOURCE_LAST                    /* No source after this */
} vtss_interrupt_source_t;

typedef enum
{
    INTERRUPT_PRIORITY_NORMAL, 
    INTERRUPT_PRIORITY_CLOCK,  
    INTERRUPT_PRIORITY_POE,    
    INTERRUPT_PRIORITY_PROTECT,
    INTERRUPT_PRIORITY_HIGHEST,
    INTERRUPT_PRIORITY_LAST      /* No priority after this */
} vtss_interrupt_priority_t;

typedef void (* vtss_interrupt_function_hook_t)(vtss_interrupt_source_t     source_id,
                                                u32                         instance_id);


/******************************************************************************
 * Description: Assign a function_hook to a source instance - in a device.
 * When hook is assigned function_hook is inserted in a prioritized list and the interrupt on source is enabled.
 * When an interrupt occur on the source, the interrupt is disabled and all function_hook are removed from list and called in a prioritized order.
 * Allways re-assign hook before getting the actual status on the source (not to miss an interrupt).
 * Remember that whatever code excecuted in this function_hook is excecuted on interrupt thread priority.
 * The code in the function_hook could be merely registration of interrupt and signal of a semaphore to activate own thread
 *
 * \param function_hook (input): Call back function pointer
 * \param source_id (input): Identification of source.
 * \param instance_id (input): Identification of instance.
 * \param priority (input): Call back priority.
 *
 * \return : Return code.    VTSS_INVALID_PARAMETER
 *                           VTSS_UNSPECIFIED_ERROR       means function_hook list is full on this source
 *                           VTSS_OK
 ******************************************************************************/
vtss_rc  vtss_interrupt_source_hook_set(vtss_interrupt_function_hook_t function_hook,
                                        vtss_interrupt_source_t        source_id,
                                        vtss_interrupt_priority_t      priority);

vtss_rc  vtss_interrupt_source_hook_clear(vtss_interrupt_function_hook_t function_hook,
                                          vtss_interrupt_source_t        source_id);

#if defined(VTSS_SW_OPTION_POE)
void interrupt_poe_init_done(void);
#endif

#endif /* _VTSS_INTERRUPT_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
