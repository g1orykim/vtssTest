#ifndef _VTSS_VENICE_REGS_LINE_MAC_H_
#define _VTSS_VENICE_REGS_LINE_MAC_H_

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

#include "vtss_venice_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a LINE_MAC
 *
 * 10/100/1000/2500/10000/Overclocked XAUI (12,5 Gbps) MAC - Only full duplex
 * - no half duplex is supported.
 * Requirements relate to wrapper exbg10_std_mac, which includes
 * host_loopback and phy_loopback module. 
 * All remaining functionality is included in KE
 *
 ***********************************************************************/

/**
 * Register Group: \a LINE_MAC:CONFIG
 *
 * MAC10G Configuration Registers
 */


/** 
 * \brief Mode Configuration Register
 *
 * \details
 * Register: \a LINE_MAC:CONFIG:MAC_ENA_CFG
 */
#define VTSS_LINE_MAC_CONFIG_MAC_ENA_CFG     VTSS_IOREG(0x03, 1, 0xf200)

/** 
 * \brief
 * MAC Rx clock enable.
 *
 * \details 
 * 0 : All clocks for this module with the exception of CSR clock are
 * disabled.
 * 1 : All clocks for this module are enabled.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ENA_CFG . RX_CLK_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_RX_CLK_ENA  VTSS_BIT(0)

/** 
 * \brief
 * MAC Tx clock enable.
 *
 * \details 
 * 0 : All clocks for this module with the exception of CSR clock are
 * disabled.
 * 1 : All clocks for this module are enabled.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ENA_CFG . TX_CLK_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_TX_CLK_ENA  VTSS_BIT(4)

/** 
 * \brief
 * MAC Rx SW reset
 *
 * \details 
 * 0 : Block operates normally.
 * 1 : All logic (other than CSR target) is held in reset, clocks are not
 * disabled.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ENA_CFG . RX_SW_RST
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_RX_SW_RST  VTSS_BIT(8)

/** 
 * \brief
 * MAC Tx SW reset
 *
 * \details 
 * 0 : Block operates normally.
 * 1 : All logic (other than CSR target) is held in reset, clocks are not
 * disabled.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ENA_CFG . TX_SW_RST
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_TX_SW_RST  VTSS_BIT(12)

/** 
 * \brief
 * Enable Receiver.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ENA_CFG . RX_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_RX_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Enable Transmitter.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ENA_CFG . TX_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_TX_ENA  VTSS_BIT(20)


/** 
 * \brief Mode Configuration Register
 *
 * \details
 * Register: \a LINE_MAC:CONFIG:MAC_MODE_CFG
 */
#define VTSS_LINE_MAC_CONFIG_MAC_MODE_CFG    VTSS_IOREG(0x03, 1, 0xf201)

/** 
 * \brief
 * When this value is '0' MAC10G will follow 0-3 DIC algorithm to insert
 * IPG, averaging to 12.
 * When this value is '1' MAC10G will not follow DIC alogrithm for IPG
 * insertion and so back pressure to host block from kernel is not issued.
 *
 * \details 
 * '0': IPG insertion in MAC10G is enabled.
 * '1': IPG insertion in MAC10G is disabled.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_MODE_CFG . DISABLE_DIC
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_MODE_CFG_DISABLE_DIC  VTSS_BIT(0)


/** 
 * \brief Max Length Configuration Register
 *
 * \details
 * Register: \a LINE_MAC:CONFIG:MAC_MAXLEN_CFG
 */
#define VTSS_LINE_MAC_CONFIG_MAC_MAXLEN_CFG  VTSS_IOREG(0x03, 1, 0xf202)

/** 
 * \brief
 * Configures whether the Max Length Check takes the number of Q tags into
 * consideration when assing if a frame is too long.
 * 
 * If asserted, 
 *  - 4 bytes will be added to MAX_LEN for single tagged frame.
 *  - 8 bytes will be added to MAX_LEN for double tagged frame.
 *  - 12 bytes will be added to MAX_LEN for tripple tagged frame.
 *
 * \details 
 * '0' : Only check max frame length against MAX_LEN
 * '1' : Add 4/8/12 bytes to MAX_LEN when checking for max frame length
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_MAXLEN_CFG . MAX_LEN_TAG_CHK
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK  VTSS_BIT(16)

/** 
 * \brief
 * The maximum frame length accepted by the Receive Module. If the length
 * is exceeded, this is indicated in the Statistics Engine (LONG_FRAME).
 * The maximum length is automatically adjusted to accommodate maximum
 * sized frames containing a VLAN tag - given that the MAC is configured to
 * be VLAN aware (default):
 * 
 * MTU size = 10056 Bytes. This includes all encapsulations and TAGs. Not
 * including IFH.
 *  
 * Reason is: QS supports a max of 63 segments. A segment is 160 Bytes. The
 * IFH must be stored in the QS also, so room must be allocated.
 * Thereby:
 * 63 x 160B - 24B (IFH) = 10056 Bytes
 *
 * \details 
 * The maximum allowable size is 10056 Bytes.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_MAXLEN_CFG . MAX_LEN
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_MAXLEN_CFG_MAX_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_LINE_MAC_CONFIG_MAC_MAXLEN_CFG_MAX_LEN     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_LINE_MAC_CONFIG_MAC_MAXLEN_CFG_MAX_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Register to specify no.of tags supoorted
 *
 * \details
 * Register: \a LINE_MAC:CONFIG:MAC_NUM_TAGS_CFG
 */
