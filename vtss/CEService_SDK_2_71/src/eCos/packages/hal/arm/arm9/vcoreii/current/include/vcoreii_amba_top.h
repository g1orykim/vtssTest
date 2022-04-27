#ifndef _VTSS_H_VCOREII_AMBA_TOP_
#define _VTSS_H_VCOREII_AMBA_TOP_

//=============================================================================
//
//      vcoreii_amba_top.h
//
//      Platform specific support (CPU-domain register layout)
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Lars Povlsen
// Contributors:
// Date:         2006-03-22
// Purpose:      VCore-II CPU Domain Register layout
// Description: 
// Usage:        #include <cyg/hal/vcoreii.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#ifndef VTSS_BITOPS_DEFINED
#ifdef __ASSEMBLER__
#define VTSS_BIT(x)                   (1 << (x))
#define VTSS_BITMASK(x)               ((1 << (x)) - 1)
#else
#define VTSS_BIT(x)                   (1U << (x))
#define VTSS_BITMASK(x)               ((1U << (x)) - 1)
#endif
#define VTSS_EXTRACT_BITFIELD(x,o,w)  (((x) >> (o)) & VTSS_BITMASK(w))
#define VTSS_ENCODE_BITFIELD(x,o,w)   (((x) & VTSS_BITMASK(w)) << (o))
#define VTSS_ENCODE_BITMASK(o,w)      (VTSS_BITMASK(w) << (o))
#define VTSS_BITOPS_DEFINED
#endif /* VTSS_BITOPS_DEFINED */

/* Main target address offsets */
#define VTSS_TB_ICPU_CFG             0xc0000000
#define VTSS_TB_UART                 0xc0100000
#define VTSS_TB_TWI                  0xc0100400
#define VTSS_TB_SBA                  0xc0110000
#define VTSS_TB_MEMCTRL              0xc0110400
#define VTSS_TB_FDMA                 0xc0110800

/* IO address mapping macro - may be changed for platform */
#if !defined(VTSS_IOADDR)
#define VTSS_IOADDR(t,o)		(VTSS_TB_ ## t + o)
#endif

/* IO register access macro - may be changed for platform */
#if !defined(VTSS_REGISTER)
#define VTSS_REGISTER(t,o)      (*((volatile unsigned long*)(VTSS_IOADDR(t,o))))
#endif

/*********************************************************************** 
 *
 * Target ICPU_CFG
 *
 * Parallel Interface Master Configuration
 *
 ***********************************************************************/

/***********************************************************************
 * Register group CPU_SYSTEM_CTRL
 *
 * Configurations for the CPU system.
 */

#define VTSS_RB_ICPU_CFG_CPU_SYSTEM_CTRL       VTSS_IOADDR(ICPU_CFG,0x0000)

/****** Group CPU_SYSTEM_CTRL, Register CLOCK **************************
 * Clock Settings
 */
#define VTSS_CPU_SYSTEM_CTRL_CLOCK           VTSS_REGISTER(ICPU_CFG,0x0000)

/* Use this field to configure the V-Core-II system clock. */
#define  VTSS_F_PLL_DIV_CFG(x)                VTSS_ENCODE_BITFIELD(x,0,4)
#define   VTSS_F_PLL_DIV_CFG_FPOS              0
#define   VTSS_F_PLL_DIV_CFG_FLEN              4

/****** Group CPU_SYSTEM_CTRL, Register RESET **************************
 * Reset Settings
 */
#define VTSS_CPU_SYSTEM_CTRL_RESET           VTSS_REGISTER(ICPU_CFG,0x0004)

/* Use this field to enable the DRAM Controller DLL. The DLL requires
 * approximately 50 us before it is locked (DLL_STATUS). */
#define  VTSS_F_MEMCTRL_DLL_RST_FORCE         VTSS_BIT(2)

/* Use this field to enable the DRAM Controller. The DRAM Controller must
 * not be taken out of reset before the DRAM Controller DLL is locked
 * (DLL_STATUS). */
#define  VTSS_F_MEMCTRL_RST_FORCE             VTSS_BIT(1)

/* Use this field to generate a soft reset for the entire V-Core-II
 * system. */
#define  VTSS_F_SOFT_RST                      VTSS_BIT(0)

/****** Group CPU_SYSTEM_CTRL, Register GENERAL_CTRL *******************
 * General control
 */
#define VTSS_CPU_SYSTEM_CTRL_GENERAL_CTRL    VTSS_REGISTER(ICPU_CFG,0x0008)

/* Use this field to change from Boot mode to Normal mode. In Boot mode,
 * the reset vector of the V-Core-II CPU maps to CS0 on the parallel
 * interface. When in Normal mode, this address maps instead to the DRAM
 * Controller. The DRAM Controller must be operational before disabling
 * Boot mode.
 * After setting Boot mode, this register must be read back. The change in
 * Boot mode becomes effective during reading. */
#define  VTSS_F_BOOT_MODE_ENA                 VTSS_BIT(0)

/***********************************************************************
 * Register group PI_MST
 *
 * Parallel Interface Master Configuration
 */

#define VTSS_RB_ICPU_CFG_PI_MST                VTSS_IOADDR(ICPU_CFG,0x0010)

/****** Group PI_MST, Register PI_MST_CFG ******************************
 * PI Master Configuration
 */
#define VTSS_PI_MST_CFG                      VTSS_REGISTER(ICPU_CFG,0x0010)

/* Controls the clock-polarity of the PI Master. */
#define  VTSS_F_CLK_POL                       VTSS_BIT(6)

/* Controls the clock for the PI Controller. */
#define  VTSS_F_CLK_DIV(x)                    VTSS_ENCODE_BITFIELD(x,0,5)
#define   VTSS_F_CLK_DIV_FPOS                  0
#define   VTSS_F_CLK_DIV_FLEN                  5

/****** Group PI_MST, Register PI_MST_CTRL *****************************
 * PI Master Control Register
 */
#define VTSS_PI_MST_CTRL_0                   VTSS_REGISTER(ICPU_CFG,0x0014)
#define VTSS_PI_MST_CTRL_1                   VTSS_REGISTER(ICPU_CFG,0x0018)
#define VTSS_PI_MST_CTRL_2                   VTSS_REGISTER(ICPU_CFG,0x001c)
#define VTSS_PI_MST_CTRL_3                   VTSS_REGISTER(ICPU_CFG,0x0020)

/* Data width. In 8-bit mode, the unused data-bits contain additional
 * address information. */
#define  VTSS_F_DATA_WID                      VTSS_BIT(23)

/* Device-paced transfer enable. When enabled, use PI_nDone to end a
 * transfer. */
#define  VTSS_F_DEVICE_PACED_XFER_ENA         VTSS_BIT(22)

/* Enable timeout on device-paced transfers. If enabled, a
 * device_paced_transfer transfer does not wait indefinitely for assertion
 * of PI_nDone. If a timeout occurs, the TIMEOUT_ERR_STICKY bit is set in
 * the status register and the current transfer is terminated (read-data
 * will be invalid). */
#define  VTSS_F_DEVICE_PACED_TIMEOUT_ENA      VTSS_BIT(21)

/* Determines the number of PI_Clk cycles from the start of a transfer
 * until a timeout occurs. This field is only valid when timeout for
 * device-paced transfer is enabled. */
#define  VTSS_F_DEVICE_PACED_TIMEOUT(x)       VTSS_ENCODE_BITFIELD(x,18,3)
#define   VTSS_F_DEVICE_PACED_TIMEOUT_FPOS     18
#define   VTSS_F_DEVICE_PACED_TIMEOUT_FLEN     3

/* Polarity of PI_nDone for device-paced transfers. */
#define  VTSS_F_DONE_POL                      VTSS_BIT(16)

/* Controls when data is sampled in relation to assertion of PI_nDone for
 * device-paced reads. */
#define  VTSS_F_SMPL_ON_DONE                  VTSS_BIT(15)

/* Number of wait states measured in PI_Clk cycles on both read and write
 * transfers. */
#define  VTSS_F_WAITCC(x)                     VTSS_ENCODE_BITFIELD(x,7,8)
#define   VTSS_F_WAITCC_FPOS                   7
#define   VTSS_F_WAITCC_FLEN                   8

/* Number of PI_Clk cycles from address driven to PI_nCS[x] low. */
#define  VTSS_F_CSCC(x)                       VTSS_ENCODE_BITFIELD(x,5,2)
#define   VTSS_F_CSCC_FPOS                     5
#define   VTSS_F_CSCC_FLEN                     2

/* Number of PI_Clk cycles from PI_nCS[x] low to PI_nOE low. */
#define  VTSS_F_OECC(x)                       VTSS_ENCODE_BITFIELD(x,3,2)
#define   VTSS_F_OECC_FPOS                     3
#define   VTSS_F_OECC_FLEN                     2

/* Number of PI_Clk cycles to insert at the end of a transfer. */
#define  VTSS_F_HLDCC(x)                      VTSS_ENCODE_BITFIELD(x,0,3)
#define   VTSS_F_HLDCC_FPOS                    0
#define   VTSS_F_HLDCC_FLEN                    3

/****** Group PI_MST, Register PI_MST_STATUS ***************************
 * PI Master Status Registers
 */
#define VTSS_PI_MST_STATUS_0                 VTSS_REGISTER(ICPU_CFG,0x0024)
#define VTSS_PI_MST_STATUS_1                 VTSS_REGISTER(ICPU_CFG,0x0028)
#define VTSS_PI_MST_STATUS_2                 VTSS_REGISTER(ICPU_CFG,0x002c)
#define VTSS_PI_MST_STATUS_3                 VTSS_REGISTER(ICPU_CFG,0x0030)

/* If a timeout is enabled and timeout occurs during a device-paced
 * transfer, this bit is set. */
#define  VTSS_F_TIMEOUT_ERR_STICKY            VTSS_BIT(0)

/***********************************************************************
 * Register group INTR
 *
 * Interrupt Registers
 */

#define VTSS_RB_ICPU_CFG_INTR                  VTSS_IOADDR(ICPU_CFG,0x0034)

/****** Group INTR, Register INTR_CTRL *********************************
 * Interrupt Control
 */
#define VTSS_INTR_CTRL                       VTSS_REGISTER(ICPU_CFG,0x0034)

/* Use this bit to turn on or turn off interrupt IRQ generation. */
#define  VTSS_F_GLBL_IRQ_ENA                  VTSS_BIT(31)

/* Use this bit to turn on or turn off interrupt IRQ generation. */
#define  VTSS_F_GLBL_FIQ_ENA                  VTSS_BIT(30)

/* Controls the polarity of the PI_IRQ pins. Changing the polarity causes
 * a spurious PI interrupt. To avoid interrupts, write to INTR_CLR before
 * interrupts are enabled. */
#define  VTSS_F_PI_IRQ_POL_1                  VTSS_BIT(29)
#define  VTSS_F_PI_IRQ_POL_0                  VTSS_BIT(28)

/* Controls whether interrupts on the PI_IRQ pins are level (interrupt
 * when high value is seen) or edge (low-to-high-transition) sensitive.
 * Note: The PI_IRQ pins are corrected for polarity (PI_IRQ_POL) before
 * triggering. */
#define  VTSS_F_TRIGGER_CTRL_PI_1             VTSS_BIT(9)
#define  VTSS_F_TRIGGER_CTRL_PI_0             VTSS_BIT(8)

/* Selects type of two-wire serial interface controller interrupt for
 * interrupt request. */
#define  VTSS_F_SEL_TWI                       VTSS_BIT(7)

/* Selects type of FDMA interrupt for interrupt request. */
#define  VTSS_F_SEL_FDMA                      VTSS_BIT(6)

/* Selects type of timer interrupt for interrupt request. */
#define  VTSS_F_SEL_TIMER_2                   VTSS_BIT(5)
#define  VTSS_F_SEL_TIMER_1                   VTSS_BIT(4)
#define  VTSS_F_SEL_TIMER_0                   VTSS_BIT(3)

/* Selects type of UART interrupt for interrupt request. */
#define  VTSS_F_SEL_UART                      VTSS_BIT(2)

/* Selects type of PI_IRQ interrupt for interrupt request. */
#define  VTSS_F_SEL_PI_1                      VTSS_BIT(1)
#define  VTSS_F_SEL_PI_0                      VTSS_BIT(0)

/****** Group INTR, Register INTR_STATUS *******************************
 * Interrupt Status
 */
#define VTSS_INTR_STATUS                     VTSS_REGISTER(ICPU_CFG,0x0040)
#define  VTSS_F_STATUS_TWI                    VTSS_BIT(7)
#define  VTSS_F_STATUS_FDMA                   VTSS_BIT(6)
#define  VTSS_F_STATUS_TIMER_2                VTSS_BIT(5)
#define  VTSS_F_STATUS_TIMER_1                VTSS_BIT(4)
#define  VTSS_F_STATUS_TIMER_0                VTSS_BIT(3)
#define  VTSS_F_STATUS_UART                   VTSS_BIT(2)

/* This field is corrected for polarity (IRQ_POL). */
#define  VTSS_F_STATUS_PI_1                   VTSS_BIT(1)
#define  VTSS_F_STATUS_PI_0                   VTSS_BIT(0)

/****** Group INTR, Register INTR_CLR **********************************
 * Interrupt Clear Register
 */
#define VTSS_INTR_CLR                        VTSS_REGISTER(ICPU_CFG,0x0044)
#define  VTSS_F_CLR_TWI                       VTSS_BIT(7)
#define  VTSS_F_CLR_FDMA                      VTSS_BIT(6)
#define  VTSS_F_CLR_TIMER_2                   VTSS_BIT(5)
#define  VTSS_F_CLR_TIMER_1                   VTSS_BIT(4)
#define  VTSS_F_CLR_TIMER_0                   VTSS_BIT(3)
#define  VTSS_F_CLR_UART                      VTSS_BIT(2)
#define  VTSS_F_CLR_PI_1                      VTSS_BIT(1)
#define  VTSS_F_CLR_PI_0                      VTSS_BIT(0)

/****** Group INTR, Register INTR_MASK *********************************
 * Interrupt Mask
 */
#define VTSS_INTR_MASK                       VTSS_REGISTER(ICPU_CFG,0x0048)

/* Two-wire serial interface interrupts enable mask. */
#define  VTSS_F_MASK_TWI                      VTSS_BIT(7)

/* FDMA interrupts enable mask. Interrupts from the individual FDMA
 * channels are controlled through the FDMA's group of registers. */
#define  VTSS_F_MASK_FDMA                     VTSS_BIT(6)

/* Timer interrupts enable mask - one per timer. */
#define  VTSS_F_MASK_TIMER_2                  VTSS_BIT(5)
#define  VTSS_F_MASK_TIMER_1                  VTSS_BIT(4)
#define  VTSS_F_MASK_TIMER_0                  VTSS_BIT(3)

/* UART interrupts enable mask. */
#define  VTSS_F_MASK_UART                     VTSS_BIT(2)

/* PI interrupts enable mask. */
#define  VTSS_F_MASK_PI_1                     VTSS_BIT(1)
#define  VTSS_F_MASK_PI_0                     VTSS_BIT(0)

/****** Group INTR, Register INTR_MASK_SET *****************************
 * Interrupt Mask Set
 */
#define VTSS_INTR_MASK_SET                   VTSS_REGISTER(ICPU_CFG,0x004c)
#define  VTSS_F_MASK_SET_TWI                  VTSS_BIT(7)
#define  VTSS_F_MASK_SET_FDMA                 VTSS_BIT(6)
#define  VTSS_F_MASK_SET_TIMER_2              VTSS_BIT(5)
#define  VTSS_F_MASK_SET_TIMER_1              VTSS_BIT(4)
#define  VTSS_F_MASK_SET_TIMER_0              VTSS_BIT(3)
#define  VTSS_F_MASK_SET_UART                 VTSS_BIT(2)
#define  VTSS_F_MASK_SET_PI_1                 VTSS_BIT(1)
#define  VTSS_F_MASK_SET_PI_0                 VTSS_BIT(0)

/****** Group INTR, Register INTR_MASK_CLR *****************************
 * Interrupt Mask Clear
 */
#define VTSS_INTR_MASK_CLR                   VTSS_REGISTER(ICPU_CFG,0x0050)
#define  VTSS_F_MASK_CLR_TWI                  VTSS_BIT(7)
#define  VTSS_F_MASK_CLR_FDMA                 VTSS_BIT(6)
#define  VTSS_F_MASK_CLR_TIMER_2              VTSS_BIT(5)
#define  VTSS_F_MASK_CLR_TIMER_1              VTSS_BIT(4)
#define  VTSS_F_MASK_CLR_TIMER_0              VTSS_BIT(3)
#define  VTSS_F_MASK_CLR_UART                 VTSS_BIT(2)
#define  VTSS_F_MASK_CLR_PI_1                 VTSS_BIT(1)
#define  VTSS_F_MASK_CLR_PI_0                 VTSS_BIT(0)

/****** Group INTR, Register INTR_IRQ_IDENT ****************************
 * Interrupt IRQ Identification
 */
