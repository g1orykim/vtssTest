

/******************************************************************************
*
*  $Id: zl303xx_PllFuncs.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This provides functions to convert from register values to physical values
*     and vice versa. It also provides functions to calculate recommended
*     monitor values for custom input frequencies.
*
******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_Global.h"
#include "zl303xx_PllFuncs.h"
#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES     **********************************************/
#define MAX_SYNTH_CLK_FREQ 131064 /* 0x3FFF * 8 kHz (highest px_clk0 frequency) */

#define MAX_SYNTH_CLK0_REGVAL 16383
#define MIN_SYNTH_CLK1_REGVAL -32
#define MAX_SYNTH_CLK1_REGVAL 31

#define MIN_OUTPUT_FINE_OFFSET -15360
#define MAX_OUTPUT_FINE_OFFSET 15240
#define MIN_OUTPUT_FINE_OFFSET_REGVAL -128
#define MAX_OUTPUT_FINE_OFFSET_REGVAL 127

#define MAX_SDH_FINE_OFFSET_REGVAL 38879
#define MAX_SDH_FINE_OFFSET_DIV3_REGVAL 25919

#define MAX_FP_OFFSET 8000000
#define MAX_FP_FINE_OFFSET_REGVAL 32767

#define MAX_CUST_FREQ 77760
#define MAX_CUST_FREQ_REGVAL 9720
#define MIN_CUST_FREQ 2
#define MIN_CUST_SCM_FREQ 1768 /* 1,765 kHz rounded up to the nearest 8 kHz */

#define MAX_SCM_REGVAL 255
#define MAX_CFM_REGVAL 65535

#define MAX_SCM_VALUE 850
#define MAX_CFM_VALUE 819187
#define MAX_CFM_CYCLES_VALUE 256

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/
_ZL303XX_LOCAL Uint32T divideAndRound(Uint32T numerator, Uint32T denominator);
_ZL303XX_LOCAL Sint32T divideAndRoundSigned(Sint32T numerator, Sint32T denominator);

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/

/*

  Function Name:
   zl303xx_ClosestSynthClk0FreqReg

  Details:
   Calculates the synthesizer clock 0 register value that corresponds
   to the freqency passed in. If frequency does not correspond exactly to a
   register value, the closest register value is returned.

  Parameters:
   [in]    freq    Freqency in kHz

   [out]    regVal    multiple of 8 kHz to be written to register (14 bits)

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_ClosestSynthClk0FreqReg(Uint32T freq, Uint32T *regVal)
{
   zlStatusE status = ZL303XX_CHECK_POINTER(regVal);

   if (status == ZL303XX_OK)
   {
      /* Ensure that a value larger than 8 KHz x 16383 is not being set
         Note: this allows the customer to exceed officially supported device limits. */
      if (freq <= MAX_SYNTH_CLK_FREQ)
      {
         *regVal = divideAndRound(freq, 8);

         /* clip to largest register value */
         if(*regVal > MAX_SYNTH_CLK0_REGVAL)
         {
            *regVal = MAX_SYNTH_CLK0_REGVAL;
         }
      }
      else
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   return status;
}


/*

  Function Name:
   zl303xx_SynthClk0FreqInKHz

  Details:
   Calculates the frequency of a synth's Clk0 in kHz.

  Parameters:
   [in]    regVal    Value from pX_clk0_freq (stored as a multiple of 8 kHz)

  Return Value:
   Uint32T Frequency in kHz

*******************************************************************************/
Uint32T zl303xx_SynthClk0FreqInKHz(Uint32T regVal)
{
   return ((regVal > 0) ? (regVal * 8) : (2));
}


