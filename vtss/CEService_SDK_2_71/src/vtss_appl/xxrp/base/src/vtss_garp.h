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

#ifndef _VTSS_GARP_H_
#define _VTSS_GARP_H_

#include "vtss_common_os.h"

typedef size_t garp_typevalue_t;
typedef u8     garp_attribute_type_t;


#define IS_PN_IN_LIST(pn) (pn->next)
struct PN_participant {
    struct PN_participant* prev;
    struct PN_participant* next;
};
 
struct garp_participant;


/*
 * GARP GID structure
 */
struct garp_gid_instance {
    u8  registrar_state;
    u8  applicant_state;
    u8  reference_count; // 0 or 1

    cyg_tick_count_t leave_timeout;

    struct PN_participant leave_list;
    struct garp_participant *participant;

    // Application specific data, that GARP does not need to understand
    void *data;
};


/*
 * GARP participant structure
 */
struct garp_participant {

    struct garp_gid_instance* (*get_gid_TypeValue2obj)(struct garp_participant*, garp_attribute_type_t, void*);

    garp_typevalue_t (*map_gid_obj2TV)(struct garp_participant  *p, struct garp_gid_instance*);
    struct garp_gid_instance* (*map_gid_TV2obj)(struct garp_participant  *p, garp_typevalue_t);
    void (*done_gid)(struct garp_participant *p, struct garp_gid_instance *gid);

    u16 gid_index;

    int   port_no;
    u8   *pdu;
    u8   *pdu2;
    int   space_left;
    void *context;

    int leaveall_state;

    struct PN_participant enabled_port;
    struct PN_participant gip[8+1];
    BOOL   port_msti_enabled[8];
  //    u8     trace_gip_change[8]; // 0=initial, i.e. always detect as change, 1=out, 2=in

#define IS_PORT_MGMT_ENABLED(port_no) (GVRP_participant[port_no].enabled_port.next ? TRUE : FALSE)
#define IS_PORT_MGMT_ENABLED2(p) (p->enabled_port.next ? TRUE : FALSE)
#define IS_IN_GIP_CONTEXT(p,msti) (p->gip[msti].next ? TRUE : FALSE)
#define IS_GID_IN_TXPDU_QUEUE(gid) (gid->participant->transmitPDU_list.next ? TRUE : FALSE)
#define IS_PARTICIPANT_IN_TXPDU_QUEUE(p) (p->transmitPDU_list.next ? TRUE : FALSE)

    cyg_tick_count_t GARP_transmitPDU_timeout;
    struct PN_participant transmitPDU_list;


    cyg_tick_count_t      leaveAll_timeout;
    struct PN_participant leaveAll_participant_list;

    // if not gid object exit, i.e., vid2gid[] is null, then the state
    // of the applicant SM is VO or LO depending on the bit in the array below
    u32 partial_applicant[0x1000/0x20]; // = 0x80*4bytes = 512bytes

    struct  {
        u32 tx_list[0x1000/0x20]; // = 0x80*4bytes = 512bytes
        int index;
        int bit;
        struct garp_gid_instance *gid;
        int count;
    } txPDU;

};



enum garp_attribute_event {
    garp_attribute_event__LeaveAll,
    garp_attribute_event__JoinEmpty,
    garp_attribute_event__JoinIn,
    garp_attribute_event__LeaveEmpty,
    garp_attribute_event__LeaveIn,
    garp_attribute_event__Empty,
    garp_attribute_event__MAX
};

// --- Timers
enum timer_context {
    GARP_TC__transmitPDU,
    GARP_TC__leavetimer,
    GARP_TC__leavealltimer,
    GARP_TC__MAX
};


/*
 * GARP application structure
 */
struct garp_application {

    // --- transmitPDU timer list.
    struct PN_participant *GARP_timer_list_transmitPDU;
    //  cyg_tick_count_t GARP_transmitPDU_timeout;

    // --- Leave timer list. This is list of registrar.
    struct PN_participant *GARP_timer_list_leave;

    // --- LeaveAll timer list. This is a list of participants.
    struct PN_participant *GARP_timer_list_leaveAll;

    struct PN_participant *port_enabled_list;
    struct PN_participant *gip[8+1];
  
    int timer_kick;

    int (*packet_build)(struct garp_participant *p, struct garp_gid_instance *gid, enum garp_attribute_event a);
    int (*packet_done)(struct garp_participant *p);
    void (*ref_count)(struct garp_participant  *p, struct garp_gid_instance *g, int R);
    int  (*calc_registrations)(struct garp_participant *p, struct garp_gid_instance *gip, struct garp_participant **pp);
    void (*leaveall)(struct garp_participant *p);

    void (*restore_pdulist)(struct garp_participant *p);
    struct garp_gid_instance* (*next_pdulist)(struct garp_participant *p);
    int (*add_pdulist)(struct garp_participant *p, struct garp_gid_instance *gid);
    int (*remove_pdulist)(struct garp_participant *p, struct garp_gid_instance *gid);

    void (*GID_Leave_indication)(struct garp_participant *p, struct garp_gid_instance *gid);
    void (*GID_Join_indication)(struct garp_participant *p, struct garp_gid_instance *gid);

    void (*restore_gip)(struct garp_gid_instance *gid, struct garp_participant *p);
    struct garp_participant* (*next_gip)(void);

    struct {
        struct PN_participant *R;
        struct PN_participant *N;
        u32 msti;
    } iter__gip;

    struct {
        struct PN_participant *R;
        struct PN_participant *N;
        BOOL done;
    } iter__port_enable;

    // --- From this point on, the structure is not cleared by bzero in the
    //     constructor.

#define GARP_SCALE_UNIT (8)

