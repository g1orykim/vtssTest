/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "zl303xx_Api.h"
#include "zl303xx_LogToMsgQ.h"

#include "vtss_types.h"
#include "zl_3034x_api.h"
#include "zl_3034x_api_api.h"
#include "vtss_tod_api.h"
#include "vtss_ecos_mutex_api.h"
#include "critd_api.h"

#include <stdlib.h>
#include <sys/time.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_ZL_3034X_API

#define OK      0
#define NO_WAIT                         0
#define WAIT_FOREVER                    -1
#define ERROR   (-1)
#define CYG_FLAG_BASED_SEMA4        

/* TBD: Replace with appropriate locks */
#define TASK_LOCK()   {cyg_scheduler_lock();}
#define TASK_UNLOCK() {cyg_scheduler_unlock();}

#define MQ_LOCK()   {cyg_scheduler_lock();}
#define MQ_UNLOCK() {cyg_scheduler_unlock();}

#define ZL303XX_ABORT_FN exit(-1)
const osStatusT OsStatusOk = (osStatusT)OK;
const osSMiscT OsNoWait = (osSMiscT)NO_WAIT;
const osSMiscT OsWaitForever = (osSMiscT)WAIT_FOREVER;
const osStatusT OsStatusError = (osStatusT)ERROR;
const char      err_msg_parm[64] = "Error(%d) in file: %s at line:%d ";

/***********************************************************************************
 * Task information block:-                                                        *
 * t_stack:   Task stack                                                           *
 * t_handler: Task handler                                                         *
 * t_info:    Task information                                                     *
 ***********************************************************************************/
typedef struct task {
    char         *t_stack;
    cyg_handle_t t_handler;
    cyg_thread   t_info;
    struct task  *next;
}task_t;

static task_t *g_task_list;

#ifdef TASK_MGMT
static vtss_rc task_list_init(task_t *task_list)
{
    task_list = NULL;

    return VTSS_RC_OK;
}

static vtss_rc task_find(task_t *task_list, cyg_handle_t t_handler, task_t **task)
{
    task_t *tmp_task = task_list;

    while (tmp_task) {
        if (tmp_task->t_handler == t_handler) {
            *task = tmp_task;
            break;
        }
        tmp_task = tmp_task->next;
    }
    if (tmp_task == NULL) {
        *task = NULL;
        T_EG(TRACE_GRP_OS_PORT, "Specified Task not found in the Task list");
        return VTSS_RC_ERROR;
    } else {
        return VTSS_RC_OK;
    }
}
#endif /* end of TASK_MGMT. These functions are not used yet */

static vtss_rc task_insert(task_t **task_list,  task_t *task)
{
    if (!task) {
        return VTSS_RC_ERROR;
    }
    task->next = *task_list;
    *task_list = task;

    return VTSS_RC_OK;
}

static vtss_rc _task_delete(task_t **task_list,  cyg_handle_t t_handler)
{
    task_t *cur_task = *task_list;
    task_t *prev_task = NULL;
    vtss_rc  rc = VTSS_RC_ERROR;

    while(cur_task) {
        if (cur_task->t_handler == t_handler) {
            if (prev_task == NULL) { /* Delete the first task */
                *task_list = (*task_list)->next;
            } else {
                prev_task->next = cur_task->next;
            }
            VTSS_FREE(cur_task->t_stack);
            cur_task->t_stack = NULL;
            VTSS_FREE(cur_task);
            rc = VTSS_RC_OK;
            break;
        }
        prev_task = cur_task;
        cur_task = cur_task->next;
    }

    return rc;
}

static vtss_rc task_create(char             *name,
                           Sint32T          priority,
                           Sint32T          options,
                           Sint32T          stackSize,
                           OS_TASK_FUNC_PTR entryPt,
                           Sint32T          taskArg,
                           i32              *tcb_id,
                           OS_TASK_ID       *task_id)

