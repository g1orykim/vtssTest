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
 * \file vtss_aneg_api.h
 * \brief  ANEG  API
 */

#ifndef _VTSS_ANEG_API_H_
#define _VTSS_ANEG_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_ANEG)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */
/** \brief aneg config data   */
typedef struct vtss_aneg_cfg_t{
   BOOL  aneg_enable;
   u16   parallel_det_wait_time;
   BOOL  ctrl_ob;
  
}vtss_aneg_cfg_t;


/** \brief aneg status   */
typedef struct vtss_aneg_status_t{
   BOOL aneg_complete;
   BOOL lp_aneg_able;
   BOOL incompatible_link;
}vtss_aneg_status_t;


/** \brief aneg advertisement for ability   */
typedef struct vtss_aneg_ability_t{
  u8     sel_field;
  u8     echoed_nonce;
  u8     pause;
  BOOL   rf;
  BOOL   np;
  u8     tx_nonce;
  BOOL   cap_1gkx;
  BOOL   cap_10gkx4;
  BOOL   cap_10gkr;
  u8     lsb_reserved;
}vtss_aneg_ability_t;


/** \brief aneg advertisement for ability   */
typedef struct vtss_aneg_next_page_t{
   u16    next_page_mlsb;
   u32    next_page_lsb;
}vtss_aneg_next_page_t;


/** \brief aneg state - container for vtss_state    */
typedef struct vtss_aneg_state_t{
    vtss_aneg_cfg_t     aneg_cfg;
    vtss_aneg_status_t  aneg_status;
    vtss_aneg_ability_t aneg_ability;
    /* atleast one next page is supported */   
    vtss_aneg_next_page_t  next_page;
}vtss_aneg_state_t;



/** \brief  aneg events */
typedef enum vtss_aneg_events_t {
    parallel_defect_fault = (1 << 0),
    incompatible_link     = (1 << 1),
    page_recvd            = (1 << 2),
    aneg_complete         = (1 << 3)    
} vtss_aneg_events_t;

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief Set ANEG configuration.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       ANEG configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_set_config(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          const vtss_aneg_cfg_t *const cfg
                          );


/**
 * \brief Get ANEG configuration.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [OUT]      ANEG configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_get_config(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_aneg_cfg_t *const cfg
                          );


/**
 * \brief Set ANEG ability.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param ability [IN]   ANEG ability.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_set_ability(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          const vtss_aneg_ability_t *const ability
                          );
                          

/**
 * \brief Get ANEG ability.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param ability [OUT]  ANEG ability.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_get_ability(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_aneg_ability_t *const ability
                          );
                          

/**
 * \brief Get ANEG next page.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param next_page [OUT]   ANEG next page value.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_get_next_page(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_aneg_next_page_t *const next_page
                          );                          
                          
                          
/**
 * \brief Set ANEG next page.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param next_page [IN]    ANEG next page value.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_set_next_page(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          const vtss_aneg_next_page_t *const next_page
                          );          
                          

/**
 * \brief Enable ANEG.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param enable [IN]    TRUE => Enable, FALSE => Disable.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_set_enable(const vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          BOOL enable
                          );                                          
                          

/**
 * \brief Reset ANEG.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_reset(const vtss_inst_t inst,
                          vtss_port_no_t port_no                          
                          );                          
                          
/**
 * \brief Restart ANEG.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_restart(const vtss_inst_t inst,
                          vtss_port_no_t port_no                          
                          );                                                    
/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief Get ANEG events.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param events [OUT]      pointer to event status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_events_get(vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_aneg_events_t *const events);
                          
                          
/**
 * \brief Get ANEG status.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param status [OUT]      pointer to ANEG status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_get_status(vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_aneg_status_t *const status);                          
                          
/**
 * \brief Gt ANEG lp ability.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param lp_ability [OUT]  pointer to ANEG lp ability status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_aneg_get_lp_ability(vtss_inst_t inst,
                          vtss_port_no_t port_no,
                          vtss_aneg_ability_t *const lp_ability);
/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

#endif /* VTSS_FEATURE_ANEG */

#endif /* _VTSS_ANEG_API_H_ */
