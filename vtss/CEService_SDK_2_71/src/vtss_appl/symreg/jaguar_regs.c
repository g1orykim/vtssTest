#ifdef VTSS_ARCH_JAGUAR_1
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
#define VTSS_IO_ORIGIN1_OFFSET 0x60000000
#define VTSS_IO_ORIGIN1_SIZE   0x01000000
#define VTSS_IO_ORIGIN2_OFFSET 0x70000000
#define VTSS_IO_ORIGIN2_SIZE   0x00200000
#ifndef VTSS_IO_OFFSET1
#define VTSS_IO_OFFSET1(offset) (VTSS_IO_ORIGIN1_OFFSET + offset)
#endif
#ifndef VTSS_IO_OFFSET2
#define VTSS_IO_OFFSET2(offset) (VTSS_IO_ORIGIN2_OFFSET + offset)
#endif

static const SYMREG_reg_t regs_within_DEV1G_DEV_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DEV_RST_CTRL"                         , 0x00000000, 0x00000001, 0x00000001},



  {"DEV_LB_CFG"                           , 0x00000002, 0x00000001, 0x00000001},
  {"DEV_DBG_CFG"                          , 0x00000003, 0x00000001, 0x00000001},
  {"DEV_PORT_PROTECT"                     , 0x00000004, 0x00000001, 0x00000001},
  {"DEV_PTP_CFG"                          , 0x00000005, 0x00000001, 0x00000001},
  {"DEV_PTP_TX_STICKY"                    , 0x00000006, 0x00000001, 0x00000001},
  {"DEV_PTP_TX_TSTAMP"                    , 0x00000007, 0x00000003, 0x00000001},
  {"DEV_PTP_TX_ID"                        , 0x0000000a, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV1G_MAC_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MAC_ENA_CFG"                          , 0x00000000, 0x00000001, 0x00000001},
  {"MAC_MODE_CFG"                         , 0x00000001, 0x00000001, 0x00000001},
  {"MAC_MAXLEN_CFG"                       , 0x00000002, 0x00000001, 0x00000001},
  {"MAC_TAGS_CFG"                         , 0x00000003, 0x00000001, 0x00000001},
  {"MAC_ADV_CHK_CFG"                      , 0x00000004, 0x00000001, 0x00000001},
  {"MAC_IFG_CFG"                          , 0x00000005, 0x00000001, 0x00000001},
  {"MAC_HDX_CFG"                          , 0x00000006, 0x00000001, 0x00000001},
  {"MAC_STICKY"                           , 0x00000007, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV1G_PCS1G_CFG_STATUS[] = {
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









  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV1G_PCS1G_TSTPAT_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS1G_TSTPAT_MODE_CFG"                , 0x00000000, 0x00000001, 0x00000001},
  {"PCS1G_TSTPAT_STATUS"                  , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV1G_PCS_FX100_CONFIGURATION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_FX100_CFG"                        , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV1G_PCS_FX100_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_FX100_STATUS"                     , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV1G_DEV1G_INTR_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DEV1G_INTR_CFG"                       , 0x00000000, 0x00000001, 0x00000001},
  {"DEV1G_INTR"                           , 0x00000001, 0x00000001, 0x00000001},
  {"DEV1G_INTR_IDENT"                     , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEV1G[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"DEV_CFG_STATUS"                       , 0x00000000, 0x00000001, 0x0000000d, regs_within_DEV1G_DEV_CFG_STATUS},
  {"MAC_CFG_STATUS"                       , 0x0000000d, 0x00000001, 0x00000008, regs_within_DEV1G_MAC_CFG_STATUS},
  {"PCS1G_CFG_STATUS"                     , 0x00000015, 0x00000001, 0x00000010, regs_within_DEV1G_PCS1G_CFG_STATUS},
  {"PCS1G_TSTPAT_CFG_STATUS"              , 0x00000025, 0x00000001, 0x00000002, regs_within_DEV1G_PCS1G_TSTPAT_CFG_STATUS},
  {"PCS_FX100_CONFIGURATION"              , 0x00000027, 0x00000001, 0x00000001, regs_within_DEV1G_PCS_FX100_CONFIGURATION},
  {"PCS_FX100_STATUS"                     , 0x00000028, 0x00000001, 0x00000001, regs_within_DEV1G_PCS_FX100_STATUS},
  {"DEV1G_INTR_CFG_STATUS"                , 0x00000029, 0x00000001, 0x00000003, regs_within_DEV1G_DEV1G_INTR_CFG_STATUS},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEV2G5_DEV_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DEV_RST_CTRL"                         , 0x00000000, 0x00000001, 0x00000001},



  {"DEV_LB_CFG"                           , 0x00000002, 0x00000001, 0x00000001},
  {"DEV_DBG_CFG"                          , 0x00000003, 0x00000001, 0x00000001},
  {"DEV_PORT_PROTECT"                     , 0x00000004, 0x00000001, 0x00000001},
  {"DEV_PTP_CFG"                          , 0x00000005, 0x00000001, 0x00000001},
  {"DEV_PTP_TX_STICKY"                    , 0x00000006, 0x00000001, 0x00000001},
  {"DEV_PTP_TX_TSTAMP"                    , 0x00000007, 0x00000003, 0x00000001},
  {"DEV_PTP_TX_ID"                        , 0x0000000a, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV2G5_MAC_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MAC_ENA_CFG"                          , 0x00000000, 0x00000001, 0x00000001},
  {"MAC_MODE_CFG"                         , 0x00000001, 0x00000001, 0x00000001},
  {"MAC_MAXLEN_CFG"                       , 0x00000002, 0x00000001, 0x00000001},
  {"MAC_TAGS_CFG"                         , 0x00000003, 0x00000001, 0x00000001},
  {"MAC_ADV_CHK_CFG"                      , 0x00000004, 0x00000001, 0x00000001},
  {"MAC_IFG_CFG"                          , 0x00000005, 0x00000001, 0x00000001},
  {"MAC_HDX_CFG"                          , 0x00000006, 0x00000001, 0x00000001},
  {"MAC_STICKY"                           , 0x00000007, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV2G5_PCS1G_CFG_STATUS[] = {
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









  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV2G5_PCS1G_TSTPAT_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS1G_TSTPAT_MODE_CFG"                , 0x00000000, 0x00000001, 0x00000001},
  {"PCS1G_TSTPAT_STATUS"                  , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV2G5_PCS_FX100_CONFIGURATION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_FX100_CFG"                        , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV2G5_PCS_FX100_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_FX100_STATUS"                     , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV2G5_DEV2G5_INTR_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DEV2G5_INTR_CFG"                      , 0x00000000, 0x00000001, 0x00000001},
  {"DEV2G5_INTR"                          , 0x00000001, 0x00000001, 0x00000001},
  {"DEV2G5_INTR_IDENT"                    , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEV2G5[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"DEV_CFG_STATUS"                       , 0x00000000, 0x00000001, 0x0000000d, regs_within_DEV2G5_DEV_CFG_STATUS},
  {"MAC_CFG_STATUS"                       , 0x0000000d, 0x00000001, 0x00000008, regs_within_DEV2G5_MAC_CFG_STATUS},
  {"PCS1G_CFG_STATUS"                     , 0x00000015, 0x00000001, 0x00000010, regs_within_DEV2G5_PCS1G_CFG_STATUS},
  {"PCS1G_TSTPAT_CFG_STATUS"              , 0x00000025, 0x00000001, 0x00000002, regs_within_DEV2G5_PCS1G_TSTPAT_CFG_STATUS},
  {"PCS_FX100_CONFIGURATION"              , 0x00000027, 0x00000001, 0x00000001, regs_within_DEV2G5_PCS_FX100_CONFIGURATION},
  {"PCS_FX100_STATUS"                     , 0x00000028, 0x00000001, 0x00000001, regs_within_DEV2G5_PCS_FX100_STATUS},
  {"DEV2G5_INTR_CFG_STATUS"               , 0x00000029, 0x00000001, 0x00000003, regs_within_DEV2G5_DEV2G5_INTR_CFG_STATUS},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEVNPI_DEV_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DEV_RST_CTRL"                         , 0x00000000, 0x00000001, 0x00000001},



  {"DEV_LB_CFG"                           , 0x00000002, 0x00000001, 0x00000001},
  {"DEV_DBG_CFG"                          , 0x00000003, 0x00000001, 0x00000001},
  {"DEV_PORT_PROTECT"                     , 0x00000004, 0x00000001, 0x00000001},
  {"DEV_PTP_CFG"                          , 0x00000005, 0x00000001, 0x00000001},
  {"DEV_PTP_TX_STICKY"                    , 0x00000006, 0x00000001, 0x00000001},
  {"DEV_PTP_TX_TSTAMP"                    , 0x00000007, 0x00000003, 0x00000001},
  {"DEV_PTP_TX_ID"                        , 0x0000000a, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVNPI_MAC_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MAC_ENA_CFG"                          , 0x00000000, 0x00000001, 0x00000001},
  {"MAC_MODE_CFG"                         , 0x00000001, 0x00000001, 0x00000001},
  {"MAC_MAXLEN_CFG"                       , 0x00000002, 0x00000001, 0x00000001},
  {"MAC_TAGS_CFG"                         , 0x00000003, 0x00000001, 0x00000001},
  {"MAC_ADV_CHK_CFG"                      , 0x00000004, 0x00000001, 0x00000001},
  {"MAC_IFG_CFG"                          , 0x00000005, 0x00000001, 0x00000001},
  {"MAC_HDX_CFG"                          , 0x00000006, 0x00000001, 0x00000001},
  {"MAC_STICKY"                           , 0x00000007, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVNPI_PCS1G_CFG_STATUS[] = {
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



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVNPI_PCS1G_TSTPAT_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS1G_TSTPAT_MODE_CFG"                , 0x00000000, 0x00000001, 0x00000001},
  {"PCS1G_TSTPAT_STATUS"                  , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVNPI_PCS_FX100_CONFIGURATION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_FX100_CFG"                        , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVNPI_PCS_FX100_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_FX100_STATUS"                     , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVNPI_DEVNPI_INTR_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DEVNPI_INTR_CFG"                      , 0x00000000, 0x00000001, 0x00000001},
  {"DEVNPI_INTR"                          , 0x00000001, 0x00000001, 0x00000001},
  {"DEVNPI_INTR_IDENT"                    , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEVNPI[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"DEV_CFG_STATUS"                       , 0x00000000, 0x00000001, 0x0000000d, regs_within_DEVNPI_DEV_CFG_STATUS},
  {"MAC_CFG_STATUS"                       , 0x0000000d, 0x00000001, 0x00000008, regs_within_DEVNPI_MAC_CFG_STATUS},
  {"PCS1G_CFG_STATUS"                     , 0x00000015, 0x00000001, 0x00000010, regs_within_DEVNPI_PCS1G_CFG_STATUS},
  {"PCS1G_TSTPAT_CFG_STATUS"              , 0x00000025, 0x00000001, 0x00000002, regs_within_DEVNPI_PCS1G_TSTPAT_CFG_STATUS},
  {"PCS_FX100_CONFIGURATION"              , 0x00000027, 0x00000001, 0x00000001, regs_within_DEVNPI_PCS_FX100_CONFIGURATION},
  {"PCS_FX100_STATUS"                     , 0x00000028, 0x00000001, 0x00000001, regs_within_DEVNPI_PCS_FX100_STATUS},
  {"DEVNPI_INTR_CFG_STATUS"               , 0x00000029, 0x00000001, 0x00000003, regs_within_DEVNPI_DEVNPI_INTR_CFG_STATUS},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS1G_CFG_STATUS[] = {
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









  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS1G_TSTPAT_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS1G_TSTPAT_MODE_CFG"                , 0x00000000, 0x00000001, 0x00000001},
  {"PCS1G_TSTPAT_STATUS"                  , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_MAC_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MAC_ENA_CFG"                          , 0x00000000, 0x00000001, 0x00000001},
  {"MAC_MODE_CFG"                         , 0x00000001, 0x00000001, 0x00000001},
  {"MAC_MAXLEN_CFG"                       , 0x00000002, 0x00000001, 0x00000001},
  {"MAC_TAGS_CFG"                         , 0x00000003, 0x00000001, 0x00000001},
  {"MAC_ADV_CHK_CFG"                      , 0x00000004, 0x00000001, 0x00000001},
  {"MAC_LFS_CFG"                          , 0x00000005, 0x00000001, 0x00000001},
  {"MAC_LB_CFG"                           , 0x00000006, 0x00000001, 0x00000001},









  {"MAC_STICKY"                           , 0x0000000a, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_DEV_STATISTICS_32BIT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RX_SYMBOL_ERR_CNT"                    , 0x00000000, 0x00000001, 0x00000001},
  {"RX_PAUSE_CNT"                         , 0x00000001, 0x00000001, 0x00000001},
  {"RX_UNSUP_OPCODE_CNT"                  , 0x00000002, 0x00000001, 0x00000001},
  {"RX_UC_CNT"                            , 0x00000003, 0x00000001, 0x00000001},
  {"RX_MC_CNT"                            , 0x00000004, 0x00000001, 0x00000001},
  {"RX_BC_CNT"                            , 0x00000005, 0x00000001, 0x00000001},
  {"RX_CRC_ERR_CNT"                       , 0x00000006, 0x00000001, 0x00000001},
  {"RX_UNDERSIZE_CNT"                     , 0x00000007, 0x00000001, 0x00000001},
  {"RX_FRAGMENTS_CNT"                     , 0x00000008, 0x00000001, 0x00000001},
  {"RX_IN_RANGE_LEN_ERR_CNT"              , 0x00000009, 0x00000001, 0x00000001},
  {"RX_OUT_OF_RANGE_LEN_ERR_CNT"          , 0x0000000a, 0x00000001, 0x00000001},
  {"RX_OVERSIZE_CNT"                      , 0x0000000b, 0x00000001, 0x00000001},
  {"RX_JABBERS_CNT"                       , 0x0000000c, 0x00000001, 0x00000001},
  {"RX_SIZE64_CNT"                        , 0x0000000d, 0x00000001, 0x00000001},
  {"RX_SIZE65TO127_CNT"                   , 0x0000000e, 0x00000001, 0x00000001},
  {"RX_SIZE128TO255_CNT"                  , 0x0000000f, 0x00000001, 0x00000001},
  {"RX_SIZE256TO511_CNT"                  , 0x00000010, 0x00000001, 0x00000001},
  {"RX_SIZE512TO1023_CNT"                 , 0x00000011, 0x00000001, 0x00000001},
  {"RX_SIZE1024TO1518_CNT"                , 0x00000012, 0x00000001, 0x00000001},
  {"RX_SIZE1519TOMAX_CNT"                 , 0x00000013, 0x00000001, 0x00000001},
  {"RX_IPG_SHRINK_CNT"                    , 0x00000014, 0x00000001, 0x00000001},
  {"TX_PAUSE_CNT"                         , 0x00000015, 0x00000001, 0x00000001},
  {"TX_UC_CNT"                            , 0x00000016, 0x00000001, 0x00000001},
  {"TX_MC_CNT"                            , 0x00000017, 0x00000001, 0x00000001},
  {"TX_BC_CNT"                            , 0x00000018, 0x00000001, 0x00000001},
  {"TX_SIZE64_CNT"                        , 0x00000019, 0x00000001, 0x00000001},
  {"TX_SIZE65TO127_CNT"                   , 0x0000001a, 0x00000001, 0x00000001},
  {"TX_SIZE128TO255_CNT"                  , 0x0000001b, 0x00000001, 0x00000001},
  {"TX_SIZE256TO511_CNT"                  , 0x0000001c, 0x00000001, 0x00000001},
  {"TX_SIZE512TO1023_CNT"                 , 0x0000001d, 0x00000001, 0x00000001},
  {"TX_SIZE1024TO1518_CNT"                , 0x0000001e, 0x00000001, 0x00000001},
  {"TX_SIZE1519TOMAX_CNT"                 , 0x0000001f, 0x00000001, 0x00000001},
  {"RX_HIH_CKSM_ERR_CNT"                  , 0x00000020, 0x00000001, 0x00000001},
  {"RX_XGMII_PROT_ERR_CNT"                , 0x00000021, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_DEV_STATISTICS_40BIT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RX_IN_BYTES_CNT"                      , 0x00000000, 0x00000001, 0x00000001},
  {"RX_IN_BYTES_MSB_CNT"                  , 0x00000001, 0x00000001, 0x00000001},
  {"RX_OK_BYTES_CNT"                      , 0x00000002, 0x00000001, 0x00000001},
  {"RX_OK_BYTES_MSB_CNT"                  , 0x00000003, 0x00000001, 0x00000001},
  {"RX_BAD_BYTES_CNT"                     , 0x00000004, 0x00000001, 0x00000001},
  {"RX_BAD_BYTES_MSB_CNT"                 , 0x00000005, 0x00000001, 0x00000001},
  {"TX_OUT_BYTES_CNT"                     , 0x00000006, 0x00000001, 0x00000001},
  {"TX_OUT_BYTES_MSB_CNT"                 , 0x00000007, 0x00000001, 0x00000001},
  {"TX_OK_BYTES_CNT"                      , 0x00000008, 0x00000001, 0x00000001},
  {"TX_OK_BYTES_MSB_CNT"                  , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS_XAUI_CONFIGURATION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_XAUI_CFG"                         , 0x00000000, 0x00000001, 0x00000001},
  {"PCS_XAUI_EXT_CFG"                     , 0x00000001, 0x00000001, 0x00000001},
  {"PCS_XAUI_SD_CFG"                      , 0x00000002, 0x00000001, 0x00000001},
  {"PCS_XAUI_TX_SEQ_CFG"                  , 0x00000003, 0x00000001, 0x00000001},
  {"PCS_XAUI_RX_ERR_CNT_CFG"              , 0x00000004, 0x00000001, 0x00000001},
  {"PCS_XAUI_INTERLEAVE_MODE_CFG"         , 0x00000005, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS_XAUI_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_XAUI_RX_STATUS"                   , 0x00000000, 0x00000001, 0x00000001},
  {"PCS_XAUI_RX_ERROR_STATUS"             , 0x00000001, 0x00000001, 0x00000001},
  {"PCS_XAUI_RX_SEQ_REC_STATUS"           , 0x00000002, 0x00000001, 0x00000001},
  {"PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS", 0x00000003, 0x00000001, 0x00000001},
  {"PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS", 0x00000004, 0x00000001, 0x00000001},
  {"PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS" , 0x00000005, 0x00000001, 0x00000001},
  {"PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS", 0x00000006, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_XAUI_TSTPAT_CFG"                  , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS_XAUI_TSTPAT_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS_XAUI_TSTPAT_RX_SEQ_CNT_STATUS"    , 0x00000000, 0x00000001, 0x00000001},
  {"PCS_XAUI_TSTPAT_TX_SEQ_CNT_STATUS"    , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS2X6G_CONFIGURATION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS2X6G_CFG"                          , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS2X6G_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS2X6G_STATUS"                       , 0x00000000, 0x00000001, 0x00000001},
  {"PCS2X6G_ERR_STATUS"                   , 0x00000001, 0x00000001, 0x00000001},
  {"PCS2X6G_ERR_CNT_STAT"                 , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_PCS2X6G_EXT_CONFIGURATION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PCS2X6G_EXT_CFG"                      , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEV10G_DEV_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DEV_RST_CTRL"                         , 0x00000000, 0x00000001, 0x00000001},
  {"DEV_PORT_PROTECT"                     , 0x00000001, 0x00000001, 0x00000001},









  {"INTR"                                 , 0x00000005, 0x00000001, 0x00000001},
  {"INTR_ENA"                             , 0x00000006, 0x00000001, 0x00000001},
  {"INTR_IDENT"                           , 0x00000007, 0x00000001, 0x00000001},






  {"DEV_PTP_CFG"                          , 0x0000000a, 0x00000001, 0x00000001},
  {"DEV_PTP_TX_STICKY"                    , 0x0000000b, 0x00000001, 0x00000001},
  {"DEV_PTP_TX_TSTAMP"                    , 0x0000000c, 0x00000003, 0x00000001},
  {"DEV_PTP_TX_ID"                        , 0x0000000f, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEV10G[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"PCS1G_CFG_STATUS"                     , 0x00000000, 0x00000001, 0x00000010, regs_within_DEV10G_PCS1G_CFG_STATUS},
  {"PCS1G_TSTPAT_CFG_STATUS"              , 0x00000010, 0x00000001, 0x00000002, regs_within_DEV10G_PCS1G_TSTPAT_CFG_STATUS},
  {"MAC_CFG_STATUS"                       , 0x00000012, 0x00000001, 0x0000000b, regs_within_DEV10G_MAC_CFG_STATUS},
  {"DEV_STATISTICS_32BIT"                 , 0x0000001d, 0x00000001, 0x00000022, regs_within_DEV10G_DEV_STATISTICS_32BIT},
  {"DEV_STATISTICS_40BIT"                 , 0x0000003f, 0x00000001, 0x0000000a, regs_within_DEV10G_DEV_STATISTICS_40BIT},
  {"PCS_XAUI_CONFIGURATION"               , 0x00000049, 0x00000001, 0x00000007, regs_within_DEV10G_PCS_XAUI_CONFIGURATION},
  {"PCS_XAUI_STATUS"                      , 0x00000050, 0x00000001, 0x00000008, regs_within_DEV10G_PCS_XAUI_STATUS},
  {"PCS_XAUI_TSTPAT_CONFIGURATION"        , 0x00000058, 0x00000001, 0x00000001, regs_within_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION},
  {"PCS_XAUI_TSTPAT_STATUS"               , 0x00000059, 0x00000001, 0x00000002, regs_within_DEV10G_PCS_XAUI_TSTPAT_STATUS},
  {"PCS2X6G_CONFIGURATION"                , 0x0000005b, 0x00000001, 0x00000001, regs_within_DEV10G_PCS2X6G_CONFIGURATION},
  {"PCS2X6G_STATUS"                       , 0x0000005c, 0x00000001, 0x00000003, regs_within_DEV10G_PCS2X6G_STATUS},
  {"PCS2X6G_EXT_CONFIGURATION"            , 0x0000005f, 0x00000001, 0x00000001, regs_within_DEV10G_PCS2X6G_EXT_CONFIGURATION},
  {"DEV_CFG_STATUS"                       , 0x00000060, 0x00000001, 0x00000012, regs_within_DEV10G_DEV_CFG_STATUS},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_HSIO_PLL5G_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PLL5G_CFG0"                           , 0x00000000, 0x00000001, 0x00000001},
  {"PLL5G_CFG1"                           , 0x00000001, 0x00000001, 0x00000001},












  {NULL, 0, 0, 0}
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
  {"SYNC_ETH_CFG"                         , 0x00000000, 0x00000001, 0x00000001},
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












  {"SERDES1G_MISC_CFG"                    , 0x00000004, 0x00000001, 0x00000001},
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















  {"SERDES6G_MISC_CFG"                    , 0x00000006, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_HSIO_MCB_SERDES6G_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MCB_SERDES6G_ADDR_CFG"                , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_VAUI_CHANNEL_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VAUI_CHANNEL_CFG"                     , 0x00000000, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_ANEG_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ANEG_CFG"                             , 0x00000000, 0x00000001, 0x00000001},
  {"ANEG_ADV_ABILITY_0"                   , 0x00000001, 0x00000001, 0x00000001},
  {"ANEG_ADV_ABILITY_1"                   , 0x00000002, 0x00000001, 0x00000001},
  {"ANEG_NEXT_PAGE_0"                     , 0x00000003, 0x00000001, 0x00000001},
  {"ANEG_NEXT_PAGE_1"                     , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_HSIO_ANEG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ANEG_LP_ADV_ABILITY_0"                , 0x00000000, 0x00000001, 0x00000001},
  {"ANEG_LP_ADV_ABILITY_1"                , 0x00000001, 0x00000001, 0x00000001},
  {"ANEG_STATUS"                          , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_HSIO[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"PLL5G_CFG"                            , 0x00000000, 0x00000001, 0x00000006, regs_within_HSIO_PLL5G_CFG},
  {"PLL5G_STATUS"                         , 0x00000006, 0x00000001, 0x00000002, regs_within_HSIO_PLL5G_STATUS},



  {"RCOMP_STATUS"                         , 0x00000009, 0x00000001, 0x00000001, regs_within_HSIO_RCOMP_STATUS},
  {"SYNC_ETH_CFG"                         , 0x0000000a, 0x00000001, 0x00000001, regs_within_HSIO_SYNC_ETH_CFG},
  {"SERDES1G_ANA_CFG"                     , 0x0000000b, 0x00000001, 0x00000006, regs_within_HSIO_SERDES1G_ANA_CFG},



  {"SERDES1G_DIG_CFG"                     , 0x00000012, 0x00000001, 0x00000005, regs_within_HSIO_SERDES1G_DIG_CFG},
  {"SERDES1G_DIG_STATUS"                  , 0x00000017, 0x00000001, 0x00000001, regs_within_HSIO_SERDES1G_DIG_STATUS},
  {"MCB_SERDES1G_CFG"                     , 0x00000018, 0x00000001, 0x00000001, regs_within_HSIO_MCB_SERDES1G_CFG},
  {"SERDES6G_ANA_CFG"                     , 0x00000019, 0x00000001, 0x00000008, regs_within_HSIO_SERDES6G_ANA_CFG},



  {"SERDES6G_DIG_CFG"                     , 0x00000022, 0x00000001, 0x00000008, regs_within_HSIO_SERDES6G_DIG_CFG},



  {"MCB_SERDES6G_CFG"                     , 0x0000002b, 0x00000001, 0x00000001, regs_within_HSIO_MCB_SERDES6G_CFG},
  {"VAUI_CHANNEL_CFG"                     , 0x0000002c, 0x00000001, 0x00000004, regs_within_HSIO_VAUI_CHANNEL_CFG},
  {"ANEG_CFG"                             , 0x00000030, 0x00000010, 0x00000005, regs_within_HSIO_ANEG_CFG},
  {"ANEG_STATUS"                          , 0x00000080, 0x00000010, 0x00000003, regs_within_HSIO_ANEG_STATUS},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_ASM_DEV_STATISTICS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RX_IN_BYTES_CNT"                      , 0x00000000, 0x00000001, 0x00000001},
  {"RX_SYMBOL_ERR_CNT"                    , 0x00000001, 0x00000001, 0x00000001},
  {"RX_PAUSE_CNT"                         , 0x00000002, 0x00000001, 0x00000001},
  {"RX_UNSUP_OPCODE_CNT"                  , 0x00000003, 0x00000001, 0x00000001},
  {"RX_OK_BYTES_CNT"                      , 0x00000004, 0x00000001, 0x00000001},
  {"RX_BAD_BYTES_CNT"                     , 0x00000005, 0x00000001, 0x00000001},
  {"RX_UC_CNT"                            , 0x00000006, 0x00000001, 0x00000001},
  {"RX_MC_CNT"                            , 0x00000007, 0x00000001, 0x00000001},
  {"RX_BC_CNT"                            , 0x00000008, 0x00000001, 0x00000001},
  {"RX_CRC_ERR_CNT"                       , 0x00000009, 0x00000001, 0x00000001},
  {"RX_UNDERSIZE_CNT"                     , 0x0000000a, 0x00000001, 0x00000001},
  {"RX_FRAGMENTS_CNT"                     , 0x0000000b, 0x00000001, 0x00000001},
  {"RX_IN_RANGE_LEN_ERR_CNT"              , 0x0000000c, 0x00000001, 0x00000001},
  {"RX_OUT_OF_RANGE_LEN_ERR_CNT"          , 0x0000000d, 0x00000001, 0x00000001},
  {"RX_OVERSIZE_CNT"                      , 0x0000000e, 0x00000001, 0x00000001},
  {"RX_JABBERS_CNT"                       , 0x0000000f, 0x00000001, 0x00000001},
  {"RX_SIZE64_CNT"                        , 0x00000010, 0x00000001, 0x00000001},
  {"RX_SIZE65TO127_CNT"                   , 0x00000011, 0x00000001, 0x00000001},
  {"RX_SIZE128TO255_CNT"                  , 0x00000012, 0x00000001, 0x00000001},
  {"RX_SIZE256TO511_CNT"                  , 0x00000013, 0x00000001, 0x00000001},
  {"RX_SIZE512TO1023_CNT"                 , 0x00000014, 0x00000001, 0x00000001},
  {"RX_SIZE1024TO1518_CNT"                , 0x00000015, 0x00000001, 0x00000001},
  {"RX_SIZE1519TOMAX_CNT"                 , 0x00000016, 0x00000001, 0x00000001},
  {"RX_IPG_SHRINK_CNT"                    , 0x00000017, 0x00000001, 0x00000001},
  {"TX_OUT_BYTES_CNT"                     , 0x00000018, 0x00000001, 0x00000001},
  {"TX_PAUSE_CNT"                         , 0x00000019, 0x00000001, 0x00000001},
  {"TX_OK_BYTES_CNT"                      , 0x0000001a, 0x00000001, 0x00000001},
  {"TX_UC_CNT"                            , 0x0000001b, 0x00000001, 0x00000001},
  {"TX_MC_CNT"                            , 0x0000001c, 0x00000001, 0x00000001},
  {"TX_BC_CNT"                            , 0x0000001d, 0x00000001, 0x00000001},
  {"TX_SIZE64_CNT"                        , 0x0000001e, 0x00000001, 0x00000001},
  {"TX_SIZE65TO127_CNT"                   , 0x0000001f, 0x00000001, 0x00000001},
  {"TX_SIZE128TO255_CNT"                  , 0x00000020, 0x00000001, 0x00000001},
  {"TX_SIZE256TO511_CNT"                  , 0x00000021, 0x00000001, 0x00000001},
  {"TX_SIZE512TO1023_CNT"                 , 0x00000022, 0x00000001, 0x00000001},
  {"TX_SIZE1024TO1518_CNT"                , 0x00000023, 0x00000001, 0x00000001},
  {"TX_SIZE1519TOMAX_CNT"                 , 0x00000024, 0x00000001, 0x00000001},
  {"TX_MULTI_COLL_CNT"                    , 0x00000025, 0x00000001, 0x00000001},
  {"TX_LATE_COLL_CNT"                     , 0x00000026, 0x00000001, 0x00000001},
  {"TX_XCOLL_CNT"                         , 0x00000027, 0x00000001, 0x00000001},
  {"TX_DEFER_CNT"                         , 0x00000028, 0x00000001, 0x00000001},
  {"TX_XDEFER_CNT"                        , 0x00000029, 0x00000001, 0x00000001},
  {"TX_BACKOFF1_CNT"                      , 0x0000002a, 0x00000001, 0x00000001},



  {"RX_IN_BYTES_MSB_CNT"                  , 0x0000002c, 0x00000001, 0x00000001},
  {"RX_OK_BYTES_MSB_CNT"                  , 0x0000002d, 0x00000001, 0x00000001},
  {"RX_BAD_BYTES_MSB_CNT"                 , 0x0000002e, 0x00000001, 0x00000001},
  {"TX_OUT_BYTES_MSB_CNT"                 , 0x0000002f, 0x00000001, 0x00000001},
  {"TX_OK_BYTES_MSB_CNT"                  , 0x00000030, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ASM_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_CFG"                             , 0x00000000, 0x00000001, 0x00000001},
  {"CBC_CFG"                              , 0x00000001, 0x00000100, 0x00000001},
  {"CBC_LEN_CFG"                          , 0x00000101, 0x00000001, 0x00000001},
  {"MAC_ADDR_HIGH_CFG"                    , 0x00000102, 0x00000020, 0x00000001},
  {"MAC_ADDR_LOW_CFG"                     , 0x00000122, 0x00000020, 0x00000001},
  {"ETH_CFG"                              , 0x00000142, 0x00000020, 0x00000001},
  {"CPU_CFG"                              , 0x00000162, 0x00000002, 0x00000001},



  {"PAUSE_CFG"                            , 0x00000165, 0x00000020, 0x00000001},
  {NULL, 0, 0, 0}
};
























static const SYMREG_reg_t regs_within_ASM_CM_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CMEF_RX_CFG"                          , 0x00000000, 0x00000007, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ASM_CM_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CMEF_CNT"                             , 0x00000000, 0x00000002, 0x00000001},
  {"CMEF_DISCARD_STICKY"                  , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ASM_SP_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SP_RX_CFG"                            , 0x00000000, 0x00000007, 0x00000001},
  {"SP_UPSID_CFG"                         , 0x00000007, 0x00000001, 0x00000001},
  {"SP_KEEP_TTL_CFG"                      , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ASM_SP_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SP_STICKY"                            , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ASM[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"DEV_STATISTICS"                       , 0x00000000, 0x0000001c, 0x00000040, regs_within_ASM_DEV_STATISTICS},
  {"CFG"                                  , 0x00000700, 0x00000001, 0x00000185, regs_within_ASM_CFG},









  {"CM_CFG"                               , 0x000008b1, 0x00000001, 0x00000007, regs_within_ASM_CM_CFG},
  {"CM_STATUS"                            , 0x000008b8, 0x00000001, 0x00000003, regs_within_ASM_CM_STATUS},
  {"SP_CFG"                               , 0x000008bb, 0x00000001, 0x00000009, regs_within_ASM_SP_CFG},
  {"SP_STATUS"                            , 0x000008c4, 0x00000001, 0x00000001, regs_within_ASM_SP_STATUS},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_ANA_CL_INTEGRETY_IDENT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"INTEGRETY_IDENT"                      , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_CL_VQ_COUNTER[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DROP_CNT"                             , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_CL_PORT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"FILTER_CTRL"                          , 0x00000000, 0x00000001, 0x00000001},
  {"STACKING_CTRL"                        , 0x00000001, 0x00000001, 0x00000001},
  {"VLAN_CTRL"                            , 0x00000002, 0x00000001, 0x00000001},
  {"PORT_ID_CFG"                          , 0x00000003, 0x00000001, 0x00000001},
  {"UPRIO_MAP_CFG"                        , 0x00000004, 0x00000010, 0x00000001},
  {"QOS_CFG"                              , 0x00000014, 0x00000001, 0x00000001},
  {"CAPTURE_CFG"                          , 0x00000015, 0x00000001, 0x00000001},
  {"CAPTURE_Y1731_AG_CFG"                 , 0x00000016, 0x00000001, 0x00000001},
  {"CAPTURE_GXRP_CFG"                     , 0x00000017, 0x00000001, 0x00000001},
  {"CAPTURE_BPDU_CFG"                     , 0x00000018, 0x00000001, 0x00000001},
  {"ADV_CL_CFG"                           , 0x00000019, 0x00000001, 0x00000001},
  {"DP_CONFIG"                            , 0x0000001a, 0x00000010, 0x00000001},
  {"IS0_CFG_1"                            , 0x0000002a, 0x00000001, 0x00000001},
  {"IS0_CFG_2"                            , 0x0000002b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_CL_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"IFH_CHKSUM_CFG"                       , 0x00000000, 0x00000002, 0x00000001},
  {"UPSID_CFG"                            , 0x00000002, 0x00000001, 0x00000001},
  {"COMMON_CTRL"                          , 0x00000003, 0x00000001, 0x00000001},
  {"DSCP_CFG"                             , 0x00000004, 0x00000040, 0x00000001},
  {"QOS_MAP_CFG"                          , 0x00000044, 0x00000008, 0x00000001},
  {"AGGR_CFG"                             , 0x0000004c, 0x00000001, 0x00000001},
  {"VLAN_STAG_CFG"                        , 0x0000004d, 0x00000001, 0x00000001},
  {"CPU_BPDU_QU_CFG"                      , 0x0000004e, 0x00000010, 0x00000001},
  {"CPU_PROTO_QU_CFG"                     , 0x0000005e, 0x00000001, 0x00000001},
  {"ADV_RNG_CTRL"                         , 0x0000005f, 0x00000008, 0x00000001},
  {"ADV_RNG_VALUE_CFG"                    , 0x00000067, 0x00000008, 0x00000001},
  {"ADV_RNG_OFFSET_CFG"                   , 0x0000006f, 0x00000001, 0x00000001},
  {"VQ_CFG"                               , 0x00000070, 0x00000001, 0x00000001},
  {"VQ_ONE_SHOT"                          , 0x00000071, 0x00000001, 0x00000001},
  {"IFH_CFG"                              , 0x00000072, 0x00000001, 0x00000001},
  {"CUSTOM_STAG_ETYPE_CTRL"               , 0x00000073, 0x00000002, 0x00000001},
  {"HM_CFG"                               , 0x00000075, 0x00000004, 0x00000001},
  {"BPDU_AG_GARP_QOS"                     , 0x00000079, 0x00000010, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_CL_STICKY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"FILTER_STICKY"                        , 0x00000000, 0x00000001, 0x00000001},
  {"CLASS_STICKY"                         , 0x00000001, 0x00000001, 0x00000001},
  {"CAT_STICKY"                           , 0x00000002, 0x00000001, 0x00000001},
  {"IP_HDR_CHK_STICKY"                    , 0x00000003, 0x00000001, 0x00000001},
  {"VQ_STICKY_REG"                        , 0x00000004, 0x00000001, 0x00000001},
  {"MISC_CONF_STICKY"                     , 0x00000005, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_CL_STICKY_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"FILTER_STICKY_MASK"                   , 0x00000000, 0x00000001, 0x00000001},
  {"CLASS_STICKY_MASK"                    , 0x00000001, 0x00000001, 0x00000001},
  {"CAT_STICKY_MASK"                      , 0x00000002, 0x00000001, 0x00000001},
  {"IP_HDR_CHK_STICKY_MASK"               , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ANA_CL[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"INTEGRETY_IDENT"                      , 0x000008c0, 0x00000001, 0x00000001, regs_within_ANA_CL_INTEGRETY_IDENT},
  {"VQ_COUNTER"                           , 0x000008c1, 0x00000001, 0x00000001, regs_within_ANA_CL_VQ_COUNTER},
  {"PORT"                                 , 0x00000000, 0x00000023, 0x00000040, regs_within_ANA_CL_PORT},
  {"COMMON"                               , 0x000008c2, 0x00000001, 0x00000089, regs_within_ANA_CL_COMMON},
  {"STICKY"                               , 0x0000094b, 0x00000001, 0x00000006, regs_within_ANA_CL_STICKY},
  {"STICKY_MASK"                          , 0x00000951, 0x00000004, 0x00000004, regs_within_ANA_CL_STICKY_MASK},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_IS0_CONTROL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACL_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_UPDATE_CTRL"                      , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_MV_CFG"                           , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_STATUS"                           , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_STICKY"                           , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_ISID_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"ISID0"                                , 0x00000001, 0x00000001, 0x00000001},
  {"ISID1"                                , 0x00000002, 0x00000001, 0x00000001},
  {"ISID2"                                , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_DBL_VID_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"DBL_VID0"                             , 0x00000001, 0x00000001, 0x00000001},
  {"DBL_VID1"                             , 0x00000002, 0x00000001, 0x00000001},
  {"DBL_VID2"                             , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_MPLS_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"MPLS0"                                , 0x00000001, 0x00000001, 0x00000001},
  {"MPLS1"                                , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_MAC_ADDR_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"MAC_ADDR0"                            , 0x00000001, 0x00000001, 0x00000001},
  {"MAC_ADDR1"                            , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_ISID_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"ISID0"                                , 0x00000001, 0x00000001, 0x00000001},
  {"ISID1"                                , 0x00000002, 0x00000001, 0x00000001},
  {"ISID2"                                , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_DBL_VID_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"DBL_VID0"                             , 0x00000001, 0x00000001, 0x00000001},
  {"DBL_VID1"                             , 0x00000002, 0x00000001, 0x00000001},
  {"DBL_VID2"                             , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_MPLS_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"MPLS0"                                , 0x00000001, 0x00000001, 0x00000001},
  {"MPLS1"                                , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_MAC_ADDR_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"MAC_ADDR0"                            , 0x00000001, 0x00000001, 0x00000001},
  {"MAC_ADDR1"                            , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_BASETYPE_ACTION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"B"                                    , 0x00000001, 0x00000001, 0x00000001},
  {"C"                                    , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS0_TCAM_BIST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TCAM_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},






  {"TCAM_STAT"                            , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_VCAP_IS0[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"IS0_CONTROL"                          , 0x00000000, 0x00000001, 0x00000005, regs_within_VCAP_IS0_IS0_CONTROL},
  {"ISID_ENTRY"                           , 0x00000005, 0x00000001, 0x00000001, regs_within_VCAP_IS0_ISID_ENTRY},
  {"DBL_VID_ENTRY"                        , 0x00000009, 0x00000001, 0x00000001, regs_within_VCAP_IS0_DBL_VID_ENTRY},
  {"MPLS_ENTRY"                           , 0x0000000d, 0x00000001, 0x00000001, regs_within_VCAP_IS0_MPLS_ENTRY},
  {"MAC_ADDR_ENTRY"                       , 0x00000011, 0x00000001, 0x00000001, regs_within_VCAP_IS0_MAC_ADDR_ENTRY},
  {"ISID_MASK"                            , 0x00000015, 0x00000001, 0x00000001, regs_within_VCAP_IS0_ISID_MASK},
  {"DBL_VID_MASK"                         , 0x00000019, 0x00000001, 0x00000001, regs_within_VCAP_IS0_DBL_VID_MASK},
  {"MPLS_MASK"                            , 0x0000001d, 0x00000001, 0x00000001, regs_within_VCAP_IS0_MPLS_MASK},
  {"MAC_ADDR_MASK"                        , 0x00000021, 0x00000001, 0x00000001, regs_within_VCAP_IS0_MAC_ADDR_MASK},
  {"BASETYPE_ACTION"                      , 0x00000025, 0x00000001, 0x00000001, regs_within_VCAP_IS0_BASETYPE_ACTION},
  {"TCAM_BIST"                            , 0x00000029, 0x00000001, 0x00000004, regs_within_VCAP_IS0_TCAM_BIST},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_VCAP_IS1_IS1_CONTROL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACL_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_UPDATE_CTRL"                      , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_MV_CFG"                           , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_STATUS"                           , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_STICKY"                           , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS1_VLAN_PAG_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ENTRY"                                , 0x00000000, 0x00000001, 0x00000001},
  {"IF_GRP"                               , 0x00000001, 0x00000001, 0x00000001},
  {"VLAN"                                 , 0x00000002, 0x00000001, 0x00000001},
  {"FLAGS"                                , 0x00000003, 0x00000001, 0x00000001},
  {"L2_MAC_ADDR_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"L2_MAC_ADDR_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"L3_IP4_SIP"                           , 0x00000006, 0x00000001, 0x00000001},
  {"L3_MISC"                              , 0x00000007, 0x00000001, 0x00000001},
  {"L4_MISC"                              , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS1_QOS_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ENTRY"                                , 0x00000000, 0x00000001, 0x00000001},
  {"IF_GRP"                               , 0x00000001, 0x00000001, 0x00000001},
  {"VLAN"                                 , 0x00000002, 0x00000001, 0x00000001},
  {"FLAGS"                                , 0x00000003, 0x00000001, 0x00000001},
  {"L2_MAC_ADDR_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"L3_IP4_SIP"                           , 0x00000005, 0x00000001, 0x00000001},
  {"L3_MISC"                              , 0x00000006, 0x00000001, 0x00000001},
  {"L4_PORT"                              , 0x00000007, 0x00000001, 0x00000001},
  {"L4_MISC"                              , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS1_VLAN_PAG_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ENTRY"                                , 0x00000000, 0x00000001, 0x00000001},
  {"IF_GRP"                               , 0x00000001, 0x00000001, 0x00000001},
  {"VLAN"                                 , 0x00000002, 0x00000001, 0x00000001},
  {"FLAGS"                                , 0x00000003, 0x00000001, 0x00000001},
  {"L2_MAC_ADDR_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"L2_MAC_ADDR_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"L3_IP4_SIP"                           , 0x00000006, 0x00000001, 0x00000001},
  {"L3_MISC"                              , 0x00000007, 0x00000001, 0x00000001},
  {"L4_MISC"                              , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS1_QOS_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ENTRY"                                , 0x00000000, 0x00000001, 0x00000001},
  {"IF_GRP"                               , 0x00000001, 0x00000001, 0x00000001},
  {"VLAN"                                 , 0x00000002, 0x00000001, 0x00000001},
  {"FLAGS"                                , 0x00000003, 0x00000001, 0x00000001},
  {"L2_MAC_ADDR_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"L3_IP4_SIP"                           , 0x00000005, 0x00000001, 0x00000001},
  {"L3_MISC"                              , 0x00000006, 0x00000001, 0x00000001},
  {"L4_PORT"                              , 0x00000007, 0x00000001, 0x00000001},
  {"L4_MISC"                              , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS1_VLAN_PAG_ACTION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PAG"                                  , 0x00000000, 0x00000001, 0x00000001},
  {"MISC"                                 , 0x00000001, 0x00000001, 0x00000001},
  {"CUSTOM_POS"                           , 0x00000002, 0x00000001, 0x00000001},
  {"ISDX"                                 , 0x00000003, 0x00000001, 0x00000001},
  {"STICKY"                               , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS1_QOS_ACTION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DSCP"                                 , 0x00000000, 0x00000001, 0x00000001},
  {"QOS"                                  , 0x00000001, 0x00000001, 0x00000001},
  {"DP"                                   , 0x00000002, 0x00000001, 0x00000001},
  {"STICKY"                               , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS1_TCAM_BIST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TCAM_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},






  {"TCAM_STAT"                            , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_VCAP_IS1[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"IS1_CONTROL"                          , 0x00000000, 0x00000001, 0x00000005, regs_within_VCAP_IS1_IS1_CONTROL},
  {"VLAN_PAG_ENTRY"                       , 0x00000005, 0x00000001, 0x00000001, regs_within_VCAP_IS1_VLAN_PAG_ENTRY},
  {"QOS_ENTRY"                            , 0x00000015, 0x00000001, 0x00000001, regs_within_VCAP_IS1_QOS_ENTRY},
  {"VLAN_PAG_MASK"                        , 0x00000025, 0x00000001, 0x00000001, regs_within_VCAP_IS1_VLAN_PAG_MASK},
  {"QOS_MASK"                             , 0x00000035, 0x00000001, 0x00000001, regs_within_VCAP_IS1_QOS_MASK},
  {"VLAN_PAG_ACTION"                      , 0x00000045, 0x00000001, 0x00000001, regs_within_VCAP_IS1_VLAN_PAG_ACTION},
  {"QOS_ACTION"                           , 0x0000004d, 0x00000001, 0x00000001, regs_within_VCAP_IS1_QOS_ACTION},
  {"TCAM_BIST"                            , 0x00000055, 0x00000001, 0x00000004, regs_within_VCAP_IS1_TCAM_BIST},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_ANA_L3_BMSTP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"BMSTP_LRN_CFG"                        , 0x00000000, 0x00000001, 0x00000001},
  {"BMSTP_FWD_CFG"                        , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_INTEGRITY_IDENT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"INTEGRITY_IDENT"                      , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_L3_STICKY_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RLEG_STICKY_MASK"                     , 0x00000000, 0x00000001, 0x00000001},
  {"ROUT_STICKY_MASK"                     , 0x00000001, 0x00000001, 0x00000001},
  {"SECUR_STICKY_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"VLAN_MSTP_STICKY_MASK"                , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"BVLAN_FILTER_CTRL"                    , 0x00000000, 0x00000001, 0x00000001},



  {"VLAN_CTRL"                            , 0x00000002, 0x00000001, 0x00000001},
  {"L3_UC_ENA"                            , 0x00000003, 0x00000001, 0x00000001},
  {"TABLE_CTRL"                           , 0x00000004, 0x00000001, 0x00000001},
  {"L3_MC_ENA"                            , 0x00000005, 0x00000001, 0x00000001},
  {"PORT_FWD_CTRL"                        , 0x00000006, 0x00000001, 0x00000001},
  {"PORT_LRN_CTRL"                        , 0x00000007, 0x00000001, 0x00000001},
  {"VLAN_FILTER_CTRL"                     , 0x00000008, 0x00000001, 0x00000001},
  {"ROUTING_CFG"                          , 0x00000009, 0x00000001, 0x00000001},
  {"VLAN_ISOLATED_CFG"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"RLEG_CFG_0"                           , 0x0000000b, 0x00000001, 0x00000001},
  {"RLEG_CFG_1"                           , 0x0000000c, 0x00000001, 0x00000001},
  {"VLAN_COMMUNITY_CFG"                   , 0x0000000d, 0x00000001, 0x00000001},
  {"CPU_QU_CFG"                           , 0x0000000e, 0x00000001, 0x00000001},
  {"VRRP_CFG_0"                           , 0x0000000f, 0x00000001, 0x00000001},
  {"SIP_SECURE_ENA"                       , 0x00000010, 0x00000001, 0x00000001},
  {"DIP_SECURE_ENA"                       , 0x00000011, 0x00000001, 0x00000001},
  {"VRRP_CFG_1"                           , 0x00000012, 0x00000001, 0x00000001},
  {"VLAN_PORT_TYPE_MASK_CFG"              , 0x00000013, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_VLAN[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"BVLAN_CFG"                            , 0x00000000, 0x00000001, 0x00000001},
  {"VMID_CFG"                             , 0x00000001, 0x00000001, 0x00000001},
  {"VLAN_CFG"                             , 0x00000002, 0x00000001, 0x00000001},
  {"VLAN_MASK_CFG"                        , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_MSTP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MSTP_FWD_CFG"                         , 0x00000000, 0x00000001, 0x00000001},
  {"MSTP_LRN_CFG"                         , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_VMID[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RLEG_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},
  {"VRRP_CFG"                             , 0x00000001, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_LPM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACCESS_CTRL"                          , 0x00000000, 0x00000001, 0x00000001},
  {"ACCESS_MV_CFG"                        , 0x00000001, 0x00000001, 0x00000001},
  {"LPM_DATA_CFG"                         , 0x00000002, 0x00000004, 0x00000001},
  {"LPM_MASK_CFG"                         , 0x00000006, 0x00000004, 0x00000001},
  {"LPM_USAGE_CFG"                        , 0x0000000a, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_REMAP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"REMAP_CFG"                            , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_ARP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ARP_CFG_0"                            , 0x00000000, 0x00000001, 0x00000001},
  {"ARP_CFG_1"                            , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_L3MC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"EVMID_MASK_CFG"                       , 0x00000000, 0x00000004, 0x00000001},
  {"L3MC_CTRL"                            , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_LPM_REMAP_STICKY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"L3_LPM_REMAP_STICKY"                  , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_VLAN_ARP_L3MC_STICKY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VLAN_STICKY"                          , 0x00000000, 0x00000001, 0x00000001},
  {"L3_ARP_IPMC_STICKY"                   , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L3_TCAM_BIST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TCAM_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},






  {"TCAM_STAT"                            , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ANA_L3[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"BMSTP"                                , 0x00004e00, 0x00000042, 0x00000002, regs_within_ANA_L3_BMSTP},
  {"INTEGRITY_IDENT"                      , 0x00004e84, 0x00000001, 0x00000001, regs_within_ANA_L3_INTEGRITY_IDENT},
  {"L3_STICKY_MASK"                       , 0x00004e85, 0x00000004, 0x00000004, regs_within_ANA_L3_L3_STICKY_MASK},
  {"COMMON"                               , 0x00004e95, 0x00000001, 0x00000014, regs_within_ANA_L3_COMMON},
  {"VLAN"                                 , 0x00000000, 0x00001000, 0x00000004, regs_within_ANA_L3_VLAN},
  {"MSTP"                                 , 0x00004ea9, 0x00000042, 0x00000002, regs_within_ANA_L3_MSTP},
  {"VMID"                                 , 0x00004c00, 0x00000080, 0x00000004, regs_within_ANA_L3_VMID},
  {"LPM"                                  , 0x00004f2d, 0x00000001, 0x0000000e, regs_within_ANA_L3_LPM},
  {"REMAP"                                , 0x00004f3b, 0x00000001, 0x00000001, regs_within_ANA_L3_REMAP},
  {"ARP"                                  , 0x00004000, 0x00000400, 0x00000002, regs_within_ANA_L3_ARP},
  {"L3MC"                                 , 0x00004800, 0x00000080, 0x00000008, regs_within_ANA_L3_L3MC},
  {"LPM_REMAP_STICKY"                     , 0x00004f3c, 0x00000001, 0x00000001, regs_within_ANA_L3_LPM_REMAP_STICKY},
  {"VLAN_ARP_L3MC_STICKY"                 , 0x00004f3d, 0x00000001, 0x00000002, regs_within_ANA_L3_VLAN_ARP_L3MC_STICKY},
  {"TCAM_BIST"                            , 0x00004f3f, 0x00000001, 0x00000004, regs_within_ANA_L3_TCAM_BIST},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_IS2_CONTROL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACL_CFG"                              , 0x00000000, 0x00000001, 0x00000001},



  {"ACE_UPDATE_CTRL"                      , 0x00000024, 0x00000001, 0x00000001},
  {"ACE_MV_CFG"                           , 0x00000025, 0x00000001, 0x00000001},
  {"ACE_STATUS"                           , 0x00000026, 0x00000001, 0x00000001},
  {"ACE_STICKY"                           , 0x00000027, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_MAC_ETYPE_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_HIGH"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_LOW"                      , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L2_ETYPE"                         , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_MAC_LLC_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_HIGH"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_LOW"                      , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L2_LLC"                           , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_MAC_SNAP_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_HIGH"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_LOW"                      , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L2_SNAP_LOW"                      , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_L2_SNAP_HIGH"                     , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_ARP_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L3_MISC"                          , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L3_ARP"                           , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_DIP"                       , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_SIP"                       , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_IP_TCP_UDP_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_MISC"                          , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L3_MISC"                          , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_DIP"                       , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_SIP"                       , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L4_PORT"                          , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_L4_MISC"                          , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_IP_OTHER_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_MISC"                          , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L3_MISC"                          , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_DIP"                       , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_SIP"                       , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_IP4_OTHER_0"                      , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_IP4_OTHER_1"                      , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_IP6_STD_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L3_MISC"                          , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L3_IP6_SIP_0"                     , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L3_IP6_SIP_1"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L3_IP6_SIP_2"                     , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L3_IP6_SIP_3"                     , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_OAM_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_HIGH"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_LOW"                      , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_OAM_0"                            , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_OAM_1"                            , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_0_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_1_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_2_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_3_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_4_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_5_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_6_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_7_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_MAC_ETYPE_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_HIGH"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_LOW"                      , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L2_ETYPE"                         , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_MAC_LLC_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_HIGH"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_LOW"                      , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L2_LLC"                           , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_MAC_SNAP_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_HIGH"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_LOW"                      , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L2_SNAP_LOW"                      , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_L2_SNAP_HIGH"                     , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_ARP_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L3_MISC"                          , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L3_ARP"                           , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_DIP"                       , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_SIP"                       , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_IP_TCP_UDP_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_MISC"                          , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L3_MISC"                          , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_DIP"                       , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_SIP"                       , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L4_PORT"                          , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_L4_MISC"                          , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_IP_OTHER_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_MISC"                          , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L3_MISC"                          , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_DIP"                       , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L3_IP4_SIP"                       , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_IP4_OTHER_0"                      , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_IP4_OTHER_1"                      , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_IP6_STD_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L3_MISC"                          , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L3_IP6_SIP_0"                     , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L3_IP6_SIP_1"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L3_IP6_SIP_2"                     , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_L3_IP6_SIP_3"                     , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_OAM_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_HIGH"                     , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_L2_SMAC_LOW"                      , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_HIGH"                     , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_L2_DMAC_LOW"                      , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_OAM_0"                            , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_OAM_1"                            , 0x00000009, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_0_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_1_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_2_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_3_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_4_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_5_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_6_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_CUSTOM_7_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACE_VLD"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_TYPE"                             , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_IGR_PORT_MASK"                    , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_PAG"                              , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_0"                    , 0x00000004, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_1"                    , 0x00000005, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_2"                    , 0x00000006, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_3"                    , 0x00000007, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_4"                    , 0x00000008, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_5"                    , 0x00000009, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_6"                    , 0x0000000a, 0x00000001, 0x00000001},
  {"ACE_CUSTOM_DATA_7"                    , 0x0000000b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_BASETYPE_ACTION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"B"                                    , 0x00000001, 0x00000001, 0x00000001},
  {"C"                                    , 0x00000002, 0x00000001, 0x00000001},
  {"D"                                    , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_IS2_TCAM_BIST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TCAM_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},






  {"TCAM_STAT"                            , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_VCAP_IS2[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"IS2_CONTROL"                          , 0x00000000, 0x00000001, 0x00000028, regs_within_VCAP_IS2_IS2_CONTROL},
  {"MAC_ETYPE_ENTRY"                      , 0x00000028, 0x00000001, 0x00000001, regs_within_VCAP_IS2_MAC_ETYPE_ENTRY},
  {"MAC_LLC_ENTRY"                        , 0x00000038, 0x00000001, 0x00000001, regs_within_VCAP_IS2_MAC_LLC_ENTRY},
  {"MAC_SNAP_ENTRY"                       , 0x00000048, 0x00000001, 0x00000001, regs_within_VCAP_IS2_MAC_SNAP_ENTRY},
  {"ARP_ENTRY"                            , 0x00000058, 0x00000001, 0x00000001, regs_within_VCAP_IS2_ARP_ENTRY},
  {"IP_TCP_UDP_ENTRY"                     , 0x00000068, 0x00000001, 0x00000001, regs_within_VCAP_IS2_IP_TCP_UDP_ENTRY},
  {"IP_OTHER_ENTRY"                       , 0x00000078, 0x00000001, 0x00000001, regs_within_VCAP_IS2_IP_OTHER_ENTRY},
  {"IP6_STD_ENTRY"                        , 0x00000088, 0x00000001, 0x00000001, regs_within_VCAP_IS2_IP6_STD_ENTRY},
  {"OAM_ENTRY"                            , 0x00000098, 0x00000001, 0x00000001, regs_within_VCAP_IS2_OAM_ENTRY},
  {"CUSTOM_0_ENTRY"                       , 0x000000a8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_0_ENTRY},
  {"CUSTOM_1_ENTRY"                       , 0x000000b8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_1_ENTRY},
  {"CUSTOM_2_ENTRY"                       , 0x000000c8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_2_ENTRY},
  {"CUSTOM_3_ENTRY"                       , 0x000000d8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_3_ENTRY},
  {"CUSTOM_4_ENTRY"                       , 0x000000e8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_4_ENTRY},
  {"CUSTOM_5_ENTRY"                       , 0x000000f8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_5_ENTRY},
  {"CUSTOM_6_ENTRY"                       , 0x00000108, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_6_ENTRY},
  {"CUSTOM_7_ENTRY"                       , 0x00000118, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_7_ENTRY},
  {"MAC_ETYPE_MASK"                       , 0x00000128, 0x00000001, 0x00000001, regs_within_VCAP_IS2_MAC_ETYPE_MASK},
  {"MAC_LLC_MASK"                         , 0x00000138, 0x00000001, 0x00000001, regs_within_VCAP_IS2_MAC_LLC_MASK},
  {"MAC_SNAP_MASK"                        , 0x00000148, 0x00000001, 0x00000001, regs_within_VCAP_IS2_MAC_SNAP_MASK},
  {"ARP_MASK"                             , 0x00000158, 0x00000001, 0x00000001, regs_within_VCAP_IS2_ARP_MASK},
  {"IP_TCP_UDP_MASK"                      , 0x00000168, 0x00000001, 0x00000001, regs_within_VCAP_IS2_IP_TCP_UDP_MASK},
  {"IP_OTHER_MASK"                        , 0x00000178, 0x00000001, 0x00000001, regs_within_VCAP_IS2_IP_OTHER_MASK},
  {"IP6_STD_MASK"                         , 0x00000188, 0x00000001, 0x00000001, regs_within_VCAP_IS2_IP6_STD_MASK},
  {"OAM_MASK"                             , 0x00000198, 0x00000001, 0x00000001, regs_within_VCAP_IS2_OAM_MASK},
  {"CUSTOM_0_MASK"                        , 0x000001a8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_0_MASK},
  {"CUSTOM_1_MASK"                        , 0x000001b8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_1_MASK},
  {"CUSTOM_2_MASK"                        , 0x000001c8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_2_MASK},
  {"CUSTOM_3_MASK"                        , 0x000001d8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_3_MASK},
  {"CUSTOM_4_MASK"                        , 0x000001e8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_4_MASK},
  {"CUSTOM_5_MASK"                        , 0x000001f8, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_5_MASK},
  {"CUSTOM_6_MASK"                        , 0x00000208, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_6_MASK},
  {"CUSTOM_7_MASK"                        , 0x00000218, 0x00000001, 0x00000001, regs_within_VCAP_IS2_CUSTOM_7_MASK},
  {"BASETYPE_ACTION"                      , 0x00000228, 0x00000001, 0x00000001, regs_within_VCAP_IS2_BASETYPE_ACTION},
  {"TCAM_BIST"                            , 0x0000022c, 0x00000001, 0x00000004, regs_within_VCAP_IS2_TCAM_BIST},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_ANA_L2_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CCM_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {"HM_CFG"                               , 0x00000001, 0x00000001, 0x00000001},
  {"FWD_CFG"                              , 0x00000002, 0x00000001, 0x00000001},
  {"TABLE_CLR_CFG"                        , 0x00000003, 0x00000001, 0x00000001},
  {"LRN_CFG"                              , 0x00000004, 0x00000001, 0x00000001},
  {"FILTER_OTHER_CTRL"                    , 0x00000005, 0x00000001, 0x00000001},
  {"FILTER_LOCAL_CTRL"                    , 0x00000006, 0x00000001, 0x00000001},
  {"BLRN_CFG"                             , 0x00000007, 0x00000001, 0x00000001},
  {"AUTO_LRN_CFG"                         , 0x00000008, 0x00000001, 0x00000001},
  {"LRN_SECUR_CFG"                        , 0x00000009, 0x00000001, 0x00000001},
  {"LRN_SECUR_LOCKED_CFG"                 , 0x0000000a, 0x00000001, 0x00000001},
  {"LRN_COPY_CFG"                         , 0x0000000b, 0x00000001, 0x00000001},
  {"AUTO_BLRN_CFG"                        , 0x0000000c, 0x00000001, 0x00000001},
  {"BLRN_SECUR_CFG"                       , 0x0000000d, 0x00000001, 0x00000001},
  {"BLRN_COPY_CFG"                        , 0x0000000e, 0x00000001, 0x00000001},
  {"PORT_DLB_CFG"                         , 0x0000000f, 0x00000020, 0x00000001},
  {"PORT_CCM_CTRL"                        , 0x0000002f, 0x00000020, 0x00000001},
  {"MOVELOG_STICKY"                       , 0x0000004f, 0x00000001, 0x00000001},
  {"VSTAX_CTRL"                           , 0x00000050, 0x00000001, 0x00000001},
  {"LRN_CNT_CTRL"                         , 0x00000051, 0x00000020, 0x00000001},
  {"INTR"                                 , 0x00000071, 0x00000001, 0x00000001},
  {"INTR_ENA"                             , 0x00000072, 0x00000001, 0x00000001},
  {"INTR_IDENT"                           , 0x00000073, 0x00000001, 0x00000001},
  {"RAM_INTGR_ERR_IDENT"                  , 0x00000074, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L2_PATHGRP[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CONID_CFG"                            , 0x00000000, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L2_ISDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PORT_MASK_CFG"                        , 0x00000000, 0x00000001, 0x00000001},
  {"SERVICE_CTRL"                         , 0x00000001, 0x00000001, 0x00000001},
  {"CCM_CTRL"                             , 0x00000002, 0x00000001, 0x00000001},
  {"DLB_CFG"                              , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L2_STICKY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CCM_STICKY"                           , 0x00000000, 0x00000001, 0x00000001},
  {"STICKY"                               , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_L2_STICKY_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CCM_STICKY_MASK"                      , 0x00000000, 0x00000001, 0x00000001},
  {"STICKY_MASK"                          , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ANA_L2[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"COMMON"                               , 0x00004400, 0x00000001, 0x00000075, regs_within_ANA_L2_COMMON},
  {"PATHGRP"                              , 0x00004000, 0x00000200, 0x00000002, regs_within_ANA_L2_PATHGRP},
  {"ISDX"                                 , 0x00000000, 0x00001000, 0x00000004, regs_within_ANA_L2_ISDX},
  {"STICKY"                               , 0x00004475, 0x00000001, 0x00000002, regs_within_ANA_L2_STICKY},
  {"STICKY_MASK"                          , 0x00004477, 0x00000004, 0x00000002, regs_within_ANA_L2_STICKY_MASK},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_LRN_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"COMMON_ACCESS_CTRL"                   , 0x00000000, 0x00000001, 0x00000001},
  {"MAC_ACCESS_CFG_0"                     , 0x00000001, 0x00000001, 0x00000001},
  {"MAC_ACCESS_CFG_1"                     , 0x00000002, 0x00000001, 0x00000001},
  {"MAC_ACCESS_CFG_2"                     , 0x00000003, 0x00000001, 0x00000001},
  {"MAC_ACCESS_CFG_3"                     , 0x00000004, 0x00000001, 0x00000001},
  {"MAC_ACCESS_CFG_4"                     , 0x00000005, 0x00000001, 0x00000001},
  {"SCAN_NEXT_CFG"                        , 0x00000006, 0x00000001, 0x00000001},
  {"SCAN_NEXT_CFG_1"                      , 0x00000007, 0x00000001, 0x00000001},
  {"SCAN_LAST_ROW_CFG"                    , 0x00000008, 0x00000001, 0x00000001},
  {"SCAN_NEXT_CNT"                        , 0x00000009, 0x00000001, 0x00000001},
  {"AUTOAGE_CFG"                          , 0x0000000a, 0x00000001, 0x00000001},
  {"AUTOAGE_CFG_1"                        , 0x0000000b, 0x00000001, 0x00000001},
  {"AUTO_LRN_CFG"                         , 0x0000000c, 0x00000001, 0x00000001},
  {"CCM_CTRL"                             , 0x0000000d, 0x00000001, 0x00000001},
  {"CCM_TYPE_CFG"                         , 0x0000000e, 0x00000003, 0x00000001},
  {"CCM_STATUS"                           , 0x00000011, 0x00000001, 0x00000001},
  {"EVENT_STICKY"                         , 0x00000012, 0x00000001, 0x00000001},
  {"LATEST_POS_STATUS"                    , 0x00000013, 0x00000001, 0x00000001},
  {"RAM_INTGR_ERR_IDENT"                  , 0x00000014, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_LRN[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"COMMON"                               , 0x00000000, 0x00000001, 0x00000015, regs_within_LRN_COMMON},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_ANA_AC_PS_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MISC_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},
  {"HM_CFG"                               , 0x00000001, 0x00000004, 0x00000001},
  {"PS_COMMON_CFG"                        , 0x00000005, 0x00000001, 0x00000001},
  {"PS_TABLE_CLR_CFG"                     , 0x00000006, 0x00000001, 0x00000001},
  {"SFLOW_CFG"                            , 0x00000007, 0x00000001, 0x00000001},
  {"SFLOW_RESET_CTRL"                     , 0x00000008, 0x00000001, 0x00000001},
  {"SFLOW_CTRL"                           , 0x00000009, 0x00000020, 0x00000001},
  {"SFLOW_CNT"                            , 0x00000029, 0x00000020, 0x00000001},
  {"PHYS_SRC_AGGR_CFG"                    , 0x00000049, 0x00000001, 0x00000001},
  {"STACK_CFG"                            , 0x0000004a, 0x00000001, 0x00000001},
  {"STACK_A_CFG"                          , 0x0000004b, 0x00000001, 0x00000001},
  {"COMMON_VSTAX_CFG"                     , 0x0000004c, 0x00000001, 0x00000001},
  {"COMMON_EQUAL_STACK_LINK_TTL_CFG"      , 0x0000004d, 0x00000001, 0x00000001},
  {"VSTAX_CTRL"                           , 0x0000004e, 0x00000020, 0x00000001},
  {"VSTAX_GMIRROR_CFG"                    , 0x0000006e, 0x00000001, 0x00000001},



  {"CPU_CFG"                              , 0x00000077, 0x00000001, 0x00000001},









  {"RAM_INTGR_ERR_IDENT"                  , 0x0000007b, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_MIRROR_PROBE[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PROBE_CFG"                            , 0x00000000, 0x00000001, 0x00000001},
  {"PROBE_PORT_CFG"                       , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_AGGR[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"AGGR_CFG"                             , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_SRC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SRC_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_UPSID[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STACK_LINK_EQUAL_COST_CFG"            , 0x00000000, 0x00000001, 0x00000001},
  {"UPSID_CFG"                            , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_GLAG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"GLAG_CFG"                             , 0x00000000, 0x00000001, 0x00000001},
  {"MBR_CNT_CFG"                          , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_PGID[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PGID_CFG_0"                           , 0x00000000, 0x00000001, 0x00000001},
  {"PGID_CFG_1"                           , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_PS_STICKY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STICKY"                               , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_PS_STICKY_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STICKY_MASK"                          , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_POL_ALL_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"POL_ACL_RATE_CFG"                     , 0x00000000, 0x00000020, 0x00000001},
  {"POL_ACL_THRES_CFG"                    , 0x00000020, 0x00000020, 0x00000001},
  {"POL_ACL_CTRL"                         , 0x00000040, 0x00000020, 0x00000001},
  {"POL_PORT_FC_CFG"                      , 0x00000060, 0x00000021, 0x00000001},
  {"POL_ALL_CFG"                          , 0x00000081, 0x00000001, 0x00000001},
  {"POL_STICKY"                           , 0x00000082, 0x00000001, 0x00000001},
  {"POL_RAM_INTGR_ERR_IDENT"              , 0x00000083, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_POL_PORT_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"POL_PORT_THRES_CFG_0"                 , 0x00000000, 0x00000084, 0x00000001},
  {"POL_PORT_THRES_CFG_1"                 , 0x00000100, 0x00000084, 0x00000001},
  {"POL_PORT_RATE_CFG"                    , 0x00000200, 0x00000084, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_POL_PORT_CTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"POL_PORT_GAP"                         , 0x00000000, 0x00000001, 0x00000001},
  {"POL_PORT_CFG"                         , 0x00000001, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_POL_PRIO_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"POL_PRIO_RATE_CFG"                    , 0x00000000, 0x00000108, 0x00000001},
  {"POL_PRIO_THRES_CFG"                   , 0x00000200, 0x00000108, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_POL_PRIO_CTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"POL_PRIO_GAP"                         , 0x00000000, 0x00000001, 0x00000001},
  {"POL_PRIO_CFG"                         , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_GLOBAL_CFG_PORT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000000, 0x00000004, 0x00000001},
  {"STAT_RESET"                           , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_CNT_CFG_PORT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_EVENTS_STICKY"                   , 0x00000000, 0x00000001, 0x00000001},
  {"STAT_CFG"                             , 0x00000001, 0x00000004, 0x00000001},
  {"STAT_LSB_CNT"                         , 0x00000005, 0x00000004, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000009, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_GLOBAL_CFG_ISDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_CFG"                      , 0x00000000, 0x00000006, 0x00000001},
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000006, 0x00000006, 0x00000001},
  {"STAT_RESET"                           , 0x0000000c, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_CNT_CFG_ISDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_LSB_CNT"                         , 0x00000000, 0x00000006, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000006, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_GLOBAL_CFG_TUNNEL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_CFG"                      , 0x00000000, 0x00000006, 0x00000001},
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000006, 0x00000006, 0x00000001},
  {"STAT_RESET"                           , 0x0000000c, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_CNT_CFG_TUNNEL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_LSB_CNT"                         , 0x00000000, 0x00000006, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000006, 0x00000003, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_GLOBAL_CFG_IRLEG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_CFG"                      , 0x00000000, 0x00000006, 0x00000001},
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000006, 0x00000006, 0x00000001},
  {"STAT_RESET"                           , 0x0000000c, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_CNT_CFG_IRLEG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_LSB_CNT"                         , 0x00000000, 0x00000006, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000006, 0x00000006, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_GLOBAL_CFG_ERLEG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_CFG"                      , 0x00000000, 0x00000006, 0x00000001},
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000006, 0x00000006, 0x00000001},
  {"STAT_RESET"                           , 0x0000000c, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_STAT_CNT_CFG_ERLEG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_LSB_CNT"                         , 0x00000000, 0x00000006, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000006, 0x00000006, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_COMMON_SDLB[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DLB_CTRL"                             , 0x00000000, 0x00000001, 0x00000001},



  {"DLB_STICKY"                           , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_SDLB[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DLB_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {"LB_CFG"                               , 0x00000001, 0x00000002, 0x00000001},






  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_COMMON_TDLB[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DLB_CTRL"                             , 0x00000000, 0x00000001, 0x00000001},



  {"DLB_STICKY"                           , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ANA_AC_TDLB[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DLB_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {"LB_CFG"                               , 0x00000001, 0x00000002, 0x00000001},






  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ANA_AC[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"PS_COMMON"                            , 0x0001ba63, 0x00000001, 0x0000007c, regs_within_ANA_AC_PS_COMMON},
  {"MIRROR_PROBE"                         , 0x0001badf, 0x00000003, 0x00000008, regs_within_ANA_AC_MIRROR_PROBE},
  {"AGGR"                                 , 0x0001baf7, 0x00000010, 0x00000001, regs_within_ANA_AC_AGGR},
  {"SRC"                                  , 0x0001ba40, 0x00000023, 0x00000001, regs_within_ANA_AC_SRC},
  {"UPSID"                                , 0x0001bb07, 0x00000020, 0x00000002, regs_within_ANA_AC_UPSID},
  {"GLAG"                                 , 0x0001bb47, 0x00000020, 0x00000002, regs_within_ANA_AC_GLAG},
  {"PGID"                                 , 0x0001b000, 0x00000520, 0x00000002, regs_within_ANA_AC_PGID},
  {"PS_STICKY"                            , 0x0001bb87, 0x00000001, 0x00000001, regs_within_ANA_AC_PS_STICKY},
  {"PS_STICKY_MASK"                       , 0x0001bb88, 0x00000004, 0x00000010, regs_within_ANA_AC_PS_STICKY_MASK},
  {"POL_ALL_CFG"                          , 0x0001d630, 0x00000001, 0x00000084, regs_within_ANA_AC_POL_ALL_CFG},
  {"POL_PORT_CFG"                         , 0x0001bc00, 0x00000001, 0x00000400, regs_within_ANA_AC_POL_PORT_CFG},
  {"POL_PORT_CTRL"                        , 0x0001dc00, 0x00000021, 0x00000008, regs_within_ANA_AC_POL_PORT_CTRL},
  {"POL_PRIO_CFG"                         , 0x0001d000, 0x00000001, 0x00000400, regs_within_ANA_AC_POL_PRIO_CFG},
  {"POL_PRIO_CTRL"                        , 0x0001d800, 0x00000108, 0x00000002, regs_within_ANA_AC_POL_PRIO_CTRL},
  {"STAT_GLOBAL_CFG_PORT"                 , 0x0001bbc8, 0x00000001, 0x00000005, regs_within_ANA_AC_STAT_GLOBAL_CFG_PORT},
  {"STAT_CNT_CFG_PORT"                    , 0x0001d400, 0x00000023, 0x00000010, regs_within_ANA_AC_STAT_CNT_CFG_PORT},
  {"STAT_GLOBAL_CFG_ISDX"                 , 0x0001bbcd, 0x00000001, 0x0000000d, regs_within_ANA_AC_STAT_GLOBAL_CFG_ISDX},
  {"STAT_CNT_CFG_ISDX"                    , 0x00000000, 0x00001000, 0x00000010, regs_within_ANA_AC_STAT_CNT_CFG_ISDX},
  {"STAT_GLOBAL_CFG_TUNNEL"               , 0x0001bbda, 0x00000001, 0x0000000d, regs_within_ANA_AC_STAT_GLOBAL_CFG_TUNNEL},
  {"STAT_CNT_CFG_TUNNEL"                  , 0x00018000, 0x00000200, 0x00000010, regs_within_ANA_AC_STAT_CNT_CFG_TUNNEL},
  {"STAT_GLOBAL_CFG_IRLEG"                , 0x0001bbe7, 0x00000001, 0x0000000d, regs_within_ANA_AC_STAT_GLOBAL_CFG_IRLEG},
  {"STAT_CNT_CFG_IRLEG"                   , 0x0001c000, 0x00000080, 0x00000010, regs_within_ANA_AC_STAT_CNT_CFG_IRLEG},
  {"STAT_GLOBAL_CFG_ERLEG"                , 0x0001d6b4, 0x00000001, 0x0000000d, regs_within_ANA_AC_STAT_GLOBAL_CFG_ERLEG},
  {"STAT_CNT_CFG_ERLEG"                   , 0x0001c800, 0x00000080, 0x00000010, regs_within_ANA_AC_STAT_CNT_CFG_ERLEG},
  {"COMMON_SDLB"                          , 0x0001bbf4, 0x00000001, 0x00000003, regs_within_ANA_AC_COMMON_SDLB},
  {"SDLB"                                 , 0x00010000, 0x00001000, 0x00000008, regs_within_ANA_AC_SDLB},
  {"COMMON_TDLB"                          , 0x0001bbf7, 0x00000001, 0x00000003, regs_within_ANA_AC_COMMON_TDLB},
  {"TDLB"                                 , 0x0001a000, 0x00000200, 0x00000008, regs_within_ANA_AC_TDLB},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_IQS_CONG_CTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CH_CFG"                               , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_MTU[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MTU_FRM_SIZE_CFG"                     , 0x00000000, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_QU_RAM_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"QU_RC_CFG"                            , 0x00000000, 0x00000118, 0x00000001},
  {"QU_RC_HLWM_CFG"                       , 0x00000200, 0x00000118, 0x00000001},
  {"QU_RC_ATOP_SWM_CFG"                   , 0x00000400, 0x00000118, 0x00000001},
  {"MTU_QU_MAP_CFG"                       , 0x00000600, 0x00000118, 0x00000001},
  {"RED_ECN_MINTH_MAXTH_CFG"              , 0x00000800, 0x00000118, 0x00000001},
  {"RED_ECN_MISC_CFG"                     , 0x00000a00, 0x00000118, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_PORT_RAM_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PORT_RC_CFG"                          , 0x00000000, 0x00000023, 0x00000001},
  {"PORT_RC_HLWM_CFG"                     , 0x00000040, 0x00000023, 0x00000001},
  {"PORT_RC_ATOP_SWM_CFG"                 , 0x00000080, 0x00000023, 0x00000001},
  {"PORT_RC_GWM_CFG"                      , 0x000000c0, 0x00000023, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_RESOURCE_CTRL_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"BUF_RC_CFG"                           , 0x00000000, 0x00000001, 0x00000001},
  {"BUF_PORT_RC_HLWM_CFG"                 , 0x00000001, 0x00000001, 0x00000001},
  {"BUF_PRIO_RC_CFG"                      , 0x00000002, 0x00000001, 0x00000001},
  {"BUF_PRIO_RC_HLWM_CFG"                 , 0x00000003, 0x00000001, 0x00000001},
  {"PRIO_RC_CFG"                          , 0x00000004, 0x00000008, 0x00000001},
  {"PRIO_RC_HLWM_CFG"                     , 0x0000000c, 0x00000008, 0x00000001},
  {"PRIO_RC_ATOP_SWM_CFG"                 , 0x00000014, 0x00000008, 0x00000001},
  {NULL, 0, 0, 0}
};












static const SYMREG_reg_t regs_within_IQS_RED_RAM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"QU_MEM_AVG_ALLOC_FRAC_STATUS"         , 0x00000000, 0x00000118, 0x00000001},
  {"QU_MEM_AVG_ALLOC_STATUS"              , 0x00000200, 0x00000118, 0x00000001},
  {"RED_ECN_WQ_CFG"                       , 0x00000400, 0x00000118, 0x00000001},
  {NULL, 0, 0, 0}
};



























static const SYMREG_reg_t regs_within_IQS_MAIN[] = {
  //reg name                              , addr      , repl_cnt  , repl_width






  {"MAIN_REINIT_1SHOT"                    , 0x00000002, 0x00000001, 0x00000001},



  {"RAM_INTEGRITY_ERR_STICKY"             , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};






























static const SYMREG_reg_t regs_within_IQS_ABM_DBG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width









  {"ABM_EMPTY_STICKY"                     , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_RESERVED[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RESERVED"                             , 0x00000000, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};




































static const SYMREG_reg_t regs_within_IQS_STAT_GLOBAL_CFG_RX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000000, 0x00000004, 0x00000001},
  {"STAT_RESET"                           , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_STAT_CNT_CFG_RX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_EVENTS_STICKY"                   , 0x00000000, 0x00000001, 0x00000001},
  {"STAT_CFG"                             , 0x00000001, 0x00000004, 0x00000001},
  {"STAT_LSB_CNT"                         , 0x00000005, 0x00000004, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000009, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_STAT_GLOBAL_CFG_TX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000000, 0x00000002, 0x00000001},
  {"STAT_RESET"                           , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_STAT_CNT_CFG_TX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_EVENTS_STICKY"                   , 0x00000000, 0x00000001, 0x00000001},
  {"STAT_CFG"                             , 0x00000001, 0x00000002, 0x00000001},
  {"STAT_LSB_CNT"                         , 0x00000003, 0x00000002, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000005, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_STAT_GLOBAL_CFG_ISDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_CFG"                      , 0x00000000, 0x00000002, 0x00000001},
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000002, 0x00000002, 0x00000001},
  {"STAT_RESET"                           , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_IQS_STAT_CNT_CFG_ISDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_LSB_CNT"                         , 0x00000000, 0x00000002, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_IQS[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CONG_CTRL"                            , 0x00005300, 0x00000001, 0x00000001, regs_within_IQS_CONG_CTRL},
  {"MTU"                                  , 0x00005301, 0x00000001, 0x00000004, regs_within_IQS_MTU},
  {"QU_RAM_CFG"                           , 0x00006000, 0x00000001, 0x00001000, regs_within_IQS_QU_RAM_CFG},
  {"PORT_RAM_CFG"                         , 0x00005200, 0x00000001, 0x00000100, regs_within_IQS_PORT_RAM_CFG},
  {"RESOURCE_CTRL_CFG"                    , 0x00005305, 0x00000001, 0x0000001c, regs_within_IQS_RESOURCE_CTRL_CFG},



  {"RED_RAM"                              , 0x00005800, 0x00000001, 0x00000800, regs_within_IQS_RED_RAM},






  {"MAIN"                                 , 0x0000532b, 0x00000001, 0x00000005, regs_within_IQS_MAIN},



  {"ABM_DBG"                              , 0x00005338, 0x00000001, 0x00000004, regs_within_IQS_ABM_DBG},
  {"RESERVED"                             , 0x0000533c, 0x00000001, 0x00000002, regs_within_IQS_RESERVED},









  {"STAT_GLOBAL_CFG_RX"                   , 0x00005354, 0x00000001, 0x00000005, regs_within_IQS_STAT_GLOBAL_CFG_RX},
  {"STAT_CNT_CFG_RX"                      , 0x00004000, 0x00000118, 0x00000010, regs_within_IQS_STAT_CNT_CFG_RX},
  {"STAT_GLOBAL_CFG_TX"                   , 0x00005359, 0x00000001, 0x00000003, regs_within_IQS_STAT_GLOBAL_CFG_TX},
  {"STAT_CNT_CFG_TX"                      , 0x00007000, 0x00000118, 0x00000008, regs_within_IQS_STAT_CNT_CFG_TX},
  {"STAT_GLOBAL_CFG_ISDX"                 , 0x0000535c, 0x00000001, 0x00000005, regs_within_IQS_STAT_GLOBAL_CFG_ISDX},
  {"STAT_CNT_CFG_ISDX"                    , 0x00000000, 0x00001000, 0x00000004, regs_within_IQS_STAT_CNT_CFG_ISDX},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_ARB_CFG_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PORT_ARB_CFG"                         , 0x00000000, 0x00000001, 0x00000001},
  {"INGR_SHAPER_CFG"                      , 0x00000001, 0x00000023, 0x00000001},
  {"PRIO_ARB_CFG"                         , 0x00000024, 0x00000008, 0x00000001},
  {"PRIO_COST_CFG"                        , 0x0000002c, 0x00000006, 0x00000001},
  {"DWRR_CFG"                             , 0x00000032, 0x00000001, 0x00000001},
  {"CM_CFG_VEC0"                          , 0x00000033, 0x00000008, 0x00000001},
  {"CM_CFG_VEC1"                          , 0x0000003b, 0x00000008, 0x00000001},
  {"CM_ACCESS"                            , 0x00000043, 0x00000001, 0x00000001},
  {"CM_DATA"                              , 0x00000044, 0x00000001, 0x00000001},
  {"CM_IGNORE_UPDATE"                     , 0x00000045, 0x00000001, 0x00000001},
  {"STACK_CFG"                            , 0x00000046, 0x00000001, 0x00000001},
  {"GCPU_PRIO_CFG"                        , 0x00000047, 0x00000008, 0x00000001},
  {"MIRROR_CFG"                           , 0x0000004f, 0x00000003, 0x00000001},
  {"HM_CFG"                               , 0x00000052, 0x00000001, 0x00000001},



  {"CH_CFG"                               , 0x00000054, 0x00000001, 0x00000001},
  {"FLUSH_CTRL"                           , 0x00000055, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ARB_DROP_MODE_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"OUTB_ETH_DROP_MODE_CFG"               , 0x00000000, 0x00000008, 0x00000001},
  {"OUTB_CPU_DROP_MODE_CFG"               , 0x00000008, 0x00000008, 0x00000001},
  {"OUTB_VD_DROP_MODE_CFG"                , 0x00000010, 0x00000008, 0x00000001},
  {"MC_DROP_MODE_CFG"                     , 0x00000018, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ARB_AGING_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ERA_CTRL"                             , 0x00000000, 0x00000001, 0x00000001},
  {"ERA_PRESCALER"                        , 0x00000001, 0x00000001, 0x00000001},
  {"PORT_AGING_ENA_VEC0"                  , 0x00000002, 0x00000001, 0x00000001},
  {"PORT_AGING_ENA_VEC1"                  , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ARB_STICKY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width












  {"OQS_ETH_QU_FC_STATUS"                 , 0x00000012, 0x00000008, 0x00000001},
  {"OQS_VD_QU_FC_STATUS"                  , 0x0000001a, 0x00000008, 0x00000001},
  {"OQS_CPU_QU_FC_STATUS"                 , 0x00000022, 0x00000008, 0x00000001},
  {"OQS_ETH_PORT_FC_STATUS"               , 0x0000002a, 0x00000001, 0x00000001},
  {"OQS_VD_PORT_FC_STATUS"                , 0x0000002b, 0x00000001, 0x00000001},
  {"OQS_CPU_PORT_FC_STATUS"               , 0x0000002c, 0x00000001, 0x00000001},
  {"OQS_PRIO_FC_STATUS"                   , 0x0000002d, 0x00000001, 0x00000001},
  {"OQS_BUF_FC_STATUS"                    , 0x0000002e, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ARB[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CFG_STATUS"                           , 0x00000000, 0x00000001, 0x00000057, regs_within_ARB_CFG_STATUS},
  {"DROP_MODE_CFG"                        , 0x00000057, 0x00000001, 0x00000019, regs_within_ARB_DROP_MODE_CFG},
  {"AGING_CFG"                            , 0x00000070, 0x00000001, 0x00000004, regs_within_ARB_AGING_CFG},
  {"STICKY"                               , 0x00000074, 0x00000001, 0x0000002f, regs_within_ARB_STICKY},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_OQS_CONG_CTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CH_CFG"                               , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_MTU[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"MTU_FRM_SIZE_CFG"                     , 0x00000000, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_QU_RAM_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"QU_RC_CFG"                            , 0x00000000, 0x000001d0, 0x00000001},
  {"QU_RC_HLWM_CFG"                       , 0x00000200, 0x000001d0, 0x00000001},
  {"QU_RC_ATOP_SWM_CFG"                   , 0x00000400, 0x000001d0, 0x00000001},
  {"MTU_QU_MAP_CFG"                       , 0x00000600, 0x000001d0, 0x00000001},



  {"RED_ECN_MINTH_MAXTH_CFG"              , 0x00000a00, 0x000001d0, 0x00000001},
  {"RED_ECN_MISC_CFG"                     , 0x00000c00, 0x000001d0, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_PORT_RAM_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PORT_RC_CFG"                          , 0x00000000, 0x00000059, 0x00000001},
  {"PORT_RC_HLWM_CFG"                     , 0x00000080, 0x00000059, 0x00000001},
  {"PORT_RC_ATOP_SWM_CFG"                 , 0x00000100, 0x00000059, 0x00000001},
  {"CM_PORT_WM"                           , 0x00000180, 0x00000059, 0x00000001},
  {"PORT_RC_GWM_CFG"                      , 0x00000200, 0x00000059, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_RESOURCE_CTRL_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"BUF_RC_CFG"                           , 0x00000000, 0x00000001, 0x00000001},
  {"BUF_PORT_RC_HLWM_CFG"                 , 0x00000001, 0x00000001, 0x00000001},
  {"BUF_PRIO_RC_CFG"                      , 0x00000002, 0x00000001, 0x00000001},
  {"BUF_PRIO_RC_HLWM_CFG"                 , 0x00000003, 0x00000001, 0x00000001},
  {"PRIO_RC_CFG"                          , 0x00000004, 0x00000008, 0x00000001},
  {"PRIO_RC_HLWM_CFG"                     , 0x0000000c, 0x00000008, 0x00000001},
  {"PRIO_RC_ATOP_SWM_CFG"                 , 0x00000014, 0x00000008, 0x00000001},
  {NULL, 0, 0, 0}
};












static const SYMREG_reg_t regs_within_OQS_RED_RAM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"QU_MEM_AVG_ALLOC_FRAC_STATUS"         , 0x00000000, 0x000001d0, 0x00000001},
  {"QU_MEM_AVG_ALLOC_STATUS"              , 0x00000200, 0x000001d0, 0x00000001},
  {"RED_ECN_WQ_CFG"                       , 0x00000400, 0x000001d0, 0x00000001},
  {NULL, 0, 0, 0}
};



























static const SYMREG_reg_t regs_within_OQS_MAIN[] = {
  //reg name                              , addr      , repl_cnt  , repl_width






  {"MAIN_REINIT_1SHOT"                    , 0x00000002, 0x00000001, 0x00000001},



  {"IFH_BIP8_ERR_STICKY"                  , 0x00000004, 0x00000008, 0x00000001},
  {"RAM_INTEGRITY_ERR_STICKY"             , 0x0000000c, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};






























static const SYMREG_reg_t regs_within_OQS_ABM_DBG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width









  {"ABM_EMPTY_STICKY"                     , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_RESERVED[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RESERVED"                             , 0x00000000, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};



























static const SYMREG_reg_t regs_within_OQS_CE_HOST_MODE_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"HOST_MODE_CM_CFG"                     , 0x00000000, 0x00000001, 0x00000001},
  {"HOST_PORT_RESRC_CTL_CFG"              , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_CM_BUF_WM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CM_BUF_WM"                            , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_CPU_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"COPIES_TO_CPU_PORTS_CFG"              , 0x00000000, 0x00000001, 0x00000001},
  {"OQS_PRUNE_MODE_CFG"                   , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_OQS_STAT_GLOBAL_CFG_RX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000000, 0x00000004, 0x00000001},
  {"STAT_RESET"                           , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_STAT_CNT_CFG_RX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_EVENTS_STICKY"                   , 0x00000000, 0x00000001, 0x00000001},
  {"STAT_CFG"                             , 0x00000001, 0x00000004, 0x00000001},
  {"STAT_LSB_CNT"                         , 0x00000005, 0x00000004, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000009, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_STAT_GLOBAL_CFG_TX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000000, 0x00000002, 0x00000001},
  {"STAT_RESET"                           , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_STAT_CNT_CFG_TX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_EVENTS_STICKY"                   , 0x00000000, 0x00000001, 0x00000001},
  {"STAT_CFG"                             , 0x00000001, 0x00000002, 0x00000001},
  {"STAT_LSB_CNT"                         , 0x00000003, 0x00000002, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000005, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_STAT_GLOBAL_CFG_ISDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_CFG"                      , 0x00000000, 0x00000002, 0x00000001},
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000002, 0x00000002, 0x00000001},
  {"STAT_RESET"                           , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_OQS_STAT_CNT_CFG_ISDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_LSB_CNT"                         , 0x00000000, 0x00000002, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_OQS[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CONG_CTRL"                            , 0x00005e00, 0x00000001, 0x00000001, regs_within_OQS_CONG_CTRL},
  {"MTU"                                  , 0x00005e01, 0x00000001, 0x00000004, regs_within_OQS_MTU},
  {"QU_RAM_CFG"                           , 0x00006000, 0x00000001, 0x00001000, regs_within_OQS_QU_RAM_CFG},
  {"PORT_RAM_CFG"                         , 0x00008800, 0x00000001, 0x00000400, regs_within_OQS_PORT_RAM_CFG},
  {"RESOURCE_CTRL_CFG"                    , 0x00005e05, 0x00000001, 0x0000001c, regs_within_OQS_RESOURCE_CTRL_CFG},



  {"RED_RAM"                              , 0x00008000, 0x00000001, 0x00000800, regs_within_OQS_RED_RAM},






  {"MAIN"                                 , 0x00005e2b, 0x00000001, 0x0000000d, regs_within_OQS_MAIN},



  {"ABM_DBG"                              , 0x00005e40, 0x00000001, 0x00000004, regs_within_OQS_ABM_DBG},
  {"RESERVED"                             , 0x00005e44, 0x00000001, 0x00000002, regs_within_OQS_RESERVED},






  {"CE_HOST_MODE_CFG"                     , 0x00005e67, 0x00000001, 0x00000002, regs_within_OQS_CE_HOST_MODE_CFG},
  {"CM_BUF_WM"                            , 0x00005e69, 0x00000001, 0x00000001, regs_within_OQS_CM_BUF_WM},
  {"CPU_CFG"                              , 0x00005e6a, 0x00000001, 0x00000002, regs_within_OQS_CPU_CFG},



  {"STAT_GLOBAL_CFG_RX"                   , 0x00005e6d, 0x00000001, 0x00000005, regs_within_OQS_STAT_GLOBAL_CFG_RX},
  {"STAT_CNT_CFG_RX"                      , 0x00004000, 0x000001d0, 0x00000010, regs_within_OQS_STAT_CNT_CFG_RX},
  {"STAT_GLOBAL_CFG_TX"                   , 0x00005e72, 0x00000001, 0x00000003, regs_within_OQS_STAT_GLOBAL_CFG_TX},
  {"STAT_CNT_CFG_TX"                      , 0x00007000, 0x000001d0, 0x00000008, regs_within_OQS_STAT_CNT_CFG_TX},
  {"STAT_GLOBAL_CFG_ISDX"                 , 0x00005e75, 0x00000001, 0x00000005, regs_within_OQS_STAT_GLOBAL_CFG_ISDX},
  {"STAT_CNT_CFG_ISDX"                    , 0x00000000, 0x00001000, 0x00000004, regs_within_OQS_STAT_CNT_CFG_ISDX},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_SCH_QSIF[] = {
  //reg name                              , addr      , repl_cnt  , repl_width




















































































  {"QSIF_FLUSH_CTRL"                      , 0x0000001c, 0x00000001, 0x00000001},
  {"QSIF_CTRL"                            , 0x0000001d, 0x00000001, 0x00000001},
  {"QSIF_HM_CTRL"                         , 0x0000001e, 0x00000001, 0x00000001},
  {"QSIF_HMDA_QU_FC_ENA"                  , 0x0000001f, 0x00000006, 0x00000001},
  {"QSIF_HMDB_QU_FC_ENA"                  , 0x00000025, 0x00000006, 0x00000001},
  {"QSIF_HMDA_VP_FC_ENA"                  , 0x0000002b, 0x00000002, 0x00000001},
  {"QSIF_HMDB_VP_FC_ENA"                  , 0x0000002d, 0x00000002, 0x00000001},












  {"QSIF_ETH_PORT_CTRL"                   , 0x00000053, 0x00000020, 0x00000001},
  {"QSIF_VD_PORT_CTRL"                    , 0x00000073, 0x00000001, 0x00000001},
  {"QSIF_HM_PORT_CTRL"                    , 0x00000074, 0x000000c0, 0x00000001},
  {"QSIF_CPU_PORT_CTRL"                   , 0x00000134, 0x00000008, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SCH_HM[] = {
  //reg name                              , addr      , repl_cnt  , repl_width















  {"HM_DWRR_COST"                         , 0x0000000f, 0x000000c0, 0x00000001},
  {"HM_DWRR_AND_LB"                       , 0x000000cf, 0x00000030, 0x00000001},
  {"HM_LB_CFG"                            , 0x000000ff, 0x00000090, 0x00000001},
  {"HM_SCH_CTRL"                          , 0x0000018f, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SCH_HM_LB[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"HM_LB_THRES"                          , 0x00000000, 0x00000090, 0x00000001},
  {"HM_LB_RATE"                           , 0x00000100, 0x00000090, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SCH_ETH[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ETH_CTRL"                             , 0x00000000, 0x00000001, 0x00000001},



























  {"ETH_PORT_PAUSE_FRM_MODE"              , 0x0000000a, 0x00000021, 0x00000001},
  {"ETH_PORT_PAUSE_FRM_PRIO_MASK"         , 0x0000002b, 0x00000001, 0x00000001},
  {"ETH_LB_DWRR_FRM_ADJ"                  , 0x0000002c, 0x00000001, 0x00000001},
  {"ETH_LB_DWRR_CFG"                      , 0x0000002d, 0x00000021, 0x00000001},
  {"ETH_DWRR_CFG"                         , 0x0000004e, 0x00000021, 0x00000001},



  {"ETH_SHAPING_CTRL"                     , 0x00000070, 0x00000021, 0x00000001},
  {"ETH_LB_CTRL"                          , 0x00000091, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SCH_ETH_LB[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ETH_LB_THRES"                         , 0x00000000, 0x00000129, 0x00000001},
  {"ETH_LB_RATE"                          , 0x00000200, 0x00000129, 0x00000001},
  {NULL, 0, 0, 0}
};





















static const SYMREG_reg_t regs_within_SCH_CBC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width



  {"CBC_LEN_CFG"                          , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SCH_CBC_DEV_ID_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CBC_DEV_ID"                           , 0x00000000, 0x00000400, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_SCH_MISC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ERA_CTRL"                             , 0x00000000, 0x00000001, 0x00000001},
  {"ERA_PRESCALER"                        , 0x00000001, 0x00000001, 0x00000001},
  {"RAM_INTEGRITY_ERR_STICKY"             , 0x00000002, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_SCH[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"QSIF"                                 , 0x00000a00, 0x00000001, 0x0000013c, regs_within_SCH_QSIF},
  {"HM"                                   , 0x00000b3c, 0x00000001, 0x00000190, regs_within_SCH_HM},
  {"HM_LB"                                , 0x00000800, 0x00000001, 0x00000200, regs_within_SCH_HM_LB},
  {"ETH"                                  , 0x00000ccc, 0x00000001, 0x00000092, regs_within_SCH_ETH},
  {"ETH_LB"                               , 0x00000000, 0x00000001, 0x00000400, regs_within_SCH_ETH_LB},



  {"CBC"                                  , 0x00000d63, 0x00000001, 0x00000002, regs_within_SCH_CBC},
  {"CBC_DEV_ID_CFG"                       , 0x00000400, 0x00000001, 0x00000400, regs_within_SCH_CBC_DEV_ID_CFG},
  {"MISC"                                 , 0x00000d65, 0x00000001, 0x00000003, regs_within_SCH_MISC},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_REW_COMMON[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"COMMON_CTRL"                          , 0x00000000, 0x00000001, 0x00000001},
  {"SERVICE_CTRL"                         , 0x00000001, 0x00000001, 0x00000001},
  {"MIRROR_PROBE_CFG"                     , 0x00000002, 0x00000003, 0x00000001},
  {"DSCP_REMAP"                           , 0x00000005, 0x00000040, 0x00000001},
  {"RLEG_CFG_0"                           , 0x00000045, 0x00000001, 0x00000001},
  {"RLEG_CFG_1"                           , 0x00000046, 0x00000001, 0x00000001},
  {"ITAG_SMAC_HI"                         , 0x00000047, 0x00000020, 0x00000001},
  {"ITAG_SMAC_LO"                         , 0x00000067, 0x00000020, 0x00000001},
  {"CNT_CTRL"                             , 0x00000087, 0x00000001, 0x00000001},
  {"STICKY_EVENT_COUNT"                   , 0x00000088, 0x00000001, 0x00000001},
  {"STICKY_EVENT_CNT_MASK_CFG"            , 0x00000089, 0x00000001, 0x00000001},
  {"STICKY_EVENT"                         , 0x0000008a, 0x00000001, 0x00000001},
  {"RAM_INTEGRITY_ERR_STICKY"             , 0x0000008b, 0x00000001, 0x00000001},



  {"VSTAX_GCPU_CFG"                       , 0x0000008d, 0x00000001, 0x00000001},
  {"VSTAX_PORT_GRP_CFG"                   , 0x0000008e, 0x00000002, 0x00000001},
  {"HM_CTRL"                              , 0x00000090, 0x00000001, 0x00000001},
  {"HIH_CTRL"                             , 0x00000091, 0x00000004, 0x00000001},
  {"HIH_DEF_CFG"                          , 0x00000095, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_REW_PORT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PORT_TAG_DEFAULT"                     , 0x00000000, 0x00000001, 0x00000001},
  {"PORT_DP_MAP"                          , 0x00000001, 0x00000001, 0x00000001},
  {"PCP_MAP_DE0"                          , 0x00000002, 0x00000008, 0x00000001},
  {"PCP_MAP_DE1"                          , 0x0000000a, 0x00000008, 0x00000001},
  {"DEI_MAP_DE0"                          , 0x00000012, 0x00000008, 0x00000001},
  {"DEI_MAP_DE1"                          , 0x0000001a, 0x00000008, 0x00000001},
  {"TAG_CTRL"                             , 0x00000022, 0x00000001, 0x00000001},
  {"DSCP_MAP"                             , 0x00000023, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_REW_PHYSPORT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VSTAX_CTRL"                           , 0x00000000, 0x00000001, 0x00000001},
  {"PORT_CTRL"                            , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_REW_VMID[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RLEG_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_REW_STAT_GLOBAL_CFG_ESDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_GLOBAL_CFG"                      , 0x00000000, 0x00000004, 0x00000001},
  {"STAT_GLOBAL_EVENT_MASK"               , 0x00000004, 0x00000004, 0x00000001},
  {"STAT_RESET"                           , 0x00000008, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_REW_STAT_CNT_CFG_ESDX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"STAT_LSB_CNT"                         , 0x00000000, 0x00000004, 0x00000001},
  {"STAT_MSB_CNT"                         , 0x00000004, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_REW[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"COMMON"                               , 0x0000b900, 0x00000001, 0x00000099, regs_within_REW_COMMON},
  {"PORT"                                 , 0x00008000, 0x000000e0, 0x00000040, regs_within_REW_PORT},
  {"PHYSPORT"                             , 0x0000b999, 0x00000020, 0x00000002, regs_within_REW_PHYSPORT},
  {"VMID"                                 , 0x0000b800, 0x00000100, 0x00000001, regs_within_REW_VMID},
  {"STAT_GLOBAL_CFG_ESDX"                 , 0x0000b9d9, 0x00000001, 0x00000009, regs_within_REW_STAT_GLOBAL_CFG_ESDX},
  {"STAT_CNT_CFG_ESDX"                    , 0x00000000, 0x00001000, 0x00000008, regs_within_REW_STAT_CNT_CFG_ESDX},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_VCAP_ES0_ES0_CONTROL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ACL_CFG"                              , 0x00000000, 0x00000001, 0x00000001},
  {"ACE_UPDATE_CTRL"                      , 0x00000001, 0x00000001, 0x00000001},
  {"ACE_MV_CFG"                           , 0x00000002, 0x00000001, 0x00000001},
  {"ACE_STATUS"                           , 0x00000003, 0x00000001, 0x00000001},
  {"ACE_STICKY"                           , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_ES0_ISDX_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"ISDX1"                                , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_ES0_VID_ENTRY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"VID1"                                 , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_ES0_ISDX_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"ISDX1"                                , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_ES0_VID_MASK[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"VID1"                                 , 0x00000001, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_ES0_MACINMAC_ACTION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"MACINMAC1"                            , 0x00000001, 0x00000001, 0x00000001},
  {"MACINMAC2"                            , 0x00000002, 0x00000001, 0x00000001},
  {"B"                                    , 0x00000003, 0x00000001, 0x00000001},
  {"C"                                    , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_ES0_TAG_ACTION[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"A"                                    , 0x00000000, 0x00000001, 0x00000001},
  {"TAG1"                                 , 0x00000001, 0x00000001, 0x00000001},
  {"B"                                    , 0x00000002, 0x00000001, 0x00000001},
  {"C"                                    , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_VCAP_ES0_TCAM_BIST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TCAM_CTRL"                            , 0x00000000, 0x00000001, 0x00000001},






  {"TCAM_STAT"                            , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_VCAP_ES0[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"ES0_CONTROL"                          , 0x00000000, 0x00000001, 0x00000005, regs_within_VCAP_ES0_ES0_CONTROL},
  {"ISDX_ENTRY"                           , 0x00000005, 0x00000001, 0x00000001, regs_within_VCAP_ES0_ISDX_ENTRY},
  {"VID_ENTRY"                            , 0x00000007, 0x00000001, 0x00000001, regs_within_VCAP_ES0_VID_ENTRY},
  {"ISDX_MASK"                            , 0x00000009, 0x00000001, 0x00000001, regs_within_VCAP_ES0_ISDX_MASK},
  {"VID_MASK"                             , 0x0000000b, 0x00000001, 0x00000001, regs_within_VCAP_ES0_VID_MASK},
  {"MACINMAC_ACTION"                      , 0x0000000d, 0x00000001, 0x00000001, regs_within_VCAP_ES0_MACINMAC_ACTION},
  {"TAG_ACTION"                           , 0x00000015, 0x00000001, 0x00000001, regs_within_VCAP_ES0_TAG_ACTION},
  {"TCAM_BIST"                            , 0x0000001d, 0x00000001, 0x00000004, regs_within_VCAP_ES0_TCAM_BIST},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DSM_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width



  {"BUF_CFG"                              , 0x00000001, 0x00000020, 0x00000001},
  {"RATE_CTRL"                            , 0x00000021, 0x00000020, 0x00000001},
  {"RATE_CTRL_WM"                         , 0x00000041, 0x00000001, 0x00000001},
  {"RATE_CTRL_WM_DEV2G5"                  , 0x00000042, 0x00000009, 0x00000001},
  {"RATE_CTRL_WM_DEV10G"                  , 0x0000004b, 0x00000004, 0x00000001},
  {"IPG_SHRINK_CFG"                       , 0x0000004f, 0x00000001, 0x00000001},



  {"CLR_BUF"                              , 0x00000051, 0x00000002, 0x00000001},
  {"SCH_STOP_WM_CFG"                      , 0x00000053, 0x00000020, 0x00000001},
  {"RX_PAUSE_CFG"                         , 0x00000073, 0x00000020, 0x00000001},
  {"ETH_FC_GEN"                           , 0x00000093, 0x00000020, 0x00000001},
  {"MAC_CFG"                              , 0x000000b3, 0x00000020, 0x00000001},
  {"MAC_ADDR_BASE_HIGH_CFG"               , 0x000000d3, 0x00000020, 0x00000001},
  {"MAC_ADDR_BASE_LOW_CFG"                , 0x000000f3, 0x00000020, 0x00000001},















  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_DSM_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"AGED_FRMS"                            , 0x00000000, 0x00000021, 0x00000001},
  {"CELL_BUS_STICKY"                      , 0x00000021, 0x00000001, 0x00000001},
  {"BUF_OFLW_STICKY"                      , 0x00000022, 0x00000001, 0x00000001},
  {"BUF_UFLW_STICKY"                      , 0x00000023, 0x00000001, 0x00000001},





















  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_CM_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width



  {"CMM_MASK_CFG"                         , 0x00000001, 0x00000001, 0x00000001},
  {"CMM_QU_MASK_CFG"                      , 0x00000002, 0x00000008, 0x00000001},
  {"CMM_HOST_QU_MASK_CFG"                 , 0x0000000a, 0x00000006, 0x00000001},
  {"CMM_HOST_LPORT_MASK_CFG"              , 0x00000010, 0x00000002, 0x00000001},
  {"HOST_PORT_RESRC_CTL_CFG"              , 0x00000012, 0x00000001, 0x00000001},
  {"CMM_TO_ARB_CFG"                       , 0x00000013, 0x00000001, 0x00000001},
  {"CMEF_RATE_CFG"                        , 0x00000014, 0x00000001, 0x00000001},
  {"CMEF_GEN_CFG"                         , 0x00000015, 0x00000007, 0x00000001},
  {"CMEF_MAC_ADDR_LOW_CFG"                , 0x0000001c, 0x00000001, 0x00000001},
  {"CMEF_MAC_ADDR_HIGH_CFG"               , 0x0000001d, 0x00000001, 0x00000001},
  {"CMEF_OWN_UPSID_CFG"                   , 0x0000001e, 0x00000001, 0x00000001},
  {"CMEF_UPSID_ACTIVE_CFG"                , 0x0000001f, 0x00000001, 0x00000001},
  {"LPORT_NUM_CFG"                        , 0x00000020, 0x00000020, 0x00000001},
  {"LPORT_NUM_INIT"                       , 0x00000040, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_CM_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CMEF_RELAY_BUF_OFLW_STICKY"           , 0x00000000, 0x00000001, 0x00000001},
  {"CMM_XTRCT_CNT"                        , 0x00000001, 0x00000002, 0x00000001},
  {"CMM_XTRCT_STICKY"                     , 0x00000003, 0x00000001, 0x00000001},
  {"CMEF_DROP_STICKY"                     , 0x00000004, 0x00000002, 0x00000001},
  {"CMEF_RELAYED_STATUS"                  , 0x00000006, 0x00000002, 0x00000001},
  {"CMEF_GENERATED_STATUS"                , 0x00000008, 0x00000002, 0x00000001},
  {"CURRENT_CM_STATUS"                    , 0x0000000a, 0x0000000f, 0x00000001},
  {"CURRENT_LPORT_CM_STATUS"              , 0x00000019, 0x00000002, 0x00000001},






  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_SP_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width



  {"SP_KEEP_TTL_CFG"                      , 0x00000001, 0x00000001, 0x00000001},
  {"SP_TX_CFG"                            , 0x00000002, 0x00000007, 0x00000001},
  {"SP_XTRCT_CFG"                         , 0x00000009, 0x00000001, 0x00000001},
  {"SP_FRONT_PORT_INJ_CFG"                , 0x0000000a, 0x00000001, 0x00000001},



  {"SP_INJ_CFG"                           , 0x0000000c, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_SP_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width



  {"SP_FRONT_PORT_INJ_STAT"               , 0x00000001, 0x00000001, 0x00000001},
  {"SP_FRAME_CNT"                         , 0x00000002, 0x00000007, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_RATE_LIMIT_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TX_RATE_LIMIT_MODE"                   , 0x00000000, 0x00000020, 0x00000001},
  {"TX_IPG_STRETCH_RATIO_CFG"             , 0x00000020, 0x00000020, 0x00000001},
  {"TX_FRAME_RATE_START_CFG"              , 0x00000040, 0x00000020, 0x00000001},
  {"TX_RATE_LIMIT_HDR_CFG"                , 0x00000060, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_RATE_LIMIT_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TX_RATE_LIMIT_STICKY"                 , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_HOST_IF_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"HIH_CFG"                              , 0x00000000, 0x00000004, 0x00000001},
  {"HIH_ITAG_TPID_CFG"                    , 0x00000004, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_HOST_IF_IBS_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"XAUI_IBS_CFG"                         , 0x00000000, 0x00000002, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_RAM_INTEGRITY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RAM_INTEGRITY_ERR_STICKY"             , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_HOST_IF_CONF_IBS_CAL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"XAUI_IBS_CAL"                         , 0x00000000, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_HOST_IF_IBS_COMMON_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"CAPTURE_UPSID_CFG"                    , 0x00000000, 0x0000000a, 0x00000001},



  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_DSM_HOST_IF_OBS_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"XAUI_OBS_CFG"                         , 0x00000000, 0x00000002, 0x00000001},






  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_HOST_IF_CONF_OBS_CAL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"XAUI_OBS_CAL"                         , 0x00000000, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DSM_HOST_IF_OBS_STATUS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"XAUI_OBS_DIP2_ERR_CNT"                , 0x00000000, 0x00000002, 0x00000001},






  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DSM[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CFG"                                  , 0x00000298, 0x00000001, 0x0000011d, regs_within_DSM_CFG},



  {"STATUS"                               , 0x000005e0, 0x00000001, 0x0000006f, regs_within_DSM_STATUS},
  {"CM_CFG"                               , 0x000003b5, 0x00000001, 0x00000041, regs_within_DSM_CM_CFG},
  {"CM_STATUS"                            , 0x0000064f, 0x00000001, 0x0000002c, regs_within_DSM_CM_STATUS},
  {"SP_CFG"                               , 0x0000067b, 0x00000001, 0x0000000d, regs_within_DSM_SP_CFG},
  {"SP_STATUS"                            , 0x000003f6, 0x00000001, 0x00000009, regs_within_DSM_SP_STATUS},
  {"RATE_LIMIT_CFG"                       , 0x00000688, 0x00000001, 0x00000061, regs_within_DSM_RATE_LIMIT_CFG},
  {"RATE_LIMIT_STATUS"                    , 0x000003ff, 0x00000001, 0x00000001, regs_within_DSM_RATE_LIMIT_STATUS},
  {"HOST_IF_CFG"                          , 0x000006e9, 0x00000001, 0x00000005, regs_within_DSM_HOST_IF_CFG},
  {"HOST_IF_IBS_CFG"                      , 0x000006ee, 0x00000001, 0x00000004, regs_within_DSM_HOST_IF_IBS_CFG},
  {"RAM_INTEGRITY"                        , 0x000006f2, 0x00000001, 0x00000001, regs_within_DSM_RAM_INTEGRITY},
  {"HOST_IF_CONF_IBS_CAL"                 , 0x00000000, 0x00000140, 0x00000002, regs_within_DSM_HOST_IF_CONF_IBS_CAL},
  {"HOST_IF_IBS_COMMON_CFG"               , 0x000006f3, 0x00000001, 0x0000000b, regs_within_DSM_HOST_IF_IBS_COMMON_CFG},



  {"HOST_IF_OBS_CFG"                      , 0x000006fe, 0x00000001, 0x00000006, regs_within_DSM_HOST_IF_OBS_CFG},
  {"HOST_IF_CONF_OBS_CAL"                 , 0x00000400, 0x000000f0, 0x00000002, regs_within_DSM_HOST_IF_CONF_OBS_CAL},
  {"HOST_IF_OBS_STATUS"                   , 0x00000704, 0x00000001, 0x00000006, regs_within_DSM_HOST_IF_OBS_STATUS},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEVCPU_ORG_ORG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"ERR_ACCESS_DROP"                      , 0x00000000, 0x00000001, 0x00000001},



  {"ERR_TGT"                              , 0x00000002, 0x00000001, 0x00000001},
  {"ERR_CNTS"                             , 0x00000003, 0x00000001, 0x00000001},









  {"CFG_STATUS"                           , 0x00000007, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEVCPU_ORG[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"ORG"                                  , 0x00000000, 0x00000001, 0x00000008, regs_within_DEVCPU_ORG_ORG},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_CHIP_REGS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"GENERAL_PURPOSE"                      , 0x00000000, 0x00000001, 0x00000001},
  {"SI"                                   , 0x00000001, 0x00000001, 0x00000001},
  {"CHIP_ID"                              , 0x00000002, 0x00000001, 0x00000001},
  {"HW_STAT"                              , 0x00000003, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};









static const SYMREG_reg_t regs_within_DEVCPU_GCB_SW_REGS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SEMA_INTR_ENA"                        , 0x00000000, 0x00000001, 0x00000001},
  {"SEMA_INTR_ENA_CLR"                    , 0x00000001, 0x00000001, 0x00000001},
  {"SEMA_INTR_ENA_SET"                    , 0x00000002, 0x00000001, 0x00000001},
  {"SEMA"                                 , 0x00000003, 0x00000008, 0x00000001},
  {"SEMA_FREE"                            , 0x0000000b, 0x00000001, 0x00000001},
  {"SW_INTR"                              , 0x0000000c, 0x00000001, 0x00000001},
  {"MAILBOX"                              , 0x0000000d, 0x00000001, 0x00000001},
  {"MAILBOX_CLR"                          , 0x0000000e, 0x00000001, 0x00000001},
  {"MAILBOX_SET"                          , 0x0000000f, 0x00000001, 0x00000001},
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
static const SYMREG_reg_t regs_within_DEVCPU_GCB_DEVCPU_RST_REGS[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SOFT_CHIP_RST"                        , 0x00000000, 0x00000001, 0x00000001},
  {"SOFT_DEVCPU_RST"                      , 0x00000001, 0x00000001, 0x00000001},
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
static const SYMREG_reg_t regs_within_DEVCPU_GCB_VAUI2_MUX[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"VAUI2_MUX"                            , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};










static const SYMREG_reg_t regs_within_DEVCPU_GCB_RAM_STAT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RAM_INTEGRITY_ERR_STICKY"             , 0x00000000, 0x00000001, 0x00000001},
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
static const SYMREG_reg_t regs_within_DEVCPU_GCB_PTP_CFG[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PTP_MISC_CFG"                         , 0x00000000, 0x00000001, 0x00000001},
  {"PTP_UPPER_LIMIT_CFG"                  , 0x00000001, 0x00000001, 0x00000001},
  {"PTP_UPPER_LIMIT_1_TIME_ADJ_CFG"       , 0x00000002, 0x00000001, 0x00000001},
  {"PTP_SYNC_INTR_ENA_CFG"                , 0x00000003, 0x00000001, 0x00000001},
  {"GEN_EXT_CLK_HIGH_PERIOD_CFG"          , 0x00000004, 0x00000001, 0x00000001},
  {"GEN_EXT_CLK_LOW_PERIOD_CFG"           , 0x00000005, 0x00000001, 0x00000001},
  {"GEN_EXT_CLK_CFG"                      , 0x00000006, 0x00000001, 0x00000001},
  {"CLK_ADJ_CFG"                          , 0x00000007, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_GCB_PTP_STAT[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PTP_CURRENT_TIME_STAT"                , 0x00000000, 0x00000001, 0x00000001},
  {"EXT_SYNC_CURRENT_TIME_STAT"           , 0x00000001, 0x00000001, 0x00000001},
  {"PTP_EVT_STAT"                         , 0x00000002, 0x00000001, 0x00000001},
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
static const SYMREG_reggrp_t reggrps_within_DEVCPU_GCB[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CHIP_REGS"                            , 0x00000000, 0x00000001, 0x00000004, regs_within_DEVCPU_GCB_CHIP_REGS},



  {"SW_REGS"                              , 0x00000005, 0x00000001, 0x00000010, regs_within_DEVCPU_GCB_SW_REGS},
  {"VCORE_ACCESS"                         , 0x00000015, 0x00000001, 0x00000005, regs_within_DEVCPU_GCB_VCORE_ACCESS},
  {"GPIO"                                 , 0x0000001a, 0x00000001, 0x0000000a, regs_within_DEVCPU_GCB_GPIO},
  {"DEVCPU_RST_REGS"                      , 0x00000024, 0x00000001, 0x00000002, regs_within_DEVCPU_GCB_DEVCPU_RST_REGS},



  {"MIIM"                                 , 0x00000028, 0x00000002, 0x00000009, regs_within_DEVCPU_GCB_MIIM},
  {"MIIM_READ_SCAN"                       , 0x0000003a, 0x00000001, 0x00000002, regs_within_DEVCPU_GCB_MIIM_READ_SCAN},
  {"VAUI2_MUX"                            , 0x0000003c, 0x00000001, 0x00000001, regs_within_DEVCPU_GCB_VAUI2_MUX},



  {"RAM_STAT"                             , 0x0000003f, 0x00000001, 0x00000001, regs_within_DEVCPU_GCB_RAM_STAT},
  {"TEMP_SENSOR"                          , 0x00000040, 0x00000001, 0x00000002, regs_within_DEVCPU_GCB_TEMP_SENSOR},
  {"SIO_CTRL"                             , 0x00000042, 0x00000002, 0x00000030, regs_within_DEVCPU_GCB_SIO_CTRL},
  {"PTP_CFG"                              , 0x000000a2, 0x00000001, 0x00000008, regs_within_DEVCPU_GCB_PTP_CFG},
  {"PTP_STAT"                             , 0x000000aa, 0x00000001, 0x00000003, regs_within_DEVCPU_GCB_PTP_STAT},
  {"MEMITGR"                              , 0x000000ad, 0x00000001, 0x00000006, regs_within_DEVCPU_GCB_MEMITGR},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEVCPU_QS_XTR[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"XTR_FRM_PRUNING"                      , 0x00000000, 0x0000000a, 0x00000001},
  {"XTR_GRP_CFG"                          , 0x0000000a, 0x00000004, 0x00000001},
  {"XTR_MAP"                              , 0x0000000e, 0x0000000a, 0x00000001},
  {"XTR_RD"                               , 0x00000018, 0x00000004, 0x00000001},
  {"XTR_QU_SEL"                           , 0x0000001c, 0x00000004, 0x00000001},
  {"XTR_QU_FLUSH"                         , 0x00000020, 0x00000001, 0x00000001},
  {"XTR_DATA_PRESENT"                     , 0x00000021, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_DEVCPU_QS_INJ[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"INJ_GRP_CFG"                          , 0x00000000, 0x00000005, 0x00000001},
  {"INJ_WR"                               , 0x00000005, 0x00000005, 0x00000001},
  {"INJ_CTRL"                             , 0x0000000a, 0x00000005, 0x00000001},
  {"INJ_STATUS"                           , 0x0000000f, 0x00000001, 0x00000001},
  {"INJ_ERR"                              , 0x00000010, 0x00000005, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEVCPU_QS[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"XTR"                                  , 0x00000000, 0x00000001, 0x00000023, regs_within_DEVCPU_QS_XTR},
  {"INJ"                                  , 0x00000023, 0x00000001, 0x00000016, regs_within_DEVCPU_QS_INJ},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_DEVCPU_PI_PI[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PI_CTRL"                              , 0x00000000, 0x00000001, 0x00000001},
  {"PI_CFG"                               , 0x00000001, 0x00000001, 0x00000001},
  {"PI_STAT"                              , 0x00000002, 0x00000001, 0x00000001},
  {"PI_MODE"                              , 0x00000003, 0x00000001, 0x00000001},
  {"PI_SLOW_DATA"                         , 0x00000004, 0x00000002, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_DEVCPU_PI[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"PI"                                   , 0x00000000, 0x00000001, 0x00000006, regs_within_DEVCPU_PI_PI},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_CPU_SYSTEM_CTRL[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"GPR"                                  , 0x00000000, 0x00000008, 0x00000001},



  {"RESET"                                , 0x00000009, 0x00000001, 0x00000001},
  {"GENERAL_CTRL"                         , 0x0000000a, 0x00000001, 0x00000001},



  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_PI_MST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PI_MST_CFG"                           , 0x00000000, 0x00000001, 0x00000001},
  {"PI_MST_CTRL"                          , 0x00000001, 0x00000004, 0x00000001},
  {"PI_MST_STATUS"                        , 0x00000005, 0x00000004, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_SPI_MST[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SPI_MST_CFG"                          , 0x00000000, 0x00000001, 0x00000001},



  {"SW_MODE"                              , 0x00000005, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_INTR[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"INTR"                                 , 0x00000000, 0x00000001, 0x00000001},
  {"INTR_ENA"                             , 0x00000001, 0x00000001, 0x00000001},
  {"INTR_ENA_CLR"                         , 0x00000002, 0x00000001, 0x00000001},
  {"INTR_ENA_SET"                         , 0x00000003, 0x00000001, 0x00000001},
  {"INTR_RAW"                             , 0x00000004, 0x00000001, 0x00000001},
  {"ICPU_IRQ0_ENA"                        , 0x00000005, 0x00000001, 0x00000001},
  {"ICPU_IRQ0_IDENT"                      , 0x00000006, 0x00000001, 0x00000001},
  {"ICPU_IRQ1_ENA"                        , 0x00000007, 0x00000001, 0x00000001},
  {"ICPU_IRQ1_IDENT"                      , 0x00000008, 0x00000001, 0x00000001},
  {"EXT_IRQ0_ENA"                         , 0x00000009, 0x00000001, 0x00000001},
  {"EXT_IRQ0_IDENT"                       , 0x0000000a, 0x00000001, 0x00000001},
  {"EXT_IRQ1_ENA"                         , 0x0000000b, 0x00000001, 0x00000001},
  {"EXT_IRQ1_IDENT"                       , 0x0000000c, 0x00000001, 0x00000001},
  {"DEV_IDENT"                            , 0x0000000d, 0x00000001, 0x00000001},
  {"EXT_IRQ0_INTR_CFG"                    , 0x0000000e, 0x00000001, 0x00000001},
  {"EXT_IRQ1_INTR_CFG"                    , 0x0000000f, 0x00000001, 0x00000001},
  {"SW0_INTR_CFG"                         , 0x00000010, 0x00000001, 0x00000001},
  {"SW1_INTR_CFG"                         , 0x00000011, 0x00000001, 0x00000001},
  {"MIIM1_INTR_CFG"                       , 0x00000012, 0x00000001, 0x00000001},
  {"MIIM0_INTR_CFG"                       , 0x00000013, 0x00000001, 0x00000001},
  {"PI_SD0_INTR_CFG"                      , 0x00000014, 0x00000001, 0x00000001},
  {"PI_SD1_INTR_CFG"                      , 0x00000015, 0x00000001, 0x00000001},
  {"UART_INTR_CFG"                        , 0x00000016, 0x00000001, 0x00000001},
  {"TIMER0_INTR_CFG"                      , 0x00000017, 0x00000001, 0x00000001},
  {"TIMER1_INTR_CFG"                      , 0x00000018, 0x00000001, 0x00000001},
  {"TIMER2_INTR_CFG"                      , 0x00000019, 0x00000001, 0x00000001},
  {"FDMA_INTR_CFG"                        , 0x0000001a, 0x00000001, 0x00000001},
  {"TWI_INTR_CFG"                         , 0x0000001b, 0x00000001, 0x00000001},
  {"GPIO_INTR_CFG"                        , 0x0000001c, 0x00000001, 0x00000001},
  {"SGPIO_INTR_CFG"                       , 0x0000001d, 0x00000001, 0x00000001},
  {"DEV_ALL_INTR_CFG"                     , 0x0000001e, 0x00000001, 0x00000001},
  {"BLK_ANA_INTR_CFG"                     , 0x0000001f, 0x00000001, 0x00000001},
  {"XTR_RDY0_INTR_CFG"                    , 0x00000020, 0x00000001, 0x00000001},
  {"XTR_RDY1_INTR_CFG"                    , 0x00000021, 0x00000001, 0x00000001},
  {"XTR_RDY2_INTR_CFG"                    , 0x00000022, 0x00000001, 0x00000001},
  {"XTR_RDY3_INTR_CFG"                    , 0x00000023, 0x00000001, 0x00000001},
  {"INJ_RDY0_INTR_CFG"                    , 0x00000024, 0x00000001, 0x00000001},
  {"INJ_RDY1_INTR_CFG"                    , 0x00000025, 0x00000001, 0x00000001},
  {"INJ_RDY2_INTR_CFG"                    , 0x00000026, 0x00000001, 0x00000001},
  {"INJ_RDY3_INTR_CFG"                    , 0x00000027, 0x00000001, 0x00000001},
  {"INJ_RDY4_INTR_CFG"                    , 0x00000028, 0x00000001, 0x00000001},
  {"INTEGRITY_INTR_CFG"                   , 0x00000029, 0x00000001, 0x00000001},
  {"PTP_SYNC_INTR_CFG"                    , 0x0000002a, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_GPDMA[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"FDMA_CFG"                             , 0x00000000, 0x00000001, 0x00000001},



  {"FDMA_CH_CFG"                          , 0x00000002, 0x00000008, 0x00000001},
  {"FDMA_INJ_CFG"                         , 0x0000000a, 0x00000005, 0x00000001},
  {"FDMA_XTR_CFG"                         , 0x0000000f, 0x00000004, 0x00000001},
  {"FDMA_XTR_STAT_LAST_DCB"               , 0x00000013, 0x00000004, 0x00000001},
  {"FDMA_FRM_CNT"                         , 0x00000017, 0x00000001, 0x00000001},
  {"FDMA_BP_TO_INT"                       , 0x00000018, 0x00000001, 0x00000001},
  {"FDMA_BP_TO_DIV"                       , 0x00000019, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_INJ_FRM_SPC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"INJ_FRM_SPC_TMR"                      , 0x00000000, 0x00000001, 0x00000001},
  {"INJ_FRM_SPC_TMR_CFG"                  , 0x00000001, 0x00000001, 0x00000001},
  {"INJ_FRM_SPC_LACK_CNTR"                , 0x00000002, 0x00000001, 0x00000001},
  {"INJ_FRM_SPC_CFG"                      , 0x00000003, 0x00000001, 0x00000001},
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



  {"MEMCTRL_TIMING0"                      , 0x00000005, 0x00000001, 0x00000001},
  {"MEMCTRL_TIMING1"                      , 0x00000006, 0x00000001, 0x00000001},
  {"MEMCTRL_TIMING2"                      , 0x00000007, 0x00000001, 0x00000001},
  {"MEMCTRL_TIMING3"                      , 0x00000008, 0x00000001, 0x00000001},
  {"MEMCTRL_MR0_VAL"                      , 0x00000009, 0x00000001, 0x00000001},
  {"MEMCTRL_MR1_VAL"                      , 0x0000000a, 0x00000001, 0x00000001},
  {"MEMCTRL_MR2_VAL"                      , 0x0000000b, 0x00000001, 0x00000001},
  {"MEMCTRL_MR3_VAL"                      , 0x0000000c, 0x00000001, 0x00000001},
  {"MEMCTRL_TERMRES_CTRL"                 , 0x0000000d, 0x00000001, 0x00000001},



  {"MEMCTRL_DQS_DLY"                      , 0x0000000f, 0x00000002, 0x00000001},
  {"MEMCTRL_DQS_AUTO"                     , 0x00000011, 0x00000002, 0x00000001},
  {"MEMPHY_CFG"                           , 0x00000013, 0x00000001, 0x00000001},












  {"MEMPHY_ZCAL"                          , 0x0000001d, 0x00000001, 0x00000001},









  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_ICPU_CFG_TWI_DELAY[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"TWI_CONFIG"                           , 0x00000000, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_ICPU_CFG[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CPU_SYSTEM_CTRL"                      , 0x00000000, 0x00000001, 0x0000000c, regs_within_ICPU_CFG_CPU_SYSTEM_CTRL},
  {"PI_MST"                               , 0x0000000c, 0x00000001, 0x00000009, regs_within_ICPU_CFG_PI_MST},
  {"SPI_MST"                              , 0x00000015, 0x00000001, 0x00000006, regs_within_ICPU_CFG_SPI_MST},
  {"INTR"                                 , 0x0000001b, 0x00000001, 0x0000002b, regs_within_ICPU_CFG_INTR},
  {"GPDMA"                                , 0x00000046, 0x00000001, 0x0000001a, regs_within_ICPU_CFG_GPDMA},
  {"INJ_FRM_SPC"                          , 0x00000060, 0x00000008, 0x00000004, regs_within_ICPU_CFG_INJ_FRM_SPC},
  {"TIMERS"                               , 0x00000080, 0x00000001, 0x0000000b, regs_within_ICPU_CFG_TIMERS},
  {"MEMCTRL"                              , 0x0000008b, 0x00000001, 0x00000021, regs_within_ICPU_CFG_MEMCTRL},
  {"TWI_DELAY"                            , 0x000000ac, 0x00000001, 0x00000001, regs_within_ICPU_CFG_TWI_DELAY},
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
static const SYMREG_reg_t regs_within_SBA_SBA[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"PL1"                                  , 0x00000000, 0x00000001, 0x00000001},
  {"PL2"                                  , 0x00000001, 0x00000001, 0x00000001},
  {"PL3"                                  , 0x00000002, 0x00000001, 0x00000001},












  {"WT_EN"                                , 0x00000013, 0x00000001, 0x00000001},
  {"WT_TCL"                               , 0x00000014, 0x00000001, 0x00000001},
  {"WT_CL1"                               , 0x00000015, 0x00000001, 0x00000001},
  {"WT_CL2"                               , 0x00000016, 0x00000001, 0x00000001},
  {"WT_CL3"                               , 0x00000017, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_SBA[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"SBA"                                  , 0x00000000, 0x00000001, 0x00000018, regs_within_SBA_SBA},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_reg_t regs_within_FDMA_CH[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"SAR"                                  , 0x00000000, 0x00000001, 0x00000001},
  {"DAR"                                  , 0x00000002, 0x00000001, 0x00000001},
  {"LLP"                                  , 0x00000004, 0x00000001, 0x00000001},
  {"CTL0"                                 , 0x00000006, 0x00000001, 0x00000001},
  {"CTL1"                                 , 0x00000007, 0x00000001, 0x00000001},
  {"DSTAT"                                , 0x0000000a, 0x00000001, 0x00000001},
  {"DSTATAR"                              , 0x0000000e, 0x00000001, 0x00000001},
  {"CFG0"                                 , 0x00000010, 0x00000001, 0x00000001},
  {"CFG1"                                 , 0x00000011, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reg_t regs_within_FDMA_INTR[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"RAW_TFR"                              , 0x00000000, 0x00000001, 0x00000001},
  {"RAW_BLOCK"                            , 0x00000002, 0x00000001, 0x00000001},






  {"RAW_ERR"                              , 0x00000008, 0x00000001, 0x00000001},
  {"STATUS_TFR"                           , 0x0000000a, 0x00000001, 0x00000001},
  {"STATUS_BLOCK"                         , 0x0000000c, 0x00000001, 0x00000001},






  {"STATUS_ERR"                           , 0x00000012, 0x00000001, 0x00000001},
  {"MASK_TFR"                             , 0x00000014, 0x00000001, 0x00000001},
  {"MASK_BLOCK"                           , 0x00000016, 0x00000001, 0x00000001},






  {"MASK_ERR"                             , 0x0000001c, 0x00000001, 0x00000001},
  {"STATUSINT"                            , 0x00000028, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};












static const SYMREG_reg_t regs_within_FDMA_MISC[] = {
  //reg name                              , addr      , repl_cnt  , repl_width
  {"DMA_CFG_REG"                          , 0x00000000, 0x00000001, 0x00000001},
  {"CH_EN_REG"                            , 0x00000002, 0x00000001, 0x00000001},










































  {"DMA_COMP_VERSION"                     , 0x00000019, 0x00000001, 0x00000001},
  {NULL, 0, 0, 0}
};
static const SYMREG_reggrp_t reggrps_within_FDMA[] = {
  //reggrp name                           , base_addr , repl_cnt  , repl_width, reg list
  {"CH"                                   , 0x00000000, 0x00000008, 0x00000016, regs_within_FDMA_CH},
  {"INTR"                                 , 0x000000b0, 0x00000001, 0x0000002a, regs_within_FDMA_INTR},



  {"MISC"                                 , 0x000000e6, 0x00000001, 0x0000001a, regs_within_FDMA_MISC},
  {NULL, 0, 0, 0, NULL}
};
static const SYMREG_target_t SYMREG_targets[] = {
  //target name         , repl, tgt_id    , base_addr                  , register group list
  {"DEV1G"              ,    0, 0x00000004, VTSS_IO_OFFSET1(0x00040000), reggrps_within_DEV1G},
  {"DEV1G"              ,    1, 0x00000005, VTSS_IO_OFFSET1(0x00050000), reggrps_within_DEV1G},
  {"DEV1G"              ,    2, 0x00000006, VTSS_IO_OFFSET1(0x00060000), reggrps_within_DEV1G},
  {"DEV1G"              ,    3, 0x00000007, VTSS_IO_OFFSET1(0x00070000), reggrps_within_DEV1G},
  {"DEV1G"              ,    4, 0x00000008, VTSS_IO_OFFSET1(0x00080000), reggrps_within_DEV1G},
  {"DEV1G"              ,    5, 0x00000009, VTSS_IO_OFFSET1(0x00090000), reggrps_within_DEV1G},
  {"DEV1G"              ,    6, 0x0000000a, VTSS_IO_OFFSET1(0x000a0000), reggrps_within_DEV1G},
  {"DEV1G"              ,    7, 0x0000000b, VTSS_IO_OFFSET1(0x000b0000), reggrps_within_DEV1G},
  {"DEV1G"              ,    8, 0x0000000c, VTSS_IO_OFFSET1(0x000c0000), reggrps_within_DEV1G},
  {"DEV1G"              ,    9, 0x0000000d, VTSS_IO_OFFSET1(0x000d0000), reggrps_within_DEV1G},
  {"DEV1G"              ,   10, 0x0000000e, VTSS_IO_OFFSET1(0x000e0000), reggrps_within_DEV1G},
  {"DEV1G"              ,   11, 0x0000000f, VTSS_IO_OFFSET1(0x000f0000), reggrps_within_DEV1G},
  {"DEV1G"              ,   12, 0x00000010, VTSS_IO_OFFSET1(0x00100000), reggrps_within_DEV1G},
  {"DEV1G"              ,   13, 0x00000011, VTSS_IO_OFFSET1(0x00110000), reggrps_within_DEV1G},
  {"DEV1G"              ,   14, 0x00000012, VTSS_IO_OFFSET1(0x00120000), reggrps_within_DEV1G},
  {"DEV1G"              ,   15, 0x00000013, VTSS_IO_OFFSET1(0x00130000), reggrps_within_DEV1G},
  {"DEV1G"              ,   16, 0x00000014, VTSS_IO_OFFSET1(0x00140000), reggrps_within_DEV1G},
  {"DEV1G"              ,   17, 0x00000015, VTSS_IO_OFFSET1(0x00150000), reggrps_within_DEV1G},
  {"DEV2G5"             ,    0, 0x00000016, VTSS_IO_OFFSET1(0x00160000), reggrps_within_DEV2G5},
  {"DEV2G5"             ,    1, 0x00000017, VTSS_IO_OFFSET1(0x00170000), reggrps_within_DEV2G5},
  {"DEV2G5"             ,    2, 0x00000018, VTSS_IO_OFFSET1(0x00180000), reggrps_within_DEV2G5},
  {"DEV2G5"             ,    3, 0x00000019, VTSS_IO_OFFSET1(0x00190000), reggrps_within_DEV2G5},
  {"DEV2G5"             ,    4, 0x0000001a, VTSS_IO_OFFSET1(0x001a0000), reggrps_within_DEV2G5},
  {"DEV2G5"             ,    5, 0x0000001b, VTSS_IO_OFFSET1(0x001b0000), reggrps_within_DEV2G5},
  {"DEV2G5"             ,    6, 0x0000001c, VTSS_IO_OFFSET1(0x001c0000), reggrps_within_DEV2G5},
  {"DEV2G5"             ,    7, 0x0000001d, VTSS_IO_OFFSET1(0x001d0000), reggrps_within_DEV2G5},
  {"DEV2G5"             ,    8, 0x0000001e, VTSS_IO_OFFSET1(0x001e0000), reggrps_within_DEV2G5},
  {"DEVNPI"             ,   -1, 0x0000001f, VTSS_IO_OFFSET1(0x001f0000), reggrps_within_DEVNPI},
  {"DEV10G"             ,    0, 0x00000020, VTSS_IO_OFFSET1(0x00200000), reggrps_within_DEV10G},
  {"DEV10G"             ,    1, 0x00000021, VTSS_IO_OFFSET1(0x00210000), reggrps_within_DEV10G},
  {"DEV10G"             ,    2, 0x00000022, VTSS_IO_OFFSET1(0x00220000), reggrps_within_DEV10G},
  {"DEV10G"             ,    3, 0x00000023, VTSS_IO_OFFSET1(0x00230000), reggrps_within_DEV10G},
  {"HSIO"               ,   -1, 0x0000002d, VTSS_IO_OFFSET1(0x002d0000), reggrps_within_HSIO},
  {"ASM"                ,   -1, 0x00000024, VTSS_IO_OFFSET1(0x00240000), reggrps_within_ASM},
  {"ANA_CL"             ,   -1, 0x00000025, VTSS_IO_OFFSET1(0x00250000), reggrps_within_ANA_CL},
  {"VCAP_IS0"           ,   -1, 0x00000027, VTSS_IO_OFFSET1(0x00270000), reggrps_within_VCAP_IS0},
  {"VCAP_IS1"           ,   -1, 0x00000028, VTSS_IO_OFFSET1(0x00280000), reggrps_within_VCAP_IS1},
  {"ANA_L3"             ,   -1, 0x00000080, VTSS_IO_OFFSET1(0x00800000), reggrps_within_ANA_L3},
  {"VCAP_IS2"           ,   -1, 0x00000029, VTSS_IO_OFFSET1(0x00290000), reggrps_within_VCAP_IS2},
  {"ANA_L2"             ,   -1, 0x00000090, VTSS_IO_OFFSET1(0x00900000), reggrps_within_ANA_L2},
  {"LRN"                ,   -1, 0x00000026, VTSS_IO_OFFSET1(0x00260000), reggrps_within_LRN},
  {"ANA_AC"             ,   -1, 0x000000a0, VTSS_IO_OFFSET1(0x00a00000), reggrps_within_ANA_AC},
  {"IQS"                ,   -1, 0x000000b0, VTSS_IO_OFFSET1(0x00b00000), reggrps_within_IQS},
  {"ARB"                ,   -1, 0x0000002a, VTSS_IO_OFFSET1(0x002a0000), reggrps_within_ARB},
  {"OQS"                ,   -1, 0x000000c0, VTSS_IO_OFFSET1(0x00c00000), reggrps_within_OQS},
  {"SCH"                ,   -1, 0x0000002b, VTSS_IO_OFFSET1(0x002b0000), reggrps_within_SCH},
  {"REW"                ,   -1, 0x000000d0, VTSS_IO_OFFSET1(0x00d00000), reggrps_within_REW},
  {"VCAP_ES0"           ,   -1, 0x0000002e, VTSS_IO_OFFSET1(0x002e0000), reggrps_within_VCAP_ES0},
  {"DSM"                ,   -1, 0x0000002c, VTSS_IO_OFFSET1(0x002c0000), reggrps_within_DSM},
  {"DEVCPU_ORG"         ,   -1, 0x00000000, VTSS_IO_OFFSET1(0x00000000), reggrps_within_DEVCPU_ORG},
  {"DEVCPU_GCB"         ,   -1, 0x00000001, VTSS_IO_OFFSET1(0x00010000), reggrps_within_DEVCPU_GCB},
  {"DEVCPU_QS"          ,   -1, 0x00000002, VTSS_IO_OFFSET1(0x00020000), reggrps_within_DEVCPU_QS},
  {"DEVCPU_PI"          ,   -1, 0x00000003, VTSS_IO_OFFSET1(0x00030000), reggrps_within_DEVCPU_PI},
  {"ICPU_CFG"           ,   -1, 0x000000ff, VTSS_IO_OFFSET2(0x00000000), reggrps_within_ICPU_CFG},
  {"UART"               ,   -1, 0x00000001, VTSS_IO_OFFSET2(0x00100000), reggrps_within_UART},
  {"TWI"                ,   -1, 0x00000002, VTSS_IO_OFFSET2(0x00100400), reggrps_within_TWI},
  {"SBA"                ,   -1, 0x00000003, VTSS_IO_OFFSET2(0x00110000), reggrps_within_SBA},
  {"FDMA"               ,   -1, 0x00000004, VTSS_IO_OFFSET2(0x00110800), reggrps_within_FDMA},
};

#define SYMREG_REPL_CNT_MAX 4096
#define SYMREG_NAME_LEN_MAX 37
#endif /* VTSS_ARCH_JAGUAR_1 */
