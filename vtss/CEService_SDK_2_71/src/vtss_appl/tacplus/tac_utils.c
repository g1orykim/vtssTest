#include "libtacplus.h"

int tac_error(const char *format, ...)
{
    va_list ap;
    int result;

    /*lint -e{530} ... 'ap' is initialized by va_start() */
    va_start(ap, format);
    result = vfprintf(stderr, format, ap);
    va_end(ap);
    return result;
}

/* free avpairs array */
void tac_free_avpairs(char **avp)
{
    int i = 0;
    while (avp[i] != NULL) {
        tac_callout_free(avp[i++]);
    }
}
