/*

 Vitesse Switch Software.

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
/* vtss_tod_api.c */

#include "vtss_tod_api.h"
#include "tod.h"
#include "main.h"

#if defined(VTSS_ARCH_LUTON28)
#include "cyg/hal/vcoreii_clockadj.h"
#include "cyg/hal/vcoreii.h"
#else
#include "vtss_ts_api.h"

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "vtss_tod_mod_man.h"
#endif

//static i32 adjust_divisor = 0;

#endif

#if defined(VTSS_ARCH_LUTON28)
static uint actual_period = CYGNUM_HAL_RTC_PERIOD;
static cyg_tick_count_t ticks_per_second = 1000000000/
        (CYGNUM_HAL_RTC_NUMERATOR/CYGNUM_HAL_RTC_DENOMINATOR);
static const u32 ns_pr_tick = CYGNUM_HAL_RTC_NUMERATOR/CYGNUM_HAL_RTC_DENOMINATOR;
static uint tick_counter = 0; /* counts 10 ms timer ticks */
static u32 hw_timeofday = 0; /* time of day in seconds */

/* called from real time clock ISR */
int vtss_tod_api_callout(void)
{
    int rc = 0;
    /* update clock output */
    if (++tick_counter >= ticks_per_second) {
        rc = 1;
        tick_counter = 0;
        ++hw_timeofday;
    }
    if (tick_counter == ticks_per_second/2) {
        rc = 2;
    }
    return rc;
}
#endif

// -------------------------------------------------------------------------
// vtss_tod_gettimeofday()
// Get the current time in a Timestamp
// Luton 28: get time from HAL layer with 1 us resolution.
// Jaguar  : get time from API layer (SW) with 4 ns resolution.
// Luton 26: get time from API layer (HW) with 4 ns resolution.
void vtss_tod_gettimeofday(vtss_timestamp_t *ts, u32 *tc)
{
#if defined(VTSS_ARCH_LUTON28)
    cyg_uint32 clks;
    u32 cur_sec;
    uint ticks;
    // read hw counter and timer ticks as an atomic operation
    cyg_scheduler_lock();
    HAL_CLOCK_READ(&clks);
    cur_sec = hw_timeofday;
    ticks = tick_counter;
    cyg_scheduler_unlock();

    ts->sec_msb = 0;
    ts->seconds = cur_sec;
    ts->nanoseconds = (ticks * 1000000)/((uint)ticks_per_second);
    ts->nanoseconds += (10000 * clks) / actual_period;
    ts->nanoseconds = 1000*ts->nanoseconds;
    T_NG(_C,"t_sec = %d,  t_nsec = %d\n",ts->seconds, ts->nanoseconds);
    *tc = clks +(ticks<<16);
#else
    TOD_RC(vtss_ts_timeofday_get(NULL, ts, tc));
    //if (adjust_divisor) {
    //    T_DG(_C,"before ns: %lu, tc: %lu", ts->nanoseconds, *tc);
    //    ts->nanoseconds += (i32)ts->nanoseconds/adjust_divisor;
    //    T_DG(_C,"adjustment != 0: adjust_factor = %ld", adjust_divisor);
    //    T_DG(_C,"after ns: %lu", ts->nanoseconds);
    //}
    
#endif
}


// -------------------------------------------------------------------------
// vtss_tod_settimeofday()
// Set the current time from a Timestamp
// Luton 28: set internal time .
// Jaguar  : set time in API layer (SW).
// Luton 26: set time in API layer (HW).
void vtss_tod_settimeofday(const vtss_timestamp_t *ts)
{
#if defined(VTSS_ARCH_LUTON28)
    cyg_scheduler_lock();
    hw_timeofday = ts->seconds;
    tick_counter = ts->nanoseconds/ns_pr_tick;
    cyg_scheduler_unlock();
#else
    TOD_RC(vtss_ts_timeofday_set(NULL, ts));
#endif
}

// -------------------------------------------------------------------------
// vtss_tod_settimeofday_delta()
// Set delta from the current time 
// Luton 28: set internal time .
// Jaguar  : set time in API layer (SW).
// Luton 26, Serval: set time in API layer (HW).
void vtss_tod_settimeofday_delta(const vtss_timestamp_t *ts, BOOL negative)
{
#if defined(VTSS_ARCH_LUTON28)
    cyg_scheduler_lock();
    if (negative) {
        hw_timeofday -= ts->seconds;
        tick_counter -= ts->nanoseconds/ns_pr_tick;
        if (++tick_counter >= ticks_per_second) {
          tick_counter += ticks_per_second;
          --hw_timeofday;
        }
    } else {
        hw_timeofday += ts->seconds;
        tick_counter += ts->nanoseconds/ns_pr_tick;
        if (++tick_counter >= ticks_per_second) {
            tick_counter -= ticks_per_second;
            ++hw_timeofday;
        }
    }
    cyg_scheduler_unlock();
#else
    TOD_RC(vtss_ts_timeofday_set_delta(NULL, ts, negative));
#endif
}

