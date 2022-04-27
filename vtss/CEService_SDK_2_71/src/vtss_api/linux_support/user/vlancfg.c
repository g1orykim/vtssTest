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

static inline BOOL invalid_port(int port)
{
    return port < 1 || port > VTSS_PORTS;
}

static int
parse_range(BOOL *members, const char *range)
{
    char buf[256], *token, *endptr;
    const char *delims = ",";

    if(strcmp(range, "all") == 0) {
        memset(members, 1, VTSS_PORTS);
        return 0;
    }

    strncpy(buf, range, strlen(range));
    memset(members, 0, VTSS_PORTS);
    if(strcmp(range, "none") == 0)
        return 0;

    token = strtok(buf, delims);
    do {
        int i, start, end;
        if((2 == sscanf(token, "%d-%d", &start, &end))) {
            if(start == end ||
               start > end ||
               invalid_port(start) ||
               invalid_port(end))
                goto error;
            for(i = start; i <= end; i++)
                members[i-1] = TRUE;
        } else if((i = strtol(token, &endptr, 10)) &&
                  *endptr == '\0' &&
                  !invalid_port(i)) {
            members[i-1] = TRUE;
        } else {
            goto error;
        }
    } while((token = strtok(NULL, delims)));
    
    return 0;
    
error:
    printf("Invalid token \"%s\" in range: %s\n", token, range);
    return 1;
}

static void print_port_list(BOOL *members)
{
    vtss_port_no_t port_no;
    u32            count = 0, max = VTSS_PORTS-1;
    BOOL           member, first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no <= max; port_no++) {
        member = (members[port_no] != 0);
        if ((member && (count == 0 || port_no == max)) || (!member && count > 1)) {
            printf("%s%u",
                   first ? "" : count > (member ? 1 : 2) ? "-" : ",",
                   member ? port_no + 1 : (port_no));
            first = 0;
        }
        if (member)
            count++;
        else
            count=0;
    }
    printf("%s\n", first ? "None" : "");
}


int
main(int argc, char **argv)
{
    vtss_port_no_t port_no;
    vtss_vlan_port_conf_t vlan_cfg;
    BOOL member[VTSS_PORT_ARRAY_SIZE];
    int option;
    int verbose = 0, list = 0, filter = 0, vlan = 1, aware = VTSS_VLAN_PORT_TYPE_UNAWARE;
    static const struct option options[] = {
        { "list",        0, NULL, 'l' },
        { "verbose",     0, NULL, 'v' },
        { "help",        0, NULL, 'h' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "lvh", options, NULL);
        if (option == -1)
            break;
        switch (option) {
        case 'l':
            list++;
            break;
        case 'v':
            verbose++;
            break;
        case 'h':
            printf("Usage: %s members vlan [<portrange>]\n"
                   "       %s [<portrange>] [cport] [sport] [unaware] [filter] [nofilter]\n"
                   "       %s -l [<portrange>]\n"
                   " -l: List\n"
                   " -v: Verbose\n"
                   " -h: Display this\n"
                   " <portrange>: Ex: '1-4,6,8-9' or 'none' or 'all'\n", 
                   argv[0], 
                   argv[0], 
                   argv[0]);
        default:
            goto exit;
        }
    }

    if(optind < argc &&
       strcmp("members", argv[optind]) == 0) {
        optind++;               /* skip "members" */
        if(optind < argc &&
           (vlan = atoi(argv[optind++])) &&
           vlan < 4096) {
            if(optind < argc) {
                if((argc - optind) > 1) {
                    printf("Extra arguments after range: %s\n", argv[optind+1]);
                    goto exit;
                }
                if(parse_range(member, argv[optind++]))
                        goto exit;
                if(vtss_vlan_port_members_set(NULL, vlan, member)) {
                    printf("Error setting vlan(%d) members\n", vlan);
                }
            } else {
                if(vtss_vlan_port_members_get(NULL, vlan, member)) {
                    printf("Error getting vlan(%d) members\n", vlan);
                } else {
                    printf("Members of VLAN %d: ", vlan);
                    print_port_list(member);
                }
            }
        } else {
            printf("Bad or missing vlan\n");
            goto exit;
        }
    } else {
        if(optind < argc)
            if(parse_range(member, argv[optind++]))
                goto exit;
        if(list) {
            for(port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                if(member[port_no] &&
                   vtss_vlan_port_conf_get(NULL, port_no, &vlan_cfg) == VTSS_RC_OK) {
                    printf("Port %2d pvid = %d, port_type %d, untagged_vid %d, ingress_filter %s\n", 
                           port_no + 1, vlan_cfg.pvid, vlan_cfg.port_type, vlan_cfg.untagged_vid,
                           vlan_cfg.ingress_filter ? "on" : "off");
                }
            }
        } else {
            while(optind < argc) {
                const char *kw = argv[optind++];
                if(strcmp("cport", kw) == 0) 
                    aware = VTSS_VLAN_PORT_TYPE_C;
                else if(strcmp("sport", kw) == 0)
                    aware = VTSS_VLAN_PORT_TYPE_S;
                else if(strcmp("unaware", kw) == 0)
                    aware = VTSS_VLAN_PORT_TYPE_UNAWARE;
                else if(strcmp("filter", kw) == 0)
                    filter = TRUE;
                else if(strcmp("nofilter", kw) == 0)
                    filter = FALSE;
                else {
                    printf("Illegal keyword: %s\n", kw);
                }
                for(port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
                    if(member[port_no] &&
                       vtss_vlan_port_conf_get(NULL, port_no, &vlan_cfg) == VTSS_RC_OK) {
                        vlan_cfg.port_type = aware;
                        vlan_cfg.ingress_filter = filter;
                        //vlan_cfg.untagged_vid = vlan_cfg.pvid = vlan;
                        if(vtss_vlan_port_conf_set(NULL, port_no, &vlan_cfg))
                            printf("Port %2d: Error setting vlan config\n", port_no + 1);
                    }
                }
            }
        }
    }

exit:
    return 0;
}
