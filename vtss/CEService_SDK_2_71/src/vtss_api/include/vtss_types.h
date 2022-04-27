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
 * \brief Generic types API
 * \details This header file describes generic types used in the API
 */

#ifndef _VTSS_TYPES_H_
#define _VTSS_TYPES_H_

#include <vtss_options.h>

/*
 * This determines whether to use integer standard types as defined by
 * <stdint.h>. If a particular compiler has this, but the check below
 * fails, you can either define VTSS_USE_STDINT_H or instruct the
 * compiler to turn on C99 mode or similar. (GCC: -std=gnu99).
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) || \
    defined(__linux__) || defined(__unix__) || defined(VTSS_USE_STDINT_H)

#include <stdint.h>
#include <inttypes.h>

/** \brief C99 Integer types */
typedef int8_t             i8;   /**<  8-bit signed */
typedef int16_t            i16;  /**< 16-bit signed */
typedef int32_t            i32;  /**< 32-bit signed */
typedef int64_t            i64;  /**< 64-bit signed */

typedef uint8_t            u8;   /**<  8-bit unsigned */
typedef uint16_t           u16;  /**< 16-bit unsigned */
typedef uint32_t           u32;  /**< 32-bit unsigned */
typedef uint64_t           u64;  /**< 64-bit unsigned */

typedef uint8_t            BOOL; /**< Boolean implemented as 8-bit unsigned */

#else  /* Apparently cannot use stdint.h, fallback */

/** \brief Fallback Integer types */
typedef signed char        i8;   /**<  8-bit signed */
typedef signed short       i16;  /**< 16-bit signed */
typedef signed int         i32;  /**< 32-bit signed */
typedef signed long long   i64;  /**< 64-bit signed */

typedef unsigned char      u8;   /**<  8-bit unsigned */
typedef unsigned short     u16;  /**< 16-bit unsigned */
typedef unsigned int       u32;  /**< 32-bit unsigned */
typedef unsigned long long u64;  /**< 64-bit unsigned */

typedef unsigned char      BOOL; /**< Boolean implemented as 8-bit unsigned */
typedef unsigned int       uintptr_t; /**< Unsigned integer big enough to hold pointers */

#endif

/* Formatting defaults if no inttypes available */

#if !defined(PRIu64)
#define PRIu64 "llu"           /**< Fallback un-signed 64-bit formatting string */
#endif

#if !defined(PRIi64)
#define PRIi64 "lli"           /**< Fallback signed 64-bit formatting string */
#endif

#if !defined(PRIx64)
#define PRIx64 "llx"           /**< Fallback hex 64-bit formatting string */
#endif

#define VTSS_BIT64(x)                  (1ULL << (x))                           /**< Set one bit in a 64-bit mask               */
#define VTSS_BITMASK64(x)              ((1ULL << (x)) - 1)                     /**< Get a bitmask consisting of x ones         */
#define VTSS_EXTRACT_BITFIELD64(x,o,w) (((x) >> (o)) & VTSS_BITMASK64(w))      /**< Extract w bits from bit position o in x    */
#define VTSS_ENCODE_BITFIELD64(x,o,w)  (((u64)(x) & VTSS_BITMASK64(w)) << (o)) /**< Place w bits of x at bit position o        */
#define VTSS_ENCODE_BITMASK64(o,w)     (VTSS_BITMASK64(w) << (o))              /**< Create a bitmask of w bits positioned at o */

#if !defined(TRUE)
#define TRUE  1 /**< True boolean value */
#endif
#if !defined(FALSE)
#define FALSE 0 /**< False boolean value */
#endif

/** \brief Error code type */
typedef int vtss_rc;

/** \brief Error codes */
enum
{
    VTSS_RC_OK                                  =  0,  /**< Success */
    VTSS_RC_ERROR                               = -1,  /**< Unspecified error */
    VTSS_RC_INV_STATE                           = -2,  /**< Invalid state for operation */
    VTSS_RC_INCOMPLETE                          = -3,  /**< Incomplete result */
    VTSS_RC_ERR_MACSEC_NO_SCI                   = -4,  /**< MACSEC Could not find SC (from sci) */
    VTSS_RC_ERR_MACSEC_INVALID_SCI_MACADDR      = -5,   /**< From IEEE 802.1AE-2006, section 9.9 - The 64-bit value FF-FF-FF-FF-FF-FF is never used as an SCI and is reserved for use by implementations to indicate the absence of an SC or an SCI in contexts where an SC can be present */
    VTSS_RC_ERR_CLK_CONF_NOT_SUPPORTED          = -6, /**< The PHY doesn't support clock configuration (for SynceE) */
    VTSS_RC_ERR_KR_CONF_NOT_SUPPORTED           = -7, /**< The PHY doesn't support 10GBASE_KR equalization */
    VTSS_RC_ERR_KR_CONF_INVALID_PARAMETER       = -8, /**< One of the parameters are out of range */
    VTSS_RC_ERR_PHY_BASE_NO_NOT_FOUND           = -50, /**< Port base number (first port within a chip) is not found */
    VTSS_RC_ERR_PHY_6G_MACRO_SETUP              = -51, /**< Setup of 6G macro failed */
    VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED      = -52, /**< PHY does not support the selected media mode */
    VTSS_RC_ERR_PHY_CLK_CONF_NOT_SUPPORTED      = -53, /**< The PHY doesn't support clock configuration (for SynceE) */
    VTSS_RC_ERR_PHY_GPIO_ALT_MODE_NOT_SUPPORTED = -54, /**< The PHY doesn't support the alternative mode for the selected GPIO pin*/
    VTSS_RC_ERR_PHY_GPIO_PIN_NOT_SUPPORTED      = -55, /**< The PHY doesn't support the selected GPIO pin */
}; // Leave it anonymous.