// -------------------------------------------------------------------------
// vtss_tod_get_ts_cnt()
// Get the current time as a time counter
// Luton 28: get time from HAL layer with 1 us resolution.
// Jaguar  : get time from API layer (SW) with 4 ns resolution.
// Luton 26: get time from API layer (HW) with 4 ns resolution.
u32 vtss_tod_get_ts_cnt(void)
{
#if defined(VTSS_ARCH_LUTON28)
    cyg_uint32 clks;
    unsigned int my_ticks;
    // read hw counter and timer ticks as an atomic operation
    HAL_CLOCK_READ(&clks);
    my_ticks = tick_counter;
    if ((VTSS_INTR_STATUS & VTSS_F_STATUS_TIMER_0) &&
            (clks < 5000)) {
        ++my_ticks;
//      if (my_ticks > ticks_per_second) my_ticks = 0;
//      my_ticks |= 0x8000;
    }

    clks += (my_ticks*(1<<16));
    return clks;
#else
    vtss_timestamp_t ts;
    u32 tc;
    TOD_RC(vtss_ts_timeofday_get(NULL, &ts, &tc));
    return tc;
#endif
}

// -------------------------------------------------------------------------
// vtss_tod_ts_to_time()
// Convert a hw time counter to a timestamp, assuming that the hw time
// counter has not wrapped more than once
void vtss_tod_ts_to_time( u32 hw_time, vtss_timestamp_t *ts)
{
#if defined(VTSS_ARCH_LUTON28)
    uint clks;
    uint ticks;
    u32 now;
    ts->seconds = hw_timeofday;

    //if (hw_time & 0x80000000) {
    //  hw_time &= 0x7fffffff;
    //  T_WG(_C,"interrupt pending while reading hw_time = %lx",hw_time);
    //}
    now = vtss_tod_get_ts_cnt();
    ticks = (hw_time>>16) & 0x7fff;
    if (ticks > ((now>>16) & 0x7fff)) {
        --ts->seconds;
        T_NG(_C,"timer has wrapped hw_time = %X, now = %X",hw_time, now);
    }
    clks = hw_time & 0xffff;

    ts->nanoseconds = (ticks * 1000000)/((uint)ticks_per_second);
    ts->nanoseconds += (10000 * clks) / actual_period;
    ts->nanoseconds = 1000*ts->nanoseconds;
    T_NG(_C,"t_sec = %d,  t_nsec = %d",ts->seconds, ts->nanoseconds);

//    if (clks <= 2)
//    {
//        T_WG(_C,"HW timer may have wrapped ? cur_time = %lld,  cyg_time = %lld\n",cur_time, cyg_current_time());
//    }
#else
#if defined(VTSS_ARCH_JAGUAR_1)
    u32 tc;
    TOD_RC(vtss_ts_timeofday_get(NULL, ts, &tc));
    if (ts->nanoseconds < hw_time) {
        if (--ts->seconds == 0) --ts->sec_msb;
    }
    ts->nanoseconds = hw_time;
    T_DG(_C,"then: ts_sec = %d,  ts_nsec = %d, hw_time = %u",ts->seconds, ts->nanoseconds, hw_time);
#else
    u32 tc;
    u32 diff;
    TOD_RC(vtss_ts_timeofday_get(NULL, ts, &tc));
    /* add time counter difference */
    T_DG(_C,"now: ts_sec = %d,  ts_nsec = %d, tc = %u",ts->seconds, ts->nanoseconds, tc);
    diff = tc-hw_time;
    if (tc < hw_time) { /* time counter has wrapped */
        diff += VTSS_HW_TIME_WRAP_LIMIT;
        T_IG(_C,"counter wrapped: tc = %u,  hw_time = %u, diff = %u",tc, hw_time, diff);
    }
    /* clock rate offset adjustment */
    //if (adjust_divisor) {
    //    T_IG(_C,"diff before ns: %u", diff);
    //    diff += (i32)diff/adjust_divisor;
    //    T_IG(_C,"diff after ns: %u", diff);
    //}
    
    while (diff > VTSS_HW_TIME_CNT_PR_SEC) {
        --ts->seconds;
        diff -= VTSS_HW_TIME_CNT_PR_SEC;
        T_IG(_C,"big diff: tc = %u, hw_time = %u",tc, hw_time);
    }
    
    diff = diff*VTSS_HW_TIME_NSEC_PR_CNT;
    if (diff > ts->nanoseconds) {
        --ts->seconds;
        ts->nanoseconds += 1000000000L;
    }
    ts->nanoseconds -= diff;
        
    T_DG(_C,"then: ts_sec = %d,  ts_nsec = %d, hw_time = %u",ts->seconds, ts->nanoseconds, hw_time);
    
    return;
#endif
#endif
}

