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

#include "icli_api.h"

/**
 * \file
 * \brief port security icli functions
 * \details This header file describes port security icli functions
 */

/**
 * \brief Function for re-open an interface that has been port security'ed shut down
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to print statistics for.
 * \return Error code.
 **/
vtss_rc psec_limit_icli_no_shutdown(i32 session_id, icli_stack_port_range_t *plist);

/**
 * \brief Function for enable/disable port security for a specific interface
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to print statistics for.
 * \param no    [IN] TRUE to set to default.
 * \return Error code.
 **/
vtss_rc psec_limit_icli_enable(i32 session_id, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for setting the maximum number of addresses
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to print statistics for.
 * \param v_1_to_1024 [IN] Max number of addresses
 * \param no    [IN] TRUE to set to default.
 * \return Error code.
 **/
vtss_rc psec_limit_icli_maximum(i32 session_id, icli_stack_port_range_t *plist, u32 v_1_to_1024, BOOL no);

/**
 * \brief Function for setting what to do in case of port security violation
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to print statistics for.
 * \param has_protect [IN] TRUE to set action to protect
 * \param has_trap [IN] TRUE to set action to trap
 * \param has_trap_shut [IN] TRUE to set action to trap_shut
 * \param has_shutdown [IN] TRUE to set action to shutdown
 * \param no    [IN] TRUE to set to default.
 * \return Error code.
 **/
vtss_rc psec_limit_icli_violation(i32 session_id, BOOL has_protect, BOOL has_trap, BOOL has_trap_shut, BOOL has_shutdown, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for setting the aging time
 *
 * \param session_id [IN]  Needed for being able to print
 * \param value [IN] New aging time
 * \param no    [IN] TRUE to set to default.
 * \return Error code.
 **/
vtss_rc psec_limit_icli_aging_time(i32 session_id, u32 value, BOOL no);

/**
 * \brief Function for enabling aging
 *
 * \param session_id [IN]  Needed for being able to print
 * \param no    [IN] TRUE to set to default.
 * \return Error code.
 **/
vtss_rc psec_limit_icli_aging(i32 session_id, BOOL no);

/**
 * \brief Function for enabling port security globally
 *
 * \param session_id [IN]  Needed for being able to print
 * \param no    [IN] TRUE to set to default.
 * \return Error code.
 **/
vtss_rc psec_limit_icli_enable_global(i32 session_id, BOOL no);

/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc psec_limit_icfg_init(void);


