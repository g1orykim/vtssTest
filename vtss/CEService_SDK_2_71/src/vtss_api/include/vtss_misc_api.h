/*

 Vitesse API software.

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

 $Id$
 $Revision$

*/

/**
 * \file
 * \brief Miscellaneous API
 * \details This header file describes miscellaneous API functions
 */

#ifndef _VTSS_MISC_API_H_
#define _VTSS_MISC_API_H_

#include <vtss_options.h>
#include <vtss_types.h>
#include "vtss_init_api.h"

/** \brief Trace group layer */
typedef enum
{
    VTSS_TRACE_LAYER_AIL,   /**< Application Interface Layer */
    VTSS_TRACE_LAYER_CIL,   /**< Chip Interface Layer */
 
    VTSS_TRACE_LAYER_COUNT  /**< Number of layers */
} vtss_trace_layer_t;

/** \brief Trace groups */
typedef enum
{
    VTSS_TRACE_GROUP_DEFAULT,       /**< Default trace group */
    VTSS_TRACE_GROUP_PORT,          /**< Port control */
    VTSS_TRACE_GROUP_PHY,           /**< PHY control */
    VTSS_TRACE_GROUP_PACKET,        /**< Packet control */
    VTSS_TRACE_GROUP_QOS,           /**< Quality of Service */
    VTSS_TRACE_GROUP_L2,            /**< Layer 2 */
    VTSS_TRACE_GROUP_L3,            /**< Layer 3 */
    VTSS_TRACE_GROUP_SECURITY,      /**< Security */
    VTSS_TRACE_GROUP_EVC,           /**< Ethernet Virtual Connections */
    VTSS_TRACE_GROUP_FDMA_NORMAL,   /**< Frame DMA Extraction and Injection when interrupts/scheduler is enabled */
    VTSS_TRACE_GROUP_FDMA_IRQ,      /**< Frame DMA when interrupts/scheduler is disabled */
#ifdef VTSS_ARCH_DAYTONA            /**< Daytona layers are placed before the PHY layer, othewise they are not shown in the CLI commands */
    VTSS_TRACE_GROUP_UPI,
    VTSS_TRACE_GROUP_XFI,
    VTSS_TRACE_GROUP_PCS_10GBASE_R, /**< PCS 10GBASE_R layer */
    VTSS_TRACE_GROUP_MAC10G,        /**< MAC 10G             */
    VTSS_TRACE_GROUP_RAB,           /**< RAB                 */
#endif /* VTSS_ARCH_DAYTONA */
    VTSS_TRACE_GROUP_REG_CHECK,     /**< Register access errors (must be able to print when interrupts/scheduler is disabled) */
    VTSS_TRACE_GROUP_MPLS,          /**< MPLS */
    VTSS_TRACE_GROUP_MACSEC,        /**< MACSEC control */
    VTSS_TRACE_GROUP_VCAP,          /**< VCAP */
    VTSS_TRACE_GROUP_OAM,           /**< OAM */
    VTSS_TRACE_GROUP_TS,            /**< Timestamping */

    VTSS_TRACE_GROUP_COUNT          /**< Number of trace groups */
} vtss_trace_group_t;

/** \brief Trace levels */
typedef enum
{
    VTSS_TRACE_LEVEL_NONE,  /**< No trace */
    VTSS_TRACE_LEVEL_ERROR, /**< Error trace */
    VTSS_TRACE_LEVEL_INFO,  /**< Information trace */
    VTSS_TRACE_LEVEL_DEBUG, /**< Debug trace */
    VTSS_TRACE_LEVEL_NOISE, /**< More debug information */

    VTSS_TRACE_LEVEL_COUNT  /**< Number of trace levels */
} vtss_trace_level_t;

/** \brief Trace group configuration */
typedef struct
{
    vtss_trace_level_t level[VTSS_TRACE_LAYER_COUNT]; /**< Trace level per layer */
} vtss_trace_conf_t;

/**
 * \brief Get trace configuration
 *
 * \param group [IN]  Trace group
 * \param conf [OUT]  Trace group configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_trace_conf_get(const vtss_trace_group_t  group,
                            vtss_trace_conf_t         *const conf);


/**
 * \brief Set trace configuration
 *
 * \param group [IN]  Trace group
 * \param conf [IN]   Trace group configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_trace_conf_set(const vtss_trace_group_t  group,
                            const vtss_trace_conf_t   *const conf);


/**
 * \brief Trace callout function
 *
 * \param layer [IN]     Trace layer
 * \param group [IN]     Trace group
 * \param level [IN]     Trace level
 * \param file [IN]      File name string
 * \param line [IN]      Line number in file
 * \param function [IN]  Function name string
 * \param format [IN]    Print format string
 *
 * \return Nothing.
 **/
void vtss_callout_trace_printf(const vtss_trace_layer_t  layer,
                               const vtss_trace_group_t  group,
                               const vtss_trace_level_t  level,
                               const char                *file,
                               const int                 line,
                               const char                *function,
                               const char                *format,
                               ...)
#if defined(__GNUC__) && (__GNUC__ > 2)
    __attribute__ ((format(printf,7,8)))
#endif
;

/**
 * \brief Trace hex-dump callout function
 *
 * \param layer [IN]     Trace layer
 * \param group [IN]     Trace group
 * \param level [IN]     Trace level
 * \param file [IN]      The file from where the trace were called.
 * \param line [IN]      The line from where the trace were called.
 * \param function [IN]  The function from where the trace were called.
 * \param byte_p [IN]    Pointer to start of area to print
 * \param byte_cnt [IN]  Number of bytes to print
 *
 * \return Nothing.
 **/
void vtss_callout_trace_hex_dump(const vtss_trace_layer_t  layer,
                                 const vtss_trace_group_t  group,
                                 const vtss_trace_level_t  level,
                                 const char                *file,
                                 const int                 line,
                                 const char                *function,
                                 const u8                  *byte_p,
                                 const int                 byte_cnt);

/** \brief Debug layer */
/* Debug layer */
typedef enum {
    VTSS_DEBUG_LAYER_ALL, /**< All layers */
    VTSS_DEBUG_LAYER_AIL, /**< Application Interface Layer */
    VTSS_DEBUG_LAYER_CIL  /**< Chip Interface Layer */
} vtss_debug_layer_t;
 
