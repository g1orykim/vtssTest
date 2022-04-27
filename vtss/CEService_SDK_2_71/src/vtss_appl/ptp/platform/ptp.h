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

#ifndef _PTP_H_
#define _PTP_H_

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_PTP
#define VTSS_TRACE_GRP_SERVO        (1 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_INTERFACE    (2 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_CLOCK        (3 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_1_PPS        (4 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_EGR_LAT      (5 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_PHY_TS       (6 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_REM_PHY      (7 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_PTP_SER      (8 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_PTP_PIM      (9 + VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_PTP_ICLI     (10+ VTSS_TRACE_GRP_PTP_CNT)
#define VTSS_TRACE_GRP_PHY_1PPS     (11+ VTSS_TRACE_GRP_PTP_CNT)
#define TRACE_GRP_CNT               (12+ VTSS_TRACE_GRP_PTP_CNT)

#define _S VTSS_TRACE_GRP_SERVO
#define _I VTSS_TRACE_GRP_INTERFACE
#define _C VTSS_TRACE_GRP_CLOCK

/****************************************************************************/
// API Error Return Codes (vtss_rc)
/****************************************************************************/
enum {
    PTP_ERROR_INV_PARAM = MODULE_ERROR_START(VTSS_MODULE_ID_PTP),
    PTP_RC_INVALID_PORT_NUMBER,
    PTP_RC_INTERNAL_PORT_NOT_ALLOWED,
    PTP_RC_MISSING_PHY_TIMESTAMP_RESOURCE,
    PTP_RC_MISSING_IP_ADDRESS,
    PTP_RC_UNSUPPORTED_ACL_FRAME_TYPE,
    PTP_RC_UNSUPPORTED_PTP_ENCAPSULATION_TYPE,
    PTP_RC_UNSUPPORTED_1PPS_OPERATION_MODE,
};



#define PTP_CONF_VERSION    10

#define PTP_PHYS_PORTS     (VTSS_PORTS) /* Number of physical ports */

/* physical ports */
#define PTP_CLOCK_PORTS   (PTP_PHYS_PORTS)
#define VTSS_TS_CPU_PORT_NO PTP_CLOCK_PORTS

#define CTLFLAG_PTP_DEFCONFIG           (1 << 3)
#define CTLFLAG_PTP_SET_ACL             (1 << 4)
#define CTLFLAG_PTP_TIMER               (1 << 5)

#define PTP_READY()    (ptp_global.ready)

#define LOCK_TRACE_LEVEL VTSS_TRACE_LVL_NOISE

#define TEMP_LOCK() cyg_scheduler_lock()
#define TEMP_UNLOCK()   cyg_scheduler_unlock()

#define PTP_CORE_LOCK()        critd_enter(&ptp_global.coremutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define PTP_CORE_UNLOCK()      critd_exit (&ptp_global.coremutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)

/* ARP Inspection ACE IDs */
#define PTP_ACE_ID_START        1

/*
 * used to initialize the run time options default
 */
#define DEFAULT_MAX_FOREIGN_RECORDS  5
#define DEFAULT_MAX_OUTSTANDING_RECORDS 25
#define DEFAULT_INGRESS_LATENCY      0       /* in nsec */
#define DEFAULT_EGRESS_LATENCY       0       /* in nsec */
#define DEFAULT_DELAY_ASYMMETRY      0       /* in nsec */
#define DEFAULT_PTP_DOMAIN_NUMBER     0
#define DEFAULT_UTC_OFFSET           0
#define DEFAULT_SYNC_INTERVAL        0         /* sync interval = 2**n sec */
#define DEFAULT_ANNOUNCE_INTERVAL    1         /* announce interval = 2**n sec */
#define DEFAULT_DELAY_REQ_INTERVAL   3         /* logarithmic value */
#define DEFAULT_ANNOUNCE_RECEIPT_TIMEOUT 3     /* timeout in announce interval periods */
#define DEFAULT_DELAY_S              6
#define DEFAULT_AP                   3
#define DEFAULT_AI                   80
#define DEFAULT_AD                   40

#define PTP_RC(expr) { vtss_rc my_ptp_rc = (expr); if (my_ptp_rc < VTSS_RC_OK) { \
        T_W("Error code: %s", error_txt(my_ptp_rc)); }}

#define PTP_RETURN(expr) { vtss_rc my_ptp_rc = (expr); if (my_ptp_rc < VTSS_RC_OK) return my_ptp_rc; }
        
#endif /* _PTP_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
