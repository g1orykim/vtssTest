/*

 Vitesse API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

/**
 * \file
 * \brief 10G PHY API
 * \details This header file describes 10G PHY control functions
 */

#ifndef _VTSS_PHY_10G_API_H_
#define _VTSS_PHY_10G_API_H_

#include <vtss_options.h>
#include <vtss_types.h>
#include <vtss_misc_api.h>

#ifdef VTSS_CHIP_10G_PHY

/** \brief 10G Phy operating mode enum type */
typedef enum {
        VTSS_PHY_LAN_MODE,          /**< LAN mode: Single clock (XREFCK=156,25 MHz), no recovered clock output  */
        VTSS_PHY_WAN_MODE,          /**< WAN mode:\n */
                                    /**< 848X:   Dual clock (XREFCK=156,25 MHz, WREFCK=155,52 MHz), no recovered clock output\n */
                                    /**< Venice: Single clock (XREFCK), no recovered clock output\n */
        VTSS_PHY_1G_MODE,           /**< 8488:   1G pass-through mode\n */
                                    /**< Venice: 1G mode, Single clock (XREFCK=156,25 MHz), no recovered clock output */
#if defined(VTSS_FEATURE_SYNCE_10G)
        VTSS_PHY_LAN_SYNCE_MODE,    /**< LAN SyncE:\n */
                                    /**< if hl_clk_synth == 1:\n */
                                    /**< 8488:   Single clock (XREFCK=156,25 MHz), recovered clock output enabled\n */
                                    /**< Venice: Single clock (XREFCK=156,25 MHz), recovered clock output enabled\n */
                                    /**< if hl_clk_synth == 0:\n */
                                    /**< 8488:   Dual clock (XREFCK=156,25 MHz, SREFCK=156,25 MHz), recovered clock output enabled\n */
                                    /**< Venice: Dual clock (XREFCK=156,25 MHz, SREFCK=156,25 MHz), recovered clock output enabled\n */
        VTSS_PHY_WAN_SYNCE_MODE,    /**< WAN SyncE:\n */
                                    /**< if hl_clk_synth == 1:\n */
                                    /**< 8488:   Single clock (WREFCK=155,52 MHz or 622,08 MHz), recovered clock output enabled\n */
                                    /**< Venice: Single clock (XREFCK=156,25 MHz), recovered clock output enabled\n */
                                    /**< if hl_clk_synth == 0:\n */
                                    /**< 8488:   Dual clock (WREFCK=155,52 MHz or 622,08 MHz, SREFCK=155,52 MHz), recovered clock output enabled\n */
                                    /**< Venice: Dual clock (XREFCK=156,25 MHz, SREFCK=155,52 MHz), recovered clock output enabled\n */
        VTSS_PHY_LAN_MIXED_SYNCE_MODE, /**< 8488:   Channels are in different modes, channel being configured is in LAN\n */
                                       /**< Venice: Same as VTSS_PHY_LAN_SYNCE_MODE */
        VTSS_PHY_WAN_MIXED_SYNCE_MODE, /**< 8488:   Channels are in different modes, channel being configured is in WAN\n */
                                       /**< Venice: Same as VTSS_PHY_WAN_SYNCE_MODE */
#endif /* VTSS_FEATURE_SYNCE_10G */
    } oper_mode_t;

/** \brief 10G Phy operating mode */
typedef struct {
    oper_mode_t oper_mode;                    /**< Phy operational mode */

    enum {
        VTSS_PHY_XAUI_XFI,          /**< XAUI  <-> XFI - Interface mode. */
        VTSS_PHY_XGMII_XFI,         /**< XGMII <-> XFI - Interface mode. Only for VSC8486 */
        VTSS_PHY_RXAUI_XFI,         /**< RXAUI <-> XFI - Interface mode. Only for Venice */
        VTSS_PHY_SGMII_LANE_0_XFI,  /**< SGMII <-> XFI - LANE 0. Only for Venice */
        VTSS_PHY_SGMII_LANE_3_XFI,  /**< SGMII <-> XFI - LANE 3. Only for Venice */
    } interface;             /**< Interface mode. */

    enum {
        VTSS_WREFCLK_155_52, /**< WREFCLK = 155.52Mhz - WAN ref clock */
        VTSS_WREFCLK_622_08  /**< WREFCLK = 622.08Mhz - WAN ref clock */
    } wrefclk;               /**< 848X only: WAN ref clock */

    BOOL  high_input_gain;   /**< Disable=0 (default), Enable=1. Should not be enabled unless needed */
    BOOL  xfi_pol_invert;    /**< Selects polarity ot the TX XFI data. 1:Invert 0:Normal */
    BOOL  xaui_lane_flip;     /**< Swaps lane 0 <--> 3 and 1 <--> 2 for both RX and TX */

    enum {
        VTSS_CHANNEL_AUTO,   /**< Automatically detects the channel id based on the phy order.  
                                The phys be setup in the consecutive order, from the lowest MDIO to highest MDIO address */
        VTSS_CHANNEL_0,      /**< Channel id is hardcoded to 0  */
        VTSS_CHANNEL_1,      /**< Channel id is hardcoded to 1  */
        VTSS_CHANNEL_2,      /**< Channel id is hardcoded to 2  */
        VTSS_CHANNEL_3,      /**< Channel id is hardcoded to 3  */
    } channel_id;            /**< Channel id of this instance of the Phy  */

#if defined(VTSS_FEATURE_SYNCE_10G)
    BOOL hl_clk_synth;       /**< 0: Free running clock 
                                  1: Hitless clock */
    enum {
      VTSS_RECVRD_RXCLKOUT,  /**< RXCLKOUT is used for recovered clock */
      VTSS_RECVRD_TXCLKOUT,  /**< TXCLKOUT is used for recovered clock */
    } rcvrd_clk;              /**< RXCLKOUT/TXCLKOUT used as recovered clock */
                              /**< (not used any more, instead use the api functions: */
                              /**< vtss_phy_10g_rxckout_set and vtss_phy_10g_txckout_set */
    
    enum {
      VTSS_RECVRDCLK_CDR_DIV_64, /**< recovered clock is /64 */
      VTSS_RECVRDCLK_CDR_DIV_66, /**< recovered clock is /66 */
    } rcvrd_clk_div;              /**< 8488 only: recovered clock's divisor */
    
    enum {
     VTSS_SREFCLK_DIV_64,    /**< SREFCLK/64  */
     VTSS_SREFCLK_DIV_66,    /**< SREFCLK/64  */
     VTSS_SREFCLK_DIV_16,    /**< SREFCLK/16 */ 
    } sref_clk_div;           /**< 8488 only: SRERCLK divisor */

    enum {
     VTSS_WREFCLK_NONE,      /**< NA */
     VTSS_WREFCLK_DIV_16,    /**< WREFCLK/16 */
    } wref_clk_div;           /**< 8488 only: WREFCLK divisor */
#endif /* VTSS_FEATURE_SYNCE_10G */

#if defined(VTSS_FEATURE_EDC_FW_LOAD)
    enum {
     VTSS_EDC_FW_LOAD_MDIO,    /**< Load EDC FW through MDIO to iCPU */
     VTSS_EDC_FW_LOAD_NOTHING, /**< Do not load FW to iCPU */
    } edc_fw_load;             /**< EDC Firmware load */
#endif /* VTSS_FEATURE_EDC_FW_LOAD */

    struct {
        BOOL use_conf;        /**< Use this configuration instead of default  (only for Venice family) */
        u32 d_filter;         /**< SD10G Transmit filter coefficients for FIR taps (default 0x7DF820)  */
        u32 ib_ini_lp;        /**< SD6G Init force value for low-pass gain regulation (default 1 )     */
        u32 ib_min_lp;        /**< SD6G Min value for low-pass gain regulation (default 0)             */
        u32 ib_max_lp;        /**< SD6G Max value for low-pass gain regulation (default 63)            */
    } serdes_conf;            /**< Serdes configuration                                                */
    u16  pma_txratecontrol;   /**< Normal pma_txratecontrol value to be restored when loopback is disabled */
} vtss_phy_10g_mode_t; 

