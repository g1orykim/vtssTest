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

#ifndef _VTSS_MSG_TEST_API_H_
#define _VTSS_MSG_TEST_API_H_

#include "main.h"

/****************************************************************************/
//
//                   MESSAGE MODULE TEST API.
//
/****************************************************************************/

/****************************************************************************/
// msg_test_suite()
// If @num == 0 or out of bounds, it prints the defined tests using
// @print_function..
// If @num > 0 and within bounds, it executes the test with the argument
// given by @arg.
// This should be called from CLI, only, and is (currently) independent of
// the current stack state, i.e. the same messages are sent whether or not
// the switch is master.
// Only one test can be in progress at a time.
/****************************************************************************/
void msg_test_suite(u32 num, u32 arg0, u32 arg1, u32 arg2, int (*print_function)(const char *fmt, ...));

/****************************************************************************/
// msg_test_init()
// Message Test Module initialization function.
/****************************************************************************/
vtss_rc msg_test_init(vtss_init_data_t *data);

#endif // _VTSS_MSG_TEST_API_H_

