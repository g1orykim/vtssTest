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

/**
 * \file
 * \brief TimeStamping API.
 * \details This header file describes PTP/OAM TimeStamping API functions and
 * associated types.
 */

#ifndef _VTSS_TS_API_H_
#define _VTSS_TS_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

// ***************************************************************************
//
//  TimeStamping interface
//
// ***************************************************************************

#if defined(VTSS_FEATURE_TIMESTAMP)

/** \brief Number of clock cycle counts pr sec. */
#if defined (VTSS_ARCH_SERVAL)
#define VTSS_HW_TIME_CNT_PR_SEC 1000000000 /* Serval counts ns instead of clock cycles */
/** \brief Number of nanoseconds pr clock count. */
#define VTSS_HW_TIME_NSEC_PR_CNT 1
#endif
#if defined (VTSS_ARCH_LUTON26)
#define VTSS_HW_TIME_CNT_PR_SEC 250000000 /* L26 counts clock cycles instead of ns */
/** \brief Number of nanoseconds pr clock count. */
#define VTSS_HW_TIME_NSEC_PR_CNT 4
#endif
#if defined (VTSS_ARCH_JAGUAR_1)
#define VTSS_HW_TIME_CNT_PR_SEC 1000000000
/** \brief Number of nanoseconds pr clock count. */
#define VTSS_HW_TIME_NSEC_PR_CNT 1
#endif
#if defined (VTSS_ARCH_JAGUAR_1)
/** \brief Jaguar nanosecond time counter wrap around value (jaguar time counter wraps each second). */
#define VTSS_HW_TIME_WRAP_LIMIT  VTSS_HW_TIME_CNT_PR_SEC       /* time counter wrap around limit+1 */
#endif
#if defined (VTSS_ARCH_LUTON26) || defined (VTSS_ARCH_SERVAL)
/** \brief Caracal nanosecond time counter wrap around value (Caracal time counter wraps when 0xffffffff is reached). */
#define VTSS_HW_TIME_WRAP_LIMIT  0       /* time counter wrap around limit+1 (=0 if wrap at 0xffffffff) */
#endif

#if defined (VTSS_ARCH_LUTON26) || defined (VTSS_ARCH_JAGUAR_1)
/** \brief Jaguar/Luton26 minimum adjustment rate in units of 0,1 ppb. */
#define VTSS_HW_TIME_MIN_ADJ_RATE  40       /* 4 ppb */
#endif
#if defined (VTSS_ARCH_SERVAL)
/** \brief Serval minimum adjustment rate in units of 0,1 ppb. */
#define VTSS_HW_TIME_MIN_ADJ_RATE  10       /* 1 ppb */
#endif

/**
 * \brief Set the current time in a Timestamp format.
 * \param inst [IN]     handle to an API instance.
 * \param ts [IN]       pointer to a TimeStamp structure.
 *
 */
vtss_rc vtss_ts_timeofday_set(const vtss_inst_t             inst,
                              const vtss_timestamp_t               *const ts);

/**
 * \brief Set delta the current time in a Timestamp format.
 * \param inst [IN]     handle to an API instance.
 * \param ts [IN]       pointer to a TimeStamp structure.
 * \param negative [IN] True if ts is subtracted from current time, else ts is added.
 *
 */
vtss_rc vtss_ts_timeofday_set_delta(const vtss_inst_t       inst,
                                    const vtss_timestamp_t         *ts,
                                    BOOL                      negative);

/**
 * \brief Subtract offset from the current time.
 * \param inst [IN]     handle to an API instance.
 * \param offset [IN]   offset in ns.
 *
 */
vtss_rc vtss_ts_timeofday_offset_set(const vtss_inst_t          inst,
                                     const i32                  offset);

/**
 * \brief Do the one sec administration in the Timestamp function.
 * \param inst [IN]     handle to an API instance.
 * \param ongoing_adjustment [OUT]  True if clock adjustment is ongoing
 *
 */
vtss_rc vtss_ts_adjtimer_one_sec(const vtss_inst_t             inst,
                                 BOOL                           *const ongoing_adjustment);

/**
 * \brief Check if the clock adjustment is ongoing.
 * \param inst [IN]     handle to an API instance.
 * \param ongoing_adjustment [OUT]  True if clock adjustment is ongoing
 *
 */
