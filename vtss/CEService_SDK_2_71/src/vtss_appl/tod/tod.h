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

*/

#ifndef _TOD_H_
#define _TOD_H_

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_TOD
//#define VTSS_TRACE_GRP_DEFAULT 0
#define VTSS_TRACE_GRP_CLOCK   1
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#define VTSS_TRACE_GRP_MOD_MAN 2
#define VTSS_TRACE_GRP_PHY_TS  3
#define VTSS_TRACE_GRP_REM_PHY 4
#define VTSS_TRACE_GRP_PHY_ENG 5
#define TRACE_GRP_CNT          6
#else
#define TRACE_GRP_CNT          2
#endif
#define _C VTSS_TRACE_GRP_CLOCK

#define TOD_CONF_VERSION    1



#define LOCK_TRACE_LEVEL VTSS_TRACE_LVL_NOISE
#define TOD_DATA_LOCK()        critd_enter(&tod_global.datamutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define TOD_DATA_UNLOCK()      critd_exit (&tod_global.datamutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)


#define TOD_RC(expr) { vtss_rc _rc_ = (expr); if (_rc_ < VTSS_RC_OK) { \
        T_I("Error code: %x", _rc_); }}


#endif /* _TOD_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
