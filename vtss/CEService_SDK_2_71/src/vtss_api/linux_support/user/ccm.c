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
#include <linux/types.h>

#include "vtss_switch_usermode.h"

int main(int argc, char **argv)
{
    int option, verbose = 0, length = 64, qos = 0, port = 0, cancel = 0, framecount = 0, 
        status = 0, session;
    u32 rate = 100;
    vtss_rc rc;
    static const struct option options[] = {
        { "verbose",     no_argument,       NULL, 'V' },
        { "help",        no_argument,       NULL, 'h' },
        { "length",      required_argument, NULL, 'l' },
        { "port",        required_argument, NULL, 'p' },
        { "qos",         required_argument, NULL, 'q' },
        { "rate",        required_argument, NULL, 'r' },
        { "cancel",      required_argument, NULL, 'c' },
        { "framecount",  no_argument,       NULL, 'f' },
        { "status",      required_argument, NULL, 's' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "Vhl:p:q:r:c:fs:", options, NULL);
        if (option == -1)
            break;
        switch (option) {
        case 'V':
            verbose++;
            break;
        case 'l':
            length = atoi(optarg);
            if(length > 1514)
                length = 1514;
            break;
        case 'q':
            qos = atoi(optarg);
            if(qos < 0 || qos > 7) {
                fprintf(stderr, "Invalid QoS: %d, defaulting to 0\n", qos);
                qos = 0;
            }
            break;
        case 'r':
#if defined(VTSS_FDMA_CCM_FPS_MAX)
            if(sscanf(optarg, "%d", &rate) != 1 ||
               (rate < 1 || rate > VTSS_FDMA_CCM_FPS_MAX)) {
                printf("Invalid rate: %s\n", optarg);
                exit(1);
            }
#endif
            break;
        case 'p':
            if(sscanf(optarg, "%d", &port) != 1 ||
               port <= 0 || port > VTSS_PORTS) {
                printf("Invalid port number: %s\n", optarg);
                exit(1);
            }
            port -= 1;
            break;
        case 'c':
            if(sscanf(optarg, "%d", &session) != 1) {
                printf("Invalid session number: %s\n", optarg);
                exit(1);
            }
            cancel++;
            break;
        case 'f':
            framecount++;
            break;
        case 's':
            if(sscanf(optarg, "%d", &session) != 1) {
                printf("Invalid session number: %s\n", optarg);
                exit(1);
            }
            status++;
            break;
        case 'h':
            printf("Usage: %s [-hV] [-l <length>] [-p <port>] [-q <qos>] [-r <rate>] [-f]\n"
                   "    or %s -s <sess>\n"
                   "    or %s -c <sess>\n"
                   " -p/--port <port>      : Do CCM on this port \n"
                   " -r/--rate <rate>      : CCM rate in fps\n"
                   " -l/--length <length>  : Length of frame\n"
                   " -q/--qos              : Use qos class (0-7)\n"
                   " -f/--framecount       : Enable CCM (tx) frame counting\n"
                   " -V/--verbose          : Verbose operation\n"
                   " -h/--help             : Display this\n"
                   " -s/--status <session> : Show session status\n"
                   " -c/--cancel <session> : Cancel session\n"
                   , argv[0], argv[0], argv[0]);
        default:
            exit(-1);
        }
    }

    if(cancel) {
        if((rc = vtssx_ccm_cancel(NULL, (vtssx_ccm_session_t) session)) != VTSS_RC_OK) {
            fprintf(stderr, "vtssx_ccm_cancel(%d) error: rc %d\n", session, rc);
            exit(1);
        }
    } else if(status) {
        vtssx_ccm_status_t status;
        if((rc = vtssx_ccm_status_get(NULL, (vtssx_ccm_session_t) session, &status)) != VTSS_RC_OK) {
            fprintf(stderr, "vtssx_ccm_status_get(%d) error: rc %d\n", session, rc);
            exit(1);
        }
        printf("Session %d\n", session);
        printf("==========\n");
        printf("Active: %s\n", status.active ? "Yes" : "No");
        printf("Frames: %lld\n", status.ccm_frm_cnt);
    } else {
        vtssx_ccm_opt_t ccm_opt;
        vtssx_ccm_session_t ccm_sess;
        u8 *frame;

        if((frame = malloc(length)) == NULL) {
            fprintf(stderr, "CCM malloc error\n");
            exit(1);
        }

        /* Initialize dummy frame contents (broadcast) */
        memset(frame +  0, 0xff, 6);
        memset(frame +  6, 0x44, 6);
        memset(frame + 12, 0x11, 2);
        memset(frame + 14, 0xaa, length - 14);

        /* CCM options */
        memset(&ccm_opt, 0, sizeof(ccm_opt));
        ccm_opt.port_no = port;
        ccm_opt.qos_class = qos;
        ccm_opt.ccm_fps = rate;
        ccm_opt.ccm_count = framecount;

        if((rc = vtssx_ccm_start(NULL, frame, length, &ccm_opt, &ccm_sess) != VTSS_RC_OK)) {
            fprintf(stderr, "vtssx_ccm_start() error: rc %d\n", rc);
            exit(1);
        }

        fprintf(stdout, "CCM started session '%p', length %d on port %d\n", 
                ccm_sess, length, port + 1);
    }

    return 0;
}
