/*
 * EAP peer state machine functions (RFC 4137)
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef _PEER_SM_H_
#define _PEER_SM_H_

#include "peer_methods.h"
#include "vtss_nas_platform_api.h"

/* RFC 4137 - EAP Peer state machine */

typedef enum {
    DECISION_FAIL,
    DECISION_COND_SUCC,
    DECISION_UNCOND_SUCC
} EapDecision;

typedef enum {
    METHOD_NONE,
    METHOD_INIT,
    METHOD_CONT,
    METHOD_MAY_CONT,
    METHOD_DONE
} EapMethodState;

typedef enum {
    EAP_INITIALIZE,
    EAP_DISABLED,
    EAP_IDLE,
    EAP_RECEIVED,
    EAP_GET_METHOD,
    EAP_METHOD,
    EAP_SEND_RESPONSE,
    EAP_DISCARD_IT, /* RBN: Renamed from EAP_DISCARD because of gcc error */
    EAP_IDENTITY,
    EAP_NOTIFICATION,
    EAP_RETRANSMIT,
    EAP_SUCCESS,
    EAP_FAILURE
} EAP_state_t;

/**
 * struct eap_method_ret - EAP return values from struct eap_method::process()
 *
 * These structure contains OUT variables for the interface between peer state
 * machine and methods (RFC 4137, Sect. 4.2). eapRespData will be returned as
 * the return value of struct eap_method::process() so it is not included in
 * this structure.
 */
struct eap_method_ret {
    /**
     * ignore - Whether method decided to drop the current packed (OUT)
     */
    BOOL ignore;

    /**
     * methodState - Method-specific state (IN/OUT)
     */
    EapMethodState methodState;

    /**
     * decision - Authentication decision (OUT)
     */
    EapDecision decision;

    /**
     * allowNotifications - Whether method allows notifications (OUT)
     */
    BOOL allowNotifications;
};

/**
 * struct eap_method - EAP method interface
 * This structure defines the EAP method interface. Each method will need to
 * register its own EAP type, EAP name, and set of function pointers for method
 * specific operations. This interface is based on section 4.4 of RFC 4137.
 */
struct eap_method {
    /**
     * vendor - EAP Vendor-ID (EAP_VENDOR_*) (0 = IETF)
     */
    int vendor;

    /**
     * method - EAP type number (EAP_TYPE_*)
     */
    EapType method;

    /**
     * name - Name of the method (e.g., "TLS")
     */
    const char *name;

    /**
     * init - Initialize an EAP method
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * Returns: Pointer to allocated private data, or %NULL on failure
     *
     * This function is used to initialize the EAP method explicitly
     * instead of using METHOD_INIT state as specific in RFC 4137. The
     * method is expected to initialize it method-specific state and return
     * a pointer that will be used as the priv argument to other calls.
     */
    void *(*init)(struct nas_sm *sm);

    /**
     * deinit - Deinitialize an EAP method
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * @priv: Pointer to private EAP method data from eap_method::init()
     *
     * Deinitialize the EAP method and free any allocated private data.
     */
    void (*deinit)(struct nas_sm *sm, void *priv);

    /**
     * process - Process an EAP request
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * @priv: Pointer to private EAP method data from eap_method::init()
     * @ret: Return values from EAP request validation and processing
     * @reqData: EAP request to be processed (eapReqData)
     * @reqDataLen: Length of the EAP request
     * @respDataLen: Length of the returned EAP response
     * Returns: Pointer to allocated EAP response packet (eapRespData)
     *
     * This function is a combination of m.check(), m.process(), and
     * m.buildResp() procedures defined in section 4.4 of RFC 4137 In other
     * words, this function validates the incoming request, processes it,
     * and build a response packet. m.check() and m.process() return values
     * are returned through struct eap_method_ret *ret variable. Caller is
     * responsible for freeing the returned EAP response packet.
     */
    u8 *(*process)(struct nas_sm *sm, void *priv,
                   struct eap_method_ret *ret,
                   const u8 *reqData, size_t reqDataLen,
                   size_t *respDataLen);

