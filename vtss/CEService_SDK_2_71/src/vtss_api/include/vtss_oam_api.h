/*

 Vitesse API software.

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



/**
 * \file
 * \brief OAM API
 * \details This header file describes Y.1731/IEEE802.1ag OAM functions.
 */

#ifndef _VTSS_OAM_API_H_
#define _VTSS_OAM_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_OAM)

/** \page oam Y.1731/IEEE802.1ag OAM
 * 
 * The Vitesse Serval architecture supports a large number of OAM features in
 * hardware, either completely or with CPU assist.
 * 
 * The OAM functionality is logically divided into two main areas:
 * 
 * 1) The Vitesse OAM Processor, the VOP. It is responsible for handling
 *    certain global settings across all hardware OAM instances
 * 2) The Vitesse OAM Engines, VOEs. Each VOE implements functionality for
 *    supporting an OAM MIP or MEP.
 * 
 */

#if defined(VTSS_ARCH_SERVAL)
#define VTSS_OAM_PATH_SERVICE_VOE_CNT   (64)                        /**< Number of path/service VOEs in HW */
#define VTSS_OAM_PORT_VOE_CNT           (11)                        /**< Number of port VOEs in HW */
#define VTSS_OAM_PORT_VOE_BASE_IDX      (64)                        /**< Index of first port VOE */

#define VTSS_OAM_VOE_CNT                ((VTSS_OAM_PATH_SERVICE_VOE_CNT) + (VTSS_OAM_PORT_VOE_CNT))        /**< Total count of port + path/service VOEs */

#define VTSS_OAM_GENERIC_OPCODE_CFG_CNT (7)                         /**< Number of configurable generic opcodes */


#else
#error "Only VTSS_ARCH_SERVAL is supported at this time."
#endif

/** \brief VOE index */
typedef u32 vtss_oam_voe_idx_t;

#define VTSS_OAM_VOE_IDX_NONE 0xFFFFFFFF /**< Special value meaning no VOE */

#define VTSS_OAM_EVENT_MASK_ARRAY_SIZE  ((VTSS_OAM_VOE_CNT+31)/32)  /**< Size in u32's of event mask array used by #vtss_oam_event_poll() */

/** \brief Event mask bits used by #vtss_oam_voe_event_enable() and #vtss_oam_voe_event_poll() */
#define VTSS_OAM_VOE_EVENT_CCM_PERIOD       (1<<7)                  /**< CCM: Period field value has changed */
#define VTSS_OAM_VOE_EVENT_CCM_PRIORITY     (1<<6)                  /**< CCM: Priority field value has changed */
#define VTSS_OAM_VOE_EVENT_CCM_ZERO_PERIOD  (1<<5)                  /**< CCM: Period zero-check result has changed */
#define VTSS_OAM_VOE_EVENT_CCM_RX_RDI       (1<<4)                  /**< CCM: RDI flag value has changed */
#define VTSS_OAM_VOE_EVENT_CCM_LOC          (1<<3)                  /**< CCM: Loss Of Continuity state has changed */
#define VTSS_OAM_VOE_EVENT_CCM_MEP_ID       (1<<2)                  /**< CCM: MEP ID check result has changed */
#define VTSS_OAM_VOE_EVENT_CCM_MEG_ID       (1<<1)                  /**< CCM: MEG ID check result has changed */
#define VTSS_OAM_VOE_EVENT_MEG_LEVEL        (1<<0)                  /**< PDU with unexpected MEG Level was seen */
#define VTSS_OAM_VOE_EVENT_MASK             (VTSS_BITMASK(8))       /**< Mask of configurable bits */



/** \brief MEP type -- Up/Down MEP or MIP
 */
typedef enum {
    VTSS_OAM_DOWNMEP = 0,
    VTSS_OAM_UPMEP,
    VTSS_OAM_MIP
} vtss_oam_mep_type_t;

/** \brief VOE type -- Service/Path or Port
 */
typedef enum {
    VTSS_OAM_VOE_SERVICE = 0,
    VTSS_OAM_VOE_PORT
} vtss_oam_voe_type_t;

/** \brief OAM PDU OpCode values, from Y.1731 table 9-1
 */
typedef enum {
    VTSS_OAM_OPCODE_CCM                 =  1,
    VTSS_OAM_OPCODE_LBM                 =  3,
    VTSS_OAM_OPCODE_LBR                 =  2,
    VTSS_OAM_OPCODE_LTM                 =  5,
    VTSS_OAM_OPCODE_LTR                 =  4,
    VTSS_OAM_OPCODE_AIS                 = 33,
    VTSS_OAM_OPCODE_LCK                 = 35,
    VTSS_OAM_OPCODE_TST                 = 37,
    VTSS_OAM_OPCODE_LINEAR_APS          = 39,
    VTSS_OAM_OPCODE_RING_APS            = 40,
    VTSS_OAM_OPCODE_MCC                 = 41,
    VTSS_OAM_OPCODE_LMM                 = 43,
    VTSS_OAM_OPCODE_LMR                 = 42,
    VTSS_OAM_OPCODE_IDM                 = 45,
    VTSS_OAM_OPCODE_DMM                 = 47,
    VTSS_OAM_OPCODE_DMR                 = 46,
    VTSS_OAM_OPCODE_EXM                 = 49,
    VTSS_OAM_OPCODE_EXR                 = 48,
    VTSS_OAM_OPCODE_VSM                 = 51,
    VTSS_OAM_OPCODE_VSR                 = 50            
} vtss_oam_opcode_value_t;

/** \brief OAM PDU TLV type values, from Y.1731 table 9-2
 */
