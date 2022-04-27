/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "icfg.h"
#include "icfg_api.h"
#include "msg_api.h"
#include "port_api.h" /* For port_no_is_stack() */
#include "critd_api.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_TIMER
#include "vtss_timer_api.h"
#endif

#ifdef VTSS_SW_OPTION_VLAN
#include "vlan_api.h"
#endif

#ifdef VTSS_SW_OPTION_RFC2544
#include "rfc2544_api.h"
#endif

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cyg/compress/zlib.h>
#if defined(CYGPKG_FS_RAM)
#include "os_file_api.h"
#endif
#include "conf_api.h"

#if defined(VTSS_SW_OPTION_IP2)
#include "ip2_api.h"
#endif /* VTSS_SW_OPTION_IP2 */

#include "mgmt_api.h"

#if defined(VTSS_SW_OPTION_IPMC)
#include "ipmc_api.h"
#endif /* VTSS_SW_OPTION_IPMC */

#if defined(VTSS_SW_OPTION_IPMC_LIB)
#include "ipmc_lib.h"
#endif /* VTSS_SW_OPTION_IPMC_LIB */

#if defined(VTSS_SW_OPTION_SNMP)
#include "vtss_snmp_api.h"
#endif /* VTSS_SW_OPTION_SNMP */

#if defined(VTSS_SW_OPTION_DHCP_SERVER)
#include "dhcp_server_api.h"
#endif



#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_ICFG

#define VTSS_RC(expr)   { vtss_rc __rc__ = (expr); if (__rc__ < VTSS_RC_OK) return __rc__; }
#define ICLI_RC(expr)   { i32     __rc__ = (expr); if (__rc__ < ICLI_RC_OK) return VTSS_RC_ERROR; }

// Allocation quant for query results
#define DEFAULT_TEXT_BLOCK_SIZE (1024*1024)

// Size of static buffer for ICLI command output
#define OUTPUT_BUFFER_SIZE      (4*1024)

// Maximum number of syntax errors to report when validating a configuration
#define MAX_ERROR_CNT   (5)



/* Map: Mode => { first, last } */
typedef struct {
    icli_cmd_mode_t      mode;
    vtss_icfg_ordering_t first;
    vtss_icfg_ordering_t last;
} icfg_map_t;

/* Adapt this table when new modes are introduced: */
static const icfg_map_t mode_to_order_map[] = {
    { ICLI_CMD_MODE_CONFIG_VLAN,         VTSS_ICFG_VLAN_BEGIN,               VTSS_ICFG_VLAN_END               },
#if defined(VTSS_SW_OPTION_IPMC_LIB)
    { ICLI_CMD_MODE_IPMC_PROFILE,        VTSS_ICFG_IPMC_BEGIN,               VTSS_ICFG_IPMC_END               },
#endif /* VTSS_SW_OPTION_IPMC_LIB */
#if defined(VTSS_SW_OPTION_RFC2544)
    { ICLI_CMD_MODE_RFC2544_PROFILE,     VTSS_ICFG_RFC2544_BEGIN,            VTSS_ICFG_RFC2544_END            },
#endif /* VTSS_SW_OPTION_RFC2544 */
#if defined(VTSS_SW_OPTION_SNMP)
    { ICLI_CMD_MODE_SNMPS_HOST,          VTSS_ICFG_SNMPSERVER_HOST_BEGIN,    VTSS_ICFG_SNMPSERVER_HOST_END    },
#endif /* VTSS_SW_OPTION_SNMP */
    { ICLI_CMD_MODE_INTERFACE_PORT_LIST, VTSS_ICFG_INTERFACE_ETHERNET_BEGIN, VTSS_ICFG_INTERFACE_ETHERNET_END },
    { ICLI_CMD_MODE_INTERFACE_VLAN,      VTSS_ICFG_INTERFACE_VLAN_BEGIN,     VTSS_ICFG_INTERFACE_VLAN_END     },
    { ICLI_CMD_MODE_STP_AGGR,            VTSS_ICFG_STP_AGGR_BEGIN,           VTSS_ICFG_STP_AGGR_END           },
    { ICLI_CMD_MODE_CONFIG_LINE,         VTSS_ICFG_LINE_BEGIN,               VTSS_ICFG_LINE_END               },
#if defined(VTSS_SW_OPTION_DHCP_SERVER)
    { ICLI_CMD_MODE_DHCP_POOL,           VTSS_ICFG_DHCP_POOL_BEGIN,          VTSS_ICFG_DHCP_POOL_END          },
#endif
};

#define MAP_TABLE_CNT   (sizeof(mode_to_order_map)/sizeof(icfg_map_t))



typedef struct {
    vtss_icfg_query_func_t  func;
    const char              *feature_name;
} icfg_callback_data_t;

// List of callbacks. We waste a few entries for the ...BEGIN and ...END values,
// but that's so little we don't want to exchange it for more complex code.
static icfg_callback_data_t icfg_callbacks[VTSS_ICFG_LAST];



// Critical regions
static critd_t icfg_crit;       // General access to data


// ICFG thread.

#define ICFG_THREAD_FLAG_COMMIT_FILE            VTSS_BIT(0)    // Begin commit
#define ICFG_THREAD_FLAG_COMMIT_DONE            VTSS_BIT(1)    // Commit completed
#define ICFG_THREAD_FLAG_SYNTH_STARTUP_CONFIG   VTSS_BIT(2)    // Synthesize STARTUP-CONFIG (for Silent Upgrade only)

static cyg_handle_t             icfg_thread_handle;
static cyg_thread               icfg_thread_block;
static char                     icfg_thread_stack[8 * THREAD_DEFAULT_STACK_SIZE];
static cyg_flag_t               icfg_thread_flag;

// Shared between threads; protected by icfg_crit.
static char                     icfg_commit_filename[PATH_MAX];     // [0] != 0 => load is initiated; == 0 => ready for next file
static vtss_icfg_query_result_t icfg_commit_buf;
static BOOL                     icfg_commit_abort_flag;
static char                     icfg_commit_output_buffer[OUTPUT_BUFFER_SIZE];
static BOOL                     icfg_commit_echo_to_console;
static u32                      icfg_commit_output_buffer_length;
static u32                      icfg_commit_error_cnt;
static BOOL                     icfg_commit_running;
static BOOL                     icfg_silent_upgrade_flash_empty;
static BOOL                     icfg_silent_upgrade_active;
static BOOL                     icfg_silent_upgrade_invoked;

#if defined(VTSS_SW_OPTION_IP2)
static struct {
    struct {
        vtss_ip_conf_t          ip;
        vtss_routing_entry_t    route;
        BOOL                    ip_ok;
        BOOL                    r_ok;
    } v4, v6;
} icfg_ip_save;
#endif /* defined(VTSS_SW_OPTION_IP2) */

// State for protecting whole-file load/save ops: Only one can be in
// progress at any time; overlapping requests must be denied with an error
// message.
static BOOL                     icfg_io_in_progress;



#if VTSS_SWITCH_STACKABLE
// Stacking + silent upgrade: Wait-for-slave timer. Started at end of late INIT_CMD_MASTER_UP,
// reset at each subsequent INIT_CMD_SWITCH_ADD, unless it has expired in advance. Once expired,
// silent upgrade completion takes place (deletion of 'conf' blocks, generation of STARTUP_CONFIG).
static vtss_timer_t icfg_slave_wait_timer;

#define SLAVE_WAIT_SECS     30
#endif


// Well-known file names
#define STARTUP_CONFIG "startup-config"
#define DEFAULT_CONFIG "default-config"

/* In order to conserve RAM memory all ICFG-related file writing and reading must take place
 * via the functions icfg_file_read() and icfg_file_write(). Those functions will decompress/
 * compress the file being read/written on the fly. This reduces the RAM use for the RAM file
 * system and it reduces the allocation of temporary memory by 'conf'; in both cases to
 * roughly the size of the compressed data. Since a 2MB text may compress to as little as
 * 30kB or so this is a big win: We save 2*(2MB-30kB).
 *
 * In order for this to work we also provide our own version of 'stat()', icfg_file_stat().
 * It must *always* be used when the attributes of a file -- particularly size -- are being
 * determined.
 *
 * In an ideal world this would all be handled transparently by the RAM file system. But
 * such is not the eCos world.
 *
 * Since we need to maintain backwards compatibility with previous software releases that
 * don't support this, we adopt this scheme:
 *
 *   - When a file is accessed for reading/stat'ing, we try to read a magic header.
 *      - If this fails, the file is not compressed and we proceed as usual.
 *      - If the header could be read and contains the valid magic, the uncompressed length
 *        of the file is retrieved and used henceforth.
 *   - When a file is being written we'll write the magic header at the very beginning of
 *     the file and then compress the data and append it. This works because we only allow
 *     complete (re-)writes of files with icfg_file_write().
 */

// Header for files that are stored zipped in the FS
#define ZIP_HEADER_MAGIC        "ICFG-zip-\xff\x00\x80"
#define ZIP_HEADER_MAGIC_SIZE   12
typedef struct {
    char    magic[ZIP_HEADER_MAGIC_SIZE];
    u32     size;                           // Uncompressed length of file
} icfg_zip_header_t;



