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

 $Id$
 $Revision$

*/

#ifndef _VTSS_MSG_API_H_
#define _VTSS_MSG_API_H_

#include "vtss_api.h"
#include "vtss_module_id.h"
#include "main.h"           /* For MODULE_ERROR_START */
#include <time.h>

/* Message Module error codes (vtss_rc) */
typedef enum {
    MSG_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_MSG),  /* Generic error code */
    MSG_ERROR_PARM,                                          /* Illegal parameter  */
    MSG_ASSERT_FAILURE,                                      /* Assertion failed   */
} msg_error_t;

/****************************************************************************/
//
//                          MESSAGE SERVICE API
//
/****************************************************************************/

/****************************************************************************/
// msg_switch_is_master()
// Returns TRUE if we're master, FALSE if we're slave or right after boot.
/****************************************************************************/
BOOL msg_switch_is_master(void);

/****************************************************************************/
// msg_switch_exists()
// If we're not currently master, this function returns FALSE. Otherwise:
// Returns TRUE if switch referred to with #isid is present in the stack.
/****************************************************************************/
BOOL msg_switch_exists(vtss_isid_t isid);

/****************************************************************************/
// msg_switch_configurable()
// Returns TRUE if we're master and the switch has been seen in the stack
// before. Otherwise returns FALSE.
// Notice that the switch doesn't have to be present currently for the function
// to return TRUE.
/****************************************************************************/
BOOL msg_switch_configurable(vtss_isid_t isid);

/****************************************************************************/
// msg_existing_switches()
// Returns a bitmask with a '1' in bit-positions for existing switches and
// '0' in bit-positions for non-existing switches.
// The return value is zero when called on a slave.
// This function eliminates the need to call (the expensive) msg_switch_exists()
// multiple times when you wish to iterate over all existing switches in a
// stack.
// Only bits in the range [VTSS_ISID_START; VTSS_ISID_END[ can be set.
/****************************************************************************/
u32 msg_existing_switches(void);

/****************************************************************************/
// msg_configurable_switches()
// Returns a bitmask with a '1' in bit-positions for configurable switches and
// '0' in bit-positions for non-configurable switches.
// The return value is zero when called on a slave.
// This function eliminates the need to call (the expensive) msg_switch_configurable()
// multiple times when you wish to iterate over all configurable switches in a
// stack.
// Only bits in the range [VTSS_ISID_START; VTSS_ISID_END[ can be set.
/****************************************************************************/
u32 msg_configurable_switches(void);

/****************************************************************************/
// msg_switch_is_local()
// Returns TRUE if we're master and the @isid matches the local switch's
// ISID, FALSE otherwise.
/****************************************************************************/
BOOL msg_switch_is_local(vtss_isid_t isid);

/****************************************************************************/
// msg_master_isid()
// Returns our own isid if we're master, VTSS_ISID_UNKNOWN otherwise.
/****************************************************************************/
vtss_isid_t msg_master_isid(void);

#define MSG_MAX_VERSION_STRING_LEN 120
#define MSG_MAX_PRODUCT_NAME_LEN   120

/****************************************************************************/
// msg_version_string_get()
// Returns the version string of a given switch in the stack. The call is only
// valid on the master, and the caller must allocate MSG_MAX_VERSION_STRING_LEN
// bytes for @ver_str prior to the call.
// @ver_str[0] is set to '\0' if an error occurred or no version string was
// received from the slave switch.
// The reason for not just returning a pointer is that the stack may change
// right after the pointer is returned, thus resulting in a non-consistent
// version string.
/****************************************************************************/
void msg_version_string_get(vtss_isid_t isid, char *ver_str);

/****************************************************************************/
// msg_product_name_get()
// Returns the product name of a given switch in the stack. The call is only
// valid on the master, and the caller must allocate MSG_MAX_PRODUCT_NAME_LEN
// bytes for @prod_name prior to the call.
// @prod_name[0] is set to '\0' if an error occurred or no product name was
// received from slave switch.
// The reason for not just returning a pointer is that the stack may change
// right after the pointer is returned, thus resulting in a non-consistent
// product name.
/****************************************************************************/
void msg_product_name_get(vtss_isid_t isid, char *prod_name);

