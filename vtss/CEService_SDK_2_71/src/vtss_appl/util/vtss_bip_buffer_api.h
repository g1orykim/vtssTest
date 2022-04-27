/*
   Copyright (c) 2003 Simon Cooke, All Rights Reserved

   Licensed royalty-free for commercial and non-commercial
   use. All that I ask is that you send me an email
   telling me that you're using my code. It'll make me
   feel warm and fuzzy inside. simoncooke@earthlink.net
*/

#ifndef _VTSS_BIP_BUFFER_API_H_
#define _VTSS_BIP_BUFFER_API_H_

/**
 * \file
 * \brief Bip Buffer.
 * \details Implementation of a circular buffer that always uses contiguous memory blocks.
 * Ported from C++ header file found here: http://www.codeproject.com/KB/IP/bipbuffer.aspx
 * 'bip' stands for "bi-partite", that is, a buffer partitioned into two regions.
 *
 * Normal use:
 * Init:
 *   Call vtss_bip_buffer_init() once at boot.
 *   Call vtss_bip_buffer_clear() to clear the buffer (but not freeing the memory).
 *
 * Write:
 *   Call vtss_bip_buffer_reserve()
 *   Call vtss_bip_buffer_commit()
 *
 * Read:
 *   Call vtss_bip_buffer_get_contiguous_block()
 *   Call vtss_bip_buffer_decommit_block()
 *
 * Normally the producer will write a small header in each block to hold the size of the
 * piece of information you're adding next.
 * The consumer will first read the small header to figure out how much data is
 * available afterwareds.
 *
 * Notes:
 *   You must provide the synchronization primitives around access to the buffer.
 *   For instance, calls to vtss_bip_buffer_reserve() and vtss_bip_buffer_commit()
 *   must be done atomically. Likewise for the two read-related calls, unless you
 *   make sure to call vtss_bip_buffer_init()/clear() from the consumer thread only.
 */

#include "vtss_types.h" /* For u8 and BOOL */

/******************************************************************************/
// The user normally doesn't need to be concerned about this structure.
// In C++ the members would've been declared private.
/******************************************************************************/
typedef struct {
  u8  *pBuffer;
  int ixa;
  int sza;
  int ixb;
  int szb;
  int buflen;
  int ixResrv;
  int szResrv;
} vtss_bip_buffer_t;

/**
 * \brief Initialize bip buffer.
 *
 * Allocates or deallocates a buffer on the heap.
 *
 * \param bip        [IN] Pointer to bip-buffer object.
 * \param buffersize [IN] Size - in bytes - of buffer to allocate. 0 to deallocate.
 *
 * \return TRUE if successful, FALSE if buffer cannot be allocated.
 */
BOOL vtss_bip_buffer_init(vtss_bip_buffer_t *bip, int buffersize);

/**
 * \brief Clears the buffer of any allocations.
 *
 * Clears the buffer of any allocations or reservations. Note; it
 * does not wipe the buffer memory; it merely resets all pointers,
 * returning the buffer to a completely empty state ready for new
 * allocations.
 *
 * \param bip [IN] Pointer to bip-buffer object.
 *
 * \return Nothing.
 */
void vtss_bip_buffer_clear(vtss_bip_buffer_t *bip);

/**
 * \brief Reserve space.
 *
 * Reserves space in the buffer for a memory write operation.
 *
 * VTSS_BEGIN
 * Notes:
 *   Will return NULL for the pointer if #size bytes cannot be allocated.
 *   Will return NULL if a previous reservation has not been committed.
 *   Compared to the original code, #reserved parameter is removed.
 * VTSS_END.
 *
 * ORIGINAL_BEGIN
 * Notes:
 *   Will return NULL for the pointer if no space can be allocated.
 *   Can return any value from 1 to size in reserved.
 *   Will return NULL if a previous reservation has not been committed.
 * ORIGINAL_END.
 *
 * \param bip      [IN]  Pointer to bip-buffer object.
 * \param size     [IN]  Amount of space (in bytes) to reserve.
 * ORIGINAL_BEGIN 
 * \param reserved [OUT] Size of space actually reserved.
 * ORIGINAL_END.
 *
 * \return Pointer to reserved block.
 */
u8 *vtss_bip_buffer_reserve(vtss_bip_buffer_t *bip, int size);

/**
 * \brief Commit previously reserved space.
 *
 * Commits space that has been written to the buffer.
 *
 * VTSS_BEGIN
 * The size previously requested with vtss_bip_buffer_reserve()
 * will be committed.
 * VTSS_END
 *
 * ORIGINAL_BEGIN
 * Notes:
 *   Committing a size > than the reserved size will cause an assert in a debug build;
 *   in a release build, the actual reserved size will be used.
 *   Committing a size < than the reserved size will commit that amount of data, and release
 *   the rest of the space.
 *   Committing a size of 0 will release the reservation.
 * ORIGINAL_END.
 *
 * \param bip  [IN] Pointer to bip-buffer object.
 * ORIGINAL_BEGIN
 * \param size [IN] Number of bytes to commit.
 * ORIGINAL_END
 *
 * \return Pointer to reserved block.
 */
void vtss_bip_buffer_commit(vtss_bip_buffer_t *bip);

/**
 * \brief Get contiguous block.
 *
 * Gets a pointer to the first contiguous block in the buffer,
 * and returns the size of that block.
 * VTSS_BEGIN
 *   Note that the bip-buffer code has no notion of length of
 *   each commit, so the value returned in #size is the total length
 *   of unread data from Region A.
 * VTSS_END
 *
 * \param bip  [IN]  Pointer to bip-buffer object.
 * \param size [OUT] Returns the size of the first contiguous block.
 *
 * \return Pointer to the first contiguous block, or NULL if empty.
 */
u8* vtss_bip_buffer_get_contiguous_block(vtss_bip_buffer_t *bip, int *size);

/**
 * \brief De-commit a block.
 *
 * Decommits space from the first contiguous block.
 *
 * \param bip  [IN] Pointer to bip-buffer object.
 * \param size [IN] Amount of memory (in bytes) to decommit.
 *
 * \return Nothing.
 */
void vtss_bip_buffer_decommit_block(vtss_bip_buffer_t *bip, int size);

/**
 * \brief Get committed size.
 *
 * Queries how much data (in total) has been committed in the buffer.
 *
 * \param bip [IN] Pointer to bip-buffer object.
 *
 * \return Total amount of committed data in the buffer.
 */
int vtss_bip_buffer_get_committed_size(vtss_bip_buffer_t *bip);

/**
 * \brief Get reservation size.
 *
 * Queries how much space has been reserved in the buffer.
 * Notes:
 *   A return value of 0 indicates that no space has been reserved.
 *
 * \param bip [IN] Pointer to bip-buffer object.
 *
 * \return Number of bytes that have been reserved.
 */
int vtss_bip_buffer_get_reservation_size(vtss_bip_buffer_t *bip);

/**
 * \brief Get buffer size.
 *
 * Queries the maximum total size of the buffer.
 *
 * \param bip [IN] Pointer to bip-buffer object.
 *
 * \return Total size of buffer.
 */
int vtss_bip_buffer_get_buffer_size(vtss_bip_buffer_t *bip);

/**
 * \brief Is initialized?
 *
 * Queries whether or not the buffer has been allocated.
 *
 * \param bip [IN] Pointer to bip-buffer object.
 *
 * \return TRUE if the buffer has been allocated. FALSE otherwise.
 */
BOOL vtss_bip_buffer_is_initialized(vtss_bip_buffer_t *bip);

#endif /* _VTSS_BIP_BUFFER_API_H_ */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

