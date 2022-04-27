/*

 Vitesse Switch API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_TOD_API_H_
#define _VTSS_TOD_API_H_
#include "vtss_types.h"

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "vtss_phy_ts_api.h"
#endif

// -------------------------------------------------------------------------
// vtss_hw_get_ns_cnt()
// Get the current time in a internal hw format (time counter)
u32 vtss_tod_get_ts_cnt(void);


/**
 * \brief Timer Callback, called from interupt.
 * In estax (Luton28 architecture) called from internal timer interrupt every 10 ms.
 * In Jaguar applications called every 1 sec. (When Jaguar NS timer wraps)
 * \return 0 normal, 1 => set GPIO out in Estax, 2 => clear GPIO out in Estax
 *
 */
int vtss_tod_api_callout(void);

/**
 * \brief Get the current time in a Timestamp
 * \param ts pointer to a TimeStamp structure
 * \param tc pointer to a time counter
 *
 */
void vtss_tod_gettimeofday(vtss_timestamp_t *ts, u32 *tc);

/**
 * \brief Set the current time in a Timestamp
 * \param t pointer to a TimeStamp structure
 *
 */
void vtss_tod_settimeofday(const vtss_timestamp_t *t);

/**
 * \brief Set delta from the current time  
 * \param ts       [IN] pointer to a TimeStamp structure
 * \param begative [IN] True if ts is subtracted from cuttent time, else ts is added.
 *
 */
void vtss_tod_settimeofday_delta(const vtss_timestamp_t *ts, BOOL negative);

/**
 * \brief Convert a hw time counter to current time (in sec and ns)
 *
 * \param hw_time hw time counter
 * \param t pointer to a TimeStamp structure
 *
 */
void vtss_tod_ts_to_time( u32 hw_time, vtss_timestamp_t *t);

/**
 * \brief Adjust the clock timer ratio
 *
 * \param adj Clock ratio frequency offset in 0,1ppb (parts pr billion).
 *      ratio > 0 => clock runs faster
 *
 */
void vtss_tod_set_adjtimer(i32 adj);

/* hw implementation dependent */

void vtss_tod_ts_cnt_sub(u32 *r, u32 x, u32 y);

void vtss_tod_ts_cnt_add(u32 *r, u32 x, u32 y);

u32 vtss_tod_ns_to_ts_cnt(u32 ns);

u32 vtss_tod_ts_cnt_to_ns(u32 ts);

void vtss_tod_timeinterval_to_ts_cnt(u32 *r, vtss_timeinterval_t x);

void vtss_tod_ts_cnt_to_timeinterval(vtss_timeinterval_t *r, u32 x);

char *vtss_tod_TimeInterval_To_String(const vtss_timeinterval_t *t, char* str, char delim);

void vtss_tod_sub_TimeInterval(vtss_timeinterval_t *r, const vtss_timestamp_t *x, const vtss_timestamp_t *y);

void vtss_tod_add_TimeInterval(vtss_timestamp_t *r, const vtss_timestamp_t *x, const vtss_timeinterval_t *y);

void vtss_tod_sub_TimeStamp(vtss_timestamp_t *r, const vtss_timestamp_t *x, const vtss_timeinterval_t *y);

char *vtss_tod_ns2str(u32 nanoseconds, char *str, char delim);

static inline u16 vtss_tod_unpack16(const u8 *buf)
{
    return (buf[0]<<8) + buf[1];
}

static inline void vtss_tod_pack16(u16 v, u8 *buf)
{
    buf[0] = (v>>8) & 0xff;
    buf[1] = v & 0xff;
}

static inline u32 vtss_tod_unpack32(const u8 *buf)
{
    return (buf[0]<<24) + (buf[1]<<16) + (buf[2]<<8) + buf[3];
}

static inline void vtss_tod_pack32(u32 v, u8 *buf)
{
    buf[0] = (v>>24) & 0xff;
    buf[1] = (v>>16) & 0xff;
    buf[2] = (v>>8) & 0xff;
    buf[3] = v & 0xff;
}

static inline i64 vtss_tod_unpack64(const u8 *buf)
{
    int i;
    u64 v = 0;
    for (i = 0; i < 8; i++)
    {
        v = v<<8;
        v += buf[i];
    }
    return v;
}

static inline void vtss_tod_pack64(i64 v, u8 *buf)
{
    int i;
    for (i = 7; i >= 0; i--)
    {
        buf[i] = v & 0xff;
        v = v>>8;
    }
}

#endif // _VTSS_TOD_API_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
