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

#include "vtss_rowStatus.h"

/*

A: not_existent
B: not_ready
C: not_inservice
D: destroy

format: <cur_state>/<action>/<params_valid>/<capability>/<rc>/<next_state>

<2>     A/createAndGo/FALSE/DC/inconsistentVal/A
<1>     A/createAndGo/TRUE/DC/noError/D
<5>     A/createAndWait/DC/FALSE/wrongVal/A
<3>     A/createAndWait/FALSE/TRUE/noError/B
<4>     A/createAndWait/TRUE/TRUE/noError/C
<6>     A/active/DC/DC/inconsistentVal/A 
<7>     A/notInService/DC/DC/inconsistentVal/A
<8>     A/destroy/DC/DC/noError)/A
<11>    A/set/DC/FALSE/inconsistentName/A
<9>     A/set/FALSE/TRUE/noError/B
<10>    A/set/TRUE/TRUE/noError/C

<12>    B/createAndGo/DC/DC/inconsistentVal/B
<13>    B/createAndWait/DC/DC/inconsistentVal/B
<14>    B/active/FALSE/DC/inconsistentVal)/B
<15>    B/active/TURE/DC/noError/D
<16>    B/notInService/FALSE/DC/inconsistentVal/B
<17>    B/notInService/TURE/DC/noError/C
<18>    B/destroy/DC/DC/noError/A
<19>    B/set/FALSE/DC/noError/B
<20>    B/set/TRUE/DC/noError/C

<21>    C/createAndGo/DC/DC/inconsistentVal/C
<22>    C/createAndWait/DC/DC/inconsistentVal/C
<24>    C/active/FALSE/DC/inconsistentVal/C
<23>    C/active/TRUE/DC/noError/D
<25>    C/notInService/DC/DC/noError/C
<26>    C/destroy/DC/DC/noError/A
<27>    C/set/DC/DC/noError/C

<28>    D/createAndGo/DC/DC/inconsistentVal/D
<29>    D/createAndWait/DC/DC/inconsistentVal/D
<30>    D/active/DC/DC/noError/D
<32>    D/notInService/DC/no_create_and_wait/wrongVal/D
<33>    D/notInService/DC/FALSE/inconsistentVal/D
<31>    D/notInService/DC/TRUE/noError/C
<35>    D/destroy/DC/FALSE/inconsistentVal/D
<34>    D/destroy/DC/TRUE/noError/A
<36>    D/set/TRUE/TURE/noError/D
<37>    D/set/FALSE/DC/inconsistentVal/D

*/


static BOOL nonExistent( rowEntry_action_t action, BOOL parameters_valid, rowEntry_cap_t cap, rowEntry_status_t *next_state, rowEntry_rc_t *rc )
{
    rowEntry_rc_t rc_tmp;
    rowEntry_status_t next_state_tmp = ROWENTRY_STATE_NOT_EXISTENT;
    switch(action) {
        case ROWENTRY_ACTION_CREATE_AND_GO:
            if ( FALSE == parameters_valid ) {
                rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <2> */
            } else {
                next_state_tmp = ROWENTRY_STATE_ACTIVE;
                rc_tmp = ROWENTRY_RC_NO_ERROR;         /*   <1> */
            }
            break;
        case ROWENTRY_ACTION_CREATE_AND_WAIT:
            if (0 != (cap[ROWENTRY_STATE_NOT_EXISTENT] & ROWENTRY_NO_CREATE_AND_WAIT)) {
                rc_tmp = ROWENTRY_RC_WRONG_VAL;         /*  <5> */
            } else if ( FALSE == parameters_valid ) {
                next_state_tmp = ROWENTRY_STATE_NOT_READY;
                rc_tmp = ROWENTRY_RC_NO_ERROR;         /*   <3> */
            } else {
                next_state_tmp = ROWENTRY_STATE_NOT_IN_SERVICE;
                rc_tmp = ROWENTRY_RC_NO_ERROR;         /*   <4> */
            }
            break;
        case ROWENTRY_ACTION_ACTIVE:
            rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <6> */
            break;
        case ROWENTRY_ACTION_NOT_IN_SERVICE:
            rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <7> */
            break;
        case ROWENTRY_ACTION_DESTROY:
            rc_tmp = ROWENTRY_RC_NO_ERROR;          /*   <8> */
            break;
        case ROWENTRY_ACTION_SET:
            if (0 != (cap[ROWENTRY_STATE_NOT_EXISTENT] & ROWENTRY_NO_SET)) {
                rc_tmp = ROWENTRY_RC_INCONSISTENT_NAME; /*   <11> */
            } else if ( FALSE == parameters_valid ) {
                next_state_tmp = ROWENTRY_STATE_NOT_READY;
                rc_tmp = ROWENTRY_RC_NO_ERROR;         /*   <9> */
            } else {
                next_state_tmp = ROWENTRY_STATE_NOT_IN_SERVICE;
                rc_tmp = ROWENTRY_RC_NO_ERROR;         /*   <10> */
            }
            break;
        default:
            return FALSE;
    }

    if(rc) {
        *rc = rc_tmp;
    }
    if(next_state) {
        *next_state = next_state_tmp;
    }
    return TRUE;

}