#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "icfg",
    .descr     = "Industrial Configuration Engine"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_INFO,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define ICFG_CRIT_ENTER()    critd_enter(&icfg_crit,    TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define ICFG_CRIT_EXIT()     critd_exit( &icfg_crit,    TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define ICFG_CRIT_ENTER()    critd_enter(&icfg_crit)
#define ICFG_CRIT_EXIT()     critd_exit( &icfg_crit)
#endif /* VTSS_TRACE_ENABLED */


//-----------------------------------------------------------------------------
// Query Registration
//-----------------------------------------------------------------------------

vtss_rc vtss_icfg_query_register(vtss_icfg_ordering_t   order,
                                 const char *const      feature_name,
                                 vtss_icfg_query_func_t query_cb)
{
    /*lint --e{459} */
    vtss_rc rc = VTSS_RC_OK;

    if (order >= VTSS_ICFG_LAST  ||  icfg_callbacks[order].func != NULL) {
        rc = VTSS_RC_ERROR;
    } else {
        icfg_callbacks[order].func         = query_cb;
        icfg_callbacks[order].feature_name = feature_name;
    }

    return rc;
}



//-----------------------------------------------------------------------------
// Query Request
//-----------------------------------------------------------------------------

static vtss_rc icfg_format_mode_header(vtss_icfg_query_request_t *req, vtss_icfg_query_result_t  *result)
{
    switch (req->cmd_mode) {
    case ICLI_CMD_MODE_CONFIG_VLAN:
        T_N(                             "vlan %u",   req->instance_id.vlan);
        VTSS_RC(vtss_icfg_printf(result, "vlan %u\n", req->instance_id.vlan));
        break;

#if defined(VTSS_SW_OPTION_IPMC_LIB)
    case ICLI_CMD_MODE_IPMC_PROFILE:
        T_N(                             "ipmc profile %s",   req->instance_id.string);
        VTSS_RC(vtss_icfg_printf(result, "ipmc profile %s\n", req->instance_id.string));
        break;
#endif /* VTSS_SW_OPTION_IPMC_LIB */

#if defined(VTSS_SW_OPTION_RFC2544)
    case ICLI_CMD_MODE_RFC2544_PROFILE:
        T_N(                             "rfc2544 profile %s",   req->instance_id.string);
        VTSS_RC(vtss_icfg_printf(result, "rfc2544 profile %s\n", req->instance_id.string));
        break;
#endif /* VTSS_SW_OPTION_RFC2544 */

#if defined(VTSS_SW_OPTION_SNMP)
    case ICLI_CMD_MODE_SNMPS_HOST:
        T_N(                             "snmp-server host %s",   req->instance_id.string);
        VTSS_RC(vtss_icfg_printf(result, "snmp-server host %s\n", req->instance_id.string));
        break;
#endif /* VTSS_SW_OPTION_SNMP */

    case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
        T_N("interface %s %u/%u",
            icli_port_type_get_name(req->instance_id.port.port_type),
            req->instance_id.port.switch_id,
            req->instance_id.port.begin_port);
        VTSS_RC(vtss_icfg_printf(result, "interface %s %u/%u\n",
                                 icli_port_type_get_name(req->instance_id.port.port_type),
                                 req->instance_id.port.switch_id,
                                 req->instance_id.port.begin_port));
        break;

    case ICLI_CMD_MODE_INTERFACE_VLAN:
        T_N(                             "interface vlan %u",   req->instance_id.vlan);
        VTSS_RC(vtss_icfg_printf(result, "interface vlan %u\n", req->instance_id.vlan));
        break;

    case ICLI_CMD_MODE_STP_AGGR:
        T_N(                             "spanning-tree aggregation");
        VTSS_RC(vtss_icfg_printf(result, "spanning-tree aggregation\n"));
        break;

    case ICLI_CMD_MODE_CONFIG_LINE:
        if (req->instance_id.line == 0) {
            T_N(                             "line console 0");
            VTSS_RC(vtss_icfg_printf(result, "line console 0\n"));
        } else {
            T_N(                             "line vty %u",   req->instance_id.line - 1);
            VTSS_RC(vtss_icfg_printf(result, "line vty %u\n", req->instance_id.line - 1));
        }
        break;

#if defined(VTSS_SW_OPTION_DHCP_SERVER)
    case ICLI_CMD_MODE_DHCP_POOL:
        T_N(                             "ip dhcp pool %s",   req->instance_id.string);
        VTSS_RC(vtss_icfg_printf(result, "ip dhcp pool %s\n", req->instance_id.string));
        break;
#endif

    default:
        // Ignore
        break;
    }

    return VTSS_RC_OK;

}

static vtss_rc icfg_iterate_callbacks(vtss_icfg_query_request_t *req,
                                      vtss_icfg_ordering_t      first,
                                      vtss_icfg_ordering_t      last,
                                      const char *const        feature_name,
                                      vtss_icfg_query_result_t  *result)
{
    vtss_rc                rc = VTSS_RC_OK;
    vtss_icfg_query_func_t f;

    T_N("Beginning iteration, [%d;%d[", first, last);
    ICFG_CRIT_ENTER();
    for (; (first < last)  &&  (rc == VTSS_RC_OK); ++first) {
        BOOL invoke = icfg_callbacks[first].func != NULL;
        if (feature_name != NULL) {
            invoke = invoke  &&  (icfg_callbacks[first].feature_name != NULL)  &&  !strcmp(feature_name, icfg_callbacks[first].feature_name);
        }

        if (invoke) {
            T_N("Invoking callback, order %d", first);
            req->order = first;
            f = icfg_callbacks[first].func;
            ICFG_CRIT_EXIT();
            rc = (f)(req, result);
            T_N("Callback done, rc %d", rc);
            if (rc != VTSS_RC_OK) {
                T_D("ICFG Synth. failure for order %d, rc = %d", first, rc);
                rc = VTSS_RC_OK;
            }
            ICFG_CRIT_ENTER();
        }
    }
    ICFG_CRIT_EXIT();
    T_N("Iteration done, rc %d", rc);

    return rc;
}

#define ICFG_VTSS_RC(x) {                          \
    vtss_rc rc = (x);                              \
    if (rc != VTSS_RC_OK) {                        \
        T_D("ICFG Synth. failure, rc = %d", rc);   \
    }                                              \
}

static vtss_rc icfg_process_entity(vtss_icfg_query_request_t *req,
                                   vtss_icfg_ordering_t       first,
                                   vtss_icfg_ordering_t       last,
                                   const char *const          feature_name,
                                   vtss_icfg_query_result_t   *result)
{
    switch (req->cmd_mode) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG:
    case ICLI_CMD_MODE_CONFIG_VLAN:
    case ICLI_CMD_MODE_RFC2544_PROFILE:
    case ICLI_CMD_MODE_SNMPS_HOST:
    case ICLI_CMD_MODE_IPMC_PROFILE:
    case ICLI_CMD_MODE_STP_AGGR:
    case ICLI_CMD_MODE_CONFIG_LINE:
    case ICLI_CMD_MODE_DHCP_POOL:
        ICFG_VTSS_RC(icfg_format_mode_header(req, result));
        ICFG_VTSS_RC(icfg_iterate_callbacks(req, first, last, feature_name, result));
        ICFG_VTSS_RC(vtss_icfg_printf(result, VTSS_ICFG_COMMENT_LEADIN"\n"));
        break;

    case ICLI_CMD_MODE_INTERFACE_PORT_LIST: {
        ICFG_VTSS_RC(icfg_format_mode_header(req, result));

        if (port_isid_port_no_is_stack(req->instance_id.port.isid, req->instance_id.port.begin_iport)) {
            ICFG_VTSS_RC(vtss_icfg_printf(result, " " VTSS_ICFG_COMMENT_LEADIN " (Stack interface)\n"));
        } else {
            ICFG_VTSS_RC(icfg_iterate_callbacks(req, first, last, feature_name, result));
        }

        ICFG_VTSS_RC(vtss_icfg_printf(result, VTSS_ICFG_COMMENT_LEADIN"\n"));
    }
    break;

    case ICLI_CMD_MODE_INTERFACE_VLAN: {
        BOOL       do_cb = FALSE;
        vtss_vid_t vidx = req->instance_id.vlan;

        /*
            check if the VLAN interface is enabled or not
            if it is disabled then all configurations should be hidden
            that is why break
        */
        BOOL    bb;

        if ( icli_vlan_enable_get(vidx, &bb) == ICLI_RC_OK && bb ) {
            if ( icli_vlan_enter_get(vidx, &bb) == ICLI_RC_OK && bb ) {
                do_cb = TRUE;
            }
        } else {
            break;
        }

#if defined(VTSS_SW_OPTION_IP2)
        do_cb = do_cb || vtss_ip2_if_exists(vidx);
#endif /* VTSS_SW_OPTION_IP2 */

#if defined(VTSS_SW_OPTION_IPMC)
        {
            BOOL       dummy;
            vtss_vid_t idx;

            idx = vidx;
            do_cb = do_cb || ipmc_mgmt_get_intf_state_querier(TRUE, &idx, &dummy, &dummy, FALSE, IPMC_IP_VERSION_IGMP) == VTSS_OK;

            idx = vidx;
            do_cb = do_cb || ipmc_mgmt_get_intf_state_querier(TRUE, &idx, &dummy, &dummy, FALSE, IPMC_IP_VERSION_MLD) == VTSS_OK;
        }
#endif /* VTSS_SW_OPTION_IPMC */

#if defined(VTSS_SW_OPTION_DHCP_SERVER)
        {
            BOOL b_enable;
            do_cb = do_cb || ((dhcp_server_vlan_enable_get(vidx, &b_enable) == DHCP_SERVER_RC_OK) && b_enable);
        }
#endif
        if (do_cb) {
            ICFG_VTSS_RC(icfg_format_mode_header(req, result));
            ICFG_VTSS_RC(icfg_iterate_callbacks(req, first, last, feature_name, result));
            ICFG_VTSS_RC(vtss_icfg_printf(result, VTSS_ICFG_COMMENT_LEADIN"\n"));
        }
    }
    break;

    default:
        T_E("That's really odd; shouldn't get here. Porting issue?");
        break;
    }

    return VTSS_RC_OK;
}

