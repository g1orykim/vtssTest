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

#ifndef _PTP_1PPS_CLOSED_LOOP_
#define _PTP_1PPS_CLOSED_LOOP_
/******************************************************************************/
//
// This header file contains various definitions and functions for
// implementing the 1pps synchronization feature supported bu the Vitesse Gen2
// PHY's.
//
//
/******************************************************************************/



/*
 * 1PPS synchronization mode.
 */
typedef enum {
    VTSS_PTP_1PPS_CLOSED_LOOP_MAN,      /* Manually enter the cable delay */
    VTSS_PTP_1PPS_CLOSED_LOOP_AUTO,     /* Automatically measure the closed loop delay  */
    VTSS_PTP_1PPS_CLOSED_LOOP_DISABLE,  /* Set the default cable delay */
} vtss_1pps_closed_loop_mode_t;

/*
 * 1PPS closed loop configuration.
 */
typedef struct {
    vtss_1pps_closed_loop_mode_t    mode;    /* 1pps delay mode */
    u32                      master_port;    /* Master port used for measuring the pulse delay */
    u32                      cable_delay;    /* manually entered cable delay used in man mode */
} vtss_1pps_closed_loop_conf_t;


/******************************************************************************/
// Interface functions.
/******************************************************************************/

/**
 * \brief Function for configuring the 1PPS node synchronization using the Gen2 PHY's
 *
 * \param port_no       [IN]  Interface port number to be configured.
 * \param conf          [IN]  The configuration parameters.
 * \return Errorcode.   PTP_RC_MISSING_PHY_TIMESTAMP_RESOURCE (itf the port does not support Gen2 features
 *                      Errorcode from the phy_ts_api
 **/
vtss_rc ptp_1pps_closed_loop_mode_set(vtss_port_no_t port_no, const vtss_1pps_closed_loop_conf_t *conf);

/**
 * \brief Function to get the configuration the 1PPS node synchronization using the Gen2 PHY's
 *
 * \param port_no       [IN]  Interface port number to be configured.
 * \param conf          [OUT]  The configuration parameters.
 * \return Errorcode.   PTP_RC_MISSING_PHY_TIMESTAMP_RESOURCE (itf the port does not support Gen2 features
 *                      Errorcode from the phy_ts_api
 **/
vtss_rc ptp_1pps_closed_loop_mode_get(vtss_port_no_t port_no, vtss_1pps_closed_loop_conf_t *conf);

/**
 * \brief Function for initializing the 1PPS node synchronization using the Gen2 PHY's
 *
 * \return none
 **/
vtss_rc ptp_1pps_closed_loop_init(void);


#endif /* _PTP_1PPS_CLOSED_LOOP_ */

/******************************************************************************/
//  End of file.
/******************************************************************************/