/****************************************************************************/
// msg_uptime_get()
// Returns the up-time measured in seconds of a switch.
// This function may be called both as a master and as a slave.
//
// If calling as a slave, @isid must be VTSS_ISID_LOCAL.
// If calling as a master, @isid may be any legal ISID and VTSS_ISID_LOCAL.
// If calling as a master and @isid is not present, 0 is returned.
/****************************************************************************/
time_t msg_uptime_get(vtss_isid_t isid);

/****************************************************************************/
// msg_abstime_get()
// As input it takes a relative time measured in seconds since boot and an
// isid. The isid is used to tell on which switch the relative time was
// recorded on. The returned time is a time_t value that takes into account
// the current switch's SNTP-obtained time and timezone correction.
// The return value can be converted to a string using misc_time2str().
// If the @rel_event_time occurred on the slave switch before the master booted
// and the master hasn't obtained an SNTP time yet, the event will be converted
// to 0.
//
// If calling as a slave, @isid must be VTSS_ISID_LOCAL.
// If calling as a master, @isid may be any legal ISID and VTSS_ISID_LOCAL.
// If calling as a master and @isid is not present, 0 is returned.
/****************************************************************************/
time_t msg_abstime_get(vtss_isid_t isid, time_t rel_event_time);

/****************************************************************************/
// msg_switch_info_t
// Holds various info about a given switch in the stack.
/****************************************************************************/
typedef struct {
    // Vendor Specific Version string
    char version_string[MSG_MAX_VERSION_STRING_LEN];

    // Vendor Specific Product Name
    char product_name[MSG_MAX_PRODUCT_NAME_LEN];

    // Uptime in seconds of the slave
    u32 slv_uptime_secs;

    // The time in seconds that the master had been up when it received first frame from slave.
    u32 mst_uptime_secs;

    // Additional info about the slave
    init_switch_info_t info;

    // And more info about the slave
    init_port_map_t port_map[VTSS_PORT_ARRAY_SIZE];
} msg_switch_info_t;

/****************************************************************************/
// msg_switch_info_get()
// Returns various info of a given switch in the stack.
// The function returns VTSS_RC_OK if the switch pointed by @isid exists,
// and in that case, the @info structure is filled with the relevant info.
// If the switch doesn't exist or arguments are invalid or the current
// switch is not master, the function returns VTSS_RC_ERROR.
/****************************************************************************/
vtss_rc msg_switch_info_get(vtss_isid_t isid, msg_switch_info_t *info);

/****************************************************************************/
//
//                        MESSAGE TRANSMISSION API
//
/****************************************************************************/

