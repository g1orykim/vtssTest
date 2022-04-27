/*

  Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

/*lint -sem(vtss_mrp_crit_enter, thread_lock) */
/*lint -sem(vtss_mrp_crit_exit, thread_unlock) */

#include "vtss_gvrp.h"
#include "vtss_garp.h"
#include "vtss_garptt.h"
#include <vtss_xxrp_api.h>
#include <mstp_api.h>
#include <vtss_xxrp_callout.h>
#include <vlan_api.h>
#include <xxrp_api.h>

#include <vtss_trace_api.h>

#include <cyg/kernel/kapi.h>

#define VTSS_TRACE_MODULE_ID  VTSS_MODULE_ID_XXRP
#define VTSS_ALLOC_MODULE_ID  VTSS_MODULE_ID_XXRP
#undef _GVRP_DUMP_PACKET_
#define MSTI_COPY 8

#define TRACE_GRP_GVRP         6

//#define T_N1(...) do{}while(0)
#define T_N1(...) T_N(__VA_ARGS__)

#define T_EXTERN(fmt, ...) do {} while(0)
#define T_EXTERN2(fmt, ...) do { printf("%s: " fmt "\n", __func__, ##__VA_ARGS__); } while(0)

//#define GVRP_CRIT_ENTER() vtss_mrp_crit_enter(VTSS_GARP_APPL_GVRP)
//#define GVRP_CRIT_EXIT()  vtss_mrp_crit_exit(VTSS_GARP_APPL_GVRP)
#define GVRP_ASSERT_LOCKED() vtss_mrp_crit_assert_locked(VTSS_GARP_APPL_GVRP)

#define GET_PDU_X(p,l,x,t) (l-=x, p += x, *(t*)(p-x))
#define GET_PDU_1(p,l) GET_PDU_X(p,l,1,u8)

#define SET_PDU_X(p,v,x,t) do { *(t*)(p) = (v); (p) += x; } while(0)
#define SET_PDU_1(p,v)  SET_PDU_X(p,v,1,u8)

#define GVRP_ATTRIBUTE_TYPE 1
#define VTSS_GVRP_MAX_PDU_SIZE 1514

extern  void dump_packet(int len, const u8 *p);

struct gvrp_gid_ctrl {
    struct garp_gid_instance *gid_array;
    int    next; // =-1 is last in list
};

enum gvrp_alloc_type {
    GVRP_ALLOC_DEFAULT,
    GVRP_ALLOC_TX,
    GVRP_ALLOC_ICLI,
    GVRP_ALLOC_MAX
};


static struct {
    int count;
    int max;
    int peek;
} alloc_book_keeper[GVRP_ALLOC_MAX];


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

#define GET_VID(gid) ( ((struct gvrp_vid2gid*)(gid->data))->vid )
#define GET_MSTI(gid) ( ((struct gvrp_vid2gid*)(gid->data))->msti )
#define GET_PORT(p) ( p->gid_index )
#define GET_GID_ARRAY(gid) ((struct gvrp_vid2gid*)gid->data)->gid
#define VID_RANGE_CHECK(vid) ( vid > 0 && vid < NO_VLAN )
#define PORT_RANGE_CHECK(x) (((x) > -1) && ((x) < GVRP_participant_max))

#define NO_GIP_CONTEXTS 8
#define NO_MSTI 8
#define NO_VLAN (4096)

// (1) ------ Data structures for keeping track of GVRP
/*lint -sem(GVRP_application, thread_protected) */
static int GVRP_application_partial_init = 0;
static struct garp_application   GVRP_application;
static struct garp_participant  *GVRP_participant = 0;
static int                       GVRP_participant_max = 0;
static struct gvrp_vid2gid       GVRP_vid2gid[NO_VLAN];
static struct gvrp_gid_ctrl     *GVRP_free_gid_list;
static int                       GVRP_free_available_gid_index; // Index of free GVRP_vid2gid. =-1 if none in list
static int                       GVRP_free_gid_index; // Index of free GVRP_vid2gid. =-1 if none in list
static int msti_list[NO_MSTI];

int warning_limit[3];


// Forward declatations
vtss_rc vtss_gvrp_port_control_conf_set(u32 port_no, BOOL enable);
static vtss_rc update_port_state(struct garp_participant  *P, u8 msti);
static void update_port_state2(struct garp_participant  *P, vtss_vid_t vid, u8 msti_old, u8 msti_new, u8 msti_transit);
static vtss_rc vtss_gvrp_registrar_administrative_control2(int port_no, vtss_vid_t vid, vlan_registration_type_t S);

// (2) ------ GVRP specific mapping functions

vtss_vid_t vtss_get_vid(struct garp_gid_instance *gid)
{
    if (gid) {
        return GET_VID(gid);
    }

    T_E("gid=0");
    return 0;
}

static garp_typevalue_t gvrp_map_gid_obj2TV(struct garp_participant  *p, struct garp_gid_instance *g)
{
    return GET_VID(g);
}


/*
 *  Allocate a gid_array, i.e., gid objects for a given vid, for all ports.
 *  This may come from a cache or from real allocation.
 *  If the resource can not be allocated, then NULL is returned.
 *
 */
static struct garp_gid_instance *alloc_gid(vtss_vid_t vid, enum gvrp_alloc_type at)
{
    int i;
    struct garp_gid_instance *gid_array = 0;


    // --- Check that it is allowed to alloc in group 'at'.
    if ( !(alloc_book_keeper[at].count < alloc_book_keeper[at].max) ) {
        if (warning_limit[0] < 3) {
            T_W("Allocation denied. Limit reached. Only %d simultanious VLAN allowed for group %d", alloc_book_keeper[at].max, at);
            ++warning_limit[0];
            // We are not allowed to allocate any
        }
        return 0;
    }


    // --- Take resource from pool. Othervise allocate.
    if (GVRP_free_available_gid_index == -1) {

        T_N1("Do real allocation %d/%d", alloc_book_keeper[at].count, alloc_book_keeper[at].max);

        gid_array = VTSS_CALLOC(GVRP_participant_max, sizeof(struct garp_gid_instance));
        if (gid_array) {

            for (i = 0; i < GVRP_participant_max; ++i) {
                gid_array[i].participant = &GVRP_participant[i];
            }
        }

    } else {

        //    T_N1("Take element from free list");
        // Take element from list of freed elements
        i = GVRP_free_available_gid_index;
        gid_array = GVRP_free_gid_list[i].gid_array;

        GVRP_free_available_gid_index = GVRP_free_gid_list[i].next;
        GVRP_free_gid_list[i].next = GVRP_free_gid_index;
        GVRP_free_gid_index = i;

        if (!gid_array) {
            T_E("alloc_gid(%d): ", (int)vid);
        }

    }

    if (gid_array) {
        for (i = 0; i < GVRP_participant_max; ++i) {
            gid_array[i].data = (void *)&GVRP_vid2gid[vid];
            GVRP_vid2gid[vid].at = at;
        }


        ++alloc_book_keeper[at].count;
        if (alloc_book_keeper[at].count > alloc_book_keeper[at].peek) {
            alloc_book_keeper[at].peek = alloc_book_keeper[at].count;
        }

    }
    return gid_array;
}



static void free_gid(int vid)
{
    int j;

    // --- Check that there is a free entry to place the gid
    //     resource into. If not, then it is an error.
    if (GVRP_free_gid_index == -1) {
        T_E("No GVRP_free_gid_list[] resources");
        return; // This will generate a memory leak
    }

    j = GVRP_free_gid_index;

    GVRP_free_gid_index = GVRP_free_gid_list[GVRP_free_gid_index].next;

    GVRP_free_gid_list[j].next = GVRP_free_available_gid_index;
    GVRP_free_gid_list[j].gid_array = GVRP_vid2gid[vid].gid;
    if (!GVRP_vid2gid[vid].gid) {
        T_E("GVRP_vid2gid[%d].gid=%d", vid, (int)GVRP_vid2gid[vid].gid);
    }

    GVRP_vid2gid[vid].gid = 0;
    --alloc_book_keeper[GVRP_vid2gid[vid].at].count;

    GVRP_free_available_gid_index = j;
}

/*
 *  This function is called by Applicant and Registrar SMs with R=1 if
 *  the init state is left, i.e., VO and MT respectivily, and with R=-1
 *  if init state is entered.
 *  In this way the GVRP application can descide how to reuse resources.
 */
static void ref_count(struct garp_participant  *p, struct garp_gid_instance *gid, int R)
{
    size_t vid = GET_VID(gid);

    GVRP_vid2gid[vid].reference_count += R;
    gid->reference_count += R;

    // If all SMs are in init-state, then this resource
    // may be used by others if needed
    if (!GVRP_vid2gid[vid].reference_count) {

        free_gid(vid);

    }
}

/*lint -sem(ref_count2, thread_protected) */
static void ref_count2(struct garp_participant  *p, struct garp_gid_instance *g, int R)
{
    size_t i = GET_VID(g);

    GVRP_vid2gid[i].reference_count += R;
    g->reference_count += R;
}


/*
 * Get gid object for port 'p' with 'vid'
 * If object can not be allocated, then NULL is returned
 */
static struct garp_gid_instance *gvrp_get_gid_obj(struct garp_participant  *p, vtss_vid_t vid, enum gvrp_alloc_type at)
{
    struct garp_gid_instance *gid_array;
    struct garp_gid_instance *gid = 0;

    if (VID_RANGE_CHECK(vid)) {

        gid_array = GVRP_vid2gid[vid].gid;

        if (!gid_array) {
            gid_array = GVRP_vid2gid[vid].gid = alloc_gid(vid, at);
            //      if (2==alloc_book_keeper[at].count) T_EXTERN2("X.1 %d", at);
        }

        if (gid_array) {
            gid = &gid_array[p->gid_index];

            if (gid->reference_count == 0) {

                gid->applicant_state = (p->partial_applicant[ vid >> 5 ] & (1 << (vid & 0x1f))) ?
                                       garp_applicant_state__LO : garp_applicant_state__VO;
                gid->registrar_state = garp_registrar_state__MT;

            }

            ref_count(p, gid, 1);
        }
    }

    return gid;
}


/*lint -sem(gvrp_map_gid_TV2obj, thread_protected) */
static struct garp_gid_instance *gvrp_map_gid_TV2obj(struct garp_participant  *p, garp_typevalue_t tv)
{
    struct garp_gid_instance *gid;
    gid = gvrp_get_gid_obj(p, (vtss_vid_t)tv, GVRP_ALLOC_DEFAULT);
    //  if (2==alloc_book_keeper[GVRP_ALLOC_DEFAULT].count) T_EXTERN2("X.2");

