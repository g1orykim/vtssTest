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
/* ptp_local_clock.c */

#include "vtss_ptp_local_clock.h"   /* define callouts from base part */
#include "ptp_local_clock.h"        /* define platform specific part */
#include "vtss_tod_api.h"
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT) && defined(CYGBLD_HAL_TIMEADJ_H)
#include CYGBLD_HAL_TIMEADJ_H
#endif
#include "ptp.h"
#include "ptp_api.h"
#include <sys/time.h>
#include "critd_api.h"
#include "port_custom_api.h"

#include "vtss_ptp_synce_api.h"

#define CLOCK_ADJ_SLEW_RATE 100000

#if defined(VTSS_ARCH_LUTON28)
/*
 * This is copied from led.h (including led.h give conflicts)
 **/
#include <cyg/hal/vcoreii.h>

// Unfortunately we need to bypass the API when setting the GPIO outputs
// because API calls must be called with VTSS_RCS(), which start by
// acquiring a semaphore, which is not necessarily available when we need
// it, because we may have been called from a function that already
// has the semaphore. So let's re-define the register addresses:
#define SYSTEM_GPIO (*((volatile unsigned long*)(VCOREII_SWC_REG(7, 0, 0x34)))) /* VCOREII_SWC_REG() is defined in ../eCos/packages/hal/arm/arm9/vcoreii/current/include/vcoreii.h */
#define SYSTEM_GPIO_F_DATA_VALUE_FPOS     0
#define SYSTEM_GPIO_F_OUTPUT_ENABLE_FPOS 16

#define SYSTEM_GPIOCTRL (*((volatile unsigned long*)(VCOREII_SWC_REG(7, 0, 0x33)))) /* VCOREII_SWC_REG() is defined in ../eCos/packages/hal/arm/arm9/vcoreii/current/include/vcoreii.h */
#define SYSTEM_GPIOCTRL_F_REGCTRL_FPOS   0

#define VTSS_SW_OPTION_PTP_GPIO
#define PTP_CLOCK_GPIO 12


/* The reference implementation - estax_34_ref */
static void ptp_clock_gpio_set(BOOL cl)
{
#ifdef VTSS_SW_OPTION_PTP_GPIO
    // The following piece of code must be protected from pre-emption, because it's a read-modify-write operation.
    // It doesn't help much to protect it with a local semaphore, since the API code may be called in between anyway.
    // As it is called from an ISR, no lock is needed here.
    ulong cur_gpio, mask;
    cur_gpio = SYSTEM_GPIO;
    mask      = VTSS_BIT((SYSTEM_GPIO_F_DATA_VALUE_FPOS + PTP_CLOCK_GPIO));
    cur_gpio &= ~mask; // Clear bits.
    cur_gpio |= (cl<<(SYSTEM_GPIO_F_DATA_VALUE_FPOS + PTP_CLOCK_GPIO)); // Set bits
    SYSTEM_GPIO = cur_gpio;
#endif
}



/* called from real time clock ISR */
/*lint -esym(459, vtss_hw_ptp_callout) */
static void vtss_hw_ptp_callout(void)
{
    int clock_pulse;
    /* update clock output */
    clock_pulse = vtss_tod_api_callout();
    if (clock_pulse == 1) {
        ptp_clock_gpio_set(1);
    }
    if (clock_pulse == 2) {
        ptp_clock_gpio_set(0);
    }
}
#endif /* VTSS_ARCH_LUTON28 */

/**
 * \brief internal clock management
 *
 * in a HW based timer, actual time = hw_time + ptp_offset
 * in a sw based timer, actual time = hw_time + drift + ptp_offset
 *                      drift = +(hw_time - t0)*ratio
 */
static struct {
    vtss_timeinterval_t        drift; /* accumulated drift relative to the HW clock */
    vtss_timestamp_t       t0;    /* Time of latest asjustment */
    vtss_timeinterval_t    ratio; /* actual clock drift ratio in pbb */
    Integer32 ptp_offset;  /* offset in seconds between hw clock and PTP clock */
    int             clock_option; /* clock adjustment option */
}   sw_clock_offset[PTP_CLOCK_INSTANCES];

static critd_t clockmutex;          /* clock internal data protection */