/**
 * \brief Get the Phy operating mode.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param mode [IN]     Mode configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/

vtss_rc vtss_phy_10g_mode_get (const vtss_inst_t inst, 
                               const vtss_port_no_t port_no, 
                               vtss_phy_10g_mode_t *const mode);


/**
 * \brief Identify, Reset and set the operating mode of the PHY.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param mode [IN]     Mode configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_mode_set (const vtss_inst_t inst, 
                               const vtss_port_no_t port_no, 
                               const vtss_phy_10g_mode_t *const mode);


#if defined(VTSS_FEATURE_SYNCE_10G) 
/**
 * \brief Get the status of recovered clock from PHY. (recommended to use vtss_phy_10g_rxckout_get instead)
 *
 * \param inst [IN]          Target instance reference.
 * \param port_no [IN]       Port number.
 * \param synce_clkout [IN]  Recovered clock configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_synce_clkout_get (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       BOOL *const synce_clkout);

/**
 * \brief Enable or Disable the recovered clock from PHY. (recommended to use vtss_phy_10g_rxckout_set instead)
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param synce_clkout [IN]  Recovered clock to be enabled or disabled.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_synce_clkout_set (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       const BOOL synce_clkout);


/**
 * \brief Get the status of RXCLKOUT/TXCLKOUT from PHY. (recommended to use vtss_phy_10g_txckout_get instead)
 *
 * \param inst [IN]          Target instance reference.
 * \param port_no [IN]       Port number.
 * \param xfp_clkout [IN]    XFP clock configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_xfp_clkout_get (const vtss_inst_t inst,
                                     const vtss_port_no_t port_no,
                                     BOOL *const xfp_clkout);

/**
 * \brief Enable or Disable the RXCLKOUT/TXCLKOUT from PHY. (recommended to use vtss_phy_10g_txckout_set instead)
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param xfp_clkout [IN]  XFP clock to be enabled or disabled.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_xfp_clkout_set (const vtss_inst_t inst,
                                     const vtss_port_no_t port_no,
                                     const BOOL xfp_clkout);

/** \brief Modes for (rx/tx) recovered clock output*/
typedef enum {
    VTSS_RECVRD_CLKOUT_DISABLE,             /**< recovered clock output is disabled */
    VTSS_RECVRD_CLKOUT_LINE_SIDE_RX_CLOCK,  /**< recovered clock output is derived from Lineside Rx clock */
    VTSS_RECVRD_CLKOUT_LINE_SIDE_TX_CLOCK,  /**< recovered clock output is derived from Lineside Tx clock */
} vtss_recvrd_clkout_t;

/** \brief 10G Phy RXCKOUT config data */
typedef struct {
    vtss_recvrd_clkout_t   mode;    /**< RXCKOUT output mode (DISABLE/RX_CLK/TX_CLK) */
    BOOL   squelch_on_pcs_fault;    /**< Enable squelching on PCS_FAULT (Venice family only) */
    BOOL   squelch_on_lopc;         /**< Enable squelching on LOPC (Venice family only) */
} vtss_phy_10g_rxckout_conf_t; 

