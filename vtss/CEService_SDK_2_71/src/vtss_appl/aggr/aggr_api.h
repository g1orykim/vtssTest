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

 $Id$
 $Revision$

*/

#ifndef _VTSS_AGGR_API_H_
#define _VTSS_AGGR_API_H_

/* In this module LAGS start from 1 and GLAGS start where the LAGS ends.
   (In the vtss api the LAGS and GLAGS start from 0)
   To browse all aggregations use AGGR_MGMT_GROUP_NO_START to AGGR_MGMT_GROUP_NO_END.
*/

/* Determine the number of LLAGs, GLAGs and LAGs */
#define AGGR_UPLINK_NONE 0xFFFF
#if VTSS_SWITCH_STACKABLE
#if defined(VTSS_FEATURE_VSTAX_V1)
#define AGGR_LLAG_CNT VTSS_AGGRS       /* Stackable, VSTAX_V1: LLAGs available */
#define AGGR_UPLINK   VTSS_AGGRS       /* Stackable, VSTAX_V1: Reserved LLAG */
#else
#define AGGR_LLAG_CNT 0                /* Stackable, VSTAX_V2: No LLAGs available */
#define AGGR_UPLINK   AGGR_UPLINK_NONE /* Stackable, VSTAX_V2: No reserved LLAG */
#endif /* VTSS_FEATURE_VSTAX_V2 */
#define AGGR_GLAG_CNT VTSS_GLAGS       /* Stackable: GLAGs available */
#else
#define AGGR_LLAG_CNT VTSS_AGGRS       /* Standalone: LLAGs available */
#define AGGR_UPLINK   AGGR_UPLINK_NONE /* Standalone: No reserved LLAG */
#define AGGR_GLAG_CNT 0                /* Standalone: No GLAGs available */
#endif /* VTSS_WITCH_STACKABLE */
#define AGGR_LAG_CNT (AGGR_LLAG_CNT + AGGR_GLAG_CNT)

/* Aggregation numbers starting from 1 */
#define AGGR_MGMT_GROUP_NO_START   1
#define AGGR_MGMT_GLAG_START       (AGGR_MGMT_GROUP_NO_START + AGGR_LLAG_CNT)
#define AGGR_MGMT_GLAG_END         (AGGR_MGMT_GLAG_START + AGGR_GLAG_CNT)
#define AGGR_MGMT_GROUP_NO_END     (AGGR_MGMT_GLAG_END)

/* Determine whether x is LAG/GLAG/AGGR */
#define AGGR_MGMT_GROUP_IS_LAG(x)  ((x) >= AGGR_MGMT_GROUP_NO_START && (x) < AGGR_MGMT_GLAG_START && (x) != AGGR_UPLINK)
#define AGGR_MGMT_GROUP_IS_GLAG(x) ((x) >= AGGR_MGMT_GLAG_START && (x) < AGGR_MGMT_GLAG_END)
#define AGGR_MGMT_GROUP_IS_AGGR(x) (AGGR_MGMT_GROUP_IS_LAG(x) || AGGR_MGMT_GROUP_IS_GLAG(x))

/* Use for management where LAGs and GLAG starts from 1 */
#define AGGR_MGMT_NO_TO_ID(x) ((x<AGGR_MGMT_GROUP_NO_START||x>AGGR_MGMT_GROUP_NO_END)?0:(x>=AGGR_MGMT_GLAG_START?(x-AGGR_MGMT_GLAG_START+1):x))
#define AGGR_MGMT_GLAG_PORTS_MAX   8 /* JR always supports max 8 ports in a GLAG */
/* Number of ports in LLAG/GLAG */
#if defined(VTSS_FEATURE_AGGR_GLAG) && defined(VTSS_FEATURE_VSTAX_V2)
#define AGGR_MGMT_LAG_PORTS_MAX    AGGR_MGMT_GLAG_PORTS_MAX
#else
#define AGGR_MGMT_LAG_PORTS_MAX    (/*lint -e(506)*/(VTSS_PORTS < 16) ? VTSS_PORTS : 16)
#endif

/* Aggr API error codes (vtss_rc) */
enum {
    AGGR_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_AGGR),  /* Generic error code */
    AGGR_ERROR_PARM,           /* Illegal parameter             */
    AGGR_ERROR_REG_TABLE_FULL, /* Registration table full       */
    AGGR_ERROR_REQ_TIMEOUT,    /* Timeout on message request    */
    AGGR_ERROR_STACK_STATE,    /* Illegal MASTER/SLAVE state    */
    AGGR_ERROR_NOT_MASTER,     /* Not Master                    */
    AGGR_ERROR_GROUP_IN_USE,   /* Group already in use          */
    AGGR_ERROR_PORT_IN_GROUP,  /* Port is already a member      */
    AGGR_ERROR_LACP_ENABLED,   /* Static aggregation is enabled */
    AGGR_ERROR_DOT1X_ENABLED,  /* DOT1X is enabled              */
    AGGR_ERROR_ENTRY_NOT_FOUND,/* Entry not found               */
    AGGR_ERROR_HDX_SPEED_ERROR,/* Illegal duplex or speed state */
    AGGR_ERROR_MEMBER_OVERFLOW,/* To many port members          */
    AGGR_ERROR_INVALID_ID,     /* Invalid group id              */
    AGGR_ERROR_INVALID_PORT,   /* Invalid port                  */
    AGGR_ERROR_INVALID_ISID,   /* Invalid ISID                  */
};

