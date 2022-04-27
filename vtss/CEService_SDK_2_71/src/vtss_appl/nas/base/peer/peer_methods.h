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

#ifndef PEER_METHODS_H__
#define PEER_METHODS_H__

#include "../common/nas_types.h"
#include "peer_sm.h"

const struct eap_method *eap_peer_get_eap_method(int vendor, EapType method);
const struct eap_method *eap_peer_get_methods(size_t *count);

struct eap_method *eap_peer_method_alloc(int version, int vendor, EapType method, const char *name);
void eap_peer_method_free(struct eap_method *method);
int eap_peer_method_register(struct eap_method *method);

EapType eap_peer_get_type(const char *name, int *vendor);
const char *eap_get_name(int vendor, EapType type);
size_t eap_get_names(char *buf, size_t buflen);
int eap_peer_register_methods(void);

#endif /* PEER_METHODS_H__ */
