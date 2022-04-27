#ifndef _TAC_CALLOUT_H_
#define _TAC_CALLOUT_H_

// These must be defined outside of this library.
// They could simply map to their standard counterparts.
#include <stdlib.h> /* For size_t */
void *tac_callout_malloc(size_t size);
char *tac_callout_strdup(const char *str);
void  tac_callout_free(void *ptr);

#endif  /* _TAC_CALLOUT_H_ */