/**
 * \brief Get the rx recovered clock output configuration.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param rxckout [OUT]     RXCKOUT clock configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_rxckout_get (const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  vtss_phy_10g_rxckout_conf_t *const rxckout);

/**
 * \brief Set the rx recovered clock output configuration.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param rxckout [IN]      RXCKOUT clock configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_rxckout_set (const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  const vtss_phy_10g_rxckout_conf_t *const rxckout);

/** \brief 10G Phy TXCKOUT config data */
typedef struct {
    vtss_recvrd_clkout_t   mode;    /**< TXCKOUT output mode (DISABLE/RX_CLK/TX_CLK) */
} vtss_phy_10g_txckout_conf_t; 

/**
 * \brief Get the status of tx recovered clock output configuration.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param txckout [OUT]     TXCKOUT clock configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_txckout_get (const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  vtss_phy_10g_txckout_conf_t *const txckout);

/**
 * \brief Set the tx recovered clock output configuration.
 *
 * \param inst [IN]         Target instance reference.
 * \param port_no [IN]      Port number.
 * \param txckout [IN]      TXCKOUT clock configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_txckout_set (const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  const vtss_phy_10g_txckout_conf_t *const txckout);


/** \brief 10G Phy sref clock input frequency */
typedef enum {    
    VTSS_PHY_10G_SREFCLK_156_25,  /**< 156,25 MHz */
    VTSS_PHY_10G_SREFCLK_125_00,  /**< 125,00 MHz */
    VTSS_PHY_10G_SREFCLK_155_52,  /**< 155,52 MHz */
    VTSS_PHY_10G_SREFCLK_INVALID  /**< Other values are not allowed*/
} vtss_phy_10g_srefclk_freq_t; 

/** \brief 10G Phy srefclk config data */
typedef struct {
    BOOL                        enable;    /**< Enable locking line tx clock to srefclk input  */
    vtss_phy_10g_srefclk_freq_t freq;      /**< The srefclk input frequency */
} vtss_phy_10g_srefclk_mode_t; 

/**
 * \brief Get the configuration of srefclk setting\n
 *  Avaliable for PHY family VENICE\n
 *           This function should not be used any more, instead use the API function vtss_phy_10g_mode_get,
 *           see the parameter documentation for that function.
 *
 * \param inst [IN]          Target instance reference.
 * \param port_no [IN]       Port number.
 * \param srefclk [OUT]      srefclk configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_srefclk_conf_get (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       vtss_phy_10g_srefclk_mode_t *const srefclk);

/**
 * \brief Set the configuration of srefclk setting.
 * Avaliable for PHY family VENICE\n
 *           This function should not be used any more, instead use the API function vtss_phy_10g_mode_set,
 *           see the parameter documentation for that function.
 *
 * \param inst [IN]          Target instance reference.
 * \param port_no [IN]       Port number.
 * \param srefclk [IN]       srefclk configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_srefclk_conf_set (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       const vtss_phy_10g_srefclk_mode_t *const srefclk);


#endif /* VTSS_FEATURE_SYNCE_10G */

#ifdef VTSS_FEATURE_10GBASE_KR

/** \brief Slew rate ctrl of OB  */
#define VTSS_SLEWRATE_25PS    0          /**< Slewrate = 25ps */
#define VTSS_SLEWRATE_35PS    1          /**< Slewrate = 35ps */
#define VTSS_SLEWRATE_55PS    2          /**< Slewrate = 55ps */
#define VTSS_SLEWRATE_70PS    3          /**< Slewrate = 70ps */
#define VTSS_SLEWRATE_120PS   4          /**< Slewrate = 120ps */
#define VTSS_SLEWRATE_INVALID 5          /**< Slewrate is invalid */

/** \brief 10G Phy 10f_base_kr_conf config data according to 802.3-2008 clause 72.7 Figure 72-11*/

typedef struct {
    i32 cm1;        /**< The minus 1 coefficient c(-1). Range: -32..31 */
    i32 c0;         /**< The 0 coefficient c(0).        Range: -32..31 */
    i32 c1;         /**< The plus 1 coefficient c(1).   Range: -32..31 */
    u32 ampl;       /**< The Amplitude value in nVpp. Range: 300..1275 */
    u32 slewrate;   /**< Slew rate ctrl of OB */
    BOOL en_ob;     /**< Enable output buffer and serializer */
    BOOL ser_inv;   /**< Invert input to serializer */
} vtss_phy_10g_base_kr_conf_t; 

/**
 * \brief Get the configuration of 10f_base_kr setting.
 * Avaliable for PHY family VENICE
 *
 * \param inst [IN]          Target instance reference.
 * \param port_no [IN]       Port number.
 * \param kr_conf [OUT]      10f_base_kr configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_base_kr_conf_get (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       vtss_phy_10g_base_kr_conf_t *const kr_conf);

/**
 * \brief Set the configuration of 10f_base_kr setting.
 * Avaliable for PHY family VENICE:
 * Note: The parameters cm1,c0, c1 have a range of -32..31. The Output signal from the KR circuit is symmetric,
 *       I.e. the voltage output is configured value + 1/2lsb. (ex: -1 => -1/2lsb voltage level, 0 => +1/2lsb voltage level
 *       The parameter ampl is set in steps of 25 mV, therefore the setting is rounded up to a multiplum og 25 mV,
 *       I.e. 1101..1125 => 1125 mVppd
 *
 * \param inst [IN]          Target instance reference.
 * \param port_no [IN]       Port number.
 * \param kr_conf [IN]       10f_base_kr configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERR_KR_CONF_NOT_SUPPORTED if the PHY on the port does not support 10GBASE_KR configuration
 *   VTSS_RC_ERR_KR_CONF_INVALID_PARAMETER if one of the parameter values are invalid or (|cm1|+ |c0| + |c1|) > 31
 *   VTSS_RC_ERROR on other errors.
 **/
vtss_rc vtss_phy_10g_base_kr_conf_set (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       const vtss_phy_10g_base_kr_conf_t *const kr_conf);

#endif /* VTSS_FEATURE_10GBASE_KR */


