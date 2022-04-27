/*

 Vitesse Switch API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_MGMT_PORT_API_H_
#define _VTSS_MGMT_PORT_API_H_

#include "vtss_mgmt_api.h"

/* Port API error codes (vtss_rc) */
enum {
    PORT_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_PORT),  /* Generic error code */
    PORT_ERROR_INCOMPLETE,     /* VeriPHY still running */
    PORT_ERROR_PARM,           /* Illegal parameter */
    PORT_ERROR_REG_TABLE_FULL, /* Registration table full */
    PORT_ERROR_REQ_TIMEOUT,    /* Timeout on message request */
    PORT_ERROR_STACK_STATE,    /* Illegal MASTER/SLAVE state */
    PORT_ERROR_MUST_BE_MASTER  /* Not allowed at slave switch */
};

/* Port information */
typedef struct {
    BOOL              link;      /* Link status */
    vtss_port_speed_t speed;     /* Port speed */
    BOOL              stack;     /* Stack port */
    BOOL              phy;       /* PHY on port */
    vtss_port_no_t    chip_port; /* Chip port number */
    vtss_chip_no_t    chip_no;   /* Chip number */
    BOOL              fiber;     /* true when PHY port media is fiber */
    BOOL              fdx;       /* Full duplex */
} port_info_t;

/* Get port information */
vtss_rc port_info_get(vtss_port_no_t port_no, port_info_t *info);

#endif // _VTSS_MGMT_PORT_API_H_

// ***************************************************************************
// 
//  End of file.
// 
// ***************************************************************************
