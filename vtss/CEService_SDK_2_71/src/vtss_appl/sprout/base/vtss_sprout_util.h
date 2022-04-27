/*

 Vitesse Switch Software.

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


 This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/








#ifndef _VTSS_SPROUT_UTIL_H_
#define _VTSS_SPROUT_UTIL_H_

extern const vtss_sprout_switch_addr_t switch_addr_undef;







int vtss_sprout__switch_addr_cmp(
    const vtss_sprout_switch_addr_t   *switch_addr_x_p,
    const vtss_sprout_switch_addr_t   *switch_addr_y_p);



int vtss_sprout__str_append(
    char         *str,
    size_t        size,   
    const char   *fmt,
    ...);


#endif 






