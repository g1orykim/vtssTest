/*

 Vitesse Switch API software.

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

 $Id$
 $Revision$

*/

#ifndef _VTSS_CLI_GRP_HELP_H
#define _VTSS_CLI_GRP_HELP_H

#ifdef VTSS_CLI_SEC_GRP
#define VTSS_CLI_GRP_SEC          "Security"
#define VTSS_CLI_GRP_SEC_SWITCH   "Switch"
#define VTSS_CLI_GRP_SEC_NETWORK  "Network"

#define VTSS_CLI_GRP_SEC_PATH           "Security "
#define VTSS_CLI_GRP_SEC_SWITCH_PATH     VTSS_CLI_GRP_SEC_PATH "Switch "
#define VTSS_CLI_GRP_SEC_NETWORK_PATH    VTSS_CLI_GRP_SEC_PATH "Network "
#define VTSS_CLI_GRP_SEC_AAA_PATH        VTSS_CLI_GRP_SEC_PATH "AAA "
#else
#define VTSS_CLI_GRP_SEC_SWITCH_PATH     ""
#define VTSS_CLI_GRP_SEC_NETWORK_PATH    ""
#define VTSS_CLI_GRP_SEC_AAA_PATH        ""
#endif /* VTSS_CLI_SEC_GRP */

void cli_cmd_grp_disp(void);
#ifdef VTSS_CLI_SEC_GRP
void cli_cmd_sec_grp_disp(void);
void cli_cmd_sec_switch_grp_disp(void);
void cli_cmd_sec_network_grp_disp(void);
#endif

#endif /* _VTSS_CLI_GRP_HELP_H */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
