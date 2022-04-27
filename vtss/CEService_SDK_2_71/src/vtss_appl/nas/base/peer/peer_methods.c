/*
 * EAP peer: Method registration
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

#include "peer_methods.h"

static struct eap_method *eap_methods = NULL;

/**
 * eap_peer_get_eap_method - Get EAP method based on type number
 * @vendor: EAP Vendor-Id (0 = IETF)
 * @method: EAP type number
 * Returns: Pointer to EAP method or %NULL if not found
 */
const struct eap_method *eap_peer_get_eap_method(int vendor, EapType method)
{
    struct eap_method *m;
    for (m = eap_methods; m; m = m->next) {
        if (m->vendor == vendor && m->method == method) {
            return m;
        }
    }
    return NULL;
}

/**
 * eap_peer_get_type - Get EAP type for the given EAP method name
 * @name: EAP method name, e.g., TLS
 * @vendor: Buffer for returning EAP Vendor-Id
 * Returns: EAP method type or %EAP_TYPE_NONE if not found
 *
 * This function maps EAP type names into EAP type numbers based on the list of
 * EAP methods included in the build.
 */
EapType eap_peer_get_type(const char *name, int *vendor)
{
    struct eap_method *m;
    for (m = eap_methods; m; m = m->next) {
        if (strcmp(m->name, name) == 0) {
            *vendor = m->vendor;
            return m->method;
        }
    }
    *vendor = EAP_VENDOR_IETF;
    return EAP_TYPE_NONE;
}

/**
 * eap_get_name - Get EAP method name for the given EAP type
 * @vendor: EAP Vendor-Id (0 = IETF)
 * @type: EAP method type
 * Returns: EAP method name, e.g., TLS, or %NULL if not found
 *
 * This function maps EAP type numbers into EAP type names based on the list of
 * EAP methods included in the build.
 */
const char *eap_get_name(int vendor, EapType type)
{
    struct eap_method *m;
    for (m = eap_methods; m; m = m->next) {
        if (m->vendor == vendor && m->method == type) {
            return m->name;
        }
    }
    return NULL;
}

/**
 * eap_get_names - Get space separated list of names for supported EAP methods
 * @buf: Buffer for names
 * @buflen: Buffer length
 * Returns: Number of characters written into buf (not including nul
 * termination)
 */
size_t eap_get_names(char *buf, size_t buflen)
{
    char *pos, *end;
    struct eap_method *m;
    int ret;

    if (buflen == 0) {
        return 0;
    }

    pos = buf;
    end = pos + buflen;

    for (m = eap_methods; m; m = m->next) {
        ret = snprintf(pos, end - pos, "%s%s", m == eap_methods ? "" : " ", m->name);
        if (ret < 0 || ret >= end - pos) {
            break;
        }
        pos += ret;
    }
    buf[buflen - 1] = '\0';

    return pos - buf;
}

/**
 * eap_peer_get_methods - Get a list of enabled EAP peer methods
 * @count: Set to number of available methods
 * Returns: List of enabled EAP peer methods
 */
const struct eap_method *eap_peer_get_methods(size_t *count)
{
    int c = 0;
    struct eap_method *m;

    for (m = eap_methods; m; m = m->next) {
        c++;
    }

    *count = c;
    return eap_methods;
}

/**
 * eap_peer_method_register - Register an EAP peer method
 * @method: EAP method to register
 * Returns: 0 on success, -1 on invalid method, or -2 if a matching EAP method
 * has already been registered
 *
 * Each EAP peer method needs to call this function to register itself as a
 * supported EAP method.
 */
int eap_peer_method_register(struct eap_method *method)
{
    struct eap_method *m, *last = NULL;

    if (method == NULL || method->name == NULL || method->version != EAP_PEER_METHOD_INTERFACE_VERSION) {
        return -1;
    }

    for (m = eap_methods; m; m = m->next) {
        if ((m->vendor == method->vendor && m->method == method->method) || strcmp(m->name, method->name) == 0) {
            return -2;
        }
        last = m;
    }

    if (last) {
        last->next = method;
    } else {
        eap_methods = method;
    }

    return 0;
}

/**
 * eap_peer_register_methods - Register statically linked EAP peer methods
 * Returns: 0 on success, -1 on failure
 *
 * This function is called at program initialization to register all EAP peer
 * methods that were linked in statically.
 */
