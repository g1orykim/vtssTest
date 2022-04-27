/*

 Vitesse Switch API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _ZL_3034X_API_API_H_
#define _ZL_3034X_API_API_H_

#include "main.h"

vtss_rc zl_3034x_api_init(vtss_init_data_t *data);

vtss_rc zl_3034x_register_get(const u32   page,
                              const u32   reg,
                              u32 *const  value);
                              
vtss_rc zl_3034x_register_set(const u32   page,
                              const u32   reg,
                              const u32   value);

vtss_rc zl_3034x_debug_pll_status(void);
vtss_rc zl_3034x_debug_hw_ref_status(u32 ref_id);
vtss_rc zl_3034x_debug_hw_ref_cfg(u32 ref_id);
vtss_rc zl_3034x_debug_dpll_status(u32 pll_id);
vtss_rc zl_3034x_debug_dpll_cfg(u32 pll_id);

void zl_3034x_spi_read(u32 address, u8 *data, u32 size);

void zl_3034x_spi_write(u32 address, u8 *data, u32 size);


#endif // _ZL_3034X_API_API_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
