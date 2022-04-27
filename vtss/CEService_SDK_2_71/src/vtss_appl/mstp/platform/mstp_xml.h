/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_MSTP_XML_H_
#define _VTSS_MSTP_XML_H_

#include "mstp_api.h"

/* STP specific set state structure */
typedef struct {
    cx_line_t    line_stp;  /* STP line */

    mstp_bridge_param_t bridge_conf;
    mstp_msti_config_t  msti_conf;
    BOOL port_list[VTSS_PORT_ARRAY_SIZE+1];
    struct {
        BOOL                   enable_stp;
        mstp_port_param_t      conf;
        mstp_msti_port_param_t mstiport[N_MSTI_MAX];
    } stp[VTSS_PORT_ARRAY_SIZE]; /* STP conf */
} mstp_cx_set_state_t;


#endif /* _VTSS_MSTP_XML_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
