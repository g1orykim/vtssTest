/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_SYSTEM_API_H_
#define _VTSS_SYSTEM_API_H_

#include "main.h"

/* system error codes (vtss_rc) */
typedef enum {
    SYSTEM_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_SYSTEM),  /* Generic error code */
    SYSTEM_ERROR_PARM,                      /* Illegal parameter */
    SYSTEM_ERROR_STACK_STATE,               /* Illegal MASTER/SLAVE state */
    SYSTEM_ERROR_NOT_ADMINISTRATIVELY_NAME, /* Illegal administratively name */
} system_error_t;

/* system services */
typedef enum {
    SYSTEM_SERVICES_PHYSICAL     = 1,     /* layer functionality 1 physical (e.g., repeaters) */
    SYSTEM_SERVICES_DATALINK     = 2,     /* layer functionality 2 datalink/subnetwork (e.g., bridges) */
    SYSTEM_SERVICES_INTERNET     = 4,     /* layer functionality 3 internet (e.g., supports the IP) */
    SYSTEM_SERVICES_END_TO_END   = 8,     /* layer functionality 4 end-to-end (e.g., supports the TCP) */
    SYSTEM_SERVICES_SESSION      = 16,     /* layer functionality 5 session */
    SYSTEM_SERVICES_PRESENTATION = 32,    /* layer functionality 6 presentation */
    SYSTEM_SERVICES_APPLICATION  = 64     /* layer functionality 7 application (e.g., supports sthe SMTP) */
} system_services_t;

/* system configuration */
#define VTSS_SYS_ADMIN_NAME         "admin"
#define VTSS_SYS_STRING_LEN         256  /* The valid length is 0-255 */
#define VTSS_SYS_INPUT_STRING_LEN   255
#define VTSS_SYS_USERNAME_LEN       32  /* The valid length is 0-31 */
#define VTSS_SYS_INPUT_USERNAME_LEN 31
#define VTSS_SYS_PASSWD_LEN         32  /* The valid length is 0-31 */
#define VTSS_SYS_INPUT_PASSWD_LEN   31
#define VTSS_SYS_HOSTNAME_LEN       46  /* The valid length is 0-45 */
#define VTSS_SYS_INPUT_HOSTNAME_LEN       (VTSS_SYS_HOSTNAME_LEN - 1 )

typedef struct {
    char sys_contact[VTSS_SYS_STRING_LEN];
    char sys_name[VTSS_SYS_STRING_LEN];
    char sys_location[VTSS_SYS_STRING_LEN];
#ifndef VTSS_SW_OPTION_USERS
    char sys_passwd[VTSS_SYS_PASSWD_LEN];
#endif /* VTSS_SW_OPTION_USERS */
    int  sys_services;
    int  tz_off;                /* +- 720 minutest */
} system_conf_t;

/* system error text */
char *system_error_txt(vtss_rc rc);

/* check string is administratively name */
BOOL system_name_is_administratively(char string[VTSS_SYS_STRING_LEN]);

int system_get_tz_off(void);                /* TZ offset in minutes */
const char *system_get_tz_display(void);    /* TZ offset for display per ISO8601: +-hhmm */

/* Get system description */
char *system_get_descr(void);

/* Get system configuration */
vtss_rc system_get_config(system_conf_t *conf);

/* Set system configuration */
vtss_rc system_set_config(system_conf_t *conf);

#ifndef VTSS_SW_OPTION_USERS
/* Get system passwd */
const char *system_get_passwd(void);

/* Set system passwd */
vtss_rc system_set_passwd(char *pass);
#endif /* VTSS_SW_OPTION_USERS */

/* Initialize module */
vtss_rc system_init(vtss_init_data_t *data);

#endif /* _VTSS_SYSTEM_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

