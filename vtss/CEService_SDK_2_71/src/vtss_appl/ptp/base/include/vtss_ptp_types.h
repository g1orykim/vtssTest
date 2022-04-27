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

#ifndef _VTSS_PTP_TYPES_H_
#define _VTSS_PTP_TYPES_H_
/**
 * \file vtss_ptp_types.h
 * \brief PTP protocol engine type definitions file
 *
 * This file contain the definitions of types used by the API interface and
 * implementation.
 *
 */


#if defined(__LINUX__)

#include <sys/types.h>

/* Shorthand types from linux platform types */
typedef u_int8_t  u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int8_t  bool;
typedef u_int8_t  BOOL;

#elif defined(VTSS_OPSYS_ECOS)

#include "main.h"
#include <sys/types.h>
/* Shorthand types from ecos platform types */

#else

#error "You must supply type definitions for your platform OS"

#endif

#if defined(_lint)
/* This is mostly for lint */
#define offsetof(s,m) ((size_t)(unsigned long)&(((s *)0)->m))
#endif /* offsetof() */

#if !defined(PTP_ATTRIBUTE_PACKED)
#define PTP_ATTRIBUTE_PACKED __attribute__((packed)) /* GCC defined */
#endif

#if !defined(FALSE)
#define FALSE ((bool) 0)
#endif
#if !defined(TRUE)
#define TRUE  ((bool) 1)
#endif
#define ARR_SZ(a) (sizeof(a)/sizeof(a[0]))

/**
 * Types (and coresponding functions used in the IEEE p1588 standard)
 */
typedef unsigned char Octet;
typedef signed char Integer8;
typedef signed short Integer16;
typedef signed int Integer32;
typedef signed long long Integer64;
typedef unsigned char UInteger8;
typedef unsigned short UInteger16;
typedef unsigned int UInteger32;
typedef unsigned long long UInteger64;

/**
 * \brief PTP IP multicast Addresses
 *
 */
// "224.0.1.129"
#define PTP_PRIMARY_DEST_IP (((((((in_addr_t)224<<8)+ (in_addr_t)0)<<8) +(in_addr_t)1)<<8) + 129)
// "224.0.0.107"
#define PTP_PDELAY_DEST_IP  (((((((in_addr_t)224<<8)+ (in_addr_t)0)<<8) +(in_addr_t)0)<<8) + 107)



/**
 * \brief Convert a time stamp to a text.
 *
 * format %10d s %11d ns.
 *
 * \param t [IN]  pointer to time stamp
 * \param str [IN/OUT]  buffer to store the text string in
 */
char * TimeStampToString (const vtss_timestamp_t *t, char* str);

/**
 * \brief PTP clock unique identifier
 */
#define CLOCK_IDENTITY_LENGTH 8
typedef Octet ClockIdentity [CLOCK_IDENTITY_LENGTH];

/**
 * \brief Convert a clock identifier to text.
 *
 * \param clockIdentity [IN]  pointer to clock identifier
 * \param str [IN/OUT]  buffer to store the text string in
 */
char * ClockIdentityToString (const ClockIdentity clockIdentity, char *str);

/**
 * \brief PTP clock Quality specification
 */
typedef struct {
    UInteger8  clockClass;
    UInteger8  clockAccuracy;
    UInteger16 offsetScaledLogVariance;
} ClockQuality;

/**
 * \brief Convert a clock quality to text.
 *
 * format: "Cl:%03d Ac:%03d Va:%05d"
 *                           ^-offsetScaledLogVariance
 *                   ^---------clockAccuracy
 *           ^-----------------clockClass
 * \param clockQuality [IN]  pointer to clock identifier
 * \param str [IN/OUT]  buffer to store the text string in
 */
char * ClockQualityToString (const ClockQuality *clockQuality, char *str);

/**
 * \brief PTP clock port identifier
 */
typedef struct {
    ClockIdentity clockIdentity;
    UInteger16 portNumber;
} PortIdentity;

/**
 * \brief compare two PortIdentities.
 *
 * Like memcmp.
 */
