



/*******************************************************************************
*
*  $Id: zl303xx_DebugMisc.c 8815 2012-08-13 18:58:21Z JK $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains miscellaneous debug functions.
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#ifdef __STRICT_ANSI__
    #define __STRICT_ANSI_TMP__ __STRICT_ANSI__
    #undef __STRICT_ANSI__  /* Undefine __STRICT_ANSI__ so ctime_r() is visible */
#endif
#include <time.h>
#ifdef __STRICT_ANSI_TMP__
    #define __STRICT_ANSI__ __STRICT_ANSI_TMP__ /* Redefine __STRICT_ANSI__ as per above dance */
    #undef __STRICT_ANSI_TMP__
#endif

#include "zl303xx_Global.h"
#include "zl303xx_Api.h"
#include "zl303xx.h"
#include "zl303xx_RdWr.h"
//#include "zl303xx_AprStatistics.h"
//#include "zl303xx_Apr.h"
//#include "zl303xx_Apr1Hz.h"
//#include "zl303xx_DebugMisc.h"
#include "zl303xx_AddressMap.h"

#include "zl303xx_IsrLow.h"
#include "zl303xx_DpllLow.h"
#include "zl303xx_RefLow.h"
#include "zl303xx_TsIfTypes.h"




/*****************   DEFINES     **********************************************/



/*****************   STATIC DATA STRUCTURES   *********************************/
typedef struct
{
   Uint32T value;
   const char *string;
} zl303xx_DebugValToStringS;

/*****************   FORWARD FUNCTION DECLARATIONS   **************************/
zlStatusE devRead(zl303xx_ParamsS *zl303xx_Params, Uint32T page, Uint32T regAddr, Uint32T regSize, Uint32T overlay);
zlStatusE devWrite(zl303xx_ParamsS *zl303xx_Params, Uint32T page, Uint32T regAddr, Uint32T regSize, Uint32T data, Uint32T overlay);



/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

static zlStatusE local_DebugRefConfig(zl303xx_ParamsS *zl303xx_Params, zl303xx_RefIdE refId);
static zlStatusE local_DebugHwRefStatus(zl303xx_ParamsS *zl303xx_Params, zl303xx_RefIdE refId);
static zlStatusE local_DebugDpllStatus(zl303xx_ParamsS *zl303xx_Params, zl303xx_DpllIdE dpllId);
static zlStatusE local_DebugDpllConfig(zl303xx_ParamsS *zl303xx_Params, zl303xx_DpllIdE dpllId);

static char const *zl303xx_DebugValToString(zl303xx_DebugValToStringS valToStr[],
                                          Uint32T val);
/*****************   STATIC GLOBAL VARIABLES   ********************************/


/* List of strings for identifying MAC result types.
   This should be indexed using the zl303xx_MacStatsIndexE enum */

static char const * const invalidStr = "INVALID";

static zl303xx_DebugValToStringS refModeToString[] = {
      {ZL303XX_REF_MODE_AUTO,      "Auto Freq. Detect"           },
      {ZL303XX_REF_MODE_CUSTA,     "Custom A Config."            },
      {ZL303XX_REF_MODE_CUSTB,     "Custom B Config."            },
      {ZL303XX_REF_MODE_REV2AUTO,  "Reserved Sets -- Auto Detect"},
      {(Uint32T)-1,              NULL                          }
};

static zl303xx_DebugValToStringS refDivToString[] = {
      {ZL303XX_REF_DIV_1,    "1"  },
      {ZL303XX_REF_DIV_2,    "2"  },
      {ZL303XX_REF_DIV_3,    "3"  },
      {ZL303XX_REF_DIV_4,    "4"  },
      {ZL303XX_REF_DIV_5,    "5"  },
      {ZL303XX_REF_DIV_6,    "6"  },
      {ZL303XX_REF_DIV_7,    "7"  },
      {ZL303XX_REF_DIV_8,    "8"  },
      {ZL303XX_REF_DIV_1P5,  "1.5"},
      {ZL303XX_REF_DIV_2P5,  "2.5"},
      {(Uint32T)-1,        NULL }
};

static zl303xx_DebugValToStringS refOorToString[] = {
      {ZL303XX_OOR_9_12PPM,     "+/- 9-12 ppm"   },
      {ZL303XX_OOR_40_52PPM,    "+/- 40-52 ppm"  },
      {ZL303XX_OOR_100_130PPM,  "+/- 100-130 ppm"},
      {ZL303XX_OOR_64_83PPM,    "+/- 64-83 ppm"  },
      {ZL303XX_OOR_13_18PPM,    "+/- 13-18 ppm"  },
      {ZL303XX_OOR_24_32PPM,    "+/- 24-32 ppm"  },
      {ZL303XX_OOR_36_47PPM,    "+/- 36-47 ppm"  },
      {ZL303XX_OOR_52_67PPM,    "+/- 52-67 ppm"  },
      {(Uint32T)-1,           NULL             }
};

