/*

 Vitesse Switch Software.

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

*/

#ifndef _VTSS_RMON_TIMER_H_
#define _VTSS_RMON_TIMER_H_

#include <time.h>
typedef void (RMONTimerCallback)(unsigned int clientreg, void *clientarg);

/* alarm flags */
#define RMON_TIMER_REPEAT 0x01  /* keep repeating every X seconds */

/* the ones you should need */
void rmon_timer_unregister(unsigned int clientreg);
unsigned int rmon_timer_register(unsigned int when, unsigned int flags,
                                 RMONTimerCallback *thecallback,
                                 void *clientarg);

/* the ones you shouldn't */
void run_rmon_timer(void);


#endif /*   _VTSS_RMON_TIMER_H_     */
