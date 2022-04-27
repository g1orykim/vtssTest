#ifndef _NTP_CALLOUT_H_
#define _NTP_CALLOUT_H_

// These must be defined outside of this library.
// They could simply map to their standard counterparts.
#include <stdlib.h> /* For size_t */
void *ntp_callout_malloc(size_t size);
void *ntp_callout_calloc(size_t nmemb, size_t size);
char *ntp_callout_strdup(const char *str);
void  ntp_callout_free(void *ptr);

#endif  /* _NTP_CALLOUT_H_ */

