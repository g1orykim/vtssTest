/*

 Vitesse API software.

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

 $Id$
 $Revision$

*/

#if defined(BOARD_DAYTONA_EVAL) || defined(BOARD_DAYTONA) || defined(BOARD_TALLADEGA)

/* #define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_CIL */
#ifdef DAYTONA_CHIP_SIMULATE
#include "../base/daytona/vtss_daytona_regs.h"
#endif
#include <string.h>
#include <sys/types.h>
#undef __USE_EXTERN_INLINES /* Avoid warning */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/vitgenio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "vtss_api.h"
#include "vtss_appl.h"

#define VTSS_RC(expr) { vtss_rc __rc__ = (expr); if (__rc__ < VTSS_RC_OK) return __rc__; }

#if defined(VTSS_FEATURE_PORT_CONTROL)
/* should be defined otherwise */
#define VTSS_LPORT_MAP_DEFAULT 0xffffffff

/* Board port map */
static vtss_port_map_t board_port_map[VTSS_PORT_ARRAY_SIZE] = {
    {  0, 0, VTSS_MIIM_CONTROLLER_0,  10 },     /* chip port 0 = Client side channel 0 */
    {  1, 0, VTSS_MIIM_CONTROLLER_0,  21 },     /* chip port 1 = Client side channel 1 */
    {  2, 0, VTSS_MIIM_CONTROLLER_NONE,  0 },  /* chip port 2 = Line side channel 0 */
    {  3, 0, VTSS_MIIM_CONTROLLER_NONE,  0 },  /* chip port 3 = Line side channel 1 */
};
#endif /* BOARD_DAYTONA_EVAL || BOARD_DAYTONA */

#if defined(BOARD_DAYTONA_EVAL)
#define DAYTONA_CHIP_SIMULATE
#else 
static int pi_fd; /* Parllel interface descriptor */
#endif /* BOARD_DAYTONA_EVAL */

static int  interrupt_fd;

/* Various options */
static struct {
    BOOL if_100fx; /* 100FX used for SFP ports */
    BOOL if_mix;   /* SERDES/100FX used for SFP ports */
} daytona_options;

static const char *config_txt[] = {
    [VTSS_CONFIG_MODE_PSS]                  =  "PSS",
    [VTSS_CONFIG_MODE_PSS_W]                =  "PSS_W",
    [VTSS_CONFIG_MODE_PSOC_A]               =  "PSOC_A",
    [VTSS_CONFIG_MODE_PSOC_WA]              =  "PSOC_WA",
    [VTSS_CONFIG_MODE_PSOC_S]               =  "PSOC_S",
    [VTSS_CONFIG_MODE_PSOC_WS]              =  "PSOC_WS",
    [VTSS_CONFIG_MODE_PEO_WMWS]             =  "PEO_WMWS",
    [VTSS_CONFIG_MODE_PEO_WMWA]             =  "PEO_WMWA",
    [VTSS_CONFIG_MODE_PEE_W]                =  "PEE_W",
    [VTSS_CONFIG_MODE_PEE_MW]               =  "PEE_MW",
    [VTSS_CONFIG_MODE_XAUI_2M_PEE_MW]       =  "XAUI_2M_PEE_MW",
    [VTSS_CONFIG_MODE_PEE_P]                =  "PEE_P",
    [VTSS_CONFIG_MODE_PEE_MP]               =  "PEE_MP",
    [VTSS_CONFIG_MODE_XAUI_2M_PEE_MP]       =  "XAUI_2M_PEE_MP",
    [VTSS_CONFIG_MODE_PFF_P]                =  "PFF_P",
    [VTSS_CONFIG_MODE_PEO_WA]               =  "PEO_WA",
    [VTSS_CONFIG_MODE_PEO_MWA]              =  "PEO_MWA",
    [VTSS_CONFIG_MODE_PEO_WS]               =  "PEO_WS",
    [VTSS_CONFIG_MODE_PEO_MWS]              =  "PEO_MWS",
    [VTSS_CONFIG_MODE_PEO_P2E_20]           =  "PEO_P2E_20",
    [VTSS_CONFIG_MODE_PEO_MP2E_20]          =  "PEO_MP2E_20",
    [VTSS_CONFIG_MODE_PEO_P2E_100]          =  "PEO_P2E_100",
    [VTSS_CONFIG_MODE_PEO_P1E_100]          =  "PEO_P1E_100",
    [VTSS_CONFIG_MODE_XAUI_2M_PEO_P1E_100]  =  "XAUI_2M_PEO_P1E_100",
    [VTSS_CONFIG_MODE_PFO_P2F]              =  "PFO_P2F",
    [VTSS_CONFIG_MODE_PGO_2]                =  "PGO_2",
    [VTSS_CONFIG_MODE_PGO_2SUB]             =  "PGO_2SUB",
    [VTSS_CONFIG_MODE_PGO_2SUBSMALL]        =  "PGO_2SUBSMALL",
    [VTSS_CONFIG_MODE_PGO_2GEN]             =  "PGO_2GEN",
    [VTSS_CONFIG_MODE_BOO_2]                =  "BOO_2",
    [VTSS_CONFIG_MODE_BOO_1E]               =  "BOO_1E",
    [VTSS_CONFIG_MODE_BOO_2E]               =  "BOO_2E",
    [VTSS_CONFIG_MODE_BOO_2F]               =  "BOO_2F",
    [VTSS_CONFIG_MODE_BMX_2]                =  "BMX_2",
    [VTSS_CONFIG_MODE_BMX_2E]               =  "BMX_2E",
    [VTSS_CONFIG_MODE_BMX_1E]               =  "BMX_1E",
    [VTSS_CONFIG_MODE_BMX_2F]               =  "BMX_2F",
    [VTSS_CONFIG_MODE_BOE_WMWA]             =  "BOE_WMWA",
    [VTSS_CONFIG_MODE_BOE_WMWS]             =  "BOE_WMWS",
    [VTSS_CONFIG_MODE_BOE_PMP2E_20]         =  "BOE_PMP2E_20",
    [VTSS_CONFIG_MODE_BOE_PMP1E_20]         =  "BOE_PMP1E_20",
    [VTSS_CONFIG_MODE_BOEC_2E]              =  "BOEC_2E",
    [VTSS_CONFIG_MODE_BOEC_1E]              =  "BOEC_1E",
    [VTSS_CONFIG_MODE_BOG_2]                =  "BOG_2",
    [VTSS_CONFIG_MODE_BOG_2SUB]             =  "BOG_2SUB",
    [VTSS_CONFIG_MODE_BOG_W2]               =  "BOG_W2",
    [VTSS_CONFIG_MODE_BOSC_A]               =  "BOSC_A",
    [VTSS_CONFIG_MODE_BOSC_WA]              =  "BOSC_WA",
    [VTSS_CONFIG_MODE_BOSC_S]               =  "BOSC_S",
    [VTSS_CONFIG_MODE_BOSC_WS]              =  "BOSC_WS",
    [VTSS_CONFIG_MODE_BOFC_2F]              =  "BOFC_2F",
    [VTSS_CONFIG_MODE_BO8F]                 =  "BO8F",
    [VTSS_CONFIG_MODE_TSOC_A]               =  "TSOC_A",
    [VTSS_CONFIG_MODE_TSOC_WA]              =  "TSOC_WA",
    [VTSS_CONFIG_MODE_TSOC_S]               =  "TSOC_S",
    [VTSS_CONFIG_MODE_TSOC_WS]              =  "TSOC_WS",
    [VTSS_CONFIG_MODE_TEO_PMP_2E]           =  "TEO_PMP_2E",
    [VTSS_CONFIG_MODE_TEO_PMP_1E]           =  "TEO_PMP_1E",
    [VTSS_CONFIG_MODE_TEO_PP_2E]            =  "TEO_PP_2E",
    [VTSS_CONFIG_MODE_TFO_PP2F]             =  "TFO_PP2F",
    [VTSS_CONFIG_MODE_TEOC_2E]              =  "TEOC_2E",
    [VTSS_CONFIG_MODE_TEOC_1E]              =  "TEOC_1E",
    [VTSS_CONFIG_MODE_TFOC_2F]              =  "TFOC_2F",
    [VTSS_CONFIG_MODE_TEO_WMWA]             =  "TEO_WMWA",
    [VTSS_CONFIG_MODE_TEO_WMWS]             =  "TEO_WMWS",
    [VTSS_CONFIG_MODE_TGO_2]                =  "TGO_2",
    [VTSS_CONFIG_MODE_TGO_2SUB]             =  "TGO_2SUB",
    [VTSS_CONFIG_MODE_TGO_2SUBSMALL]        =  "TGO_2SUBSMALL",
    [VTSS_CONFIG_MODE_TGO_W2]               =  "TGO_W2",
    [VTSS_CONFIG_MODE_T8FO]                 =  "T8FO",
    [VTSS_CONFIG_MODE_TOO_2]                =  "TOO_2",
    [VTSS_CONFIG_MODE_TOO_2E2]              =  "TOO_2E2",
    [VTSS_CONFIG_MODE_TOO_1E2]              =  "TOO_1E2",
    [VTSS_CONFIG_MODE_TOO_2F2]              =  "TOO_2F2",
    [VTSS_CONFIG_MODE_TEE_L]                =  "TEE_L",
    [VTSS_CONFIG_MODE_TEE_PMP_S]            =  "TEE_PMP_S",
    [VTSS_CONFIG_MODE_TEE_PMP_A]            =  "TEE_PMP_A",
    [VTSS_CONFIG_MODE_TEE_PMW]              =  "TEE_PMW",
    [VTSS_CONFIG_MODE_TEE_PP]               =  "TEE_PP",
    [VTSS_CONFIG_MODE_TEE_W]                =  "TEE_W",
    [VTSS_CONFIG_MODE_TFF]                  =  "TFF",
    [VTSS_CONFIG_MODE_TSS]                  =  "TSS",
    [VTSS_CONFIG_MODE_DISABLED]             =  "DISABLED"
};

#ifdef DAYTONA_EVAL_INIT
static u16 si5338_val[PROTO_MAX][SI5338_MAX_FIELDS] = {
   [PROTO_SONET]       = {0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00},
   [PROTO_10GEWAN]     = {0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00},
   [PROTO_OTU2]        = {0x00, 0x6f, 0x05, 0x00, 0x24, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00},
   [PROTO_OTU2E]       = {0x00, 0x2d, 0x05, 0x00, 0x1d, 0xc3, 0x75, 0x00, 0xdf, 0xeb, 0xde, 0x00, 0x06, 0x00, 0x00},
   [PROTO_OTU1E]       = {0x00, 0x34, 0x05, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x70, 0x43, 0x00, 0x00, 0x06, 0x00, 0x00},
   [PROTO_XAUI_10GE]   = {0x00, 0xf6, 0x05, 0x00, 0x1a, 0x69, 0x00, 0x00, 0x09, 0x3d, 0x00, 0x00, 0x06, 0x00, 0x00},
   [PROTO_250]         = {0x00, 0xfa, 0x02, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x35, 0x0c, 0x00, 0x00, 0x06, 0x00, 0x00},
   [PROTO_OTU2_SFI42]  = {0x00, 0x35, 0x05, 0x00, 0x07, 0x21, 0x00, 0x00, 0xf5, 0x0a, 0x00, 0x00, 0x06, 0x00, 0x00},
   [PROTO_OTU2E_SFI42] = {0x00, 0xf5, 0x04, 0x00, 0x00, 0x45, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x06, 0x00, 0x00},
   [PROTO_10GELAN]     = {0x00, 0xb8, 0x05, 0x00, 0x18, 0xef, 0x06, 0x00, 0x63, 0x9f, 0x02, 0x00, 0x06, 0x00, 0x00},
   [PROTO_10GFC]       = {0x00, 0x91, 0x05, 0x00, 0x05, 0x71, 0x06, 0x00, 0x8b, 0xc8, 0x01, 0x00, 0x06, 0x00, 0x00},
   [PROTO_OTU2F]       = {0x00, 0x09, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0xc5, 0xb2, 0x02, 0x00, 0x06, 0x00, 0x00},
};

static u8 si5326_val[BS_MAX][SI5326_MAX_FIELDS] = {
   [BS_64_66]       = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, \
                       0x05, 0x01, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x57, 0x00, 0x00, 0x57, 0x03, 0x03},
   [BS_66_64]       = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, \
                       0x03, 0x07, 0x00, 0x00, 0xFB, 0x00, 0x00, 0x53, 0x00, 0x00, 0x53, 0x03, 0x03},
   [BS_237_255]     = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x02, 0x00, 0x00, 0x05, 0x00, 0x00, \
                       0x05, 0x05, 0x00, 0x01, 0x3B, 0x00, 0x00, 0x54, 0x00, 0x00, 0x54, 0x00, 0x00}, 
   [BS_237_255_X4]  = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, \
                       0x01, 0x00, 0x00, 0x02, 0x77, 0x00, 0x00, 0x54, 0x00, 0x00, 0x54, 0x03, 0x03},
   [BS_255_237]     = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, \
                       0x03, 0x04, 0x00, 0x01, 0x53, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x4E, 0x03, 0x03},  
   [BS_255_237_X4]  = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, \
                       0x01, 0x01, 0x00, 0x02, 0x1F, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x4E, 0x03, 0x03},
   [BS_238_255]     = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x02, 0x00, 0x00, 0x05, 0x00, 0x00, \
                       0x05, 0x03, 0x00, 0x01, 0x97, 0x00, 0x00, 0x54, 0x00, 0x00, 0x54, 0x03, 0x03}, 
   [BS_255_238]     = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, \
                       0x05, 0x02, 0x00, 0x01, 0xC1, 0x00, 0x00, 0x53, 0x00, 0x00, 0x53, 0x03, 0x03},  
   [BS_15168_16830] = {0x00, 0x01, 0x00, 0x07, 0x02, 0x01, 0x03, 0x07, 0x05, 0x00, 0x00, 0x03, 0x00, 0x00, \
                       0x03, 0x00, 0x00, 0x1D, 0x9F, 0x00, 0x03, 0xA6, 0x00, 0x03, 0xA6, 0x03, 0x03},
   [BS_15232_16830] = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x03, 0x07, 0x05, 0x00, 0x00, 0x03, 0x00, 0x00, \
                       0x03, 0x03, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x6D, 0x00, 0x00, 0x6D, 0x03, 0x03},
   [BS_16830_15168] = {0x00, 0x01, 0x00, 0x09, 0x02, 0x01, 0x03, 0x07, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, \
                       0x07, 0x07, 0x00, 0x01, 0xFD, 0x00, 0x00, 0x9D, 0x00, 0x00, 0x9D, 0x03, 0x03},
   [BS_16830_15232] = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x03, 0x07, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, \
                       0x07, 0x01, 0x00, 0x02, 0x51, 0x00, 0x00, 0x53, 0x00, 0x00, 0x53, 0x03, 0x03}, 
   [BS_X4]          = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, \
                       0x00, 0x02, 0x00, 0x01, 0xD9, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x4E, 0x03, 0x03},
   [BS_X4_OTU2E]    = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x03, 0x07, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, \
                       0x00, 0x02, 0x00, 0x01, 0xD9, 0x00, 0x00, 0x4E, 0x00, 0x00, 0x4E, 0x03, 0x03},
   [BS_CLEANUP]     = {0x00, 0x01, 0x00, 0x05, 0x02, 0x01, 0x02, 0x07, 0x05, 0x00, 0x00, 0x03, 0x00, 0x00, \
                       0x03, 0x05, 0x00, 0x01, 0x3b, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x4e, 0x03, 0x03}, 
   [BS_BYP_X4_SONET]= {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, \
                       0x00, 0x05, 0x00, 0x01, 0x37, 0x00, 0x01, 0x37, 0x00, 0x00, 0x4D, 0x03, 0x05},
   [BS_BYP_X4_OTU2] = {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, \
                       0x01, 0x02, 0x00, 0x01, 0xBF, 0x00, 0x01, 0x4F, 0x00, 0x00, 0x53, 0x03, 0x05},
   [BS_BYP_X4_OTU2E]= {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x03, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, \
                       0x01, 0x00, 0x00, 0x02, 0xB7, 0x00, 0x01, 0x5B, 0x00, 0x00, 0x56, 0x03, 0x05},
   [BS_BYP_X4_TSOC1]= {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, \
                       0x01, 0x02, 0x00, 0x01, 0xD9, 0x00, 0x01, 0x53, 0x00, 0x00, 0x54, 0x03, 0x05}, 
   [BS_BYP_X4_TSOC2]= {0x00, 0x01, 0x00, 0x0A, 0x02, 0x01, 0x02, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, \
                       0x01, 0x01, 0x00, 0x02, 0x1F, 0x00, 0x01, 0x3B, 0x00, 0x00, 0x4E, 0x03, 0x05},
   [BS_BYP_REF]     = {0x01, 0x00, 0x01, 0x05, 0x02, 0x00, 0x01, 0x04, 0x05, 0x00, 0x00, 0x03, 0x00, 0x00, \
                       0x03, 0x05, 0x00, 0x01, 0x3b, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x4e, 0x03, 0x03},
};

static int fd; /* Socket descriptor */

static vtss_rc db_setup_switch(int, int);

/* Execute Rabbit command */
static vtss_rc vtss_rabbit_cmd(const char *cmd, u32 *value)
{
    char buf[100];
    int  n;
    char *ptr;
    /* Send command terminated by newline */
    strcpy(buf, cmd);
    n = strlen(buf);
    buf[n] = '\n';
    if (send(fd, buf, n + 1, 0) < 0) {
        T_E("send cmd failed: %s", cmd);
        return VTSS_RC_ERROR;
    }

    T_N("send: %s", cmd);
    /* Receive command reply */
    if ((n = recv(fd, buf, sizeof(buf), 0)) < 0) {
        T_E("recv cmd failed: %s", cmd);
        return VTSS_RC_ERROR;
    }
    buf[n - 1] = '\0';
    T_N("recv: %s", buf);
    if ((value != NULL) && (strstr(buf, "[") == buf)) {
        if (strncmp("[0x", buf, 3) != 0) {
            char buf1[100] = "[0x";
            strcat(buf1,(buf+1));
            strcpy(buf,buf1);
        }
        *value = strtoul(buf + 1, &ptr, 0);
        if ((ptr != NULL) && ( *ptr == ':')) {
            char hstr[100] = "0x";
            int temp = 0;
            strcat(hstr,ptr+1);
            ptr = NULL;
            temp = strtoul(hstr, &ptr, 0);
            if ((ptr != NULL) && (*ptr == ':')) {
                *value = (*value << 8) | temp ;
            }
        }
        T_N("value: 0x%08x", *value);
    }
    return VTSS_RC_OK;
}

/* VSC3172 */
static vtss_rc db_hw_reset(void)
{
    u32 value;
    char cmd[MAX_CPU_CMD_LEN];
    sprintf(cmd, "%s", "reset_val_board  1");
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    sprintf(cmd, "%s", "reset_val_board  0");
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    return VTSS_RC_OK;
}

static vtss_rc disconnect_all_i2c_secondary_busses(void)
{
    u32 value;
    u16 count;
    char cmd[MAX_CPU_CMD_LEN];

    /* Write 0 to all primary bus addresses */
    for (count = 0; count < 6; count++) {
        sprintf(cmd, "p_i2c_wr 0x%x  0x00", (0x71 + count));
        VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    }
    return VTSS_RC_OK;
}

static vtss_rc connect_i2c_secondary_bus(u32 i2c_bus_number)
{
    u32 value;
    char cmd[MAX_CPU_CMD_LEN];
    u16 switch_address;
    u16 switch_value;
    u16 base_address = 0x71;

    switch_address = base_address + (i2c_bus_number / 8);
    switch_value   =  1 << (i2c_bus_number % 8) ;

    sprintf(cmd, "p_i2c_wr 0x%2.2x 0x%2.2x", switch_address, switch_value);
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    return VTSS_RC_OK;
}

vtss_rc lbfpga_read(u32 reg_addr, u32 *data)
{
    char cmd[MAX_CPU_CMD_LEN];
    u32  lb_address;

    lb_address =  BASE_ADDR_LBFPGA + reg_addr;

    sprintf(cmd, "WPRP 0x2018 0x%8.8x03", lb_address);
    VTSS_RC(vtss_rabbit_cmd(cmd, data));
    return VTSS_RC_OK;
}

vtss_rc lbfpga_write(u32 reg_addr, u32 lb_data, u32 mask)
{
    char cmd[MAX_CPU_CMD_LEN];
    u32  lb_address;
    u32  value;
    u32  data;

    if(mask != 0xffffffff){
        VTSS_RC(lbfpga_read(reg_addr, &data));
        lb_data = (data & ~mask) | lb_data;
    }

    lb_address =  BASE_ADDR_LBFPGA + reg_addr;

    sprintf(cmd, "WP 0x2000 0x%8.8x%8.8x02", lb_address, lb_data);

    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    return VTSS_RC_OK;
}

vtss_rc pcf8575_read(u32 target_id, u16 *data)
{
    char cmd[MAX_CPU_CMD_LEN];
    u16 i2c_addr;
    u32 value = 0;

    if(target_id == 0){
        i2c_addr = 0x23;
    } else if (target_id == 1){
        i2c_addr = 0x21;
    } else if (target_id == 2) {
        i2c_addr = 0x22;
    } else {
        return VTSS_RC_ERROR;
    }

    VTSS_RC(disconnect_all_i2c_secondary_busses());

    sprintf(cmd, "p_i2c_rd 0x%x 2", i2c_addr);

    VTSS_RC(vtss_rabbit_cmd(cmd, &value));

    *data = (u16)value;
    *data = (*data << 8) | (*data >> 8);
    return VTSS_RC_OK;
}

vtss_rc pcf8575_write(u32 target_id, u16 lb_data, u16 mask)
{
    char cmd[MAX_CPU_CMD_LEN];
    u32 value;
    u16 i2c_addr, data;
    /*
      Determine the i2c address for this pcf8575
      Notice tgt_id 0 maps to 0x23, to avoid VSC3406 address
      collision
    */

    VTSS_RC(pcf8575_read(target_id, &data)); 

    lb_data = (data & ~mask) | lb_data;

    if(target_id == 0){
        i2c_addr = 0x23;
    } else if (target_id == 1){
        i2c_addr = 0x21;
    } else if (target_id == 2) {
        i2c_addr = 0x22;
    } else {
        return VTSS_RC_ERROR; 
    }

    VTSS_RC(disconnect_all_i2c_secondary_busses());
    /* Swap LB Data */
    lb_data = ((lb_data << 8) | (lb_data >> 8));
    sprintf(cmd, "p_i2c_wr 0x%x 0x%4.4x", i2c_addr, lb_data);

    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    return VTSS_RC_OK;
}

vtss_rc vsc3172_read(u32 reg_addr, u16 *data)
{
    u32 lb_address = BASE_ADDR_VSC3172;
    char cmd[MAX_CPU_CMD_LEN];
    u32 value;

    lb_address = BASE_ADDR_VSC3172 + (reg_addr << 1);
    sprintf(cmd, "WPRP 0x2018 0x%x03", lb_address);

    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    *data = (u16)value;
    return VTSS_RC_OK;
}

vtss_rc vsc3172_write(u16 reg_addr, u16 lb_data, u16 mask)
{
    u32 lb_address = BASE_ADDR_VSC3172;
    char cmd[MAX_CPU_CMD_LEN];
    u32 value;
    u16 data;

    VTSS_RC(vsc3172_read(reg_addr, &data));

    lb_data = (data & ~mask) | lb_data;
    lb_address = BASE_ADDR_VSC3172 + (reg_addr << 1);
    sprintf(cmd, "WP 0x2000 0x%x%8.8x02", lb_address, lb_data);

    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    return VTSS_RC_OK;
}

vtss_rc si5338_read(u32 target_id, u16 reg_addr, u16 *data)
{
    u32 i2c_bus_number = 0;
    char cmd[MAX_CPU_CMD_LEN];
    u32 value = 0;

    if (target_id > 7) {
      return -1;
    }

    if (target_id < 6) {
        i2c_bus_number = 27 + target_id; /* 27 + target_id = 36 */
    } else {
        (target_id == 6) ? (i2c_bus_number = 37) : (i2c_bus_number = 38) ;
    }
    VTSS_RC(disconnect_all_i2c_secondary_busses());
    VTSS_RC(connect_i2c_secondary_bus(i2c_bus_number));
    sprintf(cmd, "p_i2c_wr 0x%x 0x%2.2x", SI5338_I2C_ADDR, reg_addr);
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    sprintf(cmd, "p_i2c_rd 0x%x 1", SI5338_I2C_ADDR);
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    *data = (u16)value;

    return VTSS_RC_OK;
}

vtss_rc si5338_write(u32 target_id, u16 reg_addr, u16 lb_data, u16 mask)
{
    u32 i2c_bus_number = 0;
    char cmd[MAX_CPU_CMD_LEN];
    u32 value;
    u16 data;

    if (target_id > 7) {
      return -1;
    }

    if (target_id < 6) {
        i2c_bus_number = 27 + target_id;
    } else {
        (target_id == 6) ? (i2c_bus_number = 37) : (i2c_bus_number = 38) ;
    }

    VTSS_RC(si5338_read(target_id , reg_addr, &data));

    lb_data = ((data & ~mask) | lb_data) ;

    VTSS_RC(disconnect_all_i2c_secondary_busses());
    VTSS_RC(connect_i2c_secondary_bus(i2c_bus_number));
    sprintf(cmd, "p_i2c_wr 0x%x 0x%2.2x%2.2x", SI5338_I2C_ADDR, reg_addr, lb_data);
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    return VTSS_RC_OK;
}

vtss_rc si5326_read(u32 target_id, u16 reg_addr, u16 *data)
{
    u32 i2c_bus_number = 0;
    char cmd[MAX_CPU_CMD_LEN];
    u16 address = 0 ;
    u32 value = 0;

    if(target_id > 16){
        return -1;
    }

    i2c_bus_number = 24 + (target_id / 8) ;
    address = 0x68 + (target_id % 8);

    VTSS_RC(disconnect_all_i2c_secondary_busses());
    VTSS_RC(connect_i2c_secondary_bus(i2c_bus_number));

    /*
      Reads from the si5326 are indirect and take two
      i2c transactions.  The address of the register
      being read needs to be written into the si5326 using
      an i2c write command.
      Then an i2c read command is issued which will return
      the data from that register.
    */

    sprintf(cmd, "p_i2c_wr 0x%x 0x%2.2x", address, reg_addr);
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));

    sprintf(cmd, "p_i2c_rd 0x%x 1", address);
    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    *data = (u16)value;
    return VTSS_RC_OK;
}