/***************************************************/
/*******   Common types                            */
/***************************************************/

/** \brief Instance identifier */
typedef struct vtss_state_s *vtss_inst_t;

/**
 * \brief Description: Event type.
 * When a variable of this type is used as an input parameter, the API will set the variable if the event has occured.
 * The API will never clear the variable. If is up to the application to clear the variable, when the event has been handled.
 **/
typedef BOOL vtss_event_t;

/** \brief Policer packet rate in PPS */
typedef u32 vtss_packet_rate_t;

#define VTSS_PACKET_RATE_DISABLED 0xffffffff /**< Special value for disabling packet policer */

/** \brief Port Number */
typedef u32 vtss_port_no_t;

/** \brief Physical port number */
typedef u32 vtss_phys_port_no_t;

/**
 * \brief Memory allocation flags.
 *
 * The VTSS API asks the application to
 * allocate dynamic memory for its internal structures
 * through calls to VTSS_OS_MALLOC().
 *
 * The application should normally just associate
 * this with a call to malloc() or kmalloc()
 * depending on the OS and the runtime model (API running
 * in Kernel or User space).
 *
 * However, on some OSs, it's required to allocate
 * specially if the memory is going to be associated
 * with DMA, hence the VTSS_MEM_FLAGS_DMA enumeration.
 *
 * Also, to be able to support warm restart, another
 * enumeration, VTSS_MEM_FLAGS_PERSIST, tells
 * the application to allocate the memory in a part
 * of RAM that won't be affected by a subsequent boot.
 *
 * VTSS_OS_MALLOC() must not block or make waiting points
 * if called with flags != VTSS_MEM_FLAGS_NONE.
 *
 * Each of the enumerations are ORed together to form
 * the final flags that are used in a call to VTSS_OS_MALLOC().
 *
 * The same set of flags are used in calls to VTSS_OS_FREE().
 */
typedef enum {
    VTSS_MEM_FLAGS_NONE    = 0x0, /**< Allocate normally according to runtime model (User or Kernel space). */
    VTSS_MEM_FLAGS_DMA     = 0x1, /**< Allocate memory that can be used with a DMA.                         */
    VTSS_MEM_FLAGS_PERSIST = 0x2, /**< Allocate memory that will survive a warm restart.                    */
} vtss_mem_flags_t;

#define VTSS_PORT_COUNT 1 /**< Default number of ports */

#if defined(VTSS_CHIP_SERVAL_LITE)
#if (VTSS_PORT_COUNT < 7)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 7 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 7 */
#endif /* SERVAL_LITE */

#if defined(VTSS_CHIP_SPARX_III_10) || defined(VTSS_CHIP_SPARX_III_10_UM) || \
    defined(VTSS_CHIP_SPARX_III_10_01) || defined(VTSS_CHIP_SEVILLE)
#if (VTSS_PORT_COUNT < 10)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 10 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 10 */
#endif /* SPARX_III_10 */

#if defined(VTSS_CHIP_CARACAL_1) || defined(VTSS_CHIP_SERVAL) || defined(VTSS_CHIP_SPARX_III_11)
#if (VTSS_PORT_COUNT < 11)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 11 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 11 */
#endif /* CARACAL_1/SERVAL */

#if defined(VTSS_CHIP_SPARX_II_16) 
#if (VTSS_PORT_COUNT < 16)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 16 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 16 */
#endif /* SPARX_II_16 */

#if defined(VTSS_CHIP_SPARX_III_17_UM)
#if (VTSS_PORT_COUNT < 17)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 17 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 17 */
#endif /* SPARX_III_17_UM */

#if defined(VTSS_CHIP_SPARX_III_18)
#if (VTSS_PORT_COUNT < 18)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 18 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 18 */
#endif /* SPARX_III_18 */

#if defined(VTSS_CHIP_SPARX_II_24) || defined(VTSS_CHIP_SPARX_III_24)
#if (VTSS_PORT_COUNT < 24)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 24 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 24 */
#endif /* SPARX_II_24 || SPARX_III_24 */

#if defined(VTSS_CHIP_SPARX_III_25_UM) 
#if (VTSS_PORT_COUNT < 25)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 25 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 25 */
#endif /* SPARX_III_25_UM */

#if defined(VTSS_CHIP_SPARX_III_26) || defined(VTSS_CHIP_CARACAL_2)
#if (VTSS_PORT_COUNT < 26)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 26 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 26 */
#endif /* SPARX_III_26 */

#if defined(VTSS_CHIP_E_STAX_34)
#if VTSS_OPT_INT_AGGR
#if (VTSS_PORT_COUNT < 26)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 26  /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 26 */
#else
#if (VTSS_PORT_COUNT < 28)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 28  /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 28 */
#endif /* VTSS_OPT_INT_AGGR */
#endif /* E_STAX_34 */

#if defined(VTSS_CHIP_BARRINGTON_II)
#if (VTSS_PORT_COUNT < 24)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 26 /**< Number of ports incl. host port */
#endif /* VTSS_PORT_COUNT < 24 */
#endif /* VTSS_CHIP_BARRINGTON_II */