#define VTSS_INTR_IRQ_IDENT                  VTSS_REGISTER(ICPU_CFG,0x0054)
#define  VTSS_F_IRQ_IDENT_TWI                 VTSS_BIT(7)
#define  VTSS_F_IRQ_IDENT_FDMA                VTSS_BIT(6)
#define  VTSS_F_IRQ_IDENT_TIMER_2             VTSS_BIT(5)
#define  VTSS_F_IRQ_IDENT_TIMER_1             VTSS_BIT(4)
#define  VTSS_F_IRQ_IDENT_TIMER_0             VTSS_BIT(3)
#define  VTSS_F_IRQ_IDENT_UART                VTSS_BIT(2)
#define  VTSS_F_IRQ_IDENT_PI_1                VTSS_BIT(1)
#define  VTSS_F_IRQ_IDENT_PI_0                VTSS_BIT(0)

/****** Group INTR, Register INTR_FIQ_IDENT ****************************
 * Interrupt FIQ Identification
 */
#define VTSS_INTR_FIQ_IDENT                  VTSS_REGISTER(ICPU_CFG,0x0058)
#define  VTSS_F_FIQ_IDENT_TWI                 VTSS_BIT(7)
#define  VTSS_F_FIQ_IDENT_FDMA                VTSS_BIT(6)
#define  VTSS_F_FIQ_IDENT_TIMER_2             VTSS_BIT(5)
#define  VTSS_F_FIQ_IDENT_TIMER_1             VTSS_BIT(4)
#define  VTSS_F_FIQ_IDENT_TIMER_0             VTSS_BIT(3)
#define  VTSS_F_FIQ_IDENT_UART                VTSS_BIT(2)
#define  VTSS_F_FIQ_IDENT_PI_1                VTSS_BIT(1)
#define  VTSS_F_FIQ_IDENT_PI_0                VTSS_BIT(0)

/***********************************************************************
 * Register group GPDMA
 *
 * General Purpose DMA
 */

#define VTSS_RB_ICPU_CFG_GPDMA                 VTSS_IOADDR(ICPU_CFG,0x005c)

/****** Group GPDMA, Register FDMA_COMMON_CFG **************************
 * Configure the Device select to be used for remote access
 */
#define VTSS_GPDMA_FDMA_COMMON_CFG           VTSS_REGISTER(ICPU_CFG,0x005c)

/* Chip select for remote PI injection / extraction */
#define  VTSS_F_PI_ADDR(x)                    VTSS_ENCODE_BITFIELD(x,1,2)
#define   VTSS_F_PI_ADDR_FPOS                  1
#define   VTSS_F_PI_ADDR_FLEN                  2

/****** Group GPDMA, Register FDMA_CH_CFG_0 ****************************
 * Configures data addresses for injection / extraction
 */
#define VTSS_GPDMA_FDMA_CH_CFG_0_0           VTSS_REGISTER(ICPU_CFG,0x0060)
#define VTSS_GPDMA_FDMA_CH_CFG_0_1           VTSS_REGISTER(ICPU_CFG,0x0064)
#define VTSS_GPDMA_FDMA_CH_CFG_0_2           VTSS_REGISTER(ICPU_CFG,0x0068)
#define VTSS_GPDMA_FDMA_CH_CFG_0_3           VTSS_REGISTER(ICPU_CFG,0x006c)
#define VTSS_GPDMA_FDMA_CH_CFG_0_4           VTSS_REGISTER(ICPU_CFG,0x0070)
#define VTSS_GPDMA_FDMA_CH_CFG_0_5           VTSS_REGISTER(ICPU_CFG,0x0074)
#define VTSS_GPDMA_FDMA_CH_CFG_0_6           VTSS_REGISTER(ICPU_CFG,0x0078)
#define VTSS_GPDMA_FDMA_CH_CFG_0_7           VTSS_REGISTER(ICPU_CFG,0x007c)

/* Specifies the Module for injection / extraction. This is used together
 * with SUB_MODULE to specify which Port Module to access. */
#define  VTSS_F_MODULE(x)                     VTSS_ENCODE_BITFIELD(x,12,3)
#define   VTSS_F_MODULE_FPOS                   12
#define   VTSS_F_MODULE_FLEN                   3

/* Specifies the Submodule for injection / extraction. This is used
 * together with MODULE to specify which Port Module to access. */
#define  VTSS_F_SUB_MODULE(x)                 VTSS_ENCODE_BITFIELD(x,8,4)
#define   VTSS_F_SUB_MODULE_FPOS               8
#define   VTSS_F_SUB_MODULE_FLEN               4

/* Specifies the data address for injection / extraction of frame data. */
#define  VTSS_F_DATA_ADDR(x)                  VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_DATA_ADDR_FPOS                0
#define   VTSS_F_DATA_ADDR_FLEN                8

/****** Group GPDMA, Register FDMA_CH_CFG_1 ****************************
 * 
 */
#define VTSS_GPDMA_FDMA_CH_CFG_1_0           VTSS_REGISTER(ICPU_CFG,0x0080)
#define VTSS_GPDMA_FDMA_CH_CFG_1_1           VTSS_REGISTER(ICPU_CFG,0x0084)
#define VTSS_GPDMA_FDMA_CH_CFG_1_2           VTSS_REGISTER(ICPU_CFG,0x0088)
#define VTSS_GPDMA_FDMA_CH_CFG_1_3           VTSS_REGISTER(ICPU_CFG,0x008c)
#define VTSS_GPDMA_FDMA_CH_CFG_1_4           VTSS_REGISTER(ICPU_CFG,0x0090)
#define VTSS_GPDMA_FDMA_CH_CFG_1_5           VTSS_REGISTER(ICPU_CFG,0x0094)
#define VTSS_GPDMA_FDMA_CH_CFG_1_6           VTSS_REGISTER(ICPU_CFG,0x0098)
#define VTSS_GPDMA_FDMA_CH_CFG_1_7           VTSS_REGISTER(ICPU_CFG,0x009c)

/* Controls the usage of the channel. The channel can be configured for
 * either frame extraction (XTR) or frame injection (INJ) */
#define  VTSS_F_USAGE                         VTSS_BIT(7)

/* Specifies a mask of IP_IRQ to enable extraction/injection by this
 * channel. */
#define  VTSS_F_FC_PI_IRQ_ENA(x)              VTSS_ENCODE_BITFIELD(x,5,2)
#define   VTSS_F_FC_PI_IRQ_ENA_FPOS            5
#define   VTSS_F_FC_PI_IRQ_ENA_FLEN            2

/* Specifies flowcontrol for injection of frames when below watermark. */
#define  VTSS_F_FC_INJ_ENA                    VTSS_BIT(4)

/* Specifies a flowcontrol mask on frame present in the corresponding
 * extraction queue. */
#define  VTSS_F_FC_XTR_ENA(x)                 VTSS_ENCODE_BITFIELD(x,0,4)
#define   VTSS_F_FC_XTR_ENA_FPOS               0
#define   VTSS_F_FC_XTR_ENA_FLEN               4

/****** Group GPDMA, Register FDMA_INJ_CFG *****************************
 * Injection Parameters
 */
#define VTSS_GPDMA_FDMA_INJ_CFG_0            VTSS_REGISTER(ICPU_CFG,0x00a0)
#define VTSS_GPDMA_FDMA_INJ_CFG_1            VTSS_REGISTER(ICPU_CFG,0x00a4)
#define VTSS_GPDMA_FDMA_INJ_CFG_2            VTSS_REGISTER(ICPU_CFG,0x00a8)
#define VTSS_GPDMA_FDMA_INJ_CFG_3            VTSS_REGISTER(ICPU_CFG,0x00ac)
#define VTSS_GPDMA_FDMA_INJ_CFG_4            VTSS_REGISTER(ICPU_CFG,0x00b0)
#define VTSS_GPDMA_FDMA_INJ_CFG_5            VTSS_REGISTER(ICPU_CFG,0x00b4)
#define VTSS_GPDMA_FDMA_INJ_CFG_6            VTSS_REGISTER(ICPU_CFG,0x00b8)
#define VTSS_GPDMA_FDMA_INJ_CFG_7            VTSS_REGISTER(ICPU_CFG,0x00bc)

/* Enable / Disable injection of header */
#define  VTSS_F_INJ_HDR_ENA                   VTSS_BIT(0)

/****** Group GPDMA, Register FDMA_XTR_CFG *****************************
 * Extraction parameters as well as channel reset
 */
#define VTSS_GPDMA_FDMA_XTR_CFG_0            VTSS_REGISTER(ICPU_CFG,0x00c0)
#define VTSS_GPDMA_FDMA_XTR_CFG_1            VTSS_REGISTER(ICPU_CFG,0x00c4)
#define VTSS_GPDMA_FDMA_XTR_CFG_2            VTSS_REGISTER(ICPU_CFG,0x00c8)
#define VTSS_GPDMA_FDMA_XTR_CFG_3            VTSS_REGISTER(ICPU_CFG,0x00cc)
#define VTSS_GPDMA_FDMA_XTR_CFG_4            VTSS_REGISTER(ICPU_CFG,0x00d0)
#define VTSS_GPDMA_FDMA_XTR_CFG_5            VTSS_REGISTER(ICPU_CFG,0x00d4)
#define VTSS_GPDMA_FDMA_XTR_CFG_6            VTSS_REGISTER(ICPU_CFG,0x00d8)
#define VTSS_GPDMA_FDMA_XTR_CFG_7            VTSS_REGISTER(ICPU_CFG,0x00dc)

/* One shot bit to initialize channel (bot injection and extraction). */
#define  VTSS_F_XTR_INIT_SHOT                 VTSS_BIT(24)

/* Specifies whether burst mode is used or not. (If used it is assumed
 * that the same burst length is used throughout a frame and that the burst
 * length is less than 32 bytes). */
#define  VTSS_F_XTR_USE_BURST_ENA             VTSS_BIT(23)

/* Specifies the size - in 32-bit words - of the gap to insert at the end
 * of a frame, to make room for a possible new tail, in case the extracted
 * frame needs to be re-injected. */
#define  VTSS_F_XTR_END_GAP(x)                VTSS_ENCODE_BITFIELD(x,8,6)
#define   VTSS_F_XTR_END_GAP_FPOS              8
#define   VTSS_F_XTR_END_GAP_FLEN              6

/* Specifies the size - in 32-bit words - of the gap to insert at the
 * beginning of a frame after IFH, to make room for a possible new header,
 * in case the extracted frame needs to be re-injected. */
#define  VTSS_F_XTR_START_GAP(x)              VTSS_ENCODE_BITFIELD(x,0,6)
#define   VTSS_F_XTR_START_GAP_FPOS            0
#define   VTSS_F_XTR_START_GAP_FLEN            6

/****** Group GPDMA, Register XTR_LAST_CHUNK_STAT **********************
 * Extraction status to be used by FDMA engine.
 */
#define VTSS_GPDMA_XTR_LAST_CHUNK_STAT_0     VTSS_REGISTER(ICPU_CFG,0x00e0)
#define VTSS_GPDMA_XTR_LAST_CHUNK_STAT_1     VTSS_REGISTER(ICPU_CFG,0x00e4)
#define VTSS_GPDMA_XTR_LAST_CHUNK_STAT_2     VTSS_REGISTER(ICPU_CFG,0x00e8)
#define VTSS_GPDMA_XTR_LAST_CHUNK_STAT_3     VTSS_REGISTER(ICPU_CFG,0x00ec)
#define VTSS_GPDMA_XTR_LAST_CHUNK_STAT_4     VTSS_REGISTER(ICPU_CFG,0x00f0)
#define VTSS_GPDMA_XTR_LAST_CHUNK_STAT_5     VTSS_REGISTER(ICPU_CFG,0x00f4)
#define VTSS_GPDMA_XTR_LAST_CHUNK_STAT_6     VTSS_REGISTER(ICPU_CFG,0x00f8)
#define VTSS_GPDMA_XTR_LAST_CHUNK_STAT_7     VTSS_REGISTER(ICPU_CFG,0x00fc)

/* CPU queue number (will be 0 if AUTOQ_SEL is enabled in SWC). */
#define  VTSS_F_XTR_CPU_QU(x)                 VTSS_ENCODE_BITFIELD(x,4,2)
#define   VTSS_F_XTR_CPU_QU_FPOS               4
#define   VTSS_F_XTR_CPU_QU_FLEN               2

/* End of frame. */
#define  VTSS_F_XTR_EOF                       VTSS_BIT(1)

/* Start of frame. */
#define  VTSS_F_XTR_SOF                       VTSS_BIT(0)

/****** Group GPDMA, Register FDMA_FRM_CNT *****************************
 * Frame Counter and flow control status
 */
#define VTSS_GPDMA_FDMA_FRM_CNT              VTSS_REGISTER(ICPU_CFG,0x0100)

/* CPU injection queue below watermark status */
#define  VTSS_F_INJ_BELOW_WM_STATUS           VTSS_BIT(24)

/* PI interupt status */
#define  VTSS_F_PI_IRQ_STATUS(x)              VTSS_ENCODE_BITFIELD(x,20,2)
#define   VTSS_F_PI_IRQ_STATUS_FPOS            20
#define   VTSS_F_PI_IRQ_STATUS_FLEN            2

/* CPU extraction queue frame available (one for each CPU queue) */
#define  VTSS_F_XTR_QU_STATUS(x)              VTSS_ENCODE_BITFIELD(x,16,4)
#define   VTSS_F_XTR_QU_STATUS_FPOS            16
#define   VTSS_F_XTR_QU_STATUS_FLEN            4

/* XTR/INJ: This is a free running, wrapping counter, counting the total
 * number of frames extracted or injected.
 */
#define  VTSS_F_FRM_CNT(x)                    VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_FRM_CNT_FPOS                  0
#define   VTSS_F_FRM_CNT_FLEN                  16

/****** Group GPDMA, Register XTR_STICKY *******************************
 * Sticky bits register for extraction
 */
#define VTSS_GPDMA_XTR_STICKY                VTSS_REGISTER(ICPU_CFG,0x0104)

/* The DMA did not use 32 bits word towards switchcore. */
#define  VTSS_F_DMA_SIZE_ERR_STICKY           VTSS_BIT(8)

/* Timeout error towards either PI or Switchcore */
#define  VTSS_F_SWC_PI_HS_ERR_STICKY          VTSS_BIT(7)

/* Frame extraction attemted without frame available (probably due to
 * change of configuration or multiple channels setup for same queue) */
#define  VTSS_F_XTR_REQ_WITHOUT_AVAIL_ERR_STICKY  VTSS_BIT(6)

/* Set if frame length less than 40  */
#define  VTSS_F_XTR_SHORT_LEN_ERR_STICKY      VTSS_BIT(5)

/* Set if start gap was inserted */
#define  VTSS_F_XTR_START_GAP_STICKY          VTSS_BIT(4)

/* Set if end gap was inserted */
#define  VTSS_F_XTR_END_GAP_STICKY            VTSS_BIT(3)

/* Set if a new DCB is needed for holding a frame */
#define  VTSS_F_XTR_FRAME_SPAN_MULTIPLE_CHUNK_STICKY  VTSS_BIT(2)

/* Set if a complete frame has been extracted */
#define  VTSS_F_XTR_FRAME_DONE_STICKY         VTSS_BIT(1)

/* Set if frame extraction has started */
#define  VTSS_F_XTR_FRAME_STARTED_STICKY      VTSS_BIT(0)

/****** Group GPDMA, Register INJ_STICKY *******************************
 * Sticky bits register for injection
 */
#define VTSS_GPDMA_INJ_STICKY                VTSS_REGISTER(ICPU_CFG,0x0108)

/* Two injection channels are trying to simultanously use statemachine.
 * Channel locking is required if multiple injection channels are enabled
 * simultanously. */
#define  VTSS_F_INJ_CH_CONFLICT_ERR_STICKY    VTSS_BIT(5)

/* The first chunk did not contain a SOF flag.  */
#define  VTSS_F_INJ_NO_SOF_ERR_STICKY         VTSS_BIT(4)

/* Set when frame injection has completed */
#define  VTSS_F_INJ_FRAME_COMPLETE_STICKY     VTSS_BIT(3)

/* Set when frame data CMD field has been injected */
#define  VTSS_F_INJ_FRAME_CMD_STICKY          VTSS_BIT(2)

/* Set when frame HDR has been injected */
#define  VTSS_F_INJ_FRAME_HDR_STICKY          VTSS_BIT(1)

/* Set when frame injection has started */
#define  VTSS_F_INJ_FRAME_STARTED_STICKY      VTSS_BIT(0)

/***********************************************************************
 * Register group TIMERS
 *
 * Timer Registers
 */

#define VTSS_RB_ICPU_CFG_TIMERS                VTSS_IOADDR(ICPU_CFG,0x010c)

/****** Group TIMERS, Register WDT *************************************
 * Watchdog Timer
 */
#define VTSS_TIMERS_WDT                      VTSS_REGISTER(ICPU_CFG,0x010c)

/* Shows whether the last reset was caused by a watchdog timer reset. This
 * field is updated during reset, therefore it is always valid. */
#define  VTSS_F_WDT_STATUS                    VTSS_BIT(9)