/** \brief 10G Phy link and fault status */
typedef struct {
    BOOL           rx_link;    /**< The rx link status  */
    vtss_event_t   link_down;  /**< Link down event status. Clear on read  */
    BOOL           rx_fault;   /**< Rx fault event status.  Clear on read */
    BOOL           tx_fault;   /**< Tx fault event status.  Clear on read */
} vtss_sublayer_status_t; 

/** \brief 10G Phy link and fault status for all sublayers */
typedef struct {
    vtss_sublayer_status_t	pma; /**< Status for PMA sublayer */
    vtss_sublayer_status_t	wis; /**< Status for WIS sublayer */
    vtss_sublayer_status_t	pcs; /**< Status for PCS sublayer */
    vtss_sublayer_status_t	xs;  /**< Status for XS  sublayer */
} vtss_phy_10g_status_t; 

/**
 * \brief Get the link and fault status of the PHY sublayers.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param status [IN]   Status of all sublayers
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_status_get (const vtss_inst_t inst, 
                    		 const vtss_port_no_t port_no, 
                    		 vtss_phy_10g_status_t *const status);



/**
 * \brief Reset the phy.  Phy is reset to default values.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_reset(const vtss_inst_t       	inst,
                           const vtss_port_no_t    	port_no);

/**
 * Advertisement Word (Refer to IEEE 802.3 Clause 37):
 *  MSB                                                                         LSB
 *  D15  D14  D13  D12  D11  D10   D9   D8   D7   D6   D5   D4   D3   D2   D1   D0 
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | NP | Ack| RF2| RF1|rsvd|rsvd|rsvd| PS2| PS1| HD | FD |rsvd|rsvd|rsvd|rsvd|rsvd|
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 **/

/** \brief Auto-negotiation remote fault type */
typedef enum                  
{
    VTSS_PHY_10G_CLAUSE_37_RF_LINK_OK,        /**< Link OK */
    VTSS_PHY_10G_CLAUSE_37_RF_OFFLINE,        /**< Off line */
    VTSS_PHY_10G_CLAUSE_37_RF_LINK_FAILURE,   /**< Link failure */
    VTSS_PHY_10G_CLAUSE_37_RF_AUTONEG_ERROR   /**< Autoneg error */
} vtss_phy_10g_clause_37_remote_fault_t;


/** \brief Advertisement control data for Clause 37 aneg */
typedef struct
{
    BOOL                                  fdx;               /**< (FD) */
    BOOL                                  hdx;               /**< (HD) */
    BOOL                                  symmetric_pause;   /**< (PS1) */
    BOOL                                  asymmetric_pause;  /**< (PS2) */
    vtss_phy_10g_clause_37_remote_fault_t remote_fault;      /**< (RF1) + (RF2) */
    BOOL                                  acknowledge;       /**< (Ack) */
    BOOL                                  next_page;         /**< (NP) */
} vtss_phy_10g_clause_37_adv_t;

/** \brief Clause 37 Auto-negotiation status */
typedef struct
{    
    BOOL link;                                             /**< FALSE if link has been down since last status read */
    struct {
        BOOL                         complete;             /**< Aneg completion status */
        vtss_phy_10g_clause_37_adv_t partner_advertisement;/**< Clause 37 Advertisement control data */
    } autoneg;                                             /**< Autoneg status */
} vtss_phy_10g_clause_37_status_t;

/** \brief Clause 37 Auto-negotiation status for line and host */
typedef struct
{
    vtss_phy_10g_clause_37_status_t line;  /**< Line clause 37 status */
    vtss_phy_10g_clause_37_status_t host;  /**< Host clause 37 status */
} vtss_phy_10g_clause_37_cmn_status_t;


/** \brief Clause 37 control struct */
typedef struct
{
    BOOL                         enable;        /**< Enable of Autoneg */
    vtss_phy_10g_clause_37_adv_t advertisement; /**< Clause 37 Advertisement data */
} vtss_phy_10g_clause_37_control_t;


