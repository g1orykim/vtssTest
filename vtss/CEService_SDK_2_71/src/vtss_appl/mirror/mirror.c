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

#include "mirror_custom_api.h"
#include "main.h"
#include "conf_api.h"
#include "port_api.h"
#include "mirror_api.h"
#include "mirror.h"
#include "critd_api.h"
#include "msg_api.h"
#include "misc_api.h"
#ifdef VTSS_SW_OPTION_ICLI
#include "mirror_icli_functions.h"
#endif
#ifdef VTSS_SW_OPTION_VCLI
#include "mirror_cli.h"
#endif

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* IP configuration */


static mirror_global_t mem_mirror_conf; // Mirror configuration located in RAM memory


#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "mirror",
    .descr     = "Mirror"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CLI] = {
        .name       = "cli",
        .descr      = "CLI",
        .lvl        = VTSS_TRACE_LVL_WARNING,
        .timestamp  = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
static critd_t    crit;     /* Critical region protection protecting the following block of variables */
#define MIRROR_CRIT_ENTER() critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MIRROR_CRIT_EXIT()  critd_exit(&crit,  TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define MIRROR_CRIT_ENTER() critd_enter(&crit)
#define MIRROR_CRIT_EXIT()  critd_exit(&crit)
#endif /* VTSS_TRACE_ENABLED */


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

static void mirror_get_local_conf(mirror_local_switch_conf_t *conf, vtss_isid_t isid)
{
    MIRROR_CRIT_ENTER();
    //
    // Ports
    //
    int port_idx;
    int isid_idx = isid - VTSS_ISID_START;
    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
        conf->src_enable[port_idx] = mem_mirror_conf.stack_conf.src_enable[isid_idx][port_idx];
        conf->dst_enable[port_idx] = mem_mirror_conf.stack_conf.dst_enable[isid_idx][port_idx];
    }

#ifdef VTSS_FEATURE_MIRROR_CPU
    //
    // CPU
    //
    conf->cpu_src_enable = mem_mirror_conf.stack_conf.cpu_src_enable[isid_idx];
    conf->cpu_dst_enable = mem_mirror_conf.stack_conf.cpu_dst_enable[isid_idx];
#endif
    MIRROR_CRIT_EXIT();
}

/* Procedure that writes the global configuration variable to the chip registers */
static void mirror_update_regs(mirror_local_switch_conf_t conf)
{

#if VTSS_SWITCH_STACKABLE
    T_N ("Entering mirror_update_regs stacking enable:%d", vtss_stacking_enabled());
    if (vtss_stacking_enabled()) {
        // Signal to sprout if a mirror is up or down
        if (conf.mirror_port == VTSS_PORT_NO_NONE ) {
            if (topo_have_mirror_port_set(TOPO_CHIP_IDX_START, 0) != VTSS_RC_OK) {
                T_E("Problem setting mirror port for Topo");
            }
        } else {
            if (topo_have_mirror_port_set(TOPO_CHIP_IDX_START, 1) != VTSS_RC_OK) {
                T_E("Problem setting mirror port for Topo");
            };
        }
    }
#endif

    T_N ("Register write: Mirror port being changed to %u", conf.mirror_port);
    (void) vtss_mirror_monitor_port_set(NULL, conf.mirror_port);

    T_N ("Register write: Source and destination being changed");
    (void) vtss_mirror_ingress_ports_set(NULL, &conf.src_enable[0]);
    (void) vtss_mirror_egress_ports_set(NULL, &conf.dst_enable[0]);
#ifdef VTSS_FEATURE_MIRROR_CPU
    (void) vtss_mirror_cpu_egress_set(NULL, conf.cpu_src_enable);

    // Setup CPU TX mirroring
    (void) vtss_mirror_cpu_ingress_set(NULL, conf.cpu_dst_enable);
#endif
    T_R ("Exiting mirror_update_regs");
}

// Convert Mirror error code to text
// In : rc - error return code
char *mirror_error_txt(vtss_rc rc)
{
    switch (rc) {
    case MIRROR_ERROR_PORT_CONF_MUST_BE_MASTER:
        return "Ports configuration can only be read from master switch";
    case MIRROR_ERROR_INVALID_MIRROR_PORT:
        return "Invalid mirror destination port (port to mirror to)";
    case MIRROR_ERROR_INVALID_MIRROR_SWITCH:
        return "Invalid mirror destination switch (switch to mirror to)";
    default:
        return "Mirror: Unknown error code";
    }
}