// -------------------------------------------------------------------------
// vtss_tod_set_adjtimer()
// Adjust the time rate in the HAL layer (for Luton28)
// adj = clockrate adjustment in 0,1 PPB.
void vtss_tod_set_adjtimer(i32 adj)
{
#if defined(VTSS_ARCH_LUTON28)
    actual_period = CYGNUM_HAL_RTC_PERIOD - adj/1000000;
    hal_clock_set_adjtimer(-adj/10000);
    T_IG(_C,"frequency adjustment: adj = %d actual_period = %d timer_adj(ppm) = %d)\n", adj, actual_period, -adj/10000);
#else
    if (VTSS_LABS(adj) < VTSS_HW_TIME_MIN_ADJ_RATE) adj = 0;
    TOD_RC(vtss_ts_adjtimer_set(NULL, adj));
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    vtss_phy_ts_scaled_ppb_t phy_adj = ((vtss_phy_ts_scaled_ppb_t)adj<<16)/10;
    TOD_RC(ptp_module_man_rateadj(&phy_adj));
#endif
    
#endif
}

// -------------------------------------------------------------------------
// vtss_tod_ts_cnt_sub()
// Calculate time difference (in time counter units) r = x-y
void vtss_tod_ts_cnt_sub(u32 *r, u32 x, u32 y)
{
#if defined(VTSS_ARCH_LUTON28)
    /* Ecos time counter = (10 ms ticks)<<16 + clocks */
    u16 xticks = (x>>16) & 0xfff;
    u16 yticks = (y>>16) & 0xfff;
    u16 xclk = x & 0xffff;
    u16 yclk = y & 0xffff;
    if (xticks < yticks) {xticks += ticks_per_second;} /* timer tick has wrapped */
    *r = xticks - yticks;
    if (xclk < yclk) {xclk += actual_period; --*r;} /* clocks part has wrapped */
    *r = (*r<<16) + xclk - yclk;
#else
    *r = x-y;
    if (x < y) {*r += VTSS_HW_TIME_WRAP_LIMIT;} /* time counter has wrapped */
#endif
}

// -------------------------------------------------------------------------
// vtss_tod_ts_cnt_add()
// Calculate time sum (in time counter units) r = x+y
void vtss_tod_ts_cnt_add(u32 *r, u32 x, u32 y)
{
#if defined(VTSS_ARCH_LUTON28)
    /* Ecos time counter = (10 ms ticks)<<16 + clocks */
    u16 xticks = (x>>16) & 0xfff;
    u16 yticks = (y>>16) & 0xfff;
    u16 xclk = x & 0xffff;
    u16 yclk = y & 0xffff;
    u16 rclk = xclk + yclk;
    *r = xticks + yticks;
    if (rclk >= actual_period) {rclk -= actual_period; ++*r;} /* clocks part has wrapped */
    if (*r >= ticks_per_second) {*r -= ticks_per_second;}
    *r = (*r<<16) + rclk;
#else
    *r = x+y;
    if (*r > VTSS_HW_TIME_WRAP_LIMIT) {*r -= VTSS_HW_TIME_WRAP_LIMIT;} /* time counter has wrapped */
#endif
}


