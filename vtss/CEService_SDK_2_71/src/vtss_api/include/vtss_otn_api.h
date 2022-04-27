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

 $Id:$
 $Revision: $
*/

/**
 * \file vtss_otn_api.h
 * \brief  OTN API
 */

#ifndef _VTSS_OTN_API_H_
#define _VTSS_OTN_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_OTN)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */

/** \brief otn config data   */
typedef struct {    
}vtss_otn_cfg_t;



/* ================================================================= *
 *  OCH Types
 * ================================================================= */
/** \brief  OTN OCH FEC type enum */
typedef enum {
    VTSS_OTN_OCH_FEC_NONE,     /**< FEC disabled */
    VTSS_OTN_OCH_FEC_RS,       /**< FEC type is Standard RS FEC (ITU-T G.709) */
    VTSS_OTN_OCH_FEC_I4,       /**< FEC type is enhanced FEC (ITU-TG.795.1, I.4) */
    VTSS_OTN_OCH_FEC_I7,       /**< FEC type is enhanced FEC (ITU-TG.795.1, I.7) */
    VTSS_OTN_OCH_FEC_MAX,
} vtss_otn_och_fec_type_t;

/** \brief  OTN OCH FEC configuration struct */
typedef struct {          
    vtss_otn_och_fec_type_t   type;     /**< FEC type */
    u16                       ber_threshold; /**< FEC Ber Threshold. For RS Fec, 0 => 10**-4 and 1 => 10**-3. 
                                                  For I7 Fec,8356 is default value.8356 => 2*10**-3 */
} vtss_otn_och_fec_t;


/** \brief  OTN OCH loop back configuration struct */
typedef struct {          
    BOOL  line_side_equipment_loopback;        /**< TRUE => select line side equipment loopback (TX->RX), FALSE => no loopback */
    BOOL  line_side_line_loopback;             /**< TRUE => select line side line loopback (RX->TX), FALSE => no loopback */
    BOOL  system_side_line_loopback;           /**< TRUE => select system side line loopback (RX->TX) FALSE => no loopback */
    BOOL  system_side_equipment_loopback;      /**< TRUE => select system side equipment loopback (TX->RX) FALSE => no loopback */
} vtss_otn_och_loop_t;


/** \brief  OTN OCH defect struct */
typedef struct {
    BOOL  dlos;      /**< LOS defect status */
    BOOL  dlof;      /**< LOF defect status */
    BOOL  dlom;      /**< LOM defect status */
    BOOL  dais;      /**< AIS defect status */
    BOOL  doof;      /**< OOF defect status */
    BOOL  doom;      /**< OOM defect status */
} vtss_otn_och_defects_t;


/** \brief  OTN OCH performance struct */
typedef struct {
    u32  pfec_corrected_symbols;        /**< FEC corrected symbols */
    u32  pfec_corrected_zero;           /**< FEC corrected zero */
    u32  pfec_corrected_one;            /**< FEC corrected one */
    u32  pfec_uncorrectable_codewords;  /**< FEC un-correctable codewords */
} vtss_otn_och_perf_t;



/* ================================================================= *
 *  OTU Types
 * ================================================================= */
#define VTSS_OTN_OTU_TTI_NUMBER           64  /**< OTU TTI byte numbers */

/** \brief  OTN OTU TTI configuration struct */
typedef struct {          
    BOOL  tim_sapi;                           /**< Enable/disable TIM detection on SAPI */
    BOOL  tim_dapi;                           /**< Enable/disable TIM detection on DAPI */
    BOOL  tim_os;                             /**< Enable/disable TIM detection on OS */
                                              
    BOOL  tx_enable;                          /**< Enable/disable TTI transmission */
    u8    tx_tti[VTSS_OTN_OTU_TTI_NUMBER];    /**< Transmitted TTI */
    u8    exp_tti[VTSS_OTN_OTU_TTI_NUMBER];   /**< Expected TTI */
} vtss_otn_otu_tti_t;


/** \brief  OTN OTU accepted TTI struct */
typedef struct {
    BOOL  inconsistent;                       /**< Received TTI is determind inconsistent */
    u8    tti[VTSS_OTN_OTU_TTI_NUMBER];       /**< Accepted TTI */
} vtss_otn_otu_acc_tti_t;


/** \brief  OTN OTU AIS configuration struct */
typedef struct {
    BOOL  so_enable;     /**< Enable/disable forced OTU source AIS */
} vtss_otn_otu_ais_t;


/** \brief  OTN OTU auto consequent action configuration struct */
typedef struct {
    BOOL  beibiae;       /**< Enable/disable BEI+BIAE consequent action */
    BOOL  bdi;           /**< Enable/disable BDI consequent action */
    BOOL  ais;           /**< Enable/disable AIS consequent action */
    BOOL  iae;           /**< Enable/disable IAE consequent action */

    BOOL  bdi_los;       /**< Enable/disable LOS constribution to BDI (SSF) */
    BOOL  bdi_lof;       /**< Enable/disable LOF constribution to BDI (SSF) */
    BOOL  bdi_lom;       /**< Enable/disable LOM constribution to BDI (SSF) */
    BOOL  bdi_ais;       /**< Enable/disable AIS constribution to BDI (SSF) */
    BOOL  bdi_tim;       /**< Enable/disable TIM constribution to BDI */

    BOOL  ais_lom;       /**< Enable/disable LOM constribution to AIS */
    BOOL  ais_tim;       /**< Enable/disable TIM constribution to AIS */
} vtss_otn_otu_cons_act_t;


/** \brief  OTN OTU OH insertion mode enum */
typedef enum {
    VTSS_OTN_OTU_OH_INS_NORMAL,     /**< Overhead insertion is automatic or by value in register */
    VTSS_OTN_OTU_OH_INS_PORT,       /**< Overhead insertion is from overhead data port */
    VTSS_OTN_OTU_OH_INS_BLANK,      /**< Overhead insertion is all blank */
    VTSS_OTN_OTU_OH_INS_MAX,
} vtss_otn_otu_oh_ins_mode_t;

/** \brief  OTN OTU over head insertion configuration struct */
typedef struct {
    vtss_otn_otu_oh_ins_mode_t  mode;   /**< Overhead insertion mode */
} vtss_otn_otu_oh_ins_t;


/** \brief  OTN OTU transmitted RES configuration struct */
typedef struct {
    BOOL  enable;     /**< Enable/disable of RES value transmission*/
    u32   value;      /**< Transmitted RES value if enabled */
} vtss_otn_otu_tx_res_t;


/** \brief  OTN OTU accepted RES struct */
typedef struct {
    BOOL  inconsistent; /**< Received RES value is determind inconsistent */
    u32   value;        /**< Received and accepted RES value */
} vtss_otn_otu_acc_res_t;


/** \brief  OTN OTU transmitted SM RES configuration struct */
typedef struct {
    BOOL  enable;     /**< Enable/disable of SM RES value transmission*/
    u32   value;      /**< Transmitted SM RES value if enabled */
} vtss_otn_otu_tx_smres_t;


/** \brief  OTN OTU accepted SM RES struct */
typedef struct {
    BOOL  inconsistent; /**< Received SM RES value is determind inconsistent */
    u32   value;        /**< Received and accepted SM RES value */
} vtss_otn_otu_acc_smres_t;


