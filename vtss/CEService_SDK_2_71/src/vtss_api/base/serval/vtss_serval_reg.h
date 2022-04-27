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

#ifndef _SERVAL_REG_H_
#define _SERVAL_REG_H_

#include "vtss_serval_regs.h"

/* Port -> DEV_RGMII, DEV Target Offset */
#define VTSS_TO_DEV(port)   (VTSS_TO_DEV_0 + (port * (VTSS_TO_DEV_1 - VTSS_TO_DEV_0)))

/* Commands for Mac Table Command register */
#define MAC_CMD_IDLE        0  /* Idle */
#define MAC_CMD_LEARN       1  /* Insert (Learn) 1 entry */
#define MAC_CMD_FORGET      2  /* Delete (Forget) 1 entry */
#define MAC_CMD_TABLE_AGE   3  /* Age entire table */
#define MAC_CMD_GET_NEXT    4  /* Get next entry */
#define MAC_CMD_TABLE_CLEAR 5  /* Delete all entries in table */
#define MAC_CMD_READ        6  /* Read entry at Mac Table Index */
#define MAC_CMD_WRITE       7  /* Write entry at Mac Table Index */

#define MAC_TYPE_NORMAL  0 /* Normal entry */
#define MAC_TYPE_LOCKED  1 /* Locked entry */
#define MAC_TYPE_IPV4_MC 2 /* IPv4 MC entry */
#define MAC_TYPE_IPV6_MC 3 /* IPv6 MC entry */

/* Commands for VLAN Table Command register */
#define VLAN_CMD_IDLE        0  /* Idle */
#define VLAN_CMD_READ        1  /* Read entry at VLAN Table Index */
#define VLAN_CMD_WRITE       2  /* Write entry at VLAN Table Index */
#define VLAN_CMD_TABLE_CLEAR 3  /* Reset all entries in table */

/* Commands for ISDX Table Command register */
#define ISDX_CMD_IDLE        0  /* Idle */
#define ISDX_CMD_READ        1  /* Read entry at VLAN Table Index */
#define ISDX_CMD_WRITE       2  /* Write entry at VLAN Table Index */
#define ISDX_CMD_TABLE_CLEAR 3  /* Reset all entries in table */

/* Types for REW_PORT::TAG_CFG */
#define TAG_CFG_DISABLE       0 /* Port tagging disabled */
#define TAG_CFG_ALL_NPV_NNUL  1 /* Tag all frames, except when VID=PORT_VLAN_CFG.PORT_VID or VID=0 */
#define TAG_CFG_ALL_NNUL      2 /* Tag all frames, except when VID=0 */
#define TAG_CFG_ALL           3 /* Tag all frames */

/* Types for REW_PORT::TAG_TPID_CFG */
#define TAG_TPID_CFG_0x8100   0  /* Use 0x8100 */
#define TAG_TPID_CFG_0x88A8   1  /* Use 0x88A8 */
#define TAG_TPID_CFG_PTPID    2  /* Use custom from PORT_VLAN_CFG.PORT_TPID */
#define TAG_TPID_CFG_PTPID_NC 3  /* As above, but unless ingress tag was a C-tag */

/* Types for SYS_POL::POL_MODE_CFG */
#define POL_MODE_LINERATE       0 /* Line rate. Police bytes including IPG_SIZE */
#define POL_MODE_DATARATE       1 /* Data rate. Police bytes excluding IPG. */
#define POL_MODE_FRMRATE_100FPS 2 /* Frame rate. Rate unit = 100 fps  (100/3 fps for Rev. B and later) */
#define POL_MODE_FRMRATE_1FPS   3 /* Frame rate. Rate unit = 1 fps (1/3 fps for Rev. B and later)*/

/* ACL Mask bits (inverse of chip polarity) */
#define ACL_MASK_ONES		  0xFFFFFFFF
#define ACL_MASK_ZERO		  0

/* ================================================================= *
 *  VCAP Common
 * ================================================================= */

/* VCAP Type-Group values */
#define VCAP_TG_NONE    0 /* Entry is invalid */
#define VCAP_TG_FULL    1 /* Full entry */
#define VCAP_TG_HALF    2 /* Half entry */
#define VCAP_TG_QUARTER 3 /* Quarter entry */

/* ================================================================= *
 *  VCAP IS0
 * ================================================================= */

/* IS0 full key - MLL */
#define IS0_FKO_MLL_IGR_PORT 0
#define IS0_FKL_MLL_IGR_PORT 4
#define IS0_FKO_MLL_TTYPE    4
#define IS0_FKL_MLL_TTYPE    2
#define IS0_FKO_MLL_BVID     6
#define IS0_FKL_MLL_BVID     12
#define IS0_FKO_MLL_M_DMAC   18
#define IS0_FKL_MLL_M_DMAC   48
#define IS0_FKO_MLL_M_SMAC   66
#define IS0_FKL_MLL_M_SMAC   48
#define IS0_FKO_MLL_E_TYPE   114
#define IS0_FKL_MLL_E_TYPE   2

/* IS0 half key - MLBS */
#define IS0_HKO_MLBS_LL_IDX    0
#define IS0_HKL_MLBS_LL_IDX    7
#define IS0_HKO_MLBS_LBL_0_VAL 7
#define IS0_HKL_MLBS_LBL_0_VAL 20
#define IS0_HKO_MLBS_LBL_0_TC  27
#define IS0_HKL_MLBS_LBL_0_TC  3
#define IS0_HKO_MLBS_LBL_1_VAL 30
#define IS0_HKL_MLBS_LBL_1_VAL 20
#define IS0_HKO_MLBS_LBL_1_TC  50
#define IS0_HKL_MLBS_LBL_1_TC  3
#define IS0_HKO_MLBS_LBL_2_VAL 53
#define IS0_HKL_MLBS_LBL_2_VAL 20
#define IS0_HKO_MLBS_LBL_2_TC  73
#define IS0_HKL_MLBS_LBL_2_TC  3

