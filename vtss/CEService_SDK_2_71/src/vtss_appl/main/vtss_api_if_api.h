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

#ifndef _VTSS_API_IF_API_H_
#define _VTSS_API_IF_API_H_

#include "port_custom_api.h"
#include "main_types.h" /* For vtss_init_data_t */

/* Chip Compile Target */
unsigned int vtss_api_chipid(void);

extern const port_custom_entry_t *port_custom_table;

/* Initialize module */
vtss_rc vtss_api_if_init(vtss_init_data_t *data);

/* Get number of chips making up this target */
u32 vtss_api_if_chip_count(void);

/* Get board information */
void vtss_board_info_get(vtss_board_info_t *info);

#ifdef VTSS_SW_OPTION_DEBUG
void vtss_api_if_reg_access_cnt_get(u64 read_cnts[2], u64 write_cnts[2]);
#endif

#endif /* _VTSS_API_IF_API_H_ */