#if defined(VTSS_CHIP_SCHAUMBURG_II)
#if (VTSS_PORT_COUNT < 13)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 13 /**< Number of ports incl. host port */
#endif /* VTSS_PORT_COUNT < 13 */
#endif /* VTSS_CHIP_SCHAUMBURG_II */

#if defined(VTSS_CHIP_EXEC_1)
#if (VTSS_PORT_COUNT < 3)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 3 /**< Number of ports incl. host port */
#endif /* VTSS_PORT_COUNT < 2 */
#endif /* VTSS_CHIP_EXEC_1 */

#if defined(VTSS_CHIP_DAYTONA) || defined(VTSS_CHIP_TALLADEGA)
#if (VTSS_PORT_COUNT < 4)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 4 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 2 */
#endif /* VTSS_CHIP_DAYTONA */

/* 12x1G + 4x2.5G + 1x10G + NPI (port mux mode 1) */
#if defined(VTSS_CHIP_CE_MAX_12)
#if (VTSS_PORT_COUNT < 18)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 18 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 18 */
#endif /* VTSS_CHIP_CE_MAX_12 */

/* 23x1G + 4x2.5G + 3x10G + NPI (port mux mode 1) */
#if defined(VTSS_CHIP_CE_MAX_24)
#if (VTSS_PORT_COUNT < 31)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 31 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 31 */
#endif /* VTSS_CHIP_CE_MAX_24 */

/* 12x1G + 4x2.5G + 3x10G + NPI (port mux mode 1) */
#if defined(VTSS_CHIP_LYNX_1)
#if (VTSS_PORT_COUNT < 20)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 20 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 20 */
#endif /* VTSS_CHIP_LYNX_1 */

/* 24x1G + 2x10G + NPI (port mux mode 0) */
#if defined(VTSS_CHIP_E_STAX_III_48)
#if (VTSS_PORT_COUNT < 27)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 27 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 27 */
#endif /* VTSS_CHIP_E_STAX_III_48 */

/* 24x1G + 4x10G + NPI (port mux mode 0) */
#if defined(VTSS_CHIP_E_STAX_III_68)
#if (VTSS_PORT_COUNT < 29)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 29 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 29 */
#endif /* VTSS_CHIP_E_STAX_III_68 */

/* 23x1G + 4x2.5G + 3x10G + NPI (port mux mode 1) */
#if defined(VTSS_CHIP_JAGUAR_1)
#if (VTSS_PORT_COUNT < 31)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 31 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 31 */
#endif /* VTSS_CHIP_JAGUAR_1 */

/* 23x1G + 4x2.5G + NPI on each device (port mux mode 1) */
#if defined(VTSS_CHIP_E_STAX_III_24_DUAL)
#if (VTSS_PORT_COUNT < 56)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 56 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 56 */
#endif /* VTSS_CHIP_E_STAX_III_24_DUAL */

/* 24x1G + 2x10G + NPI on each device (port mux mode 0) */
#if defined(VTSS_CHIP_E_STAX_III_68_DUAL)
#if (VTSS_PORT_COUNT < 54)
#undef VTSS_PORT_COUNT
#define VTSS_PORT_COUNT 54 /**< Number of ports */
#endif /* VTSS_PORT_COUNT < 54 */
#endif /* VTSS_CHIP_E_STAX_III_68_DUAL */

/* Number of ports may optionally be less than number of chip ports */
#if VTSS_OPT_PORT_COUNT && ((VTSS_PORT_COUNT == 1) || (VTSS_OPT_PORT_COUNT < VTSS_PORT_COUNT))
#define VTSS_PORTS VTSS_OPT_PORT_COUNT /**< Number of ports */
#else
#define VTSS_PORTS VTSS_PORT_COUNT     /**< Number of ports */
#endif /* VTSS_OPT_PORT_COUNT */

/* The first logical port number is 0. */
#define VTSS_PORT_NO_NONE    (0xffffffff) /**< Port number none */
#define VTSS_PORT_NO_CPU     (0xfffffffe) /**< Port number for CPU for special purposes */
#define VTSS_PORT_NO_START   (0)          /**< Port start number */
#define VTSS_PORT_NO_END     (VTSS_PORT_NO_START+VTSS_PORTS) /**< Port end number */
#define VTSS_PORT_ARRAY_SIZE VTSS_PORT_NO_END /**< Port number array size */

#define VTSS_PORT_IS_PORT(x) ((x)<VTSS_PORT_NO_END) /**< Valid port number */

/** \brief The different interfaces for connecting MAC and PHY */
typedef enum
{
    VTSS_PORT_INTERFACE_NO_CONNECTION, /**< No connection */
    VTSS_PORT_INTERFACE_LOOPBACK,      /**< Internal loopback in MAC */
    VTSS_PORT_INTERFACE_INTERNAL,      /**< Internal interface */
    VTSS_PORT_INTERFACE_MII,           /**< MII (RMII does not exist) */
    VTSS_PORT_INTERFACE_GMII,          /**< GMII */
    VTSS_PORT_INTERFACE_RGMII,         /**< RGMII */
    VTSS_PORT_INTERFACE_TBI,           /**< TBI */
    VTSS_PORT_INTERFACE_RTBI,          /**< RTBI */
    VTSS_PORT_INTERFACE_SGMII,         /**< SGMII */
    VTSS_PORT_INTERFACE_SGMII_CISCO,   /**< SGMII using Cisco aneg  */
    VTSS_PORT_INTERFACE_SERDES,        /**< SERDES */
    VTSS_PORT_INTERFACE_VAUI,          /**< VAUI */
    VTSS_PORT_INTERFACE_100FX,         /**< 100FX */
    VTSS_PORT_INTERFACE_XAUI,          /**< XAUI */
    VTSS_PORT_INTERFACE_RXAUI,         /**< RXAUI */
    VTSS_PORT_INTERFACE_XGMII,         /**< XGMII */
    VTSS_PORT_INTERFACE_SPI4,          /**< SPI4 */
    VTSS_PORT_INTERFACE_QSGMII         /**< QSGMII */
} vtss_port_interface_t;