/**
 * \brief Get clause 37 status.
 *
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number.
 * \param status  [OUT] Clause 37 status of the line and host link.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_clause_37_status_get(const vtss_inst_t                          inst,
                                          vtss_port_no_t                             port_no,
                                          vtss_phy_10g_clause_37_cmn_status_t *const status);


/**
 * \brief Get clause 37 control configuration.
 *
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number.
 * \param control [OUT] Clause 37 configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_clause_37_control_get(const vtss_inst_t                 inst,
                                           const vtss_port_no_t              port_no,
                                           vtss_phy_10g_clause_37_control_t  *const control);

/**
 * \brief Set clause 37 control configuration.
 *
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number.
 * \param control [OUT] Clause 37 configuration.
 *  Same configuration is applied to Host and Line interface.                  
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_clause_37_control_set(const vtss_inst_t                       inst,
                                           const vtss_port_no_t                    port_no,
                                           const vtss_phy_10g_clause_37_control_t  *const control);



/** \brief 10G Phy system and network loopbacks */
typedef struct {
    enum {
        VTSS_LB_NONE, 	             /**< No looback   */
        VTSS_LB_SYSTEM_XS_SHALLOW,   /**< System Loopback B,  XAUI -> XS -> XAUI   4x800E.13,  Venice: H2  */
        VTSS_LB_SYSTEM_XS_DEEP,      /**< System Loopback C,  XAUI -> XS -> XAUI   4x800F.2,  Venice: N.A.     */
        VTSS_LB_SYSTEM_PCS_SHALLOW,  /**< System Loopback E,  XAUI -> PCS FIFO -> XAUI 3x8005.2,  Venice: N.A. */
        VTSS_LB_SYSTEM_PCS_DEEP,     /**< System Loopback G,  XAUI -> PCS -> XAUI  3x0000.14,  Venice: H3    */
        VTSS_LB_SYSTEM_PMA,	         /**< System Loopback J,  XAUI -> PMA -> XAUI  1x0000.0,  Venice: H4     */
        VTSS_LB_NETWORK_XS_SHALLOW,  /**< Network Loopback D,  XFI -> XS -> XFI   4x800F.1,  Venice: N.A.       */
        VTSS_LB_NETWORK_XS_DEEP,     /**< Network Loopback A,  XFI -> XS -> XFI   4x0000.1  4x800E.13=0,  Venice: L1  */
        VTSS_LB_NETWORK_PCS,	     /**< Network Loopback F,  XFI -> PCS -> XFI  3x8005.3,  Venice: L2       */
        VTSS_LB_NETWORK_WIS,	     /**< Network Loopback H,  XFI -> WIS -> XFI  2xE600.0,  Venice: N.A.       */
        VTSS_LB_NETWORK_PMA,         /**< Network Loopback K,  XFI -> PMA -> XFI  1x8000.8,  Venice: L3       */
        /* Venice specific loopbacks, the Venice implementation is different, and therefore the loopbacks are not exactly the same */
        VTSS_LB_H2,                  /**< Host Loopback 2, 40-bit XAUI-PHY interface Mirror XAUI data */
        VTSS_LB_H3,                  /**< Host Loopback 3, 64-bit PCS after the gearbox FF00 repeating IEEE PCS system loopback */
        VTSS_LB_H4,                  /**< Host Loopback 4, 64-bit WIS FF00 repeating IEEE WIS system loopback */
        VTSS_LB_H5,                  /**< Host Loopback 5, 1-bit SFP+ after SerDes Mirror XAUI data IEEE PMA system loopback */
        VTSS_LB_H6,                  /**< Host Loopback 6, 32-bit XAUI-PHY interface Mirror XAUI data */
        VTSS_LB_L0,                  /**< Line Loopback 0, 4-bit XAUI before SerDes Mirror SFP+ data */
        VTSS_LB_L1,                  /**< Line Loopback 1, 4-bit XAUI after SerDes Mirror SFP+ data IEEE PHY-XS network loopback */
        VTSS_LB_L2,                  /**< Line Loopback 2, 64-bit XGMII after FIFO Mirror SFP+ data */
        VTSS_LB_L3                   /**< Line Loopback 3, 64-bit PMA interface Mirror SFP+ data */
    } lb_type;                       /**< Looback types                                          */  
    BOOL enable;                     /**< Enable/Disable loopback given in \<lb_type\>             */
} vtss_phy_10g_loopback_t; 

/**
 * \brief Enable/Disable a phy network or system loopback. \n
 * Only one loopback mode can be active at the same time. 
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param loopback [IN] Loopback settings. When disabling a loopback, the lb_type is ignored, i.e. the active loopback is disabled.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.\n
 *       Error conditions: 
 *           Loopback not supported for the PHY
 *           Attempt to enable loopback while loopback is already active
 *           Attempt to disable loopback while no loopback is active
 **/
vtss_rc vtss_phy_10g_loopback_set(const vtss_inst_t       	inst,
                                  const vtss_port_no_t   	port_no,
                                  const vtss_phy_10g_loopback_t	*const loopback);

/**
 * \brief Get loopback settings.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param loopback [OUT] Current loopback settings.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_loopback_get(const vtss_inst_t       	inst,
                                  const vtss_port_no_t   	port_no,
                                  vtss_phy_10g_loopback_t	*const loopback);



/** \brief 10G Phy PCS counters */
typedef struct {
    BOOL                        block_lock_latched; /**< Latched block status      */
    BOOL                        high_ber_latched;   /**< Lathced high ber status   */
    u8 				ber_cnt; 	    /**< BER counter. Saturating, clear on read */
    u8 				err_blk_cnt; 	    /**< ERROR block counter. Saturating, clear on read */
} vtss_phy_pcs_cnt_t; 

/** \brief 10G Phy Sublayer counters */
typedef struct {
//  vtss_phy_pma_cnt_t		pma;
//  vtss_phy_wis_cnt_t		wis;
    vtss_phy_pcs_cnt_t		pcs;  /**< PCS counters */
//  vtss_phy_xs_cnt_t		xs;
} vtss_phy_10g_cnt_t; 

/**
 * \brief Get counters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cnt [OUT] Phy counters
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_cnt_get(	const vtss_inst_t       inst,
				const vtss_port_no_t    port_no,
				vtss_phy_10g_cnt_t   	*const cnt);

/** \brief 10G Phy power setting */
typedef enum {    
    VTSS_PHY_10G_POWER_ENABLE,  /**< Enable Phy power for all sublayers */
    VTSS_PHY_10G_POWER_DISABLE  /**< Disable Phy power for all sublayers*/
} vtss_phy_10g_power_t; 


/**
 * \brief Get the power settings 
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param power [OUT] power settings
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_power_get(const vtss_inst_t      inst,
                               const vtss_port_no_t   port_no,
                               vtss_phy_10g_power_t  *const power);
/**
 * \brief Set the power settings.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param power [IN] power settings
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_power_set(const vtss_inst_t        inst,
                               const vtss_port_no_t     port_no,
                               const vtss_phy_10g_power_t  *const power);

/**
 * \brief Gives a True/False value if the Phy is supported by the API\n
 *  Only Vitesse phys are supported.  vtss_phy_10g_mode_set() must be applied.
 *
 * \param inst [IN] Target instance reference. 
 * \param port_no [IN]  Port number.
 *
 * \return
 *   TRUE  : Phy is supported.\n
 *   FALSE : Phy is not supported.
 **/
BOOL vtss_phy_10G_is_valid(const vtss_inst_t        inst,
                           const vtss_port_no_t port_no);

