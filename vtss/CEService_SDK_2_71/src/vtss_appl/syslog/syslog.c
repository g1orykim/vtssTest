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

#include "vtss_os.h"
#include "main.h"
#include "conf_api.h"
#include "syslog_api.h"
#include "syslog.h"
#include "critd_api.h"
#include "led_api.h"
#include "control_api.h"
#include "misc_api.h"
#include "msg_api.h"
#include "flash_mgmt_api.h"
#include <string.h>
#ifdef VTSS_SW_OPTION_VCLI
#include "syslog_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "syslog_icfg.h"
#endif
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif
#include <network.h>    // For struct sockaddr_in
#include <arpa/inet.h>  // For inet_addr()

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SYSLOG

/****************************************************************************/
/*  Private global variables                                                */
/****************************************************************************/

/* Private global structure */
static BOOL SYSLOG_init = FALSE;
static syslog_global_t SYSLOG_global;

/* Thread variables */
#define NTP_DELAY_SEC               (15)    // 15 seconds
#define SL_THREAD_DELAY_SEC         (2)     // 2 seconds
#define SYSLOG_THREAD_STACK_SIZE    (16*1024)
static cyg_handle_t SYSLOG_thread_handle;
static cyg_thread   SYSLOG_thread_block;
static char         SYSLOG_thread_stack[SYSLOG_THREAD_STACK_SIZE];

/****************************************************************************
 * Trace definitions
 ****************************************************************************/
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_SYSLOG
#define VTSS_TRACE_GRP_DEFAULT  0
#define TRACE_GRP_CRIT          1
#define TRACE_GRP_CNT           2
#include <vtss_trace_api.h>

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "Syslog",
    .descr     = "Syslog Module"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};

#define SYSLOG_FLASH_CRIT_ENTER()   critd_enter(&SL_flash_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SYSLOG_FLASH_CRIT_EXIT()    critd_exit( &SL_flash_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SYSLOG_RAM_CRIT_ENTER()     critd_enter(&SL_ram_crit,   TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SYSLOG_RAM_CRIT_EXIT()      critd_exit( &SL_ram_crit,   TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SYSLOG_CRIT_ENTER()         critd_enter(&SYSLOG_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SYSLOG_CRIT_EXIT()          critd_exit( &SYSLOG_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define SYSLOG_FLASH_CRIT_ENTER()   critd_enter(&SL_flash_crit)
#define SYSLOG_FLASH_CRIT_EXIT()    critd_exit( &SL_flash_crit)
#define SYSLOG_RAM_CRIT_ENTER()     critd_enter(&SL_ram_crit)
#define SYSLOG_RAM_CRIT_EXIT()      critd_exit( &SL_ram_crit)
#define SYSLOG_CRIT_ENTER()         critd_enter(&SYSLOG_global.crit)
#define SYSLOG_CRIT_EXIT()          critd_exit( &SYSLOG_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*                                                                          */
/*  NAMING CONVENTION FOR INTERNAL FUNCTIONS:                               */
/*    SL_<function_name>                                                    */
/*                                                                          */
/*  NAMING CONVENTION FOR EXTERNAL (API) FUNCTIONS:                         */
/*    syslog_<function_name>                                                */
/*                                                                          */
/****************************************************************************/

// Flash Syslog Variables
static BOOL            SL_flash_enabled;
static cyg_flashaddr_t SL_flash_next_free_entry;
static critd_t         SL_flash_crit;
static int             SL_flash_entry_cnt[SYSLOG_CAT_ALL][SYSLOG_LVL_ALL]; // Counts per category and per level the number of entries in the syslog.

// These are populated during boot.
static flash_mgmt_section_info_t SL_flash_info;

#define SYSLOG_NEXT_32_BIT_BOUNDARY(sz) (4*(((sz)+3)/4))

