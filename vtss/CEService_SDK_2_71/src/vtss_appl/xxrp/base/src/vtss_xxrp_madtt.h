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
#ifndef _VTSS_XXRP_MADTT_H_
#define _VTSS_XXRP_MADTT_H_
#include "vtss_xxrp_types.h"

#define     MADTT_INVALID_INDEX     0xFFFF

/* LeaveAll events */
typedef enum {
    begin_la,
    tx_la,
    rLA_la,
    leavealltimer_la,
    last_la_event
} xxrp_la_event;

/* LeaveAll States */
typedef enum {
    active_la,
    passive_la,
    last_la_state
} xxrp_la_states;

/* LeaveAll structures and callbacks */
typedef struct {
    unsigned char state;
    u32 (*la_stm_callback)(vtss_mrp_mad_t *mad);
} xxrp_la_stm_t;

/* Periodic STM arguments */
typedef struct {
    vtss_mrp_mad_t              *mad;
    vtss_mrp_appl_t             application;
} periodic_stm_args;


/* Periodic STM events */
typedef enum {
    begin_periodic,
    enabled_periodic,
    disabled_periodic,
    periodictimer_periodic,
    last_periodic_event
} xxrp_periodic_event;

/* Periodic STM States */
typedef enum {
    active_periodic,
    passive_periodic,
    last_periodic_state
} xxrp_periodic_states;

/* Periodic STM structures and callbacks */
typedef struct {
    unsigned char state;
    u32 (*periodic_stm_callback)(periodic_stm_args *args);
} xxrp_periodic_stm_t;

/* Registrar STM events */
typedef enum {
    begin_reg,
    rNew_reg,
    rJoinIn_reg,
    rJoinMt_reg,
    rLv_reg,
    rLA_reg,
    txLA_reg,
    redeclare_reg,
    flush_reg,
    leavetimer_reg,
    last_reg_event
} xxrp_reg_event;

/* Registrar STM States */
typedef enum {
    IN,
    LV,
    MT,
    last_reg_state
} xxrp_reg_states;

/* Registrar STM arguments */
typedef struct {
    vtss_mrp_mad_t              *mad;
    u16                         indx;
    BOOL                        restart;
    vtss_mrp_appl_t             application;
    vtss_mrp_attribute_type_t   *attr_type;
} reg_stm_args;

/* Registrar structures and callbacks */
typedef struct {
    unsigned char state;
    u32 (*reg_stm_callback)(reg_stm_args *args);
} xxrp_reg_stm_t;

/* Applicant STM events */
typedef enum {
    begin_app,
    new_app,
    join_app,
    lv_app,
    rNew_app,
    rJoinIn_app,
    rIn_app,
    rJoinMt_app,
    rMt_app,
    rLv_app,
    rLA_app,
    redeclare_app,
    periodic_app,
    tx_app,
    txLA_app,
    txLAF_app,
    last_appl_event
} xxrp_appl_event;

/* Applicant STM states */
typedef enum {
    VO,
    VP,
    VN,
    AN,
    AA,
    QA,
    LA,
    AO,
    QO,
    AP,
    QP,
    LO,
    last_appl_state
} xxrp_appl_states;

/* Applicant STM arguments */
typedef struct {
    vtss_mrp_mad_t              *mad;
    BOOL                        restart;
} appl_stm_args;

/* Applicant structures and callbacks */
typedef struct {
    unsigned char state;
    u32 (*appl_stm_callback)(appl_stm_args *args);
} xxrp_appl_stm_t;

typedef struct {
    xxrp_la_event           la_event;
    xxrp_periodic_event     periodic_event;
    xxrp_reg_event          reg_event;
    xxrp_appl_event         appl_event;
} vtss_mad_fsm_events;


void vtss_mrp_madtt_init_state_machine(vtss_mrp_mad_machine_t *machine, BOOL is_attr_reg);


/* FSM Event Handler */
u32 vtss_mrp_madtt_event_handler(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, vtss_mrp_mad_t *mad_port, 
                                 u16 mad_fsm_indx, vtss_mad_fsm_events *events);


void vtss_mrp_madtt_init_la_and_periodic_state_machines(vtss_mrp_mad_t *mad_port);


static inline u8 vtss_mrp_madtt_get_mad_applicant_state(vtss_mrp_mad_t *mad_port, u16 mad_fsm_indx)
{   
    return mad_port->machines[mad_fsm_indx].applicant;
}


static inline u8 vtss_mrp_madtt_get_mad_registrar_state(vtss_mrp_mad_t *mad_port, u16 mad_fsm_indx)
{       
    return mad_port->machines[mad_fsm_indx].registrar;
}       


static inline u8 vtss_mrp_madtt_get_mad_la_state(vtss_mrp_mad_t *mad_port)
{   
    return mad_port->leaveall_stm_current_state;
}       


static inline u8 vtss_mrp_madtt_get_mad_periodic_state(vtss_mrp_mad_t *mad_port)
{           
    return mad_port->periodic_stm_current_state;
}


static inline void vtss_mrp_madtt_set_mad_registrar_state(vtss_mrp_mad_t *mad_port, u16 mad_fsm_indx, xxrp_reg_states state)
{       
    mad_port->machines[mad_fsm_indx].registrar = state;
}       


#endif /* _VTSS_XXRP_MADTT_H_ */