vtss_rc vtss_ts_ongoing_adjustment(const vtss_inst_t           inst,
                                   BOOL                        *const ongoing_adjustment);

/**
 * \brief Get the current time in a Timestamp format, and the corresponding
 *        time counter.
 * \param inst [IN]     handle to an API instance
 * \param ts [OUT]      pointer to a TimeStamp structure
 * \param tc [OUT]      pointer to a time counter (internal hw format)
 *                      Jaguar: tc = nanoseconds/nanosec_pr_clock_cycle (0..249999999)
 *                      Caracal:tc = free running clock cycle counter
 *                      Serval: tc = (nanoseconds + seconds*10**9) mod 2**32
 */
vtss_rc vtss_ts_timeofday_get(const vtss_inst_t             inst,
                              vtss_timestamp_t                     *const ts,
                              u32                           *const tc);

/**
 * \brief Get the time at the next 1PPS pulse edge in a Timestamp format.
 * \param inst [IN]     handle to an API instance
 * \param ts [OUT]      pointer to a TimeStamp structure
 */
vtss_rc vtss_ts_timeofday_next_pps_get(const vtss_inst_t             inst,
                              vtss_timestamp_t                     *const ts);

/**
 * \brief Adjust the clock timer ratio.
 * \param inst [IN]     handle to an API instance.
 * \param adj [IN]      Clock ratio frequency offset in units of 0,1 ppb (parts pr billion).
 *                      ratio > 0 => clock runs faster
 */
vtss_rc vtss_ts_adjtimer_set(const vtss_inst_t              inst,
                             const i32                      adj);

/**
* \brief get the clock timer ratio.
*
* \param inst [IN]     handle to an API instance.
* \param adj [OUT]     Clock ratio frequency offset in ppb (parts pr billion).
*                      ratio > 0 => clock runs faster
*
*/
vtss_rc vtss_ts_adjtimer_get(const vtss_inst_t              inst,
                             i32                            *const adj);

/**
* \brief get the clock internal timer frequency offset, compared to external clock input.
*
* \param inst [IN]     handle to an API instance.
* \param adj [OUT]     Clock ratio frequency offset in ppb (parts pr billion).
*                      ratio > 0 => internal clock runs faster than external clock
*
*/
vtss_rc vtss_ts_freq_offset_get(const vtss_inst_t           inst,
                             i32                            *const adj);

#if defined(VTSS_ARCH_SERVAL)
/**
 * \brief parameter for setting the alternative  clock mode.
 */
/** \brief external clock output configuration. */
typedef struct vtss_ts_alt_clock_mode_t {
    BOOL one_pps_out;       /**< Enable 1pps output */
    BOOL one_pps_in;        /**< Enable 1pps input */
    BOOL save;              /**< Save actual time counter at next 1 PPS input */
    BOOL load;              /**< Load actual time counter with at next 1 PPS input */
} vtss_ts_alt_clock_mode_t;

/**
 * \brief Get the latest saved nanosec counter from the alternative clock.
 *
 * \param inst [IN]             handle to an API instance
 * \param saved [OUT]           latest saved value.
 */
vtss_rc vtss_ts_alt_clock_saved_get(
                                const vtss_inst_t           inst,
                                u32    *const               saved);

/**
 * \brief Get the alternative external clock mode.
 *
 * \param inst [IN]             handle to an API instance
 * \param alt_clock_mode [OUT]  alternative clock mode.
 */
vtss_rc vtss_ts_alt_clock_mode_get(
                                const vtss_inst_t              inst,
                                vtss_ts_alt_clock_mode_t       *const alt_clock_mode);

/**
 * \brief Set the alternative external clock mode.
 *  This function configures the 1PPS, L/S pin usage for pin set no 0 in Serval
 *
 * \param inst [IN]             handle to an API instance
 * \param alt_clock_mode [IN]   alternative clock mode.
 */
vtss_rc vtss_ts_alt_clock_mode_set(
                                const vtss_inst_t              inst,
                                const vtss_ts_alt_clock_mode_t *const alt_clock_mode);

/**
 * \brief Set the time at the next 1PPS pulse edge in a Timestamp format.
 * \param inst [IN]     handle to an API instance
 * \param ts [OUT]      pointer to a TimeStamp structure
 */
vtss_rc vtss_ts_timeofday_next_pps_set(const vtss_inst_t       inst,
                                const vtss_timestamp_t         *const ts);
#endif