/* IS0 MLL action */
#define IS0_AO_MLL_LL_IDX          0
#define IS0_AL_MLL_LL_IDX          7
#define IS0_AO_MLL_MPLS_FWD        7
#define IS0_AL_MLL_MPLS_FWD        1
#define IS0_AO_MLL_B_PM            8
#define IS0_AL_MLL_B_PM            12
#define IS0_AO_MLL_CPU_Q           20
#define IS0_AL_MLL_CPU_Q           3
#define IS0_AO_MLL_ISDX            23
#define IS0_AL_MLL_ISDX            10
#define IS0_AO_MLL_VPROFILE_IDX    33
#define IS0_AL_MLL_VPROFILE_IDX    6
#define IS0_AO_MLL_SCONFIG_ENA     39
#define IS0_AL_MLL_SCONFIG_ENA     1
#define IS0_AO_MLL_CL_VID          40
#define IS0_AL_MLL_CL_VID          12
#define IS0_AO_MLL_QOS_DEFAULT_VAL 52
#define IS0_AL_MLL_QOS_DEFAULT_VAL 3
#define IS0_AO_MLL_DP_DEFAULT_VAL  55
#define IS0_AL_MLL_DP_DEFAULT_VAL  1
#define IS0_AO_MLL_OAM_ISDX        56
#define IS0_AL_MLL_OAM_ISDX        10
#define IS0_AO_MLL_ISDX_ADD_REPL   66
#define IS0_AL_MLL_ISDX_ADD_REPL   1

/* IS0 MLBS action */
#define IS0_AO_MLBS_ISDX          0
#define IS0_AL_MLBS_ISDX          10
#define IS0_AO_MLBS_B_PM          10
#define IS0_AL_MLBS_B_PM          12
#define IS0_AO_MLBS_CPU_Q         22
#define IS0_AL_MLBS_CPU_Q         3
#define IS0_AO_MLBS_OAM_ENA       25
#define IS0_AL_MLBS_OAM_ENA       3
#define IS0_AO_MLBS_BURIED_MIP    28
#define IS0_AL_MLBS_BURIED_MIP    1
#define IS0_AO_MLBS_RSVD_LBL_VAL  29
#define IS0_AL_MLBS_RSVD_LBL_VAL  4
#define IS0_AO_MLBS_RSVD_LBL_BOS  33
#define IS0_AL_MLBS_RSVD_LBL_BOS  1
#define IS0_AO_MLBS_CW_ENA        34
#define IS0_AL_MLBS_CW_ENA        1
#define IS0_AO_MLBS_TC_LABEL      35
#define IS0_AL_MLBS_TC_LABEL      2
#define IS0_AO_MLBS_TTL_LABEL     37
#define IS0_AL_MLBS_TTL_LABEL     2
#define IS0_AO_MLBS_SWAP_LABEL    39
#define IS0_AL_MLBS_SWAP_LABEL    2
#define IS0_AO_MLBS_TERMINATE_PW  41
#define IS0_AL_MLBS_TERMINATE_PW  1
#define IS0_AO_MLBS_POP_CNT       42
#define IS0_AL_MLBS_POP_CNT       3
#define IS0_AO_MLBS_E_LSP         45
#define IS0_AL_MLBS_E_LSP         1
#define IS0_AO_MLBS_TC_MAP        46
#define IS0_AL_MLBS_TC_MAP        3
#define IS0_AO_MLBS_L_LSP_COS     49
#define IS0_AL_MLBS_L_LSP_COS     3
#define IS0_AO_MLBS_INC_ISDX      52
#define IS0_AL_MLBS_INC_ISDX      1
#define IS0_AO_MLBS_OAM_ISDX      53
#define IS0_AL_MLBS_OAM_ISDX      10
#define IS0_AO_MLBS_ISDX_ADD_REPL 63
#define IS0_AL_MLBS_ISDX_ADD_REPL 1
#define IS0_AO_MLBS_SWAP_IS_BOS   64
#define IS0_AL_MLBS_SWAP_IS_BOS   1
#define IS0_AO_MLBS_VPROFILE_IDX  65
#define IS0_AL_MLBS_VPROFILE_IDX  6
#define IS0_AO_MLBS_SCONFIG_ENA   71
#define IS0_AL_MLBS_SCONFIG_ENA   1
#define IS0_AO_MLBS_CL_VID        72
#define IS0_AL_MLBS_CL_VID        12
#define IS0_AO_MLBS_VLAN_STAGD    84
#define IS0_AL_MLBS_VLAN_STAGD    1
#define IS0_AO_MLBS_VLAN_DEI      85
#define IS0_AL_MLBS_VLAN_DEI      1
#define IS0_AO_MLBS_VLAN_PCP      86
#define IS0_AL_MLBS_VLAN_PCP      3

/* ================================================================= *
 *  VCAP IS1
 * ================================================================= */

/* IS1 half key types */
#define IS1_TYPE_NORMAL     0
#define IS1_TYPE_5TUPLE_IP4 1

/* IS1 full key types */
#define IS1_TYPE_NORMAL_IP6 0
#define IS1_TYPE_7TUPLE     1
#define IS1_TYPE_5TUPLE_IP6 2

/* IS1 quarter key - DBL_VID */
#define IS1_QKO_LOOKUP          0
#define IS1_QKL_LOOKUP          2
#define IS1_QKO_IGR_PORT_MASK   2
#define IS1_QKL_IGR_PORT_MASK   12
#define IS1_QKO_ISDX            14
#define IS1_QKL_ISDX            10
#define IS1_QKO_L2_MC           24
#define IS1_QKL_L2_MC           1
#define IS1_QKO_L2_BC           25
#define IS1_QKL_L2_BC           1
#define IS1_QKO_IP_MC           26
#define IS1_QKL_IP_MC           1
#define IS1_QKO_VLAN_TAGGED     27
#define IS1_QKL_VLAN_TAGGED     1
#define IS1_QKO_VLAN_DBL_TAGGED 28
#define IS1_QKL_VLAN_DBL_TAGGED 1
#define IS1_QKO_TPID            29
#define IS1_QKL_TPID            1
#define IS1_QKO_VID             30
#define IS1_QKL_VID             12
#define IS1_QKO_DEI             42
#define IS1_QKL_DEI             1
#define IS1_QKO_PCP             43
#define IS1_QKL_PCP             3
#define IS1_QKO_INNER_TPID      46
#define IS1_QKL_INNER_TPID      1
#define IS1_QKO_INNER_VID       47
#define IS1_QKL_INNER_VID       12
#define IS1_QKO_INNER_DEI       59
#define IS1_QKL_INNER_DEI       1
#define IS1_QKO_INNER_PCP       60
#define IS1_QKL_INNER_PCP       3
#define IS1_QKO_ETYPE_LEN       63
#define IS1_QKL_ETYPE_LEN       1
#define IS1_QKO_ETYPE           64
#define IS1_QKL_ETYPE           16
#define IS1_QKO_IP_SNAP         80
#define IS1_QKL_IP_SNAP         1
#define IS1_QKO_IP4             81
#define IS1_QKL_IP4             1
#define IS1_QKO_L3_FRAGMENT     82
#define IS1_QKL_L3_FRAGMENT     1
#define IS1_QKO_L3_FRAG_OFS_GT0 83
#define IS1_QKL_L3_FRAG_OFS_GT0 1
#define IS1_QKO_L3_OPTIONS      84
#define IS1_QKL_L3_OPTIONS      1
#define IS1_QKO_L3_DSCP         85
#define IS1_QKL_L3_DSCP         6
#define IS1_QKO_TCP_UDP         91
#define IS1_QKL_TCP_UDP         1
#define IS1_QKO_TCP             92
#define IS1_QKL_TCP             1

