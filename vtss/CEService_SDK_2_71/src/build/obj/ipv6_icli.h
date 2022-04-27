/*

 Vitesse Switch Application software.

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

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        This file is automatically generated from script file.
        Please modify the corresponding script file if needed.
        Modifying this file will -NOT- take effect.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

#ifndef ___IPV6_ICLI_H___
#define ___IPV6_ICLI_H___

//===== MODULE_IF_FLAG =======================================================
#if defined(VTSS_SW_OPTION_IP2) && defined(VTSS_SW_OPTION_IPV6)

/*
******************************************************************************

    < auto-generation >
    global API for command registration
    return value -
        ICLI_RC_OK    : successful
        ICLI_RC_ERROR : failed

******************************************************************************
*/
i32 ipv6_icli_cmd_register(void);

/*
******************************************************************************

    < auto-generation >
    global API for command ID retrieval
    return value -
        >= 0 : valid command ID
        < 0  : failed, cmd_seq is out of boundary or cmd not registered

******************************************************************************
*/
i32 ipv6_icli_cmd_id_get(
    IN i32 cmd_seq
);

//===== MODULE_IF_FLAG =======================================================
#endif // defined(VTSS_SW_OPTION_IP2) && defined(VTSS_SW_OPTION_IPV6)

#endif //___IPV6_ICLI_H___