/* Use this field to enable or disable the watchdog timer. When the WDT is
 * enabled, it causes a reset after 2 seconds if it is not periodically
 * reset. This field is only read by the WDT after a sucessful lock
 * sequence (WDT_LOCK). */
#define  VTSS_F_WDT_ENABLE                    VTSS_BIT(8)

/* Use this field to configure and reset the WDT. When writing 0xBE to
 * this field immediately followed by writing 0xEF, the WDT resets and
 * configurations are read from this register (as set when the 0xEF is
 * written). Once 0xBE is written, then writing any value other than 0xBE
 * or 0xEF causes a WDT reset as if the timer had run out. */
#define  VTSS_F_WDT_LOCK(x)                   VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_WDT_LOCK_FPOS                 0
#define   VTSS_F_WDT_LOCK_FLEN                 8

/****** Group TIMERS, Register TIMER_TICK_DIV **************************
 * Timer Tick Divider
 */
#define VTSS_TIMERS_TIMER_TICK_DIV           VTSS_REGISTER(ICPU_CFG,0x0110)

/* The timer tick generator runs off the Switch core frequency. By
 * default, the divider value generates a timer tick every 100 us (10 KHz).
 * The timer tick is used for all of the timers (except the WDT). This
 * field must not be set to generate a timer tick of less than 0.1 us (or
 * higher than 10 MHz). If this field is changed, it takes up to 10 ms
 * before the timers are running stable at the new frequency.
 */
#define  VTSS_F_TIMER_TICK_DIV(x)             VTSS_ENCODE_BITFIELD(x,0,17)
#define   VTSS_F_TIMER_TICK_DIV_FPOS           0
#define   VTSS_F_TIMER_TICK_DIV_FLEN           17

/****** Group TIMERS, Register TIMER_VALUE *****************************
 * Timer value
 */
#define VTSS_TIMERS_TIMER_VALUE_0            VTSS_REGISTER(ICPU_CFG,0x0114)
#define VTSS_TIMERS_TIMER_VALUE_1            VTSS_REGISTER(ICPU_CFG,0x0118)
#define VTSS_TIMERS_TIMER_VALUE_2            VTSS_REGISTER(ICPU_CFG,0x011c)

/* The current value of the timer. When the timer is at its maximum value
 * and increments from that, an interrupt is generated and the reload value
 * is loaded into the timer.  */
#define  VTSS_F_TIMER_VAL(x)                  VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_TIMER_VAL_FPOS                0
#define   VTSS_F_TIMER_VAL_FLEN                16

/****** Group TIMERS, Register TIMER_RELOAD_VALUE **********************
 * Timer Reload Value
 */
#define VTSS_TIMERS_TIMER_RELOAD_VALUE_0     VTSS_REGISTER(ICPU_CFG,0x0120)
#define VTSS_TIMERS_TIMER_RELOAD_VALUE_1     VTSS_REGISTER(ICPU_CFG,0x0124)
#define VTSS_TIMERS_TIMER_RELOAD_VALUE_2     VTSS_REGISTER(ICPU_CFG,0x0128)

/* The contents of this field are loaded into the timer when it wraps.
 * When a specific period is needed from the timer, translate that period
 * into a number of ticks. Then configure the reload value for the timer to
 * (0xFFFF - ticks +1). */
#define  VTSS_F_RELOAD_VAL(x)                 VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_RELOAD_VAL_FPOS               0
#define   VTSS_F_RELOAD_VAL_FLEN               16

/****** Group TIMERS, Register TIMER_CTRL ******************************
 * Timer Control
 */
#define VTSS_TIMERS_TIMER_CTRL_0             VTSS_REGISTER(ICPU_CFG,0x012c)
#define VTSS_TIMERS_TIMER_CTRL_1             VTSS_REGISTER(ICPU_CFG,0x0130)
#define VTSS_TIMERS_TIMER_CTRL_2             VTSS_REGISTER(ICPU_CFG,0x0134)

/* When enabled, the timer increments at each timer-tick. */
#define  VTSS_F_TIMER_ENA                     VTSS_BIT(1)

/* Set this field to force the reload value (TIMER_RELOAD_VALUE) into the
 * timer. */
#define  VTSS_F_FORCE_RELOAD                  VTSS_BIT(0)

/***********************************************************************
 * Register group MEMCTRL_DDR
 *
 * DDR Memory Controller Registers
 */

#define VTSS_RB_ICPU_CFG_MEMCTRL_DDR           VTSS_IOADDR(ICPU_CFG,0x0138)

/****** Group MEMCTRL_DDR, Register DLL_STATUS *************************
 * Master DLL status
 */
#define VTSS_MEMCTRL_DDR_DLL_STATUS          VTSS_REGISTER(ICPU_CFG,0x013c)

/* Status for the DLL. */
#define  VTSS_F_DLL_LOCK                      VTSS_BIT(11)

/* This field is set when the DLL is out of lock. */
#define  VTSS_F_DLL_LOSS_OF_LOCK              VTSS_BIT(10)

/****** Group MEMCTRL_DDR, Register DDR_PAD_DRIVE **********************
 * Used to control DDR pad drivers
 */
#define VTSS_MEMCTRL_DDR_DDR_PAD_DRIVE       VTSS_REGISTER(ICPU_CFG,0x0148)

/* This register sets the drive strength of DDR pad drivers. Typical
 * values are 101101 for nominal strength, 100100 for reduced strength, and
 * 111111 for max driver strength. */
#define  VTSS_F_DDR_PAD_DRIVE_CTRL(x)         VTSS_ENCODE_BITFIELD(x,8,6)
#define   VTSS_F_DDR_PAD_DRIVE_CTRL_FPOS       8
#define   VTSS_F_DDR_PAD_DRIVE_CTRL_FLEN       6

/* This register holds the fixed-point unsigned result of a measurement of
 * the relative driver strength of pull-up devices compared to pull-down
 * devices. The result may vary between product samples due to process
 * variations. A small value is an indication of a relatively weak pull-up
 * device. */
#define  VTSS_F_DDR_PAD_DRIVE_STAT(x)         VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_DDR_PAD_DRIVE_STAT_FPOS       0
#define   VTSS_F_DDR_PAD_DRIVE_STAT_FLEN       8

/*********************************************************************** 
 *
 * Target UART
 *
 * 
 *
 ***********************************************************************/

/***********************************************************************
 * Register group UART
 *
 * UART registers
 */

#define VTSS_RB_UART_UART                      VTSS_IOADDR(UART,0x0000)

/****** Group UART, Register RBR_THR ***********************************
 * Receive Buffer / Transmit Holding Register / Divisor (Low)
 */
#define VTSS_UART_RBR_THR                    VTSS_REGISTER(UART,0x0000)

/* Use this register to access the Rx and Tx FIFOs.
 * When reading: The data in this register is valid only if LSR.DR is set.
 * If FIFOs are disabled (IIR_FCR.FIFOE), the data in this register must be
 * read before the next data arrives, otherwise it is overwritten,
 * resulting in an overrun error. When FIFOs are enabled (IIR_FCR.FIFOE),
 * this register accesses the head of the receive FIFO. If the receive FIFO
 * is full and this register is not read before the next data character
 * arrives, then the data already in the FIFO is preserved, but any
 * incoming data is lost and an overrun error occurs.
 * When writing: Data should only be written to this register when the
 * LSR.THRE indicates that there is room in the FIFO. If FIFOs are disabled
 * (IIR_FCR.FIFOE), writes to this register while LSR.THRE is zero, causes
 * the register to be overwritten. When FIFOs are enabled (IIR_FCR.FIFOE)
 * and LSR.THRE is set, 16 characters may be written to this register
 * before the FIFO is full. Any attempt to write data when the FIFO is full
 * results in the write data being lost. */
#define  VTSS_F_UART_RBR_THR(x)               VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_UART_RBR_THR_FPOS             0
#define   VTSS_F_UART_RBR_THR_FLEN             8

/****** Group UART, Register IER ***************************************
 * Interrupt Enable Register / Divisor (High)
 */
#define VTSS_UART_IER                        VTSS_REGISTER(UART,0x0004)

/* Programmable THRE interrupt mode enable. This is used to enable or
 * disable the generation of THRE interrupt. */
#define  VTSS_F_UART_IER_PTIME                VTSS_BIT(7)

/* Enable modem status interrupt. This is used to enable or disable the
 * generation of Modem Status interrupt. This is the fourth highest
 * priority interrupt. */
#define  VTSS_F_UART_IER_EDSSI                VTSS_BIT(3)

/* Enable receiver line status interrupt. This is used to enable or
 * disable the generation of Receiver Line Status interrupt. This is the
 * highest priority interrupt. */
#define  VTSS_F_UART_IER_ELSI                 VTSS_BIT(2)

/* Enable transmit holding register empty interrupt. This is used to
 * enable or disable the generation of Transmitter Holding Register Empty
 * interrupt. This is the third highest priority interrupt. */
#define  VTSS_F_UART_IER_ETBEI                VTSS_BIT(1)

/* Enable received data available interrupt. This is used to enable or
 * disable the generation of Received Data Available interrupt and the
 * Character Timeout interrupt (if FIFOs are enabled). These are the second
 * highest priority interrupts. */
#define  VTSS_F_UART_IER_ERBFI                VTSS_BIT(0)

/****** Group UART, Register IIR_FCR ***********************************
 * Interrupt Identification Register / FIFO Control Register
 */
#define VTSS_UART_IIR_FCR                    VTSS_REGISTER(UART,0x0008)

/* When reading this field, the current status of the FIFO is returned; 00
 * for disabled or 11 for enabled. Writing this field selects the trigger
 * level in the receive FIFO at which the Received Data Available interrupt
 * is generated (see encoding.) In auto flow control mode, it is used to
 * determine when to generate back-pressure using the RTS signal. */
#define  VTSS_F_UART_IIR_FCR_FIFOSE_RT(x)     VTSS_ENCODE_BITFIELD(x,6,2)
#define   VTSS_F_UART_IIR_FCR_FIFOSE_RT_FPOS   6
#define   VTSS_F_UART_IIR_FCR_FIFOSE_RT_FLEN   2

/* Tx empty trigger. When the THRE mode is enabled (IER.PTIME), this field
 * selects the empty threshold level at which the THRE Interrupts are
 * generated. */
#define  VTSS_F_UART_IIR_FCR_TET(x)           VTSS_ENCODE_BITFIELD(x,4,2)
#define   VTSS_F_UART_IIR_FCR_TET_FPOS         4
#define   VTSS_F_UART_IIR_FCR_TET_FLEN         2

/* This description is valid for writes only. Reading this field has
 * special meaning; for more information, see the general register
 * description.
 * Tx FIFO Reset. This resets the control portion of the transmit FIFO and
 * treats the FIFO as empty. Note that this bit is self-clearing. It is not
 * necessary to clear this bit. */
#define  VTSS_F_UART_IIR_FCR_XFIFOR           VTSS_BIT(2)

/* This description is valid for writes only. Reading this field has
 * special meaning; for more information, see the general register
 * description.
 * Rx FIFO Reset. This resets the control portion of the receive FIFO and
 * treats the FIFO as empty. Note that this bit is self-clearing. It is not
 * necessary to clear this bit. */
#define  VTSS_F_UART_IIR_FCR_RFIFOR           VTSS_BIT(1)

/* This description is valid for writes only. Reading this field has
 * special meaning; for more information, see the general register
 * description.
 * FIFO Enable. This enables or disables the transmit (XMIT) and receive
 * (RCVR) FIFOs. Whenever the value of this bit is changed, both the XMIT
 * and RCVR controller portion of FIFOs are reset. */
#define  VTSS_F_UART_IIR_FCR_FIFOE            VTSS_BIT(0)

/****** Group UART, Register LCR ***************************************
 * Line Control Register
 */
#define VTSS_UART_LCR                        VTSS_REGISTER(UART,0x000c)

/* Divisor latch access bit. This bit is used to enable reading and
 * writing of the Divisor registers (RBR_THR and IER) to set the baud rate
 * of the UART. To access other registers, this bit must be cleared after
 * initial baud rate setup. */
#define  VTSS_F_UART_LCR_DLAB                 VTSS_BIT(7)

/* Break control bit.This bit is used to cause a break condition to be
 * transmitted to the receiving device. If set to one, the serial output is
 * forced to the spacing (logic 0) state. When not in Loopback Mode, as
 * determined by MCR[4], the sout line is forced low until the Break bit is
 * cleared. */
#define  VTSS_F_UART_LCR_BC                   VTSS_BIT(6)

/* Even parity aelect. This bit is used to select between even and odd
 * parity, when parity is enabled (PEN set to one). If set to one, an even
 * number of logic 1s is transmitted or checked. If set to zero, an odd
 * number of logic 1s is transmitted or checked. */
#define  VTSS_F_UART_LCR_EPS                  VTSS_BIT(4)

/* Parity enable. This bit is used to enable or disable parity generation
 * and detection in both transmitted and received serial characters. */
#define  VTSS_F_UART_LCR_PEN                  VTSS_BIT(3)

/* Number of stop bits. This is used to select the number of stop bits per
 * character that the peripheral transmits and receives. If set to zero,
 * one stop bit is transmitted in the serial data.
 * If set to one and the data bits are set to 5 (LCR.DLS), one and a half
 * stop bits are transmitted. Otherwise, two stop bits are transmitted.
 * Note that regardless of the number of stop bits selected, the receiver
 * checks only the first stop bit. */
#define  VTSS_F_UART_LCR_STOP                 VTSS_BIT(2)

/* Data length select. This is used to select the number of data bits per
 * character that the peripheral transmits and receives. The following
 * settings specify the number of bits that may be selected. */
#define  VTSS_F_UART_LCR_DLS(x)               VTSS_ENCODE_BITFIELD(x,0,2)
#define   VTSS_F_UART_LCR_DLS_FPOS             0
#define   VTSS_F_UART_LCR_DLS_FLEN             2

/****** Group UART, Register MCR ***************************************
 * Modem Control Register
 */
#define VTSS_UART_MCR                        VTSS_REGISTER(UART,0x0010)

/* Auto flow control enable. This mode requires that FIFOs are enabled and
 * that MCR.RTS is set. */
#define  VTSS_F_UART_MCR_AFCE                 VTSS_BIT(5)

/* Loopback Bit. This is used to put the UART into a diagnostic mode for
 * test purposes.
 * The transmit line is held high, while serial transmit data is looped
 * back to the receive line internally. In this mode, all the interrupts
 * are fully functional. In addition, in loopback mode, the modem control
 * input CTS is disconnected, and the modem control output RTS is looped
 * back to the input internally. */
#define  VTSS_F_UART_MCR_LB                   VTSS_BIT(4)

/* Request to send. This is used to directly control the Request to Send
 * (RTS) output. The RTS output is used to inform the partner that the UART
 * is ready to exchange data.
 * The RTS is still controlled from this field when Auto RTS Flow Control
 * is enabled (MCR.AFCE), but the output can be forced high by the flow
 * control mechanism. If this field is cleared, the UART permanently
 * indicates backpressure to the partner. */
#define  VTSS_F_UART_MCR_RTS                  VTSS_BIT(1)

/****** Group UART, Register LSR ***************************************
 * Line Status Register
 */
#define VTSS_UART_LSR                        VTSS_REGISTER(UART,0x0014)

/* Receiver FIFO error bit. This bit is only valid when FIFOs are enabled.
 * This is used to indicate whether there is at least one parity error,
 * framing error, or break indication in the FIFO.
 * This bit is cleared when the LSR is read, the character with the error
 * is at the top of the receiver FIFO, and there are no subsequent errors
 * in the FIFO. */
#define  VTSS_F_UART_LSR_RFE                  VTSS_BIT(7)

/* Transmitter empty bit. If FIFOs are enabled, this bit is set whenever
 * the Transmitter Shift Register and the FIFO are both empty. */
#define  VTSS_F_UART_LSR_TEMT                 VTSS_BIT(6)

/* If FIFO (IIR_FCR.FIFOE) and THRE mode are enabled (IER.PTIME), this bit
 * indicates that the Tx FIFO is full. Otherwise, this bit indicates that
 * the Tx FIFO is empty. */
#define  VTSS_F_UART_LSR_THRE                 VTSS_BIT(5)

/* Break interrupt bit. This is used to indicate the detection of a break
 * sequence on the serial input data.
 * It is set whenever the serial input is held in a logic 0 state for
 * longer than the sum of start time + data bits + parity + stop bits.
 * A break condition on serial input causes one and only one character,
 * consisting of all-zeros, to be received by the UART.
 * In the FIFO mode, the character associated with the break condition is
 * carried through the FIFO and is revealed when the character is at the
 * top of the FIFO. Reading the LSR clears the BI bit. In the non-FIFO
 * mode, the BI indication occurs immediately and persists until the LSR is
 * read. */
#define  VTSS_F_UART_LSR_BI                   VTSS_BIT(4)

/* Framing error bit. This is used to indicate the a framing error in the
 * receiver. A framing error occurs when the receiver does not detect a
 * valid STOP bit in the received data.
 * A framing error is associated with a received character. Therefore, in
 * FIFO mode, an error is revealed when the character with the framing
 * error is at the top of the FIFO. When a framing error occurs, the UART
 * tries to resynchronize. It does this by assuming that the error was due
 * to the start bit of the next character and then continues to receive the
 * other bit, that is, data and/or parity, and then stops. Note that this
 * field is set if a break interrupt has occurred, as indicated by Break
 * Interrupt (LSR.BI).
 * This field is cleared on read. */