/** \brief  OTN OTU transmitted GCC0 configuration struct */
typedef struct {
    BOOL  enable;     /**< Enable/disable of GCC0 value transmission*/
    u32   value;      /**< Transmitted GCC0 value if enabled */
} vtss_otn_otu_tx_gcc0_t;


/** \brief  OTN OTU accepted GCC0 struct */
typedef struct {
    BOOL  inconsistent; /**< Received GCC0 value is determind inconsistent */
    u32   value;        /**< Received and accepted GCC0 value */
} vtss_otn_otu_acc_gcc0_t;


/** \brief  OTN OTU defect struct */
typedef struct {
    BOOL  dtim;      /**< TIM defect status - Mismatch or Inconsistent */
    BOOL  dbdi;      /**< BDI defect status */
    BOOL  diae;      /**< IAE defect status */
    BOOL  dbiae;     /**< BIAE defect status */
} vtss_otn_otu_defects_t;


/** \brief  OTN OTU performance struct */
typedef struct {
    u32  pnebc;       /**< Near End Errored Block Count */
    u32  pfebc;       /**< Far End Errored Block Count */
} vtss_otn_otu_perf_t;



/* ================================================================= *
 *  ODU Types
 * ================================================================= */
#define VTSS_OTN_ODU_APS_NUMBER           4   /**< ODU APS byte numbers */
#define VTSS_OTN_ODU_TTI_NUMBER           64  /**< ODU TTI byte numbers */

/** \brief  OTN ODU mode enum */
typedef enum {
    VTSS_OTN_ODU_MODE_TERMINATION,   /**< ODU mode is Termination - ODU BIP-8 is generated */
    VTSS_OTN_ODU_MODE_MONITOR,       /**< ODU mode is Monitor - no ODU BIP-8 is generated */
    VTSS_OTN_ODU_MODE_MAX,
} vtss_otn_odu_mode_mode_t;

/** \brief  OTN ODU Mode configuration */
typedef struct {
    vtss_otn_odu_mode_mode_t  mode;   /**< ODU mode */
} vtss_otn_odu_mode_t;


/** \brief  OTN ODU TTI configuration struct */
typedef struct {          
    BOOL  tim_sapi;                           /**< Enable/disable TIM detection on SAPI */
    BOOL  tim_dapi;                           /**< Enable/disable TIM detection on DAPI */
    BOOL  tim_os;                             /**< Enable/disable TIM detection on OS */
                                              
    BOOL  tx_enable;                          /**< Enable/disable TTI transmission */
    u8    tx_tti[VTSS_OTN_ODU_TTI_NUMBER];    /**< Transmitted TTI */
    u8    exp_tti[VTSS_OTN_ODU_TTI_NUMBER];   /**< Expected TTI */
} vtss_otn_odu_tti_t;


/** \brief  OTN ODU accepted TTI struct */
typedef struct {
    BOOL  inconsistent;                       /**< Received TTI is determind inconsistent */
    u8    tti[VTSS_OTN_ODU_TTI_NUMBER];       /**< Accepted TTI */
} vtss_otn_odu_acc_tti_t;


/** \brief  OTN ODU AIS configuration struct */
typedef struct {
    BOOL  so_enable;   /**< Enable/disable forced ODU source AIS */
    BOOL  si_enable;   /**< Enable/disable forced ODU sink AIS */
} vtss_otn_odu_ais_t;


/** \brief  OTN ODU auto consequent action configuration struct */
typedef struct {
    BOOL  bei;            /**< Enable/disable BEI consequent action */
    BOOL  bdi;            /**< Enable/disable BDI consequent action */
    BOOL  ais;            /**< Enable/disable AIS consequent action */
                          
    BOOL  bdi_los;        /**< Enable/disable LOS constribution to BDI (SSF) */
    BOOL  bdi_lof;        /**< Enable/disable LOF constribution to BDI (SSF) */
    BOOL  bdi_lom;        /**< Enable/disable LOM constribution to BDI (SSF) */
    BOOL  bdi_otu_ais;    /**< Enable/disable OTU AIS constribution to BDI (SSF) */
    BOOL  bdi_otu_tim;    /**< Enable/disable OTU TIM constribution to BDI (SSF) */
    BOOL  bdi_ais;        /**< Enable/disable ODU AIS constribution to BDI */
    BOOL  bdi_oci;        /**< Enable/disable OCI constribution to BDI */
    BOOL  bdi_lck;        /**< Enable/disable LCK constribution to BDI */
    BOOL  bdi_tim;        /**< Enable/disable LCK constribution to BDI */
                          
    BOOL  ais_timocilck;  /**< Enable/disable TIM/OCI/LCK constribution to AIS */
    BOOL  ais_plm;        /**< Enable/disable PLM constribution to AIS */
} vtss_otn_odu_cons_act_t;


/** \brief  OTN ODU OH insertion mode enum */
typedef enum {
    VTSS_OTN_ODU_OH_INS_NORMAL,     /**< Overhead insertion is automatic or by value in register */
    VTSS_OTN_ODU_OH_INS_PORT,       /**< Overhead insertion is from overhead data port */
    VTSS_OTN_ODU_OH_INS_BLANK,      /**< Overhead insertion is all blank */
    VTSS_OTN_ODU_OH_INS_MAX,
} vtss_otn_odu_oh_ins_mode_t;

/** \brief  OTN ODU over head inseertion configuration struct */
typedef struct {
    vtss_otn_odu_oh_ins_mode_t  mode;   /**< Overhead insertion mode */
} vtss_otn_odu_oh_ins_t;


/** \brief  OTN ODU transmitted RES configuration struct */
typedef struct {
    BOOL  enable;     /**< Enable/disable of RES value transmission*/
    u32   value;      /**< Transmitted RES value if enabled */
} vtss_otn_odu_tx_res_t;

/** \brief  OTN ODU accepted RES struct */
typedef struct {
    BOOL  inconsistent; /**< Received RES value is determind inconsistent */
    u32   value;        /**< Received and accepted RES value */
} vtss_otn_odu_acc_res_t;


/** \brief  OTN ODU transmitted EXP configuration struct */
typedef struct {
    BOOL  enable;     /**< Enable/disable of EXP value transmission*/
    u32   value;      /**< Transmitted EXP value if enabled */
} vtss_otn_odu_tx_exp_t;


/** \brief  OTN ODU accepted EXP struct */
typedef struct {
    BOOL  inconsistent; /**< Received EXP value is determind inconsistent */
    u32   value;        /**< Received and accepted EXP value */
} vtss_otn_odu_acc_exp_t;


/** \brief  OTN ODU transmitted FTFL configuration struct */
typedef struct {
    BOOL  enable;          /**< Enable/disable of FTFL value transmission*/
    u8    value[256];      /**< Transmitted FTFL value if enabled */
} vtss_otn_odu_tx_ftfl_t;


/** \brief  OTN ODU accepted FTFL struct */
typedef struct {
    BOOL  inconsistent; /**< Received FTFL value is determind inconsistent */
    u8    value[256];   /**< Received and accepted FTFL value */
} vtss_otn_odu_acc_ftfl_t;