/** \brief Debug function group */
typedef enum {
    VTSS_DEBUG_GROUP_ALL,       /**< All groups */
    VTSS_DEBUG_GROUP_INIT,      /**< Initialization */
    VTSS_DEBUG_GROUP_MISC,      /**< Miscellaneous */
    VTSS_DEBUG_GROUP_PORT,      /**< Port configuration */
    VTSS_DEBUG_GROUP_PORT_CNT,  /**< Port counters */
    VTSS_DEBUG_GROUP_PHY,       /**< PHY */
    VTSS_DEBUG_GROUP_VLAN,      /**< VLAN */
    VTSS_DEBUG_GROUP_PVLAN,     /**< PVLAN */
    VTSS_DEBUG_GROUP_MAC_TABLE, /**< MAC address table */
    VTSS_DEBUG_GROUP_ACL,       /**< ACL */
    VTSS_DEBUG_GROUP_QOS,       /**< QoS */
    VTSS_DEBUG_GROUP_AGGR,      /**< Link aggregation */
    VTSS_DEBUG_GROUP_GLAG,      /**< Global link aggregation */
    VTSS_DEBUG_GROUP_STP,       /**< Spanning Tree */
    VTSS_DEBUG_GROUP_MIRROR,    /**< Mirroring */
    VTSS_DEBUG_GROUP_EVC,       /**< EVC */
    VTSS_DEBUG_GROUP_ERPS,      /**< ERPS */
    VTSS_DEBUG_GROUP_EPS,       /**< EPS */
    VTSS_DEBUG_GROUP_PACKET,    /**< Packet control */
    VTSS_DEBUG_GROUP_FDMA,      /**< FDMA */
    VTSS_DEBUG_GROUP_TS,        /**< TS: TimeStamping */
    VTSS_DEBUG_GROUP_PHY_TS,    /**< PHY_TS: PHY TimeStamping */
    VTSS_DEBUG_GROUP_WM,        /**< WaterMarks */
    VTSS_DEBUG_GROUP_LRN,       /**< LRN:COMMON */
    VTSS_DEBUG_GROUP_IPMC,      /**< IP Multicast */
    VTSS_DEBUG_GROUP_STACK,     /**< Stacking */
    VTSS_DEBUG_GROUP_CMEF,      /**< Congestion Management */
    VTSS_DEBUG_GROUP_HOST,      /**< CE-MAX Host configuration */
    VTSS_DEBUG_GROUP_MPLS,      /**< MPLS */
    VTSS_DEBUG_GROUP_MPLS_OAM,  /**< MPLS OAM */
    VTSS_DEBUG_GROUP_VXLAT,     /**< VLAN Translation */
    VTSS_DEBUG_GROUP_OAM,       /**< OAM, incl. VOEs/VOP */
    VTSS_DEBUG_GROUP_SER_GPIO,  /**< Serial GPIO configuration */
    VTSS_DEBUG_GROUP_L3,        /**< L3 services */
    VTSS_DEBUG_GROUP_AFI,       /**< Automatic Frame Injector */
    VTSS_DEBUG_GROUP_MACSEC,    /**< 802.1AE MacSec */
#ifdef VTSS_ARCH_DAYTONA
    VTSS_DEBUG_GROUP_XFI,       /**< XFI layer */
    VTSS_DEBUG_GROUP_UPI,       /**< UPI layer */
    VTSS_DEBUG_GROUP_PCS_10GBASE_R, /**< PCS 10GBASE_R layer */
    VTSS_DEBUG_GROUP_MAC10G,    /**< MAC 10G   */
    VTSS_DEBUG_GROUP_WIS,       /**< WIS Group errors */
    VTSS_DEBUG_GROUP_RAB,       /**< RAB              */
    VTSS_DEBUG_GROUP_XAUI,      /**< PCS XAUI         */
    VTSS_DEBUG_GROUP_OTN,       /**< OTN              */
    VTSS_DEBUG_GROUP_GFP,       /**< GFP              */
#endif /* VTSS_ARCH_DAYTONA */

    /* New groups are added above this line */
    VTSS_DEBUG_GROUP_COUNT      /**< Number of groups */
} vtss_debug_group_t;

/** \brief Special chip number value for showing information from all chips */
#define VTSS_CHIP_NO_ALL 0xffffffff

/** \brief Debug information structure */
typedef struct {
    vtss_debug_layer_t layer;                           /**< Layer */
    vtss_debug_group_t group;                           /**< Function group */
    vtss_chip_no_t     chip_no;                         /**< Chip number, multi-chip targets */
    BOOL               port_list[VTSS_PORT_ARRAY_SIZE]; /**< Port list */
    BOOL               full;                            /**< Full information dump */
    BOOL               clear;                           /**< Clear counters */
} vtss_debug_info_t;
 
/** \brief Debug printf function */
#if defined(__GNUC__) && (__GNUC__ > 2)
typedef void (* vtss_debug_printf_t)(const char *fmt, ...) __attribute__ ((format(printf,1,2)));
#else
typedef void (* vtss_debug_printf_t)(const char *fmt, ...);
#endif /* __GNUC__ */
 
/**
 * \brief Get default debug information structure
 *
 * \param info [OUT]  Debug information
 *
 * \return Return code.
 **/
vtss_rc vtss_debug_info_get(vtss_debug_info_t *const info);
 
/**
 * \brief Print default information
 *
 * \param inst [IN]   Target instance reference.
 * \param prntf [IN]  Debug printf function.
 * \param info [IN]   Debug information
 *
 * \return Return code.
 **/
vtss_rc vtss_debug_info_print(const vtss_inst_t         inst,
                              const vtss_debug_printf_t prntf,
                              const vtss_debug_info_t   *const info);

/* - API protection functions -------------------------------------- */

/** \brief API lock structure */
typedef struct {
    vtss_inst_t inst;     /**< Target instance reference */
    const char *function; /**< Function name */
    const char *file;     /**< File name */
    int        line;      /**< Line number */
} vtss_api_lock_t;

/**
 * \brief Lock API access
 *
 * \param lock [IN]  Lock information
*/
void vtss_callout_lock(const vtss_api_lock_t *const lock);

/**
 * \brief Unlock API access
 *
 * \param lock [IN]  Lock information
*/
void vtss_callout_unlock(const vtss_api_lock_t *const lock);

#if defined(VTSS_FEATURE_MISC)
/* - Direct register access (for debugging only) ------------------- */

/**
 * \brief Read value from target register.
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to read. Format depends on target.
 * \param value [OUT]   Register value.
 *
 * \return Return code.
 **/
vtss_rc vtss_reg_read(const vtss_inst_t    inst,
                      const vtss_chip_no_t chip_no,
                      const u32            addr,
                      u32                  *const value);


/**
 * \brief Write value to target register.
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to read. Format depends on target.
 * \param value [IN]    Register value.
 *
 * \return Return code.
 **/
vtss_rc vtss_reg_write(const vtss_inst_t    inst,
                       const vtss_chip_no_t chip_no,
                       const u32            addr,
                       const u32            value);


/**
 * \brief Read, modify and write value to target register.
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to read. Format depends on target.
 * \param value [IN]    Register value.
 * \param mask [IN]     Register mask, only bits enabled are changed.
 *
 * \return Return code.
 **/
vtss_rc vtss_reg_write_masked(const vtss_inst_t    inst,
                              const vtss_chip_no_t chip_no,
                              const u32            addr,
                              const u32            value,
                              const u32            mask);

/* - Secondary chip if ------------------- */

/**
 * \brief Clear EXT0-1 interrupt sticky bits on secondary chip.
 *
 * \param inst [IN]    Target instance reference.
 * \param ext [IN]     EXT number (0-1).
 *
 * \return Return code.
 **/
vtss_rc vtss_intr_sticky_clear(const vtss_inst_t    inst,
                               u32                  ext);

/* - Chip ID and revision ------------------------------------------ */

/** \brief Chip ID */
typedef struct
{
    u16  part_number;   /**< BCD encoded part number */
    u16  revision;      /**< Chip revision */
#if defined(VTSS_ARCH_DAYTONA)
    u16  variant;      /**< Chip variant */
#endif /* VTSS_ARCH_DAYTONA */
} vtss_chip_id_t;

/**
 * \brief Get chip ID and revision.
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_id [IN]  Pointer to chip ID structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_chip_id_get(const vtss_inst_t  inst,
                         vtss_chip_id_t     *const chip_id);


/* - Polling functions ---------------------------------------- */