static zl303xx_DebugValToStringS refFreqToString[] = {
      {ZL303XX_REF_2KHz,      "2 kHz"     },
      {ZL303XX_REF_8KHz,      "8 kHz"     },
      {ZL303XX_REF_64KHz,     "64 kHz"    },
      {ZL303XX_REF_1544KHz,   "1.544 MHz" },
      {ZL303XX_REF_2048KHz,   "2.048 MHz" },
      {ZL303XX_REF_6480KHz,   "6.48 MHz"  },
      {ZL303XX_REF_8192KHz,   "8.192 MHz" },
      {ZL303XX_REF_16384KHz,  "16.384 MHz"},
      {ZL303XX_REF_19449KHz,  "19.44 MHz" },
      {ZL303XX_REF_38880KHz,  "38.88 MHz" },
      {ZL303XX_REF_77760KHz,  "77.76 MHz" },
      {(Uint32T)-1,         NULL        }
};

static zl303xx_DebugValToStringS syncFreqToString[] = {
      {ZL303XX_SYNC_166Hz,   "166 Hz"},
      {ZL303XX_SYNC_400Hz,   "400 Hz"},
      {ZL303XX_SYNC_1000Hz,  "1 kHz" },
      {ZL303XX_SYNC_2000Hz,  "2 kHz" },
      {ZL303XX_SYNC_1Hz,     "1 Hz"  },
      {ZL303XX_SYNC_8000Hz,  "8 kHz" },
      {ZL303XX_SYNC_64KHz,   "64 kHz"},
      {(Uint32T)-1,        NULL    }
};

static zl303xx_DebugValToStringS dpllBwToString[] = {
      {ZL303XX_DPLL_BW_P1Hz,        "0.1 Hz"          },
      {ZL303XX_DPLL_BW_1P7Hz,       "1.7 Hz"          },
      {ZL303XX_DPLL_BW_3P5Hz,       "3.5 Hz"          },
      {ZL303XX_DPLL_BW_14Hz,        "14 Hz"           },
      {ZL303XX_DPLL_BW_28Hz,        "28 Hz"           },
      {ZL303XX_DPLL_BW_890Hz,       "890 Hz"          },
      {ZL303XX_DPLL_BW_FAST,        "Fast Lock (7 Hz)"},
      {ZL303XX_DPLL_BW_S3E_0P3mHZ,  "0.3 mHz (S3E)"   },
      {ZL303XX_DPLL_BW_S3E_1mHZ,    "1.0 mHz (S3E)"   },
      {ZL303XX_DPLL_BW_S3E_3mHZ,    "3.0 mHz (S3E)"   },
      {ZL303XX_DPLL_BW_INVALID,     "INVALID"         },
      {(Uint32T)-1,               NULL              }
};

static zl303xx_DebugValToStringS dpllPslToString[] = {
      {ZL303XX_DPLL_PSL_P885US,     "885 ns/s" },
      {ZL303XX_DPLL_PSL_7P5US,      "7.5 us/s" },
      {ZL303XX_DPLL_PSL_61US,       "61 us/s"  },
      {ZL303XX_DPLL_PSL_UNLIMITED,  "Unlimited"},
      {(Uint32T)-1,               NULL       }
};

static zl303xx_DebugValToStringS dpllHoldUpdateToString[] = {
      {ZL303XX_DPLL_HLD_UPD_26mS,  "26 ms"},
      {ZL303XX_DPLL_HLD_UPD_1S,    "1 s"  },
      {ZL303XX_DPLL_HLD_UPD_10S,   "10 s" },
      {ZL303XX_DPLL_HLD_UPD_60S,   "60 s" },
      {(Uint32T)-1,              NULL   }
};

static zl303xx_DebugValToStringS dpllHoldFiltBwToString[] = {
      {ZL303XX_DPLL_HLD_FILT_BYPASS,  "Bypass -- no filtering"},
      {ZL303XX_DPLL_HLD_FILT_18mHz,   "18 mHz"                },
      {ZL303XX_DPLL_HLD_FILT_P6Hz,    "0.6 Hz"                },
      {ZL303XX_DPLL_HLD_FILT_10Hz,    "10 Hz"                 },
      {(Uint32T)-1,                 NULL                    }
};

static zl303xx_DebugValToStringS dpllModeSelToString[] = {
      {ZL303XX_DPLL_MODE_NORM,  "Manual Normal Mode"   },
      {ZL303XX_DPLL_MODE_HOLD,  "Manual Holdover Mode" },
      {ZL303XX_DPLL_MODE_FREE,  "Manual Freerun Mode"  },
      {ZL303XX_DPLL_MODE_AUTO,  "Automatic Normal Mode"},
      {ZL303XX_DPLL_MODE_TOP,   "ToP Mode"             },
      {(Uint32T)-1,           NULL                   }
};

static zl303xx_DebugValToStringS dpllPullInRangeToString[] = {
      {ZL303XX_DPLL_PULLIN_12PPM,   "+/- 12 ppm" },
      {ZL303XX_DPLL_PULLIN_52PPM,   "+/- 52 ppm" },
      {ZL303XX_DPLL_PULLIN_130PPM,  "+/- 130 ppm"},
      {ZL303XX_DPLL_PULLIN_83PPM,   "+/- 83 ppm" },
      {(Uint32T)-1,               NULL         }
};