/** \brief  OTN ODU transmitted APS configuration struct */
typedef struct {
    BOOL  enable;                            /**< Enable/disable of APS value transmission*/
    u8    value[VTSS_OTN_ODU_APS_NUMBER];    /**< Transmitted APS value if enabled */
} vtss_otn_odu_tx_aps_t;


/** \brief  OTN ODU accepted APS struct */
typedef struct {
    BOOL  inconsistent;                      /**< Received APS value is determind inconsistent */
    u8    value[VTSS_OTN_ODU_APS_NUMBER];    /**< Received and accepted APS value */
} vtss_otn_odu_acc_aps_t;


/** \brief  OTN ODU defect struct */
typedef struct {
    BOOL  dais;      /**< AIS defect status */
    BOOL  dtim;      /**< TIM defect status - Mismatch or Inconsistent */
    BOOL  dbdi;      /**< BDI defect status */
    BOOL  doci;      /**< OCI defect status */
    BOOL  dlck;      /**< LCK defect status */
    BOOL  dplm;      /**< PLM defect status - Mismatch or Inconsistent */
} vtss_otn_odu_defects_t;


/** \brief  OTN ODU Payload Type configuration struct */
typedef struct {          
    BOOL  plm;          /**< Enable/disable PLM detection */
    BOOL  tx_enable;    /**< Enable/disable Payload type transmission */
    u32   pt;           /**< Transmitted and expected Payload type */
} vtss_otn_odu_pt_t;


/** \brief  OTN ODU accepted PT struct */
typedef struct {
    BOOL  inconsistent;  /**< Received PT is determind inconsistent */
    u32   pt;            /**< Accepted PT */
} vtss_otn_odu_acc_pt_t;


/** \brief  OTN ODU OPU Payload OH insertion mode enum */
typedef enum {
    VTSS_OTN_ODU_OPU_OH_INS_NORMAL,     /**< Payload overhead insertion is automatic or by value in register */
    VTSS_OTN_ODU_OPU_OH_INS_PORT,       /**< Payload overhead insertion is from overhead data port */
    VTSS_OTN_ODU_OPU_OH_INS_BLANK,      /**< Payload overhead insertion is all blank */
    VTSS_OTN_ODU_OPU_OH_INS_MAX,
} vtss_otn_odu_opu_oh_ins_mode_t;

/** \brief  OTN ODU OPU Payload over head inseertion configuration struct */
typedef struct {
    vtss_otn_odu_opu_oh_ins_mode_t  mode;   /**< Overhead insertion mode */
} vtss_otn_odu_opu_oh_ins_t;


/** \brief  OTN ODU OPU Test payload insertion mode enum */
typedef enum {
    VTSS_OTN_ODU_OPU_TEST_INS_NONE,     /**< Test payload insertion disabled */
    VTSS_OTN_ODU_OPU_TEST_INS_FIXED,    /**< Test payload insertion is fixed value */
    VTSS_OTN_ODU_OPU_TEST_INS_PN_23,    /**< Test payload insertion psuedo random PN-23 sequence */
    VTSS_OTN_ODU_OPU_TEST_INS_PN_31,    /**< Test payload insertion psuedo random PN-31 sequence */
    VTSS_OTN_ODU_OPU_TEST_INS_MAX,
} vtss_otn_odu_opu_test_ins_mode_t;

/** \brief  OTN ODU Test payload inseertion configuration struct */
typedef struct {
    u32                               payload_value;       /**< Test payload fixed value (bit0-bit7) */
    vtss_otn_odu_opu_test_ins_mode_t  ins_payload;         /**< Test payload insertion mode */
    BOOL                              ins_payload_type;    /**< Test payload type insertion mode */
} vtss_otn_odu_opu_test_ins_t;


/** \brief  OTN ODU performance struct */
typedef struct {
    u32  pnebc;       /**< Near End Errored Block Count */
    u32  pfebc;       /**< Far End Errored Block Count */
    u32  njus;        /**< OPU Negative justification Count */
    u32  pjus;        /**< OPU Positive justification Count */
    u32  prbs;        /**< OPU PRBS error Count */
} vtss_otn_odu_perf_t;



/* ================================================================= *
 *  ODUT Types
 * ================================================================= */
#define VTSS_OTN_ODUT_LEVEL_NUMBER         6   /**< ODU TC level numbers */
#define VTSS_OTN_ODUT_APS_NUMBER           4   /**< ODU TC APS byte numbers */
#define VTSS_OTN_ODUT_TTI_NUMBER           64  /**< ODU TTI byte numbers */

/** \brief  OTN ODUT mode enum */
typedef enum {
    VTSS_OTN_ODUT_MODE_MONITOR,       /**< ODU TC mode is Monitor - no ODUT BIP-8 is generated */
    VTSS_OTN_ODUT_MODE_TERMINATION,   /**< ODU TC mode is Termination - ODUT BIP-8 is generated */
    VTSS_OTN_ODUT_MODE_MAX,
} vtss_otn_odut_mode_mode_t;


/** \brief  OTN ODUT Mode configuration */
typedef struct {
    vtss_otn_odut_mode_mode_t  mode;   /**< ODU TC mode */
} vtss_otn_odut_mode_t;


/** \brief  OTN ODUT TTI configuration struct */
typedef struct {          
    BOOL  tim_sapi;                             /**< Enable/disable TIM detection on SAPI */
    BOOL  tim_dapi;                             /**< Enable/disable TIM detection on DAPI */
    BOOL  tim_os;                               /**< Enable/disable TIM detection on OS */
                        
    BOOL  tx_enable;                            /**< Enable/disable TTI transmission */
    u8    tx_tti[VTSS_OTN_ODUT_TTI_NUMBER];     /**< Transmitted TTI */
    u8    exp_tti[VTSS_OTN_ODUT_TTI_NUMBER];    /**< Expected TTI */
} vtss_otn_odut_tti_t;


/** \brief  OTN ODUT accepted TTI struct */
typedef struct {
    BOOL  inconsistent;                         /**< Received TTI is determind inconsistent */
    u8    tti[VTSS_OTN_ODUT_TTI_NUMBER];        /**< Accepted TTI */
} vtss_otn_odut_acc_tti_t;


/** \brief  OTN ODUT auto consequent action configuration struct */
typedef struct {
    BOOL  beibiae;        /**< Enable/disable BEI and BIAE consequent action */
    BOOL  bdi;            /**< Enable/disable BDI consequent action */
    BOOL  ais;            /**< Enable/disable AIS consequent action */
    BOOL  iae;            /**< Enable/disable IAE consequent action */
                          
    BOOL  bdi_los;        /**< Enable/disable LOS constribution to BDI (SSF) - There is only one common for all levels */
    BOOL  bdi_lof;        /**< Enable/disable LOF constribution to BDI (SSF) - There is only one common for all levels  */
    BOOL  bdi_lom;        /**< Enable/disable LOM constribution to BDI (SSF) - There is only one common for all levels  */
    BOOL  bdi_otu_ais;    /**< Enable/disable OTU AIS constribution to BDI (SSF) - There is only one common for all levels  */
    BOOL  bdi_otu_tim;    /**< Enable/disable OTU TIM constribution to BDI (SSF) - There is only one common for all levels  */
    BOOL  bdi_ais;        /**< Enable/disable ODU AIS constribution to BDI - There is only one common for all levels  */
    BOOL  bdi_oci;        /**< Enable/disable OCI constribution to BDI - There is only one common for all levels  */
    BOOL  bdi_lck;        /**< Enable/disable LCK constribution to BDI - There is only one common for all levels  */
    BOOL  bdi_ltc;        /**< Enable/disable LTC constribution to BDI - There is only one common for all levels  */
    BOOL  bdi_tim;        /**< Enable/disable LCK constribution to BDI - There is only one common for all levels  */
                          
    BOOL  ais_timocilck;  /**< Enable/disable TIM/OCI/LCK constribution to AIS */
} vtss_otn_odut_cons_act_t;