int PortIdentitycmp(const PortIdentity* a, const PortIdentity* b);

/**
 * \brief main port states
 */
enum {
    PTP_INITIALIZING=0,  PTP_FAULTY,  PTP_DISABLED,
    PTP_LISTENING,  PTP_PRE_MASTER,  PTP_MASTER,
    PTP_PASSIVE,  PTP_UNCALIBRATED,  PTP_SLAVE,
    PTP_P2P_TRANSPARENT, PTP_E2E_TRANSPARENT
};

/**
 * \brief main PTP Protocol types
 */
enum {
    PTP_PROTOCOL_ETHERNET = 0,
    PTP_PROTOCOL_IP4MULTI = 1,
    PTP_PROTOCOL_IP4UNI = 2,
    PTP_PROTOCOL_OAM = 3,
    PTP_PROTOCOL_1PPS = 4,
    PTP_PROTOCOL_MAX_TYPE
};

/**
 * \brief Master-slave communication state
 */
enum {
	PTP_COMM_STATE_IDLE = 0,
	PTP_COMM_STATE_INIT = 1,
    PTP_COMM_STATE_CONN = 2,
    PTP_COMM_STATE_SELL = 3,
    PTP_COMM_STATE_SYNC = 4,
    PTP_COMM_STATE_MAX_TYPE
};
/**
 * \brief main PTP Device types
 */
enum {
    PTP_DEVICE_NONE = 0,        /* passive clock instance */
    PTP_DEVICE_ORD_BOUND ,      /* ordinary/bpundary clock instance*/
    PTP_DEVICE_P2P_TRANSPARENT, /* peer-to-peer transparent clock instance */
    PTP_DEVICE_E2E_TRANSPARENT, /* end-to-end transparent clock instance */
    PTP_DEVICE_MASTER_ONLY,     /* master only clock instance */
    PTP_DEVICE_SLAVE_ONLY,      /* slave only clock instance */
    PTP_DEVICE_MAX_TYPE
};
/**
 * \brief Convert a device type to text.
 *
 * \param state [IN]  device type
 * \return buffer with the text string
 */
char *  DeviceTypeToString(u8 state);


/**
 * \brief Convert a port state to text.
 *
 * \param state [IN]  port state
 * \param str [IN/OUT]  buffer to store the text string in
 */
char *  PortStateToString(u8 state);


/**
 * \brief PTP clock port identifier
 */
typedef struct {
    u32 ip;
    mac_addr_t mac;
} Protocol_adr_t;

#define MAX_UNICAST_MASTERS_PR_SLAVE 5
#define MAX_UNICAST_SLAVES_PR_MASTER 100


/*
 * used to initialize the run time options default
 */
#define DEFAULT_CLOCK_VARIANCE       (0xffff) /* indicates that the variance is not computed (spec 7.6.3.3) */
#define DEFAULT_CLOCK_CLASS          251
#define DEFAULT_NO_RESET_CLOCK       TRUE
/* spec defined constants  */

/* features, only change to reflect changes in implementation */
#define CLOCK_FOLLOWUP    TRUE

/**
 * \brief delayMechanism values
 */
enum { DELAY_MECH_E2E = 1, DELAY_MECH_P2P, DELAY_MECH_DISABLED = 0xfe };

/* features, only change to reflect changes in implementation */
/* used in initData */
#define VERSION_PTP       2

/**
 * \brief message header messageType field values
 */
enum {
    PTP_MESSAGE_TYPE_SYNC=0,  PTP_MESSAGE_TYPE_DELAY_REQ,  PTP_MESSAGE_TYPE_P_DELAY_REQ,
    PTP_MESSAGE_TYPE_P_DELAY_RESP,
    PTP_MESSAGE_TYPE_FOLLOWUP=8,  PTP_MESSAGE_TYPE_DELAY_RESP,
    PTP_MESSAGE_TYPE_P_DELAY_RESP_FOLLOWUP, PTP_MESSAGE_TYPE_ANNOUNCE, PTP_MESSAGE_TYPE_SIGNALLING,
    PTP_MESSAGE_TYPE_MANAGEMENT,
    PTP_MESSAGE_TYPE_ALL_OTHERS
};