/**
 * \brief parameter for setting the external clock mode.
 */
typedef enum  {
    TS_EXT_CLOCK_MODE_ONE_PPS_DISABLE,
    TS_EXT_CLOCK_MODE_ONE_PPS_OUTPUT,
    TS_EXT_CLOCK_MODE_ONE_PPS_INPUT,
    TS_EXT_CLOCK_MODE_MAX
} vtss_ts_ext_clock_one_pps_mode_t;

/** \brief external clock output configuration. */
typedef struct vtss_ts_ext_clock_mode_t {
    vtss_ts_ext_clock_one_pps_mode_t   one_pps_mode;    
                            /**< Select 1pps ext clock mode:
                               input :  lock clock to 1pps input
                               output:  enable external sync pulse output
                               disable: disable 1 pps */
    BOOL enable;            /**< Select internal sync pulse (enable = false) 
                                or external sync pulse (enable = true) */
    u32  freq;              /**< clock output frequency (hz [1..25.000.000]). */
} vtss_ts_ext_clock_mode_t;


/**
 * \brief Get the external clock mode.
 *  The mode depends on the hardware capability, it may be:
 *          Enable/disable external synch pulse
 *          Set clock output frequency
 *
 * \param inst [IN]             handle to an API instance
 * \param ext_clock_mode [OUT]  external clock mode.
 *
 */
vtss_rc vtss_ts_external_clock_mode_get(
                                const vtss_inst_t           inst,
                                vtss_ts_ext_clock_mode_t    *const ext_clock_mode);

/**
 * \brief Set the external clock mode.
 *  The mode depends on the hardware capability, it may be:
 *          Enable/disable external synch pulse
 *          Set clock output frequency
 *
 * \param inst [IN]             handle to an API instance
 * \param ext_clock_mode [IN]   external clock mode.
 *
 */
vtss_rc vtss_ts_external_clock_mode_set(
                                const vtss_inst_t              inst,
                                const vtss_ts_ext_clock_mode_t *const ext_clock_mode);
/**
 * \brief Set the ingress latency.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param ingress_latency [IN]  pointer to ingress latency
 *
 */
vtss_rc vtss_ts_ingress_latency_set(const vtss_inst_t              inst,
                                    const vtss_port_no_t           port_no,
                                    const vtss_timeinterval_t             *const ingress_latency);

/**
 * \brief Get the ingress latency.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param ingress_latency [OUT] pointer to ingress_latency
 */
vtss_rc vtss_ts_ingress_latency_get(const vtss_inst_t              inst,
                                    const vtss_port_no_t           port_no,
                                    vtss_timeinterval_t                   *const ingress_latency);

/**
 * \brief Set the P2P delay.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param p2p_delay [IN]        peer-2-peer delay (measured)
 *
 */
vtss_rc vtss_ts_p2p_delay_set(const vtss_inst_t                   inst,
                              const vtss_port_no_t                port_no,
                              const vtss_timeinterval_t                  *const p2p_delay);

/**
 * \brief Get the P2P delay.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param p2p_delay [OUT]       pointer to peer-2-peer delay
 */
vtss_rc vtss_ts_p2p_delay_get(const vtss_inst_t              inst,
                              const vtss_port_no_t           port_no,
                              vtss_timeinterval_t                   *const p2p_delay);


/**
 * \brief Set the egress latency.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param egress_latency [IN]   egress latency
 *
 */
vtss_rc vtss_ts_egress_latency_set(const vtss_inst_t            inst,
                                   const vtss_port_no_t         port_no,
                                   const vtss_timeinterval_t    *const egress_latency);

/**
 * \brief Get the egress latency.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param egress_latency [OUT]  pointer to  egress latency
 */
vtss_rc vtss_ts_egress_latency_get(const vtss_inst_t            inst,
                                   const vtss_port_no_t         port_no,
                                   vtss_timeinterval_t          *const egress_latency);

/**
 * \brief Set the delay asymmetry.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param delay_asymmetry [IN]  delay asymmetry
 *
 */
vtss_rc vtss_ts_delay_asymmetry_set(const vtss_inst_t           inst,
                                   const vtss_port_no_t         port_no,
                                   const vtss_timeinterval_t    *const delay_asymmetry);

/**
 * \brief Get the delay asymmetry.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param delay_asymmetry [OUT] pointer to  delay asymmetry
 */
