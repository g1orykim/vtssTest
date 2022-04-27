

/*******************************************************************************
*
*  $Id: zl303xx_TsIfTypes.h 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains software structures and data types for the Timestamp
*     Interface APIs.
*
*******************************************************************************/

#ifndef ZL303XX_TSIF_TYPES_H_
#define ZL303XX_TSIF_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"     /* For regular data types.                */

/*****************   DEFINES   ************************************************/

/* Only the CPU and WAN ports of the Timestamp Interface are configurable. */
#define ZL303XX_TSIF_MAX_PKT_SIZE_DEFAULT   (Uint32T)(1518)

/**** Number of Statistics values for each internal Interface port ****/
#define ZL303XX_TSIF_STATS_NUM_COUNTERS     (Uint32T)(28)

/*****************   DATA TYPES   *********************************************/

/* IDs of the Individual Timestamp Interface Ports. */
typedef enum
{
   /* Define descriptive names for the Internal Interface ports. */
   ZL303XX_TSIF_PORT_ID_CPU = 0,
   ZL303XX_TSIF_PORT_ID_WAN,

   /* For indexing and range checking. */
   ZL303XX_TSIF_PORT_ID_NUM_PORTS,

   ZL303XX_TSIF_PORT_ID_MIN = 0,   /* Keep as the first item above. */
   ZL303XX_TSIF_PORT_ID_MAX = (ZL303XX_TSIF_PORT_ID_NUM_PORTS - 1),

   /* A Flag to represent that an API call is to apply to all ports of the
    * Timestamp Interface */
   ZL303XX_TSIF_PORT_ID_ALL = (1 << ZL303XX_TSIF_PORT_ID_NUM_PORTS)
} zl303xx_TsIfPortIdE;

/* The Timestamp Interface is driven by a source clock. The following types
 * define the source of the interface's clock an its rate. */
typedef enum
{
   ZL303XX_TSIF_SCLK_SRC_SDH = 0,
   ZL303XX_TSIF_SCLK_SRC_M1_PIN,
   ZL303XX_TSIF_SCLK_SRC_M2_PIN,

   /* The Maximum Identifier value. */
   ZL303XX_TSIF_SCLK_SRC_MAX = ZL303XX_TSIF_SCLK_SRC_M2_PIN
} zl303xx_TsIfSrcClkE;

typedef enum
{
   ZL303XX_TSIF_SCLK_RATE_12_5MHZ = 0,
   ZL303XX_TSIF_SCLK_RATE_125MHZ,

   /* The Maximum Identifier value. */
   ZL303XX_TSIF_SCLK_RATE_MAX = ZL303XX_TSIF_SCLK_RATE_125MHZ
} zl303xx_TsIfSrcClkRateE;


/* The Operating Type of the internal ports of the Interface. */
typedef enum
{
   ZL303XX_TSIF_PORT_TYPE_MAC = 0,
   ZL303XX_TSIF_PORT_TYPE_PHY,

   /* The Maximum Identifier value. */
   ZL303XX_TSIF_PORT_TYPE_MAX = ZL303XX_TSIF_PORT_TYPE_PHY
} zl303xx_TsIfPortTypeE;

/* The Operating Mode of the internal ports of the Interface. */
typedef enum
{
   ZL303XX_TSIF_PORT_MODE_RMII = 0 ,
   ZL303XX_TSIF_PORT_MODE_GMII_MII,
   ZL303XX_TSIF_PORT_MODE_TBI,

   /* The Maximum Identifier value. */
   ZL303XX_TSIF_PORT_MODE_MAX = ZL303XX_TSIF_PORT_MODE_TBI
} zl303xx_TsIfPortModeE;

/* The Line Rate of the internal ports of the Interface. */
typedef enum
{
   ZL303XX_TSIF_PORT_RATE_10M = 0,
   ZL303XX_TSIF_PORT_RATE_100M,
   ZL303XX_TSIF_PORT_RATE_1G,

   /* The Maximum Identifier value. */
   ZL303XX_TSIF_PORT_RATE_MAX = ZL303XX_TSIF_PORT_RATE_1G
} zl303xx_TsIfPortRateE;

