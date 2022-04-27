#ifndef _SSH_LIB_CALLOUT_H_
#define _SSH_LIB_CALLOUT_H_

// These must be defined outside of this library.
// They could simply map to their standard counterparts.
void *ssh_lib_callout_malloc(size_t size);
void *ssh_lib_callout_calloc(size_t nmemb, size_t size);
void *ssh_lib_callout_realloc(void *ptr, size_t size);
char *ssh_lib_callout_strdup(const char *str);
void  ssh_lib_callout_free(void *ptr);

#endif  /* _SSH_LIB_CALLOUT_H_ */

