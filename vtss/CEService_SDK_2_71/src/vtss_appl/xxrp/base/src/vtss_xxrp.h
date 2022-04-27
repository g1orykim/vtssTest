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
#ifndef _VTSS_XXRP_XXRP_H_
#define _VTSS_XXRP_XXRP_H_

#include "vtss_xxrp_api.h"
#include "vtss_xxrp_mvrp.h"
#include "vtss_xxrp_mad.h"
#include "vtss_xxrp_madtt.h"

#define VTSS_XXRP_MAX_PDU_SIZE                1500                          /* Maximum MVRP PDU size          */
#define VTSS_XXRP_MIN_PDU_SIZE                21                            /* Minimum MVRP PDU size          */
#define VTSS_XXRP_MIN_MSG_SIZE                6                             /* Minimum MVRP msg size          */
#define VTSS_XXRP_VEC_ATTR_HDR_SIZE           4                             /* Vector header size             */
#define VTSS_MRP_ENDMARK                      0                             /* MRP End Mark                   */ 

#define VTSS_XXRP_APPL_EVENT_NEW              0                             /* New event                      */
#define VTSS_XXRP_APPL_EVENT_JOININ           1                             /* JoinIn event                   */
#define VTSS_XXRP_APPL_EVENT_IN               2                             /* In event                       */
#define VTSS_XXRP_APPL_EVENT_JOINMT           3                             /* JoinMt event                   */
#define VTSS_XXRP_APPL_EVENT_MT               4                             /* Mt event                       */
#define VTSS_XXRP_APPL_EVENT_LV               5                             /* Leave event                    */
#define VTSS_XXRP_APPL_EVENT_INVALID          6                             /* Invalid event                  */
#define VTSS_XXRP_APPL_EVENT_LA               0xFF                          /* LeaveAll event                 */
#define VTSS_XXRP_APPL_EVENT_TX_PKTS          7                             /* Transmitted frames             */
#define VTSS_XXRP_APPL_EVENT_INVALID_BYTE     ((VTSS_XXRP_APPL_EVENT_INVALID << 4) | VTSS_XXRP_APPL_EVENT_INVALID)

#define VTSS_XXRP_SET_EVENT(arr, indx, val)   ((indx % 2) ? (arr[indx/2] = ((arr[indx/2] & 0x0F) | (val << 4))) \
                                                          : (arr[indx/2] = ((arr[indx/2] & 0xF0) | val)))
#define VTSS_XXRP_GET_EVENT(arr, indx)        ((indx % 2) ? (((arr[indx/2]) >> 4) & 0xF) : ((arr[indx/2] & 0xF)))

typedef struct {
    u8    dst_mac[VTSS_XXRP_MAC_ADDR_LEN];
    u8    src_mac[VTSS_XXRP_MAC_ADDR_LEN];
    u8    eth_type[VTSS_XXRP_ETH_TYPE_LEN];
    u8    dsap;
    u8    lsap;
    u8    control;
} XXRP_ATTRIBUTE_PACKED xxrp_eth_hdr_t;

u32 vtss_mrp_port_mad_get(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_mad_t **mad);
u32 vtss_mrp_process_leaveall(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no);
u32 vtss_mrp_process_new_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index);
u32 vtss_mrp_process_joinin_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index);
u32 vtss_mrp_process_in_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index);
u32 vtss_mrp_process_joinmt_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index);
u32 vtss_mrp_process_mt_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index);
u32 vtss_mrp_process_lv_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index);
u32 vtss_mrp_join_indication(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_attribute_type_t *attr_type, u32 mad_indx, BOOL new);
u32 vtss_mrp_leave_indication(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_attribute_type_t *attr_type, u32 mad_indx);
u32 vtss_mrp_mad_process_events(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no,
				u16 mad_fsm_indx, vtss_mad_fsm_events *fsm_events);
u32 vtss_mrp_handle_periodic_timer(vtss_mrp_appl_t application, u32 port_no);
u32 vtss_xxrp_vlan_change_handler(vtss_mrp_appl_t application, u32 fsm_index, u32 port_no, BOOL is_add);
u32 vtss_mrp_map_port_change_handler(vtss_mrp_appl_t application, u32 port_no, BOOL is_add);
u32 vtss_mrp_port_update_peer_mac_addr(vtss_mrp_appl_t application, u32 port_no, u8 *mac_addr);
u32 vtss_mrp_rx_statistics_set(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_stat_type_t stat_type);
u32 vtss_mrp_tx_statistics_set(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_stat_type_t stat_type);
void vtss_xxrp_update_tx_stats(vtss_mrp_appl_t application, u32 port_no, u8 event);
BOOL vtss_mrp_is_attr_registered(vtss_mrp_appl_t application, u32 port_no, u32 attr);

u32 vtss_mrp_port_control_conf_get(vtss_mrp_appl_t application, u32 port_no, BOOL *const status);
#endif /* _VTSS_XXRP_XXRP_H_ */