    /**
     * get_status - Get EAP method status
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * @priv: Pointer to private EAP method data from eap_method::init()
     * @buf: Buffer for status information
     * @buflen: Maximum buffer length
     * @verbose: Whether to include verbose status information
     * Returns: Number of bytes written to buf
     *
     * Query EAP method for status information. This function fills in a
     * text area with current status information from the EAP method. If
     * the buffer (buf) is not large enough, status information will be
     * truncated to fit the buffer.
     */
    int (*get_status)(struct nas_sm *sm, void *priv, char *buf,
                      size_t buflen, int verbose);

    /**
     * has_reauth_data - Whether method is ready for fast reauthentication
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * @priv: Pointer to private EAP method data from eap_method::init()
     * Returns: %TRUE or %FALSE based on whether fast reauthentication is
     * possible
     *
     * This function is an optional handler that only EAP methods
     * supporting fast re-authentication need to implement.
     */
    BOOL (*has_reauth_data)(struct nas_sm *sm, void *priv);

    /**
     * deinit_for_reauth - Release data that is not needed for fast re-auth
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * @priv: Pointer to private EAP method data from eap_method::init()
     *
     * This function is an optional handler that only EAP methods
     * supporting fast re-authentication need to implement. This is called
     * when authentication has been completed and EAP state machine is
     * requesting that enough state information is maintained for fast
     * re-authentication
     */
    void (*deinit_for_reauth)(struct nas_sm *sm, void *priv);

    /**
     * init_for_reauth - Prepare for start of fast re-authentication
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * @priv: Pointer to private EAP method data from eap_method::init()
     *
     * This function is an optional handler that only EAP methods
     * supporting fast re-authentication need to implement. This is called
     * when EAP authentication is started and EAP state machine is
     * requesting fast re-authentication to be used.
     */
    void *(*init_for_reauth)(struct nas_sm *sm, void *priv);

    /**
     * get_identity - Get method specific identity for re-authentication
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * @priv: Pointer to private EAP method data from eap_method::init()
     * @len: Length of the returned identity
     * Returns: Pointer to the method specific identity or %NULL if default
     * identity is to be used
     *
     * This function is an optional handler that only EAP methods
     * that use method specific identity need to implement.
     */
    const u8 *(*get_identity)(struct nas_sm *sm, void *priv, size_t *len);

    /**
     * free - Free EAP method data
     * @method: Pointer to the method data registered with
     * eap_peer_method_register().
     *
     * This function will be called when the EAP method is being
     * unregistered. If the EAP method allocated resources during
     * registration (e.g., allocated struct eap_method), they should be
     * freed in this function. No other method functions will be called
     * after this call. If this function is not defined (i.e., function
     * pointer is %NULL), a default handler is used to release the method
     * data with free(method). This is suitable for most cases.
     */
    void (*free)(struct eap_method *method);

#define EAP_PEER_METHOD_INTERFACE_VERSION 1
    /**
     * version - Version of the EAP peer method interface
     *
     * The EAP peer method implementation should set this variable to
     * EAP_PEER_METHOD_INTERFACE_VERSION. This is used to verify that the
     * EAP method is using supported API version when using dynamically
     * loadable EAP methods.
     */
    int version;

    /**
     * next - Pointer to the next EAP method
     *
     * This variable is used internally in the EAP method registration code
     * to create a linked list of registered EAP methods.
     */
    struct eap_method *next;

#ifdef CONFIG_DYNAMIC_EAP_METHODS
    /**
     * dl_handle - Handle for the dynamic library
     *
     * This variable is used internally in the EAP method registration code
     * to store a handle for the dynamic library. If the method is linked
     * in statically, this is %NULL.
     */
    void *dl_handle;
#endif /* CONFIG_DYNAMIC_EAP_METHODS */

