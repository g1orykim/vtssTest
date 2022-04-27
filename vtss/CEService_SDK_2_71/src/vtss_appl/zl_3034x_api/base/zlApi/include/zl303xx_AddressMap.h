

/*******************************************************************************
*
*  $Id: zl303xx_AddressMap.h 7546 2012-01-31 18:38:58Z JK $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Values and constants for the device internal address map
*
*******************************************************************************/

#ifndef ZL303XX_ADDRESS_MAP_H_
#define ZL303XX_ADDRESS_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"

/*****************   DEFINES   ************************************************/

/* A virtual register address consists of the following bits:
   [31:20]  Reserved for application use
   [19:18]  Encoding for the overlay registers
               00b = Device hardware register (no overlay)
               01b = Reserved (Potentially for software register mappings)
               10b = Bridge configuration overlay
               11b = Bridge memory register overlay
   [17:12]  Register size
               - range of 0-63 represents 1-64 bytes
   [11:8]   Register address extension
               - non-overlay addresses: page extension
               - overlay addresses: address extension
   [7:0]    Register final address
*/

/* For numeric, zero based parts of the address there is a mask, allowing the
   relevant bits to be isolated and a shift right factor which will locate the
   extracted bits into the LSB */

/*******  DEVICE  **********/

/* The following define gives a mask to identify register accesses */
#define ZL303XX_MEM_DEVICE_MASK       (Uint32T)0xFFF00000
#define ZL303XX_MEM_DEVICE_SHIFT      (Uint16T)20

/* Macro to extract the encoded DEVICE info from a virtual address */
#define ZL303XX_MEM_DEVICE_EXTRACT(addr)     \
      (Uint16T)(((addr) & ZL303XX_MEM_DEVICE_MASK) >> ZL303XX_MEM_DEVICE_SHIFT)

/* Macro to encode the DEVICE info for insertion into a virtual address */
#define ZL303XX_MEM_DEVICE_INSERT(device)     \
      (Uint32T)(((Uint32T)(device) << ZL303XX_MEM_DEVICE_SHIFT)  \
                                                   & ZL303XX_MEM_DEVICE_MASK)

/*******  OVERLAY  *********/

/* The overlay bits indicate the area of memory to which the virtual address
   is associated. (No shift is required). */
#define ZL303XX_MEM_OVRLY_MASK              (Uint32T)0x000C0000

/* Overlay definitions */
#define ZL303XX_MEM_OVRLY_NONE              (Uint32T)0x00000000
#define ZL303XX_MEM_OVRLY_RESERVED          (Uint32T)0x00040000
#define ZL303XX_MEM_OVRLY_BRIDGE_CONFIG     (Uint32T)0x00080000
#define ZL303XX_MEM_OVRLY_BRIDGE_MEMORY     (Uint32T)0x000C0000

/* This KEY is a faster way of determining a BRIDGE/Overlay address.  */
#define ZL303XX_MEM_OVRLY_BRIDGE_KEY        (Uint32T)0x00080000

/* Macro to extract the encoded OVERLAY CODE from a virtual address */
#define ZL303XX_MEM_OVRLY_EXTRACT(addr)     (Uint32T)((addr) & ZL303XX_MEM_OVRLY_MASK)

/* Macro to encode the OVERLAY CODE for insertion into a virtual address */
#define ZL303XX_MEM_OVRLY_INSERT(ovrly)     (Uint32T)((ovrly) & ZL303XX_MEM_OVRLY_MASK)


/******* BYTE SIZE *********/

/* Mask to cover all possible size bits:
   - 6 bits allows a range of 0-63 (corresponds to size of 1-64 bytes) */
#define ZL303XX_MEM_SIZE_MASK         (Uint32T)0x0003F000
#define ZL303XX_MEM_SIZE_SHIFT        (Uint16T)12

/* Macro to extract the encoded SIZE (in bytes) from a virtual address */
#define ZL303XX_MEM_SIZE_EXTRACT(addr)   \
      (Uint8T)(((addr & ZL303XX_MEM_SIZE_MASK) >> ZL303XX_MEM_SIZE_SHIFT) + 1)

/* Macro to encode the register SIZE for insertion into a virtual address */
#define ZL303XX_MEM_SIZE_INSERT(bytes)       \
      (Uint32T)((((Uint32T)(bytes) - 1) << ZL303XX_MEM_SIZE_SHIFT)  \
                                                      & ZL303XX_MEM_SIZE_MASK)

/* The most common values for the size of the virtual register */
#define ZL303XX_MEM_SIZE_1_BYTE       (Uint32T)(1)
#define ZL303XX_MEM_SIZE_2_BYTE       (Uint32T)(2)
#define ZL303XX_MEM_SIZE_4_BYTE       (Uint32T)(4)
#define ZL303XX_MEM_SIZE_8_BYTE       (Uint32T)(8)


/*******  ADDRESS  *********/

/* The Address (and any extension) */

/* ADDRESS value */
/*****************/

/* Overlay bridge addresses have 12-bit addresses, whereas direct addresses
   (direct to the device) only have 7-bit addresses. Both are offset at the
   0-bit position so we do not need to do any shifting. */