/* IS1 half key - common */
#define IS1_HKO_TYPE            0
#define IS1_HKL_TYPE            1
#define IS1_HKO_LOOKUP          1
#define IS1_HKL_LOOKUP          2
#define IS1_HKO_IGR_PORT_MASK   3   
#define IS1_HKL_IGR_PORT_MASK   12
#define IS1_HKO_ISDX            15
#define IS1_HKL_ISDX            10
#define IS1_HKO_L2_MC           25
#define IS1_HKL_L2_MC           1
#define IS1_HKO_L2_BC           26
#define IS1_HKL_L2_BC           1
#define IS1_HKO_IP_MC           27
#define IS1_HKL_IP_MC           1
#define IS1_HKO_VLAN_TAGGED     28
#define IS1_HKL_VLAN_TAGGED     1
#define IS1_HKO_VLAN_DBL_TAGGED 29
#define IS1_HKL_VLAN_DBL_TAGGED 1
#define IS1_HKO_TPID            30
#define IS1_HKL_TPID            1
#define IS1_HKO_VID             31
#define IS1_HKL_VID             12
#define IS1_HKO_DEI             43
#define IS1_HKL_DEI             1
#define IS1_HKO_PCP             44
#define IS1_HKL_PCP             3

/* IS1 half key - S1_NORMAL */
#define IS1_HKO_NORMAL_L2_SMAC         47
#define IS1_HKL_NORMAL_L2_SMAC         48
#define IS1_HKO_NORMAL_ETYPE_LEN       95
#define IS1_HKL_NORMAL_ETYPE_LEN       1
#define IS1_HKO_NORMAL_ETYPE           96
#define IS1_HKL_NORMAL_ETYPE           16
#define IS1_HKO_NORMAL_IP_SNAP         112
#define IS1_HKL_NORMAL_IP_SNAP         1
#define IS1_HKO_NORMAL_IP4             113
#define IS1_HKL_NORMAL_IP4             1
#define IS1_HKO_NORMAL_L3_FRAGMENT     114
#define IS1_HKL_NORMAL_L3_FRAGMENT     1
#define IS1_HKO_NORMAL_L3_FRAG_OFS_GT0 115
#define IS1_HKL_NORMAL_L3_FRAG_OFS_GT0 1
#define IS1_HKO_NORMAL_L3_OPTIONS      116
#define IS1_HKL_NORMAL_L3_OPTIONS      1
#define IS1_HKO_NORMAL_L3_DSCP         117
#define IS1_HKL_NORMAL_L3_DSCP         6
#define IS1_HKO_NORMAL_L3_IP4_SIP      123
#define IS1_HKL_NORMAL_L3_IP4_SIP      32
#define IS1_HKO_NORMAL_TCP_UDP         155
#define IS1_HKL_NORMAL_TCP_UDP         1
#define IS1_HKO_NORMAL_TCP             156
#define IS1_HKL_NORMAL_TCP             1
#define IS1_HKO_NORMAL_L4_SPORT        157
#define IS1_HKL_NORMAL_L4_SPORT        16
#define IS1_HKO_NORMAL_L4_RNG          173
#define IS1_HKL_NORMAL_L4_RNG          8

/* IS1 half key - S1_5TUPLE_IP4 */
#define IS1_HKO_5TUPLE_IP4_INNER_TPID      47
#define IS1_HKL_5TUPLE_IP4_INNER_TPID      1
#define IS1_HKO_5TUPLE_IP4_INNER_VID       48
#define IS1_HKL_5TUPLE_IP4_INNER_VID       12
#define IS1_HKO_5TUPLE_IP4_INNER_DEI       60
#define IS1_HKL_5TUPLE_IP4_INNER_DEI       1
#define IS1_HKO_5TUPLE_IP4_INNER_PCP       61
#define IS1_HKL_5TUPLE_IP4_INNER_PCP       3
#define IS1_HKO_5TUPLE_IP4_IP4             64
#define IS1_HKL_5TUPLE_IP4_IP4             1 
#define IS1_HKO_5TUPLE_IP4_L3_FRAGMENT     65
#define IS1_HKL_5TUPLE_IP4_L3_FRAGMENT     1
#define IS1_HKO_5TUPLE_IP4_L3_FRAG_OFS_GT0 66
#define IS1_HKL_5TUPLE_IP4_L3_FRAG_OFS_GT0 1
#define IS1_HKO_5TUPLE_IP4_L3_OPTIONS      67
#define IS1_HKL_5TUPLE_IP4_L3_OPTIONS      1
#define IS1_HKO_5TUPLE_IP4_L3_DSCP         68
#define IS1_HKL_5TUPLE_IP4_L3_DSCP         6
#define IS1_HKO_5TUPLE_IP4_L3_IP4_DIP      74
#define IS1_HKL_5TUPLE_IP4_L3_IP4_DIP      32
#define IS1_HKO_5TUPLE_IP4_L3_IP4_SIP      106
#define IS1_HKL_5TUPLE_IP4_L3_IP4_SIP      32
#define IS1_HKO_5TUPLE_IP4_L3_IP_PROTO     138
#define IS1_HKL_5TUPLE_IP4_L3_IP_PROTO     8
#define IS1_HKO_5TUPLE_IP4_TCP_UDP         146
#define IS1_HKL_5TUPLE_IP4_TCP_UDP         1
#define IS1_HKO_5TUPLE_IP4_TCP             147
#define IS1_HKL_5TUPLE_IP4_TCP             1
#define IS1_HKO_5TUPLE_IP4_L4_RNG          148
#define IS1_HKL_5TUPLE_IP4_L4_RNG          8
#define IS1_HKO_5TUPLE_IP4_IP_PAYLOAD      156
#define IS1_HKL_5TUPLE_IP4_IP_PAYLOAD      32