{
    vtss_rc rc = VTSS_RC_OK;
    task_t  *new_task = NULL;

    do {
        new_task = (task_t *) VTSS_MALLOC(sizeof (task_t));
        if (new_task == NULL) {
            T_EG (TRACE_GRP_OS_PORT, "Unable to allocate memory for task: %s", name);
            rc = VTSS_RC_ERROR;
            break;
        } else {
            new_task->next = NULL;
            new_task->t_stack = (i8 *)VTSS_MALLOC(stackSize);
            if (new_task->t_stack == NULL) {
                T_EG(TRACE_GRP_OS_PORT, "Unable to allocate memory for task(%s) stack", name);
                VTSS_FREE(new_task);
                rc = VTSS_RC_ERROR;
                break;
            }
            cyg_thread_create(priority,
                    (cyg_thread_entry_t *)entryPt,
                    0,
                    name,
                    new_task->t_stack,
                    stackSize,
                    &new_task->t_handler,
                    &new_task->t_info);

            *task_id = (OS_TASK_ID) new_task->t_handler;
            TASK_LOCK();
            if (task_insert(&g_task_list, new_task) != VTSS_RC_OK) {
                rc = VTSS_RC_ERROR;
            }
            TASK_UNLOCK();
            if (rc == VTSS_RC_ERROR) {
                T_EG (TRACE_GRP_OS_PORT, "Unable to insert task(%s) in task list", name);
            }
            cyg_thread_resume(new_task->t_handler);
        }
    }while(0); /* end of do-while */


    return rc;
}

static vtss_rc task_delete(cyg_handle_t t_handler)
{
    vtss_rc rc = VTSS_RC_OK;

    if (cyg_thread_delete(t_handler) == TRUE) { 
        TASK_LOCK();
        if (_task_delete(&g_task_list, t_handler) != VTSS_RC_OK) {
            rc = VTSS_RC_ERROR;
        }
        TASK_UNLOCK();

        if (rc == VTSS_RC_ERROR) {
            T_EG (TRACE_GRP_OS_PORT, "Unable to delete the specified task from task list");
        }
    } else {
            T_EG (TRACE_GRP_OS_PORT, "Unable to delete the specified task from Kernel");
    }

    return rc;
}

/******************************************************************************
 * Trace function: Filters the trace messages based on debug flag             *
 *                                                                            *
 *****************************************************************************/

void zl303xx_TraceFnFiltered(Uint32T modId, Uint32T level, const char *str,
      UnativeT arg0, UnativeT arg1, UnativeT arg2, UnativeT arg3, UnativeT arg4, UnativeT arg5)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    i8  buf[MAX_FMT_LEN] = {0};
    i32 temp_len;

    temp_len = snprintf(buf, MAX_FMT_LEN, str, arg0, arg1, arg2, arg3, arg4, arg5);
    if ((temp_len < 0) || (temp_len > MAX_FMT_LEN)) {
        T_EG(TRACE_GRP_ZL_TRACE, "Buffer Size too Small to fit the supplied message");
        return;
    }
    if (modId == ZL303XX_MOD_ID_NOTIFY) { level = 0;}  // rise level for notifications
    switch(level) {
        case 0:
            T_IG(TRACE_GRP_ZL_TRACE, "modId %d,%s", modId, buf);
            break;
        case 1:
            T_DG(TRACE_GRP_ZL_TRACE, "modId %d,%s", modId, buf);
            break;
        case 2:
            T_NG(TRACE_GRP_ZL_TRACE, "modId %d,%s", modId, buf);
            break;
        default:
            T_RG(TRACE_GRP_ZL_TRACE, "modId %d,%s", modId, buf);
            break;
    }
#endif

}

/******************************************************************************
 * Trace function:                                                            * 
 *                                                                            *
 *****************************************************************************/
void zl303xx_TraceFnNoFilter(const char * str, UnativeT arg0, UnativeT arg1, UnativeT arg2, UnativeT arg3, UnativeT arg4, UnativeT arg5)
{
    i8 buf[MAX_FMT_LEN] = {0};
    i32 tmp_len;
    tmp_len = snprintf(buf, MAX_FMT_LEN, str, arg0, arg1, arg2, arg3, arg4, arg5);
    if (tmp_len < 0 || tmp_len > MAX_FMT_LEN) {
        T_EG(TRACE_GRP_ZL_TRACE, "Buffer Size too Small to fit the supplied message");
        return;
    }
    T_WG(TRACE_GRP_OS_PORT, buf);
}

