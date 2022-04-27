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

#include "vtss_xxrp_callout.h"
#include <vtss_trace_api.h>
#include <main.h>
#include "vtss_garp.h"
#include "vtss_garptt.h"


#define VTSS_TRACE_MODULE_ID  VTSS_MODULE_ID_XXRP

enum gvrp_alloc_type {
    GVRP_ALLOC_DEFAULT,
    GVRP_ALLOC_TX,
    GVRP_ALLOC_ICLI,
    GVRP_ALLOC_MAX
};


// --- Repeat of definition in vtss_gvrp.c. Here just for debug.
struct gvrp_vid2gid {
    struct garp_gid_instance *gid;
    vtss_vid_t vid;
    u16 next_vlan;
    u8 msti;
    u8 old_msti;
    int reference_count; // For gid array
    int vid_ref_count;
    enum gvrp_alloc_type at;
};


extern vtss_vid_t vtss_get_vid(struct garp_gid_instance *gid);


// --- Debug macros
#define T_N2(...) do{}while(0)
//#define T_N2(...) T_N(__VA_ARGS__)
#define T_N1(...) do{}while(0)
//#define T_N1(...) T_N(__VA_ARGS__)

//#define T_N3(...) do { if ( gid==0 || gid==(struct garp_gid_instance*)1 || ((struct gvrp_vid2gid*)(gid->data))->vid==4 || ((struct gvrp_vid2gid*)(gid->data))->vid==5) T_N(__VA_ARGS__); } while(0)
#define T_N3(...) do{}while(0)

#define T_N4(...) T_N(__VA_ARGS__)
#define T_N5(...) T_N(__VA_ARGS__)

#define T_EXTERN(fmt, ...) do {} while(0)
#define T_EXTERN2(fmt, ...) do { printf("%s: " fmt "\n", __func__, ##__VA_ARGS__); } while(0)



// --- To highest possible values in ticks

// Scale 1-20
#define RAND_GARP_TRANSMITPDU_DELAY ( (VTSS_OS_MSEC2TICK(200) * (2+rand()%0xfd))>>8 )

// Scale 1-5
#define GARP_LEAVETIMER_DELAY VTSS_OS_MSEC2TICK(3000)

// Scale 1-5
#define RAND_GARP_LEAVEALLTIMER_DELAY ( (VTSS_OS_MSEC2TICK(50000) * ((1<<12) + rand()%0x7ff))>>12 )



/*
 *  Context:
 * -------------------
 * (1): Applicant SM
 * (2): Registrar SM
 * (3): LeaveAll SM
 * (4): Handlers to the Applicant, Registrar and LeaveAll SMs.
 * (5): Public interface functions
 * (6): Timer
 * (7): Init and mapping function for debugging.
 * (8): General list function to add and remove elements from lists
 */


// (1) --- Applicant SM

struct garp_gid_SM_events {
    unsigned char applicant;
    unsigned char registrar;
    unsigned char leaveall;
    unsigned char gid;
};


#define APPLICANT_SM_ELEMENT(NS, ACTION) {garp_applicant_state__##NS, applicant_action__##ACTION}


static void applicant_action__NoAction(ACTION_SIGNATUR)
{
    return;
}

static void applicant_action__InApplicable(ACTION_SIGNATUR)
{
    return;
}


// IEEE 802.1D-2004 section 12.7, comment for sJ[E,I]
static void applicant_action__sJoinEI(ACTION_SIGNATUR)
{
    vtss_rc rc;

    switch (gid->registrar_state) {

    case garp_registrar_state__Fixed:
    case garp_registrar_state__IN:  // Send Join In message
        rc = a->packet_build(p, gid, garp_attribute_event__JoinIn);
        if (rc) {
            T_D("packet_build failed rc=%d", rc);
        }
        break;

    default:                        // Send Join Empty message
        rc = a->packet_build(p, gid, garp_attribute_event__JoinEmpty);
        if (rc) {
            T_D("packet_build failed rc=%d", rc);
        }
        break;
    }

}


// IEEE 802.1D-2004 section 12.7, comment for sLE
static void applicant_action__sLE(ACTION_SIGNATUR)
{
    vtss_rc rc;

    rc = a->packet_build(p, gid, garp_attribute_event__LeaveEmpty);
    if (rc) {
        T_D("packet_build failed rc=%d", rc);
    }
}

// IEEE 802.1D-2004 section 12.7, comment for sE
static void applicant_action__sE(ACTION_SIGNATUR)
{
    vtss_rc rc;

    rc = a->packet_build(p, gid, garp_attribute_event__Empty);
    if (rc) {
        T_D("packet_build failed rc=%d", rc);
    }
}