/** \brief  OTN ODUT BDI mode enum */
typedef enum {
    VTSS_OTN_ODUT_BDI_UNFORCED,      /**< BDI is not forced */
    VTSS_OTN_ODUT_BDI_FORCED_HIGH,   /**< BDI is forced high */
    VTSS_OTN_ODUT_BDI_FORCED_LOW,    /**< BDI is forced low */
} vtss_otn_odut_bdi_mode_t;


/** \brief  OTN ODUT BDI configuration */
typedef struct {
    vtss_otn_odut_bdi_mode_t  mode;   /**< BDI mode */
} vtss_otn_odut_bdi_t;


/** \brief  OTN ODUT transmitted STAT configuration struct */
typedef struct {
    BOOL  enable;     /**< Enable/disable of STAT value transmission */
    u32   value;      /**< Transmitted STAT value if enabled (bit0-bit2) */
} vtss_otn_odut_tx_stat_t;


/** \brief  OTN ODUT accepted STAT struct */
typedef struct {
    BOOL  inconsistent; /**< Received STAT value is determind inconsistent */
    u32   value;        /**< Received and accepted STAT value (bit0-bit2) */
} vtss_otn_odut_acc_stat_t;


/** \brief  OTN ODUT transmitted APS configuration struct */
typedef struct {
    BOOL  enable;                            /**< Enable/disable of APS value transmission*/
    u8   value[VTSS_OTN_ODUT_APS_NUMBER];    /**< Transmitted APS value if enabled */
} vtss_otn_odut_tx_aps_t;


/** \brief  OTN ODUT accepted APS struct */
typedef struct {
    BOOL  inconsistent;                      /**< Received APS value is determind inconsistent */
    u8   value[VTSS_OTN_ODUT_APS_NUMBER];    /**< Received and accepted APS value */
} vtss_otn_odut_acc_aps_t;


/** \brief  OTN ODUT defect struct */
typedef struct {
    BOOL  dais;      /**< AIS defect status */
    BOOL  dtim;      /**< TIM defect status - Mismatch or Inconsistent */
    BOOL  dbdi;      /**< BDI defect status */
    BOOL  doci;      /**< OCI defect status */
    BOOL  dlck;      /**< LCK defect status */
    BOOL  dltc;      /**< LTC defect status */
    BOOL  diae;      /**< IAE defect status */
    BOOL  dbiae;     /**< BIAE defect status */
} vtss_otn_odut_defects_t;


/** \brief  OTN ODUT performance struct */
typedef struct {
    u32  pnebc;       /**< Near End Errored Block Count */
    u32  pfebc;       /**< Far End Errored Block Count */
} vtss_otn_odut_perf_t;



/** \brief  OTN internal API state */
typedef struct {
    vtss_otn_och_loop_t         och_loop;      /**< OCH loop internal state */
    vtss_otn_och_fec_t          och_fec;       /**< OCH fec internal state */

    vtss_otn_otu_tti_t          otu_tti;       /**< OTU tti internal state */
    vtss_otn_otu_ais_t          otu_ais;       /**< OTU ais internal state */
    vtss_otn_otu_cons_act_t     otu_cons;      /**< OTU cons internal state */
    vtss_otn_otu_oh_ins_t       otu_oh;        /**< OTU oh internal state */
    vtss_otn_otu_tx_res_t       otu_res;       /**< OTU res internal state */
    vtss_otn_otu_tx_smres_t     otu_smres;     /**< OTU smres internal state */
    vtss_otn_otu_tx_gcc0_t      otu_gcc0;      /**< OTU gcc0 internal state */

    vtss_otn_odu_mode_t         odu_mode;      /**< ODU mode internal state */
    vtss_otn_odu_tti_t          odu_tti;       /**< ODU tti internal state */
    vtss_otn_odu_ais_t          odu_ais;       /**< ODU ais internal state */
    vtss_otn_odu_cons_act_t     odu_cons;      /**< ODU cons internal state */
    vtss_otn_odu_oh_ins_t       odu_oh;        /**< ODU oh internal state */
    vtss_otn_odu_tx_res_t       odu_res;       /**< ODU res internal state */
    vtss_otn_odu_tx_exp_t       odu_exp;       /**< ODU exp internal state */
    vtss_otn_odu_tx_ftfl_t      odu_ftfl;      /**< ODU ftfl internal state */
    vtss_otn_odu_tx_aps_t       odu_aps;       /**< ODU aps internal state */
    vtss_otn_odu_pt_t           odu_pt;        /**< ODU pt internal state */
    vtss_otn_odu_opu_oh_ins_t   odu_opu_oh;    /**< ODU OPU oh internal state */
    vtss_otn_odu_opu_test_ins_t odu_opu_test;  /**< ODU OPU test internal state */

    vtss_otn_odut_mode_t        odut_mode[VTSS_OTN_ODUT_LEVEL_NUMBER];      /**< ODUT mode internal state */
    vtss_otn_odut_tti_t         odut_tti[VTSS_OTN_ODUT_LEVEL_NUMBER];       /**< ODUT tti internal state */
    vtss_otn_odut_cons_act_t    odut_cons[VTSS_OTN_ODUT_LEVEL_NUMBER];      /**< ODUT cons internal state */
    vtss_otn_odut_bdi_t         odut_bdi[VTSS_OTN_ODUT_LEVEL_NUMBER];       /**< ODUT bdi internal state */
    vtss_otn_odut_tx_stat_t     odut_stat[VTSS_OTN_ODUT_LEVEL_NUMBER];      /**< ODUT stat internal state */
    vtss_otn_odut_tx_aps_t      odut_aps[VTSS_OTN_ODUT_LEVEL_NUMBER];       /**< ODUT aps internal state */
} vtss_otn_state_t;



/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief   OTN common configuration set.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       Common configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_config_set(const vtss_inst_t       inst,
                            const vtss_port_no_t    port_no,
                            const vtss_otn_cfg_t    *const cfg);


/**
 * \brief   OTN common configuration get.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [OUT]      Common configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_config_get(const vtss_inst_t       inst,
                            const vtss_port_no_t    port_no,
                            vtss_otn_cfg_t          *const cfg);
             
                          


/* ================================================================= *
 *  OCH Config
 * ================================================================= */