#define  VTSS_F_UART_LSR_FE                   VTSS_BIT(3)

/* Parity error bit. This is used to indicate the occurrence of a parity
 * error in the receiver if the Parity Enable bit (LCR.PEN) is set.
 * A parity error is associated with a received character. Therefore, in
 * FIFO mode, an error is revealed when the character with the parity error
 * arrives at the top of the FIFO. Note that this field is set if a break
 * interrupt has occurred, as indicated by Break Interrupt (LSR.BI).
 * This field is cleared on read. */
#define  VTSS_F_UART_LSR_PE                   VTSS_BIT(2)

/* Overrun error bit. This is used to indicate the occurrence of an
 * overrun error. This occurs if a new data character was received before
 * the previous data was read.
 * In non-FIFO mode, the OE bit is set when a new character arrives before
 * the previous character was read. When this happens, the data in the RBR
 * is overwritten. 
 * In FIFO mode, an overrun error occurs when the FIFO is full and a new
 * character arrives at the receiver. The data in the FIFO is retained and
 * the data in the receive shift register is lost.
 * This field is cleared on read. */
#define  VTSS_F_UART_LSR_OE                   VTSS_BIT(1)

/* Data ready. This is used to indicate that the receiver contains at
 * least one character in the receiver FIFO. This bit is cleared when the
 * RX FIFO is empty. */
#define  VTSS_F_UART_LSR_DR                   VTSS_BIT(0)

/****** Group UART, Register MSR ***************************************
 * Modem Status Register
 */
#define VTSS_UART_MSR                        VTSS_REGISTER(UART,0x0018)

/* Clear to send. This field indicates the current state of the modem
 * control line, CTS. When the Clear to Send input (CTS) is asserted, it is
 * an indication that the partner is ready to exchange data with the UART. */
#define  VTSS_F_UART_MSR_CTS                  VTSS_BIT(4)

/* Delta clear to send. This is used to indicate that the modem control
 * line, CTS, has changed since the last time the MSR was read. Reading the
 * MSR clears the DCTS bit.
 * Note: If the DCTS bit is not set, the CTS signal is asserted, and a
 * reset occurs (software or otherwise), then the DCTS bit is set when the
 * reset is removed, if the CTS signal remains asserted. A read of the MSR
 * after reset can be performed to prevent unwanted interrupts. */
#define  VTSS_F_UART_MSR_DCTS                 VTSS_BIT(0)

/****** Group UART, Register SCR ***************************************
 * Scratchpad Register
 */
#define VTSS_UART_SCR                        VTSS_REGISTER(UART,0x001c)

/* This register is for programmers to use as a temporary storage space.
 * It has no functional purpose for the UART. */
#define  VTSS_F_UART_SCR(x)                   VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_UART_SCR_FPOS                 0
#define   VTSS_F_UART_SCR_FLEN                 8

/****** Group UART, Register USR ***************************************
 * UART Status Register
 */
#define VTSS_UART_USR                        VTSS_REGISTER(UART,0x007c)

/* UART busy. */
#define  VTSS_F_UART_USR_BUSY                 VTSS_BIT(0)

/*********************************************************************** 
 *
 * Target MEMCTRL
 *
 * 
 *
 ***********************************************************************/

/***********************************************************************
 * Register group MEMCTRL
 *
 * DDR memory controller registers
 */

#define VTSS_RB_MEMCTRL_MEMCTRL                VTSS_IOADDR(MEMCTRL,0x0000)

/****** Group MEMCTRL, Register SCONR **********************************
 * SDRAM Config Register
 */
#define VTSS_MEMCTRL_SCONR                   VTSS_REGISTER(MEMCTRL,0x0000)

/* Specifies SDRAM data width in bits. */
#define  VTSS_F_S_DATA_WIDTH(x)               VTSS_ENCODE_BITFIELD(x,13,2)
#define   VTSS_F_S_DATA_WIDTH_FPOS             13
#define   VTSS_F_S_DATA_WIDTH_FLEN             2

/* Number of address bits for column address. */
#define  VTSS_F_S_COL_ADDR_WIDTH(x)           VTSS_ENCODE_BITFIELD(x,9,4)
#define   VTSS_F_S_COL_ADDR_WIDTH_FPOS         9
#define   VTSS_F_S_COL_ADDR_WIDTH_FLEN         4

/* Number of address bits for row address. */
#define  VTSS_F_S_ROW_ADDR_WIDTH(x)           VTSS_ENCODE_BITFIELD(x,5,4)
#define   VTSS_F_S_ROW_ADDR_WIDTH_FPOS         5
#define   VTSS_F_S_ROW_ADDR_WIDTH_FLEN         4

/* Number of bank address bits. */
#define  VTSS_F_S_BANK_ADDR_WIDTH(x)          VTSS_ENCODE_BITFIELD(x,3,2)
#define   VTSS_F_S_BANK_ADDR_WIDTH_FPOS        3
#define   VTSS_F_S_BANK_ADDR_WIDTH_FLEN        2

/****** Group MEMCTRL, Register STMG0R *********************************
 * SDRAM Timing Register0
 */
#define VTSS_MEMCTRL_STMG0R                  VTSS_REGISTER(MEMCTRL,0x0004)

/* See T_XSR. */
#define  VTSS_F_EXTENDED_T_XSR(x)             VTSS_ENCODE_BITFIELD(x,27,5)
#define   VTSS_F_EXTENDED_T_XSR_FPOS           27
#define   VTSS_F_EXTENDED_T_XSR_FLEN           5

/* See CAS_LATENCY.
 */
#define  VTSS_F_EXTENDED_CAS_LATENCY          VTSS_BIT(26)

/* Active-to-active command period. */
#define  VTSS_F_T_RC(x)                       VTSS_ENCODE_BITFIELD(x,22,4)
#define   VTSS_F_T_RC_FPOS                     22
#define   VTSS_F_T_RC_FLEN                     4

/* This field is extended with the EXTENDED_T_XSR field.
 * Exit self-refresh to active or auto-refresh command time; minimum time
 * controller should wait after taking SDRAM out of self-refresh mode
 * before issuing any active or auto-refresh commands. */
#define  VTSS_F_T_XSR(x)                      VTSS_ENCODE_BITFIELD(x,18,4)
#define   VTSS_F_T_XSR_FPOS                    18
#define   VTSS_F_T_XSR_FLEN                    4

/* Auto-refresh period; minimum time between two auto-refresh commands. */
#define  VTSS_F_T_RCAR(x)                     VTSS_ENCODE_BITFIELD(x,14,4)
#define   VTSS_F_T_RCAR_FPOS                   14
#define   VTSS_F_T_RCAR_FLEN                   4

/* For writes, delay from last data in to next precharge command. */
#define  VTSS_F_T_WR(x)                       VTSS_ENCODE_BITFIELD(x,12,2)
#define   VTSS_F_T_WR_FPOS                     12
#define   VTSS_F_T_WR_FLEN                     2

/* Precharge period. */
#define  VTSS_F_T_RP(x)                       VTSS_ENCODE_BITFIELD(x,9,3)
#define   VTSS_F_T_RP_FPOS                     9
#define   VTSS_F_T_RP_FLEN                     3

/* Minimum delay between active and read/write commands. */
#define  VTSS_F_T_RCD(x)                      VTSS_ENCODE_BITFIELD(x,6,3)
#define   VTSS_F_T_RCD_FPOS                    6
#define   VTSS_F_T_RCD_FLEN                    3

/* Minimum delay between active and precharge commands. */
#define  VTSS_F_T_RAS_MIN(x)                  VTSS_ENCODE_BITFIELD(x,2,4)
#define   VTSS_F_T_RAS_MIN_FPOS                2
#define   VTSS_F_T_RAS_MIN_FLEN                4

/* This field is extended with the EXTENDED_CAS_LATENCY field.
 * Delay in clock cycles between read command and availability of first
 * data. */
#define  VTSS_F_CAS_LATENCY(x)                VTSS_ENCODE_BITFIELD(x,0,2)
#define   VTSS_F_CAS_LATENCY_FPOS              0
#define   VTSS_F_CAS_LATENCY_FLEN              2

/****** Group MEMCTRL, Register STMG1R *********************************
 * SDRAM Timing Register1
 */
#define VTSS_MEMCTRL_STMG1R                  VTSS_REGISTER(MEMCTRL,0x0008)

/* Internal write-to-read delay for DDR-SDRAMs. */
#define  VTSS_F_T_WTR(x)                      VTSS_ENCODE_BITFIELD(x,20,2)
#define   VTSS_F_T_WTR_FPOS                    20
#define   VTSS_F_T_WTR_FLEN                    2

/* Number of auto-refreshes during initialization. DDR400 requires two
 * autorefreshes. */
#define  VTSS_F_NUM_INIT_REF(x)               VTSS_ENCODE_BITFIELD(x,16,4)
#define   VTSS_F_NUM_INIT_REF_FPOS             16
#define   VTSS_F_NUM_INIT_REF_FLEN             4

/* Number of clock cycles to hold SDRAM inputs stable after power up,
 * before issuing any commands. */
#define  VTSS_F_T_INIT(x)                     VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_T_INIT_FPOS                   0
#define   VTSS_F_T_INIT_FLEN                   16

/****** Group MEMCTRL, Register SCTLR **********************************
 * SDRAM Control Register
 */
#define VTSS_MEMCTRL_SCTLR                   VTSS_REGISTER(MEMCTRL,0x000c)

/* When set to 1, forces controller to do update of SDRAM extended mode
 * register. The bit is cleared by controller after it finishes extended
 * mode register update. */
#define  VTSS_F_SET_EXN_MODE_REG              VTSS_BIT(18)

/* SDRAM read-data-ready mode. When set to 1, indicates SDRAM read data is
 * sampled after s_rd_ready goes active. */
#define  VTSS_F_S_RD_READY_MODE               VTSS_BIT(17)

/* Number of SDRAM internal banks to be open at any time. */
#define  VTSS_F_NUM_OPEN_BANKS(x)             VTSS_ENCODE_BITFIELD(x,12,5)
#define   VTSS_F_NUM_OPEN_BANKS_FPOS           12
#define   VTSS_F_NUM_OPEN_BANKS_FLEN           5

/* When 1, indicates SDRAM is in self-refresh mode. When
 * self_refresh/deep_power_mode bit (bit 1 of SCTLR) is set, it may take
 * some time before SDRAM is put into self-refresh mode, depending on
 * whether all rows or one row are refreshed before entering self-refresh
 * mode defined by full_refresh_before_sr bit.
 * Before gating clock in self-refresh mode, ensure this bit is set. */
#define  VTSS_F_SELF_REFRESH_STATUS           VTSS_BIT(11)

/* When set to 1, forces controller to do update of SDRAM mode register.
 * The bit is cleared by controller once it finishes mode register update. */
#define  VTSS_F_SET_MODE_REG                  VTSS_BIT(9)

/* When reading from the memory, this indicates a delay in clock-cycles
 * before data from SDRAM is ready inside controller. DDR400 requires 2
 * cycles. */
#define  VTSS_F_READ_PIPE(x)                  VTSS_ENCODE_BITFIELD(x,6,3)
#define   VTSS_F_READ_PIPE_FPOS                6
#define   VTSS_F_READ_PIPE_FLEN                3

/* Controls number of refreshes done by memctl after SDRAM is taken out of
 * self-refresh mode. */
#define  VTSS_F_FULL_REFRESH_AFTER_SR         VTSS_BIT(5)

/* Controls number of refreshes done by memctl before putting SDRAM into
 * self-refresh mode. */
#define  VTSS_F_FULL_REFRESH_BEFORE_SR        VTSS_BIT(4)

/* Determines when row is precharged. */
#define  VTSS_F_PRECHARGE_ALGORITHM           VTSS_BIT(3)

/* Forces memctl to put SDRAM in power-down mode. Bit 19 determines the
 * type of power-down mode requested. */
#define  VTSS_F_POWER_DOWN_MODE               VTSS_BIT(2)

/* Forces memctl to put SDRAM in self-refresh mode. */
#define  VTSS_F_SELF_REFRESH                  VTSS_BIT(1)

/* Forces SDRAM controller to initialize SDRAM. This field is
 * automatically reset to 0 once initialization sequence is complete. */
#define  VTSS_F_INITIALIZE                    VTSS_BIT(0)

/****** Group MEMCTRL, Register SREFR **********************************
 * SDRAM Refresh Interval Register
 */
#define VTSS_MEMCTRL_SREFR                   VTSS_REGISTER(MEMCTRL,0x0010)

/* Number of clock cycles between consecutive refresh cycles. */
#define  VTSS_F_T_REF(x)                      VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_T_REF_FPOS                    0
#define   VTSS_F_T_REF_FLEN                    16

/****** Group MEMCTRL, Register EXN_MODE_REG ***************************
 * Extended Mode Register
 */
#define VTSS_MEMCTRL_EXN_MODE_REG            VTSS_REGISTER(MEMCTRL,0x00ac)

/* See datasheet for the selected SDRAM. */
#define  VTSS_F_OPERATION_MODE(x)             VTSS_ENCODE_BITFIELD(x,3,10)
#define   VTSS_F_OPERATION_MODE_FPOS           3
#define   VTSS_F_OPERATION_MODE_FLEN           10

/* See datasheet for the selected SDRAM. */
#define  VTSS_F_QFC                           VTSS_BIT(2)

/* See datasheet for the selected SDRAM. */
#define  VTSS_F_DRIVE_STRENGTH                VTSS_BIT(1)

/* See datasheet for the selected SDRAM. */
#define  VTSS_F_DLL                           VTSS_BIT(0)

/*********************************************************************** 
 *
 * Target TWI
 *
 * Two-Wire Interface Controller Registers
 *
 ***********************************************************************/

/***********************************************************************
 * Register group TWI
 *
 * Two-Wire Interface Controller Registers
 */

#define VTSS_RB_TWI_TWI                        VTSS_IOADDR(TWI,0x0000)

/****** Group TWI, Register CFG ****************************************
 * TWI Configuration
 */
#define VTSS_TWI_CFG                         VTSS_REGISTER(TWI,0x0000)

/* This bit controls whether the TWI controller has its slave disabled. If
 * this bit is set (slave is disabled), the controller functions only as a
 * master and does not perform any action that requires a slave. */
#define  VTSS_F_SLAVE_DIS                     VTSS_BIT(6)

/* Determines whether RESTART conditions may be sent when acting as a
 * master. Some older slaves do not support handling RESTART conditions;
 * however, RESTART conditions are used in several operations.
 * When RESTART is disabled, the master is prohibited from performing the
 * following functions:
 *  * Change direction within a transfer (split)
 *  * Send a START BYTE
 *  * Combined format transfers in 7-bit addressing modes
 *  * Read operation with a 10-bit address
 *  * Send multiple bytes per transfer
 * By replacing RESTART condition followed by a STOP and a subsequent START
 * condition, split operations are  broken down into multiple transfers. If
 * the above operations are performed, it will result in setting
 * RAW_INTR_STAT.TX_ABRT. */
#define  VTSS_F_RESTART_ENA                   VTSS_BIT(5)

/* Controls whether transfers starts in 7- or 10-bit addressing mode when
 * acting as a master. */
#define  VTSS_F_MASTER_10BITADDR              VTSS_BIT(4)

/* Controls whether the the TWI controller responds to 7- or 10-bit
 * addresses in slave mode. In 7-bit mode; transactions that involve 10-bit
 * addressing are ignored and only the lower 7 bits of the SAR register are
 * compared.  */
#define  VTSS_F_SLAVE_10BITADDR               VTSS_BIT(3)

/* These bits control at which speed the TWI controller operates; its
 * setting is relevant only in master mode. Hardware protects against
 * illegal values being programmed by software. */
#define  VTSS_F_SPEED(x)                      VTSS_ENCODE_BITFIELD(x,1,2)
#define   VTSS_F_SPEED_FPOS                    1
#define   VTSS_F_SPEED_FLEN                    2

/* This bit controls whether the TWI master is enabled. */
#define  VTSS_F_MASTER_ENA                    VTSS_BIT(0)

/****** Group TWI, Register TAR ****************************************
 * Target Address
 */
#define VTSS_TWI_TAR                         VTSS_REGISTER(TWI,0x0004)

/* This bit indicates whether software performs a General Call or START
 * BYTE command. */
#define  VTSS_F_GC_OR_START_ENA               VTSS_BIT(11)

/* If TAR.SPECIAL is set to 1, then this bit indicates whether a General
 * Call or START byte command is to be performed. */
#define  VTSS_F_GC_OR_START                   VTSS_BIT(10)

/* This is the target address for any master transaction. When
 * transmitting a General Call, these bits are ignored. To generate a START
 * BYTE, the CPU needs to write only once into these bits.
 * If the TAR and SAR are the same, loopback exists but the FIFOs are
 * shared between master and slave, so full loopback is not feasible. Only
 * one direction loopback mode is supported (simplex), not duplex. A master
 * cannot transmit to itself; it can transmit to only a slave. */