static vtss_rc icfg_process_range(icli_cmd_mode_t          mode,
                                  vtss_icfg_ordering_t     first,
                                  vtss_icfg_ordering_t     last,
                                  BOOL                     all_defaults,
                                  const char *const        feature_name,
                                  vtss_icfg_query_result_t *result)
{
    vtss_icfg_query_request_t req;

    T_N("Entry: mode %d, [%d;%d[", mode, first, last);

    req.cmd_mode     = mode;
    req.all_defaults = all_defaults;

    switch (mode) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG:
    case ICLI_CMD_MODE_STP_AGGR:
        // The above are global/single-instance, so no enumeration is required
        (void) icfg_process_entity(&req, first, last, feature_name, result);
        break;

    case ICLI_CMD_MODE_CONFIG_VLAN: {
#ifdef VTSS_SW_OPTION_VLAN
        vlan_mgmt_entry_t v = { .vid = 0 };
        while (vlan_mgmt_vlan_get(VTSS_ISID_GLOBAL, v.vid, &v, TRUE, VLAN_USER_STATIC) == VTSS_RC_OK) {
            req.instance_id.vlan = v.vid;
            (void) icfg_process_entity(&req, first, last, feature_name, result);
        }
#endif
    }
    break;

    case ICLI_CMD_MODE_RFC2544_PROFILE: {
#ifdef VTSS_SW_OPTION_RFC2544
        char profile_name[RFC2544_PROFILE_NAME_LEN];

        profile_name[0] = '\0';

        while (1) {
            VTSS_RC(rfc2544_mgmt_profile_names_get(profile_name));
            if (profile_name[0] == '\0') {
                // No more profiles.
                break;
            }

            strcpy(req.instance_id.string, profile_name);
            (void) icfg_process_entity(&req, first, last, feature_name, result);
        }
#endif
    }
    break;

    case ICLI_CMD_MODE_SNMPS_HOST: {
#if defined(VTSS_SW_OPTION_SNMP)
        vtss_trap_entry_t trap_entry;

        memset(trap_entry.trap_conf_name, 0, sizeof(trap_entry.trap_conf_name));
        while (VTSS_RC_OK == trap_mgmt_conf_get_next (&trap_entry)) {
            strncpy(req.instance_id.string, trap_entry.trap_conf_name, TRAP_MAX_NAME_LEN);
            req.instance_id.string[TRAP_MAX_NAME_LEN] = 0;
            (void) icfg_process_entity(&req, first, last, feature_name, result);
        }
#endif
    }
    break;

    case ICLI_CMD_MODE_IPMC_PROFILE: {
#if defined(VTSS_SW_OPTION_IPMC_LIB)
        ipmc_lib_profile_mem_t      *pf;
        ipmc_lib_grp_fltr_profile_t *fltr_profile;

        if (!IPMC_MEM_PROFILE_MTAKE(pf)) {
            T_E("Cannot alloc temporary memory");
            return VTSS_RC_ERROR;
        }

        fltr_profile = &pf->profile;
        memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
        while (ipmc_lib_mgmt_fltr_profile_get_next(fltr_profile, TRUE) == VTSS_OK) {
            memset(req.instance_id.string, 0x0, sizeof(req.instance_id.string));
            memcpy(req.instance_id.string, fltr_profile->data.name, sizeof(fltr_profile->data.name));
            (void) icfg_process_entity(&req, first, last, feature_name, result);
        }

        IPMC_MEM_PROFILE_MGIVE(pf);
#endif
    }
    break;

    case ICLI_CMD_MODE_INTERFACE_PORT_LIST: {
        icli_switch_port_range_t switch_range;
        BOOL                     good = icli_port_get_first(&switch_range);

        while (good) {
            req.instance_id.port = switch_range;
            (void) icfg_process_entity(&req, first, last, feature_name, result);
            good = icli_port_get_next(&switch_range);
        }
    }
    break;

    case ICLI_CMD_MODE_INTERFACE_VLAN: {
        vtss_vid_t vidx;
        for (vidx = VTSS_VID_NULL + 1; vidx < VTSS_VIDS; vidx++) {
            req.instance_id.vlan = vidx;
            (void) icfg_process_entity(&req, first, last, feature_name, result);
        }
    }
    break;

    case ICLI_CMD_MODE_CONFIG_LINE: {
        u32                 max = icli_session_max_get();
        u32                 i;
        icli_session_data_t ses;

        for (i = 0; i < max; ++i) {
            ses.session_id = i;
            ICLI_RC(icli_session_data_get(&ses));
            req.instance_id.line = i;
            (void) icfg_process_entity(&req, first, last, feature_name, result);
        }
    }
    break;

    case ICLI_CMD_MODE_DHCP_POOL: {
#if defined(VTSS_SW_OPTION_DHCP_SERVER)
        dhcp_server_pool_t  pool;

        memset(&pool, 0, sizeof(dhcp_server_pool_t));
        while ( dhcp_server_pool_get_next(&pool) == DHCP_SERVER_RC_OK ) {
            memset(req.instance_id.string, 0, sizeof(req.instance_id.string));
            strcpy(req.instance_id.string, pool.pool_name);
            (void) icfg_process_entity(&req, first, last, feature_name, result);
        }
#endif
    }
    break;

    default:
        T_E("That's really odd; shouldn't get here. Porting issue?");
        break;
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_icfg_query_feature(BOOL                     all_defaults,
                                const char *const       feature_name,
                                vtss_icfg_query_result_t *result)
{
    u32                  map_idx = 0;
    vtss_icfg_ordering_t next = 0;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    VTSS_RC(vtss_icfg_init_query_result(0, result));

    while (next < VTSS_ICFG_LAST) {
        if (map_idx < MAP_TABLE_CNT) {
            if (next < mode_to_order_map[map_idx].first) {
                /* Global section prior to/in-between submode(s) */
                VTSS_RC(icfg_process_range(ICLI_CMD_MODE_GLOBAL_CONFIG, next, mode_to_order_map[map_idx].first, all_defaults, feature_name, result));
                next = mode_to_order_map[map_idx].first + 1;
            } else {
                /* Submode */
                VTSS_RC(icfg_process_range(mode_to_order_map[map_idx].mode, next, mode_to_order_map[map_idx].last, all_defaults, feature_name, result));
                next = mode_to_order_map[map_idx].last + 1;
                map_idx++;
            }
        } else {
            /* Global section after last submode */
            VTSS_RC(icfg_process_range(ICLI_CMD_MODE_GLOBAL_CONFIG, next, VTSS_ICFG_LAST, all_defaults, feature_name, result));
            next = VTSS_ICFG_LAST;
        }
    }

    VTSS_RC(vtss_icfg_printf(result, "end\n"));
    return VTSS_RC_OK;
}

vtss_rc vtss_icfg_query_all(BOOL all_defaults, vtss_icfg_query_result_t *result)
{
    return vtss_icfg_query_feature(all_defaults, NULL, result);
}

vtss_rc vtss_icfg_query_specific(vtss_icfg_query_request_t *req,
                                 vtss_icfg_query_result_t  *result)
{
    u32 i;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    for (i = 0; i < MAP_TABLE_CNT  &&  mode_to_order_map[i].mode != req->cmd_mode; ++i) {
        // search
    }

    if (i < MAP_TABLE_CNT) {
        (void) icfg_process_entity(req, mode_to_order_map[i].first + 1, mode_to_order_map[i].last, NULL, result);
    }

    return VTSS_RC_OK;
}



//-----------------------------------------------------------------------------
// Query Result
//-----------------------------------------------------------------------------

static vtss_icfg_query_result_buf_t *icfg_alloc_buf(u32 initial_size)
{
    vtss_icfg_query_result_buf_t *buf;

    if (!initial_size) {
        initial_size = DEFAULT_TEXT_BLOCK_SIZE;
    }

    buf = (vtss_icfg_query_result_buf_t *)VTSS_MALLOC(sizeof(vtss_icfg_query_result_buf_t));

    if (buf) {
        buf->text      = (char *) VTSS_MALLOC(initial_size);
        buf->free_text = TRUE;
        buf->used      = 0;
        buf->size      = initial_size;
        buf->next      = NULL;
    }

    return buf;
}

vtss_rc vtss_icfg_init_query_result(u32 initial_size,
                                    vtss_icfg_query_result_t *res)
{
    if (!msg_switch_is_master()) {
        res->head = NULL;
        res->tail = NULL;
        return VTSS_RC_ERROR;
    }

    res->head = icfg_alloc_buf(initial_size);
    res->tail = res->head;

    return res->head ? VTSS_RC_OK : VTSS_RC_ERROR;
}

void vtss_icfg_free_query_result(vtss_icfg_query_result_t *res)
{
    if (!msg_switch_is_master()) {
        return;
    }

    vtss_icfg_query_result_buf_t *current, *next;

    current = res->head;
    while (current != NULL) {
        next = current->next;
        if (current->free_text  &&  current->text) {
            VTSS_FREE(current->text);
        }
        VTSS_FREE(current);
        current = next;
    }

    res->head = NULL;
    res->tail = NULL;
}

vtss_rc vtss_icfg_overlay_query_result(char *in_buf, u32 length,
                                       vtss_icfg_query_result_t *res)
{
    vtss_icfg_query_result_buf_t *buf;

    if (!msg_switch_is_master()) {
        res->head = NULL;
        res->tail = NULL;
        return VTSS_RC_ERROR;
    }

    buf = (vtss_icfg_query_result_buf_t *)VTSS_MALLOC(sizeof(vtss_icfg_query_result_buf_t));

    if (buf) {
        buf->text      = in_buf;
        buf->free_text = FALSE;
        buf->used      = length;
        buf->size      = length;
        buf->next      = NULL;
    }

    res->head = buf;
    res->tail = res->head;

    return res->head ? VTSS_RC_OK : VTSS_RC_ERROR;
}

static BOOL icfg_extend_buf(vtss_icfg_query_result_t *res)
{
    char *buf;

    if (!res || !res->tail || !res->tail->text) {
        T_D("Bug: NULL input");
        return FALSE;
    }

    if (!res->tail->free_text) {
        T_D("Cannot extend read-only buffer");
        return FALSE;
    }

    // Try to realloc the tail. If that fails, alloc a new block and append it.

    buf = (char *)VTSS_REALLOC(res->tail->text, res->tail->size + DEFAULT_TEXT_BLOCK_SIZE);
    if (buf) {
        T_D("Realloc: %d => %d bytes, 0x%8p => 0x%8p", res->tail->size, res->tail->size + DEFAULT_TEXT_BLOCK_SIZE, res->tail->text, buf);
        res->tail->text =  buf;
        res->tail->size += DEFAULT_TEXT_BLOCK_SIZE;
    } else {
        T_D("Appending buffer");
        vtss_icfg_query_result_buf_t *next = icfg_alloc_buf(0);
        if (next == NULL) {
            return FALSE;
        }
        res->tail->next = next;
        res->tail       = next;
    }

    return TRUE;
}

// Function for print a configuration to iCFG.
// This function only prints default values if the all->default parameter is set. It also takes into account if the
// configuration is a global configuration or an interface configuration, and adds the extra space that is needed for interfaces configurations.
// One detail is that it is that if you give a "print" format, but don't give any parameter no printout is done. This can be used if a command has multiple parameter, where some of them can be the default values while others are not. See example in "dot1x global" command.
// In  - req - Containing the all default
//       is_default - Set to true is the current configuration is the default configuration.
//       cmd_string - The iCLI user command
//       format and ... - The parameters to the command.
// Out - result - Point to the icfg print
vtss_rc vtss_icfg_conf_print(const vtss_icfg_query_request_t *req,
                             vtss_icfg_query_result_t        *result,
                             vtss_icfg_conf_print_t          conf_print,
                             const char                      *cmd_string,
                             const char                      *format, ...)
{
    char cmd_parameters_string[512];
    char bool_list_string[512];

    strcpy(bool_list_string, ""); // Clear string

    T_N("max:%d, min%d", conf_print.bool_list_max, conf_print.bool_list_min);

    // If there is given a bool list then concatenate the list to a text string (e.g. 1,2,5-6)
    if (conf_print.bool_list != NULL) {
        if (conf_print.bool_list_max > conf_print.bool_list_min) {
            (void) mgmt_list2txt(&conf_print.bool_list[0], conf_print.bool_list_min, conf_print.bool_list_max, bool_list_string);

            T_N("bool_list_string:%s, is_default:%d, len:%zu", bool_list_string, conf_print.is_default, strlen(bool_list_string));
            if (strlen(bool_list_string) == 0) {
                return VTSS_RC_OK;
            }

            strcat(bool_list_string, " "); // Add space between the bool list and the parameters staring
        } else {
            T_W("max:%d must be greater than min:%d", conf_print.bool_list_max, conf_print.bool_list_min);
            return VTSS_RC_ERROR;
        }
    }

    va_list args;
    /*lint -e{530} ... 'args' is initialized by va_start() */
    va_start (args, format);
    (void) vsnprintf (cmd_parameters_string, 100, format, args);
    va_end (args);

    if (req->all_defaults ||
        !conf_print.is_default) {

        // Format given but no parameters - skip printing.
        if (strlen(format) != 0 && strlen(cmd_parameters_string) == 0 && !conf_print.is_default) {
            return VTSS_RC_OK;
        }

        VTSS_RC(vtss_icfg_printf(result, "%s", req->cmd_mode != ICLI_CMD_MODE_GLOBAL_CONFIG ? " " : ""));  // Add the extra space for non global configurations

        strcat(cmd_parameters_string, " "); // Add space between the bool list and the parameters staring
        if (conf_print.is_default) {
            // No command
            if (!conf_print.print_no_arguments) {
                strcpy(cmd_parameters_string, ""); // Skipping the command parameters
            }

            VTSS_RC(vtss_icfg_printf(result, "no "));
        }

        VTSS_RC(vtss_icfg_printf(result, "%s %s%s\n",
                                 cmd_string,
                                 conf_print.bool_list_in_front_of_parameter ? bool_list_string : cmd_parameters_string,
                                 conf_print.bool_list_in_front_of_parameter ? cmd_parameters_string : bool_list_string));

    }
    return VTSS_RC_OK;
}

// Function for initialize vtss_icfg_conf_print_t struct
// In - conf_print - Pointer to the struct to initialize
void vtss_icfg_conf_print_init(vtss_icfg_conf_print_t *conf_print)
{
    conf_print->is_default = FALSE;
    conf_print->print_no_arguments = FALSE;
    conf_print->bool_list = NULL;
    conf_print->bool_list_max = 0;
    conf_print->bool_list_min = 0;
    conf_print->bool_list_in_front_of_parameter = TRUE;
}


vtss_rc vtss_icfg_printf(vtss_icfg_query_result_t *res, const char *format, ...)
{
    va_list va;
    int n;
    int bytes_available;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    /* The eCos implementation of vsnprintf() has a weakness: The return value
     * doesn't indicate whether the function ran out of destination buffer
     * space or not. It could return -1 if it did, or return the number of
     * characters that _would_ have been written to the destination (not
     * including the trailing '\0' byte).
     *
     * But it doesn't. If called with a size of N bytes, it will always write
     * up to N-1 bytes and return that value, no matter if the desired result
     * would have been longer than that.
     *
     * So the only way to determine if we're running out is this:
     *
     *   * Supply buffer with capacity N
     *   * Max. return value from vsnprintf is then N-1
     *   * So if the return value is N-2 or less, then we know that there was
     *     room for at least one more char in the buffer, i.e. we printed all
     *     there was to print.
     */

    while (TRUE) {
        bytes_available = res->tail->size - res->tail->used;

        if (bytes_available < 3) {   // 3 == One byte for output, one for '\0', one to work-around non-C99 eCos vsnprintf()
            if (!icfg_extend_buf(res)) {
                return VTSS_RC_ERROR;
            }
            bytes_available = res->tail->size - res->tail->used;
        }

        /*lint -e{530} ... 'va' is initialized by va_start() */
        va_start(va, format);
        n = vsnprintf(res->tail->text + res->tail->used, bytes_available, format, va);
        va_end(va);

        if (n >= 0  &&  n < (bytes_available - 1)) {    // -1 due to eCos workaround
            res->tail->used += n;
            return VTSS_RC_OK;
        } else if (n >= DEFAULT_TEXT_BLOCK_SIZE - 1 - 1) {   // -1 for '\0', -1 for eCos workaround
            /* Not enough contiguous mem in our buffer _and_ the string is too
             * long for our (fairly large) buffer: Give up with an error
             * message
             */
            T_E("vtss_icfg_printf cannot print string of length %u; too long", n);
            return VTSS_RC_ERROR;
        } else {
            /* Not enough contiguous mem in our buffer, but the string will fit
             * inside an empty one: Allocate more space and vsnprintf() again.
             *
             * Note that this strategy is, of course, subject to an attack where
             * the requested buffer capacity is always half the max buffer size
             * plus one.
             */
            if (!icfg_extend_buf(res)) {
                return VTSS_RC_ERROR;
            }
        }
    }  /* while (TRUE) */
}



//-----------------------------------------------------------------------------
// Feature list utilities
//-----------------------------------------------------------------------------

void icfg_feature_list_get(const u32 cnt, const char *list[])
{
    u32 i, n;

    // Insert unique entries into list. We do that by searching for existing
    // matching entries. That's expensive, of course, but it's a rare operation
    // so we live with it.

    ICFG_CRIT_ENTER();
    for (i = n = 0; i < VTSS_ICFG_LAST  &&  n < cnt - 1; i++) {
        if (icfg_callbacks[i].feature_name) {
            u32 k;
            for (k = 0; k < n  &&  strcmp(icfg_callbacks[i].feature_name, list[k]); k++)
                /* loop */ {
                ;
            }
            if (k == n) {
                list[n++] = icfg_callbacks[i].feature_name;
            }
        }
    }
    ICFG_CRIT_EXIT();
    if (i < VTSS_ICFG_LAST  &&  n == cnt - 1) {
        T_E("ICFG feature list full; truncating. i = %d, n = %d", i, n);
    }
    list[n] = NULL;
}

//-----------------------------------------------------------------------------
// Flash I/O utilities
//-----------------------------------------------------------------------------

/* Our user-visible FS model doesn't support subdirs, so we don't allow them in user-supplied filenames */
static BOOL is_valid_filename_format(const char *path)
{
    while (*path && *path != '/') {
        path++;
    }
    return *path == '\0';
}

int icfg_file_stat(const char *path, struct stat *buf, off_t *compressed_size)
{
    int               res = -1;
    int               fd  = -1;
    icfg_zip_header_t header;

    if (!is_valid_filename_format(path)) {
        T_D("Invalid filename format: %s", path);
        goto out;
    }

    if (stat(path, buf) < 0) {
        if (errno == ENOSYS) {
            T_D("%s: <no status available>", path);
        } else {
            T_D("Cannot retrieve stat info for %s: %s", path, strerror(errno));
        }
        goto out;
    }

    if (compressed_size) {
        *compressed_size = buf->st_size;
    }

    // default-config is read-only, but the eCos FS doesn't correctly support that attribute:
    if (!strcmp(path, DEFAULT_CONFIG)) {
        buf->st_mode = S_IRUSR;
    }

    if ((fd = open(path, O_RDONLY)) < 0) {
        T_W("Failed to access %s: %s", path, strerror(errno));
        goto out;
    }

    if (buf->st_size >= (off_t) sizeof(header)) {
        if (read(fd, &header, sizeof(header)) < (ssize_t) sizeof(header)) {
            T_W("Failed to read %s: %s", path, strerror(errno));
            goto out;
        }

        if (memcmp(header.magic, ZIP_HEADER_MAGIC, ZIP_HEADER_MAGIC_SIZE) == 0) {
            // Magic matches -- this is a compressed file
            if (compressed_size) {
                *compressed_size = buf->st_size - sizeof(header);
            }
            buf->st_size = header.size;
        }
    }

    res = 0;
out:
    if (fd >= 0) {
        (void) close(fd);
    }
    return res;
}

// Return FALSE == error, don't trust results
BOOL icfg_get_flash_file_count(u32 *ro_count, u32 *rw_count)
{
    DIR  *dirp;
    BOOL rc = FALSE;

    *ro_count = 0;
    *rw_count = 0;

    dirp = opendir("/");
    if (dirp == NULL) {
        T_D("Cannot list directory: %s", strerror(errno));
        return FALSE;
    }

    for (;;) {
        struct dirent *entry = readdir(dirp);
        struct stat   sbuf;

        if (entry == NULL) {
            break;
        }

        if (!strcmp(entry->d_name, ".")  ||  !strcmp(entry->d_name, "..")) {
            continue;
        }

        if (icfg_file_stat(entry->d_name, &sbuf, NULL) < 0) {
            goto out;
        }

        if ((sbuf.st_mode & S_IWUSR)) {
            (*rw_count)++;
        } else if ((sbuf.st_mode & S_IRUSR)) {
            (*ro_count)++;
        }
    }
    rc = TRUE;

out:
    if (closedir(dirp) < 0) {
        T_D("closedir: %s", strerror(errno));
        rc = FALSE;
    }

    return rc;
}

/* Remove all R/W files from the file system. This takes place upon MASTER_DOWN to ensure
 * that nothing is left behind in case we later get a MASTER_UP. In that case we re-read
 * the 'conf' block with the file system and create all the files there -- this may overwrite
 * some files, but if a file isn't in the 'conf' block, it stays put in the RAM file system.
 */
static void icfg_filesystem_reset(void)
{
    DIR *dirp;

    dirp = opendir("/");
    if (dirp == NULL) {
        T_D("Cannot list directory: %s", strerror(errno));
        return;
    }

    for (;;) {
        struct dirent *entry = readdir(dirp);
        struct stat   sbuf;

        if (entry == NULL) {
            break;
        }

        if (!strcmp(entry->d_name, ".")  ||  !strcmp(entry->d_name, "..")) {
            continue;
        }

        if (icfg_file_stat(entry->d_name, &sbuf, NULL) < 0) {
            goto out;
        }

        if ((sbuf.st_mode & S_IWUSR)) {
            char fullname[PATH_MAX];
            fullname[0] = '/';
            fullname[1] = 0;
            strcat(fullname, entry->d_name);
            T_D("Unlinking %s", fullname);
            if (unlink(fullname) < 0) {
                T_D("unlink: %s", strerror(errno));
            }
        }
    }

out:
    if (closedir(dirp) < 0) {
        T_D("closedir: %s", strerror(errno));
    }
}



//-----------------------------------------------------------------------------
// VLAN 1 IP Address Save/Restore
//-----------------------------------------------------------------------------

#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
static void icfg_vlan1_ip_save_zerodata(void)
{
    memset(&icfg_ip_save, 0, sizeof(icfg_ip_save));
}

static void icfg_vlan1_ip_save(void)
{
#if defined(VTSS_SW_OPTION_IP2)
    vtss_routing_status_t routes[IP2_MAX_ROUTES];
    u32                   cnt, i;

    if (!msg_switch_is_master()) {
        T_D("Cannot save VLAN 1 setup; not master");
        return;
    }

    T_D("Attempting to save VLAN 1 IP configuration");

    // First get IP address setup

    icfg_ip_save.v4.ip_ok = vtss_ip2_ipv4_conf_get(1, &icfg_ip_save.v4.ip) == VTSS_RC_OK;
    icfg_ip_save.v6.ip_ok = vtss_ip2_ipv6_conf_get(1, &icfg_ip_save.v6.ip) == VTSS_RC_OK;

    // Then try to find a default route that's up and statically configured. Do this
    // for both IPv4 and 6.

    if (vtss_ip2_route_get(VTSS_ROUTING_ENTRY_TYPE_IPV4_UC, IP2_MAX_ROUTES, routes, &cnt) != VTSS_RC_OK) {
        T_D("Cannot get IPv4 routes");
        return;
    }

    for (i = 0; i < cnt  &&  !icfg_ip_save.v4.r_ok; ++i) {
        vtss_routing_entry_t const *rt = &routes[i].rt;

        T_D("Route entry %d of %d, prefix %d, flags 0x%08x, owner %d", i, cnt, rt->route.ipv4_uc.network.prefix_size, routes[i].flags, routes[i].params.owner);

        if (rt->route.ipv4_uc.network.prefix_size != 0) {
            continue;
        }

        if (!(routes[i].flags & VTSS_ROUTING_FLAG_UP)) {
            continue;
        }

        if (!(routes[i].params.owner & VTSS_BIT(VTSS_ROUTING_PARAM_OWNER_STATIC_USER))) {
            continue;
        }

        T_D("Route entry %d selected", i);
        memcpy(&icfg_ip_save.v4.route, rt, sizeof(icfg_ip_save.v4.route));
        icfg_ip_save.v4.r_ok = TRUE;
    }

    if (vtss_ip2_route_get(VTSS_ROUTING_ENTRY_TYPE_IPV6_UC, IP2_MAX_ROUTES, routes, &cnt) != VTSS_RC_OK) {
        T_D("Cannot get IPv6 routes");
        return;
    }

    for (i = 0; i < cnt  &&  !icfg_ip_save.v6.r_ok; ++i) {
        vtss_routing_entry_t const *rt = &routes[i].rt;

        T_D("Route entry %d of %d, prefix %d, flags 0x%08x, owner %d", i, cnt, rt->route.ipv6_uc.network.prefix_size, routes[i].flags, routes[i].params.owner);

        if (rt->route.ipv6_uc.network.prefix_size != 0) {
            continue;
        }

        if (!(routes[i].flags & VTSS_ROUTING_FLAG_UP)) {
            continue;
        }

        if (!(routes[i].params.owner & VTSS_BIT(VTSS_ROUTING_PARAM_OWNER_STATIC_USER))) {
            continue;
        }

        T_D("Route entry %d selected", i);
        memcpy(&icfg_ip_save.v6.route, rt, sizeof(icfg_ip_save.v6.route));
        icfg_ip_save.v6.r_ok = TRUE;
    }
#else
    T_W("Cannot save VLAN 1 setup; no IP stack");
#endif /* defined(VTSS_SW_OPTION_IP2) */
}
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

static void icfg_vlan1_ip_restore(void)
{
#if defined(VTSS_SW_OPTION_IP2)
    if (!msg_switch_is_master()) {
        T_D("Cannot restore VLAN 1 setup; not master");
        return;
    }

    ICFG_CRIT_ENTER();
    if (icfg_ip_save.v4.ip_ok  ||  icfg_ip_save.v6.ip_ok) {
        T_D("Restoring VLAN 1 IP settings");
        if (icfg_ip_save.v4.ip_ok) {
            if (vtss_ip2_ipv4_conf_set(1, &icfg_ip_save.v4.ip) == VTSS_RC_OK) {
                vtss_routing_params_t params = { .owner = VTSS_ROUTING_PARAM_OWNER_STATIC_USER };
                if (icfg_ip_save.v4.r_ok  &&  (vtss_ip2_route_add(&icfg_ip_save.v4.route, &params) != VTSS_RC_OK)) {
                    T_D("Failed to restore IPv4 default route");
                }
            } else {
                T_D("Failed to restore VLAN 1 IPv4 configuration");
            }
        }
        if (icfg_ip_save.v6.ip_ok) {
            if (vtss_ip2_ipv6_conf_set(1, &icfg_ip_save.v6.ip) == VTSS_RC_OK) {
                vtss_routing_params_t params = { .owner = VTSS_ROUTING_PARAM_OWNER_STATIC_USER };
                if (icfg_ip_save.v6.r_ok  &&  (vtss_ip2_route_add(&icfg_ip_save.v6.route, &params) != VTSS_RC_OK)) {
                    T_D("Failed to restore IPv6 default route");
                }
            } else {
                T_D("Failed to restore VLAN 1 IPv6 configuration");
            }
        }
    }
    // Always clear saved config so we don't apply it again inadvertently
    icfg_vlan1_ip_save_zerodata();
    ICFG_CRIT_EXIT();
#else
    T_W("Cannot restore VLAN 1 setup; no IP stack");
#endif /* defined(VTSS_SW_OPTION_IP2) */
}

//=============================================================================
// Configuration File Loading
//=============================================================================

//-----------------------------------------------------------------------------
// ICLI Output Buffer
//-----------------------------------------------------------------------------

// Signature due to ICLI requirements.
static BOOL icfg_commit_output_buffer_char_put(u32 _id, char ch)
{
    ICFG_CRIT_ENTER();
    if (icfg_commit_echo_to_console) {
        (void) putc(ch, stdout);
    }
    if (icfg_commit_output_buffer_length < OUTPUT_BUFFER_SIZE) {
        icfg_commit_output_buffer[icfg_commit_output_buffer_length++] = ch;
    }
    // Silently discard output if buffer is full
    ICFG_CRIT_EXIT();
    return TRUE;
}

// Signature due to ICLI requirements.
static BOOL icfg_commit_output_buffer_str_put(u32 _id, char *str)
{
    ICFG_CRIT_ENTER();
    if (str) {
        if (icfg_commit_echo_to_console) {
            (void) fputs(str, stdout);
        }
        for (; *str && (icfg_commit_output_buffer_length < OUTPUT_BUFFER_SIZE); ++str, ++icfg_commit_output_buffer_length) {
            icfg_commit_output_buffer[icfg_commit_output_buffer_length] = *str;
        }
    }
    ICFG_CRIT_EXIT();
    return TRUE;
}

static void icfg_commit_output_buffer_clear(void)
{
    T_D("Entry");
    ICFG_CRIT_ENTER();
    icfg_commit_output_buffer_length = 0;
    ICFG_CRIT_EXIT();
}

const char *icfg_commit_output_buffer_get(u32 *length)

{
    const char *res;        // Need a temp to avoid lint thread warnings
    ICFG_CRIT_ENTER();
    res = icfg_commit_output_buffer;
    if (length) {
        *length = icfg_commit_output_buffer_length;
    }
    ICFG_CRIT_EXIT();
    return res;
}

void icfg_commit_output_buffer_append(const char *str)
{
    (void) icfg_commit_output_buffer_str_put(0, (char *) str);
}


//-----------------------------------------------------------------------------
// ICLI Line Commit
//-----------------------------------------------------------------------------

static inline BOOL is_end(const char *s)
{
    while (*s == ' '  ||  *s == '\t') {
        s++;
    }
    return ((*s++ == 'e')  &&  (*s++ == 'n')  &&  (*s++ == 'd')  &&
            (*s == '\0'  ||  *s == '\n'  ||  *s == '\r'  ||  *s == ' '  ||  *s == '\t'));
}

/* Commit one line to ICLI. */
static BOOL icfg_commit_one_line_to_icli(u32        session_id,
                                         const char *cmd,
                                         const char *source_name,
                                         BOOL       syntax_check_only,
                                         u32        line_cnt,
                                         u32        *err_cnt,
                                         u32        max_err_cnt)
{
    i32        rc;
    const char *p;

    // ICLI doesn't much like an empty line, so skip it here
    for (p = cmd; *p == ' '  ||  *p == '\t'  ||  *p == '\n'  ||  *p == '\r'; ++p) {
        /* loop */
    }
    if (!*p) {
        return TRUE;
    }

    T_N("%5d %s", line_cnt, cmd);
    if ((rc = ICLI_CMD_EXEC_ERR_FILE_LINE((char *)cmd, !syntax_check_only, (char *)source_name, line_cnt, TRUE)) != ICLI_RC_OK) {
        ++(*err_cnt);
        T_D("ICLI cmd exec err: rc=%d, file %s, line %d", rc, source_name, line_cnt);
        T_D("                   cmd '%s'", cmd);
        if (syntax_check_only  &&  *err_cnt == max_err_cnt) {
            ICLI_PRINTF("%% Too many errors (%d), processing canceled.\n", max_err_cnt);
            return FALSE;
        }
    }
    T_N("Cmd exec done, rc = %d", rc);
    return TRUE;
}

//-----------------------------------------------------------------------------
// ICLI Buffer Commit
//-----------------------------------------------------------------------------

BOOL icfg_commit(u32                            session_id,
                 const char                     *source_name,
                 BOOL                           syntax_check_only,
                 BOOL                           use_output_buffer,
                 const vtss_icfg_query_result_t *res)
{
#define CMD_LINE_LEN (4*1024)
    const BOOL                         open_local_session = session_id == ICLI_SESSION_ID_NONE;
    icli_session_open_data_t           open_data;          // Session data if open_local_session
    const vtss_icfg_query_result_buf_t *buf = NULL;        // Current result buffer block
    const char                         *p;                 // Pointer into buffer; current char being processed
    u32                                len;                // Remaining bytes in current buffer
    char                               cmd[CMD_LINE_LEN];  // Command line being built for ICLI
    char                               *pcmd;              // Pointer into cmd[]
    u32                                err_cnt;            // Error count: How many times ICLI has complained.
    u32                                line_num = 0;       // Line number of current line
    u32                                line_cnt = 0;       // Total number of lines in the file
    i32                                rc;                 // Return code from ICLI calls
    i32                                cmd_mode_level = 0; // ICLI command mode level
    BOOL                               do_abort;           // Copy of icfg_commit_abort_flag
    struct timeval                     t0, t1;             // Measure time per chunk of lines submitted to ICLI

    if (!msg_switch_is_master()) {
        T_D("Cannot commit %s; not master", source_name);
        return FALSE;
    }

    T_D("Session %d, file %s, %s mode, ICLI output => %s", session_id, source_name,
        (syntax_check_only ? "syntax check" : "commit"),
        (use_output_buffer ? "buffer" : "console"));

    // err_cnt must start at 1 to ensure early exit via label 'done' leads to
    // return value FALSE
    err_cnt = 1;

    icfg_commit_output_buffer_clear();     // Always clear, even if we don't use it

    if (open_local_session) {
        /* Open new ICLI session and enter global config mode */

        memset(&open_data, 0, sizeof(open_data));

        open_data.name     = "ICFG";
        open_data.way      = ICLI_SESSION_WAY_APP_EXEC;
        open_data.char_put = use_output_buffer ? icfg_commit_output_buffer_char_put : NULL;
        open_data.str_put  = use_output_buffer ? icfg_commit_output_buffer_str_put  : NULL;

        if ((rc = icli_session_open(&open_data, &session_id)) != ICLI_RC_OK) {
            T_E("Cannot open ICFG session; configuration aborted. rc=%d", rc);
            return FALSE;
        }

        if ((rc = icli_session_privilege_set(session_id, ICLI_PRIVILEGE_DEBUG - 1)) != ICLI_RC_OK) {
            ICLI_PRINTF("%% Failed to configure session; configuration aborted.\n");
            T_E("Cannot set ICFG session privilege; rc=%d", rc);
            goto done;
        }
    }

    if ((cmd_mode_level = icli_session_mode_enter(session_id, ICLI_CMD_MODE_GLOBAL_CONFIG)) < 0) {
        ICLI_PRINTF("%% Failed to enter configuration mode; configuration aborted.\n");
        T_E("Cannot enter ICLI config mode; rc=%d", cmd_mode_level);
        return FALSE;
    }

    if (syntax_check_only) {
        if (icli_session_cmd_parsing_begin(session_id) != ICLI_RC_OK) {
            T_E("Cannot enter ICLI syntax check mode");
            goto done;
        }
    }

    err_cnt = 0;

    ICFG_CRIT_ENTER();
    icfg_commit_running   = ICFG_COMMIT_RUNNING;
    icfg_commit_error_cnt = 0;
    ICFG_CRIT_EXIT();

    // Count number of lines in the buffer; used for progress reporting.
    // We recognize CR, LF, CR+LF, LF+CR as one end-of-line sequence.
    buf = res->head;
    while (buf != NULL  &&  buf->used > 0) {
        len = buf->used;
        p   = buf->text;
        while (len > 0) {
            if (((*p == '\n'  &&  *(p + 1) == '\r')  ||  (*p == '\r'  &&  *(p + 1) == '\n')) ) {
                p++;
                len--;
            }
            if (*p == '\n'  ||  *p == '\r') {
                line_cnt++;
            }
            p++;
            len--;
        }
        buf = buf->next;
    }
    T_D("About to process %d lines", line_cnt);

    // We need to feed ICLI one command line at a time.

    buf  = res->head;
    pcmd = cmd;
    (void) gettimeofday(&t0, NULL);
    while (buf != NULL  &&  buf->used > 0) {
        len = buf->used;
        p   = buf->text;
        while (len > 0) {
            if (((*p == '\n'  &&  *(p + 1) == '\r')  ||  (*p == '\r'  &&  *(p + 1) == '\n')) ) {
                p++;
                len--;
            }
            if (*p == '\n'  ||  *p == '\r') {
                line_num++;
                if ((line_num & 0xff) == 0) {
                    long delta;
                    (void) gettimeofday(&t1, NULL);
                    delta = (t1.tv_sec - t0.tv_sec) * (1000 * 1000) + (t1.tv_usec - t0.tv_usec);
                    t0 = t1;
                    T_D("Processed %d out of %d lines, %ld msec", line_num, line_cnt, delta / 1000);
                }
            } else {
                *pcmd++ = *p;
            }
            if (*p == '\n'  ||  *p == '\r'  ||  (pcmd == cmd + CMD_LINE_LEN - 1)) {
                *pcmd = '\0';
                pcmd = cmd;

                ICFG_CRIT_ENTER();
                do_abort = icfg_commit_abort_flag;
                ICFG_CRIT_EXIT();

                if (is_end(cmd)  ||  do_abort  ||  !icfg_commit_one_line_to_icli(session_id, cmd, source_name, syntax_check_only, line_num, &err_cnt, MAX_ERROR_CNT)) {
                    goto done;
                }
            }
            p++;
            len--;
        }
        buf = buf->next;
    }

    // Handle case where last line isn't newline terminated
    if (pcmd != cmd) {
        *pcmd = '\0';
        line_num++;
        ICFG_CRIT_ENTER();
        do_abort = icfg_commit_abort_flag;
        ICFG_CRIT_EXIT();
        if (!is_end(cmd)  &&  !do_abort) {
            (void) icfg_commit_one_line_to_icli(session_id, cmd, source_name, syntax_check_only, line_num, &err_cnt, MAX_ERROR_CNT);
        }
    }

done:
    if (err_cnt) {
        if (syntax_check_only) {
            ICLI_PRINTF("%% Syntax check done, %u problem%s found.\n", err_cnt, (err_cnt == 1 ? "" : "s"));
        } else {
            ICLI_PRINTF("%% %u problem%s found during configuration.\n", err_cnt, (err_cnt == 1 ? "" : "s"));
        }
    }

    if (syntax_check_only) {
        (void) icli_session_cmd_parsing_end(session_id);
    }

    if (open_local_session) {
        if ((rc = icli_session_close(session_id)) != ICLI_RC_OK) {
            T_E("Cannot close ICFG session; rc=%d\n", rc);
        }
    } else {
        while (((rc = icli_session_mode_exit(session_id)) != (cmd_mode_level - 1))  &&  (rc >= 0)) {
            /* pop levels off until we're back where we came from */
        }
        if (rc < 0) {
            T_E("Cannot exit command mode; rc=%d\n", rc);
        }
    }

    ICFG_CRIT_ENTER();
    icfg_commit_error_cnt       = err_cnt;
    icfg_commit_running         = err_cnt == 0 ? ICFG_COMMIT_DONE : (syntax_check_only ? ICFG_COMMIT_SYNTAX_ERROR : ICFG_COMMIT_ERROR);
    icfg_commit_echo_to_console = FALSE;
    ICFG_CRIT_EXIT();

    return err_cnt == 0;
#undef CMD_LINE_LEN
}

void icfg_commit_status_get(icfg_commit_state_t *state, u32 *error_cnt)
{
    ICFG_CRIT_ENTER();
    if (state) {
        *state = icfg_commit_running;
    }
    if (error_cnt) {
        *error_cnt = icfg_commit_error_cnt;
    }
    ICFG_CRIT_EXIT();
}



//-----------------------------------------------------------------------------
// Configuration Thread
//-----------------------------------------------------------------------------

// Set filename and kick thread. Takes ownership of buffer.
const char *icfg_commit_trigger(const char *filename, vtss_icfg_query_result_t *buf)
{
    const char *msg = NULL;

    ICFG_CRIT_ENTER();

    if (!msg_switch_is_master()) {
        msg = "Not master; cannot trigger load.";
        T_D("%s: %s", filename, msg);
        goto out;
    }

    if (icfg_commit_filename[0]) {
        msg = "Cannot load; another operation is currently in progress.";
        T_D("%s: %s (Current: %s)", filename, msg, icfg_commit_filename);
        goto out;
    }

    misc_strncpyz(icfg_commit_filename, filename, sizeof(icfg_commit_filename));
    icfg_commit_buf = *buf;
    memset(buf, 0, sizeof(*buf));

    T_D("Triggering load of file %s", filename);
    cyg_flag_maskbits(&icfg_thread_flag, 0);
    cyg_flag_setbits(&icfg_thread_flag, ICFG_THREAD_FLAG_COMMIT_FILE);

out:
    vtss_icfg_free_query_result(buf);   // Free buf in case of error
    ICFG_CRIT_EXIT();
    return msg;
}

#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
// Load file into buffer, then kick thread.
static const char *icfg_commit_load_and_trigger(const char *filename)
{
    const char               *msg;
    vtss_icfg_query_result_t buf = { NULL, NULL };

    if (!msg_switch_is_master()) {
        msg = "Not master; cannot trigger load.";
        T_D("%s: %s", filename, msg);
        return msg;
    }

    msg = icfg_file_read(filename, &buf);
    if (msg) {
        vtss_icfg_free_query_result(&buf);
        return msg;
    }

    // When committing a file due to INIT_CMD_CONF_DEF or INIT_CMD_MASTER_UP we want to
    // output problems to the console.
    ICFG_CRIT_ENTER();
    icfg_commit_echo_to_console = TRUE;
    ICFG_CRIT_EXIT();

    msg = icfg_commit_trigger(filename, &buf);  // Takes ownership of buf

    return msg;
}
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

// Trigger synthesis of STARTUP_CONFIG. This occurs for stacking only, and is
// caused by expiry of the the wait-for-slaves timer. The timer event handler
// is fast-path, so we can't synth. on it but need a friendly thread to do it
// for us.
const char *icfg_synth_trigger(void)
{
    const char *msg = NULL;

    ICFG_CRIT_ENTER();

    if (!msg_switch_is_master()) {
        msg = "Not master; cannot trigger synth.";
        T_D("Synth: %s", msg);
        goto out;
    }

    if (icfg_commit_filename[0]) {
        msg = "Cannot synthesize " STARTUP_CONFIG "; another operation is currently in progress.";
        T_D("Synth: %s (Current: %s)", msg, icfg_commit_filename);
        goto out;
    }

    cyg_flag_maskbits(&icfg_thread_flag, 0);
    cyg_flag_setbits(&icfg_thread_flag, ICFG_THREAD_FLAG_SYNTH_STARTUP_CONFIG);

out:
    ICFG_CRIT_EXIT();
    return msg;
}


void icfg_commit_complete_wait(void)
{
    T_D("Waiting for commit to complete");
    (void) cyg_flag_wait(&icfg_thread_flag, ICFG_THREAD_FLAG_COMMIT_DONE, CYG_FLAG_WAITMODE_OR);
    T_D("Commit complete");
}

/* We make an assumption about always being called from the initfuns, and hence
 * only from one thread. If this assumption is broken in the future, we have a
 * race on icfg_commit_abort_flag.
 */
static void icfg_commit_abort(void)
{
    T_D("Aborting commit (if in progress)");

    ICFG_CRIT_ENTER();
    icfg_commit_abort_flag = TRUE;
    ICFG_CRIT_EXIT();

    icfg_commit_complete_wait();

    ICFG_CRIT_ENTER();
    icfg_commit_abort_flag = FALSE;
    ICFG_CRIT_EXIT();
}

#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
static void icfg_silent_upgrade_finish(BOOL);
#endif

static void icfg_thread(cyg_addrword_t data)
{
    char                     filename[sizeof(icfg_commit_filename)];
    vtss_icfg_query_result_t buf = { NULL, NULL };
    cyg_flag_value_t         flag;

    while (1) {
        T_D("Config Commit thread is running.");
        while (msg_switch_is_master()) {
            flag = cyg_flag_wait(&icfg_thread_flag,
                                 ICFG_THREAD_FLAG_COMMIT_FILE | ICFG_THREAD_FLAG_SYNTH_STARTUP_CONFIG,
                                 CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
            T_D("Config Load thread is activated, flag = 0x%08x.", flag);

            if (flag & ICFG_THREAD_FLAG_COMMIT_FILE) {
                ICFG_CRIT_ENTER();
                memcpy(filename, icfg_commit_filename, sizeof(filename));
                buf = icfg_commit_buf;
                memset(&icfg_commit_buf, 0, sizeof(icfg_commit_buf));
                ICFG_CRIT_EXIT();

                if (!vtss_icfg_try_lock_io_mutex()) {
                    icfg_commit_output_buffer_clear();
                    (void) icfg_commit_output_buffer_str_put(0, "Cannot commit configuration; another operation is in progress. Please try again later.");
                    goto skip;
                }

                // No syntax check before application; the buffer is all we've got so we
                // have to try:
                (void) icfg_commit(ICLI_SESSION_ID_NONE, filename, FALSE, TRUE, &buf);

                icfg_vlan1_ip_restore();

                vtss_icfg_unlock_io_mutex();
                T_D("Commit of %s done", filename);

skip:
                vtss_icfg_free_query_result(&buf);

                ICFG_CRIT_ENTER();
                icfg_commit_filename[0] = 0;      // Clear filename == done, ready for next
                ICFG_CRIT_EXIT();
            } else if (flag & ICFG_THREAD_FLAG_SYNTH_STARTUP_CONFIG) {
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
                icfg_silent_upgrade_finish(TRUE);
#endif
            }

            // Must always be set when not actively comitting:
            cyg_flag_setbits(&icfg_thread_flag, ICFG_THREAD_FLAG_COMMIT_DONE);
        } // while
        T_D("Config Commit thread is suspending.");
        cyg_thread_suspend(icfg_thread_handle);
    }
}


//-----------------------------------------------------------------------------
// Read/Write Configuration File
//-----------------------------------------------------------------------------

// We use zlib for compression of files and operate on one chunk at a time:
#define ZIP_CHUNK_SIZE  (128*1024)

// Read file. Allocate buffer of sufficient size. Caller must release buffer,
// even if an error occurs.
// \return NULL if load was OK, pointer to constant string error message otherwise


const char *icfg_file_read(const char *filename, vtss_icfg_query_result_t *res)
{
    struct stat   sbuf;
    ssize_t       bytes_read;
    int           fd              = -1;
    off_t         compressed_size = 0;
    void          *compressed_buf = NULL;
    const char    *msg            = NULL;

    memset(res, 0, sizeof(*res));

    if (!msg_switch_is_master()) {
        msg = "Failed to read file; not master.";
        T_D("%s: %s", filename, msg);
        goto out;
    }

    if (!is_valid_filename_format(filename)) {
        msg = "Invalid file name.";
        T_D("Invalid filename format: %s", filename);
        goto out;
    }

    if (icfg_file_stat(filename, &sbuf, &compressed_size) < 0) {
        msg = "Failed to read file status.";
        T_D("%s: %s %s", filename, msg, strerror(errno));
        goto out;
    }

    if (vtss_icfg_init_query_result(sbuf.st_size & 0xffffffffL, res) != VTSS_RC_OK) {
        msg = "Failed to read file; insufficient memory.";
        T_D("%s: %s (needed %lu bytes)", filename, msg, sbuf.st_size);
        goto out;
    }

    if ((fd = open(filename, O_RDONLY)) < 0) {
        msg = "Failed to open file.";
        T_D("%s: %s %s", filename, msg, strerror(errno));
        goto out;
    }

    T_D("Uncompressed size: %lu  Compressed size: %lu", sbuf.st_size, compressed_size);

    if (sbuf.st_size != compressed_size) {  // Compressed file
        ssize_t       to_read;
        size_t        remaining;
        size_t        avail_out;
        z_stream      strm;
        int           ret;

        if (lseek(fd, sizeof(icfg_zip_header_t), SEEK_SET) < 0) {
            msg = "Failed to read file.";
            T_D("%s: %s %s (seek)", filename, msg, strerror(errno));
            goto out;
        }

        if ((compressed_buf = VTSS_MALLOC(ZIP_CHUNK_SIZE)) == NULL) {
            msg = "Out of memory for file processing.";
            T_D("%s: %s %s", filename, msg, strerror(errno));
            goto out;
        }

        /* allocate inflate state */
        strm.zalloc   = Z_NULL;
        strm.zfree    = Z_NULL;
        strm.opaque   = Z_NULL;
        strm.avail_in = 0;
        strm.next_in  = Z_NULL;
        if (inflateInit(&strm) != Z_OK) {
            msg = "Failed to read file.";
            T_D("%s: %s (inflate init)", filename, msg);
            goto out;
        }

        remaining = compressed_size;
        do {
            to_read = (remaining < ZIP_CHUNK_SIZE) ? remaining : ZIP_CHUNK_SIZE;
            T_D("About to read %8ld bytes; %8d remaining", to_read, remaining);
            if (((bytes_read = read(fd, compressed_buf, to_read)) < to_read) || (bytes_read <= 0)) {
                msg = "Failed to read file (short read).";
                T_D("%s: %s Got %ld bytes out of %ld; %s", filename, msg, bytes_read, to_read, strerror(errno));
                (void) inflateEnd(&strm);
                goto out;
            }
            remaining -= bytes_read;

            avail_out      = res->head->size - res->head->used;
            strm.avail_out = avail_out;
            strm.next_out  = (z_Bytef *) res->head->text + res->head->used;

            strm.avail_in  = bytes_read;
            strm.next_in   = (z_Bytef *)compressed_buf;

            ret = inflate(&strm, Z_NO_FLUSH);

            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void) inflateEnd(&strm);
                msg = "Failed to read file.";
                T_D("%s: %s Inflation failed; managed %u bytes of inflated output; code %d", filename, msg, res->head->used, ret);
                goto out;
            }

            T_D("Inflated %8ld bytes to %8d bytes", bytes_read, (avail_out - strm.avail_out));
            res->head->used += (avail_out - strm.avail_out);

        } while (ret != Z_STREAM_END);
        (void) inflateEnd(&strm);
    } else { // Uncompressed file
        bytes_read = read(fd, res->head->text, res->head->size);

        if (bytes_read < 0) {
            msg = "Failed to read file.";
            T_D("%s: %s %s", filename, msg, strerror(errno));
            goto out;
        }

        if (bytes_read < (ssize_t) res->head->size) {
            msg = "Failed to read file (short read).";
            T_D("%s: %s Got %ld bytes out of %d; %s", filename, msg, bytes_read, res->head->size, strerror(errno));
            goto out;
        }

        res->head->used = bytes_read;
    }

out:
    if (compressed_buf) {
        VTSS_FREE(compressed_buf);
    }
    if (fd >= 0) {
        (void) close(fd);
    }

    return msg;
}



