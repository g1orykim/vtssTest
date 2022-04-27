

/*******************************************************************************
*
*  $Id: zl303xx_TsEngInternal.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Internal device constants for the Timestamp Engine
*
*******************************************************************************/

#ifndef ZL303XX_TS_ENG_INTERNAL_H_
#define ZL303XX_TS_ENG_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"
#include "zl303xx_AddressMap.h"

/*****************   DEFINES   ************************************************/

/*****  TASK RELATED DEFINES  ***********************/
#ifdef OS_VXWORKS
#define ZL303XX_TSENG_TASK_PRIORITY      (Uint32T)35
#endif
#ifndef OS_VXWORKS
#define ZL303XX_TSENG_TASK_PRIORITY      (Uint32T)97
#endif

#define ZL303XX_TSENG_TASK_STACK_SIZE    (Uint32T)4000
#define ZL303XX_TSENG_TASK_EVENTQ_SIZE   (Uint32T)32

/*****  CONFIGURATION DEFINES  **********************/
/* Timestamp Protocol Frequency Conversion Definitions */
#define ZL303XX_TSENG_PTP_CONV_FACTOR             (Uint16T)(31250)
#define ZL303XX_TSENG_NTP_CONV_FACTOR             (Uint16T)(33554)
#define ZL303XX_TSENG_RTP_CONV_FACTOR(rtpFreqKhz) (Uint16T)((rtpFreqKhz)/2)

/* Timestamp Protocol Modulus Definitions */
#define ZL303XX_TSENG_PTP_MODULUS_FACTOR          (Uint8T)(0)
#define ZL303XX_TSENG_NTP_MODULUS_FACTOR          (Uint8T)(124)
#define ZL303XX_TSENG_RTP_MODULUS_FACTOR          (Uint8T)(0)

/* Timestamp Protocol Period Definitions */
/* Use 16KHz as a Base since we can define its period in ScaledNs Units. */
#define ZL303XX_TSENG_FREQ_8K_MULT                (Uint32T)(8000)
#define ZL303XX_TSENG_FREQ_16K_MULT               (Uint32T)(16000)
#define ZL303XX_TSENG_16KHZ_PERIOD_SNS            (Uint32T)(0xF4240000)

/*****  REGISTER ADDRESS DEFINES  **********************/

/* DCO related registers */
#define ZL303XX_TSENG_DCO_FRQ_OFFSET ZL303XX_MAKE_MEM_ADDR(0x01, 0x65, ZL303XX_MEM_SIZE_4_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_DCO_PHASE      ZL303XX_MAKE_MEM_ADDR(0x01, 0x69, ZL303XX_MEM_SIZE_8_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_LOCAL_TS       ZL303XX_MAKE_MEM_ADDR(0x01, 0x71, ZL303XX_MEM_SIZE_4_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_INSRTN_TS      ZL303XX_MAKE_MEM_ADDR(0x01, 0x75, ZL303XX_MEM_SIZE_8_BYTE, ZL303XX_MEM_OVRLY_NONE)

/* A special case to optimize the reading of the hardware timestamps over the SPI */
#define ZL303XX_TSENG_SAMPLE_TS_SIZE  (Uint8T)(ZL303XX_MEM_SIZE_8_BYTE +   \
                                               ZL303XX_MEM_SIZE_4_BYTE +   \
                                               ZL303XX_MEM_SIZE_8_BYTE)
#define ZL303XX_TSENG_SAMPLE_TS      ZL303XX_MAKE_MEM_ADDR(0x01, 0x69, ZL303XX_TSENG_SAMPLE_TS_SIZE, ZL303XX_MEM_OVRLY_NONE)

/* Timestamp Engine register address definitions */
#define ZL303XX_TSENG_EXIT_TS        ZL303XX_MAKE_MEM_ADDR(0x0A, 0x65, ZL303XX_MEM_SIZE_4_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_EXIT_INDEX     ZL303XX_MAKE_MEM_ADDR(0x0A, 0x69, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)

/* A special case to optimize the reading of exitTs over the SPI */
#define ZL303XX_TSENG_EXIT_TS_PAIR_SIZE  (Uint8T)(ZL303XX_MEM_SIZE_4_BYTE + ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_TSENG_EXIT_TS_PAIR   ZL303XX_MAKE_MEM_ADDR(0x0A, 0x65, ZL303XX_TSENG_EXIT_TS_PAIR_SIZE, ZL303XX_MEM_OVRLY_NONE)