#define VTSS_LINE_MAC_CONFIG_MAC_NUM_TAGS_CFG  VTSS_IOREG(0x03, 1, 0xf203)

/** 
 * \brief
 * Configuration for number of consecutive VLAN Tags supported by the MAC.
 * Maximum value is 3.
 *
 * \details 
 * '0': No tags are detected by MAC.
 * 'n': Maximum of n consecutive VLAN Tags are detected by the MAC and
 * accordingly MAX LEN is modified for frame length calculations.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_NUM_TAGS_CFG . NUM_TAGS
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_NUM_TAGS_CFG_NUM_TAGS(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_LINE_MAC_CONFIG_MAC_NUM_TAGS_CFG_NUM_TAGS     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_LINE_MAC_CONFIG_MAC_NUM_TAGS_CFG_NUM_TAGS(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief VLAN / Service tag configuration register
 *
 * \details
 * The MAC can be configured to accept 0, 1, 2 and 3 tags and the TAG value
 * can be user-defined.
 *
 * Register: \a LINE_MAC:CONFIG:MAC_TAGS_CFG
 *
 * @param ri Register: MAC_TAGS_CFG, 0-2
 */
#define VTSS_LINE_MAC_CONFIG_MAC_TAGS_CFG(ri)  VTSS_IOREG(0x03, 1, 0xf204 | (ri))

/** 
 * \brief
 * This field defines which value is regarded as a VLAN/Service tag -
 * besides 0x8100 and 0x88A8. The value is used for all ALL tag positions.
 * I.e. a double tagged frame can have the following tag values:
 * (INNER_TAG, OUTER_TAG):
 * ( 0x8100, 0x8100 )
 * ( 0x8100, TAG_ID ) or
 * ( TAG_ID, TAG_ID )
 *
 * \details 
 * "0x8100" - Standard Ethernet Bridge ethertype (C-tag)
 * "0x88A8" - Provider Bridge Ethertype  (S-tag)
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_TAGS_CFG . TAG_ID
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_TAGS_CFG_TAG_ID(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_LINE_MAC_CONFIG_MAC_TAGS_CFG_TAG_ID     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_LINE_MAC_CONFIG_MAC_TAGS_CFG_TAG_ID(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Enables TAG_ID apart from standard 0x8100 and 0x88A8 for Tag
 * comparision.
 *
 * \details 
 * '0': The MAC doesn't take TAG_ID for tag identification.
 * '1': The MAC looks for tag according to encoding of TAG_ID
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_TAGS_CFG . TAG_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_TAGS_CFG_TAG_ENA  VTSS_BIT(4)


/** 
 * \brief Advanced Check Feature Configuration Register
 *
 * \details
 * Register: \a LINE_MAC:CONFIG:MAC_ADV_CHK_CFG
 */
#define VTSS_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG  VTSS_IOREG(0x03, 1, 0xf207)

/** 
 * \brief
 * Extended End Of Packet Check: Specifies the requirement for the Rx
 * column when holding an EOP character.
 *
 * \details 
 * '0': The values of the remaining Rx lanes of a column holding an EOP are
 * ignored. E.g. if lane 1 holds an EOP, the value of lanes 2 and 3 are
 * ignored
 * '1': A received frame is error-marked if an Error character is received
 * in any lane of the column holding the EOP character. E.g. if lane 1
 * holds an EOP, the frame is error-marked if lanes 0, 2, or 3 holds an
 * Error character.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG . EXT_EOP_CHK_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Extended Start Of Packet Check Enable: Specifies the requirement for the
 * Rx column prior to SOP character.
 *
 * \details 
 * '0': Value of Rx column at the XGMII interface prior to a SOP character
 * is ignored
 * '1': An IDLE column at the XGMII interface must be received prior to a
 * SOP character for the MAC to detect a start of frame.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG . EXT_SOP_CHK_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_EXT_SOP_CHK_ENA  VTSS_BIT(20)

/** 
 * \brief
 * Start-of-Frame-Delimiter Check Enable: Specifies the requirement for a
 * successful frame reception.
 * When disabled (='0'), MAC10G will assume that preamble is 8 bytes (incl.
 * SOP & SFD) when SOP is received. No checking of SFD is carried out.
 * When enabled (='1'), MAC10G will search for SFD in lane 3/7 after
 * reception of SOP, before accepting frame data. MAC10G will search for
 * SFD until SFD is found or a control character is encountered.
 *
 * \details 
 * '0': Skip SFD check
 * '1': Strict SFD check enabled, i.e. the SFD must be "D5" for a
 * successful frame reception.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG . SFD_CHK_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_SFD_CHK_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Preamble Check Enable: Specifies the preamble requirement for a
 * successful frame reception.
 *
 * \details 
 * '0': Skip preamble check. A SOP control character is sufficient for a
 * successful frame reception. The minimum allowed preamble size is still 8
 * bytes (incl. SOP and SFD) but the preamble bytes between the SOP and SFD
 * can have any data value.
 * '1': Strict preamble check enabled, i.e. the last 6 bytes of a preamble
 * - prior to the SFD - must all be equal to 0x55 for a successful frame
 * reception. For preambles larger than 8 bytes, only the last 6 preamble
 * bytes prior to the SFD are checked when this bit is set to 1.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG . PRM_CHK_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_PRM_CHK_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Out-Of-Range Error Check Enable: Determines whether or not a received
 * frame should be discarded if the frame length field is out of range.
 *
 * \details 
 * '0': Out-of-range errors are ignored
 * '1': A frame is discarded if the frame length field value is out of
 * range
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG . OOR_ERR_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_OOR_ERR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * In-Range Error Check Enable: Determines whether or not a received frame
 * should be discarded if the frame length does not match the frame PDU
 * size:
 *
 * \details 
 * '0':
 * Frames which have a frame length field inconsistent with the actual
 * frame length are not error-marked
 * '1': Frames with inconsistent frame length fields are error marked and
 * will be discarded by the Rx Queue System.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG . INR_ERR_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_INR_ERR_ENA  VTSS_BIT(0)


/** 
 * \brief Link Fault Signaling Register
 *
 * \details
 * Register: \a LINE_MAC:CONFIG:MAC_LFS_CFG
 */