/* IS1 full key - common */
#define IS1_FKO_TYPE            0
#define IS1_FKL_TYPE            2
#define IS1_FKO_LOOKUP          2
#define IS1_FKL_LOOKUP          2
#define IS1_FKO_IGR_PORT_MASK   4
#define IS1_FKL_IGR_PORT_MASK   12
#define IS1_FKO_ISDX            16
#define IS1_FKL_ISDX            10
#define IS1_FKO_L2_MC           26
#define IS1_FKL_L2_MC           1
#define IS1_FKO_L2_BC           27
#define IS1_FKL_L2_BC           1
#define IS1_FKO_IP_MC           28
#define IS1_FKL_IP_MC           1
#define IS1_FKO_VLAN_TAGGED     29
#define IS1_FKL_VLAN_TAGGED     1
#define IS1_FKO_VLAN_DBL_TAGGED 30
#define IS1_FKL_VLAN_DBL_TAGGED 1
#define IS1_FKO_TPID            31
#define IS1_FKL_TPID            1
#define IS1_FKO_VID             32
#define IS1_FKL_VID             12
#define IS1_FKO_DEI             44
#define IS1_FKL_DEI             1
#define IS1_FKO_PCP             45
#define IS1_FKL_PCP             3
#define IS1_FKO_INNER_TPID      48
#define IS1_FKL_INNER_TPID      1
#define IS1_FKO_INNER_VID       49
#define IS1_FKL_INNER_VID       12
#define IS1_FKO_INNER_DEI       61
#define IS1_FKL_INNER_DEI       1
#define IS1_FKO_INNER_PCP       62
#define IS1_FKL_INNER_PCP       3

/* IS1 full key - S1_NORMAL_IP6 */
#define IS1_FKO_NORMAL_IP6_L2_SMAC     65
#define IS1_FKL_NORMAL_IP6_L2_SMAC     48
#define IS1_FKO_NORMAL_IP6_L3_DSCP     113
#define IS1_FKL_NORMAL_IP6_L3_DSCP     6
#define IS1_FKO_NORMAL_IP6_L3_IP6_SIP  119
#define IS1_FKL_NORMAL_IP6_L3_IP6_SIP  128
#define IS1_FKO_NORMAL_IP6_L3_IP_PROTO 247
#define IS1_FKL_NORMAL_IP6_L3_IP_PROTO 8
#define IS1_FKO_NORMAL_IP6_TCP_UDP     255
#define IS1_FKL_NORMAL_IP6_TCP_UDP     1
#define IS1_FKO_NORMAL_IP6_L4_RNG      256
#define IS1_FKL_NORMAL_IP6_L4_RNG      8
#define IS1_FKO_NORMAL_IP6_IP_PAYLOAD  264
#define IS1_FKL_NORMAL_IP6_IP_PAYLOAD  112

/* IS1 full key - S1_7TUPLE */
#define IS1_FKO_7TUPLE_L2_DMAC         65
#define IS1_FKL_7TUPLE_L2_DMAC         48
#define IS1_FKO_7TUPLE_L2_SMAC         113
#define IS1_FKL_7TUPLE_L2_SMAC         48
#define IS1_FKO_7TUPLE_ETYPE_LEN       161
#define IS1_FKL_7TUPLE_ETYPE_LEN       1
#define IS1_FKO_7TUPLE_ETYPE           162
#define IS1_FKL_7TUPLE_ETYPE           16
#define IS1_FKO_7TUPLE_IP_SNAP         178
#define IS1_FKL_7TUPLE_IP_SNAP         1
#define IS1_FKO_7TUPLE_IP4             179
#define IS1_FKL_7TUPLE_IP4             1
#define IS1_FKO_7TUPLE_L3_FRAGMENT     180
#define IS1_FKL_7TUPLE_L3_FRAGMENT     1
#define IS1_FKO_7TUPLE_L3_FRAG_OFS_GT0 181
#define IS1_FKL_7TUPLE_L3_FRAG_OFS_GT0 1
#define IS1_FKO_7TUPLE_L3_OPTIONS      182
#define IS1_FKL_7TUPLE_L3_OPTIONS      1
#define IS1_FKO_7TUPLE_L3_DSCP         183
#define IS1_FKL_7TUPLE_L3_DSCP         6
#define IS1_FKO_7TUPLE_L3_IP6_DIP_MSB  189
#define IS1_FKL_7TUPLE_L3_IP6_DIP_MSB  16
#define IS1_FKO_7TUPLE_L3_IP6_DIP      205
#define IS1_FKL_7TUPLE_L3_IP6_DIP      64
#define IS1_FKO_7TUPLE_L3_IP6_SIP_MSB  269
#define IS1_FKL_7TUPLE_L3_IP6_SIP_MSB  16
#define IS1_FKO_7TUPLE_L3_IP6_SIP      285
#define IS1_FKL_7TUPLE_L3_IP6_SIP      64
#define IS1_FKO_7TUPLE_TCP_UDP         349
#define IS1_FKL_7TUPLE_TCP_UDP         1
#define IS1_FKO_7TUPLE_TCP             350
#define IS1_FKL_7TUPLE_TCP             1
#define IS1_FKO_7TUPLE_L4_SPORT        351
#define IS1_FKL_7TUPLE_L4_SPORT        16
#define IS1_FKO_7TUPLE_L4_RNG          367
#define IS1_FKL_7TUPLE_L4_RNG          8

/* IS1 full key - S1_5TUPLE_IP6 */
#define IS1_FKO_5TUPLE_IP6_L3_DSCP     65
#define IS1_FKL_5TUPLE_IP6_L3_DSCP     6
#define IS1_FKO_5TUPLE_IP6_L3_IP6_DIP  71
#define IS1_FKL_5TUPLE_IP6_L3_IP6_DIP  128
#define IS1_FKO_5TUPLE_IP6_L3_IP6_SIP  199
#define IS1_FKL_5TUPLE_IP6_L3_IP6_SIP  128
#define IS1_FKO_5TUPLE_IP6_L3_IP_PROTO 327
#define IS1_FKL_5TUPLE_IP6_L3_IP_PROTO 8
#define IS1_FKO_5TUPLE_IP6_TCP_UDP     335
#define IS1_FKL_5TUPLE_IP6_TCP_UDP     1
#define IS1_FKO_5TUPLE_IP6_L4_RNG      336
#define IS1_FKL_5TUPLE_IP6_L4_RNG      8
#define IS1_FKO_5TUPLE_IP6_IP_PAYLOAD  344
#define IS1_FKL_5TUPLE_IP6_IP_PAYLOAD  32

