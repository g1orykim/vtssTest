/*

 Vitesse API software.

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

#ifndef _VTSS_DAYTONA_H_
#define _VTSS_DAYTONA_H_

#include "vtss_daytona_basics.h"
#include "vtss_daytona_reg_init_block_states.h"

#if defined(VTSS_ARCH_DAYTONA)
typedef struct {
       daytona_block_id_t block;
       u32 offset;
       u32 value;
       u32 mask;
   } static_cfg_t;


vtss_rc vtss_daytona_inst_create(vtss_state_t *vtss_state);

vtss_rc vtss_daytona_blk_oper_mode_get(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no, daytona_channel_t channel, block_name_t blk_name, u8 *blk_mode);

const static_cfg_t ** vtss_daytona_gcb_config_get(daytona_channel_t channel);
const static_cfg_t * vtss_daytona_rab_config_get(u32 mode);

#endif /* VTSS_ARCH_DAYTONA */
#endif /* _VTSS_DAYTONA_H_ */