/*

  Function Name:
   zl303xx_ClosestSynthClk1DivReg

  Details:
   Calculates the synthesizer Clk1 register value that corresponds
   to the freqency passed in. If frequency does not correspond exactly to a
   register value, the closest register value is returned. Clk1 is defined
   as a multiple of Clk0.

  Parameters:
   [in]    clk0Freq    Freqency of Clk0 in kHz
   [in]    clk1Freq    Desired frequency of Clk1

   [out]    regVal      value of x that gets ( clk0Freq / 2^x ) closest to clk1Freq
               (6 bits)

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_ClosestSynthClk1DivReg(Uint32T clk0Freq, Uint32T clk1Freq, Uint32T *regVal)
{
   zlStatusE status = ZL303XX_CHECK_POINTER(regVal);

   Uint32T searching = 1, currentVal = clk0Freq, lastVal;
   Sint32T divider = 0;

   /* some arbitrary limits set so integer overflow does not occur */
   if (status == ZL303XX_OK)
   {
      if ((clk1Freq > 0) && (clk1Freq <= MAX_SYNTH_CLK_FREQ))
      {
         while (searching)
         {
            /* If clk1Freq needs to be set smaller than clk0Freq */
            if (clk1Freq < currentVal)
            {
               lastVal = currentVal;
               currentVal /= 2;
               divider++;

               /* stop looping if currentVal becomes too small */
               if ((clk1Freq >= currentVal) || (currentVal == 0))
               {
                  searching = 0;
               }
            }

            /* If clk1Freq needs to be set larger than clk0Freq */
            else
            {
               lastVal = currentVal;
               currentVal *= 2;
               divider--;

               /* stop looping if currentVal becomes too large */
               if ((clk1Freq <= currentVal) || (currentVal > MAX_SYNTH_CLK_FREQ))
               {
                  searching = 0;
               }
            }
         }

         /* correct for divider off by 1 */
         if ((divider < 0) && ((clk1Freq - lastVal) < (currentVal - clk1Freq)))
         {
            divider++;
         }
         else if ((divider >= 0) && ((lastVal - clk1Freq) < (clk1Freq - currentVal)))
         {
            divider--;
         }

         /* clip divider to maximum/minimum register values */
         if (divider > MAX_SYNTH_CLK1_REGVAL)
         {
            divider = MAX_SYNTH_CLK1_REGVAL;
         }
         else if (divider < MIN_SYNTH_CLK1_REGVAL)
         {
            divider = MIN_SYNTH_CLK1_REGVAL;
         }

         *regVal = divider & 0x3F;
      }
      else
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   return status;
}


/*

  Function Name:
   zl303xx_SynthClk1FreqInKHz

  Details:
   Calculates the frequency of a synthesizer's Clk1 based on the Clk0
   frequency and Clk1's divider

  Parameters:
   [in]    regVal    Clk1 divider value (6-bit signed int)
   [in]    clk0Freq  Frequency of Clk0 in kHz

  Return Value:
   Uint32T   Frequency of Clk1 in kHz

*******************************************************************************/
Uint32T zl303xx_SynthClk1FreqInKHz(Uint32T regVal, Uint32T clk0Freq)
{
   Sint32T clkDiv = regVal;
   Uint32T clk1Freq;

   /* if regVal is negative (bit5 = 1) */
   if (regVal & (1 << 5))
   {
      /* do sign extension */
      clkDiv |= 0xFFFFFFC0;

      /* get absolute value of divider (to use as a shift value) */
      clkDiv = -clkDiv;

      /* clk1Freq = clk0Freq / (2 ^ regVal) */
      clk1Freq = clk0Freq << clkDiv;
   }

   /* If regVal is positive, divide clk0 (equivalent to a right shift) */
   else
   {
      clk1Freq = clk0Freq >> clkDiv;
   }

   return clk1Freq;
}


/*

  Function Name:
   zl303xx_CheckSynthClkFreq

  Details:
   Checks that the frequency value for a synth clock output is valid.

  Parameters:
   [in]    clkId    Clock Id
   [in]    freq     Frequency in kHz

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_CheckSynthClkFreq(const zl303xx_ClockIdE clkId, const Uint32T freq)
{
   zlStatusE status = ZL303XX_CHECK_CLOCK_ID(clkId);
   Uint32T maxFreq = 0;

   if (status == ZL303XX_OK)
   {
      switch(clkId)
      {
         case ZL303XX_CLOCK_ID_0 :
         case ZL303XX_CLOCK_ID_1 :  /* Intentional fall-through */
            maxFreq = MAX_SYNTH_CLK_FREQ;
            break;

         default :
            break;
      }

      if (freq > maxFreq)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   return status;
}