#define CLOCK_LOCK()        critd_enter(&clockmutex, _C, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define CLOCK_UNLOCK()      critd_exit (&clockmutex, _C, LOCK_TRACE_LEVEL, __FILE__, __LINE__)


#if defined(VTSS_ARCH_JAGUAR_1)
#include <cyg/io/i2c_vcoreiii.h>
#define CYG_I2C_VCXO_DEVICE CYG_I2C_VCOREIII_DEVICE

#define AD5667_I2C_ADDR 0x0f
#define MAX_5667_I2C_LEN 3
static const u16 init_dac = 0x9999;
#endif

static BOOL vtss_dac_clock_init_adjtimer(BOOL cold)
{
    BOOL ret_val = FALSE;
#if defined(VTSS_ARCH_JAGUAR_1)
    CYG_I2C_VCXO_DEVICE(ad_5667_device,AD5667_I2C_ADDR);
    u8 buf[MAX_5667_I2C_LEN];
    cyg_uint32 write_cnt;
     
    buf[0] = 0x38;  /*command: internal reference setup, reg addr */
    buf[1] = 0x00;  /*reg high val */
    buf[2] = 0x01;  /*reg low val */
    write_cnt = cyg_i2c_tx(&ad_5667_device,&buf[0],MAX_5667_I2C_LEN);
    buf[0] = 0x17;  /*command: write to input reg, reg addr both DAC's */
    buf[1] = init_dac>>8;  /*reg high val */
    buf[2] = init_dac & 0x0ff;  /*reg low val */
    write_cnt = cyg_i2c_tx(&ad_5667_device,&buf[0],MAX_5667_I2C_LEN);
    T_DG(_C,"write_cnt = %d", write_cnt);
    ret_val = (write_cnt == MAX_5667_I2C_LEN);
#endif
    return ret_val;
}
/*
 * dac value: 65535 ~ 2,5V, nominel freq ~1,5V, +/-1V ~+/- 5ppm =>
 *            adj = 0 => dac = 1,5 V  = 39321
 *            0,1 ppb = 0,1ppb/5ppm*1V => dac = (0,1/5000)*26214
 *            dac = 39321 + 26214/50000*adj
 */
static BOOL vtss_dac_clock_set_adjtimer(i32 adj)
{
    BOOL ret_val = FALSE;
#if defined(VTSS_ARCH_JAGUAR_1)
    u16 dac = init_dac;
    cyg_uint32 write_cnt;
    i32 temp;
    if (adj > 50000) adj = 50000;
    if (adj < -50000) adj = -50000;
    temp = (26214 * adj)/50000;
    dac += temp;
    CYG_I2C_VCXO_DEVICE(ad_5667_device,AD5667_I2C_ADDR);
    u8 buf[MAX_5667_I2C_LEN];
    buf[0] = 0x17;  /*command: write to input reg, reg addr both DAC's */
    buf[1] = dac>>8;  /*reg high val */
    buf[2] = dac & 0xff;  /*reg low val */
    write_cnt = cyg_i2c_tx(&ad_5667_device,&buf[0],MAX_5667_I2C_LEN);
    T_DG(_C,"write_cnt = %d, dac = %d", write_cnt, dac);
    ret_val = (write_cnt == MAX_5667_I2C_LEN);
#endif
    return ret_val;
}

static BOOL synce_module_present(void)
{
    BOOL ret_val = FALSE;
#if defined(VTSS_ARCH_JAGUAR_1)
    vtss_sgpio_port_data_t data[VTSS_SGPIO_PORTS];
    if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
        T_IG(_C,"Board PCB107");
        //sgpio port 31, bit 0 indicates if there is a syncE board. If one there is no SyncE board.         
        if (vtss_sgpio_read(NULL, 0, 0, data) == VTSS_RC_OK) {
            // Set Synce feature if there is a SyncE board present
            T_IG(_C,"Data 31 value 0 %d", data[31].value[0]);
            
            if (!data[31].value[0]) {
                ret_val = TRUE;
            }
        }
    }
#endif
    return ret_val;
}


