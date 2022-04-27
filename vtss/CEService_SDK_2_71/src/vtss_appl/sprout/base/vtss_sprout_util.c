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


#include "vtss_sprout.h"
#include <stdio.h>
#include "misc_api.h"

const vtss_sprout_switch_addr_t switch_addr_undef = VTSS_SPROUT_SWITCH_ADDR_NULL;







int vtss_sprout__switch_addr_cmp(
    const vtss_sprout_switch_addr_t *switch_addr_x_p,
    const vtss_sprout_switch_addr_t *switch_addr_y_p)
{
    uint i = 0;

    for (i = 0; i < 6; i++) {
        if (switch_addr_x_p->addr[i] < switch_addr_y_p->addr[i]) {
            return -1;
        } else if (switch_addr_x_p->addr[i] > switch_addr_y_p->addr[i]) {
            return +1;
        } else {
            
        }
    }
    return 0;
} 



int vtss_sprout__str_append(
    char         *str,
    size_t        size,   
    const char   *fmt,
    ...)
{
    va_list ap;
    int len = strlen(str);
    int cnt;

    VTSS_SPROUT_ASSERT(len < size,
                       ("len=%d, size=%d", len, size));

    va_start(ap, fmt);
    cnt = vsnprintf(&str[len], size - len, fmt, ap);
    va_end(ap);

    return cnt;
} 



char *vtss_sprout_port_mask_to_str(u64 port_mask)
{
    static char    s[4][sizeof("25-48,49-51,53 extra")];
    static int     i = 0;
    const  int     size = sizeof(s) / 2;
    vtss_port_no_t port;
    vtss_port_no_t port_start       = 0;
    vtss_port_no_t port_end         = 0;
    BOOL           port_start_found = 0;
    BOOL           first_range = 1;

    i = (i + 1) % ARRSZ(s);

    s[i][0] = 0;

    for (port = 0; port < 63; port++) {
        if ((port_mask >> port) & 1) {
            
            if (!port_start_found) {
                
                port_start = port;
                port_start_found = 1;
            } else if (port != port_end + 1) {
                

                if (!first_range) {
                    vtss_sprout__str_append(s[i], size, ",");
                }

                if (port_start == port_end) {
                    
                    vtss_sprout__str_append(s[i], size, "%d",
                                            iport2uport(port_start));
                } else {
                    
                    vtss_sprout__str_append(s[i], size, "%d-%d",
                                            iport2uport(port_start),
                                            iport2uport(port_end));
                }
                port_start = port;
                first_range = 0;
            }
            port_end = port;
        }
    }

    if (port_start_found) {
        if (!first_range) {
            vtss_sprout__str_append(s[i], size, ",");
        }

        if (port_start == port_end) {
            
            vtss_sprout__str_append(s[i], size, "%d",
                                    iport2uport(port_start));
        } else {
            
            vtss_sprout__str_append(s[i], size, "%d-%d",
                                    iport2uport(port_start),
                                    iport2uport(port_end));
        }
    }

    return s[i];
} 







