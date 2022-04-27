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

 $Id$
 $Revision$

*/

/**
 * \file
 * \brief LLDP-MED iCLI functions
 * \details This header file describes LLDP iCLI functions
 */


#ifndef _VTSS_ICLI_LLDPMED_H_
#define _VTSS_ICLI_LLDPMED_H_

/**
 * \brief Function for configuration of the LLDP-MED civic address
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param v_string250 [IN] The value of the selected civic address parameter.
 * \param has_* [IN]  TRUE if the v_string250 is the corresponding LLDP-MED civic address parameter.
 * \return error code.
 **/
vtss_rc lldpmed_icli_civic_addr(i32 session_id, BOOL has_country, BOOL has_state, BOOL has_county, BOOL has_city, BOOL has_district,
                                BOOL has_block, BOOL has_street, BOOL has_leading_street_direction, BOOL has_trailing_street_suffix,
                                BOOL has_str_suf, BOOL has_house_no, BOOL has_house_no_suffix, BOOL has_landmark, BOOL has_additional_info,
                                BOOL has_name, BOOL has_zip_code, BOOL has_building, BOOL has_apartment, BOOL has_floor,
                                BOOL has_room_number, BOOL has_place_type, BOOL has_postal_com_name, BOOL has_p_o_box, BOOL has_additional_code,
                                char *v_string250);

/**
 * \brief Function for configuration of the LLDP-MED ELIN parameter
 *
 * \param session_id [IN]  The session id.
 * \param elin_string [IN] The ELIN value .
 * \return error code.
 **/
vtss_rc lldpmed_icli_elin_addr(i32 session_id, char *elin_string);

/**
 * \brief Function for showing lldp med neighbor status
 *
 * \param session_id [IN]  The session id.
 * \param has_interface [IN] TRUE if user has specified a specific interface
 * \param plist [IN]  Port list in case user has specified a specific interface.
 * \return None.
 **/
void lldpmed_icli_show_remote_device(i32 session_id, BOOL has_interface, icli_stack_port_range_t *plist);

/**
 * \brief Function for showing lldp policies
 *
 * \param session_id [IN]  The session id.
 * \param policies_list [IN]  List of policies to show.
 * \return None.
 **/
void lldpmed_icli_show_policies(i32 session_id, icli_unsigned_range_t *policies_list);

/**
 * \brief Function for configuring LLDP-MED latitude
 *
 * \param session_id [IN]  The session id.
 * \param north IN]  TRUE to set latitude direction to north
 * \param south [IN]  TRUE to set latitude direction to south
 * \param degree [IN]  Latitude degrees value
 * \return error code.
 **/
vtss_rc lldpmed_icli_latitude(i32 session_id, BOOL north, BOOL south, char *degree);

/**
 * \brief Function for configuring LLDP-MED longitude
 *
 * \param session_id [IN]  The session id.
 * \param west IN]  TRUE to set latitude direction to north
 * \param south [IN]  TRUE to set latitude direction to south
 * \param degree [IN]  Latitude degrees value
 * \return error code.
 **/
vtss_rc lldpmed_icli_longitude(i32 session_id, BOOL east, BOOL west, char *degree);

/**
 * \brief Function for configuring LLDP-MED altitude
 *
 * \param session_id [IN]  The session id.
 * \param meters IN]  TRUE to set altitude to meters
 * \param floors [IN]  TRUE to set altitude to floors.
 * \param value_str [IN]  Value of altitude.
 * \return error code.
 **/
vtss_rc lldpmed_icli_altitude(i32 session_id, BOOL meters, BOOL floors, char *value_str);


/**
 * \brief Function for configuring LLDP-MED optional TLVs
 *
 * \param session_id [IN]  The session id.
 * \param plist  [IN]  Port list with ports to configure.
 * \param has_capabilities [IN]  TRUE if optional capabilities TLV shall be enabled.
 * \param has_location [IN]  TRUE if optional location TLV shall be enabled.
 * \param has_network_policy [IN]  TRUE if optional network_policy TLV shall be enabled.
 * \param no [IN] TRUE to set optional TLVs to their default values
 * \return error code.
 **/