void vtss_local_clock_initialize(int instance, init_synce_t *init_synce, BOOL cold, BOOL vcxo_enable)
{
    T_NG(_C,"instance %d", instance);
    if (instance == 0) {
        critd_init(&clockmutex, "clockmutex", VTSS_MODULE_ID_PTP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    } else {
        CLOCK_LOCK();
    }

    sw_clock_offset[instance].ptp_offset = 0;
    /* SW simulated clock */
    sw_clock_offset[instance].drift = 0;
    sw_clock_offset[instance].t0.seconds = 0;
    sw_clock_offset[instance].t0.nanoseconds = 0;
    sw_clock_offset[instance].ratio = 0;
    /*
     * HW initialization
     */
    if (instance == 0) {    /* only instance 0 is a HW clock */
#ifdef VTSS_SW_OPTION_PTP_GPIO
        ulong cur_reg_val;
        // Set the GPIO12 (Cable test A) to be output.
        cyg_scheduler_lock();
        cur_reg_val = SYSTEM_GPIO;
        cur_reg_val |=
            VTSS_BIT((SYSTEM_GPIO_F_OUTPUT_ENABLE_FPOS + PTP_CLOCK_GPIO));
        SYSTEM_GPIO = cur_reg_val;

        // Set the control register to gate GPIO.
        cur_reg_val = SYSTEM_GPIOCTRL;
        cur_reg_val |=
            VTSS_BIT((SYSTEM_GPIOCTRL_F_REGCTRL_FPOS + PTP_CLOCK_GPIO));
        SYSTEM_GPIOCTRL = cur_reg_val;
        cyg_scheduler_unlock();
#endif
#if defined(VTSS_ARCH_LUTON28)
        vtss_tod_set_ns_cnt_cb(vtss_tod_get_ts_cnt);
#endif
        sw_clock_offset[instance].clock_option = CLOCK_OPTION_INTERNAL_TIMER;
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
        if(vtss_board_type() == VTSS_BOARD_ESTAX_34_ENZO ||
           vtss_board_type() == VTSS_BOARD_ESTAX_34_ENZO_SFP) {
            if (vtss_synce_clock_init_adjtimer(init_synce, cold)) {
                hal_clock_enable_set_adjtimer(0, vtss_hw_ptp_callout);
                T_IG(_C,"SyncE Clock adjustment enabled");
                sw_clock_offset[instance].clock_option = CLOCK_OPTION_SYNCE_XO;
            } else {
                hal_clock_enable_set_adjtimer(1, vtss_hw_ptp_callout);
                T_IG(_C,"ENZO without SyncE Clock adjustment enabled");
            }
        } else {
            hal_clock_enable_set_adjtimer(1, vtss_hw_ptp_callout);
            T_IG(_C,"Internal Timer Clock adjustment enabled");
        }
#endif

        if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
            if (vtss_dac_clock_init_adjtimer(cold)) {
                T_DG(_C,"AD5667 DAC Clock adjustment enabled");
                if (FALSE == synce_module_present()) {
                    T_IG(_C,"Synce module is not present, i.e. use DAC for frequency control");
                    sw_clock_offset[instance].clock_option = CLOCK_OPTION_DAC;
                }
            }
        }
        
        if ((vtss_board_features() & VTSS_BOARD_FEATURE_VCXO) && vcxo_enable) {
            if (vtss_synce_clock_init_adjtimer(init_synce, cold)) {
                T_WG(_C,"Si570 Clock adjustment enabled");
                sw_clock_offset[instance].clock_option = CLOCK_OPTION_SYNCE_XO;
            }
        }
    } else {
        sw_clock_offset[instance].clock_option = CLOCK_OPTION_SOFTWARE;
    }
    CLOCK_UNLOCK();
}

void vtss_local_clock_time_get(vtss_timestamp_t *t, int instance, u32 *hw_time)
{
    vtss_tod_gettimeofday(t, hw_time);
    if (instance != 0) {
        vtss_timeinterval_t deltat;
        CLOCK_LOCK();
        vtss_tod_sub_TimeInterval(&deltat, t, &sw_clock_offset[instance].t0);
        deltat = deltat/(1<<16); /* normalize to ns */
        deltat = (deltat*sw_clock_offset[instance].ratio);
        deltat = (deltat/VTSS_ONE_MIA)<<16;
        vtss_tod_add_TimeInterval(t, t, &deltat);
        vtss_tod_add_TimeInterval(t, t, &sw_clock_offset[instance].drift);
        t->seconds += sw_clock_offset[instance].ptp_offset;
        CLOCK_UNLOCK();
    }
}