// (1.1) --- Applicant SM, see IEEE802.1D-2004, table 12.3)
static const struct garp_applicant_SM_element garp_applicant_SM[garp_applicant_state__MAX][garp_applicant_event__MAX] = {
    {/* State: VO*/
        APPLICANT_SM_ELEMENT(VO, InApplicable),
        APPLICANT_SM_ELEMENT(AO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VO, InApplicable),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: VA*/
        /*                   Next state,    Action */
        APPLICANT_SM_ELEMENT(AA, sJoinEI),   /*txPDU*/
        APPLICANT_SM_ELEMENT(AA, NoAction),  /*rJoinIn*/
        APPLICANT_SM_ELEMENT(VA, NoAction),  /*rJoinEmpty*/
        APPLICANT_SM_ELEMENT(VA, NoAction),  /*rEmpty*/
        APPLICANT_SM_ELEMENT(VA, NoAction),  /*rLeaveIn*/
        APPLICANT_SM_ELEMENT(VP, NoAction),  /*rLeaveEmpty*/
        APPLICANT_SM_ELEMENT(VP, NoAction),  /*LeaveAll*/
        APPLICANT_SM_ELEMENT(VA, InApplicable), /*ReqJoin*/
        APPLICANT_SM_ELEMENT(LA, NoAction),     /*ReqLeave*/
        APPLICANT_SM_ELEMENT(VO, NoAction)      /*Initialyze*/
    },
    {/* State: AA*/
        APPLICANT_SM_ELEMENT(QA, sJoinEI),
        APPLICANT_SM_ELEMENT(QA, NoAction),
        APPLICANT_SM_ELEMENT(VA, NoAction),
        APPLICANT_SM_ELEMENT(VA, NoAction),
        APPLICANT_SM_ELEMENT(VA, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(AA, InApplicable),
        APPLICANT_SM_ELEMENT(LA, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: QA*/
        APPLICANT_SM_ELEMENT(QA, InApplicable),
        APPLICANT_SM_ELEMENT(QA, NoAction),
        APPLICANT_SM_ELEMENT(VA, NoAction),
        APPLICANT_SM_ELEMENT(VA, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(QA, InApplicable),
        APPLICANT_SM_ELEMENT(LA, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: LA*/
        APPLICANT_SM_ELEMENT(VO, sLE),
        APPLICANT_SM_ELEMENT(LA, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(LA, NoAction),
        APPLICANT_SM_ELEMENT(LA, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(VA, NoAction),
        APPLICANT_SM_ELEMENT(LA, InApplicable),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: VP*/
        APPLICANT_SM_ELEMENT(AA, sJoinEI),
        APPLICANT_SM_ELEMENT(AP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, InApplicable),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: AP*/
        APPLICANT_SM_ELEMENT(QA, sJoinEI),
        APPLICANT_SM_ELEMENT(QP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(AP, InApplicable),
        APPLICANT_SM_ELEMENT(AO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: QP*/
        APPLICANT_SM_ELEMENT(QP, InApplicable),
        APPLICANT_SM_ELEMENT(QP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(QP, InApplicable),
        APPLICANT_SM_ELEMENT(QO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: AO*/
        APPLICANT_SM_ELEMENT(AO, InApplicable),
        APPLICANT_SM_ELEMENT(QO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(AP, NoAction),
        APPLICANT_SM_ELEMENT(AO, InApplicable),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: QO*/
        APPLICANT_SM_ELEMENT(QO, InApplicable),
        APPLICANT_SM_ELEMENT(QO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(QP, NoAction),
        APPLICANT_SM_ELEMENT(QO, InApplicable),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    },
    {/* State: LO*/
        APPLICANT_SM_ELEMENT(VO, sE),
        APPLICANT_SM_ELEMENT(AO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(VO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(LO, NoAction),
        APPLICANT_SM_ELEMENT(VP, NoAction),
        APPLICANT_SM_ELEMENT(LO, InApplicable),
        APPLICANT_SM_ELEMENT(VO, NoAction)
    }
};


// (2) --- Registrar SM


#define REGISTRAR_SM_ELEMENT(NS, ACTION) {garp_registrar_state__##NS, registrar_action__##ACTION}

static void registrar_action__NoAction(ACTION_SIGNATUR)
{
    return;
}

static void registrar_action__InApplicable(ACTION_SIGNATUR)
{
    return;
}


static void registrar_action__StartLeaveTimer(ACTION_SIGNATUR)
{
    // Start LeaveTimer
    gid->leave_timeout = vtss_garp_get_clock(a, GARP_TC__leavetimer) + GARP_LEAVETIMER_DELAY;

    if (a->GARP_timer_list_leave) {
        a->timer_kick = 1;
    }
    vtss_garp_add_list(&a->GARP_timer_list_leave, &gid->leave_list);
}

/*
 * Variant of registrar_action__IndJoin that do not call a->GID_Join_indication()
 */
static void registrar_action__IndJoin2(ACTION_SIGNATUR, BOOL emit)
{
    struct garp_participant  *tmp_p;
    struct garp_gid_instance *tmp_gid;
    garp_typevalue_t tv;

    tv = p->map_gid_obj2TV(p, gid);

    T_N5("IndJoin on port=%d, vid=%d\n", p->gid_index, ((struct gvrp_vid2gid *)(gid->data))->vid);

    T_N1("(1) --- registrar_action__IndJoin");

    if (emit) {
        a->GID_Join_indication(p, gid);
    }

    // --- Sent GID_Join.request to all other active participants
    for (a->restore_gip(gid, p); (tmp_p = a->next_gip()); ) {
        T_N5("(2) --- registrar_action__IndJoin: GIP port %d -> %d", p->gid_index, tmp_p->gid_index);

        tmp_gid = p->map_gid_TV2obj(tmp_p, tv);
        if (tmp_gid) {
            T_N5("(3) --- registrar_action__IndJoin: GIP port %d -> %d", p->gid_index, tmp_p->gid_index);

            (void)vtss_garp_gidtt_event_handler2(a, tmp_p, tmp_gid, garp_event__ReqJoin);
        }
        p->done_gid(tmp_p, tmp_gid);

    }

    T_N5("(3) --- registrar_action__IndJoin: End");
}


/*
 * Send GID_Join.indication [IEEE 802.1D-2004, clause 12.9.4.9] ?tf?
 */
static void registrar_action__IndJoin(ACTION_SIGNATUR)
{
    registrar_action__IndJoin2(a, p, gid, TRUE);
}


/*
 * Send GID_Leave.indication [IEEE 802.1D-2004, clause 12.9.4.10] ?tf?
 */
static void registrar_action__IndLeave(ACTION_SIGNATUR)
{
    struct garp_participant  *tmp_p, *registrarIN;
    struct garp_gid_instance *tmp_gid;
    garp_typevalue_t tv;
    int reg;

    tv = p->map_gid_obj2TV(p, gid);

    // Sent GID_Leave.request to all other active participants under
    // certain conditions.  See 802.1D-2004, clause 12.2.3
    //  gip = a->get_gip_context(gid);

    //  reg = gip->calc_registrations(gip, p, tv);

    reg = a->calc_registrations(p, gid, &registrarIN);

    a->GID_Leave_indication(p, gid);

    // De-registrartion process, see [IEEE 802.1D-2004, claus 12.2.3]
    switch (reg) {
    case 0: // De-register all
        a->restore_gip(gid, p);
        while ( (tmp_p = a->next_gip()) ) {
            tmp_gid = p->map_gid_TV2obj(tmp_p, tv);
            if (tmp_gid) {
                (void)vtss_garp_gidtt_event_handler2(a, tmp_p, tmp_gid, garp_event__ReqLeave);
            }
            p->done_gid(tmp_p, tmp_gid);
        }

        break;

    case 1:
        tmp_p = registrarIN;
        tmp_gid = p->map_gid_TV2obj(tmp_p, tv);
        if (tmp_gid) {
            (void)vtss_garp_gidtt_event_handler2(a, tmp_p, tmp_gid, garp_event__ReqLeave);
        }
        p->done_gid(tmp_p, tmp_gid);

        break;

    default: // De-register none.
        break;
    }

    T_N5("IndLeave on port=%d, vid=%d\n", p->gid_index, ((struct gvrp_vid2gid *)(gid->data))->vid);
}


static void registrar_action__StopLeaveTimer(ACTION_SIGNATUR)
{
    // (1) --- Remove from leave timer list
    vtss_garp_remove_list(&a->GARP_timer_list_leave, &gid->leave_list);

    // (2) --- and
    registrar_action__IndJoin2(a, p, gid, FALSE);
}

static void registrar_action__StopLeaveTimer_IndLeave(ACTION_SIGNATUR)
{
    // (1) --- Remove from leave timer list
    vtss_garp_remove_list(&a->GARP_timer_list_leave, &gid->leave_list);

    // (2) --- and
    registrar_action__IndLeave(a, p, gid);
}



// (2.1) --- Registrar SM, see IEEE802.1D-2004, table 12.4)
static const struct garp_registrar_SM_element garp_registrar_SM[garp_registrar_state__MAX][garp_registrar_event__MAX] = {
    {/* State: MT */
        /*                   Next state,    Action */
        REGISTRAR_SM_ELEMENT(IN, IndJoin),      // rJoinIn
        REGISTRAR_SM_ELEMENT(IN, IndJoin),      // rJoinEmpty
        REGISTRAR_SM_ELEMENT(MT, NoAction),     // rEmpty
        REGISTRAR_SM_ELEMENT(MT, NoAction),     // rLeaveIn
        REGISTRAR_SM_ELEMENT(MT, NoAction),     // rLeaveEmpty
        REGISTRAR_SM_ELEMENT(MT, NoAction),     // LeaveAll
        REGISTRAR_SM_ELEMENT(MT, InApplicable), // leavetimer
        REGISTRAR_SM_ELEMENT(MT, NoAction),     // Initialize

        REGISTRAR_SM_ELEMENT(MT, NoAction),         // Normal
        REGISTRAR_SM_ELEMENT(Fixed, IndJoin),       // Fixed
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),  // Forbidden
    },
    {/* State: IN */
        /*                   Next state,    Action */
        REGISTRAR_SM_ELEMENT(IN, NoAction),
        REGISTRAR_SM_ELEMENT(IN, NoAction),
        REGISTRAR_SM_ELEMENT(IN, NoAction),
        REGISTRAR_SM_ELEMENT(LV, StartLeaveTimer),
        REGISTRAR_SM_ELEMENT(LV, StartLeaveTimer),
        REGISTRAR_SM_ELEMENT(LV, StartLeaveTimer),
        REGISTRAR_SM_ELEMENT(IN, InApplicable),
        REGISTRAR_SM_ELEMENT(MT, NoAction),

        REGISTRAR_SM_ELEMENT(IN, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction), // Already in IN
        REGISTRAR_SM_ELEMENT(Forbidden, IndLeave)
    },
    {/* State: LV */
        /*                   Next state,    Action */
        REGISTRAR_SM_ELEMENT(IN, StopLeaveTimer /*, IndJoin*/),
        REGISTRAR_SM_ELEMENT(IN, StopLeaveTimer /*, IndJoin*/),
        REGISTRAR_SM_ELEMENT(LV, NoAction),
        REGISTRAR_SM_ELEMENT(LV, StartLeaveTimer),
        REGISTRAR_SM_ELEMENT(LV, StartLeaveTimer),
        REGISTRAR_SM_ELEMENT(LV, StartLeaveTimer),
        REGISTRAR_SM_ELEMENT(MT, IndLeave),
        REGISTRAR_SM_ELEMENT(MT, NoAction),

        REGISTRAR_SM_ELEMENT(LV, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, StopLeaveTimer_IndLeave),
        REGISTRAR_SM_ELEMENT(Forbidden, IndLeave)
    },

    {/* State: Fixed -> IN */
        /*                   Next state,    Action */
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),

        REGISTRAR_SM_ELEMENT(MT, IndLeave),
        REGISTRAR_SM_ELEMENT(Fixed, NoAction),
        REGISTRAR_SM_ELEMENT(Forbidden, IndLeave)
    },

    {/* State: Forbidden -> MT */
        /*                   Next state,    Action */
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction),

        REGISTRAR_SM_ELEMENT(MT, NoAction),
        REGISTRAR_SM_ELEMENT(Fixed, IndJoin),
        REGISTRAR_SM_ELEMENT(Forbidden, NoAction)
    }
};


// (3) --- LeaveAll SM


#define LEAVEALL_SM_ELEMENT(NS, ACTION) {garp_leaveall_state__##NS, leaveall_action__##ACTION}

static void leaveall_action__InApplicable(ACTION_SIGNATUR)
{
    return;
}

static void leaveall_action__sLeaveAll(ACTION_SIGNATUR)
{
    vtss_rc rc;

    // --- LeaveAll in txPDU
    rc = a->packet_build(p, gid, garp_attribute_event__LeaveAll);
    if (rc) {
        T_D("packet_build failed rc=%d", rc);
    }

    // --- Send LeaveAll to my own Applicant and Registrar SMs.
    a->leaveall(p);
}

static void StartLeaveAllTimer(struct garp_application *a, struct garp_participant *p)
{

    // Start LeaveAll timer if not already running.
    if (!p->leaveAll_participant_list.next) {
        T_N1("(1) --- StartLeaveAllTimer: starting");
        p->leaveAll_timeout = vtss_garp_get_clock(a, GARP_TC__leavealltimer) + RAND_GARP_LEAVEALLTIMER_DELAY;

        vtss_garp_add_list(&a->GARP_timer_list_leaveAll, &p->leaveAll_participant_list);

        a->timer_kick = 1;
        //      vtss_mrp_timer_kick();
    } else {
        T_N1("(2) --- StartLeaveAllTimer: has been started");
    }
}


void vtss_garp_StopLeaveAllTimer(struct garp_application *a, struct garp_participant *p)
{
    vtss_garp_remove_list(&a->GARP_timer_list_leaveAll, &p->leaveAll_participant_list);
}



static void leaveall_action__StartLeaveAllTimer(ACTION_SIGNATUR)
{
    // Restart, so remove participant from list first, and then start timer.
    vtss_garp_remove_list(&a->GARP_timer_list_leaveAll, &p->leaveAll_participant_list);
    StartLeaveAllTimer(a, p);
}

static void leaveall_action__StartLeaveAllTimer2(ACTION_SIGNATUR)
{
    // Indicate timer has run out, before starting it again.

    T_N1("(1) --- leaveall_action__StartLeaveAllTimer2: For port=%d", p->gid_index);
    StartLeaveAllTimer(a, p);
}

// (3.1) --- LeaveAll SM, see IEEE802.1D-2004, table 12.5)
static const struct garp_leaveall_SM_element garp_leaveall_SM[garp_leaveall_state__MAX][garp_leaveall_event__MAX] = {
    {/* State: Active */
        /*                  Next state,     Action */
        LEAVEALL_SM_ELEMENT(Passive, sLeaveAll),           // transmitPDU
        LEAVEALL_SM_ELEMENT(Passive, StartLeaveAllTimer),  // LeaveAll
        LEAVEALL_SM_ELEMENT(Active,  StartLeaveAllTimer2)  // leavealltimer!
    },
    {/* State: Passive */
        /*                  Next state,     Action */
        LEAVEALL_SM_ELEMENT(Passive, InApplicable),
        LEAVEALL_SM_ELEMENT(Passive, StartLeaveAllTimer),
        LEAVEALL_SM_ELEMENT(Active,  StartLeaveAllTimer2)
    }
};



// (4) --- Handlers to the Applicant, Registrar and LeaveAll SMs.


// (4.1) --- Support functions
static int testA(int s)
{
    return
        s == garp_applicant_state__QA ||
        s == garp_applicant_state__QP ||
        s == garp_applicant_state__VO ||
        s == garp_applicant_state__AO ||
        s == garp_applicant_state__QO     ? 1 : 0;
}


static int change_transmitPDU_list(int a1, int a2)
{
    // Try to do a fast decision
    if (a1 == a2) {
        return 0;
    }

    return testA(a1) == testA(a2) ? 0 : 1;
}

static int testA3(int s)
{
    return (s == garp_applicant_state__VO || s == garp_applicant_state__LO) ? 1 : 0;
}


static int change_applicant_VO_LO(int a1, int a2)
{
    return testA3(a1) == testA3(a2) ? 0 : 1;
}

static int testR2(int s)
{
    return
        (s == garp_registrar_state__MT) ? 1 : 0;
}


static int change_registrar_MT(int r1, int r2)
{
    return testR2(r1) == testR2(r2) ? 0 : 1;
}


static BOOL participant_less(struct PN_participant *pn1, struct PN_participant *pn2);

#define ADD_TO_TIMER_LIST(a,p,N)                                        \
    do {                                                                \
        if (IS_PARTICIPANT_IN_TXPDU_QUEUE(p)) {                         \
        } else {                                                        \
            p->GARP_transmitPDU_timeout = vtss_garp_get_clock(a, GARP_TC__transmitPDU) + RAND_GARP_TRANSMITPDU_DELAY; \
            vtss_garp_add_sort_list(&a->GARP_timer_list_##N, &p->N##_list, participant_less); \
            a->timer_kick=1;                                            \
        }                                                               \
    } while (0)


#define APPLICANT_ADD_TO_LIST(a,p,gid,N)        \
    do {                                        \
        if ( 1==a->add_pdulist(p,gid) ) {       \
            ADD_TO_TIMER_LIST(a,p,N);           \
        }                                       \
    } while (0)


#define REMOVE_FROM_TIMER_LIST(a,p,N)                                   \
    vtss_garp_remove_list(&a->GARP_timer_list_##N, &p->N##_list)


#define APPLICANT_REMOVE_FROM_LIST(a,p,gid,N)   \
    do {                                        \
        if ( 0==a->remove_pdulist(p,gid) ) {    \
            REMOVE_FROM_TIMER_LIST(a,p,N);      \
        }                                       \
    } while (0)


// (4.2) --- Top handler
static vtss_rc vtss_garp_gidtt_event_handler3(struct garp_application  *a,
                                              struct garp_participant  *p,
                                              struct garp_gid_instance *gid,
                                              const struct garp_gid_SM_events  *events)
{

    // --- It is an internal error if a or p is NULL
    if (a == 0 || p == 0 || events == 0) {
        T_E("Application or Participant is missing a=%p, p=%p event=%p", a, p, events);
        return VTSS_RC_ERROR;
    }

    // --- It may happen that gid==NULL, if resource limit has been reached
    if (gid == 0) {
        T_E("No gid resource");
        return VTSS_RC_ERROR;
    }

    /*lint -esym(613,gid) */


    // (1) --- Handle applicant SM */
    if (events->applicant < garp_applicant_event__MAX) {
        T_N3("(2) --- Applicant:");

        // (1.1) --- Check if transmitPDU list shall be updated for this SM
        if (change_transmitPDU_list(gid->applicant_state, garp_applicant_SM[gid->applicant_state][events->applicant].next_state)) {

            // Add to or remove from the transmitPDU list
            if (testA(gid->applicant_state)) {
                T_N3("(2.1) --- Add to txPDU list");
                APPLICANT_ADD_TO_LIST(a, p, gid, transmitPDU);
            } else {
                T_N3("(2.2) --- Remove to txPDU list");
                APPLICANT_REMOVE_FROM_LIST(a, p, gid, transmitPDU);
            }
        }


        // (1.2) --- Check if reference count for this applicant shall be updated.
        if (change_applicant_VO_LO(gid->applicant_state, garp_applicant_SM[gid->applicant_state][events->applicant].next_state)) {
            if (testA3(gid->applicant_state)) {
                // Moved out of {VO,LO} set
                a->ref_count(p, gid, 1);
                T_N3("(2.3) --- Moved out of {VO,LO} set => Ref count up");
            } else {
                // Moved into {VO,LO} set
                a->ref_count(p, gid, -1);
                T_N3("(2.4) --- Moved into {VO,LO} set => Ref count down");
            }
        }

        if (vtss_get_vid(gid) == 5 || vtss_get_vid(gid) == 4)
            T_EXTERN("(1.3) --- Applicant State change: %s-(%s)->%s, port=%d, vid=%d",
                     applicant_state_name(gid->applicant_state),
                     applicant_event_name(events->applicant),
                     applicant_state_name(garp_applicant_SM[gid->applicant_state][events->applicant].next_state),
                     p->gid_index, vtss_get_vid(gid));

        // (1.4) --- Call action
        garp_applicant_SM[gid->applicant_state][events->applicant].action(a, p, gid);

        // (1.5) --- Change state
        gid->applicant_state = garp_applicant_SM[gid->applicant_state][events->applicant].next_state;
    }



    // (3) --- Handle registrar SM */
    if (events->registrar < garp_registrar_event__MAX) {

        if (vtss_get_vid(gid) == 5 || vtss_get_vid(gid) == 4)
            T_EXTERN("(3.1) --- Registrar State change: %s-(%s)->%s, port=%d, vid=%d",
                     registrar_state_name(gid->registrar_state),
                     registrar_event_name(events->registrar),
                     registrar_state_name(garp_registrar_SM[gid->registrar_state][events->registrar].next_state),
                     p->gid_index, vtss_get_vid(gid));

        // (3.1) --- Check if reference count for this applicant shall be updated.
        if (change_registrar_MT(gid->registrar_state, garp_registrar_SM[gid->registrar_state][events->registrar].next_state)) {
            if (testR2(gid->registrar_state)) {
                // Moved out of {MT} set
                a->ref_count(p, gid, 1);
                T_N3("(3.2) --- Moved out of {MT} set => Ref count up");
            } else {
                // Moved into {MT} set
                a->ref_count(p, gid, -1);
                T_N3("(3.3) --- Moved into {MT} set => Ref count down");
            }
        }

        // (3.2) --- Call action
        garp_registrar_SM[gid->registrar_state][events->registrar].action(a, p, gid);

        // (3.3) --- Change state
        gid->registrar_state = garp_registrar_SM[gid->registrar_state][events->registrar].next_state;
    }


    // (4) --- Handle leaveall SM */
    if (events->leaveall < garp_leaveall_event__MAX) {

        T_N3("(4.1) --- LeaveAll State change: %s-(%s)->%s\n", leaveall_state_name(p->leaveall_state),
             leaveall_event_name(events->leaveall),
             leaveall_state_name(garp_leaveall_SM[p->leaveall_state][events->leaveall].next_state));

        // (4.1) --- Call action
        garp_leaveall_SM[p->leaveall_state][events->leaveall].action(a, p, gid);

        // (4.2) --- Change state
        p->leaveall_state = garp_leaveall_SM[p->leaveall_state][events->leaveall].next_state;
    }

    return VTSS_RC_OK;
}


#define EVENT_MAP_0010(A) {garp_applicant_event__MAX, garp_registrar_event__MAX,  garp_leaveall_event__##A,  garp_gip_event__MAX}
#define EVENT_MAP_0100(A) {garp_applicant_event__MAX, garp_registrar_event__##A,  garp_leaveall_event__MAX,  garp_gip_event__MAX}
#define EVENT_MAP_1000(A) {garp_applicant_event__##A, garp_registrar_event__MAX,  garp_leaveall_event__MAX,  garp_gip_event__MAX}
#define EVENT_MAP_1100(A) {garp_applicant_event__##A, garp_registrar_event__##A,  garp_leaveall_event__MAX,  garp_gip_event__MAX}
#define EVENT_MAP_1010(A) {garp_applicant_event__##A, garp_registrar_event__MAX,  garp_leaveall_event__##A,  garp_gip_event__MAX}
#define EVENT_MAP_1110(A) {garp_applicant_event__##A, garp_registrar_event__##A,  garp_leaveall_event__##A,  garp_gip_event__MAX}

#define EVENT_MAP_1101(A) {garp_applicant_event__##A, garp_registrar_event__##A,   garp_leaveall_event__MAX, garp_gip_event__##A}



/*lint -sem(vtss_garp_gidtt_event_handler2, thread_protected) */
vtss_rc vtss_garp_gidtt_event_handler2(struct garp_application *a,
                                       struct garp_participant  *participant,
                                       struct garp_gid_instance *gid,
                                       enum garp_all_event event)
{
    static const struct garp_gid_SM_events event_map[garp_event__MAX] = {
        EVENT_MAP_1010(transmitPDU),
        EVENT_MAP_1101(rJoinIn),
        EVENT_MAP_1100(rJoinEmpty),
        EVENT_MAP_1100(rEmpty),
        EVENT_MAP_1101(rLeaveIn),
        EVENT_MAP_1100(rLeaveEmpty),
        EVENT_MAP_1110(LeaveAll),
        EVENT_MAP_1000(ReqJoin),
        EVENT_MAP_1000(ReqLeave),
        EVENT_MAP_1100(Initialize),
        EVENT_MAP_0100(leavetimer),
        EVENT_MAP_0010(leavealltimer),

        // --- Variants of the above, where LeaveAll SM is removed
        EVENT_MAP_1100(LeaveAll),    // This is LeaveAll2 only sent
        EVENT_MAP_1000(transmitPDU), // This is garp_event__transmitPDU2 only sent
        EVENT_MAP_0010(transmitPDU),  // This is garp_event__transmitPDU3 only sent

        EVENT_MAP_0100(Normal), // Normal (Registrar)
        EVENT_MAP_0100(Fixed),      // Fixed  (Registrar)
        EVENT_MAP_0100(Forbidden)   // Forbidden (Registrar)
    };

    return vtss_garp_gidtt_event_handler3(a, participant, gid, &event_map[event]);
}

vtss_rc vtss_garp_gidtt_event_handler(struct garp_application *a,
                                      struct garp_participant  *participant,
                                      struct garp_gid_instance *gid,
                                      enum garp_all_event event)
{
    vtss_rc rc;
    int kick;

    a->timer_kick = 0;
    rc = vtss_garp_gidtt_event_handler2(a, participant, gid, event);
    kick = a->timer_kick;

    if (kick) {
        vtss_mrp_timer_kick();
    }

    return rc;
}


// (4.3) --- Frontend for RX PDUs
vtss_rc vtss_garp_gidtt_rx_pdu_event_handler(struct garp_application *a,
                                             struct garp_participant *p,
                                             struct garp_gid_instance *gid,
                                             enum garp_attribute_event attribute_event)
{
    static const enum garp_all_event const map[garp_attribute_event__MAX] = {
        garp_event__LeaveAll,
        garp_event__rJoinEmpty,
        garp_event__rJoinIn,
        garp_event__rLeaveEmpty,
        garp_event__rLeaveIn,
        garp_event__rEmpty
    };

    return vtss_garp_gidtt_event_handler(a, p, gid, map[attribute_event]);
}


// (5) --- Public interface functions


/*
 * Send transmitPDU! event to all GID SMs
 */
static void vtss_garp_transmitPDU(struct garp_application *a,
                                  struct garp_participant *p)
{
    struct garp_gid_instance *gid;
    vtss_rc rc = VTSS_RC_OK;

    T_N1("--- vtss_garp_transmitPDU");

    // (1) --- Emit PDU for all Applicant and Registrar SMs within 'p'
    for (a->restore_pdulist(p); (gid = a->next_pdulist(p)); ) {

        rc |= vtss_garp_gidtt_event_handler2(a, p, gid, garp_event__transmitPDU2);
    }

    // (2) --- Emit PDU LeaveAll SMs within 'p'
    gid = (struct garp_gid_instance *)1; // This is ok for garp_event__transmitPDU3, since gid is not used.
    rc |= vtss_garp_gidtt_event_handler2(a, p, gid, garp_event__transmitPDU3);

    rc = a->packet_done(p);
    if (rc) {
        T_D("packet_done failed rc=%d", rc);
    }
}

#define GARP_INTERNAL_CHECK()                           \
    do {                        \
        if (a==0 || p==0 || value==0) {                 \
            T_E("a=%p, p=%p, value=%p", a,p,value); \
            return -1;                                  \
        }                       \
    } while(0)


/*
 * GID_Join.request to attribute (type,value) within participant 'p'.
 */
vtss_rc vtss_gid_join_request(struct garp_application  *a,
                              struct garp_participant  *p,
                              garp_attribute_type_t type,
                              void *value)
{
    vtss_rc rc;
    struct garp_gid_instance *gid;

    T_N1("--- vtss_gid_join_request");

    GARP_INTERNAL_CHECK();

    gid = p->get_gid_TypeValue2obj(p, type, value);
    rc = vtss_garp_gidtt_event_handler(a, p, gid, garp_event__ReqJoin);
    p->done_gid(p, gid);

    return rc;
}



/*
 * GID_Leave.request to attribute (type,value) within participant 'p'.
 */
vtss_rc vtss_gid_leave_request(struct garp_application *a,
                               struct garp_participant *p,
                               garp_attribute_type_t    type,
                               void                    *value)
{
    vtss_rc rc;
    struct garp_gid_instance *gid;

    T_N1("--- vtss_gid_leave_request");

    GARP_INTERNAL_CHECK();

    gid = p->get_gid_TypeValue2obj(p, type, value);
    rc = vtss_garp_gidtt_event_handler(a, p, gid, garp_event__ReqLeave);
    p->done_gid(p, gid);

    return rc;
}



/*
 * IEEE802.1D-2004, clause 12.8.1
 */

vtss_rc vtss_garp_registrar_administrative_control(struct garp_application *a,
                                                   struct garp_participant *p,
                                                   garp_attribute_type_t type,
                                                   void *value,
                                                   vlan_registration_type_t S)
{
    static const enum garp_all_event const M[3] = {
        garp_event__Normal,
        garp_event__Fixed,
        garp_event__Forbidden
    };

    vtss_rc rc = VTSS_RC_ERROR;
    struct garp_gid_instance *gid;

    GARP_INTERNAL_CHECK();

    // --- If port is not GARP enabled, then just quit.
    if (!IS_PORT_MGMT_ENABLED2(p)) {
        return VTSS_RC_OK;
    }

    if ( S > VLAN_REGISTRATION_TYPE_FORBIDDEN ) {
        return VTSS_RC_ERROR;
    }

    // --- So port is enabled
    gid = p->get_gid_TypeValue2obj(p, type, value);
    if (gid) {

        rc = vtss_garp_gidtt_event_handler(a, p, gid, M[S]);
        p->done_gid(p, gid);

    }

    return rc;
}



void vtss_garp_leaveall(struct garp_application *a,
                        struct garp_participant *p)
{
    T_N1("--- vtss_garp_leaveall port=%d", p->gid_index);

    garp_leaveall_SM[p->leaveall_state][garp_leaveall_event__LeaveAll].action(a, p, 0);
    p->leaveall_state = garp_leaveall_SM[p->leaveall_state][garp_leaveall_event__LeaveAll].next_state;

    // --- Send LeaveAll to all GIDs in the participant 'p'.
    a->leaveall(p);

    if (a->timer_kick) {
        a->timer_kick = 0;
        vtss_mrp_timer_kick();
    }

}


void vtss_garp_force_txPDU(struct garp_application *a)
{
    struct PN_participant  *pn, *pn_next, *pn_end;
    struct garp_participant *p;

    if ( (pn = a->GARP_timer_list_transmitPDU) ) {

        pn_end = pn->prev;
        pn_next = pn;

        do {
            pn = pn_next;
            pn_next = pn->next;
            p = MEMBER(struct garp_participant, transmitPDU_list, pn);
            vtss_garp_transmitPDU(a, p);
        } while ( pn != pn_end );
    }
}


/* (6) --- Timer
 *
 * There are 3 timer queues:
 *  - transmitPDU, a list of participants, which have Applicant SMs
 *    that will transmit a PDU upon an txPDU! event. The participant
 *    is holding the timeout value. Therefore the txPDU! event comes
 *    at the same time to all GIDs within a participant.
 *  - leaveTimer, a list in which Registrar SMs link up when
 *    starting leavetimer. A Registrar link up in the end of
 *    this timer list, since the timers will expirer in the
 *    order the Registrar SMs start the leavetimer
 *  - leaveAllTimer, a list of Particimants whos LeaveAll SMs
 *    start the leavealltimer.
 */


/*
 * Map logical-time to real-time.
 */
static cyg_tick_count_t map_lt2rt(struct garp_application *a, enum timer_context tc, cyg_tick_count_t lt)
{
    cyg_tick_count_t rt;

    if (!a->GARP_timer[tc].scale) {
        T_E("a->GARP_timer[%d].scale=%llu", tc, a->GARP_timer[tc].scale);
    }

    rt = ((lt - a->GARP_timer[tc].offset) << GARP_SCALE_UNIT) / a->GARP_timer[tc].scale + a->GARP_timer[tc].actual;

    return VTSS_OS_MSEC2TICK(rt * 10);
}


// (6.1) --- Timer handler is called whenever an timeout event is
//           supposed to have occured.
uint vtss_garp_timer_tick(uint delay, struct garp_application *a)
{
    vtss_rc rc;
    cyg_tick_count_t t, X, next_time = 0;
    int is_next_time_set;

    uint D;

    struct garp_participant  *p;
    struct PN_participant  *pn, *pn_next, *pn_end;
    struct garp_gid_instance *gid;

    T_N1("Enter");

    do {

        if ( (pn = a->GARP_timer_list_transmitPDU) ) {
            p = MEMBER(struct garp_participant, transmitPDU_list, pn);

            t = vtss_garp_get_clock(a, GARP_TC__transmitPDU);
            pn_end = pn->prev;

            while (t - p->GARP_transmitPDU_timeout < GARP_POSITIVE_U32) {

                // (1.1) --- Must get the next now, since vtss_garp_transmitPDU() may remove the 'pn' element.
                pn_next = pn->next;

                // (1.2) --- Transmit PDU for this participant.
                vtss_garp_transmitPDU(a, p);

                // (1.3) --- If participant is still in the txPDU list, then set it up for a new transmit.
                if ( IS_PN_IN_LIST(pn) ) {

                    vtss_garp_remove_list(&a->GARP_timer_list_transmitPDU, &p->transmitPDU_list);
                    p->GARP_transmitPDU_timeout = vtss_garp_get_clock(a, GARP_TC__transmitPDU);
                    p->GARP_transmitPDU_timeout += RAND_GARP_TRANSMITPDU_DELAY;
                    vtss_garp_add_sort_list(&a->GARP_timer_list_transmitPDU, &p->transmitPDU_list, participant_less);
                }

                // (1.4) --- If txPDU list has become empty OR pn_next is pointing t
                //           the first element in the list, then we are done.
                if (pn == pn_end) {
                    break;
                }

                pn = pn_next;
                p = MEMBER(struct garp_participant, transmitPDU_list, pn);
            }
        }


        T_N1("(2) --- LeaveAll");

        // (2) --- LeaveAll: If GARP_timer_list_leaveAll is empty, then skip this step.
        //    if ( (p=a->GARP_timer_list_leaveAll) ) {
        if ( (pn = a->GARP_timer_list_leaveAll) ) {

            t = vtss_garp_get_clock(a, GARP_TC__leavealltimer);

            p = MEMBER(struct garp_participant, leaveAll_participant_list, pn);

            T_N1("pn=%p, t=%d", pn, (int)(t - p->leaveAll_timeout));

            for (; pn && (t - p->leaveAll_timeout < GARP_POSITIVE_U32);
                 pn = a->GARP_timer_list_leaveAll, p = MEMBER(struct garp_participant, leaveAll_participant_list, pn) )  {

                T_N1("leaveAll:");
                T_N4("--- (2) tick LeaveAll port=%d", p->gid_index);

                // (2.1) --- Take this participant out of the LeaveAll list,
                //           and indicate timer is stopped, since the .action()
                //           will insert it again.
                vtss_garp_remove_list(&a->GARP_timer_list_leaveAll, pn);

                //  a->GARP_timer_list_leaveAll = pn->next_leaveAll;
                //  p->leaveAll_timer_running = 0;

                // (2.2) ---
                a->leaveall(p);

                // (2.3) --- Call action and change state
                garp_leaveall_SM[p->leaveall_state][garp_leaveall_event__leavealltimer].action(a, p, 0);
                p->leaveall_state = garp_leaveall_SM[p->leaveall_state][garp_leaveall_event__leavealltimer].next_state;

            }
        }


        T_N1("(3) --- Leave");

        // (3) --- Leave: If GARP_timer_list_leave is empty, then skip this step.
        if ((pn = a->GARP_timer_list_leave)) {

            t = vtss_garp_get_clock(a, GARP_TC__leavetimer);

            do {
                T_N1("leave:");

                gid = MEMBER(struct garp_gid_instance, leave_list, pn);

                if (t - gid->leave_timeout < GARP_POSITIVE_U32) {

                    T_N4("--- (3) tick Leave");

                    // (3.1) --- Take head out of timer list
                    vtss_garp_remove_list(&a->GARP_timer_list_leave, pn);

                    //?tf?
                    rc = vtss_garp_gidtt_event_handler2(a, gid->participant, gid, garp_event__leavetimer);
                    if (rc) {
                        T_E("rc=%d", rc);
                    }

                    //    // (3.2) --- Call action and change state
                    //    rc = garp_registrar_SM[gid->registrar_state][garp_registrar_event__leavetimer].action(a, p, gid);
                    //    gid->registrar_state = garp_registrar_SM[gid->registrar_state][garp_registrar_event__leavetimer].next_state;

                } else {

                    // (3.2) --- The p element has not timeout, so stop here
                    break;
                }
            } while ( (pn = a->GARP_timer_list_leave) );
        }

        is_next_time_set = 0; // No


        T_N1("(4) --- Calculate when next time tick");

        // (4) --- Calculate when next time tick should occur
        //    if ((p=a->GARP_timer_list_transmitPDU)) {
        if (a->GARP_timer_list_transmitPDU) {
            p = MEMBER(struct garp_participant, transmitPDU_list, a->GARP_timer_list_transmitPDU);

            next_time = map_lt2rt(a, GARP_TC__transmitPDU, p->GARP_transmitPDU_timeout);
            is_next_time_set = 1;
            T_N1("(4.1) --- Calculate when next time tick/transmitPDU next_time=%llu", next_time);

        }

        t = cyg_current_time();
        T_N1("E.4 t=%llu, next_time=%llu, next_time-t=%llu", t, next_time, next_time - t);


        if (a->GARP_timer_list_leaveAll) {
            T_N1("E.4.1");

            X = map_lt2rt(a, GARP_TC__leavealltimer,
                          MEMBER(struct garp_participant,
                                 leaveAll_participant_list,
                                 a->GARP_timer_list_leaveAll)->leaveAll_timeout);

            //      X = map_lt2rt(a, GARP_TC__leavealltimer, a->GARP_timer_list_leaveAll->leaveAll_timeout);
            if (!is_next_time_set || next_time - X < GARP_POSITIVE_U32)  {
                next_time = X;
                is_next_time_set = 1;
                T_N1("(4.2) --- Calculate when next time tick/leaveall next_time=%llu", next_time);
            }
        }

        if (a->GARP_timer_list_leave) {
            T_N1("E.4.2");

            X = map_lt2rt(a, GARP_TC__leavetimer,
                          MEMBER(struct garp_gid_instance,
                                 leave_list,
                                 a->GARP_timer_list_leave)->leave_timeout);

            // a->GARP_timer_list_leave->leave_timeout);


            if (!is_next_time_set || next_time - X < GARP_POSITIVE_U32) {
                next_time = X;
                is_next_time_set = 1;
                T_N1("(4.3) --- Calculate when next time tick/leave next_time=%llu", next_time);
            }

        }

        // if no next time, then stop timer
        if (!is_next_time_set) {
            T_N1("Stopping timer");
            return 0;
        }

        t = cyg_current_time();
        D = next_time ? next_time - t : 0;

        T_N1("E.5 D=%llu, t=%u", D, t);

    } while (D > GARP_POSITIVE_U32 || D == 0);

    T_N1("Next timer at  D=%u", D);


    // Ok, so next timetick after delay D.
    // This is ok, since there is always at least one timer running
    // namely the transmitPDU - even if it has just expired, a new
    // timer is initiated in that process.

    return D;
}


static void vtss_garp_change_clock_scale(struct garp_application *a, enum timer_context tc, cyg_tick_count_t scale, u32 time)
{
    cyg_tick_count_t t, t2;

    t = cyg_current_time();

    t2 = ( ( (t - a->GARP_timer[tc].actual) * a->GARP_timer[tc].scale ) >> GARP_SCALE_UNIT) + a->GARP_timer[tc].offset;

    T_N1("Current: actual=%llu, offset=%llu, scale=%llu", a->GARP_timer[tc].actual, a->GARP_timer[tc].offset, a->GARP_timer[tc].scale);

    a->GARP_timer[tc].offset = t2;
    a->GARP_timer[tc].actual = t;
    a->GARP_timer[tc].scale = scale;
    a->GARP_timer[tc].time = time;

    T_N1("New    : actual=%llu, offset=%llu, scale=%llu", a->GARP_timer[tc].actual, a->GARP_timer[tc].offset, a->GARP_timer[tc].scale);

}


// (6.2) --- Set the timeout value 'new_time' for timer 'tc' within application 'a'.
vtss_rc vtss_garp_set_timer(struct garp_application *a, enum timer_context tc, u32 new_time, int full)
{
    cyg_tick_count_t scale;

    // (1) --- Range check, and calc scale
    switch (tc) {
    case GARP_TC__transmitPDU:
        if (new_time < 1 || new_time > 20) {
            return VTSS_RC_ERROR;
        }
        scale = ((1 << GARP_SCALE_UNIT) * 20) / new_time;
        break;

    case GARP_TC__leavetimer:
        if (new_time < 60 || new_time > 300) {
            return VTSS_RC_ERROR;
        }
        scale = ((1 << GARP_SCALE_UNIT) * 300) / new_time;
        break;

    case GARP_TC__leavealltimer:
        if (new_time < 1000 || new_time > 5000) {
            return VTSS_RC_ERROR;
        }
        scale = ((1 << GARP_SCALE_UNIT) * 5000) / new_time;
        break;

    default:
        return VTSS_RC_ERROR;
    }

    // (2) --- Set scale
    vtss_garp_change_clock_scale(a, tc, scale, new_time);

    if (full) {
        vtss_mrp_timer_kick();
    }

    return VTSS_RC_OK;
}

// (6.2) --- Set the timeout value 'new_time' for timer 'tc' within application 'a'.
u32 vtss_garp_get_timer(struct garp_application *a, enum timer_context tc)
{
    return  a->GARP_timer[tc].time;
}


// (6.3) --- Get the value set byfunction in (6.2).
cyg_tick_count_t vtss_garp_get_clock(struct garp_application *a, enum timer_context tc)
{
    cyg_tick_count_t x, y;

    x = cyg_current_time();
    y = ( (x - a->GARP_timer[tc].actual) * a->GARP_timer[tc].scale ) >> GARP_SCALE_UNIT;
    y = y + a->GARP_timer[tc].offset;

    T_N1("tc=%d cyg_current_time=%llu, calc_time=%llu", tc, x, y);

    return y;
}



// (7) --- Init and mapping function for debugging.

void vtss_garp_init(struct garp_application *a)
{
    a->GARP_timer[GARP_TC__transmitPDU].offset = 0;
    a->GARP_timer[GARP_TC__transmitPDU].actual = 0;
    a->GARP_timer[GARP_TC__transmitPDU].scale = (1 << GARP_SCALE_UNIT);
    a->GARP_timer[GARP_TC__transmitPDU].time = 20;

    a->GARP_timer[GARP_TC__leavetimer].offset = 0;
    a->GARP_timer[GARP_TC__leavetimer].actual = 0;
    a->GARP_timer[GARP_TC__leavetimer].scale = (1 << GARP_SCALE_UNIT) * 5;
    a->GARP_timer[GARP_TC__leavetimer].time = 60;

    a->GARP_timer[GARP_TC__leavealltimer].offset = 0;
    a->GARP_timer[GARP_TC__leavealltimer].actual = 0;
    a->GARP_timer[GARP_TC__leavealltimer].scale = (1 << GARP_SCALE_UNIT) * 5;
    a->GARP_timer[GARP_TC__leavealltimer].time = 1000;

    a->GARP_timer_list_transmitPDU = 0;
    a->GARP_timer_list_leave = 0;
    a->GARP_timer_list_leaveAll = 0;
}


const char *registrar_state_name(enum garp_registrar_state S)
{
    static const char *N[] = {"MT", "IN", "LV",
                              "Fixed", "Forbidden"
                             };
    return N[S];
}
const char *leaveall_state_name(enum garp_leaveall_state S)
{
    static const char *N[] = {"Active ", "Passive"};
    return N[S];
}

const char *applicant_state_name(enum garp_applicant_state S)
{
    static const char *N[] = {"VO", "VA", "AA", "QA", "LA", "VP", "AP", "QP", "AO", "QO", "LO"};
    return N[S];
}

const char *applicant_event_name(enum garp_applicant_event E)
{
    static const char *N[] = {"txPDU", "rJoinIn", "rJoinEmpty", "rEmpty", "rLeaveIn", "rLeaveEmpty", "LeaveAll", "ReqJoin", "ReqLeave", "Initialize"};
    return N[E];
}

const char *registrar_event_name(enum garp_registrar_event E)
{
    static const char *N[] = {"rJoinIn", "rJoinEmpty", "rEmpty", "rLeaveIn", "rLeaveEmpty", "LeaveAll", "leavetimer", "Initialize",
                              "Normal", "Fixed", "Forbidden"
                             };
    return N[E];
}

const char *leaveall_event_name(enum garp_leaveall_event E)
{
    static const char *N[] = {"txPDU", "LeaveAll", "leavealltimer"};
    return N[E];
}



// (8) --- General list function to add and remove elements from lists


// (8.1) --- Insert element 'p' in the end of the list 'P'
void vtss_garp_add_list(struct PN_participant **P, struct PN_participant *p)
{
    // --- Insert participant into port enabled list
    if ( *P ) {

        (*P)->prev->next = p;

        p->next = *P;
        p->prev = (*P)->prev;

        (*P)->prev = p;

    } else {

        *P = p;
        p->prev = p->next = p;

    }
}




// (8.2) --- This function insert the element 'p' into the list 'P' in such a way,
//            that it is sorted wrt 'less()'.
//            *P is the smallest element and in general, if q is an element in the list,
//            then less(q, q->next) is true ( or q < q->next ) as long at q is not
//            the end of the list, i.e., q->next != *P.
void vtss_garp_add_sort_list(struct PN_participant **P, struct PN_participant *p, garp_less_t less)
{
    struct PN_participant *l;

    // --- Insert participant into port enabled list
    if ( *P ) {

        l = (*P)->prev;

        if (!less(p, l)) {
            // (1) --- l<=p, when l is the last element in sorted list, so put p in the end.
            vtss_garp_add_list(P, p);

        } else {


            // (2) --- find the l in sorted list for which p<l, and insert p just before l
            for ( ; l != *P && !less(p, l); l = l->prev) {
                ;
            }
            vtss_garp_add_list(&l, p);

            // (2.1) --- If p has been inserted before *P, then *P must point at p, since
            //     it is the smallest element in the list
            if (l == *P) {
                *P = (*P)->prev;
            }

        }
    } else
        // (3) --- List is empty, so just insert the normal way.
    {
        vtss_garp_add_list(P, p);
    }

}


// (8.3) --- Remove element 'p' in the end of the list 'P'
void vtss_garp_remove_list(struct PN_participant **P, struct PN_participant *p)
{
    // --- If *P is null, then p is not in that list, so just quit.
    if ( !(*P) ) {
        T_N("PN_participant empty list P=%p", P);
        return;
    }

    // --- If next field is null, then p is not part of a list
    if (!p->next) {
        T_N("PN_participant next field is NULL, P=%p, *P=%p", P, *P);
        return;
    }

    // --- If prev is null at this point, then it is an error
    if (!p->prev) {
        T_E("PN_participant prev field is NULL");
        return;
    }

    // --- Remove p from port enabled list
    p->next->prev = p->prev;
    p->prev->next = p->next;

    // --- If p was entry element, then choose another
    if ( (*P) == p )  {
        (*P) = p->next;
        // --- If still entry element, then p is the only one in the list, so delete list
        if ( (*P) == p ) {
            (*P) = 0;
        }
    }

    // --- Null pointers indicate that this participant is not in a list
    p->next = p->prev = 0;

}


// (8.4) --- less function used in list function (8.2).
//           return true if p1<p2
static BOOL participant_less(struct PN_participant *pn1, struct PN_participant *pn2)
{
    struct garp_participant *p1, *p2;

    if (!pn1 || !pn2) {
        T_E("pn1=%p, pn2=%p", pn1, pn2);
        return TRUE;
    }

    p1 = MEMBER(struct garp_participant, transmitPDU_list, pn1);
    p2 = MEMBER(struct garp_participant, transmitPDU_list, pn2);

    return p2->GARP_transmitPDU_timeout - p1->GARP_transmitPDU_timeout < GARP_POSITIVE_U32 ? TRUE : FALSE;
}


// (8.5) --- Remove element 'p' in the end of the list 'P'
void vtss_garp_copy_list(struct PN_participant *from, struct PN_participant **to,
                         size_t from_offset, size_t to_offset)
{
    ssize_t delta;
    struct PN_participant *tmp_from, *tmp_to;

    // --- If from list is empty, then to list shall be empty too.
    if (!from) {
        *to = 0;
        return;
    }

    delta = ((ssize_t)to_offset) - ((ssize_t)from_offset);

    tmp_from = from;
    tmp_to = tmp_from + delta;

    *to = tmp_to;

    do {
        tmp_to = tmp_from + delta;
        tmp_to->next = tmp_from->next + delta;
        tmp_to->prev = tmp_from->prev + delta;
        tmp_from = tmp_from->next;
    } while (tmp_from != from);

}