typedef enum {
    VTSS_OAM_TLV_VALUE_END              =  0,
    VTSS_OAM_TLV_VALUE_DATA             =  3,
    VTSS_OAM_TLV_VALUE_REPLY_INGRESS    =  5,
    VTSS_OAM_TLV_VALUE_REPLY_EGRESS     =  6,
    VTSS_OAM_TLV_VALUE_LTM_EGRESS_ID    =  7,
    VTSS_OAM_TLV_VALUE_LTR_EGRESS_ID    =  8,
    VTSS_OAM_TLV_VALUE_TEST             = 32
} vtss_oam_tlv_value_t;

/** \brief Supported OAM periods for CCM/CCM-LM */
typedef enum {
    VTSS_OAM_PERIOD_INV                 = 0,
    VTSS_OAM_PERIOD_3_3_MS              = 1,
    VTSS_OAM_PERIOD_10_MS               = 2,
    VTSS_OAM_PERIOD_100_MS              = 3,
    VTSS_OAM_PERIOD_1_SEC               = 4
} vtss_oam_period_t;



/* ------------------------------------------------------------------------- */
/* --- VOP - Vitesse OAM Processor ----------------------------------------- */
/* ------------------------------------------------------------------------- */

/* --- Configuration --- */

/** \page oam
    \section VOP - Vitesse OAM Processor
 The VOP configures various global parameters that influence overall OAM PDU
 processing.
**/

/** \brief Configuration for CPU/frontport extraction.
 * Configure if a particular OAM PDU type is extracted to the CPU, or to a
 * front port.
 */
typedef struct {
    BOOL                                to_front;                   /**< TRUE => front port; false: CPU queue */
    vtss_packet_rx_queue_t              rx_queue;                   /**< Index of CPU queue or external port */
} vtss_oam_vop_extract_conf_t;

/** \brief Configuration for generic opcodes.
 * Similar to vtss_oam_vop_extract_conf_t, this structure configures extraction
 * for a generic opcode, and allows for DMAC checking for incoming OAM PDUs
 * with that opcode value.
 */
typedef struct {
    u32                                 opcode;                     /**< Opcode */
    BOOL                                check_dmac;                 /**< TRUE => check DMAC; FALSE => don't */
    vtss_oam_vop_extract_conf_t         extract;                    /**< Extraction settings */
} vtss_oam_vop_generic_opcode_conf_t;

/** \brief Global per-PDU type configuration.
 */
typedef struct {
    vtss_oam_vop_generic_opcode_conf_t  generic[VTSS_OAM_GENERIC_OPCODE_CFG_CNT];  /**< Generic opcodes */
    vtss_oam_vop_extract_conf_t         ccm;                        /**< CCM (not CCM-LM) */
    vtss_oam_vop_extract_conf_t         ccm_lm;                     /**< CCM-LM */
    vtss_oam_vop_extract_conf_t         lt;                         /**< LTM, LTR - common settings */
    vtss_oam_vop_extract_conf_t         dmm;                        /**< DMM */
    vtss_oam_vop_extract_conf_t         dmr;                        /**< DMR */
    vtss_oam_vop_extract_conf_t         lmm;                        /**< LMM */
    vtss_oam_vop_extract_conf_t         lmr;                        /**< LMR */
    vtss_oam_vop_extract_conf_t         lbm;                        /**< LBM */
    vtss_oam_vop_extract_conf_t         lbr;                        /**< LBR */
    vtss_oam_vop_extract_conf_t         err;                        /**< Errored frames */
    vtss_oam_vop_extract_conf_t         other;                      /**< Other frames */
} vtss_oam_vop_pdu_type_conf_t;

/** \brief VOP configuration. Once the VOP is configured, VOEs can be configured.
 */
typedef struct {
    BOOL                                enable_all_voe;             /**< Master enable/disable all VOEs */
    BOOL                                ccm_lm_enable_rx_fcf_in_reserved_field; /**< TRUE => Use reserved field in RX CCM-LM PDUs for RX FCf */
    BOOL                                down_mep_lmr_proprietary_fcf_use;       /**< TRUE => Use proprietary */
    vtss_mac_t                          common_multicast_dmac;      /**< Common MC DMAC for all VOEs */
    u32                                 external_cpu_portmask;      /**< Front port(s) to use for external CPU/FPGA */
    u32                                 sdlb_cpy_copy_idx;          /**< Index of S-DLB to use for OAM CPU frame copy */
    vtss_oam_vop_pdu_type_conf_t        pdu_type;                   /**< Per-PDU type configuration */
} vtss_oam_vop_conf_t;

/** \brief Get VOP configuration.
 * At startup, the VOP is configured with default values and all VOE
 * processing is disabled.
 * \param inst     [IN]  Target instance reference.
 * \param cfg      [OUT] Configuration for the VOP.
 * \return Return code.
 */
vtss_rc vtss_oam_vop_conf_get(const vtss_inst_t          inst,
                              vtss_oam_vop_conf_t *const cfg);

/** \brief Set VOP configuration.
 * \param inst     [IN]  Target instance reference.
 * \param cfg      [OUT] New configuration for the VOP.
 * \return Return code.
 */
vtss_rc vtss_oam_vop_conf_set(const vtss_inst_t                inst,
                              const vtss_oam_vop_conf_t *const cfg);



/* --- Events --- */

/** \brief Event polling across all VOEs.
 */

typedef struct {
    u32    voe_mask[VTSS_OAM_EVENT_MASK_ARRAY_SIZE]; /**< Bit mask for VOEs with pending interrupts. LSB of [0] is VOE index 0. */
} vtss_oam_event_mask_t;

/**
 * \brief VOP polling function called by interrupt or periodically
 *
 * \param inst       [IN]  Target instance reference.
 * \param mask       [OUT] Mask.
 *
 * \return Return code.
 **/
