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

#include "poe_cli.h"
#include "cli.h"
#include "poe_api.h"
#include "poe_custom_api.h"
#include "cli_trace_def.h"
#include "mgmt_api.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif

#include "network.h"
#include <tftp_support.h>
#include <arpa/inet.h>
#include "firmware_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_POE

typedef struct {
    ushort           poe;
    poe_priority_t   poe_priority;
    poe_power_mgmt_t poe_power_mgmt;
    poe_mode_t       poe_mode;
    BOOL             poe_port_power;
    BOOL             poe_supply_power;
    u16              addr;
    u16              data;
} poe_cli_req_t;


static char prim_power_max_hlp[128];
static char backup_power_max_hlp[128];
static char backup_power_cmd[128] = "PoE Backup_Supply";
static char backup_power_cmd_usage[128] = "PoE Backup_Supply [<supply_power>]";


/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void poe_cli_txt_init(void)
{
    sprintf(prim_power_max_hlp,
            "Set or show the value of the primary power supply (%u-%u W), default: Show maximum primary power",
            poe_custom_get_power_supply_min(), POE_POWER_SUPPLY_MAX);

    sprintf(backup_power_max_hlp,
            "Set or show the value of the backup power supply (%u-%u W), default: Show maximum backup power",
            poe_custom_get_power_supply_min(), POE_POWER_SUPPLY_MAX);


    // If backup power supply isn't support we move the cli command to a debug command
    if (poe_mgmt_is_backup_power_supported()) {
        sprintf(backup_power_cmd,
                "PoE Backup_Supply");

        sprintf(backup_power_cmd_usage,
                "PoE Backup_Supply [<supply_power>]");
    } else {
        sprintf(backup_power_cmd,
                "Debug PoE Backup_Supply");

        sprintf(backup_power_cmd_usage,
                "Debug PoE Backup_Supply [<supply_power>]");

    }
}

void poe_cli_init(void)
{
    /* register the size required for poe req. structure */
    cli_req_size_register(sizeof(poe_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

// Function for checking is if PoE is supported for a specific port. If PoE isn't supported for the port, a printout is done.
// In : iport - Internal port
//      isid  - Internal switch id.
//      set   - TRUE if is a set command calling, else FALSE.
// Return: TRUE if PoE chipset is found for the iport, else FALSE
static BOOL is_poe_supported(vtss_port_no_t iport, vtss_isid_t isid, BOOL set, poe_chipset_t *poe_chip_found)
{
    if (poe_chip_found[iport] == NO_POE_CHIPSET_FOUND) {
        if (set) {
#if VTSS_SWITCH_STACKABLE
            CPRINTF("Switch %d, Port %u doesn't support PoE\n", topo_isid2usid(isid), iport2uport(iport));
#else
            CPRINTF("Port %u doesn't support PoE\n", iport2uport(iport));
#endif

        } else {
            CPRINTF("%-5u PoE not supported \n", iport2uport(iport));
        }
        return FALSE;
    } else {
        return TRUE;
    }
}



// Getting or setting PoE mode
static void poe_mode_conf(cli_req_t *req, poe_local_conf_t *poe_local_conf, vtss_isid_t isid)
{
    port_iter_t     pit;
    poe_cli_req_t *poe_req = req->module_req;
    if (!req->set) {
        cli_table_header("Port  Mode");
    }
    poe_chipset_t poe_chip_found[VTSS_PORTS];
    poe_mgmt_is_chip_found(isid, &poe_chip_found[0]); // Get a list with which PoE chip there is associated to the port.

    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }

            // Ignore if PoE isn't supported for this port.
            if (is_poe_supported(pit.iport, isid, req->set, &poe_chip_found[0])) {
                T_DG_PORT(VTSS_TRACE_GRP_POE, pit.iport, "enabled = %d, req->poe_mode = %d", poe_local_conf->poe_mode[pit.iport], poe_req->poe_mode);
                if (req->set) {
                    poe_local_conf->poe_mode[pit.iport] = poe_req->poe_mode;
                } else {
                    char *poe_mode_str[] = {"Disabled", "PoE", "PoE+"}; // Use for converting a poe_mode_t variable to string.
                    CPRINTF("%-5u %s \n", pit.uport, poe_mode_str[poe_local_conf->poe_mode[pit.iport]]);
                }
            }
        }
    }
}



