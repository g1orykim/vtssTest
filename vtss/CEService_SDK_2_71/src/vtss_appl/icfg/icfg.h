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

#ifndef __ICFG_H__
#define __ICFG_H__

#include "icfg_api.h"
#include <sys/stat.h>

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_ICFG

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

// Maximum number of writable files in flash: fs (fs sync-to-flash implementation
// limitation). This allows for 'startup-config' + three other files.
#define ICFG_MAX_WRITABLE_FILES_IN_FLASH_CNT (4)

typedef enum {
    ICFG_COMMIT_IDLE,
    ICFG_COMMIT_RUNNING,
    ICFG_COMMIT_DONE,
    ICFG_COMMIT_SYNTAX_ERROR,
    ICFG_COMMIT_ERROR
} icfg_commit_state_t;

/** \brief Return number of R/O and R/W files in the flash: file system.
    \return FALSE == error, don't trust results; TRUE = all is well.
*/
BOOL icfg_flash_file_count_get(u32 *ro_count, u32 *rw_count);

/** \section Configuration File Load.
 *
 * \details When a set of lines have been assembled, usually through the loading
 * of a file from the local file system, from a TFTP server, or via the web UI,
 * they must be submitted to ICLI for processing and execution.
 */

/** \brief Save running-config to startup-config
 *  \return NULL if save was OK, pointer to constant string error message
 *          otherwise
 */
const char *icfg_running_config_save(void);

/** \brief  Save a buffer to a file name.
 *  \return NULL if save was OK, pointer to constant string error message
 *          otherwise
 */
const char *icfg_file_write(const char *filename, vtss_icfg_query_result_t *res);

/** \brief  Load file from flash into a buffer.
 *  \return NULL if load was OK, pointer to constant string error message
 *          otherwise.
 */
const char *icfg_file_read(const char *filename, vtss_icfg_query_result_t *res);

/** \brief Stat a file that's under ICFG control. Semantically equivalent to
 *         the stdlib stat() function except for the additional compressed_size
 *         parameter, but hides some ugliness about file permissions and size.
 */
int icfg_file_stat(const char *path, struct stat *buf, off_t *compressed_size);


/** \brief Commit set of lines to ICLI.
 *
 * \param session_id        [IN] ICLI session ID of open session, or
 *                               ICLI_SESSION_ID_NONE if temp. must be alloc'd
 * \param source_name       [IN] Source file name
 * \param syntax_check_only [IN] TRUE  == syntax check only, don't exec cmds;
 *                               FALSE == exec cmds
 * \param use_output_buffer [IN] TRUE  == command output/errors are stored in
 *                               an internal output buffer that can later be
 *                               accessed with icfg_output_buffer_get().
 *                               FALSE == output to session_id, or console if
 *                               ICLI_SESSION_ID_NONE
 * \param buffer            [IN] Buffer with lines to execute/syntax check
 * \return                       TRUE  == commit succeeded without errors
 *                               FALSE == errors occured during commit
 */
BOOL icfg_commit(u32                            session_id,
                 const char                     *source_name,
                 BOOL                           syntax_check_only,
                 BOOL                           use_output_buffer,
                 const vtss_icfg_query_result_t *buffer);

/** \brief Return status of most recently submitted commit.
 *
 *  \param running   [OUT] Current commit state
 *  \param error_cnt [OUT] Error count. Not valid while commit is running.
 */
void icfg_commit_status_get(icfg_commit_state_t *state, u32 *error_cnt);

/** \brief Trigger commit thread.
 *  \return NULL if trigger was OK, pointer to constant string error message
 *          otherwise
 */
const char *icfg_commit_trigger(const char *filename, vtss_icfg_query_result_t *buf);

/** \brief Wait for load to complete. Returns immediately if no load is in
 *         progress; otherwise blocks.
 */
void icfg_commit_complete_wait(void);

/** \brief Return pointer to output buffer and its length. The buffer is
 *         updated when icfg_commit_to_icli() is called with
 *         ICLI_SESSION_ID_NONE + use_output_buffer.
 */
const char *icfg_commit_output_buffer_get(u32 *length);

/** \brief Append string to output buffer. */
void icfg_commit_output_buffer_append(const char *str);

#endif /* ICFG_H */