vtss_rc si5326_write(u32 target_id, u16 reg_addr, u16 lb_data, u16 mask)
{
    u32 i2c_bus_number = 0;
    char cmd[MAX_CPU_CMD_LEN];
    u16 address = 0 ;
    u32 value;
    u16 data;

    if(target_id > 16){
        return -1;
    }

    VTSS_RC(si5326_read(target_id , reg_addr, &data));
    lb_data = ((data & ~mask) | lb_data) ;

    i2c_bus_number = 24 + (target_id / 8) ;
    address = 0x68 + (target_id % 8);

    VTSS_RC(disconnect_all_i2c_secondary_busses());
    VTSS_RC(connect_i2c_secondary_bus(i2c_bus_number));

    sprintf(cmd, "p_i2c_wr 0x%x 0x%2.2x%2.2x", address, reg_addr, lb_data);

    VTSS_RC(vtss_rabbit_cmd(cmd, &value));
    return VTSS_RC_OK;
}

static vtss_rc db_rst_si5326(u16 field, u16 reset)
{
    if (field > 16){
        return VTSS_RC_ERROR;
    }

    if (field == 16) {
        VTSS_RC(pcf8575_write(1, reset, 0x1));
    }
    else {
        VTSS_RC(pcf8575_write(0, (reset << field), (0x1 << field)));
    }

    return VTSS_RC_OK;
}

static vtss_rc db_init_si5338_chan(u32 target, u32 channel_num, u32 mode)
{

    if ((channel_num < 0) || (channel_num > 4)){
        return -1;
    }
    VTSS_RC(si5338_write(target, 51, si5338_val[mode][0], (0x1 << (4+channel_num))));
    VTSS_RC(si5338_write(target, (53 + (11 * channel_num)), si5338_val[mode][1], 0xff));
    VTSS_RC(si5338_write(target, (54 + (11 * channel_num)), si5338_val[mode][2], 0xff));
    VTSS_RC(si5338_write(target, (55 + (11 * channel_num)), si5338_val[mode][3], 0x02));
    VTSS_RC(si5338_write(target, (55 + (11 * channel_num)), (si5338_val[mode][4] << 2), 0xfc));
    VTSS_RC(si5338_write(target, (56 + (11 * channel_num)), si5338_val[mode][5], 0xff));
    VTSS_RC(si5338_write(target, (57 + (11 * channel_num)), si5338_val[mode][6], 0xff));
    VTSS_RC(si5338_write(target, (58 + (11 * channel_num)), si5338_val[mode][7], 0xff));
    VTSS_RC(si5338_write(target, (59 + (11 * channel_num)), si5338_val[mode][8], 0xff));
    VTSS_RC(si5338_write(target, (60 + (11 * channel_num)), si5338_val[mode][9], 0xff));
    VTSS_RC(si5338_write(target, (61 + (11 * channel_num)), si5338_val[mode][10],0xff));
    VTSS_RC(si5338_write(target, (62 + (11 * channel_num)), si5338_val[mode][11], 0x3f));
    VTSS_RC(si5338_write(target, (31 + channel_num), (si5338_val[mode][12] << 5), 0xd0));
    VTSS_RC(si5338_write(target, (31 + channel_num), si5338_val[mode][13], 0x01));
    VTSS_RC(si5338_write(target, (31 + channel_num), (si5338_val[mode][14] << 1), 0x02));
    /* Is this required here? Added just to be in sync with TCL */
    VTSS_RC(si5338_write(target, (31 + channel_num), 0, 0x01));
    return VTSS_RC_OK;
}

