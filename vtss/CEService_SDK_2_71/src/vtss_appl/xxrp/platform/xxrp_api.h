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

#ifndef _XXRP_API_H_
#define _XXRP_API_H_

#include "main.h"
#include "vtss_xxrp_api.h"
#include <vlan_api.h>


/***************************************************************************************************
 * Definition of rc errors - See also xxrp_error_txt in xxrp.c
 **************************************************************************************************/
enum {
    XXRP_ERROR_ISID = MODULE_ERROR_START(VTSS_MODULE_ID_XXRP),
    XXRP_ERROR_PORT,
    XXRP_ERROR_FLASH,
    XXRP_ERROR_SLAVE,
    XXRP_ERROR_NOT_MASTER,
    XXRP_ERROR_VALUE,
};
char *xxrp_error_txt(vtss_rc rc);

/***************************************************************************************************
 * Configuration definition
 **************************************************************************************************/

/***************************************************************************************************
 * Functions
 **************************************************************************************************/
vtss_rc xxrp_mgmt_global_enabled_get(vtss_mrp_appl_t appl, BOOL *enable);
vtss_rc xxrp_mgmt_global_enabled_set(vtss_mrp_appl_t appl, BOOL enable);
vtss_rc xxrp_mgmt_enabled_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL *enable);
vtss_rc xxrp_mgmt_enabled_set(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL enable);
vtss_rc xxrp_mgmt_periodic_tx_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL *enable);
vtss_rc xxrp_mgmt_periodic_tx_set(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, BOOL enable);
vtss_rc xxrp_mgmt_timers_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, vtss_mrp_timer_conf_t *timers);
vtss_rc xxrp_mgmt_timers_set(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, const vtss_mrp_timer_conf_t *timers);
vtss_rc xxrp_mgmt_applicant_adm_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, vtss_mrp_attribute_type_t attr_type,
                                    BOOL *participant);
vtss_rc xxrp_mgmt_applicant_adm_set(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, vtss_mrp_attribute_type_t attr_type,
                                    BOOL participant);
vtss_rc xxrp_mgmt_port_stats_get(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl, vtss_mrp_statistics_t *stats);
vtss_rc xxrp_mgmt_port_stats_clear(vtss_isid_t isid, vtss_port_no_t iport, vtss_mrp_appl_t appl);
vtss_rc xxrp_init(vtss_init_data_t *data);
vtss_rc xxrp_mgmt_print_connected_ring(u8 msti);
u64 xxrp_mgmt_memory_mgmt_get_alloc_count(void);
u64 xxrp_mgmt_memory_mgmt_get_free_count(void);
vtss_rc xxrp_mgmt_pkt_dump_set(BOOL pkt_control);
vtss_rc xxrp_mgmt_mad_port_print(vtss_isid_t isid, vtss_port_no_t iport, u32 index);
vtss_rc xxrp_mgmt_vlan_state(vtss_common_port_t l2port, vlan_registration_type_t *array /* VLAN_ID_MAX + 1 entries */);

#endif /* _XXRP_API_H_ */