/* Initialize module */
vtss_rc aggr_init(vtss_init_data_t *data);

/* Type which holds ports and aggregation port groups id's */
typedef vtss_aggr_no_t aggr_mgmt_group_no_t;

/* Mgmt API structs */
typedef struct {
    BOOL member[VTSS_PORT_ARRAY_SIZE];
} aggr_mgmt_group_t;

typedef struct {
    aggr_mgmt_group_t    entry;
    aggr_mgmt_group_no_t aggr_no;
} aggr_mgmt_group_member_t;

/* aggr_mgmt_members_get:
 * Get members of a aggr group (both static and dynamic created).
 * 'aggr_no' : Aggregation group id. 'aggr_no=0' and 'next=1' gets the group and members of the first active group.
 *           :'aggr_no='x' and 'next=1' gets the group and members of the group after 'x'
 * 'members' : Points to the updated group and memberlist.
 *  'next'   : Next = 0 returns the memberlist of the group.   next = 1 see above.
 *
 *  Note: Ports which are members of a statically created group - but without a portlink
 *  will not be included in the returned portlist. */
vtss_rc aggr_mgmt_members_get(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_member_t *members, BOOL next);

/*  Same functionality as for <aggr_mgmt_members_get> - only for STATIC created groups
 *  Note: Ports which are members of aggregation group will always be included in the returned portlist,
 *  even though the portlink is down.*/
vtss_rc aggr_mgmt_port_members_get(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_member_t *members, BOOL next);

/* Adds member(s) or modifies a group. Per switch */
/* Members must be in the same speed and in full duplex */
vtss_rc aggr_mgmt_port_members_add(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no, aggr_mgmt_group_t *members);

/* Deletes all members from a group. Per switch  */
vtss_rc aggr_mgmt_group_del(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no);

/* Set the aggregation mode for all groups. See the 'vtss_aggr_mode_t' description in vtss.h */
/* All switches in the stack will be configured with the same aggr mode */
vtss_rc aggr_mgmt_aggr_mode_set(vtss_aggr_mode_t *mode);

/* Returns the aggregation mode (same for all switches in a stack)  */
vtss_rc aggr_mgmt_aggr_mode_get(vtss_aggr_mode_t *mode);

/* Register for aggregation changes */
typedef void (*aggr_change_callback_t)(vtss_isid_t isid, uint aggr_no);
void aggr_change_register(aggr_change_callback_t cb);

/* Returns the LLAG/GLAG id for the given port. Returns 0 if the port is down or not aggregated */
aggr_mgmt_group_no_t aggr_mgmt_get_aggr_id(vtss_isid_t isid, vtss_port_no_t port_no);

/* Returns the LLAG/GLAG id for the given port. Returns 0 if the port is not aggregated */
aggr_mgmt_group_no_t aggr_mgmt_get_port_aggr_id(vtss_isid_t isid, vtss_port_no_t port_no);

/* Returns information if the port is participating in LACP or Static aggregation, */
/* regardless of link status.                                                      */
/* 0 = No participation                                                            */
/* 1 = Static aggregation participation                                            */
/* 2 = LACP aggregation participation                                              */
int aggr_mgmt_port_participation(vtss_isid_t isid, vtss_port_no_t port_no);

/* Returns the speed of the group  */
vtss_port_speed_t aggr_mgmt_speed_get(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no);

void aggr_mgmt_test(void);

/* aggr error text */
char *aggr_error_txt(vtss_rc rc);

/* Debug dump */
typedef int (*aggr_dbg_printf_t)(const char *fmt, ...) __attribute__ ((format (__printf__, 1, 2)));
void aggr_mgmt_dump(aggr_dbg_printf_t dbg_printf);

#ifdef VTSS_SW_OPTION_LACP
/****************************/
/* LACP specific functions: */
/****************************/

/* LACP function. Adds member(s) or modifies a group. */
vtss_rc aggr_mgmt_lacp_members_add(uint aid, int new_port);

/* LACP function. Deletes a member. */
void aggr_mgmt_lacp_member_del(uint aid, int old_port);

/* Same functionality as for <aggr_mgmt_port_members_get> - only for LACP created groups */
vtss_rc aggr_mgmt_lacp_members_get(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_no,  aggr_mgmt_group_member_t *members, BOOL next);

/* Convert the lacp 'core' aggregation id into Aggr API id */
aggr_mgmt_group_no_t lacp_to_aggr_id(int aid);

/* Loop through LACP id's in use */
vtss_rc aggr_mgmt_lacp_id_get_next(int *search_aid, int *return_aid);

#endif /* VTSS_SW_OPTION_LACP */

#endif /* _VTSS_AGGR_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
