

/******************************************************************************
*
*  $Id: zl303xx_PortingFunctions.h 8488 2012-06-21 22:56:57Z SW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     The zl303xx API relies on various functions that need to be provided by
*     the 'host' operating system.  In order to make porting the API to a different
*     'host' easier to achieve an abstraction layer has been used to provide
*     the required functionality.
*
******************************************************************************/

#ifndef _ZL303XX_PORTING_FUNCTIONS_H_
#define _ZL303XX_PORTING_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx.h"
#include "zl303xx_Porting.h"
/*****************   DEFINES     **********************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/**

  Function Name:    osSetIpAddr

  Details:  Sets the IP address for the CPU for driving the zl303xx hardware

  Parameters:
   [in]  ifName        Name of the network interface.
   [in]  srcIpAddress  The source IP address to use in IPv4 dot format,
                            e.g. "10.0.0.5".
   [in]  subnetMask    The subnet mask to use in IPv4 dot format,
                            e.g., "255.255.255.0".

*******************************************************************************/
void osSetIpAddr(const char *ifName, const char *srcIpAddress, const char *subnetMask);

/**

  Function Name:    osAddIpAddr

  Details:  Adds an IP address to a specified network interface.

  Parameters:
   [in]  ifName      Name of the network interface.
   [in]  srcIp       The source IP address to use in IPv4 dot format,
                          e.g. "10.0.0.5".
   [in]  subnetMask  The subnet mask to use in IPv4 dot format,
                          e.g., "255.255.255.0".

  Return Value:   N/A

*******************************************************************************/
void osAddIpAddr(const char *ifName, const char *srcIp, const char *subnetMask);

/**

  Function Name:
   osAddGateway

  Details:
   Adds a gateway address for the CPU for driving the zl303xx hardware

  Parameters:
       srcIP    The source/sub-net IP address to route: in IPv4 format ("10.9.8.5")
        gwIP     The gateway IP address to route to: in IPv4 format ("10.9.5.5")
        netMask  The network mask to apply to the route: in net format (0xffff0000)

  Return Value:
   Nothing

  Notes:
   None

*******************************************************************************/
void osAddGateway(char *srcIpAddr, char *gwIpAddr, Uint32T netMask);

Sint32T osGetMacAddr(const char *ifName, Uint8T *macAddr);
Sint32T osSetIfPromiscuous(const char *ifName, zl303xx_BooleanE isPromiscuous);

#ifdef ZL_INCLUDE_IPV6_SOCKET
/* osSetIpv6Addr */
/**
   Sets the IPv6 address for a board interface that is driving the zl303xx
   hardware. Calling this function removes all non-link-local IPv6 addresses
   from the interface.

  Parameters:
   [in]   ifName     Name of the network interface.
   [in]   addr       The source IPV6 address to use as a string.

  Return Value:
   Nothing

  Notes:
   This function will only set the interface when it is run with root privileges.
   Also the function relies on the driver supporting the functionality.
   If the interface has already been configured, this function may fail.
   This function has a high degree of OS and hardware dependence.

*******************************************************************************/
void osSetIpv6Addr(const char *ifName, const char *addr);

/* osAddIpv6Addr */
/**
   Adds the IPv6 address for a board interface that is driving the zl303xx
   hardware.

  Parameters:
   [in]   ifName     Name of the network interface.
   [in]   addr       The source IPV6 address to use as a string.

  Return Value:
   Nothing

  Notes:
   This function will only set the interface when it is run with root privileges.
   Also the function relies on the driver supporting the functionality.
   If the interface has already been configured, this function may fail.
   This function has a high degree of OS and hardware dependence.

*******************************************************************************/
void osAddIpv6Addr(const char *ifName, const char *addr);
#endif

/*******************************************************************************

  Function Name:   cpuConfigInterrupts

  Details: Performs any configuration necessary on the CPU to allow external
   interrupts to be routed to the interrupt controller.

  Return Value:
   cpuStatusE

*******************************************************************************/

cpuStatusE cpuConfigInterrupts(void);

