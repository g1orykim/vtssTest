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
 * \file vtss_gfp_api.h
 * \brief  GFP API
 */

#ifndef _VTSS_GFP_API_H_
#define _VTSS_GFP_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_GFP)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */

/** \brief  GFP Single Error Correction configuration struct */
typedef struct {
    BOOL  core_enable;     /**< Enable/disable core header single error correction */
    BOOL  pay_enable;      /**< Enable/disable payload header single error correction */
    BOOL  trans_enable;    /**< Enable/disable Transparent super block Single error correction */
} vtss_gfp_err_corr_t;


/** \brief  GFP Client Data Frame FCS insert configuration struct */
typedef struct {
    BOOL  enable;     /**< Enable/disable FCS insert */
} vtss_gfp_cdf_fcs_insert_t;


/** \brief  GFP Client Data Frame UPI configuration struct */
/* In fiber channel mode this has no effect. Transmitted and expected value is 0x03 */
typedef struct {
    u32  upi;             /**< Client Data frame Transmitted and expected UPI - Ethernet Packet mode and Ethernet PP/OS mode */
    u32  ordered_upi;     /**< Client Data frame Transmitted and expected Ordered Set UPI - Ethernet PP/OS mode */
} vtss_gfp_cdf_upi_t;


/** \brief  GFP Client Management Frame Signal Fail generation enum */
typedef enum {
    VTSS_GFP_CMF_SF_NONE,          /**< SF CMF generation is disabled */
    VTSS_GFP_CMF_SF_HW,            /**< SF is inserted based on hardware input */
    VTSS_GFP_CMF_SF_FORCED_ON,     /**< SF is inserted based on software - forced on */
    VTSS_GFP_CMF_SF_FORCED_OFF,    /**< SF is inserted based on software - forced off */
    VTSS_GFP_CMF_SF_MAX,
} vtss_gfp_cmf_sf_t;

/** \brief  brief GFP Client Management Frame Auto Signal Fail generation configuration struct */
typedef struct {
    vtss_gfp_cmf_sf_t  los_sf;     /**< Configure LOS SF generation */
    vtss_gfp_cmf_sf_t  lsync_sf;   /**< Configure LSYNC SF generation */
} vtss_gfp_cmf_auto_sf_t;


/** \brief  brief GFP Client Management Frame Force generation configuration struct */
typedef struct {
    BOOL    enable;     /**< Enable/disable forced CMF generation */
    u32     interval;   /**< CMF interval in 100ms. 0 <= valid <= 10 */
    u32     frame_len;  /**< CMF Frame length */
    u32     upi;        /**< CMF UPI value */
    u32     pti;        /**< CMF PTI value */
} vtss_gfp_cmf_forced_t;

/** \brief  GFP 8B/10B transmitted error control code enum */
typedef enum {
    VTSS_GFP_10B_ERR_ERROR_CODE,   /**< 10B_ERR control code */
    VTSS_GFP_K30_7_ERROR_CODE,     /**< K30.7 control code */
    VTSS_GFP_K30_7_MAX,
} vtss_gfp_8b10b_err_code_t;


/** \brief  GFP 8B/10B Error Control Code configuration struct */
typedef struct {
    BOOL  force_los;                        /**< Enable/disable forced 8B10B loss of signal - 10B_ERR is transmitted ???*/
    BOOL  force_lsync;                      /**< Enable/disable forced 8B10B loss of sync - 10B_ERR is transmitted ??? */
    vtss_gfp_8b10b_err_code_t  err_code;    /**< Error Control Code transmitted on disparity error or illegal code word */
} vtss_gfp_8b10b_control_code_t;


/** \brief  GFP defect struct */
typedef struct {
    BOOL  dssf;                /**< GFP SSF defect status */
    BOOL  dcmf_inconsistent;   /**< CMF inconsistent defect status */
    BOOL  dlos;                /**< 8B10B Client Loss Of Signal status */
    BOOL  dlsync;              /**< 8B10B Client Loss Of Sync status */
} vtss_gfp_defects_t;


/** \brief  GFP performance struct */
typedef struct {
    u32  psuper_block_corrected;         /**< Super blocks corrected */
    u32  psuper_block_uncorrected;       /**< Super blocks uncorrected */
    u32  pcore_header_corrected;         /**< Core header corrected */
    u32  ppyld_header_corrected;         /**< Payload header corrected */
    u32  ppyld_header_uncorrected;       /**< Payload header uncorrected */

    u32  ptx_control_frames;             /**< Transmitted control frames */
    u32  ptx_client_management_frames;   /**< Transmitted management frames */
    u32  ptx_client_data_frames;         /**< Transmitted data frames */

    u32  prx_control_frames;             /**< Received control frames */
    u32  prx_client_management_frames;   /**< Received client management frames */
    u32  prx_client_data_frames;         /**< Received client data frames */

    u32  prx_client_invalid_frames;      /**< Received client invalid frames */
} vtss_gfp_perf_t;