/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/

/*******************************************************************************

  Function Name:
    devRead

  Details:
   Perform a device read

  Parameters:
   [in]
   zl303xx_Params   Pointer to the device instance parameter structure
   page           page for register access
   regAddr        register address within the page
   regSize        size of access in number of bytes - 1, 2, 4
   overlay        0 = standard address map
                  1 = bridge config
                  2 = bridge memory

    [out]
   None

  Return Value:
   zlStatusE


*******************************************************************************/

zlStatusE devRead(zl303xx_ParamsS *zl303xx_Params, Uint32T page, Uint32T regAddr, Uint32T regSize, Uint32T overlay)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T value;
   Uint32T addr, overlayAddr;

   if ((status = ZL303XX_CHECK_POINTER(zl303xx_Params)) != ZL303XX_OK)
       return (status);

   switch (overlay)
   {
      case 1 :
         overlayAddr = ZL303XX_MEM_OVRLY_BRIDGE_CONFIG;
         break;

      case 2 :
         overlayAddr = ZL303XX_MEM_OVRLY_BRIDGE_MEMORY;
         break;

      default :
      case 0 :
         overlayAddr = ZL303XX_MEM_OVRLY_NONE;
         break;
   }

   switch (regSize)
   {
      case 1 : value = ZL303XX_MEM_SIZE_1_BYTE;
         break;

      case 2 : value = ZL303XX_MEM_SIZE_2_BYTE;
         break;

      case 4 :
      default:
            value = ZL303XX_MEM_SIZE_4_BYTE;
         break;
   }

   addr = ZL303XX_MAKE_MEM_ADDR(page, regAddr, value, overlayAddr);

   status = zl303xx_Read(zl303xx_Params, (zl303xx_ReadWriteS *)NULL, addr, &value);

   printf("%08X : %08X, %u, %d, err %d\n", addr, value, value, (Sint32T)value, status);

   return status;
}

/*******************************************************************************

  Function Name:
    devWrite

  Details:
   Perform a device write

  Parameters:
   [in]
   zl303xx_Params   Pointer to the device instance parameter structure
   page           page for register access
   regAddr        register address within the page
   regSize        size of access in number of bytes - 1, 2, 4
   data           data to write to the device
   overlay        0 = standard address map
                  1 = bridge config
                  2 = bridge memory

    [out]
   None

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE devWrite(zl303xx_ParamsS *zl303xx_Params, Uint32T page, Uint32T regAddr, Uint32T regSize, Uint32T data, Uint32T overlay)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T value = 0;
   Uint32T addr, overlayAddr;

   if ((status = ZL303XX_CHECK_POINTER(zl303xx_Params)) != ZL303XX_OK)
       return (status);

   switch (overlay)
   {
      case 1 :
         overlayAddr = ZL303XX_MEM_OVRLY_BRIDGE_CONFIG;
         break;

      case 2 :
         overlayAddr = ZL303XX_MEM_OVRLY_BRIDGE_MEMORY;
         break;

      default :
      case 0 :
         overlayAddr = ZL303XX_MEM_OVRLY_NONE;
         break;
   }

   switch (regSize)
   {
      case 1 : value = ZL303XX_MEM_SIZE_1_BYTE;
         break;

      case 2 : value = ZL303XX_MEM_SIZE_2_BYTE;
         break;

      case 4 : value = ZL303XX_MEM_SIZE_4_BYTE;
         break;

      default:
         break;
   }

   {
      addr = ZL303XX_MAKE_MEM_ADDR(page, regAddr, value, overlayAddr);
   }

   status = zl303xx_Write(zl303xx_Params, (zl303xx_ReadWriteS *)NULL, addr, data);

   return status;
}


/*******************************************************************************
  Function Name:
   sprintf64u

  Details:
   Formats a 64-bit number as a decimal string

  Parameters:
   [in]
   buf      The buffer to store the resulting string
   value    The 64-bit number

    [out]
   buf      Contains the resulting string, with no terminating null

  Return Value:
   The number of digits in the string

  Notes:
   This method for formatting 64-bit integers was extracted from:
   http://developer.novell.com/wiki/index.php/A_nice_platform-independent_way_to_print_64-bit_integers

*******************************************************************************/

Sint32T sprintf64u(char* buffer, Uint64T value);
Sint32T sprintf64u(char* buffer, Uint64T value)
{
   Uint64T quot = value / 1000;
   Sint32T chars_written;

   if (quot != 0)
   {
     chars_written = sprintf64u(buffer, quot);  /* Recurse */
     chars_written += sprintf(buffer + chars_written, "%03u", (Uint16T)(value % 1000));
   }
   else
   {
     chars_written = sprintf(buffer, "%u", (Uint16T)(value % 1000));
   }
   return chars_written;
 }

/*******************************************************************************
  Function Name:
   zl303xx_FormatUint64S

  Details:
   Formats a Uint64S structure representing a 64-bit number into
   a decimal string

  Parameters:
   [in]
   buf      The buffer to store the resulting string
   pValue   The 64-bit number structure

    [out]
   buf      Contains the resulting string with a terminating null

  Return Value:
   The length of the string


*******************************************************************************/