// Getting or setting PoE priority
static void poe_priority_conf(cli_req_t *req, vtss_isid_t isid, poe_local_conf_t *poe_local_conf)
{
    char *poe_priority_str[] = {"Low", "High", "Critical"}; // Use for converting a poe_priority_t variable to string.
    poe_cli_req_t *poe_req = req->module_req;
    port_iter_t        pit;

    if (!req->set) {
        cli_table_header("Port  Priority");
    }

    poe_chipset_t poe_chip_found[VTSS_PORTS];
    poe_mgmt_is_chip_found(isid, &poe_chip_found[0]); // Get a list with which PoE chip there is associated to the port.

    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }

            // Ignore if PoE isn't supported for this port.
            if (is_poe_supported(pit.iport, isid, req->set, &poe_chip_found[0])) {
                if (req->set) {
                    if (poe_req->poe_priority == LOW) {
                        T_D("Setting PoE priority to LOW");
                        poe_local_conf->priority[pit.iport] = LOW;
                    } else if (poe_req->poe_priority == HIGH) {
                        T_D("Setting PoE priority to HIGH");
                        poe_local_conf->priority[pit.iport] = HIGH;
                    } else if (poe_req->poe_priority == CRITICAL) {
                        T_D("Setting PoE priority to CRITICAL");
                        poe_local_conf->priority[pit.iport] = CRITICAL;
                    } else {
                        // This shall never happen
                        T_E("Unknown priority");
                    }
                } else {
                    CPRINTF("%-5u %s \n", pit.uport, poe_priority_str[poe_local_conf->priority[pit.iport]]);
                }
            }
        }
    }
}

// Setting or Getting Maximum power per port
static void poe_port_power_conf(cli_req_t *req, vtss_isid_t isid, poe_local_conf_t *poe_local_conf)
{
    u16 port_power;
    poe_cli_req_t *poe_req = req->module_req;
    char txt_string[50];
    port_iter_t        pit;

    if (!req->set) {
        cli_table_header("Port  Max. Power [W]");
    }

    poe_chipset_t poe_chip_found[VTSS_PORTS];
    poe_mgmt_is_chip_found(isid, &poe_chip_found[0]); // Get a list with which PoE chip there is associated to the port.

    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {

            if (req->uport_list[pit.uport] == 0) {
                continue;
            }

            // Ignore if PoE isn't supported for this port.
            if (is_poe_supported(pit.iport, isid, req->set, &poe_chip_found[0])) {
                if (req->set) {
                    T_D("Setting max port power to %d, poe_max_power_mode_dependent = %d",
                        poe_req->poe, poe_max_power_mode_dependent(pit.iport, poe_local_conf->poe_mode[pit.iport]));

                    if (poe_req->poe > poe_max_power_mode_dependent(pit.iport, poe_local_conf->poe_mode[pit.iport])) {
                        port_power = poe_max_power_mode_dependent(pit.iport, poe_local_conf->poe_mode[pit.iport]);
                        CPRINTF("Maximum allowed power (for the current mode) for port %u is limited to %s W\n", pit.uport, one_digi_float2str(port_power, &txt_string[0]));
                    } else {
                        port_power = poe_req->poe;
                    }

                    poe_local_conf->max_port_power[pit.iport] = port_power;
                } else {
                    CPRINTF("%-5u %s\n", pit.uport, one_digi_float2str(poe_local_conf->max_port_power[pit.iport], &txt_string[0]));
                }
            }
        }
    }
}

// Setting or Getting primary power
static void poe_primary_power_conf(cli_req_t *req, poe_local_conf_t *poe_local_conf)
{
    poe_cli_req_t *poe_req = req->module_req;

    if (req->set) {
        T_D("Setting primary power to %d", poe_req->poe);
        poe_local_conf->primary_power_supply = poe_req->poe;
    } else {
        cli_table_header("\nPrimary Power Supply");
        CPRINTF("%d [W]\n", poe_local_conf->primary_power_supply);
    }
}

