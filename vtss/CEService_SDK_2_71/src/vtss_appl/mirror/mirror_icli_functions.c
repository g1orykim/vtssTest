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

#include "icli_api.h"
#include "icli_porting_util.h"

#include "cli.h"
#include "msg_api.h"
#include "mirror.h"
#include "mirror_api.h" // For mirror_conf_t
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

/***************************************************************************/
/*  Code start :)                                                          */
/***************************************************************************/



/***************************************************************************/
/*  Internal functions                                                     */
/***************************************************************************/

// Function doing the source configuration
vtss_rc mirror_source_conf(i32  session_id, BOOL interface, icli_stack_port_range_t *port_list, BOOL cpu,
                           const icli_range_t *cpu_switch_range, BOOL both, BOOL rx, BOOL tx, BOOL no)
{
    switch_iter_t        sit;
    mirror_switch_conf_t switch_conf;
    mirror_conf_t        global_conf;
    i8                   str_buf[100];

    // If no parameters then default to both.
    if (!both && !rx && !tx) {
        both = TRUE;
    }

    T_I("Interface:%d, cpu:%d, no:%d", interface, cpu, no);

    mirror_mgmt_conf_get(&global_conf); // Get configuration that is shared by all switches in a stack.

    VTSS_RC(icli_switch_iter_init(&sit));
    while (icli_switch_iter_getnext(&sit, port_list)) {
        port_iter_t pit;
        // Get current configuration
        VTSS_RC(mirror_mgmt_switch_conf_get(sit.isid, &switch_conf));

        // Loop through all ports
        if (interface) {
            VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI));
            while (icli_port_iter_getnext(&pit, port_list)) {
                T_I("port:%u, usid:%d, no:%d, both:%d, rx:%d, tx:%d, global_conf.dst_port:%d, global_conf.mirror_switch:%d, sit.isid:%d, pit.iport:%d",
                    pit.uport, sit.usid, no, both, rx, tx, global_conf.dst_port, global_conf.mirror_switch, sit.isid, pit.iport);

                if (no) {// No command
                    switch_conf.src_enable[pit.iport] = FALSE;
                    switch_conf.dst_enable[pit.iport] = FALSE;
                } else {
                    switch_conf.src_enable[pit.iport] = both | rx;
                    switch_conf.dst_enable[pit.iport] = both | tx;

                    if (pit.iport == global_conf.dst_port && global_conf.mirror_switch == sit.isid && switch_conf.dst_enable[pit.iport] == TRUE) {
                        // Note: This is done in mirror.c when the configuration is updated, so here we just print information.
                        ICLI_PRINTF("Setting Tx mirroring for %s has no effect (because this interface is the destination interface). Tx mirroring ignored.\n",
                                    icli_port_info_txt(sit.usid, pit.uport, str_buf));
                    }
                }
            }
        }

#ifdef VTSS_FEATURE_MIRROR_CPU
        if (cpu) {
            T_I("no:%d, both:%d, rx:%d, tx:%d, switch_conf.cpu_src_enable:%d, switch_conf.cpu_dst_enable:%d",
                no, both, rx, tx, switch_conf.cpu_src_enable, switch_conf.cpu_dst_enable );
            if (no) {
                switch_conf.cpu_src_enable = FALSE;
                switch_conf.cpu_dst_enable = FALSE;
            } else {
                switch_conf.cpu_src_enable = both | rx;
                switch_conf.cpu_dst_enable = both | tx;
            }

            T_I("no:%d, both:%d, rx:%d, tx:%d, switch_conf.cpu_src_enable:%d, switch_conf.cpu_dst_enable:%d",
                no, both, rx, tx, switch_conf.cpu_src_enable, switch_conf.cpu_dst_enable );
        }
#endif //VTSS_FEATURE_MIRROR_CPU

        mirror_mgmt_switch_conf_set(sit.isid, &switch_conf);
    }

    return VTSS_RC_OK;
}

/***************************************************************************/
/*  functions called from iCLI                                             */
/****************************************************************************/