void vtss_local_clock_time_set(vtss_timestamp_t *t, int instance)
{
    char buf [40];
    vtss_timestamp_t thw = {0,0,0};
    vtss_timestamp_t my_time;
    Integer32 ptp_offset;
    u32 tc;

    if (instance == 0) {
        T_IG(_C,"Set timeofday %s",TimeStampToString(t,buf));
        vtss_tod_settimeofday(t);
    } else {
        vtss_tod_gettimeofday(&thw, &tc);
        ptp_offset = t->seconds - thw.seconds;
        //if (t->nanoseconds > (500000000 + thw.nanoseconds)) {
        //    ++ptp_offset;
        //    T_IG(_C,"increment ptp_offset");
        //} else if (thw.nanoseconds > (500000000 + t->nanoseconds)) {
        //    --ptp_offset;
        //    T_IG(_C,"decrement ptp_offset");
        //}
        CLOCK_LOCK();
        sw_clock_offset[instance].t0 = *t;
        sw_clock_offset[instance].drift = ((vtss_timeinterval_t)(t->nanoseconds) - (vtss_timeinterval_t)(thw.nanoseconds))<<16;
        vtss_tod_add_TimeInterval(&my_time, &thw, &sw_clock_offset[instance].drift);
        my_time.seconds += ptp_offset;

        T_IG(_C,"drift: %s",vtss_tod_TimeInterval_To_String(&sw_clock_offset[instance].drift,buf,0));
        T_IG(_C,"his time:t_sec = %d,  t_nsec = %d, ptp_offset = %d",t->seconds, t->nanoseconds, ptp_offset);
        T_IG(_C,"my time :t_sec = %d,  t_nsec = %d",my_time.seconds, my_time.nanoseconds);
        sw_clock_offset[instance].ptp_offset = ptp_offset;
        CLOCK_UNLOCK();
    }
}


void vtss_local_clock_convert_to_time( UInteger32 cur_time, vtss_timestamp_t *t, int instance)
{
    char buf1 [20];
    char buf2 [20];
    vtss_tod_ts_to_time( cur_time, t);

    CLOCK_LOCK();
    if (instance == 0) {
        t->seconds += sw_clock_offset[instance].ptp_offset;
    } else {
        vtss_timeinterval_t deltat;
        vtss_tod_sub_TimeInterval(&deltat, t, &sw_clock_offset[instance].t0);
        deltat = deltat/(1<<16); /* normalize to ns */
        deltat = (deltat*sw_clock_offset[instance].ratio);
        deltat = (deltat/VTSS_ONE_MIA)<<16;
        vtss_tod_add_TimeInterval(t, t, &deltat);
        vtss_tod_add_TimeInterval(t, t, &sw_clock_offset[instance].drift);
        t->seconds += sw_clock_offset[instance].ptp_offset;
        T_IG(_C,"instance = %d,  deltat = %s,drift = %s",instance, vtss_tod_TimeInterval_To_String(&deltat,buf1,0), 
             vtss_tod_TimeInterval_To_String(&sw_clock_offset[instance].drift,buf2,0));


    }

    T_DG(_C,"t_sec = %d,  t_nsec = %d, ptp_offset = %d",t->seconds, t->nanoseconds, sw_clock_offset[instance].ptp_offset);
    CLOCK_UNLOCK();
}

void vtss_local_clock_convert_to_hw_tc( UInteger32 ns, UInteger32 *cur_time)
{
    *cur_time = vtss_tod_ns_to_ts_cnt(ns);
    T_NG(_C,"ns = %u,  cur_time = %u",ns, *cur_time);
}