/*************************************************************************
 ** Message module functions
 *************************************************************************/

// Getting message from the message module.
static BOOL mirror_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    mirror_msg_id_t msg_id = *(mirror_msg_id_t *)rx_msg;

    T_N("Entering mirror_msg_rx");
    T_D("msg_id: %d,  len: %zd, isid: %u", msg_id, len, isid);

    switch (msg_id) {
    case MIRROR_MSG_ID_CONF_SET_REQ: {
        T_N("mirror_msg_rx");
        mirror_conf_set_req_t *msg;
        msg = (mirror_conf_set_req_t *)rx_msg;
        mirror_update_regs(msg->conf);
        break;
    }
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }

    return TRUE;
}


static char *mirror_msg_id_txt(mirror_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case MIRROR_MSG_ID_CONF_SET_REQ:
        txt = "MIRROR_MSG_ID_CONF_SET_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}

/* Allocate request/reply buffer */
static void mirror_msg_alloc(mirror_msg_buf_t *buf, BOOL request)
{
    MIRROR_CRIT_ENTER();
    buf->sem = (request ? &mem_mirror_conf.request.sem : &mem_mirror_conf.reply.sem);
    buf->msg = (request ?  mem_mirror_conf.request.msg :  mem_mirror_conf.reply.msg);
    MIRROR_CRIT_EXIT();
    (void) VTSS_OS_SEM_WAIT(buf->sem);
}

/* Release message buffer */
static void mirror_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    mirror_msg_id_t msg_id = *(mirror_msg_id_t *)msg;
    T_D("msg_id: %d, %s", msg_id, mirror_msg_id_txt(msg_id));
    VTSS_OS_SEM_POST(contxt);
}

/* Send message */
static void mirror_msg_tx(mirror_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    mirror_msg_id_t msg_id = *(mirror_msg_id_t *)buf->msg;
    T_D("msg_id: %s, len: %zd, isid: %d", mirror_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, mirror_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_MIRROR, isid, buf->msg, len);
}

/**********************************************************************
 ** Configuration functions
 **********************************************************************/

// Read configuration and transmit configuration to the switch added
static vtss_rc mirror_conf_set(vtss_isid_t isid)
{
    mirror_msg_buf_t         buf;
    mirror_conf_set_req_t   *msg;

    T_N("Entering mirror_conf_set, isid = %d ", isid);

    if (msg_switch_exists(isid)) {
        T_D("Transmitting new configuration to isid: %d", isid);

        // Prepare for message tranfer
        mirror_msg_alloc(&buf, 1);
        msg = (mirror_conf_set_req_t *)buf.msg;
        msg->msg_id = MIRROR_MSG_ID_CONF_SET_REQ;
        MIRROR_CRIT_ENTER();

        // Get configuration and prepare to transmit it to the slave
        if (mem_mirror_conf.stack_conf.mirror_switch == isid ) {
            // The switch in question contains the mirror port
            msg->conf.mirror_port = mem_mirror_conf.stack_conf.mirror_port;
        } else {
            // The switch in question doesn't contain the mirror port. This is signaled by setting mirror_port to VTSS_PORT_NO_NONE
            msg->conf.mirror_port = VTSS_PORT_NO_NONE;
        }
        MIRROR_CRIT_EXIT();

        // Get src and dst for the switch in question
        mirror_get_local_conf(&msg->conf, isid);

        // Transmit the message
        mirror_msg_tx(&buf, isid, sizeof(*msg));
    }
    T_R("Exiting mirror_conf_set, isid = %d ", isid);
    return VTSS_OK;
}