Uint32T zl303xx_FormatUint64S(char *buf, Uint64S *pValue);
Uint32T zl303xx_FormatUint64S(char *buf, Uint64S *pValue)
{
   Uint64T value64T;
   Uint32T numChars;

   /* Formats a 64-bit unsigned value into a decimal string.
      Value is left justified, no leading zeros
      Uses the 64-bit math library */
   /* Returns the number of digits in the string */

   /* Convert 64-bit structure to a true 64-bit value */
   value64T = ((Uint64T)pValue->hi << (Uint64T)32) + (Uint64T)pValue->lo;

   /* Format the number */
   numChars = sprintf64u(buf, value64T);
   /* Add the terminating NULL */
   buf[numChars++] = '\0';

   return numChars;
}



static const char *zl303xx_PllDetectFpFreqStr[] =
{
   "FP FREQ 166HZ",
   "FP FREQ 400HZ",
   "FP FREQ 1KHZ",
   "FP FREQ 2KHZ",
   "FP FREQ 1HZ",
   "FP FREQ 8KHZ",
   "FP FREQ INVALID",
   "FP FREQ 64KHZ",
   "FP FREQ NONE"
};

static const char *zl303xx_PllDetectRefFreqStr[] =
{
   "REF FREQ 2KHZ",
   "REF FREQ 8KHZ",
   "REF FREQ 64KHZ",
   "REF FREQ 1.544MHZ",
   "REF FREQ 2.048MHZ",
   "REF FREQ 6.48MHZ",
   "REF FREQ 8.192MHZ",
   "REF FREQ 16.384MHZ",
   "REF FREQ 19.44MHZ",
   "REF FREQ 38.88MHZ",
   "REF FREQ 77.76MHZ",
   "ERROR",
   "ERROR",
   "ERROR",
   "ERROR",
   "REF FREQ NONE"
};

/*******************************************************************************

  Function Name:
   zl303xx_DebugPllStatus

  Details:
   Display PLL status information

  Parameters:
   [in]
   zl303xx_Params   Pointer to the device instance parameter structure

    [out]
   None

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DebugPllStatus(zl303xx_ParamsS *zl303xx_Params)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T loop;
   Sint32T dcoFreq;
   zl303xx_RefIdE refNum;
   zl303xx_RefFreqE refFreq;
   zl303xx_RefIsrStatusS refIsr;
   zl303xx_SyncFreqE syncFreq;
   zl303xx_DpllStatusS dpllStatus;

   if ((status = ZL303XX_CHECK_POINTER(zl303xx_Params)) != ZL303XX_OK)
       return (status);

   for (loop = 0; loop < ZL303XX_DPLL_NUM_REFS; loop++)
   {
      status = zl303xx_RefFreqDetectedGet(zl303xx_Params, (zl303xx_RefIdE)loop, &refFreq);

      if (status == ZL303XX_OK)
      {
         printf("Ref %u: %s (%d)", loop, zl303xx_PllDetectRefFreqStr[refFreq], refFreq);
      }

      if (status == ZL303XX_OK)
      {
         refIsr.Id = (zl303xx_RefIdE)loop;

         /* Clear any sticky bits in register */
         status |= zl303xx_RefIsrStatusGet(zl303xx_Params, &refIsr);

         /* Sticky bits cleared; get current values */
         status |= zl303xx_RefIsrStatusGet(zl303xx_Params, &refIsr);
      }

      if (status == ZL303XX_OK)
      {
         printf(", flags( SCM:%d CFM:%d GST:%d PFM:%d )\n", refIsr.scmFail, refIsr.cfmFail, refIsr.gstFail, refIsr.pfmFail);
      }
      else
      {
         printf(" Error reading status bits!\n");
      }
   }

   printf("\n");

   for (loop = 0; loop <= ZL303XX_SYNC_ID_MAX; loop++)
   {
      if (ZL303XX_CHECK_SYNC_ID(loop) == ZL303XX_OK)
      {
         status = zl303xx_RefSyncFreqDetectedGet(zl303xx_Params, (zl303xx_SyncIdE)loop, &syncFreq);

         if (status == ZL303XX_OK)
         {
            printf("Sync %u: %s\n", loop, zl303xx_PllDetectFpFreqStr[syncFreq]);
         }
      }
   }

   printf("\n");

   for (loop = ZL303XX_DPLL_ID_MIN; loop <= ZL303XX_DPLL_ID_MAX; loop++)
   {
      status = zl303xx_DpllRefSelectGet(zl303xx_Params,  (zl303xx_DpllIdE)loop, &refNum);

      if (status == ZL303XX_OK)
      {
         dpllStatus.Id = (zl303xx_DpllIdE)loop;
         status = zl303xx_DpllStatusGet(zl303xx_Params, &dpllStatus);
      }

      if (status == ZL303XX_OK)
      {
         printf("Pll %u: lock %d, ref fail %d, hold %d, ref %d \n",
                loop,
                dpllStatus.locked,
                dpllStatus.refFailed,
                dpllStatus.holdover,
                refNum);
      }
   }

   status = zl303xx_DcoGetFreq(zl303xx_Params, &dcoFreq);

   if (status == ZL303XX_OK)
   {
      printf("\nPll freq %d\n", dcoFreq);
   }

   return status;
}