vtss_rc vtss_ts_delay_asymmetry_get(const vtss_inst_t           inst,
                                   const vtss_port_no_t         port_no,
                                   vtss_timeinterval_t          *const delay_asymmetry);

/**
 * \brief parameter for setting the timestamp operating mode
 */
typedef enum  {
    TS_MODE_NONE,
    TS_MODE_EXTERNAL,
    TS_MODE_INTERNAL,
    TX_MODE_MAX
} vtss_ts_mode_t;

/** \brief Timestamp operation */
typedef struct vtss_ts_operation_mode_t {
    vtss_ts_mode_t mode;                /**< Hardware Timestamping mode for a port(EXTERNAL or INTERNAL) */
} vtss_ts_operation_mode_t;


/**
 * \brief Set the timestamping operation mode for a port.
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param mode [IN]             pointer to a struct holding the operation mode
 *
 * Serval: Used to set backplane (INTERNAL) mode/normal(EXTERNAL) mode
 * Other : Not used
 */
vtss_rc vtss_ts_operation_mode_set(const vtss_inst_t              inst,
                                   const vtss_port_no_t           port_no,
                                   const vtss_ts_operation_mode_t *const mode);

/**
 * \brief Get the timestamping operation mode for a port
 * \param inst [IN]             handle to an API instance
 * \param port_no [IN]          port number
 * \param mode [OUT]            pointer to a struct holding the operation mode
 *
 */
vtss_rc vtss_ts_operation_mode_get(const vtss_inst_t              inst,
                                   const vtss_port_no_t           port_no,
                                   vtss_ts_operation_mode_t       *const mode);
                                   
/**
 * \brief parameter for setting the internal timestamp format
 */
typedef enum  {
    TS_INTERNAL_FMT_NONE,
    TS_INTERNAL_FMT_RESERVED_LEN_30BIT,        /* Ts is stored in reservedField as 30 bit (ns only) */
    TS_INTERNAL_FMT_RESERVED_LEN_32BIT,        /* Ts is stored in reservedField as 32 bit (ns+sec*10^9) mod 2^32) */
    TS_INTERNAL_FMT_SUB_ADD_LEN_44BIT_CF62,    /* Ts is subtracted from cf at ingress as 44 bit (ns+sec*10^9) mod 2^44), MSB is saved in cf bit 62 */
    TS_INTERNAL_FMT_RESERVED_LEN_48BIT_CF_3_0, /* Ts is subtracted from cf at ingress as 48 bit (ns+sec*10^9) mod 2^48), MSBs are saved in cf bit 3-0 */
    TS_INTERNAL_FMT_RESERVED_LEN_48BIT_CF_0,   /* Ts is subtracted from cf at ingress as 48 bit (ns+sec*10^9) mod 2^48), MSB is saved in cf bit 0 */
    TX_INTERNAL_FMT_MAX
} vtss_ts_internal_fmt_t;

/** \brief Hardware timestamping format mode for internal ports */
typedef struct vtss_ts_internal_mode_t {
    vtss_ts_internal_fmt_t int_fmt;    /**< Hardware Timestamping format mode for INTERNAL ports */
} vtss_ts_internal_mode_t;


/**
 * \brief Set the internal timestamping mode.
 * \param inst [IN]             handle to an API instance
 * \param mode [IN]             pointer to a struct holding the operation mode
 *
 * Serval: Used to set INTERNAL mode timestamping format
 * Other : Not used
 */
vtss_rc vtss_ts_internal_mode_set(const vtss_inst_t              inst,
                                   const vtss_ts_internal_mode_t *const mode);

/**
 * \brief Get the internal timestamping mode.
 * \param inst [IN]             handle to an API instance
 * \param mode [OUT]            pointer to a struct holding the operation mode
 */
vtss_rc vtss_ts_internal_mode_get(const vtss_inst_t              inst,
                                   vtss_ts_internal_mode_t       *const mode);

/** \brief Timestamp identifier */
typedef struct vtss_ts_id_t {
    u32                 ts_id;  /**< Timestamp identifier */
} vtss_ts_id_t;

/** \brief Timestamp structure */
typedef struct vtss_ts_timestamp_t {
    u32 ts;                     /**< Timestamp value */
    u32 id;                     /**< Timestamp identifier */
    void * context;             /**< Application specific context */
    BOOL ts_valid;              /**< Timestamp is valid (can be not valid if timestamp is not received */
} vtss_ts_timestamp_t;