static Integer32 actual_adj = 0;
void vtss_local_clock_ratio_set(Integer32 adj, int instance)
{
    char buf1 [20];
    char buf2 [20];
    //vtss_ptp__clock_mode_t my_new_mode;
    if (adj > ADJ_FREQ_MAX*10)
        adj = ADJ_FREQ_MAX*10;
    else if (adj < -ADJ_FREQ_MAX*10)
        adj = -ADJ_FREQ_MAX*10;
    CLOCK_LOCK();
    if (instance == 0) {
        if (adj != actual_adj) {
            T_DG(_C,"before: adj %d, actual_adj %d", adj, actual_adj);
            if (adj  > actual_adj + CLOCK_ADJ_SLEW_RATE) adj = actual_adj + CLOCK_ADJ_SLEW_RATE;
            if (adj  < actual_adj - CLOCK_ADJ_SLEW_RATE) adj = actual_adj - CLOCK_ADJ_SLEW_RATE;
            T_DG(_C,"after: adj %d", adj);
            if (sw_clock_offset[instance].clock_option == CLOCK_OPTION_INTERNAL_TIMER) {
        /* PTP Module should not lock here for the adjtimer if remote_ts is enabled */



                vtss_tod_set_adjtimer(adj);



            } else if (sw_clock_offset[instance].clock_option == CLOCK_OPTION_SYNCE_XO &&
                       (vtss_board_features() & VTSS_BOARD_FEATURE_VCXO)) {
                // Do frequency adjustment in SYNCE/Si570 module.
                vtss_synce_clock_set_adjtimer(adj);
            } else if (sw_clock_offset[instance].clock_option == CLOCK_OPTION_DAC) {
                if (vtss_dac_clock_set_adjtimer(adj)) {
                    T_DG(_C,"AD5667 DAC Clock adjustment");
                }
            } else {
                T_WG(_C,"undefined clock adj method %d", sw_clock_offset[instance].clock_option);
            }

            T_IG(_C,"frequency adjustment: adj = %d", adj);
            actual_adj = adj;
        }

    } else {
        vtss_timeinterval_t deltat;
        vtss_timestamp_t t = {0,0,0};
        u32 tc;
        vtss_tod_gettimeofday(&t, &tc);
        vtss_tod_sub_TimeInterval(&deltat, &t, &sw_clock_offset[instance].t0);
        T_NG(_C,"deltat = %s, ratio %lld",vtss_tod_TimeInterval_To_String(&deltat,buf1,0), sw_clock_offset[instance].ratio);
        deltat = deltat/(1<<16); /* normalize to ns */
        deltat = (deltat*sw_clock_offset[instance].ratio);
        T_NG(_C,"deltat = %lld",deltat);
        deltat = (deltat/VTSS_ONE_MIA)<<16;
        sw_clock_offset[instance].drift += deltat;

        if (sw_clock_offset[instance].drift > ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16) {
            ++sw_clock_offset[instance].ptp_offset;
            sw_clock_offset[instance].drift -= ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16;
            T_IG(_C,"drift adjusted = %s ptp_offset = %d",vtss_tod_TimeInterval_To_String(&deltat,buf1,0), sw_clock_offset[instance].ptp_offset);
        } else if (deltat < (-((vtss_timeinterval_t)VTSS_ONE_MIA))<<16) {
            --sw_clock_offset[instance].ptp_offset;
            sw_clock_offset[instance].drift += ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16;
            T_IG(_C,"drift adjusted = %s, ptp_offset = %d",vtss_tod_TimeInterval_To_String(&deltat,buf1,0), sw_clock_offset[instance].ptp_offset);
        }
        sw_clock_offset[instance].t0 = t;
        sw_clock_offset[instance].ratio = adj/10;
        T_DG(_C,"adj = %d,  deltat = %s, drift = %s",adj,
             vtss_tod_TimeInterval_To_String(&deltat,buf1,0), vtss_tod_TimeInterval_To_String(&sw_clock_offset[instance].drift,buf2,0));
    }
    CLOCK_UNLOCK();
}