/*******************************************************************************

  Function Name:
   zl303xx_DebugHwRefStatus

  Details:
   Display hardware reference input status information.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device instance parameter structure
   [in]   refId          Reference number to display status for (0-8).
                            -1 will display info for all references.

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_DebugHwRefStatus(zl303xx_ParamsS *zl303xx_Params, Uint32T refId)
{
   zlStatusE status = ZL303XX_OK;

   /* Check input parameters. */
   if (status == ZL303XX_OK)
   {
      if (zl303xx_Params == NULL)
      {
         printf("\nNULL pointer passed in for zl303xx_Params.\n");
         status = ZL303XX_PARAMETER_INVALID;
      }
   }
   if ((status == ZL303XX_OK) && (refId != (Uint32T)-1))
   {
      status = ZL303XX_CHECK_REF_ID(refId);

      if (status != ZL303XX_OK)
      {
         printf("\nINVALID value for refId: %u.\n", refId);
      }
   }

   if (status == ZL303XX_OK)
   {
      zl303xx_RefIdE startRef = refId, endRef = refId;

      if (refId == (Uint32T)-1)
      {
         startRef = ZL303XX_REF_ID_MIN;
         endRef = ZL303XX_REF_ID_MAX;
      }

      for (; startRef <= endRef; startRef++)
      {
         status |= local_DebugHwRefStatus(zl303xx_Params, startRef);
      }
   }

   return status;
}

/*******************************************************************************

  Function Name:
   zl303xx_DebugHwRefCfg

  Details:
   Display hardware reference input configuration.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device instance parameter structure
   [in]   refId          Reference number to display configuration for (0-8).
                            -1 will display info for all references.

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DebugHwRefCfg(zl303xx_ParamsS *zl303xx_Params, Uint32T refId)
{
   zlStatusE status = ZL303XX_OK;

   /* Check input parameters. */
   if (status == ZL303XX_OK)
   {
      if (zl303xx_Params == NULL)
      {
         printf("\nNULL pointer passed in for zl303xx_Params.\n");
         status = ZL303XX_PARAMETER_INVALID;
      }
   }
   if ((status == ZL303XX_OK) && (refId != (Uint32T)-1))
   {
      status = ZL303XX_CHECK_REF_ID(refId);

      if (status != ZL303XX_OK)
      {
         printf("\nINVALID value for refId: %u.\n", refId);
      }
   }

   if (status == ZL303XX_OK)
   {
      zl303xx_RefIdE startRef = refId, endRef = refId;

      if (refId == (Uint32T)-1)
      {
         startRef = ZL303XX_REF_ID_MIN;
         endRef = ZL303XX_REF_ID_MAX;
      }

      for (; startRef <= endRef; startRef++)
      {
         status |= local_DebugRefConfig(zl303xx_Params, startRef);
      }
   }

   return status;
}

/*******************************************************************************

  Function Name:
   zl303xx_DebugDpllStatus

  Details:
   Display hardware DPLL status information.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device instance parameter structure
   [in]   dpllId         DPLL number to display status for (1-2). -1 will
                            display info for all DPLLs.

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_DebugDpllStatus(zl303xx_ParamsS *zl303xx_Params, Uint32T dpllId)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_DpllIdE dpllIndex = dpllId - 1;

   /* Check input parameters. */
   if (status == ZL303XX_OK)
   {
      if (zl303xx_Params == NULL)
      {
         printf("\nNULL pointer passed in for zl303xx_Params.\n");
         status = ZL303XX_PARAMETER_INVALID;
      }
   }
   if ((status == ZL303XX_OK) && (dpllId != (Uint32T)-1))
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllIndex);

      if (status != ZL303XX_OK)
      {
         printf("\nINVALID value for dpllId: %u.\n", dpllId);
      }
   }

   if (status == ZL303XX_OK)
   {
      zl303xx_DpllIdE startDpll = dpllIndex, endDpll = dpllIndex;

      if (dpllId == (Uint32T)-1)
      {
         startDpll = ZL303XX_DPLL_ID_MIN;
         endDpll = ZL303XX_DPLL_ID_MAX;
      }

      for (; startDpll <= endDpll; startDpll++)
      {
         status |= local_DebugDpllStatus(zl303xx_Params, startDpll);
      }
   }

   return status;
}