#define VTSS_LINE_MAC_CONFIG_MAC_LFS_CFG     VTSS_IOREG(0x03, 1, 0xf208)

/** 
 * \brief
 * Link Fault Signaling - Inhibit Tx:\nDetermine the behavior when a
 * (remote or local) Link Fault has been detected and LFS_MODE is set.
 *
 * \details 
 * '0': The transmitter continues to request frames from the FIFO. These
 * frames will not be transmitted on the XGMII interface, because LFS
 * requires special Sequence Ordered Sets to be transmitted during a Link
 * Fault. Consequently the transmitter will drop all frames from the FIFO.
 * '1': When a Link Fault has been detected, the transmitter will cease to
 * request frames from the FIFO. When the receiver has cleared the Link
 * Fault state, the transmitter will automatically start to request and
 * successfully transmit frame again - if LFS_DIS_TX is not set.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_LFS_CFG . LFS_INH_TX
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_LFS_CFG_LFS_INH_TX  VTSS_BIT(8)

/** 
 * \brief
 * Link Fault Signaling, DisableTransmitter:\nDetermines the behavior when
 * a (remote or local) Link Fault has been detected and both LFS_MODE and
 * LFS_INH_TX are asserted:
 *
 * \details 
 * '0': The transmitter continues to be enabled during and after a Link
 * Fault
 * '1': When a Link Fault has been detected, the transmitter will
 * automatically be disabled. S/W must actively enable the transmitter
 * again by de-asserting TX_ENA followed by an assertion of TX_ENA.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_LFS_CFG . LFS_DIS_TX
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_LFS_CFG_LFS_DIS_TX  VTSS_BIT(4)

/** 
 * \brief
 * LFS Unidirectional mode enable. Implementation of 802.3 clause 66. When
 * asserted, this enables MAC to transmit data during reception of Local
 * Fault and Remote Fault ordered sets from the PHY. 
 * 
 * When in Unidirectional mode:
 * When receiving LF, frames are transmitted separated by RF ordered sets.
 * When receiving RF, frames are transmitted separated by IDLE symbols
 *
 * \details 
 * '0' : LFS unidirectional mode is disabled
 * '1' : LFS unidirectional mode is enabled
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_LFS_CFG . LFS_UNIDIR_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_LFS_CFG_LFS_UNIDIR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Link Fault Signaling Mode Enable: If enabled, the transmitter reacts on
 * received Link Fault indications.
 *
 * \details 
 * 
 * '0': Ignore Link Faults detected by the MAC receiver module
 * '1': React on detected Link Faults and transmit the appropriate Sequence
 * Ordered Set.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_LFS_CFG . LFS_MODE_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_LFS_CFG_LFS_MODE_ENA  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Configuration Register
 *
 * \details
 * Register: \a LINE_MAC:CONFIG:MAC_LB_CFG
 */
#define VTSS_LINE_MAC_CONFIG_MAC_LB_CFG      VTSS_IOREG(0x03, 1, 0xf209)

/** 
 * \brief
 * Enables loopback from egress to ingress in the device. The MAC Rx clock
 * is automatically set equal to the MAC Tx clock when the loop back is
 * enabled.
 *
 * \details 
 * '0': Host loopback disabled at XGMII interface.
 * '1': Host loopback enabled at XGMII interface.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_LB_CFG . XGMII_HOST_LB_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_LB_CFG_XGMII_HOST_LB_ENA  VTSS_BIT(4)


/** 
 * \brief Packet Interface Configuration
 *
 * \details
 * This register bits configures packet interface module
 *
 * Register: \a LINE_MAC:CONFIG:MAC_PKTINF_CFG
 */
#define VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG  VTSS_IOREG(0x03, 1, 0xf20a)