#define  VTSS_F_TAR(x)                        VTSS_ENCODE_BITFIELD(x,0,10)
#define   VTSS_F_TAR_FPOS                      0
#define   VTSS_F_TAR_FLEN                      10

/****** Group TWI, Register SAR ****************************************
 * Slave Address
 */
#define VTSS_TWI_SAR                         VTSS_REGISTER(TWI,0x0008)

/* The SAR holds the slave address when the TWI is operating as a slave.
 * For 7-bit addressing, only SAR[6:0] is used.
 * This register can be written only when the TWI interface is disabled
 * (ENABLE = 0). */
#define  VTSS_F_SAR(x)                        VTSS_ENCODE_BITFIELD(x,0,10)
#define   VTSS_F_SAR_FPOS                      0
#define   VTSS_F_SAR_FLEN                      10

/****** Group TWI, Register DATA_CMD ***********************************
 * Rx/Tx Data Buffer and Command
 */
#define VTSS_TWI_DATA_CMD                    VTSS_REGISTER(TWI,0x0010)

/* This bit controls whether a read or a write is performed. This bit does
 * not control the direction when the TWI acts as a slave. It controls only
 * the direction when it acts as a master.
 * When a command is entered in the TX FIFO, this bit distinguishes the
 * write and read commands. In slave-receiver mode, this bit is a "don't
 * care" because writes to this register are not required. In
 * slave-transmitter mode, a "0" indicates that CPU data is to be
 * transmitted and as DATA. 
 * When programming this bit, please remember the following: attempting to
 * perform a read operation after a General Call command has been sent
 * results in a TX_ABRT interrupt (RAW_INTR_STAT.R_TX_ABRT), unless
 * TAR.SPECIAL has been cleared.
 * If a "1" is written to this bit after receiving a RD_REQ interrupt, then
 * a TX_ABRT interrupt occurs.
 * NOTE: It is possible that while attempting a master TWI read transfer, a
 * RD_REQ interrupt may have occurred simultaneously due to a remote TWI
 * master addressing this controller. In this type of scenario, the TWI
 * controller ignores the DATA_CMD write, generates a TX_ABRT interrupt,
 * and waits to service the RD_REQ interrupt. */
#define  VTSS_F_CMD                           VTSS_BIT(8)

/* This register contains the data to be transmitted or received on the
 * TWI bus. If you are writing to this register and want to perform a read,
 * this field is ignored by the controller. However, when you read this
 * register, these bits return the value of data received on the TWI
 * interface. */
#define  VTSS_F_DATA(x)                       VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_DATA_FPOS                     0
#define   VTSS_F_DATA_FLEN                     8

/****** Group TWI, Register SS_SCL_HCNT ********************************
 * Standard Speed TWI Clock SCL High Count
 */
#define VTSS_TWI_SS_SCL_HCNT                 VTSS_REGISTER(TWI,0x0014)

/* This register sets the SCL clock divider for the high-period in
 * standard speed. This value must result in a high period of no less than
 * 4us. */
#define  VTSS_F_SS_SCL_HCNT(x)                VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_SS_SCL_HCNT_FPOS              0
#define   VTSS_F_SS_SCL_HCNT_FLEN              16

/****** Group TWI, Register SS_SCL_LCNT ********************************
 * Standard Speed TWI Clock SCL Low Count
 */
#define VTSS_TWI_SS_SCL_LCNT                 VTSS_REGISTER(TWI,0x0018)

/* This register sets the SCL clock divider for the low-period in standard
 * speed. This value must result in a value no less than 4.7us. */
#define  VTSS_F_SS_SCL_LCNT(x)                VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_SS_SCL_LCNT_FPOS              0
#define   VTSS_F_SS_SCL_LCNT_FLEN              16

/****** Group TWI, Register FS_SCL_HCNT ********************************
 * Fast Speed TWI Clock SCL High Count
 */
#define VTSS_TWI_FS_SCL_HCNT                 VTSS_REGISTER(TWI,0x001c)

/* This register sets the SCL clock divider for the high-period in fast
 * speed. This value must result in a value no less than 0.6us. */
#define  VTSS_F_FS_SCL_HCNT(x)                VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_FS_SCL_HCNT_FPOS              0
#define   VTSS_F_FS_SCL_HCNT_FLEN              16

/****** Group TWI, Register FS_SCL_LCNT ********************************
 * Fast Speed TWI Clock SCL Low Count
 */
#define VTSS_TWI_FS_SCL_LCNT                 VTSS_REGISTER(TWI,0x0020)

/* This register sets the SCL clock divider for the low-period in fast
 * speed. This value must result in a value no less than 1.3us. */
#define  VTSS_F_FS_SCL_LCNT(x)                VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_FS_SCL_LCNT_FPOS              0
#define   VTSS_F_FS_SCL_LCNT_FLEN              16

/****** Group TWI, Register INTR_STAT **********************************
 * Interrupt Status
 */
#define VTSS_TWI_INTR_STAT                   VTSS_REGISTER(TWI,0x002c)
#define  VTSS_F_GEN_CALL                      VTSS_BIT(11)
#define  VTSS_F_START_DET                     VTSS_BIT(10)
#define  VTSS_F_STOP_DET                      VTSS_BIT(9)
#define  VTSS_F_ACTIVITY                      VTSS_BIT(8)
#define  VTSS_F_RX_DONE                       VTSS_BIT(7)
#define  VTSS_F_TX_ABRT                       VTSS_BIT(6)
#define  VTSS_F_RD_REQ                        VTSS_BIT(5)
#define  VTSS_F_TX_EMPTY                      VTSS_BIT(4)
#define  VTSS_F_TX_OVER                       VTSS_BIT(3)
#define  VTSS_F_RX_FULL                       VTSS_BIT(2)
#define  VTSS_F_RX_OVER                       VTSS_BIT(1)
#define  VTSS_F_RX_UNDER                      VTSS_BIT(0)

/****** Group TWI, Register INTR_MASK **********************************
 * Interrupt Mask
 */
#define VTSS_TWI_INTR_MASK                   VTSS_REGISTER(TWI,0x0030)
#define  VTSS_F_M_GEN_CALL                    VTSS_BIT(11)
#define  VTSS_F_M_START_DET                   VTSS_BIT(10)
#define  VTSS_F_M_STOP_DET                    VTSS_BIT(9)
#define  VTSS_F_M_ACTIVITY                    VTSS_BIT(8)
#define  VTSS_F_M_RX_DONE                     VTSS_BIT(7)
#define  VTSS_F_M_TX_ABRT                     VTSS_BIT(6)
#define  VTSS_F_M_RD_REQ                      VTSS_BIT(5)
#define  VTSS_F_M_TX_EMPTY                    VTSS_BIT(4)
#define  VTSS_F_M_TX_OVER                     VTSS_BIT(3)
#define  VTSS_F_M_RX_FULL                     VTSS_BIT(2)
#define  VTSS_F_M_RX_OVER                     VTSS_BIT(1)
#define  VTSS_F_M_RX_UNDER                    VTSS_BIT(0)

/****** Group TWI, Register RAW_INTR_STAT ******************************
 * Raw Interrupt Status
 */
#define VTSS_TWI_RAW_INTR_STAT               VTSS_REGISTER(TWI,0x0034)

/* Set only when a General Call address is received and it is
 * acknowledged. It stays set until it is cleared either by disabling TWI
 * controller or when the CPU reads bit 0 of the CLR_GEN_CALL register. The
 * TWI controller stores the received data in the Rx buffer. */
#define  VTSS_F_R_GEN_CALL                    VTSS_BIT(11)

/* Indicates whether a START or RESTART condition has occurred on the TWI
 * regardless of whether the TWI controller is operating in slave or master
 * mode. */
#define  VTSS_F_R_START_DET                   VTSS_BIT(10)

/* Indicates whether a STOP condition has occurred on the TWI controller
 * regardless of whether the TWI controller is operating in slave or master
 * mode. */
#define  VTSS_F_R_STOP_DET                    VTSS_BIT(9)

/* This bit captures TWI activity and stays set until it is cleared. There
 * are four ways to clear it:
 * * Disabling the TWI controller
 * * Reading the CLR_ACTIVITY register
 * * Reading the CLR_INTR register
 * * VCore-II system reset
 * Once this bit is set, it stays set unless one of the four methods is
 * used to clear it. Even if the TWI controller module is idle, this bit
 * remains set until cleared, indicating that there was activity on the
 * bus. */
#define  VTSS_F_R_ACTIVITY                    VTSS_BIT(8)

/* When the TWI controller is acting as a slave-transmitter, this bit is
 * set to 1 if the master does not acknowledge a transmitted byte. This
 * occurs on the last byte of the transmission, indicating that the
 * transmission is done. */
#define  VTSS_F_R_RX_DONE                     VTSS_BIT(7)

/* This bit is set to 1 when the TWI controller is acting as a master is
 * unable to complete a command that the processor has sent. The conditions
 * that set this field are:
 * * No slave acknowledges the address byte.
 * * The addressed slave receiver does not acknowledge a byte of data.
 * * Attempting to send a master command when configured only to be a
 * slave.
 * * When CFG.RESTART_ENA is set to 0 (RESTART condition disabled), and the
 * processor attempts to issue a TWI function that is impossible to perform
 * without using RESTART conditions.
 * * High-speed master code is acknowledged (this controller does not
 * support high-speed).
 * * START BYTE is acknowledged.
 * * General Call address is not acknowledged.
 * * When a read request interrupt occurs and the processor has previously
 * placed data in the Tx buffer that has not been transmitted yet. This
 * data could have been intended to service a multi-byte RD_REQ that ended
 * up having fewer numbers of bytes requested.
 * *The TWI controller loses arbitration of the bus between transfers and
 * is then accessed as a slave-transmitter.
 * * If a read command is issued after a General Call command has been
 * issued. Disabling the TWI reverts it back to normal operation.
 * * If the CPU attempts to issue read command before a RD_REQ is serviced.
 * Anytime this bit is set, the contents of the transmit and receive
 * buffers are flushed. */
#define  VTSS_F_R_TX_ABRT                     VTSS_BIT(6)

/* This bit is set to 1 when the TWI controller acts as a slave and
 * another TWI master is attempting to read data from this controller. The
 * TWI controller holds the TWI bus in a wait state (SCL=0) until this
 * interrupt is serviced, which means that the slave has been addressed by
 * a remote master that is asking for data to be transferred. The processor
 * must respond to this interrupt and then write the requested data to the
 * DATA_CMD register. This bit is set to 0 just after the required data is
 * written to the DATA_CMD register. */
#define  VTSS_F_R_RD_REQ                      VTSS_BIT(5)

/* This bit is set to 1 when the transmit buffer is at or below the
 * threshold value set in the TX_TL register. It is automatically cleared
 * by hardware when the buffer level goes above the threshold. When ENABLE
 * is 0, the TX FIFO is flushed and held in reset. There the TX FIFO looks
 * like it has no data within it, so this bit is set to 1, provided there
 * is activity in the master or slave state machines. When there is no
 * longer activity, then with ENABLE_STATUS.BUSY=0, this bit is set to 0. */
#define  VTSS_F_R_TX_EMPTY                    VTSS_BIT(4)

/* Set during transmit if the transmit buffer is filled to TX_BUFFER_DEPTH
 * and the processor attempts to issue another TWI command by writing to
 * the DATA_CMD register. When the module is disabled, this bit keeps its
 * level until the master or slave state machines go into idle, and when
 * ENABLE_STATUS.BUSY goes to 0, this interrupt is cleared. */
#define  VTSS_F_R_TX_OVER                     VTSS_BIT(3)

/* Set when the receive buffer reaches or goes above the RX_TL threshold
 * in the RX_TL register. It is automatically cleared by hardware when
 * buffer level goes below the threshold. If the module is disabled
 * (ENABLE=0), the RX FIFO is flushed and held in reset; therefore the RX
 * FIFO is not full. So this bit is cleared once the ENABLE field is
 * programmed with a 0, regardless of the activity that continues. */
#define  VTSS_F_R_RX_FULL                     VTSS_BIT(2)

/* Set if the receive buffer is completely filled to RX_BUFFER_DEPTH and
 * an additional byte is received from an external TWI device. The TWI
 * controller acknowledges this, but any data bytes received after the FIFO
 * is full are lost. If the module is disabled (ENABLE=0), this bit keeps
 * its level until the master or slave state machines go into idle, and
 * when ENABLE_STATUS.BUSY goes to 0, this interrupt is cleared. */
#define  VTSS_F_R_RX_OVER                     VTSS_BIT(1)

/* Set if the processor attempts to read the receive buffer when it is
 * empty by reading from the DATA_CMD register. If the module is disabled
 * (ENABLE=0), this bit keeps its level until the master or slave state
 * machines go into idle, and when ENABLE_STATUS.BUSY goes to 0, this
 * interrupt is cleared. */
#define  VTSS_F_R_RX_UNDER                    VTSS_BIT(0)

/****** Group TWI, Register RX_TL **************************************
 * Receive FIFO Threshold
 */
#define VTSS_TWI_RX_TL                       VTSS_REGISTER(TWI,0x0038)

/* Controls the level of entries (or above) that triggers the RX_FULL
 * interrupt (bit 2 in RAW_INTR_STAT register). The valid range is 0-7. A
 * value of 0 sets the threshold for 1 entry, and a value of 7 sets the
 * threshold for 8 entries. */
#define  VTSS_F_RX_TL(x)                      VTSS_ENCODE_BITFIELD(x,0,3)
#define   VTSS_F_RX_TL_FPOS                    0
#define   VTSS_F_RX_TL_FLEN                    3

/****** Group TWI, Register TX_TL **************************************
 * Transmit FIFO Threshold
 */
#define VTSS_TWI_TX_TL                       VTSS_REGISTER(TWI,0x003c)

/* Controls the level of entries (or below) that trigger the TX_EMPTY
 * interrupt (bit 4 in RAW_INTR_STAT register). The valid range is 0-7. A
 * value of 0 sets the threshold for 0 entries, and a value of 7 sets the
 * threshold for 7 entries. */
#define  VTSS_F_TX_TL(x)                      VTSS_ENCODE_BITFIELD(x,0,3)
#define   VTSS_F_TX_TL_FPOS                    0
#define   VTSS_F_TX_TL_FLEN                    3

/****** Group TWI, Register CLR_INTR ***********************************
 * Clear Combined and Individual Interrupt
 */
#define VTSS_TWI_CLR_INTR                    VTSS_REGISTER(TWI,0x0040)

/* Read this register to clear the combined interrupt, all individual
 * interrupts, and the TX_ABRT_SOURCE register. This bit does not clear
 * hardware clearable interrupts but software clearable interrupts. Refer
 * to Bit 9 of the TX_ABRT_SOURCE register for an exception to clearing
 * TX_ABRT_SOURCE. */
#define  VTSS_F_CLR_INTR                      VTSS_BIT(0)

/****** Group TWI, Register CLR_RX_UNDER *******************************
 * Clear RX_UNDER Interrupt
 */
#define VTSS_TWI_CLR_RX_UNDER                VTSS_REGISTER(TWI,0x0044)

/* Read this register to clear the R_RX_UNDER interrupt (bit 0) of the
 * RAW_INTR_STAT register. */
#define  VTSS_F_CLR_RX_UNDER                  VTSS_BIT(0)

/****** Group TWI, Register CLR_RX_OVER ********************************
 * Clear RX_OVER Interrupt
 */
#define VTSS_TWI_CLR_RX_OVER                 VTSS_REGISTER(TWI,0x0048)

/* Read this register to clear the R_RX_OVER interrupt (bit 1) of the
 * RAW_INTR_STAT register. */
#define  VTSS_F_CLR_RX_OVER                   VTSS_BIT(0)

/****** Group TWI, Register CLR_TX_OVER ********************************
 * Clear TX_OVER Interrupt
 */
#define VTSS_TWI_CLR_TX_OVER                 VTSS_REGISTER(TWI,0x004c)

/* Read this register to clear the R_TX_OVER interrupt (bit 3) of the
 * RAW_INTR_STAT register. */
#define  VTSS_F_CLR_TX_OVER                   VTSS_BIT(0)

/****** Group TWI, Register CLR_RD_REQ *********************************
 * Clear RD_REQ Interrupt
 */
#define VTSS_TWI_CLR_RD_REQ                  VTSS_REGISTER(TWI,0x0050)

/* Read this register to clear the R_RD_REQ interrupt (bit 5) of the
 * RAW_INTR_STAT register. */
#define  VTSS_F_CLR_RD_REQ                    VTSS_BIT(0)

/****** Group TWI, Register CLR_TX_ABRT ********************************
 * Clear TX_ABRT Interrupt
 */
#define VTSS_TWI_CLR_TX_ABRT                 VTSS_REGISTER(TWI,0x0054)

/* Read this register to clear the R_TX_ABRT interrupt (bit 6) of the
 * RAW_INTR_STAT register, and the TX_ABRT_SOURCE register. Refer to Bit 9
 * of the TX_ABRT_SOURCE register for an exception to clearing
 * TX_ABRT_SOURCE. */