vtss_rc vtss_oam_event_poll(const vtss_inst_t            inst,
                            vtss_oam_event_mask_t *const mask);




/* ------------------------------------------------------------------------- */
/* --- VOE - Vitesse OAM Engine -------------------------------------------- */
/* ------------------------------------------------------------------------- */

/** \page oam
 *  \section VOE - Vitesse OAM Engine\n
 *
 * A VOE is the hardware entity responsible for per-MEP/MIP processing.
 *
 * Before a VOE can be used, it must be allocated. When it is no longer
 * needed, it must be released.
 *
 * Upon allocation a VOE index value is returned; that value must then be used
 * for all subsequent calls to VOE-related functions.
 *
 * Thus, the lifetime for a VOE will, in pseudo code, resemble this:
 *
 *   vtss_oam_voe_idx_t voe_idx;
 *   vtss_oam_voe_alloc_cfg_t data;
 *
 *   vtss_oam_voe_alloc(..., VTSS_OAM_VOE_..., &data, &voe_idx)
 *   vtss_oam_voe_conf_get(..., voe_idx, ...)
 *   vtss_oam_voe_conf_set()
 *     // (operational life of VOE: cfg changes, poll, counter get/clear, etc.)
 *   voe_oam_voe_free(voe_idx)
**/

/* --- Allocation / release of VOE instances --- */

/** \brief Extra data used by #vtss_oam_voe_alloc().
 * Only relevant for port VOEs. */
typedef struct {
    u32                                 phys_port;                  /**< For port-VOE: Physical port index */
} vtss_oam_voe_alloc_cfg_t;

/** \brief Allocate a VOE.
 * \param inst     [IN]  Target instance reference.
 * \param type     [IN]  VOE type: Service or port
 * \param data     [IN]  Extra data for port VOE type. Can be NULL for service VOE type.
 * \param voe_idx  [OUT] Index of allocated VOE instance.
 * \return Return code. Will return an error code if no suitable VOE can be allocated.
 */
vtss_rc vtss_oam_voe_alloc(const vtss_inst_t              inst,
                           const vtss_oam_voe_type_t      type,
                           const vtss_oam_voe_alloc_cfg_t *data,
                           vtss_oam_voe_idx_t             *voe_idx);

/** \brief Free a VOE. If it's still enabled, it will also be disabled.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [OUT] Index of previously allocated VOE instance.
 * \return Return code.
 */
vtss_rc vtss_oam_voe_free(const vtss_inst_t        inst,
                          const vtss_oam_voe_idx_t voe_idx);

/* --- Configuration --- */


/** \brief MEG ID type.
 */
typedef struct {
    u8                                  data[48];                   /**< MEG ID bytes */
} vtss_oam_meg_id_t;

/** \brief LM counter configuration.
 */
typedef struct {
    u32                                 priority_mask;              /**< Per-priority counter enable. Zero-bit means count in prio. 7 */
    BOOL                                yellow;                     /**< TRUE => include yellow frames in count */
} vtss_oam_voe_lm_counter_conf_t;



/** \brief Kind of automatic sequence number update to perform at CCM TX.
 */
typedef enum {
    VTSS_OAM_AUTOSEQ_DONT = 0,
    VTSS_OAM_AUTOSEQ_INCREMENT_AND_UPDATE,
    VTSS_OAM_AUTOSEQ_UPDATE
} vtss_oam_voe_auto_seq_no_conf_t;



/** \brief CCM configuration.
 */
typedef struct {
    BOOL                                enable;                     /**< TRUE => process CMM in hardware */
    BOOL                                copy_to_cpu;                /**< TRUE => send good CCM PDUs to CPU. All or "hitme once" is determined by #vtss_oam_proc_conf_t.copy_next_only */
    BOOL                                forward;                    /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    BOOL                                count_as_data;              /**< TRUE => count frames as data for LM*/
    u32                                 mepid;                      /**< CCM MEP ID */
    vtss_oam_meg_id_t                   megid;                      /**< CCM MEG ID */
    vtss_oam_voe_auto_seq_no_conf_t     tx_seq_no_auto_upd_op;      /**< Type of automatic sequence number update to perform at TX */
    u32                                 tx_seq_no;                  /**< Next sequence number to use at TX */
    BOOL                                rx_seq_no_check;            /**< TRUE => check incoming CCM sequence number against value in rx_seq_no */
    u32                                 rx_seq_no;                  /**< Next sequence number expected at RX */
    u32                                 rx_priority;                /**< Expected CCM priority */
    vtss_oam_period_t                   rx_period;                  /**< Expected CCM period. Also used for Loss Of Connectivity scan */
} vtss_oam_voe_ccm_conf_t;



/** \brief CCM-LM (dual-ended LM) configuration. NOTE that this configuration
 * extends and augments the CCM configuration in struct vtss_oam_voe_ccm_conf_t.
 * Thus, in order to configure CCM-LM, both structs need to be filled in.
 */
typedef struct {
    BOOL                                enable;                     /**< TRUE => process CMM-LM in hardware (requires that CCM is HW-enabled, too) */
    BOOL                                copy_to_cpu;                /**< TRUE => send good CCM-LM PDUs to CPU. All or "hitme once" is determined by #vtss_oam_proc_conf_t.copy_next_only */
    BOOL                                forward;                    /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    vtss_oam_voe_lm_counter_conf_t      count;                      /**< LM counter configuration */
    vtss_oam_period_t                   period;                     /**< Down-MEP only: LM injection period counter. NOTE: Only 100ms and 1sec values are valid */
} vtss_oam_voe_ccm_lm_conf_t;



/** \brief Single-ended LM configuration.
 */