/** \brief Port speed */
typedef enum
{
    VTSS_SPEED_UNDEFINED,   /**< Undefined */
    VTSS_SPEED_10M,         /**< 10 M */
    VTSS_SPEED_100M,        /**< 100 M */
    VTSS_SPEED_1G,          /**< 1 G */
    VTSS_SPEED_2500M,       /**< 2.5G */
    VTSS_SPEED_5G,          /**< 5G or 2x2.5G */
    VTSS_SPEED_10G,         /**< 10 G */
    VTSS_SPEED_12G          /**< 12G */
} vtss_port_speed_t;

/** \brief Auto negotiation struct */
typedef struct
{
    BOOL obey_pause;               /**< This port should obey PAUSE frames */
    BOOL generate_pause;           /**< Link partner obeys PAUSE frames */
} vtss_aneg_t;                            /**< Auto negotiation result */

/** \brief Port status parameter struct */
typedef struct
{
    vtss_event_t      link_down;       /**< Link down event occurred since last call */
    BOOL              link;            /**< Link is up. Remaining fields only valid if TRUE */
    vtss_port_speed_t speed;           /**< Speed */
    BOOL              fdx;             /**< Full duplex */
    BOOL              remote_fault;    /**< Remote fault signalled */
    BOOL              aneg_complete;   /**< Autoneg completed (for clause_37 and Cisco aneg) */
    BOOL              unidirectional_ability; /**<TRUE: PHY able to transmit from media independent interface regardless of whether the PHY has 
                                                 determined that a valid link has been established.FALSE: PHY able to transmit from media 
                                                 independent interface only when the PHY has determined that a valid link has been established. 
                                                 Note This bit is only applicable to 100BASE-FX and 1000BASE-X fiber media modes.*/
    vtss_aneg_t aneg;                  /**< Auto negotiation result */
    BOOL mdi_cross;                    /**< Indication of if Auto-MDIX crossover is performed */
    BOOL fiber;                        /**< Indication of if the link is a fiber link, TRUE if link is a fiber link. FALSE if link is cu link */
} vtss_port_status_t;

#if defined(VTSS_FEATURE_PORT_CONTROL)
/** \brief Serdes macro mode */
typedef enum
{
    VTSS_SERDES_MODE_DISABLE,   /**< Disable serdes */
    VTSS_SERDES_MODE_XAUI_12G,  /**< XAUI 12G mode  */
    VTSS_SERDES_MODE_XAUI,      /**< XAUI 10G mode  */
    VTSS_SERDES_MODE_RXAUI,     /**< RXAUI 10G mode */
    VTSS_SERDES_MODE_RXAUI_12G, /**< RXAUI 12G mode */
    VTSS_SERDES_MODE_2G5,       /**< 2.5G mode      */
    VTSS_SERDES_MODE_QSGMII,    /**< QSGMII mode    */
    VTSS_SERDES_MODE_SGMII,     /**< SGMII mode     */
    VTSS_SERDES_MODE_100FX,     /**< 100FX mode     */
    VTSS_SERDES_MODE_1000BaseX  /**< 1000BaseX mode */
} vtss_serdes_mode_t;
#endif //VTSS_FEATURE_PORT_CONTROL

/** \brief VLAN Identifier */
typedef u16 vtss_vid_t; /* 0-4095 */

#define VTSS_VID_NULL     ((const vtss_vid_t)0)     /**< NULL VLAN ID */
#define VTSS_VID_DEFAULT  ((const vtss_vid_t)1)     /**< Default VLAN ID */
#define VTSS_VID_RESERVED ((const vtss_vid_t)0xFFF) /**< Reserved VLAN ID */
#define VTSS_VIDS         ((const vtss_vid_t)4096)  /**< Number of VLAN IDs */
#define VTSS_VID_ALL      ((const vtss_vid_t)0x1000)/**< Untagged VID: All VLAN IDs */

/** \brief Ethernet Type **/
typedef u16 vtss_etype_t;

#define VTSS_ETYPE_VTSS 0x8880 /**< Vitesse Ethernet Type */

/** \brief MAC Address */
typedef struct
{
    u8 addr[6];   /**< Network byte order */
} vtss_mac_t;

#define MAC_ADDR_BROADCAST {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}  /**< Broadcast address used for addr in the vtss_mac_t struct */


typedef u32 vtss_isdx_t;   /**< Ingress Service Index type */
#define VTSS_ISDX_NONE (0) /**< Ingress Service Index number none */

#if defined(VTSS_FEATURE_QOS)
/** \brief Priority number **/
typedef u32 vtss_prio_t;

#define VTSS_PRIOS 1 /**< Default number of priorities */

#if defined(VTSS_ARCH_LUTON28)
#undef VTSS_PRIOS
#define VTSS_PRIOS 4 /**< Number of priorities */
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_ARCH_B2)      || defined(VTSS_ARCH_JAGUAR_1) || \
    defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)   || \
    defined(VTSS_ARCH_JAGUAR_2)