/* define ptp packet header fields */
#define PTP_MESSAGE_MESSAGE_TYPE_OFFSET     0       /* messageType's offset in the ptp packet */
#define PTP_MESSAGE_VERSION_PTP_OFFSET      1       /* versionPTP's offset in the ptp packet */
#define PTP_MESSAGE_MESSAGE_LENGTH_OFFSET   2       /* messageLength's offset in the ptp packet */
#define PTP_MESSAGE_DOMAIN_OFFSET           4       /* domain's offset in the ptp packet */
#define PTP_MESSAGE_RESERVED_BYTE_OFFSET    5       /* first reserved byte's offset in the ptp packet */
#define PTP_MESSAGE_FLAG_FIELD_OFFSET       6       /* flag field's offset in the ptp packet */
#define PTP_MESSAGE_CORRECTION_FIELD_OFFSET 8       /* correction field's offset in the ptp packet */
#define PTP_MESSAGE_RESERVED_FOR_TS_OFFSET  16      /* reserved field's offset in the ptp packet */
#define PTP_MESSAGE_SOURCE_PORT_ID_OFFSET   20      /* soiecePortIdentity field's offset in the ptp packet */
#define PTP_MESSAGE_SEQUENCE_ID_OFFSET      30      /* sequenceID's offset in the ptp packet */
#define PTP_MESSAGE_CONTROL_FIELD_OFFSET    32      /* controlField's offset in the ptp packet */
#define PTP_MESSAGE_LOGMSG_INTERVAL_OFFSET  33      /* logMessageInterval's offset in the ptp packet */


/* define ptp packet sync/follow_up/announce fields */
#define PTP_MESSAGE_ORIGIN_TIMESTAMP_OFFSET 34      /* originTimestamp's offset in the ptp packet */

/* define ptp packet delay_Resp fields */
#define PTP_MESSAGE_RECEIVE_TIMESTAMP_OFFSET 34     /* receiveTimestamp's offset in the ptp delay_resp packet */
#define PTP_MESSAGE_REQ_PORT_ID_OFFSET      44      /* requestingPortIdentity's offset in the ptp packet */

/* define ptp packet Pdelay_xxx fields */
#define PTP_MESSAGE_PDELAY_TIMESTAMP_OFFSET 34     /* receiveTimestamp's offset in the ptp delay_resp packet */
#define PTP_MESSAGE_PDELAY_PORT_ID_OFFSET   44      /* requestingPortIdentity's offset in the ptp packet */

/* define ptp packet announce fields */
#define PTP_MESSAGE_CURRENT_UTC_OFFSET      44      /* current utc's offset in the ptp packet */
#define PTP_MESSAGE_GM_PRI1_OFFSET          47      
#define PTP_MESSAGE_GM_CLOCK_Q_OFFSET       48      
#define PTP_MESSAGE_GM_PRI2_OFFSET          52      
#define PTP_MESSAGE_GM_IDENTITY_OFFSET      53      
#define PTP_MESSAGE_STEPS_REMOVED_OFFSET    61      
#define PTP_MESSAGE_TIME_SOURCE_OFFSET      63      

/* define ptp packet Signalling fields */
#define PTP_MESSAGE_SIGNAL_TARGETPORTIDENTITY 34     /* targetPortIdentity's offset in the ptp signalling packet */


/**
 * \brief Local Clock operational mode
 */
typedef enum {
    VTSS_PTP_CLOCK_FREERUN,
    VTSS_PTP_CLOCK_LOCKING,
    VTSS_PTP_CLOCK_LOCKED,
} vtss_ptp_clock_mode_t;


/**
 * \brief Opaque PTP handle.
 */
typedef struct ptp_clock_t *ptp_clock_handle_t;


#endif /* _VTSS_PTP_TYPES_H_ */
