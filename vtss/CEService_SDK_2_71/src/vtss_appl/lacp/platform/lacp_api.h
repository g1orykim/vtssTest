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

#ifndef _LACP_API_H_
#define _LACP_API_H_

#include "vtss_lacp.h"

typedef struct {
    vtss_lacp_system_config_t system;
    vtss_lacp_port_config_t   ports[VTSS_LACP_MAX_PORTS + 1];
} lacp_conf_t;

enum {
    LACP_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_PORT),  /* Generic error code            */
    LACP_ERROR_STATIC_AGGR_ENABLED,                            /* Static aggregation is enabled */
    LACP_ERROR_DOT1X_ENABLED,                                  /* DOT1X is enabled              */
};

vtss_rc lacp_init(vtss_init_data_t *data);

/* Get the system configuration  */
vtss_rc lacp_mgmt_system_conf_get(vtss_lacp_system_config_t *conf);

/* Set the system configuration  */
vtss_rc lacp_mgmt_system_conf_set(const vtss_lacp_system_config_t *conf);

/* Get the port configuration  */
vtss_rc lacp_mgmt_port_conf_get(vtss_isid_t isid, vtss_port_no_t port_no, vtss_lacp_port_config_t *pconf);

/* Set the port configuration  */
vtss_rc lacp_mgmt_port_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, const vtss_lacp_port_config_t *pconf);

/* Get the system aggregation status  */
vtss_common_bool_t lacp_mgmt_aggr_status_get(unsigned int aid, vtss_lacp_aggregatorstatus_t *stat);

/* Get the port status  */
vtss_rc lacp_mgmt_port_status_get(l2_port_no_t l2port, vtss_lacp_portstatus_t *stat);

/* Clear the statistics  */
void lacp_mgmt_statistics_clear(l2_port_no_t l2port);

/* aggr error text */
char *lacp_error_txt(vtss_rc rc);

#endif /* _LACP_API_H_ */

