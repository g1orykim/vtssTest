/*

 Vitesse Switch Software.

==========================================================================

      Vitesse Switch Software

      Register Definitions

==========================================================================
 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####
 ==========================================================================
 #####DESCRIPTIONBEGIN####

 Author(s):    lpovlsen
 Contributors: 
 Date:         2011-10-18
 Purpose:      HAL register definitions
 Description:  This file contains register definitions used by the
               HAL.

 ####DESCRIPTIONEND####

*/
#ifndef _VTSS_JAGUAR_REGS_SBA_H_
#define _VTSS_JAGUAR_REGS_SBA_H_

#include "vtss_jaguar_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a SBA
 *
 * Shared Bus arbiter registers
 *
 ***********************************************************************/

/**
 * Register Group: \a SBA:SBA
 *
 * Shared Bus arbiter registers
 */


/** 
 * \brief Arbitration Priority CPU
 *
 * \details
 * Register: \a SBA:SBA:PL1
 */
#define VTSS_SBA_SBA_PL1                     VTSS_IOREG(VTSS_TO_SBA,0x0)

/** 
 * \brief
 * Arbitration priority for CPU.
 *
 * \details 
 * Values 0x1 through 0xF, higher value is prioritized over lower value.
 *
 * Field: ::VTSS_SBA_SBA_PL1 . PL1
 */
#define  VTSS_F_SBA_SBA_PL1_PL1(x)            VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SBA_SBA_PL1_PL1               VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SBA_SBA_PL1_PL1(x)            VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Arbitration Priority Frame DMA
 *
 * \details
 * Register: \a SBA:SBA:PL2
 */
#define VTSS_SBA_SBA_PL2                     VTSS_IOREG(VTSS_TO_SBA,0x1)

/** 
 * \brief
 * Arbitration priority for Frame DMA.
 *
 * \details 
 * Values 0x1 through 0xF, higher value is prioritized over lower value.
 *
 * Field: ::VTSS_SBA_SBA_PL2 . PL2
 */
#define  VTSS_F_SBA_SBA_PL2_PL2(x)            VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SBA_SBA_PL2_PL2               VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SBA_SBA_PL2_PL2(x)            VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Arbitration Priority External CPU
 *
 * \details
 * Register: \a SBA:SBA:PL3
 */
#define VTSS_SBA_SBA_PL3                     VTSS_IOREG(VTSS_TO_SBA,0x2)

/** 
 * \brief
 * Arbitration priority for External CPU.
 *
 * \details 
 * Values 0x1 through 0xF, higher value is prioritized over lower value.
 *
 * Field: ::VTSS_SBA_SBA_PL3 . PL3
 */
#define  VTSS_F_SBA_SBA_PL3_PL3(x)            VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SBA_SBA_PL3_PL3               VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SBA_SBA_PL3_PL3(x)            VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Weighted-Token Arbitration Scheme Enable
 *
 * \details
 * When weighted token arbitration is enabled, each master on the shared
 * bus is granted a configurable number of tokens at the start of each
 * refresh period. The length of each refresh period is configurable. In
 * each clock-cycle that a master uses the bus, the token counter for that
 * master decreases. Once all tokens are spent, the master is forced to a
 * low priority. A master with tokens remaining, always takes priority over
 * masters with no tokens remaining.
 *
 * Register: \a SBA:SBA:WT_EN
 */
#define VTSS_SBA_SBA_WT_EN                   VTSS_IOREG(VTSS_TO_SBA,0x13)

/** 
 * \brief
 * Set this field to enable weighted-token arbitration scheme.
 *
 * \details 
 * Field: ::VTSS_SBA_SBA_WT_EN . WT_EN
 */
#define  VTSS_F_SBA_SBA_WT_EN_WT_EN           VTSS_BIT(0)


/** 
 * \brief Clock Tokens Refresh Period
 *
 * \details
 * Register: \a SBA:SBA:WT_TCL
 */
#define VTSS_SBA_SBA_WT_TCL                  VTSS_IOREG(VTSS_TO_SBA,0x14)

/** 
 * \brief
 * Refresh period length for the weighted-token arbitration scheme.
 *
 * \details 
 * Field: ::VTSS_SBA_SBA_WT_TCL . WT_TCL
 */
#define  VTSS_F_SBA_SBA_WT_TCL_WT_TCL(x)      VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SBA_SBA_WT_TCL_WT_TCL         VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SBA_SBA_WT_TCL_WT_TCL(x)      VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Clock Tokens CPU
 *
 * \details
 * Register: \a SBA:SBA:WT_CL1
 */
#define VTSS_SBA_SBA_WT_CL1                  VTSS_IOREG(VTSS_TO_SBA,0x15)

/** 
 * \brief
 * Number of tokens the CPU is granted at the start of each refresh period
 * for weighted-token arbitration scheme. If configured with a value of
 * zero, the master is considered to have infinite tokens.
 *
 * \details 
 * Field: ::VTSS_SBA_SBA_WT_CL1 . WT_CL1
 */
#define  VTSS_F_SBA_SBA_WT_CL1_WT_CL1(x)      VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SBA_SBA_WT_CL1_WT_CL1         VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SBA_SBA_WT_CL1_WT_CL1(x)      VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Clock Tokens Frame DMA
 *
 * \details
 * Register: \a SBA:SBA:WT_CL2
 */
#define VTSS_SBA_SBA_WT_CL2                  VTSS_IOREG(VTSS_TO_SBA,0x16)

/** 
 * \brief
 * Number of tokens the Frame DMA is granted at the start of each refresh
 * period for weighted-token arbitration scheme. If configured with a value
 * of zero, the master is considered to have infinite tokens.
 *
 * \details 
 * Field: ::VTSS_SBA_SBA_WT_CL2 . WT_CL2
 */
#define  VTSS_F_SBA_SBA_WT_CL2_WT_CL2(x)      VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SBA_SBA_WT_CL2_WT_CL2         VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SBA_SBA_WT_CL2_WT_CL2(x)      VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Clock Tokens External CPU
 *
 * \details
 * Register: \a SBA:SBA:WT_CL3
 */
#define VTSS_SBA_SBA_WT_CL3                  VTSS_IOREG(VTSS_TO_SBA,0x17)

/** 
 * \brief
 * Number of tokens the External CPU is granted at the start of each
 * refresh period for weighted-token arbitration scheme. If configured with
 * a value of zero, the master is considered to have infinite tokens.
 *
 * \details 
 * Field: ::VTSS_SBA_SBA_WT_CL3 . WT_CL3
 */
#define  VTSS_F_SBA_SBA_WT_CL3_WT_CL3(x)      VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SBA_SBA_WT_CL3_WT_CL3         VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SBA_SBA_WT_CL3_WT_CL3(x)      VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_JAGUAR_REGS_SBA_H_ */