static vtss_rc db_init_si5326_chan(int target, int block_status)
{
    u16 data;
    u32 count;

    /*
     first check that things are okay with the inputs on the device
    */
    VTSS_RC(si5326_read(target, 129, &data));
    VTSS_RC(si5326_read(target, 129, &data));
    VTSS_RC(si5326_read(target, 129, &data));

    if (data & 0x01) {
        T_E("(db_init_si5326_chan) No External Reference detected, target $target\n");
        return VTSS_RC_ERROR;
    }

    if ((((data & 0x02) >> 1) == 1) && (((data & 0x04) >> 2) == 1)){
         T_E("(db_init_si5326_chan) No Input Clock detected on either input, target %d reg129: x%x ", target, data);
        return VTSS_RC_ERROR;
    }
    VTSS_RC(si5326_write(target, 0, (si5326_val[block_status][0] << 1), 0x02));    /* bypass_reg */
    VTSS_RC(si5326_write(target, 1, (si5326_val[block_status][1] << 2), 0x0c));    /* ck_prior2 */
    VTSS_RC(si5326_write(target, 1, si5326_val[block_status][2], 0x03));           /* ck_prior1 */
    VTSS_RC(si5326_write(target, 2, (si5326_val[block_status][3] << 4), 0xf0));    /* BWSEL_REG */
    VTSS_RC(si5326_write(target, 4, (si5326_val[block_status][4] << 6), 0xc0));    /* AUTOSEL_REG */
    VTSS_RC(si5326_write(target, 19, (si5326_val[block_status][5] << 7), 0x80)); 
    VTSS_RC(si5326_write(target, 19, (si5326_val[block_status][6] << 5), 0x60)); 
    VTSS_RC(si5326_write(target, 19, si5326_val[block_status][7], 0x07)); 
    VTSS_RC(si5326_write(target, 25,(si5326_val[block_status][8] << 5), 0xe0));    /* N1_HS */
    VTSS_RC(si5326_write(target, 31, si5326_val[block_status][9], 0x0f));          /* NC1_LS */
    VTSS_RC(si5326_write(target, 32, si5326_val[block_status][10], 0xff));          /* NC1_LS */
    VTSS_RC(si5326_write(target, 33, si5326_val[block_status][11], 0xff));          /* NC1_LS */
    VTSS_RC(si5326_write(target, 34, si5326_val[block_status][12], 0x0f));          /* NC2_LS */
    VTSS_RC(si5326_write(target, 35, si5326_val[block_status][13], 0xff));         /* NC2_LS */
    VTSS_RC(si5326_write(target, 36, si5326_val[block_status][14], 0xff));         /* NC2_LS */
    VTSS_RC(si5326_write(target, 40, (si5326_val[block_status][15] << 5), 0xe0));  /* N2_HS */
    VTSS_RC(si5326_write(target, 40, si5326_val[block_status][16], 0x0f));         /* N2_LS */
    VTSS_RC(si5326_write(target, 41, si5326_val[block_status][17], 0xff));         /* N2_LS */
    VTSS_RC(si5326_write(target, 42, si5326_val[block_status][18], 0xff));         /* N2_LS */
    VTSS_RC(si5326_write(target, 43, si5326_val[block_status][19], 0x07));         /* N31 */
    VTSS_RC(si5326_write(target, 44, si5326_val[block_status][20], 0xff));         /* N31 */
    VTSS_RC(si5326_write(target, 45, si5326_val[block_status][21], 0xff));         /* N31 */
    VTSS_RC(si5326_write(target, 46, si5326_val[block_status][22], 0x07));         /* N32 */
    VTSS_RC(si5326_write(target, 47, si5326_val[block_status][23], 0xff));         /* N32 */
    VTSS_RC(si5326_write(target, 48, si5326_val[block_status][24], 0xff));         /* N32 */
    VTSS_RC(si5326_write(target, 55, (si5326_val[block_status][25] << 3), 0x38));  /* CLKIN2RATE */
    VTSS_RC(si5326_write(target, 55, si5326_val[block_status][26], 0x07));         /* CLKIN1RATE */
  
    /* once new settings are in place, do a re-calibration */
    VTSS_RC(si5326_write(target, 136, (0x01 << 6) , 0x40));         /* ical */

    /* poll until lol goes away (up to 1.2 seconds!) */
    for (count = 0; count < 10; count++) {
        usleep((15 * 100 * 1000));
        data = 0;
        VTSS_RC(si5326_read(target, 130, &data));
        if ((data & 0x01) == 0)
           break;
        if (count == 9) {
           T_E("LOL is not cleared");
        }
    }
  
    /* spinOnBit 0 $target register136 ical si5326 10 */
    for (count = 0; count < 10; count++) {
        usleep(10 * 1000);
        data = 0;
        VTSS_RC(si5326_read(target, 136, &data));
        if ((data & 0x40) == 0)
           break;
        if (count == 9) {
           T_E("ical is not cleared");
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc db_init_si5338(u32 target, u32 hs_mode)
{
    u32 i;
    /* hs_mode considered none for now */
    u8 pllparam[] = {0x3a, 0x4, 0x3, 0x7};
    u8 feedparam[] = {0xc4, 0x2f, 0x0, 0x3c, 0x1, 0x0, 0x0, 0x71, 0x2, 0x0, 0x0};
    u8 vddlevel = 0, outmode = 0, invert = 0, trim = 0;
    u16 data;
    u32 count;

    /* now do full reset, clearing all registers*/
    VTSS_RC(si5338_write(target, 246, 1, 0xff));
    /* poll to make sure reset cleared */
    for(count =0; count < 10; count++) {
        usleep(10 * 1000);
        data = 0;
        VTSS_RC(si5338_read(target, 246, &data));
        if ((data & 0x01) == 0)
            break;
        T_E("Polling again for POR_RESET ");
        if (count == 9) {
           T_E("POR_RESET is not cleared");
        }
    }

    /*
     then check that we have an input clock that is good
    */
    VTSS_RC(si5338_read(target, 218, &data));
    if(data){
       T_E("(db_init_si5338) input clock is not good");
       return VTSS_RC_ERROR;
    }

    /*
      Check pll_lol
    */
    VTSS_RC(si5338_read(target, 218, &data));
    if(data){
        T_E("(db_init_si5338) Check pll_lol");
        return VTSS_RC_ERROR;
    }

    /*
    # set these which deal with the feedback input clock
   */
    VTSS_RC(si5338_write(target, 106, (0x1 << 7), 0x80));
    VTSS_RC(si5338_write(target, 116, (0x1 << 7), 0x80));
    VTSS_RC(si5338_write(target, 30, (0x2 << 3), 0x18));

    /*
     now power down all outputs so we do not get runt pulses
     will re-enable each output as it is individually configured
    */
    VTSS_RC(si5338_write(target, SI5338_POWERDOWN_0_ADDR, 0x1, 0x1));
    VTSS_RC(si5338_write(target, SI5338_POWERDOWN_1_ADDR, 0x1, 0x1));
    VTSS_RC(si5338_write(target, SI5338_POWERDOWN_2_ADDR, 0x1, 0x1));
    VTSS_RC(si5338_write(target, SI5338_POWERDOWN_3_ADDR, 0x1, 0x1));

    /*
     pll
    */
    VTSS_RC(si5338_write(target, 48, pllparam[0], 0x7f));
    VTSS_RC(si5338_write(target, 50, pllparam[1], 0x3f));
    VTSS_RC(si5338_write(target, 50, (pllparam[2] << 6), 0x60));
    VTSS_RC(si5338_write(target, 51, pllparam[3], 0x07));

    /*
    # feedback
    */
    VTSS_RC(si5338_write(target, 97, feedparam[0], 0xff));
    VTSS_RC(si5338_write(target, 98, feedparam[1], 0xff));
    VTSS_RC(si5338_write(target, 99, feedparam[2], 0x03));
    VTSS_RC(si5338_write(target, 99, ((feedparam[3]) << 2), 0xf6));
    VTSS_RC(si5338_write(target, 100, feedparam[4], 0xff));
    VTSS_RC(si5338_write(target, 101, feedparam[5], 0xff));
    VTSS_RC(si5338_write(target, 102, feedparam[6], 0xff));
    VTSS_RC(si5338_write(target, 103, feedparam[7], 0xff));
    VTSS_RC(si5338_write(target, 104, feedparam[8], 0xff));
    VTSS_RC(si5338_write(target, 105, feedparam[9], 0xff));
    VTSS_RC(si5338_write(target, 106, feedparam[10], 0x3f));

    /* setup output voltage levels on each channel */

    for(i = 0; i < 4; i++) {
        switch(target){
        /* si5338_x */
        case 4:
        case 5:
            vddlevel = 0x0;
            outmode = 0x6;
            invert = 0x0;
            trim = 0x3;
            break;
        /* MISC */
        case 6:
            vddlevel = 0x0;
            outmode = 0x6;
            invert = 0x0;
            trim = 0x3;
            break;
         /* 3406 */
        case 7:
            vddlevel = 0x0;
            outmode = 0x6;
            invert = 0x0;
            trim = 0x3;
            break;
        default:
            T_E("Invalid Device");
        }

        VTSS_RC(si5338_write(target, (i+36), outmode, 0x0f));

        if (i == 0) {
            VTSS_RC(si5338_write(target, 40, trim, 0x1f));
        } else if (i == 1) {
            VTSS_RC(si5338_write(target, 40, ((trim & 0x7) << 5), 0xe0));
            VTSS_RC(si5338_write(target, 41, (trim >> 3), 0x03));
        } else if (i == 2) {
            VTSS_RC(si5338_write(target, 41, (trim << 2), 0x7c));
        } else if (i == 3) {
            VTSS_RC(si5338_write(target, 42, trim, 0x1f));
        }
    }

    /*
    now reset multisynths since we messed around with the front-end of the chip
    */
    VTSS_RC(si5338_write(target, 246, (0x1 << 1), 0x02));
    VTSS_RC(si5338_write(target, 246, 0x0, 0x02));
    return VTSS_RC_OK;
}

static vtss_rc db_init_si5326(u32 target)
{
    u16 data1, data2, outmode;
    u32 count;

    VTSS_RC(si5326_read(target, 134, &data1));
    VTSS_RC(si5326_read(target, 135, &data2));


    if ((data1 != 0x1) || ((data2 & 0xf0) != 0xa0)) {
      T_D("db_init_si5326) Invalid device ID read, (exp 0x01a2), target %d data1: %x data2: %x", (int)target, data1, data2);
    }

    /* # force an internal reset before we start configuring */
    VTSS_RC(si5326_write(target, 136, (0x1 << 7) , 0x80));

    for (count = 0; count < 10; count++) {
        usleep(10 * 1000); /* Sleep for 10 ms */
        data1 = 0;
        VTSS_RC(si5326_read(target, 136, &data1));
        if ((data1 & 0x80) == 0)
            break;
        T_E("Polling again for RST_REG");
        if (count == 9) {
            T_E("RST_REG is not cleared");
        }
    }

    /*
       keep clock running during reset/calibration, a bouncing clock will be more stressful
       for Daytona

       use automatic switching if we lose a clock
    */
    VTSS_RC(si5326_write(target, 3, 0, 0x10));
    VTSS_RC(si5326_write(target, 4, (0x2 << 6), 0xc0));

    outmode = 0x7;
    VTSS_RC(si5326_write(target, 6, (outmode << 3), 0x38));
    VTSS_RC(si5326_write(target, 6, outmode, 0x07));

    /* # set input clk range for alarm monitoring (all clocks between 95 and 215Mhz */
    VTSS_RC(si5326_write(target, 55, (0x3 << 3), 0x38)); /* clkin2rate */
    VTSS_RC(si5326_write(target, 55, 0x3, 0x07)); /* clkin1rate */
    return VTSS_RC_OK;
}

static vtss_rc db_init_clocks(u32 board)
{
    u32 num_channels;

    if (board == EVAL_BOARD_DAYTONA){
        num_channels = 2;
    } else if(board == EVAL_BOARD_TALLADEGA){
        num_channels = 4;
    } else {
        return VTSS_RC_ERROR; 
    }
    /*
      SI5338
      bring devices out of reset
      this device covers the sys and test clock for DUT and 2 3406's
    */
    if (db_init_si5338(SI5338_TGTID_MISC, HS_MODE_NONE) != VTSS_RC_OK) {
        T_E("db_init_si5338 failed for target %d and protocol %d", SI5338_TGTID_MISC, HS_MODE_NONE); 
        return VTSS_RC_ERROR;
    }
    /* this device covers 3406's per channel */
    if (db_init_si5338(SI5338_TGTID_VSC3406, HS_MODE_NONE) != VTSS_RC_OK) {
        T_E("db_init_si5338 failed for target %d and protocol %d", SI5338_TGTID_VSC3406, HS_MODE_NONE); 
        return VTSS_RC_ERROR;
    }

    /* there is one device that feeds 2 channels on the DUT */
    if (db_init_si5338(SI5338_TGTID_CHAN_0_1, HS_MODE_NONE != VTSS_RC_OK)) {
        T_E("db_init_si5338 failed for target %d and protocol %d", SI5338_TGTID_CHAN_0_1, HS_MODE_NONE); 
        return VTSS_RC_ERROR;
    }

    /*
    sysclk (0) and test clk (3) are static after power-on, so set these up now
    */
#ifdef BOARD_TALLADEGA
    if (db_init_si5338_chan(SI5338_TGTID_MISC, 0, PROTO_SONET) != VTSS_RC_OK) {
#else
    if (db_init_si5338_chan(SI5338_TGTID_MISC, 0, PROTO_XAUI_10GE) != VTSS_RC_OK) {
#endif
        T_E("db_init_si5338_chan failed for target %d and protocol %d", SI5338_TGTID_MISC, PROTO_XAUI_10GE);
        return VTSS_RC_ERROR;
    }

    /*
    TRN - this used to be si5338_misc 1, which I think was incorrect
    */
    if (db_init_si5338_chan(SI5338_TGTID_MISC, 3, PROTO_250)!= VTSS_RC_OK) {
        T_E("db_init_si5338_chan failed for target %d and protocol %d", SI5338_TGTID_MISC, PROTO_250);
        return VTSS_RC_ERROR;
    }

    /*
      SI5326
      STFPGA
    */

    /* For two sides two channels */
    /* The sequence of 2,0,3,1 is to ensure that the seqence of cmds are same as TCL generated*/
    VTSS_RC(db_rst_si5326(2, 0));
    VTSS_RC(db_rst_si5326(2, 1));
    VTSS_RC(db_init_si5326(2));
    VTSS_RC(db_rst_si5326(0, 0));
    VTSS_RC(db_rst_si5326(0, 1));
    VTSS_RC(db_init_si5326(0));
    VTSS_RC(db_rst_si5326(3, 0));
    VTSS_RC(db_rst_si5326(3, 1));
    VTSS_RC(db_init_si5326(3));
    VTSS_RC(db_rst_si5326(1, 0));
    VTSS_RC(db_rst_si5326(1, 1));
    VTSS_RC(db_init_si5326(1));

    /*
     8486
     bring devices out of HW reset
    */
    usleep(10);
    VTSS_RC(db_rst_si5326(16, 0));
    VTSS_RC(db_rst_si5326(16, 1));
    /* do a sw reset and set up levels so we do not blow out any devices */

    db_init_si5326(16);
    return VTSS_RC_OK;
}


static vtss_rc d_init_board(u32 board)
{
    u32 data;
#ifdef BOARD_TALLADEGA
    int i;
#endif

    VTSS_RC(db_hw_reset()) ;

    /* Board type can be read from LBFPGA to find out the type of board.
       As of now it is fixed
    */

    printf("(d_init_board) Setting up Validation Board.\n");

    switch(board){
    case EVAL_BOARD_DAYTONA:
         /* Fall-through */
    case EVAL_BOARD_TALLADEGA:
         /* take 3 I2C controllers out of reset */
        VTSS_RC(lbfpga_write(LB_I2C_CONTROLLER_RESET, 0, 0xffffffff));
        if( db_init_clocks(board) != VTSS_RC_OK) {
            T_E("db_init_clocks failed to initialize");
            return VTSS_RC_ERROR;
        }
#ifdef BOARD_TALLADEGA
        /* set CPU mode*/
        VTSS_RC(lbfpga_write(LB_MOTOROLA_MODE, 0, 1));
        /*Determine the mode */
        VTSS_RC(lbfpga_read (LB_GPIN, &data));
        if(data == 0) {
          VTSS_RC(pcf8575_write(1, 0, (1 << 15)));
          VTSS_RC(pcf8575_write(1, (1 << 15), (1 << 15)));
        }
#endif
        VTSS_RC(pcf8575_write(1, 0, (1 << 11))); /* client_0_refclk_sel_2 */
        VTSS_RC(pcf8575_write(1, 0, (1 << 12))); /* client_1_refclk_sel_2 */
        VTSS_RC(pcf8575_write(1, 0, (1 << 13))); /* client_2_refclk_sel_2 */
        VTSS_RC(pcf8575_write(1, 0, (1 << 14))); /* client_3_refclk_sel_2 */
        VTSS_RC(lbfpga_write(LB_VSC3172_CTRL, 0, 0x1));
        VTSS_RC(vsc3172_write(VSC3172_REGISTER_INIT, 0xff, 0x7ff));
        VTSS_RC(vsc3172_write(VSC3172_SWITCH_CONFIGURATION, 0, (0x1 << 5))); /* striping_mode_select */

#ifdef BOARD_TALLADEGA
        for (i=IF_UPI0;i< (IF_MAX-1);i++)
          db_setup_switch(i,i);

        /*setup generic drive level for all ports*/
        VTSS_RC(vsc3172_write(0x28C,0 ,(0x1 << 1)));
        VTSS_RC(vsc3172_write(0x28C,0x1 ,(0x1)));
        VTSS_RC(vsc3172_write(0x28A,(0x1 << 5),(0x1 << 5)));
        VTSS_RC(vsc3172_write(0x28A,0 ,(0x1 << 4)));
        VTSS_RC(vsc3172_write(0x28A,0 ,(0x1 << 7)));
        VTSS_RC(vsc3172_write(0x28A,(0x1<<6) ,(0x1 << 6)));
        VTSS_RC(vsc3172_write(0x28A,0,(0x1 << 1)));
        VTSS_RC(vsc3172_write(0x28A,0,0x1));

        VTSS_RC(vsc3172_write(0x28A,0,(0x1 << 2)));
        VTSS_RC(vsc3172_write(0x28A,(0x1 << 3),(0x1 << 3)));
        VTSS_RC(vsc3172_write(0x28A,0,0x1));
        VTSS_RC(vsc3172_write(0x28A,(0x1 << 1),(0x1 << 1)));
        /* TODO 1 more vsc3172_write */
#endif

        /* check that TRST and reset are active on Daytona */
        VTSS_RC(lbfpga_read(LB_VSC8492_RESET_CTRL, &data));
        if((data & 0x1) != 1) {
            T_D("(d_init_board) Daytana TRST not asserted at startup, re-asserting");
            VTSS_RC(lbfpga_write(LB_VSC8492_RESET_CTRL, 1, 0x1));
        }
        data = 0;
        VTSS_RC(lbfpga_read(LB_VSC8492_RESET_CTRL, &data));
        if((data & 0x2) != 2) {
            T_D("(d_init_board) Daytana RESET not asserted at startup, re-asserting");
            VTSS_RC(lbfpga_write(LB_VSC8492_RESET_CTRL, 2, 0x2));
        }
        VTSS_RC(lbfpga_write(0x2c, 0x3,0xffffffff));
        break;

    default:
        T_E("Invalid Board Type\n");
    }
#ifdef BOARD_TALLADEGA
    /*Turn off red light and turn on green light */
    lbfpga_write(4, 0, 1);
    lbfpga_write(4, (0x1 << 1), 2);
#endif

    return VTSS_RC_OK;
}

static vtss_rc di_sw(u32 board, u32 device)
{
    /*
    this device covers the line and client hold-over clocks for the DUT
    */
    printf("(d_init_device) Bring SiLabs clocking chips out of reset\n");

    if (db_init_si5338(SI5338_TGTID_CHAN_0_1, HS_MODE_NONE) != VTSS_RC_OK) {
        T_E("db_init_si5338 Failed for target %d protocol %d", SI5338_TGTID_CHAN_0_1, HS_MODE_NONE);
    }

    if(device == DEVICE_TALLADEGA){
        if(db_init_si5338(SI5338_TGTID_CHAN_2_3, HS_MODE_NONE) != VTSS_RC_OK) {
            T_E("db_init_si5338 Failed for target %d protocol %d", SI5338_TGTID_CHAN_2_3, HS_MODE_NONE);
        }
    }

    /*
      SI5326
      STFPGA
    */

    /* For two sides two channels */
    /* The sequence of 2,0,3,1 is to ensure that the seqence of cmds are same as TCL generated*/
    VTSS_RC(db_rst_si5326(9, 0));
    VTSS_RC(db_rst_si5326(9, 1));
    VTSS_RC(db_init_si5326(SI5326_CLK_GEN_BASE+1));
    VTSS_RC(db_rst_si5326(8, 0));
    VTSS_RC(db_rst_si5326(8, 1));
    VTSS_RC(db_init_si5326(SI5326_CLK_GEN_BASE+0));
    VTSS_RC(db_rst_si5326(11, 0));
    VTSS_RC(db_rst_si5326(11, 1));
    VTSS_RC(db_init_si5326(SI5326_CLK_GEN_BASE+3));
    VTSS_RC(db_rst_si5326(10, 0));
    VTSS_RC(db_rst_si5326(10, 1));
    VTSS_RC(db_init_si5326(SI5326_CLK_GEN_BASE+2));

    if(device == DEVICE_TALLADEGA){
        VTSS_RC(db_rst_si5326(13, 0));
        VTSS_RC(db_rst_si5326(13, 1));
        VTSS_RC(db_init_si5326(SI5326_CLK_GEN_BASE+5));
        VTSS_RC(db_rst_si5326(12, 0));
        VTSS_RC(db_rst_si5326(12, 1));
        VTSS_RC(db_init_si5326(SI5326_CLK_GEN_BASE+4));
        VTSS_RC(db_rst_si5326(15, 0));
        VTSS_RC(db_rst_si5326(15, 1));
        VTSS_RC(db_init_si5326(SI5326_CLK_GEN_BASE+7));
        VTSS_RC(db_rst_si5326(14, 0));
        VTSS_RC(db_rst_si5326(14, 1));
        VTSS_RC(db_init_si5326(SI5326_CLK_GEN_BASE+6));
    }

    /*
      # FIXME...SW reset device assert...and deasert reset
    */
    /* TODO Writing the RESET and TSRT both to 1 and clear only RESET --> TCL output*/
    VTSS_RC(lbfpga_write(LB_VSC8492_RESET_CTRL, 0x3, 0x2));
    VTSS_RC(lbfpga_write(LB_VSC8492_RESET_CTRL, 0x1, 0x2));
    printf("(d_init_device) Device Init Complete, ready to begin Channel Init.\n");
    return VTSS_RC_OK;
}

static vtss_rc db_init_clk_mux(int channel, int side, int mode)
{
    /* Side 1 is client side 2 is line */

    if ((channel == 0) && (side == 1)) {
        if ((mode == VTSS_CONFIG_MODE_PEO_P1E_100) ||
            (mode == VTSS_CONFIG_MODE_PEO_P2E_100) ||
            (mode == VTSS_CONFIG_MODE_BOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TEE_L)       ||
            (mode == VTSS_CONFIG_MODE_TEE_PMP_S)    ||
            (mode == VTSS_CONFIG_MODE_TEE_W)       ||
            (mode == VTSS_CONFIG_MODE_TFF)         ||
            (mode == VTSS_CONFIG_MODE_TEOC_2E)     ||
            (mode == VTSS_CONFIG_MODE_TEOC_1E)     ||
            (mode == VTSS_CONFIG_MODE_TSS)         ||
            (mode == VTSS_CONFIG_MODE_TOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TOO_2F2)     ||
            (mode == VTSS_CONFIG_MODE_TOO_2E2)     ||
            (mode == VTSS_CONFIG_MODE_TSOC_S)      || 
            (mode == VTSS_CONFIG_MODE_BMX_2)       ||
            (mode == VTSS_CONFIG_MODE_TSOC_WS)) {
            VTSS_RC(pcf8575_write(1, (0 << 0x3), (0x1 << 0x3)));
            VTSS_RC(pcf8575_write(2, 1, 0x3));
        } else {
            VTSS_RC(pcf8575_write(1, 0, (0x1 << 0x3)));
            VTSS_RC(pcf8575_write(2, 0x3, 0x3));
        }
    }

    if ((channel == 0) && (side == 2)) {
        if ((mode == VTSS_CONFIG_MODE_PEO_P1E_100) ||
            (mode == VTSS_CONFIG_MODE_PEO_P2E_100) ||
            (mode == VTSS_CONFIG_MODE_BOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TEE_L)      ||
            (mode == VTSS_CONFIG_MODE_TEE_PMP_S)    ||
            (mode == VTSS_CONFIG_MODE_TEE_W)       ||
            (mode == VTSS_CONFIG_MODE_TFF)         ||
            (mode == VTSS_CONFIG_MODE_TEOC_2E)     ||
            (mode == VTSS_CONFIG_MODE_TEOC_1E)     ||
            (mode == VTSS_CONFIG_MODE_TSS)         ||
            (mode == VTSS_CONFIG_MODE_TOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TOO_2F2)     ||
            (mode == VTSS_CONFIG_MODE_TOO_2E2)     ||
            (mode == VTSS_CONFIG_MODE_TSOC_S)      || 
            (mode == VTSS_CONFIG_MODE_BMX_2)       ||
            (mode == VTSS_CONFIG_MODE_TSOC_WS)) {
            VTSS_RC(pcf8575_write(1, 0, (0x1 << 0x3)));
            VTSS_RC(pcf8575_write(2, 0, (0x3 << 2)));
        } else {
            VTSS_RC(pcf8575_write(1, 0, (0x1 << 0x3)));
            VTSS_RC(pcf8575_write(2, (0x3 << 2), (0x3 << 2)));
        }
    }

    if ((channel == 1) && (side == 1)) {
        if ((mode == VTSS_CONFIG_MODE_PEO_P1E_100) ||
            (mode == VTSS_CONFIG_MODE_PEO_P2E_100) ||
            (mode == VTSS_CONFIG_MODE_BOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TEE_L)       ||
            (mode == VTSS_CONFIG_MODE_TEE_PMP_S)    ||
            (mode == VTSS_CONFIG_MODE_TEE_W)       ||
            (mode == VTSS_CONFIG_MODE_TFF)         ||
            (mode == VTSS_CONFIG_MODE_TEOC_2E)     ||
            (mode == VTSS_CONFIG_MODE_TEOC_1E)     ||
            (mode == VTSS_CONFIG_MODE_TSS)         ||
            (mode == VTSS_CONFIG_MODE_TOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TOO_2F2)     ||
            (mode == VTSS_CONFIG_MODE_TOO_2E2)     ||
            (mode == VTSS_CONFIG_MODE_TSOC_S)      || 
            (mode == VTSS_CONFIG_MODE_BMX_2)       ||
            (mode == VTSS_CONFIG_MODE_TSOC_WS)) {
            VTSS_RC(pcf8575_write(1, 0, (0x1 << 12)));
            VTSS_RC(pcf8575_write(2, (1 << 4), (0x3 << 4)));
        } else {
            VTSS_RC(pcf8575_write(1, 0, 0x0800));
            VTSS_RC(pcf8575_write(2, (0x3 << 4), (0x3 << 4)));
        }
    }

    if ((channel == 1) && (side == 2)) {
        if ((mode == VTSS_CONFIG_MODE_PEO_P1E_100) ||
            (mode == VTSS_CONFIG_MODE_PEO_P2E_100) ||
            (mode == VTSS_CONFIG_MODE_BOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TEE_L)       ||
            (mode == VTSS_CONFIG_MODE_TEE_PMP_S)    ||
            (mode == VTSS_CONFIG_MODE_TEE_W)       ||
            (mode == VTSS_CONFIG_MODE_TFF)         ||
            (mode == VTSS_CONFIG_MODE_TEOC_2E)     ||
            (mode == VTSS_CONFIG_MODE_TEOC_1E)     ||
            (mode == VTSS_CONFIG_MODE_TSS)         ||
            (mode == VTSS_CONFIG_MODE_TOO_2)       || 
            (mode == VTSS_CONFIG_MODE_TOO_2F2)     ||
            (mode == VTSS_CONFIG_MODE_TOO_2E2)     ||
            (mode == VTSS_CONFIG_MODE_TSOC_S)      || 
            (mode == VTSS_CONFIG_MODE_BMX_2)       ||
            (mode == VTSS_CONFIG_MODE_TSOC_WS)) {
            VTSS_RC(pcf8575_write(1, 0, (0x1 << 12)));
            VTSS_RC(pcf8575_write(2, 0, (0x3 << 6)));
        } else {
            VTSS_RC(pcf8575_write(1, 0, 0x0800));
            VTSS_RC(pcf8575_write(2, (0x3 << 6), (0x3 << 6)));
        }
    }

    if ((channel == 2) && (side == 1)) {
        if ((mode == VTSS_CONFIG_MODE_PEO_P1E_100) ||
            (mode == VTSS_CONFIG_MODE_PEO_P2E_100) ||
            (mode == VTSS_CONFIG_MODE_BOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TEE_L)       ||
            (mode == VTSS_CONFIG_MODE_TEE_PMP_S)    ||
            (mode == VTSS_CONFIG_MODE_TEE_W)       ||
            (mode == VTSS_CONFIG_MODE_TFF)         ||
            (mode == VTSS_CONFIG_MODE_TEOC_2E)     ||
            (mode == VTSS_CONFIG_MODE_TEOC_1E)     ||
            (mode == VTSS_CONFIG_MODE_TSS)         ||
            (mode == VTSS_CONFIG_MODE_TOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TOO_2F2)     ||
            (mode == VTSS_CONFIG_MODE_TOO_2E2)     ||
            (mode == VTSS_CONFIG_MODE_TSOC_S)      ||
            (mode == VTSS_CONFIG_MODE_BMX_2)       ||
            (mode == VTSS_CONFIG_MODE_TSOC_WS)) {
            VTSS_RC(pcf8575_write(1, 0, (0x2 << 12)));
            VTSS_RC(pcf8575_write(2, (0x1 << 8), (0x3 << 8)));
        } else {
            VTSS_RC(pcf8575_write(1, 0, (0x2 << 12)));
            VTSS_RC(pcf8575_write(2, (0x3 << 8), 0x3 << 8 ));
        }
    }

    if ((channel == 2) && (side == 2)) {
        if ((mode == VTSS_CONFIG_MODE_PEO_P1E_100) ||
            (mode == VTSS_CONFIG_MODE_PEO_P2E_100) ||
            (mode == VTSS_CONFIG_MODE_BOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TEE_L)       ||
            (mode == VTSS_CONFIG_MODE_TEE_PMP_S)    ||
            (mode == VTSS_CONFIG_MODE_TEE_W)       ||
            (mode == VTSS_CONFIG_MODE_TFF)         ||
            (mode == VTSS_CONFIG_MODE_TEOC_2E)     ||
            (mode == VTSS_CONFIG_MODE_TEOC_1E)     ||
            (mode == VTSS_CONFIG_MODE_TSS)         ||
            (mode == VTSS_CONFIG_MODE_TOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TOO_2F2)     ||
            (mode == VTSS_CONFIG_MODE_TOO_2E2)     ||
            (mode == VTSS_CONFIG_MODE_TSOC_S)      ||
            (mode == VTSS_CONFIG_MODE_BMX_2)       ||
            (mode == VTSS_CONFIG_MODE_TSOC_WS)) {
            VTSS_RC(pcf8575_write(1, 0, (0x2 << 12)));
            VTSS_RC(pcf8575_write(2, 0, (0x0C << 8)));
        } else {
            VTSS_RC(pcf8575_write(1, 0, (0x2 << 12)));
            VTSS_RC(pcf8575_write(2, (0x3 << 10), (0x0C << 8)));
        }
    }

    if ((channel == 3) && (side == 1)) {
        if ((mode == VTSS_CONFIG_MODE_PEO_P1E_100) ||
            (mode == VTSS_CONFIG_MODE_PEO_P2E_100) ||
            (mode == VTSS_CONFIG_MODE_BOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TEE_L)       ||
            (mode == VTSS_CONFIG_MODE_TEE_PMP_S)    ||
            (mode == VTSS_CONFIG_MODE_TEE_W)       ||
            (mode == VTSS_CONFIG_MODE_TFF)         ||
            (mode == VTSS_CONFIG_MODE_TEOC_2E)     ||
            (mode == VTSS_CONFIG_MODE_TEOC_1E)     ||
            (mode == VTSS_CONFIG_MODE_TSS)         ||
            (mode == VTSS_CONFIG_MODE_TOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TOO_2F2)     ||
            (mode == VTSS_CONFIG_MODE_TOO_2E2)     ||
            (mode == VTSS_CONFIG_MODE_TSOC_S)      ||
            (mode == VTSS_CONFIG_MODE_BMX_2)       ||
            (mode == VTSS_CONFIG_MODE_TSOC_WS)) {
            VTSS_RC(pcf8575_write(1, 0, (0x4 << 12)));
            VTSS_RC(pcf8575_write(2, (1 << 12), 0x3 << 12));
        } else {
            VTSS_RC(pcf8575_write(1, 0, (0x4 << 12)));
            VTSS_RC(pcf8575_write(2, (0x3 << 12), 0x3 << 12 ));
        }
    }

    if ((channel == 3) && (side == 2)) {
        if ((mode == VTSS_CONFIG_MODE_PEO_P1E_100) ||
            (mode == VTSS_CONFIG_MODE_PEO_P2E_100) ||
            (mode == VTSS_CONFIG_MODE_BOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TEE_L)       ||
            (mode == VTSS_CONFIG_MODE_TEE_PMP_S)    ||
            (mode == VTSS_CONFIG_MODE_TEE_W)       ||
            (mode == VTSS_CONFIG_MODE_TFF)         ||
            (mode == VTSS_CONFIG_MODE_TEOC_2E)     ||
            (mode == VTSS_CONFIG_MODE_TEOC_1E)     ||
            (mode == VTSS_CONFIG_MODE_TSS)         ||
            (mode == VTSS_CONFIG_MODE_TOO_2)       ||
            (mode == VTSS_CONFIG_MODE_TOO_2F2)     ||
            (mode == VTSS_CONFIG_MODE_TOO_2E2)     ||
            (mode == VTSS_CONFIG_MODE_TSOC_S)      ||
            (mode == VTSS_CONFIG_MODE_BMX_2)       ||
            (mode == VTSS_CONFIG_MODE_TSOC_WS)) {
            VTSS_RC(pcf8575_write(1, 0, (0x4 << 12)));
            VTSS_RC(pcf8575_write(2, 0, (0x0C << 12)));
        } else {
            VTSS_RC(pcf8575_write(1, 0, (0x4 << 12)));
            VTSS_RC(pcf8575_write(2, (0x3 << 14), (0x0C << 12)));
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc dd_get_block_status(int block, vtss_config_mode_t mode, int *block_state, u32 x4_case)
{
    if ((block == BLOCK_SI5326_1) || (block == BLOCK_SI5326_2))
    {
        switch (mode) {
        case VTSS_CONFIG_MODE_PEE_W:
        case VTSS_CONFIG_MODE_PEE_MW:
        case VTSS_CONFIG_MODE_PEE_P:
        case VTSS_CONFIG_MODE_PEE_MP: 
        case VTSS_CONFIG_MODE_PEO_WA:
        case VTSS_CONFIG_MODE_PEO_MWA:
        case VTSS_CONFIG_MODE_PEO_WS:
        case VTSS_CONFIG_MODE_PEO_MWS:
        case VTSS_CONFIG_MODE_PEO_P2E_20:
        case VTSS_CONFIG_MODE_PEO_MP2E_20:
        case VTSS_CONFIG_MODE_PGO_2:
        case VTSS_CONFIG_MODE_PGO_2SUB:
        case VTSS_CONFIG_MODE_TGO_2:
        case VTSS_CONFIG_MODE_TGO_2SUB:
        case VTSS_CONFIG_MODE_TGO_2SUBSMALL:
        case VTSS_CONFIG_MODE_TFO_PP2F:
        case VTSS_CONFIG_MODE_TEO_PMP_2E:
        case VTSS_CONFIG_MODE_TEO_PMP_1E:
        case VTSS_CONFIG_MODE_TEO_PP_2E:
        case VTSS_CONFIG_MODE_TEE_PMP_A:
        case VTSS_CONFIG_MODE_TEE_PMW:
        case VTSS_CONFIG_MODE_TEE_PP:
            *block_state = BS_BYP_REF;
            break;
        case VTSS_CONFIG_MODE_PEO_P1E_100:
            (block == BLOCK_SI5326_1) ? (*block_state = BS_15232_16830) : (*block_state = BS_16830_15232);
            break;
        case VTSS_CONFIG_MODE_PEO_P2E_100:
            (block == BLOCK_SI5326_1) ? (*block_state = BS_15168_16830) : (*block_state = BS_16830_15168);
            break;
        case VTSS_CONFIG_MODE_BOO_2:
            (block == BLOCK_SI5326_1) ? (*block_state = BS_66_64) : (*block_state = BS_64_66);
            break;
        case VTSS_CONFIG_MODE_TEE_L:
        case VTSS_CONFIG_MODE_TEE_PMP_S:
        case VTSS_CONFIG_MODE_TEE_W:
        case VTSS_CONFIG_MODE_TFF:
        case VTSS_CONFIG_MODE_TSS:
        case VTSS_CONFIG_MODE_TOO_2:
        case VTSS_CONFIG_MODE_TOO_2E2: 
        case VTSS_CONFIG_MODE_TOO_2F2: 
            *block_state = BS_CLEANUP;
            break;
        case VTSS_CONFIG_MODE_TEOC_2E:
        case VTSS_CONFIG_MODE_TSOC_S:
        case VTSS_CONFIG_MODE_TSOC_WS:
            (block == BLOCK_SI5326_1) ? (*block_state = BS_237_255) : (*block_state = BS_255_237);
            break;
        case VTSS_CONFIG_MODE_TEOC_1E:
            (block == BLOCK_SI5326_1) ? (*block_state = BS_238_255) : (*block_state = BS_255_238);
            break;
        case VTSS_CONFIG_MODE_BMX_2:
            (block == BLOCK_SI5326_1) ? (*block_state = BS_66_64) : (*block_state = BS_64_66);
            break;
        default:
            T_E("dd_get_block_status: Unsupported Mode %d for block %d", mode, block);
            return VTSS_RC_ERROR;
        }
        if ((x4_case == 1) || (x4_case == 2)) {
            switch(mode) {
            case VTSS_CONFIG_MODE_TSS:
                *block_state = (x4_case == 1) ? BS_X4 : BS_BYP_X4_SONET;
                break;
            case VTSS_CONFIG_MODE_TSOC_S:
                if (block == BLOCK_SI5326_1)
                    *block_state = (x4_case == 1) ? BS_237_255_X4 : BS_BYP_X4_TSOC1;
                else if (block == BLOCK_SI5326_2)
                    *block_state = (x4_case == 1) ? BS_255_237_X4 : BS_BYP_X4_TSOC2;
                break;
            case VTSS_CONFIG_MODE_TOO_2:
                *block_state = (x4_case == 1) ? BS_X4 : BS_BYP_X4_OTU2;
                break;
            case VTSS_CONFIG_MODE_TOO_2E2:
                *block_state = (x4_case == 1) ? BS_X4_OTU2E : BS_BYP_X4_OTU2E;
                break;
            default:
                T_E("Block status get unsupported mode %d for X4 case", mode);
                return VTSS_RC_ERROR;
            }                
        }
    }

    if (block == BLOCK_SI5338_1) 
    { 
        switch (mode) {
        case VTSS_CONFIG_MODE_PEE_W:
        case VTSS_CONFIG_MODE_PEE_MW:
        case VTSS_CONFIG_MODE_PEE_P:
        case VTSS_CONFIG_MODE_PEE_MP: 
        case VTSS_CONFIG_MODE_PEO_WA:
        case VTSS_CONFIG_MODE_PEO_MWA:
        case VTSS_CONFIG_MODE_PEO_WS:
        case VTSS_CONFIG_MODE_PEO_MWS:
        case VTSS_CONFIG_MODE_PEO_P2E_20:
        case VTSS_CONFIG_MODE_PEO_MP2E_20:
        case VTSS_CONFIG_MODE_PGO_2:
        case VTSS_CONFIG_MODE_PGO_2SUB:
            *block_state = PROTO_XAUI_10GE;
            break;
        case VTSS_CONFIG_MODE_PEO_P1E_100:
        case VTSS_CONFIG_MODE_TEOC_1E:
            *block_state = PROTO_OTU1E;
            break;
        case VTSS_CONFIG_MODE_PEO_P2E_100:
        case VTSS_CONFIG_MODE_TEOC_2E:
        case VTSS_CONFIG_MODE_TOO_2E2:
            *block_state = PROTO_OTU2E;
            break;
        case VTSS_CONFIG_MODE_BOO_2:
        case VTSS_CONFIG_MODE_TSOC_S:
        case VTSS_CONFIG_MODE_TSOC_WS:
        case VTSS_CONFIG_MODE_TOO_2:
        case VTSS_CONFIG_MODE_BMX_2:
            *block_state = PROTO_OTU2;
            break;
        case VTSS_CONFIG_MODE_TGO_2:
        case VTSS_CONFIG_MODE_TGO_2SUB:
        case VTSS_CONFIG_MODE_TGO_2SUBSMALL:
        case VTSS_CONFIG_MODE_TEE_L:
        case VTSS_CONFIG_MODE_TEE_PMP_S:
        case VTSS_CONFIG_MODE_TEE_PMP_A:
        case VTSS_CONFIG_MODE_TEE_PMW:
        case VTSS_CONFIG_MODE_TEE_PP:
        case VTSS_CONFIG_MODE_TEO_PMP_2E:
        case VTSS_CONFIG_MODE_TEO_PMP_1E:
        case VTSS_CONFIG_MODE_TEO_PP_2E:
            *block_state = PROTO_10GELAN;
            break;
        case VTSS_CONFIG_MODE_TFF:
        case VTSS_CONFIG_MODE_TFO_PP2F:
            *block_state = PROTO_10GFC;
            break;
        case VTSS_CONFIG_MODE_TSS:
            *block_state = PROTO_SONET;
            break;
        case VTSS_CONFIG_MODE_TOO_2F2:
            *block_state = PROTO_OTU2F;
            break;
        case VTSS_CONFIG_MODE_TEE_W:
            *block_state = PROTO_10GEWAN;
            break;
        default:
            T_E("dd_get_block_status: Unsupported Mode %d for block %d", mode, block);
            return VTSS_RC_ERROR;
        }
    }

    if (block == BLOCK_SI5338_2) 
    { 
        switch (mode) {
        case VTSS_CONFIG_MODE_PEE_W:
        case VTSS_CONFIG_MODE_PEE_MW: 
        case VTSS_CONFIG_MODE_TEE_W:
        case VTSS_CONFIG_MODE_TEE_PMW:
            *block_state = PROTO_10GEWAN;
            break;
        case VTSS_CONFIG_MODE_PEE_P:
        case VTSS_CONFIG_MODE_PEE_MP: 
        case VTSS_CONFIG_MODE_PEO_P1E_100:
        case VTSS_CONFIG_MODE_PEO_P2E_100:
            *block_state = PROTO_XAUI_10GE;
            break;
        case VTSS_CONFIG_MODE_PEO_WA:
        case VTSS_CONFIG_MODE_PEO_MWA:
        case VTSS_CONFIG_MODE_PEO_WS:
        case VTSS_CONFIG_MODE_PEO_MWS:
        case VTSS_CONFIG_MODE_PGO_2:
        case VTSS_CONFIG_MODE_PGO_2SUB:
        case VTSS_CONFIG_MODE_TGO_2:
        case VTSS_CONFIG_MODE_TGO_2SUB:
        case VTSS_CONFIG_MODE_TGO_2SUBSMALL:
        case VTSS_CONFIG_MODE_TOO_2:
            *block_state = PROTO_OTU2;
            break;
        case VTSS_CONFIG_MODE_PEO_P2E_20:
        case VTSS_CONFIG_MODE_PEO_MP2E_20:
        case VTSS_CONFIG_MODE_TOO_2E2:
        case VTSS_CONFIG_MODE_TEO_PMP_2E:
        case VTSS_CONFIG_MODE_TEO_PP_2E:
            *block_state = PROTO_OTU2E;
            break;
        case VTSS_CONFIG_MODE_BOO_2:
        case VTSS_CONFIG_MODE_BMX_2:
            *block_state = PROTO_OTU2_SFI42;
            break;
        case VTSS_CONFIG_MODE_TEE_L:
        case VTSS_CONFIG_MODE_TEE_PMP_S:
        case VTSS_CONFIG_MODE_TEE_PMP_A:
        case VTSS_CONFIG_MODE_TEE_PP:
        case VTSS_CONFIG_MODE_TEOC_2E:
        case VTSS_CONFIG_MODE_TEOC_1E:
            *block_state = PROTO_10GELAN;
            break;
        case VTSS_CONFIG_MODE_TFF:
            *block_state = PROTO_10GFC;
            break;
        case VTSS_CONFIG_MODE_TSS:
        case VTSS_CONFIG_MODE_TSOC_S:
        case VTSS_CONFIG_MODE_TSOC_WS:
            *block_state = PROTO_SONET;
            break;
        case VTSS_CONFIG_MODE_TOO_2F2:
        case VTSS_CONFIG_MODE_TFO_PP2F:
            *block_state = PROTO_OTU2F;
            break;
        case VTSS_CONFIG_MODE_TEO_PMP_1E:
            *block_state = PROTO_OTU1E;
            break;
        default:
            T_E("dd_get_block_status: Unsupported Mode %d for block %d", mode, block);
            return VTSS_RC_ERROR;
        }
    }
    return VTSS_RC_OK;
}

vtss_rc d_init_channel_clock_sw(int channel, vtss_config_mode_t mode, u32 xfi_clk, u32 x4_case)
{
    int target;
    int portoffset;
    int block_status;

    if(mode > VTSS_CONFIG_MODE_MAX){
        return VTSS_RC_OK;
    }

    printf("Daytona channel %d configuration mode =: %s(%d)\n", channel, config_txt[mode],mode);

    T_N("(d_init_channel) Beginning Channel Init, Channel: %d, Mode: %d", channel, mode);

    /* TODO d_init_channel_default(channel, mode); */

    if((channel == 0) || (channel == 1)) {
        target = SI5338_TGTID_CHAN_0_1;
    }
    else if ((channel == 2) || (channel == 3)) {
        target = SI5338_TGTID_CHAN_2_3;
    } else {
        T_E("(d_init_channel_clock_sw)Invalid target\n");
        return VTSS_RC_ERROR;
    }

    portoffset = (channel % 2) * 2; /* si5338 ports 0-Line0 1-Client0 2-Line1 3-Client1 */

    /* do reference on 53338 first */

    VTSS_RC(dd_get_block_status(BLOCK_SI5338_2, mode, &block_status, x4_case));
    if ((mode == VTSS_CONFIG_MODE_PEE_P || mode == VTSS_CONFIG_MODE_PEE_MP)&& (xfi_clk == 161))
      block_status = PROTO_10GELAN;
    if (db_init_si5338_chan(target, portoffset, block_status) != VTSS_RC_OK) { /* line */
        T_E("db_init_si5338_chan failed for target %d and protocol %d", target, block_status);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(dd_get_block_status(BLOCK_SI5338_1, mode, &block_status, x4_case));
    if (db_init_si5338_chan(target, portoffset+1, block_status) != VTSS_RC_OK) { /* client */
        T_E("db_init_si5338_chan failed for target %d and protocol %d", target, block_status);
        return VTSS_RC_ERROR;
    }
    db_init_clk_mux(channel, 1, mode); /* client */
    db_init_clk_mux(channel, 2, mode); /* line */

    VTSS_RC(dd_get_block_status(BLOCK_SI5326_2, mode, &block_status, x4_case));
    if (db_init_si5326_chan((SI5326_CLK_GEN_BASE+(channel*2)), block_status) != VTSS_RC_OK) { /* line */
        T_E("db_init_si5326_chan failed for target %d block_status %d", (SI5326_CLK_GEN_BASE+(channel*2)), block_status);
        return VTSS_RC_ERROR;
    }
    VTSS_RC(dd_get_block_status(BLOCK_SI5326_1, mode, &block_status, x4_case));
    if (db_init_si5326_chan((SI5326_CLK_GEN_BASE+(channel*2)+1), block_status) != VTSS_RC_OK) { /* client */
        T_E("db_init_si5326_chan failed for target %d block_status %d", (SI5326_CLK_GEN_BASE+(channel*2)), block_status);
        return VTSS_RC_ERROR;
    }

    T_D("(d_init_channel) Completed Channel Init, Channel: %d, Mode: %d", channel, mode);

    return VTSS_RC_OK;
}

/* Function to program the electrical switch to interchange two points on the board. */
static vtss_rc db_setup_switch(int conn1 , int conn2)
{
    int conn1_in_base = 0, conn1_out_base = 0;
    int conn2_in_base = 0, conn2_out_base = 0;
    int mode, i;
    u16 data;
    int static connections[IF_MAX][2]={{1,16},{3,14},{0,0},{2,2},
                                   {11,17},{13,15},{10,3},{12,5},
                                   {17,11},{15,13},{14,7},{16,9},
                                   {9, 1},{4, 4},{6, 6},{8,8},
                                   {7,12},{5,10}};

    if ((conn1 < IF_UPI0) || (conn1 > IF_MAX-1)) {
      T_E("Invalid interface for conn1: %d", conn1);
    }
    else {
      conn1_in_base = connections[conn1][0]; 
      conn1_out_base = connections[conn1][1];
    }

    if (((conn2) < IF_UPI0) || (conn2 > IF_MAX-1)){
      T_E("Invalid interface for conn2: %d", conn2);
    }
    else {
      conn2_in_base = connections[conn2][0];
      conn2_out_base = connections[conn2][1];
    }

    /*   check which mode the device is in (striping or not) */

    /*   set mode [erf_rd_quick -inst:$dut(inst_vsc3172) vsc3172 SWITCH_CONFIGURATION STRIPING_MODE_SELECT]; */

    VTSS_RC(vsc3172_read(0x281, &data)); /* SWITCH_CONFIGURATION */

    mode = ((data >> 5) & 1);

    if (mode == 1) {
        T_E("(db_setup_switch) Currently unsupported mode");
        VTSS_RC(vsc3172_write((conn1_in_base << 2), (conn2_out_base << 2),0x7ff));
        VTSS_RC(vsc3172_write((conn2_in_base << 2), (conn1_out_base << 2),0x7ff));
    } else if (mode == 0) {
        /*
        Normal Mode, need to program each lane independantly
        Need to re-calculate the base port based on the striping Lane
        if even, just multiply by 4
        if odd, subtract 1, multiply by 4, then add 1 back
        */

        if ((conn1_in_base % 2) == 0){
            conn1_in_base = (conn1_in_base * 4);
        } else {
            conn1_in_base = (((conn1_in_base - 1) * 4) + 1);
        }

        if ((conn2_in_base % 2) == 0){
            conn2_in_base = (conn2_in_base * 4);
        } else {
            conn2_in_base = (((conn2_in_base - 1) * 4) + 1);
        }

        if ((conn1_out_base % 2) == 0){
            conn1_out_base = (conn1_out_base * 4);
        } else {
            conn1_out_base = (((conn1_out_base - 1) * 4) + 1);
        }

        if ((conn2_out_base % 2) == 0){
            conn2_out_base = (conn2_out_base * 4);
        } else {
            conn2_out_base = (((conn2_out_base - 1) * 4) + 1);
        }
        for (i = 0; i < 4; i++ ) {
            VTSS_RC(vsc3172_write((conn2_out_base + (i * 2)), (conn1_in_base+ (i * 2)), 0x7ff));
            VTSS_RC(vsc3172_write((conn1_out_base + (i * 2)), (conn2_in_base+ (i * 2)), 0x7ff));
        }
    }
    return VTSS_RC_OK;
}

/* proc d_init_board_conn_sw {dutif boardif client {switch_loop 0}} */

vtss_rc d_init_board_conn_sw(int chan, int dutif, int boardif, int client_proto, int swith_loop)
{
    u32 data;
    if ((boardif == IF_XAUI0) || (boardif == IF_XAUI1) ||
        (boardif == IF_XAUI2) || (boardif == IF_XAUI3)) {
        db_setup_switch(dutif, boardif);
        /*
         8486 programming
         put device in reset to begin
        */
        VTSS_RC(lbfpga_write((0x48 + (chan * 4)), 1, 0x1));
        /* check status of XFP */
        VTSS_RC(lbfpga_read((0x58 + (chan * 4)), &data));

        if (data == 1){
            T_D("(d_init_board_conn) No XFP installed next to VSC8486 %d chan, but %d boardif is selected", chan, boardif);
        } 
        /* setup clock (1 for 10GFC and 0 for 10GE) */
        if (client_proto == PROTO_10GFC) {
            VTSS_RC(lbfpga_write((0x48 + (chan * 4)), (1 << 1), 0x6));
        } else {
            VTSS_RC(lbfpga_write((0x48 + (chan * 4)), (0 << 1), 0x6));
        }
        VTSS_RC(lbfpga_write((0x48 + (chan * 4)), 0, 0x1));
    } /* boardif XAUI */
    return VTSS_RC_OK;
}

vtss_rc daytona_eval_init(int inst, vtss_ev_conf_t ev_conf0, vtss_ev_conf_t ev_conf1)
{
    /* Initialize board */
#ifdef BOARD_TALLADEGA
    if (inst == 0)
      if (d_init_board(EVAL_BOARD_TALLADEGA) != VTSS_RC_OK) {
#else
    if (d_init_board(EVAL_BOARD_DAYTONA) != VTSS_RC_OK) {
#endif
        T_E("d_init_board returned Failure");
        return VTSS_RC_ERROR;
    }

    /* Initialize device */
#ifdef BOARD_TALLADEGA
    if (inst == 0)
      if (di_sw(EVAL_BOARD_TALLADEGA, DEVICE_TALLADEGA) != VTSS_RC_OK) {
#else
    if (di_sw(EVAL_BOARD_DAYTONA, DEVICE_DAYTONA) != VTSS_RC_OK) {
#endif
        T_E("di_sw returned Failure");
        return VTSS_RC_ERROR;
    }

    if (inst == 0 && ev_conf0.mode != VTSS_CONFIG_MODE_DISABLED)
      if (d_init_channel_clock_sw(0, ev_conf0.mode, ev_conf0.xfi_clk, ev_conf0.x4_case) != VTSS_RC_OK) {
        T_E("d_init_channel_clock_sw returned Failure for channel 0");
        return VTSS_RC_ERROR;
      }
    if (inst == 0 && ev_conf1.mode != VTSS_CONFIG_MODE_DISABLED)
      if (d_init_channel_clock_sw(1, ev_conf1.mode, ev_conf1.xfi_clk, ev_conf1.x4_case) != VTSS_RC_OK) {
        T_E("d_init_channel_clock_sw returned Failure for channel 1");
        return VTSS_RC_ERROR;
      }

#ifdef BOARD_TALLADEGA
    if (inst == 1 && ev_conf0.mode != VTSS_CONFIG_MODE_DISABLED)
      if (d_init_channel_clock_sw(2, ev_conf0.mode, ev_conf0.xfi_clk, ev_conf0.x4_case) != VTSS_RC_OK) {
        T_E("d_init_channel_clock_sw returned Failure for channel 2");
        return VTSS_RC_ERROR;
      }

    if (inst == 1 && ev_conf1.mode != VTSS_CONFIG_MODE_DISABLED)
      if (d_init_channel_clock_sw(3, ev_conf1.mode, ev_conf1.xfi_clk, ev_conf1.x4_case) != VTSS_RC_OK) {
        T_E("d_init_channel_clock_sw returned Failure for channel 3");
        return VTSS_RC_ERROR;
      }
#endif

#ifdef BOARD_TALLADEGA
    if (inst == 1) {
#endif
      if (d_init_board_conn_sw(0, IF_UPI0, IF_XAUI0, PROTO_10GELAN, 0) != VTSS_RC_OK) {
        T_E("d_init_board_conn_sw returned Failure for channel 0");
        return VTSS_RC_ERROR;
      }
      if (d_init_board_conn_sw(1, IF_UPI1, IF_XAUI1, PROTO_10GELAN, 1) != VTSS_RC_OK) {
        T_E("d_init_board_conn_sw returned Failure for channel 1");
        return VTSS_RC_ERROR;
      }
#ifdef BOARD_TALLADEGA
    }
#endif

#ifdef BOARD_TALLADEGA
    if (inst == 1) {
      if (d_init_board_conn_sw(2, IF_UPI2, IF_XAUI2, PROTO_10GELAN, 0) != VTSS_RC_OK) {
        T_E("d_init_board_conn_sw returned Failure for channel 0");
        return VTSS_RC_ERROR;
      }
      if (d_init_board_conn_sw(3, IF_UPI3, IF_XAUI3, PROTO_10GELAN, 1) != VTSS_RC_OK) {
        T_E("d_init_board_conn_sw returned Failure for channel 1");
        return VTSS_RC_ERROR;
      }
    }
#endif
    return VTSS_RC_OK;
}
#endif /* DAYTONA_EVAL_INIT */
#ifdef DAYTONA_CHIP_SIMULATE
/**
 * \brief Daytona register interface simulation
 *
 * \details
 * reg_bank_xxx: array of u32 that holds a mirror of the registers for one target component
 *          The init values should reflect the default values for the chip registers
 */

static u32    reg_bank_CFG_IF_ORG [0x0e] = {0};
static u32    reg_bank_CFG_IF_FAST_REGS [0x1f] =    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,1, 0x8492, 0x74, 0x1, 0,0, 1
                                                    };
static u32    reg_bank_CFG_IF_GCB [0x118] = {0,0,0,0, 0x18492075};
static u32    reg_bank_UPI_0       [0x2aa0] = {0};
static u32    reg_bank_LXFI_0      [0x1709] = {0};
static u32    reg_bank_CXFI_0      [0x1709] = {0};
static u32    reg_bank_UPI_1       [0x2aa0] = {0};
static u32    reg_bank_LXFI_1      [0x1709] = {0};
static u32    reg_bank_CXFI_1      [0x1709] = {0};
static u32    reg_bank_TFI_0       [0x14] = {0};
static u32    reg_bank_SFI_0       [0x07] = {0};
static u32    reg_bank_XAUI_0      [0x74] = {0};
static u32    reg_bank_OTU_1_0     [0xfb7] = {0};
static u32    reg_bank_WIS_1_0     [0x1c1] = {0};
static u32    reg_bank_PCS_1_0     [0x2b] = {0};
static u32    reg_bank_GFP_1_0     [0x800] = {0};
static u32    reg_bank_MAC_1_0     [0x140] = {0};
static u32    reg_bank_RAB_0       [0x611] = {0};
static u32    reg_bank_MAC_2_0     [0x140] = {0};
static u32    reg_bank_GFP_2_0     [0x800] = {0};
static u32    reg_bank_PCS_2_0     [0x2b] = {0};
static u32    reg_bank_WIS_2_0     [0x1c1] = {0};
static u32    reg_bank_OTU_2_0     [0xfb7] = {0};
static u32    reg_bank_EFEC_0      [0x29] = {0};
static u32    reg_bank_TFI_1       [0x14] = {0};
static u32    reg_bank_SFI_1       [0x07] = {0};
static u32    reg_bank_XAUI_1      [0x74] = {0};
static u32    reg_bank_OTU_1_1     [0xfb7] = {0};
static u32    reg_bank_WIS_1_1     [0x1c1] = {0};
static u32    reg_bank_PCS_1_1     [0x2b] = {0};
static u32    reg_bank_GFP_1_1     [0x800] = {0};
static u32    reg_bank_MAC_1_1     [0x140] = {0};
static u32    reg_bank_RAB_1       [0x611] = {0};
static u32    reg_bank_MAC_2_1     [0x140] = {0};
static u32    reg_bank_GFP_2_1     [0x800] = {0};
static u32    reg_bank_PCS_2_1     [0x2b] = {0};
static u32    reg_bank_WIS_2_1     [0x1c1] = {0};
static u32    reg_bank_OTU_2_1     [0xfb7] = {0};
static u32    reg_bank_EFEC_1      [0x29] = {0};

/**
 * \brief Daytona register interface simulation
 *
 * \details
 * bank: pointer to register bank for target [x]
 * size: sixe of tha register bank in number of 32 bit registers.
 */

typedef struct daytona_reg_bank_t {
    u32 tgt;
    u32 *bank;
    u32 size;
} daytona_reg_bank_t;

static daytona_reg_bank_t daytona_reg_bank [] = {
    {VTSS_TO_CFG_IF_ORG, reg_bank_CFG_IF_ORG, sizeof(reg_bank_CFG_IF_ORG)/sizeof(u32) },
    {VTSS_TO_CFG_IF_FAST_REGS, reg_bank_CFG_IF_FAST_REGS, sizeof(reg_bank_CFG_IF_FAST_REGS)/sizeof(u32) },
    {VTSS_TO_CFG_IF_GCB, reg_bank_CFG_IF_GCB, sizeof(reg_bank_CFG_IF_GCB)/sizeof(u32) },
    {VTSS_TO_UPI_0,   reg_bank_UPI_0      , sizeof(reg_bank_UPI_0      )/sizeof(u32) },
    {VTSS_TO_LXFI_0,  reg_bank_LXFI_0     , sizeof(reg_bank_LXFI_0     )/sizeof(u32) },
    {VTSS_TO_CXFI_0,  reg_bank_CXFI_0     , sizeof(reg_bank_CXFI_0     )/sizeof(u32) },
    {VTSS_TO_UPI_1,   reg_bank_UPI_1      , sizeof(reg_bank_UPI_1      )/sizeof(u32) },
    {VTSS_TO_LXFI_1,  reg_bank_LXFI_1     , sizeof(reg_bank_LXFI_1     )/sizeof(u32) },
    {VTSS_TO_CXFI_1,  reg_bank_CXFI_1     , sizeof(reg_bank_CXFI_1     )/sizeof(u32) },
    {VTSS_TO_TFI_0,   reg_bank_TFI_0      , sizeof(reg_bank_TFI_0      )/sizeof(u32) },
    {VTSS_TO_SFI_0,   reg_bank_SFI_0      , sizeof(reg_bank_SFI_0      )/sizeof(u32) },
    {VTSS_TO_XAUI_0,  reg_bank_XAUI_0     , sizeof(reg_bank_XAUI_0     )/sizeof(u32) },
    {VTSS_TO_OTU_1_0, reg_bank_OTU_1_0    , sizeof(reg_bank_OTU_1_0    )/sizeof(u32) },
    {VTSS_TO_WIS_1_0, reg_bank_WIS_1_0    , sizeof(reg_bank_WIS_1_0    )/sizeof(u32) },
    {VTSS_TO_PCS_1_0, reg_bank_PCS_1_0    , sizeof(reg_bank_PCS_1_0    )/sizeof(u32) },
    {VTSS_TO_GFP_1_0, reg_bank_GFP_1_0    , sizeof(reg_bank_GFP_1_0    )/sizeof(u32) },
    {VTSS_TO_MAC_1_0, reg_bank_MAC_1_0    , sizeof(reg_bank_MAC_1_0    )/sizeof(u32) },
    {VTSS_TO_RAB_0,   reg_bank_RAB_0      , sizeof(reg_bank_RAB_0      )/sizeof(u32) },
    {VTSS_TO_MAC_2_0, reg_bank_MAC_2_0    , sizeof(reg_bank_MAC_2_0    )/sizeof(u32) },
    {VTSS_TO_GFP_2_0, reg_bank_GFP_2_0    , sizeof(reg_bank_GFP_2_0    )/sizeof(u32) },
    {VTSS_TO_PCS_2_0, reg_bank_PCS_2_0    , sizeof(reg_bank_PCS_2_0    )/sizeof(u32) },
    {VTSS_TO_WIS_2_0, reg_bank_WIS_2_0    , sizeof(reg_bank_WIS_2_0    )/sizeof(u32) },
    {VTSS_TO_OTU_2_0, reg_bank_OTU_2_0    , sizeof(reg_bank_OTU_2_0    )/sizeof(u32) },
    {VTSS_TO_EFEC_0,  reg_bank_EFEC_0     , sizeof(reg_bank_EFEC_0     )/sizeof(u32) },
    {VTSS_TO_TFI_1,   reg_bank_TFI_1      , sizeof(reg_bank_TFI_1      )/sizeof(u32) },
    {VTSS_TO_SFI_1,   reg_bank_SFI_1      , sizeof(reg_bank_SFI_1      )/sizeof(u32) },
    {VTSS_TO_XAUI_1,  reg_bank_XAUI_1     , sizeof(reg_bank_XAUI_1     )/sizeof(u32) },
    {VTSS_TO_OTU_1_1, reg_bank_OTU_1_1    , sizeof(reg_bank_OTU_1_1    )/sizeof(u32) },
    {VTSS_TO_WIS_1_1, reg_bank_WIS_1_1    , sizeof(reg_bank_WIS_1_1    )/sizeof(u32) },
    {VTSS_TO_PCS_1_1, reg_bank_PCS_1_1    , sizeof(reg_bank_PCS_1_1    )/sizeof(u32) },
    {VTSS_TO_GFP_1_1, reg_bank_GFP_1_1    , sizeof(reg_bank_GFP_1_1    )/sizeof(u32) },
    {VTSS_TO_MAC_1_1, reg_bank_MAC_1_1    , sizeof(reg_bank_MAC_1_1    )/sizeof(u32) },
    {VTSS_TO_RAB_1,   reg_bank_RAB_1      , sizeof(reg_bank_RAB_1      )/sizeof(u32) },
    {VTSS_TO_MAC_2_1, reg_bank_MAC_2_1    , sizeof(reg_bank_MAC_2_1    )/sizeof(u32) },
    {VTSS_TO_GFP_2_1, reg_bank_GFP_2_1    , sizeof(reg_bank_GFP_2_1    )/sizeof(u32) },
    {VTSS_TO_PCS_2_1, reg_bank_PCS_2_1    , sizeof(reg_bank_PCS_2_1    )/sizeof(u32) },
    {VTSS_TO_WIS_2_1, reg_bank_WIS_2_1    , sizeof(reg_bank_WIS_2_1    )/sizeof(u32) },
    {VTSS_TO_OTU_2_1, reg_bank_OTU_2_1    , sizeof(reg_bank_OTU_2_1    )/sizeof(u32) },
    {VTSS_TO_EFEC_1,  reg_bank_EFEC_1     , sizeof(reg_bank_EFEC_1     )/sizeof(u32) },
};
/* Execute Daytona simulator  read/write operations */
static vtss_rc simulated_reg_read(const vtss_chip_no_t chip_no,
                                  const u32            addr,
                                  u32                  *const value)
{
    u32 tgt_ix;
    u32 tgt = (addr) & 0x3f0000;
    u32 die = (addr >> 22) &0x01;
    u32 reg = (addr >> 2) & 0x3fff;
    BOOL valid = FALSE;
    *value = 0;

    if ((tgt == VTSS_TO_CFG_IF_FAST_REGS) || (tgt == VTSS_TO_CFG_IF_ORG)) {
        reg = (addr >> 1) & 0x3fff;
    }

    for (tgt_ix = 0; (valid == FALSE) && (tgt_ix < (sizeof(daytona_reg_bank)/sizeof(daytona_reg_bank_t))); tgt_ix++) {
        if (tgt == daytona_reg_bank[tgt_ix].tgt) valid = TRUE;
    }
    --tgt_ix;
    T_I("Tgt = %x, tgt_ix = %X, die = %d, valid = %d", tgt, tgt_ix, die, valid);
    if (valid) {
        if (reg < daytona_reg_bank[tgt_ix].size) {
            T_I("Tgt = %X, Reg = %X, max = %d", tgt_ix, reg, daytona_reg_bank[tgt_ix].size);
            *value = daytona_reg_bank[tgt_ix].bank[reg];

            if (tgt != (VTSS_TO_CFG_IF_FAST_REGS) && tgt != (VTSS_TO_CFG_IF_ORG)) {
                /* indirect register */
                reg_bank_CFG_IF_FAST_REGS[0] = (*value >> 16) & 0xffff;
                reg_bank_CFG_IF_FAST_REGS[1] = *value & 0xffff;
            }
            if ((tgt == VTSS_TO_CFG_IF_FAST_REGS) || (tgt == VTSS_TO_CFG_IF_ORG)) {
                T_I("Read chip %d, register adr %8X, returns %8X", chip_no, (tgt << 16)|(reg << 2), *value);
            } else {
                T_I("Read chip %d, register adr %8X, returns %8X", chip_no, addr, *value);
            }
            return VTSS_RC_OK;
        } else {
            T_E("Invalid register no %X, tgt %X", reg, tgt);
            return VTSS_RC_ERROR;
        }
    } else {
        T_E("Invalid target id %X", tgt);
        return VTSS_RC_ERROR;
    }
}

static vtss_rc simulated_reg_write(const vtss_chip_no_t chip_no,
                                   const u32            addr,
                                   const u32            value)
{
    u32 tgt = (addr) & 0x3f0000;
    u32 die = (addr >> 22) &0x01;
    u32 reg = (addr >> 2) & 0x3fff;
    BOOL odd_adr = ((addr & 0x2) != 0);
    BOOL valid = FALSE;
    u32 tgt_ix;

    if ((tgt == VTSS_TO_CFG_IF_FAST_REGS) || (tgt == VTSS_TO_CFG_IF_ORG)) {
        reg = (addr >> 1) & 0x3fff;
        odd_adr = FALSE; /* direct registers are always even */
    }
    for (tgt_ix = 0; (valid == FALSE) && (tgt_ix < (sizeof(daytona_reg_bank)/sizeof(daytona_reg_bank_t))); tgt_ix++) {
        if (tgt == daytona_reg_bank[tgt_ix].tgt) valid = TRUE;
    }
    --tgt_ix;
    T_I("Tgt = %x, tgt_ix = %X, die = %d, valid = %d", tgt, tgt_ix, die, valid);
    if (valid) {
        if (reg < daytona_reg_bank[tgt_ix].size) {
            if (odd_adr) {
                daytona_reg_bank[tgt_ix].bank[reg] = ((daytona_reg_bank[tgt_ix].bank[reg] << 16) | value);
            } else {
                daytona_reg_bank[tgt_ix].bank[reg] = value;
            }
            if ((tgt == VTSS_TO_CFG_IF_FAST_REGS) || (tgt == VTSS_TO_CFG_IF_ORG)) {
                T_I("Write chip %d, register adr %8X, returns %8X", chip_no, (tgt << 16) | (reg << 2), value);
            } else {
                T_I("Write chip %d, register adr %8X, returns %8X", chip_no, addr, value);
            }
            return VTSS_RC_OK;
        } else {
            T_E("Invalid register no %X, tgt %X", reg, tgt);
            return VTSS_RC_ERROR;
        }
    } else {
        T_E("Invalid target no %X", tgt);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}
#else

/**
 * \brief Read value from target register using parallel interface
 *
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to read. Byte offset from chip address base.
 * \param value [OUT]   Register value.
 *
 * \return Return code.
 **/
static vtss_rc pi_reg_read(const vtss_chip_no_t chip_no,
                           const u32            addr,
                           u32                  *const value)
{
    struct vitgenio_16bit_readwrite pibuf;
    vtss_rc                         rc = VTSS_RC_OK;

    *value=0;
    pibuf.value = 0;
    pibuf.offset = addr >> 1; /* TBD: ">>" used to see correct addresses on Genie PI address bus */
                                       
    if (ioctl(pi_fd,VITGENIO_16BIT_READ,&pibuf) < 0) {
        T_E ("PI Read operation failed: %u\n", *value);
        rc = VTSS_RC_ERROR;
    } else {
        *value = pibuf.value;
#if defined(BOARD_TALLADEGA)
        /* Swap the data bit 2 & 3 */
        *value = (*value & ~(0xc)) | ((*value & 4) << 1) | ((*value & 8) >> 1);
#endif /* VTSS_CHIP_TALLADEGA */
    }

    return rc;
}

/**
 * \brief Write value to target register using parallel interface.
 *
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to write. Byte offset from chip address base.
 * \param value [IN]    Register value.
 *
 * \return Return code.
 **/
static vtss_rc pi_reg_write(const vtss_chip_no_t chip_no,
                            const u32            addr,
                            const u32            value)
{
    struct vitgenio_16bit_readwrite pibuf;
    vtss_rc                         rc = VTSS_RC_OK;


    pibuf.value = value;
    pibuf.offset = addr >> 1; /* TBD: ">>" used to see correct addresses on Genie PI address bus */ 
#if defined(BOARD_TALLADEGA)
    /* Swap the 2 & 3 bits */
    pibuf.value = (pibuf.value & ~(0xc)) | ((pibuf.value & 4) << 1) | ((pibuf.value & 8) >> 1);
#endif /* VTSS_CHIP_TALLADEGA */

    if (ioctl(pi_fd,VITGENIO_16BIT_WRITE,&pibuf) < 0) {
        T_E ("PI Write operation failed: %u\n", value);
        rc = VTSS_RC_ERROR;
    }

    return rc;
}
#endif /* DAYTONA_CHIP_SIMULATE */

/**
 * \brief Read value from target register.
 *
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to read. Byte offset from chip address base.
 * \param value [OUT]   Register value.
 *
 * \return Return code.
 **/
static vtss_rc reg_read(const vtss_chip_no_t chip_no,
                        const u32            addr,
                        u32                  *const value)
{

#ifdef DAYTONA_CHIP_SIMULATE
    return simulated_reg_read(chip_no, addr, value);
#else
    return pi_reg_read(chip_no, addr, value);
#endif /* DAYTONA_CHIP_SIMULATE */

}

/**
 * \brief Read value from target register (channel 2 and 3).
 *
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to read. Byte offset from chip address base.
 * \param value [OUT]   Register value.
 *
 * \return Return code.
 **/
static vtss_rc reg_read_ch2_3(const vtss_chip_no_t chip_no,
                        const u32            addr,
                        u32                  *const value)
{
    return reg_read(chip_no, addr |0x400000, value);
}

/**
 * \brief Write value to target register.
 *
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to write. Byte offset from chip address base.
 * \param value [IN]    Register value.
 *
 * \return Return code.
 **/
static vtss_rc reg_write(const vtss_chip_no_t chip_no,
                         const u32            addr,
                         const u32            value)
{

#ifdef DAYTONA_CHIP_SIMULATE
    return simulated_reg_write(chip_no, addr, value);
#else
    return pi_reg_write(chip_no, addr, value);
#endif /* DAYTONA_CHIP_SIMULATE */

}

/**
 * \brief Write value to target register channel 2 and 3).
 *
 * \param chip_no [IN]  Chip number (if multi-chip instance).
 * \param addr [IN]     Address to write. Byte offset from chip address base.
 * \param value [IN]    Register value.
 *
 * \return Return code.
 **/
static vtss_rc reg_write_ch2_3(const vtss_chip_no_t chip_no,
                               const u32            addr,
                               const u32            value)
{
    return reg_write(chip_no, addr | 0x400000, value);
}

static vtss_port_interface_t port_interface(vtss_port_no_t port_no)
{
    /*Not used in Daytona but the test application expects a valid value */
    return VTSS_PORT_INTERFACE_XAUI;
}

#endif

#if defined(VTSS_ARCH_DAYTONA)

static BOOL event_screen = FALSE;

static void event_window(BOOL enable)
{
    static u8 cur_save[] = {27, '7', '\0'};
    static u8 cur_restore[] = {27, '8', '\0'};
    static u8 cur_event[] = {27, '[', '7', '6', ';', '1', 'H', '\0'};
    static u8 command_win[] = {27, '[', '1', ';', '5', '5', 'r', '\0'};
    static u8 event_win[] = {27, '[', '5', '7', ';', '7', '6', 'r', '\0'};
    
    if (event_screen) {
        if (enable) {
            printf("%s", cur_save);      /* Save cursor position */
            printf("%s", event_win);     /* Define event window */
            printf("%s", cur_event);     /* Move cursor to event window */
        }
        else {
            printf("%s", command_win);    /* Define command window */
            printf("%s", cur_restore);    /* Restore cursor position */
        }
    }
}

static void print_upi_event(vtss_port_no_t port, vtss_upi_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_UPI_RX_HSS_PRBSSYNC_0_EV:   printf("UPI  Port %u  Event %s\n", port+1, "RX_HSS_PRBSSYNC_0");  break;
        case VTSS_UPI_RX_HSS_PRBSSYNC_1_EV:   printf("UPI  Port %u  Event %s\n", port+1, "RX_HSS_PRBSSYNC_1");  break;
        case VTSS_UPI_RX_HSS_PRBSSYNC_2_EV:   printf("UPI  Port %u  Event %s\n", port+1, "RX_HSS_PRBSSYNC_2");  break;
        case VTSS_UPI_RX_HSS_PRBSSYNC_3_EV:   printf("UPI  Port %u  Event %s\n", port+1, "RX_HSS_PRBSSYNC_3");  break;
        case VTSS_UPI_TX_HSS_PRBSSYNC_0_EV:   printf("UPI  Port %u  Event %s\n", port+1, "TX_HSS_PRBSSYNC_0");  break;
        case VTSS_UPI_TX_HSS_PRBSSYNC_1_EV:   printf("UPI  Port %u  Event %s\n", port+1, "TX_HSS_PRBSSYNC_1");  break;
        case VTSS_UPI_TX_HSS_PRBSSYNC_2_EV:   printf("UPI  Port %u  Event %s\n", port+1, "TX_HSS_PRBSSYNC_2");  break;
        case VTSS_UPI_TX_HSS_PRBSSYNC_3_EV:   printf("UPI  Port %u  Event %s\n", port+1, "TX_HSS_PRBSSYNC_3");  break;
        case VTSS_UPI_HSS_RESETOUT_EV:        printf("UPI  Port %u  Event %s\n", port+1, "HSS_RESETOUT");  break;
        case VTSS_UPI_RX_LOS_EV:              printf("UPI  Port %u  Event %s\n", port+1, "RX_LOS");  break;
        case VTSS_UPI_RX_SIGDET_0_EV:         printf("UPI  Port %u  Event %s\n", port+1, "RX_SIGDET_0");  break;
        case VTSS_UPI_RX_SIGDET_1_EV:         printf("UPI  Port %u  Event %s\n", port+1, "RX_SIGDET_1");  break;
        case VTSS_UPI_RX_SIGDET_2_EV:         printf("UPI  Port %u  Event %s\n", port+1, "RX_SIGDET_2");  break;
        case VTSS_UPI_RX_SIGDET_3_EV:         printf("UPI  Port %u  Event %s\n", port+1, "RX_SIGDET_3");  break;
        case VTSS_UPI_TX_FIFO_OVERFLOW_EV:    printf("UPI  Port %u  Event %s\n", port+1, "TX_FIFO_OVERFLOW");  break;
        case VTSS_UPI_TX_FIFO_UNDERFLOW_EV:   printf("UPI  Port %u  Event %s\n", port+1, "TX_FIFO_UNDERFLOW");  break;
        case VTSS_UPI_HSS_PRTREADY_EV:        printf("UPI  Port %u  Event %s\n", port+1, "HSS_PRTREADY");  break;
        case VTSS_UPI_RX_PRBS_ERR_0_EV:       printf("UPI  Port %u  Event %s\n", port+1, "RX_PRBS_ERR_0");  break;
        case VTSS_UPI_RX_PRBS_ERR_1_EV:       printf("UPI  Port %u  Event %s\n", port+1, "RX_PRBS_ERR_1");  break;
        case VTSS_UPI_RX_PRBS_ERR_2_EV:       printf("UPI  Port %u  Event %s\n", port+1, "RX_PRBS_ERR_2");  break;
        case VTSS_UPI_RX_PRBS_ERR_3_EV:       printf("UPI  Port %u  Event %s\n", port+1, "RX_PRBS_ERR_3");  break;
        case VTSS_UPI_TX_PRBS_ERR_0_EV:       printf("UPI  Port %u  Event %s\n", port+1, "TX_PRBS_ERR_0");  break;
        case VTSS_UPI_TX_PRBS_ERR_1_EV:       printf("UPI  Port %u  Event %s\n", port+1, "TX_PRBS_ERR_1");  break;
        case VTSS_UPI_TX_PRBS_ERR_2_EV:       printf("UPI  Port %u  Event %s\n", port+1, "TX_PRBS_ERR_2");  break;
        case VTSS_UPI_TX_PRBS_ERR_3_EV:       printf("UPI  Port %u  Event %s\n", port+1, "TX_PRBS_ERR_3");  break;
        case VTSS_UPI_SSF_EV:                 printf("UPI  Port %u  Event %s\n", port+1, "SSF");  break;
        case VTSS_UPI_PLLLOCK_EV:             printf("UPI  Port %u  Event %s\n", port+1, "PLLLOCK");  break;
        case VTSS_UPI_EYE_QUALITY_ERR_EV:     printf("UPI  Port %u  Event %s\n", port+1, "EYE_QUALITY_ERR");  break;

        case VTSS_UPI_OTN_FAS_POS_EV:         printf("UPI  Port %u  Event %s\n", port+1, "FAS_POS");  break;
        case VTSS_UPI_OTN_LOM_EV:             printf("UPI  Port %u  Event %s\n", port+1, "LOM");  break;
        case VTSS_UPI_OTN_OOM_EV:             printf("UPI  Port %u  Event %s\n", port+1, "OOM");  break;
        case VTSS_UPI_OTN_LOF_EV:             printf("UPI  Port %u  Event %s\n", port+1, "LOF");  break;
        case VTSS_UPI_OTN_OOF_EV:             printf("UPI  Port %u  Event %s\n", port+1, "OOF");  break;
    }

    event_window(FALSE);
}

static void print_xfi_event(vtss_port_no_t port, vtss_xfi_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_XFI_HSS_HSSPLLLOCK_EV:        printf("XFI  Port %u  Event %s\n", port+1, "HSSPLLLOCK");  break;
        case VTSS_XFI_HSS_HSSRESETOUT_EV:       printf("XFI  Port %u  Event %s\n", port+1, "HSSRESETOUT");  break;
        case VTSS_XFI_HSS_HSSEYEQUALITY_EV:     printf("XFI  Port %u  Event %s\n", port+1, "HSSEYEQUALITY");  break;
        case VTSS_XFI_HSS_HSSPRTREADY_EV:       printf("XFI  Port %u  Event %s\n", port+1, "HSSPRTREADY");  break;
        case VTSS_XFI_HSS_RXPRBSSYNC_EV:        printf("XFI  Port %u  Event %s\n", port+1, "RXPRBSSYNC");  break;
        case VTSS_XFI_HSS_RXPRBSERR_EV:         printf("XFI  Port %u  Event %s\n", port+1, "RXPRBSERR");  break;
        case VTSS_XFI_HSS_RXASIGDET_EV:         printf("XFI  Port %u  Event %s\n", port+1, "RXASIGDET");  break;
        case VTSS_XFI_HSS_RXA_LOS_EV:           printf("XFI  Port %u  Event %s\n", port+1, "RXA_LOS");  break;
        case VTSS_XFI_HSS_IFPLL_LOCK_EV:        printf("XFI  Port %u  Event %s\n", port+1, "IFPLL_LOCK");  break;
        case VTSS_XFI_HSS_TX_FIFO_LOS_EV:       printf("XFI  Port %u  Event %s\n", port+1, "TX_FIFO_LOS");  break;
        case VTSS_XFI_HSS_RXBSIGDET_EV:         printf("XFI  Port %u  Event %s\n", port+1, "RXBSIGDET");  break;
        case VTSS_XFI_HSS_RXB_LOS_EV:           printf("XFI  Port %u  Event %s\n", port+1, "RXB_LOS");  break;

        case VTSS_XFI_AE_AET_RUN_B_EV:         printf("XFI  Port %u  Event %s\n", port+1, "AET_RUN_B");  break;
        case VTSS_XFI_AE_AET_FRM_LCK_B_EV:     printf("XFI  Port %u  Event %s\n", port+1, "AET_FRM_LCK_B");  break;
        case VTSS_XFI_AE_PCS_RX_READY_B_EV:    printf("XFI  Port %u  Event %s\n", port+1, "PCS_RX_READY_B");  break;
        case VTSS_XFI_AE_PCS_TX_READY_B_EV:    printf("XFI  Port %u  Event %s\n", port+1, "PCS_TX_READY_B");  break;
        case VTSS_XFI_AE_FEC_NEGOTIATED_B_EV:  printf("XFI  Port %u  Event %s\n", port+1, "FEC_NEGOTIATED_B");  break;
        case VTSS_XFI_AE_TRAIN_OK_B_EV:        printf("XFI  Port %u  Event %s\n", port+1, "TRAIN_OK_B");  break;
        case VTSS_XFI_AE_TRAIN_FAIL_B_EV:      printf("XFI  Port %u  Event %s\n", port+1, "TRAIN_FAIL_B");  break;
        case VTSS_XFI_AE_TRAIN_LOCAL_B_EV:     printf("XFI  Port %u  Event %s\n", port+1, "TRAIN_LOCAL_B");  break;
        case VTSS_XFI_AE_AET_RUN_A_EV:         printf("XFI  Port %u  Event %s\n", port+1, "AET_RUN_A");  break;
        case VTSS_XFI_AE_AET_FRM_LCK_A_EV:     printf("XFI  Port %u  Event %s\n", port+1, "AET_FRM_LCK_A");  break;
        case VTSS_XFI_AE_PCS_RX_READY_A_EV:    printf("XFI  Port %u  Event %s\n", port+1, "PCS_RX_READY_A");  break;
        case VTSS_XFI_AE_PCS_TX_READY_A_EV:    printf("XFI  Port %u  Event %s\n", port+1, "PCS_TX_READY_A");  break;
        case VTSS_XFI_AE_FEC_NEGOTIATED_A_EV:  printf("XFI  Port %u  Event %s\n", port+1, "FEC_NEGOTIATED_A");  break;
        case VTSS_XFI_AE_TRAIN_OK_A_EV:        printf("XFI  Port %u  Event %s\n", port+1, "TRAIN_OK_A");  break;
        case VTSS_XFI_AE_TRAIN_FAIL_A_EV:      printf("XFI  Port %u  Event %s\n", port+1, "TRAIN_FAIL_A");  break;
        case VTSS_XFI_AE_TRAIN_LOCAL_A_EV:     printf("XFI  Port %u  Event %s\n", port+1, "TRAIN_LOCAL_A");  break;

        case VTSS_XFI_OTN_FAS_POS_EV:           printf("XFI  Port %u  Event %s\n", port+1, "FAS_POS");  break;
        case VTSS_XFI_OTN_LOM_EV:               printf("XFI  Port %u  Event %s\n", port+1, "LOM");  break;
        case VTSS_XFI_OTN_OOM_EV:               printf("XFI  Port %u  Event %s\n", port+1, "OOM");  break;
        case VTSS_XFI_OTN_LOF_EV:               printf("XFI  Port %u  Event %s\n", port+1, "LOF");  break;
        case VTSS_XFI_OTN_OOF_EV:               printf("XFI  Port %u  Event %s\n", port+1, "OOF");  break;

        case VTSS_XFI_ANEG_INCOMPATIBLE_LINK_EV: printf("XFI  Port %u  Event %s\n", port+1, "INCOMPATIBLE_LINK");  break;
        case VTSS_XFI_ANEG_PAR_DETECT_FAULT_EV:  printf("XFI  Port %u  Event %s\n", port+1, "PAR_DETECT_FAULT");  break;
        case VTSS_XFI_ANEG_AN_GOOD_CHECK_EV:     printf("XFI  Port %u  Event %s\n", port+1, "AN_GOOD_CHECK");  break;
        case VTSS_XFI_ANEG_PAGE_RX_EV:           printf("XFI  Port %u  Event %s\n", port+1, "PAGE_RX");  break;
        case VTSS_XFI_ANEG_ANEG_COMPLETE_EV:     printf("XFI  Port %u  Event %s\n", port+1, "ANEG_COMPLETE");  break;
    }

    event_window(FALSE);
}

static void print_ewis_event(vtss_port_no_t port, vtss_ewis_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_EWIS_LOS_EV:         printf("EWIS  Port %u  Event %s\n", port+1, "LOS");  break;
        case VTSS_EWIS_OOF_EV:         printf("EWIS  Port %u  Event %s\n", port+1, "OOF");  break;
        case VTSS_EWIS_LOF_EV:         printf("EWIS  Port %u  Event %s\n", port+1, "LOF");  break;
        case VTSS_EWIS_RDIL_EV:        printf("EWIS  Port %u  Event %s\n", port+1, "RDIL");  break;
        case VTSS_EWIS_AISL_EV:        printf("EWIS  Port %u  Event %s\n", port+1, "AISL");  break;
        case VTSS_EWIS_S1_EV:          printf("EWIS  Port %u  Event %s\n", port+1, "S1");  break;
        case VTSS_EWIS_LOP_EV:         printf("EWIS  Port %u  Event %s\n", port+1, "LOP");  break;
        case VTSS_EWIS_AISP_EV:        printf("EWIS  Port %u  Event %s\n", port+1, "AISP");  break;
        case VTSS_EWIS_PLMP_EV:        printf("EWIS  Port %u  Event %s\n", port+1, "PLMP");  break;
        case VTSS_EWIS_UNEQP_EV:       printf("EWIS  Port %u  Event %s\n", port+1, "UNEQP");  break;
        case VTSS_EWIS_RDIP_EV:        printf("EWIS  Port %u  Event %s\n", port+1, "RDIP");  break;
        case VTSS_EWIS_FE_AIS_P_EV:    printf("EWIS  Port %u  Event %s\n", port+1, "FE_AIS_P");  break;
        case VTSS_EWIS_FEPLMP_EV:      printf("EWIS  Port %u  Event %s\n", port+1, "FEPLMP");  break;
        case VTSS_EWIS_FE_UNEQP_EV:    printf("EWIS  Port %u  Event %s\n", port+1, "FE_UNEQP");  break;
    }

    event_window(FALSE);
}

static void print_pcs_event(vtss_port_no_t port, vtss_pcs_10gbase_r_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_PCS_10GBASE_R_RX_HI_BER_EV:            printf("PCS  Port %u  Event %s\n", port+1, "RX_HI_BER_EV");  break;
        case VTSS_PCS_10GBASE_R_RX_FIFO_OFLOW_EV:        printf("PCS  Port %u  Event %s\n", port+1, "RX_FIFO_OFLOW_EV");  break;
        case VTSS_PCS_10GBASE_R_RX_FIFO_UFLOW_EV:        printf("PCS  Port %u  Event %s\n", port+1, "RX_FIFO_UFLOW_EV");  break;
        case VTSS_PCS_10GBASE_R_LOCK_CHANGED_EV:         printf("PCS  Port %u  Event %s\n", port+1, "LOCK_CHANGED_EV");  break;
        case VTSS_PCS_10GBASE_R_C64B66B_ERR_EV:          printf("PCS  Port %u  Event %s\n", port+1, "C64B66B_ERR_EV");  break;
        case VTSS_PCS_10GBASE_R_RX_OSET_EV:              printf("PCS  Port %u  Event %s\n", port+1, "RX_OSET_EV");  break;
        case VTSS_PCS_10GBASE_R_RX_OSET_FIFO_FULL_EV:    printf("PCS  Port %u  Event %s\n", port+1, "RX_OSET_FIFO_FULL");  break;
        case VTSS_PCS_10GBASE_R_TX_FIFO_OFLOW_EV:        printf("PCS  Port %u  Event %s\n", port+1, "TX_FIFO_OFLOW_EV");  break;
        case VTSS_PCS_10GBASE_R_TX_FIFO_UFLOW_EV:        printf("PCS  Port %u  Event %s\n", port+1, "TX_FIFO_UFLOW_EV");  break;
        case VTSS_PCS_10GBASE_R_XGMII_ERR_EV:            printf("PCS  Port %u  Event %s\n", port+1, "XGMII_ERR_EV");  break;
        case VTSS_PCS_10GBASE_R_RX_FSET_EV:              printf("PCS  Port %u  Event %s\n", port+1, "RX_FSET_EV");  break;
        case VTSS_PCS_10GBASE_R_RX_FSET_FIFO_FULL_EV:    printf("PCS  Port %u  Event %s\n", port+1, "RX_FSET_FIFO_FULL");  break;
        case VTSS_PCS_10GBASE_R_TX_FIFO_ECC_1BIT_EV:     printf("PCS  Port %u  Event %s\n", port+1, "TX_FIFO_ECC_1BIT");  break;
        case VTSS_PCS_10GBASE_R_TX_FIFO_ECC_2BIT_EV:     printf("PCS  Port %u  Event %s\n", port+1, "TX_FIFO_ECC_2BIT");  break;
        case VTSS_PCS_10GBASE_R_RX_FIFO_ECC_1BIT_EV:     printf("PCS  Port %u  Event %s\n", port+1, "RX_FIFO_ECC_1BIT");  break;
        case VTSS_PCS_10GBASE_R_RX_FIFO_ECC_2BIT_EV:     printf("PCS  Port %u  Event %s\n", port+1, "RX_FIFO_ECC_2BIT");  break;
    }

    event_window(FALSE);
}

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static void print_1588_event(vtss_port_no_t port, vtss_phy_ts_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_PHY_TS_INGR_ENGINE_ERR:       printf("1588  Port %u  Event %s\n", port+1, "INGR_ENGINE_ERR");  break;
        case VTSS_PHY_TS_INGR_RW_PREAM_ERR:     printf("1588  Port %u  Event %s\n", port+1, "INGR_PREAM_ERR");  break;
        case VTSS_PHY_TS_INGR_RW_FCS_ERR:       printf("1588  Port %u  Event %s\n", port+1, "INGR_FCS_ERR");  break;
        case VTSS_PHY_TS_EGR_ENGINE_ERR:        printf("1588  Port %u  Event %s\n", port+1, "EGR_ENGINE_ERR");  break;
        case VTSS_PHY_TS_EGR_RW_FCS_ERR:        printf("1588  Port %u  Event %s\n", port+1, "EGR_FCS_ERR");  break;
        case VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED:
            {
                printf("1588  Port %u  Event %s\n", port+1, "EGR_TIMESTAMP_CAPTURED");
                /* Call the vtss_phy_ts_empty function */
                printf("B4 calling vtss_phy_ts_fifo_empty \n\r");
                vtss_phy_ts_fifo_empty(NULL, port);

            }
            break;
        case VTSS_PHY_TS_EGR_FIFO_OVERFLOW:
            { 
                printf("1588  Port %u  Event %s\n", port+1, "EGR_FIFO_OVERFLOW");
                /* Call the vtss_phy_ts_empty function */
                printf("B4 calling vtss_phy_ts_fifo_empty \n\r");
                vtss_phy_ts_fifo_empty(NULL, port);

            }
            break;
    }

    event_window(FALSE);
}
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

static void print_mac10g_event(vtss_port_no_t port, vtss_mac10g_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_MAC10G_LINK_DELTA_EV:                 printf("MAC  Port %u  Event %s\n", port+1, "LINK_DELTA_EV");  break;
        case VTSS_MAC10G_REMOTE_FAULT_STATE_DELTA_EV:   printf("MAC  Port %u  Event %s\n", port+1, "REMOTE_FAULT_STATE_DELTA");  break;
        case VTSS_MAC10G_LOCAL_FAULT_STATE_DELTA_EV:    printf("MAC  Port %u  Event %s\n", port+1, "LOCAL_FAULT_STATE_DELTA");  break;
        case VTSS_MAC10G_TX_ABORT_EV:                   printf("MAC  Port %u  Event %s\n", port+1, "TX_ABORT_EV");  break;
        case VTSS_MAC10G_RX_NON_STD_PREAM_EV:           printf("MAC  Port %u  Event %s\n", port+1, "RX_NON_STD_PREAM_EV");  break;
        case VTSS_MAC10G_RX_PREAM_ERR_EV:               printf("MAC  Port %u  Event %s\n", port+1, "RX_PREAM_ERR_EV");  break;
        case VTSS_MAC10G_RX_PREAM_MISMATCH_EV:          printf("MAC  Port %u  Event %s\n", port+1, "RX_PREAM_MISMATCH_EV");  break;
        case VTSS_MAC10G_RX_PREAM_SHRINK_EV:            printf("MAC  Port %u  Event %s\n", port+1, "RX_PREAM_SHRINK_EV");  break;
        case VTSS_MAC10G_TX_IFG_COLUMN_DROP_EV:         printf("MAC  Port %u  Event %s\n", port+1, "TX_IFG_COLUMN_DROP_EV");  break;
    }

    event_window(FALSE);
}

static void print_otn_och_event(vtss_port_no_t port, vtss_otn_och_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_OTN_OCH_LOS_EV:                 printf("OTN  Port %u  Event %s\n", port+1, "OCH_LOS_EV");  break;
        case VTSS_OTN_OCH_LOF_EV:                 printf("OTN  Port %u  Event %s\n", port+1, "OCH_LOF_EV");  break;
        case VTSS_OTN_OCH_LOM_EV:                 printf("OTN  Port %u  Event %s\n", port+1, "OCH_LOM_EV");  break;
        case VTSS_OTN_OCH_AIS_EV:                 printf("OTN  Port %u  Event %s\n", port+1, "OCH_AIS_EV");  break;
        case VTSS_OTN_OCH_BER_EXCD_EV:            printf("OTN  Port %u  Event %s\n", port+1, "OCH_BER_EXCD_EV"); break;                                         
        case VTSS_OTN_OCH_UNCRCT_CODE_EV:         printf("OTN  Port %u  Event %s\n", port+1, "OCH_UNCRCT_CODE_EV"); break;                                        
    }

    event_window(FALSE);
}

static void print_otn_otu_event(vtss_port_no_t port, vtss_otn_otu_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_OTN_OTU_TIM_EV:                 printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_TIM_EV");  break;
        case VTSS_OTN_OTU_ACC_TTI_EV:             printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_ACC_TTI_EV");  break;
        case VTSS_OTN_OTU_ICONSIST_TTI_EV:        printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_ICONSIST_TTI_EV");  break;
        case VTSS_OTN_OTU_ACC_RES_EV:             printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_ACC_RES_EV");  break;
        case VTSS_OTN_OTU_INCONSIST_RES_EV:       printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_INCONSIST_RES_EV");  break;
        case VTSS_OTN_OTU_ACC_SMRES_EV:           printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_ACC_SMRES_EV");  break;
        case VTSS_OTN_OTU_INCONSIST_SMRES_EV:     printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_INCONSIST_SMRES_EV");  break;
        case VTSS_OTN_OTU_ACC_GCC0_EV:            printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_ACC_GCC0_EV");  break;
        case VTSS_OTN_OTU_INCONSIST_GCC0_EV:      printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_INCONSIST_GCC0_EV");  break;
        case VTSS_OTN_OTU_BDI_EV:                 printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_BDI_EV");  break;
        case VTSS_OTN_OTU_IAE_EV:                 printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_IAE_EV");  break;
        case VTSS_OTN_OTU_BIAE_EV:                printf("OTN  Port %u  Event %s\n", port+1, "OTN_OTU_BIAE_EV");  break;
    }

    event_window(FALSE);
}

static void print_otn_odu_event(vtss_port_no_t port, vtss_otn_odu_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_OTN_ODU_TIM_EV:                printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_TIM_EV");  break;
        case VTSS_OTN_ODU_ACC_TTI_EV:            printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ACC_TTI_EV");  break;
        case VTSS_OTN_ODU_ICONSIST_TTI_EV:       printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ICONSIST_TTI_EV");  break;
        case VTSS_OTN_ODU_PLM_EV:                printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_PLM_EV");  break;
        case VTSS_OTN_ODU_ACC_PT_EV:             printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ACC_PT_EV");  break;
        case VTSS_OTN_ODU_ICONSIST_PT_EV:        printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ICONSIST_PT_EV");  break;
        case VTSS_OTN_ODU_ACC_EXP_EV:            printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ACC_EXP_EV");  break;
        case VTSS_OTN_ODU_ICONSIST_EXP_EV:       printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ICONSIST_EXP_EV");  break;
        case VTSS_OTN_ODU_ACC_RES_EV:            printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ACC_RES_EV");  break;
        case VTSS_OTN_ODU_ICONSIST_RES_EV:       printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ICONSIST_RES_EV");  break;
        case VTSS_OTN_ODU_ACC_FTFL_EV:           printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ACC_FTFL_EV");  break;
        case VTSS_OTN_ODU_ICONSIST_FTFL_EV:      printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ICONSIST_FTFL_EV");  break;
        case VTSS_OTN_ODU_ACC_APS_EV:            printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ACC_APS_EV");  break;
        case VTSS_OTN_ODU_ICONSIST_APS_EV:       printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_ICONSIST_APS_EV");  break;
        case VTSS_OTN_ODU_OCI_EV:                printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_OCI_EV");  break;
        case VTSS_OTN_ODU_LCK_EV:                printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_LCK_EV");  break;
        case VTSS_OTN_ODU_BDI_EV:                printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_BDI_EV");  break;
        case VTSS_OTN_ODU_AIS_EV:                printf("OTN  Port %u  Event %s\n", port+1, "OTN_ODU_AIS_EV");  break;
   }

    event_window(FALSE);
}

static void print_otn_odut_event(vtss_port_no_t port, u32 level, vtss_otn_odu_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_OTN_ODUT_TIM_EV:                printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_TIM_EV");  break;
        case VTSS_OTN_ODUT_ACC_TTI_EV:            printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_ACC_TTI_EV");  break;
        case VTSS_OTN_ODUT_ICONSIST_TTI_EV:       printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_ICONSIST_TTI_EV");  break;
        case VTSS_OTN_ODUT_ACC_STAT_EV:           printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_ACC_STAT_EV");  break;
        case VTSS_OTN_ODUT_ICONSIST_STAT_EV:      printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_ICONSIST_STAT_EV");  break;
        case VTSS_OTN_ODUT_ACC_APS_EV:            printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_ACC_APS_EV");  break;
        case VTSS_OTN_ODUT_ICONSIST_APS_EV:       printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_ICONSIST_APS_EV");  break;
        case VTSS_OTN_ODUT_OCI_EV:                printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_OCI_EV");  break;
        case VTSS_OTN_ODUT_LCK_EV:                printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_LCK_EV");  break;
        case VTSS_OTN_ODUT_BDI_EV:                printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_BDI_EV");  break;
        case VTSS_OTN_ODUT_AIS_EV:                printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_AIS_EV");  break;
        case VTSS_OTN_ODUT_LTC_EV:                printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_LTC_EV");  break;
        case VTSS_OTN_ODUT_IAE_EV:                printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_IAE_EV");  break;
        case VTSS_OTN_ODUT_BIAE_EV:               printf("OTN  Port %u  Level %u  Event %s\n", port+1, level+1, "OTN_ODUT_BIAE_EV");  break;
   }

    event_window(FALSE);
}