/*******************************************************************************

  Function Name:
   zl303xx_DebugDpllConfig

  Details:
   Display hardware DPLL status information

  Parameters:
   [in]   zl303xx_Params   Pointer to the device instance parameter structure
   [in]   dpllId         DPLL number to display configuration for (1-2).
                            -1 will display info for all DPLLs.

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_DebugDpllConfig(zl303xx_ParamsS *zl303xx_Params, Uint32T dpllId)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_DpllIdE dpllIndex = dpllId - 1;

   /* Check input parameters. */
   if (status == ZL303XX_OK)
   {
      if (zl303xx_Params == NULL)
      {
         printf("\nNULL pointer passed in for zl303xx_Params.\n");
         status = ZL303XX_PARAMETER_INVALID;
      }
   }
   if ((status == ZL303XX_OK) && (dpllId != (Uint32T)-1))
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllIndex);

      if (status != ZL303XX_OK)
      {
         printf("\nINVALID value for dpllId: %u.\n", dpllId);
      }
   }

   if (status == ZL303XX_OK)
   {
      zl303xx_DpllIdE startDpll = dpllIndex, endDpll = dpllIndex;

      if (dpllId == (Uint32T)-1)
      {
         startDpll = ZL303XX_DPLL_ID_MIN;
         endDpll = ZL303XX_DPLL_ID_MAX;
      }

      for (; startDpll <= endDpll; startDpll++)
      {
         status |= local_DebugDpllConfig(zl303xx_Params, startDpll);
      }
   }

   return status;
}




static void strReverse(Uint8T* begin, Uint8T* end)
{

    char aux;

    while(end>begin)
        aux=*end, *end--=*begin, *begin++=aux;
}

/*******************************************************************************

  Function Name:         itoa

  Details:             Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C"
                   with slight modification to optimize for specific
                  architecture.

  Parameters:
   [in]      value    Integer to convert
   [out]      str      Ptr to string for the result
   [in]       base     Base (radix) of conversion


*******************************************************************************/


void itoa(int value, Uint8T* str, int base);
void itoa(int value, Uint8T* str, int base)
{

    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    Uint8T* wstr=str;
    int sign;
    div_t res;

    /* Validate base */
    if (base<2 || base>35){ *wstr='\0'; return; }

    /* Take care of sign */
    if ((sign=value) < 0) value = -value;

    /*  Conversion. Number is reversed. */
    do {

        res = div(value,base);
        *wstr++ = num[res.rem];

    } while((value=res.quot));

    if(sign<0) *wstr++='-';

    /* Terminate string */
    *wstr='\0';

    /* Reverse string */
    strReverse(str,wstr-1);

}






#ifdef OS_VXWORKS

#endif   /* OS_VXWORKS */


/*****************   STATIC FUNCTION DEFINITIONS   ****************************/

/*******************************************************************************

  Function Name:
   local_DebugHwRefStatus

  Details:
   Internal function to print reference input status.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device instance parameter structure
   [in]   refId          Reference number to display status for (0-8).

  Return Value:
   zlStatusE

*******************************************************************************/
static zlStatusE local_DebugHwRefStatus(zl303xx_ParamsS *zl303xx_Params, zl303xx_RefIdE refId)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_RefIsrStatusS refIsr;
   zl303xx_RefFreqE freq;

   printf("\nREF%d\n====\n", refId);

   if (status == ZL303XX_OK)
   {
      refIsr.Id = refId;

      /* Read twice to clear sticky bits. */
      status = zl303xx_RefIsrStatusGet(zl303xx_Params, &refIsr);
      status |= zl303xx_RefIsrStatusGet(zl303xx_Params, &refIsr);

      if (status != ZL303XX_OK)
      {
         printf("   ERROR %d calling zl303xx_RefIsrStatusGet().\n", status);
      }
   }

   if (status == ZL303XX_OK)
   {
      printf("<Status>\n");
      printf("   Failed:         %s\n", ZL303XX_BOOL_TO_STR(refIsr.refFail));
      printf("   ISR Fail Bits:  SCM:%d CFM:%d GST:%d PFM:%d\n", refIsr.scmFail,
                                                                 refIsr.cfmFail,
                                                                 refIsr.gstFail,
                                                                 refIsr.pfmFail);
   }


   if (status == ZL303XX_OK)
   {
      status = zl303xx_RefFreqDetectedGet(zl303xx_Params, refId, &freq);

      if (status != ZL303XX_OK)
      {
         printf("   ERROR %d calling zl303xx_RefFreqDetectedGet().\n", status);
      }
   }

   if (status == ZL303XX_OK)
   {
      printf("   Detected Freq.: %s\n", zl303xx_DebugValToString(refFreqToString, freq));
   }

   if (ZL303XX_CHECK_SYNC_ID((zl303xx_SyncIdE)refId) == ZL303XX_OK)
   {
      zl303xx_SyncStatusS syncStatus;

      if (status == ZL303XX_OK)
      {
         syncStatus.Id = (zl303xx_SyncIdE)refId;

         status = zl303xx_SyncStatusGet(zl303xx_Params, &syncStatus);

         if (status != ZL303XX_OK)
         {
            printf("   ERROR %d calling zl303xx_SyncStatusGet().\n", status);
         }
      }

      if (status == ZL303XX_OK)
      {
         printf("\n<Sync Status>\n");
         printf("   Failed:         %s\n", ZL303XX_BOOL_TO_STR(syncStatus.failed));
         printf("   Detected Freq.: %s\n", zl303xx_DebugValToString(syncFreqToString, syncStatus.detectedFreq));
      }
   }

   return status;
}

