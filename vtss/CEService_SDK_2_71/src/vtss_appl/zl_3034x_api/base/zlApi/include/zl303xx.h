

/*******************************************************************************
*
*  $Id: zl303xx.h 8827 2012-08-14 17:28:33Z JK $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     High level header file defining components of the zl303xx API.
*
*******************************************************************************/

#ifndef ZL303XX_MAIN_H
#define ZL303XX_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

/* This should always be the first file included */
#include "zl303xx_Global.h"

/* Now include essential files */
#include "zl303xx_Error.h"
#include "zl303xx_Trace.h"
#include "zl303xx_ErrTrace.h"
#include "zl303xx_Os.h"


#include "zl303xx_ApiLowDataTypes.h"

#include "zl303xx_TodMgrTypes.h"       /* TOD to Input Sync & DCO pulse control. */
#include "zl303xx_TodMgrInternal.h"    /* For ToD types used within the zl303xx_ParamsS structure. */


/*****************   DEFINES   ************************************************/

/***** General constants *****/
#define ZL303XX_INVALID (-1)
#define ZL303XX_INVALID_STREAM ZL303XX_INVALID

/* An often used bit mask */
#define ZL303XX_1BIT_MASK ((Uint32T)0x00000001)

/* Useful constant for converting between nanoseconds and seconds */
#define TEN_e9 1000000000
#define TEN_e6 1000000
#define TEN_e3 1000

/***** Macros for checking parameters *****/
/* Macro to check for a non-null pointer. Use of the comma operator is
   intentional here */