/*******************************************************************************

  Function Name:   zl303xx_MaskInterruptFn

  Details:   Masks or unmasks the specified interrupt from the device. The method of doing
   this is system and/or processor specific.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device specific parameters in case it is required
                  for identifying the specific device.
    [in]    cpuIntNum      CPU interrupt number to mask off.
    [in]    deviceIntNum   Device interrupt number to mask off. Values 0 or 1.
    [in]    bMasking       ZL303XX_TRUE if the interrupt is to be masked. ZL303XX_FALSE to
                  unmask the interrupt

*******************************************************************************/

void zl303xx_MaskInterruptFn
(
        zl303xx_ParamsS * zl303xx_Params,
        Uint8T deviceIntNum,
        Uint8T cpuIntNum,
        zl303xx_BooleanE bMasking
);

/*******************************************************************************

  Function Name:   cpuConfigSpi

  Details:   Initialises the SPI interface (includes related structures and interrupts).

  Return Value:
   cpuStatusE

*******************************************************************************/

cpuStatusE cpuConfigSpi( void );

/*******************************************************************************

  Function Name:   cpuConfigChipSelect

  Details:   Performs any configuration necessary on the CPU to handle the SPI
   chip select for the ZL303XX_ device.

  Parameters:
   [in]   zl303xx_Params   Pointer to the device specific parameters which include the
                  Spi chip select structure.
    [in]   csPin          Chip select mask.

  Return Value:
   cpuStatusE
*******************************************************************************/

cpuStatusE cpuConfigChipSelect
(
        zl303xx_ParamsS * zl303xx_Params,
        Uint32T csPin
);

/*******************************************************************************

  Function Name:   cpuSoftwareResetFn

  Details:   Forces the ZL303XX_ reset pin low and then re-asserts it high.

*******************************************************************************/

cpuStatusE cpuSoftwareResetFn( void *hwParams );

/*******************************************************************************

  Function Name:   cpuConfigSoftwareReset

  Details:   Performs any configuration necessary on the CPU to handle a software
   reset of the ZL303XX_ device.

  Return Value:
   cpuStatusE
*******************************************************************************/

cpuStatusE cpuConfigSoftwareReset( void *hwParams );

/*******************************************************************************

  Function Name:   cpuSpiIsrHandler

  Details:   When an SPI packet is sent or received, an interrupt is generated in the
   <blank> register.  The corresponding ISR clears the interrupt and gives a
   Semaphore so any task waiting on a Rx or Tx can continue.
*******************************************************************************/

void cpuSpiIsrHandler(void);

/*******************************************************************************

  Function Name:  cpuSpiCleanUp

  Details:  Clean up the SPI interface (includes related structures and interrupts).

  Parameters:
   [in]   zl303xx_Params   Pointer to the device specific parameters which include the
                  software reset structure.

  Return Value:
   cpuStatusE

*******************************************************************************/

cpuStatusE cpuSpiCleanUp( void);

/*******************************************************************************

  Function Name:   cpuSpiRead

  Details:   Sends a read request to a Device.

  Parameters:
   [in]  par      device structure pointer that is unused in this function
    [in]  regAddr  direct SPI address to read from
    [in]  bufLen   number of bytes to read from the device
    [in]  dataBuf  the data read from the register (if successful).

  Return Value:
   cpuStatusE

*******************************************************************************/

cpuStatusE cpuSpiRead
(
        void *par,
        Uint32T regAddr,
        Uint8T *dataBuf,
        Uint16T bufLen
);

/*******************************************************************************

  Function Name:   cpuSpiWrite

  Details:   Starting at the given address, a block of data (of a given length) is written
   to indicated device.

  Parameters:
   [in]  par      device structure pointer that is unused in this function
    [in]  regAddr  SPI address to write
    [in]  data     the data to write to the requested register.

  Return Value:
   cpuStatusE
*******************************************************************************/
cpuStatusE cpuSpiWrite
(
        void *par,
        Uint32T regAddr,
        Uint8T *dataBuf,
        Uint16T bufLen
);


#ifdef __cplusplus
}
#endif

#endif

