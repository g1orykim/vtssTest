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

*/

#ifndef _ZL_3034X_H_
#define _ZL_3034X_H_

#include "zl303xx.h"
#include "zl303xx_Os.h"

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_ZL_3034X_API
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT      1 
#define TRACE_GRP_OS_PORT   2 
#define TRACE_GRP_SYNC_INTF 3 
#define TRACE_GRP_ZL_TRACE  4 
#define TRACE_GRP_CNT       5 

#define ZL3034X_CONF_VERSION    1



#define LOCK_TRACE_LEVEL VTSS_TRACE_LVL_NOISE
#define ZL_3034X_DATA_LOCK()        critd_enter(&zl3034x_global.datamutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define ZL_3034X_DATA_UNLOCK()      critd_exit (&zl3034x_global.datamutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)


#define ZL_3034X_RC(expr) { vtss_rc _rc_ = (expr); if (_rc_ < VTSS_RC_OK) { \
        T_I("Error code: %x", _rc_); }}

#define ZL_3034X_CHECK(expr) { zlStatusE _rc_ = (expr); if (_rc_ != ZL303XX_OK) { \
        T_W("ZL Error code: %x", _rc_); }}


/**********************************************************************************
 * ZL Trace Functions                                                             *
 **********************************************************************************/
#ifndef MAX_FMT_LEN
#define MAX_FMT_LEN 256
#endif


extern zl303xx_ParamsS *zl_3034x_zl303xx_params;


#endif /* _ZL_3034X_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