int eap_peer_register_methods(void)
{
    int ret = 0;

    // Just to allow this function to be called several times
    // we need to NULLify the methods list.
    eap_methods = NULL;

// #ifdef EAP_MD5
    if (ret == 0) {
        int eap_peer_md5_register(void);
        ret = eap_peer_md5_register();
    }
// #endif

// Currently we only support the MD5 method
#if 0
#ifdef EAP_TLS
    if (ret == 0) {
        int eap_peer_tls_register(void);
        ret = eap_peer_tls_register();
    }
#endif /* EAP_TLS */

#ifdef EAP_MSCHAPv2
    if (ret == 0) {
        int eap_peer_mschapv2_register(void);
        ret = eap_peer_mschapv2_register();
    }
#endif /* EAP_MSCHAPv2 */

#ifdef EAP_PEAP
    if (ret == 0) {
        int eap_peer_peap_register(void);
        ret = eap_peer_peap_register();
    }
#endif /* EAP_PEAP */

#ifdef EAP_TTLS
    if (ret == 0) {
        int eap_peer_ttls_register(void);
        ret = eap_peer_ttls_register();
    }
#endif /* EAP_TTLS */

#ifdef EAP_GTC
    if (ret == 0) {
        int eap_peer_gtc_register(void);
        ret = eap_peer_gtc_register();
    }
#endif /* EAP_GTC */

#ifdef EAP_OTP
    if (ret == 0) {
        int eap_peer_otp_register(void);
        ret = eap_peer_otp_register();
    }
#endif /* EAP_OTP */

#ifdef EAP_SIM
    if (ret == 0) {
        int eap_peer_sim_register(void);
        ret = eap_peer_sim_register();
    }
#endif /* EAP_SIM */

#ifdef EAP_LEAP
    if (ret == 0) {
        int eap_peer_leap_register(void);
        ret = eap_peer_leap_register();
    }
#endif /* EAP_LEAP */

#ifdef EAP_PSK
    if (ret == 0) {
        int eap_peer_psk_register(void);
        ret = eap_peer_psk_register();
    }
#endif /* EAP_PSK */

#ifdef EAP_AKA
    if (ret == 0) {
        int eap_peer_aka_register(void);
        ret = eap_peer_aka_register();
    }
#endif /* EAP_AKA */

#ifdef EAP_FAST
    if (ret == 0) {
        int eap_peer_fast_register(void);
        ret = eap_peer_fast_register();
    }
#endif /* EAP_FAST */

#ifdef EAP_PAX
    if (ret == 0) {
        int eap_peer_pax_register(void);
        ret = eap_peer_pax_register();
    }
#endif /* EAP_PAX */

#ifdef EAP_SAKE
    if (ret == 0) {
        int eap_peer_sake_register(void);
        ret = eap_peer_sake_register();
    }
#endif /* EAP_SAKE */

#ifdef EAP_GPSK
    if (ret == 0) {
        int eap_peer_gpsk_register(void);
        ret = eap_peer_gpsk_register();
    }
#endif /* EAP_GPSK */

#ifdef EAP_VENDOR_TEST
    if (ret == 0) {
        int eap_peer_vendor_test_register(void);
        ret = eap_peer_vendor_test_register();
    }
#endif /* EAP_VENDOR_TEST */

#ifdef EAP_TNC
    if (ret == 0) {
        int eap_peer_tnc_register(void);
        ret = eap_peer_tnc_register();
    }
#endif /* EAP_TNC */
#endif /* if 0 */
    return ret;
}


/**
 * eap_peer_unregister_methods - Unregister EAP peer methods
 *
 * This function is called at program termination to unregister all EAP peer
 * methods.
 */
#ifdef __unused_deadweight__
void eap_peer_unregister_methods(void)
{
    struct eap_method *m;
#ifdef CONFIG_DYNAMIC_EAP_METHODS
    void *handle;
#endif /* CONFIG_DYNAMIC_EAP_METHODS */

    while (eap_methods) {
        m = eap_methods;
        eap_methods = eap_methods->next;

#ifdef CONFIG_DYNAMIC_EAP_METHODS
        handle = m->dl_handle;
#endif /* CONFIG_DYNAMIC_EAP_METHODS */

        if (m->free) {
            m->free(m);
        } else {
            eap_peer_method_free(m);
        }

#ifdef CONFIG_DYNAMIC_EAP_METHODS
        if (handle) {
            dlclose(handle);
        }
#endif /* CONFIG_DYNAMIC_EAP_METHODS */
    }
}
#endif