static rowEntry_rc_t notReady( rowEntry_action_t action, BOOL parameters_valid, rowEntry_cap_t cap, rowEntry_status_t *next_state, rowEntry_rc_t *rc ) 
{
    rowEntry_rc_t rc_tmp;
    rowEntry_status_t next_state_tmp = ROWENTRY_STATE_NOT_READY;

    switch(action) {
        case ROWENTRY_ACTION_CREATE_AND_GO:
        case ROWENTRY_ACTION_CREATE_AND_WAIT:
            rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <12>, <13>    */
            break;
        case ROWENTRY_ACTION_ACTIVE:
            if (FALSE == parameters_valid) {
                rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <14>    */
            } else {
                next_state_tmp = ROWENTRY_STATE_ACTIVE;
                rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <15>    */
            }
            break;
        case ROWENTRY_ACTION_NOT_IN_SERVICE:
            if (FALSE == parameters_valid) {
                rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <16>    */
            } else {
                next_state_tmp = ROWENTRY_STATE_NOT_IN_SERVICE;
                rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <17>    */
            }
            break;
        case ROWENTRY_ACTION_DESTROY:
            next_state_tmp = ROWENTRY_STATE_NOT_EXISTENT;
            rc_tmp = ROWENTRY_RC_NO_ERROR;          /*   <18> */
            break;
        case ROWENTRY_ACTION_SET:
            if (FALSE == parameters_valid) {
                next_state_tmp = ROWENTRY_STATE_NOT_READY;
                rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <19>    */
            } else {
                next_state_tmp = ROWENTRY_STATE_NOT_IN_SERVICE;
                rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <20>    */
            }
            break;
        default:
            return FALSE;

    }

    if(rc) {
        *rc = rc_tmp;
    }
    if(next_state) {
        *next_state = next_state_tmp;
    }
    return TRUE;
}


static rowEntry_rc_t notInService( rowEntry_action_t action, BOOL parameters_valid, rowEntry_cap_t cap, rowEntry_status_t *next_state, rowEntry_rc_t *rc ) 
{
    rowEntry_rc_t rc_tmp;
    rowEntry_status_t next_state_tmp = ROWENTRY_STATE_NOT_IN_SERVICE;

    switch(action) {
        case ROWENTRY_ACTION_CREATE_AND_GO:
        case ROWENTRY_ACTION_CREATE_AND_WAIT:
            rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <21>, <22>    */
            break;
        case ROWENTRY_ACTION_ACTIVE:
            if (FALSE == parameters_valid) {
                rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <24>    */
            } else {
                next_state_tmp = ROWENTRY_STATE_ACTIVE;
                rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <23>    */
            }
            break;
        case ROWENTRY_ACTION_NOT_IN_SERVICE:
            rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <25>    */
            break;
        case ROWENTRY_ACTION_DESTROY:
            next_state_tmp = ROWENTRY_STATE_NOT_EXISTENT;
            rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <26>    */
            break;
        case ROWENTRY_ACTION_SET:
            rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <27>    */
            break;
        default:
            return FALSE;
    }

    if(rc) {
        *rc = rc_tmp;
    }
    if(next_state) {
        *next_state = next_state_tmp;
    }
    return TRUE;

}

static rowEntry_rc_t active( rowEntry_action_t action, BOOL parameters_valid, rowEntry_cap_t cap, rowEntry_status_t *next_state, rowEntry_rc_t *rc ) 
{
    rowEntry_rc_t rc_tmp;
    rowEntry_status_t next_state_tmp = ROWENTRY_STATE_ACTIVE;

    switch(action) {
        case ROWENTRY_ACTION_CREATE_AND_GO:
        case ROWENTRY_ACTION_CREATE_AND_WAIT:
            rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL; /*   <28>, <29>    */
            break;
        case ROWENTRY_ACTION_ACTIVE:
            rc_tmp = ROWENTRY_RC_NO_ERROR; /*   <30>    */
            break;
        case ROWENTRY_ACTION_NOT_IN_SERVICE:
            if (0 != (cap[ROWENTRY_STATE_ACTIVE] & ROWENTRY_NO_CREATE_AND_WAIT)) {
                rc_tmp = ROWENTRY_RC_WRONG_VAL;         /*  <32> */
            } else if (0 != (cap[ROWENTRY_STATE_ACTIVE] & ROWENTRY_NO_NOT_IN_SERVICE)) {
                rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL;         /*  <33> */
            } else {
                next_state_tmp = ROWENTRY_STATE_NOT_IN_SERVICE;
                rc_tmp = ROWENTRY_RC_NO_ERROR;         /*  <31> */
            }
            break;
        case ROWENTRY_ACTION_DESTROY:
            if (0 != (cap[ROWENTRY_STATE_ACTIVE] & ROWENTRY_NO_DESTROY)) {
                rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL;         /*  <35> */
            }else {
                next_state_tmp = ROWENTRY_STATE_NOT_EXISTENT;
                rc_tmp = ROWENTRY_RC_NO_ERROR;         /*  <34> */
            }
            break;
        case ROWENTRY_ACTION_SET:
            if ( FALSE == parameters_valid || 0 != (cap[ROWENTRY_STATE_ACTIVE] & ROWENTRY_NO_SET)) {
                rc_tmp = ROWENTRY_RC_INCONSISTENT_VAL;         /*  <36> */
            } else {
                next_state_tmp = ROWENTRY_STATE_ACTIVE;
                rc_tmp = ROWENTRY_RC_NO_ERROR;         /*  <37> */
            }
            break;
        default:
            return FALSE;

    }
    if(rc) {
        *rc = rc_tmp;
    }
    if(next_state) {
        *next_state = next_state_tmp;
    }
    return TRUE;

}