    return gid;
}


/*lint -sem(gvrp_get_gid_TypeValue2obj, thread_protected) */
static struct garp_gid_instance *gvrp_get_gid_TypeValue2obj(struct garp_participant  *p, garp_attribute_type_t type, void *value)
{
    struct garp_gid_instance *gid;
    gid = gvrp_get_gid_obj(p, *(vtss_vid_t *)value, GVRP_ALLOC_DEFAULT);
    //  if (2==alloc_book_keeper[GVRP_ALLOC_DEFAULT].count) T_EXTERN2("X.3");

    return gid;
    //  return gvrp_get_gid_obj(p, *(vtss_vid_t *)value, GVRP_ALLOC_DEFAULT);
}


/*
 * When we are done with a particular gid, then call this function to see if the
 * gid array can be released
 */
static void gvrp_done_gid2(struct garp_participant *p, struct garp_gid_instance *gid, int R)
{
    vtss_vid_t vid;

    if (!gid) {
        return;
    }

    vid = GET_VID(gid);

    if (gid->registrar_state == garp_registrar_state__MT) {
        if (gid->applicant_state == garp_applicant_state__VO) {
            p->partial_applicant[ vid >> 5 ] &= ~(1UL << (vid & 0x1f));
        } else if (gid->applicant_state == garp_applicant_state__LO) {
            p->partial_applicant[ vid >> 5 ] |= (1UL << (vid & 0x1f));
        }
    }

    ref_count(p, gid, R);
}

/*lint -sem(gvrp_done_gid, thread_protected) */
static void gvrp_done_gid(struct garp_participant *p, struct garp_gid_instance *gid)
{
    gvrp_done_gid2(p, gid, -1);
}


// (3) ------ PDU encoder function

static const u8 const gvrp_header[] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x21, // Destination
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source
                                       0x00, 0x00,                         // Len/Type
                                       0x42, 0x42, 0x03,                   // LLC
                                       0x00, 0x01,                         // GARP protocol ID
                                       0x01                                // Attribute Type
                                      };


static int doing_tx_pdu;

/*
 * When transmitPDU! is generated for a participant, then this event is sent
 * to all applicant SMs.
 *
 * 1: Before this is done, then step_1 function is called
 *    in order to allocate and initialyze necessary resources.
 *
 * 2: Then the step_2 function is called for every applicant SM, that wants to
 *    put something in the PDU.
 *    If the PDU becomes full, then it is transmitted and a new is allocated,
 *    and the process continues.
 *
 * 3: When we are done, the step_3 is called to ensure, that the last PDU is sent.
 */
vtss_rc vtss_gvrp_tx_pdu_step_1(struct garp_participant *p)
{
    u8 *pdu;

    T_N1("Enter port=%d", p->port_no);

    // (1) --- Allocate PDU
    if ( !(pdu = vtss_mrp_mrpdu_tx_alloc(p->port_no, VTSS_GVRP_MAX_PDU_SIZE + 32, &p->context)) ) {
        //      T_D("GVRP: No memory for tx buf allocation");

        // Just to make sure
        p->pdu = 0;
        p->space_left = 0;

        T_E("Exit fail");
        return VTSS_RC_ERROR;
    }

    p->pdu2 = pdu;

    // (2) --- and initialyze the header
    memcpy(pdu, gvrp_header, sizeof(gvrp_header));
    pdu += sizeof(gvrp_header);

    p->pdu = pdu;
    p->space_left = VTSS_GVRP_MAX_PDU_SIZE - sizeof(gvrp_header);

    doing_tx_pdu = 1;

    T_N1("Exit OK");
    return VTSS_RC_OK;
}


/*
 * This function finish off the packet by:
 * - updateing the EtherNet len/type file with the
 *   actual payload length.
 * - If the packet is less then 64 bytes, the bytes upto
 *   64 is set it zero, so that unintended data is not sent.
 * Finally the packet is sent. If this fails, then we do nothing,
 * i.e., just pretent the packet has been lost.
 */
/*lint -sem(vtss_gvrp_tx_pdu_step_3, thread_protected) */
vtss_rc vtss_gvrp_tx_pdu_step_3(struct garp_participant *p)
{
    int rc = VTSS_RC_OK;
    u32 i, s;

    // Check if we really have put something in the packet.
    if (!doing_tx_pdu) {
        return rc;
    }

    i = s = VTSS_GVRP_MAX_PDU_SIZE - p->space_left;
    s -= 14;

    // Update Len/Type field
    p->pdu2[2 * 6] = s >> 8;
    p->pdu2[2 * 6 + 1] = s & 0xff;

    // If packet is less than 64, the write 0 from packet end to 64
    for (; i < 64; ++i) {
        p->pdu2[i] = 0;
    }

    doing_tx_pdu = 0;

#ifdef _GVRP_DUMP_PACKET_
    if ( !(VTSS_GVRP_MAX_PDU_SIZE - p->space_left > 128)) {
        printf("TX_PDU port=%d length=%d, pdu=%p\n", p->gid_index, VTSS_GVRP_MAX_PDU_SIZE - p->space_left, p->pdu2);
        dump_packet(VTSS_GVRP_MAX_PDU_SIZE - p->space_left > 128 ? 64 + 16 : VTSS_GVRP_MAX_PDU_SIZE - p->space_left, p->pdu2);
    }
#endif

    if (vtss_mrp_mrpdu_tx(p->port_no, p->pdu2, VTSS_GVRP_MAX_PDU_SIZE - p->space_left, p->context)) {
        rc = VTSS_RC_ERROR;
    }

    p->pdu = p->pdu2 = 0;

    return rc;
}


/*lint -sem(vtss_gvrp_tx_pdu_step_2, thread_protected) */
vtss_rc vtss_gvrp_tx_pdu_step_2(struct garp_participant *p, struct garp_gid_instance *gid, enum garp_attribute_event attribute_event)
{
    vtss_rc rc;

    T_N1("Enter doing_tx_pdu=%d", doing_tx_pdu);

    if (!doing_tx_pdu)
        if ( (rc = vtss_gvrp_tx_pdu_step_1(p)) ) {
            return rc;
        }

    // (2) --- Begin building GARP PDU [IEEE 802.1D-2004, clause 12]
    //         The PDU BNF is reduced, compared to 12.10.1.2 since
    //         only one AttributeType exist for GVRP:
    //
    //         GARP PDU  ::=  ProtocolID Message EndMark
    //         ProtocolID SHORT  ::=  1
    //         Message  ::=  AttributeType AttributeList
    //         AttributeType BYTE  ::= 1
    //         AttributeList  ::= Attribute+ EndMark
    //         Attribute  ::= OrdinaryAttribute | LeaveAllAttribute
    //         OrdinaryAttribute  ::=  AttributeLength AttributeEvent AttributeValue
    //         LeaveAllAttribute  ::=  AttributeLength LeaveAllEvent
    //         AttributeLength BYTE  ::=  2-255
    //         AttributeEvent BYTE  ::=  JoinEmpty(1) | JoinIn(2) | LeaveEmpty(3) | LeaveIn(4) | Empty(5)
    //         LeaveAllEvent BYTE  ::=  LeaveAll(0)
    //         EndMark  ::=  0x00 | "End of PDU"

    if (p->space_left >= 4) {

        if (garp_attribute_event__LeaveAll != attribute_event) {
            // --- OrdinaryAttribute
            SET_PDU_1(p->pdu, 4);
            SET_PDU_1(p->pdu, attribute_event);
            SET_PDU_1(p->pdu, GET_VID(gid) >> 8);
            SET_PDU_1(p->pdu, GET_VID(gid) & 0xff);
            p->space_left -= 4;

        } else {

            // --- LeaveAllAttribute
            SET_PDU_1(p->pdu, 2);
            SET_PDU_1(p->pdu, 0);
            p->space_left -= 2;

        }

    } else {
        T_N1("StepTwo.7");

        // --- This PDU is full, so:
        //     - Transmit it.
        //     - Allocate a new buffer, and try again.
        if ( (rc = vtss_gvrp_tx_pdu_step_3(p)) ) {
            return rc;
        }
        if ( (rc = vtss_gvrp_tx_pdu_step_1(p)) ) {
            return rc;
        }
        (void)vtss_gvrp_tx_pdu_step_2(p, gid, attribute_event); // This will succeed
    }

    T_N1("Exit");

    return VTSS_RC_OK;
}



// (4) ------ PDU decoder function

static int parse_message(int port_no, const u8 **pdu, int len);
static int parse_attribute(int port_no, const u8 **pdu, int len);
static void gvrp_gid_event(int port_no, int attribute_value, enum garp_attribute_event attribute_event);


int vtss_gvrp_rx_pdu(int port_no, const u8 *pdu, int len)
{
    u16 protocol_id;
    int len2;

    T_EXTERN("Hej port=%d", port_no);

    GVRP_CRIT_ENTER();

    // (1) --- If this port is not GVRP enabled, then exit
    if (!IS_PORT_MGMT_ENABLED(port_no)) {
        GVRP_CRIT_EXIT();
        return 0;
    }

    T_EXTERN("Enter port_no=%d pdu_len=%d", port_no, len);

    // (2) --- Jump over the header
    len2 = pdu[12] << 8 | pdu[13];
    len2 -= 3; // Not counting LLC

    len -= sizeof(gvrp_header) - 3;
    len = len < len2 ? len : len2;

#ifdef _GVRP_DUMP_PACKET_
    if (port_no == 175)
        if ( !(len + sizeof(gvrp_header) - 3 > 128)) {
            printf("RX_PDU port=%d\n", port_no);
            dump_packet(len + sizeof(gvrp_header) - 3 > 128 ? 64 + 16 : len + sizeof(gvrp_header) - 3, pdu);
        }
#endif

    pdu += sizeof(gvrp_header) - 3;

    if (len < 2) {
        T_E("length too short len=%d", len);
        GVRP_CRIT_EXIT();
        return -1;
    }


    // (3) --- Check protocol ID
    protocol_id = GET_PDU_1(pdu, len);
    protocol_id <<= 8;
    protocol_id |= GET_PDU_1(pdu, len);

    if (1 != protocol_id) {
        T_E("protocol_id=%d", protocol_id);
        GVRP_CRIT_EXIT();
        return -1;
    }

    T_N1("X.1 len=%d", len);

    // (4) --- Parse message list - one message at the time.
    while (len > 0) {
        len = parse_message(port_no, &pdu, len);
        T_N1("X.2 len=%d", len);
    }

    GVRP_CRIT_EXIT();

    return len;
}