/* The Link State of the internal ports of the Interface. */
typedef enum
{
   ZL303XX_TSIF_PORT_LINK_STATE_AU_ENABLE = 0,
   ZL303XX_TSIF_PORT_LINK_STATE_AU_LIMITED,
   ZL303XX_TSIF_PORT_LINK_STATE_FORCE_DOWN,
   ZL303XX_TSIF_PORT_LINK_STATE_FORCE_UP,

   /* The Maximum Identifier value. */
   ZL303XX_TSIF_PORT_LINK_STATE_MAX = ZL303XX_TSIF_PORT_LINK_STATE_FORCE_UP
} zl303xx_TsIfPortLinkStateE;

/* The Timestamp Interface Packet Flow. */
/* These flags are defined to match the value written to the hardware. */
typedef enum
{
   ZL303XX_TSIF_PKT_FLOW_NON_TIMING_TX = (Uint32T)0x00000400,
   ZL303XX_TSIF_PKT_FLOW_TIMING_TX = (Uint32T)0x00000800,
   ZL303XX_TSIF_PKT_FLOW_NON_TIMING_RX = (Uint32T)0x04000000,
   ZL303XX_TSIF_PKT_FLOW_TIMING_RX = (Uint32T)0x08000000,

   /* All Supported Packet Flow Directions. */
   /* The Flow from CPU to the internal time-stamper is not wired since any
    * transmitted timing packets are timestamped at the WAN interface. So,
    * omit this path from the ALL definition.   */
   ZL303XX_TSIF_PKT_FLOW_ALL = (ZL303XX_TSIF_PKT_FLOW_NON_TIMING_TX |
                                ZL303XX_TSIF_PKT_FLOW_NON_TIMING_RX  |
                                ZL303XX_TSIF_PKT_FLOW_TIMING_RX)
} zl303xx_TsIfPktFlowE;

/* IDs of the available Timestamp Interface Interrupt requests. These are
 * available for each of the Internal Timestamp Interface ports. */
typedef enum
{
   /* Define the individual Interrupts. */
   ZL303XX_TSIF_IRQ_STATS = 0,
   ZL303XX_TSIF_IRQ_LINK,

   /* For indexing and range checking. */
   ZL303XX_TSIF_IRQ_NUM_IRQS,

   ZL303XX_TSIF_IRQ_MIN = 0, /* Keep as the first item above. */
   ZL303XX_TSIF_IRQ_MAX = (ZL303XX_TSIF_IRQ_NUM_IRQS - 1),

   /* A Flag to represent that an API call is to apply to all IRQs of the
    * Timestamp Interface */
   ZL303XX_TSIF_IRQ_ALL = (1 << ZL303XX_TSIF_IRQ_NUM_IRQS)
} zl303xx_TsIfIrqIdE;

/*****************   DATA STRUCTURES   ****************************************/

/* A list of all the Timestamp Interface statistics counters */
/* Although the hardware counters are not 64-bits, each Timestamp Interface
 * maintains a history of roll-over events for every counter and therefore
 * provides 64-bit return values as defined in the zl303xx_TsIfStatsS structure.
 * It is important to keep these 64-bits since the updating is indexed into the
 * structure assuming 8 byte members. */