#define SYSLOG_MAX_WR_CNT 20
static uint SL_wr_cnt = 0;

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS                                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*  Message text functions                                                  */
/****************************************************************************/

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *SL_msg_id_txt(SL_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case SL_MSG_ID_ENTRY_GET_REQ:
        txt = "SL_MSG_ID_ENTRY_GET_REQ";
        break;
    case SL_MSG_ID_ENTRY_GET_REP:
        txt = "SL_MSG_ID_ENTRY_GET_REP";
        break;
    case SL_MSG_ID_STAT_GET_REQ:
        txt = "SL_MSG_ID_STAT_GET_REQ";
        break;
    case SL_MSG_ID_STAT_GET_REP:
        txt = "SL_MSG_ID_STAT_GET_REP";
        break;
    case SL_MSG_ID_CONF_SET_REQ:
        txt = "SL_MSG_ID_CONF_SET_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/****************************************************************************/
// SL_flash_addr_get()
// Find the syslog in the flash. This will update the SL_flash_syslog_XXX
// variables.
/****************************************************************************/
static BOOL SL_flash_addr_get(void)
{
    if (!flash_mgmt_lookup("syslog", &SL_flash_info)) {
        // Unable to obtain info about the "syslog" entry.
        return FALSE;
    }
    return TRUE;
}

/****************************************************************************/
// SL_flash_load()
// Checks the flash for the system log and updates first and next free
// pointers.
// If the syslog is present in flash, it returns TRUE, otherwise FALSE.
/****************************************************************************/
static BOOL SL_flash_load(void)
{
    cyg_flashaddr_t flptr, next_flptr;
    SL_flash_hdr_t hdr_buf, *hdr = &hdr_buf;
    BOOL result = FALSE;

    SL_flash_next_free_entry = 0;

    flptr = SL_flash_info.base_fladdr;
    memset(&SL_flash_entry_cnt[0][0], 0, sizeof(SL_flash_entry_cnt));

    if (control_flash_read(flptr, hdr, sizeof(*hdr)) != CYG_FLASH_ERR_OK ||
        hdr->size != sizeof(SL_flash_hdr_t) ||
        hdr->cookie != SYSLOG_FLASH_HDR_COOKIE ||
        hdr->version != SYSLOG_FLASH_HDR_VERSION) {
        goto do_exit; // Syslog not found
    }

    // Go to the first entry. Both the header and entries are 32-bit aligned.
    flptr += SYSLOG_NEXT_32_BIT_BOUNDARY(hdr->size);

    while (flptr < (SL_flash_info.base_fladdr + SL_flash_info.size_bytes)) {
        SL_flash_entry_t entry_buf, *entry = &entry_buf;

        // If the entry contains uninitialized flash values, we expect this to be the very first empty entry,
        // and expect this area to be writeable without erasing the whole sector.
        if (control_flash_read(flptr, entry, sizeof(*entry)) != CYG_FLASH_ERR_OK) {
            goto do_exit;
        }
        if (entry->size    == SYSLOG_FLASH_UNINIT_FLASH_VALUE_LONG &&
            entry->cookie  == SYSLOG_FLASH_UNINIT_FLASH_VALUE_LONG &&
            entry->version == SYSLOG_FLASH_UNINIT_FLASH_VALUE_LONG &&
            entry->time    == (time_t)SYSLOG_FLASH_UNINIT_FLASH_VALUE_LONG &&
            entry->cat     == (syslog_cat_t)SYSLOG_FLASH_UNINIT_FLASH_VALUE_LONG &&
            entry->lvl     == (syslog_lvl_t)SYSLOG_FLASH_UNINIT_FLASH_VALUE_LONG) {
            SL_flash_next_free_entry = flptr;
            result = TRUE;
            goto do_exit;
        }

        next_flptr = flptr + SYSLOG_NEXT_32_BIT_BOUNDARY(entry->size);

        if (entry->size    <= sizeof(SL_flash_entry_t)         || // Equal sign because the message following the header must be non-empty (at least a NULL character)
            next_flptr     <= flptr                            ||
            next_flptr     >= (SL_flash_info.base_fladdr + SL_flash_info.size_bytes) ||
            entry->cookie  != SYSLOG_FLASH_ENTRY_COOKIE        ||
            entry->version != SYSLOG_FLASH_ENTRY_VERSION       ||
            entry->cat     >= SYSLOG_CAT_ALL                   ||
            entry->lvl     >= SYSLOG_LVL_ALL) {
            memset(&SL_flash_entry_cnt[0][0], 0, sizeof(SL_flash_entry_cnt));
            goto do_exit;
        }

        flptr = next_flptr;
        SL_flash_entry_cnt[entry->cat][entry->lvl]++;
    }

    memset(&SL_flash_entry_cnt[0][0], 0, sizeof(SL_flash_entry_cnt));
    // We dropped out of the area allocated for us.

do_exit:
    return result;
}

/****************************************************************************/
// SL_flash_create()
// Unconditionally erases the flash and creates a new header signature.
// Returns FALSE on error, TRUE on success.
/****************************************************************************/
static BOOL SL_flash_create(void)
{
    SL_flash_hdr_t hdr;

    SL_flash_enabled = FALSE; // Disallow updates.
    memset(&SL_flash_entry_cnt[0][0], 0, sizeof(SL_flash_entry_cnt));
    SL_flash_next_free_entry = 0;

    // Erase the flash and create a new syslog signature.
    if (control_flash_erase(SL_flash_info.base_fladdr, SL_flash_info.size_bytes) != FLASH_ERR_OK) {
        return FALSE; // Erase failed. We keep the flash logging disabled.
    }

    hdr.size    = sizeof(SL_flash_hdr_t);
    hdr.cookie  = SYSLOG_FLASH_HDR_COOKIE;
    hdr.version = SYSLOG_FLASH_HDR_VERSION;

    if (control_flash_program(SL_flash_info.base_fladdr, &hdr, sizeof(hdr)) != FLASH_ERR_OK) {
        return FALSE; // Program failed. We keep the flash logging disabled.
    }

    SL_flash_next_free_entry = SL_flash_info.base_fladdr + SYSLOG_NEXT_32_BIT_BOUNDARY(sizeof(SL_flash_hdr_t));
    SL_flash_enabled = TRUE; // Everything is OK. Allow updates from now on.

    return TRUE;
}

/****************************************************************************/
// SL_flash_open()
// Checks if the flash contains a valid syslog. If not, the flash is erased
// and a valid syslog is created.
/****************************************************************************/
static void SL_flash_open(void)
{
    // Find the syslog in the flash.
    if (!SL_flash_addr_get()) {
        // No syslog. Keep writing to flash disabled.
        SL_flash_enabled = FALSE;
        return;
    }

    if (!SL_flash_load()) {
        // The current contents of the flash wasn't valid. Erase and create new signature.
        if (!SL_flash_create()) {
            T_E("Unable to create an empty syslog flash");
        }
    } else {
        SL_flash_enabled = TRUE;
    }
}

/****************************************************************************/
/****************************************************************************/
static char *SL_cat_to_string(syslog_cat_t cat)
{
    switch (cat) {
    case SYSLOG_CAT_DEBUG:
        return "Debug";

    case SYSLOG_CAT_SYSTEM:
        return "System";

    case SYSLOG_CAT_APP:
        return "Application";

    default:
        return "Unknown";
    }
}

/****************************************************************************/
/****************************************************************************/
static time_t SL_get_time_in_secs(void)
{
    return time(NULL);
}

/****************************************************************************/
/****************************************************************************/
static void SL_flash_print_header(int (*print_function)(const char *fmt, ...))
{
    int i;
    (void)print_function("%-11s | %-7s | %-25s | %s\n", "Category", "Level", "Time", "Message");
    for (i = 0; i < 80; i++) {
        (void)print_function("-");
    }
    (void)print_function("\n");
}

/****************************************************************************/
// Assuming critical section is taken.
/****************************************************************************/
static void SL_flash_log(syslog_cat_t cat, syslog_lvl_t lvl, char *msg)
{
    ulong msg_sz;
    ulong total_sz;
    SL_flash_entry_t entry;

    if (!SL_flash_enabled) {
        return;
    }

    msg_sz = strlen(msg) + 1; // Include NULL-terminator in size
    total_sz = sizeof(SL_flash_entry_t) + msg_sz;

    // Check if there's room for this message
    if (SL_flash_next_free_entry + total_sz >= SL_flash_info.base_fladdr + SL_flash_info.size_bytes) {
        SL_flash_enabled = FALSE;
        return;
    }

    entry.size    = total_sz;
    entry.cookie  = SYSLOG_FLASH_ENTRY_COOKIE;
    entry.version = SYSLOG_FLASH_ENTRY_VERSION;
    entry.time    = SL_get_time_in_secs();
    entry.cat     = cat;
    entry.lvl     = lvl;

    // Write the entry header to flash.
    if (control_flash_program(SL_flash_next_free_entry, &entry, sizeof(entry)) != FLASH_ERR_OK) {
        SL_flash_enabled = FALSE;
        return; // Program failed. We keep the flash logging disabled.
    }

    // Write the message to flash.
    if (control_flash_program((SL_flash_next_free_entry + sizeof(entry)), msg, msg_sz) != FLASH_ERR_OK) {
        SL_flash_enabled = FALSE;
    }

    // Update the next free entry pointer
    SL_flash_next_free_entry += SYSLOG_NEXT_32_BIT_BOUNDARY(total_sz);
    SL_flash_entry_cnt[cat][lvl]++;
}

/****************************************************************************/
// SL_assert_cb()
// Called just before an assertion occurs.
/****************************************************************************/
static char SL_reserved_msg[SYSLOG_RAM_MSG_MAX]; // The message buffer is used if VTSS_MALLOC() fails.

/* About lint error 673: Warning -- Possibly inappropriate deallocation (free) for 'static' data.
   We had protect point of "buf" already */
/*lint -e{673} */
static void SL_assert_cb(const char *file_name, const unsigned long line_num, const char *msg)
{
    int total_sz_bytes;
    char *buf;

    // Enter the front led fatal state
    led_front_led_state(LED_FRONT_LED_FATAL);

    SYSLOG_FLASH_CRIT_ENTER();

    // Allocate a buffer big enough to hold the full assertion as one string
    total_sz_bytes = strlen(file_name) + 10 /* max line sz */ + strlen(msg) + 20 /* Various */;
    if ((buf = VTSS_MALLOC(total_sz_bytes)) == NULL) {
        buf = SL_reserved_msg;
    }

    // Store the message in the buffer
    sprintf(buf, "%s(%lu): %s", file_name, line_num, msg);

    (void)diag_printf("ASSERTION FAILED: %s", buf);

    // Save the message to flash. We bypass the normal check for exceeded
    // syslog entry count per session, because fatals will only occur once
    // per session.
    SL_flash_log(SYSLOG_CAT_DEBUG, SYSLOG_LVL_ERROR, buf);

    if (buf != SL_reserved_msg) {
        VTSS_FREE(buf);
    }

    SYSLOG_FLASH_CRIT_EXIT();
}

/****************************************************************************/
/*                                                                          */
/*  MODULE EXTERNAL FUNCTIONS                                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/

/* Covert level to string */
char *syslog_lvl_to_string(syslog_lvl_t lvl, BOOL full_name)
{
    if (full_name) {
        switch (lvl) {
        case SYSLOG_LVL_INFO:
            return "information";

        case SYSLOG_LVL_WARNING:
            return "warning";

        case SYSLOG_LVL_ERROR:
            return "error";

        case SYSLOG_LVL_ALL:
            return "all";

        default:
            return "unknown";
        }
    } else {
        switch (lvl) {
        case SYSLOG_LVL_INFO:
            return "Info";

        case SYSLOG_LVL_WARNING:
            return "Warning";

        case SYSLOG_LVL_ERROR:
            return "Error";

        case SYSLOG_LVL_ALL:
            return "All";

        default:
            return "Unknown";
        }
    }
}

void syslog_flash_log(syslog_cat_t cat, syslog_lvl_t lvl, char *msg)
{
    static char too_many_msg[] = "Too many messages written to syslog. Last messages skipped.";

    if (cat >= SYSLOG_CAT_ALL || lvl >= SYSLOG_LVL_ALL) {
        return;    // Illegal.
    }

    // Gotta check if we're ready to accept trace loggings before attempting to take
    // the SL_flash_crit (with the call to SYSLOG_FLASH_CRIT_EXIT() macro), because
    // a deadlock could otherwise occur in any of the following situations:
    // 1) Any module calls T_E() from INIT_CMD_INIT
    // 2) A module that is coming before the syslog module in the module list
    //    calls T_E() from INIT_CMD_START.
    // Such calls to T_E() will *not* result in the message logged to flash.
    // The access to SL_flash_enabled is thus unprotected, but we can certainly
    // live with that since it's a simple boolean that won't become TRUE until
    // after we've exited the SL_flash_crit the first time.
    if (!SL_flash_enabled) {
        return;
    }

    SYSLOG_FLASH_CRIT_ENTER();

    if (!control_system_flash_trylock()) {
        // Someone else is currently owning the flash. Don't wait for it to complete, since
        // that may cause deadlocks (or at least critd timeouts).
        SYSLOG_FLASH_CRIT_EXIT();
        return;
    }

    if (++SL_wr_cnt > SYSLOG_MAX_WR_CNT) {
        if (SL_wr_cnt == SYSLOG_MAX_WR_CNT + 1) {
            msg = too_many_msg;
            lvl = SYSLOG_LVL_ERROR;
            cat = SYSLOG_CAT_SYSTEM;
        } else {
            SL_wr_cnt = SYSLOG_MAX_WR_CNT + 2; // Avoid wrap-around
            goto exit_func;
        }
    }

    SL_flash_log(cat, lvl, msg);


exit_func:
    control_system_flash_unlock();
    SYSLOG_FLASH_CRIT_EXIT();
}

/****************************************************************************/
/****************************************************************************/
BOOL syslog_flash_erase(void)
{
    BOOL result;

    SYSLOG_FLASH_CRIT_ENTER();
    if (!control_system_flash_trylock()) {
        SYSLOG_FLASH_CRIT_EXIT();
        return FALSE;
    }

    if ((result = SL_flash_create()) == FALSE) {
        T_E("Unable to create an empty syslog flash");
    }
    SL_wr_cnt = 0;
    control_system_flash_unlock();
    SYSLOG_FLASH_CRIT_EXIT();
    return result;
}

/****************************************************************************/
/****************************************************************************/
void syslog_flash_print(syslog_cat_t cat, syslog_lvl_t lvl, int (*print_function)(const char *fmt, ...))
{
    cyg_flashaddr_t flptr, msg;
    SL_flash_entry_t entry_buf, *entry = &entry_buf;
    BOOL at_least_one_found = FALSE;
    u8 *msgbuf = NULL;
    size_t msgbuf_len = 0;

    if (!print_function) {
        return;
    }

    if (cat > SYSLOG_CAT_ALL || lvl > SYSLOG_LVL_ALL) {
        (void)print_function("Invalid category or level\n");
        return;
    }

    SYSLOG_FLASH_CRIT_ENTER();

    // We need to take the flash lock, since reading the flash while another
    // thread is writing it will return garbage to us - and possibly
    // cause the write to fail?
    if (!control_system_flash_trylock()) {
        (void)print_function("The flash is in use by another process. Please try again later...\n");
        SYSLOG_FLASH_CRIT_EXIT();
        return;
    }

    if (!SL_flash_enabled) {
        (void)print_function("The syslog is not enabled\n");
        goto exit_func;
    }

    flptr = SL_flash_info.base_fladdr + SYSLOG_NEXT_32_BIT_BOUNDARY(sizeof(SL_flash_hdr_t));
    while (flptr < SL_flash_next_free_entry) { /*lint -e{449} ... We're aware of the realloc hazards */
        if (control_flash_read(flptr, entry, sizeof(*entry)) == CYG_FLASH_ERR_OK &&
            (cat == SYSLOG_CAT_ALL || entry->cat == cat) &&
            (lvl == SYSLOG_LVL_ALL || entry->lvl == lvl)) {
            size_t msglen = entry->size - sizeof(SL_flash_entry_t); /* Includes NULL */
            if (!at_least_one_found) {
                SL_flash_print_header(print_function);
            }
            /* Allocate message buffer */
            if (msglen > msgbuf_len) {
                u8 *newbuf = VTSS_REALLOC(msgbuf, msglen);
                if (newbuf) {
                    msgbuf_len = msglen;
                    msgbuf = newbuf; /* May have changed */
                }
            }
            msg = flptr + sizeof(SL_flash_entry_t);
            if (msgbuf && msglen <= msgbuf_len &&
                control_flash_read(msg, msgbuf, msglen) == CYG_FLASH_ERR_OK) {
                msgbuf[msglen - 1] = '\0'; /* Terminate to be safe (should be unnecessary)  */
                (void)print_function("%-11s | %-7s | %s | %s\n", SL_cat_to_string(entry->cat),
                                     syslog_lvl_to_string(entry->lvl, FALSE), misc_time2str(entry->time), msgbuf);
                at_least_one_found = TRUE;
            }
        }
        flptr += SYSLOG_NEXT_32_BIT_BOUNDARY(entry->size);
    }

    if (!at_least_one_found) {
        (void)print_function("No entries found\n");
    }

    if (msgbuf) {
        VTSS_FREE(msgbuf);
    }

exit_func:
    control_system_flash_unlock();
    SYSLOG_FLASH_CRIT_EXIT();
}

/****************************************************************************/
/****************************************************************************/
int syslog_flash_entry_cnt(syslog_cat_t cat, syslog_lvl_t lvl)
{
    int entry_cnt = 0, c, l;
    if (cat > SYSLOG_CAT_ALL || lvl > SYSLOG_LVL_ALL) {
        return 0;    // Illegal
    }

    SYSLOG_FLASH_CRIT_ENTER();
    if (cat == SYSLOG_CAT_ALL) {
        if (lvl == SYSLOG_LVL_ALL) {
            // All entries summed up
            for (c = 0; c < SYSLOG_CAT_ALL; c++) {
                for (l = 0; l < SYSLOG_LVL_ALL; l++) {
                    entry_cnt += SL_flash_entry_cnt[c][l];
                }
            }
        } else {
            // All categories, specific levels
            for (c = 0; c < SYSLOG_CAT_ALL; c++) {
                entry_cnt += SL_flash_entry_cnt[c][lvl];
            }
        }
    } else {
        if (lvl == SYSLOG_LVL_ALL) {
            // Specific category, all levels
            for (l = 0; l < SYSLOG_CAT_ALL; l++) {
                entry_cnt += SL_flash_entry_cnt[cat][l];
            }
        } else {
            // Specific category, specific level
            entry_cnt = SL_flash_entry_cnt[cat][lvl];
        }
    }
    SYSLOG_FLASH_CRIT_EXIT();
    return entry_cnt;
}

/*---- RAM System Log ------------------------------------------------------*/

static SL_ram_t SL_ram;
static critd_t  SL_ram_crit;

/* Allocate request buffer */
static SL_msg_req_t *SL_msg_req_alloc(SL_msg_id_t msg_id)
{
    SL_msg_req_t *msg = msg_buf_pool_get(SL_ram.request);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/* Allocate reply buffer */
static SL_msg_rep_t *SL_msg_rep_alloc(SL_msg_id_t msg_id)
{
    SL_msg_rep_t *msg = msg_buf_pool_get(SL_ram.reply);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

static void SL_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    (void)msg_buf_pool_put(msg);
}

static void SL_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    SL_msg_id_t msg_id = *(SL_msg_id_t *)msg;
    size_t total_len = len + MSG_TX_DATA_HDR_LEN_MAX(SL_msg_req_t, data, SL_msg_rep_t, data);

    T_D("isid: %d, msg_id: %d(%s), len: %zu", isid, msg_id, SL_msg_id_txt(msg_id), total_len);
    T_R_HEX(msg, total_len);
    msg_tx_adv(NULL, SL_msg_tx_done, MSG_TX_OPT_DONT_FREE | MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK, VTSS_MODULE_ID_SYSLOG, isid, msg, total_len);
}

/* Get RAM system log entry */
static BOOL SL_ram_get(vtss_isid_t        isid,    /* ISID */
                       BOOL               next,    /* Next or specific entry */
                       ulong              id,      /* Entry ID */
                       syslog_lvl_t       lvl,     /* SYSLOG_LVL_ALL is wildcard */
                       vtss_module_id_t   mid,     /* VTSS_MODULE_ID_NONE is wildcard */
                       syslog_ram_entry_t *entry,  /* Returned data */
                       BOOL               convert)
{
    SL_ram_entry_t *cur;
    BOOL           is_wrap, is_in_wrap;

    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_local(isid)) {
        SL_msg_req_t     *req;
        cyg_flag_value_t flag;
        BOOL             found;

        /* Wait for reply buffer semaphore and setup pointer */
        (void)VTSS_OS_SEM_WAIT(&SL_ram.mgmt_reply.sem);
        SYSLOG_RAM_CRIT_ENTER();
        SL_ram.mgmt_reply.entry = entry;
        SL_ram.mgmt_reply.found = 0;
        flag = (1 << 0);
        cyg_flag_maskbits(&SL_ram.mgmt_reply.flags, ~flag);
        SYSLOG_RAM_CRIT_EXIT();

        /* Send request message */
        req = SL_msg_req_alloc(SL_MSG_ID_ENTRY_GET_REQ);
        req->data.entry_get.next = next;
        req->data.entry_get.id = id;
        req->data.entry_get.lvl = lvl;
        req->data.entry_get.mid = mid;
        SL_msg_tx(req, isid, sizeof(req->data.entry_get));

        /* Wait for reply */
        (void)cyg_flag_timed_wait(&SL_ram.mgmt_reply.flags, flag, CYG_FLAG_WAITMODE_OR, cyg_current_time() + VTSS_OS_MSEC2TICK(5 * 1000));

        /* Clear pointer and release post reply buffer semaphore */
        SYSLOG_RAM_CRIT_ENTER();
        SL_ram.mgmt_reply.entry = NULL;
        found = SL_ram.mgmt_reply.found;
        if (found) {
            entry->time = msg_abstime_get(isid, entry->time);
        }
        SYSLOG_RAM_CRIT_EXIT();
        VTSS_OS_SEM_POST(&SL_ram.mgmt_reply.sem);

        return found;
    }

    /* Local log access */
    SYSLOG_RAM_CRIT_ENTER();

    // Check NULL point
    if (!SL_ram.first || !SL_ram.last) {
        SYSLOG_RAM_CRIT_EXIT();
        return FALSE;
    }

    is_wrap = SL_ram.first->id > SL_ram.last->id;
    is_in_wrap = id < SL_ram.first->id;
    for (cur = (SL_ram_entry_t *)SL_ram.first;
         cur != NULL; cur = cur->next) {
        /* Check ID for GET_NEXT operation */
        if (next && id) {
            if ((!is_wrap && cur->id <= id) ||
                (is_wrap && ((is_in_wrap && (cur->id <= id || cur->id >= SL_ram.first->id)) || (!is_in_wrap && cur->id <= id && cur->id >= SL_ram.first->id)))) {
                continue;
            }
        }

        /* Check ID for GET operation */
        if (!next && cur->id != id) {
            continue;
        }

        /* Check level */
        if (lvl != SYSLOG_LVL_ALL && cur->lvl != lvl) {
            continue;
        }

        /* Check module ID */
        if (mid != VTSS_MODULE_ID_NONE && cur->mid != mid) {
            continue;
        }

        /* Copy data */
        entry->id = cur->id;
        entry->lvl = cur->lvl;
        entry->mid = cur->mid;
        entry->time = (convert ? msg_abstime_get(isid, cur->time) : cur->time);

        strcpy(entry->msg, cur->msg);
        break;
    }
    SYSLOG_RAM_CRIT_EXIT();

    return (cur == NULL ? 0 : 1);
}