static int parse_message(int port_no, const u8 **pdu, int len)
{
    int attribute_type;

    // (1) --- Attribute type
    //  if (len<1) return -1; Not necessary
    attribute_type = GET_PDU_1(*pdu, len);
    if (1 != attribute_type) {
        T_E("attribute_type=%d", attribute_type);
        return -1;
    }

    // (2) --- Parse AttributeList
    do {

        T_N1("--- parse_message.1 pdu=%p len=%d", *pdu, len);
        len = parse_attribute(port_no, pdu, len);
        if (len < 0) {
            return -1;
        }
        T_N1("--- parse_message.2 pdu=%p len=%d", *pdu, len);

        // Continue if no EndMark, i.e. if 1) pdu is not empty OR 2) next byte is not EndMark(0x00)
    } while (len && *(const u8 *)(*pdu) );

    return *(const u8 *)(*pdu) ? len : 0;
}


static int parse_attribute(int port_no, const u8 **pdu, int len)
{
    int attribute_length;
    int attribute_event;
    int attribute_value;

    T_N1("X.3 len=%d", len);

    // --- There must be at least 2 bytes left
    if (len < 2) {
        return -1;
    }

    attribute_length = GET_PDU_1(*pdu, len);
    attribute_event = GET_PDU_1(*pdu, len);

    T_N1("X.4 attr_len=%d, event=%d", attribute_length, attribute_event);

    if (attribute_length == 4) {

        // --- Ordinary attribute
        if (len < 2) {
            T_E("length=%d should be >=2", len);
            return -1;
        }

        attribute_value = GET_PDU_1(*pdu, len);
        attribute_value <<= 8;
        attribute_value |= GET_PDU_1(*pdu, len);

        // --- VID range is [1;4095]
        if ( !(  (attribute_value > 0 && attribute_value < NO_VLAN) && (attribute_event > 0 && attribute_event < 6) ) ) {
            T_E("X.5 attribute_value=%d, attribute_event=%d", attribute_value, attribute_event);
            return -1;
        }

    } else {
        // --- LeaveAll attribute
        //     attribute_value not defined. This event is for attribute_type.
        if ( 0 != attribute_event ) {
            T_E("attribute_event=%d", attribute_event);
            return -1;
        }
        attribute_value = 0;
    }

    // --- Attribute OK. Send attribute_event to (port,attribute_value) GID instance.
    gvrp_gid_event(port_no, attribute_value, attribute_event);

    return len;
}

static void gvrp_gid_event(int port_no, int attribute_value, enum garp_attribute_event attribute_event)
{
    vtss_rc rc;
    struct garp_participant  *p;
    struct garp_gid_instance *gid;
    u16 V;


    T_N1("Enter port_no=%d, attribute_value=%d, attribute_event=%d", port_no, attribute_value, attribute_event);

    if ( !PORT_RANGE_CHECK(port_no) ) {
        return;
    }

    p = &GVRP_participant[port_no];

    switch (attribute_event) {

    case garp_attribute_event__LeaveAll:
        vtss_garp_leaveall(&GVRP_application, p);
        break;

    default:
        V = attribute_value;
        gid = p->get_gid_TypeValue2obj(p, 1, &V);
        if (gid) {
            rc = vtss_garp_gidtt_rx_pdu_event_handler(&GVRP_application, p, gid, attribute_event);
            if (rc) {
                T_E("port=%d, vid=%d, ribute_event=%d", GET_PORT(p), GET_VID(gid), attribute_event);
            }
            gvrp_done_gid(p, gid);
        } else {
            if (warning_limit[1] < 5) {
                T_W("GVRP-PDU on port %d for vid %d dropped due to limited resources", port_no, attribute_value);
                ++warning_limit[1];
            }
        }


        break;
    }
}

// (5) ------ GIP part
/*lint -sem(gvrp_restore_gip, thread_protected) */
static void gvrp_restore_gip(struct garp_gid_instance *gid, struct garp_participant *p)
{
    int rc;
    u8 msti;
    vtss_vid_t vid;

    vid = GET_VID(gid);

    rc = mrp_mstp_index_msti_mapping_get(vid, &msti);
    if (rc || msti > NO_GIP_CONTEXTS) {
        T_E("VID->MSTI mapping failed. rc=%d, vid=%d, msti=%d", rc, vid, msti);
        msti = 0;
    }

    T_N("gvrp_restore_gip, msti=%d", msti);

    GVRP_application.iter__gip.R = &p->gip[msti];
    GVRP_application.iter__gip.N = &p->gip[msti];
    GVRP_application.iter__gip.msti = msti;
}

/*lint -sem(gvrp_next_gip, thread_protected) */
static struct garp_participant *gvrp_next_gip(void)
{
    u8 msti = GVRP_application.iter__gip.msti;

    GVRP_application.iter__gip.N = GVRP_application.iter__gip.N->next;

    T_N1("  gvrp_next_gip %p %p", GVRP_application.iter__gip.R, GVRP_application.iter__gip.N);

    if (!GVRP_application.iter__gip.N ||
        GVRP_application.iter__gip.R == GVRP_application.iter__gip.N) {
        return 0;
    } else {
        return MEMBER(struct garp_participant, gip[msti], GVRP_application.iter__gip.N);
    }
}

/*
 *  calc_registrations
 *
 * Count the number registrar SMs in state IN for the garp attribute represented by 'gid', except for
 * the regisrar SM in 'gid' itself.
 * The GIP context is given by msti, which in turn is given by vid, and thereby 'gid'.
 */
static int calc_registrations(struct garp_participant *p, struct garp_gid_instance *gid, struct garp_participant **pp)
{
    int  rc;
    u8   msti;
    int  count = 0;
    vtss_vid_t  vid;
    struct PN_participant *next_p, *this_p;
    struct garp_gid_instance *gid_array;

    // (1) --- Retrive the vid, the associated msti, and the gid_array
    vid = GET_VID(gid);
    gid_array = GET_GID_ARRAY(gid);

    rc = mrp_mstp_index_msti_mapping_get(vid, &msti);
    if (rc || msti > NO_GIP_CONTEXTS) {
        T_E("VID->MSTI mapping failed. rc=%d, vid=%d, msti=%d", rc, vid, msti);
        msti = 0;
    }


    // (2) --- Count the number og registrar in state IN, not counting 'gid',
    //         for this vid in the GIB context wrt msti.
    //         The 1st participant that meat these requirements, are returned in 'pp'.
    this_p = &p->gip[msti];
    next_p = this_p->next;

    // --- Return 0, if P is not in the gip[msti] list.
    if (!next_p) {
        return 0;
    }

    // --- So P is in the gip[msti] list, so count registrar SMs in IN state.
    for (next_p = this_p->next; next_p != this_p; next_p = next_p->next) {

        p = MEMBER(struct garp_participant, gip[msti], next_p);

        gid = &gid_array[p->gid_index];
        if (garp_registrar_state__IN == gid->registrar_state) {
            if (0 == count) {
                *pp = p;//MEMBER(struct garp_participant, gip[msti], next_p);
            }
            ++count;
            if (2 == count) {
                break;    // No reason to continue beyond this
            }
        }
    }

    return count;
}



// (6) ------ Management functions


/*
 * GID_Join.request to attribute (type,value) for (port_no, vid).
 */
/*lint -sem(vtss_gvrp_join_request, thread_protected) */

vtss_rc vtss_gvrp_join_request(int port_no, vtss_vid_t vid)
{
    vtss_rc rc = VTSS_RC_ERROR;

    // --- ICLI has done the protection
    // --- GVRP_CRIT_ENTER();

    if ( PORT_RANGE_CHECK(port_no) && VID_RANGE_CHECK(vid) ) {

        rc = vtss_gid_join_request(&GVRP_application,
                                   &GVRP_participant[port_no],
                                   1,
                                   (void *)&vid);

    }

    // --- GVRP_CRIT_EXIT();
    return rc;
}



/*
 * GID_Leave.request to attribute (type,value) for (port_no, vid).
 */
/*lint -sem(vtss_gvrp_leave_request, thread_protected) */
vtss_rc vtss_gvrp_leave_request(int port_no, vtss_vid_t vid)
{
    vtss_rc rc = VTSS_RC_ERROR;

    // --- ICLI has done the protection
    // --- GVRP_CRIT_ENTER();

    if ( PORT_RANGE_CHECK(port_no) && VID_RANGE_CHECK(vid) ) {

        rc = vtss_gid_leave_request(&GVRP_application,
                                    &GVRP_participant[port_no],
                                    1,
                                    (void *)&vid);
    }

    // --- GVRP_CRIT_EXIT();

    return rc;
}


/*
 * Set protocol timers: txPDU, leave, leaveAll.
 */
vtss_rc vtss_gvrp_set_timer(enum timer_context tc, u32 t)
{
    vtss_rc rc;

    GVRP_CRIT_ENTER();

    if (!GVRP_application_partial_init) {
        vtss_garp_init(&GVRP_application);
        GVRP_application_partial_init = 1;
    }

    rc = vtss_garp_set_timer(&GVRP_application, tc, t, GVRP_application_partial_init == 2 ? 1 : 0);

    GVRP_CRIT_EXIT();
    return rc;
}

/*lint -sem(vtss_gvrp_get_timer, thread_protected) */
u32 vtss_gvrp_get_timer(enum timer_context tc)
{
    u32 t;

    GVRP_CRIT_ENTER();

    if (!GVRP_application_partial_init) {
        vtss_garp_init(&GVRP_application);
        GVRP_application_partial_init = 1;
    }

    t = vtss_garp_get_timer(&GVRP_application, tc);

    GVRP_CRIT_EXIT();

    return t;
}


// (7) ------ ICLI show support functions

/*
 * Called by ICLI when a gid object is wanted.
 * The ref_count is incremented so that it does not disapear. Therefore
 * vtss_gvrp_gid_done() must be called when we are done
 */
/*lint -sem(vtss_gvrp_gid, thread_protected) */

