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
#include "vtss_types.h"
#include "cli.h" /* Only for debug purpose. It will be removed later */

static BOOL packet_dump = FALSE;

/* pkt_control    : flag to dump the packet.                               */
/* Management interface to enable for mrp packet dump.                     */
void xxrp_pkt_dump_set(BOOL  pkt_control)
{
    if (pkt_control == TRUE) {
        packet_dump = TRUE;
    } else {
        packet_dump = FALSE;
    }
    return;
}

/* packet         : pdu pointer.                                           */
/* packet_transmit: Transmit or receive flag.                              */
/* Dumps the packet on console.                                            */
void xxrp_packet_dump(u32 port_no, const u8 *packet, BOOL packet_transmit)
{
    u32 l_index = 0;

    if (packet_dump) {
        if (packet_transmit) {
            printf("\nTransmitting MRP packet on port = %u\n", port_no);
        } else {
            printf("\nReceiving MRP packet on port = %u\n", port_no);
        }
        if (packet != NULL) {
            for (l_index = 0; l_index < 500; l_index++) {
                printf("%02x ", packet[l_index]);
                if (((l_index + 1) % 30) == 0) {
                    printf("\n");
                }
            }
            printf("\n");
        }
    }
}
