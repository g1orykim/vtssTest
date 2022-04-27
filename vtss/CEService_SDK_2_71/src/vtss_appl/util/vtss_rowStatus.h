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

*/

#ifndef __ROWENTRY_H__
#define __ROWENTRY_H__

#include "main.h"
#include <ucd-snmp/config.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif                          /* HAVE_STDLIB_H */
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif                          /* HAVE_STRING_H */

#include <ucd-snmp/mibincl.h>   /* Standard set of SNMP includes */

typedef enum {
    ROWENTRY_STATE_NOT_EXISTENT,
    ROWENTRY_STATE_NOT_READY,
    ROWENTRY_STATE_NOT_IN_SERVICE,
    ROWENTRY_STATE_ACTIVE,
    ROWENTRY_STATE_END
} rowEntry_status_t;

typedef enum {
    ROWENTRY_ACTION_ACTIVE = 1,
    ROWENTRY_ACTION_NOT_IN_SERVICE,
    ROWENTRY_ACTION_CREATE_AND_GO = 4,
    ROWENTRY_ACTION_CREATE_AND_WAIT,
    ROWENTRY_ACTION_DESTROY,
    ROWENTRY_ACTION_SET,
    ROWENTRY_ACTION_END
} rowEntry_action_t;

#define ROWENTRY_NO_CREATE_AND_WAIT   (1 << 0)
#define ROWENTRY_NO_NOT_IN_SERVICE    (1 << 1)
#define ROWENTRY_NO_DESTROY           (1 << 2)
#define ROWENTRY_NO_SET               (1 << 3)

typedef unsigned char rowEntry_cap_t[ROWENTRY_STATE_END];

typedef enum {
/*   Standard error codes for rowStatus. */
    ROWENTRY_RC_NO_ERROR = SNMP_ERR_NOERROR,
    ROWENTRY_RC_INCONSISTENT_VAL = SNMP_ERR_INCONSISTENTVALUE,
    ROWENTRY_RC_INCONSISTENT_NAME = SNMP_ERR_INCONSISTENTNAME,
    ROWENTRY_RC_WRONG_VAL = SNMP_ERR_WRONGVALUE,
    ROWENTRY_RC_END
} rowEntry_rc_t;

typedef enum {
    RFC2579_STATE_NOT_EXISTENT,
    RFC2579_STATE_ACTIVE = SNMP_ROW_ACTIVE,
    RFC2579_STATE_NOT_IN_SERVICE,
    RFC2579_STATE_NOT_READY,
    RFC2579_STATE_CREATE_AND_GO,
    RFC2579_STATE_CREATE_AND_WAIT,
    RFC2579_STATE_DESTROY,
    RFC2579_STATE_END
} rfc2579_status_t;


/**
  * \brief Transit the rowStatus to next state.
  *
  * \param cur_state    [IN]:    The current state.\n
  * \param action       [IN]:    The action for the current state.\n
  * \param params_valid [IN]:    The parameters are valid.\n
  * \param not_support  [IN]:    The no supporting capabilities in each state.\n
  * \param next_state   [OUT]:   The pointer of next state depends on input parameters. If the transition fails, the parameter・s value can・t be modified. If the pointer is NULL, the API will ignore the output\n
  * \param return       [OUT]:   The pointer of returning code depends on the input parameters. If the transition fails, the parameter・s value can・t be modified. If the pointer is NULL, the API will ignore the output.\n

  * \return
  *    FALSE if the transition fails.\n
  *     Otherwise, return TRUE.
  */

BOOL rowStatus_nextState(rowEntry_status_t cur_state,
                         rowEntry_action_t action,
                         BOOL   params_valid,
                         rowEntry_cap_t not_support,
                         rowEntry_status_t *next_state,
                         rowEntry_rc_t *rc );

/**
  * \brief Convert RFC2579 row status to VTSS row status.
  *
  * \param rfc_state        [IN]:    The RFC2579 row state.\n
  * \param rowEntry_state   [OUT]:   The VTSS row status.\n

  * \return
  *    FALSE if the convertion fails.\n
  *     Otherwise, return TRUE.
  */

BOOL rfc2579_2_rowEntry(rfc2579_status_t rfc_state, rowEntry_status_t *rowEntry_state);

/**
  * \brief Convert VTSS row status to RFC2579 row status.
  *
  * \param rowEntry_state   [IN]:   The VTSS row status.\n
  * \param rfc_state        [OUT]:  The RFC2579 row state.\n

  * \return
  *    FALSE if the convertion fails.\n
  *     Otherwise, return TRUE.
  */

BOOL rowEntry_2_rfc2579(rowEntry_status_t rowEntry_state, rfc2579_status_t *rfc_state);

#endif /*   __ROWENTRY_H__  */