vtss_rc vtss_gvrp_gid(int port_no, vtss_vid_t vid, struct garp_gid_instance **gid)
{
    // --- Range check AND test if port is enabled.
    if ( !(PORT_RANGE_CHECK(port_no) && VID_RANGE_CHECK(vid) ) ) {
        return VTSS_RC_ERROR;
    }
    if (!IS_PORT_MGMT_ENABLED(port_no)) {
        return VTSS_RC_ERROR;
    }

    *gid = gvrp_get_gid_obj(&GVRP_participant[port_no], vid, GVRP_ALLOC_ICLI);
    //    if (2==alloc_book_keeper[GVRP_ALLOC_DEFAULT].count) T_EXTERN2("X.4");

    return *gid ? VTSS_RC_OK : VTSS_RC_ERROR;
}


/*lint -sem(vtss_gvrp_gid_done, thread_protected) */
void vtss_gvrp_gid_done(struct garp_gid_instance **gid)
{
    gvrp_done_gid((*gid)->participant, *gid);

    *gid = 0;
}


/*lint -sem(vtss_gvrp_participant, thread_protected) */
vtss_rc vtss_gvrp_participant(int port_no, struct garp_participant **p)
{
    if ( !PORT_RANGE_CHECK(port_no) ) {
        return VTSS_RC_ERROR;
    }

    *p = &GVRP_participant[port_no];
    return VTSS_RC_OK;
}


/*lint -sem(vtss_gvrp_txPDU_timeout, thread_protected) */
u32 vtss_gvrp_txPDU_timeout(struct garp_gid_instance *gid)
{
    cyg_tick_count_t t;

    t = vtss_garp_get_clock(&GVRP_application, GARP_TC__transmitPDU);
    t = gid->participant->GARP_transmitPDU_timeout - t;

    if (t > GARP_POSITIVE_U32) {
        t = 0;
    }

    t = ( t << GARP_SCALE_UNIT ) / GVRP_application.GARP_timer[GARP_TC__transmitPDU].scale;

    return VTSS_OS_TICK2MSEC(t) / 10;
}


/*lint -sem(vtss_gvrp_leave_timeout, thread_protected) */
int vtss_gvrp_leave_timeout(struct garp_gid_instance *gid)
{
    cyg_tick_count_t t;

    t = vtss_garp_get_clock(&GVRP_application, GARP_TC__leavetimer);
    t = gid->leave_timeout - t;
    if (t > GARP_POSITIVE_U32) {
        t = 0;
    }

    if (gid->leave_list.next == 0) {
        return -1;
    }

    t = ( t << GARP_SCALE_UNIT ) / GVRP_application.GARP_timer[GARP_TC__leavetimer].scale;

    return VTSS_OS_TICK2MSEC(t) / 10;
}


/*lint -sem(vtss_gvrp_leave_timeout, thread_protected) */
int vtss_gvrp_gip_context(struct garp_gid_instance *gid)
{
    u8 msti;

    msti = GET_MSTI(gid);

    return IS_IN_GIP_CONTEXT(gid->participant, msti) ? msti : -1;
}


/*lint -sem(vtss_gvrp_leaveall_timeout, thread_protected) */
u32 vtss_gvrp_leaveall_timeout(int port_no)
{
    cyg_tick_count_t t;
    struct garp_participant *p;

    T_EXTERN("port(%d)", (int)port_no);

    if ( !PORT_RANGE_CHECK(port_no) ) {
        T_E("port_no=%d out of range", port_no);
        return 0;
    }

    p = &GVRP_participant[port_no];

    t = vtss_garp_get_clock(&GVRP_application, GARP_TC__leavealltimer);
    t = p->leaveAll_timeout - t;
    if (t > GARP_POSITIVE_U32) {
        t = 0;
    }

    t = ( t << GARP_SCALE_UNIT ) / GVRP_application.GARP_timer[GARP_TC__leavealltimer].scale;

    return VTSS_OS_TICK2MSEC(t) / 10;
}



// (8) ------ Interface for timer tick

uint vtss_gvrp_timer_tick(uint delay)
{
    uint rc = 0;

    T_EXTERN("delay(%d)", (int)delay);


    T_N1("Enter delay=%u", delay);
    GVRP_CRIT_ENTER();

    if (GVRP_participant_max) {
        rc = vtss_garp_timer_tick(delay, &GVRP_application);
    }

    GVRP_CRIT_EXIT();
    return rc;
}


// (9) ------


static void join_indication(u32 port_no, vtss_mrp_attribute_type_t *attr_type,
                            u16 joining_mad_index, BOOL new)
{}
static void leave_indication(u32 port_no, vtss_mrp_attribute_type_t *attr_type,
                             u32 leaving_mad_index)
{}
static void join_propagated(void *x, void *mad, u32 joining_mad_index, BOOL new)
{}
static void leave_propagated(void *x, void *mad, u32 leaving_mad_index)
{}
static u32  transmit(u32 port_no, u8 *all_attr_events, u32 no_events, BOOL la_flag)
{
    return 0;
}
static BOOL receive(u32 port_no, const u8 *pdu, u32 length)
{
    return 0;
}
static void added_port(void *x, u32 port_no)
{}                                         /**< Port add function              */
static void removed_port(void *x, u32 port_no)
{}


u32 vtss_gvrp_global_control_conf_create(vtss_mrp_t **mrp_app)
{
    vtss_mrp_t      *mvrp_app;
    u32             rc = VTSS_XXRP_RC_OK;

    T_N1("Enter");

    /* Create mrp application structure and fill it. This will be freed in MRP on
       global disable of the MRP application  */
    mvrp_app = (vtss_mrp_t *) XXRP_SYS_MALLOC(sizeof(vtss_mrp_t));
    mvrp_app->appl_type = VTSS_GARP_APPL_GVRP;
    mvrp_app->mad = NULL;
    mvrp_app->map = NULL;
    mvrp_app->max_mad_index = NO_VLAN - 1;
    mvrp_app->join_indication_fn = join_indication;
    mvrp_app->leave_indication_fn = leave_indication;
    mvrp_app->join_propagated_fn = join_propagated;
    mvrp_app->leave_propagated_fn = leave_propagated;
    mvrp_app->transmit_fn = transmit;
    mvrp_app->receive_fn = receive;
    mvrp_app->added_port_fn = added_port;
    mvrp_app->removed_port_fn = removed_port;
    *mrp_app = mvrp_app;

    return rc;
}


// (10) ------ Handling of port add/remove from management

static int participant_to_l2port(struct garp_participant  *p)
{
    return p->gid_index;
}



static void gvrp_restore__port_enabled(struct garp_participant *p)
{
    GVRP_application.iter__port_enable.R = &p->enabled_port;
    GVRP_application.iter__port_enable.N = &p->enabled_port;
    GVRP_application.iter__port_enable.done = !IS_PORT_MGMT_ENABLED2(p);

    // --- Sanity check
    if (GVRP_application.iter__port_enable.done) {
        T_E("p is not in port enabled list");
    }
}


static struct garp_participant *gvrp_next__port_enabled(void)
{
    struct garp_participant *p;

    if (GVRP_application.iter__port_enable.done) {
        return 0;
    }

    p = MEMBER(struct garp_participant, enabled_port, GVRP_application.iter__port_enable.N);

    GVRP_application.iter__port_enable.N = GVRP_application.iter__port_enable.N->next;

    if (GVRP_application.iter__port_enable.N == GVRP_application.iter__port_enable.R) {
        GVRP_application.iter__port_enable.done = TRUE;
    }

    return p;
}


/*
 *  GID_Join_indication
 *
 *  This function is called when registrar emit a IndJoin,
 *  see IEEE 802.1D-2004, table 12-4.
 */
/*lint -sem(GID_Join_indication, thread_protected) */
static void GID_Join_indication(struct garp_participant *p, struct garp_gid_instance *gid)
{
    u32 rc;
    vtss_vid_t vid = GET_VID(gid);

    T_DG(TRACE_GRP_GVRP, "port=%d, vid=%d vid_ref_count=%d",
         GET_PORT(p), GET_VID(gid), GVRP_vid2gid[vid].vid_ref_count);

    // --- ref_count is going from 0 -> 1, then enable all gvrp enabled ports for this vid
    if (!GVRP_vid2gid[vid].vid_ref_count) {
        for (gvrp_restore__port_enabled(p); (p = gvrp_next__port_enabled()); ) {

            T_DG(TRACE_GRP_GVRP, "  Adding port=%d from vid=%d", GET_PORT(p), vid);
            if ( (rc = XXRP_gvrp_vlan_port_membership_add(GET_PORT(p), vid) ) ) {
                T_E("GID_Join_indication rc=%u", rc);
            }
        }
    }

    ++GVRP_vid2gid[vid].vid_ref_count;
}

/*
 * When a port is added late, then the vlan membership must be updated
 * since GID_Join_indication() could not do that, since the port 'p' was not
 * GVRP enabled at the time.
 */
static void GID_Join_indication_update(struct garp_participant *p)
{
    vtss_vid_t vid;
    vtss_rc rc;

    T_DG(TRACE_GRP_GVRP, "port=%d", GET_PORT(p));

    for (vid = 0; vid < NO_VLAN; ++vid) {

        if (GVRP_vid2gid[vid].vid_ref_count) {
            T_DG(TRACE_GRP_GVRP, "  Adding port=%d from vid=%d", GET_PORT(p), vid);
            if ( (rc = XXRP_gvrp_vlan_port_membership_add(GET_PORT(p), vid) ) ) {
                T_E("Failed to add vlan %d to port %d, rc=%u", vid, GET_PORT(p), rc);
            }
        }
    }
}


/*
 *  GID_Leave_indication
 *
 *  This function is called when registrar emit a IndLeave,
 *  see IEEE 802.1D-2004, table 12-4.
 */
/*lint -sem(GID_Leave_indication, thread_protected) */
static void GID_Leave_indication(struct garp_participant *p, struct garp_gid_instance *gid)
{
    u32 rc;
    vtss_vid_t vid = GET_VID(gid);

    T_DG(TRACE_GRP_GVRP, "port=%d, vid=%d vid_ref_count=%d",
         GET_PORT(p), GET_VID(gid), GVRP_vid2gid[vid].vid_ref_count);

    --GVRP_vid2gid[vid].vid_ref_count;

    // --- ref_count is going from 1 -> 0, then disable all gvrp enabled ports for this vid
    if (!GVRP_vid2gid[vid].vid_ref_count) {

        for (gvrp_restore__port_enabled(p); (p = gvrp_next__port_enabled()); ) {

            T_DG(TRACE_GRP_GVRP, "  Deleting port=%d from vid=%d\n", GET_PORT(p), vid);

            if ( (rc = XXRP_gvrp_vlan_port_membership_del(GET_PORT(p), vid) ) ) {
                T_E("GID_Leave_indication rc=%u", rc);
            }
        }
    }
}


