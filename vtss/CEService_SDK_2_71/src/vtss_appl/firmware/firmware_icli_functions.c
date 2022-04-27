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
#ifdef VTSS_SW_OPTION_FIRMWARE

#include "icli_api.h"
#include "icli_porting_util.h"
#include "firmware_api.h"
#include "main.h"
#include "firmware.h"
#include "flash_mgmt_api.h" // For flash_mgmt_fis_lookup
#include <arpa/inet.h> // For inet_aton
#include "misc_api.h" /* For misc_ipv4_txt and misc_url_XXX() */
#include "icfg_api.h"
#include "network.h"
#include "control_api.h" /* For vtss_restart_t */


/***************************************************************************/
/*  Code start :)                                                           */
/****************************************************************************/
// Help function for loading firmware (IPv4)
// In : session_id - For printing error messages
//      tftp_server - IPv4 address of the TFTP server
//      file        - Pointer to string containing the path and file.
//      restart_type - For future use (warm restart)
static void cli_cmd_load_firmware(i32 session_id, i8 *tftp_server, u8 *file_name, vtss_restart_t restart_type)
{
  u8 server[sizeof "000.000.000.000"];
  vtss_ipv4_t ipv4_server;
  struct sockaddr_in host;
  struct hostent *hp;

  // Set up host address
  host.sin_family = AF_INET;
  host.sin_len = sizeof(host);
  if (!inet_aton(tftp_server, &host.sin_addr)) {
    hp = gethostbyname(tftp_server);
    if (hp == NULL) {
      CPRINTF("*** Invalid IP address: %s\n", tftp_server);
      return;
    } else {
      memmove(&host.sin_addr, hp->h_addr, hp->h_length);
    }
  }
  ipv4_server = htonl(host.sin_addr.s_addr);
  (void) misc_ipv4_txt(ipv4_server, (char *) server);
  {
    // Managed - just do the TFTP & flash
    char *buffer;
    size_t length;
    cli_iolayer_t *pIO = cli_get_io_handle();

    if ((buffer = firmware_tftp(pIO, server, file_name, (unsigned char *)"managed", &length))) {
      vtss_rc result;
      // This only returns if an error occurred
      if ((result = firmware_update(pIO, (unsigned char *) buffer, length, restart_type ? VTSS_RESTART_COOL : VTSS_RESTART_WARM)) != VTSS_OK) {
        ICLI_PRINTF("Error: %s\n", error_txt(result));
      }
    }
  }
}

// Help function for printing the image information.
// In : session_id - For printing error messages
static void display_props(i32 session_id, char *type, const struct fis_table_entry *fis)
{
  uchar vers[256], date[256];
  firmware_scan_props(fis, vers, sizeof(vers), date, sizeof(date));

  icli_parm_header(session_id, type);
  ICLI_PRINTF("%-17s: %s\n", "Image", fis->name);
  ICLI_PRINTF("%-17s: %s\n", "Version", vers);
  ICLI_PRINTF("%-17s: %s\n", "Date", date);
}

//  see firmware_icli_functions.h
void firmware_icli_show_version(i32 session_id)
{
  struct fis_table_entry fis;
  const char *pri_name = "managed", *alt_name = "managed.bk";
  i8 active_image[sizeof(fis.name)];
  const char *code_rev = misc_software_code_revision_txt();

  (void) CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_LOADED_ENTRY, 0, &fis);
  strcpy(active_image, (char *) fis.name);

  if (flash_mgmt_fis_lookup(active_image, &fis) >= 0) {
    display_props(session_id, "Active Image", &fis);

    if (strlen(code_rev)) {
      // version.c is always compiled, this file is not, so we must
      // check for whether there's something in the code revision
      // string or not. Only version.c knows about the CODE_REVISION
      // environment variable.
      ICLI_PRINTF("%-17s: %s\n", "Code Revision", code_rev);
    }
  } else {
    ICLI_PRINTF("Error: unable to lookup active image \"%s\"\n", active_image);
  }

  if (strcmp((i8 *)fis.name, pri_name) == 0 &&
      flash_mgmt_fis_lookup(alt_name, &fis) >= 0) {
    ICLI_PRINTF("\n");
    display_props(session_id, "Alternate Image", &fis);
  }
}

#if VTSS_SWITCH_STANDALONE
//  see firmware_icli_functions.h
void firmware_icli_swap(i32 session_id)
{
  struct fis_table_entry fis;
  (void) CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_LOADED_ENTRY, 0, &fis);
  if (strcmp((i8 *)fis.name, "managed") == 0) {
    if (firmware_swap_entries() == 0) {
      ICLI_PRINTF("Alternate image activated, now rebooting.\n");
    } else {
      ICLI_PRINTF("Alternate image activation failed.\n");
    }
  } else {
    ICLI_PRINTF("Active image is %s, alternate image activation is disabled\n", fis.name);
  }
}
#endif

//  see firmware_icli_functions.h
void firmware_icli_upgrade(i32 session_id, i8 *tftpserver_path_file)
{
  misc_url_parts_t url_parts;

  // Just making sure that we don't get a NULL pointer
  if (tftpserver_path_file == NULL) {
    ICLI_PRINTF("%% Invalid TFTP path - Expecting something like tftp://10.10.10.10/path/new_image.dat\n");
    return;
  }

  if (misc_url_decompose(tftpserver_path_file, &url_parts) && (strcmp(url_parts.protocol, "tftp") == 0)) { // So far we only support tftp (and not file system)
    cli_cmd_load_firmware(session_id, &url_parts.host[0], (u8 *)&url_parts.path[0], VTSS_RESTART_COLD);
  } else {
    T_I("url_parts.protocol:%s", url_parts.protocol);
    ICLI_PRINTF("%% %s is an invalid TFTP path - Expecting something like tftp://10.10.10.10/path/new_image.dat\n", tftpserver_path_file);
  }
}
#endif // #ifdef VTSS_SW_OPTION_FIRMWARE
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