// Writing configuration to flash
static void store_new_conf_in_flash(void)
{
    MIRROR_CRIT_ENTER();
    int isid_idx = mem_mirror_conf.stack_conf.mirror_switch - VTSS_ISID_START; // Convert isid to index starting from zero.
    T_D("Storing mirror sid:%d, .mirror_port:%u, src:%d, dst:%d",
        mem_mirror_conf.stack_conf.mirror_switch, mem_mirror_conf.stack_conf.mirror_port,
        mem_mirror_conf.stack_conf.src_enable[isid_idx][mem_mirror_conf.stack_conf.mirror_port],
        mem_mirror_conf.stack_conf.dst_enable[isid_idx][mem_mirror_conf.stack_conf.mirror_port]);


    // The mirror port must only be RX or Disabled, so set src destination to 0 for the mirror port..
    if (mem_mirror_conf.stack_conf.mirror_port != VTSS_PORT_NO_NONE) {
        mem_mirror_conf.stack_conf.dst_enable[isid_idx][mem_mirror_conf.stack_conf.mirror_port] = 0;
    }

    T_D("src:%d, dst:%d", mem_mirror_conf.stack_conf.src_enable[isid_idx][mem_mirror_conf.stack_conf.mirror_port],
        mem_mirror_conf.stack_conf.dst_enable[isid_idx][mem_mirror_conf.stack_conf.mirror_port]);

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    {
        mirror_stack_conf_t *conf_blk;
        ulong               size;
        if ((conf_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MIRROR_CONF_TABLE , &size)) == NULL || size != sizeof(*conf_blk)) {
            T_E("Could not open conf for storing");
        } else {
            *conf_blk = mem_mirror_conf.stack_conf;
            T_D("Writing mirror configuration to flash");
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MIRROR_CONF_TABLE);
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    MIRROR_CRIT_EXIT();
}

/* Determine if port configuration has changed */
static int mirror_global_conf_changed(mirror_conf_t *old, mirror_conf_t *new)
{
    return (new->dst_port      != old->dst_port ||
            new->mirror_switch != old->mirror_switch);
}

/* Determine if port configuration has changed */
static int mirror_local_conf_changed(mirror_switch_conf_t *old, mirror_switch_conf_t *new)
{
    return !(memcmp(new->src_enable, old->src_enable, sizeof(new->src_enable)) == 0 &&
#ifdef VTSS_FEATURE_MIRROR_CPU
             new->cpu_src_enable == old->cpu_src_enable &&
             new->cpu_dst_enable == old->cpu_dst_enable &&
#endif
             memcmp(new->dst_enable, old->dst_enable, sizeof(new->dst_enable)) == 0);
}


// Function that returns the default mirror switch
vtss_isid_t mirror_switch_default(void)
{
    vtss_isid_t default_mirror_switch = VTSS_ISID_START; // Default for standalone

#if VTSS_SWITCH_STACKABLE
    // Find the switch with the lowest USID number and assign that as default mirror switch
    if (vtss_stacking_enabled()) {
        int usid;
        for (usid = VTSS_USID_START; usid <= VTSS_USID_END ; usid ++) {
            if (usid == VTSS_USID_END) {
                // If USID reaches USID_END there is no switches in the stack. Assign lowest ISID as default
                default_mirror_switch = VTSS_ISID_START;
            } else {
                // Assign the first ( and thereby lowest ) usid found
                default_mirror_switch = topo_usid2isid(usid);
                if (msg_switch_configurable(default_mirror_switch)) {
                    break;
                }
            }
        }
    }
#endif
    return default_mirror_switch;
}