// Setting or Getting backup power
static void poe_backup_power_conf(cli_req_t *req, poe_local_conf_t *poe_local_conf)
{
    poe_cli_req_t *poe_req = req->module_req;

    if (poe_mgmt_is_backup_power_supported()) {
        if (req->set) {
            T_D("Setting backup power to %d", poe_req->poe);
            poe_local_conf->backup_power_supply = poe_req->poe;
        } else {
            cli_table_header("\nBackup Power Supply");
            CPRINTF("%d [W]\n", poe_local_conf->backup_power_supply);
        }
    }
}


// Setting or Getting power management
static void poe_power_mgmt_conf(cli_req_t *req, poe_master_conf_t *poe_master_conf)
{
    char max_power_determined_str[100];
    char power_mgmt_str[100];
    poe_cli_req_t *poe_req = req->module_req;

    if (req->set) {
        T_DG(VTSS_TRACE_GRP_POE, "Setting PoE power mangement, poe_power_mgmt = %d", poe_req->poe_power_mgmt);
        poe_master_conf->power_mgmt_mode = poe_req->poe_power_mgmt;

        if (poe_req->poe_power_mgmt == CLASS_RESERVED) {
            poe_master_conf->power_mgmt_mode = CLASS_RESERVED;
        }
    } else {
        cli_table_header("\nPower management");

        switch (poe_master_conf->power_mgmt_mode) {
        case CLASS_RESERVED: {
            strcpy(max_power_determined_str, "class");
            strcpy(power_mgmt_str, "reserved");
            break;
        }
        case CLASS_CONSUMP: {
            strcpy(max_power_determined_str, "class");
            strcpy(power_mgmt_str, "consumption");
            break;
        }
        case ALLOCATED_RESERVED: {
            strcpy(max_power_determined_str, "allocated");
            strcpy(power_mgmt_str, "reserved");
            break;
        }
        case ALLOCATED_CONSUMP: {
            strcpy(max_power_determined_str, "allocated");
            strcpy(power_mgmt_str, "consumption");
            break;
        }
        case LLDPMED_RESERVED: {
            strcpy(max_power_determined_str, "lldp-med");
            strcpy(power_mgmt_str, "reserved");
            break;
        }
        case LLDPMED_CONSUMP: {
            strcpy(max_power_determined_str, "lldp-med");
            strcpy(power_mgmt_str, "consumption");
            break;
        }
        default: {
            strcpy(max_power_determined_str, "");
            strcpy(power_mgmt_str, "");
            T_E("Invalid power management mode");
            break;
        }
        }
        CPRINTF("Max. port power determined by : %s. \n", max_power_determined_str);
        CPRINTF("Power management mode : %s\n", power_mgmt_str);
    }
}


// Prints the current configuration for PoE
static void poe_print_conf(cli_req_t *req, vtss_isid_t isid, poe_local_conf_t *poe_local_conf)
{
    char *poe_priority_str[] = {"Low", "High", "Critical"}; // Use for converting a poe_priority_t variable to string.
    char *poe_mode_str[] = {"Disabled", "PoE", "PoE+"}; // Use for converting a poe_mode_t variable to string.
    char txt_string[50];
    cli_table_header("Port  Mode     Priority Max.  Power [W]");
    port_iter_t        pit;

    poe_chipset_t poe_chip_found[VTSS_PORTS];
    poe_mgmt_is_chip_found(isid, &poe_chip_found[0]); // Get a list with which PoE chip there is associated to the port.

    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }

            if (is_poe_supported(pit.iport, isid, req->set, &poe_chip_found[0])) {
                CPRINTF("%-5u %-8s %-14s %-14s\n",
                        pit.uport,
                        poe_mode_str[poe_local_conf->poe_mode[pit.iport]],
                        poe_priority_str[poe_local_conf->priority[pit.iport]],
                        one_digi_float2str(poe_local_conf->max_port_power[pit.iport], &txt_string[0]));
            }
        }
    }
}