/**
 * \brief Polling function called every second.
 *
 * \param inst [IN]  Target instance reference.
 *
 * \return Return code.
 **/
vtss_rc vtss_poll_1sec(const vtss_inst_t  inst);

/**
 * \brief Define event (interrupt) types relatesd to PTP in the switch chips
 *
 **/
typedef enum {
    VTSS_PTP_SYNC_EV =      (1 << 0), /*< PTP Synchronization pulse update */
    VTSS_PTP_EXT_SYNC_EV =  (1 << 1), /*< PTP External Synchronization input  (Sync 0 on Serval)*/
    VTSS_PTP_CLK_ADJ_EV =   (1 << 2), /*< PTP Clock adjustment updated */
    VTSS_PTP_TX_TSTAMP_EV = (1 << 3), /*< Tx timestamp udated (Jaguar and Serval only)*/
    VTSS_PTP_EXT_1_SYNC_EV =(1 << 4), /*< PTP External Synchronization 1 input (Serval only)*/
} vtss_ptp_event_type_t;

/**
 * \brief PTP polling function called at by interrupt or periodicly
 *
 * \param inst       [IN]  Target instance reference.
 * \param ev_mask    [OUT] Event type mask of active events
 *
 * \note The \e ev_mask parameter can be either a single event_type or
 * multiple event types (or \e VTSS_EVTYPE_ALL). If invoked by a
 * processor interrupt signal, the type of event to check for may be
 * narrowed in to specific events.
 *
 * \return Return code.
 **/
vtss_rc vtss_ptp_event_poll(const vtss_inst_t      inst,
                            vtss_ptp_event_type_t  *const ev_mask);

/**
 * \brief Enable PTP event generation for a specific event type
 *
 * \param inst     [IN] Target instance reference.
 * \param enable   [IN] Enable or disable events
 * \param ev_mask  [IN] Event type(s) to control (mask)
 *
 * \return Return code.
 **/
vtss_rc vtss_ptp_event_enable(const vtss_inst_t            inst,
                              const vtss_ptp_event_type_t  ev_mask,
                              const BOOL                   enable);

/**
 * \brief Define the dev_all event (interrupt) types 
 *
 **/
typedef enum {
    VTSS_DEV_ALL_TX_TSTAMP_EV = (1 << 0), /*< Tx timestamp udated */
    VTSS_DEV_ALL_LINK_EV =      (1 << 1), /*< Link state changed */
} vtss_dev_all_event_type_t;

/**
 * \brief Define the dev_all polling types 
 *
 **/
typedef enum {
    VTSS_DEV_ALL_POLL_ALL,          /*< Polling all chip */
    VTSS_DEV_ALL_POLL_PRIMARY,      /*< Polling primary chip */
    VTSS_DEV_ALL_POLL_SECONDARY,    /*< Polling secondary chip */
} vtss_dev_all_event_poll_t;

/**
 * \brief DEV_ALL polling function called at by interrupt or periodically
 *
 * \param inst      [IN] Target instance reference.
 * \param poll_type [IN] Polling type
 * \param ev_mask   [OUT] Event type mask array of active events for all ports - must be of size VTSS_PORT_ARRAY_SIZE
 *
 * \note The \e ev_mask parameter can be either a single event_type or
 * multiple event types (or \e VTSS_EVTYPE_ALL). If invoked by a
 * processor interrupt signal, the type of event to check for may be
 * narrowed in to specific events.
 *
 * \return Return code.
 **/
vtss_rc vtss_dev_all_event_poll(const vtss_inst_t                 inst,
                                const vtss_dev_all_event_poll_t   poll_type,
                                vtss_dev_all_event_type_t         *const ev_mask);

/**
 * \brief Enable DEV_ALL event generation for a specific event type
 *
 * \param inst     [IN] Target instance reference.
 * \param port_no  [IN] Port number.
 * \param enable   [IN] Enable or disable events.
 * \param ev_mask  [IN] Event type(s) to control (mask).
 *
 * \return Return code.
 **/
vtss_rc vtss_dev_all_event_enable(const vtss_inst_t                inst,
                                  const vtss_port_no_t             port_no,
                                  const vtss_dev_all_event_type_t  ev_mask,
                                  const BOOL                       enable);

#endif /* VTSS_FEATURE_MISC */

/* - GPIOs --------------------------------------------------------- */

#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_LUTON28)
/** \brief Number of GPIOs */
#define VTSS_GPIOS         16
#endif /* VTSS_ARCH_B2/LUTON28 */

#if defined(VTSS_ARCH_JAGUAR_1)
/** \brief Number of GPIOs */
#undef VTSS_GPIOS
#define VTSS_GPIOS         24
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_DAYTONA)
/** \brief Number of GPIOs */
#define VTSS_GPIOS         16
#endif /* VTSS_ARCH_DAYTONA */

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
/** \brief Number of GPIOs */
#undef VTSS_GPIOS
#define VTSS_GPIOS         32
#endif /* VTSS_ARCH_LUTON26/SERVAL */

#if defined(VTSS_ARCH_JAGUAR_2)
/** \brief Number of GPIOs */
// JR2-TBD: Update with correct number
#undef VTSS_GPIOS
#define VTSS_GPIOS         32
#endif /* VTSS_ARCH_JAGUAR_2 */

#if defined(VTSS_GPIOS)

/** \brief GPIO number */
typedef u32 vtss_gpio_no_t;

/** \brief GPIO start number */
#define VTSS_GPIO_NO_START 0

/** \brief GPIO end number */
#define VTSS_GPIO_NO_END   (VTSS_GPIO_NO_START+VTSS_GPIOS)

/**
 * \brief GPIO configured mode
 **/
typedef enum
{
    VTSS_GPIO_OUT,    /**< Output enabled */
    VTSS_GPIO_IN,     /**< Input enabled */
    VTSS_GPIO_IN_INT, /**< Input enabled, IRQ gated */
    VTSS_GPIO_ALT_0,  /**< Alternate function 0 */
    VTSS_GPIO_ALT_1,  /**< Alternate function 1 */
    VTSS_GPIO_ALT_2,  /**< Alternate function 2 */
} vtss_gpio_mode_t;

/**
 * \brief Set GPIO mode.
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param gpio_no [IN]  GPIO pin number.
 * \param mode [IN]     GPIO mode.
 *
 * \return Return code.
 **/
vtss_rc vtss_gpio_mode_set(const vtss_inst_t      inst,
                           const vtss_chip_no_t   chip_no,
                           const vtss_gpio_no_t   gpio_no,
                           const vtss_gpio_mode_t mode);

/**
 * \brief Set GPIO direction to input or output.
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param gpio_no [IN]  GPIO pin number.
 * \param output [IN]   TRUE if output, FALSE if input.
 *
 * \return Return code.
 *
 * \em DEPRECATED. Use vtss_gpio_mode_set() instead.
 **/
vtss_rc vtss_gpio_direction_set(const vtss_inst_t     inst,
                                const vtss_chip_no_t  chip_no,
                                const vtss_gpio_no_t  gpio_no,
                                const BOOL            output);



/**
 * \brief Read from GPIO input pin.
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param gpio_no [IN]  GPIO pin number.
 * \param value [OUT]   TRUE if pin is high, FALSE if it is low.
 *
 * \return Return code.
 **/