/** \brief 10G Phy power setting */
typedef enum {    
    VTSS_PHY_10G_PMA_TO_FROM_XAUI_NORMAL,         /**< PMA_0/1 to XAUI_0/1. 8487: XAUI 0 to PMA 0 */
    VTSS_PHY_10G_PMA_TO_FROM_XAUI_CROSSED,        /**< PMA_0/1 to XAUI_1/0. 8487: XAUI 1 to PMA 0 */
    VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1,  /**< PMA 0 to/from XAUI 0 and to XAUI 1 */
    VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0,  /**< PMA 0 to/from XAUI 1 and to XAUI 0 */ 
    VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_0_TO_XAUI_1,  /**< PMA 1 to/from XAUI 0 and to XAUI 1.      VSC8487:N/A */
    VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_1_TO_XAUI_0,  /**< PMA 1 to/from XAUI 1 and to XAUI 0.      VSC8487:N/A */
} vtss_phy_10g_failover_mode_t; 

/**
 * \brief Set the failover mode 
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number. (Use any port within the phy).
 * \param mode [IN]     Failover mode
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_failover_set(const vtss_inst_t      inst,
                                  const vtss_port_no_t   port_no,
                                  vtss_phy_10g_failover_mode_t  *const mode);
/**
 * \brief Get the failover mode 
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param mode [OUT] failover mode 
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_failover_get(const vtss_inst_t      inst,
                                  const vtss_port_no_t   port_no,
                                  vtss_phy_10g_failover_mode_t  *const mode);

/** \brief 10g PHY type */
typedef enum {
    VTSS_PHY_TYPE_10G_NONE = 0,/* Unknown  */
    VTSS_PHY_TYPE_8484 = 8484, /* VSC8484  */
    VTSS_PHY_TYPE_8486 = 8486, /* VSC8486  */
    VTSS_PHY_TYPE_8487 = 8487, /* VSC8487  */
    VTSS_PHY_TYPE_8488 = 8488, /* VSC8488  */
    VTSS_PHY_TYPE_8489 = 8489, /* VSC8489  */
    VTSS_PHY_TYPE_8490 = 8490, /* VSC8490  */
    VTSS_PHY_TYPE_8491 = 8491, /* VSC8491  */
} vtss_phy_10g_type_t; 

/** \brief 10G PHY family */
typedef enum {
    VTSS_PHY_FAMILY_10G_NONE,        /* Unknown */
    VTSS_PHY_FAMILY_XAUI_XGMII_XFI,  /* VSC8486 */
    VTSS_PHY_FAMILY_XAUI_XFI,        /* VSC8484, VSC8487, VSC8488 */
    VTSS_PHY_FAMILY_VENICE,          /* VSC8489, VSC8490, VSC8491 */
} vtss_phy_10g_family_t;

#define VTSS_PHY_10G_ONE_LINE_ACTIVE 0x08    /**< Bit indicating PHY vith only one line interface */
#define VTSS_PHY_10G_MACSEC_DISABLED 0x04    /**< Bit indicating that macsec is disabled */
#define VTSS_PHY_10G_TIMESTAMP_DISABLED 0x02 /**< Bit indicating that timestamp feature is disabled */
#define VTSS_PHY_10G_MACSEC_KEY_128 0x01     /**< Bit indicating that only 128 bit macsec encryption key is supported, otherwise it is 128/256 key */



/** \brief 10G Phy part number and revision */
typedef struct
{
    u16                   part_number;    /**< Part number (Hex)  */
    u16                   revision;       /**< Chip revision      */
    u16                   channel_id;     /**< Channel id         */
    vtss_phy_10g_family_t family;         /**< Phy Family         */
    vtss_phy_10g_type_t   type;           /**< Phy id (Decimal)   */
    vtss_port_no_t        phy_api_base_no;/**< First API no within this phy (in case of multiple channels) */
    u16                   device_feature_status; /**< Device features depending on EFUSE */
} vtss_phy_10g_id_t; 

/**
 * \brief Read the Phy Id
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param phy_id [OUT] The part number and revision.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on error.
 **/
vtss_rc vtss_phy_10g_id_get(const vtss_inst_t   inst, 
                            const vtss_port_no_t  port_no, 
                            vtss_phy_10g_id_t *const phy_id);

/* - GPIOs ------------------------------------------------------- */
/**
 * \brief GPIO configured mode
 **/
#ifndef VTSS_GPIOS
typedef u32 vtss_gpio_no_t; /**< GPIO type for 1G ports*/
#endif

/**
 * \brief GPIO configured mode
 **/
typedef struct
{
    enum
    {
        VTSS_10G_PHY_GPIO_NOT_INITIALIZED,   /**< This GPIO pin has has been initialized by a call to API from application. aregisters contain power-up default value */
        VTSS_10G_PHY_GPIO_OUT,               /**< Output enabled */
        VTSS_10G_PHY_GPIO_IN,                /**< Input enabled */
        VTSS_10G_PHY_GPIO_WIS_INT,           /**< Output WIS interrupt channel 0 or 1 (depending on port_no) enabled */
    /*    VTSS_10G_PHY_GPIO_INT_FALLING,*/   /**< Input interrupt generated on falling edge */
    /*    VTSS_10G_PHY_GPIO_INT_RAISING,*/   /**< Input interrupt generated on raising edge */
    /*    VTSS_10G_PHY_GPIO_INT_CHANGED,*/   /**< Input interrupt generated on raising and falling edge */
        VTSS_10G_PHY_GPIO_1588_LOAD_SAVE,    /**< Input 1588 load/save function */
        VTSS_10G_PHY_GPIO_1588_1PPS_0,       /**< Output 1588 1PPS channel 0 function */
        VTSS_10G_PHY_GPIO_1588_1PPS_1,       /**< Output 1588 1PPS channel 1 function */
        VTSS_10G_PHY_GPIO_PCS_RX_FAULT,      /**< PCS_RX_FAULT (from channel 0 or 1) is transmitted on GPIO */
        /* More to come.. */
    } mode;                                  /**< Mode of this GPIO pin */
    vtss_port_no_t port;                     /**< In case of VTSS_10G_PHY_GPIO_WIS_INT mode, this is the interrupt port number that is related to this GPIO
                                                  In case of VTSS_10G_PHY_GPIO_PCS_RX_FAULT  mode, this is the PCS status port number that is related to this GPIO */
} vtss_gpio_10g_gpio_mode_t;

