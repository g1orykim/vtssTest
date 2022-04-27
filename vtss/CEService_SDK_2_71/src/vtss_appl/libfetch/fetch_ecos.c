/*-
 * Copyright (c) 2013 Vitesse Semiconductor Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "libfetch_ecos_config.h"
#include "libfetch_callout.h"
#include "fetch.h"

#include <stdlib.h>
#include <stdio.h>

#if !defined(VTSS_SW_OPTION_NTP)

#define is_leap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

time_t
timegm (struct tm *tm)
{
    static const unsigned ndays[2][12] ={
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
    time_t res = 0;
    unsigned i;

    if (tm->tm_year < 0) 
        return -1;
    if (tm->tm_mon < 0 || tm->tm_mon > 11) 
        return -1;
    if (tm->tm_mday < 1 || tm->tm_mday > ndays[is_leap(tm->tm_year)][tm->tm_mon])
        return -1;
    if (tm->tm_hour < 0 || tm->tm_hour > 23) 
        return -1;
    if (tm->tm_min < 0 || tm->tm_min > 59) 
        return -1;
    if (tm->tm_sec < 0 || tm->tm_sec > 59) 
        return -1;

    for (i = 70; i < tm->tm_year; ++i)
        res += is_leap(i) ? 366 : 365;

    for (i = 0; i < tm->tm_mon; ++i)
        res += ndays[is_leap(tm->tm_year)][i];
    res += tm->tm_mday - 1;
    res *= 24;
    res += tm->tm_hour;
    res *= 60;
    res += tm->tm_min;
    res *= 60;
    res += tm->tm_sec;
    return res;
}
#endif

int gethostname(char *name, size_t len)
{
    strncpy(name, "noname.com", len);
    return strlen(name);
}

int vasprintf(char **strp, const char *fmt, va_list ap)
{
    int len = 256, used;
    char *buf = libfetch_callout_malloc(len);
    if (buf) {
        while ((used = vsnprintf(buf, len, fmt, ap)) == len) {
            /* Exhausted buffer */
            len <<= 2;
            libfetch_callout_free(buf);
            if ((buf = libfetch_callout_malloc(len)) == NULL) {
                *strp = NULL;
                return -1;              /* memory alloc error */
            }
        }
    } else {
        *strp = NULL;
        return -1;              /* memory alloc error */
    }
    /* Store allocated buffer */
    *strp = buf;
    return used;                /* chars in buffer */
}

int asprintf(char **strp, const char *fmt, ...)
{
    int ret;
    va_list va;
    va_start(va, fmt);
    ret = vasprintf(strp, fmt, va);
    va_end(va);
    return ret;
}

int
fetch_netrc_auth(struct url *url)
{
    return (-1);
}

char *getlogin(void)
{
    static char userid[] = "root";
    return userid;
}