/* IS1 action */
#define IS1_AO_DSCP_ENA            0
#define IS1_AL_DSCP_ENA            1
#define IS1_AO_DSCP_VAL            1
#define IS1_AL_DSCP_VAL            6
#define IS1_AO_QOS_ENA             7
#define IS1_AL_QOS_ENA             1
#define IS1_AO_QOS_VAL             8
#define IS1_AL_QOS_VAL             3
#define IS1_AO_DP_ENA              11
#define IS1_AL_DP_ENA              1
#define IS1_AO_DP_VAL              12
#define IS1_AL_DP_VAL              1
#define IS1_AO_PAG_OVERRIDE_MASK   13
#define IS1_AL_PAG_OVERRIDE_MASK   8
#define IS1_AO_PAG_VAL             21
#define IS1_AL_PAG_VAL             8
#define IS1_AO_ISDX_REPLACE_ENA    29
#define IS1_AL_ISDX_REPLACE_ENA    1
#define IS1_AO_ISDX_ADD_VAL        30
#define IS1_AL_ISDX_ADD_VAL        10
#define IS1_AO_VID_REPLACE_ENA     40
#define IS1_AL_VID_REPLACE_ENA     1
#define IS1_AO_VID_ADD_VAL         41
#define IS1_AL_VID_ADD_VAL         12
#define IS1_AO_FID_SEL             53
#define IS1_AL_FID_SEL             2
#define IS1_AO_FID_VAL             55
#define IS1_AL_FID_VAL             13
#define IS1_AO_PCP_DEI_ENA         68
#define IS1_AL_PCP_DEI_ENA         1
#define IS1_AO_PCP_VAL             69
#define IS1_AL_PCP_VAL             3
#define IS1_AO_DEI_VAL             72
#define IS1_AL_DEI_VAL             1
#define IS1_AO_VLAN_POP_CNT_ENA    73
#define IS1_AL_VLAN_POP_CNT_ENA    1
#define IS1_AO_VLAN_POP_CNT        74
#define IS1_AL_VLAN_POP_CNT        2
#define IS1_AO_CUSTOM_ACE_TYPE_ENA 76
#define IS1_AL_CUSTOM_ACE_TYPE_ENA 4

/* ================================================================= *
 *  VCAP IS2
 * ================================================================= */

/* IS2 half key types */
#define IS2_TYPE_ETYPE      0
#define IS2_TYPE_LLC        1
#define IS2_TYPE_SNAP       2
#define IS2_TYPE_ARP        3
#define IS2_TYPE_IP_UDP_TCP 4
#define IS2_TYPE_IP_OTHER   5
#define IS2_TYPE_IPV6       6
#define IS2_TYPE_OAM        7
#define IS2_TYPE_SMAC_SIP6  8
#define IS2_TYPE_ANY        100 /* Pseudo type */

/* IS2 half key type mask for matching any IP */
#define IS2_TYPE_MASK_IP_ANY 0xe

/* IS2 full key types */
#define IS2_TYPE_IP6_TCP_UDP 0
#define IS2_TYPE_IP6_OTHER   1
#define IS2_TYPE_CUSTOM      2

/* IS2 action types */
#define IS2_ACTION_TYPE_NORMAL   0
#define IS2_ACTION_TYPE_SMAC_SIP 1

/* IS2 MASK_MODE values */
#define IS2_ACT_MASK_MODE_NONE   0
#define IS2_ACT_MASK_MODE_FILTER 1
#define IS2_ACT_MASK_MODE_POLICY 2
#define IS2_ACT_MASK_MODE_REDIR  3

/* IS2 REW_OP values */
#define IS2_ACT_REW_OP_NONE                0 
#define IS2_ACT_REW_OP_PTP_ONE             2
#define IS2_ACT_REW_OP_PTP_TWO             3
#define IS2_ACT_REW_OP_SPECIAL             8
#define IS2_ACT_REW_OP_PTP_ORG             9
#define IS2_ACT_REW_OP_PTP_ONE_SUB_DELAY_1 (IS2_ACT_REW_OP_PTP_ONE | (1 << 3))
#define IS2_ACT_REW_OP_PTP_ONE_SUB_DELAY_2 (IS2_ACT_REW_OP_PTP_ONE | (2 << 3))
#define IS2_ACT_REW_OP_PTP_ONE_ADD_DELAY   (IS2_ACT_REW_OP_PTP_ONE | (1 << 5))
#define IS2_ACT_REW_OP_PTP_ONE_ADD_SUB     (1 << 7)

/* IS2 quarter key - SMAC_SIP4 */
#define IS2_QKO_IGR_PORT   0
#define IS2_QKL_IGR_PORT   4
#define IS2_QKO_L2_SMAC    4
#define IS2_QKL_L2_SMAC    48
#define IS2_QKO_L3_IP4_SIP 52
#define IS2_QKL_L3_IP4_SIP 32

/* IS2 half key - common */
#define IS2_HKO_TYPE          0
#define IS2_HKL_TYPE          4
#define IS2_HKO_FIRST         4
#define IS2_HKL_FIRST         1
#define IS2_HKO_PAG           5
#define IS2_HKL_PAG           8
#define IS2_HKO_IGR_PORT_MASK 13
#define IS2_HKL_IGR_PORT_MASK 12
#define IS2_HKO_SERVICE_FRM   25
#define IS2_HKL_SERVICE_FRM   1
#define IS2_HKO_HOST_MATCH    26
#define IS2_HKL_HOST_MATCH    1
#define IS2_HKO_L2_MC         27
#define IS2_HKL_L2_MC         1
#define IS2_HKO_L2_BC         28
#define IS2_HKL_L2_BC         1
#define IS2_HKO_VLAN_TAGGED   29
#define IS2_HKL_VLAN_TAGGED   1
#define IS2_HKO_VID           30
#define IS2_HKL_VID           12
#define IS2_HKO_DEI           42
#define IS2_HKL_DEI           1
#define IS2_HKO_PCP           43
#define IS2_HKL_PCP           3

/* IS2 half key - MAC_ETYPE/MAC_LLC/MAC_SNAP/OAM common */
#define IS2_HKO_L2_DMAC 46
#define IS2_HKL_L2_DMAC 48
#define IS2_HKO_L2_SMAC 94
#define IS2_HKL_L2_SMAC 48

/* IS2 half key - MAC_ETYPE */
#define IS2_HKO_MAC_ETYPE_ETYPE      142
#define IS2_HKL_MAC_ETYPE_ETYPE      16
#define IS2_HKO_MAC_ETYPE_L2_PAYLOAD 158
#define IS2_HKL_MAC_ETYPE_L2_PAYLOAD 27