static vtss_rc SL_ram_stat_get(vtss_isid_t isid, syslog_ram_stat_t *stat)
{
    cyg_flag_value_t flag;
    SL_msg_req_t     *req;

    T_D("enter, isid: %d", isid);

    req = SL_msg_req_alloc(SL_MSG_ID_STAT_GET_REQ);
    flag = (1 << isid);
    cyg_flag_maskbits(&SL_ram.stat_flags, ~flag);
    SL_msg_tx(req, isid, 0);
    if (cyg_flag_timed_wait(&SL_ram.stat_flags, flag, CYG_FLAG_WAITMODE_OR, cyg_current_time() + VTSS_OS_MSEC2TICK(5 * 1000)) & flag) {
        SYSLOG_RAM_CRIT_ENTER();
        *stat = SL_ram.stat[isid];
        SYSLOG_RAM_CRIT_EXIT();
    }
    return VTSS_OK;
}

static BOOL SL_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    SL_msg_id_t msg_id = *(SL_msg_id_t *)rx_msg;

    T_D("isid: %d, msg_id: %d(%s), len: %zu", isid, msg_id, SL_msg_id_txt(msg_id), len);
    T_R_HEX(rx_msg, len);
    switch (msg_id) {
    case SL_MSG_ID_ENTRY_GET_REQ: {
        SL_msg_req_t *req;
        SL_msg_rep_t *rep;

        req = (SL_msg_req_t *)rx_msg;
        T_D("ENTRY_GET_REQ, next: %d, id: %d", req->data.entry_get.next, req->data.entry_get.id);
        rep = SL_msg_rep_alloc(SL_MSG_ID_ENTRY_GET_REP);
        rep->data.entry_get.found = SL_ram_get(VTSS_ISID_LOCAL,
                                               req->data.entry_get.next,
                                               req->data.entry_get.id,
                                               req->data.entry_get.lvl,
                                               req->data.entry_get.mid,
                                               &rep->data.entry_get.entry,
                                               0);
        SL_msg_tx(rep, isid,
                  sizeof(rep->data.entry_get) - SYSLOG_RAM_MSG_MAX +
                  (rep->data.entry_get.found ?
                   (strlen(rep->data.entry_get.entry.msg) + 1) : 0));
        break;
    }
    case SL_MSG_ID_ENTRY_GET_REP: {
        SL_msg_rep_t *rep;

        rep = (SL_msg_rep_t *)rx_msg;
        T_D("ENTRY_GET_REP, found: %d, id: %d", rep->data.entry_get.found, rep->data.entry_get.entry.id);
        SYSLOG_RAM_CRIT_ENTER();
        if (SL_ram.mgmt_reply.entry != NULL) {
            SL_ram.mgmt_reply.found = rep->data.entry_get.found;
            // length of entry is calculated as:  number of received bytes - offset of entry (skip "msg_id" and "found")
            memcpy(SL_ram.mgmt_reply.entry, &rep->data.entry_get.entry, len - offsetof(SL_msg_rep_t, data.entry_get.entry));
            cyg_flag_setbits(&SL_ram.mgmt_reply.flags, 1 << 0);
        }
        SYSLOG_RAM_CRIT_EXIT();
        break;
    }
    case SL_MSG_ID_STAT_GET_REQ: {
        SL_msg_rep_t *rep;

        T_D("STAT_GET_REQ");
        rep = SL_msg_rep_alloc(SL_MSG_ID_STAT_GET_REP);
        SYSLOG_RAM_CRIT_ENTER();
        rep->data.stat_get.stat = SL_ram.stat[VTSS_ISID_LOCAL];
        SYSLOG_RAM_CRIT_EXIT();
        SL_msg_tx(rep, isid, sizeof(rep->data.stat_get));
        break;
    }
    case SL_MSG_ID_STAT_GET_REP: {
        SL_msg_rep_t *rep;

        rep = (SL_msg_rep_t *)rx_msg;
        T_D("STAT_GET_REP");
        SYSLOG_RAM_CRIT_ENTER();
        SL_ram.stat[isid] = rep->data.stat_get.stat;
        SYSLOG_RAM_CRIT_EXIT();
        cyg_flag_setbits(&SL_ram.stat_flags, 1 << isid);
        break;
    }
    case SL_MSG_ID_CLR_REQ: {
        SL_msg_req_t *msg = (SL_msg_req_t *)rx_msg;
        T_D("CLR_REQ");
        syslog_ram_clear(VTSS_ISID_LOCAL, msg->data.entry_clear.lvl);
        break;
    }
    case SL_MSG_ID_CONF_SET_REQ: {
        SL_msg_req_t *msg = (SL_msg_req_t *)rx_msg;

        if (!msg_switch_is_master()) {
            SYSLOG_CRIT_ENTER();
            SYSLOG_global.conf = msg->data.conf_set.conf;
            SYSLOG_CRIT_EXIT();
        }
        break;
    }
    default:
        break;
    }
    return TRUE;
}