/** 
 * \brief
 * Enables stripping of FCS in Ingress traffic.
 *
 * \details 
 * '0': FCS is not stripped.
 * '1': FCS is stripped in Ingress.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . STRIP_FCS_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_STRIP_FCS_ENA  VTSS_BIT(0)

/** 
 * \brief
 * Enables FCS insertion in Egress traffic.
 *
 * \details 
 * '0': FCS is not added.
 * '1': FCS is added in Egress direction.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . INSERT_FCS_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_INSERT_FCS_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enables stripping of preamble from MAC frame in the Ingress direction
 *
 * \details 
 * '0': Preamble is unaltered.
 * '1': Preamble is stripped in Ingress direction.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . STRIP_PREAMBLE_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_STRIP_PREAMBLE_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enables addition of standard preamble in Egress direction
 *
 * \details 
 * '0': Standard Preamble is not inserted.
 * '1': Standard preamble is added in Egress direction.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . INSERT_PREAMBLE_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_INSERT_PREAMBLE_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Enables signaling of LPI received.
 *
 * \details 
 * '0': Disable LPI received status.
 * '1': Enable LPI received stauts signaling.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . LPI_RELAY_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_LPI_RELAY_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Enables signaling of Local Fault state.
 *
 * \details 
 * '0': Disable signaling of Local Fault state.
 * '1': Enable Local Fault state signaling.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . LF_RELAY_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_LF_RELAY_ENA  VTSS_BIT(20)

/** 
 * \brief
 * Enables signaling of Remote Fault state.
 *
 * \details 
 * '0': Disable signaling of Remote Fault state.
 * '1': Enable Remote Fault state signaling.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . RF_RELAY_ENA
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_RF_RELAY_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Enables padding to frames whose length is less than 64 while
 * transmitting. Padded bits will be all zeros.
 *
 * \details 
 * '0': Disable padding.
 * '1': Enable padding.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . ENABLE_TX_PADDING
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_ENABLE_TX_PADDING  VTSS_BIT(25)

/** 
 * \brief
 * Enables padding to frames whose length is less than 64 when received.
 * Padded bits will be all zeros.
 *
 * \details 
 * '0': Disable padding.
 * '1': Enable padding.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . ENABLE_RX_PADDING
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_ENABLE_RX_PADDING  VTSS_BIT(26)

/** 
 * \brief
 * Enables insertion of 4 byte preamble if INSERT_PREAMBLE_ENA is set.
 * Followed by 4 byte preameble is DMAC.
 * 
 * Preamble will be 4 bytes only if per frame signal
 * host_tx_4byte_preamble_i (at MAC10G packet interface) is also asserted
 * along with this configuration.
 *
 * \details 
 * '0': Disable 4 byte preamble.
 * '1': Enable insertion of 4 byte preamble.
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . ENABLE_4BYTE_PREAMBLE
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_ENABLE_4BYTE_PREAMBLE  VTSS_BIT(27)

/** 
 * \brief
 * Stalling will be used for 1588 Timestamped frame. This will ensure that
 * Timestamped frames will undergo fixed latency through MAC block.
 * 
 * This configuration specifies no.of clocks to stall for achieving fixed
 * latency in MACsec bypass mode.
 * 
 * This is in terms of enabled clock cycles and recommended value is 2.

 *
 * \details 
 * 0 : Stalling is disabled.
 * 1 : 1 clock stall is generated
 * ...
 * n : n clocks stall is generated
 *
 * Field: VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG . MACSEC_BYPASS_NUM_PTP_STALL_CLKS
 */
#define  VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_MACSEC_BYPASS_NUM_PTP_STALL_CLKS(x)  VTSS_ENCODE_BITFIELD(x,28,3)
#define  VTSS_M_LINE_MAC_CONFIG_MAC_PKTINF_CFG_MACSEC_BYPASS_NUM_PTP_STALL_CLKS     VTSS_ENCODE_BITMASK(28,3)
#define  VTSS_X_LINE_MAC_CONFIG_MAC_PKTINF_CFG_MACSEC_BYPASS_NUM_PTP_STALL_CLKS(x)  VTSS_EXTRACT_BITFIELD(x,28,3)

/**
 * Register Group: \a LINE_MAC:PAUSE_CFG
 *
 * MAC 10G Pause configurartion registers
 */


/** 
 * \brief Transmit Pause Frame Control
 *
 * \details
 * Register: \a LINE_MAC:PAUSE_CFG:PAUSE_TX_FRAME_CONTROL
 */
#define VTSS_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL  VTSS_IOREG(0x03, 1, 0xf20b)

/** 
 * \brief
 * Pause value to be used when generating pause frames, except XON frames
 * in mode 2
 *
 * \details 
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL . MAC_TX_PAUSE_VALUE
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_VALUE(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_VALUE     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Enables pause-generate module to wait for 10 clocks (for IDLE insertion)
 * before generating XOFF pause frame if MAC 10G is transmitting LPI Idles.
 * This bit should be set only if LPI generation is forced in Kernel 10G
 * and a pause frame needs to be transmitted.
 *
 * \details 
 * 0 = No IDLES are inserted before pause frame
 * 1 = Idles are inserted before pause frame
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL . MAC_TX_WAIT_FOR_LPI_LOW
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_MAC_TX_WAIT_FOR_LPI_LOW  VTSS_BIT(12)

/** 
 * \brief
 * Enables generation of stall signal when inserting XOFF/XON pause frame
 * into transmission stream or MAC Tx is in pause state
 * This can be used to upper blocks as clock enables so that their
 * pipe-line is pause.
 *
 * \details 
 * 0 = Disable stall generation
 * 1 = Enable stall generation
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL . MAC_TX_USE_PAUSE_STALL_ENA
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_MAC_TX_USE_PAUSE_STALL_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Determines the mode that the pause frame generator operates in
 *
 * \details 
 * 0 = Pause frame generation is disabled
 * 1 = Pause frames are generated only with the pause-value specified in
 * the MAC_PAUSE_VALUE register
 * 2 = XON mode, pause frames are generated with a pause-value of 0 are
 * generated when traffic is to be restarted, in addition to generating
 * pause frames as in mode 1
 * 3 = reserved

 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL . MAC_TX_PAUSE_MODE
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_MODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Transmit Pause Frame Control Part 2
 *
 * \details
 * Register: \a LINE_MAC:PAUSE_CFG:PAUSE_TX_FRAME_CONTROL_2
 */
#define VTSS_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_2  VTSS_IOREG(0x03, 1, 0xf20c)