#define  VTSS_F_CLR_TX_ABRT                   VTSS_BIT(0)

/****** Group TWI, Register CLR_RX_DONE ********************************
 * Clear RX_DONE Interrupt
 */
#define VTSS_TWI_CLR_RX_DONE                 VTSS_REGISTER(TWI,0x0058)

/* Read this register to clear the R_RX_DONE interrupt (bit 7) of the
 * RAW_INTR_STAT register. */
#define  VTSS_F_CLR_RX_DONE                   VTSS_BIT(0)

/****** Group TWI, Register CLR_ACTIVITY *******************************
 * Clear ACTIVITY Interrupt
 */
#define VTSS_TWI_CLR_ACTIVITY                VTSS_REGISTER(TWI,0x005c)

/* Reading this register clears the ACTIVITY interrupt if the TWI
 * controller is not active anymore. If the TWI  controller is still active
 * on the bus, the ACTIVITY interrupt bit continues to be set. It is
 * automatically cleared by hardware if the module is disabled and if there
 * is no further activity on the bus. The value read from this register to
 * get status of the R_ACTIVITY interrupt (bit 8) of the RAW_INTR_STAT
 * register. */
#define  VTSS_F_CLR_ACTIVITY                  VTSS_BIT(0)

/****** Group TWI, Register CLR_STOP_DET *******************************
 * Clear STOP_DET Interrupt
 */
#define VTSS_TWI_CLR_STOP_DET                VTSS_REGISTER(TWI,0x0060)

/* Read this register to clear the R_STOP_DET interrupt (bit 9) of the
 * RAW_INTR_STAT register. */
#define  VTSS_F_CLR_STOP_DET                  VTSS_BIT(0)

/****** Group TWI, Register CLR_START_DET ******************************
 * Clear START_DET Interrupt
 */
#define VTSS_TWI_CLR_START_DET               VTSS_REGISTER(TWI,0x0064)

/* Read this register to clear the R_START_DET interrupt (bit 10) of the
 * RAW_INTR_STAT register. */
#define  VTSS_F_CLR_START_DET                 VTSS_BIT(0)

/****** Group TWI, Register CLR_GEN_CALL *******************************
 * Clear GEN_CALL Interrupt
 */
#define VTSS_TWI_CLR_GEN_CALL                VTSS_REGISTER(TWI,0x0068)

/* Read this register to clear the R_GEN_CALL interrupt (bit 11) of
 * RAW_INTR_STAT register. */
#define  VTSS_F_CLR_GEN_CALL                  VTSS_BIT(0)

/****** Group TWI, Register CTRL ***************************************
 * TWI Control
 */
#define VTSS_TWI_CTRL                        VTSS_REGISTER(TWI,0x006c)

/* Controls whether the TWI controller is enabled. Software can disable
 * the controller while it is active. However, it is important that care be
 * taken to ensure that the controller is disabled properly.
 * When TWI controller is disabled, the following occurs:
 * * The TX FIFO and RX FIFO get flushed.
 * * The interrupt bits in the RAW_INTR_STAT register are cleared.
 * * Status bits in the INTR_STAT register are still active until the TWI
 * controller goes into IDLE state.
 * If the module is transmitting, it stops as well as deletes the contents
 * of the transmit buffer after the current transfer is complete. If the
 * module is receiving, the controller stops the current transfer at the
 * end of the current byte and does not acknowledge the transfer. */
#define  VTSS_F_ENABLE                        VTSS_BIT(0)

/****** Group TWI, Register STAT ***************************************
 * TWI Status
 */
#define VTSS_TWI_STAT                        VTSS_REGISTER(TWI,0x0070)

/* Slave FSM Activity Status. When the Slave Finite State Machine (FSM) is
 * not in the IDLE state, this bit is set. */
#define  VTSS_F_SLV_ACTIVITY                  VTSS_BIT(6)

/* Master FSM Activity Status. When the Master Finite State Machine (FSM)
 * is not in the IDLE state, this bit is set. */
#define  VTSS_F_MST_ACTIVITY                  VTSS_BIT(5)

/* Receive FIFO Completely Full. When the receive FIFO is completely full,
 * this bit is set. When the receive FIFO contains one or more empty
 * location, this bit is cleared. */
#define  VTSS_F_RFF                           VTSS_BIT(4)

/* Receive FIFO Not Empty. Set when the receive FIFO contains one or more
 * entries and is cleared when the receive FIFO is empty. This bit can be
 * polled by software to completely empty the receive FIFO. */
#define  VTSS_F_RFNE                          VTSS_BIT(3)

/* Transmit FIFO Completely Empty. When the transmit FIFO is completely
 * empty, this bit is set. When it contains one or more valid entries, this
 * bit is cleared. This bit field does not request an interrupt. */
#define  VTSS_F_TFE                           VTSS_BIT(2)

/* Transmit FIFO Not Full. Set when the transmit FIFO contains one or more
 * empty locations, and is cleared when the FIFO is full. */
#define  VTSS_F_TFNF                          VTSS_BIT(1)

/* TWI Activity Status. */
#define  VTSS_F_BUS_ACTIVITY                  VTSS_BIT(0)

/****** Group TWI, Register TXFLR **************************************
 * Transmit FIFO Level
 */
#define VTSS_TWI_TXFLR                       VTSS_REGISTER(TWI,0x0074)

/* Transmit FIFO Level. Contains the number of valid data entries in the
 * transmit FIFO. */
#define  VTSS_F_TXFLR(x)                      VTSS_ENCODE_BITFIELD(x,0,3)
#define   VTSS_F_TXFLR_FPOS                    0
#define   VTSS_F_TXFLR_FLEN                    3

/****** Group TWI, Register RXFLR **************************************
 * Receive FIFO Level
 */
#define VTSS_TWI_RXFLR                       VTSS_REGISTER(TWI,0x0078)

/* Receive FIFO Level. Contains the number of valid data entries in the
 * receive FIFO. */
#define  VTSS_F_RXFLR(x)                      VTSS_ENCODE_BITFIELD(x,0,3)
#define   VTSS_F_RXFLR_FPOS                    0
#define   VTSS_F_RXFLR_FLEN                    3

/****** Group TWI, Register TX_ABRT_SOURCE *****************************
 * Transmit Abort Source
 */
#define VTSS_TWI_TX_ABRT_SOURCE              VTSS_REGISTER(TWI,0x0080)

/* When the processor side responds to a slave mode request for data to be
 * transmitted to a remote master and user writes a 1 to DATA_CMD.CMD. */
#define  VTSS_F_ABRT_SLVRD_INTX               VTSS_BIT(15)

/* Slave lost the bus while transmitting data to a remote master.
 * TX_ABRT_SOURCE[12] is set at the same time.
 * Note: Even though the slave never "owns" the bus, something could go
 * wrong on the bus. This is a fail safe check. For instance, during a data
 * transmission at the low-to-high transition of SCL, if what is on the
 * data bus is not what is supposed to be transmitted, then the TWI
 * controller no longer own the bus. */
#define  VTSS_F_ABRT_SLV_ARBLOST              VTSS_BIT(14)

/* Slave has received a read command and some data exists in the TX FIFO
 * so the slave issues a TX_ABRT interrupt to flush old data in TX FIFO. */
#define  VTSS_F_ABRT_SLVFLUSH_TXFIFO          VTSS_BIT(13)

/* Master has lost arbitration, or if TX_ABRT_SOURCE[14] is also set, then
 * the slave transmitter has lost arbitration.
 * Note: the TWI controller can be both master and slave at the same time. */
#define  VTSS_F_ARB_LOST                      VTSS_BIT(12)

/* User tries to initiate a Master operation with the Master mode
 * disabled. */
#define  VTSS_F_ABRT_MASTER_DIS               VTSS_BIT(11)

/* The restart is disabled (RESTART_ENA bit (CFG[5]) = 0) and the master
 * sends a read command in 10-bit addressing mode. */
#define  VTSS_F_ABRT_10B_RD_NORSTRT           VTSS_BIT(10)

/* To clear Bit 9, the source of the ABRT_SBYTE_NORSTRT must be fixed
 * first; restart must be enabled (CFG[5]=1), the SPECIAL bit must be
 * cleared (TAR[11]), or the GC_OR_START bit must be cleared (TAR[10]).
 * Once the source of the ABRT_SBYTE_NORSTRT is fixed, then this bit can be
 * cleared in the same manner as other bits in this register. If the source
 * of the ABRT_SBYTE_NORSTRT is not fixed before attempting to clear this
 * bit, bit 9 clears for one cycle and then gets re-asserted. */
#define  VTSS_F_ABRT_SBYTE_NORSTRT            VTSS_BIT(9)

/* Master has sent a START Byte and the START Byte was acknowledged (wrong
 * behavior). */
#define  VTSS_F_ABRT_SBYTE_ACKDET             VTSS_BIT(7)

/* TWI controller in master mode sent a General Call but the user
 * programmed the byte following the General Call to be a read from the bus
 * (DATA_CMD[9] is set to 1). */
#define  VTSS_F_ABRT_GCALL_READ               VTSS_BIT(5)

/* TWI controller in master mode sent a General Call and no slave on the
 * bus acknowledged the General Call. */
#define  VTSS_F_ABRT_GCALL_NOACK              VTSS_BIT(4)

/* This is a master-mode only bit. Master has received an acknowledgement
 * for the address, but when it sent data byte(s) following the address, it
 * did not receive an acknowledge from the remote slave(s). */
#define  VTSS_F_ABRT_TXDATA_NOACK             VTSS_BIT(3)

/* Master is in 10-bit address mode and the second address byte of the
 * 10-bit address was not acknowledged by any slave. */
#define  VTSS_F_ABRT_10ADDR2_NOACK            VTSS_BIT(2)

/* Master is in 10-bit address mode and the first 10-bit address byte was
 * not acknowledged by any slave. */
#define  VTSS_F_ABRT_10ADDR1_NOACK            VTSS_BIT(1)

/* Master is in 7-bit addressing mode and the address sent was not
 * acknowledged by any slave. */
#define  VTSS_F_ABRT_7B_ADDR_NOACK            VTSS_BIT(0)

/****** Group TWI, Register SDA_SETUP **********************************
 * SDA Setup
 */
#define VTSS_TWI_SDA_SETUP                   VTSS_REGISTER(TWI,0x0094)

/* This register controls the amount of time delay (in terms of number of
 * VCore-II clock periods) introduced in the rising edge of SCL, relative
 * to SDA changing, when the TWI controller services a read request in a
 * slave-receiver operation. The minimum for fast mode is 100ns, for nomal
 * mode the minimum is 250ns. */
#define  VTSS_F_SDA_SETUP(x)                  VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_SDA_SETUP_FPOS                0
#define   VTSS_F_SDA_SETUP_FLEN                8

/****** Group TWI, Register ACK_GEN_CALL *******************************
 * ACK General Call
 */
#define VTSS_TWI_ACK_GEN_CALL                VTSS_REGISTER(TWI,0x0098)

/* ACK General Call. When set to 1, the TWI controller responds with a ACK
 * when it receives a General Call. Otherwise, the controller responds with
 * a NACK. */
#define  VTSS_F_ACK_GEN_CALL                  VTSS_BIT(0)

/****** Group TWI, Register ENABLE_STATUS ******************************
 * Enable Status
 */
#define VTSS_TWI_ENABLE_STATUS               VTSS_REGISTER(TWI,0x009c)

/* Slave FIFO Filled and Flushed. This bit indicates if a Slave-Receiver
 * operation has been aborted with at least 1 data byte received from a TWI
 * transfer due to the setting of ENABLE from 1 to 0.
 * When read as 1, the TWI controller is deemed to have been actively
 * engaged in an aborted TWI transfer (with matching address) and the data
 * phase of the TWI transfer has been entered, even though the data byte
 * has been responded with a NACK.
 * When read as 0, the TWI controller is deemed to have been disabled when
 * the TWI bus is idle. */
#define  VTSS_F_SLV_FIFO_FILLED_AND_FLUSHED   VTSS_BIT(2)

/* Slave-Receiver Operation Aborted. This bit indicates if a
 * Slave-Receiver operation has been aborted due to the setting of the
 * ENABLE register from 1 to 0.
 * When read as 1, the TWI controller is deemed to have forced a NACK
 * during any part of a TWI transfer, irrespective of whether the TWI
 * address matches the slave address set in the TWI controller (SAR
 * register).
 * When read as 0, the TWI controller is deemed to have been disabled when
 * the TWI bus is idle. */
#define  VTSS_F_SLV_RX_ABORTED                VTSS_BIT(1)

/* When read as 1, the TWI controller is deemed to be actively involved in
 * an TWI transfer, irrespective of whether being in an address or data
 * phase for all master or slave modes. When read as 0, the TWI controller
 * is deemed completely inactive. */
#define  VTSS_F_BUSY                          VTSS_BIT(0)

/*********************************************************************** 
 *
 * Target SBA
 *
 * 
 *
 ***********************************************************************/

/***********************************************************************
 * Register group SBA
 *
 * Shared Bus arbiter registers
 */

#define VTSS_RB_SBA_SBA                        VTSS_IOADDR(SBA,0x0000)

/****** Group SBA, Register PL1 ****************************************
 * Arbitration Priority CPU Data Interface
 */
#define VTSS_SBA_PL1                         VTSS_REGISTER(SBA,0x0000)

/* Arbitration priority for CPU data interface. */
#define  VTSS_F_PL1(x)                        VTSS_ENCODE_BITFIELD(x,0,4)
#define   VTSS_F_PL1_FPOS                      0
#define   VTSS_F_PL1_FLEN                      4

/****** Group SBA, Register PL2 ****************************************
 * Arbitration Priority CPU Instruction Interface
 */
#define VTSS_SBA_PL2                         VTSS_REGISTER(SBA,0x0004)

/* Arbitration priority for CPU instruction interface. The instruction
 * interface must always have lower priority than the CPU data interface. */
#define  VTSS_F_PL2(x)                        VTSS_ENCODE_BITFIELD(x,0,4)
#define   VTSS_F_PL2_FPOS                      0
#define   VTSS_F_PL2_FLEN                      4

/****** Group SBA, Register PL3 ****************************************
 * Arbitration Priority FDMA
 */
#define VTSS_SBA_PL3                         VTSS_REGISTER(SBA,0x0008)

/* Arbitration priority for FDMA. */
#define  VTSS_F_PL3(x)                        VTSS_ENCODE_BITFIELD(x,0,4)
#define   VTSS_F_PL3_FPOS                      0
#define   VTSS_F_PL3_FLEN                      4

/****** Group SBA, Register WT_EN **************************************
 * Weighted-Token Arbitration Scheme Enable
 */
#define VTSS_SBA_WT_EN                       VTSS_REGISTER(SBA,0x004c)

/* Use this field to enable weighted-token arbitration scheme. */
#define  VTSS_F_WT_EN                         VTSS_BIT(0)

/****** Group SBA, Register WT_TCL *************************************
 * Clock Tokens Refresh Period
 */
#define VTSS_SBA_WT_TCL                      VTSS_REGISTER(SBA,0x0050)

/* Refresh period length for the weighted-token arbitration scheme. */
#define  VTSS_F_WT_TCL(x)                     VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_WT_TCL_FPOS                   0
#define   VTSS_F_WT_TCL_FLEN                   16

/****** Group SBA, Register WT_CL1 *************************************
 * Clock Tokens CPU Data Interface
 */
#define VTSS_SBA_WT_CL1                      VTSS_REGISTER(SBA,0x0054)

/* Number of tokens the CPU data interface is granted at the start of each
 * refresh period for weighted-token arbitration scheme. If configured with
 * a value of zero, the master is considered to have infinite tokens. */
#define  VTSS_F_WT_CL1(x)                     VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_WT_CL1_FPOS                   0
#define   VTSS_F_WT_CL1_FLEN                   16

/****** Group SBA, Register WT_CL2 *************************************
 * Clock Tokens CPU Instruction Interface
 */
#define VTSS_SBA_WT_CL2                      VTSS_REGISTER(SBA,0x0058)

/* Number of tokens the CPU instruction interface is granted at the start
 * of each refresh period for weighted-token arbitration scheme. If
 * configured with a value of zero, the master is considered to have
 * infinite tokens. */
#define  VTSS_F_WT_CL2(x)                     VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_WT_CL2_FPOS                   0
#define   VTSS_F_WT_CL2_FLEN                   16

/****** Group SBA, Register WT_CL3 *************************************
 * Clock Tokens FDMA Interface
 */
#define VTSS_SBA_WT_CL3                      VTSS_REGISTER(SBA,0x005c)

/* Number of tokens the FDMA is granted at the start of each refresh
 * period for weighted-token arbitration scheme. If configured with a value
 * of zero, the master is considered to have infinite tokens. */
#define  VTSS_F_WT_CL3(x)                     VTSS_ENCODE_BITFIELD(x,0,16)
#define   VTSS_F_WT_CL3_FPOS                   0
#define   VTSS_F_WT_CL3_FLEN                   16

/*********************************************************************** 
 *
 * Target FDMA
 *
 * Frame DMA Controller
 *
 ***********************************************************************/

/***********************************************************************
 * Register group CH
 *
 * DMA Channel Controller configuration
 */