void vtss_local_clock_ratio_clear(int instance)
{
    char buf1 [20];
    char buf2 [20];
    Integer32 adj = 0;

    CLOCK_LOCK();
    if (instance == 0) {
        if (adj != actual_adj) {
            if (sw_clock_offset[instance].clock_option == CLOCK_OPTION_INTERNAL_TIMER) {
                /* PTP Module should not lock here for the adjtimer if remote_ts is enabled */



                vtss_tod_set_adjtimer(adj);



            } else if (sw_clock_offset[instance].clock_option == CLOCK_OPTION_SYNCE_XO &&
                       (vtss_board_features() & VTSS_BOARD_FEATURE_VCXO)) {
                // Do frequency adjustment in SYNCE/Si570 module.
                vtss_synce_clock_set_adjtimer(adj);
            } else if (sw_clock_offset[instance].clock_option == CLOCK_OPTION_DAC) {
                if (vtss_dac_clock_set_adjtimer(adj)) {
                    T_DG(_C,"AD5667 DAC Clock adjustment");
                }
            } else {
                T_WG(_C,"undefined clock adj method %d", sw_clock_offset[instance].clock_option);
            }

            T_IG(_C,"frequency adjustment: adj = %d", adj);
            actual_adj = adj;
        }

    } else {
        vtss_timeinterval_t deltat;
        vtss_timestamp_t t = {0,0,0};
        u32 tc;
        vtss_tod_gettimeofday(&t, &tc);
        vtss_tod_sub_TimeInterval(&deltat, &t, &sw_clock_offset[instance].t0);
        T_NG(_C,"deltat = %s, ratio %lld",vtss_tod_TimeInterval_To_String(&deltat,buf1,0), sw_clock_offset[instance].ratio);
        deltat = deltat/(1<<16); /* normalize to ns */
        deltat = (deltat*sw_clock_offset[instance].ratio);
        T_NG(_C,"deltat = %lld",deltat);
        deltat = (deltat/VTSS_ONE_MIA)<<16;
        sw_clock_offset[instance].drift += deltat;

        if (sw_clock_offset[instance].drift > ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16) {
            ++sw_clock_offset[instance].ptp_offset;
            sw_clock_offset[instance].drift -= ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16;
            T_IG(_C,"drift adjusted = %s ptp_offset = %d",vtss_tod_TimeInterval_To_String(&deltat,buf1,0), sw_clock_offset[instance].ptp_offset);
        } else if (deltat < (-((vtss_timeinterval_t)VTSS_ONE_MIA))<<16) {
            --sw_clock_offset[instance].ptp_offset;
            sw_clock_offset[instance].drift += ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16;
            T_IG(_C,"drift adjusted = %s, ptp_offset = %d",vtss_tod_TimeInterval_To_String(&deltat,buf1,0), sw_clock_offset[instance].ptp_offset);
        }
        sw_clock_offset[instance].t0 = t;
        sw_clock_offset[instance].ratio = adj/10;
        T_DG(_C,"adj = %d,  deltat = %s, drift = %s",adj,
             vtss_tod_TimeInterval_To_String(&deltat,buf1,0), vtss_tod_TimeInterval_To_String(&sw_clock_offset[instance].drift,buf2,0));
    }
    CLOCK_UNLOCK();
}

void vtss_local_clock_adj_offset(Integer32 offset, int instance)
{
    CLOCK_LOCK();
    if (instance == 0) {
        if (offset != 0) {
            if (sw_clock_offset[instance].clock_option == CLOCK_OPTION_INTERNAL_TIMER ||
                    sw_clock_offset[instance].clock_option == CLOCK_OPTION_DAC ||
                    (sw_clock_offset[instance].clock_option == CLOCK_OPTION_SYNCE_XO &&
                     (vtss_board_features() & VTSS_BOARD_FEATURE_VCXO))) {
#if defined(VTSS_ARCH_LUTON28)
                T_IG(_C,"currently not supported in L28");
#else
                PTP_RC(vtss_ts_timeofday_offset_set(NULL, offset));
                T_IG(_C,"clock offset adj %d", offset);
#endif
            } else {
                T_WG(_C,"undefined clock adj method %d", sw_clock_offset[instance].clock_option);
            }
        }

    } else {
        T_IG(_C,"offset adjustment only supported for instance 0");
        sw_clock_offset[instance].drift -= offset;
        
        if (sw_clock_offset[instance].drift > ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16) {
            ++sw_clock_offset[instance].ptp_offset;
            sw_clock_offset[instance].drift -= ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16;
            T_IG(_C,"drift adjusted = %d ptp_offset = %d",offset, sw_clock_offset[instance].ptp_offset);
        } else if (sw_clock_offset[instance].drift < ((vtss_timeinterval_t)0)) {
            --sw_clock_offset[instance].ptp_offset;
            sw_clock_offset[instance].drift += ((vtss_timeinterval_t)VTSS_ONE_MIA)<<16;
            T_IG(_C,"drift adjusted = %d, ptp_offset = %d",offset, sw_clock_offset[instance].ptp_offset);
        }
    }
    CLOCK_UNLOCK();
}

int vtss_ptp_adjustment_method(int instance)
{
    int option;
    CLOCK_LOCK();
    option = sw_clock_offset[instance].clock_option;
    CLOCK_UNLOCK();
    return option;
}