// See mirror_icli_functions.h
vtss_rc mirror_destination(i32 session_id, const icli_switch_port_range_t *in_port_type, BOOL no)
{
    mirror_conf_t        conf;
    mirror_mgmt_conf_get(&conf);
    vtss_port_no_t uport, iport;

    if (no) { // No command
        conf.dst_port = MIRROR_PORT_DEFAULT; // Disabling mirroring
        conf.mirror_switch = MIRROR_SWITCH_DEFAULT;
    } else {
        uport = in_port_type->begin_uport;
        iport = uport2iport(uport);

        T_I("Session_Id:%u, iport:%u, uport:%u, no:%d",
            session_id, iport, uport, no);

        if (iport && PORT_NO_IS_STACK(iport)) {
            ICLI_PRINTF("Stack port %u cannot be destination port\n", uport);
            return VTSS_RC_ERROR;
        }

        conf.dst_port = iport;
        conf.mirror_switch = topo_usid2isid(in_port_type->switch_id);
    }

    VTSS_ICLI_ERR_PRINT(mirror_mgmt_conf_set(&conf));
    return VTSS_RC_OK;
}


// See mirror_icli_functions.h
vtss_rc mirror_source(i32 session_id, BOOL interface, icli_stack_port_range_t *port_list, BOOL cpu,
                      const icli_range_t *cpu_switch_range, BOOL both, BOOL rx, BOOL tx, BOOL no)
{
    VTSS_ICLI_ERR_PRINT(mirror_source_conf(session_id, interface, port_list, cpu, cpu_switch_range, both, rx, tx, no));
    return VTSS_RC_OK;
}


// Runtime function for ICLI that determine if CPU mirroring is supported (e.g. Jaguar doesn't support this)
BOOL mirror_cpu_runtime(u32                session_id,
                        icli_runtime_ask_t ask,
                        icli_runtime_t     *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
#ifdef VTSS_FEATURE_MIRROR_CPU
        runtime->present = TRUE;
#else
        runtime->present = FALSE;
#endif
        return TRUE;
    default:
        return FALSE;
    }
}

// The CPU capability determines whether to *check* if the SID should be included or not.
BOOL mirror_sid_runtime(u32                session_id,
                        icli_runtime_ask_t ask,
                        icli_runtime_t     *runtime)
{
    switch (ask) {
    case ICLI_ASK_PRESENT:
        if (mirror_cpu_runtime(session_id, ask, runtime) && runtime->present) {
            return icli_runtime_stacking(session_id, ask, runtime);
        }
        return TRUE;
    default:
        return icli_runtime_stacking(session_id, ask, runtime);
    }
}

#ifdef VTSS_SW_OPTION_ICFG
/* ICFG callback functions */

// ICFG for the global configuration
static vtss_rc mirror_global_conf(const vtss_icfg_query_request_t *req,
                                  vtss_icfg_query_result_t *result)
{
    mirror_conf_t        conf;
    i8                   str_buf[100];
    vtss_port_no_t       iport;
    vtss_isid_t          isid;
    mirror_switch_conf_t switch_conf;

    vtss_icfg_conf_print_t conf_print;
    vtss_icfg_conf_print_init(&conf_print);

    mirror_mgmt_conf_get(&conf); // Get configuration

    // Print out if not default value (or if requested to print all)
    // Mirror switch and destination port
    conf_print.is_default = conf.dst_port == MIRROR_PORT_DEFAULT;
    T_IG(TRACE_GRP_CLI, "conf.dst_port:%d, conf.mirror_switch:%d, usid:%d, uport:%d, is_default:%d, MIRROR_SWITCH_DEFAULT:%d, MIRROR_PORT_DEFAULT:%d", conf.dst_port, conf.mirror_switch, topo_isid2usid(conf.mirror_switch), iport2uport(conf.dst_port), conf_print.is_default, MIRROR_SWITCH_DEFAULT, MIRROR_PORT_DEFAULT);
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "monitor destination", "interface %s",
                                 icli_port_info_txt(topo_isid2usid(conf.mirror_switch), iport2uport(conf.dst_port), str_buf)));

    // Loop through all switches in a stack
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (msg_switch_configurable(isid)) {
            if (mirror_mgmt_switch_conf_get(isid, &switch_conf) != VTSS_RC_OK) {
                T_E("Could not get mirror cpu conf");
            }

#ifdef VTSS_FEATURE_MIRROR_CPU
            conf_print.is_default = switch_conf.cpu_src_enable == MIRROR_CPU_SRC_ENA_DEFAULT &&
                                    switch_conf.cpu_dst_enable == MIRROR_CPU_DST_ENA_DEFAULT;

            i8 stack_info_buf[100];
#if defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
            sprintf(stack_info_buf, "%s ", topo_isid2usid(isid));// Print CPU switch ID in case that it is supported (for stacking)
#else
            strcpy(stack_info_buf, "");
#endif

            VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "monitor source cpu", "%s%s", stack_info_buf,
                                         switch_conf.cpu_src_enable && switch_conf.cpu_dst_enable ? "both" :
                                         switch_conf.cpu_src_enable ? "rx" : "tx"));