/** 
 * \brief
 * This field determines the interval at which pause frames will be
 * generated.  Each count in the pause frame interval value corresponds to
 * one cycle of the MAC clock (PCS clock divided by 2) which would
 * typically be 156.25 MHz (6.4 ns period)
 * 
 * The interval is counted from the end of one pause frame to the beginning
 * of the next (assuming no other TX traffic)
 * 
 * In TX pause mode 2 only, the internal pause interval timer will be
 * cleared whenever an XON pause frame is sent.
 * 
 * Note that the pause interval value of 0xffff gives the same pause frame
 * interval as the pause interval value of 0xfffe.  Also, do not use a
 * value of 0.
 *
 * \details 
 * Pause interval
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_2 . MAC_TX_PAUSE_INTERVAL
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_2_MAC_TX_PAUSE_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_2_MAC_TX_PAUSE_INTERVAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_LINE_MAC_PAUSE_CFG_PAUSE_TX_FRAME_CONTROL_2_MAC_TX_PAUSE_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Receive Pause Frame Control
 *
 * \details
 * Register: \a LINE_MAC:PAUSE_CFG:PAUSE_RX_FRAME_CONTROL
 */
#define VTSS_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL  VTSS_IOREG(0x03, 1, 0xf20d)

/** 
 * \brief
 * Enables pause frame detection at XGMII interface.
 *
 * \details 
 * 0 = Disable pause frame detection at XGMII interface
 * 1 = Enables pause frame detection at XGMII interface
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL . MAC_RX_EARLY_PAUSE_DETECT_ENA
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_EARLY_PAUSE_DETECT_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Configuration bit for XOFF indication before CRC check to meet pause
 * reaction time.
 * XOFF detection will be done at XGMII interface depending on
 * MAC_RX_EARLY_PAUSE_DETECT_ENA.
 * Information of CRC check failed for the XOFF pause frame is also passed
 * with a seperate side band signal and so pause timer will be re-loaded
 * with previous pause value. 
 * 
 * NOTE: If XOFF detection is done after MAC then this bit is unused.
 *
 * \details 
 * 0 = XOFF indication at XGMII is done after CRC check.
 * 1 = XOFF indication ar XGMII is done before CRC check.
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL . MAC_RX_PRE_CRC_MODE
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_PRE_CRC_MODE  VTSS_BIT(20)

/** 
 * \brief
 * Enables pause timer implementation in MAC rx clock domain for the
 * received pause frame. When Signal rx_pause_state_o is asserted while
 * pause timer is running.
 *
 * \details 
 * 0 = Disable pause timer implementation
 * 1 = Enables pause timer implementaion
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL . MAC_RX_PAUSE_TIMER_ENA
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_TIMER_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Enables pausing of transmission when a pause frame is received.
 *
 * \details 
 * 0 = Disable pause reaction
 * 1 = Enables pause reaction
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL . MAC_TX_PAUSE_REACT_ENA
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_TX_PAUSE_REACT_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enables dropping of pause frames in the Pause Frame Detector.
 *
 * \details 
 * 0 = Pause frames are not dropped
 * 1 = Pause frames are dropped
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL . MAC_RX_PAUSE_FRAME_DROP_ENA
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_FRAME_DROP_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Controls pause frame detection in receive path
 *
 * \details 
 * 0 = Pause frame detection is disabled
 * 1 = Pause frame detection is enabled

 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL . MAC_RX_PAUSE_MODE
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_MODE  VTSS_BIT(0)


/** 
 * \brief Pause Detector State Register
 *
 * \details
 * Register: \a LINE_MAC:PAUSE_CFG:PAUSE_STATE
 */
#define VTSS_LINE_MAC_PAUSE_CFG_PAUSE_STATE  VTSS_IOREG(0x03, 1, 0xf20e)

/** 
 * \brief
 * Pause state indicator.  Interface is paused when the pause timer is a
 * non-zero value
 *
 * \details 
 * 0 = Not Paused
 * 1 = Paused
 *
 * Field: VTSS_LINE_MAC_PAUSE_CFG_PAUSE_STATE . PAUSE_STATE
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_STATE_PAUSE_STATE  VTSS_BIT(0)


/** 
 * \brief MAC Address LSB
 *
 * \details
 * Register: \a LINE_MAC:PAUSE_CFG:MAC_ADDRESS_LSB
 */
#define VTSS_LINE_MAC_PAUSE_CFG_MAC_ADDRESS_LSB  VTSS_IOREG(0x03, 1, 0xf20f)


/** 
 * \brief MAC Address MSB
 *
 * \details
 * Register: \a LINE_MAC:PAUSE_CFG:MAC_ADDRESS_MSB
 */
#define VTSS_LINE_MAC_PAUSE_CFG_MAC_ADDRESS_MSB  VTSS_IOREG(0x03, 1, 0xf210)

/** 
 * \brief
 * Upper 16 bits of the MAC address
 *
 * \details 
 * Field: VTSS_LINE_MAC_PAUSE_CFG_MAC_ADDRESS_MSB . MAC_ADDRESS_MSB
 */
#define  VTSS_F_LINE_MAC_PAUSE_CFG_MAC_ADDRESS_MSB_MAC_ADDRESS_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_LINE_MAC_PAUSE_CFG_MAC_ADDRESS_MSB_MAC_ADDRESS_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_LINE_MAC_PAUSE_CFG_MAC_ADDRESS_MSB_MAC_ADDRESS_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a LINE_MAC:STATUS
 *
 * MAC10G Configuration and Status Registers
 */


/** 
 * \brief XGMII Lane Debug Sticky bit Register 0
 *
 * \details
 * Register: \a LINE_MAC:STATUS:MAC_RX_LANE_STICKY_0
 */