// \return NULL if write was OK, pointer to constant string error message otherwise
const char *icfg_file_write(const char *filename, vtss_icfg_query_result_t *res)
{
    u32                          total  = 0;
    int                          fd     = -1;
    vtss_icfg_query_result_buf_t *buf;
    struct stat                  stat_buf;
    z_stream                     strm;
    int                          ret;
    icfg_zip_header_t            header = { ZIP_HEADER_MAGIC, 0 };
    void                         *compressed_buf = NULL;
    const char                   *msg   = NULL;

    if (!msg_switch_is_master()) {
        msg = "Failed to write file; not master.";
        T_D("%s: %s\n", filename, msg);
        return msg;
    }

    if (!is_valid_filename_format(filename)) {
        msg = "Invalid file name.";
        T_D("Invalid filename format: %s", filename);
        return msg;
    }

    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) {
        msg = "Save failed; cannot write to file.";
        T_D("%s: %s %s (setup)", filename, msg, strerror(errno));
        return msg;
    }

    // Test if file exists. Yes => we'll be overwriting and don't need to check
    // if there are available R/W slots in the FS.
    // Also check if the file is R/O; yes => deny the request.

    if (icfg_file_stat(filename, &stat_buf, NULL) == 0) {
        if ((stat_buf.st_mode & S_IWUSR) == 0) {
            msg = "Save failed; file is read-only.";
            T_D("%s: %s", filename, msg);
            goto out;
        }
    } else {
        u32 ro_count, rw_count;
        if (!icfg_get_flash_file_count(&ro_count, &rw_count)) {
            msg = "Cannot access flash file system.";
            T_E("%s: %s", filename, msg);
            goto out;
        }
        if (rw_count >= ICFG_MAX_WRITABLE_FILES_IN_FLASH_CNT) {
            msg = "File system is full; save aborted.";
            T_D("%s: %s", filename, msg);
            goto out;
        }
    }

    buf = res->head;
    while (buf != NULL  &&  buf->used > 0) {
        total += buf->used;
        buf = buf->next;
    }

    T_D("Saving %u bytes to flash:%s", total, filename);

    if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0) {
        msg = "Save failed; cannot create file.";
        T_D("%s: %s %s", filename, msg, strerror(errno));
        goto out;
    }

    header.size = total;
    if (write(fd, &header, sizeof(header)) < (ssize_t) sizeof(header)) {
        msg = "Save failed; cannot create file.";
        T_D("%s: %s %s (header)", filename, msg, strerror(errno));
        goto out;
    }

    if ((compressed_buf = VTSS_MALLOC(ZIP_CHUNK_SIZE)) == NULL) {
        msg = "Out of memory for file processing.";
        T_D("%s: %s %s", filename, msg, strerror(errno));
        goto out;
    }

    buf = res->head;
    while (buf != NULL  &&  buf->used > 0) {
        int flush     = buf->next == NULL ? Z_FINISH : Z_NO_FLUSH;
        strm.avail_in = buf->used;
        strm.next_in  = (z_Bytef *) buf->text;
        do { // run deflate() on input until output buffer not full
            strm.avail_out = ZIP_CHUNK_SIZE;
            strm.next_out  = (z_Bytef *)compressed_buf;
            ret = deflate(&strm, flush);
            ssize_t to_write = ZIP_CHUNK_SIZE - strm.avail_out;

            T_D("Deflate produced %ld bytes of output", to_write);

            ssize_t wrote = write(fd, compressed_buf, to_write);
            if (wrote < 0) {
                msg = "Save failed during write; file may be corrupt.";
                T_D("%s: %s %s", filename, msg, strerror(errno));
                goto out;
            }
            if (wrote < to_write) {
                msg = "Save failed during write; file may be corrupt.";
                T_D("%s: %s %s (short write)", filename, msg, strerror(errno));
                goto out;
            }
        } while (strm.avail_out == 0);
        buf = buf->next;
    }

    (void) deflateEnd(&strm);
    VTSS_FREE(compressed_buf);

    if (close(fd) < 0) {
        msg = "Save failed during close; file may be corrupt.";
        T_D("%s: %s %s", filename, msg, strerror(errno));
    }