/* Read/create and activate port configuration */
// @force_defaults: FALSE if called from MASTER_UP, TRUE if called from CONF_DEF
// @isid_add: [VTSS_ISID_START; VTSS_ISID_END[ if for a local switch.
//            VTSS_ISID_GLOBAL if for all switches.
// If @force_defaults is TRUE, we may send updates to switches.
static void mirror_conf_read(BOOL force_defaults, vtss_isid_t isid_add)
{
    vtss_isid_t dst_isid, isid, min_isid, max_isid;
    vtss_port_no_t port_idx;
    ulong           size;
    mirror_stack_conf_t *stack_conf;
    BOOL do_create;
    BOOL changed;

    T_N("entering mirror_conf_read, isid_add: %d", isid_add);

    if (misc_conf_read_use()) {
        /* Open or create mirror configuration block */
        if ((stack_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MIRROR_CONF_TABLE , &size)) == NULL ||
            size != sizeof(*stack_conf)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            stack_conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MIRROR_CONF_TABLE, sizeof(*stack_conf));
            do_create = TRUE;
        } else if (stack_conf->version != MIRROR_CONF_VERSION) {
            T_W("Version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = force_defaults;
        }
    } else {
        stack_conf = NULL;
        do_create  = TRUE;
    }

    // Create a new mirror conf table. Either create defaults per switch or do it globally.
    MIRROR_CRIT_ENTER();
    if (do_create) {
        mem_mirror_conf.stack_conf.version = MIRROR_CONF_VERSION;
        changed = FALSE;
        dst_isid = 0;
        if (isid_add == VTSS_ISID_GLOBAL) {
            // Only send update if mirroring is globally enabled.
            changed = mem_mirror_conf.stack_conf.mirror_port != MIRROR_PORT_DEFAULT;
            dst_isid = mem_mirror_conf.stack_conf.mirror_switch; // The switch to send the update to
            mem_mirror_conf.stack_conf.mirror_port = MIRROR_PORT_DEFAULT; // Disable mirroring.

            mem_mirror_conf.stack_conf.mirror_switch = MIRROR_SWITCH_DEFAULT;
        }

        // If force_defaults is FALSE, then this function is called due to a MASTER_UP
        // event. If then do_create is TRUE, it's because something is wrong, and therefore
        // we need to create defaults for all switches.
        // If - on the other hand - force_defaults is TRUE, it's because this function is called
        // due to a CONF_DEF event, and we should only set defaults for the switch given by
        // isid_add.
        if (force_defaults) {
            // Called from CONF_DEF event.
            if (isid_add == VTSS_ISID_GLOBAL) {
                // Only restore defaults for global part. The following indices
                // make sure we don't get into the for() loop below.
                min_isid = 1;
                max_isid = 0;
            } else {
                // Restore defaults for particular switch
                min_isid = isid_add;
                max_isid = isid_add;
                dst_isid = isid_add;
            }
        } else {
            // Called from MASTER_UP event. Something went wrong. Restore defaults for all switches.
            min_isid = VTSS_ISID_START;
            max_isid = VTSS_ISID_END - 1;
        }

        for (isid = min_isid; isid <= max_isid; isid++) {
            int isid_idx = isid - VTSS_ISID_START;
            for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                if (mem_mirror_conf.stack_conf.src_enable[isid_idx][port_idx] ||
                    mem_mirror_conf.stack_conf.dst_enable[isid_idx][port_idx]) {
                    changed = TRUE;
                }
                mem_mirror_conf.stack_conf.src_enable[isid_idx][port_idx] = MIRROR_SRC_ENA_DEFAULT;
                mem_mirror_conf.stack_conf.dst_enable[isid_idx][port_idx] = MIRROR_DST_ENA_DEFAULT;
            }

#ifdef VTSS_FEATURE_MIRROR_CPU
            //
            // CPU
            //
            if (mem_mirror_conf.stack_conf.cpu_src_enable[isid_idx] ||
                mem_mirror_conf.stack_conf.cpu_dst_enable[isid_idx]) {
                changed = TRUE;
            }
            mem_mirror_conf.stack_conf.cpu_src_enable[isid_idx] = MIRROR_CPU_SRC_ENA_DEFAULT;
            mem_mirror_conf.stack_conf.cpu_dst_enable[isid_idx] = MIRROR_CPU_DST_ENA_DEFAULT;
#endif
        }

        // Copy the memory version to the conf version, if the stack_conf exists.
        // stack_conf can only be NULL by now if conf_sec_create() failed.
        if (stack_conf != NULL) {
            *stack_conf = mem_mirror_conf.stack_conf;
        }

        // Transmit the changes to the slave.
        if (force_defaults && changed && VTSS_ISID_LEGAL(dst_isid)) {
            MIRROR_CRIT_EXIT(); // Because mirror_conf_set enters the critcal region as well, we have to exit the critical region before calling mirror_conf_set.
            (void)mirror_conf_set(dst_isid);
            MIRROR_CRIT_ENTER(); // Re-enter critical region, after mirror_conf_set have been completed.
        }
    } else {
        // Don't create defaults. Use the loaded ones.
        // This should never give rise to a message to a slave, since it's
        // only possible to get here from a MASTER_UP event.
        if (stack_conf != NULL) {
            mem_mirror_conf.stack_conf = *stack_conf;
        }
    }
    T_D("Read mirror conf : mirror switch:%d", mem_mirror_conf.stack_conf.mirror_switch );

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Close configuration
    if (stack_conf != NULL) {
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MIRROR_CONF_TABLE);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    MIRROR_CRIT_EXIT();
}



/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* Get port configuration */
void mirror_mgmt_conf_get(mirror_conf_t *conf)
{
    T_D("Enter mirror_mgmt_conf_get");
    MIRROR_CRIT_ENTER();
    conf->mirror_switch = mem_mirror_conf.stack_conf.mirror_switch;
    conf->dst_port      = mem_mirror_conf.stack_conf.mirror_port;
    MIRROR_CRIT_EXIT();
    T_D("exit mirror_mgmt_conf_get, mirror switch :%d", conf->mirror_switch);
}