static void print_gfp_event(vtss_port_no_t port, vtss_gfp_event_t event)
{
    event_window(TRUE);

    switch (event)
    {
        case VTSS_GFP_SSF_EV:                    printf("GFP  Port %u  Event %s\n", port+1, "GFP_SSF_EV");  break;
        case VTSS_GFP_8B10B_LOS_EV:              printf("GFP  Port %u  Event %s\n", port+1, "GFP_8B10B_LOS_EV");  break;
        case VTSS_GFP_8B10B_LSYNC_EV:            printf("GFP  Port %u  Event %s\n", port+1, "GFP_8B10B_LSYNC_EV");  break;
        case VTSS_GFP_8B10B_ERROR_EV:            printf("GFP  Port %u  Event %s\n", port+1, "GFP_8B10B_ERROR_EV");  break;
        case VTSS_GFP_8B10B_RX_ILLEGAL_EV:       printf("GFP  Port %u  Event %s\n", port+1, "GFP_8B10B_RX_ILLEGAL_EV");  break;
        case VTSS_GFP_8B10B_RX_DISPARITY_EV:     printf("GFP  Port %u  Event %s\n", port+1, "GFP_8B10B_RX_DISPARITY_EV");  break;
        case VTSS_GFP_8B10B_TX_ILLEGAL_EV:       printf("GFP  Port %u  Event %s\n", port+1, "GFP_8B10B_TX_ILLEGAL_EV");  break;
        case VTSS_GFP_8B10B_TX_DISPARITY_EV:     printf("GFP  Port %u  Event %s\n", port+1, "GFP_8B10B_TX_DISPARITY_EV");  break;
        case VTSS_GFP_CMF_LOS_SF_EV:             printf("GFP  Port %u  Event %s\n", port+1, "GFP_CMF_LOS_SF_EV");  break;
        case VTSS_GFP_CMF_LSYNC_SF_EV:           printf("GFP  Port %u  Event %s\n", port+1, "GFP_CMF_LSYNC_SF_EV");  break;
        case VTSS_GFP_CMF_UNSUPPORTED_EV:        printf("GFP  Port %u  Event %s\n", port+1, "GFP_CMF_UNSUPPORTED_EV");  break;
        case VTSS_GFP_CMF_INCONSISTENT_EV:       printf("GFP  Port %u  Event %s\n", port+1, "GFP_CMF_INCONSISTENT_EV");  break;
        case VTSS_GFP_CDF_UPM_EV:                printf("GFP  Port %u  Event %s\n", port+1, "GFP_CDF_UPM_EV");  break;
        case VTSS_GFP_PPOS_CDF_UPM_EV:           printf("GFP  Port %u  Event %s\n", port+1, "GFP_PPOS_CDF_UPM_EV"); break;
        case VTSS_GFP_FRAME_MAPPED_ETHERNET_EV:  printf("GFP  Port %u  Event %s\n", port+1, "GFP_FRAME_MAPPED_ETHERNET_EV"); break;
        case VTSS_GFP_CMF_ACCEPTED_EV:           printf("GFP  Port %u  Event %s\n", port+1, "GFP_CMF_ACCEPTED_EV"); break;
   }

    event_window(FALSE);
}