/****************************************************************************/
// msg_tx_rc_t
// Codes that can be returned to the user-defined callback when a user message
// has been (attempted) transmitted.
// User Modules should in general not generate an error if receiving a
// non-zero return code, since this is normally due to an upcoming topology
// change. If it's a real error, the Message Module will generate an error.
// User Modules should in general not generate an error if receiving a
// non-zero return code, since this is normally due to an upcoming topology
// change. If it's a real error, the Message Module will generate an error.
/****************************************************************************/
typedef enum {
    MSG_TX_RC_OK,                        //  0, Message transmitted and acknowledged successfully.

    MSG_TX_RC_WARN_MST_NO_SLV,           //  1, Master : User Module called msg_tx(), but there was no connection towards the slave. Maybe the slave just left the stack.
    MSG_TX_RC_WARN_MST_INV_DISID,        //  2, Master : User Module called msg_tx() with an invalid DISID, probably because User Module thinks it's in Slave Mode.
    MSG_TX_RC_WARN_MST_MASTER_DOWN,      //  3, Master : The message was not sent because of master down event.
    MSG_TX_RC_WARN_MST_SLAVE_DOWN,       //  4, Master : The slave was taken out of the stack
    MSG_TX_RC_WARN_MST_NO_ACK,           //  5, Master : The message didn't reach the slave because the slave didn't reply with MACKs.
    MSG_TX_RC_WARN_MST_INV_DPORT,        //  6, Master : The message was not transmitted because Topo didn't know in which direction to send it.
    MSG_TX_RC_WARN_SLV_NO_MST,           //  7, Slave  : User Module called msg_tx(), but there was no connection towards the master (yet).
    MSG_TX_RC_WARN_SLV_INV_DCONNID,      //  8, Slave  : User Module called msg_tx() with an invalid DCONNID, probably because User Module thinks it's in Master Mode.
    MSG_TX_RC_WARN_SLV_MASTER_UP,        //  9, Slave  : The message was not sent because of master up event.
    MSG_TX_RC_WARN_SLV_NEW_MASTER,       // 10, Slave  : The message was not sent because (another) master (re-)negotiates connection.
    MSG_TX_RC_WARN_SLV_NO_ACK,           // 11, Slave  : The message didn't reach the master because the master didn't reply with MACKs.
    MSG_TX_RC_WARN_SLV_INV_DPORT,        // 12, Slave  : The message was not transmitted because Topo didn't know in which direction to send it.
    MSG_TX_RC_ERR_MST_MACK_STATUS,       // 13, Master : The slave sent a MACK with a status indicating error.
    MSG_TX_RC_ERR_SLV_MACK_STATUS,       // 14, Slave  : The slave sent a MACK with a status indicating error. This is a protocol error, but is possible anyway.
    MSG_TX_RC_WARN_SHAPED,               // 15, Master/Slave: The message was not sent because it was subject to shaping, and got filtered.
    MSG_TX_RC_ERR_MST_DBG_RENEGOTIATION, // 16, Master : User asked for a re-negotiation through the CLI (debug, only).
    MSG_TX_RC_WARN_MST_UPSID_CHANGE,     // 17, Master : Topo changed the UPSID on the master. Re-negotiating connection towards all slaves. Only possible in non-hop-by-hop approaches,

    // IMPORTANT: THIS MUST BE THE LAST ENTRY IN THIS ENUM, BECAUSE IT'S USED TO SIZE AN ARRAY.
    MSG_TX_RC_LAST_ENTRY,
} msg_tx_rc_t;

/****************************************************************************/
// msg_tx_cb_t
// A User Module-defined callback function called by the message module when
// a user message has been (attempted) transmitted. @contxt is passed as an
// argument to msg_tx_adv() and @rc is one of the error codes defined above.
// @msg is a pointer to the message that was transmitted.
// User Modules should in generally not be concerned about the return code,
// since the Message Module tracks an error if it's a real error. Most of the
// non-zero return codes are simply warnings, indicating that the stack is
// about to change.
/****************************************************************************/
typedef void (*msg_tx_cb_t)(void *contxt, void *msg, msg_tx_rc_t rc);

