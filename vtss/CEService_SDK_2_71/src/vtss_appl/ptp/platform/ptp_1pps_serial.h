/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _PTP_1PPS_SERIAL_H_
#define _PTP_1PPS_SERIAL_H_

/* Define interfaces for the serial 1pps interface */
#include "vtss_types.h"

#if defined (VTSS_ARCH_SERVAL)
/**
 * \brief Send a 1pps message on the serial port.
 * \param t [IN]  timestamp indicating the time of nest 1pps pulse.
 * \return nothing
 */
void ptp_1pps_msg_send(const vtss_timestamp_t *t);

/**
 * \brief initialize the 1pps message serial port.
 * \return nothing
 */
vtss_rc ptp_1pps_serial_init(void);

#endif /* (VTSS_ARCH_SERVAL) */
#endif /* _PTP_1PPS_SERIAL_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