static void print_efec_event(vtss_port_no_t port, vtss_efec_event_t event)
{
    event_window(TRUE);

    switch(event)
    {
        case VTSS_I4_ED_UCCW_EV:          printf("EFEC  Port %u  Event %s\n", port+1, "I4_ED_UCCW_EV"); break;                                          
        case VTSS_I4_RS_UCCW_EV:          printf("EFEC  Port %u  Event %s\n", port+1, "I4_RS_UCCW_EV"); break;                                          
        case VTSS_I4_BCH_UCCW_EV:         printf("EFEC  Port %u  Event %s\n", port+1, "I4_BCH_UCCW_EV"); break; 
    }

    event_window(FALSE);
}    
            
static void print_efeci7_event(vtss_port_no_t port, vtss_efeci7_event_t event)
{
    event_window(TRUE);

    if (event == VTSS_I7_BER_ALARM_EV)
        printf("EFEC  Port %u  Event %s\n", port+1, "VTSS_I7_BER_ALARM_EV");

    event_window(FALSE);
}    
            
static BOOL intr_flag=0;

static void daytona_interrupt(int sig) 
{
    intr_flag = 1;
    signal(SIGIO, daytona_interrupt); 
}

void vtss_board_daytona_interrupt(void)
{
    vtss_interrupt_block_t      status;
    vtss_upi_event_t            upi;
    vtss_xfi_event_t            xfi;
    vtss_ewis_event_t           ewis;
    vtss_pcs_10gbase_r_event_t  pcs;
    vtss_mac10g_event_t         mac10g;
    vtss_otn_och_event_t        otn_och;
    vtss_otn_otu_event_t        otn_otu;
    vtss_otn_odu_event_t        otn_odu;
    vtss_otn_odut_event_t       otn_odut;
    vtss_gfp_event_t       gfp;
    vtss_efec_event_t           efec;
    vtss_efeci7_event_t         efeci7;
    u64                         mask;
    u32                         i, level, port=0;

    if (!intr_flag)   return;
    intr_flag = 0;

    vtss_interrupt_status_get(NULL, &status);

    if ((status.misc & (VTSS_MISC_CXFI_0_INT | VTSS_MISC_CXFI_1_INT | VTSS_MISC_LXFI_0_INT | VTSS_MISC_LXFI_1_INT))) { /* XFI interrupt */
        if (status.misc & VTSS_MISC_CXFI_0_INT) port = 0;
        if (status.misc & VTSS_MISC_CXFI_1_INT) port = 1;
        if (status.misc & VTSS_MISC_LXFI_0_INT) port = 2;
        if (status.misc & VTSS_MISC_LXFI_1_INT) port = 3;

        if (vtss_xfi_event_poll(NULL, port, &xfi) != VTSS_RC_OK)    T_E("vtss_xfi_event_poll failed");
        for (i=0, mask=1; i<64; ++i, mask<<=1)
            print_xfi_event(port, xfi&mask);
    }

    //if (status.misc & VTSS_MISC_PMTICK_INT) {
    //}

    if ((status.ch_0 & VTSS_CH_UPI_INT) || (status.ch_1 & VTSS_CH_UPI_INT)) { /* UPI interrupt */
        if (status.ch_0 & VTSS_CH_UPI_INT) port = 0;
        if (status.ch_1 & VTSS_CH_UPI_INT) port = 1;

        if (vtss_upi_event_poll(NULL, 0, &upi) != VTSS_RC_OK)    T_E("vtss_upi_event_poll failed");
        for (i=0, mask=1; i<32; ++i, mask<<=1)
            print_upi_event(0, upi&mask);
    }


    if ((status.ch_0 & (VTSS_CH_OTN1_INT | VTSS_CH_OTN2_INT)) || (status.ch_1 & (VTSS_CH_OTN1_INT | VTSS_CH_OTN2_INT))) { /* OTN interrupt */
        if (status.ch_0 & VTSS_CH_OTN1_INT) port = 0;
        if (status.ch_0 & VTSS_CH_OTN2_INT) port = 2;
        if (status.ch_1 & VTSS_CH_OTN1_INT) port = 1;
        if (status.ch_1 & VTSS_CH_OTN2_INT) port = 3;

        if (vtss_otn_och_event_poll(NULL, port, &otn_och) != VTSS_RC_OK)    T_E("vtss_otn_och_event_poll failed");
        if (vtss_otn_otu_event_poll(NULL, port, &otn_otu) != VTSS_RC_OK)    T_E("vtss_otn_otu_event_poll failed");
        if (vtss_otn_odu_event_poll(NULL, port, &otn_odu) != VTSS_RC_OK)    T_E("vtss_otn_odu_event_poll failed");
        for (i=0, mask=1; i<32; ++i, mask<<=1) {
            print_otn_och_event(port, otn_och&mask);
            print_otn_otu_event(port, otn_otu&mask);
            print_otn_odu_event(port, otn_odu&mask);
        }
        for (level=0; level<VTSS_OTN_ODUT_LEVEL_NUMBER; ++level) {
            if (vtss_otn_odut_event_poll(NULL, port, level, &otn_odut) != VTSS_RC_OK)    T_E("vtss_otn_odut_event_poll failed");
            for (i=0, mask=1; i<32; ++i, mask<<=1) {
                print_otn_odut_event(port, level, otn_odut&mask);
            }
        }
    }


    if ((status.ch_0 & (VTSS_CH_WIS1_INT | VTSS_CH_WIS2_INT)) || (status.ch_1 & (VTSS_CH_WIS1_INT | VTSS_CH_WIS2_INT))) { /* WIS interrupt */
        if (status.ch_0 & VTSS_CH_WIS1_INT) port = 0;
        if (status.ch_0 & VTSS_CH_WIS2_INT) port = 2;
        if (status.ch_1 & VTSS_CH_WIS1_INT) port = 1;
        if (status.ch_1 & VTSS_CH_WIS2_INT) port = 3;

        if (vtss_ewis_event_poll(NULL, port, &ewis) != VTSS_RC_OK)    T_E("vtss_ewis_event_poll failed");
        for (i=0, mask=1; i<(sizeof(vtss_ewis_event_t) * 8); ++i, mask<<=1)
            print_ewis_event(port, ewis&mask);
    }


    if ((status.ch_0 & (VTSS_CH_PCS1_INT | VTSS_CH_PCS2_INT)) || (status.ch_1 & (VTSS_CH_PCS1_INT | VTSS_CH_PCS2_INT))) { /* PCS interrupt */
        if (status.ch_0 & VTSS_CH_PCS1_INT) port = 0;
        if (status.ch_0 & VTSS_CH_PCS2_INT) port = 2;
        if (status.ch_1 & VTSS_CH_PCS1_INT) port = 1;
        if (status.ch_1 & VTSS_CH_PCS2_INT) port = 3;

        if (vtss_pcs_10gbase_r_event_poll(NULL, port, &pcs) != VTSS_RC_OK)    T_E("vtss_pcs_10gbase_r_event_poll failed");
        for (i=0, mask=1; i<32; ++i, mask<<=1)
            print_pcs_event(port, pcs&mask);
    }

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    if ((status.misc & (VTSS_MISC_IP1588_0_INT | VTSS_MISC_IP1588_1_INT))) { /* 1588 interrupt */
        vtss_phy_ts_event_t event_1588;

        if (status.misc & VTSS_MISC_IP1588_0_INT) port = 2;
        if (status.misc & VTSS_MISC_IP1588_1_INT) port = 3;

        if (vtss_phy_ts_event_poll(NULL, port, &event_1588) != VTSS_RC_OK)    T_E("vtss_phy_ts_event_poll failed");
        for (i=0, mask=1; i<8; ++i, mask<<=1)
            print_1588_event(port, event_1588&mask);
    }
#endif

    if ((status.ch_0 & (VTSS_CH_MAC1_INT | VTSS_CH_MAC2_INT)) || (status.ch_1 & (VTSS_CH_MAC1_INT | VTSS_CH_MAC2_INT))) { /* MAC interrupt */
        if (status.ch_0 & VTSS_CH_MAC1_INT) port = 0;
        if (status.ch_0 & VTSS_CH_MAC2_INT) port = 2;
        if (status.ch_1 & VTSS_CH_MAC1_INT) port = 1;
        if (status.ch_1 & VTSS_CH_MAC2_INT) port = 3;

        if (vtss_mac10g_event_poll(NULL, port, &mac10g) != VTSS_RC_OK)    T_E("vtss_mac10g_event_poll failed");
        for (i=0, mask=1; i<32; ++i, mask<<=1)
            print_mac10g_event(port, mac10g&mask);
    }

    if ((status.ch_0 & (VTSS_CH_GFP1_INT | VTSS_CH_GFP2_INT)) || (status.ch_1 & (VTSS_CH_GFP1_INT | VTSS_CH_GFP2_INT))) { /* GFP interrupt */
        if (status.ch_0 & VTSS_CH_GFP1_INT) port = 0;
        if (status.ch_0 & VTSS_CH_GFP2_INT) port = 2;
        if (status.ch_1 & VTSS_CH_GFP1_INT) port = 1;
        if (status.ch_1 & VTSS_CH_GFP2_INT) port = 3;

        if (vtss_gfp_event_poll(NULL, port, &gfp) != VTSS_RC_OK)    T_E("vtss_gfp_event_poll failed");
        for (i=0, mask=1; i<32; ++i, mask<<=1)
            print_gfp_event(port, gfp&mask);
    }

    if ((status.ch_0 | status.ch_1) &  VTSS_CH_EFEC_INT) {/* EFEC interrupt */
        if (status.ch_0 & VTSS_CH_EFEC_INT) port = 2;
        else if (status.ch_1 & VTSS_CH_EFEC_INT) port = 3;
        
        if (vtss_efec_event_poll(NULL, port, &efec) != VTSS_RC_OK)
            T_E("vtss_efec_event_poll failed");
        for (i=0, mask=1; i<32; ++i, mask<<=1)
            print_efec_event(port, efec & mask);
    }

    if (status.misc & (VTSS_MISC_EFEC7_0_INT | VTSS_MISC_EFEC7_1_INT)) {/* EFECI7 interrupt */
        if (VTSS_MISC_EFEC7_0_INT) port = 2;
        else if (VTSS_MISC_EFEC7_1_INT) port = 3;

        if (vtss_efeci7_event_poll(NULL, port, &efeci7) != VTSS_RC_OK)
            T_E("vtss_efeci7_event_poll failed");
        for (i=0, mask=1; i<32; ++i, mask<<=1)
            print_efeci7_event(port, efeci7 & mask);
    }
        
    write(interrupt_fd, "", 0); /* This statement to Re-enable the interrput */
}

void vtss_board_daytona_event_screen(BOOL enable)
{
    event_screen = enable;
}


void vtss_board_daytona_interrupt_without_mask(vtss_port_no_t port)
{
    vtss_upi_event_t            upi;
    vtss_xfi_event_t            xfi;
    vtss_ewis_event_t           ewis;
    vtss_pcs_10gbase_r_event_t  pcs;
    vtss_mac10g_event_t         mac10g;
    vtss_otn_och_event_t        otn_och;
    vtss_otn_otu_event_t        otn_otu;
    vtss_otn_odu_event_t        otn_odu;
    vtss_otn_odut_event_t       otn_odut;
    vtss_gfp_event_t       gfp;
    u64                         mask;
    u32                         i, level;



   if (vtss_xfi_event_poll_without_mask(NULL, port, &xfi) != VTSS_RC_OK)    T_E("vtss_xfi_event_poll failed");
        for (i=0, mask=1; i<64; ++i, mask<<=1)
            print_xfi_event(port, xfi&mask);

   if (port < 2) {     /* UPI block exists on the client side only */
       if (vtss_upi_event_poll_without_mask(NULL, port, &upi) != VTSS_RC_OK)    T_E("vtss_upi_event_poll failed");
            for (i=0, mask=1; i<32; ++i, mask<<=1)
                print_upi_event(0, upi&mask);
   }


   if (vtss_otn_och_event_poll_without_mask(NULL, port, &otn_och) != VTSS_RC_OK)    T_E("vtss_otn_och_event_poll failed");
   if (vtss_otn_otu_event_poll_without_mask(NULL, port, &otn_otu) != VTSS_RC_OK)    T_E("vtss_otn_otu_event_poll failed");
   if (vtss_otn_odu_event_poll_without_mask(NULL, port, &otn_odu) != VTSS_RC_OK)    T_E("vtss_otn_odu_event_poll failed");
   for (i=0, mask=1; i<32; ++i, mask<<=1) {
        print_otn_och_event(port, otn_och&mask);
        print_otn_otu_event(port, otn_otu&mask);
        print_otn_odu_event(port, otn_odu&mask);
   }
   for (level=0; level<VTSS_OTN_ODUT_LEVEL_NUMBER; ++level) {
       if (vtss_otn_odut_event_poll_without_mask(NULL, port, level, &otn_odut) != VTSS_RC_OK)    T_E("vtss_otn_odut_event_poll failed");
       for (i=0, mask=1; i<32; ++i, mask<<=1) {
           print_otn_odut_event(port, level, otn_odut&mask);
       }
   }

   if (vtss_ewis_event_poll_without_mask(NULL, port, &ewis) != VTSS_RC_OK)    T_E("vtss_ewis_event_poll failed");
   for (i=0, mask=1; i<(sizeof(vtss_ewis_event_t) * 8); ++i, mask<<=1)
       print_ewis_event(port, ewis&mask);

   if (vtss_pcs_10gbase_r_event_poll(NULL, port, &pcs) != VTSS_RC_OK)    T_E("vtss_pcs_10gbase_r_event_poll failed");
   for (i=0, mask=1; i<32; ++i, mask<<=1)
        print_pcs_event(port, pcs&mask);

   if (vtss_mac10g_event_poll_without_mask(NULL, port, &mac10g) != VTSS_RC_OK)    T_E("vtss_mac10g_event_poll failed");
   for (i=0, mask=1; i<32; ++i, mask<<=1)
       print_mac10g_event(port, mac10g&mask);

   if (vtss_gfp_event_poll_without_mask(NULL, port, &gfp) != VTSS_RC_OK)    T_E("vtss_gfp_event_poll failed");
   for (i=0, mask=1; i<32; ++i, mask<<=1)
       print_gfp_event(port, gfp&mask);

}

static int daytona_parse_options(int argc, const char **argv, vtss_init_t *config)
{
    struct sockaddr_in saddr;
//    vtss_port_no_t     port_no;
//    BOOL               sfp = 0, error, val_error, done=FALSE;
    BOOL               error, val_error;
    u32                value = 0, i;
//    u32                value = 0, i, port_max = VTSS_PORTS, q_en=0;
    char               *s, *end;
    const char         *opt, *p;
//    vtss_spi4_conf_t   *spi4 = config->spi4;
//    vtss_xaui_conf_t   *xaui = config->xaui;
    vtss_init_conf_t   *conf = config->init_conf;
    static int         inst = 0;
    vtss_ev_conf_t     ev_conf[2];
//    vtss_port_map_t    *map;
//    int                a, cnt=0;
#ifdef DAYTONA_EVAL_INIT
    BOOL ev_init = 0;
#ifdef BOARD_TALLADEGA
    static BOOL first_time = TRUE;
#endif
#endif

    for(i = 0; i < 2; i++)
      conf->conf_mode[i]= VTSS_CONFIG_MODE_DISABLED;

    if ((argc > 0 && *argv[0] == '?') ||
            argc < 1 || inet_aton(argv[0], &saddr.sin_addr) == 0) {
        printf("-------- Daytona Test application --------\n");
        printf("Usage: %s <rabbit_ip> [<parm>[:<value>]]\n\n", argv[0]);
        printf("Example (config mode 10 (PEE-P and 11(PEE-MP))): %s 1.2.3.4 c0:10 c1:11\n\n", argv[0]);
        printf("Mandatory parameter:\n");
        printf("--------------------\n");
//        printf("rabbit_ip: Rabbit board IP address\n\n");
        printf("c0             : Channel 0 Configuration mode, value:\n");
        printf("c1             : Channel 1 Configuration mode, value:\n");
        printf("               : 8=PEE_W, 9=PEE_MW, 11=PEE-P, 12=PEE-MP \n");
        printf("               : 22=PEO_P1E-100 \n");
        printf("               : + a lot more according to vtss_config_mode_t in vtss_init_api.h\n");
        printf("               : (default: PSS)\n");
        printf("Optional parameters:\n");
        printf("--------------------\n");
        printf("Each optional parameter may consist of two elements:\n");
        printf("<parm> : Keyword identifying the parameter. This may be abbreviated.\n");
        printf("<value>: Value required for some <parm> values.\n\n");
        printf("upi_lanes    : UPI Lane operation: \n");
        printf("               0 => SDR, four lane operation,\n");
        printf("               1 => DDR_A, 2 => DDR_K, 3 => DDR_M two lane operation\n");
        printf("we           : WarmstartEnable\n");
        printf("wm           : Warmstartmode and version (0xMvvvv)\n");
        printf("cs_wait      : Wiat time in nano seconds to sample data after the CS\n");
        printf("cs_hold      : Hold time in nano seconds the data is valid on bus after deassert of CS\n");
#ifdef DAYTONA_EVAL_INIT
        printf("ev_init      : Initialize the Validation board\n");
#endif
        printf("f0           : XFI clock for channel 0: (161), default is 156.25MHz\n");
        printf("f1           : XFI clock for channel 1: (161), default is 156.25MHz\n");
        printf("x4_c0        : Channel 0 X4 frequency case: (1 or 2)\n");
        printf("x4_c1        : Channel 1 X4 frequency case: (1 or 2)\n");
#ifdef DAYTONA_CHIP_SIMULATE
        printf("id           : Actual chip ID (in simulated environments)\n");
        printf("var          : Actual chip Variant [0..2] (in simulated environments)\n");
#endif
        return 0;
    }

#ifdef DAYTONA_EVAL_INIT
#ifdef BOARD_TALLADEGA
    if (first_time == TRUE) {
#endif
    /* Connect to board */
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(26);
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        T_E("socket create failed");
        return 0;
      }
    if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        T_E("connect to rabbit failed");
        return 0;
      }
#ifdef BOARD_TALLADEGA
    }
#endif
#endif /* DAYTONA_EVAL_INIT */
    memset(&daytona_options, 0, sizeof(daytona_options));


    /* Parse options */
    for (i = 1 ; i < argc; i++) {
        p = argv[i];
        opt = p;
        s = strstr(p, ":");
        val_error = 1;
        value = 0;
        if (s != NULL) {
            s++;
            value = strtoul(s, &end, 0);
            val_error = (end == s);
        }
        error = 0;

        if ((strstr(p, "c0") == p) && s != NULL) { /* config_mode */
#ifdef BOARD_TALLADEGA
          if (inst == 0) {
#endif
            if (val_error || value >= VTSS_CONFIG_MODE_MAX ) {
                printf("Illegal config_mode: %s\n", s);
                return 0;
            }
            conf->conf_mode[0] = value;
#ifdef BOARD_TALLADEGA
          }
#endif
        } else if ((strstr(p, "c1") == p) && s != NULL) { /* config_mode channel 1 */
#ifdef BOARD_TALLADEGA
          if (inst == 0) {
#endif
                if (val_error || value >= VTSS_CONFIG_MODE_MAX ) {
                    printf("Illegal channel 1 config_mode: %s\n", s);
                    return 0;
            }
            conf->conf_mode[1] = value;
#ifdef BOARD_TALLADEGA
          }
        } else if ((strstr(p, "c2") == p) && s != NULL) {
          if (inst == 1) {
            if (val_error || value >= VTSS_CONFIG_MODE_MAX ) {
              printf("Illegal channel 2 config_mode: %s\n", s);
              return 0;
            }
            conf->conf_mode[0] = value;
          }
        } else if ((strstr(p, "c3") == p) && s != NULL) {
          if (inst == 1) {
            if (val_error || value >= VTSS_CONFIG_MODE_MAX ) {
              printf("Illegal channel 3 config_mode: %s\n", s);
              return 0;
            }
            conf->conf_mode[1] = value;
          }
#endif
        } else if ((strstr(p, "f0") == p) && s != NULL) {
            if (val_error || (value != 161)) {
                printf("Incorrect frequency input\n");
                return 0;
            }
            conf->xfi_clk[0] = value;
        } else if ((strstr(p, "f1") == p) && s != NULL) {
            if (val_error || (value != 161)) {
                printf("Incorrect frequency input\n");
                return 0;
            }
            conf->xfi_clk[1] = value;
        } else if ((strstr(p, "x4_c0") == p) && s != NULL) {
            if (!(value == 1 || value == 2)) {
                printf("Incorrect x4 case\n");
                return 0;
            }
            conf->x4_case[0] = value;
        } else if ((strstr(p, "x4_c1") == p) && s != NULL) {
            if (!(value == 1 || value == 2)) {
                printf("Incorrect x4 case\n");
                return 0;
            }
            conf->x4_case[1] = value;
        } else if (strstr(p, "upi_lanes") == p) {
            conf->two_lane_upi = value;

        } else if (strstr(p, "wm") == p) {
#ifdef DAYTONA_CHIP_SIMULATE
            reg_write(0,VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_0, value);
            printf("Setting Warmstart mode to (type:version) %x:%x\n", value>>16, value & 0xffff);
#endif

        } else if (strstr(p, "we") == p) {
            conf->warm_start_enable = TRUE;
            printf("Enabling Warmstart feature\n");

        } else if (strstr(p, "id") == p) {
#ifdef DAYTONA_CHIP_SIMULATE
            reg_write(0,VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID, value);
            printf("Setting Chip id to : %x\n", value);
#endif
        } else if (strstr(p, "var") == p) {
#ifdef DAYTONA_CHIP_SIMULATE
            reg_write(0,VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_MODE, value);
            printf("Setting Chip var to : %x\n", value);
#endif
#ifdef DAYTONA_EVAL_INIT
        } else if (strstr(p, "ev_init") == p) {
           value ? (ev_init = 1) : (ev_init = 0);
#endif
        } else if (strstr(p, "cs_wait") == p) {
            conf->pi.cs_wait_ns = value;
        } else if (strstr(p, "cs_hold") == p) {
            conf->pi.cs_hold_ns = value;
        } else
            error = 1;
#ifdef BOARD_TALLADEGA
      /* Parsing Errors for talladega must be recognised after parsing
         instance 1 */
      if(inst == 1) {
#endif
        if (error) {
            printf("Illegal parameter or missing/illegal value: %s\n", opt);
            return 0;
        }
#ifdef BOARD_TALLADEGA
      }
#endif
    }
    printf("Daytona channel %d configuration mode =: %s(%d)\n", 2*inst, config_txt[conf->conf_mode[0]],conf->conf_mode[0]);
    printf("Daytona channel %d configuration mode =: %s(%d)\n", 2*inst+1, config_txt[conf->conf_mode[1]],conf->conf_mode[1]);
    printf("Daytona two_lane_upi =: %s\n", 
           (conf->two_lane_upi == 0) ? "SDR" : ((conf->two_lane_upi == 1) ? "DDR_A" : ((conf->two_lane_upi == 2) ? "DDR_K": "DDR_M")));

#ifdef DAYTONA_EVAL_INIT
    if(ev_init) {
        printf("Initializing the Daytona Validation board for instance %d\n", inst);
        for (i = 0; i < 2; i++) {
            ev_conf[i].mode = conf->conf_mode[i];
            ev_conf[i].xfi_clk = conf->xfi_clk[i];
            ev_conf[i].x4_case = conf->x4_case[i];
        }
        if(daytona_eval_init(inst, ev_conf[0], ev_conf[1]) != VTSS_RC_OK) {
            T_E("daytona_eval_init returned Failure");
        }
    }
#endif
    inst++;

#if 0
    /* Apply options */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        map = &board_port_map[port_no];
        if (port_no < 2 && conf->host_mode > 4 && conf->host_mode < 10) {
            /* Port 1 and 2 are XAUI line ports */
            map->chip_port = (24 + port_no - VTSS_PORT_NO_START);
            map->miim_controller = VTSS_MIIM_CONTROLLER_1;
            map->miim_addr = map->chip_port;
        }

        if (conf->host_mode == VTSS_HOST_MODE_5) {
            // Xaui ports are mapped to 0 and 1. 1G ports are mapped to 2-23.
            // All other ports shall be mapped as unused.
            if (port_no > 23) {
                map->chip_port = CHIP_PORT_UNUSED; // Map as unused.
            }
        }

        if (conf->host_mode > 9) {
            /* Host mode 10/11, one XAUI line port used */
            map->chip_port = (port_no == 0 ? (conf->host_mode == 10 ? 24 : 25) : CHIP_PORT_UNUSED);
            map->miim_controller = (port_no == 0 ?
                                    VTSS_MIIM_CONTROLLER_1 : VTSS_MIIM_CONTROLLER_NONE);
            map->miim_addr = map->chip_port;
        }
        if (port_no > 1 && conf->host_mode > 5) {
            /* Port 3-24 are unused */
            map->chip_port = CHIP_PORT_UNUSED;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }

        if (port_no > 23 && conf->host_mode == 4) {
            /* XAUI ports 24-25 are unused */
            map->chip_port = CHIP_PORT_UNUSED;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }

        if (port_no == 25 &&
                (conf->host_mode == VTSS_HOST_MODE_0 ||
                 conf->host_mode == VTSS_HOST_MODE_1)) {
            /* Only one XAUI port in host mode 0 and 1 */
            map->chip_port = CHIP_PORT_UNUSED;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }

        if (port_no > 15 && sfp) {
            /* Port 17-20 are SFP and port 21-24 are unused */
            if (port_no > 19)
                map->chip_port = CHIP_PORT_UNUSED;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }

        if (port_no >= port_max)
            map->chip_port = CHIP_PORT_UNUSED;

#if defined(VTSS_CHIP_SCHAUMBURG_II)
        if (port_no > 11)
            map->chip_port = CHIP_PORT_UNUSED;
#endif

        /* Host ports are the last ports in the portmap */
        if ((map->chip_port == CHIP_PORT_UNUSED || port_no >= 24) && !done) {
            /*  XAUI_0, XAUI_1 or both are hosts */
            if (conf->host_mode < 4) {
                if (conf->host_mode == VTSS_HOST_MODE_0) {
                    map->chip_port = 24;
                    done = TRUE;
                } else if (conf->host_mode == VTSS_HOST_MODE_1) {
                    map->chip_port = 25;
                    done = TRUE;
                } else if (conf->host_mode == 3) {
                    map->chip_port = 24+cnt;
                    cnt > 0 ? done = TRUE : cnt++;
                }
                map->miim_controller = VTSS_MIIM_CONTROLLER_1;
//                map->miim_addr = map->chip_port;
                map->miim_addr = 0;
            } else {
                /* SPI4 host */
                map->chip_port = 26;
                map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
                done = TRUE;
            }
            map->lport_no = VTSS_LPORT_MAP_NONE;
        }
    }
    printf("Initializing chip, host mode: %u\n", conf->host_mode);
#endif
    return 1;
}
/* ================================================================= *
 *  Board init.
 * ================================================================= */
#include <fcntl.h>
#include <signal.h>

static void daytona_board_init(void)
{
    /* initialize daytona chip */
#if !defined(DAYTONA_CHIP_SIMULATE)
    struct vitgenio_cs_setup_timing timing;
    struct vitgenio_16bit_readwrite pibuf;
    u16                             value = 0;
    int                             oflags;
    u32                             fast_regs;

    /* Open driver */
    if ((pi_fd = open("/dev/vitgenio", 0)) < 0) {
        T_E ("PI open call failed\n");
        exit(1);
    } 

    /* Open interrupt driver */
    if ((interrupt_fd = open("/dev/hello", O_RDWR)) < 0) {
        T_E ("Interrupt open call failed\n");
        exit(1);
    } 
    signal(SIGIO, daytona_interrupt); 
    fcntl(interrupt_fd, F_SETOWN, getpid());
    oflags = fcntl(interrupt_fd, F_GETFL);
    fcntl(interrupt_fd, F_SETFL, oflags | FASYNC);
    write(interrupt_fd, "", 0);

    /* Setup Parallel Interface timing */
    timing.cs = VITGENIO_CS3;
    timing.bw = 16; /* Bus Width: bme=0,1,2 (aliases 8,16,32 accepted) */
    timing.bme = 0; /* Burst Mode Enable. */
    timing.twt = 255; /* Transfer Wait. Only used when bme=0. (255 is max) (40 is previous value) */
    timing.fwt = 0; /* First Wait. Only used when bme=1. */
    timing.bwt = 0; /* Burst Wait. Only used when bme=1. */
    timing.csn = 1; /* Chip Select */
    timing.oen = 1; /* Output Enable */
    timing.wbn = 1; /* Write Byte Enable On */
    timing.wbf = 1; /* Write Byte Enable Off */
    timing.th = 7;  /* Transfer Hold (7 is max) (2 is used value) */
    timing.re = 0;  /* Ready Enable */
    timing.sor = 0; /* Sample On Ready */
    timing.bem = 1; /* Byte Enable Mode */
    timing.pen = 0; /* Parity Enable */

    if (ioctl(pi_fd, VITGENIO_CS_SETUP_TIMING, &timing) < 0) {
        T_E("Failure in PI timing setups\n");
    }
    T_D("Using CS%d", timing.cs);
    if (ioctl(pi_fd, VITGENIO_CS_SELECT, timing.cs) < 0) {
        T_E("Failure in PI chip selection\n");
    } 

    /* Control the PI_HOLD and PI_WAIT */
    vtss_fast_regs_conf_get(NULL, &fast_regs);  
    pibuf.offset = ((fast_regs << 16) + (0x3 << 1)) >> 1;
    pibuf.value  = 0x00000778;

    if (ioctl(pi_fd,VITGENIO_16BIT_WRITE,&pibuf) < 0) {
        T_E ("PI write failed..%u\n", value);
        value=0;
    } 
#endif /* !DAYTONA_CHIP_SIMULATE */

}


#if defined(VTSS_ARCH_DAYTONA)
static int board_init(int argc, const char **argv, vtss_board_t *board)
{
    vtss_init_t      *board_conf = &board->init;
    vtss_init_conf_t *init_conf = board->init.init_conf;
    vtss_port_no_t   port_no;
    static BOOL first_time = TRUE;

    if (daytona_parse_options(argc, argv, board_conf) == 0)
        return 1;

    if (first_time) {
        daytona_board_init();

        init_conf->reg_read = reg_read;
        init_conf->reg_write = reg_write;
        first_time = FALSE;
    } else {
        init_conf->reg_read = reg_read_ch2_3;
        init_conf->reg_write = reg_write_ch2_3;
    }
    board->port_map = board_port_map;
    board->port_interface = port_interface;

    /* Update port count */
    board->port_count = VTSS_PORTS;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (board_port_map[port_no].chip_port == CHIP_PORT_UNUSED) {
            board->port_count = port_no;
            break;
        }
    }
    return 0;
}
#endif /* VTSS_ARCH_DAYTONA */

void vtss_board_daytona_init(vtss_board_t *board)
{

#if defined(VTSS_ARCH_DAYTONA)

    board->descr = "Daytona";
    board->target = VTSS_TARGET_DAYTONA;
    board->feature.port_control = 1;
    board->daytona = 1;
#endif /* VTSS_ARCH_DAYTONA */
    board->board_init = board_init;
}

#if defined(BOARD_DAYTONA_EVAL)
/* Execute Rabbit direct read/write command */
vtss_rc vtss_rabbit_rd_wr_fpga(u32 addr, u32 *value, BOOL read)
{
    char  cmd[64], data[9];

    if (read)
        data[0] = '\0';
    else
        sprintf(data, "%08x", *value);
    sprintf(cmd, "%s%08x%s%s",
            read ? "WPRP 0x2018 0x" : "WP 0x2000 0x",
            0x80000000 + addr,
            data,
            read ? "03" : "02");

    return vtss_rabbit_cmd(cmd, value);
}
#endif /* BOARD_DAYTONA_EVAL */

#ifdef VTSS_SW_OPTION_REMOTE_TS_PHY 

/***************************************************************************/
/***************************************************************************/
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
/* These APIs are defined to debug PHY Timestamp features, not intend
   to expose to API customers and hence move to API base folder;
   extern here to access thru Debug CLI */
extern vtss_rc vtss_phy_1588_csr_reg_write(const vtss_inst_t inst,
                                           const vtss_port_no_t port_no,
                                           const u32 blk_id,
                                           const u16 csr_address,
                                           const u32 *const value);
extern vtss_rc vtss_phy_1588_csr_reg_read(const vtss_inst_t inst,
                                          const vtss_port_no_t port_no,
                                          const u32 blk_id,
                                          const u16 csr_address,
                                          u32 *const value);
#endif

static void print_time(vtss_phy_timestamp_t *remote_phy_ts)
{
   printf("Seconds:  High = %u  Low = %u    Nano Seconds: %u \n", (unsigned int)remote_phy_ts->seconds.high, (unsigned int)remote_phy_ts->seconds.low, (unsigned int)remote_phy_ts->nanoseconds); 
}
static void print_eth_flow_conf(vtss_phy_ts_eth_conf_t *eth_flow_conf)
{
        int k,l;
        printf("                  Common Options\n");
        printf("                     Pbb_En = %s\n", (eth_flow_conf->comm_opt.pbb_en == TRUE) ? "Yes" : "No");
        printf("                     Etype  = %X\n", eth_flow_conf->comm_opt.etype);
        printf("                     tpid   = %X\n", eth_flow_conf->comm_opt.tpid);
        printf("                     flow_en = %X\n", eth_flow_conf->comm_opt.tpid);
        printf("                  Specific Options for each flow.\n");
        for(k = 0; k < 8; k++) {
                printf("                        FlowId  %d\n", k);
                printf("                        Enabled..? %d\n", eth_flow_conf->flow_opt[k].flow_en);
//                if(eth_flow_conf->flow_opt[k].flow_en) {
                        printf("                        Addr_match_mode %d", eth_flow_conf->flow_opt[k].addr_match_mode);
                        printf("                        Addr_match_select %d", eth_flow_conf->flow_opt[k].addr_match_select);
                        printf("                        Mac_addr::");
                        for(l = 0; l < 6; l++) printf("%.2X  ", eth_flow_conf->flow_opt[k].mac_addr[l]);
                        printf("\n");
                        printf("                        vlan_check..?  %d\n", eth_flow_conf->flow_opt[k].vlan_check);
                        printf("                        Num_tag  %d\n", eth_flow_conf->flow_opt[k].num_tag);
                        printf("                        outer_tag_type %d\n", eth_flow_conf->flow_opt[k].outer_tag_type);
                        printf("                        Inner_tag_type %d\n", eth_flow_conf->flow_opt[k].inner_tag_type);
                        printf("                        tag_range_mode %d\n", eth_flow_conf->flow_opt[k].tag_range_mode);
                        printf("                        Outer Tag\n");
                        printf("                        Range::: Upper :: %d Lower :: %d\n", eth_flow_conf->flow_opt[k].outer_tag.range.upper,eth_flow_conf->flow_opt[k].outer_tag.range.lower);
                        printf("                        Value::: val :: %d Mask :: %d\n", eth_flow_conf->flow_opt[k].outer_tag.value.val,eth_flow_conf->flow_opt[k].outer_tag.value.mask);
                        printf("                        Inner Tag\n");
                        printf("                        Range::: Upper :: %d Lower :: %d\n", eth_flow_conf->flow_opt[k].inner_tag.range.upper,eth_flow_conf->flow_opt[k].inner_tag.range.lower);
                        printf("                        Value::: val :: %d Mask :: %d\n", eth_flow_conf->flow_opt[k].inner_tag.value.val,eth_flow_conf->flow_opt[k].inner_tag.value.mask);
                        printf("                        I-Tag Value::: val :: %u Mask :: %u\n", (unsigned int)eth_flow_conf->flow_opt[k].inner_tag.i_tag.val, (unsigned int)eth_flow_conf->flow_opt[k].inner_tag.i_tag.mask);

//                }
        }
}
#if 0
static void print_ip_flow_conf(vtss_phy_ts_ip_conf_t *ip_flow_conf)
{
        int l,k;
        printf("                   Common Options::\n");
        printf("                     ip_mode  %u\n", ip_flow_conf->comm_opt.ip_mode);
        printf("                     sport_val %u\n", ip_flow_conf->comm_opt.sport_val);
        printf("                     sport_mask %u\n", ip_flow_conf->comm_opt.sport_mask);
        printf("                     dport_val  %u\n", ip_flow_conf->comm_opt.dport_val);
        printf("                     dport_mask %u\n", ip_flow_conf->comm_opt.dport_mask);
        printf("                   Flow Options::\n");
        for(k = 0; k < 8; k++) {
                printf("                        FlowId  %d\n", k);
                printf("                        flow_en...?  %s\n", (ip_flow_conf->flow_opt[k].flow_en == TRUE) ? "Yes" : "No");
//                if(ip_flow_conf->flow_opt[k].flow_en == TRUE) {
                        printf("                        match_mode %u\n", ip_flow_conf->flow_opt[k].match_mode);
                        printf("                        ip_addr values::\n");
                        printf("                          ipv4: addr =  %u   mask = %u \n", (unsigned int)ip_flow_conf->flow_opt[k].ip_addr.ipv4.addr, (unsigned int)ip_flow_conf->flow_opt[k].ip_addr.ipv4.mask);
                        printf("                          ipv6: addr  ");
                        for(l = 0; l < 4; l++) printf("%u ", (unsigned int)ip_flow_conf->flow_opt[k].ip_addr.ipv6.addr[l]);
                        printf("  Mask  ");
                        for(l = 0; l < 4; l++) printf("%u ", (unsigned int)ip_flow_conf->flow_opt[k].ip_addr.ipv6.mask[l]);
//                }
        }

}

static void print_mpls_flow_conf(vtss_phy_ts_mpls_conf_t *mpls_flow)
{
        int k;
        printf("                     Common Option ::  cw_en  %s\n",  (mpls_flow->comm_opt.cw_en == TRUE) ? "Yes" : "No");
        printf("                     Each Flow Options:\n");
        for(k = 0; k < 8; k++) {
                printf("                        FlowId  %d\n", k);
                printf("                          flow_en...?  %s\n", (mpls_flow->flow_opt[k].flow_en == TRUE) ? "Yes" : "No");
//                if(mpls_flow->flow_opt[k].flow_en == TRUE) {
                        printf("                          stack_depth  %u\n", mpls_flow->flow_opt[k].stack_depth);
                        printf("                          stack_ref_point  %u\n", mpls_flow->flow_opt[k].stack_ref_point);
                        printf("                          stack_level values::\n");
                        printf("                          top_down:: top: lower %u  upper %u frst_lvl_after_top: lower %u  upper %u  snd_lvl_before_end lower %u  upper %u  thrd_lvl_after_top:   lower %u  upper %u \n",  (unsigned int)mpls_flow->flow_opt[k].stack_level.top_down.top.lower, (unsigned int)mpls_flow->flow_opt[k].stack_level.top_down.top.upper, (unsigned int)mpls_flow->flow_opt[k].stack_level.top_down.frst_lvl_after_top.lower, (unsigned int)mpls_flow->flow_opt[k].stack_level.top_down.frst_lvl_after_top.upper, (unsigned int)mpls_flow->flow_opt[k].stack_level.top_down.snd_lvl_after_top.lower, (unsigned int)mpls_flow->flow_opt[k].stack_level.top_down.snd_lvl_after_top.upper,  (unsigned int)mpls_flow->flow_opt[k].stack_level.top_down.thrd_lvl_after_top.lower, (unsigned int)mpls_flow->flow_opt[k].stack_level.top_down.thrd_lvl_after_top.upper);
                        printf("                          bottom_up:: end: lower %u  upper %u  frst_lvl_before_end: lower %u  upper %u snd_lvl_before_end: lower %u  upper %u  thrd_lvl_before_end:  lower %u  upper %u \n",  (unsigned int)mpls_flow->flow_opt[k].stack_level.bottom_up.end.lower, (unsigned int)mpls_flow->flow_opt[k].stack_level.bottom_up.end.upper,  (unsigned int)mpls_flow->flow_opt[k].stack_level.bottom_up.frst_lvl_before_end.lower, (unsigned int)mpls_flow->flow_opt[k].stack_level.bottom_up.frst_lvl_before_end.upper, (unsigned int)mpls_flow->flow_opt[k].stack_level.bottom_up.snd_lvl_before_end.lower, (unsigned int)mpls_flow->flow_opt[k].stack_level.bottom_up.snd_lvl_before_end.upper, (unsigned int)mpls_flow->flow_opt[k].stack_level.bottom_up.thrd_lvl_before_end.lower, (unsigned int)mpls_flow->flow_opt[k].stack_level.bottom_up.thrd_lvl_before_end.upper);

//             }
       }
}
#endif

static void print_ptp_action_conf(vtss_phy_ts_ptp_engine_action_t *ptp_action_conf)
{
    printf("              Action Enabled     %s\n", (ptp_action_conf->enable == TRUE) ? "YES" : "NO");
//    if (ptp_action_conf->enable == TRUE) {
        printf("              channel_map     %u\n", ptp_action_conf->channel_map);
        printf("              ptp_conf::\n");
        printf("                 Range Enabled...?  %s\n", (ptp_action_conf->ptp_conf.range_en == TRUE) ? "Yes" : "No");
        if(ptp_action_conf->ptp_conf.range_en == TRUE)
           printf("                 Range: upper  %u  lower  %u\n", ptp_action_conf->ptp_conf.domain.range.upper, ptp_action_conf->ptp_conf.domain.range.lower);
        else
           printf("                 Domain:  val  %u  mask %u  \n", ptp_action_conf->ptp_conf.domain.value.val, ptp_action_conf->ptp_conf.domain.value.mask);
        printf("                 Clock_mode   %d\n", ptp_action_conf->clk_mode);
        printf("                 delaym_type   %s\n", (ptp_action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) ? "VTSS_PHY_TS_PTP_DELAYM_P2P" : "VTSS_PHY_TS_PTP_DELAYM_E2E");
//    }
}
#if 0
static void print_oam_action_conf(vtss_phy_ts_oam_engine_action_t *oam_action_conf)
{
    printf("              Action Enabled     %s\n", (oam_action_conf->enable == TRUE) ? "YES" : "NO");
//    if (oam_action_conf->enable == TRUE) {
       printf("              Y.1731 Message Format Enabled..?     %s\n", (oam_action_conf->y1731_en == TRUE) ? "YES" : "NO");
       printf("              channel_map    %u\n", oam_action_conf->channel_map);
       printf("              version    %u\n", oam_action_conf->version);
       if(oam_action_conf->y1731_en == TRUE) {
           printf("              Y.1731 Oam_conf:\n");
           printf("                  range_en ...?  %s\n", (oam_action_conf->oam_conf.y1731_oam_conf.range_en == TRUE) ? "Yes" : "No");;
           printf("                  delaym_type    %s\n", (oam_action_conf->oam_conf.y1731_oam_conf.delaym_type == VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM)? "VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM" : "VTSS_PHY_TS_Y1731_OAM_DELAYM_DMM");
           printf("                  OAM MEG level config::\n");
           if(oam_action_conf->oam_conf.y1731_oam_conf.range_en == TRUE) {
               printf("                   range: upper  %u  lower %u  \n", oam_action_conf->oam_conf.y1731_oam_conf.meg_level.range.upper, oam_action_conf->oam_conf.y1731_oam_conf.meg_level.range.lower);
           } else {
               printf("                   value: val  %u  mask  %u  \n", oam_action_conf->oam_conf.y1731_oam_conf.meg_level.value.val, oam_action_conf->oam_conf.y1731_oam_conf.meg_level.value.mask);
           }
       } else {
           printf("                  IETF OAM configuration::\n");
           printf("                      delaym_type  %s\n", (oam_action_conf->oam_conf.ietf_oam_conf.delaym_type == VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_DMM) ? "VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_DMM": "VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_LDM");
           printf("                 ts_format(Hex) %x\n", oam_action_conf->oam_conf.ietf_oam_conf.ts_format);
           printf("                 DSCP Value     %u\n", oam_action_conf->oam_conf.ietf_oam_conf.ds);
       }
//    }
}
#endif

static void print_flow_conf(vtss_phy_ts_engine_flow_conf_t* flow_conf)
{
   int j;

//   if(flow_conf->eng_mode) {
       printf("          Channel Maps");
       for (j = 0; j < 8; j++) printf("%d",flow_conf->channel_map[j]);
       /* PTP Comparators */
           printf("         FLOW CONFIGURATION\n\n         PTP Flow is configuration.. Following is its configuration..\n");
           printf("                ::Eth1 Comparator Values::");
           print_eth_flow_conf(&flow_conf->flow_conf.ptp.eth1_opt);
           printf("\n");
//           printf("                ::Eth2 Comparator Values::");
//           print_eth_flow_conf(&flow_conf->flow_conf.ptp.eth2_opt);
//       printf("\n");
//       printf("                ::IP1 Comparator Values::");
//       print_ip_flow_conf(&flow_conf->flow_conf.ptp.ip1_opt);
//       printf("\n");

//       printf("                ::IP2 Comparator Values::");
//       print_ip_flow_conf(&flow_conf->flow_conf.ptp.ip1_opt);
//       printf("\n");

//     printf("                ::MPLS comparator values:");
     //print_mpls_flow_conf(&flow_conf->flow_conf.ptp.mpls_opt);
//     printf("\n");
//     printf("                ::ACH comparator values:");
//     printf("                    Common Options:\n");
//     printf("                        version : value  %u  mask  %u\n", flow_conf->flow_conf.ptp.ach_opt.comm_opt.version.value, flow_conf->flow_conf.ptp.ach_opt.comm_opt.version.mask);
//     printf("                        channel_type : value  %u  mask  %u\n", flow_conf->flow_conf.ptp.ach_opt.comm_opt.channel_type.value, flow_conf->flow_conf.ptp.ach_opt.comm_opt.channel_type.mask);
//     printf("                        proto_id : value  %u  mask  %u\n", flow_conf->flow_conf.ptp.ach_opt.comm_opt.proto_id.value, flow_conf->flow_conf.ptp.ach_opt.comm_opt.proto_id.mask);

//   printf("End of PTP Comparators \n\n");


   /* PTP Comparators */

            /* OAM Comparators */
//           printf("         OAM Flow configuration.. Following is its configuration..\n");
//           printf("                ::Eth1 Comparator Values::");
//           print_eth_flow_conf(&flow_conf->flow_conf.oam.eth1_opt);
//           printf("\n");
//
//           printf("                ::Eth2 Comparator Values::");
//           print_eth_flow_conf(&flow_conf->flow_conf.oam.eth2_opt);
//           printf("\n");
//           printf("                ::MPLS comparator values:");
//           print_mpls_flow_conf(&flow_conf->flow_conf.oam.mpls_opt);
//   }
}