/****************************************************************************/
// msg_tx_opt_t
// Options that msg_tx_adv() can be called with.
/****************************************************************************/
typedef enum {
    // Prevent Message Module from calling VTSS_FREE() on message after Tx
    MSG_TX_OPT_DONT_FREE = 0x00000001,

    // When master and the message is looped back, default is that the message
    // module makes a copy of the message, calls back a possible TxDone function
    // (in msg_tx thread context) and then calls the RX callback function (in
    // msg_rx thread context), before freeing the copy.
    // This behavior is useful if the RX callback function can transmit messages,
    // but only has a limited number of pre-allocated buffers, which are
    // protected by semaphores, that are signaled in the TxDone function.
    // When specifying this option, the same buffer that msg_tx() was called with
    // will be looped back to the registered RX function, and in turn to the
    // TxDone callback function before possibly beeing freed by the message module.
    // The normal msg_tx() function will implicitly have this flag set.
    MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK = 0x00000002,

    // Master only. Message Module transmits this message on the highest
    // numbered CID towards the specified slave.
    MSG_TX_OPT_PRIO_HIGH = 0x00000004,

    // The message is subject to shaping.
    // The user module may specify that it's OK for the message module to drop
    // the message if the user module has more than a configurable number of
    // messages outstanding. This is useful if the user module e.g. forwards
    // all frames it receives on a front port to the current master for further
    // processing. Modules that could be interested in this shaping include
    // DHCP Snooping, ARP Inspection, L2 (BPDUs), PSEC, NAS, etc.
    // If a message gets filtered, the TxDone function will still be called
    // and the #rc argument will be MSG_TX_RC_WARN_SHAPED.
    // Note that the user module should *not* use this option for messages
    // that must be transmitted, like configuration, status, etc.
    MSG_TX_OPT_SHAPE = 0x00000008,
} msg_tx_opt_t;

// The maximum message length in bytes is 2^24-1.
#define MSG_MAX_LEN_BYTES 16777215

/****************************************************************************/
// msg_tx()
// msg_tx() is used when the user module doesn't care about when the message
// has been sent. The msg itself must be allocated on the heap with a call
// to VTSS_MALLOC(). The message module frees it after it has been transmitted.
// In case of loopback, the message module frees it after the corresponding
// RX callback has been called.
// Calling msg_tx() corresponds to calling msg_tx_adv() as follows:
//   msg_tx_adv(NULL, NULL, MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK, dmodid, did, msg, len);
//
// The MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK prevents msg_tx_adv() from making a
// copy of the message before looping it back, since this is not needed for
// a message allocated on the heap.
/****************************************************************************/
/**
 * The message module automatically frees the data we've malloc'ed.
 * The following tells FlexeLint that msg_tx() will take custody of
 * the pointer passed as the third argument.
 */
/*lint -sem(msg_tx, custodial(3)) */
void msg_tx(vtss_module_id_t dmodid, u32 did, const void *const msg, size_t len);

/****************************************************************************/
// msg_tx_adv()
// The callback function is *always* (independent of return code) called from
// message thread context. This allows the caller of msg_tx_adv() to hold
// its own mutex while calling msg_tx_adv() and take it again when calling
// the callback function.
//
// Arguments:
// @contxt:  A user-defined pointer that is used in the call to @cb.
//
// @cb:      A callback function invoked when the message is handled -
//           whether successfully or unsuccessfully. It may be NULL, if the
//           User Module doesn't care about the result.
//
// @options: Any bitwise OR of the flags in the msg_tx_opt_t enumeration.
//
// @dmodid:  The module ID to pass the message to on the destination switch.
//
// @did:     Destination ID. This parameter has two different meanings
//           depending on whether the switch is a master or a slave.
//
//             Master: The @did is an ISID, i.e. it's the ISID of the switch
//             to receive the message. A @did of 0 causes the message to be
//             looped back to the subscriber on the same switch. Masters must
//             not use @dids outside the interval [0; MAX_ISID].
//
//             Slave: The @did is a connid. connids are locally generated by
//             the Message Module upon connection establishment. The Message
//             Module looks up the master switch address based on the connid.
//             A connid of 0 identifies the current master and may be used
//             for unsolicited messages. Slaves must not use @dids in the
//             interval [1; MAX_ISID].
//
// @msg:     A pointer to the message. Do not allocate the message on the
//           stack, and do not alter the contents before the callback function
//           is called.
//
// @len:     The length of the message in bytes. The maximum message length
//           is 16MBytes-1 (2^24-1).
//
//
// Description:
//   Transmit a message to a given switch. When the Message Module has
//   transmitted the message, it automatically calls VTSS_FREE() on @msg, unless
//   the msg_tx_adv() function is used with @options including
//   MSG_TX_OPT_DONT_FREE, in which case it is up to the callback (@cb)
//   function to free the message.
//
// Return value:
//   Neither msg_tx() or msg_tx_adv() return a value, but if the callback
//   (@cb) is specified (msg_tx_adv() only), it receives the result of
//   transmitting the message.
/****************************************************************************/
/**
 * The message module may automatically free the message.
 * The following tells FlexeLint that msg_tx() will take custody of
 * the pointer passed as the third argument.
 */