typedef u16 vtss_gpio_10g_no_t; /**< GPIO type for 10G ports*/

#define VTSS_10G_PHY_GPIO_MAX   12  /**< Max value of gpio_no parameter */

/**
 * \brief Set GPIO mode. There is only one set og GPIO per PHY chip - not per port.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number that identify the PHY chip.
 * \param gpio_no [IN]  GPIO pin number < VTSS_10G_PHY_GPIO_MAX.
 * \param mode [IN]     GPIO mode.
 *
 * \return Return code.
 **/
vtss_rc vtss_phy_10g_gpio_mode_set(const vtss_inst_t                inst,
                                   const vtss_port_no_t             port_no,
                                   const vtss_gpio_10g_no_t         gpio_no,
                                   const vtss_gpio_10g_gpio_mode_t  *const mode);
/**
 * \brief Get GPIO mode.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number that identify the PHY chip.
 * \param gpio_no [IN]  GPIO pin number.
 * \param mode [OUT]    GPIO mode.
 *
 * \return Return code.
 **/

vtss_rc vtss_phy_10g_gpio_mode_get(const vtss_inst_t          inst,
                                   const vtss_port_no_t       port_no,
                                   const vtss_gpio_10g_no_t   gpio_no,
                                   vtss_gpio_10g_gpio_mode_t  *const mode);

/**
 * \brief Read from GPIO input pin.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param gpio_no [IN]  GPIO pin number.
 * \param value [OUT]   TRUE if pin is high, FALSE if it is low.
 *
 * \return Return code.
 **/
vtss_rc vtss_phy_10g_gpio_read(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               const vtss_gpio_10g_no_t  gpio_no,
                               BOOL                  *const value);

/**
 * \brief Write to GPIO output pin.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param gpio_no [IN]  GPIO pin number.
 * \param value [IN]    TRUE to set pin high, FALSE to set pin low.
 *
 * \return Return code.
 **/
vtss_rc vtss_phy_10g_gpio_write(const vtss_inst_t     inst,
                                const vtss_port_no_t  port_no,
                                const vtss_gpio_10g_no_t  gpio_no,
                                const BOOL            value);

/* - Events ------------------------------------------------------- */

/** \brief Event source identification mask values */
#define    VTSS_PHY_10G_LINK_LOS_EV            0x00000001 /**< PHY Link Los interrupt - only on 8486 */
#define    VTSS_PHY_10G_RX_LOL_EV              0x00000002 /**< PHY RXLOL interrupt - only on 8488 */
#define    VTSS_PHY_10G_TX_LOL_EV              0x00000004 /**< PHY TXLOL interrupt - only on 8488 */
#define    VTSS_PHY_10G_LOPC_EV                0x00000008 /**< PHY LOPC interrupt - only on 8488 */
#define    VTSS_PHY_10G_HIGH_BER_EV            0x00000010 /**< PHY HIGH_BER interrupt - only on 8488 */
#define    VTSS_PHY_10G_MODULE_STAT_EV         0x00000020 /**< PHY MODULE_STAT interrupt - only on 8488 */
#define    VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV   0x00000040 /**< PHY PCS_RECEIVE_FAULT interrupt - only on 8488 */
#ifdef VTSS_FEATURE_WIS
#define    VTSS_PHY_EWIS_SEF_EV                0x00000080 /**< SEF has changed state - only for 8488 */
#define    VTSS_PHY_EWIS_FPLM_EV               0x00000100 /**< far-end (PLM-P) / (LCDP) - only for 8488 */
#define    VTSS_PHY_EWIS_FAIS_EV               0x00000200 /**< far-end (AIS-P) / (LOP) - only for 8488 */
#define    VTSS_PHY_EWIS_LOF_EV                0x00000400 /**< Loss of Frame (LOF) - only for 8488 */
#define    VTSS_PHY_EWIS_RDIL_EV               0x00000800 /**< Line Remote Defect Indication (RDI-L) - only for 8488 */
#define    VTSS_PHY_EWIS_AISL_EV               0x00001000 /**< Line Alarm Indication Signal (AIS-L) - only for 8488 */
#define    VTSS_PHY_EWIS_LCDP_EV               0x00002000 /**< Loss of Code-group Delineation (LCD-P) - only for 8488 */
#define    VTSS_PHY_EWIS_PLMP_EV               0x00004000 /**< Path Label Mismatch (PLMP) - only for 8488 */
#define    VTSS_PHY_EWIS_AISP_EV               0x00008000 /**< Path Alarm Indication Signal (AIS-P) - only for 8488 */
#define    VTSS_PHY_EWIS_LOPP_EV               0x00010000 /**< Path Loss of Pointer (LOP-P) - only for 8488 */
#define    VTSS_PHY_EWIS_UNEQP_EV              0x00020000 /**< Unequiped Path (UNEQ-P) - only for 8488 */
#define    VTSS_PHY_EWIS_FEUNEQP_EV            0x00040000 /**< Far-end Unequiped Path (UNEQ-P) - only for 8488 */
#define    VTSS_PHY_EWIS_FERDIP_EV             0x00080000 /**< Far-end Path Remote Defect Identifier (RDI-P) - only for 8488 */
#define    VTSS_PHY_EWIS_REIL_EV               0x00100000 /**< Line Remote Error Indication (REI-L) - only for 8488 */
#define    VTSS_PHY_EWIS_REIP_EV               0x00200000 /**< Path Remote Error Indication (REI-P) - only for 8488 */
#define    VTSS_PHY_EWIS_B1_NZ_EV              0x00400000 /**< PMTICK B1 BIP (B1_ERR_CNT) not zero - only for 8488 */
#define    VTSS_PHY_EWIS_B2_NZ_EV              0x00800000 /**< PMTICK B2 BIP (B1_ERR_CNT) not zero - only for 8488 */
#define    VTSS_PHY_EWIS_B3_NZ_EV              0x01000000 /**< PMTICK B3 BIP (B1_ERR_CNT) not zero - only for 8488 */
#define    VTSS_PHY_EWIS_REIL_NZ_EV            0x02000000 /**< PMTICK REI-L (REIL_ERR_CNT) not zero - only for 8488 */
#define    VTSS_PHY_EWIS_REIP_NZ_EV            0x04000000 /**< PMTICK REI-P (REIP_ERR_CNT) not zero - only for 8488 */
#define    VTSS_PHY_EWIS_B1_THRESH_EV          0x08000000 /**< B1_THRESH_ERR - only for 8488 */
#define    VTSS_PHY_EWIS_B2_THRESH_EV          0x10000000 /**< B2_THRESH_ERR - only for 8488 */
#define    VTSS_PHY_EWIS_B3_THRESH_EV          0x20000000 /**< B3_THRESH_ERR - only for 8488 */
#define    VTSS_PHY_EWIS_REIL_THRESH_EV        0x40000000 /**< REIL_THRESH_ERR - only for 8488 */
#define    VTSS_PHY_EWIS_REIP_THRESH_EV        0x80000000 /**< REIp_THRESH_ERR - only for 8488 */
#endif /* VTSS_FEATURE_WIS */
typedef u32 vtss_phy_10g_event_t;   /**< The type definition to contain the above defined evant mask */