#undef VTSS_PRIOS
#define VTSS_PRIOS 8 /**< Number of priorities */
#endif /* VTSS_ARCH_B2 || VTSS_ARCH_JAGUAR_1 || ... */

#define VTSS_PRIO_START (0)                          /**< Priority start number (lowest) */
#define VTSS_PRIO_END   (VTSS_PRIO_START+VTSS_PRIOS) /**< Priority end number */
#define VTSS_PRIO_ARRAY_SIZE VTSS_PRIO_END           /**< Priority number array size */

/** \brief Queue number */
typedef u32 vtss_queue_t;

#define VTSS_QUEUES      VTSS_PRIOS                     /**< Number of queues */
#define VTSS_QUEUE_START (0)                            /**< Queue start number */
#define VTSS_QUEUE_END   (VTSS_QUEUE_START+VTSS_QUEUES) /**< Queue end number */
#define VTSS_QUEUE_ARRAY_SIZE VTSS_QUEUE_END            /**< Queue number array size */

#endif //VTSS_FEATURE_QOS 

/** \brief Aggregation Number. */
typedef u32 vtss_aggr_no_t;
#define VTSS_AGGRS           (VTSS_PORTS/2) /**< Number of LLAGs */
#define VTSS_AGGR_NO_NONE    0xffffffff     /**< Aggregation number none */
#define VTSS_AGGR_NO_START   0              /**< Aggregation start number */
#define VTSS_AGGR_NO_END     (VTSS_AGGR_NO_START+VTSS_AGGRS) /**< Aggregation number end */

/** \brief Description: GLAG number */
typedef u32 vtss_glag_no_t;

#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_GLAGS         32         /**< Number of GLAGs */
#else
#define VTSS_GLAGS         2          /**< Number of GLAGs */
#endif
#define VTSS_GLAG_NO_NONE  0xffffffff /**< GLAG number none */
#define VTSS_GLAG_NO_START 0          /**< GLAG start number */
#define VTSS_GLAG_NO_END   (VTSS_GLAG_NO_START+VTSS_GLAGS) /**< GLAG end number */

/* Maximum 8 ports per GLAG */
#define VTSS_GLAG_PORTS           8 /**< Number of GLAG ports */
#define VTSS_GLAG_PORT_START      0 /**< GLAG port start number */
#define VTSS_GLAG_PORT_END        (VTSS_GLAG_PORT_START+VTSS_GLAG_PORTS) /**< GLAG port end number */
#define VTSS_GLAG_PORT_ARRAY_SIZE VTSS_GLAG_PORT_END /**< GLAG port array size */


#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
#if defined(VTSS_ARCH_B2)
/**
 * \brief Description: Logical port numbers on host interfaces.
 * The valid range and host port mapping depend on the host mode:
 *
 * Host mode 0 : Logical ports 0-23 map to XAUI_0 LPID 0-23
 * Host mode 1 : Logical ports 0-23 map to XAUI_1 LPID 0-23
 * Host mode 3 : Logical ports 0-11 map to XAUI_0 LPID 0-11
 *               Logical ports 12-23 map to XAUI_1 LPID 0-11
 * Host mode 4 : Logical ports 0-23 map to SPI-4 channel 0-23
 * Host mode 5 : Logical ports 0-23 map to SPI-4 channel 0-23
 * Host mode 6 : Logical ports 0-1 map to SPI-4 channel 0-1
 * Host mode 8 : Logical ports 0-47 map to SPI-4 channel 0-47
 * Host mode 9 : Logical ports 0-15 map to SPI-4 channel 0-15
 * Host mode 10: Logical ports 0-47 map to SPI-4 channel 0-47
 * Host mode 11: Logical ports 0-47 map to SPI-4 channel 0-47
 **/
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
/**
 * Host mode 0 : Default: Line port 0-23 maps to Host/Logical Ports 0-23 
 * Host mode 1 : Default: Line port 0-26 maps to Host/Logical Ports 0-26
 * Host mode 3 : Default: Line port 0-23 maps to Host/Logical Ports 0-23 
 * Host mode 4 : Default: Line port 0-26 maps to Host/Logical Ports 0-26
 **/
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

typedef u32 vtss_lport_no_t;

#define VTSS_LPORTS            48         /**< Number of logical ports */
#define VTSS_LPORT_MAP_DEFAULT 0xffffffff /**< Default: One-to-one mapping */
#define VTSS_LPORT_MAP_NONE    0x1fffffff /**< Port does not map to logical port */
#endif /* VTSS_ARCH_B2 || VTSS_ARCH_JAGUAR_1_CE_MAC*/

#if defined(VTSS_FEATURE_QOS)
/** \brief Description: UDP/TCP port number */
typedef u16 vtss_udp_tcp_t;

/** \brief Description: Tag Priority or Priority Code Point (PCP)*/
typedef u32 vtss_tagprio_t;
#define VTSS_PCPS           8                           /**< Number of PCP values */
#define VTSS_PCP_START      (0)                         /**< PCP start number */
#define VTSS_PCP_END        (VTSS_PCP_START+VTSS_PCPS)  /**< PCP end number */
#define VTSS_PCP_ARRAY_SIZE VTSS_PCP_END                /**< PCP array size */