vtss_rc vtss_gpio_read(const vtss_inst_t     inst,
                       const vtss_chip_no_t  chip_no,
                       const vtss_gpio_no_t  gpio_no,
                       BOOL                  *const value);



/**
 * \brief Write to GPIO output pin.
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param gpio_no [IN]  GPIO pin number.
 * \param value [IN]    TRUE to set pin high, FALSE to set pin low.
 *
 * \return Return code.
 **/
vtss_rc vtss_gpio_write(const vtss_inst_t     inst,
                        const vtss_chip_no_t  chip_no,
                        const vtss_gpio_no_t  gpio_no,
                        const BOOL            value);

/**
 * \brief Get GPIO event indication
 *
 * \param inst [IN]         Target instance reference.
 * \param chip_no [IN]      Chip number (if multi-chip instance).
 * \param events [OUT]      Event indication for each GPIO pin - must point to VTSS_GPIOS of BOOL.
 *
 * \return Return code.
 **/
vtss_rc vtss_gpio_event_poll(const vtss_inst_t        inst,
                             const vtss_chip_no_t     chip_no,
                             BOOL                     *const events);

/**
 * \brief Set GPIO event enable
 *
 * \param inst [IN]         Target instance reference.
 * \param chip_no [IN]      Chip number (if multi-chip instance).
 * \param gpio_no [IN]      GPIO pin number.
 * \param enable  [IN]      Enable or disable event.
 *
 * \return Return code.
 **/
vtss_rc vtss_gpio_event_enable(const vtss_inst_t       inst,
                               const vtss_chip_no_t    chip_no,
                               const vtss_gpio_no_t    gpio_no,
                               BOOL                    enable);
#endif /* GPIOS */



#if defined(VTSS_ARCH_B2)
/**
 * \brief Chip version -01: Recovered clock frequency
 **/
typedef enum
{
    VTSS_RECOVERED_CLOCK_25,        /**< 25 MHz (1G port) */
    VTSS_RECOVERED_CLOCK_31_25,     /**< 31.25 MHz (1G/10G port) */
    VTSS_RECOVERED_CLOCK_39_06,     /**< 39.06 MHz (10G port) */
    VTSS_RECOVERED_CLOCK_62_5,      /**< 62.5 MHz (1G port) */
    VTSS_RECOVERED_CLOCK_78_125,    /**< 78.125 MHz (10G port) */
    VTSS_RECOVERED_CLOCK_125,       /**< 125 MHz (1G port) */
    VTSS_RECOVERED_CLOCK_156_25     /**< 156.25 MHz (10G port) */
} vtss_recovered_clock_t;

/**
 * \brief Set GPIO recovered clock source.
 *
 * \param inst [IN]     Target instance reference.
 * \param gpio_no [IN]  GPIO pin number, 0 or 1.
 * \param port_no [IN]  Port number or VTSS_PORT_NO_NONE.
 * \param clk [IN]      Recovered clock frequency.
 *
 * \return Return code.
 **/
vtss_rc vtss_gpio_clk_set(const vtss_inst_t             inst,
                          const vtss_gpio_no_t          gpio_no,
                          const vtss_port_no_t          port_no,
                          const vtss_recovered_clock_t  clk);
#endif /* VTSS_ARCH_B2 */



/* - Serial LED ---------------------------------------------------- */

#if defined(VTSS_FEATURE_SERIAL_LED)
/**
 * \brief LED mode
 **/
typedef enum
{
    VTSS_LED_MODE_IGNORE,   /**< Do not change */
    VTSS_LED_MODE_DISABLED, /**< Disabled */
    VTSS_LED_MODE_OFF,      /**< Off */
    VTSS_LED_MODE_ON,       /**< On */
    VTSS_LED_MODE_2_5,      /**< 2.5 Hz */
    VTSS_LED_MODE_5,        /**< 5 Hz */
    VTSS_LED_MODE_10,       /**< 10 Hz */
    VTSS_LED_MODE_20        /**< 20 Hz */
} vtss_led_mode_t;

/**
 * \brief LED port number
 **/
typedef u32 vtss_led_port_t;

/**
 * \brief Setup serial LED mode.
 *
 * \param inst [IN]  Target instance reference.
 * \param port [IN]  Serial LED port, 0-29.
 * \param mode [IN]  Serial LED mode for three LEDs.
 *
 * \return Return code.
 **/
vtss_rc vtss_serial_led_set(const vtss_inst_t      inst,
                            const vtss_led_port_t  port,
                            const vtss_led_mode_t  mode[3]);



/**
 * \brief Setup serial LED light intensity.
 *
 * \param inst [IN]  Target instance reference.
 * \param port [IN]  Serial LED port, 0-29.
 * \param intensity [IN]  intensity in percent (0-100).
 *
 * \return Return code.
 **/
vtss_rc vtss_serial_led_intensity_set(const vtss_inst_t      inst,
                                      const vtss_led_port_t  port,
                                      const i8               intensity);


/**
 * \brief Getting current serial LED light intensity.
 *
 * \param inst [IN]  Target instance reference.
 * \param intensity [IN]  intensity in percent (0-100).
 *
 * \return Return code.
 **/
vtss_rc vtss_serial_led_intensity_get(const vtss_inst_t      inst,
                                      i8                     *intensity);


#endif /* VTSS_FEATURE_SERIAL_LED */


/* - Serial GPIO control ---------------------------------------------------- */

#if defined(VTSS_FEATURE_SERIAL_GPIO)

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
/** \brief Number of serial GPIO groups */
#define VTSS_SGPIO_GROUPS 1
#endif /* VTSS_ARCH_LUTON26 */

#if defined(VTSS_ARCH_JAGUAR_1)
#undef VTSS_SGPIO_GROUPS
/** \brief Number of serial GPIO groups */
#define VTSS_SGPIO_GROUPS 2
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_JAGUAR_2)
// JR2-TBD: Insert correct values
#undef VTSS_SGPIO_GROUPS
/** \brief Number of serial GPIO groups */
#define VTSS_SGPIO_GROUPS 1
#endif /* VTSS_ARCH_JAGUAR_2 */

/** \brief Serial GPIO group */
typedef u32 vtss_sgpio_group_t;

/** \brief Number of serial GPIO ports */
#define VTSS_SGPIO_PORTS 32

/** \brief SGPIO output mode */
typedef enum
{
    VTSS_SGPIO_MODE_OFF,            /**< Off */
    VTSS_SGPIO_MODE_ON,             /**< On */
    VTSS_SGPIO_MODE_0,              /**< Mode 0 */
    VTSS_SGPIO_MODE_1,              /**< Mode 1 */
    VTSS_SGPIO_MODE_0_ACTIVITY,     /**< Mode 0 when link activity */
    VTSS_SGPIO_MODE_1_ACTIVITY,     /**< Mode 1 when link activity */
    VTSS_SGPIO_MODE_0_ACTIVITY_INV, /**< Mode 0 when link activity, inversed polarity */
    VTSS_SGPIO_MODE_1_ACTIVITY_INV  /**< Mode 1 when link activity, inversed polarity */
} vtss_sgpio_mode_t;

/** \brief SGPIO blink mode */
typedef enum
{
    VTSS_SGPIO_BMODE_TOGGLE, /**< Burst toggle (mode 1 only) */
    VTSS_SGPIO_BMODE_0_625,  /**< 0.625 Hz (mode 0 only) */
    VTSS_SGPIO_BMODE_1_25,   /**< 1.25 Hz */
    VTSS_SGPIO_BMODE_2_5,    /**< 2.5 Hz */
    VTSS_SGPIO_BMODE_5       /**< 5 Hz */
} vtss_sgpio_bmode_t;
    