/* IS2 half key - MAC_LLC */
#define IS2_HKO_MAC_LLC_L2_LLC 142
#define IS2_HKL_MAC_LLC_L2_LLC 40

/* IS2 half key - MAC_SNAP */
#define IS2_HKO_MAC_SNAP_L2_SNAP 142
#define IS2_HKL_MAC_SNAP_L2_SNAP 40

/* IS2 half key - ARP */
#define IS2_HKO_MAC_ARP_L2_SMAC            46
#define IS2_HKL_MAC_ARP_L2_SMAC            48
#define IS2_HKO_MAC_ARP_ARP_ADDR_SPACE_OK  94
#define IS2_HKL_MAC_ARP_ARP_ADDR_SPACE_OK  1
#define IS2_HKO_MAC_ARP_ARP_PROTO_SPACE_OK 95
#define IS2_HKL_MAC_ARP_ARP_PROTO_SPACE_OK 1
#define IS2_HKO_MAC_ARP_ARP_LEN_OK         96
#define IS2_HKL_MAC_ARP_ARP_LEN_OK         1
#define IS2_HKO_MAC_ARP_ARP_TGT_MATCH      97
#define IS2_HKL_MAC_ARP_ARP_TGT_MATCH      1
#define IS2_HKO_MAC_ARP_ARP_SENDER_MATCH   98
#define IS2_HKL_MAC_ARP_ARP_SENDER_MATCH   1
#define IS2_HKO_MAC_ARP_ARP_OPCODE_UNKNOWN 99
#define IS2_HKL_MAC_ARP_ARP_OPCODE_UNKNOWN 1
#define IS2_HKO_MAC_ARP_ARP_OPCODE         100
#define IS2_HKL_MAC_ARP_ARP_OPCODE         2
#define IS2_HKO_MAC_ARP_L3_IP4_DIP         102
#define IS2_HKL_MAC_ARP_L3_IP4_DIP         32
#define IS2_HKO_MAC_ARP_L3_IP4_SIP         134
#define IS2_HKL_MAC_ARP_L3_IP4_SIP         32
#define IS2_HKO_MAC_ARP_DIP_EQ_SIP         166
#define IS2_HKL_MAC_ARP_DIP_EQ_SIP         1

/* IS2 half key - IP4_TCP_UDP/IP4_OTHER common */
#define IS2_HKO_IP4             46
#define IS2_HKL_IP4             1
#define IS2_HKO_L3_FRAGMENT     47
#define IS2_HKL_L3_FRAGMENT     1
#define IS2_HKO_L3_FRAG_OFS_GT0 48
#define IS2_HKL_L3_FRAG_OFS_GT0 1
#define IS2_HKO_L3_OPTIONS      49
#define IS2_HKL_L3_OPTIONS      1
#define IS2_HKO_L3_TTL_GT0      50
#define IS2_HKL_L3_TTL_GT0      1
#define IS2_HKO_L3_TOS          51
#define IS2_HKL_L3_TOS          8
#define IS2_HKO_L3_IP4_DIP      59
#define IS2_HKL_L3_IP4_DIP      32
#define IS2_HKO_L3_IP4_SIP      91
#define IS2_HKL_L3_IP4_SIP      32
#define IS2_HKO_DIP_EQ_SIP      123
#define IS2_HKL_DIP_EQ_SIP      1

/* IS2 half key - IP4_TCP_UDP */
#define IS2_HKO_IP4_TCP_UDP_TCP             124
#define IS2_HKL_IP4_TCP_UDP_TCP             1
#define IS2_HKO_IP4_TCP_UDP_L4_DPORT        125
#define IS2_HKL_IP4_TCP_UDP_L4_DPORT        16
#define IS2_HKO_IP4_TCP_UDP_L4_SPORT        141
#define IS2_HKL_IP4_TCP_UDP_L4_SPORT        16
#define IS2_HKO_IP4_TCP_UDP_L4_RNG          157
#define IS2_HKL_IP4_TCP_UDP_L4_RNG          8
#define IS2_HKO_IP4_TCP_UDP_SPORT_EQ_DPORT  165
#define IS2_HKL_IP4_TCP_UDP_SPORT_EQ_DPORT  1
#define IS2_HKO_IP4_TCP_UDP_SEQUENCE_EQ0    166
#define IS2_HKL_IP4_TCP_UDP_SEQUENCE_EQ0    1
#define IS2_HKO_IP4_TCP_UDP_L4_FIN          167
#define IS2_HKL_IP4_TCP_UDP_L4_FIN          1
#define IS2_HKO_IP4_TCP_UDP_L4_SYN          168
#define IS2_HKL_IP4_TCP_UDP_L4_SYN          1
#define IS2_HKO_IP4_TCP_UDP_L4_RST          169
#define IS2_HKL_IP4_TCP_UDP_L4_RST          1
#define IS2_HKO_IP4_TCP_UDP_L4_PSH          170
#define IS2_HKL_IP4_TCP_UDP_L4_PSH          1
#define IS2_HKO_IP4_TCP_UDP_L4_ACK          171
#define IS2_HKL_IP4_TCP_UDP_L4_ACK          1
#define IS2_HKO_IP4_TCP_UDP_L4_URG          172
#define IS2_HKL_IP4_TCP_UDP_L4_URG          1
#define IS2_HKO_IP4_TCP_UDP_L4_1588_DOM     173
#define IS2_HKL_IP4_TCP_UDP_L4_1588_DOM     8
#define IS2_HKO_IP4_TCP_UDP_L4_1588_VER     181
#define IS2_HKL_IP4_TCP_UDP_L4_1588_VER     4

/* IS2 half key - IP4_OTHER */
#define IS2_HKO_IP4_OTHER_L3_PROTO   124
#define IS2_HKL_IP4_OTHER_L3_PROTO   8
#define IS2_HKO_IP4_OTHER_L3_PAYLOAD 132
#define IS2_HKL_IP4_OTHER_L3_PAYLOAD 56

/* IS2 half key - IP6_STD */
#define IS2_HKO_IP6_STD_L3_TTL_GT0 46
#define IS2_HKL_IP6_STD_L3_TTL_GT0 1
#define IS2_HKO_IP6_STD_L3_IP6_SIP 47
#define IS2_HKL_IP6_STD_L3_IP6_SIP 128
#define IS2_HKO_IP6_STD_L3_PROTO   175
#define IS2_HKL_IP6_STD_L3_PROTO   8

