/*

 Vitesse Switch API software.

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

*/

/******************************************************************************/
//
// This header file contains various definitions and helper functions for
// implementing the PTP Internal Management protocol (PIM).
//
// Please see Dsign document (DS1006) for further information.
//
// Ethernet frame structure for PIM:
//  ------------------------------------------------------------------------
// | DMAC     | SMAC     | EtherType | EPID     | PIM Header  | PIM Body    |
// | 6 octets | 6 octets | 2 octets  | 2 octets | 6 octets    | 0-n octets  |
//  ------------------------------------------------------------------------
// EtherType is 0x8880.
// EPID (Extended Protocol ID) is 0x0007.
//
// The following includes and typedefs are needed in order to compile this file:
//
// #include <string.h> /* memset, memcpy and size_t */
//
// typedef unsigned char      u8;
// typedef unsigned short     u16;
// typedef unsigned long      u32;
// typedef unsigned long long u64;
// typedef signed long long   i64;
//
// typedef struct
// {
//     u8 addr[6]; /* In network byte order */
// } vtss_mac_t;
//
// typedef u16 vtss_etype_t;
//
/******************************************************************************/

#ifndef _PTP_PIM_H_
#define _PTP_PIM_H_

#define PTP_PIM_DEFAULT_ETHERTYPE 0x8880
#define PTP_PIM_EPID              0x0008

/* Various length definitions in bytes */
#define PTP_PIM_MAC_HEADER_LENGTH               14 /* DMAC + SMAC + EtherType */
#define PTP_PIM_EPID_LENGTH                      2 /* EPID */
#define PTP_PIM_HEADER_LENGTH                    6 /* Version + MessageId + Command + MessageType + Length */

/* Message types */
#define PTP_PIM_MESSAGE_TYPE_REQUEST             0
#define PTP_PIM_MESSAGE_TYPE_REPLY               1
#define PTP_PIM_MESSAGE_TYPE_EVENT               2
#define PTP_PIM_MESSAGE_TYPE_CNT                 3 /* Number of message types */

/* commands */
#define PTP_PIM_COMMAND_1PPS                     1  /* for 1PPS transfer in binary format */
#define PTP_PIM_COMMAND_MODEM_DELAY              2
#define PTP_PIM_COMMAND_MODEM_PRE_DELAY          3
#define PTP_PIM_COMMAND_CNT                      4


/* PIM header definition */
typedef struct {
    u8     version;
    u8     message_id;
    u8     command;
    u8     message_type;
    u16    length;
} ptp_pim_header_t;


/******************************************************************************/
// Pack/unpack utilities.
/******************************************************************************/
static inline u16 PTP_PIM_unpack16(const u8 *buf)
{
    return (buf[0] << 8) + buf[1];
}

static inline void PTP_PIM_pack16(u16 v, u8 *buf)
{
    buf[0] = (v >> 8) & 0xff;
    buf[1] = v & 0xff;
}

static inline u32 PTP_PIM_unpack32(const u8 *buf)
{
    return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
}

static inline void PTP_PIM_pack32(u32 v, u8 *buf)
{
    buf[0] = (v >> 24) & 0xff;
    buf[1] = (v >> 16) & 0xff;
    buf[2] = (v >> 8) & 0xff;
    buf[3] = v & 0xff;
}

static inline i64 PTP_PIM_unpack64(const u8 *buf)
{
    int i;
    u64 v = 0;
    for (i = 0; i < 8; i++) {
        v = v << 8;
        v += buf[i];
    }
    return v;
}

static inline void PTP_PIM_pack64(i64 v, u8 *buf)
{
    int i;
    for (i = 7; i >= 0; i--) {
        buf[i] = v & 0xff;
        v = v >> 8;
    }
}

static inline size_t PTP_PIM_unpack_timestamp(const u8 *buf, vtss_timestamp_t *ts)
{
    // Ignore the two most significant bytes
    ts->sec_msb     = PTP_PIM_unpack16(buf);
    ts->seconds     = PTP_PIM_unpack32(buf + 2);
    ts->nanoseconds = PTP_PIM_unpack32(buf + 6);
    return 10; // Number of bytes in packed string
}