/** \brief SGPIO port configuration */
typedef struct
{
    BOOL              enabled; /**< Port enabled/disabled */
    vtss_sgpio_mode_t mode[4]; /**< Mode for each bit */
    BOOL              int_pol_high[4];  /**< SGPIO interrupt polarity for each bit - TRUE - Interrupt when SGPIO pin high, FALSE - Interrupt when SGPIO pin low. */
} vtss_sgpio_port_conf_t;

/** \brief SGPIO configuration for a group */
typedef struct
{
    vtss_sgpio_bmode_t     bmode[2];                     /**< Blink mode 0 and 1 */
    u8                     bit_count;                    /**< Bits enabled per port, 1-4 */
    vtss_sgpio_port_conf_t port_conf[VTSS_SGPIO_PORTS];  /**< Port configuration */
} vtss_sgpio_conf_t;

/**
 * \brief Get SGPIO configuration
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param group [IN]    SGPIO group.
 * \param conf [OUT]    SGPIO configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_sgpio_conf_get(const vtss_inst_t        inst,
                            const vtss_chip_no_t     chip_no,
                            const vtss_sgpio_group_t group,
                            vtss_sgpio_conf_t        *const conf);

/**
 * \brief Set SGPIO configuration
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param group [IN]    SGPIO group.
 * \param conf [IN]     SGPIO configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_sgpio_conf_set(const vtss_inst_t        inst,
                            const vtss_chip_no_t     chip_no,
                            const vtss_sgpio_group_t group,
                            const vtss_sgpio_conf_t  *const conf);

/** \brief SGPIO read data for a port */
typedef struct
{
    BOOL value[4]; /**< Data for each and bit */
} vtss_sgpio_port_data_t;

/**
 * \brief Read SGPIO data
 *
 * \param inst [IN]     Target instance reference.
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param group [IN]    SGPIO group.
 * \param data [OUT]    SGPIO data.
 *
 * \return Return code.
 **/
vtss_rc vtss_sgpio_read(const vtss_inst_t        inst,
                        const vtss_chip_no_t     chip_no,
                        const vtss_sgpio_group_t group,
                        vtss_sgpio_port_data_t   data[VTSS_SGPIO_PORTS]);
/**
 * \brief Get SGPIO event indication
 *
 * \param inst [IN]          Target instance reference.
 * \param chip_no [IN]       Chip number (if multi-chip instance).
 * \param group [IN]         SGPIO group.
 * \param bit [IN]           SGPIO port bit (0-3).
 * \param events [OUT]       Event indication for each port for the selected bit - must point to VTSS_SGPIO_PORTS of BOOL.
 *
 * \return Return code.
 **/
vtss_rc vtss_sgpio_event_poll(const vtss_inst_t        inst,
                              const vtss_chip_no_t     chip_no,
                              const vtss_sgpio_group_t group,
                              const u32                bit,
                              BOOL                     *const events);

/**
 * \brief Get SGPIO event enable
 *
 * \param inst [IN]          Target instance reference.
 * \param chip_no [IN]       Chip number (if multi-chip instance).
 * \param group [IN]         SGPIO group.
 * \param port [IN]          SGPIO port (0-31).
 * \param bit [IN]           SGPIO port bit (0-3).
 * \param enable [IN]        Event for each port for the selected bit is enabled or disabled.
 *
 * \return Return code.
 **/
vtss_rc vtss_sgpio_event_enable(const vtss_inst_t        inst,
                                const vtss_chip_no_t     chip_no,
                                const vtss_sgpio_group_t group,
                                const vtss_port_no_t     port,
                                const u32                bit,
                                BOOL                     enable);

#endif /* VTSS_FEATURE_SERIAL_GPIO */


/**
 * \brief Configure interrupt.
 * \param inst [IN] Target instance reference.
 * \param mask [IN] Interrupt mask - Configures the interrupts for the bits set in the mask. 
 * \param polarity [IN] Polarity - Interrupt polarity.
 * \param enable [IN] Enable   - 1 = enable, 0 = disable.
 *
 * \return Return code.
**/
vtss_rc vtss_intr_cfg(const vtss_inst_t       inst,
                      const u32               mask,
                      const BOOL              polarity,
                      const BOOL              enable);


#if defined(VTSS_FEATURE_INTERRUPTS)

/**
\brief Interrupt source structure.
*/
typedef struct {
    BOOL  link_change[VTSS_PORT_ARRAY_SIZE]; /**< Applies to XAUI, 100FX and 1000X ports */
} vtss_intr_t;

/**
 * \brief Set the interrupt mask.
 * \param inst [IN] Target instance reference.
 * \param mask [IN] Pointer to mask structure.
 *
 * \return Return code.
**/
vtss_rc vtss_intr_mask_set(const vtss_inst_t             inst,
                           vtss_intr_t *mask);

/**
 * \brief Get the interrupt status for all enabled sources.
 *        The interrupt status bit is cleared by the function
 *
 * \param inst [IN] Target instance reference.
 * \param status [OUT] Pointer to a structure with status of all enabled interrupt sources.
 *
 * \return Return code.
 **/
vtss_rc vtss_intr_status_get(const vtss_inst_t             inst,
                             vtss_intr_t *status);

/**
 * \brief This vil negate polarity on fast link fail detection signals when active
 *        This is only intended to be used on Luton26 RevB as a work around for the 
 *        Atom PHY interrupt always active problem
 *
 * \param inst [IN] Target instance reference.
 *
 * \return Return code.
 **/
vtss_rc  vtss_intr_pol_negation(const vtss_inst_t   inst);

#endif /* VTSS_FEATURE_INTERRUPTS */

#ifdef VTSS_FEATURE_IRQ_CONTROL

/**
 * \brief Interrupt sources.
 */
typedef enum
{
    VTSS_IRQ_XTR,               /**< Frame Extraction Ready(register-based) */
    VTSS_IRQ_FDMA_XTR,          /**< Frame Extraction Ready (FDMA-based) */
    VTSS_IRQ_SOFTWARE,          /**< Software IRQ */
    VTSS_IRQ_PTP_RDY,           /**< PTP Timestamp Ready */
    VTSS_IRQ_PTP_SYNC,          /**< PTP Synchronization IRQ */
    VTSS_IRQ_MAX,               /**< Maximum IRQ Source */
} vtss_irq_t;

/**
 * \brief Interrupt configuration options.
 */
typedef struct {
    BOOL external;               /**< Redirect to external IRQ */
} vtss_irq_conf_t;

/**
 * \brief Interrupt status structure.
 */
typedef struct {
    u32 active;         /**< Bitmap for pending IRQs (VTSS_IRQ_xxx) */
    u32 raw_ident;      /**< RAW (target dependentant) bitmap for active pending IRQs */
    u32 raw_status;     /**< RAW (target dependentant) bitmap for all pending IRQs */
    u32 raw_mask;       /**< RAW (target dependentant) bitmap for IRQs mask */
} vtss_irq_status_t;

/**
 * \brief Get IRQ configuration
 *
 * \param inst [IN]     Target instance reference.
 * \param irq  [IN]     Interrupt source.
 * \param conf [OUT]    IRQ configuration.
 *
 * \return Return code.
 **/
