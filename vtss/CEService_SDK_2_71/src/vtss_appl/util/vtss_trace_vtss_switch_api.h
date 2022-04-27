/*

Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

/*
  This file contains API for Vitesse turnkey SW's access to the trace module
  and should not be used for other purposes.
*/


#ifndef _VTSS_TRACE_VTSS_SWITCH_API_H_
#define _VTSS_TRACE_VTSS_SWITCH_API_H_

#if !defined(VTSS_SWITCH) || !VTSS_SWITCH
#error This file should only be included as part of Vitesse turnkey SW
#endif

#include <vtss_os.h>
#include <main.h> /* Need typedef for init_cmt_t */

/* Initialize module. Only to be called for managed build. */
/* The init function uses type int for argument cmd, since including
 * main.h will cause compilation problems, due to inclusion of trace_api.h
 * in switch API */
vtss_rc vtss_trace_init(vtss_init_data_t *data);

/* Write trace configuration to flash */
vtss_rc vtss_trace_cfg_wr(void);

/* Read trace configuration from flash */
vtss_rc vtss_trace_cfg_rd(void);

/* Erase trace configuration from flash */
vtss_rc vtss_trace_cfg_erase(void);

#endif /*_VTSS_TRACE_VTSS_SWITCH_API_H_ */


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
