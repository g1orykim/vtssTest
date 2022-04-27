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

/****************************************************************************
 * Containing iCLI functions shared by LLDP and LLDP-MED
 ****************************************************************************/
#ifdef VTSS_SW_OPTION_LLDP

#include "lldp_remote.h" // For lldp_remote_entry_t
#include "icli_api.h" // For icli_port_info_txt
#include "icli_porting_util.h" // For icli_port_info_txt

// See lldp_icli_shared_functions.h
i8 *lldp_local_interface_txt_get(i8 *buf, const lldp_remote_entry_t *entry, const switch_iter_t *sit, const port_iter_t *pit)
{
    // This function takes an iport and prints a uport or a LAG, GLAG
    if (lldp_remote_receive_port_to_string(entry->receive_port, buf, sit->isid)) {
        // Part of LAG
        return buf;
    } else {
        // Ok, this was a port. That we prints as an interface
        return icli_port_info_txt(sit->usid, pit->uport, buf);
    }
}
#endif // #ifdef VTSS_SW_OPTION_LLDP
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