/* Clear RAM system log */
static void SL_ram_clear(syslog_lvl_t lvl)
{
    SL_ram_entry_t *cur, *prev;

    if (lvl == SYSLOG_LVL_ALL) {
        SL_ram.first = SL_ram.last = NULL;
        memset(SL_ram.stat, 0, sizeof(SL_ram.stat));
    } else {
        /* The syslog poll maybe exists huge messages, we don't want to take much time for moving messages.
           To process delete individual messages, only re-structure the link list point here. */
        for (cur = (SL_ram_entry_t *)SL_ram.first, prev = NULL;
             cur != NULL;) {
            if (cur->lvl != lvl) {
                prev = cur;
                cur = cur->next;
                continue;
            }
            if (cur == (SL_ram_entry_t *)SL_ram.first) {
                if (cur->next) {
                    SL_ram.first = SL_ram.first->next;
                } else {
                    SL_ram.first = SL_ram.last = NULL;
                    break;
                }
            } else if (cur == SL_ram.last) {
                if (prev) {
                    prev->next = NULL;
                }
                SL_ram.last = prev;
                break;
            } else if (cur->next) {
                if (prev) {
                    prev->next = cur->next;
                }
                cur = cur->next;
            }
        }
        SL_ram.stat[VTSS_ISID_LOCAL].count[lvl] = 0;
    }
}

