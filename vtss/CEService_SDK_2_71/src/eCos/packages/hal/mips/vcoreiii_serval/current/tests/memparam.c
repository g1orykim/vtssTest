/*

 Vitesse Switch Software.

 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####

*/
#include <cyg/kernel/kapi.h>

#include <cyg/infra/diag.h>      // For diagnostic printing.
#include <cyg/infra/testcase.h>

void cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("Start MEMPARAMS test");

    printf("%s = %08x\n", "VTSS_MEMPARM_MEMCFG", VTSS_MEMPARM_MEMCFG);
    printf("%s = %08x\n", "VTSS_MEMPARM_PERIOD", VTSS_MEMPARM_PERIOD);
    printf("%s = %08x\n", "VTSS_MEMPARM_TIMING0", VTSS_MEMPARM_TIMING0);
    printf("%s = %08x\n", "VTSS_MEMPARM_TIMING1", VTSS_MEMPARM_TIMING1);
    printf("%s = %08x\n", "VTSS_MEMPARM_TIMING2", VTSS_MEMPARM_TIMING2);
    printf("%s = %08x\n", "VTSS_MEMPARM_TIMING3", VTSS_MEMPARM_TIMING3);

    CYG_TEST_PASS_FINISH("Done");
}
