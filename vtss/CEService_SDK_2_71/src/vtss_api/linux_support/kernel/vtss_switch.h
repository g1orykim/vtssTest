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

#ifndef _VTSS_SWITCH_H_
#define _VTSS_SWITCH_H_

#if defined(CONFIG_VTSS_VCOREIII)
#if defined(CONFIG_VTSS_VCOREIII_SERVAL1)
#include <asm/mach-serval/hardware.h>
#else
#include <asm/mach-vcoreiii/hardware.h>
#endif  /* CONFIG_VTSS_VCOREIII_SERVAL1 */
#endif  /* CONFIG_VTSS_VCOREIII */

#include "vtss_api.h"
#include "vtss_switch-ioctl.h"
#include "vtss_switch-port.h"

#define FDMA_DBG_MSG(...) /* printk(__VA_ARGS__) */
#define CHECK(x)                                                        \
    do {                                                                \
        if((x) != 0) printk("*** Failure:%s:%d: " #x "\n", __FILE__, __LINE__); \
    } while(0) 
#define VTSS_ASSERT(p) do {	\
	if (!(p)) {	\
		printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",	\
		       __FILE__, __LINE__, #p);			\
		BUG();	\
	}		\
} while (0)

#define ETH_VLAN  1             /* Default VLAN */

#define DMACH_RX        0       /* DMA Channel usage allocation */
#if defined(CONFIG_VTSS_VCOREIII_SERVAL1)
#define DMACH_TX        2
#else
#define DMACH_CCM_START 1
#define DMACH_CCM_END   3
#define DMACH_TX        4
#define DMACH_RX_SLV    7       /* DMA XTR on slave */
#endif

#define VTSS_CCM_SESSIONS    10 /* Max # CCM sessions supported */

/* The highest queue number has the highest priority */

#if VTSS_PACKET_RX_QUEUE_CNT < 6
// Not enough Rx queues on this architecture (e.g. Luton28)
// Adjust it to using only 4, even though we need 6.
  #define PACKET_QU_ARCH_ADJUSTMENT_1 1
  #define PACKET_QU_ARCH_ADJUSTMENT_2 2
#else
  #define PACKET_QU_ARCH_ADJUSTMENT_1 0
  #define PACKET_QU_ARCH_ADJUSTMENT_2 0
#endif

#define PACKET_XTR_QU_LOWEST (VTSS_PACKET_RX_QUEUE_START + 0)
#define PACKET_XTR_QU_LOWER  (VTSS_PACKET_RX_QUEUE_START + 1 - PACKET_QU_ARCH_ADJUSTMENT_1)
#define PACKET_XTR_QU_LOW    (VTSS_PACKET_RX_QUEUE_START + 2 - PACKET_QU_ARCH_ADJUSTMENT_2)
#define PACKET_XTR_QU_NORMAL (VTSS_PACKET_RX_QUEUE_START + 3 - PACKET_QU_ARCH_ADJUSTMENT_2)
#define PACKET_XTR_QU_MEDIUM (VTSS_PACKET_RX_QUEUE_START + 4 - PACKET_QU_ARCH_ADJUSTMENT_2)
#define PACKET_XTR_QU_HIGH   (VTSS_PACKET_RX_QUEUE_START + 5 - PACKET_QU_ARCH_ADJUSTMENT_2)

// Extraction Queue Allocation
// These are logical queue numbers!
#define PACKET_XTR_QU_SPROUT   PACKET_XTR_QU_HIGH
#define PACKET_XTR_QU_STACK    PACKET_XTR_QU_HIGH
#define PACKET_XTR_QU_BPDU     PACKET_XTR_QU_MEDIUM
#define PACKET_XTR_QU_IGMP     PACKET_XTR_QU_MEDIUM
#define PACKET_XTR_QU_MGMT_MAC PACKET_XTR_QU_NORMAL /* For the switch's own MAC address                */
#define PACKET_XTR_QU_OAM      PACKET_XTR_QU_NORMAL /* For OAM frames                                  */
#define PACKET_XTR_QU_MAC      PACKET_XTR_QU_LOW    /* For other MAC addresses that require CPU copies */
#define PACKET_XTR_QU_BC       PACKET_XTR_QU_LOW    /* For Broadcast MAC address frames                */
#define PACKET_XTR_QU_LEARN    PACKET_XTR_QU_LOW    /* For the sake of MAC-based Authentication        */
#define PACKET_XTR_QU_ACL      PACKET_XTR_QU_LOW
#define PACKET_XTR_QU_SFLOW    PACKET_XTR_QU_LOWER  /* Only sFlow-marked frames must be forwarded on this queue. If not, other modules will not be able to receive the frame. */
#define PACKET_XTR_QU_LRN_ALL  PACKET_XTR_QU_LOWEST /* Only Learn-All frames end up in this queue (JR Stacking + JR-48 standalone). */

#endif /* _VTSS_SWITCH_H_ */
