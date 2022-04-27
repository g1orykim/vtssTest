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

#include <main.h>
#include <vtss_fifo_api.h>
#include <stdlib.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_NONE /* Should be the caller's module ID */

/******************************************************************************/
/******************************************************************************/
vtss_rc vtss_fifo_init(vtss_fifo_t *fifo, ulong init_sz, ulong max_sz, ulong growth, ulong keep_statistics)
{
  VTSS_ASSERT(max_sz >= init_sz);
  VTSS_ASSERT((max_sz>init_sz && growth>0) || max_sz==init_sz);
  VTSS_ASSERT(init_sz>1 || max_sz>0);

  if(init_sz>0) {
    if((fifo->items = VTSS_MALLOC(init_sz*sizeof(void *)))==NULL) {
      return VTSS_UNSPECIFIED_ERROR;
    }
  }
  fifo->sz      = init_sz;
  fifo->max_sz  = max_sz;
  fifo->growth  = growth;
  fifo->wr_idx = fifo->rd_idx = fifo->cnt = fifo->max_cnt = fifo->total_cnt = fifo->overruns = fifo->underruns = 0;
  fifo->keep_statistics = keep_statistics;
  return VTSS_OK;
}

/******************************************************************************/
/******************************************************************************/
vtss_rc vtss_fifo_wr(vtss_fifo_t *fifo, void *item)
{
  if(fifo->cnt==fifo->sz) {
    // FIFO full. Reallocate memory, if we haven't reached the upper limit.
    if(fifo->sz < fifo->max_sz) {
      ulong new_sz = (fifo->sz + fifo->growth > fifo->max_sz) ? fifo->max_sz : fifo->sz + fifo->growth;
      void **new_items = VTSS_REALLOC(fifo->items, new_sz*sizeof(void *));
      if(new_items==NULL) {
        return VTSS_UNSPECIFIED_ERROR;
      }
      fifo->items = new_items;
      fifo->sz = new_sz;
    } else {
      // FIFO full and we're not allowed to allocate anymore memory.
      if (fifo->keep_statistics) {
        fifo->overruns++;
      }
      return VTSS_UNSPECIFIED_ERROR;
    }
  }

  fifo->items[fifo->wr_idx++]=item;

  // Modulo operator doesn't seem to work (at least not when
  // inside a DSR). When branching to the __umodsi3 function, it
  // changes the register set.
  if(fifo->wr_idx==fifo->sz)
    fifo->wr_idx=0;
  // fifo->wr_idx = (fifo->wr_idx+1)%fifo->sz;
  fifo->cnt++;

  if(fifo->keep_statistics) {
    if(fifo->cnt > fifo->max_cnt)
      fifo->max_cnt = fifo->cnt;
    fifo->total_cnt++;
  }
  return VTSS_OK;
}

/******************************************************************************/
/******************************************************************************/
void *vtss_fifo_rd(vtss_fifo_t *fifo)
{
  void *result;
  if(fifo->cnt==0) {
    if (fifo->keep_statistics) {
      fifo->underruns++;
    }
    return NULL;
  }

  fifo->cnt--;
  result = fifo->items[fifo->rd_idx++];

  // Modulo operator doesn't seem to work (at least not when
  // inside a DSR). When branching to the __umodsi3 function, it
  // changes the register set.
  if(fifo->rd_idx==fifo->sz)
    fifo->rd_idx=0;
  // fifo->rd_idx = (fifo->rd_idx+1)%fifo->sz;
  return result;
}

/******************************************************************************/
/******************************************************************************/
ulong vtss_fifo_cnt(vtss_fifo_t *fifo)
{
  return fifo->cnt;
}

/******************************************************************************/
/******************************************************************************/
void vtss_fifo_get_statistics(vtss_fifo_t *fifo, ulong *max_cnt, ulong *total_cnt, ulong *cur_cnt, ulong *cur_sz, ulong *overruns, ulong *underruns)
{
  *max_cnt   = fifo->max_cnt;
  *total_cnt = fifo->total_cnt;
  *cur_cnt   = fifo->cnt;
  *cur_sz    = fifo->sz;
  *overruns  = fifo->overruns;
  *underruns = fifo->underruns;
}

/******************************************************************************/
/******************************************************************************/
void vtss_fifo_clr_statistics(vtss_fifo_t *fifo)
{
  fifo->max_cnt   = 0;
  fifo->total_cnt = 0;
  fifo->overruns  = 0;
  fifo->underruns = 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