/* Initialize RAM system log */
static void SL_ram_init(BOOL init)
{
    msg_rx_filter_t filter;

    if (init) {
        SL_ram_clear(SYSLOG_LVL_ALL);
        critd_init(&SL_ram_crit, "syslog_ram.crit", VTSS_MODULE_ID_SYSLOG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        SYSLOG_RAM_CRIT_EXIT(); // Initially locked.
        SL_ram.request = msg_buf_pool_create(VTSS_MODULE_ID_SYSLOG, "Request", 2, sizeof(SL_msg_req_t));
        SL_ram.reply   = msg_buf_pool_create(VTSS_MODULE_ID_SYSLOG, "Reply",   2, sizeof(SL_msg_rep_t));
        cyg_flag_init(&SL_ram.mgmt_reply.flags);
        VTSS_OS_SEM_CREATE(&SL_ram.mgmt_reply.sem, 1);
        SL_ram.mgmt_reply.entry = NULL;
        cyg_flag_init(&SL_ram.stat_flags);
    } else {
        /* Register for stack messages */
        memset(&filter, 0, sizeof(filter));
        filter.cb = SL_msg_rx;
        filter.modid = VTSS_MODULE_ID_SYSLOG;
        (void)msg_rx_filter_register(&filter);
    }
}

void syslog_ram_log(syslog_lvl_t lvl, vtss_module_id_t mid, const char *fmt, ...)
{
    va_list        args = NULL;
    int            n;
    SL_ram_entry_t *new;
    BOOL           buf_full = FALSE;
    char           *temp_msg = NULL;
    size_t         temp_msg_len = 0, ram_entry_header_len = sizeof(SL_ram_entry_t) - SYSLOG_RAM_MSG_MAX;

    SYSLOG_RAM_CRIT_ENTER();

    if (SYSLOG_init == FALSE) {
        SYSLOG_RAM_CRIT_EXIT();
        return;
    }

    /* Add entry to list */
    if (SL_ram.last == NULL) {
        /* Insert entry first in list */
        new = (SL_ram_entry_t *)&SL_ram.log[0];
        SL_ram.first = SL_ram.last = new;
        if (SL_ram.current_id == SYSLOG_RAM_MSG_ID_MAX) { // syslog ID wrap-around
            new->id = SL_ram.current_id = 1;
        } else {
            new->id = ++SL_ram.current_id;
        }
    } else {
        /* Next entry is on 4-byte aligned address, need to consider one byye of '\0' */
        n = (strlen(SL_ram.last->msg) + 3 + 1 + ram_entry_header_len);
        new = (SL_ram_entry_t *)((uchar *)SL_ram.last + n - (n & 3));

        /* Check if log is full */
        if ((&SL_ram.log[SYSLOG_RAM_SIZE] - (uchar *)new) < (int)sizeof(SL_ram_entry_t)) {
            new = (SL_ram_entry_t *)&SL_ram.log[0];
            buf_full = TRUE;
        }

        /* Move 'SL_ram.first' flag for saving the new message */
        if (buf_full || ((int)((uchar *)SL_ram.first - (uchar *)SL_ram.last) > 0)) {
            temp_msg = VTSS_MALLOC(SYSLOG_RAM_MSG_MAX);
            if (temp_msg) {
                va_start(args, fmt);
                (void)vsnprintf(temp_msg, SYSLOG_RAM_MSG_MAX, fmt, args);
                va_end(args);

                /* Use 4-byte aligned length, need to consider one byye of '\0' */
                temp_msg_len = (strlen(temp_msg) + 3 + 1 + ram_entry_header_len);
                temp_msg_len = temp_msg_len - (temp_msg_len & 3);
            } else {
                SYSLOG_RAM_CRIT_EXIT();
                return;
            }
            while (SL_ram.first && ((int)((uchar *)new + temp_msg_len - (uchar *)SL_ram.first) > 0)) {
                if (SL_ram.stat[VTSS_ISID_LOCAL].count[SL_ram.first->lvl]) {
                    SL_ram.stat[VTSS_ISID_LOCAL].count[SL_ram.first->lvl]--;
                }
                if (SL_ram.first->next && (int)((uchar *)SL_ram.first - (uchar *)SL_ram.first->next) > 0) { // first flag wrap-around
                    SL_ram.first = SL_ram.first->next;
                    break;
                }
                SL_ram.first = SL_ram.first->next;
            }
        }

        if (SL_ram.current_id == SYSLOG_RAM_MSG_ID_MAX) { // syslog ID wrap-around
            new->id = SL_ram.current_id = 1;
            while (SL_ram.first && SL_ram.first->id <= new->id) {
                if (SL_ram.stat[VTSS_ISID_LOCAL].count[SL_ram.first->lvl]) {
                    SL_ram.stat[VTSS_ISID_LOCAL].count[SL_ram.first->lvl]--;
                }
                SL_ram.first = SL_ram.first->next;
            }
        } else {
            new->id = ++SL_ram.current_id;
        }

        SL_ram.last->next = new;
        if (SL_ram.first == NULL) {
            SL_ram.first = new;
        }
    }

    /* Store entry data */
    SL_ram.stat[VTSS_ISID_LOCAL].count[lvl]++;
    SL_ram.last = new;
    new->next = NULL;
    new->lvl = lvl;
    new->mid = mid;
    new->time = msg_uptime_get(VTSS_ISID_LOCAL);
    if (temp_msg) {
        strcpy(new->msg, temp_msg);
        VTSS_FREE(temp_msg);
    } else {
        va_start(args, fmt);
        (void)vsnprintf(new->msg, SYSLOG_RAM_MSG_MAX, fmt, args);
        va_end(args);
    }

#if defined(SYSLOG_RAM_MSG_ENTRY_CNT_MAX)
    /* Limit the max log entry number */
    {
        ulong total_count = 0;
        for (n = SYSLOG_LVL_INFO; n < SYSLOG_LVL_ALL; n++) {
            total_count += SL_ram.stat[VTSS_ISID_LOCAL].count[n];
        }
        while (total_count > SYSLOG_RAM_MSG_ENTRY_CNT_MAX) {
            SL_ram.stat[VTSS_ISID_LOCAL].count[SL_ram.first->lvl]--;
            SL_ram.first = SL_ram.first->next;
            total_count--;
        }
    }
#endif /* SYSLOG_RAM_MSG_CNT_MAX */

    SYSLOG_RAM_CRIT_EXIT();
}

/* Clear RAM system log */
void syslog_ram_clear(vtss_isid_t isid, syslog_lvl_t lvl)
{
    SL_msg_req_t *req;

    if (isid == VTSS_ISID_LOCAL) {
        SYSLOG_RAM_CRIT_ENTER();
        SL_ram_clear(lvl);
        SYSLOG_RAM_CRIT_EXIT();
    } else {
        req = SL_msg_req_alloc(SL_MSG_ID_CLR_REQ);
        req->data.entry_clear.lvl = lvl;
        SL_msg_tx(req, isid, sizeof(req->data.entry_clear));
        VTSS_OS_MSLEEP(100); // Delay for sending the clear message
    }

    SYSLOG_CRIT_ENTER();
    SYSLOG_global.send_msg_id[isid] = 0;
    SYSLOG_CRIT_EXIT();
}

/* Get RAM system log entry.
   Note: The newest log can over-write the oldest log when syslog buffer full.
 */
BOOL syslog_ram_get(vtss_isid_t        isid,    /* ISID */
                    BOOL               next,    /* Next or specific entry */
                    ulong              id,      /* Entry ID */
                    syslog_lvl_t       lvl,     /* SYSLOG_LVL_ALL is wildcard */
                    vtss_module_id_t   mid,     /* VTSS_MODULE_ID_NONE is wildcard */
                    syslog_ram_entry_t *entry)  /* Returned data */
{
    return SL_ram_get(isid, next, id, lvl, mid, entry, 1);
}

/* Get RAM system log statistics */
vtss_rc syslog_ram_stat_get(vtss_isid_t isid, syslog_ram_stat_t *stat)
{
    vtss_rc rc = VTSS_OK;

    if (isid == VTSS_ISID_LOCAL) {
        SYSLOG_RAM_CRIT_ENTER();
        *stat = SL_ram.stat[VTSS_ISID_LOCAL];
        SYSLOG_RAM_CRIT_EXIT();
    } else {
        rc = SL_ram_stat_get(isid, stat);
    }
    return rc;
}

/*--------------------------------------------------------------------------*/

/*  Default and configuration changed functions                             */
/****************************************************************************/

/* Determine if syslog configuration has changed */
static int SL_conf_changed(syslog_conf_t *old, syslog_conf_t *new)
{
    return (memcmp(new, old, sizeof(*new)));
}

/* Set syslog defaults */
static void SL_default_set(syslog_conf_t *conf)
{
    conf->server_mode = SYSLOG_MGMT_DEFAULT_MODE;
    memset(conf->syslog_server, 0, sizeof(conf->syslog_server));
    conf->udp_port = SYSLOG_MGMT_DEFAULT_UDP_PORT;
    conf->syslog_level = SYSLOG_MGMT_DEFAULT_SYSLOG_LVL;
}

/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

/* Set stack SYSLOG configuration */
static void SL_stack_conf_set(vtss_isid_t isid_add)
{
    SL_msg_req_t    *msg;
    vtss_isid_t     isid;

    T_D("enter, isid_add: %d", isid_add);

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) || (!msg_switch_exists(isid))) {
            continue;
        }
        msg = SL_msg_req_alloc(SL_MSG_ID_CONF_SET_REQ);
        SYSLOG_CRIT_ENTER();
        msg->data.conf_set.conf = SYSLOG_global.conf;
        SYSLOG_global.send_msg_id[isid] = 0;
        SYSLOG_CRIT_EXIT();
        SL_msg_tx(msg, isid, sizeof(msg->data.conf_set.conf));
    }

    T_D("exit, isid_add: %d", isid_add);
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* SYSLOG error text */
char *syslog_error_txt(vtss_rc rc)
{
    switch (rc) {
    case SYSLOG_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case SYSLOG_ERROR_ISID:
        return "Invalid Switch ID";

    case SYSLOG_ERROR_INV_PARAM:
        return "Invalid parameter supplied to function";

    default:
        return "SYSLOG: Unknown error code";
    }
}