#if defined(CYGPKG_FS_RAM)
    os_file_fs2flash();
#endif

    return msg;

    // Error handlers:

out:
    (void) deflateEnd(&strm);

    if (fd >= 0) {
        (void) close(fd);
    }

    if (compressed_buf) {
        VTSS_FREE(compressed_buf);
    }

    return msg;
}



//-----------------------------------------------------------------------------
// "copy running-config startup-config"
//-----------------------------------------------------------------------------

// \return NULL if save was OK, pointer to constant string error message otherwise
const char *icfg_running_config_save(void)
{
    vtss_icfg_query_result_t res  = { NULL, NULL };
    const char               *msg = NULL;

    if (!vtss_icfg_try_lock_io_mutex()) {
        msg = "Cannot save " STARTUP_CONFIG "; another I/O operation is in progress.";
        T_D("%s", msg);
        return msg;
    }

    T_D("Building configuration...");

    if (vtss_icfg_query_all(FALSE, &res) != VTSS_RC_OK) {
        msg = "Failed to generate configuration for " STARTUP_CONFIG ".";
        goto out;
    }

    msg = icfg_file_write(STARTUP_CONFIG, &res);

#if defined(SNMP_SUPPORT_V3)
    if (!msg) {
        msg = snmpv3_mgmt_users_conf_save() == VTSS_RC_OK ? NULL : "Failed to save SNMPv3 user table.";
    }
#endif
out:
    vtss_icfg_free_query_result(&res);
    vtss_icfg_unlock_io_mutex();

    if (msg) {
        T_W("%s", msg);
    }

    return msg;
}