void zl303xx_ErrorTrapFn(const Uint32T bStopOnError, const char *errCodeString,
                       const char *fileName, const char * const lineNum)
{
   if (errCodeString)
   {
      /* Build up the error message. We cannot assume that the error message will
         be displayed immediately so some rules apply:
            1. Any items identified by pointers must be constants
            2. Therefore we can't use any items (particularly) strings on the stack
         However, in our favour is the fact that all the parameters to this function
         are compiler generated constants so we can satisfy these rules if we are careful. */

      if (bStopOnError == ZL303XX_TRUE)
      {
         if (fileName)
         {
            if (lineNum)
            {
               ZL303XX_TRACE_ERROR("Fatal Error: \"%s\", in %s, line %s" ZL303XX_NEWLINE,
                     errCodeString,
                     fileName,
                     lineNum,
                     0, 0, 0);
               (void)OS_TASK_DELAY(1000);
            }
            else /* no linenumber given */
            {
               ZL303XX_TRACE_ERROR("Fatal Error: \"%s\", in %s" ZL303XX_NEWLINE,
                     errCodeString,
                     fileName,
                     0, 0, 0, 0);
               (void) OS_TASK_DELAY(1000);
            }
         }
         else
         {
            ZL303XX_TRACE_ERROR("Fatal Error: \"%s\"\n",
                  errCodeString, 0, 0, 0, 0, 0);
            (void) OS_TASK_DELAY(1000);
         }

         /* We should stop on error so abort the current thread */
         ZL303XX_ABORT_FN;

      }
      else  /* A "less than severe" error */
      {
         if (fileName)
         {
            if (lineNum)
            {
               ZL303XX_TRACE_ERROR("Error: \"%s\", in %s, line %s",
                     errCodeString, fileName, lineNum, 0, 0, 0);
            }
            else /* no linenumber given */
            {
               ZL303XX_TRACE_ERROR("Error: \"%s\", in %s",
                     errCodeString, fileName, 0, 0, 0, 0);
            }
         }
         else
         {
            ZL303XX_TRACE_ERROR("Error: \"%s\"\n",
                  errCodeString, 0, 0, 0, 0, 0);
         }
      }
   }
}

#if 0

ScaledNs32T ClockPeriod_ScaledNs(Uint32T freqHz)
{
/* Not finished */
    return(0);
}

/********************************************************************************
   Function to divide a Uint64S value by a Uint32T value to produce a Uint64S
      result->hi = the 32-bit whole portion of the result
      result->lo = the 32-bit remainder (modulus)
*********************************************************************************/
Uint64S Div_U64S_U32(Uint64S num, Uint32T den, Uint32T *mod)
{
    Uint64S result;
    u64 num64, result64;

    num64 = (num.hi*0x100000000LL) + num.lo;
    result64 = num64/den;
    result.hi = (result64/0x100000000LL) & 0xFFFFFFFF;
    result.lo = result64 & 0xFFFFFFFF;

    if (mod)    *mod = (num64%den) & 0xFFFFFFFF;

    return(result);
}

/********************************************************************************
   Function to shift a Uint64S value to the left by lshift number of bits.
*********************************************************************************/
Uint64S LShift_U64S(Uint64S inVal, Uint8T lshift)
{
    Uint64S result;
    u64 inVal64, result64;

    inVal64 = (inVal.hi*0x100000000LL) + inVal.lo;
    result64 = inVal64<<lshift;
    result.hi = (result64/0x100000000LL) & 0xFFFFFFFF;
    result.lo = result64 & 0xFFFFFFFF;

    return(result);
}

/********************************************************************************
   Function to add 2 Uint64S values.
*********************************************************************************/
Uint64S Add_U64S(Uint64S val1, Uint64S val2, Uint8T *carry)
{
    Uint64S result;
    u64 val164, val264, result64;

    val164 = (val1.hi*0x100000000LL) + val1.lo;
    val264 = (val2.hi*0x100000000LL) + val2.lo;
    result64 = val164 + val264;
    result.hi = (result64/0x100000000LL) & 0xFFFFFFFF;
    result.lo = result64 & 0xFFFFFFFF;

    if (carry)   *carry = ((result.hi < val1.hi) ? (1) : (0));

    return(result);
}

/********************************************************************************
   Function to find the difference between 2 Uint64S values.
*********************************************************************************/
Uint64S Diff_U64S(Uint64S val1, Uint64S val2, Uint8T *isNegative)
{
    Uint64S result;
    u64 val164, val264, result64;

    val164 = (val1.hi*0x100000000LL) + val1.lo;
    val264 = (val2.hi*0x100000000LL) + val2.lo;
    result64 = val164 - val264;
    result.hi = (result64/0x100000000LL) & 0xFFFFFFFF;
    result.lo = result64 & 0xFFFFFFFF;

    if (isNegative) *isNegative = ((result64 > val164) ? (1) : (0));

    return(result);
}

/********************************************************************************
   Function to shift a Uint64S value to the right by rshift number of bits.
*********************************************************************************/
Uint64S RShift_U64S(Uint64S inVal, Uint8T rshift)
{
    Uint64S result;
    u64 inVal64, result64;

    inVal64 = (inVal.hi*0x100000000LL) + inVal.lo;
    result64 = inVal64>>rshift;
    result.hi = (result64/0x100000000LL) & 0xFFFFFFFF;
    result.lo = result64 & 0xFFFFFFFF;

    return(result);
}

