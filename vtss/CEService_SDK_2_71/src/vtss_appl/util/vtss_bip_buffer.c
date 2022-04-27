/*
   Copyright (c) 2003 Simon Cooke, All Rights Reserved

   Licensed royalty-free for commercial and non-commercial
   use. All that I ask is that you send me an email
   telling me that you're using my code. It'll make me
   feel warm and fuzzy inside. simoncooke@earthlink.net
*/

#include <stdlib.h>              /* For malloc/free */
#include "vtss_bip_buffer_api.h" /* For our own definitions */
#include "main.h"                /* For VTSS_MALLOC()/VTSS_FREE() */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_NONE /* Should be the caller's module ID */

#if VTSS_OPSYS_ECOS
#include <cyg/infra/diag.h>
#endif

/******************************************************************************/
// GetSpaceAfterA()
/******************************************************************************/
static inline int GetSpaceAfterA(vtss_bip_buffer_t *bip)
{
  return bip->buflen - bip->ixa - bip->sza;
}

/******************************************************************************/
// GetBFreeSpace()
/******************************************************************************/
static inline int GetBFreeSpace(vtss_bip_buffer_t *bip)
{
  return bip->ixa - bip->ixb - bip->szb;
}

/******************************************************************************/
// vtss_bip_buffer_init()
/******************************************************************************/
BOOL vtss_bip_buffer_init(vtss_bip_buffer_t *bip, int buffersize)
{
  if (bip == NULL || buffersize <= 0) {
    return FALSE;
  }

  bip->ixa     = 0;
  bip->sza     = 0;
  bip->ixb     = 0;
  bip->szb     = 0;
  bip->ixResrv = 0;
  bip->szResrv = 0;

  if (bip->pBuffer != NULL) {
    VTSS_FREE(bip->pBuffer);
    bip->pBuffer = NULL;
  }

  bip->buflen  = 0;

  bip->pBuffer = VTSS_MALLOC(buffersize);
  if (bip->pBuffer == NULL) {
    return FALSE;
  }

  bip->buflen = buffersize;
  return TRUE;
}

/******************************************************************************/
// vtss_bip_buffer_clear()
/******************************************************************************/
void vtss_bip_buffer_clear(vtss_bip_buffer_t *bip)
{
  bip->ixa = bip->sza = bip->ixb = bip->szb = bip->ixResrv = bip->szResrv = 0;
}

/******************************************************************************/
// vtss_bip_buffer_reserve()
// VTSS_BEGIN
//   #reserved argument is removed from function argument list.
// VTSS_END
/******************************************************************************/
u8 *vtss_bip_buffer_reserve(vtss_bip_buffer_t *bip, int size)
{
  int freespace;

  // VTSS_BEGIN
#if VTSS_OPSYS_ECOS
  if (bip->szResrv != 0) {
    (void)diag_printf("Warning: vtss_bip_buffer_reserve(): Trying to reserve space while there's un-committed reserved space.\n");
    return NULL;
  }
  if (size <= 0) {
    return NULL;
  }
#endif
  // VTSS_END.


  // We always allocate on B if B exists. This means we have two blocks and our buffer is filling.
  if (bip->szb) {
    freespace = GetBFreeSpace(bip);

    // VTSS_BEGIN
    // We don't want to allocate whatever is there if we can't get what we ask for.
    if (size > freespace) {
      return NULL;
    }
    // VTSS_END.

    if (size < freespace) {
      freespace = size;
    }

    if (freespace == 0) {
      return NULL;
    }

    bip->szResrv = freespace;
    bip->ixResrv = bip->ixb + bip->szb;
    return bip->pBuffer + bip->ixResrv;
  } else {
    // Block B does not exist, so we can check if the space AFTER A is bigger than the space
    // before A, and allocate the bigger one.
    freespace = GetSpaceAfterA(bip);
    if (freespace >= bip->ixa) {
      if (freespace == 0) {
        return NULL;
      }

      // VTSS_BEGIN
      // We don't want to allocate whatever is there if we can't get what we ask for.
      if (size > freespace) {
        return NULL;
      }
      // VTSS_END.

      if (size < freespace) {
        freespace = size;
      }

      bip->szResrv = freespace;
      bip->ixResrv = bip->ixa + bip->sza;
      return bip->pBuffer + bip->ixResrv;
    } else {
      if (bip->ixa == 0) {
        return NULL;
      }
      // bip->ixa corresponds to free space in B region.
      if (bip->ixa < size) {
        // VTSS_BEGIN
        // We don't want to allocate whatever is there if we can't get what we ask for.
        return NULL;
        // VTSS_END.
        // ORIGINAL_BEGIN
        // size = bip->ixa;
        // ORIGINAL_END.
      }
      bip->szResrv = size;
      bip->ixResrv = 0;
      return bip->pBuffer;
    }
  }
}