/* Get SYSLOG configuration */
vtss_rc syslog_mgmt_conf_get(syslog_conf_t *glbl_cfg)
{
    T_D("enter");

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return SYSLOG_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return SYSLOG_ERROR_MUST_BE_MASTER;
    }

    SYSLOG_CRIT_ENTER();
    *glbl_cfg = SYSLOG_global.conf;
    SYSLOG_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set SYSLOG configuration */
vtss_rc syslog_mgmt_conf_set(syslog_conf_t *glbl_cfg)
{
    vtss_rc rc      = VTSS_OK;
    int     changed = 0;

    T_D("enter, server_mode: %d", glbl_cfg->server_mode);

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return SYSLOG_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return SYSLOG_ERROR_MUST_BE_MASTER;
    }

    /* Check illegal parameter */
    if (glbl_cfg->server_mode != SYSLOG_MGMT_ENABLED && glbl_cfg->server_mode != SYSLOG_MGMT_DISABLED) {
        return SYSLOG_ERROR_INV_PARAM;
    }
    if (glbl_cfg->syslog_level >= SYSLOG_LVL_ALL) {
        return SYSLOG_ERROR_INV_PARAM;
    }
    if (glbl_cfg->syslog_server[0] != '\0' && misc_str_is_hostname(glbl_cfg->syslog_server) != VTSS_OK) {
        return SYSLOG_ERROR_INV_PARAM;
    }

    SYSLOG_CRIT_ENTER();
    changed = SL_conf_changed(&SYSLOG_global.conf, glbl_cfg);
    SYSLOG_global.conf = *glbl_cfg;
    if (changed && SYSLOG_global.conf.server_mode == SYSLOG_MGMT_ENABLED) {
        /* Update current timer */
        SYSLOG_global.current_time = SL_get_time_in_secs() > (NTP_DELAY_SEC + SL_THREAD_DELAY_SEC) ? SL_get_time_in_secs() : 0;
    }
    SYSLOG_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t     blk_id = CONF_BLK_SYSLOG_CONF;
        syslog_conf_blk_t *syslog_conf_blk_p;
        if ((syslog_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open SYSLOG table");
        } else {
            syslog_conf_blk_p->conf = *glbl_cfg;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        /* Activate changed configuration */
        SL_stack_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");
    return rc;
}

void syslog_mgmt_default_get(syslog_conf_t *glbl_cfg)
{
    SL_default_set( glbl_cfg);
}
/****************************************************************************
 * SYSLOG thread
 ****************************************************************************/

static void SL_thread(cyg_addrword_t data)
{
    int                 sock;
    struct sockaddr_in  server_addr;
    static char         send_data[SYSLOG_RAM_MSG_MAX + 200];
    vtss_isid_t         isid;
    syslog_ram_stat_t   stat;
    syslog_ram_entry_t  entry;
    syslog_lvl_t        level;
    struct hostent      *host;
    struct in_addr      address;
    BOOL                server_mode, link_down_first_round = TRUE;
    time_t              current_time;

#define SRC_IP_BUF_SIZE 41
    struct sockaddr_in  my_addr;
    socklen_t           my_addr_len;
    char                my_addr_buf[SRC_IP_BUF_SIZE];

    SYSLOG_CRIT_ENTER();
    SYSLOG_global.current_time = 0;
    SYSLOG_CRIT_EXIT();

    /* Wait for SNTP/NTP process */
    VTSS_OS_MSLEEP(NTP_DELAY_SEC * 1000);

    while (1) {
        if (msg_switch_is_master()) {
            /* Process the task every 2 seconds */
            VTSS_OS_MSLEEP(SL_THREAD_DELAY_SEC * 1000);

            SYSLOG_CRIT_ENTER();
            server_mode = SYSLOG_global.conf.server_mode && SYSLOG_global.conf.syslog_server[0] != '\0';
            current_time = SYSLOG_global.current_time;
            SYSLOG_CRIT_EXIT();

            /* Do nothing when server mode is disabled or syslog server isn't configured */
            if (!server_mode) {
                continue;
            }

            SYSLOG_CRIT_ENTER();
            level = SYSLOG_global.conf.syslog_level;

            /* Fill server address information */
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(SYSLOG_global.conf.udp_port);

            /* Look up the DNS host name or IPv4 address */
            if ((server_addr.sin_addr.s_addr = inet_addr(SYSLOG_global.conf.syslog_server)) == (unsigned long) - 1) {
                if ((host = gethostbyname(SYSLOG_global.conf.syslog_server)) != NULL &&
                    (host->h_length == sizeof(struct in_addr))) {
                    address = *((struct in_addr **)host->h_addr_list)[0];
                    server_addr.sin_addr.s_addr = address.s_addr;
                } else {
                    T_D("gethostbyname() fail: %s\n", SYSLOG_global.conf.syslog_server);
                }
            }
            SYSLOG_CRIT_EXIT();
            if (server_addr.sin_addr.s_addr == (unsigned long) - 1 || server_addr.sin_addr.s_addr == 0) {
                continue;
            }

            /* Create socket */
            if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                T_D("Create syslog socket failed: %s", strerror(errno));
                continue;
            }

            /* Connect socket */
            if (connect(sock, (struct sockaddr *)&server_addr,
                        sizeof(struct sockaddr)) != 0) {
                close(sock);
                T_D("Connect syslog socket failed: %s", strerror(errno));
                continue;
            }

            /* Get my address */
            my_addr_len = sizeof(my_addr);
            if (getsockname(sock, (struct sockaddr *)&my_addr,
                            &my_addr_len) != 0) {
                close(sock);
                T_D("Get syslog my address sockname failed: %s", strerror(errno));
                continue;
            }

            /* Convert to my address string */
            my_addr_buf[0] = '\0';
            (void) inet_ntop(my_addr.sin_family,
                             (const char *)&my_addr.sin_addr,
                             my_addr_buf, SRC_IP_BUF_SIZE);


            /* Get syslog messages */
            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
                if (!msg_switch_exists(isid) || syslog_ram_stat_get(isid, &stat) != VTSS_OK) {
                    continue;
                }

                SYSLOG_CRIT_ENTER();
                entry.id = SYSLOG_global.send_msg_id[isid];
                SYSLOG_CRIT_EXIT();

                while (syslog_ram_get(isid, TRUE, entry.id, SYSLOG_LVL_ALL, VTSS_MODULE_ID_NONE, &entry)) {
                    if (entry.lvl >= level && entry.time >= current_time) {
                        ssize_t     msg_size;
                        ssize_t     send_rc;
                        u8          pri;
                        char        buf2[32], *link_down_str = "Link down";

                        /* To avoid missing the link-down log, we will send it on the second round */
                        if (strncmp(entry.msg, link_down_str, strlen(link_down_str)) == 0) {
                            if (link_down_first_round) {
                                link_down_first_round = FALSE;
                                break;
                            }
                            link_down_first_round = TRUE;
                        }

                        /* Calculate PRI field */
                        pri = 1 << 3; // The facility code value will always equal 1 (user-level message)
                        pri += (entry.lvl == SYSLOG_LVL_ERROR ? 3 : (entry.lvl == SYSLOG_LVL_WARNING ? 4 : 6)); //Mapping to the severity codes: 3(Error), 4(Warning) and 6(Information)

#if VTSS_SWITCH_STACKABLE
                        sprintf(buf2, "%s usid=\"%d\"", VTSS_PRODUCT_NAME, topo_isid2usid(isid));
#else
                        sprintf(buf2, "%s", VTSS_PRODUCT_NAME);
#endif /* VTSS_SWITCH_STACKABLE */

                        /* Fill syslog packet contents: HEADER + STRUCTURED-DATA
                           HEADER: <PRI> VERSION TIMESTAMP HOSTNAME APP-NAME RPOCID MSGID
                           STRUCTURED-DATA: [SD-ELEMENT SD-ID SD-PARM]

                           Notice:
                           HEADER: RPOCID and TRUCTURED-DATA: SD-ID aren't used in our system currently.

                           Example:
                           <14>1 2011-01-14T14:24:00Z+00:00 10.9.52.169 vtss_syslog - ID1
                           [SMBStaX usid="1"]
                           Switch just made a cold boot.
                         */
                        sprintf(send_data, "<%d>1 %s %s syslog - ID%d [%s] %s",
                                pri,
                                misc_time2str(entry.time),
                                my_addr_buf,
                                entry.id,
                                buf2,
                                entry.msg);

                        /* Send out packet to syslog server */
                        msg_size = strlen(send_data);
                        send_rc = send(sock, send_data, msg_size, 0);

                        if (send_rc < 0) {
                            T_D("Send syslog socket failed: %s", strerror(errno));
                            break;  // try it on the next cycle.
                        } else if (send_rc != msg_size) {
                            T_D("Send syslog socket failed: sendout length is short - sendout length = %lu, expected lenght = %lu", send_rc, msg_size);
                            break;  // try it on the next cycle.
                        }
                    }

                    /* Update send_msg_id */
                    SYSLOG_CRIT_ENTER();
                    SYSLOG_global.send_msg_id[isid] = entry.id;
                    SYSLOG_CRIT_EXIT();
                }
            }

            /* Close socket */
            close(sock);
        } else {
            /* Suspend SYSLOG thread (became master) */
            cyg_thread_suspend(SYSLOG_thread_handle);
        }
    }
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create SYSLOG stack configuration */
static void SL_conf_read_stack(BOOL create)
{
    int                 changed;
    BOOL                do_create;
    u32                 size;
    syslog_conf_t       *old_syslog_conf_p, new_syslog_conf;
    syslog_conf_blk_t   *conf_blk_p;
    conf_blk_id_t       blk_id;
    u32                 blk_version;

    T_D("enter, create: %d", create);

    /* Read/create SYSLOG configuration */
    blk_id = CONF_BLK_SYSLOG_CONF;
    blk_version = SYSLOG_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = 1;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = 1;
    }

    changed = 0;
    SYSLOG_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        SL_default_set(&new_syslog_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->conf = new_syslog_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            new_syslog_conf = conf_blk_p->conf;
        }
    }
    old_syslog_conf_p = &SYSLOG_global.conf;
    changed = SL_conf_changed(old_syslog_conf_p, &new_syslog_conf);
    SYSLOG_global.conf = new_syslog_conf;
    if (changed && SYSLOG_global.conf.server_mode == SYSLOG_MGMT_ENABLED) {
        /* Update current timer */
        SYSLOG_global.current_time = SL_get_time_in_secs();
    }
    SYSLOG_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open SYSLOG table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) {
        SL_stack_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");
}