//-----------------------------------------------------------------------------
// I/O Mutex
//-----------------------------------------------------------------------------

BOOL vtss_icfg_try_lock_io_mutex(void)
{
    BOOL res;
    ICFG_CRIT_ENTER();
    if (icfg_io_in_progress) {
        T_D("I/O in progress; lock denied");
        res = FALSE;
    } else {
        T_D("No I/O in progress; locking");
        icfg_io_in_progress = TRUE;
        res                 = TRUE;
    }
    ICFG_CRIT_EXIT();
    return res;
}

void vtss_icfg_unlock_io_mutex(void)
{
    ICFG_CRIT_ENTER();
    if (icfg_io_in_progress) {
        T_D("Unlocking I/O");
        icfg_io_in_progress = FALSE;
    }
    ICFG_CRIT_EXIT();
}



//-----------------------------------------------------------------------------
// Silent Upgrade
//-----------------------------------------------------------------------------

BOOL vtss_icfg_silent_upgrade_active(void)
{
    BOOL res;
    ICFG_CRIT_ENTER();
    res = icfg_silent_upgrade_active;
    ICFG_CRIT_EXIT();
    return res;
}

BOOL vtss_icfg_silent_upgrade_invoked(void)
{
    BOOL res;
    ICFG_CRIT_ENTER();
    res = icfg_silent_upgrade_invoked;
    ICFG_CRIT_EXIT();
    return res;
}