// Function for printing out status
static void poe_status(vtss_isid_t isid, BOOL debug)
{
    port_iter_t        pit;
    poe_status_t       status;
    char               txt_string[50];
    char               firmware_string[50];
    char               class_string[10];

    if (debug) {
        cli_table_header("I2C Addr.  PoE chipset found           Firmware Info  Port  PD  Class Port Status                              Power Used [W]  Current Used [mA]  ");
    } else {
        cli_table_header("Port  PD Class  Port Status                              Power Used [W]  Current Used [mA]");
    }

    poe_mgmt_get_status(isid, &status); // Update the status fields

    poe_local_conf_t poe_local_conf;
    poe_mgmt_get_local_config(&poe_local_conf, isid);

    poe_chipset_t poe_chip_found[VTSS_PORTS];
    poe_mgmt_is_chip_found(isid, &poe_chip_found[0]); // Get a list with which PoE chip there is associated to the port.
    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            if (debug) {
                poe_custom_entry_t hw_conf;

                (void) poe_custom_get_hw_config(pit.iport, &hw_conf);
                CPRINTF("0x%-8X %-27s %-15s",
                        hw_conf.i2c_addr[poe_chip_found[pit.iport]],
                        poe_chipset2txt(poe_chip_found[pit.iport], &txt_string[0]),
                        poe_mgmt_firmware_info_get(pit.iport, &firmware_string[0]));
            }


            if (is_poe_supported(pit.iport, isid, FALSE, &poe_chip_found[0])) {
                CPRINTF("%-5u %-8s %-41s %-16s %-19d \n",
                        pit.uport,
                        poe_class2str(&status, pit.iport, &class_string[0]),
                        poe_status2str(status.port_status[pit.iport], pit.iport, &poe_local_conf),
                        one_digi_float2str(status.power_used[pit.iport], &txt_string[0]),
                        status.current_used[pit.iport]);
            }
        }
    }
}

static void cli_cmd_poe_conf(cli_req_t *req, BOOL mode,
                             BOOL priority, BOOL conf, BOOL max_port_power, BOOL primary_power,
                             BOOL backup_power, BOOL power_mgmt, BOOL status, BOOL debug)
{

    vtss_usid_t      usid;
    vtss_isid_t      isid = 1;
    poe_cli_req_t   *poe_req = req->module_req;

    if (!(vtss_board_features() & VTSS_BOARD_FEATURE_POE)) {
        CPRINTF("POE not supported on this hardware platform.\n");
        return;
    }

    // We don't support setting configuration from slave
    if (cli_cmd_slave_do_not_set(req)) {
        return;
    }


    poe_local_conf_t poe_local_conf;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        if (!req->set) {
            // print switch id
            cli_cmd_usid_print(usid, req, 1);
        }

        if (status) {
            poe_status(isid, debug);
            T_RG(VTSS_TRACE_GRP_POE, "Status Done");
        } else {
            // Get current configuration
            poe_mgmt_get_local_config(&poe_local_conf, isid);


            T_DG(VTSS_TRACE_GRP_POE, "Mode = %d req->poe_mode = %d, isid:%d", mode, poe_req->poe_mode, isid);
            if (mode) {
                poe_mode_conf(req, &poe_local_conf, isid);
            }

            if (priority) {
                poe_priority_conf(req, isid, &poe_local_conf);
            }

            if (conf) {
                poe_print_conf(req, isid, &poe_local_conf);
            }

            if (max_port_power) {
                poe_port_power_conf(req, isid, &poe_local_conf);
            }

            if (primary_power) {
                poe_primary_power_conf(req, &poe_local_conf);
            }

            if (backup_power) {
                poe_backup_power_conf(req, &poe_local_conf);
            }

            if (power_mgmt) {
                T_NG(VTSS_TRACE_GRP_POE, "calling poe_power_mgmt_conf");
                poe_master_conf_t poe_master_conf;
                poe_mgmt_get_master_config(&poe_master_conf); // Get current configuration for the master
                poe_power_mgmt_conf(req, &poe_master_conf);
                if (req->set) {
                    poe_mgmt_set_master_config(&poe_master_conf);
                }
            }

            // Write back new configuration
            if (req->set) {
                poe_mgmt_set_local_config(&poe_local_conf, isid);
            }
        }
    }
    T_RG(VTSS_TRACE_GRP_POE, "Done");
}

