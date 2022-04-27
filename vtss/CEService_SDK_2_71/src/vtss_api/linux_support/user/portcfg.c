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

static const char *speeds[] = {
    "Undefined",
    "10M",
    "100M",
    "1G",
    "2.5G",
    "10G",
};

int
main(int argc, char **argv)
{
    vtss_port_no_t port = 0;
    port_conf_t conf;
    int option;
    int verbose = 0, mtu = 0, max_tags = 0, has_max_tags = 0;
    static const struct option options[] = {
        { "verbose",     0, NULL, 'v' },
        { "help",        0, NULL, 'h' },
        { "port",        1, NULL, 'p' },
        { "mtu",         1, NULL, 'm' },
        { "max_tags",    1, NULL, 't' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "vhp:m:t:", options, NULL);
        if (option == -1)
            break;

        switch (option) {
        case 'v':
            verbose++;
            break;
        case 'p':
            if(sscanf(optarg, "%d", &port) != 1 ||
                port <= 0 || port > VTSS_PORTS) {
                printf("Invalid port number: %s\n", optarg);
                exit(1);
            }
            port -= 1;
            break;
        case 'm':
            mtu = atoi(optarg);
            break;

        case 't':
            max_tags = atoi(optarg);
            if (sscanf(optarg, "%d", &max_tags) != 1 ||
                    max_tags < 0 || max_tags > 2 ) {
                printf("Invalid max_tags value: %s\n", optarg);
                exit(1);
            } else {
                has_max_tags = 1;
            }

            break;

        case 'h':
            printf("Usage: %s [-hv] [-p port] [-t max_tags] [-m mtu] [ena|dis|auto|hdx|fdx|10m|100m|1g|2.5g|5g|10g] [adv_dis <hex-mask>]\n"
                   " -v: Verbose\n"
                   " -p: Port number\n"
                   " -t: Max tags, valid values are: 0|1|2\n"
                   " -m: MTU size (bytes)\n"
                   " -h: Display this\n", argv[0]);
        default:
            goto exit;
        }
    }

    memset(&conf, 0, sizeof(conf));
    if(port_conf_get(port, &conf) != VTSS_RC_OK) {
        printf("Error getting port config: %d\n", port + 1);
        goto exit;
    }
    if(optind == argc) {
        vtss_port_status_t st;
        printf("Port %2d settings\n================\n", port + 1);
        printf("Enable   : %s\n", conf.enable ? "on" : "off");
        printf("Auto     : %s\n", conf.autoneg ? "on" : "off");
        if(!conf.autoneg)
        printf("Speed    : %s\n", speeds[conf.speed]);
        printf("adv_dis  : 0x%x\n", conf.adv_dis);
        printf("MTU      : %4d\n", conf.max_length);
        printf("Max tags : %4d\n", conf.max_tags);
        if(vtss_port_status_get(NULL, port, &st) == 0 && st.link) {
            printf("Current: %s %s\n", speeds[st.speed], st.fdx ? "fdx" : "hdx");
        }
    } else {
        if(mtu)
            conf.max_length = mtu;

        if (has_max_tags) {
            conf.max_tags = max_tags;
        }

        for(; optind < argc; optind++) {
            const char * opt = argv[optind];
            if(strcasecmp("dis", opt) == 0) {
                conf.enable = FALSE;
            }
            else if(strcasecmp("ena", opt) == 0) {
                conf.enable = TRUE;
            }
            else if(strcasecmp("auto", opt) == 0) {
                conf.autoneg = TRUE;
            }
            else if(strcasecmp("hdx", opt) == 0) {
                conf.autoneg = FALSE;
                conf.fdx = FALSE;
            }
            else if(strcasecmp("fdx", opt) == 0) {
                conf.autoneg = FALSE;
                conf.fdx = TRUE;
            }
            else if(strcasecmp("10m", opt) == 0) {
                conf.autoneg = FALSE;
                conf.speed = VTSS_SPEED_10M;
            }
            else if(strcasecmp("100m", opt) == 0) {
                conf.autoneg = FALSE;
                conf.speed = VTSS_SPEED_100M;
            }
            else if(strcasecmp("1g", opt) == 0) {
                conf.autoneg = FALSE;
                conf.speed = VTSS_SPEED_1G;
            }
            else if(strcasecmp("2.5g", opt) == 0) {
                conf.autoneg = FALSE;
                conf.speed = VTSS_SPEED_2500M;
            }
            else if(strcasecmp("5g", opt) == 0) {
                conf.autoneg = FALSE;
                conf.speed = VTSS_SPEED_5G;
            }
            else if(strcasecmp("10g", opt) == 0) {
                conf.autoneg = FALSE;
                conf.speed = VTSS_SPEED_10G;
            }
            else if(strcasecmp("adv_dis", opt) == 0) {
                if(++optind < argc) {
                    int adv_dis = 0;
                    if(sscanf(argv[optind], "%i", &adv_dis) == 1) {
                        conf.adv_dis = adv_dis;
                    } else {
                        printf("adv_dis: Invalid value argument (PORT_ADV_DIS_* mask)\n");
                    }
                } else {
                    printf("adv_dis: must be followed by a (hex) value (PORT_ADV_DIS_* mask)\n");
                }
            }
            else
                printf("Illegal keyword: %s\n", opt);
        }

        if(port_conf_set(port, &conf) != VTSS_RC_OK) {
            printf("Error setting port %d config\n", port + 1);
        }
    }

exit:
    return 0;
}