    /**
     * get_emsk - Get EAP method specific keying extended material (EMSK)
     * @sm: Pointer to EAP state machine allocated with peer_sm_init()
     * @priv: Pointer to private EAP method data from eap_method::init()
     * @len: Pointer to a variable to store EMSK length
     * Returns: EMSK or %NULL if not available
     *
     * This function can be used to get the extended keying material from
     * the EAP method. The key may already be stored in the method-specific
     * private data or this function may derive the key.
     */
    u8 *(*get_emsk)(struct nas_sm *sm, void *priv, size_t *len);
};

/**
 * struct peer_sm - EAP state machine data
 */
typedef struct peer_sm {

    // Top-level stuff
    nas_eap_info_t         eap_info;
    nas_port_status_t      authStatus;

    ///////////////////////////////////////////////////////////////////////////////
    // EAPOL state variables
    /*
     * These variables are used in the interface between EAP peer state machine and
     * lower layer. These are defined in RFC 4137, Sect. 4.1. Lower layer code is
     * expected to maintain these variables and register a callback functions for
     * EAP state machine to get and set the variables.
     */

    /**
     * EAPOL_eapSuccess - EAP SUCCESS state reached
     *
     * EAP state machine reads and writes this value.
     */
    BOOL EAPOL_eapSuccess;

    /**
     * EAPOL_eapRestart - Lower layer request to restart authentication
     *
     * Set to TRUE in lower layer, FALSE in EAP state machine.
     */
    BOOL EAPOL_eapRestart;

    /**
     * EAPOL_eapFail - EAP FAILURE state reached
     *
     * EAP state machine writes this value.
     */
    BOOL EAPOL_eapFail;

    /**
     * EAPOL_eapResp - Response to send
     *
     * Set to TRUE in EAP state machine, FALSE in lower layer.
     */
    BOOL EAPOL_eapResp;

    /**
     * EAPOL_eapNoResp - Request has been processed; no response to send
     *
     * Set to TRUE in EAP state machine, FALSE in lower layer.
     */
    BOOL EAPOL_eapNoResp;

    /**
     * EAPOL_eapReq - EAP request available from lower layer
     *
     * Set to TRUE in lower layer, FALSE in EAP state machine.
     */
    BOOL EAPOL_eapReq;

    /**
     * EAPOL_portEnabled - Lower layer is ready for communication
     *
     * EAP state machines reads this value.
     */
    BOOL EAPOL_portEnabled;

    /**
     * EAPOL_altAccept - Alternate indication of success (RFC3748)
     *
     * EAP state machines reads this value.
     */
    BOOL EAPOL_altAccept;

    /**
     * EAPOL_altReject - Alternate indication of failure (RFC3748)
     *
     * EAP state machines reads this value.
     */
    BOOL EAPOL_altReject;

    /**
     * EAPOL_idleWhile - Outside time for EAP peer timeout
     *
     * This integer variable is used to provide an outside timer that the
     * external (to EAP state machine) code must decrement by one every
     * second until the value reaches zero. This is used in the same way as
     * EAPOL state machine timers. EAP state machine reads and writes this
     * value.
     */
    u32 EAPOL_idleWhile;

    // Not used by SM itself. Counts seconds before reauthentication is needed.
    nas_timer_t reAuthWhen;
    BOOL        reAuthEnabled;

    // Controls how the INITIALIZE state should be handled.
    // If @how_to_initialize is any other value but 1 and 2 (Start All Over):
    //   EAPOL_eapSuccess, EAPOL_eapFail, and authStatus are reset to FALSE and
    //   Unauthorized, respectively, but setUnauthorized() is not called.
    //   This is useful when a new state-machine has just been allocated, and
    //   the client is already considered unauthorized by the user-module.
    // If @how_to_initialize == 1 (Re-init):
    //   EAPOL_eapSuccess, EAPOL_eapFail, and authStatus are reset to FALSE and
    //   Unauthorized, respectively, and the setUnauthorized() is called.
    //   This is useful when the user-module wishes to re-initialize an
    //   existing client, so that the setUnauthorized() function gets called
    //   to force the client unauthorized in the user-module as well.
    // If @how_to_initialize == 2 (Re-auth):
    //   EAPOL_eapSuccess, EAPOL_eapFail, and authStatus are not touched at all,
    //   and the setUnauthorized() function is not called. This is useful
    //   when reauthenticating an already authorized client, so that the client
    //   doesn't get unauthorized while the re-authentication takes places.
    int how_to_initialize;

    // If @how_to_initialize == 0 (SM is just allocated), we count the number of times
    // to fake a request identity. The reason is that at boot, the RADIUS module
    // may not be up and running (or it may not be configured at all), so we have
    // to retry the initialization procedure a number of times before giving up
    // and placing the SM in unauthorized state.
    // This variable counts down when EAPOL_idleWhile reaches 0, and if
    // it doesn't reach zero, we restart the SM with a fake request identity.
    // When both EAPOL_idleWhile and @fake_request_identities_left reach zero,
    // we go to the unauthorized state.
    u8 fake_request_identities_left;

    ///////////////////////////////////////////////////////////////////////////////

    EAP_state_t EAP_state;

    /* Long-term local variables */
    EapType        selectedMethod;
    EapMethodState methodState;
    int            lastId;
    EapDecision    decision;

    /* Short-term local variables */
    BOOL           rxReq;
    BOOL           rxSuccess;
    BOOL           rxFailure;
    int            reqId;
    EapType        reqMethod;
    int            reqVendor;
    u32            reqVendorMethod;
    BOOL           ignore;

    /* Miscellaneous variables */
    BOOL           allowNotifications; /* peer state machine <-> methods */
    u8             *eapRespData; /* peer to lower layer */
    size_t         eapRespDataLen; /* peer to lower layer */
    const struct   eap_method *m; /* selected EAP method */

    /* Not defined in RFC 4137 */
    BOOL           changed;
    void           *eap_method_priv;
    int            init_phase2;
    int            fast_reauth;

    BOOL           rxResp /* LEAP only */;
    BOOL           leap_done;
    BOOL           peap_done;
    u8             req_md5[16]; /* MD5() of the current EAP packet */
    u8             last_md5[16]; /* MD5() of the previously received EAP packet; used in duplicate request detection. */

    unsigned int   workaround;

    /* Optional challenges generated in Phase 1 (EAP-FAST) */
    u8             *peer_challenge, *auth_challenge;

    int            num_rounds;
    BOOL           force_disabled;
} peer_sm_t;

struct nas_sm; // Forward declaration because the header files would otherwise include each other.

struct eap_method_type {
    int vendor;
    u32 method;
};

void peer_sm_init(struct nas_sm *sm);
void eap_peer_sm_deinit(struct nas_sm *sm);
void peer_sm_step(struct nas_sm *sm);
void eap_sm_abort(struct nas_sm *sm);
int  eap_sm_get_status(struct nas_sm *sm, char *buf, size_t buflen, int verbose);
void eap_set_fast_reauth(struct nas_sm *sm, int enabled);
void eap_notify_success(struct nas_sm *sm);
void eap_notify_lower_layer_success(struct nas_sm *sm);
u8   *eap_get_eapRespData(struct nas_sm *sm, size_t *len);
void eap_register_scard_ctx(struct nas_sm *sm, void *ctx);
void eap_invalidate_cached_session(struct nas_sm *sm);

// Other
void peer_force_reinit(struct nas_sm *sm, BOOL this_is_an_existing_sm);
void peer_force_reauth(struct nas_sm *sm, BOOL step_sm);
void peer_timers_tick(struct nas_sm *sm);

#endif /* _PEER_SM_H_ */