static void cli_cmd_poe_conf_disp(cli_req_t *req)
{
    if (!req->set) {
        cli_header("PoE Configuration", 1);
    }
    cli_cmd_poe_conf(req, 0, 0, 1, 0, 1, 1, 1, 0, 0);
    return;
}

static void cli_cmd_poe_mode(cli_req_t *req)
{
    cli_cmd_poe_conf(req, 1, 0, 0, 0, 0, 0, 0, 0, 0);
    return;
}

static void cli_cmd_poe_prio(cli_req_t *req)
{
    cli_cmd_poe_conf(req, 0, 1, 0, 0, 0, 0, 0, 0, 0);
    return;
}

static void cli_cmd_poe_power_mgmt(cli_req_t *req)
{
    cli_cmd_poe_conf(req, 0, 0, 0, 0, 0, 0, 1, 0, 0);
    return;
}

static void cli_cmd_poe_max_power(cli_req_t *req)
{
    cli_cmd_poe_conf(req, 0, 0, 0, 1, 0, 0, 0, 0, 0);
    return;
}

static void cli_cmd_poe_primary_power_supply(cli_req_t *req)
{
    cli_cmd_poe_conf(req, 0, 0, 0, 0, 1, 0, 0, 0, 0);
    return;
}

static void cli_cmd_poe_backup_power_supply(cli_req_t *req)
{
    cli_cmd_poe_conf(req, 0, 0, 0, 0, 0, 1, 0, 0, 0);
    return;
}

static void cli_cmd_poe_status(cli_req_t *req)
{
    cli_cmd_poe_conf(req, 0, 0, 0, 0, 0, 0, 0, 1, 0);
    return;
}

static void cli_cmd_poe_debug_status(cli_req_t *req)
{
    cli_cmd_poe_conf(req, 0, 0, 0, 0, 0, 0, 0, 1, 1);
    return;
}

// Function for downloading new PoE chipset firmware
static void cli_cmd_poe_debug_firmware_update(cli_req_t *req)
{
    char server[sizeof "000.000.000.000"];
    char *file = req->parm;
    vtss_ipv4_t ipv4_server;
    struct sockaddr_in host;
    struct hostent *hp;
    int res, err;
    u8 *buffer;
    unsigned long bufferlen = 200000;

    memset((char *)&host, 0, sizeof(host)); // Initialize host

    // Set up host address
    host.sin_family = AF_INET;
    host.sin_len    = sizeof(host);
    host.sin_port   = 0;
    if (!inet_aton(req->host_name, &host.sin_addr)) {
        hp = gethostbyname(req->host_name);
        if (hp == NULL) {
            CPRINTF("*** Invalid IP address: %s\n", req->host_name);
            return;
        } else {
            memmove(&host.sin_addr, hp->h_addr, hp->h_length);
        }
    }
    ipv4_server = htonl(host.sin_addr.s_addr);
    (void) misc_ipv4_txt(ipv4_server, server);
    if (!(buffer = VTSS_MALLOC(bufferlen))) {
        T_E("Unable to allocate download buffer, aborted download\n");
        return;
    }

    // Do the TFTP download
    printf("Starting TFTP download \n");
    res = tftp_get(file, &host, (i8 *)buffer, bufferlen, TFTP_OCTET, &err); // (i8*)buffer = Pointer type cast to make lint happy.
    if (res <= 0) {
        // TFTP access failed
        char err_msg_str[100];
        firmware_tftp_err2str(err, &err_msg_str[0]);
        T_E("Download of %s from %s failed: - %s ", file, inet_ntoa(host.sin_addr), err_msg_str);
        VTSS_FREE(buffer);
        return;
    }

    poe_mgmt_firmware_update(buffer, res);
    VTSS_FREE(buffer);
}