#define ZL303XX_TSENG_EXIT_Q_LEVEL   ZL303XX_MAKE_MEM_ADDR(0x0A, 0x6A, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_EXIT_Q_THRS    ZL303XX_MAKE_MEM_ADDR(0x0A, 0x6B, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_SAMPLE_CTRL    ZL303XX_MAKE_MEM_ADDR(0x0A, 0x6C, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_SCHED_TM       ZL303XX_MAKE_MEM_ADDR(0x0A, 0x6D, ZL303XX_MEM_SIZE_4_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_INTRVL_CTRL    ZL303XX_MAKE_MEM_ADDR(0x0A, 0x71, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_INSRTN_TS_CTRL ZL303XX_MAKE_MEM_ADDR(0x0A, 0x72, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_INSRTN_TS_MDLS ZL303XX_MAKE_MEM_ADDR(0x0A, 0x73, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_INSRTN_TS_CNV  ZL303XX_MAKE_MEM_ADDR(0x0A, 0x74, ZL303XX_MEM_SIZE_2_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_TM_OF_DAY      ZL303XX_MAKE_MEM_ADDR(0x0A, 0x76, ZL303XX_MEM_SIZE_8_BYTE, ZL303XX_MEM_OVRLY_NONE)

/* Interrupt related registers */
#define ZL303XX_TSENG_ISR            ZL303XX_MAKE_MEM_ADDR(0x0F, 0x6E, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TSENG_ISR_MASK_REG   ZL303XX_MAKE_MEM_ADDR(0x0F, 0x6F, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)

/* Individual Register Bit Definitions */

/* Exit queue fill level mask & max (ZL303XX_TSENG_EXIT_Q_STS and
   ZL303XX_TSENG_EXIT_Q_THRS) */
#define ZL303XX_TSENG_EXIT_Q_LEVEL_MASK     (Uint8T)0x7F
#define ZL303XX_TSENG_EXIT_Q_LEVEL_MAX      (Uint8T)64

/* Timestamp engine control register (ZL303XX_TSENG_SAMPLE_CTRL) */
#define ZL303XX_TSENG_UPDATE_TYPE_MASK      (Uint8T)0x03
#define ZL303XX_TSENG_SAMPLE_UPDATE_SHIFT   (Uint8T)0
#define ZL303XX_TSENG_DCO_UPDATE_SHIFT      (Uint8T)2

/* Timestamp engine interval timer control (ZL303XX_TSENG_INTRVL_CTRL) */
/*********************************************************************/
#define ZL303XX_TSENG_INTRVL_CTRL_MASK      (Uint8T)0x0F

/* Writing a value of 0 to the interval register causes a sample every 2^17
   SYS clock ticks (i.e. 2^17 * 12.5ns = 0.001638400 sec). */
/* Although the bitfield is 4-bits wide, 9 is the maximum that can be written
   to the device register causing a sample every 2^26 SYS clock ticks.
   (i.e. 2^26 * 12.5ns = 0.838860800 sec). */
#define ZL303XX_TSENG_INTRVL_VALUE_MIN      (Uint8T)17
#define ZL303XX_TSENG_INTRVL_VALUE_MAX      (Uint8T)26
#define ZL303XX_CHECK_INTRVL(interval)                  \
   ( ((interval < ZL303XX_TSENG_INTRVL_VALUE_MIN) ||    \
      (interval > ZL303XX_TSENG_INTRVL_VALUE_MAX)) ?    \
     (ZL303XX_ERROR_NOTIFY("Invalid interval"), ZL303XX_PARAMETER_INVALID) : \
     (ZL303XX_OK))

/* Set the default interval to the maximum (0.838860800 sec) */
#define ZL303XX_TSENG_INTRVL_VALUE_DEFAULT  ZL303XX_TSENG_INTRVL_VALUE_MAX

#define ZL303XX_TSENG_CONV_INTRVL_TO_REG(interval)   \
        (Uint32T)(interval - ZL303XX_TSENG_INTRVL_VALUE_MIN)

/*********************************************************************/

/* Insertion timestamp control register (ZL303XX_TSENG_INSRTN_TS_CTRL) */
#define ZL303XX_TSENG_INSRTN_PROTOCOL_SHIFT       (Uint8T)0
#define ZL303XX_TSENG_INSRTN_PROTOCOL_MASK        (Uint8T)0x03

/* Frequency modulus register (ZL303XX_TSENG_INSRTN_TS_MDLS) */
#define ZL303XX_TSENG_INSRTN_TS_MDLS_MASK         (Uint8T)0xFF

/* Frequency conversion factor register (ZL303XX_TSENG_INSRTN_TS_CNV) */
#define ZL303XX_TSENG_INSRTN_TS_CNV_MASK          (Uint16T)0xFFFF

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/


#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