/******************************************************************************/
// vtss_bip_buffer_commit()
// VTSS_BEGIN
//   #size argument is removed from function argument list.
// VTSS_END
/******************************************************************************/
void vtss_bip_buffer_commit(vtss_bip_buffer_t *bip)
{
  // VTSS_BEGIN
  if (bip->szResrv == 0) {
#if VTSS_OPSYS_ECOS
    (void)diag_printf("Warning: vtss_bip_buffer_commit(): Trying to commit a non-reserved space.\n");
#endif
    return;
  }
  // VTSS_END.

  // If we have no blocks being used currently, we create one in A.
  if (bip->sza == 0 && bip->szb == 0) {
    bip->ixa = bip->ixResrv;
    bip->sza = bip->szResrv;

    bip->ixResrv = 0;
    bip->szResrv = 0;
    return;
  }

  if (bip->ixResrv == bip->sza + bip->ixa) {
    bip->sza += bip->szResrv;
  } else {
    bip->szb += bip->szResrv;
  }

  bip->ixResrv = 0;
  bip->szResrv = 0;
}

/******************************************************************************/
// vtss_bip_buffer_get_contiguous_block()
/******************************************************************************/
u8 *vtss_bip_buffer_get_contiguous_block(vtss_bip_buffer_t *bip, int *size)
{
  if (bip->sza == 0) {
    *size = 0;
    return NULL;
  }

  *size = bip->sza;
  return bip->pBuffer + bip->ixa;
}

/******************************************************************************/
// vtss_bip_buffer_decommit_block()
/******************************************************************************/
void vtss_bip_buffer_decommit_block(vtss_bip_buffer_t *bip, int size)
{
  if (size >= bip->sza) {
    bip->ixa = bip->ixb;
    bip->sza = bip->szb;
    bip->ixb = 0;
    bip->szb = 0;
  } else {
    bip->sza -= size;
    bip->ixa += size;
  }
}

/******************************************************************************/
// vtss_bip_buffer_get_committed_size()
/******************************************************************************/
int vtss_bip_buffer_get_committed_size(vtss_bip_buffer_t *bip)
{
  return bip->sza + bip->szb;
}

/******************************************************************************/
// vtss_bip_buffer_get_committed_size()
/******************************************************************************/
int GetReservationSize(vtss_bip_buffer_t *bip)
{
  return bip->szResrv;
}

/******************************************************************************/
// vtss_bip_buffer_get_committed_size()
/******************************************************************************/
int vtss_bip_buffer_get_buffer_size(vtss_bip_buffer_t *bip)
{
  return bip->buflen;
}

/******************************************************************************/
// vtss_bip_buffer_get_committed_size()
/******************************************************************************/
BOOL vtss_bip_buffer_is_initialized(vtss_bip_buffer_t *bip)
{
  return bip->pBuffer != NULL;
}

/******************************************************************************/
/*                                                                            */
/*  End of file.                                                              */
/*                                                                            */
/******************************************************************************/

