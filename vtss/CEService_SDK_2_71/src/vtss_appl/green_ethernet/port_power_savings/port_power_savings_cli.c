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

#include "cli.h"
#ifdef VTSS_SW_OPTION_EEE
#include "eee_api.h"
#endif
#include "port_api.h"
#include "cli.h"
#include "mgmt_api.h"
#include "vtss_phy_api.h"

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_CLI

typedef struct {
    /* Keywords */
    BOOL              actiphy;
    BOOL              perfectreach;
#ifdef VTSS_SW_OPTION_EEE
#if EEE_FAST_QUEUES_CNT > 0
    BOOL fast_queues;
    BOOL eee_fast_queues_list[EEE_FAST_QUEUES_MAX + 1];
#endif
#if EEE_OPTIMIZE_SUPPORT == 1
    BOOL eee_optimized_for_power;
#endif
    BOOL is_tx;
    BOOL raw;
    u16  wakeup_time;
#endif
} port_power_savings_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/
void eee_cli_init(void)
{
    /* register the size required for eee req. structure */
    cli_req_size_register(sizeof(port_power_savings_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

// Converting a list of booleans to a ulong
//
// In : bool_list - The list of booleans
//      start_index - Where is the first valid boolean in the list (Normally the list will start from 0 or 1)
//      list_length - The length of the list
//
// return - A ulong mask corresponding to the boolean list.
ulong  emgmt_bool_list2ulong(BOOL *bool_list, u8 start_index, u8 list_length)
{
    u16 i;
    ulong result = 0;
    bool_list += start_index;
    for (i = 0; i < list_length ; i++) {
        if (*bool_list) {
            result |= (1 << i);
        }
        bool_list++;
    }
    return result;
}

/* commands */
static void cli_cmd_port_power_savings(cli_req_t *req, BOOL fast_queues, BOOL mode, BOOL phy_power, BOOL eee_optimize)
{
#ifdef VTSS_SW_OPTION_EEE
    eee_switch_conf_t  eee_switch_conf;
    eee_switch_global_conf_t  eee_switch_global_conf;
    eee_switch_state_t eee_switch_state;
    vtss_rc            rc;
#endif
    i8                 header_txt[255];
    switch_iter_t      sit;
    port_conf_t        port_conf;

#ifdef VTSS_SW_OPTION_EEE
#if EEE_FAST_QUEUES_CNT > 0 || defined(VTSS_SW_OPTION_PHY_POWER_CONTROL)
    port_power_savings_cli_req_t      *port_power_savings_req = req->module_req;
#endif
#endif
    if (cli_cmd_conf_slave(req) || cli_cmd_switch_none(req)) {
        return;
    }


    /* Get/set port configuration */
    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;

#ifdef VTSS_SW_OPTION_EEE
        // Get configuration for the current switch
        if ((rc = eee_mgmt_switch_conf_get(sit.isid, &eee_switch_conf)) != VTSS_RC_OK) {
            CPRINTF("Conf get: %s\n", error_txt(rc));
            return;
        }
        if ((rc = eee_mgmt_switch_state_get(sit.isid, &eee_switch_state)) != VTSS_RC_OK) {
            CPRINTF("State get: %s\n", error_txt(rc));
            return;
        }

        // Global settings
#if EEE_OPTIMIZE_SUPPORT == 1
        if (eee_optimize) {
            // Get configuration for the current switch
            if ((rc = eee_mgmt_switch_global_conf_get(&eee_switch_global_conf)) != VTSS_RC_OK) {
                CPRINTF("Conf get: %s\n", error_txt(rc));
                return;
            }

            if (req->set) {
                eee_switch_global_conf.optimized_for_power = port_power_savings_req->eee_optimized_for_power;
            } else {
                cli_table_header("EEE optimized for  "); // Add the header for the configuration entries that needs to be printed.
                CPRINTF("%-19s \n", eee_switch_global_conf.optimized_for_power ? "Power" : "Latency");
            }
        }
#endif
#endif
        //   Local port settings

        // Check if there is any port requests, if none we skip the port configuration and goes the configuration update.
        if (!phy_power && !fast_queues && !mode) {
            goto update_conf;
        }

        // Loop through all ports
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            // Getting status (used to check power saving capabilities)
            port_status_t      port_status;
            if (port_mgmt_status_get(sit.isid, pit.iport, &port_status) != VTSS_RC_OK) {
                T_E("Could not get port status for port:%u", pit.iport);
                continue;
            }

            // Setting new configuration
            if (req->set) {
#ifdef VTSS_SW_OPTION_EEE
                if (mode
#if EEE_FAST_QUEUES_CNT > 0
                    || fast_queues
#endif
                   ) {
                    if (!eee_switch_state.port[pit.iport].eee_capable) {
                        CPRINTF("Port %u is not EEE capable. Skipping\n", pit.uport);
                        continue;
                    }
                }

                if (mode) {
                    eee_switch_conf.port[pit.iport].eee_ena = req->enable;
                }

#if EEE_FAST_QUEUES_CNT > 0
                if (fast_queues) {
                    eee_switch_conf.port[pit.iport].eee_fast_queues = emgmt_bool_list2ulong(port_power_savings_req->eee_fast_queues_list, EEE_FAST_QUEUES_MIN, EEE_FAST_QUEUES_MAX);
                }

                T_R("%u", emgmt_bool_list2ulong(port_power_savings_req->eee_fast_queues_list, EEE_FAST_QUEUES_MIN, EEE_FAST_QUEUES_MAX));
#endif

#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
                if (phy_power) {
                    //
                    // Get port configuration in order to update configuration
                    //
                    if (port_mgmt_conf_get(sit.isid, pit.iport, &port_conf) != VTSS_OK) {
                        T_E("Could not get port configuration");
                    }

                    port_conf.power_mode = (req->disable ? VTSS_PHY_POWER_NOMINAL :
                                            port_power_savings_req->actiphy ? VTSS_PHY_POWER_ACTIPHY :
                                            port_power_savings_req->perfectreach ? VTSS_PHY_POWER_DYNAMIC :
                                            VTSS_PHY_POWER_ENABLED);


                    //
                    // Check that port supports Actiphy and Perfectreach before applying.
                    //
                    if (!port_status.power.actiphy_capable) {
                        if (port_conf.power_mode == VTSS_PHY_POWER_ACTIPHY || port_conf.power_mode == VTSS_PHY_POWER_ENABLED) {
                            CPRINTF("Port %u is not ActiPHY capable. Skipping\n", pit.uport);
                            continue;
                        }
                    }

                    if (!port_status.power.perfectreach_capable) {
                        if (port_conf.power_mode == VTSS_PHY_POWER_DYNAMIC || port_conf.power_mode == VTSS_PHY_POWER_ENABLED) {
                            CPRINTF("Port %u is not PerfectReach capable. Skipping\n", pit.uport);
                            continue;
                        }
                    }


                    //
                    // Do the configuration update
                    //
                    if (port_mgmt_conf_set(sit.isid, pit.iport, &port_conf) != VTSS_OK) {
                        T_E("Could not set port configuration");
                    }
                }
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */


#endif
                continue;
            }


            // Print out table header
            if (pit.first) {
                strcpy(header_txt, "Port  ");


#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
                if (phy_power) {
                    strcat(header_txt, "Power         "); // Must to able to show "PerfectReach"
                }
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */

#ifdef VTSS_SW_OPTION_EEE
                // Add the header for the configuration entries that needs to be printed.
                if (mode) {
                    strcat(header_txt, "EEE Mode  ");
                }


#if EEE_FAST_QUEUES_CNT > 0
                if (fast_queues) {
                    strcat(header_txt, "Urgent queues  ");
                }
#endif
#endif
                cli_cmd_usid_print(sit.usid, req, 1);
                cli_table_header(header_txt);
            }

            // Print out the table entries
            CPRINTF("%-6u", pit.uport);

#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
            if (phy_power) {
                // Get port configuration
                if (port_mgmt_conf_get(sit.isid, pit.iport, &port_conf) != VTSS_OK) {
                    T_E("Could not get port configuration");
                }

                T_D("port_status.power.actiphy_capable:%d, port_status.power.perfectreach_capable:%d", port_status.power.actiphy_capable, port_status.power.perfectreach_capable);

                // If Neither Perfectreach or ActiPhy is supported for the port then show N/A
                if (!port_status.power.actiphy_capable && !port_status.power.perfectreach_capable) {
                    CPRINTF("%-14s", "N/A");
                } else {
                    CPRINTF("%-14s",
                            port_conf.power_mode == VTSS_PHY_POWER_ACTIPHY ? "ActiPHY" :
                            port_conf.power_mode == VTSS_PHY_POWER_DYNAMIC ? "PerfectReach" :
                            cli_bool_txt(port_conf.power_mode == VTSS_PHY_POWER_ENABLED));
                }
            }
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */

#ifdef VTSS_SW_OPTION_EEE
            if (mode) {
                CPRINTF("%-10s", eee_switch_state.port[pit.iport].eee_capable ? cli_bool_txt(eee_switch_conf.port[pit.iport].eee_ena) : "N/A");
            }


#if EEE_FAST_QUEUES_CNT > 0
            if (fast_queues) {
                BOOL bool_list[32];
                char buf[200];

                (void)mgmt_ulong2bool_list((ulong) eee_switch_conf.port[pit.iport].eee_fast_queues, &bool_list[0]);
                (void)mgmt_list2txt(&bool_list[0], 0, EEE_FAST_QUEUES_CNT, &buf[0]);

                if (strlen(buf) == 0) {
                    strcpy(buf, "none" );
                }

                CPRINTF("%-12s", eee_switch_state.port[pit.iport].eee_capable ? buf : "N/A");
            }
#endif
#endif
            CPRINTF("\n");
        }

        // Do the new configuration
update_conf:
        if (req->set) {
#ifdef VTSS_SW_OPTION_EEE
            if ((rc = eee_mgmt_switch_conf_set(sit.isid, &eee_switch_conf)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
            }
            if ((rc = eee_mgmt_switch_global_conf_set(&eee_switch_global_conf)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
            }
#endif
        }
    }
}


#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
static void cli_cmd_port_power(cli_req_t *req)
{
    cli_cmd_port_power_savings(req, 0, 0, 1, 0);
}
#endif

#ifdef VTSS_SW_OPTION_EEE
static void eee_cli_cmd_conf(cli_req_t *req)
{
    if (!req->set) {
        cli_header("EEE Configuration", 1);
    }
    cli_cmd_port_power_savings(req, 1, 1, 1, 1);
}


static void eee_cli_cmd_mode(cli_req_t *req)
{
    cli_cmd_port_power_savings(req, 0, 1, 0, 0);
}

#if EEE_OPTIMIZE_SUPPORT == 1
static void eee_cli_cmd_optimize(cli_req_t *req)
{
    cli_cmd_port_power_savings(req, 0, 0, 0, 1);
}
#endif

#if EEE_FAST_QUEUES_CNT > 0
static void eee_cli_cmd_fast_queues(cli_req_t *req)
{
    cli_cmd_port_power_savings(req, 1, 0, 0, 0);
}
#endif

#ifdef VTSS_ARCH_LUTON26
// Debug for disabling EEE VGA power bug - Shall be removed for Luton26 Rev B.
/*lint -esym(459, eee_cli_cmd_dbg_power_down)*/
static void eee_cli_cmd_dbg_power_down(cli_req_t *req)
{
    static BOOL        vga_power_down = FALSE; // Default the VGA is power up in the phy driver
    eee_switch_state_t state;
    port_iter_t        pit;

    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    (void)eee_mgmt_switch_state_get(VTSS_ISID_LOCAL, &state);

    if (req->set) {
        vga_power_down = req->enable;
    } else {
        if (vga_power_down) {
            CPRINTF("EEE mode is in mode rev_b\n");
        } else {
            CPRINTF("EEE mode is in mode rev_a\n");
        }
    }

    while (port_iter_getnext(&pit)) {
        if (state.port[pit.iport].eee_capable) {
            if (vga_power_down) {
                vga_adc_debug(NULL, 0x3, pit.iport);
            } else {
                vga_adc_debug(NULL, 0x0, pit.iport);
            }
        }
    }
}
#endif

/******************************************************************************/
// eee_cli_cmd_dbg_wakeup_time()
/******************************************************************************/
static void eee_cli_cmd_dbg_wakeup_time(cli_req_t *req)
{
    port_power_savings_cli_req_t      *port_power_savings_req = req->module_req;
    eee_switch_state_t state;
    port_iter_t        pit;

    if (!req->set) {
        return;
    }

    (void)cli_port_iter_init(&pit, VTSS_ISID_LOCAL, PORT_ITER_FLAGS_NORMAL);
    (void)eee_mgmt_switch_state_get(VTSS_ISID_LOCAL, &state);

    while (cli_port_iter_getnext(&pit, req)) {
        if (!state.port[pit.iport].eee_capable) {
            continue;
        }
        eee_mgmt_local_state_change(pit.iport, req->clear, port_power_savings_req->is_tx, port_power_savings_req->wakeup_time);
    }
}

static char *yn_txt(BOOL val)
{
    return val ? "Yes" : "No";
}





/******************************************************************************/
// eee_cli_print_status() - Printing status - if debug set to TRUE more status is printing
//
/******************************************************************************/
void eee_cli_print_status(cli_req_t *req , BOOL debug)
{
    vtss_rc            rc;
    eee_switch_conf_t  eee_switch_conf;
    eee_switch_state_t eee_switch_state;
    switch_iter_t      sit;
    port_status_t      port_status;

    (void)cli_switch_iter_init(&sit);

    /* Show EEE port status */
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;

        // Get configuration for the current switch
        if ((rc = eee_mgmt_switch_conf_get(sit.isid, &eee_switch_conf)) != VTSS_RC_OK) {
            CPRINTF("Conf get: %s\n", error_txt(rc));
            return;
        }
        if ((rc = eee_mgmt_switch_state_get(sit.isid, &eee_switch_state)) != VTSS_RC_OK) {
            CPRINTF("State get: %s\n", error_txt(rc));
            return;
        }

        // Loop through all ports
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            eee_port_state_t *s;
            s = &eee_switch_state.port[pit.iport];
            if (port_mgmt_status_get(sit.isid, pit.iport, &port_status) != VTSS_RC_OK) {
                T_E("Could not get port status");
            }
            // Print out table header
            if (pit.first) {

                cli_cmd_usid_print(sit.usid, req, 1);
                cli_printf("Port Lnk ActiPHY PerfectReach EEE Cap LPCap PowerSave ");
                if (debug) {
                    cli_printf("RxPwr TxPwr Run SRx   STx   LRx   LTx   LRRx  LRTx  LRxE  LTxE  RRx   RTx   RRxE  RTxE  InSync\n");
                } else {
                    cli_printf("\n");
                }

                cli_printf("---- --- ------- ------------ --- --- ----- --------- ");
                if (debug) {
                    cli_printf("----- ----- --- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ------\n");
                } else {
                    cli_printf("\n");
                }
            }


            cli_printf("%4u %-3s %-7s %-12s %-3s %-3s %-5s %-9s ",
                       pit.uport,
                       yn_txt(s->link),
                       port_status.power.actiphy_capable ? yn_txt(port_status.power.actiphy_power_savings) : "N/A",
                       port_status.power.perfectreach_capable ? yn_txt(port_status.power.perfectreach_power_savings) : "N/A",
                       eee_switch_state.port[pit.iport].eee_capable ? yn_txt(eee_switch_conf.port[pit.iport].eee_ena) : "N/A",
                       yn_txt(s->eee_capable),
                       eee_switch_state.port[pit.iport].eee_capable ? yn_txt(s->link_partner_eee_capable) : "N/A",
                       eee_switch_state.port[pit.iport].eee_capable ? yn_txt(s->rx_in_power_save_state || s->tx_in_power_save_state) : "N/A");


            if (debug) {
                cli_printf("%-5s %-5s %-3s %5u %5u %5u %5u %5u %5u %5u %5u %5u %5u %5u %5u %-6s\n",
                           yn_txt(s->rx_in_power_save_state),
                           yn_txt(s->tx_in_power_save_state),
                           yn_txt(s->running),
                           s->rx_tw,
                           s->tx_tw,
                           s->LocRxSystemValue,
                           s->LocTxSystemValue,
                           s->LocResolvedRxSystemValue,
                           s->LocResolvedTxSystemValue,
                           s->LocRxSystemValueEcho,
                           s->LocTxSystemValueEcho,
                           s->RemRxSystemValue,
                           s->RemTxSystemValue,
                           s->RemRxSystemValueEcho,
                           s->RemTxSystemValueEcho,
                           yn_txt((s->LocRxSystemValue == s->RemRxSystemValueEcho) && (s->LocTxSystemValue == s->RemTxSystemValueEcho)));
            } else {
                cli_printf("\n");
            }
        }
    }
    cli_printf("\n");
}

/******************************************************************************/
// eee_cli_cmd_status()
/******************************************************************************/

void eee_cli_cmd_status(cli_req_t *req )
{
    eee_cli_print_status(req, FALSE);
}

/******************************************************************************/
// eee_cli_cmd_dbg_status()
/******************************************************************************/
void eee_cli_cmd_dbg_status(cli_req_t *req)
{
    eee_cli_print_status(req, TRUE);
}

/******************************************************************************/
// eee_cli_cmd_dbg_phy()
/******************************************************************************/
void eee_cli_cmd_dbg_phy(cli_req_t *req)
{
    vtss_rc            rc;
    eee_switch_state_t eee_switch_state;
    port_power_savings_cli_req_t     *port_power_savings_req      = req->module_req;
    BOOL               print_header = TRUE;
    port_iter_t        pit;

    (void)cli_port_iter_init(&pit, VTSS_ISID_LOCAL, PORT_ITER_FLAGS_NORMAL);

    if (port_power_savings_req->raw) {
        cli_printf("MMD address 3.1 : PCS status (LPI indication)\n");
        cli_printf("MMD address 3.20: EEE Capability\n");
        cli_printf("MMD address 3.22: Wake Error Counter\n");
        cli_printf("MMD address 7.60: Local EEE Advertisement\n");
        cli_printf("MMD address 7.61: Remote EEE Advertisement\n");
    }

    /* Show EEE PHY registers */
    if ((rc = eee_mgmt_switch_state_get(VTSS_ISID_LOCAL, &eee_switch_state)) != VTSS_RC_OK) {
        CPRINTF("State get: %s\n", error_txt(rc));
        return;
    }

    // Loop through all ports
    while (cli_port_iter_getnext(&pit, req)) {
        eee_port_state_t *s;
        port_info_t port_info;
        u16 mmd_3_01, mmd_3_20, mmd_3_22, mmd_7_60, mmd_7_61;

        (void)port_info_get(pit.iport, &port_info);
        if (!port_info.phy) {
            continue;
        }

        (void)vtss_phy_mmd_read(NULL, pit.iport, 3,  1, &mmd_3_01);
        (void)vtss_phy_mmd_read(NULL, pit.iport, 3, 20, &mmd_3_20);
        (void)vtss_phy_mmd_read(NULL, pit.iport, 3, 22, &mmd_3_22);
        (void)vtss_phy_mmd_read(NULL, pit.iport, 7, 60, &mmd_7_60);
        (void)vtss_phy_mmd_read(NULL, pit.iport, 7, 61, &mmd_7_61);

        s = &eee_switch_state.port[pit.iport];

        // Print out table header
        if (print_header) {
            if (port_power_savings_req->raw) {
                cli_printf("Port 3.1    3.20   3.22   7.60   7.61\n");
                cli_printf("---- ------ ------ ------ ------ ------\n");
            } else {
                cli_printf("         --Advertisement-- ------LPI------ ---EEE-- -Wake-\n");
                cli_printf("         Local    Remote   Sticky  Current  Capable Errors\n");
                cli_printf("Port Lnk 100 1000 100 1000 Rx  Tx  Rx  Tx  100 1000       \n");
                cli_printf("---- --- --- ---- --- ---- --- --- --- --- --- ---- ------\n");
            }

            // Only print header once
            print_header = FALSE;
        }

        if (port_power_savings_req->raw) {
            cli_printf("%4u 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", pit.uport, mmd_3_01, mmd_3_20, mmd_3_22, mmd_7_60, mmd_7_61);
        } else {
            cli_printf("%4u %-3s %-3s %-4s %-3s %-4s %-3s %-3s %-3s %-3s %-3s %-4s %6u\n",
                       pit.uport,
                       yn_txt(s->link),
                       yn_txt(mmd_7_60 & 0x0002 ? TRUE : FALSE),
                       yn_txt(mmd_7_60 & 0x0004 ? TRUE : FALSE),
                       yn_txt(mmd_7_61 & 0x0002 ? TRUE : FALSE),
                       yn_txt(mmd_7_61 & 0x0004 ? TRUE : FALSE),
                       yn_txt(mmd_3_01 & 0x0400 ? TRUE : FALSE),
                       yn_txt(mmd_3_01 & 0x0800 ? TRUE : FALSE),
                       yn_txt(mmd_3_01 & 0x0100 ? TRUE : FALSE),
                       yn_txt(mmd_3_01 & 0x0200 ? TRUE : FALSE),
                       yn_txt(mmd_3_20 & 0x0002 ? TRUE : FALSE),
                       yn_txt(mmd_3_20 & 0x0004 ? TRUE : FALSE),
                       mmd_3_22);
        }
    }
    cli_printf("\n");
}
#endif // VTSS_SW_OPTION_EEE
/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/
static int32_t cli_eee_parse_keyword (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    port_power_savings_cli_req_t *port_power_savings_req = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        } else if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
#ifdef VTSS_SW_OPTION_EEE
        } else if (!strncmp(found, "raw", 3)) {
            port_power_savings_req->raw = 1;
        } else if (!strncmp(found, "rx", 2)) {
            port_power_savings_req->is_tx = FALSE;
        } else if (!strncmp(found, "tx", 2)) {
            port_power_savings_req->is_tx = TRUE;
#if EEE_OPTIMIZE_SUPPORT == 1
        } else if (!strncmp(found, "power", 5)) {
            port_power_savings_req->eee_optimized_for_power = TRUE;
        } else if (!strncmp(found, "latency", 7)) {
            port_power_savings_req->eee_optimized_for_power = FALSE;
#endif
#endif
        }