#define VTSS_LINE_MAC_STATUS_MAC_RX_LANE_STICKY_0  VTSS_IOREG(0x03, 1, 0xf211)


/** 
 * \brief XGMII Lane Debug Sticky bit Register 1
 *
 * \details
 * Register: \a LINE_MAC:STATUS:MAC_RX_LANE_STICKY_1
 */
#define VTSS_LINE_MAC_STATUS_MAC_RX_LANE_STICKY_1  VTSS_IOREG(0x03, 1, 0xf212)


/** 
 * \brief MAC10G Tx Monitor Sticky bit Register
 *
 * \details
 * Register: \a LINE_MAC:STATUS:MAC_TX_MONITOR_STICKY
 */
#define VTSS_LINE_MAC_STATUS_MAC_TX_MONITOR_STICKY  VTSS_IOREG(0x03, 1, 0xf213)


/** 
 * \brief MAC10G Tx Monitor interrupt mask register
 *
 * \details
 * Register: \a LINE_MAC:STATUS:MAC_TX_MONITOR_STICKY_MASK
 */
#define VTSS_LINE_MAC_STATUS_MAC_TX_MONITOR_STICKY_MASK  VTSS_IOREG(0x03, 1, 0xf214)


/** 
 * \brief Sticky Bit Register
 *
 * \details
 * Clear the sticky bits by writing a '0' in the relevant bitgroups
 * (writing a '1' sets the bit)!.
 *
 * Register: \a LINE_MAC:STATUS:MAC_STICKY
 */
#define VTSS_LINE_MAC_STATUS_MAC_STICKY      VTSS_IOREG(0x03, 1, 0xf215)

/** 
 * \brief
 * Indicates that an inter packet gap shrink was detected (IPG < 12 bytes).
 *
 * \details 
 * '0': no ipg shrink was detected
 * '1': one or more ipg shrink were detected
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . RX_IPG_SHRINK_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_RX_IPG_SHRINK_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Indicates that a preamble shrink was detected (preamble < 8 bytes).
 * This sticky bit can only be set when the port is setup in 10 Gbps mode,
 * where frames with e.g. a 4 bytes preamble will be discarded, and it
 * requires that PRM_SHK_CHK_DIS = 0 and SFD_CHK_ENA = 1.
 * In SGMII mode, all preamble sizes down to 3 bytes (including SFD) are
 * accepted and will not cause this sticky bit to be set.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . RX_PREAM_SHRINK_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_RX_PREAM_SHRINK_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * If Preamble Check is enabled and a SOP is received, this bit is set if
 * the following bytes do not match a "5555555555..55D5" pattern.
 * A 12 bytes preamble of "55555555.55555555.555555D5" will not cause this
 * sticky bit to be set.
 * 
 * This sticky bit can only be set when the port is setup in 10 Gbps mode.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . RX_PREAM_MISMATCH_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_RX_PREAM_MISMATCH_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * If a SOP is received and a following control character is received
 * within the preamble, this bit is set. (No data is passed to the host
 * interface of the MAC).
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . RX_PREAM_ERR_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_RX_PREAM_ERR_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Indicates that a frame was received with a non-standard preamble.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . RX_NON_STD_PREAM_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_RX_NON_STD_PREAM_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Indicates that a frame with MPLS multicast was received.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . RX_MPLS_MC_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_RX_MPLS_MC_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Indicates that a frame with MPLS unicast was received.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . RX_MPLS_UC_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_RX_MPLS_UC_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Indicates that a frame was received with a VLAN tag.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . RX_TAG_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_RX_TAG_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Sticky bit indicating that the MAC Transmit FIFO has dropped one or more
 * frames because of underrun.
 *
 * \details 
 * '0': No MAC Tx FIFO underrun has occured
 * '1': One or more MAC Tx FIFO underruns have occured
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . TX_UFLW_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_TX_UFLW_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Indicates that the transmit host initiated abort was executed.
 *
 * \details 
 * '0': no tx frame was aborted
 * '1': one or more tx frames were aborted
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY . TX_ABORT_STICKY
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_TX_ABORT_STICKY  VTSS_BIT(0)


/** 
 * \brief MAC Sticky Bits Interrupt Mask Register
 *
 * \details
 * Register: \a LINE_MAC:STATUS:MAC_STICKY_MASK
 */
#define VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK  VTSS_IOREG(0x03, 1, 0xf216)

/** 
 * \brief
 * Interrupt mask for RX_IPG_SHRINK_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . RX_IPG_SHRINK_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_RX_IPG_SHRINK_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Interrupt mask for RX_PREAM_SHRINK_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . RX_PREAM_SHRINK_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_RX_PREAM_SHRINK_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Interrupt mask for RX_PREAM_MISMATCH_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . RX_PREAM_MISMATCH_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_RX_PREAM_MISMATCH_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Interrupt mask for RX_PREAM_ERR_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . RX_PREAM_ERR_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_RX_PREAM_ERR_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Interrupt mask for RX_NON_STD_PREAM_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . RX_NON_STD_PREAM_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_RX_NON_STD_PREAM_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Interrupt mask for RX_MPLS_MC_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . RX_MPLS_MC_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_RX_MPLS_MC_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Interrupt mask for RX_MPLS_UC_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . RX_MPLS_UC_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_RX_MPLS_UC_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Interrupt mask for RX_TAG_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . RX_TAG_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_RX_TAG_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Interrupt mask for TX_UFLW_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . TX_UFLW_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_TX_UFLW_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Interrupt mask for TX_ABORT_STICKY
 *
 * \details 
 * '0': Disable interrupt
 * '1': Enable interrupt
 *
 * Field: VTSS_LINE_MAC_STATUS_MAC_STICKY_MASK . TX_ABORT_STICKY_MASK
 */