/** \brief Description: Drop Eligible Indicator (DEI) */
typedef BOOL vtss_dei_t;
#define VTSS_DEIS           2                           /**< Number of DEI values */
#define VTSS_DEI_START      (0)                         /**< DEI start number */
#define VTSS_DEI_END        (VTSS_DEI_START+VTSS_DEIS)  /**< DEI end number */
#define VTSS_DEI_ARRAY_SIZE VTSS_DEI_END                /**< DEI array size */

/** \brief Tag Remark Mode */
typedef enum
{
    VTSS_TAG_REMARK_MODE_CLASSIFIED = 0, /**< Use classified PCP/DEI values */
    VTSS_TAG_REMARK_MODE_DEFAULT    = 2, /**< Use default (configured) PCP/DEI values */
    VTSS_TAG_REMARK_MODE_MAPPED     = 3  /**< Use mapped versions of classified QOS class and DP level */
} vtss_tag_remark_mode_t;
#endif //VTSS_FEATURE_QOS

#if defined(VTSS_FEATURE_MAC_CPU_QUEUE)
/** \brief Description: CPU Rx queue number */
typedef u32 vtss_packet_rx_queue_t;

/** \brief Description: CPU Rx group number
 *  \details This is a value in range [0; VTSS_PACKET_RX_GRP_CNT[.
 */
typedef u32 vtss_packet_rx_grp_t;

/** \brief Description: CPU Tx group number
 *  \details This is a value in range [0; VTSS_PACKET_TX_GRP_CNT[.
 */
typedef u32 vtss_packet_tx_grp_t;
#endif

#if defined(VTSS_FEATURE_QOS)
#if defined(VTSS_ARCH_LUTON28)
#define VTSS_PACKET_RX_QUEUE_CNT    4  /**< Number of Rx packet queues */
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
#undef  VTSS_PACKET_RX_QUEUE_CNT
#define VTSS_PACKET_RX_QUEUE_CNT    8  /**< Number of Rx packet queues */
#undef  VTSS_PACKET_RX_GRP_CNT
#define VTSS_PACKET_RX_GRP_CNT      2  /**< Number of Rx packet groups to which any queue can map */
#undef  VTSS_PACKET_TX_GRP_CNT
#define VTSS_PACKET_TX_GRP_CNT      2  /**< Number of Tx packet groups */
#endif /* VTSS_ARCH_LUTON26/SERVAL */

#if defined(VTSS_ARCH_JAGUAR_2)
// JR2-TBD: Need correct values; this is cut-and-paste from Serval
#undef  VTSS_PACKET_RX_QUEUE_CNT
#define VTSS_PACKET_RX_QUEUE_CNT    8  /**< Number of Rx packet queues */
#undef  VTSS_PACKET_RX_GRP_CNT
#define VTSS_PACKET_RX_GRP_CNT      2  /**< Number of Rx packet groups to which any queue can map */
#undef  VTSS_PACKET_TX_GRP_CNT
#define VTSS_PACKET_TX_GRP_CNT      2  /**< Number of Tx packet groups */
#endif /* VTSS_ARCH_JAGUAR_2 */

#if defined(VTSS_ARCH_JAGUAR_1)
#undef VTSS_PACKET_RX_QUEUE_CNT
#define VTSS_PACKET_RX_QUEUE_CNT   10  /**< Number of Rx packet queues. The last two are only usable as super priority queues. */
#undef  VTSS_PACKET_RX_GRP_CNT
#define VTSS_PACKET_RX_GRP_CNT      4  /**< Number of Rx packet groups to which any queue can map */
#undef  VTSS_PACKET_TX_GRP_CNT
#define VTSS_PACKET_TX_GRP_CNT      5  /**< Number of Tx packet groups */
#endif

#define VTSS_PACKET_RX_QUEUE_NONE  (0xffffffff) /**< Rx queue not selected for a particular type of frames */ 
#define VTSS_PACKET_RX_QUEUE_START (0)          /**< Rx queue start number */
#define VTSS_PACKET_RX_QUEUE_END   (VTSS_PACKET_RX_QUEUE_START+VTSS_PACKET_RX_QUEUE_CNT) /**< Rx queue end number */

/**
 * \brief Percentage, 0-100
 **/
typedef u8 vtss_pct_t;

/**
 * \brief Drop Precedence Level, (0..3 on Jaguar, 0..1 on Luton 26)
 **/
typedef u8 vtss_dp_level_t;

/**
 * \brief Policer/Shaper bit rate.
 * Multiply by 1000 to get the rate in BPS.
 * The rate will be rounded to the nearest value supported by the chip
 **/
typedef u32 vtss_bitrate_t;

#define VTSS_BITRATE_DISABLED 0xffffffff /**< Bitrate disabled */

/**
 * \brief Policer/shaper burst level in bytes.
 * The level will be rounded to the nearest value supported by the chip
 **/
typedef u32 vtss_burst_level_t;

/**
 * \brief Shaper
 **/
typedef struct
{
    vtss_burst_level_t level; /**< CBS (Committed Burst Size).       Unit: bytes */
    vtss_bitrate_t     rate;  /**< CIR (Committed Information Rate). Unit: kbps. Use VTSS_BITRATE_DISABLED to disable shaper */
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
    vtss_burst_level_t ebs;   /**< EBS (Excess Burst Size).          Unit: bytes */
    vtss_bitrate_t     eir;   /**< EIR (Excess Information Rate).    Unit: kbps. Use VTSS_BITRATE_DISABLED to disable DLB */
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
} vtss_shaper_t;
#endif //VTSS_FEATURE_QOS