vtss_rc lldpmed_icli_transmit_tlv(i32 session_id, icli_stack_port_range_t *plist, BOOL has_capabilities, BOOL has_location, BOOL has_network_policy, BOOL no);

/**
 * \brief Function for configuring LLDP-MED datum
 *
 * \param session_id [IN]         The session id.
 * \param has_wgs84  [IN]         TRUE to set datum to WGS84.
 * \param has_nad83_navd88  [IN]  TRUE to set datum to NAD83_NAVD88.
 * \param has_nad83_mllw  [IN]    TRUE to set datum to NAD83_MLLW.
 * \param no [IN]                 TRUE to set datum to the default value.
 * \return error code.
 **/
vtss_rc lldpmed_icli_datum(i32 session_id, BOOL has_wgs84, BOOL has_nad83_navd88, BOOL has_nad83_mllw, BOOL no);

/*
 * \brief Function for configuring LLDP-MED fast repeat transmission
 *
 * \param session_id [IN]  The session id.
 * \param value [IN]       The number of time to repeat fast LLDP transmission
 * \param no [IN]          TRUE to set fast repeat count to the default value.
 * \return error code.
 **/
vtss_rc lldpmed_icli_fast_start(i32 session, u32 value, BOOL no);


/*
 * \brief Function for configuring LLDP-MED policies
 *
 * \param session_id [IN]  The session id.
 * \param index [IN]  The policy index
 * \param has_voice [IN]  TRUE is policy is a voice policy.
 * \param has_voice_signaling [IN]  TRUE is policy is a voice signaling policy.
 * \param has_guest_voice_signaling [IN]  TRUE is policy is a guest voice signaling policy.
 * \param has_guest_voice [IN]  TRUE is policy is a guest voice policy.
 * \param has_softphone_voice [IN]  TRUE is policy is a softphone policy.
 * \param has_video_conferencing [IN]  TRUE is policy is a video conferencing policy.
 * \param has_streaming_video [IN]  TRUE is policy is a streaming video policy.
 * \param has_video_signaling [IN]  TRUE is policy is a video signaling policy.
 * \param has_tagged [IN]  TRUE is policy is tagged.
 * \param has_untagged [IN]  TRUE is policy is un-tagged.
 * \param v_vlan_id [IN]  Value of the policy vlan.
 * \param v_0_to_7 [IN] L2 Priority value
 * \param v_0_to_63 [IN] DSCP value
 * \return error code.
 **/
vtss_rc lldpmed_icli_media_vlan_policy(i32 session_id, u32 index, BOOL has_voice, BOOL has_voice_signaling, BOOL has_guest_voice_signaling,
                                       BOOL has_guest_voice, BOOL has_softphone_voice, BOOL has_video_conferencing, BOOL has_streaming_video,
                                       BOOL has_video_signaling, BOOL has_tagged, BOOL has_untagged, u32 v_vlan_id, u32 v_0_to_7, u32 v_0_to_63);

/*
 * \brief Function for deleting LLDP-MED policies
 * \param session_id [IN]  The session id.
 * \param policies_list [IN]  The list of policies to delete.
 **/
vtss_rc lldpmed_icli_media_vlan_policy_delete(i32 session_id, icli_unsigned_range_t *policies_list);

/*
 * \brief Function for assigning policies to the ports.
 *
 * \param session_id [IN]  The session id.
 * \param plist [IN] List of port to assign the policies to.
 * \param policy_list [IN] List of policies to assign to the given ports.
 * \return error code.
 **/
vtss_rc lldpmed_icli_assign_policy(i32 session_id, icli_stack_port_range_t *plist, icli_range_t *policy_list, BOOL no);

/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc lldpmed_icfg_init(void);


#endif /* _VTSS_ICLI_LLDPMED_H_ */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
