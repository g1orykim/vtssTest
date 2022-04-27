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
#include <fcntl.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <linux/netlink.h>

#include "vtss_switch_usermode.h"

struct vtssx_inject_handle *handle;
vtssx_inject_opts_t opts;
u8 *frame;

#define FILE_NAME_BUF_LENGTH 256

int main(int argc, char **argv)
{
    int option, verbose = 0, count = 1, length = 64, vlan = 1, switch_frm = 1, qos = 8, sleep = 0;
    int file_input = 0, file_fd = 0;
    char file_name[FILE_NAME_BUF_LENGTH];

    u64 portset = 0;
    static const struct option options[] = {
        { "verbose",     no_argument,       NULL, 'V' },
        { "help",        no_argument,       NULL, 'h' },
        { "count",       required_argument, NULL, 'c' },
        { "length",      required_argument, NULL, 'l' },
        { "port",        required_argument, NULL, 'p' },
        { "vlan",        required_argument, NULL, 'v' },
        { "qos",         required_argument, NULL, 'q' },
        { "sleep",       required_argument, NULL, 's' },
        { "file",        required_argument, NULL, 'f' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "Vhc:l:v:p:q:s:f:", options, NULL);
        if (option == -1)
            break;
        switch (option) {
        case 'V':
            verbose++;
            break;
        case 'c':
            count = atoi(optarg);
            break;
        case 'l':
            length = atoi(optarg);
            if(length > 1514) {
                fprintf(stderr, "Invalid length: %d, defaulting to 1514\n", length);
                length = 1514;
            }
            break;
        case 'v':
            vlan = atoi(optarg);
            break;
        case 'q':
            qos = atoi(optarg);
            if(qos < 0 || qos > 8) {
                fprintf(stderr, "Invalid QoS: %d, defaulting to 8\n", qos);
                qos = 8;
            }
            break;
        case 's':
            sleep = atoi(optarg);
            if(sleep < 0) {
                fprintf(stderr, "Invalid Sleep perios: %d, defaulting to 500 (us)\n", sleep);
                sleep = 500;
            }
            break;
        case 'p':
            portset |= (1LL << (atoi(optarg) - 1));
            switch_frm = 0;
            break;
        case 'f':
            strncpy(file_name, optarg, FILE_NAME_BUF_LENGTH);
            file_input = 1;
            break;
        case 'h':
            printf("Usage: %s [-hV] [-c <count>] [-l <length>] [-s <usecs>] [-v <vlan>] [-q <qos>]\n"
                   " -p/--port   <port>  : Inject direct to port (disables switching)\n"
                   " -c/--count  <count> : Transmit this many frames\n"
                   " -l/--length <length>: Length of frame\n"
                   " -s/--sleep  <usecs> : Number of usecs to pause between each frame injected\n"
                   " -v/--vlan           : Inject into this VLAN\n"
                   " -q/--qos            : Use qos class (0-8)\n"
                   " -V/--verbose        : Verbose operation\n"
                   " -h/--help           : Display this\n"
                   " -f/--file           : inject file content\n"
                   , argv[0]);
        default:
            exit(-1);
        }
    }

    if (file_input) {
        file_fd = open(file_name, O_RDONLY);

        if (file_fd < 0) {
            fprintf(stderr, "Failed to open file: %s, err %m\n", file_name);
            return -1;
        }

        struct stat st;
        if (fstat(file_fd, &st) < 0) {
            fprintf(stderr, "fstat failed: %m\n");
            return -1;
        }

        length = (int)st.st_size;

        if(verbose)
            fprintf(stderr, "File inject: %s length %d\n", file_name, length);
    }


    if(VTSS_RC_OK != vtssx_inject_init(&handle) ||
       (frame = malloc(length)) == NULL) {
        fprintf(stderr, "Inject init error\n");
        vtssx_inject_deinit(&handle);
        exit(1);
    }

    if(verbose) fprintf(stderr, "Inject starting\n");

    opts.switch_frm = switch_frm;
    opts.vlan = vlan;
    opts.qos_class = qos;
    opts.port_mask = portset;

    if (file_input) {
        if (read(file_fd, frame, length) != length) {
            fprintf(stderr, "Read error %m\n");
        }

    } else {
        memset(frame +  0, 0xff, 6);
        memset(frame +  6, 0x44, 6);
        memset(frame + 12, 0x11, 2);
        memset(frame + 14, 0xaa, length - 14);
    }

    int i;
    for(i = 0; i < count; i++) {
        if(verbose) fprintf(stderr, "Inject(%d/%d): vlan %d len %d switch %d ports 0x%llx\n", 
                            1+i, count,
                            opts.vlan, length, opts.switch_frm, opts.port_mask);
        if(vtssx_inject_frame(handle, frame, length, &opts)) {
            fprintf(stderr, "Frame inject failed\n");
            break;
        }
        if(sleep)
            usleep(sleep);
    }

    vtssx_inject_deinit(&handle);

    return 0;
}