/* IS2 half key - OAM */
#define IS2_HKO_OAM_OAM_MEL_FLAGS    142
#define IS2_HKL_OAM_OAM_MEL_FLAGS    7
#define IS2_HKO_OAM_OAM_VER          149
#define IS2_HKL_OAM_OAM_VER          5
#define IS2_HKO_OAM_OAM_OPCODE       154
#define IS2_HKL_OAM_OAM_OPCODE       8
#define IS2_HKO_OAM_OAM_FLAGS        162
#define IS2_HKL_OAM_OAM_FLAGS        8
#define IS2_HKO_OAM_OAM_MEPID        170
#define IS2_HKL_OAM_OAM_MEPID        16
#define IS2_HKO_OAM_OAM_CCM_CNTS_EQ0 186
#define IS2_HKL_OAM_OAM_CCM_CNTS_EQ0 1

/* IS2 half key - SMAC_SIP6 */
#define IS2_HKO_SMAC_SIP6_IGR_PORT   4
#define IS2_HKL_SMAC_SIP6_IGR_PORT   4
#define IS2_HKO_SMAC_SIP6_L2_SMAC    8
#define IS2_HKL_SMAC_SIP6_L2_SMAC    48
#define IS2_HKO_SMAC_SIP6_L3_IP6_SIP 56
#define IS2_HKL_SMAC_SIP6_L3_IP6_SIP 128

/* IS2 full key - common */
#define IS2_FKO_TYPE          0
#define IS2_FKL_TYPE          2
#define IS2_FKO_FIRST         2
#define IS2_FKL_FIRST         1
#define IS2_FKO_PAG           3
#define IS2_FKL_PAG           8
#define IS2_FKO_IGR_PORT_MASK 11
#define IS2_FKL_IGR_PORT_MASK 12
#define IS2_FKO_SERVICE_FRM   23
#define IS2_FKL_SERVICE_FRM   1
#define IS2_FKO_HOST_MATCH    24
#define IS2_FKL_HOST_MATCH    1
#define IS2_FKO_L2_MC         25
#define IS2_FKL_L2_MC         1
#define IS2_FKO_L2_BC         26
#define IS2_FKL_L2_BC         1
#define IS2_FKO_VLAN_TAGGED   27
#define IS2_FKL_VLAN_TAGGED   1
#define IS2_FKO_VID           28
#define IS2_FKL_VID           12
#define IS2_FKO_DEI           40
#define IS2_FKL_DEI           1
#define IS2_FKO_PCP           41
#define IS2_FKL_PCP           3

/* IS2 full key - IP6_TCP_UDP/IP6_OTHER common */
#define IS2_FKO_L3_TTL_GT0 44
#define IS2_FKL_L3_TTL_GT0 1
#define IS2_FKO_L3_TOS     45
#define IS2_FKL_L3_TOS     8
#define IS2_FKO_L3_IP6_DIP 53
#define IS2_FKL_L3_IP6_DIP 128
#define IS2_FKO_L3_IP6_SIP 181
#define IS2_FKL_L3_IP6_SIP 128
#define IS2_FKO_DIP_EQ_SIP 309
#define IS2_FKL_DIP_EQ_SIP 1

/* IS2 full key - IP6_TCP_UDP */
#define IS2_FKO_IP6_TCP_UDP_TCP            310
#define IS2_FKL_IP6_TCP_UDP_TCP            1
#define IS2_FKO_IP6_TCP_UDP_L4_DPORT       311
#define IS2_FKL_IP6_TCP_UDP_L4_DPORT       16
#define IS2_FKO_IP6_TCP_UDP_L4_SPORT       327
#define IS2_FKL_IP6_TCP_UDP_L4_SPORT       16
#define IS2_FKO_IP6_TCP_UDP_L4_RNG         343
#define IS2_FKL_IP6_TCP_UDP_L4_RNG         8
#define IS2_FKO_IP6_TCP_UDP_SPORT_EQ_DPORT 351
#define IS2_FKL_IP6_TCP_UDP_SPORT_EQ_DPORT 1
#define IS2_FKO_IP6_TCP_UDP_SEQUENCE_EQ0   352
#define IS2_FKL_IP6_TCP_UDP_SEQUENCE_EQ0   1
#define IS2_FKO_IP6_TCP_UDP_L4_FIN         353
#define IS2_FKL_IP6_TCP_UDP_L4_FIN         1
#define IS2_FKO_IP6_TCP_UDP_L4_SYN         354
#define IS2_FKL_IP6_TCP_UDP_L4_SYN         1
#define IS2_FKO_IP6_TCP_UDP_L4_RST         355
#define IS2_FKL_IP6_TCP_UDP_L4_RST         1
#define IS2_FKO_IP6_TCP_UDP_L4_PSH         356
#define IS2_FKL_IP6_TCP_UDP_L4_PSH         1
#define IS2_FKO_IP6_TCP_UDP_L4_ACK         357
#define IS2_FKL_IP6_TCP_UDP_L4_ACK         1
#define IS2_FKO_IP6_TCP_UDP_L4_URG         358
#define IS2_FKL_IP6_TCP_UDP_L4_URG         1
#define IS2_FKO_IP6_TCP_UDP_L4_1588_DOM    359
#define IS2_FKL_IP6_TCP_UDP_L4_1588_DOM    8
#define IS2_FKO_IP6_TCP_UDP_L4_1588_VER    367
#define IS2_FKL_IP6_TCP_UDP_L4_1588_VER    4

/* IS2 full key - IP6_OTHER */
#define IS2_FKO_IP6_OTHER_L3_PROTO   310
#define IS2_FKL_IP6_OTHER_L3_PROTO   8
#define IS2_FKO_IP6_OTHER_L3_PAYLOAD 318
#define IS2_FKL_IP6_OTHER_L3_PAYLOAD 56

/* IS2 full key - CUSTOM */
#define IS2_FKO_CUSTOM_CUSTOM_TYPE 44
#define IS2_FKL_CUSTOM_CUSTOM_TYPE 1
#define IS2_FKO_CUSTOM_CUSTOM      45
#define IS2_FKL_CUSTOM_CUSTOM      320

