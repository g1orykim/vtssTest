#ifndef _UPNP_CALLOUT_H_
#define _UPNP_CALLOUT_H_

#include <stdlib.h> /* For size_t */
// These must be defined outside of this library.
// They could simply map to their standard counterparts.
void *upnp_callout_malloc(size_t size);
void *upnp_callout_calloc(size_t nmemb, size_t size);
void *upnp_callout_realloc(void *ptr, size_t size);
char *upnp_callout_strdup(const char *str);
void  upnp_callout_free(void *ptr);

#endif  /* _UPNP_CALLOUT_H_ */

