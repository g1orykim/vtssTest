/*

 Vitesse API software.

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

#ifndef __VTSS_MPLS_COMMON_H__
#define	__VTSS_MPLS_COMMON_H__

#if defined (VTSS_FEATURE_MPLS)

/*----------------------------------------------------------------------------
 * Index chain functions.
 *
 * There are several cases where we need to create a dynamically-sized list
 * of indices. For example, a multicast list of out-segments, or a list of
 * client LSPs that use a given server LSP.
 *
 * Since the API cannot allocate memory dynamically, and since we don't want
 * to impose too-strict per-entity limits (e.g. "a multicast list can have max
 * 6 segments"), we have this structure for creating chains of user indices
 * (i.e. indices that have meaning to the chain in question).
 *
 * *Note* that user indices are *signed* 16-bit.
 *
 * An index chain is initialized like this:
 *
 *     vtss_mpls_idxchain_t chain = VTSS_MPLS_IDXCHAIN_UNDEFINED;
 *     // or, later:
 *     VTSS_MPLS_IDXCHAIN_UNDEF(chain);
 *
 * End-of-chain can be tested like this:
 *
 *     if (VTSS_MPLS_IDXCHAIN_END(chain)) { ... }
 *
 * A user index is inserted in the list like this:
 *
 *     vtss_mpls_idxchain_add(&chain, 100);       // 100 is the user index
 *
 * Similar functions exist for removing user indices from the chain, or for
 * replacing one index with another.
 *
 * The preferred method to iterate a chain is by using two get_first/get_next
 * functions. Their main limitation is that the chain must be constant with
 * regards to inserts/removals during iteration. They are used like this:
 *
 *     vtss_mpls_idxchain_iter_t iter;
 *     vtss_mpls_idxchain_user_t user;
 *     BOOL more = vtss_mpls_idxchain_get_first(chain, &iter, &user);
 *     while (more) {
 *         // do something with user index
 *         more = vtss_mpls_idxchain_get_next(&iter, &user);
 *     }
 *
 *--------------------------------------------------------------------------*/



typedef i16                  vtss_mpls_idxchain_t;       /**< Index chain */
typedef i16                  vtss_mpls_idxchain_user_t;  /**< User index contained in an index chain entry */
typedef vtss_mpls_idxchain_t vtss_mpls_idxchain_iter_t;  /**< Index chain iterator */

#define VTSS_MPLS_IDXCHAIN_UNDEFINED    (-1)
#define VTSS_MPLS_IDXCHAIN_UNDEF(c)     do { (c) = VTSS_MPLS_IDXCHAIN_UNDEFINED; } while (0)
#define VTSS_MPLS_IDXCHAIN_END(c)       ((c) == VTSS_MPLS_IDXCHAIN_UNDEFINED)

/** \brief Index chain element. Don't use directly; treat as opaque. */
typedef struct {
    vtss_mpls_idxchain_user_t   user;   /**< Contained user index */
    vtss_mpls_idxchain_t        next;   /**< Next entry in chain, or VTSS_MPLS_IDXCHAIN_UNDEFINED for end-of-chain */
} vtss_mpls_idxchain_entry_t;



/** \brief Initialize global idxchain data structure (in vtss_state.h). */
void vtss_mpls_idxchain_init(struct vtss_state_s *vtss_state);



/** \brief Add a user index to a chain. Multiple identical user indices are
 * allowed.
 *
 * \param chain [IN/OUT]  Pointer to chain.
 * \param user  [IN]      User index to add.
 *
 * \return Error if invalid params, or no free index chain entries remain.
 */
vtss_rc vtss_mpls_idxchain_insert_at_head(struct vtss_state_s *vtss_state,
                                          vtss_mpls_idxchain_t * const    chain,
                                          const vtss_mpls_idxchain_user_t user);



/** \brief Append a user index to the tail of a chain. Multiple identical user
 * indices are allowed.
 *
 * NOTE: O(n), so slower than @vtss_mpls_idxchain_insert_at_head() which is O(1).
 *
 * \param chain [IN/OUT]  Pointer to chain.
 * \param user  [IN]      User index to add.
 *
 * \return Error if invalid params, or no free index chain entries remain.
 */
vtss_rc vtss_mpls_idxchain_append_to_tail(struct vtss_state_s *vtss_state,
                                          vtss_mpls_idxchain_t * const    chain,
                                          const vtss_mpls_idxchain_user_t user);



/** \brief Remove one user index from chain. If multiple occurrences of the
 * user index exist, only one of them is removed.
 *
 * \param chain [IN/OUT]  Pointer to chain.
 * \param user  [IN]      User index to remove.
 *
 * \return Error if invalid params, or user index wasn't found.
 */
vtss_rc vtss_mpls_idxchain_remove_first(struct vtss_state_s *vtss_state,
                                        vtss_mpls_idxchain_t * const    chain,
                                        const vtss_mpls_idxchain_user_t user);



/** \brief Remove all entries from a chain, emptying it completely.
 *
 * \param chain [IN/OUT]  Pointer to chain.
 *
 * \return Error if invalid params.
 */
vtss_rc vtss_mpls_idxchain_remove_all(struct vtss_state_s *vtss_state,
                                      vtss_mpls_idxchain_t * const chain);



/** \brief Replace one user index with another. If multiple occurrences of the
 * user index exist, one or all can be replaced.
 *
 * \param chain    [IN]      Pointer to chain.
 * \param user_old [IN]      User index to replace.
 * \param user_new [IN]      New user index.
 * \param all      [IN]      TRUE == replace all occurrences of user_old
 *
 * \return Error if invalid params, or user index wasn't found.
 */
vtss_rc vtss_mpls_idxchain_replace(struct vtss_state_s *vtss_state,
                                   const vtss_mpls_idxchain_t * const chain,
                                   const vtss_mpls_idxchain_user_t    user_old,
                                   const vtss_mpls_idxchain_user_t    user_new,
                                   const BOOL                         all);



/** \brief Initiate iteration over the user indices of a chain.
 *
 * \param chain    [IN]      Pointer to chain.
 * \param iter     [OUT]     Iterator data structure. Treat as opaque.
 * \param user     [OUT]     "First" user index (from head-of-chain)
 *
 * \return TRUE if @user is valid; FALSE if chain is empty (or error in params).
 */
BOOL vtss_mpls_idxchain_get_first(struct vtss_state_s *vtss_state,
                                  const vtss_mpls_idxchain_t * const chain,
                                  vtss_mpls_idxchain_iter_t  * const iter,
                                  vtss_mpls_idxchain_user_t  * const user);



/** \brief Get next user index from iteration.
 *
 * \param iter     [IN/OUT]  Iterator data structure. Treat as opaque.
 * \param user     [OUT]     "Next" user index
 *
 * \return TRUE if @user is valid; FALSE if at end-of-chain (or error in params).
 */
BOOL vtss_mpls_idxchain_get_next(struct vtss_state_s *vtss_state,
                                 vtss_mpls_idxchain_iter_t * const iter,
                                 vtss_mpls_idxchain_user_t * const user);



/** \brief Try to find user index.
 *
 * \param chain    [IN] Pointer to chain.
 * \param user     [IN] User index to find.
 *
 * \return TRUE if found; FALSE otherwise
 */
BOOL vtss_mpls_idxchain_find(struct vtss_state_s *vtss_state,
                             const vtss_mpls_idxchain_t * const chain,
                             const vtss_mpls_idxchain_user_t    user);



#endif  /* VTSS_FEATURE_MPLS */

#endif	/* __VTSS_MPLS_COMMON_H__ */

