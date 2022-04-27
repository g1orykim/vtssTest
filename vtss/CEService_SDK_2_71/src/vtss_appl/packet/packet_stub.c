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

#include "packet_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PACKET

vtss_rc packet_init(vtss_init_data_t *data)
{
    return VTSS_OK;
}

/******************************************************************************/
// packet_tx()
// Inject frame.
/******************************************************************************/
vtss_rc packet_tx(packet_tx_props_t *tx_props)
{
    return VTSS_RC_ERROR;
}

/******************************************************************************/
// packet_tx_alloc()
// Size argument should not include IFH, CMD, and FCS
/******************************************************************************/
unsigned char *packet_tx_alloc(size_t size)
{
    printf("packet_tx_alloc.2 packet_stub.c\n");
    return VTSS_MALLOC(size);
}

/******************************************************************************/
/******************************************************************************/
void packet_tx_free(unsigned char *buffer)
{
    VTSS_FREE(buffer);
}

/******************************************************************************/
// ip_stack_glue_set_aggr_code_enable()
// API function to enable and disable computation of aggregation codes.
/******************************************************************************/
void packet_tx_aggr_code_enable(BOOL sipdip_ena, BOOL tcpudp_ena)
{
}

vtss_rc packet_rx_filter_register(const packet_rx_filter_t *filter, void **filter_id)
{
    return VTSS_OK;
}

void packet_dbg(packet_dbg_printf_t dbg_printf, ulong parms_cnt, ulong *parms)
{
}

void packet_ipv4_set(vtss_ipv4_t ipv4_addr)
{
}

unsigned char *packet_tx_alloc_extra(size_t size, size_t extra_size_dwords, unsigned char **extra_ptr)
{
    unsigned char *buffer;
    size_t extra_size_bytes = 4 * extra_size_dwords;
    if ((buffer = VTSS_MALLOC(size + extra_size_bytes))) {
        *extra_ptr = buffer;
        buffer += extra_size_bytes;
    }
    return buffer;
}

void packet_tx_free_extra(unsigned char *extra_ptr)
{
    VTSS_FREE(extra_ptr);
}

vtss_rc packet_rx_filter_change(const packet_rx_filter_t *filter, void **filter_id)
{
    return PACKET_ERROR_GEN;
}

vtss_rc packet_rx_filter_unregister(void *filter_id)
{
    return PACKET_ERROR_GEN;
}

void packet_rx_no_arp_discard_add_ref(vtss_port_no_t port)
{
}

void packet_rx_no_arp_discard_release(vtss_port_no_t port)
{
}

void packet_rx_release_fdma_buffers(void *fdma_buffers)
{
}

void packet_tx_props_init(packet_tx_props_t *tx_props)
{
}

#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
vtss_rc packet_rx_throttle_cfg_get(vtss_chip_no_t chip_no, vtss_fdma_throttle_cfg_t *cfg)
{
    return PACKET_ERROR_GEN;
}
#endif

#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
vtss_rc packet_rx_throttle_cfg_set(vtss_chip_no_t chip_no, vtss_fdma_throttle_cfg_t *cfg)
{
    return PACKET_ERROR_GEN;
}
#endif

#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
char *packet_rx_queue_usage(u32 xtr_qu, char *buf, size_t size)
{
    return "";
}
#endif