#define  VTSS_F_LINE_MAC_STATUS_MAC_STICKY_MASK_TX_ABORT_STICKY_MASK  VTSS_BIT(0)

/**
 * Register Group: \a LINE_MAC:STATISTICS_32BIT
 *
 * Not documented
 */


/** 
 * \brief RX_HIH checksum error counter
 *
 * \details
 * If HIH CRC checking is enabled, this counter counts the number of frames
 * discarded because of HIH CRC errors.
 *
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_HIH_CKSM_ERR_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_HIH_CKSM_ERR_CNT  VTSS_IOREG(0x03, 1, 0xf217)


/** 
 * \brief Rx XGMII protocol error counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_XGMII_PROT_ERR_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_XGMII_PROT_ERR_CNT  VTSS_IOREG(0x03, 1, 0xf218)


/** 
 * \brief Rx symbol carrier error counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_SYMBOL_ERR_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_SYMBOL_ERR_CNT  VTSS_IOREG(0x03, 1, 0xf219)


/** 
 * \brief Rx pause frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_PAUSE_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_PAUSE_CNT  VTSS_IOREG(0x03, 1, 0xf21a)


/** 
 * \brief Rx control frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_UNSUP_OPCODE_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_UNSUP_OPCODE_CNT  VTSS_IOREG(0x03, 1, 0xf21b)


/** 
 * \brief Rx unicast frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_UC_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_UC_CNT  VTSS_IOREG(0x03, 1, 0xf21c)


/** 
 * \brief Rx multicast frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_MC_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_MC_CNT  VTSS_IOREG(0x03, 1, 0xf21d)


/** 
 * \brief Rx broadcast frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_BC_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_BC_CNT  VTSS_IOREG(0x03, 1, 0xf21e)


/** 
 * \brief Rx CRC error counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_CRC_ERR_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_CRC_ERR_CNT  VTSS_IOREG(0x03, 1, 0xf21f)


/** 
 * \brief Rx undersize counter (valid frame format)
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_UNDERSIZE_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_UNDERSIZE_CNT  VTSS_IOREG(0x03, 1, 0xf220)


/** 
 * \brief Rx undersize counter (CRC error)
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_FRAGMENTS_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_FRAGMENTS_CNT  VTSS_IOREG(0x03, 1, 0xf221)


/** 
 * \brief Rx in-range length error counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_IN_RANGE_LEN_ERR_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_IN_RANGE_LEN_ERR_CNT  VTSS_IOREG(0x03, 1, 0xf222)


/** 
 * \brief Rx out-of-range length error counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_OUT_OF_RANGE_LEN_ERR_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_OUT_OF_RANGE_LEN_ERR_CNT  VTSS_IOREG(0x03, 1, 0xf223)


/** 
 * \brief Rx oversize counter (valid frame format)
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_OVERSIZE_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_OVERSIZE_CNT  VTSS_IOREG(0x03, 1, 0xf224)


/** 
 * \brief Rx jabbers counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_JABBERS_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_JABBERS_CNT  VTSS_IOREG(0x03, 1, 0xf225)


/** 
 * \brief Rx 64 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_SIZE64_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_SIZE64_CNT  VTSS_IOREG(0x03, 1, 0xf226)


/** 
 * \brief Rx 65-127 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_SIZE65TO127_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_SIZE65TO127_CNT  VTSS_IOREG(0x03, 1, 0xf227)


/** 
 * \brief Rx 128-255 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_SIZE128TO255_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_SIZE128TO255_CNT  VTSS_IOREG(0x03, 1, 0xf228)


/** 
 * \brief Rx 256-511 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_SIZE256TO511_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_SIZE256TO511_CNT  VTSS_IOREG(0x03, 1, 0xf229)


/** 
 * \brief Rx 512-1023 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_SIZE512TO1023_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_SIZE512TO1023_CNT  VTSS_IOREG(0x03, 1, 0xf22a)


/** 
 * \brief Rx 1024-1518 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_SIZE1024TO1518_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_SIZE1024TO1518_CNT  VTSS_IOREG(0x03, 1, 0xf22b)


/** 
 * \brief Rx 1519 to max. length byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_SIZE1519TOMAX_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_SIZE1519TOMAX_CNT  VTSS_IOREG(0x03, 1, 0xf22c)


/** 
 * \brief Rx inter packet gap shrink counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:RX_IPG_SHRINK_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_RX_IPG_SHRINK_CNT  VTSS_IOREG(0x03, 1, 0xf22d)


/** 
 * \brief Tx pause frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_PAUSE_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_PAUSE_CNT  VTSS_IOREG(0x03, 1, 0xf22e)


/** 
 * \brief Tx unicast frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_UC_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_UC_CNT  VTSS_IOREG(0x03, 1, 0xf22f)


/** 
 * \brief Tx multicast frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_MC_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_MC_CNT  VTSS_IOREG(0x03, 1, 0xf230)


/** 
 * \brief Tx broadcast frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_BC_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_BC_CNT  VTSS_IOREG(0x03, 1, 0xf231)


/** 
 * \brief Tx 64 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_SIZE64_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_SIZE64_CNT  VTSS_IOREG(0x03, 1, 0xf232)


/** 
 * \brief Tx 65-127 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_SIZE65TO127_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_SIZE65TO127_CNT  VTSS_IOREG(0x03, 1, 0xf233)


/** 
 * \brief Tx 128-255 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_SIZE128TO255_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_SIZE128TO255_CNT  VTSS_IOREG(0x03, 1, 0xf234)


/** 
 * \brief Tx 256-511 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_SIZE256TO511_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_SIZE256TO511_CNT  VTSS_IOREG(0x03, 1, 0xf235)


/** 
 * \brief Tx 512-1023 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_SIZE512TO1023_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_SIZE512TO1023_CNT  VTSS_IOREG(0x03, 1, 0xf236)


/** 
 * \brief Tx 1024-1518 byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_SIZE1024TO1518_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_SIZE1024TO1518_CNT  VTSS_IOREG(0x03, 1, 0xf237)


/** 
 * \brief Tx 1519 to max. length byte frame counter
 *
 * \details
 * Register: \a LINE_MAC:STATISTICS_32BIT:TX_SIZE1519TOMAX_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_32BIT_TX_SIZE1519TOMAX_CNT  VTSS_IOREG(0x03, 1, 0xf238)

/**
 * Register Group: \a LINE_MAC:STATISTICS_40BIT
 *
 * Not documented
 */