// -------------------------------------------------------------------------
// vtss_tod_timeinterval_to_ts_cnt()
// Convert a timeInterval to time difference (in time counter units)
// It is assumed that the timeinterval is > 0 and < counter wrap around time
void vtss_tod_timeinterval_to_ts_cnt(u32 *r, vtss_timeinterval_t x)
{
    if (x < -VTSS_SEC_NS_INTERVAL(1,0) || x >= VTSS_SEC_NS_INTERVAL(1,0)) {
        T_WG(_C,"Time interval overflow ");
        *r = 0;
        return;
    }
#if defined(VTSS_ARCH_LUTON28)
    uint clks = ((VTSS_INTERVAL_US(x) % 10000) * actual_period + 5000)/10000;
    uint ticks = VTSS_INTERVAL_US(x) / 10000;
    *r = (ticks<<16) + clks;
#else
    if (x >= 0) {
        *r = VTSS_INTERVAL_NS(x)/VTSS_HW_TIME_NSEC_PR_CNT;
        //if (adjust_divisor) {
        //    *r -= *r/adjust_divisor;
        //}
    } else {
        *r = VTSS_HW_TIME_WRAP_LIMIT - VTSS_INTERVAL_NS(-x)/VTSS_HW_TIME_NSEC_PR_CNT;
    }
    T_NG(_C,"x %lld, r %d ", x>>16, *r);
    
#endif
}

// -------------------------------------------------------------------------
// vtss_tod_ts_cnt_to_timeinterval()
// Convert a time difference (in time counter units) to timeInterval
// it is assumed that time difference is < 1 sec.
void vtss_tod_ts_cnt_to_timeinterval(vtss_timeinterval_t *r, u32 x)
{
#if defined(VTSS_ARCH_LUTON28)
    u32 t;
    uint clks = x & 0xffff;
    uint ticks = (x>>16) & 0xffff;
    if (ticks >= 100) {
        T_EG(_C,"Time interval overflow ");
        *r = 0;
        return;
    }
    t = (ticks * 1000000)/((uint)ticks_per_second);
    t += (10000 * clks) / actual_period; /* t in us */
    t = 1000*t;
    *r = VTSS_SEC_NS_INTERVAL(0,t);
#else
    x = x*VTSS_HW_TIME_NSEC_PR_CNT;
    //if (adjust_divisor) {
    //    x += x/adjust_divisor;
    //}
    *r = VTSS_SEC_NS_INTERVAL(0,x);
#endif
}
// -------------------------------------------------------------------------
// vtss_tod_ns_to_ts_cnt(ns)
// Convert a nanosec value to time counter units
u32 vtss_tod_ns_to_ts_cnt(u32 ns)
{
#if defined(VTSS_ARCH_JAGUAR_1)
    return ns/VTSS_HW_TIME_NSEC_PR_CNT;
#else
#if defined (VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    vtss_timestamp_t ts;
    u32       tc;

    if (ns >= VTSS_ONE_MIA) {
       T_WG(_C, "Invalid ns value (%d)", ns);
    }
    vtss_tod_gettimeofday(&ts, &tc);
    if (ts.nanoseconds < ns) { // ns has wrapped
        ts.nanoseconds += VTSS_ONE_MIA;
    }
    return tc - ((ts.nanoseconds - ns) / VTSS_HW_TIME_NSEC_PR_CNT);
#else
    T_WG(_C,"Not supported in this architecture");
    return 0;
#endif
#endif
}

// -------------------------------------------------------------------------
// vtss_tod_ts_cnt_to_ns(ts)
// Convert time counter units to a nanosec value
u32 vtss_tod_ts_cnt_to_ns(u32 ts)
{
#ifndef VTSS_ARCH_LUTON28
    return ts*VTSS_HW_TIME_NSEC_PR_CNT;
#else
    T_WG(_C,"Not supported in this architecture");
    return 0;
#endif
}


char *vtss_tod_ns2str (u32 nanoseconds, char *str, char delim)
{
	/* nsec time stamp. Format: mmm,uuu,nnn */
	int m, u, n;
	char d[2];
	d[0] = delim;
	d[1] = 0;
	m = nanoseconds / 1000000;
	u = (nanoseconds / 1000) % 1000;
	n = nanoseconds % 1000;
	sprintf(str, "%03d%s%03d%s%03d", m, d, u, d, n);
	return str;
}


char *vtss_tod_TimeInterval_To_String (const vtss_timeinterval_t *t, char* str, char delim)
{
	vtss_timeinterval_t t1;
	char str1 [14];
	if (*t < 0) {
		t1 = -*t;
		str[0] = '-';
	} else {
		t1 = *t;
		str[0] = ' ';
	}
	sprintf(str+1, "%d.%s", VTSS_INTERVAL_SEC(t1), vtss_tod_ns2str (VTSS_INTERVAL_NS(t1), str1, delim));

	return str;
}

