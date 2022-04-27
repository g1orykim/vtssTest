#ifndef _LIBFETCH_CALLOUT_H_
#define _LIBFETCH_CALLOUT_H_

// These must be defined outside of this library.
// They could simply map to their standard counterparts.
void *libfetch_callout_malloc(size_t size);
void *libfetch_callout_calloc(size_t nmemb, size_t size);
void *libfetch_callout_realloc(void *ptr, size_t size);
char *libfetch_callout_strdup(const char *str);
void  libfetch_callout_free(void *ptr);

#endif  /* _LIBFETCH_CALLOUT_H_ */