/**
 * \brief Enabling / Disabling of events
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number 
 * \param ev_mask [IN]  Mask containing events that are enabled/disabled 
 * \param enable [IN]   Enable/disable of event
 *
 * \return Return code.
 **/

vtss_rc vtss_phy_10g_event_enable_set(const vtss_inst_t           inst,
                                      const vtss_port_no_t        port_no,
                                      const vtss_phy_10g_event_t  ev_mask,
                                      const BOOL                  enable);

/**
 * \brief Get Enabling of events
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number 
 * \param ev_mask [OUT]  Mask containing events that are enabled 
 *
 * \return Return code.
 **/

vtss_rc vtss_phy_10g_event_enable_get(const vtss_inst_t      inst,
                                      const vtss_port_no_t   port_no,
                                      vtss_phy_10g_event_t   *const ev_mask);

/**
 * \brief Polling for active events
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number 
 * \param ev_mask [OUT] Mask containing events that are active
 *
 * \return Return code.
 **/
vtss_rc vtss_phy_10g_event_poll(const vtss_inst_t     inst,
                                const vtss_port_no_t  port_no,
                                vtss_phy_10g_event_t  *const ev_mask);


/**
 * \brief Function is called once a second
 *
 * \param inst [IN]     Target instance reference.
 * \return Return code.
 **/
vtss_rc vtss_phy_10g_poll_1sec(const vtss_inst_t  inst);

/** \brief Firmware status */
typedef struct {
    u16             edc_fw_rev;      /**< FW revision */
    BOOL            edc_fw_chksum;   /**< FW chksum.    Fail=0, Pass=1*/
    BOOL            icpu_activity;   /**< iCPU activity.  Not Running=0, Running=1   */
    BOOL            edc_fw_api_load; /**< EDC FW is loaded through API No=0, Yes=1  */
} vtss_phy_10g_fw_status_t; 

/**
 * \brief Internal microprocessor status
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number 
 * \param status [OUT]  Status of the EDC FW running on the internal CPU
 *
 * \return Return code.
 **/
vtss_rc vtss_phy_10g_edc_fw_status_get(const vtss_inst_t     inst,
                                       const vtss_port_no_t  port_no,
                                       vtss_phy_10g_fw_status_t  *const status);

/**
 * \brief CSR register read
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number 
 * \param dev     [IN]   Device id (or MMD) 
 * \param addr    [IN]   Addr of the register, 16 or 32 bit
 * \param value   [OUT]  Return value of the register
 *
 * \return Return code.
 **/
vtss_rc vtss_phy_10g_csr_read(const vtss_inst_t           inst,
                              const vtss_port_no_t        port_no,
                              const u32                   dev,
                              const u32                   addr,
                              u32                         *const value);

/**
 * \brief CSR register write
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number 
 * \param dev     [IN]   Device id (or MMD) 
 * \param addr    [IN]   Addr of the register, 16 or 32 bit
 * \param value   [IN]   Value to be written
 *
 * \return Return code.
 **/
vtss_rc vtss_phy_10g_csr_write(const vtss_inst_t           inst,
                               const vtss_port_no_t        port_no,
                               const u32                   dev,
                               const u32                   addr,
                               const u32                   value);
/**
 * \brief Function for checking if any issue were seen during warm-start
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  The port in question.
 * \return Return code. VTSS_RC_OK if not errors ware seen during warm-start else VTSS_RC_ERROR.*/
vtss_rc vtss_phy_warm_start_10g_failed_get(const vtss_inst_t inst, const vtss_port_no_t port_no);

#endif /* VTSS_CHIP_10G_PHY */
#endif /* _VTSS_PHY_10G_API_H_ */