/*lint -sem(msg_tx_adv, custodial(6)) */
void msg_tx_adv(const void *const contxt,
                const msg_tx_cb_t cb,
                msg_tx_opt_t      opt,
                vtss_module_id_t  dmodid,
                u32               did,
                const void *const msg,
                size_t            len);

/****************************************************************************/
//
//                        MESSAGE RECEPTION API
//
/****************************************************************************/

/****************************************************************************/
// msg_rx_filter_t
// This structure is filled in prior to registering for particular messages.
// A User Module should reset it to all zeros prior to assigning specific
// values for future interoperability.
/****************************************************************************/
typedef struct {
    // User-defined info. Supplied when calling back.
    void *contxt;

    // Callback function called when a message arrives.
    // IMPORTANT #1:
    //   The User Module must not modify the message for two reasons:
    //     a) In the future it may be that multiple user modules may register for
    //        the same messages.
    //     b) In case a message is looped back (master sending to itself), the
    //        memory used in the call to msg_tx() is the same as the memory used
    //        in the callback (no copy occurs). The caller of msg_tx() may expect
    //        the contents of the message to be unchanged after it has been
    //        "transmitted".
    // IMPORTANT #2:
    //   The User Module must not free the message. Memory management is solely
    //   handled by the Message Module (due to support for loopback).
    // IMPORTANT #3:
    //   For lengthy operations, make a copy of the message and hand it to
    //   another thread, so that the caller thread can get on with its job.
    //
    // Arguments:
    //   @contxt : User-defined (the value of this structure's .contxt member)
    //   @msg : Pointer to the first byte of the message.
    //   @len : Length of message in bytes.
    //   @id  : Source ID. This parameter identifies the transmitter of the
    //          message. It's value depends on whether the switch is master
    //          or slave:
    //          Master:
    //            The @id is an ISID, i.e. it's the ISID of the switch that
    //            transmitted the message. This may be 0 if the message was
    //            transmitted by the same master and looped back. The valid range
    //            of the @id on the master is therefore [0; MAX_ISID].
    //          Slave:
    //            The @id is a connid. connids are locally generated by the
    //            Message Module upon connection establishment, and uniquely
    //            identifies the master that transmitted the message, and the
    //            connection that the master transmitted the message on. A slave
    //            must use the same @id in a call to msg_tx() or msg_tx_adv() if
    //            the received message calls for a response. The @id always lies
    //            outside the range [0; MAX_ISID].
    //
    // Return value:
    //   Currently not used. For future interoperability, always return TRUE.
    BOOL (*cb)(void *contxt, const void *const msg, size_t len, vtss_module_id_t modid, u32 id);

    // This is the Module ID that the User Module subscribes to.
    vtss_module_id_t modid;
} msg_rx_filter_t;

/****************************************************************************/
// msg_rx_filter_register()
// Subscribe to messages.
//
// Arguments:
// @filter: A pointer to the structure defining the filter.
//          The structure may be allocated on the stack, because the Message
//          Module makes a copy of it before using it. Before assigning members
//          of the structure, reset it to all-zeros for future
//          interoperability.
//
// Description:
//   After a successful registration @filter->cb will be called back whenever
//   messages that match the Module ID are received.
//
// Return value:
//   The function returns VTSS_OK on success, anything else on error.
/****************************************************************************/
vtss_rc msg_rx_filter_register(const msg_rx_filter_t *filter);

/****************************************************************************/
//
//                        OTHER MESSAGE API FUNCTIONS
//
/****************************************************************************/