/* We don't want to enable floating point operations, so this is a special
floating point parse, that take a "floating point string" with on digi
and convert it t an integer. E.g. 10.4 is converted to 104.
*/
static int cli_parse_one_digi_float(char *cmd, ulong *req, ulong min, ulong max)
{
    char *dot_location;

    dot_location = strstr(cmd, ".");
    if (dot_location == NULL) {
        // The string doesn't contain a ".", so we simply adds a 0 to it. E.g 10 is converted to 100
        strcat(cmd, "0"); // Add 0 - same as mulitply with 10
    } else {
        // The string contains a ".", so we removes the dot
        strcpy(dot_location, dot_location + 1); // Remove the dot

        // Check that the string only contained one digit ( e.g. 10.33 is illegal ).
        if (strlen(dot_location) > 1) {
            return VTSS_UNSPECIFIED_ERROR;
        }
    }

    return mgmt_txt2ulong(cmd, req, min, max);
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t cli_poe_keyword_parse(char *cmd, char *cmd2, char *stx,
                                     char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    poe_cli_req_t *poe_req;

    req->parm_parsed = 1;
    poe_req = req->module_req;

    if (found != NULL) {
        if (!strncmp(found, "critical", 8)) {
            poe_req->poe_priority = CRITICAL;
        } else if (!strncmp(found, "high", 4 )) {
            T_DG(VTSS_TRACE_GRP_POE, "poe_req->poe_priority = HIGH");
            poe_req->poe_priority = HIGH;
        } else if (!strncmp(found, "low", 3 )) {
            T_DG(VTSS_TRACE_GRP_POE, "poe_req->poe_priority = LOW");
            poe_req->poe_priority = LOW;
        } else if (!strncmp(found, "class_con", strlen("class_con"))) {
            poe_req->poe_power_mgmt = CLASS_CONSUMP;
        } else if (!strncmp(found, "class_res", strlen("class_res"))) {
            T_DG(VTSS_TRACE_GRP_POE, "req->poe_power_mgmt = CLASS_RESERVED");
            poe_req->poe_power_mgmt = CLASS_RESERVED;
        } else if (!strncmp(found, "al_con", strlen("al_con"))) {
            poe_req->poe_power_mgmt = ALLOCATED_CONSUMP;
        } else if (!strncmp(found, "al_res", strlen("al_res"))) {
            poe_req->poe_power_mgmt = ALLOCATED_RESERVED;
        } else if (!strncmp(found, "lldp_res", strlen("lldp_res"))) {
            poe_req->poe_power_mgmt = LLDPMED_RESERVED;
        } else if (!strncmp(found, "lldp_con", strlen("lldp_con"))) {
            poe_req->poe_power_mgmt = LLDPMED_CONSUMP;
        } else if (!strncmp(found, "disabled", strlen("disabled"))) {
            poe_req->poe_mode = POE_MODE_POE_DISABLED;
        } else if (!strncmp(found, "poe+", strlen("poe+"))) {
            poe_req->poe_mode = POE_MODE_POE_PLUS;
        } else if (!strncmp(found, "poe", strlen("poe"))) {
            poe_req->poe_mode = POE_MODE_POE;
        }
    }

    T_DG(VTSS_TRACE_GRP_POE, "found = %s, poe_req->poe_mode = %d", found, poe_req->poe_mode);
    return (found == NULL ? 1 : 0);
}

// Parsing the "addr" parameter and storing the result in the poe_req->addr field.
static int32_t cli_poe_port_addr_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    poe_cli_req_t *poe_req;
    ulong value = 0;
    int error = 0;

    req->parm_parsed = 1;
    poe_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    poe_req->addr = (u16) value;

    return error;
}

// Parsing the "addr" parameter and storing the result in the poe_req->data field.
static int32_t cli_poe_port_data_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    poe_cli_req_t *poe_req;
    ulong value = 0;
    int error = 0;

    req->parm_parsed = 1;
    poe_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
    poe_req->data = (u16) value;
    return error;
}

static int32_t cli_poe_port_power_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    poe_cli_req_t *poe_req;
    ulong value = 0;
    int error = 0;

    req->parm_parsed = 1;
    poe_req = req->module_req;
    error = cli_parse_one_digi_float(cmd, &value, 0, 0xFFFFFFFF);
    poe_req->poe = value;

    return error;
}

