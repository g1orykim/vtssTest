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
/**
 * \file
 * \brief OS Layer API
 * \details This header file includes the OS specific header file
 */

#ifndef _VTSS_OS_H_
#define _VTSS_OS_H_

#if defined(VTSS_OPSYS_ECOS)
 #include <vtss_os_ecos.h>
#elif defined(VTSS_OPSYS_LINUX)
 #include <vtss_os_linux.h>
#elif defined(VTSS_OS_CUSTOM)
 #include <vtss_os_custom.h>
#else
 #error "Operating system not supported".
#endif

/*
 * Don't add default VTSS_xxx() macro implementations here,
 * since that might lead to uncaught problems on new platforms.
 */

#endif /* _VTSS_OS_H_ */
