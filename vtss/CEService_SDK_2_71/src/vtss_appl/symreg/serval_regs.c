#ifdef VTSS_ARCH_SERVAL
/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#include <stdio.h> /* For NULL */

typedef struct {
  char          *field_name;
  unsigned long field_pos;
  unsigned long field_width;
} SYMREG_field_t;

typedef struct {
  char            *name;
  unsigned long   addr;
  unsigned long   repl_cnt;
  unsigned long   repl_width;
//  REG_field_t   *fields;
} SYMREG_reg_t;

typedef struct {
  char               *name;
  unsigned long      base_addr;
  unsigned long      repl_cnt;
  unsigned long      repl_width;
  SYMREG_reg_t const *regs;
} SYMREG_reggrp_t;

typedef struct {
  char                  *name;
  int                   repl_number;
  unsigned long         tgt_id;
  unsigned long         base_addr;
  SYMREG_reggrp_t const *reggrps;
} SYMREG_target_t;

/*
 * Target offset base(s)
 */
#define VTSS_IO_ORIGIN1_OFFSET 0x70000000
#define VTSS_IO_ORIGIN1_SIZE   0x00200000
#define VTSS_IO_ORIGIN2_OFFSET 0x71000000
#define VTSS_IO_ORIGIN2_SIZE   0x01000000
#ifndef VTSS_IO_OFFSET1
#define VTSS_IO_OFFSET1(offset) (VTSS_IO_ORIGIN1_OFFSET + offset)
#endif
#ifndef VTSS_IO_OFFSET2
#define VTSS_IO_OFFSET2(offset) (VTSS_IO_ORIGIN2_OFFSET + offset)
#endif