#ifdef VTSS_ARCH_LUTON26
        if (!strncmp(found, "mode_rev_a", 10)) {
            req->disable = 1;
        } else if (!strncmp(found, "mode_rev_b", 10)) {
            req->enable = 1;
        }
#endif
        else if (!strncmp(found, "actiphy", 7)) {
            port_power_savings_req->actiphy = 1;

        } else if (!strncmp(found, "perfectreach", 12)) {
            port_power_savings_req->perfectreach = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

#ifdef VTSS_SW_OPTION_EEE
#if EEE_FAST_QUEUES_CNT > 0
// Parse fast queues
static int cli_eee_fast_queues_list_parse(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                          cli_req_t *req)
{
    port_power_savings_cli_req_t *port_power_savings_req = req->module_req;
    u16 error = 0; /* As a start there is no error */
    u16 queue;

    req->parm_parsed = 1;
    char *found = cli_parse_find(cmd, stx);

    T_D("Found %s, %d, %s ,%s", found, cli_parse_none(cmd), cmd, stx);

    if ((error = cli_parse_none(cmd)) == 0) {
        error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, port_power_savings_req->eee_fast_queues_list, EEE_FAST_QUEUES_MIN, EEE_FAST_QUEUES_MAX, 0));
    } else {
        error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, port_power_savings_req->eee_fast_queues_list, EEE_FAST_QUEUES_MIN, EEE_FAST_QUEUES_MAX, 1));
    }


    if (error && (error = cli_parse_none(cmd)) == 0) {
        for (queue = 0; queue < EEE_FAST_QUEUES_CNT; queue++) {
            port_power_savings_req->eee_fast_queues_list[queue] = 0;
        }
    }

    return (error);
}
#endif

