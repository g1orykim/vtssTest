/* Switch API software.

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

#ifndef VTSS_ICLI_EPS_H
#define VTSS_ICLI_EPS_H

#include "icli_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

void eps_show_eps(i32 session_id, icli_range_t *inst,
                  BOOL has_detail);

void eps_clear_eps(i32 session_id, u32 inst);

void eps_eps(i32 session_id, u32 inst,
             BOOL has_port, BOOL has_evc, BOOL has_1p1, BOOL has_1f1, u32 flow_w, icli_switch_port_range_t port_w, u32 flow_p, icli_switch_port_range_t port_p);

void eps_no_eps(i32 session_id, u32 inst);

void eps_eps_mep(i32 session_id, u32 inst,
                 u32 mep_w, u32 mep_p, u32 mep_aps);

void eps_eps_revertive(i32 session_id, u32 inst,
                       BOOL has_10s, BOOL has_30s, BOOL has_5m, BOOL has_6m, BOOL has_7m, BOOL has_8m, BOOL has_9m, BOOL has_10m, BOOL has_11m, BOOL has_12m);

void eps_no_eps_revertive(i32 session_id, u32 inst);

void eps_eps_holdoff(i32 session_id, u32 inst,
                     u32 hold);

void eps_no_eps_holdoff(i32 session_id, u32 inst);

void eps_eps_1p1(i32 session_id, u32 inst,
                 BOOL has_bidirectional, BOOL has_unidirectional, BOOL has_aps);

void eps_eps_command(i32 session_id, u32 inst,
                     BOOL has_lockout, BOOL has_forced, BOOL has_manualp, BOOL has_manualw, BOOL has_exercise, BOOL has_freeze, BOOL has_lockoutlocal);

void eps_no_eps_command(i32 session_id, u32 inst);

vtss_rc eps_icfg_init(void);

#endif /* VTSS_ICLI_eps_H */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
