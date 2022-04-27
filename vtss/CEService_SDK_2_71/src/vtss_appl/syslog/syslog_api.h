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

#ifndef _SYSLOG_API_H_
#define _SYSLOG_API_H_

#include <main.h>
#include <time.h>
#include "sysutil_api.h"    // For VTSS_SYS_HOSTNAME_LEN

/**
 * \file syslog_api.h
 * \brief This file defines the APIs for the syslog module
 */

/**
 * syslog management enabled/disabled
 */
#define SYSLOG_MGMT_ENABLED         1       /**< Enable option  */
#define SYSLOG_MGMT_DISABLED        0       /**< Disable option */

/**
 * syslog level
 */
enum {
    SYSLOG_LVL_INFO,    // E.g. System booted
    SYSLOG_LVL_WARNING, // E.g. Sprout link down
    SYSLOG_LVL_ERROR,   // E.g. T_Exxx() or VTSS_ASSERT()
    SYSLOG_LVL_ALL      // Must come last. Don't use when adding messages to the syslog. It must only be used by callers of syslog_flash_print()
};

/**
 * Default syslog configuration
 */
#define SYSLOG_MGMT_DEFAULT_MODE        SYSLOG_MGMT_DISABLED
#define SYSLOG_MGMT_DEFAULT_PORT_MODE   SYSLOG_MGMT_DISABLED
#define SYSLOG_MGMT_DEFAULT_UDP_PORT    514
#define SYSLOG_MGMT_DEFAULT_SYSLOG_LVL  SYSLOG_LVL_INFO

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    SYSLOG_ERROR_MUST_BE_MASTER = MODULE_ERROR_START(VTSS_MODULE_ID_SYSLOG),    /**< Operation is only allowed on the master switch.                  */
    SYSLOG_ERROR_ISID,                                                          /**< isid parameter is invalid.                                       */
    SYSLOG_ERROR_INV_PARAM,                                                     /**< Invalid parameter.                                               */
};

/****************************************************************************/
// These define the category of syslog messages.
// Use bitwise categorizing, so that the user can choose which categories
// to display.
/****************************************************************************/
typedef enum {
    SYSLOG_CAT_DEBUG,  // Used by trace and VTSS_ASSERT()
    SYSLOG_CAT_SYSTEM, // E.g. system re-flashed, or booted, or...
    SYSLOG_CAT_APP,    // E.g. Sprout connectivity lost
    SYSLOG_CAT_ALL,    // Must come last. Don't use when adding messages to the syslog. It must only be used by callers of syslog_flash_print()
} syslog_cat_t;

/****************************************************************************/
// These define the level of syslog messages
/****************************************************************************/
typedef int syslog_lvl_t;

/**
 * \brief syslog configuration.
 */
typedef struct {
    BOOL            server_mode;                            /**< syslog server global mode setting. */
    char            syslog_server[VTSS_SYS_HOSTNAME_LEN];   /**< syslog server.                     */
    u16             udp_port;                               /**< syslog UDP port number.            */
    syslog_lvl_t    syslog_level;                           /**< syslog level.                      */
} syslog_conf_t;

/* Covert level to string */
char *syslog_lvl_to_string(syslog_lvl_t lvl, BOOL full_name);

/****************************************************************************/
// syslog_flash_log()
// Write a message to the flash.
/****************************************************************************/
void syslog_flash_log(syslog_cat_t cat, syslog_lvl_t lvl, char *msg);

/****************************************************************************/
// syslog_flash_erase()
// Clear the syslog flash. Returns TRUE on success, FALSE on error.
/****************************************************************************/
BOOL syslog_flash_erase(void);

/****************************************************************************/
// syslog_flash_print()
// Print the contents of the syslog flash.
/****************************************************************************/
void syslog_flash_print(syslog_cat_t cat, syslog_lvl_t lvl, int (*print_function)(const char *fmt, ...));

/****************************************************************************/
// syslog_flash_entry_cnt()
// Returns the number of entries in the syslog's flash.
/****************************************************************************/
int syslog_flash_entry_cnt(syslog_cat_t cat, syslog_lvl_t lvl);