/****************************************************************************/
// msg_init()
// Message Module initialization function.
/****************************************************************************/
vtss_rc msg_init(vtss_init_data_t *data);

typedef int (*msg_dbg_printf_t)(const char *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format (__printf__, 1, 2)))
#endif
;

/****************************************************************************/
// msg_dbg()
// Entry point to Message Protocol Debug features. Should be called from
// cli only.
/****************************************************************************/
void msg_dbg(msg_dbg_printf_t dbg_printf, u32 parms_cnt, u32 *parms);

/****************************************************************************/
//
//                        MESSAGE TOPOLOGY EVENT API
//
/****************************************************************************/

// Topology change events generated by Topo - ONLY
typedef enum {
    MSG_TOPO_EVENT_CONF_DEF,
    MSG_TOPO_EVENT_MASTER_UP,
    MSG_TOPO_EVENT_MASTER_DOWN,
    MSG_TOPO_EVENT_SWITCH_ADD,
    MSG_TOPO_EVENT_SWITCH_DEL
} msg_topo_event_t;

/****************************************************************************/
// msg_topo_event()
// Supposed to be called only from the topology module and only when a
// topology change occurs.
//
// Arguments:
//   @event:
//     Either of the events defined with msg_topo_event_t.
//   @isid:
//     The ID for the switch in question. The interpretation depends on @event:
//       MSG_TOPO_EVENT_CONF_DEF:
//          @isid = isid of switch to reset configuration for. Must not
//          currently be in use. Event is only valid in master mode.
//       MSG_TOPO_EVENT_MASTER_UP:
//          @isid = local switch's ISID. Event is only valid in slave mode.
//       MSG_TOPO_EVENT_MASTER_DOWN:
//          @isid = Unused. Event is only valid in slave mode.
//       MSG_TOPO_EVENT_SWITCH_ADD:
//          @isid = Slave switch's ISID. Event is only valid in master mode.
//       MSG_TOPO_EVENT_SWITCH_ADD:
//          @isid = isid of switch to delete from stack. Event is only valid
//          in master mode.
/****************************************************************************/
void msg_topo_event(msg_topo_event_t event, vtss_isid_t isid);

/****************************************************************************/
// The following macro may be used to calculate the length of the header
// of the user message. The assumption is that the user message is a struct
// with two parts:
//
// 1) Header
// 2) Data
/****************************************************************************/
#define MSG_TX_DATA_HDR_LEN(s, m) offsetof(s, m)

/****************************************************************************/
// The following macro may be used to find the biggest of two header lengths
// of user data messages.
/****************************************************************************/
#define MSG_TX_DATA_HDR_LEN_MAX(s1, m1, s2, m2) (offsetof(s1, m1) > offsetof(s2, m2) ? offsetof(s1, m1) : offsetof(s2, m2))


/**
 * \brief Create a message buffer pool
 *
 * A message buffer pool is a pool of #buf_cnt message buffers of size #bytes_per_buf each.
 * The buffer pool is protected by mechanisms inside the msg_buf_pool_XXX() functions, so
 * it's completely safe to call the functions without any protection around the calls.
 *
 * Typical use would be to call msg_buf_pool_create() in INIT_CMD_INIT and
 * msg_buf_pool_get() whenever a message buffer is needed and msg_buf_pool_put() in
 * the module's msg_tx_done() callback function.
 *
 * A given module may call msg_buf_pool_create() as many times as needed. Typically,
 * modules will have one request buffer pool and one response buffer pool.
 *
 * At any point in time, you can see the current buffer pool state by invoking
 * 'debug msg 7".
 *
 * \param module_id     [IN] The ID of the module calling this function. Only used for debug printing.
 * \param dscr          [IN] A description that should be shorter than 16 chars, e.g. "Request" or "Response".
 * \param buf_cnt       [IN] The number of buffers that this pool will handle.
 * \param bytes_per_buf [IN] The number of bytes one buffer contains.
 *
 * \return Pointer to a message pool that can be used in subsequent calls to msg_buf_pool_get().
 */