vtss_rc  vtss_irq_conf_get(const vtss_inst_t inst,
                           const vtss_irq_t irq,
                           vtss_irq_conf_t *conf);

/**
 * \brief Set IRQ configuration
 *
 * \param inst [IN]     Target instance reference.
 * \param irq  [IN]     Interrupt source.
 * \param conf [IN]     IRQ configuration.
 *
 * \return Return code.
 **/
vtss_rc  vtss_irq_conf_set(const vtss_inst_t inst,
                           const vtss_irq_t irq,
                           const vtss_irq_conf_t *const conf);

/**
 * \brief Get IRQ status (active sources), mask current sources
 *
 * \param inst   [IN]   Target instance reference.
 * \param status [OUT]  IRQ status.
 *
 * \return Return code.
 **/
vtss_rc  vtss_irq_status_get_and_mask(const vtss_inst_t inst,
                                      vtss_irq_status_t *status);

/**
 * \brief Control a specific interrupt source
 *
 * \param inst   [IN]   Target instance reference.
 * \param irq    [IN]   Interrupt source.
 * \param enable [IN]   Enable or disable source.
 *
 * \return Return code.
 **/
vtss_rc  vtss_irq_enable(const vtss_inst_t inst,
                         const vtss_irq_t irq,
                         BOOL enable);

#endif /* VTSS_FEATURE_IRQ_CONTROL */


/* - API tod functions -------------------------------------- */

/**
 * \brief Get the current hw nanosec time
 *  This function is called from interrupt
 * \returns actual ns counter
 */
u32 vtss_tod_get_ns_cnt(void);

/**
 * \brief If the actual HW does not support time stamping, an external callback
 * function can be set up to do the work.
 * \returns actual ns counter.
 */
typedef u32 (*tod_get_ns_cnt_cb_t)(void);  /* Get ns Counter callback  */

/**
 * \brief Set an external hw nanosec read function.
 *
 * \param cb pointer to callback function
 */
void vtss_tod_set_ns_cnt_cb(tod_get_ns_cnt_cb_t cb);

/**
 * VTSS_OS_TIMESTAMP_TYPE
 * VTSS_OS_TIMESTAMP()
 *   These two provides a mean to have the API timestamp events for use
 *   by the application. It is up to the platform specific code to
 *   implement the actual functions to obtain the timestamp.
 *   The implementation *must* be callable from interrupt context, so no
 *   implicit waits or sleeps are allowed.
 */
typedef struct {
    unsigned int hw_cnt; /**< hardware counter */
} vtss_os_timestamp_t;
#define VTSS_OS_TIMESTAMP_TYPE vtss_os_timestamp_t /**< The VTSS_OS_TIME_STAMP_TYPE defines the type */
#define VTSS_OS_TIMESTAMP(timestamp)                                    \
  do {                                                                  \
    /* Currently no need to lock scheduler, since it's only          */ \
    /* called from a function, where the sceduler is already locked. */ \
    /* cyg_scheduler_lock(); */                                         \
    (timestamp)->hw_cnt = vtss_tod_get_ns_cnt();                        \
    /* cyg_scheduler_unlock(); */                                       \
  } while(0); /**< VTSS_OS_TIMESTAMP() provides the implementation that will fill in the timestamp. */
 

/* - Fan control functions -------------------------------------- */
#if defined(VTSS_FEATURE_FAN)

/** 
 * \brief Initialize the temperature sensor 
 *
 * \param inst [IN]        Target instance reference
 * \param enable [IN]       Set to true if sensor shall be active else false
 *
 * \return Return code.
 **/
vtss_rc vtss_temp_sensor_init(const vtss_inst_t inst,
                              const BOOL enable);

/** 
 * \brief Read temperature sensor value
 *
 * \param inst [IN]        Target instance reference
 * \param temperature [OUT]      Temperature from sensor (range from -46 to 135 degC)
 *
 * \return Return code.
 **/

vtss_rc vtss_temp_sensor_get(const vtss_inst_t     inst,
                             i16                   *temperature);

/** \brief FAN PWM frequency */
typedef enum
{
    VTSS_FAN_PWM_FREQ_25KHZ,
    VTSS_FAN_PWM_FREQ_120HZ,
    VTSS_FAN_PWM_FREQ_100HZ,
    VTSS_FAN_PWM_FREQ_80HZ,
    VTSS_FAN_PWM_FREQ_60HZ,
    VTSS_FAN_PWM_FREQ_40HZ,
    VTSS_FAN_PWM_FREQ_20HZ,
    VTSS_FAN_PWM_FREQ_10HZ
} vtss_fan_pwd_freq_t;

/** \brief FAN Types  */
typedef enum
{
    VTSS_FAN_2_WIRE_TYPE,
    VTSS_FAN_3_WIRE_TYPE,
    VTSS_FAN_4_WIRE_TYPE
} vtss_fan_type_t;


/** \brief Maximum fan speed level (Fan runs at full speed) */
#define VTSS_FAN_SPEED_MAX 0x255
/** \brief Minimum fan speed level (Fan is OFF) */
#define VTSS_FAN_SPEED_MIN 0x0


/** \brief Fan specifications */
typedef struct
{
    vtss_fan_pwd_freq_t fan_pwm_freq;      /**< Fan PWM frequency*/
    BOOL                fan_low_pol;       /**< Fan polarity of the PWM output. TRUE = PWM is logic 0 when on. FALSE = PWM is logic 1 when on */   
    BOOL                fan_open_col;      /**< PWM output is open collector if TRUE.*/
    vtss_fan_type_t     type;              /**< 2,3 or 4 wire fan type*/
#if defined(__C51__) || defined(__CX51__)
    ulong               ppr;               /**< Pulses per rotation. Only valid for 3 and 4 wire fans */
#else    
    u32                 ppr;               /**< Pulses per rotation. Only valid for 3 and 4 wire fans */
#endif
} vtss_fan_conf_t;


/** 
 * \brief Get the number of fan rotations
 *
 * \param inst [IN]       Target instance reference
 * \param fan_spec [IN]   Fan specification 
 * \param rotation_count [OUT] Number of fan rotation countered
 *
 * \return Return code.
 **/
vtss_rc vtss_fan_rotation_get(const vtss_inst_t inst,
                              vtss_fan_conf_t   *const fan_spec,
                              u32               *rotation_count);



/** 
 * \brief Set fan cool level (Duty cycle)
 *
 * \param inst [IN]       Target instance reference
 * \param lvl  [IN]       Level. 0 = Fan off, 255 = fan fully on
 *
 * \return Return code.
**/
vtss_rc vtss_fan_cool_lvl_set(const vtss_inst_t     inst,
                              u8 lvl);


/** 
 * \brief Initialise fan controller)
 *
 * \param inst [IN]    Target instance reference
 * \param spec [IN]    Fan specifications
 *
 * \return Return code.
**/
vtss_rc vtss_fan_controller_init(const vtss_inst_t     inst,
                                 const vtss_fan_conf_t *const spec);
                      
                      

/** 
 * \brief Get fan cool level (Duty cycle)
 *
 * \param inst [IN]       Target instance reference
 * \param lvl  [IN]       Level. 0 = Fan off, 255 = fan fully on
 *
 * \return Return code.
**/
vtss_rc vtss_fan_cool_lvl_get(const vtss_inst_t     inst,
                              u8 * lvl);


#endif /* VTSS_FEATURE_FAN */