/* IS2 action - BASE_TYPE */
#define IS2_AO_HIT_ME_ONCE      0
#define IS2_AL_HIT_ME_ONCE      1
#define IS2_AO_CPU_COPY_ENA     1
#define IS2_AL_CPU_COPY_ENA     1
#define IS2_AO_CPU_QU_NUM       2
#define IS2_AL_CPU_QU_NUM       3
#define IS2_AO_MASK_MODE        5
#define IS2_AL_MASK_MODE        2
#define IS2_AO_MIRROR_ENA       7
#define IS2_AL_MIRROR_ENA       1
#define IS2_AO_LRN_DIS          8
#define IS2_AL_LRN_DIS          1
#define IS2_AO_POLICE_ENA       9
#define IS2_AL_POLICE_ENA       1
#define IS2_AO_POLICE_IDX       10
#define IS2_AL_POLICE_IDX       11
#define IS2_AO_POLICE_VCAP_ONLY 21
#define IS2_AL_POLICE_VCAP_ONLY 1
#define IS2_AO_PORT_MASK        22
#define IS2_AL_PORT_MASK        11
#define IS2_AO_REW_OP           33
#define IS2_AL_REW_OP           9
#define IS2_AO_LM_CNT_DIS       42
#define IS2_AL_LM_CNT_DIS       1
#define IS2_AO_ISDX_ENA         44
#define IS2_AL_ISDX_ENA         1
#define IS2_AO_ACL_ID           45
#define IS2_AL_ACL_ID           6

/* IS2 action - SMAC_SIP */
#define IS2_AO_SMAC_SIP_CPU_COPY_ENA 0
#define IS2_AL_SMAC_SIP_CPU_COPY_ENA 1
#define IS2_AO_SMAC_SIP_CPU_QU_NUM   1
#define IS2_AL_SMAC_SIP_CPU_QU_NUM   3
#define IS2_AO_SMAC_SIP_FWD_KILL_ENA 4   
#define IS2_AL_SMAC_SIP_FWD_KILL_ENA 1
#define IS2_AO_SMAC_SIP_HOST_MATCH   5
#define IS2_AL_SMAC_SIP_HOST_MATCH   1

/* ================================================================= *
 *  VCAP ES0
 * ================================================================= */

/* ES0 PUSH_OUTER_TAG values */
#define ES0_ACT_PUSH_OT_PORT_ENA 0
#define ES0_ACT_PUSH_OT_ES0      1
#define ES0_ACT_PUSH_OT_PORT     2
#define ES0_ACT_PUSH_OT_NONE     3

/* ES0 PCP_SEL values */
#define ES0_ACT_PCP_SEL_CL_PCP  0
#define ES0_ACT_PCP_SEL_PCP_ES0 1
#define ES0_ACT_PCP_SEL_MAPPED  2
#define ES0_ACT_PCP_SEL_QOS     3

/* ES0 DEI_SEL values */
#define ES0_ACT_DEI_SEL_CL_DEI  0
#define ES0_ACT_DEI_SEL_DEI_ES0 1
#define ES0_ACT_DEI_SEL_MAPPED  2
#define ES0_ACT_DEI_SEL_DP      3

/* ES0 IPT_CFG values */
#define ES0_ACT_IPT_CFG_NONE       0
#define ES0_ACT_IPT_CFG_WORKING    1
#define ES0_ACT_IPT_CFG_PROTECTION 2
#define ES0_ACT_IPT_CFG_RESV       3

/* ES0 full key */
#define ES0_FKO_EGR_PORT    0
#define ES0_FKL_EGR_PORT    4
#define ES0_FKO_IGR_PORT    4
#define ES0_FKL_IGR_PORT    4
#define ES0_FKO_SERVICE_FRM 8
#define ES0_FKL_SERVICE_FRM 1
#define ES0_FKO_KEY_ISDX    9
#define ES0_FKL_KEY_ISDX    1
#define ES0_FKO_L2_MC       10
#define ES0_FKL_L2_MC       1
#define ES0_FKO_L2_BC       11
#define ES0_FKL_L2_BC       1
#define ES0_FKO_VID         12
#define ES0_FKL_VID         12
#define ES0_FKO_DEI         24
#define ES0_FKL_DEI         1
#define ES0_FKO_PCP         25
#define ES0_FKL_PCP         3

/* ES0 action */
#define ES0_AO_PUSH_OUTER_TAG  0
#define ES0_AL_PUSH_OUTER_TAG  2
#define ES0_AO_PUSH_INNER_TAG  2
#define ES0_AL_PUSH_INNER_TAG  1
#define ES0_AO_TAG_A_TPID_SEL  3
#define ES0_AL_TAG_A_TPID_SEL  2
#define ES0_AO_TAG_A_VID_SEL   5
#define ES0_AL_TAG_A_VID_SEL   1
#define ES0_AO_TAG_A_PCP_SEL   6
#define ES0_AL_TAG_A_PCP_SEL   2
#define ES0_AO_TAG_A_DEI_SEL   8
#define ES0_AL_TAG_A_DEI_SEL   2
#define ES0_AO_TAG_B_TPID_SEL  10
#define ES0_AL_TAG_B_TPID_SEL  2
#define ES0_AO_TAG_B_VID_SEL   12
#define ES0_AL_TAG_B_VID_SEL   1
#define ES0_AO_TAG_B_PCP_SEL   13
#define ES0_AL_TAG_B_PCP_SEL   2
#define ES0_AO_TAG_B_DEI_SEL   15
#define ES0_AL_TAG_B_DEI_SEL   2
#define ES0_AO_VID_A_VAL       17
#define ES0_AL_VID_A_VAL       12
#define ES0_AO_PCP_A_VAL       29
#define ES0_AL_PCP_A_VAL       3
#define ES0_AO_DEI_A_VAL       32
#define ES0_AL_DEI_A_VAL       1
#define ES0_AO_VID_B_VAL       33
#define ES0_AL_VID_B_VAL       12
#define ES0_AO_PCP_B_VAL       45
#define ES0_AL_PCP_B_VAL       3
#define ES0_AO_DEI_B_VAL       48
#define ES0_AL_DEI_B_VAL       1
#define ES0_AO_ENCAP_ID        49
#define ES0_AL_ENCAP_ID        10
#define ES0_AO_ENCAP_LEN       59
#define ES0_AL_ENCAP_LEN       3
#define ES0_AO_MPOP_CNT        62
#define ES0_AL_MPOP_CNT        3
#define ES0_AO_ESDX            65
#define ES0_AL_ESDX            10
#define ES0_AO_OAM_MEP_IDX_VLD 75
#define ES0_AL_OAM_MEP_IDX_VLD 1
#define ES0_AO_OAM_MEP_IDX     76
#define ES0_AL_OAM_MEP_IDX     6
#define ES0_AO_IPT_CFG         82
#define ES0_AL_IPT_CFG         2
#define ES0_AO_PPT_IDX         84
#define ES0_AL_PPT_IDX         7

#endif /* _SERVAL_REG_H */