static int32_t cli_poe_supply_power_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    poe_cli_req_t *poe_req;
    ulong value = 0;
    int error = 0;

    req->parm_parsed = 1;
    poe_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, poe_custom_get_power_supply_min(), POE_POWER_SUPPLY_MAX);

    if (error != VTSS_OK) {
        CPRINTF("Primary_Supply must be between %d and %d W\n",
                poe_custom_get_power_supply_min(), POE_POWER_SUPPLY_MAX);
    }
    poe_req->poe = value;

    return error;
}

// Setting/getting the capacitor detection (DEBUG function for pd690xxx only)
static void poe_cli_cmd_debug_cap_detection(cli_req_t *req)
{
    // Since this is for debugging this is only supported for port 0
    if (req->set)   {
        poe_mgmt_capacitor_detection_set(0, req->enable);
    } else {
        CPRINTF("%s \n", cli_bool_txt(poe_mgmt_capacitor_detection_get(0)));
    }
}


// Debug command for reading PoE chipset registers
static void poe_cli_cmd_debug_rd(cli_req_t *req)
{
    u16 data;
    poe_cli_req_t *poe_req = req->module_req;
    port_iter_t pit;

    if (port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }
            // Read the register and print result.
            poe_mgmt_reg_rd(pit.iport, poe_req->addr, &data);
            CPRINTF("RegAddr:0x%X, Value:0x%X \n", poe_req->addr, data);
        }
    }
}

// Debug command for writing PoE chipset registers
static void poe_cli_cmd_debug_wr(cli_req_t *req)
{
    port_iter_t pit;
    poe_cli_req_t *poe_req = req->module_req;
    if (port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }

            // Write the register
            poe_mgmt_reg_wr(pit.iport, poe_req->addr, poe_req->data);
        }
    }
}

static int32_t cli_firmware_file_name_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                             cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_raw(cmd_org, req);

    return error;
}



/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t poe_cli_parm_table[] = {
    {
        "low|high|critical",
        "low     : Set priority to low\n"
        "high    : Set priority to high\n"
        "critival: Set priority to critical\n"
        "(default: Show PoE priority)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_poe_keyword_parse,
        cli_cmd_poe_prio
    },
    {
        // PoE power managment
        "class_con|class_res|al_con|al_res|lldp_res|lldp_con",
        "class_con : Max. port power determined by class, and power management mode to consumption  \n"
        "class_res : Max. port power determined by class, and power management mode to reserved power\n"
        "al_con    : Max. port power determined by allocation, and power management mode to consumption  \n"
        "al_res    : Max. port power determined by allocation, and power management mode to reserved power \n"
        "lldp_con  : Max. port power determined by lldp media, and power management mode to consumption  \n"
        "lldp_res  : Max. port power determined by lldp media, and power management mode to reserved power \n"
        "(default: Show PoE power management)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_poe_keyword_parse,
        cli_cmd_poe_power_mgmt
    },
    {
        "disabled|poe|poe+",
        "disables      : Disable PoE\n"
        "poe: Enables PoE IEEE 802.3af (Class 4 limited to 15.4W)\n"
        "poe+: Enables PoE+ IEEE 802.3at (Class 4 limited to 30W)\n"
        "(default: Show PoE's mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_poe_keyword_parse,
        cli_cmd_poe_mode
    },

    {
        "<port_power>",
        "PoE maximum power for the port (0-15.4 Watt for PoE mode, 0-30.0 Watt for PoE+ mode)",
        CLI_PARM_FLAG_SET,
        cli_poe_port_power_parse,
        cli_cmd_poe_max_power
    },
    {
        "<supply_power>",
        "PoE power for a power supply",
        CLI_PARM_FLAG_SET,
        cli_poe_supply_power_parse,
        NULL
    },

    {
        "enable|disable",
        "enable : Enable  - Enable legacy capacitor detection \n"
        "disable: Disable - Disable legacy capacitor detection \n"
        "(default: Show legacy capacitor detection state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        poe_cli_cmd_debug_cap_detection,
    },
    {
        "<addr>",
        "address",
        CLI_PARM_FLAG_SET,
        cli_poe_port_addr_parse,
        poe_cli_cmd_debug_rd,
    },
    {
        "<addr>",
        "address",
        CLI_PARM_FLAG_SET,
        cli_poe_port_addr_parse,
        poe_cli_cmd_debug_wr,
    },
    {
        "<data>",
        "data",
        CLI_PARM_FLAG_SET,
        cli_poe_port_data_parse,
        poe_cli_cmd_debug_wr,
    },
    {
        "<file_name>",
        "PoE Firmware file name",
        CLI_PARM_FLAG_NONE,
        cli_firmware_file_name_parse,
        NULL
    },


    {NULL, NULL, 0, 0, NULL}
};