/**
 * \brief   OTN OCH FEC set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      FEC configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_och_fec_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_och_fec_t   *const cfg);


/**
 * \brief   OTN OCH get FEC configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     FEC configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_och_fec_get(const vtss_inst_t      inst,
                             const vtss_port_no_t   port_no,
                             vtss_otn_och_fec_t     *const cfg);

/**
 * \brief   OTN OCH FEC Ber threshold set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      FEC configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_och_ber_threshold_set(const vtss_inst_t          inst,
                                        const vtss_port_no_t       port_no,
                                        const vtss_otn_och_fec_t   *const cfg);

/**
 * \brief   OTN OCH get FEC Ber threshold configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     FEC configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_och_ber_threshold_get(const vtss_inst_t      inst,
                                        const vtss_port_no_t   port_no,
                                        vtss_otn_och_fec_t     *const cfg);


/**
 * \brief   OTN OCH set Loop configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Loop configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_och_loopback_set(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  const vtss_otn_och_loop_t *const cfg);


/**
 * \brief   OTN OCH get Loop configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Loop configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_och_loopback_get(const vtss_inst_t     inst,
                                  const vtss_port_no_t  port_no,
                                  vtss_otn_och_loop_t   *const cfg);





/* ================================================================= *
 *  OTU Config
 * ================================================================= */
/**
 * \brief   OTN OTU TTI set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TTI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_tti_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_otu_tti_t   *const cfg);


/**
 * \brief   OTN OTU get TTI configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TTI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_tti_get(const vtss_inst_t      inst,
                             const vtss_port_no_t   port_no,
                             vtss_otn_otu_tti_t     *const cfg);


/**
 * \brief   OTN OTU get accepted TTI.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param tti [OUT]     Accepted TTI.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_acc_tti_get(const vtss_inst_t         inst,
                                 const vtss_port_no_t      port_no,
                                 vtss_otn_otu_acc_tti_t    *const tti);


/**
 * \brief   OTN OTU AIS set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      AIS configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_ais_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_otu_ais_t   *const cfg);


/**
 * \brief   OTN OTU AIS get configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     AIS configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_ais_get(const vtss_inst_t       inst,
                             const vtss_port_no_t    port_no,
                             vtss_otn_otu_ais_t      *const cfg);


/**
 * \brief   OTN OTU  set Consequent Action configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Consequent Action configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_consequent_actions_set(const vtss_inst_t               inst,
                                            const vtss_port_no_t            port_no,
                                            const vtss_otn_otu_cons_act_t   *const cfg);


/**
 * \brief   OTN OTU  get Consequent Action configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Consequent Action configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_consequent_actions_get(const vtss_inst_t         inst,
                                            const vtss_port_no_t      port_no,
                                            vtss_otn_otu_cons_act_t   *const cfg);


/**
 * \brief   OTN OTU  set Over Head Insertion configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Over Head Insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_oh_insertion_set(const vtss_inst_t             inst,
                                      const vtss_port_no_t          port_no,
                                      const vtss_otn_otu_oh_ins_t   *const cfg);


/**
 * \brief   OTN OTU  get Over Head Insertion configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Over Head Insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_oh_insertion_get(const vtss_inst_t       inst,
                                      const vtss_port_no_t    port_no,
                                      vtss_otn_otu_oh_ins_t   *const cfg);


/**
 * \brief   OTN OTU  set TX RES configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TX RES configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_tx_res_set(const vtss_inst_t             inst,
                                const vtss_port_no_t          port_no,
                                const vtss_otn_otu_tx_res_t   *const cfg);


/**
 * \brief   OTN OTU  get TX RES configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TX RES configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_tx_res_get(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                vtss_otn_otu_tx_res_t   *const cfg);


/**
 * \brief   OTN OTU  get Accepted RES.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param res [OUT]     Accepted RES.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_acc_res_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_otu_acc_res_t   *const res);


/**
 * \brief   OTN OTU  set TX SM RES configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TX SM RES configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_tx_smres_set(const vtss_inst_t               inst,
                                  const vtss_port_no_t            port_no,
                                  const vtss_otn_otu_tx_smres_t   *const cfg);


/**
 * \brief   OTN OTU  get TX SM RES configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TX SM RES configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_tx_smres_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  vtss_otn_otu_tx_smres_t   *const cfg);


/**
 * \brief   OTN OTU  get accepted SM RES.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param smres [OUT]   Accepted SM RES.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_acc_smres_get(const vtss_inst_t          inst,
                                   const vtss_port_no_t       port_no,
                                   vtss_otn_otu_acc_smres_t   *const smres);


/**
 * \brief   OTN OTU  set TX GCC0 configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TX GCC0 configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_tx_gcc0_set(const vtss_inst_t              inst,
                                 const vtss_port_no_t           port_no,
                                 const vtss_otn_otu_tx_gcc0_t   *const cfg);


/**
 * \brief   OTN OTU  get TX GCC0 configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TX GCC0 configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_tx_gcc0_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_otu_tx_gcc0_t   *const cfg);


/**
 * \brief   OTN OTU  get accepted GCC0.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param gcc0 [OUT]    Accepted GCC0.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_acc_gcc0_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  vtss_otn_otu_acc_gcc0_t   *const gcc0);




/* ================================================================= *
 *  ODU Config
 * ================================================================= */
