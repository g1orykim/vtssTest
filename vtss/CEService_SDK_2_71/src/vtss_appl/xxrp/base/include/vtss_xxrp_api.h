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

#ifndef _VTSS_XXRP_API_H_
#define _VTSS_XXRP_API_H_

#include "vtss_xxrp_types.h"
#include "../src/vtss_xxrp_applications.h"
#include "../src/vtss_xxrp_mad.h"
#include "../src/vtss_xxrp_map.h"
#include "../src/vtss_xxrp_debug.h"

/**
 * \file vtss_xxrp_api.h
 * \brief XXRP main API header file
 *
 * This file contain the definitions of API functions and associated types.
 *
 */

/***************************************************************************************************
 * XXRP Module error defintions.
 **************************************************************************************************/
#define VTSS_XXRP_RC_OK                         0
#define VTSS_XXRP_RC_INVALID_PARAMETER          1
#define VTSS_XXRP_RC_NOT_ENABLED                2
#define VTSS_XXRP_RC_ALREADY_CONFIGURED         3
#define VTSS_XXRP_RC_NO_MEMORY                  4
#define VTSS_XXRP_RC_NOT_SUPPORTED              5
#define VTSS_XXRP_RC_NO_SUFFIFIENT_MEMORY       6
#define VTSS_XXRP_RC_NOT_FOUND                  7
#define VTSS_XXRP_RC_UNKNOWN                    8

/***************************************************************************************************
 * Timer default and valid range definitions.
 **************************************************************************************************/
#define VTSS_MRP_JOIN_TIMER_DEF                 20 /* CentiSeconds */
#define VTSS_MRP_JOIN_TIMER_MIN                  0 /* CentiSeconds */
#define VTSS_MRP_JOIN_TIMER_MAX                 20 /* CentiSeconds */
#define VTSS_MRP_LEAVE_TIMER_DEF                60 /* CentiSeconds */
#define VTSS_MRP_LEAVE_TIMER_MIN                60 /* CentiSeconds */
#define VTSS_MRP_LEAVE_TIMER_MAX               300 /* CentiSeconds */
#define VTSS_MRP_LEAVEALL_TIMER_DEF           1000 /* CentiSeconds */
#define VTSS_MRP_LEAVEALL_TIMER_MIN           1000 /* CentiSeconds */
#define VTSS_MRP_LEAVEALL_TIMER_MAX           5000 /* CentiSeconds */
#define VTSS_MRP_PERIODIC_TIMER_DEF            100 /* CentiSeconds */
#define VTSS_MRP_PERIODIC_TIMER_MIN            100 /* CentiSeconds */
#define VTSS_MRP_PERIODIC_TIMER_MAX           6000 /* CentiSeconds */
#define VTSS_MRP_PERIODIC_TIMER_MODE_DEF      FALSE /* Default disabled */

#if !defined(XXRP_ATTRIBUTE_PACKED)
#define XXRP_ATTRIBUTE_PACKED __attribute__((packed)) /* GCC defined */
#endif












/***************************************************************************************************
 * XXRP API definitions.
 **************************************************************************************************/
/**
 * \brief function that handles global control event.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param enable      [IN]  Set this boolean variable to enable MRP application
 *                          globally and clear this to disable.
 *
 * \return Return error code.
 **/
u32 vtss_mrp_global_control_conf_set(vtss_mrp_appl_t        application,
                                     BOOL                   enable);

/**
 * \brief function to get MRP application global status.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param status      [OUT] Pointer to a boolean variable. Its value
 *                          is set to TRUE if MRP application is
 *                          enabled globally, else it is set to FALSE.
 *
 * \return Return error code.
 **/
u32 vtss_mrp_global_control_conf_get(vtss_mrp_appl_t application,
                                     BOOL  *const    status);

/**
 * \brief function that handles port control event.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]  L2 port number.
 * \param enable      [IN]  Set this boolean variable to enable MRP applicaiton on this
 *                          port and clear this to disable.
 *
 * \return Return error code.
 **/
// ?tf? mrp -> xxrp
u32 vtss_xxrp_port_control_conf_set(vtss_mrp_appl_t application,
                                    u32             port_no,
                                    BOOL            enable);

/**
 * \brief function to get the current port control status of a MRP application.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]  L2 port number.
 * \param status      [OUT] Set the value of boolean variable to enable MRP application
 *                          on this port and clear this to disable.
 *
 * \return Return error code.
 **/
// ?tf? mrp -> xxrp
u32 vtss_xxrp_port_control_conf_get(vtss_mrp_appl_t application,
                                    u32             port_no,
                                    BOOL  *const    status);

/**
 * \brief function that handles periodic timer control event.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]  L2 port number.
 * \param enable      [IN]  Set this boolean variable to enable periodic transmission
 *                          on this port and clear this to disable.
 *
 * \return Return error code.
 **/
// ?tf? mrp -> xxrp
u32 vtss_xxrp_periodic_transmission_control_conf_set(vtss_mrp_appl_t application,
                                                     u32             port_no,
                                                     BOOL            enable);