/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

/* POE CLI Command sorting order */
enum {
    CLI_CMD_POE_CONF = 0,
    CLI_CMD_POE_MODE,
    CLI_CMD_POE_PRIO_PRIO,
    CLI_CMD_POE_POWER_MGMT_PRIO,
    CLI_CMD_POE_MAX_POWER_PRIO,
    CLI_CMD_POE_STATUS_PRIO,
    CLI_CMD_POE_PRIMARY_POWER_SUPPLY_PRIO,
    CLI_CMD_POE_BACKUP_POWER_SUPPLY_PRIO,
};

/* Command table entries */
cli_cmd_tab_entry(
    "PoE Configuration [<port_list>]",
    NULL,
    "Show PoE configuration",
    CLI_CMD_POE_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_POE,
    cli_cmd_poe_conf_disp,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    "PoE Mode [<port_list>]",
    "PoE Mode [<port_list>] [disabled|poe|poe+]",
    "Set or show PoE mode",
    CLI_CMD_POE_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_POE,
    cli_cmd_poe_mode,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "PoE Priority [<port_list>]",
    "PoE Priority [<port_list>] [low|high|critical]",
    "Set or show PoE Priority",
    CLI_CMD_POE_PRIO_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_POE,
    cli_cmd_poe_prio,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "PoE Mgmt_mode",
    "PoE Mgmt_mode [class_con|class_res|al_con|al_res|lldp_res|lldp_con]",
    "Set or show PoE management mode",
    CLI_CMD_POE_POWER_MGMT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_POE,
    cli_cmd_poe_power_mgmt,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "PoE Maximum_Power",
    "PoE Maximum_Power [<port_list>] [<port_power>] ",
    "Set or show PoE maximum power per port (0-30 Watt), with one digit)",
    CLI_CMD_POE_MAX_POWER_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_POE,
    cli_cmd_poe_max_power,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "PoE Primary_Supply",
    "PoE Primary_Supply [<supply_power>]",
    prim_power_max_hlp,
    CLI_CMD_POE_PRIMARY_POWER_SUPPLY_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_POE,
    cli_cmd_poe_primary_power_supply,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    backup_power_cmd,
    backup_power_cmd_usage,
    backup_power_max_hlp,
    CLI_CMD_POE_BACKUP_POWER_SUPPLY_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_POE,
    cli_cmd_poe_backup_power_supply,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "PoE Status",
    NULL,
    "Show PoE status",
    CLI_CMD_POE_STATUS_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_POE,
    cli_cmd_poe_status,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug PoE Cap_Detection",
    "Debug PoE Cap_Detection [enable|disable]",
    "Set or show if the current value of the capacitor detection",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    poe_cli_cmd_debug_cap_detection,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "Debug PoE Status",
    "Debug PoE status",
    "Shows different PoE status information",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_poe_debug_status,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug PoE Firmware Load",
    "Debug PoE Firmware Load <ip_addr_string> <file_name>",
    "Update the PoE chipset firmware",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_poe_debug_firmware_update,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "Debug PoE Reg_Rd",
    "Debug PoE Reg_Rd <port_list> <addr>",
    "Read from PoE chipset",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    poe_cli_cmd_debug_rd,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug PoE Reg_Wr",
    "Debug PoE Reg_Wr <port_list> <addr> <data>",
    "Write to PoE chipset",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    poe_cli_cmd_debug_wr,
    NULL,
    poe_cli_parm_table,
    CLI_CMD_FLAG_NONE
);



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