/*

  Function Name:
   zl303xx_ClosestOutputFineOffsetReg

  Details:
   Calculates the closest register value for a synthesizer's fine offset in ps.

  Parameters:
   [in]    offset    All clocks and frame pulses delayed by this value in ps.

   [out]    regVal    Register value to be written

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_ClosestOutputFineOffsetReg(Sint32T offset, Uint32T *regVal)
{
   zlStatusE status = ZL303XX_OK;

   /* Sythesizer output clock frequency cannot exceed device limitations */
   if ((offset >= MIN_OUTPUT_FINE_OFFSET) && (offset <= MAX_OUTPUT_FINE_OFFSET))
   {
      /* Offset is defined as multiples of 119.2 ps */
      offset = divideAndRoundSigned((offset * 10), 1192);

      /* clip value to register limits */
      if (offset < MIN_OUTPUT_FINE_OFFSET_REGVAL)
      {
         offset = MIN_OUTPUT_FINE_OFFSET_REGVAL;
      }
      else if (offset > MAX_OUTPUT_FINE_OFFSET_REGVAL)
      {
         offset = MAX_OUTPUT_FINE_OFFSET_REGVAL;
      }

      *regVal = offset;
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_OutputFineOffsetInPs

  Details:
   Calculates the fine offset for a synthesizer's clock and frame pulse outputs
   based on the fine offset register value.

  Parameters:
   [in]    regVal    Value stored in the synth fine offset register (8-bit signed)

  Return Value:
   Sint32T Fine offset in ps.

*******************************************************************************/
Sint32T zl303xx_OutputFineOffsetInPs(Uint32T regVal)
{
   Sint32T offset = regVal;

   /* if regVal is negative (bit7 = 1), do sign extension */
   if (regVal & (1 << 7))
   {
      offset = offset | 0xFFFFFF00;
   }

   /* Fine offset is defined as a multiple of 119.2 ps */
   return (divideAndRoundSigned((offset * 1192), 10));
}


/*

  Function Name:
   zl303xx_CheckOutputFineOffset

  Details:
   Checks that the fine offset for a Synth/SDH is within valid range.

  Parameters:
   [in]    offset     Fine offset in ps.

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_CheckOutputFineOffset(const Sint32T offset)
{
   zlStatusE status;

   if ((offset >= MIN_OUTPUT_FINE_OFFSET) && (offset <= MAX_OUTPUT_FINE_OFFSET))
   {
      status = ZL303XX_OK;
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_ClosestSynthFpOffsetReg

  Details:
   Calculates the closest register value for a synthesizer's frame pulse
   offset.

  Parameters:
   [in]    offset     Frame pulse offset value in ns. Cannot be greater than
              8,000,000 ns (8 ms).

   [out]    fineReg    Value to be written to fine offset registers 0 and 1
   [out]    coarseReg  Value to be written to the coarse offset register

  Return Value:
   zlStatusE

  Notes:
   Will only produce proper register values when the associated synth/clock
   output is an E1 (2.048 MHz) multiple. e.g., this function will produce
   proper results for p1_fp0 if p1_clk0 is 4.096 MHz (or some other E1 multiple).

*******************************************************************************/
zlStatusE zl303xx_ClosestSynthFpOffsetReg(Uint32T offset, Uint32T *fineReg, Uint32T *coarseReg)
{
   zlStatusE status = ZL303XX_OK;

   if (offset <= MAX_FP_OFFSET)
   {
      /* Coarse offset is a multiple of an 8 kHz cycle (125 us) */
      *coarseReg = offset / 125000;
      offset = offset % 125000;

      /* Fine offset is a multiple of a 262.144 MHz cycle (3.8147 ns) */
      *fineReg = divideAndRound(offset * 10000, 38147);

      /* clip to largest possible value for fine offset registers */
      if (*fineReg > MAX_FP_FINE_OFFSET_REGVAL)
      {
         *fineReg = MAX_FP_FINE_OFFSET_REGVAL;
      }
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_SynthFpOffsetInNs

  Details:
   Calculates a synthesizer's frame pulse offset in ns from a register value.

  Parameters:
   [in]    fine    Value from the fine offset 0 and 1 registers
   [in]    coarse  Value from the coarse offset register

  Return Value:
   Uint32T  Offset in ns

  Notes:
    Will only produce proper results when the associated synth/clock
    output is an E1 (2.048 MHz) multiple. e.g., this function will produce
    proper results for p1_fp0 if p1_clk0 is 4.096 MHz (or some other E1 multiple).

*******************************************************************************/
Uint32T zl303xx_SynthFpOffsetInNs(Uint32T fine, Uint32T coarse)
{
   /* Fine offset is a multiple of a 262.144 MHz cycle (3.8147 ns) */
   return (coarse * 125000) + divideAndRound(fine * 38147, 10000);
}



/*

  Function Name:
   zl303xx_ClosestSdhFpOffsetReg

  Details:
   Calculates the closest register value for an APLL frame pulse output.

  Parameters:
   [in]    offset      frame pulse offset in ns
   [in]    sdhClkDiv3  value of sdhClkDiv3 bit

   [out]    fineReg     register value for fine offset registers 0 and 1
   [out]    coarseReg   register value for coarse offset register

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_ClosestSdhFpOffsetReg(Uint32T offset, Uint32T *fineReg, Uint32T *coarseReg, Uint32T sdhClkDiv3)
{
   zlStatusE status = ZL303XX_OK;

   if ((offset <= MAX_FP_OFFSET) || (sdhClkDiv3 <= 1))
   {
      /* Coarse offset is a multiple of an 8 kHz cycle (125 us) */
      *coarseReg = offset / 125000;
      offset = offset % 125000;

      /* If SDH clk is a multiple of a 311.04 MHz cycle (3.2150 ns) */
      if (sdhClkDiv3 == 0)
      {
         *fineReg = divideAndRound(offset * 10000, 32150);

         /* clip to largest possible value for fine offset registers */
         if (*fineReg > MAX_SDH_FINE_OFFSET_REGVAL)
         {
            *fineReg = MAX_SDH_FINE_OFFSET_REGVAL;
         }
      }

      /* If SDH clk is a multiple of a 207.36 MHz cycle (4.8225 ns) */
      else if (sdhClkDiv3 == 1)
      {
         *fineReg = divideAndRound(offset * 10000, 48225);

         /* clip to largest possible value for fine offset registers */
         if (*fineReg > MAX_SDH_FINE_OFFSET_DIV3_REGVAL)
         {
            *fineReg = MAX_SDH_FINE_OFFSET_DIV3_REGVAL;
         }
      }
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_SdhFpOffsetInNs

  Details:
   Calculates the APLL's fine offset for a frame pulse in ns based on a register
   value.

  Parameters:
   [in]    fine     Value from the fine offset 0 and 1 registers
   [in]    coarse   Value from the coarse offset register

   [out]    offset   APLL frame pulse offset in ns

  Return Value:
   zlStatusE

*******************************************************************************/
Uint32T zl303xx_SdhFpOffsetInNs(Uint32T fine, Uint32T coarse, Uint32T sdhClkDiv3)
{
   Uint32T offset;

   /* Coarse offset is a multiple of an 8 kHz cycle (125 us) */
   if (sdhClkDiv3 == 0)
   {
      /* If SDH clk is a multiple of a 311.04 MHz cycle (3.2150 ns) */
      offset = (coarse * 125000) + ((fine * 32150) / 10000);
   }
   else
   {
      /* If SDH clk is a multiple of a 207.36 MHz cycle (4.8225 ns) */
      offset = (coarse * 125000) + ((fine * 48225) / 10000);
   }

   return offset;
}


/*

  Function Name:
   zl303xx_CheckFpOffset

  Details:
   Checks that the frame pulse offset value is within valid range

  Parameters:
   [in]    offset   frame pulse offset in ns

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_CheckFpOffset(const Uint32T offset)
{
   zlStatusE status;

   if (offset <= MAX_FP_OFFSET)
   {
      status = ZL303XX_OK;
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_ClosestCustFreqReg

  Details:
   Calculates the closest register value needed to produce the
   custom frequency passed in

  Parameters:
   [in]    freq     desired frequency in kHz

   [out]    regVal     value to be written to register

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_ClosestCustFreqReg(Uint32T freq, Uint32T *regVal)
{
   zlStatusE status = ZL303XX_OK;

   if (freq <= MAX_CUST_FREQ)
   {
      /* Custom input frequencies are defined as multiples of 8 kHz */
      *regVal = divideAndRound(freq, 8);

      /* clip to largest register value */
      if (*regVal > MAX_CUST_FREQ_REGVAL)
      {
         *regVal = MAX_CUST_FREQ_REGVAL;
      }
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_CustFreqInKHz

  Details:
   Calculates the custom input frequency based on a register value

  Parameters:
   [in]    regVal     Value stored in custX_mult_0 & _1

  Return Value:
   Uint32T    Custom input frequency in kHz

*******************************************************************************/
Uint32T zl303xx_CustFreqInKHz(Uint32T regVal)
{
   Uint32T retVal;

   if (regVal > 0)
   {
      /* Custom frequencies are stored as a multiple of 8 kHz */
      retVal = regVal * 8;
   }
   else
   {
      /* A register value of 0 will output a 2 kHz clock */
      retVal = 2;
   }

   return retVal;
}

/*

  Function Name:
   zl303xx_CheckCustFreq

  Details:
   Checks that the custom input frequency value is valid.

  Parameters:
   [in]    freq     Frequency in kHz

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_CheckCustFreq(const Uint32T freq)
{
   zlStatusE status;

   if (freq <= MAX_CUST_FREQ)
   {
      status = ZL303XX_OK;
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_ClosestScmLimitReg

  Details:
   Calculates the SCM limit register value

  Parameters:
   [in]    scmLimit     SCM low or high limit in ns

   [out]    regVal       SCM low or high limit register value

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_ClosestScmLimitReg(Uint32T scmLimit, Uint32T *regVal)
{
   zlStatusE status = ZL303XX_OK;

   if (scmLimit <= MAX_SCM_VALUE)
   {
      /* SCM limits are defined as multiples of 3.333.. ns */
      *regVal = divideAndRound(scmLimit * 1000000, 3333333);

      /* clip to largest register value */
      if (*regVal > MAX_SCM_REGVAL)
      {
         *regVal = MAX_SCM_REGVAL;
      }
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_ScmLimitInNs

  Details:
   Calculates the SCM limit in ns

  Parameters:
   [in]    regVal     SCM low or high limit register value

  Return Value:
   Uint32T    SCM low or high limit in ns

*******************************************************************************/
Uint32T zl303xx_ScmLimitInNs(Uint32T regVal)
{
   /* SCM limits are defined as multiples of 3.333.. ns */
   return divideAndRound(regVal * 3333333, 1000000);
}


/*

  Function Name:
   zl303xx_CheckCustScmLimit

  Details:
   Checks that the SCM limit value is valid.

  Parameters:
   [in]    scmVal     SCM limit in ns

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_CheckScmLimit(const Uint32T scmVal)
{
   zlStatusE status;

   if (scmVal <= MAX_SCM_VALUE)
   {
      status = ZL303XX_OK;
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_ClosestCfmLimitReg

  Details:
   Calculates the CFM limit register value

  Parameters:
   [in]    cfmLimit     CFM low or high limit in ns

   [out]    regVal       CFM low or high limit register value

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_ClosestCfmLimitReg(Uint32T cfmLimit, Uint32T *regVal)
{
   zlStatusE status = ZL303XX_OK;

   if (cfmLimit <= MAX_CFM_VALUE)
   {
      /* CFM limits are defined as multiples of 12.5 ns */
      *regVal = divideAndRound(cfmLimit * 1000, 12500);

      /* clip to largest register value */
      if (*regVal > MAX_CFM_REGVAL)
      {
         *regVal = MAX_CFM_REGVAL;
      }
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_CfmLimitInNs

  Details:
   Calculates the CFM limit in ns

  Parameters:
   [in]    regVal     CFM low or high limit register value

  Return Value:
   Uint32T    CFM low or high limit in ns

*******************************************************************************/
Uint32T zl303xx_CfmLimitInNs(Uint32T regVal)
{
   /* CFM limits are defined as multiples of 12.5 ns */
   return divideAndRound(regVal * 12500, 1000);
}


/*

  Function Name:
   zl303xx_CheckCfmLimit

  Details:
   Checks that the CFM limit value is valid.

  Parameters:
   [in]    cfmVal     CFM limit in ns

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_CheckCfmLimit(const Uint32T cfmVal)
{
   zlStatusE status;

   if (cfmVal <= MAX_CFM_VALUE)
   {
      status = ZL303XX_OK;
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}

/*

  Function Name:
   zl303xx_CfmCyclesReg

  Details:
   Calculates the CFM monitoring cycles register value

  Parameters:
   [in]    cycles     Number of cycles the CFM will monitor

   [out]    regVal     Number of cycles the CFM will monitor register value

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_CfmCyclesReg(Uint32T cycles, Uint32T *regVal)
{
   zlStatusE status = ZL303XX_OK;

   if (cycles <= MAX_CFM_CYCLES_VALUE)
   {
      /* CFM monitoring cycles are stored as (N - 1) */
      *regVal =  cycles - 1;
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_CfmCycles

  Details:
   Calculates the number of CFM monitoring cycles stored

  Parameters:
   [in]    regVal     CFM monitoring cycles register value

  Return Value:
   Uint32T    CFM monitoring cycles

*******************************************************************************/
Uint32T zl303xx_CfmCycles(Uint32T regVal)
{
   return (regVal + 1);
}


/*

  Function Name:
   zl303xx_CheckCfmCycles

  Details:
   Checks that the CFM monitoring cycles value is valid.

  Parameters:
   [in]    cycles     Number of CFM monitoring cycles

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_CheckCfmCycles(Uint32T cycles)
{
   zlStatusE status;

   if (cycles <= MAX_CFM_CYCLES_VALUE)
   {
      status = ZL303XX_OK;
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_RecommendScmLo

  Details:
   Calculates the recommended SCM Low value based on the custom input
   frequency. This is defined to be -50% of the nominal period.

   SCM_Low[ns] = 0.5 / (freq[kHz] * 10^-6)

  Parameters:
   [in]    freq     Custom input frequency in kHz

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_RecommendScmLo(Uint32T *freq)
{
   zlStatusE status = ZL303XX_OK;

   if (*freq <= MAX_CUST_FREQ)
   {
      *freq = divideAndRound(500000, *freq);
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_RecommendScmHi

  Details:
   Calculates the recommended SCM High value based on the custom input
   frequency. This is defined to be -50% of the nominal period.

   SCM_High[ns] = 1.5 / (freq[kHz] * 10^-6)

  Parameters:
   [in]    freq     Custom input frequency in kHz

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_RecommendScmHi(Uint32T *freq)
{
   zlStatusE status = ZL303XX_OK;

   if (*freq <= MAX_CUST_FREQ)
   {
      *freq = divideAndRound(1500000, *freq);
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_RecommendCfmLoInNs

  Details:
   Calculates the recommended CFM Low value based on the custom input
   frequency. This is defined below:

   CFM_Low[ns] = (cfmDiv / (freq[kHz] * 10^-6 * 1.03)) * cycles

  Parameters:
   [in]    freq     Custom input frequency in kHz
   [in]    cfmDiv   CFM reference input divider
   [in]    cycles   Number of input reference cycles to be checked

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_RecommendCfmLoInNs(Uint32T *freq, Uint32T cfmDiv, Uint32T cycles)
{
   zlStatusE status = ZL303XX_OK;

   if ((*freq >= MIN_CUST_FREQ) && (*freq <= MAX_CUST_FREQ))
   {
      /* in above eq'n, simplify: 1 / (1.03 x 10^-6) to 970874 */
      *freq = divideAndRound(cfmDiv * cycles * 970874, *freq);
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_RecommendCfmHiInNs

  Details:
   Calculates the recommended CFM High value based on the custom input
   frequency. This is defined below:

   CFM_High[ns] = (cfmDiv / (freq[kHz] * 10^-6 * 0.97)) * cycles

  Parameters:
   [in]    freq     Custom input frequency in kHz
   [in]    cfmDiv   CFM reference input divider
   [in]    cycles   Number of input reference cycles to be checked

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_RecommendCfmHiInNs(Uint32T *freq, Uint32T cfmDiv, Uint32T cycles)
{
   zlStatusE status = ZL303XX_OK;

   if ((*freq >= MIN_CUST_FREQ) && (*freq <= MAX_CUST_FREQ))
   {
      /* in above eq'n, simplify: 1 / (0.97 x 10^-6) to 1030928 */
      *freq = divideAndRound(cfmDiv * cycles * 1030928, *freq);
   }
   else
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   return status;
}


/*

  Function Name:
   zl303xx_RecommendCfmRefDivider

  Details:
   Returns the divider the CFM uses on the reference input.

  Parameters:
   [in]    freq     frequency in kHz

  Return Value:
   Uint32T  CFM divider value

*******************************************************************************/
Uint32T zl303xx_RecommendCfmRefDivider(Uint32T freq)
{
   return (freq <= 19440) ? 1 : 4;
}


/*

  Function Name:
   zl303xx_RecommendCfmCycles

  Details:
   Calculates the divider the CFM uses on the reference input.

  Parameters:
   [in]    freq     frequency in kHz

  Return Value:
   Uint32T  Number of reference cycles expected in the CFM sample window

*******************************************************************************/
Uint32T zl303xx_RecommendCfmCycles(Uint32T freq)
{
   Uint32T cycles;

   /* 2 kHz < freq <= 64 kHz */
   if (freq <= 64)
   {
      cycles = 1;
   }

   /* 64 kHz < freq <= 2.048 MHz */
   else if (freq <= 2048)
   {
      cycles = 32;
   }

   /* 2.048 MHz < freq <= 8.192 MHz */
   else if (freq <= 8192)
   {
      cycles = 128;
   }

   /* 8.192 MHz < freq <= 19.44 MHz */
   else if (freq <= 19440)
   {
      cycles = 256;
   }

   /* 19.44 MHz < freq <= 38.88 MHz */
   else if (freq <= 38880)
   {
      cycles = 128;
   }

   /* freq > 38.88 MHz */
   else
   {
      cycles = 256;
   }

   return cycles;
}


/*

  Function Name:
   zl303xx_RecommendCustMonitors

  Details:
   Calculates recommended values for SCM Low/High, CFM Low/High, monitoring
   cycles, and CFM reference divider.

  Parameters:
   [in]    par     structure containing the custom input frequency information

   [out]   par

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_RecommendCustMonitors(zl303xx_CustConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   zlStatusE recStat = ZL303XX_OK;
   Uint32T monitorVal, cycles, cfmDiv;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(par);
   }

   /* Check for a valid CustId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(par->Id);
   }

   /* Fill config structure with recommended values */
   if (status == ZL303XX_OK)
   {
      if (par->freq >= MIN_CUST_SCM_FREQ)
      {
         /* CFM divider and monitoring cycles */
         cycles = zl303xx_RecommendCfmCycles(par->freq);
         cfmDiv = zl303xx_RecommendCfmRefDivider(par->freq);

         /* SCM Lo Limit */
         monitorVal = par->freq;
         recStat = zl303xx_RecommendScmLo(&monitorVal);

         /* If an error occurs, do not change the value in the config
          * struct, but return the error code. */
         if (recStat == ZL303XX_OK)
         {
            par->scmLo = monitorVal;
         }
         else
         {
            status |= recStat;
         }

         /* SCM Hi Limit */
         monitorVal = par->freq;
         recStat = zl303xx_RecommendScmHi(&monitorVal);

         if (recStat == ZL303XX_OK)
         {
            par->scmHi = monitorVal;
         }
         else
         {
            status |= recStat;
         }

         /* CFM Lo Limit */
         monitorVal = par->freq;
         recStat = zl303xx_RecommendCfmLoInNs(&monitorVal, cfmDiv, cycles);

         if (recStat == ZL303XX_OK)
         {
            par->cfmLo = monitorVal;
         }
         else
         {
            status |= recStat;
         }

         /* CFM Hi Limit */
         monitorVal = par->freq;
         recStat = zl303xx_RecommendCfmHiInNs(&monitorVal, cfmDiv, cycles);

         if (recStat == ZL303XX_OK)
         {
            par->cfmHi = monitorVal;
         }
         else
         {
            status |= recStat;
         }
      }
      else
      {
         /* CFM divider and monitoring cycles */
         /* When the custom input frequency is less than 1.765 MHz, the +/- 50%
            single cycle limits suggested in the datasheet cannot be programmed
            into the SCM registers. In this case, the device can use the CFM as
            an SCM to detect loss of signal. Along with the PFM, this should be
            sufficient for qualifying lower range frequencies. */
         /* Set the CFM to monitor 1 cycle. This will force the SCM to always
            qualify. */
         cycles = 1;
         cfmDiv = 1;

         /* Use the RecommendScm functions, as they return a value in nanoseconds. */
         /* CFM Lo Limit */
         monitorVal = par->freq;
         recStat = zl303xx_RecommendScmLo(&monitorVal);

         if (recStat == ZL303XX_OK)
         {
            par->cfmLo = monitorVal;
         }
         else
         {
            status |= recStat;
         }

         /* CFM Hi Limit */
         monitorVal = par->freq;
         recStat = zl303xx_RecommendScmHi(&monitorVal);

         if (recStat == ZL303XX_OK)
         {
            par->cfmHi = monitorVal;
         }
         else
         {
            status |= recStat;
         }

         /* Set SCM limits to 0 (SCM is disabled anyway). */
         if (recStat == ZL303XX_OK)
         {
            par->scmLo = 0;
            par->scmHi = 0;
         }
      }

      /* Divider and monitoring cycles */
      par->cfmDivBy4 = (cfmDiv == 4) ? ZL303XX_TRUE : ZL303XX_FALSE;
      par->cycles = cycles;
   }

   return status;
}

/*****************   STATIC FUNCTION DEFINITIONS   ****************************/

/*

  Function Name:
   divideAndRound

  Details:
   Performs division, rounded to the nearest integer.

  Parameters:
   [in]    numerator
   [in]    denominator    2 integers

  Return Value:
   Uint32T result of division

  Notes:
   No checking is done on the size of the numbers passed in. Overflow can
   occur.

*******************************************************************************/
Uint32T divideAndRound(Uint32T numerator, Uint32T denominator)
{
   return (numerator + (denominator / 2)) / denominator;
}
/*

  Function Name:
  divideAndRoundSigned

  Details:
   Performs division, rounded to the nearest integer.

  Parameters:
   [in]    num
   [in]    den    2 integers

  Return Value:
   Sint32T   result of division

  Notes:
   No checking is done on the size of the numbers passed in. Overflow can
   occur.

*******************************************************************************/

Sint32T divideAndRoundSigned(Sint32T num, Sint32T den)
{
   Uint32T result;
   Sint32T retVal;

   result = divideAndRound(((num < 0) ? (Uint32T)(-num) : (Uint32T)num),
                           ((den < 0) ? (Uint32T)(-den) : (Uint32T)den));

   if (((num > 0) && (den > 0)) ||
       ((num < 0) && (den < 0)))
   {
      retVal = (Sint32T)result;
   }
   else
   {
      retVal = -((Sint32T)result);
   }
   return retVal;
}


/*****************   END   ****************************************************/