/**
 * \brief   OTN ODU  set Mode configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Mode configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_mode_set(const vtss_inst_t             inst,
                              const vtss_port_no_t          port_no,
                              const vtss_otn_odu_mode_t     *const cfg);


/**
 * \brief   OTN ODU  get mode configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     mode configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_mode_get(const vtss_inst_t     inst,
                              const vtss_port_no_t  port_no,
                              vtss_otn_odu_mode_t   *const cfg);


/**
 * \brief   OTN ODU TTI set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TTI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tti_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_odu_tti_t   *const cfg);


/**
 * \brief   OTN ODU get TTI configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TTI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tti_get(const vtss_inst_t      inst,
                             const vtss_port_no_t   port_no,
                             vtss_otn_odu_tti_t     *const cfg);


/**
 * \brief   OTN ODU get accepted TTI.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param tti [OUT]     Accepted TTI.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_acc_tti_get(const vtss_inst_t         inst,
                                 const vtss_port_no_t      port_no,
                                 vtss_otn_odu_acc_tti_t    *const tti);


/**
 * \brief   OTN ODU AIS set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      AIS configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_ais_set(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             const vtss_otn_odu_ais_t   *const cfg);


/**
 * \brief   OTN ODU AIS get configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     AIS configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_ais_get(const vtss_inst_t       inst,
                             const vtss_port_no_t    port_no,
                             vtss_otn_odu_ais_t      *const cfg);


/**
 * \brief   OTN ODU  set Consequent Action configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Consequent Action configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_consequent_actions_set(const vtss_inst_t               inst,
                                            const vtss_port_no_t            port_no,
                                            const vtss_otn_odu_cons_act_t   *const cfg);


/**
 * \brief   OTN ODU  get Consequent Action configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Consequent Action configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_consequent_actions_get(const vtss_inst_t         inst,
                                            const vtss_port_no_t      port_no,
                                            vtss_otn_odu_cons_act_t   *const cfg);


/**
 * \brief   OTN ODU  set Over Head Insertion configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Over Head Insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_oh_insertion_set(const vtss_inst_t             inst,
                                      const vtss_port_no_t          port_no,
                                      const vtss_otn_odu_oh_ins_t   *const cfg);


/**
 * \brief   OTN ODU  get Over Head Insertion configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Over Head Insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_oh_insertion_get(const vtss_inst_t       inst,
                                      const vtss_port_no_t    port_no,
                                      vtss_otn_odu_oh_ins_t   *const cfg);


/**
 * \brief   OTN ODU  set TX RES configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TX RES configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tx_res_set(const vtss_inst_t             inst,
                                const vtss_port_no_t          port_no,
                                const vtss_otn_odu_tx_res_t   *const cfg);


/**
 * \brief   OTN ODU  get TX RES configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TX RES configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tx_res_get(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                vtss_otn_odu_tx_res_t   *const cfg);


/**
 * \brief   OTN ODU  get Accepted RES.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param res [OUT]     Accepted RES.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_acc_res_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_odu_acc_res_t   *const res);


/**
 * \brief   OTN ODU  set TX EXP configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TX EXP configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tx_exp_set(const vtss_inst_t             inst,
                                const vtss_port_no_t          port_no,
                                const vtss_otn_odu_tx_exp_t   *const cfg);


/**
 * \brief   OTN ODU  get TX EXP configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TX EXP configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tx_exp_get(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                vtss_otn_odu_tx_exp_t   *const cfg);


/**
 * \brief   OTN ODU  get Accepted EXP.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param exp [OUT]     Accepted EXP.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_acc_exp_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_odu_acc_exp_t   *const exp);


/**
 * \brief   OTN ODU  set TX FTFL configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TX FTFL configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tx_ftfl_set(const vtss_inst_t              inst,
                                 const vtss_port_no_t           port_no,
                                 const vtss_otn_odu_tx_ftfl_t   *const cfg);


/**
 * \brief   OTN ODU  get TX FTFL configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TX FTFL configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tx_ftfl_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_odu_tx_ftfl_t   *const cfg);


/**
 * \brief   OTN ODU  get Accepted FTFL.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param ftfl [OUT]    Accepted FTFL.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_acc_ftfl_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  vtss_otn_odu_acc_ftfl_t   *const ftfl);


/**
 * \brief   OTN ODU  set TX APS configuration.
 *
 * \note Transmittin APS is only possible in one MFAS channel at the time. If APS is enabled on more MFAS channels then the prioritizing is like this:
 * ODU - ODUT level 1 - ODUT level 2 - ODUT level 3 - ODUT level 4 - ODUT level 5 - ODUT level 6
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      TX APS configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tx_aps_set(const vtss_inst_t             inst,
                                const vtss_port_no_t          port_no,
                                const vtss_otn_odu_tx_aps_t   *const cfg);


/**
 * \brief   OTN ODU  get TX APS configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     TX APS configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_tx_aps_get(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                vtss_otn_odu_tx_aps_t   *const cfg);


/**
 * \brief   OTN ODU  get Accepted APS.
 *
 * \note Accepted APS can be received in all MFAS channels simultaniously
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param aps [OUT]     Accepted APS.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_acc_aps_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 vtss_otn_odu_acc_aps_t   *const aps);


/**
 * \brief   OTN ODU Payload Type set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Payload Type configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_pt_set(const vtss_inst_t         inst,
                            const vtss_port_no_t      port_no,
                            const vtss_otn_odu_pt_t   *const cfg);


/**
 * \brief   OTN ODU get Payload Type configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Payload Type configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_pt_get(const vtss_inst_t      inst,
                            const vtss_port_no_t   port_no,
                            vtss_otn_odu_pt_t      *const cfg);


/**
 * \brief   OTN ODU get accepted Payload Type.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param pt [OUT]     Accepted Payload Type.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_acc_pt_get(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_odu_acc_pt_t    *const pt);


/**
 * \brief   OTN ODU OPU  set Over Head Insertion configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Over Head Insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_opu_oh_insertion_set(const vtss_inst_t                 inst,
                                          const vtss_port_no_t              port_no,
                                          const vtss_otn_odu_opu_oh_ins_t   *const cfg);


/**
 * \brief   OTN ODU OPU  get Over Head Insertion configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Over Head Insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_opu_oh_insertion_get(const vtss_inst_t           inst,
                                          const vtss_port_no_t        port_no,
                                          vtss_otn_odu_opu_oh_ins_t   *const cfg);


/**
 * \brief   OTN ODU OPU  Test Payload Insertion configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Test Payload Insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_opu_test_insertion_set(const vtss_inst_t                   inst,
                                            const vtss_port_no_t                port_no,
                                            const vtss_otn_odu_opu_test_ins_t   *const cfg);


/**
 * \brief   OTN ODU OPU  get Test Payload Insertion configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Test Payload Insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_opu_test_insertion_get(const vtss_inst_t             inst,
                                            const vtss_port_no_t          port_no,
                                            vtss_otn_odu_opu_test_ins_t   *const cfg);




/* ================================================================= *
 *  ODUT Config
 * ================================================================= */
