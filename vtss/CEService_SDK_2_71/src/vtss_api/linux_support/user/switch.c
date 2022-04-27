/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <getopt.h>

#include "vtss_switch_usermode.h"

#define XSTR(s) STR(s)
#define STR(s) #s

int
main(int argc, char **argv)
{
    static char *board = XSTR(VTSS_BOARD);
    static char *chiptype = XSTR(VTSS_CHIP);
    vtss_port_no_t port;
    vtss_basic_counters_t ct;
    vtss_port_status_t st;
    int option;
    int counters = 1, verbose = 0, map = 0, conf = 0, chip = 0, flush = 0, qos = 0, qos_port = 1;
    static const struct option options[] = {
        { "verbose",     0, NULL, 'v' },
        { "help",        0, NULL, 'h' },
        { "map",         0, NULL, 'm' },
        { "conf",        0, NULL, 'c' },
        { "chip",        0, NULL, 'C' },
        { "flush",       0, NULL, 'F' },
        { "qos",         required_argument, NULL, 'q' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "vhmcCFq:", options, NULL);
        if (option == -1)
            break;

        switch (option) {
        case 'v':
            verbose++;
            break;
        case 'q':
            qos++;
            qos_port = atoi(optarg) - 1;
            break;
        case 'm':
            map++;
            counters = 0;
            break;
        case 'C':
            chip++;
            counters = 0;
            break;
        case 'F':
            flush++;
            counters = 0;
            break;
        case 'c':
            conf++;
            counters = 0;
            break;
        case 'h':
            printf("Usage: %s [-hvmcCF] [-q <port>]\n"
                   " -v: Verbose\n"
                   " -q: Show QoS counters (detailed counters for one port)\n"
                   " -m: Show port map\n"
                   " -c: Show port config\n"
                   " -C: Show chip ID\n"
                   " -F: Flush MAC table\n"
                   " -h: Display this\n", argv[0]);
        default:
            goto exit;
        }
    }

    if(verbose)
        printf("Compiled for board %s, chip %s, ports: %d\n", board, chiptype, VTSS_PORTS);

    if(chip) {
        vtss_chip_id_t chip_id;
        if(vtss_chip_id_get(NULL, &chip_id) == VTSS_RC_OK) {
            printf("Chip ID: VSC%04x, revision %d\n", 
                   chip_id.part_number, chip_id.revision);
        } else {
            printf("Error in vtss_chip_id_get()\n");
        }
    }

    if(flush) {
        if(vtss_mac_table_flush(NULL) == VTSS_RC_OK) {
            printf("Flushed MAC table\n");
        } else {
            printf("Error in vtss_mac_table_flush()\n");
        }
    }

    if(map) {
        vtss_port_map_t port_map[VTSS_PORT_ARRAY_SIZE];
        if(vtss_port_map_get(NULL, port_map) == VTSS_RC_OK) {
            for(port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++)
                printf("Port %d: Chip port %d, chip %d\n", port + 1, 
                       port_map[port].chip_port, port_map[port].chip_no);
        } else {
            printf("Error in vtss_port_map_get()\n");
        }
    }

    if(conf) {
        vtss_port_conf_t port_conf;
        for(port = VTSS_PORT_NO_START; port < VTSS_PORTS; port++) {
            if(vtss_port_conf_get(NULL, port, &port_conf) == VTSS_RC_OK) {
                printf("Port %d: iftype %d, speed %d, fdx %d, fc %d, mtu %d\n", 
                       port + 1, port_conf.if_type, port_conf.speed,
                       port_conf.fdx, port_conf.flow_control.obey, port_conf.max_frame_length);
            }
        }
    }

    if(counters) {
        if(qos) {
            vtss_port_counters_t counters;
            if(vtss_port_counters_get(NULL, qos_port, &counters) == 0) {
                int i;
                printf("QoS packet counters for port %d\n", qos_port + 1);
                for(i = 0; i < 8; i++)
                    printf("Prio %2d Tx %8llu Rx %8llu\n", 
                           i, 
                           counters.prop.tx_prio[i],
                           counters.prop.rx_prio[i]);
            } else {
                printf("Port%d Qos counters error (invalid port?)\n", qos_port + 1);
            }
        } else {
            for(port = VTSS_PORT_NO_START; port < VTSS_PORTS; port++) {
                if(vtss_port_status_get(NULL, port, &st) == 0 && st.link) {
                    if(vtss_port_basic_counters_get(NULL, port, &ct) == 0) {
                        printf("Port %2d Tx %8u Rx %8u\n", port + 1, ct.tx_frames, ct.rx_frames);
                    } else {
                        printf("Port%d ctrs error\n", port + 1);
                    }
                }
            }
        }
    }

exit:
    return 0;
}
