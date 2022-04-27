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

 $Id$
 $Revision$

*/

/**
 * \file
 * \brief mirror icli functions
 * \details This header file describes firmware iCLI
 */

#ifndef _VTSS_ICLI_FIRMWARE_H_
#define _VTSS_ICLI_FIRMWARE_H_

/**
 * \brief Function for printing firmware information.
 *
 * \param session_id [IN]  Needed for being able to print messages
 * \return None.
 **/
void firmware_icli_show_version(i32 session_id);

/*
 * \brief Function for swapping between firmware active and alternative images.
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \return None.
 **/
void firmware_icli_swap(i32 session_id);

/*
 * \brief Function for loading new firmware image.
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param tftpserver_path_file [IN]  IP address, path and file name for the tftp server containing the new image
 * \return None.
 **/
void firmware_icli_upgrade(i32 session_id, i8 *tftpserver_path_file);

#endif /* _VTSS_ICLI_FIRMWARE_H_ */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