/* OVERLAY address mask, extract & insert definitions */
#define ZL303XX_MEM_OVRLY_ADDR_MASK            (Uint32T)0x00000FFF

#define ZL303XX_MEM_OVRLY_ADDR_EXTRACT(addr)   \
      (Uint16T)((addr) & ZL303XX_MEM_OVRLY_ADDR_MASK)

#define ZL303XX_MEM_OVRLY_ADDR_INSERT(addr)    \
      (Uint32T)((Uint32T)(addr) & ZL303XX_MEM_OVRLY_ADDR_MASK)

/* OVERLAY address mask, extract & insert definitions */
#define ZL303XX_MEM_DIRECT_ADDR_MASK           (Uint32T)0x0000007F

#define ZL303XX_MEM_DIRECT_ADDR_EXTRACT(addr)   \
      (Uint8T)((addr) & ZL303XX_MEM_DIRECT_ADDR_MASK)

#define ZL303XX_MEM_DIRECT_ADDR_INSERT(addr)    \
      (Uint32T)((Uint32T)(addr) & ZL303XX_MEM_DIRECT_ADDR_MASK)

/* Macro to get the destination address from a given virtual register address.
   For OVERLAY virtual addresses, we return all address bits of the address.
   For DIRECT virtual addresses, bits 7 up to 11 should already be 0. */
#define ZL303XX_MEM_ADDR_EXTRACT(addr)                           \
      (Uint16T)(((addr) & ZL303XX_MEM_OVRLY_BRIDGE_KEY) ?        \
                  (ZL303XX_MEM_OVRLY_ADDR_EXTRACT(addr)) :       \
                  (ZL303XX_MEM_DIRECT_ADDR_EXTRACT(addr)))

/* Likewise for INSERTing the address (Overlay key is needed) */
#define ZL303XX_MEM_ADDR_INSERT(addr,ovrly)                            \
      (Uint32T)(((Uint32T)(ovrly) & ZL303XX_MEM_OVRLY_BRIDGE_KEY) ?    \
                  (ZL303XX_MEM_OVRLY_ADDR_INSERT(addr)) :              \
                  (ZL303XX_MEM_DIRECT_ADDR_INSERT(addr)))


/* PAGE extension */
/******************/

/* The page bits apply only to direct access registers. Overlay addresses do
   not have Pages associated with them, instead the Page bits are a part of the
   overlay address extension. */
#define ZL303XX_MEM_PAGE_MASK            (Uint32T)0x00000F00
#define ZL303XX_MEM_PAGE_SHIFT           (Uint16T)8

/* Macro to extract the encoded PAGE value from a virtual address */
#define ZL303XX_MEM_PAGE_EXTRACT(addr)   \
      (Uint8T)(((addr) & ZL303XX_MEM_PAGE_MASK) >> ZL303XX_MEM_PAGE_SHIFT)

/* Macro to encode the PAGE value for insertion into a virtual address */
#define ZL303XX_MEM_PAGE_INSERT(page)       \
      (Uint32T)(((Uint32T)(page) << ZL303XX_MEM_PAGE_SHIFT) & ZL303XX_MEM_PAGE_MASK)

/* Direct, Paged addresses are within the following range */
#define ZL303XX_PAGED_ADDR_MIN     (Uint32T)0x65
#define ZL303XX_PAGED_ADDR_MAX     (Uint32T)0x7F

/* Direct addresses with offsets > 0x64 require the page index be properly set.
   If this is an overlay virtual address, we do not need to check the page. */
#define ZL303XX_MEM_CHECK_PAGE_REG(addr)                                        \
      ( (((addr) & ZL303XX_MEM_OVRLY_BRIDGE_KEY) ||                             \
         (((addr) & ZL303XX_MEM_DIRECT_ADDR_MASK) < ZL303XX_PAGED_ADDR_MIN)) ?  \
            (ZL303XX_FALSE) :                                                   \
            (ZL303XX_TRUE))


/*****************   REGISTER ADDRESS CONSTRUCT   *****************************/

/* Construct a memory address for a register on a specific device.
   The device field is not filled in by this macro */
#define ZL303XX_MAKE_MEM_ADDR(page,addr,size,ovrly)        \
      (Uint32T)(ZL303XX_MEM_PAGE_INSERT(page) |            \
                ZL303XX_MEM_ADDR_INSERT(addr,ovrly) |      \
                ZL303XX_MEM_SIZE_INSERT(size) |            \
                ZL303XX_MEM_OVRLY_INSERT(ovrly))

/*****************   REGISTER ADDRESSES   *************************************/

/* The Page register is often used so create a definition for it. */
#define ZL303XX_ADDR_PAGE_REG      ZL303XX_MAKE_MEM_ADDR(0x00, 0x64, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)

/* The addresses for the top level interrupt registers in the device */
#define ZL303XX_TOP_ISR_REG        ZL303XX_MAKE_MEM_ADDR(0x0F, 0x7D, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)

/* The mask registers for physical interrupts 0 and 1 */
#define ZL303XX_TOP_ISR0_MASK_REG  ZL303XX_MAKE_MEM_ADDR(0x0F, 0x7E, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TOP_ISR1_MASK_REG  ZL303XX_MAKE_MEM_ADDR(0x0F, 0x7F, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/


#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

