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
 
 $Id$
 $Revision$

*/

#ifndef _VTSS_FIFO_CP_API_H_
#define _VTSS_FIFO_CP_API_H_

/******************************************************************************/
// The vtss_fifo is a dynamically growing list of any-sized user data, with a 
// write and read operator. The list cannot shrink.
// When you initialize the FIFO, you specify the initial number of items
// and its maximum number of items together with a growth size, which is used
// if an item is added to the FIFO when it's full.
// You can build up the items you write to the FIFO on the stack, since
// the data is memcpy'd into the FIFO. Likewise, when popping the FIFO, you
// can allocate the structure on the stack and call the pop operator with a
// pointer to the structure. It's not required that you use VTSS_MALLOC() to
// allocate the item (unless it's very large, in which case, I suggest that
// you use the vtss_fifo component, which stores pointers).
//
// NOTE: IF MORE THAN ONE THREAD ACCESSES THE FIFO, PROTECT IT WITH A MUTEX!!
/******************************************************************************/

/******************************************************************************/
// The user normally doesn't need to be concerned about this structure.
// In C++ the members would've been declared private.
/******************************************************************************/
typedef struct {
  ulong *items;
  ulong user_item_sz_bytes;
  ulong alloc_item_sz_dwords;
  ulong sz;
  ulong cnt;
  ulong max_sz;
  ulong growth;
  ulong wr_idx;
  ulong rd_idx;
  ulong keep_statistics;
  ulong max_cnt;   // The highest simultaneously seen number of items in the fifo
  ulong total_cnt; // The total number of items that have been added to the fifo.
  ulong overruns;  // The number of times calls to vtss_fifo_cp_wr() failed.
  ulong underruns; // The number of times calls to vtss_fifo_cp_rd() failed.
} vtss_fifo_cp_t;

/******************************************************************************/
/******************************************************************************/
vtss_rc vtss_fifo_cp_init(vtss_fifo_cp_t *fifo, ulong item_sz_bytes, ulong init_sz, ulong max_sz, ulong growth, ulong keep_statistics);
vtss_rc vtss_fifo_cp_wr(vtss_fifo_cp_t *fifo, void *item);
vtss_rc vtss_fifo_cp_rd(vtss_fifo_cp_t *fifo, void *item);
vtss_rc vtss_fifo_cp_clr(vtss_fifo_cp_t *fifo);
ulong   vtss_fifo_cp_cnt(vtss_fifo_cp_t *fifo);
void    vtss_fifo_cp_get_statistics(vtss_fifo_cp_t *fifo, ulong *max_cnt, ulong *total_cnt, ulong *cur_cnt, ulong *cur_sz, ulong *overruns, ulong *underruns);
void    vtss_fifo_cp_clr_statistics(vtss_fifo_cp_t *fifo);

#endif // _VTSS_FIFO_CP_API_H_
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
