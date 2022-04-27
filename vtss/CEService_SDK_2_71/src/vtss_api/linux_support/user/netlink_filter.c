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
#include <string.h>

#include <getopt.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <arpa/inet.h>

#include "vtss_switch_usermode.h"

int sock_fd;

int main(int argc, char **argv)
{
    int option, verbose = 0, clear = 0, acl = 0, flen = 2048;
    vtss_ace_id_t acl_index;
    struct ifreq ifr;
    static const struct option options[] = {
        { "verbose",     no_argument,       NULL, 'V' },
        { "help",        no_argument,       NULL, 'h' },
        { "clear",       no_argument,       NULL, 'c' },
        { "acl",         1,                 NULL, 'a' },
        { "maxlen",      1,                 NULL, 'l' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "Vhca:l:", options, NULL);
        if (option == -1)
            break;
        switch (option) {
        case 'V':
            verbose++;
            break;
        case 'h':
            printf("Usage: %s [-hV] [-c] [-a <index>] [-l <len>] filters...\n"
                   " -V/--verbose        : Verbose operation\n"
                   " -h/--help           : Display this\n"
                   " -c/--clear          : Clear packet filter\n"
                   " -a/--acl            : Use ACL index (and ACL filter mode)\n"
                   " -l/--maxlen         : Set (packet filter) max frame length\n"
                   , argv[0]);
            exit(0);
            break;
        case 'c':
            clear++;
            break;
        case 'a':
            acl++;
            if(sscanf(optarg, "%u", &acl_index) != 1) {
                printf("Invalid acl index: %s\n", optarg);
                exit(1);
            }
            break;
        case 'l':
            if(sscanf(optarg, "%u", &flen) != 1 || flen <= 0) {
                printf("Invalid max frame length: %s\n", optarg);
                exit(1);
            }
            break;
        default:
            exit(-1);
        }
    }

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    
    strcpy(ifr.ifr_name, "eth0");

    if(clear) {
        if(acl) {
            vtss_rc rc;
            if((rc = vtss_ace_del(NULL, acl_index)) != VTSS_RC_OK) {
                printf("Error deleting ACE %u: %d\n", acl_index, rc);
            }
        } else {
            if (ioctl(sock_fd, SIOCETHDRVCLRFILT, (caddr_t)&ifr) < 0) {
                perror("SIOCETHDRVCLRFILT: error");
            }
        }
    } else {
        if(optind == argc) {
            printf("No filters?\n");
        }
    }

    for(; optind < argc; optind++) {
        const char * opt = argv[optind];
        if(strcasecmp("stp", opt) == 0) {
            if(acl) {
                vtss_ace_t ace;
                vtss_rc rc;
                unsigned char stp_mac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
                vtss_ace_init(NULL, VTSS_ACE_TYPE_LLC, &ace);
                ace.id = acl_index;
                ace.action.cpu = TRUE;
#if defined(VTSS_FEATURE_ACL_V1)
                ace.port_no = VTSS_PORT_NO_ANY;
#endif /* VTSS_FEATURE_ACL_V1 */
#if defined(VTSS_FEATURE_ACL_V2)
                memset(ace.port_list, TRUE, sizeof(ace.port_list));
#endif /* VTSS_FEATURE_ACL_V2 */
                memcpy(ace.frame.llc.dmac.value, stp_mac, sizeof(ace.frame.llc.dmac.value));
                memset(ace.frame.llc.dmac.mask, 0xFF, sizeof(ace.frame.llc.dmac.mask));
#if 0                                              /* Serval issue */
                ace.frame.llc.llc.value[0] = 0x42; /* STP */
                ace.frame.llc.llc.value[1] = 0x42; /* STP */
                ace.frame.llc.llc.value[2] = 0x03; /* UI */
                ace.frame.llc.llc.mask[0] = 0xFF;
                ace.frame.llc.llc.mask[1] = 0xFF;
                ace.frame.llc.llc.mask[2] = 0xFF;
#endif
                if((rc = vtss_ace_add(NULL, VTSS_ACE_ID_LAST, &ace)) != VTSS_RC_OK) {
                    printf("Error adding ACE %u: %d\n", acl_index, rc);
                } else {
                    printf("Added ACE %u for STP capture\n", acl_index);
                }
            } else {
                struct SIOCETHDRV_filter flt;
                struct sock_filter filterops[] = {
                    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, 0),                 // A <- P[0:3]
                    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x0180C200, 0, 3), // if A != 0x0180c200 GOTO REJECT
                    BPF_STMT(BPF_LD + BPF_H + BPF_ABS, 4),                 // A <- P[4:5]
                    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,     0x0000, 0, 1), // if A != 0x0000 GOTO REJECT
                    // ACCEPT
                    BPF_STMT(BPF_RET, flen),                               // accept packet
                    // REJECT
                    BPF_STMT(BPF_RET, 0),                                  // reject packet
                };
                ifr.ifr_data = (caddr_t) &flt;
                flt.length = sizeof(filterops)/sizeof(filterops[0]);
                flt.netlink_filter = filterops;
                if (ioctl(sock_fd, SIOCETHDRVSETFILT, (caddr_t)&ifr) < 0) {
                    perror("SIOCETHDRVSETFILT: error");
                } else {
                    printf("Added packet filter for STP capture, max frame length %d\n", flen);
                }
            }
        } else {
            printf("Illegal filter: %s - supported: 'stp'\n", opt);
        }
    }

    close(sock_fd);

    return 0;
}