/* - EEE functions -------------------------------------- */
#ifdef VTSS_FEATURE_EEE
/** \brief EEE port configuration */
typedef struct
{
    BOOL eee_ena;          /**< Enable EEE                                                                                                                                                       */
    u8   eee_fast_queues;  /**< Queues set in this mask will activate egress path as soon as any data is available. Vector for enabling fast queues. bit 0 = queue 0, bit 1 = queue 1 and so on. */
    u16  tx_tw;            /**< Time from path is activated until frame transmission restarted.                                                                                                  */
    u8   lp_advertisement; /**< Link partner EEE advertisement. Bit 0 = Link partner advertises 100BASE-T capability. Bit 1 = Link partner advertises 1000BASE-T capability.                     */

    BOOL optimized_for_power; /**< EEE can be optimized for either most power savings or least traffic latency */
} vtss_eee_port_conf_t;

/** \brief EEE port state change what? (JR only) */
typedef enum {
    VTSS_EEE_STATE_SELECT_LPI,      /**< Change LPI signal.               */
    VTSS_EEE_STATE_SELECT_SCH,      /**< Change scheduler enable.         */
    VTSS_EEE_STATE_SELECT_FP,       /**< Change frame mirroring flag.     */
    VTSS_EEE_STATE_SELECT_INTR_ENA, /**< Enable analyzer interrupts.      */
    VTSS_EEE_STATE_SELECT_INTR_ACK, /**< Acknowledge analyzer interrupts. */
} vtss_eee_state_select_t;

/** \brief EEE port state (JR only) */
typedef struct
{
    vtss_eee_state_select_t select; /**< State to change.                                       */
    u32                     val;    /**< New value to apply. Interpretation depends on #select. */
} vtss_eee_port_state_t;

/** \brief EEE port counters (JR only) */
typedef struct
{
    BOOL fill_level_get;   /**< [IN]  FALSE => Don't get fill level. TRUE => Get fill level.                    */
    u32  fill_level_thres; /**< [IN]  Stop iterating over queues when fill level exceeds this value.            */
    u32  fill_level;       /**< [OUT] Accumulated fill level, updated by API if #fill_level_get is TRUE.        */
    BOOL tx_out_bytes_get; /**< [IN]  FALSE => Don't get transmitted bytes. TRUE => Get tx'd bytes.             */
    u32  tx_out_bytes;     /**< [OUT] Transmitted number of bytes, updated by API if #tx_out_bytes_get is TRUE. */
} vtss_eee_port_counter_t;

/**
 * \brief Set EEE configuration
 *
 * \param inst     [IN] Target instance reference
 * \param port_no  [IN] Port number
 * \param eee_conf [IN] EEE configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_eee_port_conf_set(const vtss_inst_t          inst,
                               const vtss_port_no_t       port_no,
                               const vtss_eee_port_conf_t *const eee_conf);

/**
 * \brief Change EEE Port state.
 *
 * Supported on JR only.
 *
 * \param inst      [IN] Target instance reference
 * \param port_no   [IN] Port number
 * \param eee_state [IN] New port state
 *
 * \return Return code.
 **/
vtss_rc vtss_eee_port_state_set(const vtss_inst_t                  inst,
                                const vtss_port_no_t               port_no,
                                const vtss_eee_port_state_t *const eee_state);

/**
 * \brief Get EEE-related port counters.
 *
 * Support on JR only.
 *
 * \param inst        [IN]    Target instance reference
 * \param port_no     [IN]    Port number
 * \param eee_counter [INOUT] Structure indicating which counters to get, and the returned counter value.
 *
 * \return Return code.
 */
vtss_rc vtss_eee_port_counter_get(const vtss_inst_t                    inst,
                                  const vtss_port_no_t                 port_no,
                                        vtss_eee_port_counter_t *const eee_counter);

#endif /* VTSS_FEATURE_EEE */


/* - Daytona  functions -------------------------------------- */
#ifdef VTSS_ARCH_DAYTONA

/**
 * \brief Define the channel interrupt block types
 *
 **/
#define VTSS_CH_NONE_INT      0x00000000  /**< None Interrupt */
#define VTSS_CH_TFI_INT       0x00000001  /**< TFI Interrupt */
#define VTSS_CH_SFI_INT       0x00000002  /**< SFI Interrupt */
#define VTSS_CH_XAUI_INT      0x00000004  /**< XAUI Interrupt */
#define VTSS_CH_OTN1_INT      0x00000008  /**< OTN1 Interrupt */
#define VTSS_CH_WIS1_INT      0x00000010  /**< WIS1 Interrupt */
#define VTSS_CH_PCS1_INT      0x00000020  /**< PCS1 Interrupt */
#define VTSS_CH_GFP1_INT      0x00000040  /**< GFP1 Interrupt */
#define VTSS_CH_MAC1_INT      0x00000080  /**< MAC1 Interrupt */
#define VTSS_CH_RAB_INT       0x00000100  /**< RAB Interrupt */
#define VTSS_CH_MAC2_INT      0x00000200  /**< MAC2 Interrupt */
#define VTSS_CH_GFP2_INT      0x00000400  /**< GFP2 Interrupt */
#define VTSS_CH_PCS2_INT      0x00000800  /**< PCS2 Interrupt */
#define VTSS_CH_WIS2_INT      0x00001000  /**< WIS2 Interrupt */
#define VTSS_CH_OTN2_INT      0x00002000  /**< OTN2 Interrupt */
#define VTSS_CH_EFEC_INT      0x00004000  /**< EFEC1 Interrupt */
#define VTSS_CH_UPI_INT       0x00008000  /**< UPI Interrupt */
#define VTSS_CH_ALL_INT       0xFFFFFFFF  /**< ALL Interrupt */

typedef u32 vtss_interrupt_block_ch_t; /**< Single interrupt or 'OR' multiple interrupts as defined above */

/**
 * \brief Define the Misc interrupt block types
 *
 **/
#define VTSS_MISC_NONE_INT          0x00000000  /**< None Interrupt */
#define VTSS_MISC_I2C_INT           0x00000001  /**< I2C Interrupt */
#define VTSS_MISC_PMTICK_INT        0x00000002  /**< PMTICK Interrupt */
#define VTSS_MISC_IP1588_0_INT      0x00000004  /**< IP1588 channel 0 Interrupt */
#define VTSS_MISC_IP1588_1_INT      0x00000008  /**< IP1588 channel 1 Interrupt */
#define VTSS_MISC_EFEC7_0_INT       0x00000010  /**< EFEC7 channel 0 Interrupt */
#define VTSS_MISC_EFEC7_1_INT       0x00000020  /**< EFEC7 channel 1 Interrupt */
#define VTSS_MISC_LXFI_0_INT        0x00000040  /**< LXFI channel 0 Interrupt */
#define VTSS_MISC_LXFI_1_INT        0x00000080  /**< LXFI channel 1 Interrupt */
#define VTSS_MISC_CXFI_0_INT        0x00000100  /**< CXFI channel 0 Interrupt */
#define VTSS_MISC_CXFI_1_INT        0x00000200  /**< CXFI channel 1 Interrupt */
#define VTSS_MISC_GPIO_INT          0x00000400  /**< GPIO Interrupt */
#define VTSS_MISC_CPU_IF_INT        0x00000800  /**< CPU_IF Interrupt */
#define VTSS_MISC_ALL_INT           0xFFFFFFFF  /**< ALL Interrupt */

