/* Switch API software.

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

 $Id$
 $Revision$
*/

/**
 * \file
 * \brief Vlan_Translation icli functions
 * \details This header file describes vlan_translation control functions
 */

#ifndef VTSS_ICLI_VLAN_TRANSLATION_H
#define VTSS_ICLI_VLAN_TRANSLATION_H

#include "icli_api.h"

/**
 * \brief Function for mapping VLANs to a translation VLAN
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param group [IN]  Group id containing the VLAN translation
 * \param vlan_list [IN] List of VLANs to be translated
 * \param translation_vlan [IN]  VLAN to translate to
 * \param no [IN]  TRUE to remove a translation
 * \return Error code.
 **/
vtss_rc vlan_translation_icli_map(i32 session_id, u32 group, icli_unsigned_range_t *vlan_list, u32 translation_vlan, BOOL no);

/**
 * \brief Function for mapping a interface to a VLAN translation group
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param group [IN]  List of groups to map the interface to
 * \param plsit [IN]  Containing port information
 * \param no [IN]  TRUE to remove the group from the interface
 * \return Error code.
 **/
vtss_rc vlan_translation_icli_interface_map(i32 session_id, u8 group_id, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc vlan_translation_icfg_init(void);

/**
 * \brief Function for at runtime getting information about how many groups that is supported
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param ask [IN]  Asking
 * \param runtime [IN]  Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL vlan_translation_icli_runtime_groups(u32                session_id,
                                          icli_runtime_ask_t ask,
                                          icli_runtime_t     *runtime);

#endif /* VTSS_ICLI_VLAN_TRANSLATION_H */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
