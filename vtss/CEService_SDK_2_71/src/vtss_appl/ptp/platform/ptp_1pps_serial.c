/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
/* Implement the serial 1pps interface */

#include "main.h"
#if defined (VTSS_ARCH_SERVAL)
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <time.h>

#include "ptp_1pps_serial.h"
#include "vtss_ptp_api.h"
#include "ptp.h"
#include "ptp_api.h"

static unsigned char serial_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;
static cyg_io_handle_t ser1;

static const char *ptp_1pps_convert_time_2_message(const vtss_timestamp_t *t)
{
    static char buf[64];
    struct tm *timeinfo_p;
    int len, i;
    u8 checksum = 0;
    if(t->seconds > 0) {
        time_t my_time = t->seconds;
        timeinfo_p = localtime(&my_time);
        if (timeinfo_p->tm_year >= 100) timeinfo_p->tm_year -= 100;
        len = snprintf(buf, sizeof(buf)-1, "$POLYT,%02d%02d%02d.00,%02d%02d%02d,,,,%d,,,*", 
                 timeinfo_p->tm_hour,
                 timeinfo_p->tm_min,
                 timeinfo_p->tm_sec,
                 timeinfo_p->tm_mday,
                 timeinfo_p->tm_mon+1,
                 timeinfo_p->tm_year, 
                 t->nanoseconds);
        for (i = 0; i < len; i ++) {
            if (buf[i] != '$' && buf[i] != '*') checksum += (unsigned char)buf[i];
        }
        (void)snprintf(buf+len, sizeof(buf)-1-len, "%02x%c%c", checksum, 0xd, 0xa); 
        
        return buf;
    }
    return "-";
}

static int ptp_1pps_convert_message_2_time(const char *str, vtss_timestamp_t *t)
{
    struct tm timeinfo;
    int res = 0;
    size_t i;
    t->sec_msb = 0;
    t->seconds = 0;
    t->nanoseconds = 0;
    u8 checksum = 0;
    uint rx_sum = 0;
    if(str != NULL) {
        res = sscanf(str, "$POLYT,%02d%02d%02d.00,%02d%02d%02d,,,,%u,,,*%02x", &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec,
                     &timeinfo.tm_mday, &timeinfo.tm_mon, &timeinfo.tm_year, &t->nanoseconds, &rx_sum);
        if (res == 8) {
            timeinfo.tm_mon-=1;
            if (timeinfo.tm_year < 70) timeinfo.tm_year += 100;
            t->seconds = mktime(&timeinfo);
            T_RG(VTSS_TRACE_GRP_PTP_SER,"checksum, = %x", rx_sum);
            for (i = 0; i < strlen(str) && str[i] != '*'; i ++) {
                if (str[i] != '$' ) checksum += (unsigned char)str[i];
            }
            if (checksum != rx_sum) {
                T_IG(VTSS_TRACE_GRP_PTP_SER,"Checksum error, checksum: %x, rx_sum: %x", checksum, rx_sum);
                res = 0;
            }
        } else {
            T_IG(VTSS_TRACE_GRP_PTP_SER,"Parse error, res = %d", res);
            res = 0;
        }
    }
    return res;
}

void ptp_1pps_msg_send(const vtss_timestamp_t *t)
{
    const char *buf = ptp_1pps_convert_time_2_message(t);
    size_t len = strlen(buf);
    if (cyg_io_write(ser1, buf, &len)  != ENOERR) {
        T_WG(VTSS_TRACE_GRP_PTP_SER,"Error writing to ser1");
    }
}

//---------------------------------------------------------------------------
// Serial input main function.
static void ptp_1pps_serial_thread( void )
{
    cyg_uint8 in_buffer[1];
    char rx_buffer[64];
    unsigned int len = 1;
    unsigned int rx_idx = 0;
    int p = 0;
    vtss_timestamp_t rx_t;
    char str [40];

    if (cyg_io_lookup("/dev/ser1", &ser1) != ENOERR) {
        T_EG(VTSS_TRACE_GRP_PTP_SER,"Error opening /dev/ser1");
        exit(1);
    }
    memset(rx_buffer, 0, sizeof(rx_buffer));
    while (1) {
        len = 1;
        if (cyg_io_read(ser1, in_buffer, &len) != ENOERR) {
            T_WG(VTSS_TRACE_GRP_PTP_SER,"Error reading from ser1");
        } else {
            if(len) {
                if (in_buffer[0] == '$') {
                    rx_idx = 0;
                    memset(rx_buffer, 0, sizeof(rx_buffer));
                    rx_buffer[rx_idx++] = in_buffer[0];
                } else if (in_buffer[0] == 0x0a) {
                    // call out to handler 
                    T_NG(VTSS_TRACE_GRP_PTP_SER,"Received: %s", rx_buffer);
                    if (8 != (p = ptp_1pps_convert_message_2_time(rx_buffer, &rx_t))) {
                        T_IG(VTSS_TRACE_GRP_PTP_SER,"interpreted only: %d parameters", p);
                    } else {
                        T_DG(VTSS_TRACE_GRP_PTP_SER,"time received %s", TimeStampToString (&rx_t, str));
                        vtss_ext_clock_rs422_time_set(&rx_t);
                    }
                } else {
                    rx_buffer[rx_idx++] = in_buffer[0];
                    if (rx_idx >= sizeof(rx_buffer)) {
                        T_WG(VTSS_TRACE_GRP_PTP_SER,"Rx buffer overflow");
                        rx_idx = 0;
                    }
                }
            }
        }
    }
}

vtss_rc ptp_1pps_serial_init(void)
{
    cyg_thread_create(THREAD_BELOW_NORMAL_PRIO,                     // Priority
                      (cyg_thread_entry_t*)ptp_1pps_serial_thread,  // entry
                      0,                    // 
                      "PTP-serial",         // Name
                      serial_thread_stack,            // Stack
                      sizeof(serial_thread_stack),                // Size
                      &thread_handle,       // Handle
                      &thread_data          // Thread data structure
                     );
    cyg_thread_resume(thread_handle);
    return VTSS_RC_OK;
}
#endif /* (VTSS_ARCH_SERVAL) */

