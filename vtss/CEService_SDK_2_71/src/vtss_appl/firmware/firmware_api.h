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

#ifndef _FIRMWARE_API_H_
#define _FIRMWARE_API_H_

#include "cli_api.h"
#include "ip2_api.h"
#include "simage_api.h"         /* Snap-in */
#include "control_api.h"        /* For vtss_restart_t */
#include <cyg/hal/hal_if.h>     /* For fis_table_entry */

/*
 * Firmware image trailer format
 */
typedef struct firmware_trailer {
    /* Common part - all versions */
    unsigned long magic;
#define FIRMWARE_TRAILER_MAGIC 0xadfacade
    unsigned long crc;          /* Signed hash/auth key */
    /* Auth key omitted in trailer data */
#define FIRMWARE_TRAILER_AUTHKEY 0xead34bc3
    unsigned long version;
    /* Version dependant part */
#define FIRMWARE_TRAILER_VER_1 1
    struct {
        unsigned long type;     /* Image type */
#define FIRMWARE_TRAILER_V1_TYPE_MANAGED   1
#define FIRMWARE_TRAILER_V1_TYPE_UNMANAGED 2 /* Compressed */
        unsigned long flags;
#define FIRMWARE_TRAILER_V1_FLAG_GZIP     (1 << 0)
        unsigned long image_version;
        unsigned long cpu_product_chiptype;
        unsigned long spare;
    } v1;
} firmware_trailer_t;

typedef enum {
    FIRMWARE_ERROR_IN_PROGRESS = MODULE_ERROR_START(VTSS_MODULE_ID_FIRMWARE), /**< In progress */
    FIRMWARE_ERROR_IP,                                                        /**< IP Setup error */
    FIRMWARE_ERROR_TFTP,                                                      /**< TFTP error */
    FIRMWARE_ERROR_BUSY,                                                      /**< Already updating */
    FIRMWARE_ERROR_MALLOC,                                                    /**< Memory allocation error */
    FIRMWARE_ERROR_INVALID,                                                   /**< Image error */
    FIRMWARE_ERROR_FLASH_PROGRAM,                                             /**< FLASH write error */
    FIRMWARE_ERROR_SAME,                                                      /**< Flash is already updated with this image */
    FIRMWARE_ERROR_CURRENT_UNKNOWN,                                           /**< The currently loaded image is unknown */
    FIRMWARE_ERROR_CURRENT_NOT_FOUND,                                         /**< The image that we're currently running was not found in flash */
    FIRMWARE_ERROR_UPDATE_NOT_FOUND,                                          /**< The entry we wish to update was not found in flash */
    FIRMWARE_ERROR_CRC,                                                       /**< The entry has invalid CRC */
    FIRMWARE_ERROR_SIZE,                                                      /**< The entry we wish to update was too small to hold the new image */
    FIRMWARE_ERROR_FLASH_ERASE,                                               /**< An error occurred while attempting to erase the flash */
    FIRMWARE_ERROR_INCORRECT_IMAGE_VERSION,                                   /**< The image version is incorrect. This typically happens if running on RedBoot v. 1.07 and attempting to upload an application with .dat version number set to a value smaller than 2. */
} firmware_error_t;

/* Initialize module */
vtss_rc firmware_init(vtss_init_data_t *data);

/* API functions */

/* Straight file validity check */
vtss_rc firmware_check(const unsigned char *buffer, 
                    size_t length, 
                    unsigned long type, 
                    unsigned long flags, 
                    unsigned long *image_version);

/* Check towards flash file system */
vtss_rc firmware_flash_system_check(const unsigned char *buffer,
                                    unsigned long flashlen, 
                                    const char *name, 
                                    unsigned long image_version);

vtss_rc firmware_update(cli_iolayer_t *io, 
                        const unsigned char *buffer, 
                        size_t length, 
                        vtss_restart_t restart);

char *firmware_tftp(cli_iolayer_t *io, 
                    const unsigned char *server,
                    const unsigned char *file, 
                    const unsigned char *section,
                    size_t *length);
#ifdef VTSS_SW_OPTION_IPV6
char *firmware_tftp_ipv6(cli_iolayer_t *io, 
                    const unsigned char *server,
                    const unsigned char *file, 
                    size_t *length);
#endif /* VTSS_SW_OPTION_IPV6 */

vtss_rc firmware_update_async(cli_iolayer_t *io, const unsigned char *buffer, size_t length, vtss_restart_t restart);

vtss_rc firmware_bootloader_update(cli_iolayer_t *io, const simage_t *simg, const char *name);

void firmware_scan_props(const struct fis_table_entry *fis, 
                         uchar *vers, size_t sz_vers,
                         uchar *date, size_t sz_date);

int firmware_swap_entries(void);

/**
 * \brief Firmware error txt - converts error code to text
 */
char *firmware_error_txt(vtss_rc rc);

// Function that converts a TFTP error code into a string.
void firmware_tftp_err2str(int err_num, char *err_str);

// Status
const char *firmware_status_get(void);
void firmware_status_set(const char *status);

// Debug stuff:
int  firmware_debug_pause_get(void);
void firmware_debug_pause_set(int value);
BOOL firmware_debug_check_same_get(void);
void firmware_debug_check_same_set(BOOL value);

#endif // _FIRMWARE_API_H_


// ***************************************************************************
// 
//  End of file.
// 
// ***************************************************************************
