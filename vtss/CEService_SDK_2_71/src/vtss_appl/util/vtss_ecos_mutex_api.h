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

// For eCos mutex definition:
#include <cyg/kernel/kapi.h>

/* Mutex type */
typedef enum {
  VTSS_ECOS_MUTEX_TYPE_NORMAL,   /* Just wraps a normal eCos Mutex */
  VTSS_ECOS_MUTEX_TYPE_RECURSIVE /* Allows the same mutex to be locked several times by the same thread. Use with care. */
} vtss_ecos_mutex_type_t;

/* Mutexes */
typedef struct
{
  // Public members:
  vtss_ecos_mutex_type_t type;       /* Mutex type */
 
  // Private members:
  cyg_mutex_t            cyg_mutex;  /* eCos mutex */
  int                    lock_cnt;   /* Number of locks */
} vtss_ecos_mutex_t;

// Don't use these, but rather the macros with the same, but capitalized, name.
void vtss_ecos_mutex_lock(vtss_ecos_mutex_t *mutex);
BOOL vtss_ecos_mutex_trylock(vtss_ecos_mutex_t *mutex); // Returns TRUE if now locked, FALSE if already locked by another thread.
void vtss_ecos_mutex_unlock(vtss_ecos_mutex_t *mutex);

/* Initialize mutex. */
void vtss_ecos_mutex_init(vtss_ecos_mutex_t *mutex);

/* Lock the mutex */
#define VTSS_ECOS_MUTEX_LOCK(m) vtss_ecos_mutex_lock(m)

/* Attempt to lock the mutex */
#define VTSS_ECOS_MUTEX_TRYLOCK(m) vtss_ecos_mutex_trylock(m)

/* Unlock the mutex */
#define VTSS_ECOS_MUTEX_UNLOCK(m) vtss_ecos_mutex_unlock(m)


