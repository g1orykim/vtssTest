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

#ifndef _VTSS_AGGR_XML_H_
#define _VTSS_AGGR_XML_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_types.h>

#include "aggr_api.h"
#ifdef VTSS_SW_OPTION_LACP
#include "lacp_api.h"
#endif /* VTSS_SW_OPTION_LACP */

/* Aggr specific set state structure */
typedef struct {
    aggr_mgmt_group_no_t aggr_no[VTSS_PORT_ARRAY_SIZE]; /* Aggr. membership */
    cx_line_t            line_aggr;  /* Aggregation line */
#ifdef VTSS_SW_OPTION_LACP
    vtss_lacp_port_config_t lacp[VTSS_PORT_ARRAY_SIZE]; /* LACP conf */
#endif /* VTSS_SW_OPTION_LACP */
    cx_line_t               line_lacp;  /* LACP line */
} aggr_cx_set_state_t;


#endif /* _VTSS_AGGR_XML_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
