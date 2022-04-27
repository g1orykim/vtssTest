

/******************************************************************************
*
*  $Id: zl303xx_PllFuncs.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*
*     This provides functions to convert from register values to physical values
*     and vice versa. It also provides functions to calculate recommended
*     monitor values for custom input frequencies.
*
******************************************************************************/

#ifndef _ZL303XX_PLL_FUNCS_H
#define _ZL303XX_PLL_FUNCS_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_Global.h"
#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES     **********************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

zlStatusE zl303xx_ClosestSynthClk0FreqReg(Uint32T freq, Uint32T *regVal);
Uint32T zl303xx_SynthClk0FreqInKHz(Uint32T regVal);
zlStatusE zl303xx_ClosestSynthClk1DivReg(Uint32T clk0Freq, Uint32T clk1Freq, Uint32T *regVal);
Uint32T zl303xx_SynthClk1FreqInKHz(Uint32T regVal, Uint32T clk0Freq);
zlStatusE zl303xx_CheckSynthClkFreq(const zl303xx_ClockIdE clkId, const Uint32T freq);

zlStatusE zl303xx_ClosestOutputFineOffsetReg(Sint32T offset, Uint32T *regVal);
Sint32T zl303xx_OutputFineOffsetInPs(Uint32T regVal);
zlStatusE zl303xx_CheckOutputFineOffset(const Sint32T offset);

zlStatusE zl303xx_ClosestSynthFpOffsetReg(Uint32T offset, Uint32T *fineReg, Uint32T *coarseReg);
Uint32T zl303xx_SynthFpOffsetInNs(Uint32T fine, Uint32T coarse);
zlStatusE zl303xx_ClosestSdhFpOffsetReg(Uint32T offset, Uint32T *fineReg, Uint32T *coarseReg, Uint32T sdhClkDiv3);
Uint32T zl303xx_SdhFpOffsetInNs(Uint32T fine, Uint32T coarse, Uint32T sdhClkDiv3);

zlStatusE zl303xx_CheckFpOffset(const Uint32T offset);

zlStatusE zl303xx_ClosestCustFreqReg(Uint32T freq, Uint32T *regVal);
Uint32T zl303xx_CustFreqInKHz(Uint32T regVal);
zlStatusE zl303xx_CheckCustFreq(const Uint32T freq);

zlStatusE zl303xx_ClosestScmLimitReg(Uint32T scmLimit, Uint32T *regVal);
Uint32T zl303xx_ScmLimitInNs(Uint32T regVal);
zlStatusE zl303xx_CheckScmLimit(const Uint32T scmVal);

zlStatusE zl303xx_ClosestCfmLimitReg(Uint32T cfmLimit, Uint32T *regVal);
Uint32T zl303xx_CfmLimitInNs(Uint32T regVal);
zlStatusE zl303xx_CheckCfmLimit(const Uint32T cfmVal);

zlStatusE zl303xx_CfmCyclesReg(Uint32T cycles, Uint32T *regVal);
Uint32T zl303xx_CfmCycles(Uint32T regVal);
zlStatusE zl303xx_CheckCfmCycles(Uint32T cycles);

zlStatusE zl303xx_RecommendScmLo(Uint32T *freq);
zlStatusE zl303xx_RecommendScmHi(Uint32T *freq);
zlStatusE zl303xx_RecommendCfmLoInNs(Uint32T *freq, Uint32T cfmDiv, Uint32T cycles);
zlStatusE zl303xx_RecommendCfmHiInNs(Uint32T *freq, Uint32T cfmDiv, Uint32T cycles);

Uint32T zl303xx_RecommendCfmRefDivider(Uint32T freq);
Uint32T zl303xx_RecommendCfmCycles(Uint32T freq);

zlStatusE zl303xx_RecommendCustMonitors(zl303xx_CustConfigS *par);


#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */
