/*

 Vitesse Switch Software.

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
#ifndef _VTSS_DAYTONA_REG_INIT_GFP_H
#define _VTSS_DAYTONA_REG_INIT_GFP_H

// Settings for mode GFPF

#define  VTSS_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA_GFPF                                                 VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA(0x1)
#define  VTSS_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL_GFPF                                         VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(0x2)
#define  VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE_GFPF                                 VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE(0x2)
#define  VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE_GFPF                                 VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE(0x2)
#define  VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET_GFPF                                  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET(0x1)
#define  VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET_GFPF                                  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET(0x1)


// Settings for mode PPOS

#define  VTSS_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA_PPOS                                                 VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA(0x1)
#define  VTSS_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL_PPOS                                         VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(0x3)
#define  VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE_PPOS                                 VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE(0x3)
#define  VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE_PPOS                                 VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE(0x3)
#define  VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET_PPOS                                  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET(0x1)
#define  VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET_PPOS                                  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET(0x1)


// Settings for mode GFPT

#define  VTSS_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA_GFPT                                                 VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA(0x1)
#define  VTSS_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL_GFPT                                         VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(0x0)
#define  VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE_GFPT                                 VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE(0x0)
#define  VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE_GFPT                                 VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE(0x0)
#define  VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET_GFPT                                  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET(0x1)
#define  VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET_GFPT                                  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET(0x1)


// Settings for mode BYP

#define  VTSS_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA_BYP                                                  VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA(0x0)
#define  VTSS_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL_BYP                                          VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(0x0)
#define  VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE_BYP                                  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE(0x0)
#define  VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE_BYP                                  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE(0x0)
#define  VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET_BYP                                   VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET(0x1)
#define  VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET_BYP                                   VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET(0x1)




typedef enum {
    BM_GFP_GFPF,
    BM_GFP_PPOS,
    BM_GFP_GFPT,
    BM_GFP_BYP,
    BM_GFP_LAST
} block_gfp_mode_t;

#endif /* _VTSS_DAYTONA_REG_INIT_GFP_H */