#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
// Evaluate whether S.U. is required
static void icfg_silent_upgrade_eval(void)
{
    icfg_silent_upgrade_active  = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_OS_FILE_CONF, NULL) == NULL;
    icfg_silent_upgrade_invoked = icfg_silent_upgrade_invoked || icfg_silent_upgrade_active;
    if (icfg_silent_upgrade_active) {
        T_I("Configuration will be silently upgraded.");
    }
}

// Purge unused conf blocks upon completion of silent upgrade process
static void icfg_unused_conf_purge(void)
{
    conf_blk_id_t i;

    // Mainly global blocks; most locals stay in place

    i = CONF_BLK_IP_CONF;
    T_D("Purging unused local conf block id %d", i);
    (void) conf_sec_create(CONF_SEC_LOCAL, i, 0);

    for (i = 0; i < CONF_BLK_COUNT; i++) {
        if ((i != CONF_BLK_TOPO)              && (i != CONF_BLK_OS_FILE_CONF)    &&
            (i != CONF_BLK_TRACE)             && (i != CONF_BLK_RFC2544_REPORTS) &&
            (i != CONF_BLK_MSG)               && (i != CONF_BLK_SSH_CONF)        &&
            (i != CONF_BLK_HTTPS_CONF)        && (i != CONF_BLK_PHY_CONF)        &&
            (i != CONF_BLK_PM_LM_REPORTS)     && (i != CONF_BLK_PM_DM_REPORTS)   &&
            (i != CONF_BLK_PM_EVC_REPORTS)    && (i != CONF_BLK_PM_ECE_REPORTS)  &&
            (i != CONF_BLK_SNMPV3_USERS_CONF)
           ) {
            T_D("Purging unused global conf block id %d", i);
            (void) conf_sec_create(CONF_SEC_GLOBAL, i, 0);
        }
    }
}