static inline size_t PTP_PIM_pack_timestamp(u8 *buf, const vtss_timestamp_t *ts)
{
    PTP_PIM_pack16(ts->sec_msb,     buf);
    PTP_PIM_pack32(ts->seconds,     buf + 2);
    PTP_PIM_pack32(ts->nanoseconds, buf + 6);
    return 10; // Number of bytes in packed string
}

/******************************************************************************/
// PTP_PIM_pack_mac_header()
// Convert mac fields to packed string of bytes.
// Returns number of bytes in packed string.
/******************************************************************************/
static inline size_t PTP_PIM_pack_mac_header(u8 *buf, const vtss_mac_t *dst, const vtss_mac_t *src, vtss_etype_t etype)
{
    memcpy(buf, dst, sizeof(vtss_mac_t));
    memcpy(buf + 6, src, sizeof(vtss_mac_t));
    PTP_PIM_pack16(etype, buf + 12);
    return PTP_PIM_MAC_HEADER_LENGTH;
}

/******************************************************************************/
// PTP_PIM_unpack_mac_header()
// Convert packed string of bytes to mac fields.
// Returns number of bytes in packed string.
/******************************************************************************/
static inline size_t PTP_PIM_unpack_mac_header(const u8 *buf, vtss_mac_t *dst, vtss_mac_t *src, vtss_etype_t *etype)
{
    memcpy(dst, buf, sizeof(vtss_mac_t));
    memcpy(src, buf + 6, sizeof(vtss_mac_t));
    *etype = PTP_PIM_unpack16(buf + 12);
    return PTP_PIM_MAC_HEADER_LENGTH;
}

/******************************************************************************/
// PTP_PIM_pack_header()
// Convert header struct to packed string of bytes.
// Returns number of bytes in packed string.
/******************************************************************************/
static inline size_t PTP_PIM_pack_header(u8 *buf, const ptp_pim_header_t *header)
{
    *buf       = header->version;
    *(buf + 1) = header->message_id;
    *(buf + 2) = header->command;
    *(buf + 3) = header->message_type;
    PTP_PIM_pack16(header->length, buf + 4);
    return PTP_PIM_HEADER_LENGTH;
}

/******************************************************************************/
// PTP_PIM_unpack_header()
// Convert packed string of bytes to header struct.
// Returns number of bytes in packed string.
/******************************************************************************/
static inline size_t PTP_PIM_unpack_header(const u8 *buf, ptp_pim_header_t *header)
{
    memset(header, 0, sizeof(*header));
    header->version      = *buf;
    header->message_id   = *(buf + 1);
    header->command      = *(buf + 2);
    header->message_type = *(buf + 3);
    header->length       = PTP_PIM_unpack16(buf + 4);
    return PTP_PIM_HEADER_LENGTH;
}

/******************************************************************************/
// Static variables
// Include them by defining the appropriate identifier before this header file
// is included.
/******************************************************************************/

#ifdef PTP_PIM_MESSAGE_TYPE_NAMES_DECLARE
/* Declare mapping of message_type to string for trace/debug purposes. */
static const char *const PTP_PIM_message_type_names[PTP_PIM_MESSAGE_TYPE_CNT] = {
    [PTP_PIM_MESSAGE_TYPE_REQUEST] = "Request",
    [PTP_PIM_MESSAGE_TYPE_REPLY]   = "Reply",
    [PTP_PIM_MESSAGE_TYPE_EVENT]   = "Event",
};
#endif /* PTP_PIM_MESSAGE_TYPE_NAMES_DECLARE */

#ifdef PTP_PIM_COMMAND_NAMES_DECLARE
/* Declare mapping of command to string for trace/debug purposes. */
static const char *const PTP_PIM_command_names[PTP_PIM_COMMAND_CNT] = {
    [PTP_PIM_COMMAND_1PPS]                       = "1PPS",
    [PTP_PIM_COMMAND_MODEM_DELAY]                = "ModemDelay",
};
#endif /* PTP_PIM_COMMAND_NAMES_DECLARE */


#endif /* _PTP_PIM_H_ */

/******************************************************************************/
//  End of file.
/******************************************************************************/
