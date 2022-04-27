/*

 Vitesse PHY API software.

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
#ifndef _VTSS_PHY_VERIPHY_H
#define _VTSS_PHY_VERIPHY_H

#if VTSS_PHY_OPT_VERIPHY

#if defined(__C51__) || defined(__CX51__)
#define c51_idata idata
#define c51_code code
#else
#define c51_idata
#define c51_code
#endif /* __C51__ || __CX51__ */

#define ABS(x) (((x) < 0) ? -(x) : (x))

#define MAX_ABS_COEFF_ANOM_INVALID_NOISE  70
#define MAX_ABS_COEFF_LEN_INVALID_NOISE  180

#if defined(VTSS_PHY_OPT_VERIPHY_INT) && VTSS_PHY_OPT_VERIPHY_INT
extern bit VeriPHYIntFlag;
#endif

#define VTSS_VERIPHY_STATE_DONEBIT 0x80
#define DISCARD 0x80
#define VTSS_VERIPHY_STATE_IDLE 0x00
#define FFEinit4_7anomSearch   0
#define FFEinit4_7lengthSearch 1

typedef enum {
    VERIPHY_MODE_FULL,
    VERIPHY_MODE_ANOM_XPAIR,
    VERIPHY_MODE_ANOM_ONLY
} vtss_phy_veriphy_mode_t;

typedef enum {
    VERIPHY_STATE_IDLE              = VTSS_VERIPHY_STATE_IDLE,
    VERIPHY_STATE_INIT_0            = VTSS_VERIPHY_STATE_IDLE + 0x01,
    VERIPHY_STATE_INIT_1            = VTSS_VERIPHY_STATE_IDLE + 0x02,
    VERIPHY_STATE_INIT_LINKDOWN     = VTSS_VERIPHY_STATE_IDLE + 0x03,
    VERIPHY_STATE_INIT_ANOMSEARCH_0 = VTSS_VERIPHY_STATE_IDLE + 0x04,
    VERIPHY_STATE_INIT_ANOMSEARCH_1 = VTSS_VERIPHY_STATE_IDLE + 0x05,
    VERIPHY_STATE_ANOMSEARCH_0      = VTSS_VERIPHY_STATE_IDLE + 0x06,
    VERIPHY_STATE_ANOMSEARCH_1      = VTSS_VERIPHY_STATE_IDLE + 0x07,
    VERIPHY_STATE_ANOMSEARCH_2      = VTSS_VERIPHY_STATE_IDLE + 0x08,
    VERIPHY_STATE_ANOMSEARCH_3      = VTSS_VERIPHY_STATE_IDLE + 0x09,
    VERIPHY_STATE_INIT_CABLELEN     = VTSS_VERIPHY_STATE_IDLE + 0x0a,
    VERIPHY_STATE_GETCABLELEN_0     = VTSS_VERIPHY_STATE_IDLE + 0x0b,
    VERIPHY_STATE_GETCABLELEN_1     = VTSS_VERIPHY_STATE_IDLE + 0x0c,
    VERIPHY_STATE_PAIRSWAP          = VTSS_VERIPHY_STATE_IDLE + 0x0d,
    VERIPHY_STATE_ABORT             = (VTSS_VERIPHY_STATE_DONEBIT | 0),
    VERIPHY_STATE_FINISH            = (VTSS_VERIPHY_STATE_DONEBIT | 0x0e)
} vtss_veriphy_task_state_t;

typedef struct {
    /* Variables common to all tasks */
    vtss_mtimer_t timeout;      /* Absolute timeout */
    unsigned char task_state;   /* 0x00 ==> idle (but still serviced) */
    /* 0x01 - 0x7f ==> task-defined, */
    /* bit [7]: 1 ==> abort requested */

    /* VeriPHY public variables */
    unsigned char port;          /* PHY on which VeriPHY is running/last run on */
    unsigned char flags;        /* bit [7:6] = VeriPHY operating mode */
    /*        00 ==> full VeriPHY algorithm */
    /*        01 ==> anomaly-search only */
    /*        10 ==> anomaly-search w/o x-pair only */
    /*        11 ==> reserved */
    /* bit [5:4] = unreliablePtr (if bit [3] == 1) */
    /* bit [3]   = unreliablePtr presence flag */
    /* bit [2]   = getCableLength done flag */
    /* bit [1]   = valid */
    /* bit [0]   = linkup-mode */
    unsigned char flags2;       /* bits [7:2] - reserved */
    /* bits [1:0] - ams_force_cu, ams_force_fi on entry (Spyder) */
    /* bits [1:0] - reserved, ActiPHY-enabled on entry (Luton) */
    unsigned long saveReg;      /*- TP[5.4:3], MII [28.2], MII[9.12:11], MII[0] - 21 bits */
    unsigned long tokenReg;     /*-  Token Ring Registers 29 bits */
    unsigned char stat[4];      /* status for pairs A-D (0-3), 4-bit unsigned number */
    /*        most signiciant 4-bits represents prev. status */
    unsigned char loc[4];       /* length/fault location for pairs A-D (0-3), 8-bit unsgn */

    /* VeriPHY private variables */
    signed char subchan;
    signed char nc;
    unsigned char numCoeffs;
    unsigned char firstCoeff;
    short strength[4];          /* fault strength for pairs A-D (0-3), 14-bit signed int. */
    short thresh[4];            /* threshold (used in different places), 13-bit unsgn */
    short log2VGAx256;          /* log2(VGA gain scalefactor)*256 (0 for link-down case) */
    signed char  signFlip;      /* count used to improve location accuracy */
    long tr_raw0188;            /* wastes one byte */
} vtss_veriphy_task_t;

/******************************************************************************
 * Purpose     : Prepare for VeriPHY
 * Remarks     : mode:
 *                0 ==> full VeriPHY algorithm
 *                1 ==> anomaly-search and x-pair (no cable length search)
 *                2 ==> anomaly-search  (no cable length search and no x-pair search)
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
vtss_rc vtss_phy_veriphy_task_start(struct vtss_state_s *vtss_state, vtss_port_no_t port_no, u8 mode);

/******************************************************************************
 * Purpose     : Do VeriPHY operation on specified ports.
 * Remarks     :
 *               Results of operation is returned in structure pointed to
 *               by *result.
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
vtss_rc vtss_phy_veriphy(struct vtss_state_s *vtss_state, vtss_veriphy_task_t c51_idata *tsk);

#endif /* VTSS_PHY_OPT_VERIPHY */
#endif /* _VTSS_PHY_VERIPHY_H */