/** \brief VDD power supply */
typedef enum {
    VTSS_VDD_1V0,               /**< 1.0V (default) */
    VTSS_VDD_1V2,               /**< 1.2V */
} vtss_vdd_t;

#if defined(VTSS_FEATURE_PORT_CONTROL)
/** \brief IPv4 address/mask */
typedef u32 vtss_ip_t;

/** \brief IPv4 address/mask */
typedef vtss_ip_t vtss_ipv4_t;

/** \brief Prefix size */
typedef u32 vtss_prefix_size_t;

/** \brief IPv6 address/mask */
typedef struct
{
    u8 addr[16]; /**< Address */
} vtss_ipv6_t;

/* NOTE: This type may be used directly in SNMP
 * InetAddressType types.  */

/** \brief IP address type */
typedef enum {
    VTSS_IP_TYPE_NONE = 0, /**< Matches "InetAddressType_unknown" */
    VTSS_IP_TYPE_IPV4 = 1, /**< Matches "InetAddressType_ipv4"    */
    VTSS_IP_TYPE_IPV6 = 2, /**< Matches "InetAddressType_ipv6"    */
} vtss_ip_type_t;

/** \brief Either an IPv4 or IPv6 address  */
typedef struct {
    vtss_ip_type_t  type; /**< Union type */
    union {
        vtss_ipv4_t ipv4; /**< IPv4 address */
        vtss_ipv6_t ipv6; /**< IPv6 address */
    } addr;               /**< IP address */
} vtss_ip_addr_t;

/** \brief IPv4 network */
typedef struct
{
    vtss_ipv4_t        address;     /**< Network address */
    vtss_prefix_size_t prefix_size; /**< Prefix size */
} vtss_ipv4_network_t;

/** \brief IPv6 network */
typedef struct
{
    vtss_ipv6_t        address;     /**< Network address */
    vtss_prefix_size_t prefix_size; /**< Prefix size */
} vtss_ipv6_network_t;

/** \brief IPv6 network */
typedef struct
{
    vtss_ip_addr_t     address;     /**< Network address */
    vtss_prefix_size_t prefix_size; /**< Prefix size */
} vtss_ip_network_t;

/** \brief Routing entry type */
typedef enum
{
    VTSS_ROUTING_ENTRY_TYPE_INVALID = 0,
    VTSS_ROUTING_ENTRY_TYPE_IPV6_UC = 1,
    VTSS_ROUTING_ENTRY_TYPE_IPV4_MC = 2,
    VTSS_ROUTING_ENTRY_TYPE_IPV4_UC = 3,
} vtss_routing_entry_type_t;

/** \brief IPv4 unicast routing entry */
typedef struct
{
    vtss_ipv4_network_t network;     /**< Network to route */
    vtss_ipv4_t         destination; /**< IP address of next-hop router.
                                          Zero if local route */
} vtss_ipv4_uc_t;

/** \brief IPv6 routing entry */
typedef struct
{
    vtss_ipv6_network_t network;     /**< Network to route */
    vtss_ipv6_t         destination; /**< IP address of next-hop router.
                                          Zero if local route */
} vtss_ipv6_uc_t;

/** \brief Routing entry */
typedef struct
{
   vtss_routing_entry_type_t type; /**< Type of route */

   union {
       vtss_ipv4_uc_t ipv4_uc;     /**< IPv6 unicast route */
       vtss_ipv6_uc_t ipv6_uc;     /**< IPv6 unicast route */
   } route;                        /**< Route */

   vtss_vid_t vlan;                /**< Link-local addresses needs to specify a
                                        egress vlan. */
} vtss_routing_entry_t;

/** \brief Routing interface statics counter */
typedef struct {
    u64 ipv4uc_received_octets;    /**< IPv4UC octets received and hardware forwarded */
    u64 ipv4uc_received_frames;    /**< IPv4UC frames received and hardware forwarded */
    u64 ipv6uc_received_octets;    /**< IPv6UC octets received and hardware forwarded */
    u64 ipv6uc_received_frames;    /**< IPv6UC frames received and hardware forwarded */

    u64 ipv4uc_transmitted_octets; /**< IPv4UC octets transmitted */
    u64 ipv4uc_transmitted_frames; /**< IPv4UC frames transmitted */
    u64 ipv6uc_transmitted_octets; /**< IPv6UC octets transmitted */
    u64 ipv6uc_transmitted_frames; /**< IPv6UC frames transmitted */
} vtss_l3_counters_t;
/* - VCAP types ---------------------------------------------------- */
#endif // VTSS_FEATURE_PORT_CONTROL

#if defined(VTSS_FEATURE_ACL)

/** \brief VCAP 1 bit */
typedef enum
{
    VTSS_VCAP_BIT_ANY, /**< Value 0 or 1 */
    VTSS_VCAP_BIT_0,   /**< Value 0 */
    VTSS_VCAP_BIT_1    /**< Value 1 */
} vtss_vcap_bit_t;

/** \brief VCAP 8 bit value and mask */
typedef struct
{
    u8 value;   /**< Value */
    u8 mask;    /**< Mask, cleared bits are wildcards */
} vtss_vcap_u8_t;

/** \brief VCAP 16 bit value and mask */
typedef struct
{
    u8 value[2];   /**< Value */
    u8 mask[2];    /**< Mask, cleared bits are wildcards */
} vtss_vcap_u16_t;

/** \brief VCAP 24 bit value and mask */
typedef struct
{
    u8 value[3];   /**< Value */
    u8 mask[3];    /**< Mask, cleared bits are wildcards */
} vtss_vcap_u24_t;