/* Complete silent upgrade process. This may be called from the init functions
 * or from the ICFG thread. The latter occurs in stacking scenarios where the
 * wait-for-slaves timer expires and the silent upgrade process must complete
 * on the thread to avoid blocking the fast-path timer expiry handling.
 */
static void icfg_silent_upgrade_finish(BOOL called_from_icfg_thread)
{
    const char *msg = NULL;
    T_I("Completing silent upgrade of configuration, please stand by.");

    // If the conf flash was "empty" (port module conf block doesn't exist)
    // then we must load and apply "default-config" before we generate
    // "startup-config"

    if (icfg_silent_upgrade_flash_empty) {
        T_I("(Configuration FLASH was empty at boot; applying " DEFAULT_CONFIG ")");

        if (called_from_icfg_thread) {
            // Load and commit DEFAULT_CONFIG
            vtss_icfg_query_result_t buf = { NULL, NULL };
            msg = icfg_file_read(DEFAULT_CONFIG, &buf);
            if (!msg) {
                // Output problems to the console:
                ICFG_CRIT_ENTER();
                icfg_commit_echo_to_console = TRUE;
                ICFG_CRIT_EXIT();
                (void) icfg_commit(ICLI_SESSION_ID_NONE, DEFAULT_CONFIG, FALSE, TRUE, &buf);
            }
            vtss_icfg_free_query_result(&buf);
        } else {
            if ((msg = icfg_commit_load_and_trigger(DEFAULT_CONFIG)) != NULL) {
                T_W("Load failed for " DEFAULT_CONFIG ": %s", msg);
            } else {
                icfg_commit_complete_wait();
            }
        }
    }

    // All configuration is now in RAM. Remove old conf blocks and save "startup-config"

    icfg_unused_conf_purge();

    (void)icfg_running_config_save();

    ICFG_CRIT_ENTER();
    icfg_silent_upgrade_active = FALSE;
    ICFG_CRIT_EXIT();

    T_I("Silent Upgrade completed.");
}

#if VTSS_SWITCH_STACKABLE
void icfg_slave_timer_start(void)
{
    T_D("Starting slave timer, timeout after %d secs", SLAVE_WAIT_SECS);
    if (vtss_timer_start(&icfg_slave_wait_timer) != VTSS_RC_OK) {
        T_E("Cannot start silent-upgrade slave timer");
    }
}

void icfg_slave_timer_stop(void)
{
    T_D("Cancelling slave timer");
    if (vtss_timer_cancel(&icfg_slave_wait_timer) != VTSS_RC_OK) {
        T_E("Cannot cancel silent-upgrade slave timer");
    }
}

void icfg_slave_timer_restart(void)
{
    T_D("Restarting slave timer");
    icfg_slave_timer_stop();
    icfg_slave_timer_start();
}

static void icfg_silent_upgrade_stop(void)
{
    ICFG_CRIT_ENTER();
    if (icfg_silent_upgrade_active) {
        T_I("Silent upgrade of configuration aborted on this switch");
        icfg_slave_timer_stop();
        icfg_silent_upgrade_active  = FALSE;
        icfg_silent_upgrade_invoked = FALSE;
    }
    ICFG_CRIT_EXIT();
}

static void icfg_slave_wait_expired(vtss_timer_t *timer)
{
    (void) icfg_synth_trigger();
}

#endif /* VTSS_SWITCH_STACKABLE */
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */


//-----------------------------------------------------------------------------
// ICFG module init
//-----------------------------------------------------------------------------

// Read linker section containing contents of 'default-config' file and create
// it in the file system
static void icfg_default_config_write(void)
{
    extern const char icfg_default_config[], icfg_default_config_end[];
    int fd;

    if ((fd = open(DEFAULT_CONFIG, O_WRONLY | O_CREAT | O_TRUNC, 0444)) < 0) {
        T_E("Unable to write " DEFAULT_CONFIG ": %s", strerror(errno));
    } else {
        write(fd, icfg_default_config, icfg_default_config_end - icfg_default_config);
        close(fd);
    }
}

static void icfg_master_up(void)
{
    if (vtss_icfg_silent_upgrade_active()) {
#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
#if VTSS_SWITCH_STACKABLE
        icfg_slave_timer_start();
#else
        icfg_silent_upgrade_finish(FALSE);
#endif /* VTSS_SWITCH_STACKABLE */
    } else {
        T_D("Attempting to load " STARTUP_CONFIG ", fallback to " DEFAULT_CONFIG);
        if (icfg_commit_load_and_trigger(STARTUP_CONFIG) != NULL  &&
            icfg_commit_load_and_trigger(DEFAULT_CONFIG) != NULL) {
            T_E("Load failed for both " STARTUP_CONFIG " and " DEFAULT_CONFIG);
        } else {
            icfg_commit_complete_wait();
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    }
}

vtss_rc icfg_early_init(vtss_init_data_t *data)
{
    if (data->cmd != INIT_CMD_INIT) {
        T_D("Entry: Early Init: %s, isid %d", control_init_cmd2str(data->cmd), data->isid);
    }
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace resources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        critd_init(&icfg_crit, "icfg_crit", VTSS_MODULE_ID_ICFG, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        icfg_default_config_write();

        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          icfg_thread,
                          0,
                          "ICFG Loader",
                          icfg_thread_stack,
                          sizeof(icfg_thread_stack),
                          &icfg_thread_handle,
                          &icfg_thread_block);

        cyg_flag_init(&icfg_thread_flag);
        cyg_flag_setbits(&icfg_thread_flag, ICFG_THREAD_FLAG_COMMIT_DONE);

#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        icfg_silent_upgrade_eval();
        icfg_silent_upgrade_flash_empty = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PORT_CONF_TABLE, NULL) == NULL;
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

#if VTSS_SWITCH_STACKABLE
        if (vtss_timer_initialize(&icfg_slave_wait_timer) != VTSS_RC_OK) {
            T_E("Cannot initialize silent-upgrade slave timer");
        }
        icfg_slave_wait_timer.period_us = SLAVE_WAIT_SECS * 1000U * 1000U;
        icfg_slave_wait_timer.callback  = icfg_slave_wait_expired;
#endif

        ICFG_CRIT_EXIT();
        break;

    case INIT_CMD_START:
#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        /* If we're note doing silent upgrade, purge unused blocks here. This
         * is in case the user has booted the system in this firmware order:
         *
         *   * 2.80: conf-based, hasn't run 3.40 yet
         *   * 3.40: ICFG-based; silent upgrade runs, creates file system,
         *           purges unused blocks at end
         *   * 2.80: conf-based; doesn't know about FS block but re-creates
         *           defaults for all the other blocks that were removed by
         *           silent upgrade in previous boot
         *   * 3.40: ICFG-based; file system exists so silent upgrade does not
         *           run -- but blocks from 2.80 stay in place
         */

        ICFG_CRIT_ENTER();
        if (!icfg_silent_upgrade_active) {
            icfg_unused_conf_purge();
        }
        ICFG_CRIT_EXIT();
#endif
        break;

    case INIT_CMD_CONF_DEF:
#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        if (data->isid == VTSS_ISID_GLOBAL) {
            icfg_commit_abort();
            ICFG_CRIT_ENTER();
            icfg_vlan1_ip_save_zerodata();
            if (data->flags & INIT_CMD_PARM2_FLAGS_IP) {
                icfg_vlan1_ip_save();
            }
            ICFG_CRIT_EXIT();
        }
#endif
        break;

    case INIT_CMD_MASTER_UP:
#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        ICFG_CRIT_ENTER();
        if (!icfg_silent_upgrade_active) {
            icfg_silent_upgrade_eval();
        }
        ICFG_CRIT_EXIT();
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        break;

    case INIT_CMD_SWITCH_ADD:
#if VTSS_SWITCH_STACKABLE
        ICFG_CRIT_ENTER();
        if (icfg_silent_upgrade_active) {
            icfg_slave_timer_restart();
        }
        ICFG_CRIT_EXIT();
#endif
        break;

    case INIT_CMD_SWITCH_DEL:
        break;

    case INIT_CMD_MASTER_DOWN:
#if VTSS_SWITCH_STACKABLE
        icfg_silent_upgrade_stop();
#endif
        icfg_commit_abort();
        icfg_filesystem_reset();
        break;

    default:
        break;
    }

    T_D("Exit:  Early Init: %s", control_init_cmd2str(data->cmd));
    return VTSS_OK;
}

vtss_rc icfg_late_init(vtss_init_data_t *data)
{
    T_D("Entry: Late Init: %s, isid %d", control_init_cmd2str(data->cmd), data->isid);
    switch (data->cmd) {
    case INIT_CMD_INIT:
        break;

    case INIT_CMD_START:
        break;

    case INIT_CMD_CONF_DEF:
#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        if (data->isid == VTSS_ISID_LOCAL  &&  !(data->flags & INIT_CMD_PARM2_FLAGS_NO_DEFAULT_CONFIG)) {  // Last ISID and loading of default-config hasn't been disabled
            (void)icfg_commit_load_and_trigger(DEFAULT_CONFIG);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        break;

    case INIT_CMD_MASTER_UP:
        T_D("Master Up: Resuming thread");
        cyg_flag_maskbits(&icfg_thread_flag, 0);
        cyg_flag_setbits(&icfg_thread_flag, ICFG_THREAD_FLAG_COMMIT_DONE);
        cyg_thread_resume(icfg_thread_handle);
        icfg_master_up();
        break;

    case INIT_CMD_SWITCH_ADD:
        break;

    case INIT_CMD_SWITCH_DEL:
        break;

    case INIT_CMD_MASTER_DOWN:
        break;

    default:
        break;
    }

    T_D("Exit:  Late Init: %s", control_init_cmd2str(data->cmd));
    return VTSS_OK;
}
