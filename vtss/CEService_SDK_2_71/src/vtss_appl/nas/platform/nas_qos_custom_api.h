/*

 Vitesse Switch API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _NAS_QOS_CUSTOM_API_H_
#define _NAS_QOS_CUSTOM_API_H_

/**
 * \file nas_qos_custom_api.h
 * \brief This file defines the RADIUS attribute types used in
 *        identifying the QoS class possibly handed to the Authenticator
 *        in a RADIUS Access-Accept packet.
 *
 *  The contents are only used when VTSS_SW_OPTION_NAS_BACKEND_ASSIGNED_QOS_CUSTOM
 *  is defined in .../build/make/module_dot1x.in.
 *  An alternative to using the Vendor-specific attribute is to use RFC4675. To use
 *  that one for identifying QoS attributes sent from the RADIUS Server, please
 *  have a look in .../build/make/module_dot1x.in.
 *
 * The format of the Vendor-specific is described in http://www.ietf.org/rfc/rfc2865.txt.
 *
 * AVP = Attribute Value Pair (RADIUS attribute, Length, and Value)
 * VSA = Vendor-Specific Attribute (Vendor-defined attribute/length/values)
 *
 * The whole RADIUS TLV will be interpreted as:
 * 1 octet  AVP-Type     (NAS_QOS_CUSTOM_AVP_TYPE; should be set to 26 (Vendor-Specific))
 * 1 octet  AVP-Length   (length in octects of the whole AVP, including all VSA_xxx fields)
 * 4 octets AVP-VendorID (NAS_QOS_CUSTOM_VENDOR_ID; MSByte must be 0 to comply with RFC2865)
 * 1 octet  VSA-Type     (NAS_QOS_CUSTOM_VSA_TYPE; Vendor-specific sub-type)
 * 1 octet  VSA-Length   (includes VSA-type, VSA-Length, VSA-prefix, and VSA-Value).
 * X octets VSA-Prefix   (NAS_QOS_CUSTOM_VSA_PREFIX; a string that identifies the QoS attribute).
 * Y octets VSA-Value    (the actual QoS class, which is an ASCII representation of a decimal
 *                        number in the range [0; VTSS_PRIOS[, or, if VTSS_PRIOS == 4,
 *                        a string drawn from the following set of of case-insensitive
 *                        enumerations: "low", "normal", "medium", "high", corresponding
 *                        to 0, 1, 2, and 3, respectively).
 *
 * The whole AVP-pair is thus 1 + 1 + 4 + 1 + 1 + X + Y = 8 + X + Y octets long.
 * This number must stay below 255 and the AVP cannot be split into several
 * AVP pairs as can with (some) other types.
 */

#include "vtss_radius_api.h" /* For VTSS_RADIUS_ATTRIBUTE_xxx */

/**
  * \brief This attribute type defines the top-level attribute used to identify
  *        the QoS attribute.
  *
  * THIS SHOULD NEVER BE CHANGED!
  */
#define NAS_QOS_CUSTOM_AVP_TYPE VTSS_RADIUS_ATTRIBUTE_VENDOR_SPECIFIC

/**
  * \brief Vendor ID used within the vendor-specific attribute.
  *
  * This must be a Network Management Private Enterprise Code of the Vendor. See
  * http://www.iana.org/assignments/enterprise-numbers
  *
  * E.g. 9 is Cisco and 5610 is Vitesse (registered as Exbit Technology A/S).
  */
#define NAS_QOS_CUSTOM_VENDOR_ID 5610

/**
  * \brief Vendor-specific sub-type
  *
  * 1 binary octet identifying the vendor's sub-type.
  */
#define NAS_QOS_CUSTOM_VSA_TYPE 1

/**
  * \brief Vendor-specific Prefix
  *
  * Usually an ASCII string identifying the vendor-specific
  * attribute in question.
  *
  */
#define NAS_QOS_CUSTOM_VSA_PREFIX "qos-class="

#endif
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