/*---- RAM System Log ------------------------------------------------------*/

/* Maximum size of one message */
#define SYSLOG_RAM_MSG_MAX          (10*1024)
#define SYSLOG_RAM_MSG_ID_MAX       0xFFFFFFFF

/* define it if we want to limit a max. entry count */
//#define SYSLOG_RAM_MSG_ENTRY_CNT_MAX (100)

/* RAM entry */
typedef struct {
    ulong            id;        /* Message ID */
    syslog_lvl_t     lvl;       /* Level */
    vtss_module_id_t mid;       /* Module ID */
    time_t           time;      /* Time stamp */
    char             msg[SYSLOG_RAM_MSG_MAX]; /* Message */
} syslog_ram_entry_t;

/* Write to RAM system log */
void syslog_ram_log(syslog_lvl_t lvl, vtss_module_id_t mid, const char *fmt, ...) __attribute__ ((format (__printf__, 3, 4)));

/* Macros to write to RAM system log */
#define S_I(fmt, ...) syslog_ram_log(SYSLOG_LVL_INFO, VTSS_TRACE_MODULE_ID, fmt, ##__VA_ARGS__)
#define S_W(fmt, ...) syslog_ram_log(SYSLOG_LVL_WARNING, VTSS_TRACE_MODULE_ID, fmt, ##__VA_ARGS__)
#define S_E(fmt, ...) syslog_ram_log(SYSLOG_LVL_ERROR, VTSS_TRACE_MODULE_ID, fmt, ##__VA_ARGS__)

/* Clear RAM system log */
void syslog_ram_clear(vtss_isid_t isid, syslog_lvl_t lvl);

/* Get RAM system log entry.
   Note: The newest log can over-write the oldest log when syslog buffer full.
 */
BOOL syslog_ram_get(vtss_isid_t        isid,    /* ISID */
                    BOOL               next,    /* Next or specific entry */
                    ulong              id,      /* Entry ID */
                    syslog_lvl_t       lvl,     /* SYSLOG_LVL_ALL is wildcard */
                    vtss_module_id_t   mid,     /* VTSS_MODULE_ID_NONE is wildcard */
                    syslog_ram_entry_t *entry); /* Returned data */

/* RAM system log statistics */
typedef struct {
    ulong count[SYSLOG_LVL_ALL]; /* Number of entries at each level */
} syslog_ram_stat_t;

/* Get RAM system log statistics */
vtss_rc syslog_ram_stat_get(vtss_isid_t isid, syslog_ram_stat_t *stat);

/*--------------------------------------------------------------------------*/

/****************************************************************************/
// syslog_init()
// Module initialization function.
/****************************************************************************/
vtss_rc syslog_init(vtss_init_data_t *data);

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the syslog API functions.
  *
  * \param rc [IN]: Error code that must be in the SYSLOG_ERROR_XXX range.
  */
char *syslog_error_txt(vtss_rc rc);

/**
  * \brief Get the global syslog configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that receives
  *                        the current configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    SYSLOG_ERROR_INV_PARAM if glbl_cfg is NULL.\n
  *    SYSLOG_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc syslog_mgmt_conf_get(syslog_conf_t *glbl_cfg);

/**
  * \brief Set the global syslog configuration.
  *
  * \param glbl_cfg [IN]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       syslog module.
  *
  * \return
  *    VTSS_OK on success.\n
  *    SYSLOG_ERROR_INV_PARAM if glbl_cfg is NULL or parameters error.\n
  *    SYSLOG_ERROR_MUST_BE_MASTER if called on a slave switch.\n
  */
vtss_rc syslog_mgmt_conf_set(syslog_conf_t *glbl_cfg);

/**
  * \brief Get the global default syslog configuration.
  *
  * \param glbl_cfg [OUT]: Pointer to structure that contains the
  *                       global configuration to apply to the
  *                       syslog module.
  */
void syslog_mgmt_default_get(syslog_conf_t *glbl_cfg);
#endif /* _SYSLOG_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
