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

#ifndef _VTSS_NTP_API_H_
#define _VTSS_NTP_API_H_

#include "vtss_module_id.h"
#include "main.h"
#include "sysutil_api.h"    // For VTSS_SYS_HOSTNAME_LEN

#define STEP_ENTRY_NO   10

/* ntp managent enabled/disabled */
#define NTP_MGMT_ENABLED       1
#define NTP_MGMT_DISABLED      0
#define NTP_MGMT_INITIAL       2

#define NTP_MAX_SERVER_COUNT 5
#define NTP_DEFAULT_INTERVAL 6  /* log2 default poll interval (64 s) */
#define NTP_MININTERVAL      4  /* log2 min poll interval (16 s) */
#define NTP_MAXINTERVAL      14 /* log2 max poll interval (16384 s) */

/* ntp ip type */
#define NTP_IP_TYPE_IPV4     0x0
#define NTP_IP_TYPE_IPV6     0x1

/* ntp error codes (vtss_rc) */
typedef enum {
    NTP_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_NTP),     /* Generic error code */
    NTP_ERROR_PARM,                                             /* Illegal parameter */
    NTP_ERROR_STACK_STATE,                                      /* Illegal MASTER/SLAVE state */
    NTP_ERROR_GET_CERT_INFO,                                    /* Illegal get certificate information */
} ntp_error_t;

/* ntp configuration */
typedef struct {
    ulong       ip_type;                 /* IP type 0 is IPv4, 1 is IPv6 */
    char        ip_host_string[VTSS_SYS_HOSTNAME_LEN];
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t ipv6_addr;               /* IPv6 address */
#endif
} ntp_server_config_t;

typedef struct {
    BOOL                mode_enabled;
    ulong               interval_min;
    ulong               interval_max;
    ntp_server_config_t server[NTP_MAX_SERVER_COUNT];
    char                drift_valid;   /* Indicates if drift_data valid */
    int                 drift_data;    /* default PPM */
    char                drift_trained; /* It is a test flag. When on, it means
                                          the NTP state will be initialized at S_FSET,
                                          Otherwise it will be at S_NSET which will takes
                                          15 minutes to calculate the frequency when enabling
                                          NTP.
                                        */
} ntp_conf_t;

typedef struct {
    ulong   currentime;
    int     drift;  /*default PPM*/
    int     currentppm; /* current PPM*/
    ulong   updatime;
    double  offset;
    int     first_dppm;
    char    ip_string[16];
    int     max_dppm;
    int     min_dppm;
    ulong   step_count;
    ulong   step_time[STEP_ENTRY_NO];
    uchar   timer_rest_count;
    char    last_offset_in_range;
    double  last_offest;
    int     current_status;
} ntp_sys_status_t;

typedef struct {
    ulong   curr_time;
    char    ip_string[16];
    int     flag;
    int     poll_int;
    int     burst;
    ulong   lastupdate;
    ulong   nextupdate;
    double  offset;
} ntp_server_status_t;


typedef struct {
    ulong mu;
    double curr_offset;
    double last_offset;
    double result_frequency;
} ntp_freq_data_t;

/* poll_update: at 58051 220.130.158.72 flags 0241 poll 6 burst 3 last 58041 next 58053
   printf("poll_update: at %lu %s flags %04x poll %d burst %d last %lu next %lu\n",
            current_time, ntoa(&peer->srcadr), peer->flags,
            peer->hpoll, peer->burst, peer->outdate,
            peer->nextdate);

*/



/* ntp error text */
char *ntp_error_txt(ntp_error_t rc);

/* Get ntp system status */
void ntp_mgmt_sys_status_get(ntp_sys_status_t *status);

/* Get ntp frequency status */
void vtss_ntp_freq_get(ntp_freq_data_t *freq);

void
vtss_ntp_server_info(unsigned long curr_time, char *ip_str, int flag,
                     int poll, int burst, unsigned long lastupdate,
                     unsigned long nextupdate, double offset);

void vtss_ntp_last_sys_info(char in_range, double offset, int status);

void vtss_ntp_timer_reset(void);

void vtss_ntp_timer_init(void);

void vtss_ntp_timer_reset_counter(void);

void vtss_ntp_step_count_set(unsigned long  count);

void vtss_ntp_freq_info(ulong mu, double curr_offset, double last_offset,
                        double result_frequency);

void ntp_mgmt_sys_server_get(ntp_server_status_t *status, ulong *curr_time, int num);


void vtss_ntp_adj_time(double dppm, ulong updatime, double offset, char *ipstr);

void vtss_ntp_default_set(ntp_conf_t *conf);

/* Get ntp configuration */
vtss_rc ntp_mgmt_conf_get(ntp_conf_t *conf);

/* Set ntp configuration */
vtss_rc ntp_mgmt_conf_set(ntp_conf_t *conf);

/* Delete ntp configuration */
vtss_rc ntp_mgmt_server_del(int server_idx);

/* Initialize module */
vtss_rc ntp_init(vtss_init_data_t *data);

#endif /* _VTSS_NTP_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