typedef struct {
    BOOL                                enable;                     /**< TRUE => process LM in hardware */
    BOOL                                copy_lmm_to_cpu;            /**< TRUE => send all LMM PDUs to CPU */
    BOOL                                copy_lmr_to_cpu;            /**< TRUE => send all LMR PDUs to CPU */
    BOOL                                forward_lmm;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                forward_lmr;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    BOOL                                count_as_data;              /**< TRUE => count frames as data for LM */
    vtss_oam_voe_lm_counter_conf_t      count;                      /**< LM counter configuration */
} vtss_oam_voe_se_lm_conf_t;



/** \brief LB configuration.
 * Note that the tx_... members are ignored if LB is already active. If they
 * are to be changed, LB must be disabled and re-enabled with the new values.
 * In other words, the values are only applied at enable-time.
 */
typedef struct {
    BOOL                                enable;                     /**< TRUE => process LB in hardware */
    BOOL                                copy_lbm_to_cpu;            /**< TRUE => send all LBM PDUs to CPU */
    BOOL                                copy_lbr_to_cpu;            /**< TRUE => send all LBR PDUs to CPU */
    BOOL                                forward_lbm;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                forward_lbr;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    BOOL                                count_as_data;              /**< TRUE => count frames as data for LM */
    BOOL                                tx_update_transaction_id;   /**< TRUE => update transaction ID on TX -- unless LB is already active */
    u32                                 tx_transaction_id;          /**< LBM Transaction ID to use on TX (if tx_update_transaction_id is TRUE) -- unless LB is already active */
    u32                                 rx_transaction_id;          /**< LBR Transaction ID - 1 to expect on RX -- unless LB is already active */
} vtss_oam_voe_lb_conf_t;



/** \brief TST configuration.
 * Note that the tx_... members are ignored if TST is already active. If they
 * are to be changed, TST must be disabled and re-enabled with the new values.
 * In other words, the values are only applied at enable-time.
 */
typedef struct {
    BOOL                                enable;                     /**< TRUE => process TST in hardware */
#if defined(VTSS_ARCH_SERVAL)
    /** NOTE that for Serval the below fields overload fields from vtss_oam_voe_lb_conf_t
     * A client should therefore *not* configure both TST and LB transaction
     * simultaneously (have enable = TRUE for both).
     **/
#endif
    BOOL                                copy_to_cpu;                /**< TRUE => send all TST PDUs to CPU */
    BOOL                                forward;                    /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    BOOL                                count_as_data;              /**< TRUE => count frames as data for LM */
    BOOL                                tx_seq_no_auto_update;      /**< TRUE => Automatically update TX TST sequence number -- unless TST is already active */
    u32                                 tx_seq_no;                  /**< Sequence number to use for first TST TX -- unless TST is already active */
    u32                                 rx_seq_no;                  /**< Sequence number - 1 to expect -- unless TST is already active */
} vtss_oam_voe_tst_conf_t;



/** \brief One- and Two-way DM (1DM / DMM) configuration.
 */
typedef struct {
    BOOL                                enable_dmm;                 /**< TRUE => process DMM+DMR in hardware */
    BOOL                                enable_1dm;                 /**< TRUE => process 1DM in hardware */
    BOOL                                copy_1dm_to_cpu;            /**< TRUE => send all 1DM PDUs to CPU */
    BOOL                                copy_dmm_to_cpu;            /**< TRUE => send all DMM PDUs to CPU */
    BOOL                                copy_dmr_to_cpu;            /**< TRUE => send all DMR PDUs to CPU */
    BOOL                                forward_1dm;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                forward_dmm;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                forward_dmr;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    BOOL                                count_as_data;              /**< TRUE => count frames as data for LM*/
} vtss_oam_voe_dm_conf_t;



/** \brief LT configuration.
 */
typedef struct {
    BOOL                                enable;                     /**< TRUE => process LB in hardware */
    BOOL                                copy_ltm_to_cpu;            /**< TRUE => send all LTM PDUs to CPU */
    BOOL                                copy_ltr_to_cpu;            /**< TRUE => send all LTR PDUs to CPU */
    BOOL                                forward_ltm;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                forward_ltr;                /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    BOOL                                count_as_data;              /**< TRUE => count frames as data for LM*/
} vtss_oam_voe_lt_conf_t;



/** \brief Generic OAM opcode configuration.
 * 
 * Note that the opcode settings configured here are aligned with those in
 * vtss_oam_vop_conf_t.pdu_type.generic[], i.e. if, say, opcode 40 is configured
 * in index 3 there, the corresponding VOE settings are in index 3 in
 * vtss_oam_voe_conf_t.pdu_type.generic[] as well.
 */
typedef struct {
    BOOL                                enable;                     /**< TRUE => process opcode in hardware */
    BOOL                                copy_to_cpu;                /**< TRUE => send all PDUs with the specified opcode to CPU */
    BOOL                                forward;                    /**< TRUE => forward all OAM frames of this type to the extraction destination */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    BOOL                                count_as_data;              /**< TRUE => count frames as data for LM*/
} vtss_oam_voe_generic_conf_t;



/** \brief Unknown opcode configuration.
 * An unknown opcode is one that isn't a) explicitly supported (e.g. CCM or LTM)
 * or b) configured as a generic opcode.
 */
typedef struct {
    BOOL                                enable;                     /**< TRUE => process opcode in hardware */
    BOOL                                copy_to_cpu;                /**< TRUE => send all PDUs with unknown opcodes to CPU */
    BOOL                                count_as_selected;          /**< TRUE => count frames as selected */
    BOOL                                count_as_data;              /**< TRUE => count frames as data for LM*/
} vtss_oam_voe_unknown_conf_t;



/** \brief Up-MEP configuration.
 */
