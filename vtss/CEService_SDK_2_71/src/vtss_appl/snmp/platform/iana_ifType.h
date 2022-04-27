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

/*  This header file is based on IANAifType-MIB to define the interface type which is used in ifType  */
#ifndef IANA_IFTYPE_H
#define IANA_IFTYPE_H

#define IANA_IFTYPE_OTHER       0x1     /* none of the following */
#define IANA_IFTYPE_ETHER       0x6     /* Ethernet CSMACD */
#define IANA_IFTYPE_L2VLAN      0x87    /* Layer 2 Virtual LAN using 802.1Q */
#define IANA_IFTYPE_LAG         0xa1    /* IEEE 802.3ad Link Aggregate*/

#endif

