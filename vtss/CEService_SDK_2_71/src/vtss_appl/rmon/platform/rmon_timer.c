/*

 Vitesse Switch Software.

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

#include <main.h>
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#include "rmon_timer.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_RMON
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_RMON

#define RMON_TIMER_MALLOC_STRUCT(s)   (struct s *)VTSS_CALLOC(1, sizeof(struct s))

struct rmon_timer {
    unsigned int seconds;
    unsigned int flags;
    unsigned int clientreg;
    time_t lastcall;
    time_t nextcall;
    void *clientarg;
    RMONTimerCallback *thecallback;
    struct rmon_timer *next;
};

static struct rmon_timer *thealarms;
static unsigned int regnum = 1;

static void
rmon_timer_update_entry(struct rmon_timer *alrm)
{
    if (alrm->seconds == 0) {
        return; /* illegal */
    }
    if (alrm->lastcall == 0) {
        /* never been called yet, call seconds from now. */
        alrm->lastcall = time(NULL);
        alrm->nextcall = alrm->lastcall + alrm->seconds;
    } else if (alrm->nextcall == 0) {
        /* We've been called but not reset for the next? call */
        if ((alrm->flags & RMON_TIMER_REPEAT) == RMON_TIMER_REPEAT) {
            alrm->nextcall = alrm->lastcall + alrm->seconds;
        } else {
            /* single time call, remove it */
            rmon_timer_unregister(alrm->clientreg);
        }
    }
}



static struct rmon_timer *
rmon_timer_find_next(void)
{
    struct rmon_timer *sa_ptr, *sa_ret = NULL;
    for (sa_ptr = thealarms; sa_ptr != NULL; sa_ptr = sa_ptr->next) {
        if (sa_ret == NULL || (unsigned int)sa_ptr->nextcall < (unsigned int)sa_ret->nextcall) {
            sa_ret = sa_ptr;
        }
    }
    return sa_ret;
}

void
run_rmon_timer(void)
{
    int done = 0;
    struct rmon_timer *sa_ptr;

    /* loop through everything we have repeatedly looking for the next
       thing to call until all events are finally in the future again */
    while (done == 0) {
        sa_ptr = rmon_timer_find_next();
        if (sa_ptr == NULL) {
            return;
        }
        if ((unsigned int)sa_ptr->nextcall <= (unsigned int)time(NULL)) {
            T_D("run_rmon_timer");
            (*(sa_ptr->thecallback))(sa_ptr->clientreg, sa_ptr->clientarg);
            sa_ptr->lastcall = time(NULL);
            sa_ptr->nextcall = 0;
            rmon_timer_update_entry(sa_ptr);
        } else {
            done = 1;
        }
    }
//    T_D("run_rmon_timer done");
}

void
rmon_timer_unregister(unsigned int clientreg)
{
    struct rmon_timer *sa_ptr, *alrm = NULL;

    if (thealarms == NULL) {
        return;
    }

    if (clientreg == thealarms->clientreg) {
        alrm = thealarms;
        thealarms = alrm->next;
    } else {
        for (sa_ptr = thealarms;
             sa_ptr != NULL && sa_ptr->next != NULL && sa_ptr->next->clientreg != clientreg;
             sa_ptr = sa_ptr->next) {
            ;
        }
        if (sa_ptr) {
            if (sa_ptr->next) {
                alrm = sa_ptr->next;
                sa_ptr->next = sa_ptr->next->next;
            }
        }
    }

    /* Note:  do not free the clientarg, its the clients responsibility */
    if (alrm) {
        VTSS_FREE(alrm);
    }
}


unsigned int
rmon_timer_register(unsigned int when, unsigned int flags,
                    RMONTimerCallback *thecallback, void *clientarg)
{
    struct rmon_timer **sa_pptr;
    if (thealarms != NULL) {
        for (sa_pptr = &thealarms; (*sa_pptr) != NULL;
             sa_pptr = &((*sa_pptr)->next)) {
            ;
        }
    } else {
        sa_pptr = &thealarms;
    }

    *sa_pptr = RMON_TIMER_MALLOC_STRUCT(rmon_timer);
    if (*sa_pptr == NULL) {
        return 0;
    }

    (*sa_pptr)->seconds = when;
    (*sa_pptr)->flags = flags;
    (*sa_pptr)->clientarg = clientarg;
    (*sa_pptr)->thecallback = thecallback;
    (*sa_pptr)->clientreg = regnum++;
    rmon_timer_update_entry(*sa_pptr);

    return (*sa_pptr)->clientreg;
}