/********************************************************************************
   Function to multiply 2 Uint32T values to produce a Uint64S.
*********************************************************************************/
Uint64S Mult_U32_U32(Uint32T val1, Uint32T val2)
{
    Uint64S result;
    u64 result64;

    result64 = val1 * val2;
    result.hi = (result64/0x100000000LL) & 0xFFFFFFFF;
    result.lo = result64 & 0xFFFFFFFF;

    return(result);
}

/********************************************************************************
   Function used to implement the following ratio formula:


        n1     n2                n1 * d2
       ---- = ----   >>>   n2 = ---------
        d1     d2                  d1


   Proves useful when (n1 * d2) is larger than a Uint32T value.
   Often used to convert tick counts from one frequency to another.
*********************************************************************************/
Uint32T RatioConvert_U32(Uint32T n1, Uint32T d1, Uint32T d2)
{
    u64 result64;

    result64 = (n1 * d2)/d1;

    return(result64 & 0xFFFFFFFF);
}

/********************************************************************************
   Function used to implement the following formula:


                 n1 * n2
       result = ---------
                   d1


       round       Flag indicating if the decimal remainder as a result of the
                  division is >= 0.5 ( >= 0.5 then round = 1; otherwise = 0).
                  (If round == NULL, this value is ignored).
       overflow    For results that require more than 32-bits, this contains the
                  upper 32-bits of the result.
                  (If overflow == NULL, this value is ignored).

   Proves useful when the final result should be less than a 32-bit value.
   Otherwise, the overflow output can be used to return the 64-bit extension.
*********************************************************************************/
Uint32T Mult_Mult_Div_U32(Uint32T n1, Uint32T n2, Uint32T d1,
                          Uint8T *round, Uint32T *overflow)
{
    u64 mul64, result64, reminder64;

    mul64 = n1 * n2;
    result64 = mul64/d1;
    reminder64 = mul64%d1;

    if (round)      *round = (reminder64*2 >= d1) ? 1 : 0;
    if (overflow)   *overflow = (result64/0x100000000LL) & 0xFFFFFFFF;

    return(result64 & 0xFFFFFFFF);
}

/********************************************************************************
   Used to determine the ratio of 2 Uint32T numbers and express them as a 32-bit
   fraction. Any whole portion is also available:


        n1         n2                     n1:0
       ---- = -------------   >>>   n2 = -------
        d1     0x100000000                 d1


   Proves useful when finding the ratio of 2 frequencies to create a frequency
   conversion ratio.

  Return Value:
   Uint32T  ratio of num/denom as a 32-bit fraction
*********************************************************************************/
Uint32T Ratio_U32_U32(Uint32T n1, Uint32T d1, Uint32T *whole32)
{
    u64 n164, result64;

    n164 = (n1*0x100000000LL) + d1/2;  /* n1 is multiplied with 32 bit - d1/2 for eventual rounding */
    result64 = n164/d1;
    if (whole32)   *whole32 = (result64/0x100000000LL) & 0xFFFFFFFF;

    return(result64 & 0xFFFFFFFF);
}

#endif

#ifdef CRITD_BASED_SEMA4
static critd_t *create_critd(critd_type_t  type)
{
    critd_t*       mutex;
    struct timeval tv;
    static int     mutex_count = 0;
    char           buffer[40];
    
    T_DG(TRACE_GRP_OS_PORT, "os mapper called");
    mutex = (critd_t *)VTSS_MALLOC(sizeof(critd_t));
    if (mutex == NULL) {
        return (NULL);
    }
    if (gettimeofday(&tv, NULL)) {
        T_EG(TRACE_GRP_OS_PORT, "gettimeofday failed with errno: %d", errno);
        VTSS_FREE(mutex);
        mutex = NULL;
        return mutex;
    } else {
        sprintf(buffer, "%ld:%ld:%d", tv.tv_sec, tv.tv_usec, (mutex_count++)%1000);
        critd_init(mutex, buffer, VTSS_MODULE_ID_ZL_3034X, VTSS_TRACE_MODULE_ID, type);
    }

    T_DG(TRACE_GRP_OS_PORT, "os mapper(%s) finished", __FUNCTION__);
    return(mutex);
}
#endif

osStatusT osMutexGive(OS_MUTEX_ID mutex)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    VTSS_ECOS_MUTEX_UNLOCK((vtss_ecos_mutex_t*)mutex);
    return OS_OK;
}

