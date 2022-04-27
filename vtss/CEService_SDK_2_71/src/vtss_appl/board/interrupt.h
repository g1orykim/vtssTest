/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_INTERRUPT_H_
#define _VTSS_INTERRUPT_H_

#include "interrupt_api.h"
#include "vtss_types.h"

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_INTERRUPT

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_IRQ0         1
#define TRACE_GRP_IRQ1         2
#define TRACE_GRP_CRIT         3
#define TRACE_GRP_CNT          4

#include <vtss_trace_api.h>

/****************************************************************************/
/*  Internal Interface                                                                                                                    */
/****************************************************************************/

void interrupt_board_init(void);

void interrupt_device_signal(u32 flags);
void interrupt_device_enable(u32 flags,  BOOL pending);
void interrupt_device_poll(u32 flags,  BOOL interrupt,  BOOL onesec, BOOL *pending);
void interrupt_clock_poll(BOOL  interrupt,   BOOL  *pending);

void interrupt_signal_source(vtss_interrupt_source_t     source_id,
                             u32                         instance_no);

void interrupt_source_enable(vtss_interrupt_source_t     source_id);
void interrupt_clock_source_enable(vtss_interrupt_source_t     source_id);

#endif /* _VTSS_INTERRUPT_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