#endif //VTSS_FEATURE_MIRROR_CPU

            //
            // Source ports
            //

            // We make a list of ports for each possible configuration, in order to be able to concatenate the interface list.
            BOOL default_list[VTSS_PORTS];
            BOOL rx_list[VTSS_PORTS];
            BOOL tx_list[VTSS_PORTS];
            BOOL both_list[VTSS_PORTS];

            // The found BOOLs indicates that at least one interface has the corresponding configuration.
            BOOL rx_found = FALSE;
            BOOL tx_found = FALSE;
            BOOL both_found = FALSE;
            BOOL default_found = FALSE;

            // Loop through all ports and set the BOOL list accordingly
            for (iport = 0; iport < VTSS_PORTS; iport++) {
                default_list[iport] = FALSE;
                rx_list[iport] = FALSE;
                tx_list[iport] = FALSE;
                both_list[iport] = FALSE;


                if ((switch_conf.src_enable[iport] == MIRROR_SRC_ENA_DEFAULT) &&
                    (switch_conf.dst_enable[iport] == MIRROR_DST_ENA_DEFAULT) ) {
                    default_list[iport] = TRUE;
                    default_found = TRUE;
                }

                if (switch_conf.src_enable[iport] && switch_conf.dst_enable[iport]) {
                    both_list[iport] = TRUE;
                    both_found = TRUE;
                } else if (switch_conf.src_enable[iport]) {
                    rx_list[iport] = TRUE;
                    rx_found = TRUE;

                } else if (switch_conf.dst_enable[iport]) {
                    tx_list[iport] = TRUE;
                    tx_found = TRUE;
                }
            }

            // Print out configuration if at least one  interface has that configuration
            if (default_found && req->all_defaults) {
                VTSS_RC(vtss_icfg_printf(result, "no monitor source interface %s\n", icli_port_list_info_txt(isid, &default_list[0], &str_buf[0], FALSE)));
            }

            if (rx_found) {
                VTSS_RC(vtss_icfg_printf(result, "monitor source interface %s rx\n", icli_port_list_info_txt(isid, &rx_list[0], &str_buf[0], FALSE)));
            }

            if (tx_found) {
                VTSS_RC(vtss_icfg_printf(result, "monitor source interface %s tx\n", icli_port_list_info_txt(isid, &tx_list[0], &str_buf[0], FALSE)));
            }

            if (both_found) {
                VTSS_RC(vtss_icfg_printf(result, "monitor source interface %s both\n", icli_port_list_info_txt(isid, &both_list[0], &str_buf[0], FALSE)));
            }
        }
    }
    return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc mirror_lib_icfg_init(void)
{
    vtss_rc rc;

    if ((rc = vtss_icfg_query_register(VTSS_ICFG_MIRROR_GLOBAL_CONF, "monitor", mirror_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}
#endif // VTSS_SW_OPTION_ICFG

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