static const SYMREG_reg_t regs_within_ANA_ANA[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ADVLEARN"                             , 0x00000000, 0x00000001, 0x00000001},
  {"VLANMASK"                             , 0x00000001, 0x00000001, 0x00000001},
  {"PORT_B_DOMAIN"                        , 0x00000002, 0x00000001, 0x00000001},
  {"ANAGEFIL"                             , 0x00000003, 0x00000001, 0x00000001},
  {"ANEVENTS"                             , 0x00000004, 0x00000001, 0x00000001},
  {"STORMLIMIT_BURST"                     , 0x00000005, 0x00000001, 0x00000001},
  {"STORMLIMIT_CFG"                       , 0x00000006, 0x00000004, 0x00000001},
  {"ISOLATED_PORTS"                       , 0x0000000a, 0x00000001, 0x00000001},
  {"COMMUNITY_PORTS"                      , 0x0000000b, 0x00000001, 0x00000001},
  {"AUTOAGE"                              , 0x0000000c, 0x00000001, 0x00000001},
  {"MACTOPTIONS"                          , 0x0000000d, 0x00000001, 0x00000001},
  {"LEARNDISC"                            , 0x0000000e, 0x00000001, 0x00000001},
  {"AGENCTRL"                             , 0x0000000f, 0x00000001, 0x00000001},
  {"MIRRORPORTS"                          , 0x00000010, 0x00000001, 0x00000001},
  {"EMIRRORPORTS"                         , 0x00000011, 0x00000001, 0x00000001},
  {"FLOODING"                             , 0x00000012, 0x00000001, 0x00000001},
  {"FLOODING_IPMC"                        , 0x00000013, 0x00000001, 0x00000001},
  {"SFLOW_CFG"                            , 0x00000014, 0x0000000c, 0x00000001},
  {"PORT_MODE"                            , 0x00000020, 0x0000000d, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_PGID[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PGID"                                 , 0x00000000, 0x0000005c, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_ANA_TABLES[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ANMOVED"                              , 0x0000000c, 0x00000001, 0x00000001},
  {"MACHDATA"                             , 0x0000000d, 0x00000001, 0x00000001},
  {"MACLDATA"                             , 0x0000000e, 0x00000001, 0x00000001},
  {"MACACCESS"                            , 0x0000000f, 0x00000001, 0x00000001},
  {"MACTINDX"                             , 0x00000010, 0x00000001, 0x00000001},
  {"VLANACCESS"                           , 0x00000011, 0x00000001, 0x00000001},
  {"VLANTIDX"                             , 0x00000012, 0x00000001, 0x00000001},
  {"ISDXACCESS"                           , 0x00000013, 0x00000001, 0x00000001},
  {"ISDXTIDX"                             , 0x00000014, 0x00000001, 0x00000001},
  {"ENTRYLIM"                             , 0x00000000, 0x0000000c, 0x00000001},
  {"PTP_ID_HIGH"                          , 0x00000015, 0x00000001, 0x00000001},
  {"PTP_ID_LOW"                           , 0x00000016, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_MSTI_STATE[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MSTI_STATE"                           , 0x00000000, 0x00000080, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_OAM_UPM_LM_CNT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"OAM_UPM_LM_CNT"                       , 0x00000000, 0x00000200, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_MPLS_CL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"OAM_CONF"                             , 0x00000000, 0x00000001, 0x00000001},
  {"FRAME_EXTRACT"                        , 0x00000001, 0x00000007, 0x00000001},
  {"RSVD_LABELS_ENA"                      , 0x00000008, 0x00000010, 0x00000001},
  {"SEGMENT_OAM"                          , 0x00000018, 0x0000000d, 0x00000001},
  {"EXCP_VPROFILE_CONF"                   , 0x00000025, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_MPLS_TC_MAP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TC_MAP_TBL"                           , 0x00000000, 0x00000008, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_PORT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VLAN_CFG"                             , 0x00000000, 0x00000001, 0x00000001},
  {"DROP_CFG"                             , 0x00000001, 0x00000001, 0x00000001},
  {"QOS_CFG"                              , 0x00000002, 0x00000001, 0x00000001},
  {"VCAP_CFG"                             , 0x00000003, 0x00000001, 0x00000001},
  {"VCAP_S1_KEY_CFG"                      , 0x00000004, 0x00000003, 0x00000001},
  {"VCAP_S2_CFG"                          , 0x00000007, 0x00000001, 0x00000001},
  {"QOS_PCP_DEI_MAP_CFG"                  , 0x00000008, 0x00000010, 0x00000001},
  {"CPU_FWD_CFG"                          , 0x00000018, 0x00000001, 0x00000001},
  {"CPU_FWD_BPDU_CFG"                     , 0x00000019, 0x00000001, 0x00000001},
  {"CPU_FWD_GARP_CFG"                     , 0x0000001a, 0x00000001, 0x00000001},
  {"CPU_FWD_CCM_CFG"                      , 0x0000001b, 0x00000001, 0x00000001},
  {"PORT_CFG"                             , 0x0000001c, 0x00000001, 0x00000001},
  {"POL_CFG"                              , 0x0000001d, 0x00000001, 0x00000001},
  {"PTP_CFG"                              , 0x0000001e, 0x00000001, 0x00000001},
  {"PTP_DLY1_CFG"                         , 0x0000001f, 0x00000001, 0x00000001},
  {"PTP_DLY2_CFG"                         , 0x00000020, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_PFC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PFC_CFG"                              , 0x00000000, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_IPT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"OAM_MEP_CFG"                          , 0x00000000, 0x00000001, 0x00000001},
  {"IPT"                                  , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_PPT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PPT"                                  , 0x00000000, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_FID_MAP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"FID_MAP"                              , 0x00000000, 0x00001000, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"AGGR_CFG"                             , 0x00000000, 0x00000001, 0x00000001},
  {"CPUQ_CFG"                             , 0x00000001, 0x00000001, 0x00000001},
  {"CPUQ_CFG2"                            , 0x00000002, 0x00000001, 0x00000001},
  {"CPUQ_8021_CFG"                        , 0x00000003, 0x00000010, 0x00000001},
  {"DSCP_CFG"                             , 0x00000013, 0x00000040, 0x00000001},
  {"DSCP_REWR_CFG"                        , 0x00000053, 0x00000010, 0x00000001},
  {"VCAP_RNG_TYPE_CFG"                    , 0x00000063, 0x00000008, 0x00000001},
  {"VCAP_RNG_VAL_CFG"                     , 0x0000006b, 0x00000008, 0x00000001},
  {"VRAP_CFG"                             , 0x00000073, 0x00000001, 0x00000001},
  {"VRAP_HDR_DATA"                        , 0x00000074, 0x00000001, 0x00000001},
  {"VRAP_HDR_MASK"                        , 0x00000075, 0x00000001, 0x00000001},
  {"DISCARD_CFG"                          , 0x00000076, 0x00000001, 0x00000001},
  {"FID_CFG"                              , 0x00000077, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_POL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"POL_PIR_CFG"                          , 0x00000000, 0x00000001, 0x00000001},
  {"POL_CIR_CFG"                          , 0x00000001, 0x00000001, 0x00000001},
  {"POL_MODE_CFG"                         , 0x00000002, 0x00000001, 0x00000001},
  {"POL_PIR_STATE"                        , 0x00000003, 0x00000001, 0x00000001},
  {"POL_CIR_STATE"                        , 0x00000004, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_POL_MISC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"POL_FLOWC"                            , 0x00000000, 0x0000001b, 0x00000001},
  {"POL_HYST"                             , 0x0000001b, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ANA[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"ANA"                                  , 0x00005000, 0x00000001, 0x0000002d, regs_within_ANA_ANA},
  {"PGID"                                 , 0x00002700, 0x00000001, 0x00000080, regs_within_ANA_PGID},
  {"ANA_TABLES"                           , 0x000026c0, 0x00000001, 0x00000020, regs_within_ANA_ANA_TABLES},
  {"MSTI_STATE"                           , 0x00002780, 0x00000001, 0x00000080, regs_within_ANA_MSTI_STATE},
  {"OAM_UPM_LM_CNT"                       , 0x00002400, 0x00000001, 0x00000200, regs_within_ANA_OAM_UPM_LM_CNT},
  {"MPLS_CL"                              , 0x0000502d, 0x00000001, 0x00000033, regs_within_ANA_MPLS_CL},
  {"MPLS_TC_MAP"                          , 0x00005060, 0x00000008, 0x00000008, regs_within_ANA_MPLS_TC_MAP},
  {"PORT"                                 , 0x00003000, 0x00000040, 0x00000040, regs_within_ANA_PORT},
  {"PFC"                                  , 0x00002600, 0x0000000b, 0x00000010, regs_within_ANA_PFC},
  {"IPT"                                  , 0x00002800, 0x00000400, 0x00000002, regs_within_ANA_IPT},
  {"PPT"                                  , 0x000026b0, 0x00000001, 0x00000004, regs_within_ANA_PPT},
  {"FID_MAP"                              , 0x00004000, 0x00000001, 0x00001000, regs_within_ANA_FID_MAP},
  {"COMMON"                               , 0x000050a0, 0x00000001, 0x00000078, regs_within_ANA_COMMON},
  {"POL"                                  , 0x00000000, 0x00000480, 0x00000008, regs_within_ANA_POL},
  {"POL_MISC"                             , 0x000026e0, 0x00000001, 0x0000001d, regs_within_ANA_POL_MISC},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEV_PORT_MODE[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CLOCK_CFG"                            , 0x00000000, 0x00000001, 0x00000001},
  {"PORT_MISC"                            , 0x00000001, 0x00000001, 0x00000001},



  {"EEE_CFG"                              , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV_MAC_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MAC_ENA_CFG"                          , 0x00000000, 0x00000001, 0x00000001},
  {"MAC_MODE_CFG"                         , 0x00000001, 0x00000001, 0x00000001},
  {"MAC_MAXLEN_CFG"                       , 0x00000002, 0x00000001, 0x00000001},
  {"MAC_TAGS_CFG"                         , 0x00000003, 0x00000001, 0x00000001},
  {"MAC_ADV_CHK_CFG"                      , 0x00000004, 0x00000001, 0x00000001},
  {"MAC_IFG_CFG"                          , 0x00000005, 0x00000001, 0x00000001},
  {"MAC_HDX_CFG"                          , 0x00000006, 0x00000001, 0x00000001},



  {"MAC_FC_MAC_LOW_CFG"                   , 0x00000008, 0x00000001, 0x00000001},
  {"MAC_FC_MAC_HIGH_CFG"                  , 0x00000009, 0x00000001, 0x00000001},
  {"MAC_STICKY"                           , 0x0000000a, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV_PCS1G_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS1G_CFG"                            , 0x00000000, 0x00000001, 0x00000001},
  {"PCS1G_MODE_CFG"                       , 0x00000001, 0x00000001, 0x00000001},
  {"PCS1G_SD_CFG"                         , 0x00000002, 0x00000001, 0x00000001},
  {"PCS1G_ANEG_CFG"                       , 0x00000003, 0x00000001, 0x00000001},
  {"PCS1G_ANEG_NP_CFG"                    , 0x00000004, 0x00000001, 0x00000001},
  {"PCS1G_LB_CFG"                         , 0x00000005, 0x00000001, 0x00000001},






  {"PCS1G_ANEG_STATUS"                    , 0x00000008, 0x00000001, 0x00000001},
  {"PCS1G_ANEG_NP_STATUS"                 , 0x00000009, 0x00000001, 0x00000001},
  {"PCS1G_LINK_STATUS"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"PCS1G_LINK_DOWN_CNT"                  , 0x0000000b, 0x00000001, 0x00000001},
  {"PCS1G_STICKY"                         , 0x0000000c, 0x00000001, 0x00000001},



  {"PCS1G_LPI_CFG"                        , 0x0000000e, 0x00000001, 0x00000001},
  {"PCS1G_LPI_WAKE_ERROR_CNT"             , 0x0000000f, 0x00000001, 0x00000001},
  {"PCS1G_LPI_STATUS"                     , 0x00000010, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV_PCS1G_TSTPAT_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS1G_TSTPAT_MODE_CFG"                , 0x00000000, 0x00000001, 0x00000001},
  {"PCS1G_TSTPAT_STATUS"                  , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV_PCS_FX100_CONFIGURATION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_FX100_CFG"                        , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV_PCS_FX100_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_FX100_STATUS"                     , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEV[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"PORT_MODE"                            , 0x00000000, 0x00000001, 0x00000004, regs_within_DEV_PORT_MODE},
  {"MAC_CFG_STATUS"                       , 0x00000004, 0x00000001, 0x0000000b, regs_within_DEV_MAC_CFG_STATUS},
  {"PCS1G_CFG_STATUS"                     , 0x0000000f, 0x00000001, 0x00000011, regs_within_DEV_PCS1G_CFG_STATUS},
  {"PCS1G_TSTPAT_CFG_STATUS"              , 0x00000020, 0x00000001, 0x00000002, regs_within_DEV_PCS1G_TSTPAT_CFG_STATUS},
  {"PCS_FX100_CONFIGURATION"              , 0x00000022, 0x00000001, 0x00000001, regs_within_DEV_PCS_FX100_CONFIGURATION},
  {"PCS_FX100_STATUS"                     , 0x00000023, 0x00000001, 0x00000001, regs_within_DEV_PCS_FX100_STATUS},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_CHIP_REGS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CHIP_ID"                              , 0x00000000, 0x00000001, 0x00000001},
  {"GPR"                                  , 0x00000001, 0x00000001, 0x00000001},
  {"SOFT_RST"                             , 0x00000002, 0x00000001, 0x00000001},












  {NULL, 0, 0, 0}
};







static const SYMREG_reg_t regs_within_DEVCPU_GCB_VCORE_ACCESS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VA_CTRL"                              , 0x00000000, 0x00000001, 0x00000001},
  {"VA_ADDR"                              , 0x00000001, 0x00000001, 0x00000001},
  {"VA_DATA"                              , 0x00000002, 0x00000001, 0x00000001},
  {"VA_DATA_INCR"                         , 0x00000003, 0x00000001, 0x00000001},
  {"VA_DATA_INERT"                        , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_GPIO[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"GPIO_OUT_SET"                         , 0x00000000, 0x00000001, 0x00000001},
  {"GPIO_OUT_CLR"                         , 0x00000001, 0x00000001, 0x00000001},
  {"GPIO_OUT"                             , 0x00000002, 0x00000001, 0x00000001},
  {"GPIO_IN"                              , 0x00000003, 0x00000001, 0x00000001},
  {"GPIO_OE"                              , 0x00000004, 0x00000001, 0x00000001},
  {"GPIO_INTR"                            , 0x00000005, 0x00000001, 0x00000001},
  {"GPIO_INTR_ENA"                        , 0x00000006, 0x00000001, 0x00000001},
  {"GPIO_INTR_IDENT"                      , 0x00000007, 0x00000001, 0x00000001},
  {"GPIO_ALT"                             , 0x00000008, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_MIIM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MII_STATUS"                           , 0x00000000, 0x00000001, 0x00000001},



  {"MII_CMD"                              , 0x00000002, 0x00000001, 0x00000001},
  {"MII_DATA"                             , 0x00000003, 0x00000001, 0x00000001},
  {"MII_CFG"                              , 0x00000004, 0x00000001, 0x00000001},
  {"MII_SCAN_0"                           , 0x00000005, 0x00000001, 0x00000001},
  {"MII_SCAN_1"                           , 0x00000006, 0x00000001, 0x00000001},
  {"MII_SCAN_LAST_RSLTS"                  , 0x00000007, 0x00000001, 0x00000001},
  {"MII_SCAN_LAST_RSLTS_VLD"              , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_MIIM_READ_SCAN[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MII_SCAN_RSLTS_STICKY"                , 0x00000000, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_TEMP_SENSOR[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TEMP_SENSOR_CTRL"                     , 0x00000000, 0x00000001, 0x00000001},
  {"TEMP_SENSOR_DATA"                     , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_SIO_CTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SIO_INPUT_DATA"                       , 0x00000000, 0x00000004, 0x00000001},
  {"SIO_INT_POL"                          , 0x00000004, 0x00000004, 0x00000001},
  {"SIO_PORT_INT_ENA"                     , 0x00000008, 0x00000001, 0x00000001},
  {"SIO_PORT_CONFIG"                      , 0x00000009, 0x00000020, 0x00000001},
  {"SIO_PORT_ENABLE"                      , 0x00000029, 0x00000001, 0x00000001},
  {"SIO_CONFIG"                           , 0x0000002a, 0x00000001, 0x00000001},
  {"SIO_CLOCK"                            , 0x0000002b, 0x00000001, 0x00000001},
  {"SIO_INT_REG"                          , 0x0000002c, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_FAN_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"FAN_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_FAN_STAT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"FAN_CNT"                              , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_PTP_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PTP_MISC_CFG"                         , 0x00000000, 0x00000001, 0x00000001},
  {"PTP_UPPER_LIMIT_CFG"                  , 0x00000001, 0x00000001, 0x00000001},
  {"PTP_UPPER_LIMIT_1_TIME_ADJ_CFG"       , 0x00000002, 0x00000001, 0x00000001},
  {"PTP_SYNC_INTR_ENA_CFG"                , 0x00000003, 0x00000001, 0x00000001},
  {"GEN_EXT_CLK_HIGH_PERIOD_CFG"          , 0x00000004, 0x00000002, 0x00000001},
  {"GEN_EXT_CLK_LOW_PERIOD_CFG"           , 0x00000006, 0x00000002, 0x00000001},
  {"GEN_EXT_CLK_CFG"                      , 0x00000008, 0x00000002, 0x00000001},
  {"CLK_ADJ_CFG"                          , 0x0000000a, 0x00000001, 0x00000001},
  {"CLK_ADJ_FRQ"                          , 0x0000000b, 0x00000001, 0x00000001},
  {"PTP_EXT_SYNC_TIME_CFG"                , 0x0000000c, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_PTP_STAT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PTP_CURRENT_TIME_STAT"                , 0x00000000, 0x00000001, 0x00000001},
  {"EXT_SYNC_CURRENT_TIME_STAT"           , 0x00000001, 0x00000002, 0x00000001},
  {"PTP_EVT_STAT"                         , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_MEMITGR[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MEMITGR_CTRL"                         , 0x00000000, 0x00000001, 0x00000001},
  {"MEMITGR_STAT"                         , 0x00000001, 0x00000001, 0x00000001},
  {"MEMITGR_INFO"                         , 0x00000002, 0x00000001, 0x00000001},
  {"MEMITGR_IDX"                          , 0x00000003, 0x00000001, 0x00000001},






  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_VRAP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VRAP_ACCESS_STAT"                     , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEVCPU_GCB[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CHIP_REGS"                            , 0x00000000, 0x00000001, 0x00000007, regs_within_DEVCPU_GCB_CHIP_REGS},



  {"VCORE_ACCESS"                         , 0x00000008, 0x00000001, 0x00000005, regs_within_DEVCPU_GCB_VCORE_ACCESS},
  {"GPIO"                                 , 0x0000000d, 0x00000001, 0x0000000a, regs_within_DEVCPU_GCB_GPIO},
  {"MIIM"                                 , 0x00000017, 0x00000002, 0x00000009, regs_within_DEVCPU_GCB_MIIM},
  {"MIIM_READ_SCAN"                       , 0x00000029, 0x00000001, 0x00000002, regs_within_DEVCPU_GCB_MIIM_READ_SCAN},
  {"TEMP_SENSOR"                          , 0x0000002b, 0x00000001, 0x00000002, regs_within_DEVCPU_GCB_TEMP_SENSOR},
  {"SIO_CTRL"                             , 0x0000002d, 0x00000001, 0x00000030, regs_within_DEVCPU_GCB_SIO_CTRL},
  {"FAN_CFG"                              , 0x0000005d, 0x00000001, 0x00000001, regs_within_DEVCPU_GCB_FAN_CFG},
  {"FAN_STAT"                             , 0x0000005e, 0x00000001, 0x00000001, regs_within_DEVCPU_GCB_FAN_STAT},
  {"PTP_CFG"                              , 0x0000005f, 0x00000001, 0x0000000e, regs_within_DEVCPU_GCB_PTP_CFG},
  {"PTP_STAT"                             , 0x0000006d, 0x00000001, 0x00000004, regs_within_DEVCPU_GCB_PTP_STAT},
  {"MEMITGR"                              , 0x00000071, 0x00000001, 0x00000006, regs_within_DEVCPU_GCB_MEMITGR},
  {"VRAP"                                 , 0x00000077, 0x00000001, 0x00000001, regs_within_DEVCPU_GCB_VRAP},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEVCPU_ORG_DEVCPU_ORG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"IF_CTRL"                              , 0x00000000, 0x00000001, 0x00000001},
  {"IF_CFGSTAT"                           , 0x00000001, 0x00000001, 0x00000001},
  {"ORG_CFG"                              , 0x00000002, 0x00000001, 0x00000001},
  {"ERR_CNTS"                             , 0x00000003, 0x00000001, 0x00000001},
  {"TIMEOUT_CFG"                          , 0x00000004, 0x00000001, 0x00000001},
  {"GPR"                                  , 0x00000005, 0x00000001, 0x00000001},
  {"MAILBOX_SET"                          , 0x00000006, 0x00000001, 0x00000001},
  {"MAILBOX_CLR"                          , 0x00000007, 0x00000001, 0x00000001},
  {"MAILBOX"                              , 0x00000008, 0x00000001, 0x00000001},
  {"SEMA_CFG"                             , 0x00000009, 0x00000001, 0x00000001},
  {"SEMA0"                                , 0x0000000a, 0x00000001, 0x00000001},
  {"SEMA0_OWNER"                          , 0x0000000b, 0x00000001, 0x00000001},
  {"SEMA1"                                , 0x0000000c, 0x00000001, 0x00000001},
  {"SEMA1_OWNER"                          , 0x0000000d, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEVCPU_ORG[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"DEVCPU_ORG"                           , 0x00000000, 0x00000001, 0x0000000e, regs_within_DEVCPU_ORG_DEVCPU_ORG},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEVCPU_QS_XTR[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"XTR_GRP_CFG"                          , 0x00000000, 0x00000002, 0x00000001},
  {"XTR_RD"                               , 0x00000002, 0x00000002, 0x00000001},
  {"XTR_FRM_PRUNING"                      , 0x00000004, 0x00000002, 0x00000001},
  {"XTR_FLUSH"                            , 0x00000006, 0x00000001, 0x00000001},
  {"XTR_DATA_PRESENT"                     , 0x00000007, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_QS_INJ[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"INJ_GRP_CFG"                          , 0x00000000, 0x00000002, 0x00000001},
  {"INJ_WR"                               , 0x00000002, 0x00000002, 0x00000001},
  {"INJ_CTRL"                             , 0x00000004, 0x00000002, 0x00000001},
  {"INJ_STATUS"                           , 0x00000006, 0x00000001, 0x00000001},
  {"INJ_ERR"                              , 0x00000007, 0x00000002, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEVCPU_QS[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"XTR"                                  , 0x00000000, 0x00000001, 0x00000009, regs_within_DEVCPU_QS_XTR},
  {"INJ"                                  , 0x00000009, 0x00000001, 0x0000000a, regs_within_DEVCPU_QS_INJ},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_VCAP_CORE_VCAP_CORE_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VCAP_UPDATE_CTRL"                     , 0x00000000, 0x00000001, 0x00000001},
  {"VCAP_MV_CFG"                          , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_CORE_VCAP_CORE_CACHE[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VCAP_ENTRY_DAT"                       , 0x00000000, 0x00000020, 0x00000001},
  {"VCAP_MASK_DAT"                        , 0x00000020, 0x00000020, 0x00000001},
  {"VCAP_ACTION_DAT"                      , 0x00000040, 0x00000020, 0x00000001},
  {"VCAP_CNT_DAT"                         , 0x00000060, 0x00000020, 0x00000001},
  {"VCAP_TG_DAT"                          , 0x00000080, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_CORE_VCAP_CORE_STICKY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VCAP_STICKY"                          , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_CORE_VCAP_CONST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ENTRY_WIDTH"                          , 0x00000000, 0x00000001, 0x00000001},
  {"ENTRY_CNT"                            , 0x00000001, 0x00000001, 0x00000001},
  {"ENTRY_SWCNT"                          , 0x00000002, 0x00000001, 0x00000001},
  {"ENTRY_TG_WIDTH"                       , 0x00000003, 0x00000001, 0x00000001},
  {"ACTION_DEF_CNT"                       , 0x00000004, 0x00000001, 0x00000001},
  {"ACTION_WIDTH"                         , 0x00000005, 0x00000001, 0x00000001},
  {"CNT_WIDTH"                            , 0x00000006, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_CORE_TCAM_BIST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TCAM_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},



  {"TCAM_STAT"                            , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_VCAP_CORE[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"VCAP_CORE_CFG"                        , 0x00000000, 0x00000001, 0x00000002, regs_within_VCAP_CORE_VCAP_CORE_CFG},
  {"VCAP_CORE_CACHE"                      , 0x00000002, 0x00000001, 0x00000081, regs_within_VCAP_CORE_VCAP_CORE_CACHE},
  {"VCAP_CORE_STICKY"                     , 0x00000083, 0x00000001, 0x00000001, regs_within_VCAP_CORE_VCAP_CORE_STICKY},
  {"VCAP_CONST"                           , 0x00000084, 0x00000001, 0x00000007, regs_within_VCAP_CORE_VCAP_CONST},
  {"TCAM_BIST"                            , 0x0000008b, 0x00000001, 0x00000003, regs_within_VCAP_CORE_TCAM_BIST},
  {NULL, 0, 0, 0, NULL}
};



























static const SYMREG_reg_t regs_within_HSIO_PLL5G_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PLL5G_STATUS0"                        , 0x00000000, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_HSIO_RCOMP_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RCOMP_STATUS"                         , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_SYNC_ETH_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SYNC_ETH_CFG0"                        , 0x00000000, 0x00000001, 0x00000001},
  {"SYNC_ETH_CFG1"                        , 0x00000001, 0x00000001, 0x00000001},
  {"SYNC_ETH_CFG2"                        , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_MCB_PLL5G_RCOMP_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MCB_PLL5G_RCOMP_ADDR_CFG"             , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_SERDES1G_ANA_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SERDES1G_DES_CFG"                     , 0x00000000, 0x00000001, 0x00000001},
  {"SERDES1G_IB_CFG"                      , 0x00000001, 0x00000001, 0x00000001},
  {"SERDES1G_OB_CFG"                      , 0x00000002, 0x00000001, 0x00000001},
  {"SERDES1G_SER_CFG"                     , 0x00000003, 0x00000001, 0x00000001},
  {"SERDES1G_COMMON_CFG"                  , 0x00000004, 0x00000001, 0x00000001},
  {"SERDES1G_PLL_CFG"                     , 0x00000005, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_HSIO_SERDES1G_DIG_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width















  {"SERDES1G_MISC_CFG"                    , 0x00000005, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_SERDES1G_DIG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SERDES1G_DFT_STATUS"                  , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_MCB_SERDES1G_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MCB_SERDES1G_ADDR_CFG"                , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_SERDES6G_ANA_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SERDES6G_DES_CFG"                     , 0x00000000, 0x00000001, 0x00000001},
  {"SERDES6G_IB_CFG"                      , 0x00000001, 0x00000001, 0x00000001},
  {"SERDES6G_IB_CFG1"                     , 0x00000002, 0x00000001, 0x00000001},
  {"SERDES6G_OB_CFG"                      , 0x00000003, 0x00000001, 0x00000001},
  {"SERDES6G_OB_CFG1"                     , 0x00000004, 0x00000001, 0x00000001},
  {"SERDES6G_SER_CFG"                     , 0x00000005, 0x00000001, 0x00000001},
  {"SERDES6G_COMMON_CFG"                  , 0x00000006, 0x00000001, 0x00000001},
  {"SERDES6G_PLL_CFG"                     , 0x00000007, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_HSIO_SERDES6G_DIG_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SERDES6G_DIG_CFG"                     , 0x00000000, 0x00000001, 0x00000001},


















  {"SERDES6G_MISC_CFG"                    , 0x00000007, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_HSIO_MCB_SERDES6G_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MCB_SERDES6G_ADDR_CFG"                , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_HSIO[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list



  {"PLL5G_STATUS"                         , 0x00000007, 0x00000001, 0x00000002, regs_within_HSIO_PLL5G_STATUS},



  {"RCOMP_STATUS"                         , 0x0000000a, 0x00000001, 0x00000001, regs_within_HSIO_RCOMP_STATUS},
  {"SYNC_ETH_CFG"                         , 0x0000000b, 0x00000001, 0x00000003, regs_within_HSIO_SYNC_ETH_CFG},
  {"MCB_PLL5G_RCOMP_CFG"                  , 0x0000000e, 0x00000001, 0x00000001, regs_within_HSIO_MCB_PLL5G_RCOMP_CFG},
  {"SERDES1G_ANA_CFG"                     , 0x0000000f, 0x00000001, 0x00000006, regs_within_HSIO_SERDES1G_ANA_CFG},



  {"SERDES1G_DIG_CFG"                     , 0x00000016, 0x00000001, 0x00000006, regs_within_HSIO_SERDES1G_DIG_CFG},
  {"SERDES1G_DIG_STATUS"                  , 0x0000001c, 0x00000001, 0x00000001, regs_within_HSIO_SERDES1G_DIG_STATUS},
  {"MCB_SERDES1G_CFG"                     , 0x0000001d, 0x00000001, 0x00000001, regs_within_HSIO_MCB_SERDES1G_CFG},
  {"SERDES6G_ANA_CFG"                     , 0x0000001e, 0x00000001, 0x00000008, regs_within_HSIO_SERDES6G_ANA_CFG},



  {"SERDES6G_DIG_CFG"                     , 0x00000027, 0x00000001, 0x00000009, regs_within_HSIO_SERDES6G_DIG_CFG},



  {"MCB_SERDES6G_CFG"                     , 0x00000031, 0x00000001, 0x00000001, regs_within_HSIO_MCB_SERDES6G_CFG},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_CPU_SYSTEM_CTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"GPR"                                  , 0x00000000, 0x00000008, 0x00000001},
  {"RESET"                                , 0x00000008, 0x00000001, 0x00000001},
  {"GENERAL_CTRL"                         , 0x00000009, 0x00000001, 0x00000001},
  {"GENERAL_STAT"                         , 0x0000000a, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};


















static const SYMREG_reg_t regs_within_ICPU_CFG_SPI_MST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SPI_MST_CFG"                          , 0x00000000, 0x00000001, 0x00000001},



  {"SW_MODE"                              , 0x00000005, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_MPU8051[] = {
  //reg name                              , addr      , repl_cnt  , repl_width



  {"MPU8051_STAT"                         , 0x00000001, 0x00000001, 0x00000001},
  {"MPU8051_MMAP"                         , 0x00000002, 0x00000001, 0x00000001},
  {"MEMACC_CTRL"                          , 0x00000003, 0x00000001, 0x00000001},
  {"MEMACC"                               , 0x00000004, 0x00000001, 0x00000001},
  {"MEMACC_SBA"                           , 0x00000005, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_INTR[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"INTR_RAW"                             , 0x00000000, 0x00000001, 0x00000001},
  {"INTR_TRIGGER"                         , 0x00000001, 0x00000001, 0x00000001},
  {"INTR_FORCE"                           , 0x00000002, 0x00000001, 0x00000001},
  {"INTR_STICKY"                          , 0x00000003, 0x00000001, 0x00000001},
  {"INTR_BYPASS"                          , 0x00000004, 0x00000001, 0x00000001},
  {"INTR_ENA"                             , 0x00000005, 0x00000001, 0x00000001},
  {"INTR_ENA_CLR"                         , 0x00000006, 0x00000001, 0x00000001},
  {"INTR_ENA_SET"                         , 0x00000007, 0x00000001, 0x00000001},
  {"INTR_IDENT"                           , 0x00000008, 0x00000001, 0x00000001},
  {"DST_INTR_MAP"                         , 0x00000009, 0x00000004, 0x00000001},
  {"DST_INTR_IDENT"                       , 0x0000000d, 0x00000004, 0x00000001},
  {"EXT_INTR_POL"                         , 0x00000011, 0x00000001, 0x00000001},
  {"EXT_INTR_DRV"                         , 0x00000012, 0x00000001, 0x00000001},
  {"EXT_INTR_DIR"                         , 0x00000013, 0x00000001, 0x00000001},
  {"DEV_INTR_POL"                         , 0x00000014, 0x00000001, 0x00000001},
  {"DEV_INTR_RAW"                         , 0x00000015, 0x00000001, 0x00000001},
  {"DEV_INTR_TRIGGER"                     , 0x00000016, 0x00000001, 0x00000001},
  {"DEV_INTR_STICKY"                      , 0x00000017, 0x00000001, 0x00000001},
  {"DEV_INTR_BYPASS"                      , 0x00000018, 0x00000001, 0x00000001},
  {"DEV_INTR_ENA"                         , 0x00000019, 0x00000001, 0x00000001},
  {"DEV_INTR_IDENT"                       , 0x0000001a, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_TIMERS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"WDT"                                  , 0x00000000, 0x00000001, 0x00000001},
  {"TIMER_TICK_DIV"                       , 0x00000001, 0x00000001, 0x00000001},
  {"TIMER_VALUE"                          , 0x00000002, 0x00000003, 0x00000001},
  {"TIMER_RELOAD_VALUE"                   , 0x00000005, 0x00000003, 0x00000001},
  {"TIMER_CTRL"                           , 0x00000008, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_MEMCTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MEMCTRL_CTRL"                         , 0x00000000, 0x00000001, 0x00000001},
  {"MEMCTRL_CFG"                          , 0x00000001, 0x00000001, 0x00000001},
  {"MEMCTRL_STAT"                         , 0x00000002, 0x00000001, 0x00000001},
  {"MEMCTRL_REF_PERIOD"                   , 0x00000003, 0x00000001, 0x00000001},
  {"MEMCTRL_ZQCAL"                        , 0x00000004, 0x00000001, 0x00000001},
  {"MEMCTRL_TIMING0"                      , 0x00000005, 0x00000001, 0x00000001},
  {"MEMCTRL_TIMING1"                      , 0x00000006, 0x00000001, 0x00000001},
  {"MEMCTRL_TIMING2"                      , 0x00000007, 0x00000001, 0x00000001},
  {"MEMCTRL_TIMING3"                      , 0x00000008, 0x00000001, 0x00000001},



  {"MEMCTRL_MR0_VAL"                      , 0x0000000a, 0x00000001, 0x00000001},
  {"MEMCTRL_MR1_VAL"                      , 0x0000000b, 0x00000001, 0x00000001},
  {"MEMCTRL_MR2_VAL"                      , 0x0000000c, 0x00000001, 0x00000001},
  {"MEMCTRL_MR3_VAL"                      , 0x0000000d, 0x00000001, 0x00000001},
  {"MEMCTRL_TERMRES_CTRL"                 , 0x0000000e, 0x00000001, 0x00000001},



  {"MEMCTRL_DQS_DLY"                      , 0x00000010, 0x00000002, 0x00000001},



  {"MEMPHY_CFG"                           , 0x00000014, 0x00000001, 0x00000001},












  {"MEMPHY_ZCAL"                          , 0x0000001e, 0x00000001, 0x00000001},









  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_TWI_DELAY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TWI_CONFIG"                           , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_TWI_SPIKE_FILTER[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TWI_SPIKE_FILTER_CFG"                 , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_FDMA[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"FDMA_DCB_LLP"                         , 0x00000000, 0x00000004, 0x00000001},









  {"FDMA_DCB_LLP_PREV"                    , 0x00000010, 0x00000004, 0x00000001},
  {"FDMA_CH_STAT"                         , 0x00000014, 0x00000001, 0x00000001},
  {"FDMA_CH_SAFE"                         , 0x00000015, 0x00000001, 0x00000001},
  {"FDMA_CH_ACTIVATE"                     , 0x00000016, 0x00000001, 0x00000001},
  {"FDMA_CH_DISABLE"                      , 0x00000017, 0x00000001, 0x00000001},
  {"FDMA_CH_FORCEDIS"                     , 0x00000018, 0x00000001, 0x00000001},
  {"FDMA_CH_CNT"                          , 0x00000019, 0x00000004, 0x00000001},
  {"FDMA_CH_INJ_TOKEN_CNT"                , 0x0000001d, 0x00000002, 0x00000001},
  {"FDMA_CH_INJ_TOKEN_TICK_RLD"           , 0x0000001f, 0x00000002, 0x00000001},
  {"FDMA_CH_INJ_TOKEN_TICK_CNT"           , 0x00000021, 0x00000002, 0x00000001},
  {"FDMA_EVT_ERR"                         , 0x00000023, 0x00000001, 0x00000001},
  {"FDMA_EVT_ERR_CODE"                    , 0x00000024, 0x00000001, 0x00000001},
  {"FDMA_INTR_LLP"                        , 0x00000025, 0x00000001, 0x00000001},
  {"FDMA_INTR_LLP_ENA"                    , 0x00000026, 0x00000001, 0x00000001},
  {"FDMA_INTR_FRM"                        , 0x00000027, 0x00000001, 0x00000001},
  {"FDMA_INTR_FRM_ENA"                    , 0x00000028, 0x00000001, 0x00000001},
  {"FDMA_INTR_SIG"                        , 0x00000029, 0x00000001, 0x00000001},
  {"FDMA_INTR_SIG_ENA"                    , 0x0000002a, 0x00000001, 0x00000001},
  {"FDMA_INTR_ENA"                        , 0x0000002b, 0x00000001, 0x00000001},
  {"FDMA_INTR_IDENT"                      , 0x0000002c, 0x00000001, 0x00000001},
  {"FDMA_CH_CFG"                          , 0x0000002d, 0x00000004, 0x00000001},
  {"FDMA_GCFG"                            , 0x00000031, 0x00000001, 0x00000001},
  {"FDMA_GSTAT"                           , 0x00000032, 0x00000001, 0x00000001},
  {"FDMA_IDLECNT"                         , 0x00000033, 0x00000001, 0x00000001},
  {"FDMA_CONST"                           , 0x00000034, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_PCIE[] = {
  //reg name                              , addr      , repl_cnt  , repl_width



  {"PCIE_CFG"                             , 0x00000001, 0x00000001, 0x00000001},
  {"PCIE_STAT"                            , 0x00000002, 0x00000001, 0x00000001},
  {"PCIE_AUX_CFG"                         , 0x00000003, 0x00000001, 0x00000001},


















  {"PCIESLV_FDMA"                         , 0x0000000a, 0x00000001, 0x00000001},
  {"PCIESLV_SBA"                          , 0x0000000b, 0x00000001, 0x00000001},
  {"PCIEPCS_CFG"                          , 0x0000000c, 0x00000001, 0x00000001},



  {"PCIE_INTR"                            , 0x0000000e, 0x00000001, 0x00000001},
  {"PCIE_INTR_ENA"                        , 0x0000000f, 0x00000001, 0x00000001},
  {"PCIE_INTR_IDENT"                      , 0x00000010, 0x00000001, 0x00000001},
  {"PCIE_INTR_COMMON_CFG"                 , 0x00000011, 0x00000001, 0x00000001},
  {"PCIE_INTR_CFG"                        , 0x00000012, 0x00000002, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_MANUAL_XTRINJ[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MANUAL_XTR"                           , 0x00000000, 0x00001000, 0x00000001},
  {"MANUAL_INJ"                           , 0x00001000, 0x00001000, 0x00000001},
  {"MANUAL_CFG"                           , 0x00002000, 0x00000001, 0x00000001},
  {"MANUAL_INTR"                          , 0x00002001, 0x00000001, 0x00000001},
  {"MANUAL_INTR_ENA"                      , 0x00002002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ICPU_CFG[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CPU_SYSTEM_CTRL"                      , 0x00000000, 0x00000001, 0x0000000b, regs_within_ICPU_CFG_CPU_SYSTEM_CTRL},



  {"SPI_MST"                              , 0x0000000f, 0x00000001, 0x00000006, regs_within_ICPU_CFG_SPI_MST},
  {"MPU8051"                              , 0x00000015, 0x00000001, 0x00000007, regs_within_ICPU_CFG_MPU8051},
  {"INTR"                                 , 0x0000001c, 0x00000001, 0x0000001b, regs_within_ICPU_CFG_INTR},
  {"TIMERS"                               , 0x00000037, 0x00000001, 0x0000000b, regs_within_ICPU_CFG_TIMERS},
  {"MEMCTRL"                              , 0x00000042, 0x00000001, 0x00000022, regs_within_ICPU_CFG_MEMCTRL},
  {"TWI_DELAY"                            , 0x00000064, 0x00000001, 0x00000001, regs_within_ICPU_CFG_TWI_DELAY},
  {"TWI_SPIKE_FILTER"                     , 0x00000065, 0x00000001, 0x00000001, regs_within_ICPU_CFG_TWI_SPIKE_FILTER},
  {"FDMA"                                 , 0x00000066, 0x00000001, 0x00000035, regs_within_ICPU_CFG_FDMA},
  {"PCIE"                                 , 0x0000009b, 0x00000001, 0x00000016, regs_within_ICPU_CFG_PCIE},
  {"MANUAL_XTRINJ"                        , 0x00001000, 0x00000001, 0x00002003, regs_within_ICPU_CFG_MANUAL_XTRINJ},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_OAM_MEP_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MEP_CTRL"                             , 0x00000000, 0x00000001, 0x00000001},
  {"CPU_CFG"                              , 0x00000001, 0x00000001, 0x00000001},
  {"CPU_CFG_1"                            , 0x00000002, 0x00000001, 0x00000001},
  {"OAM_SDLB_CPU_COPY"                    , 0x00000003, 0x00000001, 0x00000001},
  {"OAM_GENERIC_CFG"                      , 0x00000004, 0x00000008, 0x00000001},
  {"CCM_PERIOD_CFG"                       , 0x0000000c, 0x00000007, 0x00000001},
  {"CCM_CTRL"                             , 0x00000013, 0x00000001, 0x00000001},
  {"CCM_SCAN_STICKY"                      , 0x00000014, 0x00000001, 0x00000001},
  {"MASTER_INTR_CTRL"                     , 0x00000015, 0x00000001, 0x00000001},
  {"INTR"                                 , 0x00000016, 0x00000003, 0x00000001},
  {"VOE_CFG"                              , 0x00000019, 0x00000040, 0x00000001},
  {"UPMEP_LM_CNT_VLD"                     , 0x00000059, 0x00000040, 0x00000001},
  {"VOE_MAP_CTRL"                         , 0x00000099, 0x00000040, 0x00000001},
  {"VOE_CNT_CTRL"                         , 0x000000d9, 0x0000004b, 0x00000001},
  {"COMMON_MEP_MC_MAC_LSB"                , 0x00000124, 0x00000001, 0x00000001},
  {"COMMON_MEP_MC_MAC_MSB"                , 0x00000125, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OAM_MEP_VOE[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"BASIC_CTRL"                           , 0x00000000, 0x00000001, 0x00000001},
  {"MEL_CTRL"                             , 0x00000001, 0x00000001, 0x00000001},
  {"OAM_CPU_COPY_CTRL"                    , 0x00000002, 0x00000001, 0x00000001},
  {"OAM_FWD_CTRL"                         , 0x00000003, 0x00000001, 0x00000001},
  {"OAM_SDLB_CTRL"                        , 0x00000004, 0x00000001, 0x00000001},
  {"OAM_CNT_OAM_CTRL"                     , 0x00000005, 0x00000001, 0x00000001},
  {"OAM_CNT_DATA_CTRL"                    , 0x00000006, 0x00000001, 0x00000001},
  {"MEP_UC_MAC_LSB"                       , 0x00000007, 0x00000001, 0x00000001},
  {"MEP_UC_MAC_MSB"                       , 0x00000008, 0x00000001, 0x00000001},
  {"OAM_HW_CTRL"                          , 0x00000009, 0x00000001, 0x00000001},
  {"LOOPBACK_CTRL"                        , 0x0000000a, 0x00000001, 0x00000001},
  {"LBR_RX_FRM_CNT"                       , 0x0000000b, 0x00000001, 0x00000001},
  {"LBR_RX_TRANSID_CFG"                   , 0x0000000c, 0x00000001, 0x00000001},
  {"LBM_TX_TRANSID_UPDATE"                , 0x0000000d, 0x00000001, 0x00000001},
  {"LBM_TX_TRANSID_CFG"                   , 0x0000000e, 0x00000001, 0x00000001},
  {"CCM_CFG"                              , 0x0000000f, 0x00000001, 0x00000001},
  {"CCM_RX_VLD_FC_CNT"                    , 0x00000010, 0x00000001, 0x00000001},
  {"CCM_RX_INVLD_FC_CNT"                  , 0x00000011, 0x00000001, 0x00000001},
  {"CCM_CAP_TX_FCF"                       , 0x00000012, 0x00000001, 0x00000001},
  {"CCM_CAP_RX_FCL"                       , 0x00000013, 0x00000001, 0x00000001},
  {"UPMEP_LMR_RX_LM_CNT"                  , 0x00000014, 0x00000001, 0x00000001},
  {"CCM_TX_SEQ_CFG"                       , 0x00000015, 0x00000001, 0x00000001},
  {"CCM_RX_SEQ_CFG"                       , 0x00000016, 0x00000001, 0x00000001},
  {"CCM_MEPID_CFG"                        , 0x00000017, 0x00000001, 0x00000001},
  {"CCM_MEGID_CFG"                        , 0x00000018, 0x0000000c, 0x00000001},
  {"OAM_RX_STICKY"                        , 0x00000024, 0x00000001, 0x00000001},
  {"STICKY"                               , 0x00000025, 0x00000001, 0x00000001},
  {"UPMEP_LM_CNT_STICKY"                  , 0x00000026, 0x00000001, 0x00000001},
  {"INTR_ENA"                             , 0x00000027, 0x00000001, 0x00000001},
  {"RX_SEL_OAM_CNT"                       , 0x00000028, 0x00000001, 0x00000001},
  {"RX_OAM_FRM_CNT"                       , 0x00000029, 0x00000001, 0x00000001},
  {"TX_SEL_OAM_CNT"                       , 0x0000002a, 0x00000001, 0x00000001},
  {"TX_OAM_FRM_CNT"                       , 0x0000002b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OAM_MEP_PORT_PM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PORT_RX_FRM_CNT"                      , 0x00000000, 0x00000001, 0x00000001},
  {"PORT_TX_FRM_CNT"                      , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OAM_MEP_RX_VOE_PM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MEP_RX_FRM_CNT"                       , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OAM_MEP_TX_VOE_PM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MEP_TX_FRM_CNT"                       , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_OAM_MEP[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"COMMON"                               , 0x00001800, 0x00000001, 0x00000126, regs_within_OAM_MEP_COMMON},
  {"VOE"                                  , 0x00000000, 0x0000004b, 0x00000040, regs_within_OAM_MEP_VOE},
  {"PORT_PM"                              , 0x00001300, 0x00000058, 0x00000002, regs_within_OAM_MEP_PORT_PM},
  {"RX_VOE_PM"                            , 0x00001400, 0x00000200, 0x00000001, regs_within_OAM_MEP_RX_VOE_PM},
  {"TX_VOE_PM"                            , 0x00001600, 0x00000200, 0x00000001, regs_within_OAM_MEP_TX_VOE_PM},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_PCIE_PCIE[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CONF_SPACE"                           , 0x00000000, 0x000001c0, 0x00000001},





















































































































































































































  {"ATU_REGION"                           , 0x00000240, 0x00000001, 0x00000001},
  {"ATU_CFG1"                             , 0x00000241, 0x00000001, 0x00000001},
  {"ATU_CFG2"                             , 0x00000242, 0x00000001, 0x00000001},
  {"ATU_BASE_ADDR_LOW"                    , 0x00000243, 0x00000001, 0x00000001},



  {"ATU_LIMIT_ADDR"                       , 0x00000245, 0x00000001, 0x00000001},
  {"ATU_TGT_ADDR_LOW"                     , 0x00000246, 0x00000001, 0x00000001},
  {"ATU_TGT_ADDR_HIGH"                    , 0x00000247, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_PCIE[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"PCIE"                                 , 0x00000000, 0x00000001, 0x00000248, regs_within_PCIE_PCIE},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_QSYS_SYSTEM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PORT_MODE"                            , 0x00000000, 0x0000000d, 0x00000001},
  {"SWITCH_PORT_MODE"                     , 0x0000000d, 0x0000000c, 0x00000001},
  {"STAT_CNT_CFG"                         , 0x00000019, 0x00000001, 0x00000001},
  {"EEE_CFG"                              , 0x0000001a, 0x0000000b, 0x00000001},
  {"EEE_THRES"                            , 0x00000025, 0x00000001, 0x00000001},
  {"IGR_NO_SHARING"                       , 0x00000026, 0x00000001, 0x00000001},
  {"EGR_NO_SHARING"                       , 0x00000027, 0x00000001, 0x00000001},
  {"SW_STATUS"                            , 0x00000028, 0x0000000c, 0x00000001},
  {"EXT_CPU_CFG"                          , 0x00000034, 0x00000001, 0x00000001},



  {"CPU_GROUP_MAP"                        , 0x00000036, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_QSYS_QMAP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"QMAP"                                 , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_QSYS_SGRP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ISDX_SGRP"                            , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};







static const SYMREG_reg_t regs_within_QSYS_TIMED_FRAME_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TFRM_MISC"                            , 0x00000000, 0x00000001, 0x00000001},
  {"TFRM_PORT_DLY"                        , 0x00000001, 0x00000001, 0x00000001},
  {"TFRM_TIMER_CFG_1"                     , 0x00000002, 0x00000001, 0x00000001},
  {"TFRM_TIMER_CFG_2"                     , 0x00000003, 0x00000001, 0x00000001},
  {"TFRM_TIMER_CFG_3"                     , 0x00000004, 0x00000001, 0x00000001},
  {"TFRM_TIMER_CFG_4"                     , 0x00000005, 0x00000001, 0x00000001},
  {"TFRM_TIMER_CFG_5"                     , 0x00000006, 0x00000001, 0x00000001},
  {"TFRM_TIMER_CFG_6"                     , 0x00000007, 0x00000001, 0x00000001},
  {"TFRM_TIMER_CFG_7"                     , 0x00000008, 0x00000001, 0x00000001},
  {"TFRM_TIMER_CFG_8"                     , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_QSYS_RES_QOS_ADV[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RED_PROFILE"                          , 0x00000000, 0x00000010, 0x00000001},
  {"RES_QOS_MODE"                         , 0x00000010, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_QSYS_RES_CTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RES_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {"RES_STAT"                             , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_QSYS_DROP_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"EGR_DROP_MODE"                        , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_QSYS_MMGT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"EQ_CTRL"                              , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_QSYS_HSCH[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CIR_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {"EIR_CFG"                              , 0x00000001, 0x00000001, 0x00000001},
  {"SE_CFG"                               , 0x00000002, 0x00000001, 0x00000001},
  {"SE_DWRR_CFG"                          , 0x00000003, 0x0000000c, 0x00000001},
  {"SE_CONNECT"                           , 0x0000000f, 0x00000001, 0x00000001},
  {"SE_DLB_SENSE"                         , 0x00000010, 0x00000001, 0x00000001},
  {"CIR_STATE"                            , 0x00000011, 0x00000001, 0x00000001},
  {"EIR_STATE"                            , 0x00000012, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_QSYS_HSCH_MISC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"HSCH_MISC_CFG"                        , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_QSYS[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"SYSTEM"                               , 0x00005680, 0x00000001, 0x00000037, regs_within_QSYS_SYSTEM},
  {"QMAP"                                 , 0x000056b7, 0x0000000d, 0x00000001, regs_within_QSYS_QMAP},
  {"SGRP"                                 , 0x00006000, 0x00000400, 0x00000001, regs_within_QSYS_SGRP},



  {"TIMED_FRAME_CFG"                      , 0x000056c4, 0x00000001, 0x0000000a, regs_within_QSYS_TIMED_FRAME_CFG},
  {"RES_QOS_ADV"                          , 0x000056ce, 0x00000001, 0x00000011, regs_within_QSYS_RES_QOS_ADV},
  {"RES_CTRL"                             , 0x00005800, 0x00000400, 0x00000002, regs_within_QSYS_RES_CTRL},
  {"DROP_CFG"                             , 0x000056df, 0x00000001, 0x00000001, regs_within_QSYS_DROP_CFG},
  {"MMGT"                                 , 0x000056e0, 0x00000001, 0x00000001, regs_within_QSYS_MMGT},



  {"HSCH"                                 , 0x00000000, 0x000002b4, 0x00000020, regs_within_QSYS_HSCH},
  {"HSCH_MISC"                            , 0x000056e2, 0x00000001, 0x00000001, regs_within_QSYS_HSCH_MISC},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_REW_PORT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PORT_VLAN_CFG"                        , 0x00000000, 0x00000001, 0x00000001},
  {"TAG_CFG"                              , 0x00000001, 0x00000001, 0x00000001},
  {"PORT_CFG"                             , 0x00000002, 0x00000001, 0x00000001},
  {"DSCP_CFG"                             , 0x00000003, 0x00000001, 0x00000001},
  {"PCP_DEI_QOS_MAP_CFG"                  , 0x00000004, 0x00000010, 0x00000001},
  {"PTP_CFG"                              , 0x00000014, 0x00000001, 0x00000001},
  {"PTP_DLY1_CFG"                         , 0x00000015, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_REW_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DSCP_REMAP_DP1_CFG"                   , 0x00000000, 0x00000040, 0x00000001},
  {"DSCP_REMAP_CFG"                       , 0x00000040, 0x00000040, 0x00000001},
  {"STAT_CFG"                             , 0x00000080, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_REW_PPT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PPT"                                  , 0x00000000, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_REW[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"PORT"                                 , 0x00000000, 0x0000000d, 0x00000020, regs_within_REW_PORT},
  {"COMMON"                               , 0x000001a4, 0x00000001, 0x00000081, regs_within_REW_COMMON},
  {"PPT"                                  , 0x000001a0, 0x00000001, 0x00000004, regs_within_REW_PPT},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_SBA_SBA[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PL_CPU"                               , 0x00000000, 0x00000001, 0x00000001},
  {"PL_PCIE"                              , 0x00000001, 0x00000001, 0x00000001},
  {"PL_CSR"                               , 0x00000002, 0x00000001, 0x00000001},















  {"WT_EN"                                , 0x00000013, 0x00000001, 0x00000001},
  {"WT_TCL"                               , 0x00000014, 0x00000001, 0x00000001},
  {"WT_CPU"                               , 0x00000015, 0x00000001, 0x00000001},
  {"WT_PCIE"                              , 0x00000016, 0x00000001, 0x00000001},
  {"WT_CSR"                               , 0x00000017, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_SBA[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"SBA"                                  , 0x00000000, 0x00000001, 0x00000018, regs_within_SBA_SBA},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_SYS_SYSTEM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RESET_CFG"                            , 0x00000000, 0x00000001, 0x00000001},



  {"VLAN_ETYPE_CFG"                       , 0x00000002, 0x00000001, 0x00000001},
  {"PORT_MODE"                            , 0x00000003, 0x0000000d, 0x00000001},
  {"FRONT_PORT_MODE"                      , 0x00000010, 0x0000000b, 0x00000001},
  {"FRM_AGING"                            , 0x0000001b, 0x00000001, 0x00000001},
  {"STAT_CFG"                             , 0x0000001c, 0x00000001, 0x00000001},



  {"MPLS_QOS_MAP_CFG"                     , 0x00000029, 0x00000010, 0x00000001},
  {"IO_PATH_DELAY"                        , 0x00000039, 0x0000000c, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SYS_ENCAPSULATIONS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ENCAP_CTRL"                           , 0x00000000, 0x00000001, 0x00000001},
  {"ENCAP_DATA"                           , 0x00000001, 0x0000000a, 0x00000001},
  {NULL, 0, 0, 0}
};








static const SYMREG_reg_t regs_within_SYS_PAUSE_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PAUSE_CFG"                            , 0x00000000, 0x0000000c, 0x00000001},
  {"PAUSE_TOT_CFG"                        , 0x0000000c, 0x00000001, 0x00000001},
  {"ATOP"                                 , 0x0000000d, 0x0000000c, 0x00000001},
  {"ATOP_TOT_CFG"                         , 0x00000019, 0x00000001, 0x00000001},
  {"MAC_FC_CFG"                           , 0x0000001a, 0x0000000b, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SYS_MMGT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MMGT"                                 , 0x00000000, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};












static const SYMREG_reg_t regs_within_SYS_STAT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CNT"                                  , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SYS_PTP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PTP_STATUS"                           , 0x00000000, 0x00000001, 0x00000001},
  {"PTP_TXSTAMP"                          , 0x00000001, 0x00000001, 0x00000001},
  {"PTP_NXT"                              , 0x00000002, 0x00000001, 0x00000001},
  {"PTP_CFG"                              , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SYS_PTP_TOD[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PTP_TOD_MSB"                          , 0x00000000, 0x00000001, 0x00000001},
  {"PTP_TOD_LSB"                          , 0x00000001, 0x00000001, 0x00000001},
  {"PTP_TOD_NSEC"                         , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_SYS[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"SYSTEM"                               , 0x00000146, 0x00000001, 0x00000046, regs_within_SYS_SYSTEM},
  {"ENCAPSULATIONS"                       , 0x0000018c, 0x00000001, 0x0000000b, regs_within_SYS_ENCAPSULATIONS},



  {"PAUSE_CFG"                            , 0x00000197, 0x00000001, 0x00000025, regs_within_SYS_PAUSE_CFG},
  {"MMGT"                                 , 0x000001bc, 0x00000001, 0x00000002, regs_within_SYS_MMGT},



  {"STAT"                                 , 0x00000000, 0x00000140, 0x00000001, regs_within_SYS_STAT},
  {"PTP"                                  , 0x000001c3, 0x00000001, 0x00000004, regs_within_SYS_PTP},
  {"PTP_TOD"                              , 0x00000140, 0x00000001, 0x00000004, regs_within_SYS_PTP_TOD},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_TWI_TWI[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CFG"                                  , 0x00000000, 0x00000001, 0x00000001},
  {"TAR"                                  , 0x00000001, 0x00000001, 0x00000001},
  {"SAR"                                  , 0x00000002, 0x00000001, 0x00000001},



  {"DATA_CMD"                             , 0x00000004, 0x00000001, 0x00000001},
  {"SS_SCL_HCNT"                          , 0x00000005, 0x00000001, 0x00000001},
  {"SS_SCL_LCNT"                          , 0x00000006, 0x00000001, 0x00000001},
  {"FS_SCL_HCNT"                          , 0x00000007, 0x00000001, 0x00000001},
  {"FS_SCL_LCNT"                          , 0x00000008, 0x00000001, 0x00000001},



  {"INTR_STAT"                            , 0x0000000b, 0x00000001, 0x00000001},
  {"INTR_MASK"                            , 0x0000000c, 0x00000001, 0x00000001},
  {"RAW_INTR_STAT"                        , 0x0000000d, 0x00000001, 0x00000001},
  {"RX_TL"                                , 0x0000000e, 0x00000001, 0x00000001},
  {"TX_TL"                                , 0x0000000f, 0x00000001, 0x00000001},
  {"CLR_INTR"                             , 0x00000010, 0x00000001, 0x00000001},
  {"CLR_RX_UNDER"                         , 0x00000011, 0x00000001, 0x00000001},
  {"CLR_RX_OVER"                          , 0x00000012, 0x00000001, 0x00000001},
  {"CLR_TX_OVER"                          , 0x00000013, 0x00000001, 0x00000001},
  {"CLR_RD_REQ"                           , 0x00000014, 0x00000001, 0x00000001},
  {"CLR_TX_ABRT"                          , 0x00000015, 0x00000001, 0x00000001},
  {"CLR_RX_DONE"                          , 0x00000016, 0x00000001, 0x00000001},
  {"CLR_ACTIVITY"                         , 0x00000017, 0x00000001, 0x00000001},
  {"CLR_STOP_DET"                         , 0x00000018, 0x00000001, 0x00000001},
  {"CLR_START_DET"                        , 0x00000019, 0x00000001, 0x00000001},
  {"CLR_GEN_CALL"                         , 0x0000001a, 0x00000001, 0x00000001},
  {"CTRL"                                 , 0x0000001b, 0x00000001, 0x00000001},
  {"STAT"                                 , 0x0000001c, 0x00000001, 0x00000001},
  {"TXFLR"                                , 0x0000001d, 0x00000001, 0x00000001},
  {"RXFLR"                                , 0x0000001e, 0x00000001, 0x00000001},



  {"TX_ABRT_SOURCE"                       , 0x00000020, 0x00000001, 0x00000001},












  {"SDA_SETUP"                            , 0x00000025, 0x00000001, 0x00000001},
  {"ACK_GEN_CALL"                         , 0x00000026, 0x00000001, 0x00000001},
  {"ENABLE_STATUS"                        , 0x00000027, 0x00000001, 0x00000001},












  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_TWI[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"TWI"                                  , 0x00000000, 0x00000001, 0x00000040, regs_within_TWI_TWI},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_UART_UART[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RBR_THR"                              , 0x00000000, 0x00000001, 0x00000001},
  {"IER"                                  , 0x00000001, 0x00000001, 0x00000001},
  {"IIR_FCR"                              , 0x00000002, 0x00000001, 0x00000001},
  {"LCR"                                  , 0x00000003, 0x00000001, 0x00000001},
  {"MCR"                                  , 0x00000004, 0x00000001, 0x00000001},
  {"LSR"                                  , 0x00000005, 0x00000001, 0x00000001},
  {"MSR"                                  , 0x00000006, 0x00000001, 0x00000001},
  {"SCR"                                  , 0x00000007, 0x00000001, 0x00000001},



  {"USR"                                  , 0x0000001f, 0x00000001, 0x00000001},






  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_UART[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"UART"                                 , 0x00000000, 0x00000001, 0x0000002a, regs_within_UART_UART},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_target_t SYMREG_targets[] = {
  //target name         , repl, tgt_id    , base_addr                  , register group list
  {"ANA"                ,   -1, 0x00000090, VTSS_IO_OFFSET2(0x00900000), reggrps_within_ANA},
  {"DEV"                ,    0, 0x0000001e, VTSS_IO_OFFSET2(0x001e0000), reggrps_within_DEV},
  {"DEV"                ,    1, 0x0000001f, VTSS_IO_OFFSET2(0x001f0000), reggrps_within_DEV},
  {"DEV"                ,    2, 0x00000020, VTSS_IO_OFFSET2(0x00200000), reggrps_within_DEV},
  {"DEV"                ,    3, 0x00000021, VTSS_IO_OFFSET2(0x00210000), reggrps_within_DEV},
  {"DEV"                ,    4, 0x00000022, VTSS_IO_OFFSET2(0x00220000), reggrps_within_DEV},
  {"DEV"                ,    5, 0x00000023, VTSS_IO_OFFSET2(0x00230000), reggrps_within_DEV},
  {"DEV"                ,    6, 0x00000024, VTSS_IO_OFFSET2(0x00240000), reggrps_within_DEV},
  {"DEV"                ,    7, 0x00000025, VTSS_IO_OFFSET2(0x00250000), reggrps_within_DEV},
  {"DEV"                ,    8, 0x00000026, VTSS_IO_OFFSET2(0x00260000), reggrps_within_DEV},
  {"DEV"                ,    9, 0x00000027, VTSS_IO_OFFSET2(0x00270000), reggrps_within_DEV},
  {"DEV"                ,   10, 0x00000028, VTSS_IO_OFFSET2(0x00280000), reggrps_within_DEV},
  {"DEVCPU_GCB"         ,   -1, 0x00000007, VTSS_IO_OFFSET2(0x00070000), reggrps_within_DEVCPU_GCB},
  {"DEVCPU_ORG"         ,   -1, 0x00000000, VTSS_IO_OFFSET2(0x00000000), reggrps_within_DEVCPU_ORG},
  {"DEVCPU_QS"          ,   -1, 0x00000008, VTSS_IO_OFFSET2(0x00080000), reggrps_within_DEVCPU_QS},
  {"VCAP_CORE"          ,   -1, 0x00000004, VTSS_IO_OFFSET2(0x00040000), reggrps_within_VCAP_CORE},
  {"HSIO"               ,   -1, 0x0000000a, VTSS_IO_OFFSET2(0x000a0000), reggrps_within_HSIO},
  {"ICPU_CFG"           ,   -1, 0x000000f1, VTSS_IO_OFFSET1(0x00000000), reggrps_within_ICPU_CFG},
  {"VCAP_CORE"          ,   -1, 0x0000000b, VTSS_IO_OFFSET2(0x000b0000), reggrps_within_VCAP_CORE},
  {"VCAP_CORE"          ,   -1, 0x00000005, VTSS_IO_OFFSET2(0x00050000), reggrps_within_VCAP_CORE},
  {"VCAP_CORE"          ,   -1, 0x00000006, VTSS_IO_OFFSET2(0x00060000), reggrps_within_VCAP_CORE},
  {"OAM_MEP"            ,   -1, 0x0000000c, VTSS_IO_OFFSET2(0x000c0000), reggrps_within_OAM_MEP},
  {"PCIE"               ,   -1, 0x000000f5, VTSS_IO_OFFSET1(0x00101000), reggrps_within_PCIE},
  {"QSYS"               ,   -1, 0x00000080, VTSS_IO_OFFSET2(0x00800000), reggrps_within_QSYS},
  {"REW"                ,   -1, 0x00000003, VTSS_IO_OFFSET2(0x00030000), reggrps_within_REW},
  {"SBA"                ,   -1, 0x000000f4, VTSS_IO_OFFSET1(0x00110000), reggrps_within_SBA},
  {"SYS"                ,   -1, 0x00000001, VTSS_IO_OFFSET2(0x00010000), reggrps_within_SYS},
  {"TWI"                ,   -1, 0x000000f3, VTSS_IO_OFFSET1(0x00100400), reggrps_within_TWI},
  {"UART"               ,   -1, 0x000000f2, VTSS_IO_OFFSET1(0x00100000), reggrps_within_UART},
  {"UART"               ,   -1, 0x000000f6, VTSS_IO_OFFSET1(0x00100800), reggrps_within_UART},
};

#define SYMREG_REPL_CNT_MAX 4096
#define SYMREG_NAME_LEN_MAX 30
#endif /* VTSS_ARCH_SERVAL */