typedef struct
{
   Uint64S octetsTx;                   /* Total bytes sent                    */
   Uint64S ucastFramesTx;              /* Total Unicast frames sent           */
   Uint64S failedFramesTx;             /* Number of sent frames to fail       */
   Uint64S flowCtrlTx;                 /* Flow control frames sent            */
   Uint64S nonUcastFramesTx;           /* Total non-Unicast frames sent       */

   Uint64S totalOctetsRx;              /* Total bytes received                */
   Uint64S totalFramesRx;              /* Total frames received               */
   Uint64S goodOctetsRx;               /* Good bytes received                 */
   Uint64S goodFramesRx;               /* Good frames received                */
   Uint64S flowCtrlRx;                 /* Flow control frames received        */
   Uint64S mcastFramesRx;              /* Multicast frames received           */
   Uint64S bcastFramesRx;              /* Broadcast frames received           */
   Uint64S frameSize64Octets;          /* Frames with length of 64 bytes      */
   Uint64S jabberFrames;               /* Number of frames received with size
                                        * larger than the Jabber Lockup
                                        * Protection Timer - TW3              */
   Uint64S frameSize65To127Octets;     /* Frames of length 65 - 127 bytes     */
   Uint64S oversizedFrames;            /* Over-size frames                    */
   Uint64S frameSize128To255Octets;    /* Frames of length 128 - 255 bytes    */
   Uint64S frameSize256To511Octets;    /* Frames of length 256 - 511 bytes    */
   Uint64S frameSize512To1023Octets;   /* Frames of length 512 - 1023 bytes   */
   Uint64S frameSize1024To1518Octets;  /* Frames of length 1024 - MaxPktSize bytes */
   Uint64S frameFragments;             /* Number of frames below 64 bytes with
                                        * bad FCS */
   Uint64S alignmentErrors;            /* Number of valid frames with bad alignment */
   Uint64S undersizedFrames;           /* Under-size frames (below 64 bytes)  */
   Uint64S crcErrors;                  /* Number of frames with CRC errors    */
   Uint64S shortEvents;                /* Number of frames with size less than
                                        *    the length of a Short Event      */
   Uint64S collisions;                 /* Early Collisions frames - collision
                                        * before the 64 byte Late Event Threshold */
   Uint64S dropEvents;                 /* Dropped frames                             */
   Uint64S filter;                     /* Number of packets filtered          */
} zl303xx_TsIfStatsS;


/* The Timestamp Interface Port Control Data structure. */
typedef struct
{
   zl303xx_TsIfPortTypeE type;     /* MAC or PHY.                       */
   Uint8T addr;                    /* 0 for MAC; user specified for PHY. */

   zl303xx_TsIfPortModeE mode;     /* RMII, GMII/MII or TBI.            */
   zl303xx_BooleanE loopbackEn;    /* TRUE if the port is in loopback mode. */
} zl303xx_TsIfPortCtrlS;

/* The Timestamp Interface Port Configuration Data structure. */
typedef struct
{
   zl303xx_BooleanE           flowCtrlEn;
   zl303xx_BooleanE           asymFlowCtrlEn;  /* Valid if flowCtrolEn is TRUE */
   zl303xx_BooleanE           fullDuplex;

   zl303xx_TsIfPortRateE      lineRate;
   zl303xx_TsIfPortLinkStateE linkState;       /* Down/Up only for PHY ports */
   zl303xx_BooleanE           rxSampleRisingEdge;
} zl303xx_TsIfPortConfigS;

/* The Timestamp Interface Port Operational Status structure. */
typedef struct
{
   zl303xx_BooleanE           flowCtrlEn;
   zl303xx_BooleanE           fullDuplex;
   zl303xx_BooleanE           linkUp;          /* Down/Up only */
   zl303xx_TsIfPortRateE      lineRate;
   zl303xx_BooleanE           tbiModeEn;
} zl303xx_TsIfPortOperS;

/* The Timestamp Interface Initialization Data structure. */
typedef struct
{
   /* Source Clock parameters */
   zl303xx_TsIfSrcClkE     sclkSrc;      /* The Source of the Clock that drives
                                          *   the interface hardware.      */
   zl303xx_TsIfSrcClkRateE sclkRate;     /* The Rate of the Source Clock.  */

   zl303xx_TsIfSrcClkRateE gRefClkSrc;   /* The 125MHz MAC clock.          */

   /* Maximum packet length supported on the Interface. */
   Uint16T maxPktLen;

   /* Control Data for each internal port of the interface. */
   zl303xx_TsIfPortCtrlS portCtrl[ZL303XX_TSIF_PORT_ID_NUM_PORTS];
} zl303xx_TsIfInitS;

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