/**
  * \brief Transit the rowStatus to next state.
  *
  * \param cur_state    [IN]:    The current state.\n
  * \param action       [IN]:    The action for the current state.\n
  * \param params_valid [IN]:    The parameters are valid.\n
  * \param capability   [IN]:    The capability in each state.\n
  * \param next_state   [OUT]:   The pointer of next state depends on input parameters. If the transition fails, the parameter・s value can・t be modified.\n
                                 If the pointer is NULL, the API will ignore the output\n
  * \param return       [OUT]:   The pointer of returning code depends on the input parameters. If the transition fails, the parameter・s value can・t be modified.\n 
                                 If the pointer is NULL, the API will ignore the output.\n
  * \return
  *    FALSE if the transition fails.\n
  *     Otherwise, return TRUE.
  */

BOOL rowStatus_nextState(rowEntry_status_t cur_state,
                         rowEntry_action_t action,
                         BOOL   params_valid,
                         rowEntry_cap_t capability,
                         rowEntry_status_t *next_state,
                         rowEntry_rc_t *rc )
{
    switch(cur_state) {
        case ROWENTRY_STATE_NOT_EXISTENT:
            return nonExistent(action, params_valid, capability, next_state, rc);
        case ROWENTRY_STATE_NOT_READY:
            return notReady(action, params_valid, capability, next_state, rc);
        case ROWENTRY_STATE_NOT_IN_SERVICE:
            return notInService(action, params_valid, capability, next_state, rc);
        case ROWENTRY_STATE_ACTIVE:
            return active(action, params_valid, capability, next_state, rc);
        default:
            return FALSE;
    }
}

/**
  * \brief Convert RFC2579 row status to VTSS row status.
  *
  * \param rfc_state        [IN]:    The RFC2579 row state.\n
  * \param rowEntry_state   [OUT]:   The VTSS row status.\n

  * \return
  *    FALSE if the convertion fails.\n
  *     Otherwise, return TRUE.
  */

BOOL rfc2579_2_rowEntry(rfc2579_status_t rfc_state, rowEntry_status_t *rowEntry_state)
{
   switch(rfc_state) {
       case RFC2579_STATE_ACTIVE:
           *rowEntry_state = ROWENTRY_STATE_ACTIVE;
           break;
       case RFC2579_STATE_NOT_READY:
           *rowEntry_state = ROWENTRY_STATE_NOT_READY;
           break;
       case RFC2579_STATE_NOT_IN_SERVICE:
           *rowEntry_state = ROWENTRY_STATE_NOT_IN_SERVICE;
           break;
       case RFC2579_STATE_NOT_EXISTENT:
           *rowEntry_state = ROWENTRY_STATE_NOT_EXISTENT;
           break;
       default:
           return FALSE;
   }
   return TRUE;
}

/**
  * \brief Convert VTSS row status to RFC2579 row status.
  *
  * \param rowEntry_state   [IN]:   The VTSS row status.\n
  * \param rfc_state        [OUT]:  The RFC2579 row state.\n

  * \return
  *    FALSE if the convertion fails.\n
  *     Otherwise, return TRUE.
  */

BOOL rowEntry_2_rfc2579(rowEntry_status_t rowEntry_state, rfc2579_status_t *rfc_state)
{
   switch(rowEntry_state) {
       case ROWENTRY_STATE_ACTIVE:
           *rfc_state = RFC2579_STATE_ACTIVE;
           break;
       case ROWENTRY_STATE_NOT_READY:
           *rfc_state = RFC2579_STATE_NOT_READY;
           break;
       case ROWENTRY_STATE_NOT_IN_SERVICE:
           *rfc_state = RFC2579_STATE_NOT_IN_SERVICE;
           break;
       case ROWENTRY_STATE_NOT_EXISTENT:
           *rfc_state = RFC2579_STATE_NOT_EXISTENT;
           break;
       default:
           return FALSE;
   }
   return TRUE;

}


