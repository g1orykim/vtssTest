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

#ifndef __PACKET_CLI_H__
#define __PACKET_CLI_H__

/* Function prototype */
void packet_cli_init(void);

// CLI contains a debug command that requires not-so-public functions from packet.c.
// Therefore, they're publicized in this private header file.
#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
vtss_rc packet_rx_throttle_cfg_set(vtss_fdma_throttle_cfg_t *cfg);
char *packet_rx_queue_usage(u32 xtr_qu, char *buf, size_t size);

// Also, these two defines are not so public, but nice to see in CLI.
#define PACKET_THROTTLE_PERIOD_MS 100
#define PACKET_THROTTLE_FREQ_HZ   (1000 / PACKET_THROTTLE_PERIOD_MS)
#endif

#endif /*__PACKET_CLI_H__*/