osStatusT osMutexTake(OS_MUTEX_ID mutex)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    VTSS_ECOS_MUTEX_LOCK((vtss_ecos_mutex_t*)mutex);
    return OS_OK;
}

osStatusT osMutexTakeT(OS_MUTEX_ID mutex, Sint32T timeout)
{
/* Not finished - critd do not support this */
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    if (timeout) {
        T_IG(TRACE_GRP_OS_PORT, "timeout %d", timeout);
    }
    VTSS_ECOS_MUTEX_LOCK((vtss_ecos_mutex_t*)mutex);
    return OS_OK;
}

osStatusT osMutexDelete(OS_MUTEX_ID mutex)
{
/* Not finished - critd do not support this */
    T_WG(TRACE_GRP_OS_PORT, "os mapper called");
    return OS_OK;
}

OS_SEM_ID osMutexCreate(void)
{
    vtss_ecos_mutex_t *mutex;

    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    mutex = (vtss_ecos_mutex_t *)VTSS_MALLOC(sizeof(vtss_ecos_mutex_t));
    if (mutex == NULL) {
        return (OS_MUTEX_INVALID);
    }

    vtss_ecos_mutex_init(mutex);
    return (OS_SEM_ID)mutex;
}

#if 0
OS_SEM_ID osMutexCreate(void)
{
    critd_t*  mutex;
/* Not finished - critd do not support this */
/*
          - Multiple calls to take the mutex may be nested within the same
             calling task
*/

    mutex = create_critd(CRITD_TYPE_MUTEX);
    osMutexGive((OS_SEM_ID)mutex);
    return (OS_SEM_ID)mutex;
}
#endif

osStatusT osSema4Take(OS_SEM_ID semId, Sint32T timeout)
{
/* Not finished - critd do not support this (timeout) */
    T_IG(TRACE_GRP_OS_PORT, "os mapper(%s) called, semId %lx", __FUNCTION__, semId);
    if (timeout) {
        T_WG(TRACE_GRP_OS_PORT, "timeout %d", timeout);
    }
    
#if defined CYG_FLAG_BASED_SEMA4
    if (!cyg_flag_wait((cyg_flag_t *)semId, 0x1, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR))  {
        T_IG(TRACE_GRP_OS_PORT, "Thread got woken-up forcefully");
    }
#else
    (void)VTSS_OS_SEM_WAIT((cyg_sem_t *)semId);
#endif /* CRITD_BASED_SEMA4 */
    return OS_OK;
}

osStatusT osSema4Give(OS_SEM_ID semId)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper(%s) called, semId %lx", __FUNCTION__, semId);
#if defined CYG_FLAG_BASED_SEMA4
    cyg_flag_setbits((cyg_flag_t *)semId, 0x1);
#else
    VTSS_OS_SEM_POST((cyg_sem_t *)semId);
#endif /* CRITD_BASED_SEMA4 */
    return OS_OK;
}

osStatusT osSema4Delete(OS_SEM_ID semId)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper(%s) called, semId %lx", __FUNCTION__, semId);
#ifdef CYG_FLAG_BASED_SEMA4
    cyg_flag_destroy((cyg_flag_t *) semId);
    VTSS_FREE((cyg_flag_t *) semId);
#elif defined(CRITD_BASED_SEMA4)
/* Not finished - critd do not support this */
#else
    cyg_semaphore_destroy((cyg_sem_t *) semId);
    VTSS_FREE((cyg_sem_t *) semId);
#endif /* CYG_FLAG_BASED_SEMA4 */
    return OS_OK;
}

OS_SEM_ID osSema4CreateBinary(osMiscT initialState)
{
#ifdef CRITD_BASED_SEMA4
    critd_t*  semId;


    semId = create_critd(CRITD_TYPE_SEMAPHORE);
    T_IG(TRACE_GRP_OS_PORT, "critd created, semId %p, initialState %lu", semId, initialState);
    osSema4Give((OS_SEM_ID)semId);
    if (!initialState)  /* Initially a critd semaphore counter is set to '1', it is cleared by osSema4Take() */
        osSema4Take((OS_SEM_ID)semId, 0);

    T_IG(TRACE_GRP_OS_PORT, "semId %p", semId);
#elif defined CYG_FLAG_BASED_SEMA4 
    cyg_flag_t* semId;

    semId = (cyg_flag_t *)VTSS_MALLOC(sizeof(cyg_flag_t));
    T_IG(TRACE_GRP_OS_PORT, "critd created, semId %p, initialState %lu", semId, initialState);
    if (semId == NULL) {
        return OS_SEM_INVALID;
    }
    cyg_flag_init(semId);
    if (initialState == OS_SEM_FULL) {
        cyg_flag_setbits(semId, 0x1);
    } 
#else
    cyg_sem_t *semId;

    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    semId = (cyg_sem_t *)VTSS_MALLOC(sizeof(critd_t));
    if (semId == NULL) {
        T_EG(TRACE_GRP_OS_PORT, "Unable to create Semaphore");
        return OS_SEM_INVALID;
    }
    VTSS_OS_SEM_CREATE(semId, initialState);

#endif
    return (OS_SEM_ID)semId;
}


