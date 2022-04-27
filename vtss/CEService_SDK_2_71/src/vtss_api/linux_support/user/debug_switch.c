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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include <getopt.h>

#include "vtss_switch_usermode.h"

typedef struct kw {
    const char *name;
    int         value;
} kw_t;

kw_t n_layers[] = {
    {"all",  VTSS_DEBUG_LAYER_ALL },
    {"app",  VTSS_DEBUG_LAYER_AIL },
    {"chip", VTSS_DEBUG_LAYER_CIL },
    {}
};

kw_t n_groups[] = {
    {"all", VTSS_DEBUG_GROUP_ALL },
    {"init", VTSS_DEBUG_GROUP_INIT },
    {"misc", VTSS_DEBUG_GROUP_MISC },
    {"port", VTSS_DEBUG_GROUP_PORT },
    {"pcount", VTSS_DEBUG_GROUP_PORT_CNT },
    {"phy", VTSS_DEBUG_GROUP_PHY },
    {"vlan", VTSS_DEBUG_GROUP_VLAN },
    {"mac", VTSS_DEBUG_GROUP_MAC_TABLE },
    {"acl", VTSS_DEBUG_GROUP_ACL },
    {"qos", VTSS_DEBUG_GROUP_QOS },
    {"aggr", VTSS_DEBUG_GROUP_AGGR },
    {"stp", VTSS_DEBUG_GROUP_STP },
    {"mirror", VTSS_DEBUG_GROUP_MIRROR },
    {"evc", VTSS_DEBUG_GROUP_EVC },
    {"erps", VTSS_DEBUG_GROUP_ERPS },
    {"eps", VTSS_DEBUG_GROUP_EPS },
    {"packet", VTSS_DEBUG_GROUP_PACKET },
    {"fdma", VTSS_DEBUG_GROUP_FDMA },
    {"time", VTSS_DEBUG_GROUP_TS },
    {"watermarks", VTSS_DEBUG_GROUP_WM },
    {"lrn", VTSS_DEBUG_GROUP_LRN },
    {"ipmc", VTSS_DEBUG_GROUP_IPMC },
    {"stack", VTSS_DEBUG_GROUP_STACK },
    {"cmef", VTSS_DEBUG_GROUP_CMEF },
    {"host", VTSS_DEBUG_GROUP_HOST },
    {"vxlat", VTSS_DEBUG_GROUP_VXLAT },
    {"oam", VTSS_DEBUG_GROUP_OAM },
    {"sgpio", VTSS_DEBUG_GROUP_SER_GPIO },
    {"l3", VTSS_DEBUG_GROUP_L3},
    {}
};

int kw2val(const char *name, kw_t *keywords)
{
    int i;
    for(i = 0; keywords[i].name != NULL; i++) {
        if(strcasecmp(name, keywords[i].name) == 0)
            return keywords[i].value;
    }
    fprintf(stderr, "Illegal value \'%s\' - use one of: ", name);
    for(i = 0; keywords[i].name != NULL; i++)
        fprintf(stderr, "\'%s\'%s", keywords[i].name, keywords[i+1].name ? ", " : "\n");
    return -1;
}

int
main(int argc, char **argv)
{
    int option, verbose = 0, full = FALSE,
        layer = VTSS_DEBUG_LAYER_ALL, portix = -1;
    static const struct option options[] = {
        { "verbose",     0, NULL, 'v' },
        { "help",        0, NULL, 'h' },
        { "full",        0, NULL, 'f' },
        { "layer",       1, NULL, 'l' },
        { "port",        1, NULL, 'p' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "vhfl:p:", options, NULL);
        if (option == -1)
            break;

        switch (option) {
        case 'v':
            verbose++;
            break;
        case 'f':
            full = TRUE;
            break;
        case 'l':
            if((layer = kw2val(optarg, n_layers)) < 0)
                goto exit;
            break;
        case 'p':
            if(sscanf(optarg, "%d", &portix) != 1 ||
               portix <= 0 || portix > VTSS_PORTS) {
                printf("Invalid port number: %s\n", optarg);
                exit(1);
            }
            portix -= 1;
            break;
        case 'h':
            printf("Usage: %s [-hf] [-p <port>] [-l <layer>] [group(s)]\n"
                   " -p <port>: This port only (default all ports)\n"
                   " -l <all|app|chip>: Display layer (default ALL)\n"
                   " -f: Display full debug (default OFF)\n"
                   " -h: Display this\n", argv[0]);
        default:
            goto exit;
        }
    }

    vtss_debug_info_t info;
    memset(&info, 0, sizeof(info));
    info.chip_no = VTSS_CHIP_NO_ALL;
    if(portix >= 0 && portix < VTSS_PORTS)
        info.port_list[portix] = TRUE;
    else
        memset(&info.port_list, TRUE, sizeof(info.port_list));
    info.layer = layer;
    info.full = full;

    if (optind < argc) {
        while (optind < argc) {
            int group;
            if((group = kw2val(argv[optind++], n_groups)) >= 0) {
                info.group = group;
                vtss_debug_info_print(NULL, NULL, &info);
            }
        }
    } else {
        /* Default to all groups */
        info.group = VTSS_DEBUG_GROUP_ALL;
        vtss_debug_info_print(NULL, NULL, &info);
    }

exit:
    return 0;
}