void *msg_buf_pool_create(vtss_module_id_t module_id, i8 *dscr, u32 buf_cnt, u32 bytes_per_buf);

/**
 * \brief Get a message buffer from #buf_pool.
 *
 * Call this function to obtain a buffer from #buf_pool.
 * The function will block if there are no available buffers.
 * You may cast the return value directly to a message of the required type.
 *
 * The buffer holds a reference count that - upon return from this function - will
 * be set to 1. The reference count may be changed with a call to msg_buf_pool_ref_cnt_set()
 * prior to using the buffer. Subsequent calls to msg_buf_pool_put() will decrement
 * the ref-count and only release the buffer when the ref-count goes from 1 to 0.
 *
 * \param buf_pool [IN] Pointer to the buffer pool as returned by msg_buf_pool_create().
 *
 * \return Pointer to a message buffer that can be cast directly to a message of the
 * required type. The length is guaranteed to be #bytes_per_buf.
 * Only in case of coding errors inside the msg_buf_pool_XXX() functions, will the
 * return value be NULL.
 */
void *msg_buf_pool_get(void *buf_pool);

/**
 * \brief Return a buffer to the buffer pool.
 *
 * Call this function to decrement the buffer's reference count and return
 * it to its pool once the reference count reaches 0.
 *
 * Internally, msg_buf_pool_put() knows the message pool to return it to.
 *
 * You should return the buffer from within your module's msg_tx_done()
 * function. The msg_tx_done()'s #msg argument can be used for that purpose.
 * The call to msg_buf_pool_put() is non-blocking.
 *
 * \param buf [IN] Pointer to the buffer to return to the pool.
 *
 * \return New reference count, i.e. 0 if the buffer is really released to its pool.
 */
u32 msg_buf_pool_put(void *buf);

/**
 * \brief Set the reference count of a buffer.
 *
 * Call this function to set the reference count of a buffer to a particular.
 * value. Setting it to 0 will release the buffer immediately.
 *
 * If your module wishes to send identical messages to several switches,
 * using the reference counting capabilities of the message buffers can be
 * of big help.
 *
 * Typical use is as follows:
 *   After having initialized your switch iterator with switch_iter_init(),
 *   obtain a message buffer with msg_buf_pool_get() and call
 *   msg_buf_pool_ref_cnt_set() with #ref_cnt set to the swich iterator's
 *   #remaining parameter. Then fill in the message and start sending messages
 *   as long as switch_iter_getnext() returns TRUE. In your module's msg_tx_done()
 *   function, always call msg_puf_pool_put() with the msg_tx_done()'s #msg argument.
 *   This will decrement the buffer's ref-count and release it once it reaches 0.
 *
 * \param buf     [IN] Pointer to the buffer you wish to set the ref-count for.
 * \param ref_cnt [IN] New reference count value. 0 to release immediately.
 *
 * \return Nothing.
 */
void msg_buf_pool_ref_cnt_set(void *buf, u32 ref_cnt);

/**
 * \brief Get the highest user priority that is allowed on this build.
 *
 * This function should only be called by the QoS module.
 *
 * In stackable builds, the message module can be configured to either transmit
 * frames hop-by-hop to slave switches or to transmit them directly to the
 * destination switch. In the latter case, it may either use a super-priority
 * queue or one of the 8 user priorities. If it uses one of the 8 user priorities,
 * the QoS module should remap all user requests for the highest priority to
 * the second highest priority.
 *
 * So in order to make the QoS module agnostic to the current message module
 * configuration, this function can be called to get the highest allowed
 * user priority.
 *
 * \return Maximum allowed user priority (e.g. 3 on Lu28, and 6 or 7 on JR).
 */
u32 msg_max_user_prio(void);

#endif /* _VTSS_MSG_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