/** \brief  GFP internal API state */
typedef struct {
    vtss_gfp_err_corr_t             err_corr;       /**< Single error correction state */
    vtss_gfp_cdf_fcs_insert_t       fcs_insert;     /**< FCS insert state */
    vtss_gfp_cdf_upi_t              cdf_upi;        /**< Client data UPI */
    vtss_gfp_cmf_auto_sf_t          cmf_auto_sf;    /**< Auto Signal fail state */
    vtss_gfp_cmf_forced_t           cmf_forced;     /**< CMF forced state */
    vtss_gfp_8b10b_control_code_t   control_code;   /**< 8B10B control code state */
} vtss_gfp_state_t;



/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief   GFP Single Error Correction set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      correction configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_single_err_corr_set(const vtss_inst_t           inst,
                                     const vtss_port_no_t        port_no,
                                     const vtss_gfp_err_corr_t   *const cfg);


/**
 * \brief   GFP Single Error Correction get configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     correction configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_single_err_corr_get(const vtss_inst_t        inst,
                                     const vtss_port_no_t     port_no,
                                     vtss_gfp_err_corr_t      *const cfg);


/**
 * \brief   GFP Client Management Frame Auto Signal Fail generation set configuration.
 * The automatic SF CMF generation is configured. It can be disable or enabled to insert SF based on HW or SW input
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Signal Fail configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cmf_auto_sf_gen_set(const vtss_inst_t              inst,
                                     const vtss_port_no_t           port_no,
                                     const vtss_gfp_cmf_auto_sf_t   *const cfg);


/**
 * \brief   GFP Client Management Frame Auto Signal Fail generation get configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Signal Fail configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cmf_auto_sf_gen_get(const vtss_inst_t         inst,
                                     const vtss_port_no_t      port_no,
                                     vtss_gfp_cmf_auto_sf_t    *const cfg);


/**
 * \brief   GFP Client Management Frame Forced generation set configuration.
 * The forced CMF generation is configured. It can be disable or enabled to insert any UPI value
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      Forced configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cmf_forced_gen_set(const vtss_inst_t              inst,
                                    const vtss_port_no_t           port_no,
                                    const vtss_gfp_cmf_forced_t    *const cfg);


/**
 * \brief   GFP Client Management Frame Forced generation get configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     Forced configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cmf_forced_gen_get(const vtss_inst_t       inst,
                                    const vtss_port_no_t    port_no,
                                    vtss_gfp_cmf_forced_t   *const cfg);


/**
 * \brief   GFP Accepted Client Management Frame UPI get.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param upi [OUT]     Accepted client management UPI.
 * \param pti [OUT]     Accepted client management PTI.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cmf_acc_upi_pti_get(const vtss_inst_t       inst,
                                 const vtss_port_no_t    port_no,
                                 u32                     *const upi,
                                 u32                     *const pti);


/**
 * \brief   GFP Client Data Frame FCS Insertion set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      FCS insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cdf_fcs_insert_set(const vtss_inst_t                 inst,
                                    const vtss_port_no_t              port_no,
                                    const vtss_gfp_cdf_fcs_insert_t   *const cfg);


/**
 * \brief   GFP Client Data Frame FCS Insertion get configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     FCS insertion configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cdf_fcs_insert_get(const vtss_inst_t            inst,
                                    const vtss_port_no_t         port_no,
                                    vtss_gfp_cdf_fcs_insert_t    *const cfg);


/**
 * \brief   GFP Client Data Frame UPI set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      UPI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cdf_upi_set(const vtss_inst_t           inst,
                             const vtss_port_no_t        port_no,
                             const vtss_gfp_cdf_upi_t    *const cfg);


/**
 * \brief   GFP Client Data Frame UPI get configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     UPI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cdf_upi_get(const vtss_inst_t      inst,
                             const vtss_port_no_t   port_no,
                             vtss_gfp_cdf_upi_t     *const cfg);


/**
 * \brief   GFP Accepted (not matching) Client Data Frame UPI get.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param upi [OUT]     Accepted but not matching UPI.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_cdf_acc_upi_get(const vtss_inst_t       inst,
                                 const vtss_port_no_t    port_no,
                                 u32                     *const upi);


/**
 * \brief   GFP 8B/10B Error Control Code set configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      8B/10B Control Code configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_8b10b_control_code_set(const vtss_inst_t                     inst,
                                        const vtss_port_no_t                  port_no,
                                        const vtss_gfp_8b10b_control_code_t   *const cfg);


/**
 * \brief   GFP GFP 8B/10B Error Control Code get configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     8B/10B Control Code configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_8b10b_control_code_get(const vtss_inst_t                inst,
                                        const vtss_port_no_t             port_no,
                                        vtss_gfp_8b10b_control_code_t    *const cfg);




/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief   GFP get defects.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param defects [OUT]   Defect status.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_defects_get(const vtss_inst_t       inst,
                             const vtss_port_no_t    port_no,
                             vtss_gfp_defects_t      *const defects);




/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

/**
 * \brief   GFP get performance.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port number.
 * \param perf [OUT]      Performance counters.
 *
 * \return Return code.
 **/