typedef struct {
    BOOL                                discard_rx;                 /**< TRUE => all non-OAM RX data is discarded */
    BOOL                                loopback;                   /**< TRUE => loop OAM mode; FALSE => in-service mode */
    u32                                 port;                       /**< Front port where up-MEP is located */
} vtss_oam_voe_up_mep_conf_t;



/** \brief Kind of DMAC check to perform on incoming OAM PDUs
 */
typedef enum {
    VTSS_OAM_DMAC_CHECK_UNICAST = 0,
    VTSS_OAM_DMAC_CHECK_MULTICAST,
    VTSS_OAM_DMAC_CHECK_BOTH,
    VTSS_OAM_DMAC_CHECK_NONE
} vtss_oam_dmac_check_t;



/** \brief PDU processing checks.
 */
typedef struct {
    u32                                 meg_level;                  /**< MEG Level (MEL) */
    vtss_oam_dmac_check_t               dmac_check_type;            /**< Kind of DMAC check to perform */
    BOOL                                ccm_check_only;             /**< TRUE => Only check CCM frames for valid MEL/valid DMAC/version==0; FALSE => check all OAM frames */

    BOOL                                copy_next_only;             /**< TRUE => For most of the following 'copy_on_...' flags: Copy next PDU only ("hit me once"); FALSE => copy all */
    
    BOOL                                copy_on_ccm_err;            /**< TRUE => Copy to CPU if CCM validation fails (honors #copy_next_only) */
    BOOL                                copy_on_mel_too_low_err;    /**< TRUE => Copy to CPU on MEG Level error (honors #copy_next_only) */

#if defined(VTSS_ARCH_SERVAL)
    BOOL                                copy_on_ccm_more_than_one_tlv;  /**< TRUE => Copy to CPU when CCM PDU with more than one TLV arrives (Serval: Only used when #copy_next_only == TRUE) */
    BOOL                                copy_on_dmac_err;           /**< TRUE => Copy to CPU on DMAC error (no "hit me once") */
#else
    BOOL                                copy_on_version_err;        /**< TRUE => Copy to CPU on version error (no "hit me once") */
#endif    
} vtss_oam_proc_conf_t;



/** \brief Main VOE configuration structure.
 */
typedef struct {
    vtss_oam_voe_type_t                 voe_type;                   /**< Read-Only: Service/Path or Port VOE type */
    vtss_mac_t                          unicast_mac;                /**< This VOE's unicast MAC */

    vtss_oam_mep_type_t                 mep_type;                   /**< Up-MEP, Down-MEP, or MIP */

    BOOL                                svc_to_path;                /**< TRUE => count Service frames on Path VOE */
    u32                                 svc_to_path_idx_w;          /**< Service VOE only: Index of associated Path VOE, working */
    u32                                 svc_to_path_idx_p;          /**< Service VOE only: Index of associated Path VOE, protected */
    
    u32                                 loop_isdx_w;                /**< Loop (LB, DM) Service index, working */
    u32                                 loop_isdx_p;                /**< Loop (LB, DM) Service index, protected */
    u32                                 loop_portidx_p;             /**< Loop (LB, DM) port index, protected */

    BOOL                                sdlb_enable;                /**< TRUE => Use another S-DLB (sdlb_idx) for OAM frames counted as such (not as data) */
    u32                                 sdlb_idx;                   /**< Index of S-DLB to use */
    
    vtss_oam_proc_conf_t                proc;                       /**< Overall PDU processing setup */
    
    /* Per-OAM type configuration */
    vtss_oam_voe_generic_conf_t         generic[VTSS_OAM_GENERIC_OPCODE_CFG_CNT];  /**< Generic opcodes */
    vtss_oam_voe_unknown_conf_t         unknown;                    /**< Unknown opcodes */
    vtss_oam_voe_ccm_conf_t             ccm;                        /**< CCM */
    vtss_oam_voe_ccm_lm_conf_t          ccm_lm;                     /**< CCM-LM */
    vtss_oam_voe_se_lm_conf_t           single_ended_lm;            /**< Single-ended LM */
    vtss_oam_voe_lb_conf_t              lb;                         /**< LB */
    vtss_oam_voe_tst_conf_t             tst;                        /**< TST */
    vtss_oam_voe_dm_conf_t              dm;                         /**< DM */
    vtss_oam_voe_lt_conf_t              lt;                         /**< LT */

    /* MEP-type specific configuration */
    vtss_oam_voe_up_mep_conf_t          upmep;                      /**< Up-MEP */
} vtss_oam_voe_conf_t;

/** \brief Get VOE configuration.
 * At startup, each VOE is configured with default values and is disabled.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE configuration to get.
 * \param cfg      [OUT] Configuration for the VOE.
 * \return Return code.
 */
vtss_rc vtss_oam_voe_conf_get(const vtss_inst_t          inst,
                              const vtss_oam_voe_idx_t   voe_idx,
                              vtss_oam_voe_conf_t *const cfg);

/** \brief Set VOE configuration.
 * Note that RDI and hitme configuration must be (re-)applied after
 * a call to this function.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE configuration to get.
 * \param cfg      [OUT] New configuration for the VOE.
 * \return Return code.
 */
vtss_rc vtss_oam_voe_conf_set(const vtss_inst_t                inst,
                              const vtss_oam_voe_idx_t         voe_idx,
                              const vtss_oam_voe_conf_t *const cfg);



/* --- Events --- */

/** \brief Event generation flags.
 * It is possible to generate events (interrupts) based on specific kinds of
 * OAM PDU processing.
 */
typedef u32 vtss_oam_voe_event_mask_t;

