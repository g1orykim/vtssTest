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
#ifndef _VTSS_XXRP_MVRP_H_
#define _VTSS_XXRP_MVRP_H_

#include "vtss_xxrp.h"
#include "vtss_xxrp_types.h"
#include "vtss_xxrp_api.h"
#include "vtss_xxrp_callout.h"

#include <vlan_api.h>

#define VTSS_MVRP_MAX_VECTORS                 ((VLAN_ENTRY_CNT+2) / 3)     /* Each vector can encode 3 VLANs */
#define VTSS_MVRP_VECTOR_MAX_EVENTS           3                             /* Three-packed events            */
#define VTSS_MVRP_VERSION                     0                             /* MVRP Version                   */
//#define VTSS_MVRP_ETH_TYPE                    0x88F5
#define VTSS_MVRP_ATTR_TYPE_VLAN              1
#define VTSS_MVRP_ATTR_LEN_VLAN               2
#define VTSS_MVRP_LA_AND_NUM_OF_VALS_FLD_SIZE 2
#define VTSS_MVRP_VECTOR_HDR_SIZE             4
#define VTSS_MVRP_LA_BIT_OFFSET               5
#define VTSS_MVRP_LA_BYTE_OFFSET              18
//#define VTSS_MVRP_NUM_OF_VALS_BYTE0_OFFSET    18
#define VTSS_MVRP_NUM_OF_VALS_BYTE1_OFFSET    19
#define VTSS_MVRP_FIRST_VAL_BYTE0_OFFSET      20
#define VTSS_MVRP_FIRST_VAL_BYTE1_OFFSET      21
//#define VTSS_MVRP_IS_MSG_ENDMARK(msg)         (((msg[1] | msg[0] << 8) == 0x0) ? TRUE : FALSE)
#define VTSS_MVRP_EXTRACT_LA_EVENT(bytes)     ((bytes[0] >> VTSS_MVRP_LA_BIT_OFFSET) & 1) /* 13 bits - NumOfVals; 
                                                                                         1 bit - LA in u16 */
#define VTSS_MVRP_NUMOFVALS_HIGHER_BYTE_MASK  0x1F
#define VTSS_MVRP_PDU_SIZE_OF_FIXED_FLDS      17  /* dest_mac+src_mac+eth_type+version+attr_type+attr_len */  

/* MVRP vector attribute */
typedef struct {
    u8 la_and_num_of_vals[VTSS_MVRP_LA_AND_NUM_OF_VALS_FLD_SIZE];
    u8 first_value[VTSS_MVRP_ATTR_LEN_VLAN];
    u8 vectors[VTSS_MVRP_MAX_VECTORS];
} XXRP_ATTRIBUTE_PACKED mvrp_pdu_vector_attr_t;

/* MVRP message */
typedef struct {
    u8 attr_type;
    u8 attr_len;
} XXRP_ATTRIBUTE_PACKED mvrp_pdu_msg_t;

/* Fixed fields in MVRP PDU */
typedef struct {
    /* Ethernet Header */
    u8 dst_mac[VTSS_XXRP_MAC_ADDR_LEN];
    u8 src_mac[VTSS_XXRP_MAC_ADDR_LEN];
    u8 eth_type[VTSS_XXRP_ETH_TYPE_LEN];

    /* MVRP PDU start */
    u8 version;
} XXRP_ATTRIBUTE_PACKED mvrp_pdu_fixed_flds_t;

u32 vtss_mvrp_global_control_conf_create(vtss_mrp_t **mrp_app);
BOOL vtss_mvrp_is_vlan_registered(u32 port_no, u16 vid);
BOOL vtss_mvrp_pdu_rx(u32 port_no, const u8 *pdu, u32 length);
void mvrp_mad_fsm_index_to_vid(u16 mad_fsm_index, u16 *vid);
void vtss_mrp_get_msti_from_mad_index(u16 mad_fsm_index, u8 *msti);
#endif /* _VTSS_XXRP_MVRP_H_ */