/**
 * \brief   OTN ODUT  set Mode configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [IN]      Mode configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_mode_set(const vtss_inst_t              inst,
                               const vtss_port_no_t           port_no,
                               const u32                      level,
                               const vtss_otn_odut_mode_t     *const cfg);


/**
 * \brief   OTN ODUT  get mode configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [OUT]     mode configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_mode_get(const vtss_inst_t      inst,
                               const vtss_port_no_t   port_no,
                               const u32              level,
                               vtss_otn_odut_mode_t   *const cfg);


/**
 * \brief   OTN ODUT TTI set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [IN]      TTI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_tti_set(const vtss_inst_t           inst,
                              const vtss_port_no_t        port_no,
                              const u32                   level,
                              const vtss_otn_odut_tti_t   *const cfg);


/**
 * \brief   OTN ODUT get TTI configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [OUT]     TTI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_tti_get(const vtss_inst_t       inst,
                              const vtss_port_no_t    port_no,
                              const u32               level,
                              vtss_otn_odut_tti_t     *const cfg);


/**
 * \brief   OTN ODUT get accepted TTI.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param tti [OUT]     Accepted TTI.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_acc_tti_get(const vtss_inst_t          inst,
                                  const vtss_port_no_t       port_no,
                                  const u32                  level,
                                  vtss_otn_odut_acc_tti_t    *const tti);


/**
 * \brief   OTN ODUT  set Consequent Action configuration.
 *
 * \note The BDI insertion can be enabled/disable individually per level, but there is only one common configuration of constribution to auto BDI for all levels.
 * The actually constribution to auto BDI on all levels, will be an or of the configured constributions from all levels.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [IN]      Consequent Action configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_consequent_actions_set(const vtss_inst_t                inst,
                                             const vtss_port_no_t             port_no,
                                             const u32                        level,
                                             const vtss_otn_odut_cons_act_t   *const cfg);


/**
 * \brief   OTN ODUT  get Consequent Action configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [OUT]     Consequent Action configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_consequent_actions_get(const vtss_inst_t          inst,
                                             const vtss_port_no_t       port_no,
                                             const u32                  level,
                                             vtss_otn_odut_cons_act_t   *const cfg);


/**
 * \brief   OTN ODUT  set BDI configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [IN]      BDI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_bdi_set(const vtss_inst_t           inst,
                              const vtss_port_no_t        port_no,
                              const u32                   level,
                              const vtss_otn_odut_bdi_t   *const cfg);


/**
 * \brief   OTN ODUT  get BDI configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [OUT]     BDI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_bdi_get(const vtss_inst_t       inst,
                              const vtss_port_no_t    port_no,
                              const u32               level,
                              vtss_otn_odut_bdi_t     *const cfg);


/**
 * \brief   OTN ODUT  set TX STAT configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [IN]      TX STAT configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_tx_stat_set(const vtss_inst_t               inst,
                                  const vtss_port_no_t            port_no,
                                  const u32                       level,
                                  const vtss_otn_odut_tx_stat_t   *const cfg);


/**
 * \brief   OTN ODUT  get TX STAT configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [OUT]     TX STAT configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_tx_stat_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  const u32                 level,
                                  vtss_otn_odut_tx_stat_t   *const cfg);


/**
 * \brief   OTN ODUT  get Accepted STAT.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param stat [OUT]    Accepted STAT.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_acc_stat_get(const vtss_inst_t          inst,
                                   const vtss_port_no_t       port_no,
                                   const u32                  level,
                                   vtss_otn_odut_acc_stat_t   *const stat);


/**
 * \brief   OTN ODUT  set TX APS configuration.
 *
 * \note Transmitting APS is only possible in one MFAS channel at the time. If APS is enabled on more MFAS channels then the prioritizing is like this:
 * ODU - ODUT level 1 - ODUT level 2 - ODUT level 3 - ODUT level 4 - ODUT level 5 - ODUT level 6
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [IN]      TX APS configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_tx_aps_set(const vtss_inst_t              inst,
                                 const vtss_port_no_t           port_no,
                                 const u32                      level,
                                 const vtss_otn_odut_tx_aps_t   *const cfg);


/**
 * \brief   OTN ODUT  get TX APS configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param cfg [OUT]     TX APS configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_tx_aps_get(const vtss_inst_t        inst,
                                 const vtss_port_no_t     port_no,
                                 const u32                level,
                                 vtss_otn_odut_tx_aps_t   *const cfg);


/**
 * \brief   OTN ODUT  get Accepted APS.
 *
 * \note Accepted APS can be received in all MFAS channels simultaniously
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param level [IN]    TC level number. 1-6
 * \param aps [OUT]     Accepted APS.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_acc_aps_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  const u32                 level,
                                  vtss_otn_odut_acc_aps_t   *const aps);



/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief   OTN OCH get defects.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param defects [OUT]   Defect status.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_och_defects_get(const vtss_inst_t       inst,
                                 const vtss_port_no_t    port_no,
                                 vtss_otn_och_defects_t  *const defects);


/**
 * \brief   OTN OTU get defects.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param defects [OUT]   Defect status.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_defects_get(const vtss_inst_t       inst,
                                 const vtss_port_no_t    port_no,
                                 vtss_otn_otu_defects_t  *const defects);


/**
 * \brief   OTN ODU get defects.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param defects [OUT]   Defect status.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_defects_get(const vtss_inst_t       inst,
                                 const vtss_port_no_t    port_no,
                                 vtss_otn_odu_defects_t  *const defects);


/**
 * \brief   OTN ODUT get defects.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param level [IN]      TC level number. 1-6
 * \param defects [OUT]   Defect status.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_defects_get(const vtss_inst_t        inst,
                                  const vtss_port_no_t     port_no,
                                  const u32                level,
                                  vtss_otn_odut_defects_t  *const defects);


/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

/**
 * \brief   OTN OCH get performance.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param perf [OUT]      Performance counters.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_och_perf_get(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_otn_och_perf_t    *const perf);



/**
 * \brief   OTN OTU get performance.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param perf [OUT]      Performance counters.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_otu_perf_get(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_otn_otu_perf_t    *const perf);


/**
 * \brief   OTN ODU get performance.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param perf [OUT]      Performance counters.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odu_perf_get(const vtss_inst_t      inst,
                              const vtss_port_no_t   port_no,
                              vtss_otn_odu_perf_t    *const perf);


/**
 * \brief   OTN ODUT get performance.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param level [IN]      TC level number. 1-6
 * \param perf [OUT]      Performance counters.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_perf_get(const vtss_inst_t       inst,
                               const vtss_port_no_t    port_no,
                               const u32               level,
                               vtss_otn_odut_perf_t    *const perf);



/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
/**
 * \brief OTN OCH interrupt event mask values
 **/
#define VTSS_OTN_OCH_LOS_EV            0x00000001  /**< Loss of Signal */
#define VTSS_OTN_OCH_LOF_EV            0x00000002  /**< Loss of Frame */
#define VTSS_OTN_OCH_LOM_EV            0x00000004  /**< Loss of multi Frame */
#define VTSS_OTN_OCH_AIS_EV            0x00000008  /**< Alarm Idication Signal */
#define VTSS_OTN_OCH_BER_EXCD_EV       0x00000010  /**< Ber exceeded detected by decoder */
#define VTSS_OTN_OCH_UNCRCT_CODE_EV    0x00000020  /**< Uncorrectable codeword detected by fec decoder*/

typedef u32 vtss_otn_och_event_t; /**< Int events: Single event or 'OR' multiple events above */

/**
 * \brief OTN OCH enable/disable event generation for a specific event type or group of events
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param enable  [IN]   Enable or disable events
 * \param ev_mask [IN]   Event type(s) to control (mask)
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_och_event_enable(const vtss_inst_t            inst,
                                  const vtss_port_no_t         port_no,
                                  const BOOL                   enable,
                                  const vtss_otn_och_event_t   ev_mask);

/**
 * \brief OTN OCH event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_och_event_poll(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_och_event_t     *const status);

/**
 * \brief OTN OCH event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected irrespective of the mask register 
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_och_event_poll_without_mask(const vtss_inst_t        inst,
                                             const vtss_port_no_t     port_no,
                                             vtss_otn_och_event_t     *const status);



/**
 * \brief OTN OTU interrupt event mask values
 **/
#define VTSS_OTN_OTU_TIM_EV               0x00000001  /**< Trace Identifyer mismatch */
#define VTSS_OTN_OTU_ACC_TTI_EV           0x00000002  /**< Trace Identifyer accepted */
#define VTSS_OTN_OTU_ICONSIST_TTI_EV      0x00000004  /**< Trace Identifyer inconsistent */
#define VTSS_OTN_OTU_ACC_RES_EV           0x00000008  /**< RES value accepted */
#define VTSS_OTN_OTU_INCONSIST_RES_EV     0x00000010  /**< RES value inconsistent */
#define VTSS_OTN_OTU_ACC_SMRES_EV         0x00000020  /**< SM RES value accepted */
#define VTSS_OTN_OTU_INCONSIST_SMRES_EV   0x00000040  /**< SM RES value inconsistent */
#define VTSS_OTN_OTU_ACC_GCC0_EV          0x00000080  /**< GCC0 value accepted */
#define VTSS_OTN_OTU_INCONSIST_GCC0_EV    0x00000100  /**< GCC0 value inconsistent */
#define VTSS_OTN_OTU_BDI_EV               0x00000200  /**< Backward Defect Indication */
#define VTSS_OTN_OTU_IAE_EV               0x00000400  /**< Incomming Alignment Error */
#define VTSS_OTN_OTU_BIAE_EV              0x00000800  /**< Backward Incomming Alignment Error */