#define VTSS_RB_FDMA_CH                        VTSS_IOADDR(FDMA,0x0000)

/****** Group CH, Register SAR *****************************************
 * Source Address
 */
#define VTSS_CH0_SAR                         VTSS_REGISTER(FDMA,0x0000)
#define VTSS_CH1_SAR                         VTSS_REGISTER(FDMA,0x0058)
#define VTSS_CH2_SAR                         VTSS_REGISTER(FDMA,0x00b0)
#define VTSS_CH3_SAR                         VTSS_REGISTER(FDMA,0x0108)
#define VTSS_CH4_SAR                         VTSS_REGISTER(FDMA,0x0160)
#define VTSS_CH5_SAR                         VTSS_REGISTER(FDMA,0x01b8)
#define VTSS_CH6_SAR                         VTSS_REGISTER(FDMA,0x0210)
#define VTSS_CH7_SAR                         VTSS_REGISTER(FDMA,0x0268)

/****** Group CH, Register DAR *****************************************
 * Destination Address
 */
#define VTSS_CH0_DAR                         VTSS_REGISTER(FDMA,0x0008)
#define VTSS_CH1_DAR                         VTSS_REGISTER(FDMA,0x0060)
#define VTSS_CH2_DAR                         VTSS_REGISTER(FDMA,0x00b8)
#define VTSS_CH3_DAR                         VTSS_REGISTER(FDMA,0x0110)
#define VTSS_CH4_DAR                         VTSS_REGISTER(FDMA,0x0168)
#define VTSS_CH5_DAR                         VTSS_REGISTER(FDMA,0x01c0)
#define VTSS_CH6_DAR                         VTSS_REGISTER(FDMA,0x0218)
#define VTSS_CH7_DAR                         VTSS_REGISTER(FDMA,0x0270)

/****** Group CH, Register LLP *****************************************
 * Linked List Pointer
 */
#define VTSS_CH0_LLP                         VTSS_REGISTER(FDMA,0x0010)
#define VTSS_CH1_LLP                         VTSS_REGISTER(FDMA,0x0068)
#define VTSS_CH2_LLP                         VTSS_REGISTER(FDMA,0x00c0)
#define VTSS_CH3_LLP                         VTSS_REGISTER(FDMA,0x0118)
#define VTSS_CH4_LLP                         VTSS_REGISTER(FDMA,0x0170)
#define VTSS_CH5_LLP                         VTSS_REGISTER(FDMA,0x01c8)
#define VTSS_CH6_LLP                         VTSS_REGISTER(FDMA,0x0220)
#define VTSS_CH7_LLP                         VTSS_REGISTER(FDMA,0x0278)

/* Write the 32-bit aligned address of the first DCB in the chain of DCBs.
 * The DMA channel will update this field as it traverses the list of DCBs.
 * The two least significant bits will be zeroed out before being used. */
#define  VTSS_F_LOC(x)                        VTSS_ENCODE_BITFIELD(x,2,29)
#define   VTSS_F_LOC_FPOS                      2
#define   VTSS_F_LOC_FLEN                      29

/****** Group CH, Register CTL0 ****************************************
 * DMA transfer control
 */
#define VTSS_CH0_CTL0                        VTSS_REGISTER(FDMA,0x0018)
#define VTSS_CH1_CTL0                        VTSS_REGISTER(FDMA,0x0070)
#define VTSS_CH2_CTL0                        VTSS_REGISTER(FDMA,0x00c8)
#define VTSS_CH3_CTL0                        VTSS_REGISTER(FDMA,0x0120)
#define VTSS_CH4_CTL0                        VTSS_REGISTER(FDMA,0x0178)
#define VTSS_CH5_CTL0                        VTSS_REGISTER(FDMA,0x01d0)
#define VTSS_CH6_CTL0                        VTSS_REGISTER(FDMA,0x0228)
#define VTSS_CH7_CTL0                        VTSS_REGISTER(FDMA,0x0280)

/* Block chaining is enabled on the source side if the LLP_SRC_EN field is
 * high and the LLP is non-zero.  If enabled and LLP != 0 will continue on
 * the next DCB upon completion of current DCB. 
 */
#define  VTSS_F_LLP_SRC_EN                    VTSS_BIT(28)

/* Block chaining is enabled on the destination side if the LLP_DST_EN
 * field is high and LLP is non-zero.  If enabled and LLP != 0 will
 * continue on the next DCB upon completion of current DCB.
 */
#define  VTSS_F_LLP_DST_EN                    VTSS_BIT(27)

/* Source Master Select. 
 * 
 * INJ / GP: Must be set to 0
 * 
 * XTR: Must be set to 1
 */
#define  VTSS_F_SMS(x)                        VTSS_ENCODE_BITFIELD(x,25,2)
#define   VTSS_F_SMS_FPOS                      25
#define   VTSS_F_SMS_FLEN                      2

/* Destination Master Select. 
 * 
 * XTR / GP: Must be set to 0
 * 
 * INJ: Must be set to 1
 */
#define  VTSS_F_DMS(x)                        VTSS_ENCODE_BITFIELD(x,23,2)
#define   VTSS_F_DMS_FPOS                      23
#define   VTSS_F_DMS_FLEN                      2

/* Transfer Type and Flow Control. 
 * 
 * GP: Must be set to 0
 * INJ: Must be set to 0 or 1
 * XTR: Must be set to 4
 */
#define  VTSS_F_TT_FC(x)                      VTSS_ENCODE_BITFIELD(x,20,3)
#define   VTSS_F_TT_FC_FPOS                    20
#define   VTSS_F_TT_FC_FLEN                    3

/* Source Burst Transaction Length. 
 * 
 * INJ / GP: Number of data items, each of width CTL.SRC_TR_WIDTH, to be
 * read from the source every time a source burst transaction request is
 * made from either the corresponding hardware or software handshaking
 * interface. 
 * 
 * XTR : Must be <3 */
#define  VTSS_F_SRC_MSIZE(x)                  VTSS_ENCODE_BITFIELD(x,14,3)
#define   VTSS_F_SRC_MSIZE_FPOS                14
#define   VTSS_F_SRC_MSIZE_FLEN                3

/* Destination Burst Transaction Length. 
 * 
 * INJ / GP: Number of data items, each of width CTL.DST_TR_WIDTH, to be
 * written to the destination every time a destination burst transaction
 * request is made from either the corresponding hardware or software
 * handshaking interface. 
 * 
 * XTR : Must be <3 */
#define  VTSS_F_DEST_MSIZE(x)                 VTSS_ENCODE_BITFIELD(x,11,3)
#define   VTSS_F_DEST_MSIZE_FPOS               11
#define   VTSS_F_DEST_MSIZE_FLEN               3

/* Source Address Increment. 
 * 
 * INJ / GP: Indicates whether to increment or decrement the source address
 * on every source transfer. If the device is fetching data from a source
 * peripheral FIFO with a fixed address, then set this field to "No
 * change."
 * 
 * XTR: Must be set to "No change." */
#define  VTSS_F_SINC(x)                       VTSS_ENCODE_BITFIELD(x,9,2)
#define   VTSS_F_SINC_FPOS                     9
#define   VTSS_F_SINC_FLEN                     2

/* Destination Address Increment.
 * 
 * XTR / GP: Indicates whether to increment or decrement the destination
 * address on every destination transfer. If your device is writing data to
 * a destination peripheral FIFO with a fixed address, then set this field
 * to "No change."
 * 
 * INJ: Must be set to "No change." */
#define  VTSS_F_DINC(x)                       VTSS_ENCODE_BITFIELD(x,7,2)
#define   VTSS_F_DINC_FPOS                     7
#define   VTSS_F_DINC_FLEN                     2

/* Source Transfer Width.
 * 
 * GP:	Specifies source address alignment (e.g. 32-bit transfer can only
 * be 32 bit alligned).
 * 
 * INJ /  XTR: Must be set to 2.
 */
#define  VTSS_F_SRC_TR_WIDTH(x)               VTSS_ENCODE_BITFIELD(x,4,3)
#define   VTSS_F_SRC_TR_WIDTH_FPOS             4
#define   VTSS_F_SRC_TR_WIDTH_FLEN             3

/* Destination Transfer Width.
 * 
 * GP:	Specifies destination address alignment (e.g. 32-bit transfer can
 * only be 32 bit alligned).
 * 
 * INJ /  XTR: Must be set to 2.
 */
#define  VTSS_F_DST_TR_WIDTH(x)               VTSS_ENCODE_BITFIELD(x,1,3)
#define   VTSS_F_DST_TR_WIDTH_FPOS             1
#define   VTSS_F_DST_TR_WIDTH_FLEN             3

/* Interrupt Enable Bit. If set, then all interrupt-generating sources are
 * enabled.
 */
#define  VTSS_F_INT_EN                        VTSS_BIT(0)

/****** Group CH, Register CTL1 ****************************************
 * DMA transfer control
 */
#define VTSS_CH0_CTL1                        VTSS_REGISTER(FDMA,0x001c)
#define VTSS_CH1_CTL1                        VTSS_REGISTER(FDMA,0x0074)
#define VTSS_CH2_CTL1                        VTSS_REGISTER(FDMA,0x00cc)
#define VTSS_CH3_CTL1                        VTSS_REGISTER(FDMA,0x0124)
#define VTSS_CH4_CTL1                        VTSS_REGISTER(FDMA,0x017c)
#define VTSS_CH5_CTL1                        VTSS_REGISTER(FDMA,0x01d4)
#define VTSS_CH6_CTL1                        VTSS_REGISTER(FDMA,0x022c)
#define VTSS_CH7_CTL1                        VTSS_REGISTER(FDMA,0x0284)

/* Done bit
 * Software can poll the DCB CTL.DONE bit to see when a block transfer is
 * complete. The DCB CTL.DONE bit should be cleared
 * when the linked lists are set up in memory prior to enabling the
 * channel. */
#define  VTSS_F_DONE                          VTSS_BIT(12)

/* Block Transfer Size. 
 * 
 * INJ / GP : The number programmed into BLOCK_TS indicates the total
 * number of single transactions to perform for every block transfer.
 * 
 * XTR: Updated with the number of 32-bits words returned.
 * 
 * Once the transfer starts, the read-back value is the total number of
 * data items already read from the source peripheral, regardless of what
 * is the flow controller.
 */
#define  VTSS_F_BLOCK_TS(x)                   VTSS_ENCODE_BITFIELD(x,0,12)
#define   VTSS_F_BLOCK_TS_FPOS                 0
#define   VTSS_F_BLOCK_TS_FLEN                 12

/****** Group CH, Register SSTAT ***************************************
 * Source Status
 */
#define VTSS_CH0_SSTAT                       VTSS_REGISTER(FDMA,0x0020)
#define VTSS_CH1_SSTAT                       VTSS_REGISTER(FDMA,0x0078)
#define VTSS_CH2_SSTAT                       VTSS_REGISTER(FDMA,0x00d0)
#define VTSS_CH3_SSTAT                       VTSS_REGISTER(FDMA,0x0128)
#define VTSS_CH4_SSTAT                       VTSS_REGISTER(FDMA,0x0180)
#define VTSS_CH5_SSTAT                       VTSS_REGISTER(FDMA,0x01d8)
#define VTSS_CH6_SSTAT                       VTSS_REGISTER(FDMA,0x0230)
#define VTSS_CH7_SSTAT                       VTSS_REGISTER(FDMA,0x0288)

/****** Group CH, Register DSTAT ***************************************
 * Destination Status
 */
#define VTSS_CH0_DSTAT                       VTSS_REGISTER(FDMA,0x0028)
#define VTSS_CH1_DSTAT                       VTSS_REGISTER(FDMA,0x0080)
#define VTSS_CH2_DSTAT                       VTSS_REGISTER(FDMA,0x00d8)
#define VTSS_CH3_DSTAT                       VTSS_REGISTER(FDMA,0x0130)
#define VTSS_CH4_DSTAT                       VTSS_REGISTER(FDMA,0x0188)
#define VTSS_CH5_DSTAT                       VTSS_REGISTER(FDMA,0x01e0)
#define VTSS_CH6_DSTAT                       VTSS_REGISTER(FDMA,0x0238)
#define VTSS_CH7_DSTAT                       VTSS_REGISTER(FDMA,0x0290)

/****** Group CH, Register SSTATAR *************************************
 * Source Status Address Location
 */
#define VTSS_CH0_SSTATAR                     VTSS_REGISTER(FDMA,0x0030)
#define VTSS_CH1_SSTATAR                     VTSS_REGISTER(FDMA,0x0088)
#define VTSS_CH2_SSTATAR                     VTSS_REGISTER(FDMA,0x00e0)
#define VTSS_CH3_SSTATAR                     VTSS_REGISTER(FDMA,0x0138)
#define VTSS_CH4_SSTATAR                     VTSS_REGISTER(FDMA,0x0190)
#define VTSS_CH5_SSTATAR                     VTSS_REGISTER(FDMA,0x01e8)
#define VTSS_CH6_SSTATAR                     VTSS_REGISTER(FDMA,0x0240)
#define VTSS_CH7_SSTATAR                     VTSS_REGISTER(FDMA,0x0298)

/****** Group CH, Register DSTATAR *************************************
 * Destination Status Address Location
 */
#define VTSS_CH0_DSTATAR                     VTSS_REGISTER(FDMA,0x0038)
#define VTSS_CH1_DSTATAR                     VTSS_REGISTER(FDMA,0x0090)
#define VTSS_CH2_DSTATAR                     VTSS_REGISTER(FDMA,0x00e8)
#define VTSS_CH3_DSTATAR                     VTSS_REGISTER(FDMA,0x0140)
#define VTSS_CH4_DSTATAR                     VTSS_REGISTER(FDMA,0x0198)
#define VTSS_CH5_DSTATAR                     VTSS_REGISTER(FDMA,0x01f0)
#define VTSS_CH6_DSTATAR                     VTSS_REGISTER(FDMA,0x0248)
#define VTSS_CH7_DSTATAR                     VTSS_REGISTER(FDMA,0x02a0)

/****** Group CH, Register CFG0 ****************************************
 * DMA transfer configuration
 */
#define VTSS_CH0_CFG0                        VTSS_REGISTER(FDMA,0x0040)
#define VTSS_CH1_CFG0                        VTSS_REGISTER(FDMA,0x0098)
#define VTSS_CH2_CFG0                        VTSS_REGISTER(FDMA,0x00f0)
#define VTSS_CH3_CFG0                        VTSS_REGISTER(FDMA,0x0148)
#define VTSS_CH4_CFG0                        VTSS_REGISTER(FDMA,0x01a0)
#define VTSS_CH5_CFG0                        VTSS_REGISTER(FDMA,0x01f8)
#define VTSS_CH6_CFG0                        VTSS_REGISTER(FDMA,0x0250)
#define VTSS_CH7_CFG0                        VTSS_REGISTER(FDMA,0x02a8)

/* GP: Automatic Destination Reload. The DAR register can be automatically
 * reloaded from its initial value at the end of every block for
 * multi-block transfers. A new block transfer is then initiated. 
 * 
 * INJ / XTR : must be zero */
#define  VTSS_F_RELOAD_DST                    VTSS_BIT(31)

/* GP: Automatic Source Reload. The SAR register can be automatically
 * reloaded from its initial value at the end of every block for
 * multi-block transfers. A new block transfer is then initiated.
 * 
 * INJ / XTR : must be zero */
#define  VTSS_F_RELOAD_SRC                    VTSS_BIT(30)

/* Bus Lock Bit. When active, the AHB bus master signal hlock is asserted
 * for the duration specified in CFG.LOCK_B_L.  */
#define  VTSS_F_LOCK_B                        VTSS_BIT(17)

/* Channel Lock Bit. When the channel is granted control of the master bus
 * interface and if the CFG0.LOCK_CH bit is asserted, then no other
 * channels are granted control of the master bus interface for the
 * duration specified in CFG0.LOCK_CH_L. Indicates to the master bus
 * interface arbiter that this channel wants exclusive access to the master
 * bus interface for the duration specified in CFG0.LOCK_CH_L.
 */
#define  VTSS_F_LOCK_CH                       VTSS_BIT(16)

/* Bus Lock Level. Indicates the duration over which CFG0.LOCK_B bit
 * applies. 
 */
#define  VTSS_F_LOCK_B_L(x)                   VTSS_ENCODE_BITFIELD(x,14,2)
#define   VTSS_F_LOCK_B_L_FPOS                 14
#define   VTSS_F_LOCK_B_L_FLEN                 2

/* Channel Lock Level. Indicates the duration over which CFG0.LOCK_CH bit
 * applies.  */
#define  VTSS_F_LOCK_CH_L(x)                  VTSS_ENCODE_BITFIELD(x,12,2)
#define   VTSS_F_LOCK_CH_L_FPOS                12
#define   VTSS_F_LOCK_CH_L_FLEN                2

/* Source Software or Hardware Handshaking Select. 
 * 
 * INJ / GP :  Must be 1
 * 
 * XTR: Must be 0
 */
#define  VTSS_F_HS_SEL_SRC                    VTSS_BIT(11)