/* Get port configuration */
vtss_rc mirror_mgmt_switch_conf_get(vtss_isid_t isid, mirror_switch_conf_t *conf)
{
    int port_idx;
    int isid_idx = isid - VTSS_ISID_START;

    if (!msg_switch_is_master()) {
        return MIRROR_ERROR_PORT_CONF_MUST_BE_MASTER;
    }

    T_N("Enitering mirror_mgmt_switch_conf_get : isid_idx = %d", isid_idx);
    MIRROR_CRIT_ENTER();

    // Get to src and dst for the local switch
    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
        conf->src_enable[port_idx] = mem_mirror_conf.stack_conf.src_enable[isid_idx][port_idx];
        conf->dst_enable[port_idx] = mem_mirror_conf.stack_conf.dst_enable[isid_idx][port_idx];
    }

#ifdef VTSS_FEATURE_MIRROR_CPU
    conf->cpu_src_enable = mem_mirror_conf.stack_conf.cpu_src_enable[isid_idx];
    conf->cpu_dst_enable = mem_mirror_conf.stack_conf.cpu_dst_enable[isid_idx];
#endif

    MIRROR_CRIT_EXIT();
    T_R("mirror_mgmt_switch_conf_get");

    return VTSS_OK;
}

// Set port configuration
vtss_rc mirror_mgmt_conf_set(mirror_conf_t *new_conf)
{
    mirror_conf_t old_conf;
    MIRROR_CRIT_ENTER();
    old_conf.dst_port = mem_mirror_conf.stack_conf.mirror_port;
    old_conf.mirror_switch = mem_mirror_conf.stack_conf.mirror_switch;
    MIRROR_CRIT_EXIT();

    // Update and store new configuration if the configuration has changed.
    if (mirror_global_conf_changed(new_conf, &old_conf)) {


        T_D("port_isid_port_no_is_front_port(%d, %u) = %d", new_conf->mirror_switch, new_conf->dst_port,
            port_isid_port_no_is_front_port(new_conf->mirror_switch, new_conf->dst_port));

        // Return error code in the case where the destination ports isn't a valid for the switch selected.
        if (!port_isid_port_no_is_front_port(new_conf->mirror_switch, new_conf->dst_port) &&
            new_conf->dst_port != VTSS_PORT_NO_NONE) {
            return MIRROR_ERROR_INVALID_MIRROR_PORT;
        }


        // First we disable mirroring for the old configuration
        MIRROR_CRIT_ENTER();
        mem_mirror_conf.stack_conf.mirror_port   = VTSS_PORT_NO_NONE;
        mem_mirror_conf.stack_conf.mirror_switch = new_conf->mirror_switch;
        MIRROR_CRIT_EXIT();

        if (VTSS_ISID_LEGAL(old_conf.mirror_switch)) {
            VTSS_RC(mirror_conf_set(old_conf.mirror_switch));    // Update the old switch
        } else {
            return MIRROR_ERROR_INVALID_MIRROR_SWITCH;
        }

        // Then we update with the new mirroring configuration.
        MIRROR_CRIT_ENTER();
        mem_mirror_conf.stack_conf.mirror_port   = new_conf->dst_port;
        mem_mirror_conf.stack_conf.mirror_switch = new_conf->mirror_switch;
        MIRROR_CRIT_EXIT();

        // Store in flash
        store_new_conf_in_flash();

        // Update the switch in question
        if (VTSS_ISID_LEGAL(new_conf->mirror_switch)) {
            VTSS_RC(mirror_conf_set(new_conf->mirror_switch));
        }
    }
    return VTSS_OK;
}

void mirror_mgmt_switch_conf_set(vtss_isid_t isid, mirror_switch_conf_t *new_conf)
{
    int port_idx;
    int isid_idx = isid - VTSS_ISID_START;
    mirror_switch_conf_t old_conf;

    if (!msg_switch_configurable(isid)) {
        T_D("Invalid ISID: %u", isid);
        return;
    }

    MIRROR_CRIT_ENTER();
    // Get src and dst for the local switch
    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
        old_conf.src_enable[port_idx] = mem_mirror_conf.stack_conf.src_enable[isid_idx][port_idx];
        old_conf.dst_enable[port_idx] = mem_mirror_conf.stack_conf.dst_enable[isid_idx][port_idx];
        T_D_PORT((vtss_port_no_t) port_idx, "src:%d, dst:%d, isid:%u", mem_mirror_conf.stack_conf.src_enable[isid_idx][port_idx],
                 mem_mirror_conf.stack_conf.dst_enable[isid_idx][port_idx], isid);
    }