/**
 * \brief Enable/disable VOE event generation.
 * \param inst     [IN] Target instance reference.
 * \param voe_idx  [IN] Index of VOE to configure.
 * \param mask     [IN] Mask of events to either enable or disable.
 * \param enable   [IN] Whether to enable or disable the events specified in the mask.
 * \return Return code.
 **/
vtss_rc vtss_oam_voe_event_enable(const vtss_inst_t               inst,
                                  const vtss_oam_voe_idx_t        voe_idx,
                                  const vtss_oam_voe_event_mask_t mask,
                                  const BOOL                      enable);

/**
 * \brief VOE event polling.
 * \param inst      [IN]  Target instance reference.
 * \param voe_idx   [IN]  Index of VOE to configure.
 * \param mask      [OUT] Mask of occurred events.
 * \return Return code.
 **/
vtss_rc vtss_oam_voe_event_poll(const vtss_inst_t                inst,
                                const vtss_oam_voe_idx_t         voe_idx,
                                vtss_oam_voe_event_mask_t *const mask);



/* --- Dynamic configuration --- */

/** \brief Arm "hitme" copy-to-CPU processing. The flags reset to zero
 * after each occurrence, so software needs to re-arm after each event.
 * \param inst     [IN] Target instance reference.
 * \param voe_idx  [IN] Index of VOE to configure.
 * \param enable   [IN] TRUE => arm
 * \return Return code.
 */
vtss_rc vtss_oam_voe_ccm_arm_hitme(const vtss_inst_t        inst,
                                   const vtss_oam_voe_idx_t voe_idx,
                                   BOOL                     enable);



/** \brief Configure RDI flag for a CCM-enabled VOE.
 * \param inst     [IN] Target instance reference.
 * \param voe_idx  [IN] Index of VOE to configure.
 * \param flag     [IN] RDI flag. TRUE => 1; FALSE => 0
 * \return Return code.
 */
vtss_rc vtss_oam_voe_ccm_set_rdi_flag(const vtss_inst_t        inst,
                                      const vtss_oam_voe_idx_t voe_idx,
                                      BOOL                     flag);



/* --- Status --- */

/** \brief Status for most recently processed RX CCM PDU.
 */
typedef struct {
    BOOL                                period_err;                 /**< Status for last RX frame */
    BOOL                                priority_err;               /**< Status for last RX frame */
    BOOL                                zero_period_err;            /**< Status for last RX frame */
    BOOL                                rx_rdi;                     /**< Status for last RX frame */
    BOOL                                loc;                        /**< Status for last RX frame */
    BOOL                                mep_id_err;                 /**< Status for last RX frame */
    BOOL                                meg_id_err;                 /**< Status for last RX frame */
} vtss_oam_ccm_status_t;

/** \brief Read CCM status from a VOE.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE to read from.
 * \param value    [OUT] Status values.
 * \return Return code.
 */
vtss_rc vtss_oam_ccm_status_get(const vtss_inst_t        inst,
                                const vtss_oam_voe_idx_t voe_idx,
                                vtss_oam_ccm_status_t    *value);



/** \brief Indicate whether one or more PDUs of various types has been seen since last check.
 * A PDU must pass MEG Level and DMAC checks before being indicated here.
 * The values are cleared after reading.
 */
typedef struct {
    BOOL                                generic_seen[VTSS_OAM_GENERIC_OPCODE_CFG_CNT];    /**< Generic opcodes */
    BOOL                                unknown_seen;               /**< Unknown opcode */
    BOOL                                ltm_seen;                   /**< LTM */
    BOOL                                ltr_seen;                   /**< LTR */
    BOOL                                lmm_seen;                   /**< LMM */
    BOOL                                lmr_seen;                   /**< LMR */
    BOOL                                lbm_seen;                   /**< LBM */
    BOOL                                lbr_seen;                   /**< LBR */
    BOOL                                dmm_seen;                   /**< DMM */
    BOOL                                dmr_seen;                   /**< DMR */
    BOOL                                one_dm_seen;                /**< 1DM */
    BOOL                                ccm_seen;                   /**< CCM, including CCM-LM */
    BOOL                                ccm_lm_seen;                /**< CCM-LM (only) */
} vtss_oam_pdu_seen_status_t;

/** \brief Read PDU-seen status from a VOE.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE to read from.
 * \param status   [OUT] Status values.
 * \return Return code.
 */
vtss_rc vtss_oam_pdu_seen_status_get(const vtss_inst_t          inst,
                                     const vtss_oam_voe_idx_t   voe_idx,
                                     vtss_oam_pdu_seen_status_t *status);



/** \brief Processing status: change indications and status values for most 
 * recently processed PDU(s).
 * ..._err: Status for the most recently processed PDU.
 * ..._seen: This has happened at least once since last poll. Cleared after reading.
 */
typedef struct {
#if defined(VTSS_ARCH_SERVAL)
    u32                                 rx_ccm_seq_no;              /**< Most recently received CCM/CCM-LM sequence number. Note: For Serval-1 and up-MEP this value only contains the lower 16 bits of the sequence number. */
#else
    u32                                 rx_ccm_seq_no;              /**< Most recently received CCM/CCM-LM sequence number */
#endif

    u32                                 tx_next_ccm_seq_no;         /**< Next CCM/CCM-LM sequence number to be used (the value is updated by HW upon TX, if so enabled) */

#if defined(VTSS_ARCH_SERVAL)
    /** NOTE that for Serval rx_tst_seq_no overlays rx_lbr_transaction_id due
     * to HW register re-use. But logically they're distinct and future HW is
     * expected to reflect this.
     **/
    u32                                 rx_lbr_transaction_id;      /**< Most recently received LBR transaction ID (NOTE: On Serval, overlays #rx_tst_seq_no) */
    u32                                 rx_tst_seq_no;              /**< Most recently received TST sequence number (NOTE: On Serval, overlays #rx_lbr_transaction_id) */
#else
    u32                                 rx_lbr_transaction_id;      /**< Most recently received LBR transaction ID */
    u32                                 rx_tst_seq_no;              /**< Most recently received TST sequence number */
#endif

    u32                                 tx_next_lbm_transaction_id; /**< Next LBM transaction ID to be used (the value is updated by HW upon TX, if so enabled) */

    BOOL                                rx_meg_level_err;           /**< Status for most recently RX'd frame */

    BOOL                                rx_meg_level_err_seen;      /**< TRUE => MEG Level (MEL) test failed */
    BOOL                                rx_dmac_err_seen;           /**< TRUE => DMAC test failed */
    BOOL                                tx_meg_level_err_seen;      /**< TRUE => TX MEG Level test failed */
} vtss_oam_proc_status_t;