    struct _timer_context {
        cyg_tick_count_t actual;
        cyg_tick_count_t scale; // 1= scale=2^GARP_SCALE_UNIT
        cyg_tick_count_t offset;
        u32              time;
    } GARP_timer[GARP_TC__MAX];
};


#define ACTION_SIGNATUR struct garp_application *a, struct garp_participant *p, struct garp_gid_instance *gid
typedef void (*action_t)(ACTION_SIGNATUR);



// (1) --- Applicant SM

// (1.1) --- Applicant SM events (IEEE802.1D-2004, table 12.3)
enum garp_applicant_event {
    garp_applicant_event__transmitPDU,
    garp_applicant_event__rJoinIn,
    garp_applicant_event__rJoinEmpty,
    garp_applicant_event__rEmpty,
    garp_applicant_event__rLeaveIn,
    garp_applicant_event__rLeaveEmpty,
    garp_applicant_event__LeaveAll,
    garp_applicant_event__ReqJoin,
    garp_applicant_event__ReqLeave,
    garp_applicant_event__Initialize, 
    garp_applicant_event__MAX 
};

const char* applicant_event_name(enum garp_applicant_event E);

// (1.2) --- Applicant SM states (IEEE802.1D-2004, table 12.3)
enum garp_applicant_state {
    garp_applicant_state__VO, // Place init state first so that is is zero
    garp_applicant_state__VA,
    garp_applicant_state__AA,
    garp_applicant_state__QA,
    garp_applicant_state__LA,
    garp_applicant_state__VP,
    garp_applicant_state__AP,
    garp_applicant_state__QP,

    garp_applicant_state__AO,
    garp_applicant_state__QO,
    garp_applicant_state__LO,
    garp_applicant_state__MAX
};

extern const char* applicant_state_name(enum garp_applicant_state S);

/* (1.3) --- Applicant element in state event table */
typedef struct garp_applicant_SM_element {
    unsigned char next_state;    /* enum garp_applicant_state_t */
    action_t action;
} garp_applicant_SM_element_t;



// (2) --- Registrar SM

// (2.1) --- Registrar SM events (IEEE802.1D-2004, table 12.4)
enum garp_registrar_event {
    garp_registrar_event__rJoinIn,
    garp_registrar_event__rJoinEmpty,
    garp_registrar_event__rEmpty,
    garp_registrar_event__rLeaveIn,
    garp_registrar_event__rLeaveEmpty,
    garp_registrar_event__LeaveAll,
    garp_registrar_event__leavetimer,
    garp_registrar_event__Initialize,

    garp_registrar_event__Normal,
    garp_registrar_event__Fixed,
    garp_registrar_event__Forbidden,

    garp_registrar_event__MAX
};

extern const char* registrar_event_name(enum garp_registrar_event E);

// (2.2) --- Registrar SM events (IEEE802.1D-2004, table 12.4)
enum garp_registrar_state {
    garp_registrar_state__MT, // Place init state first so that it is zero
    garp_registrar_state__IN,
    garp_registrar_state__LV,

    garp_registrar_state__Fixed,
    garp_registrar_state__Forbidden,


    garp_registrar_state__MAX
};

extern const char* registrar_state_name(enum garp_registrar_state S);

/* (2.3) --- Registrar element in state event table */
typedef struct garp_registrar_SM_element {
    unsigned char next_state;    /* enum garp_applicant_state_t */
    action_t action;
} garp_registrar_SM_element_t;



// (3) --- LeaveAll SM

// (3.1) --- LeaveAll SM events (IEEE802.1D-2004, table 12.5)
enum garp_leaveall_event {
    garp_leaveall_event__transmitPDU,
    garp_leaveall_event__LeaveAll,
    garp_leaveall_event__leavealltimer,
    garp_leaveall_event__MAX
};

extern const char* leaveall_event_name(enum garp_leaveall_event S);

// (3.2) --- LeaveAll SM events (IEEE802.1D-2004, table 12.5)
enum garp_leaveall_state {
    garp_leaveall_state__Active,
    garp_leaveall_state__Passive,
    garp_leaveall_state__MAX
};

extern const char* leaveall_state_name(enum garp_leaveall_state S);

/* (3.3) --- LeaveAll element in state event table */
typedef struct garp_leaveall_SM_element {
    unsigned char next_state;    /* enum garp_applicant_state_t */
    action_t action;
} garp_leaveall_SM_element_t;


// (4) --- GIP
enum garp_gip_event {
    garp_gip_event__rJoinIn,
    garp_gip_event__rLeaveIn,
    garp_gip_event__MAX
};


// (4) --- Collection of events
enum garp_all_event {
    garp_event__transmitPDU,
    garp_event__rJoinIn,
    garp_event__rJoinEmpty,
    garp_event__rEmpty,
    garp_event__rLeaveIn,
    garp_event__rLeaveEmpty,
    garp_event__LeaveAll,
    garp_event__ReqJoin,
    garp_event__ReqLeave,
    garp_event__Initialize, 

    garp_event__leavetimer,

    garp_event__leavealltimer,

    garp_event__LeaveAll2, // Same as garp_event__LeaveAll, but only sent to applicant and registrar SMs
    garp_event__transmitPDU2, // Same as garp_event__LeaveAll, but only sent to applicant and registrar SMs
    garp_event__transmitPDU3, // Same as garp_event__LeaveAll, but only sent to applicant and registrar SMs

    garp_event__Normal,
    garp_event__Fixed,
    garp_event__Forbidden,
 
    garp_event__MAX
};

#endif /* _VTSS_GARP_H_ */