/********************************************************************************
 * osInterruptLock: Disables all interrupts                                     *
 *                                                                              *
 ********************************************************************************/
Uint32T osInterruptLock(void)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    cyg_interrupt_disable();
    return(0);
}

/********************************************************************************
 * osInterruptUnlock: Enables all interrupts                                    *
 *                                                                              *
 ********************************************************************************/
void osInterruptUnlock(Uint32T lockKey)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    cyg_interrupt_enable();
}

/*******************************************************************************
 * osSysTimestamp:                                                             *
 *                                                                             *
 *******************************************************************************/
Uint32T osSysTimestamp(void)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    return cyg_current_time();
}

/*******************************************************************************
 * osTickGet:                                                                  *
 *                                                                             *
 *******************************************************************************/
Uint32T osTickGet(void)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    return cyg_current_time();
}

osStatusT osTimeGet(Uint64S *pTime, Uint32T *pEpoch)
{
    vtss_timestamp_t ts;
    u32 tc;

    (void) pEpoch; /* reserved for future use */

    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    vtss_tod_gettimeofday(&ts, &tc);
    pTime->lo = ts.nanoseconds;
    pTime->hi = ts.seconds;

    return OS_OK;
}

zlStatusE zl303xx_SetHWTimer(Uint32T  rtSignal, timer_t *timerId, Sint32T osTimeDelay, void (*callout)(timer_t, int))
{
    T_WG(TRACE_GRP_OS_PORT, "os mapper called");
    return ZL303XX_OK;
}

zlStatusE zl303xx_DeleteHWTimer(timer_t *timerId)
{
    T_WG(TRACE_GRP_OS_PORT, "os mapper called");
    return ZL303XX_OK;
}


/*******************************************************************************
 * osMsgQCreate: Creates a new message queue                                   *
 *                                                                             *
 *******************************************************************************/
OS_MSG_Q_ID osMsgQCreate(Sint32T maxMsgs, Sint32T maxMsgLength)
{
    mqd_t          tmp_q;
    i8             q_name[NAME_MAX] = "/";
    struct mq_attr attr;
    mode_t         mode;
    struct timeval tv;
    static u16     tmp_cnt;

    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    attr.mq_maxmsg = maxMsgs;
    attr.mq_msgsize = maxMsgLength;
    mode = S_IRWXU|S_IRWXG|S_IRWXO;

    MQ_LOCK();
    if(gettimeofday(&tv, NULL)) {
        T_EG(TRACE_GRP_OS_PORT, "gettimeofday failed with errno: %d\n", errno);
        MQ_UNLOCK();
        return OS_MSG_Q_INVALID;
    } else {
        sprintf(q_name+1, "%ld:%ld:%u", tv.tv_sec, tv.tv_usec, (tmp_cnt++)%1000);
    }
    MQ_UNLOCK();

    tmp_q = mq_open(q_name, O_RDWR|O_CREAT,  mode, &attr);

    if (tmp_q == (mqd_t)-1) {
        sprintf(q_name, err_msg_parm, errno, __FILE__, __LINE__);
        T_EG (TRACE_GRP_OS_PORT, q_name);
        return OS_MSG_Q_INVALID;
    } else {
        return (OS_MSG_Q_ID) tmp_q;
    }
}

/*******************************************************************************
 * osMsgQDelete: Delete a message queue                                        *
 *                                                                             *
 *******************************************************************************/
osStatusT osMsgQDelete(OS_MSG_Q_ID msgQId)
{
    i8             err_msg[MAX_FMT_LEN];

    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    if (mq_close((mqd_t) msgQId)) {
        sprintf (err_msg, err_msg_parm, errno, __FILE__, __LINE__);
        T_EG(TRACE_GRP_OS_PORT, err_msg);
    }
    return OS_OK;
}

/********************************************************************************
 * osMsgQSend: Enqueues the messages                                            *
 *                                                                              * 
 ********************************************************************************/