/** \brief VCAP 32 bit value and mask */
typedef struct
{
    u8 value[4];   /**< Value */
    u8 mask[4];    /**< Mask, cleared bits are wildcards */
} vtss_vcap_u32_t;

/** \brief VCAP 40 bit value and mask */
typedef struct
{
    u8 value[5];   /**< Value */
    u8 mask[5];    /**< Mask, cleared bits are wildcards */
} vtss_vcap_u40_t;

/** \brief VCAP 48 bit value and mask */
typedef struct
{
    u8 value[6];   /**< Value */
    u8 mask[6];    /**< Mask, cleared bits are wildcards */
} vtss_vcap_u48_t;

/** \brief VCAP 128 bit value and mask */
typedef struct
{
    u8 value[16];   /**< Value */
    u8 mask[16];    /**< Mask, cleared bits are wildcards */
} vtss_vcap_u128_t;

/** \brief VCAP VLAN ID value and mask */
typedef struct
{
    u16 value;   /**< Value */
    u16 mask;    /**< Mask, cleared bits are wildcards */
} vtss_vcap_vid_t;

/** \brief VCAP IP address value and mask */
typedef struct
{
    vtss_ip_t value;   /**< Value */
    vtss_ip_t mask;    /**< Mask, cleared bits are wildcards */
} vtss_vcap_ip_t; 

/** \brief VCAP UDP/TCP port range */
typedef struct
{
    BOOL           in_range;   /**< Port in range match */
    vtss_udp_tcp_t low;        /**< Port low value */
    vtss_udp_tcp_t high;       /**< Port high value */
} vtss_vcap_udp_tcp_t;

/** \brief Value/Range type */
typedef enum
{
    VTSS_VCAP_VR_TYPE_VALUE_MASK,        /**< Used as value/mask */
    VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE,   /**< Used as inclusive range: low <= range <= high */
    VTSS_VCAP_VR_TYPE_RANGE_EXCLUSIVE    /**< Used as exclusive range: range < low or range > high */
} vtss_vcap_vr_type_t;

/** \brief VCAP universal value or range type */
typedef u16 vtss_vcap_vr_value_t;

/** \brief VCAP universal value or range */
typedef struct
{
    vtss_vcap_vr_type_t type; /**< Type */
    union
    {
        struct
        {
            vtss_vcap_vr_value_t value; /**< Value */
            vtss_vcap_vr_value_t mask;  /**< Mask, cleared bits are wildcards */
        } v; /**< type == VTSS_VCAP_VR_TYPE_VALUE_MASK */
        struct
        {
            vtss_vcap_vr_value_t low;   /**< Low value */
            vtss_vcap_vr_value_t high;  /**< High value */
        } r; /**< type == VTSS_VCAP_VR_TYPE_RANGE_XXXXXX */
    } vr; /**< Value or range */
} vtss_vcap_vr_t;
#endif /* VTSS_FEATURE_ACL */

/***************************************************/
/*******   1588 types                              */
/***************************************************/

/**
 * \brief Clock adjustment rate in parts per billion (ppb) * 1<<16.
 * Range is +-2**47 ppb
 * For example, 8.25 ppb is expressed as 0x0000.0000.0008.4000 
 */
typedef i64 vtss_clk_adj_rate_t;

/**
 * \brief Time interval in ns * 1<<16
 * range +-2**47 ns = 140737 sec = 39 hours
 * For example, 2.5 ns is expressed as 0x0000.0000.0002.8000 
 */
typedef i64 vtss_timeinterval_t;

#define VTSS_ONE_MIA 1000000000 /**< One billion */
#define VTSS_ONE_MILL   1000000 /**< One million */
#define VTSS_MAX_TIMEINTERVAL 0x7fffffffffffffffLL /**< Maximum time interval */

#define VTSS_INTERVAL_SEC(t) ((i32)VTSS_DIV64((t)>>16, VTSS_ONE_MIA)) /**< One Second time interval */
#define VTSS_INTERVAL_MS(t)  ((i32)VTSS_DIV64((t)>>16, VTSS_ONE_MILL)) /**< One millisecond time interval */
#define VTSS_INTERVAL_US(t)  ((i32)VTSS_DIV64((t)>>16, 1000)) /**< One microsecond time interval */
#define VTSS_INTERVAL_NS(t)  ((i32)VTSS_MOD64((t)>>16, VTSS_ONE_MIA)) /**< This returns the ns part of the interval, not the total number of ns */
#define VTSS_INTERVAL_PS(t)  (((((i32)(t & 0xffff))*1000)+0x8000)/0x10000) /**< This returns the ps part of the interval, not the total number of ps */
#define VTSS_SEC_NS_INTERVAL(s,n) (((vtss_timeinterval_t)(n)+(vtss_timeinterval_t)(s)*VTSS_ONE_MIA)<<16)  /**< TBD */

/**
 * \brief Time stamp in seconds and nanoseconds
 */
typedef struct {
    u16 sec_msb; /**< Seconds msb */
    u32 seconds; /**< Seconds */
    u32 nanoseconds; /**< nanoseconds */
} vtss_timestamp_t;



/***************************************************/
/*******   SYNCE types                             */
/***************************************************/

#define VTSS_SYNCE_CLK_PORT_ARRAY_SIZE  2    /**< SYNCE clock out port numberarray size */
#endif /* _VTSS_TYPES_H_ */
