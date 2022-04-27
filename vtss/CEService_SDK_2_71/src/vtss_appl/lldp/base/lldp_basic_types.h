/*

 Vitesse Switch Software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
#ifndef LLDP_BASIC_TYPES_H
#define LLDP_BASIC_TYPES_H
#include "main.h"
#include "port_api.h"

typedef BOOL lldp_bool_t;
typedef ushort lldp_counter_t;
typedef ushort lldp_timer_t;
typedef char lldp_8_t;
typedef u8  lldp_u8_t;
typedef u16 lldp_u16_t;
typedef i16 lldp_16_t;
typedef u32 lldp_u32_t;
typedef i32 lldp_32_t;
typedef u64 lldp_u64_t;
typedef i64 lldp_64_t;

/* port number,  counting from 1..LLDP_PORTS */
typedef vtss_port_no_t lldp_port_t;

#define LLDP_PORTS VTSS_PORTS
#endif  //LLDP_BASIC_TYPES_H