osStatusT osMsgQSend(OS_MSG_Q_ID msgQId, Sint8T *buffer, Uint32T nBytes, Sint32T timeout)
{
    i32            ret;
    struct mq_attr attr;
    struct mq_attr old_attr;
    i8             err_msg[MAX_FMT_LEN];

    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    ret = mq_getattr( (mqd_t) msgQId, &attr);
    if (ret == -1) {
        T_WG(TRACE_GRP_OS_PORT, "Error: mq_getattr failed");
        return ((osStatusT) ret);
    }

    if (timeout == (Sint32T)OS_NO_WAIT) {
        attr.mq_flags |= O_NONBLOCK;
        ret = mq_setattr((mqd_t) msgQId, &attr, &old_attr);
        if (ret == -1) {
            T_WG(TRACE_GRP_OS_PORT, "Error: mq_setattr failed");
            return ((osStatusT) ret);
        }
    } else {
        attr.mq_flags &= ~O_NONBLOCK;
        ret = mq_setattr((mqd_t) msgQId, &attr, &old_attr);
        if (ret == -1) {
            T_WG(TRACE_GRP_OS_PORT, "Error: mq_setattr failed");
            return ((osStatusT) ret);
        }
    }

    if (timeout != (Sint32T)OS_WAIT_FOREVER && timeout != (Sint32T)OS_NO_WAIT) {
        ret = -1; /* TBD: Need to handle this correctly */
        T_EG(TRACE_GRP_OS_PORT, "Error: timeout %d", timeout);
    }
    ret = mq_send((mqd_t) msgQId, buffer, nBytes, 1);
    if (ret == -1) {
        sprintf (err_msg, err_msg_parm, errno, __FILE__, __LINE__);
        T_EG(TRACE_GRP_OS_PORT, err_msg);
    }

    return ((osStatusT) ret);
}

/*******************************************************************************
 * osMsgQReceive: Dequeues message                                             *
 *                                                                             *
 *******************************************************************************/
osStatusT osMsgQReceive(OS_MSG_Q_ID msgQId, Sint8T *buffer, Uint32T maxNBytes, Sint32T timeout)
{
    i32            ret = 0;
    struct mq_attr attr;
    struct mq_attr old_attr;
    i8             err_msg[MAX_FMT_LEN];

    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    ret = mq_getattr((mqd_t) msgQId, &attr);
    if (ret == -1) {
        sprintf (err_msg, err_msg_parm, errno, __FILE__, __LINE__);
        T_EG(TRACE_GRP_OS_PORT, err_msg);
        return ((osStatusT) ret);
    }

    if (timeout == (Sint32T)OS_NO_WAIT) {
        attr.mq_flags |= O_NONBLOCK;
        ret = mq_setattr((mqd_t) msgQId, &attr, &old_attr);
        if (ret == -1) {
            sprintf (err_msg, err_msg_parm, errno, __FILE__, __LINE__);
            T_EG(TRACE_GRP_OS_PORT, err_msg);
            return ((osStatusT) ret);
        }
    } else {
        attr.mq_flags &= ~O_NONBLOCK;
        ret = mq_setattr((mqd_t) msgQId, &attr, &old_attr);
        if (ret == -1) {
            sprintf (err_msg, err_msg_parm, errno, __FILE__, __LINE__);
            T_EG(TRACE_GRP_OS_PORT, err_msg);
            return ((osStatusT) ret);
        }
    }
    if (timeout != (Sint32T)OS_WAIT_FOREVER) {
        ret = -1; /* TBD: Need to handle this correctly */
        sprintf (err_msg, err_msg_parm, (int)ret, __FILE__, __LINE__);
        T_EG(TRACE_GRP_OS_PORT, err_msg);
    } else {
        ret = mq_receive ((mqd_t) msgQId, buffer, maxNBytes, NULL);
        if (ret == -1) {
            sprintf (err_msg, err_msg_parm, errno, __FILE__, __LINE__);
            return ((osStatusT) ret);
        }
    }

    return ((osStatusT) ret);
}

/*******************************************************************************
*  The function sends "trace" msgs to the msgQ for customer logging.
 *******************************************************************************/
Sint32T osLogMsg(const char *fmt, UnativeT arg1, UnativeT arg2, UnativeT arg3, UnativeT arg4, UnativeT arg5, UnativeT arg6)
{
    T_DG(TRACE_GRP_OS_PORT, "os mapper called");
    return zl303xx_LogToMsgQ(LOG_FMT_STR, fmt, (UnativeT)arg1, (UnativeT)arg2, (UnativeT)arg3, (UnativeT)arg4, (UnativeT)arg5, (UnativeT)arg6);
}
/*******************************************************************************
 * osTickRateGet: Rate get                                                     *
 *                                                                             *
 *******************************************************************************/