/** \brief Read PDU processing status from a VOE.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE to read from.
 * \param value    [OUT] Status values.
 * \return Return code.
 */
vtss_rc vtss_oam_proc_status_get(const vtss_inst_t        inst,
                                 const vtss_oam_voe_idx_t voe_idx,
                                 vtss_oam_proc_status_t   *value);



/* --- Counters --- */

/** Some counters can be cleared, others not. If a counter cannot be cleared,
 * by calling #vtss_oam_voe_counter_clear() this is indicated in the comments
 * for the counter.
 */

/** \brief Simple RX/TX counter structure. */
typedef struct {
    u64                                 rx;                         /**< RX count */
    u64                                 tx;                         /**< TX count */
} vtss_oam_rx_tx_counter_t;

/** \brief Up-/Down-MEP LM counters and per-priority RX/TX LM counters. */
typedef struct {
    struct {
        u32                             tx_FCf;                     /**< CCM-LM: Latest received FCf. Cannot be cleared. */
        u32                             rx_FCl;                     /**< CCM-LM: Latest sampled CCM-LM RX frame counter. Cannot be cleared. */
    } down_mep;                                                     /**< Down-MEP specific */
    struct {
#if defined(VTSS_ARCH_SERVAL)
        /** When LM values are sampled by Serval they cannot be provided
        * as updates to the received PDU. Instead the most recent values are
        * stored in the VOE and a sample sequence number is updated and sent to
        * the CPU as PDU sideband data. It is then up to the application code to
        * match an extracted LM PDU with the values in the VOE based on
        * comparing the sample sequence number values from the sideband data and
        * the appropriate field here.
        * 
        * Future HW will provide simpler functionality.
        **/
        u32                             rx_lmm_sample_seq_no;       /**< Serval: 5-bit LMR sample sequence number (in-service only). Cannot be cleared. */
        u32                             rx_lmr_sample_seq_no;       /**< Serval: 5-bit LMM sample sequence number. Cannot be cleared. */
        u32                             rx_ccm_lm_sample_seq_no;    /**< Serval: 5-bit CCM-LM sample sequence number. Cannot be cleared. */
#endif
        u32                             lmm;                        /**< LMM: Latest sampled LM RX frame counter. Cannot be cleared. */
        u32                             lmr;                        /**< LMR: Latest sampled LM RX frame counter. Cannot be cleared. */
        u32                             ccm_lm;                     /**< CCM-LM: Latest sampled LM RX frame counter. Cannot be cleared. */

        BOOL                            lmm_valid;                  /**< TRUE => value in lmm is valid. Cannot be cleared. */
        BOOL                            lmr_valid;                  /**< TRUE => value in lmr is valid. Cannot be cleared. */
        BOOL                            ccm_lm_valid;               /**< TRUE => value in ccm_lm is valid. Cannot be cleared. */

        BOOL                            lmm_sample_lost;            /**< TRUE => one or more lmm sample values have been lost since last poll. Cannot be cleared. */
        BOOL                            lmr_sample_lost;            /**< TRUE => one or more lmr sample values have been lost since last poll. Cannot be cleared. */
        BOOL                            ccm_lm_sample_lost;         /**< TRUE => one or more ccm-lm sample values have been lost since last poll. Cannot be cleared. */
    } up_mep;                                                       /**< Up-MEP specific */
    vtss_oam_rx_tx_counter_t            lm_count[VTSS_PRIO_ARRAY_SIZE]; /**< One counter-set per priority */
} vtss_oam_lm_counter_t;

/** \brief CCM counters */
typedef struct {
    u64                                 rx_valid_count;             /**< Count of valid RX CCM PDUs */
    u64                                 rx_invalid_count;           /**< Count of invalid RX CCM PDUs */
#if defined(VTSS_ARCH_SERVAL)
    /* For Serval this counter is inexact. The only semantic it supports is a
     * simple "at least one more invalid sequence number was seen since last
     * poll" indication if the value has changed.
     * 
     * Serval HW only has a 1-bit indication ("1-bit saturating counter"), but
     * future devices will have a counter with more bits.
     */
    u64                                 rx_invalid_seq_no;          /**< Count of RX CCM PDUs with invalid sequence number. NOTE: Inexact for Serval; do not rely on precise value. */
#else
    u64                                 rx_invalid_seq_no;          /**< Count of RX CCM PDUs with invalid sequence number */
#endif
} vtss_oam_ccm_counter_t;

/** \brief RX/TX OAM PDU counters. By default, all OAM PDU types are counted
 * as 'non-selected', but by setting the appropriate count_as_selected field
 * in the configuration structures, a PDU type can be "moved" to the 'selected'
 * counter.
 **/
typedef struct {
    vtss_oam_rx_tx_counter_t            selected_frames;            /**< Count of selected OAM frames */
    vtss_oam_rx_tx_counter_t            nonselected_frames;         /**< Count of non-selected OAM frames */
} vtss_oam_sel_counter_t;