static void print_action_conf(vtss_phy_ts_engine_action_t *action_conf)
{
   int j;

    /*Action Configurastion*/
    printf("         ACTION CONFIGURATION\n\n");
        for(j = 0; j < 1; j++) {
            print_ptp_action_conf(&action_conf->action.ptp_conf[j]);
        }
//        for(j = 0; j < 6; j++) {
//            print_oam_action_conf(&action_conf->action.oam_conf[j]);
//        }

}

vtss_rc vtss_daytona_remote_phy_ts_rx_parse(remote_ts_msg_header_t *msg_hdr, u8 *data)
{
    vtss_rc rc = VTSS_RC_OK;
    vtss_timeinterval_t            remote_latency;
    BOOL                           remote_phy_mode; 
    vtss_phy_ts_fifo_sig_mask_t    remote_sig_mask;
    vtss_phy_timestamp_t           remote_phy_ts;
    vtss_phy_ts_scaled_ppb_t       remote_clk_rate;
    BOOL                           remote_rate_adj_flag;
//    vtss_phy_ts_eng_init_conf_t    remote_eng_init_conf;
    vtss_phy_ts_engine_flow_conf_t remote_flow_conf; 
    vtss_phy_ts_engine_t           remote_eng_id;
    engine_init_t                  eng_init_conf;
    vtss_phy_ts_engine_action_t    remote_action_conf;
    event_conf_t                   remote_event_conf;
    vtss_phy_ts_stats_t            remote_phy_stats;
    BOOL                           ingr_overflow;
    BOOL                           egr_overflow;
    csr_reg_t                      reg_conf;

    //printf("Inside FN :: %s command :: %d \n\r",__FUNCTION__, msg_hdr->command);
    switch(msg_hdr->command) {
        case REMOTE_TS_PHY_COMMAND_MODE_SET               : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            remote_phy_mode = *((BOOL *)data);
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_MODE_SET    %s\n", (remote_phy_mode == TRUE) ? "TRUE" : "FALSE");

            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_mode_set(NULL, msg_hdr->port_no, remote_phy_mode);

            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_MODE_GET               : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            /* Print the values */
            remote_phy_mode = FALSE;
            //printf("REMOTE_TS_PHY_COMMAND_MODE_GET    %llu\n", remote_latency);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_mode_get(NULL, msg_hdr->port_no, &remote_phy_mode);
            /* update the return value and data_length in msg_hdr */
            memcpy(data, &remote_phy_mode, sizeof(BOOL));

            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(BOOL);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_DELAY_METHOD_SET       : 
            break;
        case REMOTE_TS_PHY_COMMAND_DELAY_METHOD_GET       : 
            break;
        case REMOTE_TS_PHY_COMMAND_INGRESS_LATENCY_SET    : 

            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_latency, 0, sizeof(vtss_timeinterval_t));
            memcpy(&remote_latency, data, sizeof(vtss_timeinterval_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_INGRESS_LATENCY_SET   %llu port_no : %d \n", remote_latency, msg_hdr->port_no);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ingress_latency_set(NULL, msg_hdr->port_no, &remote_latency);
            /* update the return value and data_length in msg_hdr */            
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE); 
            msg_hdr->length = sizeof(remote_ts_msg_header_t); 
            break;
        case REMOTE_TS_PHY_COMMAND_INGRESS_LATENCY_GET    : 
            //printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
            memset(&remote_latency, 0, sizeof(vtss_timeinterval_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_INGRESS_LATENCY_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ingress_latency_get(NULL, msg_hdr->port_no, &remote_latency);
            //printf("REMOTE_TS_PHY_COMMAND_INGRESS_LATENCY_GET   %llu port_no : %d \n", remote_latency, msg_hdr->port_no);
            /* update the return value and data_length in msg_hdr */
            memcpy(data, &remote_latency, sizeof(vtss_timeinterval_t));
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = msg_hdr->length + sizeof(vtss_timeinterval_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_EGRESS_LATENCY_SET     : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_latency, 0, sizeof(vtss_timeinterval_t));
            memcpy(&remote_latency, data, sizeof(vtss_timeinterval_t));
            /* Print the values */
  //          printf("REMOTE_TS_PHY_COMMAND_EGRESS_LATENCY_SET   %llu\n", remote_latency);
            //printf("REMOTE_TS_PHY_COMMAND_EGRESS_LATENCY_SET   %llu port_no : %d \n", remote_latency, msg_hdr->port_no);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_egress_latency_set(NULL, msg_hdr->port_no, &remote_latency);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t); 
            break;
        case REMOTE_TS_PHY_COMMAND_EGRESS_LATENCY_GET     : 
    //        printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
            memset(&remote_latency, 0, sizeof(vtss_timeinterval_t));
            /* Print the values */
      //      printf("REMOTE_TS_PHY_COMMAND_EGRESS_LATENCY_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_egress_latency_get(NULL, msg_hdr->port_no, &remote_latency);
            //printf("REMOTE_TS_PHY_COMMAND_EGRESS_LATENCY_GET   %llu port_no : %d \n", remote_latency, msg_hdr->port_no);
            /* update the return value and data_length in msg_hdr */
            memcpy(data, &remote_latency, sizeof(vtss_timeinterval_t));
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_timeinterval_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_DELAY_ASYMMETRY_SET    : 
        //    printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_latency, 0, sizeof(vtss_timeinterval_t));
            memcpy(&remote_latency, data, sizeof(vtss_timeinterval_t));
            /* Print the values */
          //  printf("REMOTE_TS_PHY_COMMAND_DELAY_ASYMMETRY_SET   %llu\n", remote_latency);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_delay_asymmetry_set(NULL, msg_hdr->port_no, &remote_latency);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);

            break;
        case REMOTE_TS_PHY_COMMAND_DELAY_ASYMMETRY_GET    : 
            //printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
            memset(&remote_latency, 0, sizeof(vtss_timeinterval_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_DELAY_ASYMMETRY_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_delay_asymmetry_get(NULL, msg_hdr->port_no, &remote_latency);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            memcpy(data, &remote_latency, sizeof(vtss_timeinterval_t));
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_timeinterval_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_PATH_DELAY_SET         : 
  //          printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
            memset(&remote_latency, 0, sizeof(vtss_timeinterval_t));
            memcpy(&remote_latency, data, sizeof(vtss_timeinterval_t));
            /* Print the values */
    //        printf("REMOTE_TS_PHY_COMMAND_PATH_DELAY_SET  %llu\n", remote_latency);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_path_delay_set(NULL, msg_hdr->port_no, &remote_latency);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;
        case REMOTE_TS_PHY_COMMAND_PATH_DELAY_GET         : 
      //      printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
            memset(&remote_latency, 0, sizeof(vtss_timeinterval_t));
            /* Print the values */
        //    printf("REMOTE_TS_PHY_COMMAND_PATH_DELAY_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_path_delay_get(NULL, msg_hdr->port_no, &remote_latency);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            memcpy(data, &remote_latency, sizeof(vtss_timeinterval_t));
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_timeinterval_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_SIGNATURE_MASK_SET     : 
          //  printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_sig_mask, 0, sizeof(vtss_phy_ts_fifo_sig_mask_t));
            memcpy(&remote_sig_mask, data, sizeof(vtss_phy_ts_fifo_sig_mask_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_DELAY_ASYMMETRY_SET   %llu\n", remote_latency);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_fifo_sig_set(NULL, msg_hdr->port_no, remote_sig_mask);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_SIGNATURE_MASK_GET     : 
//            printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
            memset(&remote_sig_mask, 0, sizeof(vtss_phy_ts_fifo_sig_mask_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_PATH_DELAY_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_fifo_sig_get(NULL, msg_hdr->port_no, &remote_sig_mask);
            /* update the return value and data_length in msg_hdr */
            memcpy(data, &remote_sig_mask, sizeof(vtss_phy_ts_fifo_sig_mask_t));
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_fifo_sig_mask_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_TS_FIFO_EMPTY          : 
//            printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
//            printf("REMOTE_TS_PHY_COMMAND_TS_FIFO_EMPTY:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_fifo_empty(NULL, msg_hdr->port_no);
            /* update the return value and data_length in msg_hdr */
            break;
        case REMOTE_TS_PHY_COMMAND_TX_TIMESTAMP_GET       : 
            
            break;
        case REMOTE_TS_PHY_COMMAND_TX_TIMESTAMP_SPI       : 
            break;
        case REMOTE_TS_PHY_COMMAND_TX_TIMESTAMP_INDICATION: 
            break;
        case REMOTE_TS_PHY_COMMAND_NS_TO_TIMESTAMP_GET    : 
            break;
        case REMOTE_TS_PHY_COMMAND_PTP_TIME_SET           : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_phy_ts, 0, sizeof(vtss_phy_timestamp_t));
            memcpy(&remote_phy_ts, data, sizeof(vtss_phy_timestamp_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_PTP_TIME_SET: port_no : %d \n", msg_hdr->port_no);
//            print_time(&remote_phy_ts);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ptptime_set(NULL, msg_hdr->port_no, &remote_phy_ts);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_PTP_TIME_SET_DONE      : 
//            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_PTP_TIME_SET_DONE:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ptptime_set_done(NULL, msg_hdr->port_no);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_PTP_TIME_ARM           : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_PTP_TIME_ARM:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ptptime_arm(NULL, msg_hdr->port_no);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_PTP_TIME_GET           : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_phy_ts, 0, sizeof(vtss_phy_timestamp_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_PTP_TIME_GET: port_no : %d \n", msg_hdr->port_no);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ptptime_get(NULL, msg_hdr->port_no, &remote_phy_ts);

//            print_time(&remote_phy_ts);
            /* update the return value and data_length in msg_hdr */
            memcpy(data, &remote_phy_ts, sizeof(vtss_phy_timestamp_t));
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_timestamp_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_SET       : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_clk_rate, 0, sizeof(vtss_phy_ts_scaled_ppb_t));
            memcpy(&remote_clk_rate, data, sizeof(vtss_phy_ts_scaled_ppb_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_SET: %llu\n", (unsigned long long)remote_clk_rate);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_clock_rateadj_set(NULL, msg_hdr->port_no, &remote_clk_rate);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_GET       : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_clk_rate, 0, sizeof(vtss_phy_ts_scaled_ppb_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_clock_rateadj_get(NULL, msg_hdr->port_no, &remote_clk_rate);
            /* update the return value and data_length in msg_hdr */
            memcpy(data, &remote_clk_rate, sizeof(vtss_phy_ts_scaled_ppb_t));
//            printf("REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_GET: %llu\n", (unsigned long long)remote_clk_rate);
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_scaled_ppb_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_1NS       : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            remote_rate_adj_flag = *((BOOL *) data);
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_1NS: %d\n", remote_rate_adj_flag);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ptptime_adj1ns(NULL, msg_hdr->port_no, remote_rate_adj_flag);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_INGR_ENG_INIT          : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            /* Print the values */
  /*          printf("REMOTE_TS_PHY_COMMAND_INGR_ENG_INIT:\n");
            printf("Eng_id = %u encap_type = %u flow_st_index = %u flow_end_index = %u flow_match_mode %d", eng_init_conf.eng_id,
                                                                                        eng_init_conf.encap_type,
                                                                                        eng_init_conf.flow_st_index,
                                                                                        eng_init_conf.flow_end_index,
                                                                                        eng_init_conf.flow_match_mode);
  */
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ingress_engine_init(NULL, msg_hdr->port_no, 
                                                 eng_init_conf.eng_id,
                                                 eng_init_conf.encap_type,
                                                 eng_init_conf.flow_st_index,
                                                 eng_init_conf.flow_end_index,
                                                 eng_init_conf.flow_match_mode);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_INGR_ENG_INIT_CONF_GET : 
#if 0
            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_eng_init_conf, 0, sizeof(vtss_phy_ts_eng_init_conf_t));
            remote_eng_id = *((vtss_phy_ts_engine_t *)data);
            /* Print the values */
            printf("REMOTE_TS_PHY_COMMAND_INGR_ENG_INIT_CONF_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ingress_engine_init_conf_get(NULL, msg_hdr->port_no, remote_eng_id, &remote_eng_init_conf); 
            memcpy(data, &remote_eng_init_conf, sizeof(vtss_phy_ts_eng_init_conf_t));
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(_phy_ts_eng_init_conf_t	ss_phy_ts_eng_init_conf_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }

#endif /* 0 */
            break;
        case REMOTE_TS_PHY_COMMAND_INGR_ENG_CLEAR         : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));

            remote_eng_id = eng_init_conf.eng_id;
//             printf("REMOTE_TS_PHY_COMMAND_INGR_ENG_CLEAR:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ingress_engine_clear(NULL, msg_hdr->port_no, remote_eng_id);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_EGR_ENG_INIT           : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_INGR_ENG_INIT:\n");
            printf("Eng_id = %u encap_type = %u flow_st_index = %u flow_end_index - [%d] flow_match_mode %d", eng_init_conf.eng_id,
                                                                                        eng_init_conf.encap_type,
                                                                                        eng_init_conf.flow_st_index,
                                                                                        eng_init_conf.flow_end_index,
                                                                                        eng_init_conf.flow_match_mode);
                                       
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_egress_engine_init(NULL, msg_hdr->port_no,
                                                 eng_init_conf.eng_id,
                                                 eng_init_conf.encap_type,
                                                 eng_init_conf.flow_st_index,
                                                 eng_init_conf.flow_end_index,
                                                 eng_init_conf.flow_match_mode);
//            printf("Line : %d rc - [%d] \n\r",__LINE__, rc);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_EGR_ENG_INIT_CONF_GET  : 
#if 0
            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_eng_init_conf, 0, sizeof(vtss_phy_ts_eng_init_conf_t));
            remote_eng_id = *((vtss_phy_ts_engine_t *)data);
            /* Print the values */
            printf("REMOTE_TS_PHY_COMMAND_EGR_ENG_INIT_CONF_GET:\n");
            /* Call the vtss_phy_ts_API */
#if 0
            rc = vtss_phy_ts_egress_engine_init_conf_get(NULL, msg_hdr->port_no, remote_eng_id, &remote_eng_init_conf);
#endif
            /* update the return value and data_length in msg_hdr */
            memcpy(data, &remote_eng_init_conf, sizeof(vtss_phy_ts_eng_init_conf_t));
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_eng_init_conf_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
#endif /* 0 */
            break;
        case REMOTE_TS_PHY_COMMAND_EGR_ENG_CLEAR          :
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));        
            remote_eng_id = eng_init_conf.eng_id;
//            printf("REMOTE_TS_PHY_COMMAND_EGR_ENG_CLEAR: eng_id %u\n", remote_eng_id);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_egress_engine_clear(NULL, msg_hdr->port_no, remote_eng_id);
//            printf("Line : %d rc - [%d] \n\r",__LINE__, rc);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_INGR_ENG_CONF_SET      : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            remote_eng_id = eng_init_conf.eng_id;
            memset(&remote_flow_conf, 0, sizeof(vtss_phy_ts_engine_flow_conf_t));
            memcpy(&remote_flow_conf, data + sizeof(engine_init_t), sizeof(vtss_phy_ts_engine_flow_conf_t));
            /* Print the values */
            printf("REMOTE_TS_PHY_COMMAND_INGR_ENG_CONF_SET:\n");
            printf("Recieved: eng_id  %u  encap_type %u flow_st_index %u  flow_end_index  %u flow_match_mode  %u\n", eng_init_conf.eng_id,  eng_init_conf.encap_type, eng_init_conf.flow_st_index, eng_init_conf.flow_end_index , eng_init_conf.flow_match_mode);

//            print_flow_conf(&remote_flow_conf);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ingress_engine_conf_set(NULL, msg_hdr->port_no, remote_eng_id, &remote_flow_conf); 
//            printf("Line : %d rc - [%d] \n\r",__LINE__, rc);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_INGR_ENG_CONF_GET      : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            remote_eng_id = eng_init_conf.eng_id;
            memset(&remote_flow_conf, 0, sizeof(vtss_phy_ts_engine_flow_conf_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_INGR_ENG_CONF_GET:\n");
 printf("Recieved: eng_id  %u  encap_type %u flow_st_index %u  flow_end_index  %u flow_match_mode  %u\n", eng_init_conf.eng_id,  eng_init_conf.encap_type, eng_init_conf.flow_st_index, eng_init_conf.flow_end_index , eng_init_conf.flow_match_mode);

            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ingress_engine_conf_get(NULL, msg_hdr->port_no, remote_eng_id, &remote_flow_conf);
//            printf("Line : %d rc - [%d] \n\r",__LINE__, rc);
//            print_flow_conf(&remote_flow_conf);
            memcpy(data, &remote_flow_conf, sizeof(vtss_phy_ts_engine_flow_conf_t));
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_engine_flow_conf_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_EGR_ENG_CONF_SET       : 
//            printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            remote_eng_id = eng_init_conf.eng_id;
//            printf("REMOTE_TS_PHY_COMMAND_EGR_ENG_CONF_SET:\n");
//            printf("Recieved: eng_id  %u  encap_type %u flow_st_index %u  flow_end_index  %u flow_match_mode  %u\n", eng_init_conf.eng_id,  eng_init_conf.encap_type, eng_init_conf.flow_st_index, eng_init_conf.flow_end_index , eng_init_conf.flow_match_mode);
            memset(&remote_flow_conf, 0, sizeof(vtss_phy_ts_engine_flow_conf_t));
            memcpy(&remote_flow_conf, data + sizeof(engine_init_t), sizeof(vtss_phy_ts_engine_flow_conf_t));
            /* Print the values */
            printf("REMOTE_TS_PHY_COMMAND_EGR_ENG_CONF_SET:\n");
//            print_flow_conf(&remote_flow_conf);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_egress_engine_conf_set(NULL, msg_hdr->port_no, remote_eng_id, &remote_flow_conf);
//            printf("Line : %d rc - [%d] \n\r",__LINE__, rc);
            ///* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_EGR_ENG_CONF_GET       : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            remote_eng_id = eng_init_conf.eng_id;
            memset(&remote_flow_conf, 0, sizeof(vtss_phy_ts_engine_flow_conf_t));
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_EGR_ENG_CONF_GET: String    %s\n", data);
//            printf("Recieved: eng_id  %u  encap_type %u flow_st_index %u  flow_end_index  %u flow_match_mode  %u\n", eng_init_conf.eng_id,  eng_init_conf.encap_type, eng_init_conf.flow_st_index, eng_init_conf.flow_end_index , eng_init_conf.flow_match_mode);
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_egress_engine_conf_get(NULL, msg_hdr->port_no, remote_eng_id, &remote_flow_conf);
            //printf("Line : %d rc - [%d] \n\r",__LINE__, rc);
//            print_flow_conf(&remote_flow_conf);
            memcpy(data, &remote_flow_conf, sizeof(vtss_phy_ts_engine_flow_conf_t));
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_engine_flow_conf_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_INGR_ENG_ACTION_SET    : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            remote_eng_id = eng_init_conf.eng_id;
            memset(&remote_action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
            memcpy(&remote_action_conf, &data[sizeof(engine_init_t)], sizeof(vtss_phy_ts_engine_action_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_INGR_ENG_ACTION_SET:\n");
//            print_action_conf(&remote_action_conf);
            /* Call the vtss_phy_ts_API */
            //printf("Inside FN :: %s port_no : %d remote_eng_id : %d \n\r",__FUNCTION__, msg_hdr->port_no, remote_eng_id);
            rc = vtss_phy_ts_ingress_engine_action_set(NULL, msg_hdr->port_no, remote_eng_id, &remote_action_conf);
            //printf("Inside FN :: %s rc - [%d] \n\r",__FUNCTION__, rc);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_INGR_ENG_ACTION_GET    : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            remote_eng_id = eng_init_conf.eng_id;
            memset(&remote_action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_INGR_ENG_ACTION_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_ingress_engine_action_get(NULL, msg_hdr->port_no, remote_eng_id, &remote_action_conf);
            memcpy(data, &remote_action_conf, sizeof(vtss_phy_ts_engine_action_t));
//            print_action_conf(&remote_action_conf);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_engine_action_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_EGR_ENG_ACTION_SET     : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            remote_eng_id = eng_init_conf.eng_id;
            memset(&remote_action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
            memcpy(&remote_action_conf, data + sizeof(engine_init_t), sizeof(vtss_phy_ts_engine_action_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_EGR_ENG_ACTION_SET:\n");
//            print_action_conf(&remote_action_conf);
            /* Call the vtss_phy_ts_API */
            //printf("port_no : %d remote_eng_id : %d \n\r", msg_hdr->port_no, remote_eng_id);
            rc = vtss_phy_ts_egress_engine_action_set(NULL, msg_hdr->port_no, remote_eng_id, &remote_action_conf);
            //printf("Inside Fn :: %s rc - [%d] \n\r", __FUNCTION__, rc);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_EGR_ENG_ACTION_GET     : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&eng_init_conf, 0, sizeof(engine_init_t));
            memcpy(&eng_init_conf, data, sizeof(engine_init_t));
            remote_eng_id = eng_init_conf.eng_id;
            //printf("Inside Fn :: %s eng_id : %d \n\r",__FUNCTION__, remote_eng_id);
            memset(&remote_action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_EGR_ENG_ACTION_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_egress_engine_action_get(NULL, msg_hdr->port_no, remote_eng_id, &remote_action_conf);
            memcpy(data, &remote_action_conf, sizeof(vtss_phy_ts_engine_action_t));
//            print_action_conf(&remote_action_conf);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_engine_action_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_EVENT_CONF_SET         : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_event_conf, 0, sizeof(event_conf_t));
            memcpy(&remote_event_conf, data, sizeof(event_conf_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_EVENT_CONF_SET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_event_enable_set(NULL, msg_hdr->port_no, remote_event_conf.enable, remote_event_conf.ev_mask);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;

        case REMOTE_TS_PHY_COMMAND_EVENT_CONF_GET         : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_event_conf, 0, sizeof(event_conf_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_EVENT_CONF_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_event_enable_get(NULL, msg_hdr->port_no, &remote_event_conf.ev_mask);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_event_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_EVENT_POLL             : 
            //printf("Inside FN :: %s case - [%d] \n\r",__FUNCTION__, msg_hdr->command);
            memset(&remote_event_conf, 0, sizeof(event_conf_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_EVENT_CONF_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_event_poll(NULL, msg_hdr->port_no, &remote_event_conf.status);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_event_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_STATS_GET              : 
            //printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
            memset(&remote_phy_stats, 0, sizeof(vtss_phy_ts_stats_t));
            /* Print the values */
            //printf("REMOTE_TS_PHY_COMMAND_EVENT_CONF_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_stats_get(NULL, msg_hdr->port_no, &remote_phy_stats);
            /* update the return value and data_length in msg_hdr */
            memcpy(data, &remote_phy_stats, sizeof(vtss_phy_ts_stats_t));
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_ts_stats_t);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_CORR_OVERFLOW_GET      : 
            //printf("Inside FN :: %s case - [%d] \n\r", __FUNCTION__, msg_hdr->command);
            /* Print the values */
//            printf("REMOTE_TS_PHY_COMMAND_CORR_OVERFLOW_GET:\n");
            /* Call the vtss_phy_ts_API */
            rc = vtss_phy_ts_correction_overflow_get(NULL, msg_hdr->port_no, &ingr_overflow, &egr_overflow);
            /* update the return value and data_length in msg_hdr */
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                memcpy(data, &ingr_overflow, sizeof(BOOL));
                memcpy(data + sizeof(BOOL), &egr_overflow, sizeof(BOOL));
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + 2*sizeof(BOOL);
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;

        case REMOTE_TS_PHY_COMMAND_INIT                   :
            break;
        case REMOTE_TS_PHY_COMMAND_PHY_MODE_CHANGE        : 
            break;
        case REMOTE_TS_PHY_COMMAND_1588_CSR_READ          :
            memset(&reg_conf, 0, sizeof(csr_reg_t)); 
            memcpy(&reg_conf, data, sizeof(csr_reg_t)); 
            rc = vtss_phy_1588_csr_reg_read(NULL, msg_hdr->port_no, reg_conf.blk_id, reg_conf.csr_address, &reg_conf.value);
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            if (rc == VTSS_RC_OK) {
                memcpy(data, &reg_conf, sizeof(reg_conf));
                msg_hdr->length = sizeof(remote_ts_msg_header_t) + sizeof(csr_reg_t); 
            } else {
                msg_hdr->length = sizeof(remote_ts_msg_header_t);
            }
            break;
        case REMOTE_TS_PHY_COMMAND_1588_CSR_WRITE         :
            memset(&reg_conf, 0, sizeof(csr_reg_t)); 
            memcpy(&reg_conf, data, sizeof(csr_reg_t));
            rc = vtss_phy_1588_csr_reg_write(NULL, msg_hdr->port_no, reg_conf.blk_id, reg_conf.csr_address, &reg_conf.value);
            msg_hdr->status = ((rc == VTSS_RC_OK) ? REMOTE_TS_PHY_STATUS_SUCCESS : REMOTE_TS_PHY_STATUS_FAILURE);
            msg_hdr->length = sizeof(remote_ts_msg_header_t);
            break;
    }
    return rc;
}
#endif /* VTSS_SW_OPTION_REMOTE_TS_PHY */

#endif /* VTSS_ARCH_DAYTONA */