Uint32T osTickRateGet(void)
{
    T_DG(TRACE_GRP_OS_PORT, "os mapper, rate %d", ECOS_TICKS_PER_SEC);
    return (ECOS_TICKS_PER_SEC);
}
/*
 * Convert task priorities to eCos priorities, the priority numbers used in the call from ZL code are Linux priorities,
 *   i.e. 99 is highest, 0 is lowest
 */
static Sint32T task_priority_map(Sint32T priority)
{
    if (priority >= 99) return THREAD_HIGHEST_PRIO;
    if (priority >= 98) return THREAD_HIGH_PRIO;
    if (priority >= 97) return THREAD_ABOVE_NORMAL_PRIO;
    if (priority >= 88) return THREAD_DEFAULT_PRIO;
    return THREAD_BELOW_NORMAL_PRIO;
}

/*******************************************************************************
 * osTaskSpawn: Creates a new task                                             *
 *                                                                             *
 *******************************************************************************/
OS_TASK_ID osTaskSpawn(const char *name, Sint32T priority, Sint32T options, Sint32T stackSize, OS_TASK_FUNC_PTR entryPt, Sint32T taskArg)
{
    OS_TASK_ID task_id;
    i32        tmp_id;

    T_IG(TRACE_GRP_OS_PORT, "os mapper(%s) called, name %s, pri %d", __FUNCTION__, name, priority);
    priority = task_priority_map(priority);
    T_IG(TRACE_GRP_OS_PORT, "eCos pri %d", priority);
    if (task_create((char *)name,
                    priority,
                    options,
                    stackSize,
                    entryPt,
                    taskArg,
                    &tmp_id,
                    &task_id) != VTSS_RC_OK) {
        task_id = OS_TASK_INVALID;
    }
    T_IG(TRACE_GRP_OS_PORT, "created thread, taskId %lx", task_id);
    return task_id;
}

/*******************************************************************************
 * osTaskDelay:                                                                *
 *                                                                             *
 *******************************************************************************/
osStatusT osTaskDelay(Sint32T millis)
{
/*lint -e{571}  we need to cast the millis to (cyg_tick_count_t) */
    cyg_tick_count_t tick = (cyg_tick_count_t)millis;
    T_DG(TRACE_GRP_OS_PORT, "os mapper called %d", millis);
    cyg_thread_delay(((tick*ECOS_TICKS_PER_SEC)/1000)+1);
    return OS_OK;
}


/*******************************************************************************
 * osTaskDelete:                                                                *
 *                                                                             *
 *******************************************************************************/
osStatusT osTaskDelete (OS_TASK_ID tid)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    if (task_delete((cyg_handle_t)tid) != VTSS_RC_OK)
        return OS_ERROR;
    else
        return OS_OK;
}

void *osCalloc(size_t NumberOfElements, size_t size)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    return VTSS_CALLOC(NumberOfElements, size);
}

void osFree(void *ptrToMem)
{
    T_IG(TRACE_GRP_OS_PORT, "os mapper called");
    VTSS_FREE(ptrToMem);
}

cpuStatusE cpuSpiWrite(void *par, Uint32T regAddr, Uint8T *dataBuf, Uint16T bufLen)
{
//printf("cpuSpiWrite  regAddr %X  bufLen %u  data %X-%X\n", regAddr, bufLen, dataBuf[0], dataBuf[1]);
    zl_3034x_spi_write(regAddr, dataBuf, bufLen);

    return(CPU_OK);
}

cpuStatusE cpuSpiRead(void *par, Uint32T regAddr, Uint8T *dataBuf, Uint16T bufLen)
{
    zl_3034x_spi_read(regAddr, dataBuf, bufLen);
//printf("cpuSpiRead  regAddr %X  bufLen %u  data %X-%X\n", regAddr, bufLen, dataBuf[0], dataBuf[1]);

    return(CPU_OK);
}

int * __errno_location(void)
{
    T_EG(TRACE_GRP_OS_PORT, "!? is used");
    return (int *) cyg_thread_get_data_ptr(CYGNUM_KERNEL_THREADS_DATA_ERRNO);
}

int __libc_current_sigrtmax(void)
{
    T_EG(TRACE_GRP_OS_PORT, "!? is used");
    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