typedef u32 vtss_interrupt_block_misc_t; /**< Single interrupt or 'OR' multiple interrupts as defined above */

/** \brief Interrupt block mask values */
typedef struct
{
    vtss_interrupt_block_ch_t    ch_0;           /**< Channel 0 interrupt blocks mask */
    vtss_interrupt_block_ch_t    ch_1;           /**< Channel 1 interrupt blocks mask */
    vtss_interrupt_block_misc_t  misc;           /**< Misc interrupt blocks mask */
} vtss_interrupt_block_t;

/** 
 * \brief Set Interrupt block enable/disable configuration
 *
 * \param inst [IN]      Target instance reference
 * \param blocks [IN]    Blocks Interrupt enable mask
 *
 * \return Return code.
 **/
vtss_rc vtss_interrupt_enable_set(const vtss_inst_t             inst,
                                  const vtss_interrupt_block_t  *const blocks);

/** 
 * \brief Get Interrupt block enable/disable configuration
 *
 * \param inst [IN]      Target instance reference
 * \param blocks [OUT]   Blocks Interrupt enable mask
 *
 * \return Return code.
 **/
vtss_rc vtss_interrupt_enable_get(const vtss_inst_t       inst,
                                  vtss_interrupt_block_t  *const blocks);

/** 
 * \brief Get Interrupt block status
 *
 * \param inst [IN]      Target instance reference
 * \param status [IN]    Interrupt status indication
 *
 * \return Return code.
 **/
vtss_rc vtss_interrupt_status_get(const vtss_inst_t        inst,
                                  vtss_interrupt_block_t   *const status);

/** \brief PMTICK control data structure */
typedef enum  {
    VTSS_MISC_PMTICK_DISABLE,
    VTSS_MISC_PMTICK_INTERNAL,
    VTSS_MISC_PMTICK_EXTERNAL,
} vtss_pmtick_mode_t;

/** \brief PMTICK control data structure */
typedef struct vtss_pmtick_control_s {
    vtss_pmtick_mode_t mode;    /**< PMTICK mode (disabled, internal or external */
    u32 period;                 /**< PMTICK period in us, if internal mode is selected */
} vtss_pmtick_control_t;

/**
 * \brief Set the PMTICK configuration.
 * \note  The configuration is common to both client and line sides of a channel.
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port (Channel) number. 
 * \param control [IN]  pmtick control structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_pmtick_set(const vtss_inst_t     inst,
                        const vtss_port_no_t port_no,
                        const vtss_pmtick_control_t *const control);
/**
 * =============================================================================================
 * Events
 * =============================================================================================
 **/
#define VTSS_I4_ED_UCCW_EV  0x00000001  /*I4 ED Fec event */ 
#define VTSS_I4_RS_UCCW_EV  0x00000002  /*I4 RS Fec event */
#define VTSS_I4_BCH_UCCW_EV 0x00000004  /*I4 BCH Fec event */

typedef u32 vtss_efec_event_t;

/**
 * \brief Set EFEC enable/disable event generation for a specific event type or group of events
 * 
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param enable  [IN]   Enable or disable events
 * \param ev_mask [IN]   Event type(s) to control (mask)
 *
 * \return Return code.
 **/
vtss_rc vtss_efec_event_enable(const vtss_inst_t         inst,
                               const vtss_port_no_t      port_no,
                               const BOOL                enable,
                               const vtss_efec_event_t   ev_mask);
/**
 * \brief EFEC event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected
 *
 * \return Return code.
 **/
vtss_rc vtss_efec_event_poll(const vtss_inst_t        inst,
                             const vtss_port_no_t     port_no,
                             vtss_efec_event_t        *const status);

/**
 * \brief EFEC event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected irrespective of the mask register
 *
 * \return Return code.
 **/
vtss_rc vtss_efec_event_poll_without_mask(const vtss_inst_t     inst,
                                          const vtss_port_no_t  port_no,
                                          vtss_efec_event_t     *const status);


#define VTSS_I7_BER_ALARM_EV 0x00000001 /* I7 Ber Alarm event */

typedef u32 vtss_efeci7_event_t;

/**
 * \brief Set EFECI7 enable/disable event generation for a specific event type or group of events
 * 
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param enable  [IN]   Enable or disable events
 * \param ev_mask [IN]   Event type(s) to control (mask)
 *
 * \return Return code.
 **/
vtss_rc vtss_efeci7_event_enable(const vtss_inst_t        inst,
                                const vtss_port_no_t      port_no,
                                const BOOL                enable,
                                const vtss_efeci7_event_t ev_mask);
/**
 * \brief EFECI7 event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected
 *
 * \return Return code.
 **/
vtss_rc vtss_efeci7_event_poll(const vtss_inst_t        inst,
                                const vtss_port_no_t    port_no,
                                vtss_efeci7_event_t     *const status);

/**
 * \brief EFECI7 event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected irrespective of the mask register
 *
 * \return Return code.
 **/
vtss_rc vtss_efeci7_event_poll_without_mask(const vtss_inst_t   inst,
                                          const vtss_port_no_t  port_no,
                                          vtss_efeci7_event_t   *const status);

/**
 * \brief Set operating mode of the channel.
 * \param inst [IN]     Target instance reference.
 * \param channel      [IN] Channel number.
 * \param conf_mode    [IN] Channel mode.
 * \param two_lane_upi [IN] XAUI configuration.
 * \return Return code.
 **/
vtss_rc vtss_channel_mode_set(const vtss_inst_t inst,
                              const u16 channel,
                              const vtss_config_mode_t conf_mode,
                              const u32 two_lane_upi);

/**
 * \brief Get FAST_REGS Offset
 *
 * \param inst     [IN]  Target instance reference.
 * \param reg_addr [OUT] CFG_FAST_REGS Base Offset
 *
 **/
vtss_rc vtss_fast_regs_conf_get(const vtss_inst_t inst,
                                u32 *const reg_addr);


#endif /* VTSS_ARCH_DAYTONA */

/**
 * \brief Enable or disable register access checking.
 *
 * When enabled, every call to the register read/write
 * callouts (init_conf.reg_read()/write()) will be verified
 * for success. This will slow-down execution, so it is recommended
 * only to use this feature in dedicated debug builds.
 *
 * In multi-chip targets, only chip number 0 will be verified.
 *
 * The number of calls will be reference counted, according to
 * the following rules:
 * 1) Calls with enable = FALSE will increase the reference count.
 * 2) Calls with enable = TRUE will decrease the reference count,
 *    which cannot go lower than 0.
 * The reference count is initialized to 1 at API instantiation,
 * effectively disabling register access checking. A reference count of
 * 0 enables the feature.
 *
 * Error indications get printed with a call to
 * VTSS_EG(VTSS_TRACE_GROUP_REG_CHECK, ...), which will request
 * to be interrupt/scheduler disabled tolerant.
 *
 * Notice that this feature may not be available on all platforms.
 *
 * This feature will not work if the API is instantiated more than once,
 * since it uses the default instance to find its state.
 *
 * \param inst   [IN] Target instance reference.
 * \param enable [IN] Enable or disable register access checking (ref. counted).
 *
 * \return Return code.
 **/
vtss_rc vtss_debug_reg_check_set(const vtss_inst_t inst,
                                 const BOOL        enable);

#endif /* VTSS_MISC_API_H_ */