/*
 * When a port is remove early, then the vlan membership must be updated
 * since GID_Leave_indication() will not do that, since the port 'p' was not
 * GVRP enabled at that time.
 */
static void GID_Leave_indication_update(struct garp_application *a, vtss_vid_t vid)
{
    vtss_rc rc;
    struct garp_participant  *p;

    T_DG(TRACE_GRP_GVRP, " port_enabled_list=%p  vid=%d vid_ref_count=%d",
         a->port_enabled_list, vid, GVRP_vid2gid[vid].vid_ref_count);

    // --- If no ports are GVRP enabled, then just exit.
    if ( !(a->port_enabled_list) ) {
        return;
    }

    // --- ref_count is going from 1 -> 0, then disable all gvrp enabled ports for this vid
    if (!GVRP_vid2gid[vid].vid_ref_count) {

        p = MEMBER(struct garp_participant, enabled_port, a->port_enabled_list);

        for (gvrp_restore__port_enabled(p); (p = gvrp_next__port_enabled()); ) {

            T_DG(TRACE_GRP_GVRP, "  Deleting port=%d from vid=%d\n", GET_PORT(p), vid);

            if ( (rc = XXRP_gvrp_vlan_port_membership_del(GET_PORT(p), vid) ) ) {
                T_E("GID_Leave_indication rc=%u", rc);
            }
        }
    }
}


/*
 *  This function will mgmt port enable, i.e., added to the participant 'port_enabled_list'.
 *  The port is not added to GIP context here, but by update().
 */
static vtss_rc vtss_port_add_participant(struct garp_application *a, struct garp_participant  *p)
{
    //    vtss_stp_state_t st;
    uchar msti;
    vtss_rc rc;
    vtss_common_port_t l2port;
    vtss_vid_t vid;

    T_EXTERN("+ Add participant %d", GET_PORT(p));

    // --- Too stack expensive; and function is already semaphore protected.
    static  vlan_registration_type_t     port_registration[VLAN_ID_MAX + 1];

    // --- Check that participant is not already part of a list
    if (IS_PORT_MGMT_ENABLED2(p)) {
        return VTSS_RC_ERROR;
    }

    l2port = participant_to_l2port(p);

    // --- Get VLAN state, i.e., Normal,Fixed,Forbidden. We'll set it later in (*),
    //     but get it here if it fails, in order to prevent un-doing stuff.
    if ( (rc = xxrp_mgmt_vlan_state(l2port, port_registration)) ) {
        return rc;
    }

    // --- Add to port enabled list
    vtss_garp_add_list(&a->port_enabled_list, &p->enabled_port);


    // --- Add vlan membership of this port. This updated the work done
    //     by GID_Join_indication.
    GID_Join_indication_update(p);


    // ?? This should be handled by vtss_gvrp_mstp_port_state_change_handler()
#if 0
    // --- Add to relevant GIP context lists
    for (msti = 0; msti < NO_MSTI; ++msti) {

        st = l2_get_msti_stpstate(msti, l2port);
        p->port_msti_enabled[msti] = (VTSS_STP_STATE_FORWARDING == st ? TRUE : FALSE);

    }
#endif


    // (*) --- Update administrative state, if not 'Normal', since 'Normal' is default
    //         and probably most common.
    for (vid = 1; vid < NO_VLAN; ++vid) {

        if (port_registration[vid] != VLAN_REGISTRATION_TYPE_NORMAL) {
            rc = vtss_gvrp_registrar_administrative_control2(l2port, vid, port_registration[vid]);
            if (rc && 0) {
                T_E("l2port=%d, vid=%d, reg=%d rc=%d", l2port, vid, port_registration[vid], rc);
            }
        }
    }

    // --- Update this ports Applicant SMs.
    //     The standard does not say we shall do this,
    //     but in order to bring Applicant SMs up to date
    //     with Fixed Registrar SMs, we do this anyway.
    for (msti = 0; msti < NO_MSTI; ++msti) {
        (void)update_port_state(p,  msti);
    }


    return VTSS_RC_OK;
}

static void vtss_gid_remove(struct garp_application *a, struct garp_participant  *p)
{
    struct garp_gid_instance *gid, *gid_array;
    vtss_vid_t vid;
    vtss_rc rc;
    int count_part;

    for (vid = 1; vid < NO_VLAN; ++vid) {
        gid_array = GVRP_vid2gid[vid].gid;
        if (gid_array) {

            gid = &gid_array[GET_PORT(p)];

            count_part = gid->registrar_state == garp_registrar_state__MT ? 0 : 1;

            // --- If in used, then:
            //     - Put applicant and registrar SMs to init state.
            //     - Remove gid from leave list.
            //     - Decrement ref count to zero.
            if (gid->reference_count) {
                gid->registrar_state = garp_registrar_state__MT;
                gid->applicant_state = garp_applicant_state__VO;

                vtss_garp_remove_list(&a->GARP_timer_list_leave, &gid->leave_list);

                gvrp_done_gid2(p, gid, -gid->reference_count);
            }
        } else {

            count_part = 0;

        }

        // --- If vid_ref_count is not 0, then this port has been added to
        //     vlan earlier. So it must be deleted.
        if (GVRP_vid2gid[vid].vid_ref_count) {
            if ( (rc = XXRP_gvrp_vlan_port_membership_del(GET_PORT(p), vid) ) ) {
                T_E("Failed to add vlan %d to port %d, rc=%u", vid, GET_PORT(p), rc);
            }
        }

        // --- If gid was in IN or LV, then it has contributed to the vid_ref_count,
        //     so that is updated here. If the gid entered MT before the participant
        //     is disabled, then GID_Leave_indication() would do that.

        GVRP_vid2gid[vid].vid_ref_count -= count_part;

        // --- If vid_ref_count has been decremented to 0, then all GVRP enabled ports shall be deleted
        //     in vlan module for that vid.
        if (count_part) {
            GID_Leave_indication_update(a, vid);
        }
    }
}


static vtss_rc vtss_port_remove_participant(struct garp_application *a, struct garp_participant  *p)
{
    uchar msti;

    // --- Check that participant is in 'port enabled' list. If not, then just exit.
    if (!IS_PORT_MGMT_ENABLED2(p)) {
        return VTSS_RC_ERROR;
    }

    // --- Remove from 'port enabled' list and 'leaveAll' list
    vtss_garp_remove_list(&a->port_enabled_list, &p->enabled_port);
    vtss_garp_remove_list(&a->GARP_timer_list_leaveAll, &p->leaveAll_participant_list);

    // --- Signal peer applicants, that we, i.e. p, is about to be remove
    for (msti = 0; msti < NO_MSTI; ++msti) {
        (void)update_port_state(p,  msti);
    }

    // --- Reset all GIDs in this participant
    vtss_gid_remove(a, p);

    return VTSS_RC_OK;
}