/**
 * \brief function to get the current periodic timer control status.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]  L2 port number.
 * \param status      [OUT] Pointer to a boolean variable. Its value is set to TRUE
 *                          if periodic transmission is enabled on this port, else
 *                          it is set to FALSE.
 *
 * \return Return error code.
 **/
u32 vtss_mrp_periodic_transmission_control_conf_get(vtss_mrp_appl_t application,
                                                    u32             port_no,
                                                    BOOL  *const    status);

/**
 * \brief function to configure MRP application timers.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]  L2 port number.
 * \param timers      [IN]  Timer values.
 *
 * \return Return error code.
 **/
// ?tf? mrp -> xxrp
u32 vtss_xxrp_timers_conf_set(vtss_mrp_appl_t              application,
                              u32                          port_no,
                              vtss_mrp_timer_conf_t *const timers);

/**
 * \brief function to get MRP application timers.
 *
 * \param application [IN]   Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]   L2 port number.
 * \param timers      [OUT]  Timer values.
 *
 * \return Return error code.
 **/
u32 vtss_xxrp_timers_conf_get(vtss_mrp_appl_t        application,
                              u32                    port_no,
                              vtss_mrp_timer_conf_t *const timers);

/**
 * \brief function to handle applicant admin control event.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]  L2 port number.
 * \param attr_type   [IN]  Attribute type as defined in vtss_mrp_attribute_type_t enumeration.
 * \param participant [IN]  Set this boolean variable to TRUE to make attr_type a normal participant,
 *                          else set to FALSE.
 *
 * \return Return error code.
 **/
// ?tf? mrp -> xxrp
u32 vtss_xxrp_applicant_admin_control_conf_set(vtss_mrp_appl_t             application,
                                               u32                         port_no,
                                               vtss_mrp_attribute_type_t   attr_type,
                                               BOOL                        participant);

/**
 * \brief function to get the current applicant admin control status.
 *
 * \param application [IN] Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN] L2 port number.
 * \param attr_type   [IN] Attribute type as defined in vtss_mrp_attribute_type_t enumeration.
 * \param status      [OUT] Pointer to a boolean variable. Its value is set to TRUE if attribute
 *                          type is normal participant else it is set to FALSE.
 *
 * \return Return error code.
 **/
u32 vtss_mrp_applicant_admin_control_conf_get(vtss_mrp_appl_t             application,
                                              u32                         port_no,
                                              vtss_mrp_attribute_type_t   attr_type,
                                              BOOL  *const                status);

/**
 * \brief MSTP port state change handler.
 *
 * \param port_no            [IN]  L2 port number.
 * \param msti               [IN]  MSTP instance
 * \param port_state_type    [IN]  port state type
 *
 * \return Return error code.
 **/
// ?tf? mrp -> xxrp
u32 vtss_xxrp_mstp_port_state_change_handler(u32                                     port_no,
                                             u8                                      msti,
                                             vtss_mrp_mstp_port_state_change_type_t  port_state_type);

/**
 * \brief MSTP port role change handler.
 *
 * \param port_no            [IN]  L2 port number.
 * \param msti               [IN]  MSTP instance
 * \param port_role_type     [IN]  port role type
 *
 * \return Return error code.
 **/
u32 vtss_mrp_mstp_port_role_change_handler(u32                                      port_no,
                                           u8                                       msti,
                                           vtss_mrp_mstp_port_role_change_type_t    port_role_type);
















/**
 * \brief MRP application statistics get function.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]  L2 port number.
 * \param stats       [IN]  Pointer to statistics structure;
 *
 * \return Return error code.
 **/
u32 vtss_mrp_statistics_get(vtss_mrp_appl_t              application,
                            u32                          port_no,
                            vtss_mrp_statistics_t *const stats);

/**
 * \brief MRP application statistics clear funtion.
 *
 * \param application [IN]  Application type (as defined in vtss_mrp_appl_t)
 * \param port_no     [IN]  L2 port number.
 *
 * \return Return error code.
 **/
u32 vtss_mrp_statistics_clear(vtss_mrp_appl_t application,
                              u32             port_no);

/**
 * \brief MRP timer tick handler.
 *
 * \param delay [IN] number of cs(ticks) elapsed since this handler was called last time.
 *
 * \return Return min_time.
 **/

//u32 vtss_mrp_timer_tick(u32 delay);
u32 vtss_xxrp_timer_tick(u32 delay);

/**
 * \brief MRP MRPDU receive handler.
 *
 * \param port_no [IN]  L2 port number.
 * \param mrpdu   [IN]  Pointer to MRPDU.
 * \param length  [IN]  Length of MRPDU.
 *
 * \return Return TRUE if MRPDU is approved and handled, FALSE otherwise.
 **/

BOOL vtss_mrp_mrpdu_rx(u32 port_no, const u8 *mrpdu, u32 length);
void vtss_mrp_init(void);
u32 vtss_mrp_port_ring_print(vtss_mrp_appl_t application, u8 msti);
void xxrp_packet_dump(u32 port_no, const u8 *packet, BOOL packet_transmit);
void vtss_mrp_port_mad_print(u32 port_no, u32 machine_index);
#endif /* _VTSS_XXRP_API_H_ */