/* Module start */
static void SL_start(void)
{
    syslog_conf_t *conf_p;

    T_D("enter");

    /* Initialize SYSLOG configuration */
    conf_p = &SYSLOG_global.conf;
    SL_default_set(conf_p);

    /* Initialize msg_id */
    memset(SYSLOG_global.send_msg_id, 0, sizeof(SYSLOG_global.send_msg_id));

    /* Create semaphore for critical regions */
    critd_init(&SYSLOG_global.crit, "SYSLOG_global.crit", VTSS_MODULE_ID_SYSLOG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    SYSLOG_CRIT_EXIT();

    /* Create SYSLOG thread */
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      SL_thread,
                      0,
                      "syslog",
                      SYSLOG_thread_stack,
                      sizeof(SYSLOG_thread_stack),
                      &SYSLOG_thread_handle,
                      &SYSLOG_thread_block);

    T_D("exit");
}

/****************************************************************************/
// syslog_init()
/****************************************************************************/
vtss_rc syslog_init(vtss_init_data_t *data)
{
    vtss_rc rc = VTSS_OK;
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        SL_start();
#ifdef VTSS_SW_OPTION_VCLI
        syslog_cli_req_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        (void) syslog_icfg_init();
#endif
        /* Initialize RAM log */
        SL_ram_init(1);

        // Since several modules may call the logging function at a time, we need to protect it with a critical region.
        critd_init(&SL_flash_crit, "syslog_flash.crit", VTSS_MODULE_ID_SYSLOG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        // We can safely release it now, since all public syslog functions check for SL_flash_enabled
        // prior to taking the semaphore.
        SYSLOG_FLASH_CRIT_EXIT();

        // Hook into the VTSS_ASSERT macro.
        vtss_common_assert_cb_set(SL_assert_cb);
        break;

    case INIT_CMD_START:
        T_D("START");

        /* Initialize RAM log */
        SL_ram_init(0);

        // Open the flash syslog and make it ready for reading and writing.
        SL_flash_open();

        SYSLOG_RAM_CRIT_ENTER();
        SYSLOG_init = TRUE;
        SYSLOG_RAM_CRIT_EXIT();
        break;

    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            SL_conf_read_stack(1);
        }
        break;

    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");
        /* Read stack and switch configuration */
        SL_conf_read_stack(0);

        /* Update current timer */
        SYSLOG_CRIT_ENTER();
        SYSLOG_global.current_time = SL_get_time_in_secs();
        SYSLOG_CRIT_EXIT();

        /* Starting SYSLOG thread */
        cyg_thread_resume(SYSLOG_thread_handle);
        break;
    }

    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        SL_stack_conf_set(isid);
        break;

    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;

    default:
        break;
    }

    T_D("exit");
    return rc;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