typedef u32 vtss_otn_otu_event_t; /**< Int events: Single event or 'OR' multiple events above */

/**
 * \brief OTN OTU enable/disable event generation for a specific event type or group of events
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param enable  [IN]   Enable or disable events
 * \param ev_mask [IN]   Event type(s) to control (mask)
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_otu_event_enable(const vtss_inst_t            inst,
                                  const vtss_port_no_t         port_no,
                                  const BOOL                   enable,
                                  const vtss_otn_otu_event_t   ev_mask);

/**
 * \brief OTN OTU event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_otu_event_poll(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_otu_event_t     *const status);


/**
 * \brief OTN OTU event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected irrespective of the mask register
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_otu_event_poll_without_mask(const vtss_inst_t        inst,
                                             const vtss_port_no_t     port_no,
                                             vtss_otn_otu_event_t     *const status);


/**
 * \brief OTN ODU interrupt event mask values
 **/
#define VTSS_OTN_ODU_TIM_EV               0x00000001  /**< Trace Identifyer mismatch */
#define VTSS_OTN_ODU_ACC_TTI_EV           0x00000002  /**< Trace Identifyer accepted */
#define VTSS_OTN_ODU_ICONSIST_TTI_EV      0x00000004  /**< Trace Identifyer inconsistent */
#define VTSS_OTN_ODU_PLM_EV               0x00000008  /**< Payload type mismatch */
#define VTSS_OTN_ODU_ACC_PT_EV            0x00000010  /**< Payload type Identifyer accepted */
#define VTSS_OTN_ODU_ICONSIST_PT_EV       0x00000020  /**< Payload type Identifyer inconsistent */
#define VTSS_OTN_ODU_ACC_EXP_EV           0x00000040  /**< EXP accepted */
#define VTSS_OTN_ODU_ICONSIST_EXP_EV      0x00000080  /**< EXP inconsistent */
#define VTSS_OTN_ODU_ACC_RES_EV           0x00000100  /**< RES accepted */
#define VTSS_OTN_ODU_ICONSIST_RES_EV      0x00000200  /**< RES inconsistent */
#define VTSS_OTN_ODU_ACC_FTFL_EV          0x00000400  /**< FTFL accepted */
#define VTSS_OTN_ODU_ICONSIST_FTFL_EV     0x00000800  /**< FTFL inconsistent */
#define VTSS_OTN_ODU_ACC_APS_EV           0x00001000  /**< APS/PCC accepted */
#define VTSS_OTN_ODU_ICONSIST_APS_EV      0x00002000  /**< APS/PCC inconsistent */
#define VTSS_OTN_ODU_OCI_EV               0x00004000  /**< Open Connection Indication */
#define VTSS_OTN_ODU_LCK_EV               0x00008000  /**< Locked defect */
#define VTSS_OTN_ODU_BDI_EV               0x00010000  /**< Backward Defect Indication */
#define VTSS_OTN_ODU_AIS_EV               0x00020000  /**< Alarm Indication Signal */

typedef u32 vtss_otn_odu_event_t; /**< Int events: Single event or 'OR' multiple events above */

/**
 * \brief OTN ODU enable/disable event generation for a specific event type or group of events
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param enable  [IN]   Enable or disable events
 * \param ev_mask [IN]   Event type(s) to control (mask)
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_odu_event_enable(const vtss_inst_t            inst,
                                  const vtss_port_no_t         port_no,
                                  const BOOL                   enable,
                                  const vtss_otn_odu_event_t   ev_mask);

/**
 * \brief OTN ODU event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_odu_event_poll(const vtss_inst_t        inst,
                                const vtss_port_no_t     port_no,
                                vtss_otn_odu_event_t     *const status);


/**
 * \brief OTN ODU event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected irrespective of the mask regster
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_odu_event_poll_without_mask(const vtss_inst_t        inst,
                                             const vtss_port_no_t     port_no,
                                             vtss_otn_odu_event_t     *const status);


/**
 * \brief OTN ODUT interrupt event mask values
 **/
#define VTSS_OTN_ODUT_TIM_EV               0x00000001  /**< Trace Identifyer mismatch */
#define VTSS_OTN_ODUT_ACC_TTI_EV           0x00000002  /**< Trace Identifyer accepted */
#define VTSS_OTN_ODUT_ICONSIST_TTI_EV      0x00000004  /**< Trace Identifyer inconsistent */
#define VTSS_OTN_ODUT_ACC_STAT_EV          0x00000008  /**< STAT accepted */
#define VTSS_OTN_ODUT_ICONSIST_STAT_EV     0x00000010  /**< STAT inconsistent */
#define VTSS_OTN_ODUT_ACC_APS_EV           0x00000020  /**< APS/PCC accepted */
#define VTSS_OTN_ODUT_ICONSIST_APS_EV      0x00000040  /**< APS/PCC inconsistent */
#define VTSS_OTN_ODUT_OCI_EV               0x00000080  /**< Open Connection Indication */
#define VTSS_OTN_ODUT_LCK_EV               0x00000100  /**< Locked defect */
#define VTSS_OTN_ODUT_LTC_EV               0x00000200  /**< Loss of Tandem Connection */
#define VTSS_OTN_ODUT_IAE_EV               0x00000400  /**< Incomming Alignment Error */
#define VTSS_OTN_ODUT_BDI_EV               0x00000800  /**< Backward Defect Indication */
#define VTSS_OTN_ODUT_BIAE_EV              0x00001000  /**< Backward Incomming Alignment Error */
#define VTSS_OTN_ODUT_AIS_EV               0x00002000  /**< Alarm Indication Signal */

typedef u32 vtss_otn_odut_event_t; /**< Int events: Single event or 'OR' multiple events above */

/**
 * \brief OTN ODUT enable/disable event generation for a specific event type or group of events
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param level [IN]     TC level number. 1-6
 * \param enable  [IN]   Enable or disable events
 * \param ev_mask [IN]   Event type(s) to control (mask)
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_odut_event_enable(const vtss_inst_t             inst,
                                   const vtss_port_no_t          port_no,
                                   const u32                     level,
                                   const BOOL                    enable,
                                   const vtss_otn_odut_event_t   ev_mask);

/**
 * \brief OTN ODUT event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param level [IN]     TC level number. 1-6
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected
 *
 * \return Return code.
 **/

vtss_rc vtss_otn_odut_event_poll(const vtss_inst_t         inst,
                                 const vtss_port_no_t      port_no,
                                 const u32                 level,
                                 vtss_otn_odut_event_t     *const status);

/**
 * \brief OTN ODUT event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param level [IN]     TC level number. 1-6
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected 
 *                       irrespective of the Mask register.
 *
 * \return Return code.
 **/
vtss_rc vtss_otn_odut_event_poll_without_mask(const vtss_inst_t         inst,
                                              const vtss_port_no_t      port_no,
                                              const u32                 level,
                                              vtss_otn_odut_event_t     *const status);
#endif /* VTSS_FEATURE_OTN */

#endif /* _VTSS_OTN_API_H_ */