void vtss_tod_sub_TimeInterval(vtss_timeinterval_t *r, const vtss_timestamp_t *x, const vtss_timestamp_t *y)
{
    *r = (vtss_timeinterval_t)x->seconds + (((vtss_timeinterval_t)x->sec_msb)<<32) - 
                                 (vtss_timeinterval_t)y->seconds - (((vtss_timeinterval_t)y->sec_msb)<<32);
	*r = *r*1000000000LL;
	*r += ((vtss_timeinterval_t)x->nanoseconds - (vtss_timeinterval_t)y->nanoseconds);
	*r *= (1<<16);
}

static void timestamp_fix(vtss_timestamp_t *r, const vtss_timestamp_t *x)
{
	if ((i32)r->nanoseconds < 0) {
		r->seconds -= 1;
		r->nanoseconds += VTSS_ONE_MIA;
	}
	if (r->nanoseconds > VTSS_ONE_MIA) {
		r->seconds += 1;
		r->nanoseconds -= VTSS_ONE_MIA;
	}
    if (r->seconds < 70000 && x->seconds > (0xffffffffL -70000L)) ++r->sec_msb; /* sec counter has wrapped */
    if (x->seconds < 70000 && r->seconds > (0xffffffffL -70000L)) --r->sec_msb; /* sec counter has wrapped (negative) */
}

void vtss_tod_add_TimeInterval(vtss_timestamp_t *r, const vtss_timestamp_t *x, const vtss_timeinterval_t *y)
{
	r->seconds = x->seconds + VTSS_INTERVAL_SEC(*y);
	r->sec_msb = x->sec_msb;
	r->nanoseconds = x->nanoseconds + VTSS_INTERVAL_NS(*y);

	timestamp_fix(r, x);
}

void vtss_tod_sub_TimeStamp(vtss_timestamp_t *r, const vtss_timestamp_t *x, const vtss_timeinterval_t *y)
{
    vtss_timeinterval_t y_temp = -*y;
    vtss_tod_add_TimeInterval(r, x, &y_temp);
}

#if 0
void arithTest(void)
{
	vtss_timestamp_t r;
	vtss_timestamp_t x [] = {{0,123,4567},{0,123,4567},{0,123,900000123}, {1,4294967295LU,900000123}};
	vtss_timeinterval_t y [] = {4568*(1<<16),-4568*(1<<16),(vtss_timeinterval_t)100000000*(1<<16), (vtss_timeinterval_t)200000012*(1<<16) };
	vtss_timeinterval_t r1;
	vtss_timestamp_t x1 [] = {{0,123,4567},{0,123,4567},{0,123,900000123},{0,123,900000123},{0,123,900000119},{1,1,900000119},{0,4294967295LU,900000123}};
	vtss_timestamp_t y1 [] = {{0,122,4545},{0,124,4545},{0,122,900000125},{0,124,900000125},{0,123,900000123},{0,4294967295LU,900000123},{1,1,900000119}};
	vtss_timestamp_t r2;
	vtss_timestamp_t x2 [] = {{0,123,4567},{0,123,4567},{0,123,900000123}, {1,4294967295LU,900000123}};
	vtss_timeinterval_t y2 [] = {4568*(1<<16),4565*(1<<16),(vtss_timeinterval_t)100000000*(1<<16), (vtss_timeinterval_t)200000012*(1<<16) };
	uint i;
	char str1 [40];
	char str2 [40];
	char str3 [40];
	for (i=0; i < sizeof(y)/sizeof(vtss_timeinterval_t); i++) {
		vtss_tod_add_TimeInterval(&r,&x[i],&y[i]);
		T_I ("vtss_tod_add_TimeInterval: %s = %s + %s\n", TimeStampToString(&r,str1), TimeStampToString(&x[i],str2), vtss_tod_TimeInterval_To_String (&y[i],str3,','));
	}
	for (i=0; i < sizeof(y1)/sizeof(vtss_timestamp_t); i++) {
		vtss_tod_sub_TimeInterval(&r1,&x1[i],&y1[i]);
		T_I ("vtss_tod_sub_TimeInterval: %s = %s - %s\n", vtss_tod_TimeInterval_To_String(&r1,str1,','), TimeStampToString(&x1[i],str2), TimeStampToString (&y1[i],str3));
	}
	for (i=0; i < sizeof(y2)/sizeof(vtss_timeinterval_t); i++) {
		vtss_tod_sub_TimeStamp(&r2,&x2[i],&y2[i]);
		T_I ("subTimeStamp: %s = %s - %s\n", TimeStampToString(&r2,str1), TimeStampToString(&x2[i],str2), vtss_tod_TimeInterval_To_String (&y2[i],str3, ','));
	}
}
#endif