vtss_rc vtss_gvrp_port_control_conf_get(u32 port_no, BOOL *const status)
{
    T_EXTERN("port(%d)", (int)port_no);

    GVRP_CRIT_ENTER();

    // --- If GVRP not enabled, that just return FALSE
    if (!GVRP_participant_max) {
        *status = FALSE;
        GVRP_CRIT_EXIT();

        return VTSS_RC_OK;
    }

    if ( !((int)port_no < GVRP_participant_max) ) {
        GVRP_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    *status = IS_PORT_MGMT_ENABLED(port_no);

    GVRP_CRIT_EXIT();
    return VTSS_RC_OK;
}

/*
 *  gvrp_setup_vlan_msti_map
 *
 * This function generate a list per msti, containing all the vid in that msti.
 */
static void gvrp_setup_vlan_msti_map(void)
{
    int vid, i, msti;
    mstp_msti_config_t conf;

    // (1) --- Get entire vid->msti map
    (void)mstp_get_msti_config(&conf, 0);

    // (2) --- Generate list msti->vid list
    for (i = 0; i < NO_MSTI; ++i) {
        msti_list[i] = 0xffff;
    }

    // (3) --- The list is generated so that elements with gid!=0
    //         comme first, and then gid==0
    for (vid = NO_VLAN - 1; vid > 0; --vid) {
        if (GVRP_vid2gid[vid].gid == 0) {
            msti = conf.map.map[vid];

            GVRP_vid2gid[vid].next_vlan = msti_list[msti];
            GVRP_vid2gid[vid].old_msti = GVRP_vid2gid[vid].msti;
            GVRP_vid2gid[vid].msti = msti; // for easy vid->msti lookup
            msti_list[msti] = vid;
        }
    }

    for (vid = NO_VLAN - 1; vid > 0; --vid) {
        if (GVRP_vid2gid[vid].gid) {
            msti = conf.map.map[vid];

            GVRP_vid2gid[vid].next_vlan = msti_list[msti];
            GVRP_vid2gid[vid].old_msti = GVRP_vid2gid[vid].msti;
            GVRP_vid2gid[vid].msti = msti; // for easy vid->msti lookup
            msti_list[msti] = vid;
        }
    }

}


static vtss_rc vtss_gvrp_port_control_conf_set2(u32 port_no, BOOL enable)
{
    struct garp_participant  *P;
    //    int msti;
    vtss_rc rc;

    if ( !((int)port_no < GVRP_participant_max) ) {
        return VTSS_RC_ERROR;
    }
    P = &GVRP_participant[port_no];

    // --- No change, so just exit
    if (enable == IS_PORT_MGMT_ENABLED(port_no)) {
        return VTSS_RC_OK;
    }

    // --- Add participant to port enabled list
    if (enable) {

        rc = vtss_port_add_participant(&GVRP_application, P);
        gvrp_setup_vlan_msti_map();

        // --- Kickstart the LeaveAll timer
        if (!rc) {
            vtss_garp_leaveall(&GVRP_application, P);
        }

    } else {

        rc = vtss_port_remove_participant(&GVRP_application, P);

    }



    return rc;
}


/*
 * This function is called from the platform when a port
 * shall be GVRP enabled or disabled
 */
vtss_rc vtss_gvrp_port_control_conf_set(u32 port_no, BOOL enable)
{
    vtss_rc rc;

    T_EXTERN("port(%d), status(%d)", (int)port_no, (int)enable);

    GVRP_CRIT_ENTER();
    rc = vtss_gvrp_port_control_conf_set2(port_no, enable);
    GVRP_CRIT_EXIT();

    return rc;
}



// (11) ------ Handling of port up/down for (port_no, msti)
struct {

    u16 next_vlan, this_vlan;
    struct PN_participant *src_p, *next_p;
    u8 msti;
    int gid_index;
    struct garp_participant *parcipant;
    struct garp_gid_instance *gid;
    enum gvrp_alloc_type alloc;
} gvrp_vid_state;


/*
 *  The following function is meant to be called in the fasion:
 *   for (gvrp_restore_vid(*,*); gid1=gvrp_next_vid();)
 *      ...
 *      for (gvrp_restore_gid(&p); (gid=gvrp_next_gid(&P)); )  {
 *
 */

static void gvrp_restore_vid(struct garp_participant *p, u8 msti)
{
    gvrp_vid_state.next_vlan = msti_list[msti];
    gvrp_vid_state.gid_index = p->gid_index;
    gvrp_vid_state.parcipant = p;
    gvrp_vid_state.msti = msti;
    gvrp_vid_state.gid = 0;
    gvrp_vid_state.alloc = GVRP_ALLOC_DEFAULT;

    T_N1("gvrp_restore_vid msti=%d next_vlan=%d, gid_index=%d", msti, gvrp_vid_state.next_vlan, gvrp_vid_state.gid_index);
}

static void gvrp_restore_vid2(struct garp_participant *p, u8 msti)
{
    gvrp_restore_vid(p, msti);
    gvrp_vid_state.alloc = GVRP_ALLOC_TX;
}


static struct garp_gid_instance *gvrp_next_vid(void)
{
    //  T_N1("gvrp_next_vid nextvlan=0x%x, %d", gvrp_vid_state.next_vlan, gvrp_vid_state.gid_index);

    // --- Potentially release resource
    gvrp_done_gid(gvrp_vid_state.parcipant, gvrp_vid_state.gid);

    // --- Exit if we are done
    if (gvrp_vid_state.next_vlan == 0xffff) {
        T_N1("--- gvrp_next_vid gid= 00");
        return 0;
    }

    gvrp_vid_state.this_vlan = gvrp_vid_state.next_vlan;
    gvrp_vid_state.next_vlan = GVRP_vid2gid[gvrp_vid_state.next_vlan].next_vlan;

    //  gid = &GVRP_vid2gid[gvrp_vid_state.next_vlan].gid[gvrp_vid_state.gid_index];
    //  gvrp_vid_state.gid = gvrp_vid_state.parcipant->get_gid_TypeValue2obj(gvrp_vid_state.parcipant, 1, &gvrp_vid_state.this_vlan);
    gvrp_vid_state.gid = gvrp_get_gid_obj(gvrp_vid_state.parcipant, *(vtss_vid_t *)&gvrp_vid_state.this_vlan, gvrp_vid_state.alloc);

    T_N1("--- gvrp_next_vid gid=%p, vlan=%d", gvrp_vid_state.gid, (int)gvrp_vid_state.this_vlan);

    return gvrp_vid_state.gid ;
}

static struct garp_gid_instance *gvrp_this_vid(vtss_vid_t vid)
{
    //  T_N1("gvrp_next_vid nextvlan=0x%x, %d", gvrp_vid_state.next_vlan, gvrp_vid_state.gid_index);

    // --- Potentially release resource
    gvrp_done_gid(gvrp_vid_state.parcipant, gvrp_vid_state.gid);

    // --- Exit if we are done
    if (gvrp_vid_state.next_vlan == 0xffff) {
        T_N1("--- gvrp_next_vid gid= 00");
        return 0;
    }

    gvrp_vid_state.this_vlan = vid;
    gvrp_vid_state.next_vlan = 0xffff;

    //  gid = &GVRP_vid2gid[gvrp_vid_state.next_vlan].gid[gvrp_vid_state.gid_index];
    gvrp_vid_state.gid = gvrp_vid_state.parcipant->get_gid_TypeValue2obj(gvrp_vid_state.parcipant, 1, &gvrp_vid_state.this_vlan);

    T_N1("--- gvrp_next_vid gid=%p, vlan=%d", gvrp_vid_state.gid, (int)gvrp_vid_state.this_vlan);

    return gvrp_vid_state.gid ;
}


static void gvrp_restore_gid(struct garp_participant *p)
{
    gvrp_vid_state.src_p = gvrp_vid_state.next_p = &p->gip[gvrp_vid_state.msti];
}


static struct garp_gid_instance *gvrp_next_gid(struct garp_participant **P)
{
    gvrp_vid_state.next_p = gvrp_vid_state.next_p->next;
    if (gvrp_vid_state.next_p == gvrp_vid_state.src_p) {
        return 0;
    }

    *P = MEMBER(struct garp_participant, gip[gvrp_vid_state.msti], gvrp_vid_state.next_p);

    return &GVRP_vid2gid[gvrp_vid_state.this_vlan].gid[ (*P)->gid_index ];
}


static int gvrp_add_transmitPDU_list(struct garp_participant *p, struct garp_gid_instance *gid)
{
    vtss_vid_t vid;

    vid = GET_VID(gid);

    if ( p->txPDU.tx_list[vid >> 5] & (1UL << (vid & 0x1f)) ) {
    } else {
        p->txPDU.count++;
        p->txPDU.tx_list[vid >> 5] |= (1UL << (vid & 0x1f));
        return p->txPDU.count;
    }

    // Not an error, just indicating, no changes has happened
    return -1;
}

static int gvrp_remove_transmitPDU_list(struct garp_participant *p, struct garp_gid_instance *gid)
{
    vtss_vid_t  vid;

    vid = GET_VID(gid);

    if ( p->txPDU.tx_list[ vid >> 5 ] & (1UL << (vid & 0x1f)) ) {
        p->txPDU.count--;
        p->txPDU.tx_list[ vid >> 5 ] &= ~(1UL << (vid & 0x1f));
        return p->txPDU.count;
    } else {
    }

    // Not an error, just indicating, no changes has happened
    return -1;
}

static void gvrp_restore_transmitPDU_list(struct garp_participant *p)
{
    p->txPDU.index = 0;
    p->txPDU.bit = p->txPDU.tx_list[0];
    p->txPDU.gid = 0;
}

/*lint -sem(gvrp_getnext_transmitPDU_list, thread_protected) */
static struct garp_gid_instance *gvrp_getnext_transmitPDU_list(struct garp_participant *p)
{
    garp_typevalue_t tv;
    int i, j;
    u32 tmp;

    gvrp_done_gid(p, p->txPDU.gid);

    for (tmp = p->txPDU.bit, i = p->txPDU.index; !tmp; tmp = p->txPDU.tx_list[i]) {
        if (!( ++i < ((NO_VLAN + 31) >> 5) )) {
            T_N1("--- getnext_transmitPDU_list  done");
            return 0;
        }
    }

    j = VTSS_OS_CTZ(tmp);

    p->txPDU.index = i;
    p->txPDU.bit = tmp & ~(1UL << j);

    tv = (i << 5) | j; // = i*32 + j

    //    p->txPDU.gid = gvrp_map_gid_TV2obj(p, tv);
    p->txPDU.gid = gvrp_get_gid_obj(p, (vtss_vid_t)tv, GVRP_ALLOC_TX);
    //    if (2==alloc_book_keeper[GVRP_ALLOC_DEFAULT].count) T_EXTERN2("X.6");

    if ( !(p->txPDU.gid) ) {
        T_EG(TRACE_GRP_GVRP, "Allocation failed in GVRP_ALLOC_TX");
    }

    T_N1("--- getnext_transmitPDU_list  i=%d, j=%d, tv=%d tmp=0x%x p->txPDU.bit=0x%x p->txPDU.index=0x%x", i, j, tv, tmp, p->txPDU.bit, p->txPDU.index);

    return p->txPDU.gid;
}


/*
 *  For gid 'gid' in participant 'p' update applicants
 *  in the GIP context telling for all other
 *
 */
static vtss_rc gvrp_update_enable(struct garp_participant  *p,
                                  struct garp_gid_instance *gid)
{
    vtss_rc rc = VTSS_RC_OK;

    struct garp_participant  *tmp_p;
    struct garp_gid_instance *tmp_gid;

    T_EXTERN("port=%d", GET_PORT(p));

    // --- Send ReqJoin to other Applicant SMs in this GIP context, if
    //     an attribute is registered.
    if (garp_registrar_state__MT        != gid->registrar_state &&
        garp_registrar_state__Forbidden != gid->registrar_state) {

        for (gvrp_restore_gid(p); (tmp_gid = gvrp_next_gid(&tmp_p)); )  {

            // --- Send ReqJoin to other Applicant SMs in this GIP context
            rc |= vtss_garp_gidtt_event_handler(&GVRP_application, tmp_p, tmp_gid, garp_event__ReqJoin);
        }
    }

    for (gvrp_restore_gid(p); (tmp_gid = gvrp_next_gid(&tmp_p)); )  {
        // --- If any associated Registrar SMs has a registration, then send Req-Join to
        //     my own Applicant SM.
        if (garp_registrar_state__MT        != tmp_gid->registrar_state &&
            garp_registrar_state__Forbidden != tmp_gid->registrar_state) {

            // --- If any other registrar SMs in this GIP context is in
            //     LV,IN or Fixed, i.e., a state which would have give P a
            //     ReqJoin had it been enabled; then record this, and eventually
            //     give 'gid' a ReqJoin.
            (void)vtss_garp_gidtt_event_handler(&GVRP_application, p, gid, garp_event__ReqJoin);
            break;
        }
    }

    return rc;
}


static vtss_rc gvrp_update_disable(struct garp_participant  *p,
                                   struct garp_gid_instance *gid)
{
    vtss_rc rc = VTSS_RC_OK;

    struct garp_participant  *tmp_p;
    struct garp_gid_instance *tmp_gid;

    struct garp_participant *pp = 0;
    int count;

    // --- The applicant for a disabled port must not signal an attribute.
    rc |= vtss_garp_gidtt_event_handler(&GVRP_application, p, gid, garp_event__ReqLeave);

    // --- Do nothing if attribute is not registrated
    if (garp_registrar_state__MT        == gid->registrar_state ||
        garp_registrar_state__Forbidden == gid->registrar_state) {
        return rc;
    }

    // --- Count how many others in the active topology has registrated this attribute.
    //     Let S is the new active topology i.e., with the port removed.
    //     If p is in S, then send leave to p if no ports in S\{p} has registrated
    //     on the attribute in question.
    //     Therefore, if count>1, then S\{p} will always contain ports on which there are
    //     registrations, so we shall do nothing.
    //     If count=0, S\{p} will never contain ports on which there are registrations, so
    //     so send leave to all ports in S.
    //     The last case is count=1. If p is the one that caused this count (pp in the code below), then S\{p}
    //     does not contain port with registrations, so send leave to p. But for all
    //     other values of p, S\{p} does contain registrations, so do noting for those p's.
    count = calc_registrations(p, gid, &pp);

    if (count > 1) {
        return rc;
    }

    for (gvrp_restore_gid(p); (tmp_gid = gvrp_next_gid(&tmp_p)); ) {
        if (0 == count || pp == tmp_p /* && 1==count, which is not necessary to check, since we know that*/) {

            rc |= vtss_garp_gidtt_event_handler(&GVRP_application, tmp_p, tmp_gid, garp_event__ReqLeave);
        }
    }

    return rc;
}


/*
 * This function is called by MSTP whenever a change in the
 * VLAN to MSTI mapping change.
 */
void vtss_gvrp_update_vlan_to_msti_mapping(void)
{
    struct garp_participant  *p;
    struct PN_participant *pn_end, *pn;
    vtss_vid_t vid;

    u8 old_msti, new_msti;

    GVRP_CRIT_ENTER();

    // --- If not created, then just exit.
    if ( !GVRP_participant_max ) {
        GVRP_CRIT_EXIT();
        return;
    }

    // --- Read the VLAN->MSTI mapping
    gvrp_setup_vlan_msti_map();

    // --- For every VLAN that has changed MSTI
    for (vid = 1; vid < NO_VLAN; ++vid) {

        old_msti = GVRP_vid2gid[vid].old_msti;
        new_msti = GVRP_vid2gid[vid].msti;

        // --- Only if the vid has changed msti there is something to do.
        if (old_msti == new_msti ) {
            continue;
        }

        T_DG(TRACE_GRP_GVRP, "vid=%d, msti: %d->%d", vid, old_msti, new_msti);

        // --- Make copy of old_msti to MSTI_COPY.
        vtss_garp_copy_list(GVRP_application.gip[new_msti], &GVRP_application.gip[MSTI_COPY],
                            offsetof(struct garp_participant, gip[new_msti]),
                            offsetof(struct garp_participant, gip[MSTI_COPY]));


        // --- Loop over GIP-context for old_msti.
        pn_end = pn = GVRP_application.gip[old_msti];
        if (pn) {
            do {
                p = MEMBER(struct garp_participant, gip[old_msti], pn);
                update_port_state2(p, vid, old_msti, new_msti, MSTI_COPY);

                pn = pn->next;
            } while (pn != pn_end);
        }

        // --- Loop over GIP-context for new_msti.
        pn_end = pn = GVRP_application.gip[new_msti];
        if (pn) {
            do {
                p = MEMBER(struct garp_participant, gip[new_msti], pn);
                update_port_state2(p, vid, old_msti, new_msti, MSTI_COPY);

                pn = pn->next;
            } while (pn != pn_end);
        }
    }

    GVRP_CRIT_EXIT();
}


/*
 *  vtss_gvrp_mstp_port_state_change_handler
 *
 * Called from xxrp.c::XXRP_mstp_state_change_handler() when the state of a (port,msti) change
 * from or to forwarding.
 * This is recorded in the appropiated participant, i.e. for the port,
 * no matter whether it is enabled for gvrp.
 *
 * After this the update function is called
 */
vtss_rc vtss_gvrp_mstp_port_state_change_handler(u32 port_no, u8 msti, vtss_mrp_mstp_port_state_change_type_t  port_state_type)
{
    vtss_rc rc = VTSS_RC_OK;
    struct garp_participant  *P;
    BOOL t, s;

    T_DG(TRACE_GRP_GVRP, "port=%d, msti=%d MSTI %s", port_no, msti, port_state_type == VTSS_MRP_MSTP_PORT_ADD ? "enabled" : "disabled");

    GVRP_CRIT_ENTER();

    // --- If gvrp is not created, then do not consider it an error.
    //     But if it is, then port must be in range.
    if ( GVRP_participant_max ) {

        if ( (int)port_no < GVRP_participant_max ) {

            P = &GVRP_participant[port_no];

            // (2) --- Store information in the participant, whether the msti is forwarding or not.
            t = (port_state_type == VTSS_MRP_MSTP_PORT_ADD) ? TRUE : FALSE;

            s = P->port_msti_enabled[msti];
            P->port_msti_enabled[msti] = t;

            t = (t && IS_PORT_MGMT_ENABLED2(P));
            s = (s && IS_PORT_MGMT_ENABLED2(P));

            // (3) --- Put participant in apropiated GIP context, and emit necessary join or leave
            //         events for that GIP. [IEEE 802.1d-2004, clause 12.2.3]
            if ( t != s ) {
                rc = update_port_state(P, msti);
            }

        } else {
            T_WG(TRACE_GRP_GVRP, "port_no=%d, GVRP_participant_max=%d", (int)port_no, (int)GVRP_participant_max);
            rc = VTSS_RC_ERROR;
        }
    }

    GVRP_CRIT_EXIT();

    return rc;
}



/*
 *  update_port_state
 *
 * For port represented by participant 'P' update the
 * GIP context wrt 'msti'; See (2) in the code.
 *
 * If 'P' is added or removed, then all others in this GIP context
 * shall be sent a Req_Join or Req_Leave
 */
static vtss_rc update_port_state(struct garp_participant  *P, u8 msti)
{
    vtss_rc rc = VTSS_RC_OK;
    BOOL enabled;
    struct garp_gid_instance *gid;

    T_EXTERN("(1) --- update_port_state msti=%d  port_msti_enabled[%d]=%d mgmt_enabled=%d\n",
             (int)msti, (int)msti, (int)P->port_msti_enabled[msti], IS_PORT_MGMT_ENABLED2(P));

    // (1) --- Check if port enable state for this msti has actually changed. If not, then just exit.
    enabled = P->port_msti_enabled[msti] && IS_PORT_MGMT_ENABLED2(P) ? TRUE : FALSE;

    // If:  a)  'enabled' is false, and participant P is not in gip list
    //       OR
    //      b)  'enabled' is true, and participant P is in gip list, then do nothing.
    if (enabled == IS_IN_GIP_CONTEXT(P, msti)) {
        return VTSS_RC_OK;
    }

    // --- An actually change has happend

    // (2) --- [IEEE 802.1D-2004, clause 12.2.3, c and d]
    //         Case c: Port is added, i.e., enabled is true, then GID_Join.request
    //                 shall be sent to all others in the active topology if the added
    //                 (port,attribute)s registrar SM is in IN i.e., join was more
    //                 recent then leave.
    //         Case d: See comments below in the else section.
    if (enabled) {

        vtss_garp_add_list(&GVRP_application.gip[msti], &P->gip[msti]);

        // --- Case c
        for (gvrp_restore_vid2(P, msti); (gid = gvrp_next_vid()); ) {
            (void)gvrp_update_enable(P, gid);
        }

    } else {

        // --- Case d
        for (gvrp_restore_vid2(P, msti); (gid = gvrp_next_vid()); ) {
            (void)gvrp_update_disable(P, gid);
        }

        vtss_garp_remove_list(&GVRP_application.gip[msti], &P->gip[msti]);

    }

    return rc;
}

static void update_port_state2(struct garp_participant  *P, vtss_vid_t vid, u8 msti_old, u8 msti_new, u8 msti_transit)
{
    BOOL enabled;
    struct garp_gid_instance *gid;

    // (1) --- Check if port enable state for this msti has actually changed. If not, then just exit.
    enabled = P->port_msti_enabled[msti_new] && IS_PORT_MGMT_ENABLED2(P) ? TRUE : FALSE;

    // If:  a)  'enabled' is false, and participant P is not in gip list
    //       OR
    //      b)  'enabled' is true, and participant P is in gip list, then do nothing.
    if (enabled == IS_IN_GIP_CONTEXT(P, msti_old)) {
        return;
    }


    gvrp_restore_vid(P, 0 /*does not matter*/);

    if ( !(gid = gvrp_this_vid(vid)) ) {
        T_E("gvrp_this_vid(vid=%d)=NULL\n", vid);
        return;
    }


    // --- An actually change has happend

    // (2) --- [IEEE 802.1D-2004, clause 12.2.3, c and d]
    //         Case c: Port is added, i.e., enabled is true, then GID_Join.request
    //                 shall be sent to all others in the active topology if the added
    //                 (port,attribute)s registrar SM is in IN i.e., join was more
    //                 recent then leave.
    //         Case d: See comments below in the else section.
    if (enabled) {

        // --- Case c
        vtss_garp_add_list(&GVRP_application.gip[msti_transit], &P->gip[msti_transit]);
        (void)gvrp_update_enable(P, gid);

    } else {

        // --- Case d
        (void)gvrp_update_disable(P, gid);
        vtss_garp_remove_list(&GVRP_application.gip[msti_transit], &P->gip[msti_transit]);

    }

    if ( (gvrp_this_vid(vid)) ) {
        T_E("gvrp_this_vid(vid=%d)=NULL\n", vid);
    }
}


/*
 *  This function is called when LeaveAll SM emit the sLeaveAll. This event has to go to
 *  all the Applicant- and Registrar SMs in the same participant as the sLeaveAll was sent from
 *
 *  Event though this is a general GARP mechanism, this function is GVRP specific, because of
 *  orginization.
 */
/*lint -sem(gvrp_leaveall_self, thread_protected) */
static void gvrp_leaveall_self(struct garp_participant *p)
{
    int rc;
    u8 msti;
    struct garp_gid_instance *gid;
    struct garp_application *a = &GVRP_application;

    T_N1("---gvrp_leaveall_self");

    // --- Send leave all to all applicant and registrar SMs in participant p.
    for (msti = 0; msti < NO_MSTI; ++msti)
        for (gvrp_restore_vid2(p, msti); (gid = gvrp_next_vid()); ) {
            rc = vtss_garp_gidtt_event_handler2(a, p, gid, garp_event__LeaveAll2);
            if (rc) {
                T_E("gvrp_leaveall_self port=%d, msti=%d vid=%d", GET_PORT(p), msti, GET_VID(gid));
            }
        }
}


//vtss_rc vtss_gvrp_registrar_administrative_control(int port_no, vtss_vid_t vid, enum garp_registrar_admin S)
static vtss_rc vtss_gvrp_registrar_administrative_control2(int port_no, vtss_vid_t vid, vlan_registration_type_t S)
{
    vtss_rc rc = VTSS_RC_ERROR;

    if ( PORT_RANGE_CHECK(port_no) && VID_RANGE_CHECK(vid) ) {
        rc = vtss_garp_registrar_administrative_control(&GVRP_application,
                                                        &GVRP_participant[port_no],
                                                        1, /*type*/
                                                        (void *)&vid,
                                                        S);
        if (rc && warning_limit[2] < 3) {
            ++warning_limit[2];
            T_WG(TRACE_GRP_GVRP, "port=%d, vid=%d, vlan_reg_type=%d, max_vlans=%d", (int)port_no, (int)vid, (int)S,
                 alloc_book_keeper[GVRP_ALLOC_DEFAULT].max);
        }
    }

    return rc;
}


/*
 * IEEE802.1D-2004, clause 12.8.1
 *
 * Normal Registration
 * Registration Fixed
 * Registration Forbidden
 */
vtss_rc vtss_gvrp_registrar_administrative_control(int port_no, vtss_vid_t vid, vlan_registration_type_t S)
{
    vtss_rc rc;

    //    T_DG(TRACE_GRP_GVRP, "port=%d, vid=%d, vlan_reg_type=%d", (int)port_no, (int)vid, (int)S);

    GVRP_CRIT_ENTER();
    rc = vtss_gvrp_registrar_administrative_control2(port_no, vid, S);
    GVRP_CRIT_EXIT();

    return rc;
}


// (13) ------ INIT


/*
 *  Constructor
 *
 *  This function is called when GVRP is globally enabled.
 */
vtss_rc vtss_gvrp_construct(int max_participants, int max_vlans)
{
    int i;


    GVRP_CRIT_ENTER();


    // --- If caller does not know number of port, then get them ourselves.
    if (max_participants < 1) {
        max_participants = L2_MAX_SWITCH_PORTS * VTSS_ISID_CNT;
    }

    // --- If already constructed, then exit.
    if (GVRP_participant_max) {
        GVRP_CRIT_EXIT();
        T_N("GVRP construct.2: Already constructed");
        return VTSS_RC_ERROR;
    }

    bzero(warning_limit, sizeof(warning_limit));

    // --- Setup VID to GID array mapping
    bzero(GVRP_vid2gid, sizeof(GVRP_vid2gid));
    for (i = 0; i < NO_VLAN; ++i) {
        GVRP_vid2gid[i].vid = i;
    }

    doing_tx_pdu = 0;

    // --- Allocate all participant structures.
    GVRP_participant = VTSS_CALLOC(max_participants, sizeof(struct garp_participant));
    if (!GVRP_participant) {
        goto gvrp_participant_init_l2;
    }

    if (max_vlans < 1) {
        max_vlans = 1;
    }

    bzero(alloc_book_keeper, sizeof(alloc_book_keeper));

    alloc_book_keeper[GVRP_ALLOC_DEFAULT].max = max_vlans;
    alloc_book_keeper[GVRP_ALLOC_ICLI].max = 1;
    alloc_book_keeper[GVRP_ALLOC_TX].max = 1;

    max_vlans += 2;

    // --- Allocate and configure gid array book keeping
    GVRP_free_gid_list = VTSS_CALLOC(max_vlans, sizeof(struct gvrp_gid_ctrl));
    if (!GVRP_free_gid_list) {
        goto gvrp_participant_init_l1;
    }

    GVRP_free_available_gid_index = -1;
    GVRP_free_gid_index = 0;
    for (i = 0; i < max_vlans; ++i) {
        GVRP_free_gid_list[i].next = i + 1;
    }

    GVRP_free_gid_list[i - 1].next = -1;

    // --- Configure with application specific metods.
    for (i = 0; i < max_participants; ++i) {

        GVRP_participant[i].gid_index = i;
        GVRP_participant[i].port_no   = i;
        GVRP_participant[i].get_gid_TypeValue2obj = gvrp_get_gid_TypeValue2obj;

        GVRP_participant[i].map_gid_obj2TV = gvrp_map_gid_obj2TV;
        GVRP_participant[i].map_gid_TV2obj = gvrp_map_gid_TV2obj;
        GVRP_participant[i].done_gid = gvrp_done_gid;
    }

    // --- Setup application structure

    // Clear everything, except timer configuration values, which may already
    // have been set.
    bzero(&GVRP_application, offsetof(struct garp_application, GARP_timer[0]));

    GVRP_application.packet_build = vtss_gvrp_tx_pdu_step_2;
    GVRP_application.packet_done  = vtss_gvrp_tx_pdu_step_3;

    GVRP_application.ref_count    = ref_count2;
    GVRP_application.calc_registrations = calc_registrations;
    GVRP_application.leaveall = gvrp_leaveall_self;

    GVRP_application.restore_gip = gvrp_restore_gip;
    GVRP_application.next_gip = gvrp_next_gip;

    GVRP_application.restore_pdulist = gvrp_restore_transmitPDU_list;
    GVRP_application.next_pdulist = gvrp_getnext_transmitPDU_list;
    GVRP_application.add_pdulist = gvrp_add_transmitPDU_list;
    GVRP_application.remove_pdulist = gvrp_remove_transmitPDU_list;

    GVRP_application.GID_Join_indication = GID_Join_indication;
    GVRP_application.GID_Leave_indication = GID_Leave_indication;

    // --- Do GARP init, if not already done. This may have been
    //     done, since it is allowed to set GARP timers before
    //     GVRP is enabled.
    if (!GVRP_application_partial_init) {
        vtss_garp_init(&GVRP_application);
    }

    GVRP_application_partial_init = 2;
    GVRP_participant_max = max_participants;


    // --- Setup msti list to 'empty'. This is a entry point
    //     to a list of VIDs that belong to the same MSTI.
    //     See: gvrp_setup_vlan_msti_map().
    gvrp_setup_vlan_msti_map();


    // --- Update whith the known state of MSTP.
    for (i = 0; i < max_participants; ++i) {

        vtss_common_port_t l2port;
        int msti;
        vtss_stp_state_t st;

        l2port = participant_to_l2port(&GVRP_participant[i]);

        for (msti = 0; msti < NO_MSTI; ++msti) {
            st = l2_get_msti_stpstate(msti, l2port);
            GVRP_participant[i].port_msti_enabled[msti] = (VTSS_STP_STATE_FORWARDING == st ? TRUE : FALSE);
            if (GVRP_participant[i].port_msti_enabled[msti]) {
                T_DG(TRACE_GRP_GVRP, "port=%d, msti=%d MSTI enabled", l2port, msti);
            }
        }

    }



    // --- Success and done.
    GVRP_CRIT_EXIT();


    T_N1("GVRP construct.2: Exit OK");
    return VTSS_RC_OK;


    // --- Error handling
gvrp_participant_init_l1:
    VTSS_FREE(GVRP_participant);

gvrp_participant_init_l2:
    GVRP_CRIT_EXIT();
    T_E("GVRP construct.3: Exit failed");
    return VTSS_RC_ERROR;
}


/*
 *  Destructor
 *
 *  This function is called when GVRP is globally disabled.
 */
void vtss_gvrp_destruct(BOOL set_default)
{
    int i;
    vtss_rc rc;


    GVRP_ASSERT_LOCKED();


    if (GVRP_participant_max) {

        // --- Disable all ports and force a txPDU event,
        //     so that others are informed.
        for (i = 0; i < GVRP_participant_max; ++i) {

            rc = vtss_gvrp_port_control_conf_set2(i, FALSE);
            if (rc) {
                T_E("i=%d, rc=%d", i, rc);
            }

        }

        vtss_garp_force_txPDU(&GVRP_application);

        GVRP_participant_max = 0;
        VTSS_FREE(GVRP_participant);

        for (i = 1; i < NO_VLAN; ++i) {

            if (GVRP_vid2gid[i].gid) {
                VTSS_FREE(GVRP_vid2gid[i].gid);
                GVRP_vid2gid[i].gid = 0;
            }

        }

        for (i = GVRP_free_available_gid_index; i != -1; i = GVRP_free_gid_list[i].next) {

            VTSS_FREE(GVRP_free_gid_list[i].gid_array);

        }

    }

    // --- Do not forget GARP timer setting
    if (GVRP_application_partial_init == 2) {
        GVRP_application_partial_init = 1;
    }

    if (set_default) {
        GVRP_application_partial_init = 0;
        alloc_book_keeper[GVRP_ALLOC_DEFAULT].max = 20;
    }

}

int vtss_gvrp_is_enabled(void)
{
    return GVRP_participant_max ? 1 : 0;
}

/*lint -sem(vtss_gvrp_max_vlans, thread_protected) */
int vtss_gvrp_max_vlans(void)
{
    int rc;

    GVRP_CRIT_ENTER();
    rc = alloc_book_keeper[GVRP_ALLOC_DEFAULT].max;
    GVRP_CRIT_EXIT();

    return rc;
}

/*lint -sem(vtss_gvrp_max_vlans, thread_protected) */
void vtss_gvrp_max_vlans_set(int m)
{

    GVRP_CRIT_ENTER();
    alloc_book_keeper[GVRP_ALLOC_DEFAULT].max = m;
    GVRP_CRIT_EXIT();

}


int gvrp_get_vid(struct garp_gid_instance *gid)
{
    return GET_VID(gid);
}

void gvrp_dump_msti_state(void)
{
    int i;

    for (i = 0; i < GVRP_participant_max; ++i) {

        vtss_common_port_t l2port;
        int msti;
        vtss_stp_state_t st;

        l2port = participant_to_l2port(&GVRP_participant[i]);

        for (msti = 0; msti < NO_MSTI; ++msti) {
            st = l2_get_msti_stpstate(msti, l2port);
            GVRP_participant[i].port_msti_enabled[msti] = (VTSS_STP_STATE_FORWARDING == st ? TRUE : FALSE);
            if (GVRP_participant[i].port_msti_enabled[msti]) {
                T_DG(TRACE_GRP_GVRP, "port=%d, msti=%d MSTI enabled", l2port, msti);
            }

        }

    }

}

void vtss_gvrp_internal_statistic(void)
{
    enum gvrp_alloc_type at;

    GVRP_CRIT_ENTER();

    printf("__Group__count__max__peek__\n");
    for (at = GVRP_ALLOC_DEFAULT; at < GVRP_ALLOC_MAX; ++at) {
        printf("%5d %5d %5d %5d\n", at, alloc_book_keeper[at].count, alloc_book_keeper[at].max, alloc_book_keeper[at].peek);
    }

    GVRP_CRIT_EXIT();

}