#ifndef ZL303XX_CHECK_POINTER
#define ZL303XX_CHECK_POINTER(ptr) \
   ( ((ptr) == NULL) ?  \
     (ZL303XX_ERROR_NOTIFY("Invalid pointer: "#ptr),ZL303XX_INVALID_POINTER) : \
     ZL303XX_OK    \
   )
#endif
#ifndef ZL303XX_CHECK_POINTERS
#define ZL303XX_CHECK_POINTERS(ptr1, ptr2) \
   ( ((ptr1 == NULL) || (ptr2 == NULL)) ?  \
     (ZL303XX_ERROR_NOTIFY("Invalid pointers"),ZL303XX_INVALID_POINTER) : \
     ZL303XX_OK    \
   )
#endif

/* Macro to check that a value is within an acceptable range (MIN & MAX are OK).
   Use of the comma operator is intentional here */
#ifndef ZL303XX_CHECK_RANGE
#define ZL303XX_CHECK_RANGE(val, min, max)    \
   ( ((val < min) || (val > max)) ?    \
     (ZL303XX_ERROR_NOTIFY("Value out of range:" #val),ZL303XX_PARAMETER_INVALID) : \
     ZL303XX_OK    \
   )
#endif

/* Similar to the Range Check above but also define the Error code */
#ifndef ZL303XX_CHECK_RANGE_PLUS_ERR
#define ZL303XX_CHECK_RANGE_PLUS_ERR(val, min, max, err)    \
   ( ((val < min) || (val > max)) ?   \
     (ZL303XX_ERROR_NOTIFY("Value invalid:" #val),(zlStatusE)err) : \
     ZL303XX_OK    \
   )
#endif

/***** Physical device constants *****/
/* The maximum number of simultaneously supported devices by this API */
#ifndef ZL303XX_MAX_NUM_ZL303XX_DEVICES
   #define ZL303XX_MAX_NUM_ZL303XX_DEVICES      (Uint32T)1
#endif

/* DCO and SYSTEM Clock frequencies. */
#define ZL303XX_PLL_INTERNAL_FREQ_KHZ    65536
#define ZL303XX_PLL_SYSTEM_CLK_KHZ       80000

/* Define a min/max/default system interrupt rate. */
#define ZL303XX_MIN_LOG2_SYS_INTERRUPT_PERIOD     20  /* 2^20 / 80e6 =~ 0.0131sec (76.29Hz) */
#define ZL303XX_38HZ_LOG2_SYS_INTERRUPT_PERIOD    21  /* 2^21 / 80e6 =~ 0.0262sec (38.14Hz) */
#define ZL303XX_9HZ_LOG2_SYS_INTERRUPT_PERIOD     23  /* 2^23 / 80e6 =~ 0.1048sec ( 9.53Hz) */
#define ZL303XX_MAX_LOG2_SYS_INTERRUPT_PERIOD     26  /* 2^26 / 80e6 =~ 0.8388sec ( 1.19Hz) */
#define ZL303XX_DEFAULT_LOG2_SYS_INTERRUPT_PERIOD ZL303XX_38HZ_LOG2_SYS_INTERRUPT_PERIOD

/* Number of device interrupt outputs */
    #define ZL303XX_NUM_DEVICE_IRQS    1

/* define number of SYNTH outputs */
#define ZL303XX_PLL_NUM_SYNTHS        2
#define ZL303XX_PLL_NUM_CLK_PER_SYNTH 2
#define ZL303XX_PLL_NUM_FP_PER_SYNTH  2

/* ZL303XX_NUM_PKT_CLOCK_STREAMS is deprecated. Use ZL303XX_PTP_NUM_STREAMS_MAX instead. */
#define ZL303XX_NUM_PKT_CLOCK_STREAMS      ZL303XX_PTP_NUM_STREAMS_MAX

/******************/

/* Useful macros that make the lock/unlock operations stand out from their surrounding code */
#ifndef ZL303XX_LOCK_DEV_PARAMS
#define ZL303XX_LOCK_DEV_PARAMS(x)   zl303xx_LockDevParams(x)
#define ZL303XX_UNLOCK_DEV_PARAMS(x) (void)zl303xx_UnlockDevParams(x)
#endif

/******************/

/* define a callback function pointer for SPI chip select enabling. The
   'par' is a user configurable parameter that will be passed by the calling
   function along with the required status of the chip select*/
typedef zlStatusE (*zl303xx_SpiChipSelectT)(Uint32T par, zl303xx_BooleanE enable);

/*****************   DATA TYPES   *********************************************/
#ifdef OS_LINUX
struct zl303xx_ParamsS;   /* Fwd decl */
typedef Sint32T (*OS_DEV_FUNC_PTR) (struct zl303xx_ParamsS*);
#else
typedef Sint32T (*OS_DEV_FUNC_PTR) (void *);
#endif

typedef enum
{
    UNKNOWN_DEVICETYPE = 0,
    ZL3031X_DEVICETYPE,
    ZL3034X_DEVICETYPE,
    ZL3036X_DEVICETYPE
} zl303xx_DevTypeE;

/******************/

/* Line rates */
typedef enum
{
   ZL303XX_LAN_PORT_RATE_10M,
   ZL303XX_LAN_PORT_RATE_100M,
   ZL303XX_LAN_PORT_RATE_1G
} zl303xx_LanLineRateE;

/******************/

/* Device initialization state */
typedef enum
{
   ZL303XX_INIT_STATE_NONE = 0,    /* Not yet initialized */
   ZL303XX_INIT_STATE_CLOSE_DOWN = 1, /* Device is being closed down */
   ZL303XX_INIT_STATE_DEV_RESET,   /* Device has been successfully reset */
   ZL303XX_INIT_STATE_LAN_INIT_IN_PROGRESS,
   ZL303XX_INIT_STATE_LAN_INIT_DONE,
   ZL303XX_INIT_STATE_PLL_INIT_IN_PROGRESS,
   ZL303XX_INIT_STATE_PLL_INIT_DONE,
   ZL303XX_INIT_STATE_TSENG_INIT_IN_PROGRESS,
   ZL303XX_INIT_STATE_TSENG_INIT_DONE,
   ZL303XX_INIT_STATE_DONE         /* All initialization completed successfully */
} zl303xx_InitStateE;

/******************/

typedef struct
{
   zl303xx_SynthConfigS config;
   zl303xx_SynthClkConfigS clkConfig[2];
   zl303xx_SynthFpConfigS fpConfig[2];
} zl303xx_SynthSettingsS;

/*****************   DATA STRUCTURES   ****************************************/

typedef struct
{
   zl303xx_SpiChipSelectT csFuncPtr;
   Uint32T csFuncPar;
} zl303xx_ChipSelectCtrlS;

/******************/

/* enum to determine the protocol/format of the inserted timestamp */
typedef enum
{
   ZL303XX_TS_FORMAT_PTP = 0,
   ZL303XX_TS_FORMAT_NTP = 1,
   ZL303XX_TS_FORMAT_RTP = 2
} zl303xx_TsEngTsProtocolE;

/******************/

/* enum to determine the frequency of the inserted timestamp when RTP protocol
   is used */
typedef enum
{
   ZL303XX_INVALID_FREQ = 0,
   ZL303XX_TS_RTP_FREQ_8_192M = 8192,   /* 8.192MHz */
   ZL303XX_TS_RTP_FREQ_10M = 10000
} zl303xx_RtpTsFreqE;

#define MIN_RTP_CLOCK_FREQ  1544
#define MAX_RTP_CLOCK_FREQ  10000

/******************/

typedef enum    /* If you add more RAW ipv6 protocols then add them to zl303xx_IsIPv6ProtocolType() as well! */
{
   /* Datagram socket protocols. */
   ZL303XX_ETH_IPV4_UDP_PTPV1 = 0, /* PTP protocols */
   ZL303XX_ETH_IPV4_UDP_PTPV2,
   ZL303XX_ETH_VLAN_IPV4_UDP_PTPV2_SKT,
   ZL303XX_ETH_IPV6_UDP_PTPV2_SKT,
   ZL303XX_ETH_VLAN_IPV6_UDP_PTPV2_SKT,

   ZL303XX_ETH_IPV4_UDP_RTP = 10,       /* RTP protocols */
   ZL303XX_ETH_IPV4_UDP_PW_RTP,
   ZL303XX_ETH_IPV4_UDP_RTP_PW,
   ZL303XX_ETH_IPV4_UDP_PW,
   ZL303XX_ETH_IPV4_UDP_RTP_PW_ALT,

   /* Raw socket protocols. */
   ZL303XX_PROTOCOL_RAW_PTP_START = 20,  /* PTP protocols */
   ZL303XX_ETH_DSA_IPV4_UDP_PTPV2 = ZL303XX_PROTOCOL_RAW_PTP_START,
   ZL303XX_ETH_VLAN_IPV4_UDP_PTPV2,
   ZL303XX_ETH_VLAN_VLAN_IPV4_UDP_PTPV2,
   ZL303XX_ETH_MPLS_IPV4_UDP_PTPV2,
   ZL303XX_ETH_PTPV2,
   ZL303XX_ETH_IPV6_UDP_PTPV2,             /* 25 */
   ZL303XX_ETH_CUST_ETH_IPV4_UDP_PTPV2,
   ZL303XX_ETH_CUST_ETH_PTPV2,
   ZL303XX_ETH_CUST_ETH_IPV6_UDP_PTPV2,
   ZL303XX_ETH_VLAN_PTPV2,
   ZL303XX_PROTOCOL_RAW_PTP_END = ZL303XX_ETH_VLAN_PTPV2,

   ZL303XX_PROTOCOL_RAW_RTP_START = 40,  /* RTP protocols */
   ZL303XX_ETH_IPV6_UDP_RTP = ZL303XX_PROTOCOL_RAW_RTP_START,
   ZL303XX_ETH_MPLS_PW_RTP,
   ZL303XX_ETH_MPLS_PW,
   ZL303XX_ETH_MPLS_MPLS_RTP,
   ZL303XX_ETH_MPLS_MPLS_PW_RTP,
   ZL303XX_ETH_MPLS_MPLS_PW,               /* 45 */
   ZL303XX_ETH_MPLS_RTP,
   ZL303XX_ETH_ECID_PW_RTP,
   ZL303XX_ETH_VLAN_RTP,
   /* Customized protocols, using proprietary 8 byte header for routing */
   ZL303XX_ETH_CUSTOM8_RTP_PW,
   ZL303XX_ETH_CUSTOM8_PW_RTP,
   ZL303XX_PROTOCOL_RAW_RTP_END = ZL303XX_ETH_CUSTOM8_PW_RTP,

   /* Illegal - this should remain the last entry in the
      enumeration for range checking purposes in the implementation. */
   ZL303XX_PROTOCOL_NUM_TYPES

} zl303xx_ProtocolTypesE;

/******************/

typedef enum
{
   ZL303XX_UPDATE_NONE = 0,
   ZL303XX_UPDATE_1HZ = 1,
   ZL303XX_UPDATE_SYS_TIME = 2,
   ZL303XX_UPDATE_SYS_INTRVL = 3
} zl303xx_UpdateTypeE;

#define ZL303XX_CHECK_UPDATE_TYPE(X) \
      ((((zl303xx_UpdateTypeE)(X) > ZL303XX_UPDATE_SYS_INTRVL)) ? \
         (ZL303XX_ERROR_NOTIFY("Invalid Update Mode: " #X),ZL303XX_PARAMETER_INVALID) :  \
         ZL303XX_OK)

/******************/

/* Define the number of elements in the circular array as a power of 2 since the
   MASK will be used to handle the roll-over of the indexes. */
#define ZL303XX_NUM_TS_SAMPLES              (Uint8T)64
#define ZL303XX_TS_SAMPLES_ROLLOVER_MASK    (Uint8T)(ZL303XX_NUM_TS_SAMPLES - 1)

typedef Sint32T (*hwFuncPtrTODDone)(void*);

typedef struct
{
   /* System timestamp: used for the Rx/Tx timestamps */
   Uint64S localTs;

   /* The insertTs when RTP protocol is in use */
   /* Kept in SEC:TICKS format */
   Uint64S rtpTs;

   /* Units are protocol dependent:
      PTP/NTP - units of seconds32:nanoseconds32.
      RTP - upper byte = 0: lower byte in ticks of the RTP period  */
   /* This is the raw value read from the register.               */
   Uint64S rawInsertTs;

   /* The insert timestamp converted to units of SEC:NANO   */
   /* A more accurate name would be 'nsTs' or 'todTs'       */
   Uint64S insertTs;

   /* DCO */
   Uint64S dcoTs;       /* DCO Ticks32:Phase32 => the raw sampled value    */
   Uint64S dcoExtendTs; /* DCO Ticks64 (extended to 64-bit via software)   */

   /* DCO offset at the time the sample was taken. */
   Sint32T dcoFreq;

   /* Predicted tick ratios for this sample offset. Used to convert between
    * system clock counts and adjusted clock frequencies. */
   Uint32T dcoDelta;    /* Predicted #DCO ticks in the next SYSTEM interval. */
   Uint32T nsDelta;     /* Predicted #Nanoseconds in the next SYSTEM interval.*/
   Uint32T insertDelta; /* Predicted #INSERT timestamp ticks in the next SYSTEM
                         *    interval. */

   /* CPU */
   Uint32T osTimeTicks; /* Value of the OS tick counter at the sample point */
   Uint64S cpuHwTime;   /* CPU HW timestamp at the sample point (if CPU HW
                           time-stamping is enabled). */

} zl303xx_TsSampleS;

/******************/

/* Main device instance structure. Contains internal information on the configuration
   and state of the device */
typedef struct
{
   /* Device parameters */
   Uint32T  deviceHandle;
   Uint32T  deviceId;
   Uint32T  deviceRev;

   zl303xx_InitStateE  initState;

   /* Device interrupt information */
   Uint8T isrMask[ZL303XX_NUM_DEVICE_IRQS];

   /* System Clock variables */
   Uint32T sysClockFreqHz;    /* The system clock rate */
   Uint32T sysClockPeriod;    /* The system clock period (ScaledNs32T) */

   /* DCO Clock variables */
   Uint32T dcoClockFreqHz;    /* The system clock rate */
   Uint32T dcoClockPeriod;    /* The system clock period (ScaledNs32T) */

   /******************/

   /* SPI state information */
   struct
   {
   #ifdef OS_LINUX
      #define MAX_DEV_NAME_LEN 32
      Uint8T linuxChipSelectDevName[MAX_DEV_NAME_LEN];
      Uint8T linuxChipHighIntrDevName[MAX_DEV_NAME_LEN];
      Uint8T linuxChipLowIntrDevName[MAX_DEV_NAME_LEN];
      Uint16T linuxHighIntrSignal;
      Uint16T linuxLowIntrSignal;
      /* SPI internal parameters/state variables */
   #endif
   #ifndef OS_LINUX
      zl303xx_ChipSelectCtrlS chipSelect;
   #endif
      Uint16T currentPage;
      zl303xx_DevTypeE deviceType;
   } spiParams;

   /******************/

   struct
   {
      /* Packet info based on Network type/configuration. */
      Uint16T  maxPktSize;       /* Maximum packet size for this LAN port */

      /* Combined length of all packet headers */
      Uint16T  pktHeaderSize;

      zlStatusE (*linkUpDownUserFn)(Uint8T port, Uint8T linkUp);


      Uint32T schedReservedGranules;   /* The number of internal memory granules
                                          reserved for the scheduler */
   } lanParams;

   /******************/


   /******************/

   struct
   {
      /* Insert Timestamp protocol format and parameters */
      zl303xx_TsEngTsProtocolE tsProtocol;     /* TsEng insert protocol format */
      Uint32T insertFreqHz;      /* INSERT timestamp frequency (Hz) */
      ScaledNs32T insertPeriod;  /* INSERT timestamp period (Scaled Ns) */
      Uint16T tsSizeBytes;       /* INSERT timestamp size (bytes) */

      /* Timestamp Engine Interrupt Configuration */
      Uint8T   isrEnableMask;    /* Which timestamp interrupts are enabled */

      /* Insert timestamp control parameters */
      Uint32T  txCtrlWordLoc;          /* Location of the Tx control word */
      Uint32T  txTsLocation;           /* Location of the Tx timestamp */
      Uint32T  txSchTimeLocation;      /* Location of the Tx scheduled launch time */
      Uint32T  rxTsLocation;           /* Location of the Rx timestamp */
      zl303xx_BooleanE rxTsEnabled;    /* True if Rx timing packets will be timestamped */
      Uint32T  lastRxTimestamp;        /* The Rx timestamp from the the last Rx packet */
      Uint32T  lastTxTimestamp;        /* The Tx timestamp of the last Tx packet */
      Uint32T  aprLastRxTimestamp;    /* The Rx timestamp from the the last Rx packet */
      Uint32T  aprLastTxTimestamp;    /* The Tx timestamp from the the last Tx packet */
      Uint32T  udpChksumLocation;      /* Location of the UDP checksum, if used */
      zl303xx_BooleanE udpChksumEnable;/* True if UDP checksum will be adjusted when packet
                                          is Tx to allow for inserted timestamp */

      /* Transmit timestamp parameters */
      Uint8T   pktIndex;   /* The current Tx pkt index (when recording exit timestamps) */

      /* Timestamp Engine Sampling Configuration Parameters */
      zl303xx_UpdateTypeE sampleMode;      /* How often local clocks are sampled */
      zl303xx_UpdateTypeE dcoUpdateMode;   /* How often the DCO is updated (& sample generated) */

      /* Interval parameters related to the frequency at which samples of the
         local clocks are taken. (Useful for uniform sample periods).   */
      Uint8T   sampleIntervalValue;    /* Raw interval value in the device register */
      Uint32T  sampleIntervalInSysClk; /* Sample interval in system clock counts */

      /* Structures used to store the collected Timestamp Engine Samples.
         These maintain a continuous clock on the local node, accounting for
         counter rollover and protocol conversions. */
      zl303xx_BooleanE samplesReady;   /* A sample is ready to be processed */

      Uint8T currIndex;        /* Index to the most recent sample taken by TsEng */
      Uint8T sampleCurrIndex;  /* Current Sample to be processed by the APR */

      zl303xx_TsSampleS samples[ZL303XX_NUM_TS_SAMPLES];  /* Array of the last samples collected */

      /* Extends the TOD above the 32-bit TOD seconds count (to 48-bits) */
      Uint16T epochCount;
      /* Current Time-of-Day in 32-bit Seconds 32-bit nanoseconds */
      Uint64S currentTime;
      Uint32T currentSys;


      /* function pointer to routine called after the TOD manager has finished
         changing time */
      hwFuncPtrTODDone TODdoneFuncPtr;


      /******  DEPRECATED MEMBERS ******/
      /* The following members are marked for removal from this sub-structure.
       * Users should refrain from accessing them directly or re-map to the new
       * item where applicable.     */

      /* Timestamp Pulse Alignment Control */
      zl303xx_TodAlignmentE frmPulseAlignment;    /* MOVED to todMgr:onePpsAlignment */

      /* Indicate that the device Time-of-day has changed. */
      Sint32T  todUpdated;                      /* REMOVED due to UNUSED */

      /******  DEPRECATED MEMBERS (END) ******/
   } tsEngParams;

   /******************/
   /* Parameters used to convert timestamp samples from the SYSTEM clock domain
    * to frequencies associated with the core DCO clock (which is typically at
    * some offset from the local SYSTEM clock. */
   struct
   {
      zl303xx_BooleanE initialized;   /* True if the first sample has been taken
                                       * on this device. */
      struct
      {
         /* When converting a system timestamp to another clock domain (and vice
          * versa), the converted value is interpolated using the last sample
          * point and the slope of the relation between the the 2 clocks
          * (determined at sample time from the sampled freqOfsetUppm value). To
          * speed up the math operations (and avoid 64-bit division) the system
          * interval is set to a power of 2 so that shifting can be used
          * (regardless of the actual sample delta).
          */
         Uint32T log2SysTicks;   /* For 80MHz, = 26 (2^26 = maximum SYS ticks
                                  * still less than 1 second). */

         /* When converting from one frequency to another, the following common
          * ratio is used:
          *
          *    clkDelta     sysDelta                             sysDelta
          *   ---------- = ----------  SO  clkDelta = clkFreq * ----------
          *    clkFreq      sysFreq                              sysFreq
          *
          * To speed up the arithmetic define sysDelta/sysFreq as a fractional
          * constant.
          */
         Uint32T sysFreqFrac;       /* 2^26 / 80MHz = 0.8388608 */
                                    /* 0.8388608 * 2^32 = 0xD6BF94D6 */
      } convIntvl;

   } tsEngSample;

   /******************/
   /* Parameters used to manage the Time-of-Day settings on the device.  */
   zl303xx_TodMgrS todMgr;

   /******************/
   struct
   {
      zl303xx_DpllConfigS config[ZL303XX_DPLL_NUM_DPLLS];

      Sint32T dcoCountPerPpm;

      zl303xx_BooleanE ref1HzDetectEnable;

      /* dcoFreq is stored as ppm x 1e6 */
      Sint32T dcoFreq;


      zl303xx_SynthSettingsS synth[ZL303XX_PLL_NUM_SYNTHS];
   } pllParams;

   /* APR Statistics enabled. */
   zl303xx_BooleanE statisticsEnabled;


   /* Function to reset the ZL303xx device */
   OS_ARG1_FUNC_PTR resetFuncPtr;

} zl303xx_ParamsS;

/* include prototypes for functions which operate on the zl303xx_ParamsS structure */
#include "zl303xx_Params.h"



#ifdef __cplusplus
}
#endif

#endif   /* MULTIPLE INCLUDE BARRIER */

