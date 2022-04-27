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
 
 $Id$
 $Revision$

*/

#ifndef _VTSS_FIFO_API_H_
#define _VTSS_FIFO_API_H_

#include <vtss_api.h>

/******************************************************************************/
// The vtss_fifo is a dynamically growing list of pointers, with write and 
// read operators. The list cannot shrink.
// When you initialize the FIFO, you specify the initial number of items
// and its maximum number of items together with a growth size, which is used
// if an item is added to the FIFO when it's full.
//
// NOTE: IF MORE THAN ONE THREAD ACCESSES THE FIFO, PROTECT IT WITH A MUTEX!!
//
// If you need to store more data than just a pointer, use the vtss_fifo_cp
// variant, which memcpy's your any-sized data into the FIFO when writing, 
// and memcpy's it out again when reading.
/******************************************************************************/

/******************************************************************************/
// The user normally doesn't need to be concerned about this structure.
// In C++ the members would've been declared private.
/******************************************************************************/
typedef struct {
  ulong sz;
  ulong cnt;
  ulong max_sz;
  ulong growth;
  void **items;
  ulong wr_idx;
  ulong rd_idx;
  ulong keep_statistics;
  ulong max_cnt;   // The highest simultaneously seen number of items in the fifo
  ulong total_cnt; // The total number of items that have been added to the fifo.
  ulong overruns;  // The number of times calls to vtss_fifo_cp() failed.
  ulong underruns; // The number of times calls to vtss_fifo_cp() failed.
} vtss_fifo_t;

/******************************************************************************/
// RBNTBD: API usage to be added.
/******************************************************************************/
vtss_rc vtss_fifo_init(vtss_fifo_t *fifo, ulong init_sz, ulong max_sz, ulong growth, ulong keep_statistics);
vtss_rc vtss_fifo_wr(vtss_fifo_t *fifo, void *item);
void    *vtss_fifo_rd(vtss_fifo_t *fifo);
ulong   vtss_fifo_cnt(vtss_fifo_t *fifo);
void    vtss_fifo_get_statistics(vtss_fifo_t *fifo, ulong *max_cnt, ulong *total_cnt, ulong *cur_cnt, ulong *cur_sz, ulong *overruns, ulong *underruns);
void    vtss_fifo_clr_statistics(vtss_fifo_t *fifo);

#endif // _VTSS_FIFO_API_H_
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
