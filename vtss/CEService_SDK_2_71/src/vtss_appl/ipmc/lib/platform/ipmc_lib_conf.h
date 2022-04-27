/*

 Vitesse Switch API software.

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

#ifndef _IPMC_LIB_CONF_H_
#define _IPMC_LIB_CONF_H_

/* IPMC_LIB Configuration Version */
/* If IPMC_LIB_CONF_VERSION is changed, modify ipmc_lib_conf_transition */
#define IPMC_LIB_CONF_VERINIT   0
#define IPMC_LIB_CONF_VERSION   1

#define IPMC_LIB_CONF_SYNC()    if (!ipmc_lib_conf_sync()) T_D("CONF_SYNC Failure!")

typedef struct {
    ipmc_lib_conf_profile_t     profile;        /* IPMC Profile */
} ipmc_lib_configuration_t;

typedef struct {
    u32                         blk_version;    /* Block version */

    ipmc_lib_configuration_t    ipmc_lib_conf;  /* IPMC LIB Configuration */
} ipmc_lib_conf_blk_t;


BOOL ipmc_lib_conf_sync(void);

#endif /* _IPMC_LIB_CONF_H_ */