/**
 * \brief Update the internal timestamp table, from HW
 * \param inst    [IN]          handle to an API instance
 *
 */
vtss_rc vtss_tx_timestamp_update(const vtss_inst_t              inst);

/**
* \brief Get the rx FIFO timestamp for a {timestampId}
* \param inst    [IN]          handle to an API instance
* \param ts_id   [IN]          timestamp id
* \param ts     [OUT]          pointer to a struct holding the fifo timestamp
*
*/
vtss_rc vtss_rx_timestamp_get(const vtss_inst_t              inst,
                              const vtss_ts_id_t             *const ts_id,
                              vtss_ts_timestamp_t            *const ts);

/**
* \brief Release the FIFO rx timestamp id 
* \param inst    [IN]          handle to an API instance
* \param ts_id   [IN]          timestamp id
*
*/
vtss_rc vtss_rx_timestamp_id_release(const vtss_inst_t              inst,
                              const vtss_ts_id_t             *const ts_id);

/**
* \brief Get rx timestamp from a port (convert from slave time to the master time)
* \param inst    [IN]          handle to an API instance
* \param port_no [IN]          port number
* \param ts     [IN/OUT]       pointer to a struct holding the timestamp
*
*/
vtss_rc vtss_rx_master_timestamp_get(const vtss_inst_t              inst,
                                     const vtss_port_no_t           port_no,
                                     vtss_ts_timestamp_t            *const ts);

#if defined (VTSS_ARCH_SERVAL_CE)
/**
 * \brief parameter for requesting an oam timestamp
 */
typedef struct vtss_oam_ts_id_s {
    u32                voe_id;  /**< VOE instance (Timestamp) identifier */
    u32                voe_sq;  /**< VOE (Timestamp) sequence no */
} vtss_oam_ts_id_t;

/**
 * \brief parameter for returning an oam timestamp
 */
typedef struct vtss_oam_ts_timestamp_s {
    u32             ts;         /**< Timestamp value (ns + sec*10^9) mod 2^32 */
    vtss_port_no_t  port_no;    /**< port number */
    BOOL            ts_valid;   /**< Timestamp is valid (can be not valid if no timestamp is received for the requested {voe_id, voe_sq} */
} vtss_oam_ts_timestamp_t;

/**
 * \brief Get oam timestamp
 * \param inst    [IN]          handle to an API instance
 * \param id      [IN]          identifies the requested timestamp id
 * \param ts      [OUT]         pointer to a struct holding the timestamp
 */
vtss_rc vtss_oam_timestamp_get(const vtss_inst_t             inst,
                               const vtss_oam_ts_id_t        *const id,
                               vtss_oam_ts_timestamp_t       *const ts);
#endif /* VTSS_ARCH_SERVAL_CE */

/** \brief Timestamp allocation */
typedef struct vtss_ts_timestamp_alloc_t {
    u64 port_mask;              /**< Identify the ports that a timestamp id is allocated to */
    void * context;             /**< Application specific context used as parameter in the call-out */
    void (*cb)(void *context, u32 port_no, vtss_ts_timestamp_t *ts);
                                /**< Application call-out function called when the timestamp is available */

} vtss_ts_timestamp_alloc_t;
/**
 * \brief Allocate a timestamp id for a two step transmission
 * \param inst       [IN]          handle to an API instance
 * \param alloc_parm [IN]          pointer allocation parameters
 * \param ts_id      [OUT]         timestamp id
 *
 */
vtss_rc vtss_tx_timestamp_idx_alloc(const vtss_inst_t               inst,
                                 const vtss_ts_timestamp_alloc_t    *const alloc_parm,
                                 vtss_ts_id_t                       *const ts_id);

/**
 * \brief Age the FIFO timestamps
 * \param inst    [IN]          handle to an API instance
 *
 */
vtss_rc vtss_timestamp_age(const vtss_inst_t              inst);

/**
* \brief set port speed in the timestamp API (used to compensate for the internal ingress and egress latencies)
* \param inst    [IN]          handle to an API instance
* \param port_no [IN]          port number
* \param speed   [IN]          actual port speed
*
*/
vtss_rc vtss_ts_status_change(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_port_speed_t      speed);

#endif /* VTSS_FEATURE_TIMESTAMP */

#endif // _VTSS_TS_API_H_

// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