/** 
 * \brief Rx bad bytes counter (LSB)
 *
 * \details
 * The number of received bytes in bad frames (LSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:RX_BAD_BYTES_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_RX_BAD_BYTES_CNT  VTSS_IOREG(0x03, 1, 0xf239)


/** 
 * \brief Rx bad bytes counter (MSB)
 *
 * \details
 * The number of received bytes in bad frames (MSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:RX_BAD_BYTES_MSB_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT  VTSS_IOREG(0x03, 1, 0xf23a)

/** 
 * \brief
 * The number of received bytes in bad frames (MSBs only).
 *
 * \details 
 * Counter can be written by software.
 *
 * Field: VTSS_LINE_MAC_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT . RX_BAD_BYTES_MSB_CNT
 */
#define  VTSS_F_LINE_MAC_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_LINE_MAC_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_LINE_MAC_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Rx OK bytes counter (LSB)
 *
 * \details
 * The number of received bytes in good frames (LSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:RX_OK_BYTES_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_CNT  VTSS_IOREG(0x03, 1, 0xf23b)


/** 
 * \brief Rx OK bytes counter (MSB)
 *
 * \details
 * The number of received bytes in good frames (MSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:RX_OK_BYTES_MSB_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT  VTSS_IOREG(0x03, 1, 0xf23c)

/** 
 * \brief
 * The number of received bytes in good frames (MSBs only).
 *
 * \details 
 * Counter can be written by software.
 *
 * Field: VTSS_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT . RX_OK_BYTES_MSB_CNT
 */
#define  VTSS_F_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Rx bytes received counter (LSB)
 *
 * \details
 * The number of good, bad, and framing bytes received (LSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:RX_IN_BYTES_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_RX_IN_BYTES_CNT  VTSS_IOREG(0x03, 1, 0xf23d)


/** 
 * \brief Rx bytes received counter (MSB)
 *
 * \details
 * The number of good, bad, and framing bytes received (MSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:RX_IN_BYTES_MSB_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT  VTSS_IOREG(0x03, 1, 0xf23e)

/** 
 * \brief
 * The number of good, bad, and framing bytes received (MSBs only).
 *
 * \details 
 * Counter can be written by software.
 *
 * Field: VTSS_LINE_MAC_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT . RX_IN_BYTES_MSB_CNT
 */
#define  VTSS_F_LINE_MAC_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_LINE_MAC_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_LINE_MAC_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Tx OK bytes counter (LSB)
 *
 * \details
 * The number of bytes transmitted successfully (LSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:TX_OK_BYTES_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_CNT  VTSS_IOREG(0x03, 1, 0xf23f)


/** 
 * \brief Tx OK bytes counter (MSB)
 *
 * \details
 * The number of bytes transmitted successfully (MSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:TX_OK_BYTES_MSB_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT  VTSS_IOREG(0x03, 1, 0xf240)

/** 
 * \brief
 * The number of bytes transmitted successfully (MSBs only).
 *
 * \details 
 * Counter can be written by software.
 *
 * Field: VTSS_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT . TX_OK_BYTES_MSB_CNT
 */
#define  VTSS_F_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Tx bytes transmitted counter (LSB)
 *
 * \details
 * The number of good, bad, and framing bytes transmitted (LSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:TX_OUT_BYTES_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_TX_OUT_BYTES_CNT  VTSS_IOREG(0x03, 1, 0xf241)


/** 
 * \brief Tx bytes transmitted counter (MSB)
 *
 * \details
 * The number of good, bad, and framing bytes transmitted (MSBs only).
 *
 * Register: \a LINE_MAC:STATISTICS_40BIT:TX_OUT_BYTES_MSB_CNT
 */
#define VTSS_LINE_MAC_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT  VTSS_IOREG(0x03, 1, 0xf242)

/** 
 * \brief
 * The number of good, bad, and framing bytes transmitted (MSBs only).
 *
 * \details 
 * Counter can be written by software.
 *
 * Field: VTSS_LINE_MAC_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT . TX_OUT_BYTES_MSB_CNT
 */
#define  VTSS_F_LINE_MAC_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_LINE_MAC_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_LINE_MAC_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


#endif /* _VTSS_VENICE_REGS_LINE_MAC_H_ */
