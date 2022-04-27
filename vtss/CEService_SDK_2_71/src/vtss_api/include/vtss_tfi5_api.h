/*

 Vitesse API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

/**
 * \file vtss_tfi5_api.h
 * \brief  TFI5 API
 */

#ifndef _VTSS_TFI5_API_H_
#define _VTSS_TFI5_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_TFI5)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */
#define TFI5_NUM_LANES 4 

 
/** \brief tfi5 config data   */
typedef struct vtss_tfi5_cfg_t{
    BOOL enable;    
    BOOL dual_rate;
    BOOL rx_to_tx_loopback;

    struct {
        BOOL b1_passthru;
        BOOL fixed_j0z0;        
    }tx_config;     

}vtss_tfi5_cfg_t;


/** \brief tfi5 status   */
typedef struct vtss_tfi5_status_t{    
    BOOL OOA[TFI5_NUM_LANES];
    BOOL OOF[TFI5_NUM_LANES];
    BOOL LOS[TFI5_NUM_LANES];
    BOOL LOF;    
}vtss_tfi5_status_t;

/** \brief tfi5 state - container for vtss_state    */
typedef struct vtss_tfi5_state_t{
    vtss_tfi5_cfg_t     tfi5_cfg;
    vtss_tfi5_status_t   tfi5_status;    
}vtss_tfi5_state_t;


/** \brief  tfi5 events */
typedef enum vtss_tfi5_events_t {       
       dummy_tfi5,
} vtss_tfi5_events_t;

/** \brief  tfi5 loopback */
typedef enum vtss_tfi5_loopback_t {       
    tfi5_rx_to_tx,
} vtss_tfi5_loopback_t;

/** \brief  tfi5 error insertion */
typedef enum vtss_tfi5_err_ins_t{
    tfi5_ais,
    tfi5_b1,
    tfi5_a1a2
}vtss_tfi5_err_ins_t;


/* ================================================================= *
 *  Defects/Events
 * ================================================================= */

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief   .
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_tfi5_set_config(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          const vtss_tfi5_cfg_t *const cfg
                          );


/**
 * \brief   .
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_tfi5_get_config(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_tfi5_cfg_t *const cfg
                          );
             
             
         
                          
/**
 * \brief   .
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_tfi5_set_enable(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          BOOL enable
                          );                          


/**
 * \brief   .
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_tfi5_set_reset(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          BOOL enable
                          ); 

/**
 * \brief   .
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_tfi5_set_loopback(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          BOOL enable,
                          vtss_tfi5_loopback_t dir
                          );                          


/**
 * \brief   .
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_tfi5_error_insert(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          BOOL enable,
                          vtss_tfi5_err_ins_t err
                          );             
                                         
                          

                          
                                            
/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief .
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param def [OUT]      pointer to defect status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_tfi5_events_get(vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_tfi5_events_t *const events);
                          
                          
/**
 * \brief .
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param def [OUT]      pointer to defect status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_tfi5_get_status(vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_tfi5_status_t *const status);
                          

/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

#endif /* VTSS_FEATURE_TFI5 */

#endif /* _VTSS_TFI5_API_H_ */
