/*
 * Define malloc and friends.
 */
#ifndef  _ntp_malloc_h
#define  _ntp_malloc_h

#include "ntp_callout.h"

#ifdef NTP_ECOS
#include "ntp_config_ecos.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#else /* HAVE_STDLIB_H */
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif /* HAVE_STDLIB_H */

#endif /* _ntp_malloc_h */