/** \brief LB counter */
typedef struct {
    u64                                 rx_lbr;                     /**< Count of RX LBR PDUs */
    u64                                 tx_lbm;                     /**< Count of TX LBM PDUs */
#if defined(VTSS_ARCH_SERVAL)
    /* For Serval this counter is inexact. The only semantic it supports is a
     * simple "at least one more invalid sequence number was seen since last
     * poll" indication if the value has changed.
     *
     * Serval HW only has a 1-bit indication ("1-bit saturating counter"), but
     * future devices will have a counter with more bits.
     */
    u64                                 rx_lbr_trans_id_err;        /**< Count of RX'd LBR PDUs with invalid transaction ID. NOTE: Inexact for Serval; do not rely on precise value. */
#else
    u64                                 rx_lbr_trans_id_err;        /**< Count of RX'd LBR PDUs with invalid transaction ID */
#endif
} vtss_oam_lb_counter_t;

/** \brief TST counter */
typedef struct {
    u64                                 rx_tst;                     /**< Count of RX TST PDUs */
    u64                                 tx_tst;                     /**< Count of TX TST PDUs */
    u64                                 rx_tst_trans_id_err;        /**< Count of RX'd TST PDUs with invalid transaction ID */
} vtss_oam_tst_counter_t;

/** \brief Per-VOE counters */
typedef struct {
    vtss_oam_ccm_counter_t              ccm;                        /**< CCM counters */
    vtss_oam_lm_counter_t               lm;                         /**< LM counters */
    vtss_oam_lb_counter_t               lb;                         /**< LB counters */
    vtss_oam_tst_counter_t              tst;                        /**< TST counters */
    vtss_oam_sel_counter_t              sel;                        /**< Selected/non-selected counters */
} vtss_oam_voe_counter_t;

/** \brief Update counters for a VOE.
 * This function must be called at regular intervals in order to ensure that
 * HW counters are accumulated before they get a chance to wrap around.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE.
 * \return Return code
 */
vtss_rc vtss_oam_voe_counter_update(const vtss_inst_t        inst,
                                    const vtss_oam_voe_idx_t voe_idx);

#if defined(VTSS_ARCH_SERVAL)
/** \brief Update counters for a VOE.
 * This is a utility function to quickly poll and update the "1-bit counters"
 * accumulated in vtss_oam_lb_counter_t.rx_lbr_trans_id_err and
 * vtss_oam_ccm_counter_t.rx_invalid_seq_no.
 *
 * This can improve (but not guarantee) the precision of the counters.
 *
 * The function is ONLY provided for Serval-1.
 *
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE.
 * \return Return code
 */
vtss_rc vtss_oam_voe_counter_update_serval(const vtss_inst_t        inst,
                                           const vtss_oam_voe_idx_t voe_idx);
#endif

/** \brief Read counters from a VOE. Will call #vtss_oam_voe_counter_update() first.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE.
 * \param value    [OUT] Counter values.
 * \return Return code.
 */
vtss_rc vtss_oam_voe_counter_get(const vtss_inst_t        inst,
                                 const vtss_oam_voe_idx_t voe_idx,
                                 vtss_oam_voe_counter_t   *value);

/** \brief Mask values for clearing specific counters */
#define VTSS_OAM_CNT_CCM                (1<<0)                      /**< CCM counters */
#define VTSS_OAM_CNT_LM                 (1<<1)                      /**< LM counters */
#define VTSS_OAM_CNT_LB                 (1<<2)                      /**< LB counters */
#define VTSS_OAM_CNT_TST                (1<<3)                      /**< TST counters */
#define VTSS_OAM_CNT_SEL                (1<<4)                      /**< Selected/non-selected counters */
#define VTSS_OAM_CNT_ALL                (VTSS_BITMASK(5))           /**< All counters */

/** \brief Clear counters in a VOE.
 * \param inst     [IN]  Target instance reference.
 * \param voe_idx  [IN]  Index of VOE.
 * \param mask     [IN]  Mask of counters to clear, values taken from VTSS_OAM_CNT_...
 * \return Return code.
 */
vtss_rc vtss_oam_voe_counter_clear(const vtss_inst_t        inst,
                                   const vtss_oam_voe_idx_t voe_idx,
                                   const u32                mask);


/* ------------------------------------------------------------------------- */
/* --- IPT OAM functionality ----------------------------------------------- */
/* ------------------------------------------------------------------------- */

/** \brief OAM-related configuration for the IPT table. */
typedef struct {
    BOOL                                enable;                     /**< TRUE => Use this MEP index */
    vtss_oam_voe_idx_t                  service_voe_idx;            /**< Associated (Service) VOE */
} vtss_oam_ipt_conf_t;

/** \brief Get IPT OAM configuration for a service index.
 * \param inst     [IN]  Target instance reference.
 * \param isdx     [IN]  Service index.
 * \param cfg      [OUT] IPT configuration.
 * \return Return code.
 */
vtss_rc vtss_oam_ipt_conf_get(const vtss_inst_t   inst,
                              const u32           isdx,
                              vtss_oam_ipt_conf_t *cfg);

/** \brief Set IPT OAM configuration for a service index.
 * \param inst     [IN]  Target instance reference.
 * \param isdx     [IN]  Service index.
 * \param cfg      [IN]  IPT configuration.
 * \return Return code.
 */
vtss_rc vtss_oam_ipt_conf_set(const vtss_inst_t                inst,
                              const u32                        isdx,
                              const vtss_oam_ipt_conf_t *const cfg);




#endif /* VTSS_FEATURE_OAM */

#endif /* _VTSS_OAM_API_H_ */