vtss_rc vtss_gfp_perf_get(const vtss_inst_t      inst,
                          const vtss_port_no_t   port_no,
                          vtss_gfp_perf_t        *const perf);




/* ================================================================= *
 *  Events
 * ================================================================= */
/**
 * \brief GFP interrupt event mask values
 **/
#define VTSS_GFP_SSF_EV                  0x00000001  /**< GFP Server Signal Fail Event */
#define VTSS_GFP_8B10B_LOS_EV            0x00000002  /**< 8B10B Client Loss Of Signal state change Event */
#define VTSS_GFP_8B10B_LSYNC_EV          0x00000004  /**< 8B10B Client Loss Of Sync state change Event */
#define VTSS_GFP_8B10B_ERROR_EV          0x00000008  /**< 8B10B 10B_ERR codeword state change Event */
#define VTSS_GFP_8B10B_RX_ILLEGAL_EV     0x00000010  /**< 8B10B RX Illegal codeword detected Event */
#define VTSS_GFP_8B10B_RX_DISPARITY_EV   0x00000020  /**< 8B10B RX disparity error detected Event */
#define VTSS_GFP_8B10B_TX_ILLEGAL_EV     0x00000040  /**< 8B10B TX Illegal codeword detected Event */
#define VTSS_GFP_8B10B_TX_DISPARITY_EV   0x00000080  /**< 8B10B TX disparity error detected Event */
#define VTSS_GFP_CMF_LOS_SF_EV           0x00000100  /**< CMF LOS-SF received Event */
#define VTSS_GFP_CMF_LSYNC_SF_EV         0x00000200  /**< CMF LSYNC-SF received Event */
#define VTSS_GFP_CMF_UNSUPPORTED_EV      0x00000400  /**< CMF Unsupported UPI value (NOT 1 or 2) received Event */
#define VTSS_GFP_CMF_INCONSISTENT_EV     0x00000800  /**< CMF inconsistent Event */
#define VTSS_GFP_CDF_UPM_EV              0x00001000  /**< CDF UPI mismatch detected Event */
#define VTSS_GFP_PPOS_CDF_UPM_EV         0x00002000  /**< CDF UPI mismatch detected Event PPOS mode */
#define VTSS_GFP_CMF_ACCEPTED_EV         0x00004000  /**< CMF Accepted Event */
#define VTSS_GFP_FRAME_MAPPED_ETHERNET_EV 0x00008000 /**< CDF Frame Mapped Ethernet Detected Event */

typedef u32 vtss_gfp_event_t; /**< Int events: Single event or 'OR' multiple events above */

/**
 * \brief GFP enable/disable event generation for a specific event type or group of events
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param enable  [IN]   Enable or disable events
 * \param ev_mask [IN]   Event type(s) to control (mask)
 *
 * \return Return code.
 **/

vtss_rc vtss_gfp_event_enable(const vtss_inst_t        inst,
                              const vtss_port_no_t     port_no,
                              const BOOL               enable,
                              const vtss_gfp_event_t   ev_mask);

/**
 * \brief GFP event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected
 *
 * \return Return code.
 **/

vtss_rc vtss_gfp_event_poll(const vtss_inst_t      inst,
                            const vtss_port_no_t   port_no,
                            vtss_gfp_event_t       *const status);


/**
 * \brief GFP event polling function called at by interrupt or periodically
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding event has detected irrespective of the mask register
 *
 * \return Return code.
 **/

vtss_rc vtss_gfp_event_poll_without_mask(const vtss_inst_t      inst,
                                         const vtss_port_no_t   port_no,
                                         vtss_gfp_event_t       *const status);

#endif /* VTSS_FEATURE_GFP */

#endif /* _VTSS_GFP_API_H_ */
