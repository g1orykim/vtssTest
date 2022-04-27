/*

 Vitesse Switch Application software.

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

#ifndef __ERPS_API_H__
#define __ERPS_API_H__

#include "main.h"
#include "l2proto_api.h"
#include "vtss_erps_api.h"
#include "mep_api.h"

vtss_rc erps_init(vtss_init_data_t *data);

/*******************************************************************************
                          ERPS Management Interfaces 
*******************************************************************************/
/* management req strcture, CLI &  Web uses the same
   for initiating management requests */
typedef struct erps_mgmt_conf {

    /* refer below enum for req type definitions */
    u32  req_type;

    /* erps protection group id */
    u32    group_id;

    union {
        struct {
            /* if ring-type is interconnected sub-ring, then east_port contains sub-ring link */
            /* if the given command is FS or MS, east_port contains the port,
               which needs to be blocked */
            vtss_port_no_t east_port;
            vtss_port_no_t west_port;
            vtss_erps_ring_type_t    ring_type;
            BOOL           interconnected;
            BOOL           virtual_channel;
            u32            major_ring_id;
        } create;

        struct {
            u64 time;
        } timer;

        struct {
            u16 num_vids;
            u16 p_vid;
        } vid;

        struct {
            vtss_port_no_t rpl_port;
//            vtss_port_no_t rpl_neighbour;
        } rpl_block;

        struct {
            u32 east_mep_id;
            u32 west_mep_id;
            u32 raps_eastmep;
            u32 raps_westmep;
        } mep;

        /* interconnected node related information */
        struct {
            u32 major_ring_id;
            u8  enable;
        } inter_connected;

        vtss_erps_base_conf_t get;
    } data;

} vtss_erps_mgmt_conf_t;

/*
 *  These are various interaction from platform part to base part
 *  mostly these will get invoked from either CLI/Web
 */
enum {
    ERPS_CMD_PROTECTION_GROUP_ADD = 1 ,
    ERPS_CMD_PROTECTION_GROUP_DELETE,
    ERPS_CMD_HOLD_OFF_TIMER,
    ERPS_CMD_WTR_TIMER,
    ERPS_CMD_GUARD_TIMER,
    ERPS_CMD_WTB_TIMER,
    ERPS_CMD_ADD_VLANS,
    ERPS_CMD_DEL_VLANS,
    ERPS_CMD_ADD_MEP_ASSOCIATION,
    ERPS_CMD_DEL_MEP_ASSOCIATION,
    ERPS_CMD_SET_RPL_OWNER,
    ERPS_CMD_SET_RPL_BLOCK,
    ERPS_CMD_REPLACE_RPL_BLOCK,
    ERPS_CMD_UNSET_RPL_BLOCK,
    ERPS_CMD_GET_FIRST_PROTECTION_GROUP,
    ERPS_CMD_GET_PROTECTION_GROUP,
    ERPS_CMD_SHOW_PROTECTION_GROUPS,
    ERPS_CMD_GET_NEXT_PROTECTION_GROUP,
    ERPS_CMD_SHOW_STATISTICS,
    ERPS_CMD_CLEAR_STATISTICS,
    ERPS_CMD_SET_RPL_NEIGHBOUR,
    ERPS_CMD_UNSET_RPL_NEIGHBOUR,

    /* default is revertive, thus only providing enable/disable for non-revertive */
    ERPS_CMD_ENABLE_NON_REVERTIVE,
    ERPS_CMD_DISABLE_NON_REVERTIVE,

    /* administrative driven commands, can either be given of major or sub-ring */
    ERPS_CMD_FORCED_SWITCH,
    ERPS_CMD_MANUAL_SWITCH,
    ERPS_CMD_CLEAR,

    /* this command has to be given at major ring node */
    ERPS_CMD_TOPOLOGY_CHANGE_PROPAGATE,
    ERPS_CMD_TOPOLOGY_CHANGE_NO_PROPAGATE,

    /* this command can only be given on sub-ring nodes */
    ERPS_CMD_SUB_RING_WITH_VIRTUAL_CHANNEL,
    ERPS_CMD_SUB_RING_WITHOUT_VIRTUAL_CHANNEL,

    /* this can be given only either major or sub-ring */
    ERPS_CMD_ENABLE_VERSION_1_COMPATIBLE,
    ERPS_CMD_DISABLE_VERSION_1_COMPATIBLE,

    /* this command can only be given on interconnected node */
    ERPS_CMD_ENABLE_INTERCONNECTED_NODE,
    ERPS_CMD_DISABLE_INTERCONNECTED_NODE
};

vtss_rc erps_mgmt_set_protection_group_request (const vtss_erps_mgmt_conf_t *);

/* management function invoked from CLI/Web, in order to edit protection
 * group related parameters 
 */
vtss_rc erps_mgmt_getnext_protection_group_request (vtss_erps_mgmt_conf_t *);

/* function used to get a given protection group configuration
 * from platform part of the code 
 */
vtss_rc erps_mgmt_getexact_protection_group_by_id(vtss_erps_mgmt_conf_t *);


/*******************************************************************************
                               ERPS Module Interface 
*******************************************************************************/
vtss_rc erps_sf_sd_state_set(const u32 instance, const u32 mep_instance,
                             const BOOL sf_state, const BOOL sd_state);

/*
 *  takes the incoming PDU and places in a Msg Q, ERPS Thread pick the
 *  message and process them
 *
 *  uint8_t * pdu   : contains R-APS protocol specific information
 *  uint16_t mep_id : MEP ID on which R-APS pdu is received
 *  uint8_t  len    : length of the R-APS PDU
 */
void erps_handover_raps_pdu (const u8 *pdu, u32 mep_id ,u8 len, u32 erpg);



#endif /* __ERPS_API_H__ */
