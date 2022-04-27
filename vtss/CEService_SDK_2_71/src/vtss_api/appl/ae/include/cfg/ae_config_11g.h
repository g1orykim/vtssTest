/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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


#include "ae.h"

/* The number of links being trained. This is only used in ae_main.c and ae_run.c.
	 You may prefer to use different mechanisms for training multiple links.
 */
#define LINKS             7

/* The peaking amp gain, 0 to 7. If AE_PEAK_AMP < 0, the peaking amp gain is
   left as is (unchanged).*/
#define AE_PEAK_AMP       7

/* The DFE mode, as given in the core databook. 0=DFE5, if AE_DFE_MODE < 0,
   the dfe mode is left as is.*/
#define AE_DFE_MODE       0

/* Set to 1 if the system uses the AEC macro (Adaptive Equalization Core for
	 Ethernet...)
   Will not work automatically with AE_USE_AEC = 0 and a local TX (loopback),
	 (ae_set_req() and ae_get_status() must use registers 0xE and 0xF)! 
*/
#define AE_USE_AEC        1

/* Modifications past this point will affect the adaptive equalization
	 algorithm. */
#define AE_INIT_METH      3
#define AE_CE_DECS        3
#define AE_EN_ZFE         1
#define AE_EN_H1T_EQ      1
#define AE_EN_AI_C1       1
#define AE_EN_MAX_ST      1
#define AE_EN_AMIN_TH     1
#define AE_EN_SD_TH       0
#define AE_EN_GAIN_TOG    1

#define AE_ACC_LIM        (1<< 8)
#define AE_CNV_LIM        (1<<12)
#define AE_TOG_LIM        4

#define AE_GAIN_TH        15
#define AE_AMIN_TH        20
#define AE_H1TEQ_GOAL     4

#define AE_EST_PRE        3
#define AE_EST_POST123    0
#define AE_EST_POST45     5
#define AE_EST_POST6      9
#define AE_EST_POST7      12

#define AE_SAMP_REQ_LIM   128