/*******************************************************************************

  Function Name:
   local_DebugRefConfig

  Details:
   Internal function to print reference input config.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device instance parameter structure
   [in]   refId          Reference number to display configuration for (0-8).

  Return Value:
   zlStatusE

*******************************************************************************/
static zlStatusE local_DebugRefConfig(zl303xx_ParamsS *zl303xx_Params, zl303xx_RefIdE refId)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_RefConfigS refConfig;

   printf("\nREF%d\n====\n", refId);

   if (status == ZL303XX_OK)
   {
      refConfig.Id = refId;

      status = zl303xx_RefConfigGet(zl303xx_Params, &refConfig);

      if (status != ZL303XX_OK)
      {
         printf("   ERROR %d calling zl303xx_RefConfigGet().\n", status);
      }
   }

   if (status == ZL303XX_OK)
   {
      printf("<Configuration>\n");
      printf("   Mode:         %d (%s)\n",           refConfig.mode, zl303xx_DebugValToString(refModeToString, refConfig.mode));
      printf("   Invert:       %s\n",                ZL303XX_BOOL_TO_STR(refConfig.invert));
      printf("   Prescaler:    %d (Divide by %s)\n", refConfig.prescaler, zl303xx_DebugValToString(refDivToString, refConfig.prescaler));
      printf("   Out-of-Range: %d (%s)\n",           refConfig.oorLimit, zl303xx_DebugValToString(refOorToString, refConfig.oorLimit));
   }

   if (ZL303XX_CHECK_SYNC_ID((zl303xx_SyncIdE)refId) == ZL303XX_OK)
   {
      zl303xx_SyncConfigS syncConfig;

      if (status == ZL303XX_OK)
      {
         syncConfig.Id = (zl303xx_SyncIdE)refId;

         status = zl303xx_SyncConfigGet(zl303xx_Params, &syncConfig);

         if (status != ZL303XX_OK)
         {
            printf("   ERROR %d calling zl303xx_SyncConfigGet().\n", status);
         }
      }

      if (status == ZL303XX_OK)
      {
         printf("\n<Sync Configuration>\n");
         printf("   Enabled: %s\n", ZL303XX_BOOL_TO_STR(syncConfig.enable));
         printf("   Invert:  %s\n", ZL303XX_BOOL_TO_STR(syncConfig.invert));
      }
   }

   return status;
}

/*******************************************************************************

  Function Name:
   local_DebugDpllStatus

  Details:
   Internal function to print DPLL status.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device instance parameter structure
   [in]   dpllId         DPLL number to display status for (0-1).

  Return Value:
   zlStatusE

*******************************************************************************/
static zlStatusE local_DebugDpllStatus(zl303xx_ParamsS *zl303xx_Params, zl303xx_DpllIdE dpllId)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_DpllStatusS dpllStatus;

   printf("\nDPLL%d\n=====\n", dpllId + 1);

   if (status == ZL303XX_OK)
   {
      dpllStatus.Id = dpllId;

      status = zl303xx_DpllStatusGet(zl303xx_Params, &dpllStatus);

      if (status != ZL303XX_OK)
      {
         printf("   ERROR %d calling zl303xx_DpllStatusGet().\n", status);
      }
   }

   if (status == ZL303XX_OK)
   {
      printf("<Status>\n");
      printf("   Holdover:       %s\n", ZL303XX_BOOL_TO_STR(dpllStatus.holdover));
      printf("   Locked:         %s\n", ZL303XX_BOOL_TO_STR(dpllStatus.locked));
      printf("   Cur. Ref. Fail: %s\n", ZL303XX_BOOL_TO_STR(dpllStatus.refFailed));

      if (dpllId == ZL303XX_DPLL_ID_1)
      {
         Sint32T freqOffset = 0;

         status = zl303xx_DcoGetFreq(zl303xx_Params, &freqOffset);

         if (status == ZL303XX_OK)
         {
            printf("   Freq. Offset:   %d uppm\n", freqOffset);
         }
         else
         {
            printf("   ERROR %d calling zl303xx_DcoGetFreq().\n", status);
         }
      }
   }

   return status;
}

/*******************************************************************************

  Function Name:
   local_DebugDpllConfig

  Details:
   Internal function to print DPLL config.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device instance parameter structure
   [in]   dpllId         DPLL number to display configuration for (0-1).

  Return Value:
   zlStatusE

*******************************************************************************/
static zlStatusE local_DebugDpllConfig(zl303xx_ParamsS *zl303xx_Params, zl303xx_DpllIdE dpllId)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_DpllConfigS dpllConfig;

   printf("\nDPLL%d\n=====\n", dpllId + 1);

   /* DPLL Configuration */
   if (status == ZL303XX_OK)
   {
      dpllConfig.Id = dpllId;

      status = zl303xx_DpllConfigGet(zl303xx_Params, &dpllConfig);

      if (status != ZL303XX_OK)
      {
         printf("   ERROR %d calling zl303xx_DpllConfigGet().\n", status);
      }
   }

   if (status == ZL303XX_OK)
   {
      printf("<Configuration>\n");
      printf("   Hitless Ref. Sw.: %d (%s)\n", dpllConfig.hitlessSw,     ZL303XX_BOOL_TO_STR(dpllConfig.hitlessSw));
      printf("   Bandwidth:        %d (%s)\n", dpllConfig.bandwidth,     zl303xx_DebugValToString(dpllBwToString, dpllConfig.bandwidth));
      printf("   Phase Slope Lim.: %d (%s)\n", dpllConfig.phaseSlope,    zl303xx_DebugValToString(dpllPslToString, dpllConfig.phaseSlope));
      printf("   Revertive Switch: %d (%s)\n", dpllConfig.revertEn,      ZL303XX_BOOL_TO_STR(dpllConfig.revertEn));
      printf("   Holdover Update:  %d (%s)\n", dpllConfig.hldUpdateTime, zl303xx_DebugValToString(dpllHoldUpdateToString, dpllConfig.hldUpdateTime));
      printf("   Holdover Filter:  %d (%s)\n", dpllConfig.hldFilterBw,   zl303xx_DebugValToString(dpllHoldFiltBwToString, dpllConfig.hldFilterBw));
      printf("   Pull-in Range:    %d (%s)\n", dpllConfig.pullInRange,   zl303xx_DebugValToString(dpllPullInRangeToString, dpllConfig.pullInRange));
      printf("   Mode Selection:   %d (%s)\n", dpllConfig.mode,          zl303xx_DebugValToString(dpllModeSelToString, dpllConfig.mode));
   }

   /* DPLL-Reference Configuration */
   {
      zl303xx_DpllMaskConfigS dpllMaskConfig;

      if (status == ZL303XX_OK)
      {
         dpllMaskConfig.Id = dpllId;

         status = zl303xx_DpllMaskConfigGet(zl303xx_Params, &dpllMaskConfig);

         if (status != ZL303XX_OK)
         {
            printf("   ERROR %u calling zl303xx_DpllMaskConfigGet().\n", status);
         }
      }

      if (status == ZL303XX_OK)
      {
         printf("\n<Reference Configuration>\n");
         printf("   Ref Selected:      %d\n",     dpllConfig.selectedRef);
         printf("   Wait to Restore:   %u min\n", dpllConfig.waitToRestore);
         printf("   Ref Switch Mask:   SCM:%d CFM:%d GST:%d PFM:%d\n", dpllMaskConfig.scmRefSwEn,
                                                                       dpllMaskConfig.cfmRefSwEn,
                                                                       dpllMaskConfig.gstRefSwEn,
                                                                       dpllMaskConfig.pfmRefSwEn);
         printf("   Ref Holdover Mask: SCM:%d CFM:%d GST:%d PFM:%d\n", dpllMaskConfig.scmHoldoverEn,
                                                                       dpllMaskConfig.cfmHoldoverEn,
                                                                       dpllMaskConfig.gstHoldoverEn,
                                                                       dpllMaskConfig.pfmHoldoverEn);
      }
   }

   /* DPLL-Reference Priorities */
   {
      zl303xx_DpllRefConfigS dpllRefConfig;

      if (status == ZL303XX_OK)
      {
         dpllRefConfig.Id = ZL303XX_DPLL_ID_1;
         dpllRefConfig.refId = ZL303XX_REF_ID_0;

         printf("\n<Reference Priorities>\n");

         for (; ZL303XX_CHECK_REF_ID(dpllRefConfig.refId) == ZL303XX_OK; dpllRefConfig.refId++)
         {
            printf("   Ref%d: ", dpllRefConfig.refId);

            if (zl303xx_DpllRefConfigGet(zl303xx_Params, &dpllRefConfig) == ZL303XX_OK)
            {
               printf("Priority: %u, Revertive: %s\n", dpllRefConfig.priority,
                                                       ZL303XX_BOOL_TO_STR(dpllRefConfig.revertEn));
            }
            else
            {
               printf("ERROR %d calling zl303xx_DpllRefConfigGet().\n", status);
            }
         }
      }
   }

   return status;
}


static char const *zl303xx_DebugValToString(zl303xx_DebugValToStringS valToStr[],
                                     Uint32T val)
{
   Uint32T strIndex = 0;
   char const *retStr = invalidStr;

   while (valToStr[strIndex].string != NULL)
   {
      if (valToStr[strIndex].value == val)
      {
         retStr = valToStr[strIndex].string;
         break;
      }

      strIndex++;
   }

   return retStr;
}




/* zl303xx_DebugApiBuildInfo */
/**
   Displays information about the API release and build date/times.

*******************************************************************************/
void zl303xx_DebugApiBuildInfo(void)
{
   printf("API Build Information\n" \
          "=====================\n" \
          "Release Version: %s\n" \
          "Release Date:    %s\n" \
          "Release Time:    %s\n" \
          "Release SW ID:   %s\n" \
          "Build Date:      %s\n" \
          "Build Time:      %s\n",
          zl303xx_ApiReleaseVersion, zl303xx_ApiReleaseDate, zl303xx_ApiReleaseTime,
          zl303xx_ApiReleaseSwId, zl303xx_ApiBuildDate, zl303xx_ApiBuildTime);
}

/*****************   END   ****************************************************/
