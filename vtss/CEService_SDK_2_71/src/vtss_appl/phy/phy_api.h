/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

 $Id$
 $Revision$

*/

#ifndef _PHY_API_H_
#define _PHY_API_H_

#include "vtss_types.h"
#include "main.h"
#if defined(VTSS_CHIP_10G_PHY)
#include "vtss_phy_10g_api.h"
#endif

#undef VTSS_CHIP_10G_PHY_SAVE_FAILOVER_IN_CFG /* define it to enable saving of failover cfg in icfg */
#define VTSS_SW_ICLI_PHY_INVISIBLE /* undef 'VTSS_SW_ICLI_PHY_INVISIBLE' to make PHY's module icli cmds visible */

/* To control visibility of PHY config & interface mode icli cmds */
#ifdef VTSS_SW_ICLI_PHY_INVISIBLE
#define ICLI_PHY_NONE_SHOW_CMDS_PROPERTY ICLI_CMD_PROP_INVISIBLE
#else
#define ICLI_PHY_NONE_SHOW_CMDS_PROPERTY ICLI_CMD_PROP_VISIBLE
#endif

/* if VTSS_SW_OPTION_PHY does not exist then PHY_INST=NULL */
#define PHY_INST phy_mgmt_inst_get()

/* Initialize module */
vtss_rc phy_init(vtss_init_data_t *data);

/* Get the instance  */
vtss_inst_t phy_mgmt_inst_get(void);

typedef enum {
    PHY_INST_NONE,
    PHY_INST_1G_PHY,
    PHY_INST_10G_PHY,
} phy_inst_start_t;

/* get start instance */
phy_inst_start_t phy_mgmt_start_inst_get(void);

/* Create an Phy instance */
vtss_rc phy_mgmt_inst_create(phy_inst_start_t inst_create);

typedef enum {
    COOL,
    WARM,
} phy_inst_restart_t;

/* Restart the instance */
vtss_rc phy_mgmt_inst_restart(vtss_inst_t inst, phy_inst_restart_t restart);

#if defined(VTSS_CHIP_10G_PHY)
/* Activate the Default instance, i.e. make it possible to configure the PHYs through it */
vtss_rc phy_mgmt_inst_activate_default(void);

/* Set the failover mode used after next instance restart */
vtss_rc phy_mgmt_failover_set(vtss_port_no_t port_no, vtss_phy_10g_failover_mode_t *failover);
/* Get the current failover mode */
vtss_rc phy_mgmt_failover_get(vtss_port_no_t port_no, vtss_phy_10g_failover_mode_t *failover);
#endif /* VTSS_CHIP_10G_PHY */
#endif /* _PHY_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