#ifdef VTSS_FEATURE_MIRROR_CPU
    // Set CPU
    old_conf.cpu_src_enable = mem_mirror_conf.stack_conf.cpu_src_enable[isid_idx];
    old_conf.cpu_dst_enable = mem_mirror_conf.stack_conf.cpu_dst_enable[isid_idx];
#endif
    MIRROR_CRIT_EXIT();

    // Update and store new configuration if the configuration has changed.
    if (mirror_local_conf_changed(new_conf, &old_conf)) {

        T_R("Configuration changed - Updating with new configuration, isid_idx = %d", isid_idx);
        // Update global configuration variable.

        MIRROR_CRIT_ENTER();
        // Set to src and dst for the local switch
        for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
            mem_mirror_conf.stack_conf.src_enable[isid_idx][port_idx] = new_conf->src_enable[port_idx];
            mem_mirror_conf.stack_conf.dst_enable[isid_idx][port_idx] = new_conf->dst_enable[port_idx];
            T_D_PORT((vtss_port_no_t) port_idx, "src:%d, dst:%d, isid:%u", mem_mirror_conf.stack_conf.src_enable[isid_idx][port_idx],
                     mem_mirror_conf.stack_conf.dst_enable[isid_idx][port_idx], isid_idx);

        }
#ifdef VTSS_FEATURE_MIRROR_CPU
        mem_mirror_conf.stack_conf.cpu_src_enable[isid_idx] = new_conf->cpu_src_enable;
        mem_mirror_conf.stack_conf.cpu_dst_enable[isid_idx] = new_conf->cpu_dst_enable;
#endif
        MIRROR_CRIT_EXIT();

        // Store in flash
        store_new_conf_in_flash();

        // Update the switch in question
        if (VTSS_ISID_LEGAL(isid)) {
            (void)mirror_conf_set(isid);
        }
    }
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

// Function called when booting
static int mirror_start(void)
{

    vtss_mirror_conf_t api_conf;


    /* Register for stack messages */
    msg_rx_filter_t filter;

    T_N("entering mirror_start");

    MIRROR_CRIT_ENTER();
    /* Initialize message buffers */
    VTSS_OS_SEM_CREATE(&mem_mirror_conf.request.sem, 1);
    VTSS_OS_SEM_CREATE(&mem_mirror_conf.request.sem, 1);
    MIRROR_CRIT_EXIT();

    memset(&filter, 0, sizeof(filter));
    filter.cb = mirror_msg_rx;
    filter.modid = VTSS_MODULE_ID_MIRROR;
    T_R("Exiting mirror_start");

    // Setup static mirror API configuration
    if (vtss_mirror_conf_get(NULL, &api_conf) != VTSS_RC_OK) {
        T_E("Could not get mirror_conf");
    }
    api_conf.fwd_enable = MIRROR_FWD;
    if (vtss_mirror_conf_set(NULL, &api_conf) != VTSS_RC_OK) {
        T_E("Could not set mirror_conf");
    }

    return msg_rx_filter_register(&filter);
}

/* Initialize module */
vtss_rc mirror_init(vtss_init_data_t *data)
{

    vtss_isid_t isid = data->isid; // Get switch id

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);
    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT_CMD_INIT");
        critd_init(&crit, "mem_mirror_config.crit", VTSS_MODULE_ID_MIRROR, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        MIRROR_CRIT_EXIT();

#ifdef VTSS_SW_OPTION_VCLI
        mirror_cli_init();
        mirror_cli_txt_init();
#endif
        break;
    case INIT_CMD_START:
        T_D("INIT_CMD_START");
        VTSS_RC(mirror_start());
#ifdef VTSS_SW_OPTION_ICFG
        vtss_rc rc;
        if ((rc = mirror_lib_icfg_init()) != VTSS_OK) {
            T_D("Calling mirror_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_CONF_DEF:
        T_N("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL || VTSS_ISID_LEGAL(isid)) {
            /* Reset configuration (specific switch or all switches) */
            mirror_conf_read(TRUE, isid);
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_N("MASTER_UP");
        mirror_conf_read(FALSE, VTSS_ISID_GLOBAL);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_N("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_N("SWITCH_ADD, isid: %d", isid);
        VTSS_RC(mirror_conf_set(isid));
        break;
    case INIT_CMD_SWITCH_DEL:
        T_N("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit mirror_init");
    return VTSS_OK;

}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
