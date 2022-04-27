/* iscdhcp_ecos.h

   Platform parameters definition for eCos. */

/*
 * Copyright(c) 2004-2008 by Internet Systems Consortium, Inc.("ISC")
 * Copyright(c) 1997-2003 by Internet Software Consortium
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *   Internet Systems Consortium, Inc.
 *   950 Charter Street
 *   Redwood City, CA 94063
 *   <info@isc.org>
 *   http://www.isc.org/
 *
 * This software has been written for Internet Systems Consortium
 * by Ted Lemon in cooperation with Vixie Enterprises and Nominum, Inc.
 * To learn more about Internet Systems Consortium, see
 * ``http://www.isc.org/''.  To learn more about Vixie Enterprises,
 * see ``http://www.vix.com''.   To learn more about Nominum, Inc., see
 * ``http://www.nominum.com''.
 */

#ifndef _ISCDHCP_ECOS_H_
#define _ISCDHCP_ECOS_H_

/* DHCP relay conuters */
typedef struct {
    unsigned long server_packets_relayed;       /* Packets relayed from server to client. */
    unsigned long server_packet_errors;         /* Errors sending packets to servers. */
    unsigned long client_packets_relayed;       /* Packets relayed from client to server. */
    unsigned long client_packet_errors;         /* Errors sending packets to clients. */
    unsigned long  agent_option_errors;         /* Number of packets forwarded without
                                                   agent options because there was no room. */
    unsigned long missing_agent_option;         /* Number of packets dropped because no
                                                   RAI option matching our ID was found. */
    unsigned long bad_circuit_id;               /* Circuit ID option in matching RAI option
                                                   did not match any known circuit ID. */
    unsigned long missing_circuit_id;           /* Circuit ID option in matching RAI option
                                                   was missing. */
    unsigned long bad_remote_id;                /* Remote ID option in matching RAI option
                                                   did not match any known remote ID. */
    unsigned long missing_remote_id;            /* Remote ID option in matching RAI option
                                                   was missing. */
    unsigned long receive_server_packets;       /* Receive DHCP message from server */
    unsigned long receive_client_packets;       /* Receive DHCP message from client */
    unsigned long receive_client_agent_option;  /* Receive relay agent information option from client */
    unsigned long replace_agent_option;         /* Replace relay agent information option */
    unsigned long keep_agent_option;            /* Keep relay agent information option */
    unsigned long drop_agent_option;            /* Drop relay agent information option */
} iscdhcp_relay_counter_t;

typedef int (*iscdhcp_reply_update_circuit_id_callback_t)(unsigned char *mac, unsigned int transaction_id, unsigned char *circuit_id);
typedef int (*iscdhcp_reply_check_circuit_id_callback_t)(unsigned char *mac, unsigned int transaction_id, unsigned char *circuit_id);
typedef int (*iscdhcp_reply_send_client_callback_t)(char *raw, size_t len, struct sockaddr_in *to, unsigned char *mac, unsigned int transaction_id);
typedef void (*iscdhcp_reply_send_server_callback_t)(char *raw, size_t len, unsigned long to);
typedef void (*iscdhcp_reply_fill_giaddr_callback_t)(unsigned char *mac, unsigned int transaction_id, uint32_t *agent_ip_addr);

/* Get counters */
void iscdhcp_get_couters(iscdhcp_relay_counter_t *counters);

/* Set counters */
void iscdhcp_set_couters(iscdhcp_relay_counter_t *counters);

/* Clear counters */
void iscdhcp_clear_couters(void);

/* Callback function for update circut ID
   Return -1: Update circut ID fail
   Return  0: Update circut ID success */
void iscdhcp_reply_update_circuit_id_register(iscdhcp_reply_update_circuit_id_callback_t cb);

/* Callback function for check circut ID
   Return -1: circut ID is invalid
   Return  0: circut ID is valid */
void iscdhcp_reply_check_circuit_id_register(iscdhcp_reply_check_circuit_id_callback_t cb);

/* Callback function for send DHCP message to client
   Return -1: send packet fail
   Return  0: send packet success */
void iscdhcp_reply_send_client_register(iscdhcp_reply_send_client_callback_t cb);

/* Callback function for send DHCP message to server
   Return -1: send packet fail
   Return  0: send packet success */
void iscdhcp_reply_send_server_register(iscdhcp_reply_send_server_callback_t cb);

/* Callback function for fill giaddr */
void iscdhcp_reply_fill_giaddr_register(iscdhcp_reply_fill_giaddr_callback_t cb);

/* Set platform MAC address */
void iscdhcp_set_platform_mac(unsigned char *platform_mac);

/* Set circuit ID */
void iscdhcp_set_circuit_id(unsigned char *circuit_id);

/* Set remote ID */
void iscdhcp_set_remote_id(unsigned char *remote_id);

/* Set relay mode operation */
void iscdhcp_relay_mode_set(unsigned long relay_mode);

/* Add DHCP relay server */
void iscdhcp_add_dhcp_server(unsigned long server);

/* Delete DHCP relay server */
void iscdhcp_del_dhcp_server(unsigned long server);

/* Clear DHCP relay server */
void iscdhcp_clear_dhcp_server(void);

/* Set relay agent information mode operation */
void iscdhcp_set_agent_info_mode(unsigned long relay_info_mode);

/* Set relay agent information policy */
void iscdhcp_set_relay_info_policy(unsigned long relay_info_policy);

/* Set relay Maximum hop count */
void iscdhcp_set_max_hops(unsigned long relay_max_hops);

/* change interface IP address */
void iscdhcp_change_interface_addr(char *interface_name, struct in_addr *interface_addr);

/* ISC DHCP initial function */
int iscdhcp_init(void);

#endif /* _ISCDHCP_ECOS_CONFIG_H_ */
