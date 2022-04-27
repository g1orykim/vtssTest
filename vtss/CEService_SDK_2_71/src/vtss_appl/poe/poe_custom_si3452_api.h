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
/************************************************************-*- mode: C -*-*/
/*                                                                          */
/*           Copyright (C) 2007 Vitesse Semiconductor Corporation           */
/*                           All Rights Reserved.                           */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*                            Copyright Notice:                             */
/*                                                                          */
/*  This document contains confidential and proprietary information.        */
/*  Reproduction or usage of this document, in part or whole, by any means, */
/*  electrical, mechanical, optical, chemical or otherwise is prohibited,   */
/*  without written permission from Vitesse Semiconductor Corporation.      */
/*                                                                          */
/*  The information contained herein is protected by Danish and             */
/*  international copyright laws.                                           */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*  <DESCRIBE FILE CONTENTS HERE>                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/


// Define how many ports each PoE chip controls.
#define PORTS_PER_POE_CHIP 4 //4 ports per chip - See Table 1 - SI3452 register map.



int si3452_is_chip_available(vtss_port_no_t port_index);
void si3452_poe_enable(vtss_port_no_t port_index, BOOL enable);
void si3452_poe_init(vtss_port_no_t port_index);
void si3452_get_port_measurement(poe_status_t *poe_status, vtss_port_no_t port_index);
void si3452_port_status_get(poe_port_status_t *port_status, vtss_port_no_t port_index);
void si3452_get_all_port_class(char *classes, vtss_port_no_t port_index);
void si3452_set_power_limit_channel(vtss_port_no_t port_index, int max_port_power) ;
void si3452_device_wr(vtss_port_no_t port_index, uchar reg_addr, char data);
void si3452_device_rd(vtss_port_no_t port_index, uchar reg_addr, uchar *data);
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