static int cli_eee_wakeup_time_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int            error   = 0; /* As a start there is no error */
    port_power_savings_cli_req_t *port_power_savings_req = req->module_req;
    char          *found   = cli_parse_find(cmd, "clear");

    req->parm_parsed = 1;

    if (found && !strncmp(found, "clear", 5)) {
        req->clear = 1;
    } else {
        ulong value;
        error = cli_parse_ulong(cmd, &value, 0, 65535);
        port_power_savings_req->wakeup_time = (u16)value;
    }
    return error;
}
#endif


/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t port_power_savings_cli_parm_table[] = {
#ifdef VTSS_SW_OPTION_EEE
    {
        "enable|disable",
        "enable : Enable EEE\n"
        "disable: Disable EEE\n"
        "(default: Show EEE mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eee_parse_keyword,
        eee_cli_cmd_mode
    },
    {
        "rx|tx",
        "rx: Change Rx wakeup time\n"
        "tx: Change Tx wakeup time\n"
        "(no default)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eee_parse_keyword,
        eee_cli_cmd_dbg_wakeup_time
    },
    {
        "raw",
        "raw: Show PHY register readout\n"
        "(default: Interpret registers)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eee_parse_keyword,
        eee_cli_cmd_dbg_phy
    },
    {
        "<wakeup_time>",
        "EEE TX wakeup time (0-65535) or 'clear' to remove any previously set value.\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eee_wakeup_time_parse,
        eee_cli_cmd_dbg_wakeup_time
    },
#ifdef VTSS_ARCH_LUTON26
// Debug for disabling EEE VGA power bug - Shall be removed for Luton26 Rev B.
    {
        "mode_rev_a|mode_rev_b",
        "mode_rev_b : EEE mode rev_b (Max power savings)\n"
        "mode_rev_a : EEE mode rev_a (More stable, but lees power savings)\n"
        "(default: Show eee mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eee_parse_keyword,
        eee_cli_cmd_dbg_power_down
    },
#endif
#if EEE_FAST_QUEUES_CNT > 0
    {
        "<queue_list>",
        "List of queues to configure as urgent queues (1-8 or none)",
        CLI_PARM_FLAG_SET,
        cli_eee_fast_queues_list_parse,
        NULL,
    },
#endif
#if EEE_OPTIMIZE_SUPPORT == 1
    {
        "power|latency",
        "power: Configure EEE to give most power saving\n"
        "latency: Configure EEE to give least traffic latency\n"
        "(default: Show EEE optimize)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eee_parse_keyword,
        NULL,
    },
#endif
#endif
    {
        "enable|disable|actiphy|perfectreach",
        "enable : Enable all power control\n"
        "disable: Disable all power control\n"
        "actiphy: Enable ActiPHY power control\n"
        "perfectreach: Enable PerfectReach power control",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_eee_parse_keyword,
        NULL
    },
    {NULL, NULL, 0, 0, NULL},
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/
enum {
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    CLI_CMD_PORT_POWER_PRIO,
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
#ifdef VTSS_SW_OPTION_EEE
    PRIO_EEE_CONF,
    PRIO_EEE_MODE,
#if EEE_OPTIMIZE_SUPPORT == 1
    PRIO_EEE_OPTIMIZE,
#endif
    PRIO_EEE_FAST_QUEUES,
    PRIO_EEE_STATUS             = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_EEE_DBG_STATUS         = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_EEE_DBG_PHY            = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_EEE_DBG_DETAILS        = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_EEE_DBG_WAKEUP_TIME    = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_EEE_DBG_VGA_POWER_DOWN = CLI_CMD_SORT_KEY_DEFAULT, // Debug for disabling EEE VGA power bug - Shall be removed for Luton26 Rev B.
#endif
};

#ifdef VTSS_SW_OPTION_EEE
cli_cmd_tab_entry (
    "GreenEthernet Port EEE Configuration [<port_list>]",
    NULL,
    "Show EEE configuration",
    PRIO_EEE_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_EEE,
    eee_cli_cmd_conf,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "GreenEthernet Port EEE Mode [<port_list>]",
    "GreenEthernet Port EEE Mode [<port_list>] [enable|disable]",
    "Set or show the EEE mode",
    PRIO_EEE_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EEE,
    eee_cli_cmd_mode,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#if EEE_OPTIMIZE_SUPPORT == 1
cli_cmd_tab_entry (
    "GreenEthernet Port EEE Optimize",
    "GreenEthernet Port EEE Optimize [power|latency]",
    "Set or show the EEE optimize settings. EEE can be set to optimize for most power saving or least traffic latency",
    PRIO_EEE_OPTIMIZE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EEE,
    eee_cli_cmd_optimize,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif
#if EEE_FAST_QUEUES_CNT > 0
cli_cmd_tab_entry (
    "GreenEthernet Port EEE Urgent_queues",
    "GreenEthernet Port EEE Urgent_queues [<port_list>] [<queue_list>]",
    "Set or show EEE Urgent queues",
    PRIO_EEE_FAST_QUEUES,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EEE,
    eee_cli_cmd_fast_queues,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

// EEE Status
cli_cmd_tab_entry (
    "Debug EEE Status [<port_list>]",
    NULL,
    "Debug EEE",
    PRIO_EEE_DBG_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    eee_cli_cmd_dbg_status,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);



#ifdef VTSS_ARCH_LUTON26
// Debug for disabling EEE VGA power bug - Shall be removed for Luton26 Rev B.
cli_cmd_tab_entry (
    "Debug EEE Rev_mode [mode_rev_a|mode_rev_b]",
    NULL,
    "Debug EEE",
    PRIO_EEE_DBG_VGA_POWER_DOWN,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    eee_cli_cmd_dbg_power_down,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

// For debugging - Change of Rx or Tx wakeup time.
cli_cmd_tab_entry (
    NULL,
    "Debug EEE Wakeup_time [<port_list>] [rx|tx] [<wakeup_time>]",
    "Change Rx or Tx wakeup times",
    PRIO_EEE_DBG_WAKEUP_TIME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    eee_cli_cmd_dbg_wakeup_time,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "GreenEthernet Port Status [<port_list>]",
    NULL,
    "Showing current power savings status",
    PRIO_EEE_DBG_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_EEE,
    eee_cli_cmd_status,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


// For debugging
cli_cmd_tab_entry (
    "Debug EEE PHY [<port_list>] [raw]",
    NULL,
    "Debug EEE PHY (only local switch)",
    PRIO_EEE_DBG_PHY,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    eee_cli_cmd_dbg_phy,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef VTSS_ARCH_JAGUAR_1
extern void eee_cli_cmd_dbg_details(cli_req_t *cli_req);
cli_cmd_tab_entry (
    "Debug EEE Details",
    NULL,
    "Debug EEE",
    PRIO_EEE_DBG_DETAILS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    eee_cli_cmd_dbg_details,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif
#endif
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
cli_cmd_tab_entry(
    "GreenEthernet Port Power [<port_list>]",
    "GreenEthernet Port Power [<port_list>] [enable|disable|actiphy|perfectreach]",
    "Set or show the port PHY power mode",
    CLI_CMD_PORT_POWER_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_power,
    NULL,
    port_power_savings_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */


/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