/* Destination Software or Hardware Handshaking Select. 
 * 
 * This register selects which of the handshaking interfaces - hardware or
 * software - is active for destination requests on this channel.
 * 
 * XTR / GP : Must be 1 */
#define  VTSS_F_HS_SEL_DST                    VTSS_BIT(10)

/* Indicates if there is data left in the channel FIFO. Can be used in
 * conjunction with CFG0.CH_SUSP to cleanly disable a channel.
 */
#define  VTSS_F_FIFO_EMPTY                    VTSS_BIT(9)

/* Channel Suspend. Suspends all DMA data transfers from the source until
 * this bit is cleared. There is no guarantee that the current transaction
 * will complete. Can also be used in conjunction with CFG0.FIFO_EMPTY to
 * cleanly disable a channel without losing any data. */
#define  VTSS_F_CH_SUSP                       VTSS_BIT(8)

/* Channel priority.  */
#define  VTSS_F_CH_PRIOR(x)                   VTSS_ENCODE_BITFIELD(x,5,3)
#define   VTSS_F_CH_PRIOR_FPOS                 5
#define   VTSS_F_CH_PRIOR_FLEN                 3

/****** Group CH, Register CFG1 ****************************************
 * DMA transfer configuration
 */
#define VTSS_CH0_CFG1                        VTSS_REGISTER(FDMA,0x0044)
#define VTSS_CH1_CFG1                        VTSS_REGISTER(FDMA,0x009c)
#define VTSS_CH2_CFG1                        VTSS_REGISTER(FDMA,0x00f4)
#define VTSS_CH3_CFG1                        VTSS_REGISTER(FDMA,0x014c)
#define VTSS_CH4_CFG1                        VTSS_REGISTER(FDMA,0x01a4)
#define VTSS_CH5_CFG1                        VTSS_REGISTER(FDMA,0x01fc)
#define VTSS_CH6_CFG1                        VTSS_REGISTER(FDMA,0x0254)
#define VTSS_CH7_CFG1                        VTSS_REGISTER(FDMA,0x02ac)

/* INJ: Destination Peripheral handshaking Interface. Valid if
 * CFG0.HS_SEL_DST field is 0; otherwise, this field is ignored. 
 * 
 * XTR/GP: Not used
 */
#define  VTSS_F_DST_PER(x)                    VTSS_ENCODE_BITFIELD(x,11,4)
#define   VTSS_F_DST_PER_FPOS                  11
#define   VTSS_F_DST_PER_FLEN                  4

/* XTR: Source Peripheral handshaking Interface. Valid if CFG0.HS_SEL_SRC
 * field is 0; otherwise, this field is ignored. 
 * 
 * INJ/GP: Not used */
#define  VTSS_F_SRC_PER(x)                    VTSS_ENCODE_BITFIELD(x,7,4)
#define   VTSS_F_SRC_PER_FPOS                  7
#define   VTSS_F_SRC_PER_FLEN                  4

/* Source Status Update Enable. 
 * 
 * GP: Source status information is fetched only from the location pointed
 * to by the SSTATAR register, stored in the SSTAT register and written out
 * to the DCB SSTAT  if SS_UPD_EN is high.
 * 
 * INJ / XTR : Must be zero */
#define  VTSS_F_SS_UPD_EN                     VTSS_BIT(6)

/* Destination Status Update Enable. 
 * 
 * GP: Destination status information is fetched only from the location
 * pointed to by the DSTATAR register, stored in the DSTAT register and
 * written out to the DCB DSTAT  if DS_UPD_EN is high.
 * 
 * INJ : Must be zero
 * 
 * XTR : Must be one
 */
#define  VTSS_F_DS_UPD_EN                     VTSS_BIT(5)

/* FIFO Mode Select. Determines how much space or data needs to be
 * available in the FIFO before a burst transaction request is serviced.
 */
#define  VTSS_F_FIFOMODE                      VTSS_BIT(1)

/* Flow Control Mode. 
 * 
 * GP : Determines when source transaction requests are serviced when the
 * Destination Peripheral is the flow controller.
 * 
 * INJ / XTR : Must be one
 */
#define  VTSS_F_FCMODE                        VTSS_BIT(0)

/***********************************************************************
 * Register group INTR
 *
 * DMA Interrupt Configuration
 */

#define VTSS_RB_FDMA_INTR                      VTSS_IOADDR(FDMA,0x02c0)

/****** Group INTR, Register RAW_TFR ***********************************
 * Raw Status for IntTfr Interrupt
 */
#define VTSS_INTR_RAW_TFR                    VTSS_REGISTER(FDMA,0x02c0)

/* Raw interrupt status. */
#define  VTSS_F_RAW_TFR(x)                    VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_RAW_TFR_FPOS                  0
#define   VTSS_F_RAW_TFR_FLEN                  8

/****** Group INTR, Register RAW_BLOCK *********************************
 * Raw Status for IntBlock Interrupt
 */
#define VTSS_INTR_RAW_BLOCK                  VTSS_REGISTER(FDMA,0x02c8)

/* Raw interrupt status. */
#define  VTSS_F_RAW_BLOCK(x)                  VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_RAW_BLOCK_FPOS                0
#define   VTSS_F_RAW_BLOCK_FLEN                8

/****** Group INTR, Register RAW_ERR ***********************************
 * Raw Status for IntErr Interrupt
 */
#define VTSS_INTR_RAW_ERR                    VTSS_REGISTER(FDMA,0x02e0)

/* Raw interrupt status. */
#define  VTSS_F_RAW_ERR(x)                    VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_RAW_ERR_FPOS                  0
#define   VTSS_F_RAW_ERR_FLEN                  8

/****** Group INTR, Register STATUS_TFR ********************************
 * Status for IntTfr Interrupt
 */
#define VTSS_INTR_STATUS_TFR                 VTSS_REGISTER(FDMA,0x02e8)

/* Interrupt status. */
#define  VTSS_F_STATUS_TFR(x)                 VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_STATUS_TFR_FPOS               0
#define   VTSS_F_STATUS_TFR_FLEN               8

/****** Group INTR, Register STATUS_BLOCK ******************************
 * Status for IntBlock Interrupt
 */
#define VTSS_INTR_STATUS_BLOCK               VTSS_REGISTER(FDMA,0x02f0)

/* Interrupt status. */
#define  VTSS_F_STATUS_BLOCK(x)               VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_STATUS_BLOCK_FPOS             0
#define   VTSS_F_STATUS_BLOCK_FLEN             8

/****** Group INTR, Register STATUS_ERR ********************************
 * Status for IntErr Interrupt
 */
#define VTSS_INTR_STATUS_ERR                 VTSS_REGISTER(FDMA,0x0308)

/* Interrupt status. */
#define  VTSS_F_STATUS_ERR(x)                 VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_STATUS_ERR_FPOS               0
#define   VTSS_F_STATUS_ERR_FLEN               8

/****** Group INTR, Register MASK_TFR **********************************
 * Mask for IntTfr Interrupt
 */
#define VTSS_INTR_MASK_TFR                   VTSS_REGISTER(FDMA,0x0310)

/* Interrupt Mask Write Enable
 * 0 = write disabled
 * 1 = write enabled
 * dnc = DMAH_NUM_CHANNELS
 * Reset Value: 0x0 */
#define  VTSS_F_INT_MASK_WE_TFR(x)            VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_INT_MASK_WE_TFR_FPOS          8
#define   VTSS_F_INT_MASK_WE_TFR_FLEN          8

/* Interrupt Mask
 * 0 = masked
 * 1 = unmasked
 * dnc = DMAH_NUM_CHANNELS
 * Reset Value: 0x0 */
#define  VTSS_F_INT_MASK_TFR(x)               VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_INT_MASK_TFR_FPOS             0
#define   VTSS_F_INT_MASK_TFR_FLEN             8

/****** Group INTR, Register MASK_BLOCK ********************************
 * Mask for IntBlock Interrupt
 */
#define VTSS_INTR_MASK_BLOCK                 VTSS_REGISTER(FDMA,0x0318)

/* Interrupt Mask Write Enable
 * 0 = write disabled
 * 1 = write enabled
 * dnc = DMAH_NUM_CHANNELS
 * Reset Value: 0x0 */
#define  VTSS_F_INT_MASK_WE_BLOCK(x)          VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_INT_MASK_WE_BLOCK_FPOS        8
#define   VTSS_F_INT_MASK_WE_BLOCK_FLEN        8

/* Interrupt Mask
 * 0 = masked
 * 1 = unmasked
 * dnc = DMAH_NUM_CHANNELS
 * Reset Value: 0x0 */
#define  VTSS_F_INT_MASK_BLOCK(x)             VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_INT_MASK_BLOCK_FPOS           0
#define   VTSS_F_INT_MASK_BLOCK_FLEN           8

/****** Group INTR, Register MASK_ERR **********************************
 * Mask for IntErr Interrupt
 */
#define VTSS_INTR_MASK_ERR                   VTSS_REGISTER(FDMA,0x0330)

/* Interrupt Mask Write Enable
 * 0 = write disabled
 * 1 = write enabled
 * dnc = DMAH_NUM_CHANNELS
 * Reset Value: 0x0 */
#define  VTSS_F_INT_MASK_WE_ERR(x)            VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_INT_MASK_WE_ERR_FPOS          8
#define   VTSS_F_INT_MASK_WE_ERR_FLEN          8

/* Interrupt Mask
 * 0 = masked
 * 1 = unmasked
 * dnc = DMAH_NUM_CHANNELS
 * Reset Value: 0x0 */
#define  VTSS_F_INT_MASK_ERR(x)               VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_INT_MASK_ERR_FPOS             0
#define   VTSS_F_INT_MASK_ERR_FLEN             8

/****** Group INTR, Register CLEAR_TFR *********************************
 * Clear for IntTfr Interrupt
 */
#define VTSS_INTR_CLEAR_TFR                  VTSS_REGISTER(FDMA,0x0338)

/* Interrupt clear.
 * 0 = no effect
 * 1 = clear interrupt */
#define  VTSS_F_CLEAR_TFR(x)                  VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_CLEAR_TFR_FPOS                0
#define   VTSS_F_CLEAR_TFR_FLEN                8

/****** Group INTR, Register CLEAR_BLOCK *******************************
 * Clear for IntBlock Interrupt
 */
#define VTSS_INTR_CLEAR_BLOCK                VTSS_REGISTER(FDMA,0x0340)

/* Interrupt clear.
 * 0 = no effect
 * 1 = clear interrupt */
#define  VTSS_F_CLEAR_BLOCK(x)                VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_CLEAR_BLOCK_FPOS              0
#define   VTSS_F_CLEAR_BLOCK_FLEN              8

/****** Group INTR, Register CLEAR_ERR *********************************
 * Clear for IntErr Interrupt
 */
#define VTSS_INTR_CLEAR_ERR                  VTSS_REGISTER(FDMA,0x0358)

/* Interrupt clear.
 * 0 = no effect
 * 1 = clear interrupt */
#define  VTSS_F_CLEAR_ERR(x)                  VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_CLEAR_ERR_FPOS                0
#define   VTSS_F_CLEAR_ERR_FLEN                8

/****** Group INTR, Register STATUSINT *********************************
 * Status for each interrupt type
 */
#define VTSS_INTR_STATUSINT                  VTSS_REGISTER(FDMA,0x0360)

/* OR of the contents of StatusErr register. */
#define  VTSS_F_ERR                           VTSS_BIT(4)

/* OR of the contents of StatusBlock register. */
#define  VTSS_F_BLOCK                         VTSS_BIT(1)

/* OR of the contents of StatusTfr register. */
#define  VTSS_F_TFR                           VTSS_BIT(0)

/***********************************************************************
 * Register group HS
 *
 * Handshaking Registers
 */

#define VTSS_RB_FDMA_HS                        VTSS_IOADDR(FDMA,0x0368)

/****** Group HS, Register REQ_SRC_REG *********************************
 * Source Software Transaction Request Register
 */
#define VTSS_HS_REQ_SRC_REG                  VTSS_REGISTER(FDMA,0x0368)

/* Source request write enable
 */
#define  VTSS_F_SRC_REQ_WE(x)                 VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_SRC_REQ_WE_FPOS               8
#define   VTSS_F_SRC_REQ_WE_FLEN               8

/* Source request */
#define  VTSS_F_SRC_REQ(x)                    VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_SRC_REQ_FPOS                  0
#define   VTSS_F_SRC_REQ_FLEN                  8

/****** Group HS, Register REQ_DST_REG *********************************
 * Destination Software Transaction Request Register
 */
#define VTSS_HS_REQ_DST_REG                  VTSS_REGISTER(FDMA,0x0370)

/* Destination request write enable
 */
#define  VTSS_F_DST_REQ_WE(x)                 VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_DST_REQ_WE_FPOS               8
#define   VTSS_F_DST_REQ_WE_FLEN               8

/* Destination request */
#define  VTSS_F_DST_REQ(x)                    VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_DST_REQ_FPOS                  0
#define   VTSS_F_DST_REQ_FLEN                  8

/****** Group HS, Register SGL_REQ_SRC_REG *****************************
 * Single Source Transaction Request Register
 */
#define VTSS_HS_SGL_REQ_SRC_REG              VTSS_REGISTER(FDMA,0x0378)

/* Single write enable
 */
#define  VTSS_F_SRC_SGLREQ_WE(x)              VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_SRC_SGLREQ_WE_FPOS            8
#define   VTSS_F_SRC_SGLREQ_WE_FLEN            8

/* Source single request */
#define  VTSS_F_SRC_SGLREQ(x)                 VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_SRC_SGLREQ_FPOS               0
#define   VTSS_F_SRC_SGLREQ_FLEN               8

/****** Group HS, Register SGL_REQ_DST_REG *****************************
 * Single Destination Transaction Request Register
 */
#define VTSS_HS_SGL_REQ_DST_REG              VTSS_REGISTER(FDMA,0x0380)

/* Destination write enable */
#define  VTSS_F_DST_SGLREQ_WE(x)              VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_DST_SGLREQ_WE_FPOS            8
#define   VTSS_F_DST_SGLREQ_WE_FLEN            8

/* Destination single or burst request */
#define  VTSS_F_DST_SGLREQ(x)                 VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_DST_SGLREQ_FPOS               0
#define   VTSS_F_DST_SGLREQ_FLEN               8

/****** Group HS, Register LST_SRC_REG *********************************
 * Last Source Transaction Request Register
 */
#define VTSS_HS_LST_SRC_REG                  VTSS_REGISTER(FDMA,0x0388)

/* Source last transaction request write enable */
#define  VTSS_F_LSTSRC_WE(x)                  VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_LSTSRC_WE_FPOS                8
#define   VTSS_F_LSTSRC_WE_FLEN                8

/* Source last transaction request
 * 0 = Not last transaction in current block
 * 1 = Last transaction in current block */
#define  VTSS_F_LSTSRC(x)                     VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_LSTSRC_FPOS                   0
#define   VTSS_F_LSTSRC_FLEN                   8

/****** Group HS, Register LST_DST_REG *********************************
 * Last Destination Transaction Request Register
 */
#define VTSS_HS_LST_DST_REG                  VTSS_REGISTER(FDMA,0x0390)

/* Destination last transaction request write enable */
#define  VTSS_F_LSTDST_WE(x)                  VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_LSTDST_WE_FPOS                8
#define   VTSS_F_LSTDST_WE_FLEN                8

/* Destination last transaction request
 * 0 = Not last transaction in current block
 * 1 = Last transaction in current block */
#define  VTSS_F_LSTDST(x)                     VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_LSTDST_FPOS                   0
#define   VTSS_F_LSTDST_FLEN                   8

/***********************************************************************
 * Register group MISC
 *
 * 
 */

#define VTSS_RB_FDMA_MISC                      VTSS_IOADDR(FDMA,0x0398)

/****** Group MISC, Register DMA_CFG_REG *******************************
 * DMA Enable
 */
#define VTSS_MISC_DMA_CFG_REG                VTSS_REGISTER(FDMA,0x0398)

/* DMA Enable bit.
 */
#define  VTSS_F_DMA_EN                        VTSS_BIT(0)

/****** Group MISC, Register CH_EN_REG *********************************
 * DMA Channel Enable 
 */
#define VTSS_MISC_CH_EN_REG                  VTSS_REGISTER(FDMA,0x03a0)

/* Channel enable write enable. */
#define  VTSS_F_CH_EN_WE(x)                   VTSS_ENCODE_BITFIELD(x,8,8)
#define   VTSS_F_CH_EN_WE_FPOS                 8
#define   VTSS_F_CH_EN_WE_FLEN                 8

/* Enables/Disables the channel. Setting this bit enables a channel;
 * clearing this bit disables the channel.
 * 
 * The bit is automatically cleared by hardware to disable the channel
 * after the last DMA transfer to the destination has completed. Software
 * can therefore poll this bit to determine when this channel is free for a
 * new DMA transfer. */
#define  VTSS_F_CH_EN(x)                      VTSS_ENCODE_BITFIELD(x,0,8)
#define   VTSS_F_CH_EN_FPOS                    0
#define   VTSS_F_CH_EN_FLEN                    8

#endif /* _VTSS_H_VCOREII_AMBA_TOP_ */
