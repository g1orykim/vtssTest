

/******************************************************************************
*
*  $Id: zl303xx_OsFunctions.h 8023 2012-04-04 13:08:58Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     The zl303xx API relies on various functions that need to be provided by
*     the 'host' operating system.  In order to make porting the API to a different
*     'host' easier to achieve an abstraction layer has been used to provide
*     the required functionality.
*
******************************************************************************/

#ifndef _ZL303XX_OS_FUNCTIONS_H_
#define _ZL303XX_OS_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

/*******************************************************************************

  Function Name:  osTaskSpawn

  Details:  Creates and then starts a new concurrent programming object (i.e. a task,
   a thread or a process)

  Parameters:
   [in]   name        name of new task
    [in]   priority    priority of new task
               This will be a return value from the function osTaskPriority()
    [in]  unused      unused parameter, exists for backward compatibility only
    [in]   stackSize   size in bytes of stack needed plus name
    [in]   entryPt     entry point of new task
    [in]   taskArg     argument to pass to entryPt func

  Return Value:
   ID of the created task or OS_ERROR if it could not be created

*******************************************************************************/
OS_TASK_ID osTaskSpawn(
      const char *name,
      Sint32T priority,
      Sint32T unused,
      Sint32T stackSize,
      OS_TASK_FUNC_PTR entryPt,
      Sint32T taskArg
   );

/*******************************************************************************

  Function Name:   OsTaskPriority

  Details:   Converts an absolute requested priority which will be in the range
   OS_TASK_MIN_PRIORITY to OS_TASK_MAX_PRIORITY into an OS specific
   priority.

  Return Value:   ID of the created timer or NULL if it could not be created

  Notes:   Some OS's use priority numbers in the reverse order and/or with a different
   range of values. This function converts between them.
   The relative priority of the tasks (low to high) must be maintained.

*******************************************************************************/

osStatusT osTaskPriority(Sint32T requestedPriority);

/*******************************************************************************

  Function Name:   OsNetTaskPriority

  Details:   Changes the NetTask priority to the requested priority.
   Usually to something lower than the lowest non-system task.

  Return Value:
   ID of the task or ERROR if it could not be changed

*******************************************************************************/

osStatusT osNetTaskPriority(OS_TASK_ID requestedPriority);

/*******************************************************************************

  Function Name:  osTaskDelete

  Details:  Deletes a previously created task or thread

  Parameters:
   [in]  tid         The task identifier to be destroyed

  Return Value:
   OS_OK, or OS_ERROR if the task cannot be deleted
*******************************************************************************/

osStatusT osTaskDelete (OS_TASK_ID tid);

/*******************************************************************************

  Function Name:   osTaskDelay

  Details:   Suspends the calling task until the specified number of ticks has elapsed

  Parameters:
   [in]   ticks       The number of system ticks to delay.

  Return Value:   OS_OK, or OS_ERROR if an error occurs or if the calling task receives
   a signal that is not blocked or ignored.

  Notes:   Should not be called from interrupt level

*******************************************************************************/

osStatusT  osTaskDelay(Sint32T ticks);

/*******************************************************************************

  Function Name:  osTaskLock

  Details:  Suspends the calling task until unlocked

  Return Value:   OS_OK, or OS_ERROR if an error occurs or if the calling task receives
   a signal that is not blocked or ignored.

  Notes:   Should not be called from interrupt level

*******************************************************************************/

osStatusT osTaskLock (void);

/*******************************************************************************

  Function Name:   osTaskUnLock

  Details:   Unlocked the task

  Return Value:   OS_OK, or OS_ERROR if an error occurs or if the calling task receives
   a signal that is not blocked or ignored.

  Notes:   Should not be called from interrupt level

*******************************************************************************/

osStatusT osTaskUnLock (void);

/*******************************************************************************

  Function Name:   osTickRateSet

  Details:  Sets the number of system clock cycles per second (i.e. the system "tick rate")

  Parameters:
   [in]   Clock ticks per second

  Return Value:   Success or failure
*******************************************************************************/

osStatusT osTickRateSet(Uint32T clkTicksPerSec);

/*******************************************************************************

  Function Name:   osTickRateGet

  Details:   Returns the number of system clock cycles per second (i.e. the system "tick rate")

  Return Value:   The number of clock cycles per second

*******************************************************************************/

Uint32T osTickRateGet(void);

/*******************************************************************************

  Function Name:   osTickGet

  Details:   Returns the current value of the tick counter. This value is set to zero at
   startup

  Return Value:
   The current value of the tick counter

*******************************************************************************/

Uint32T   osTickGet(void);


/*******************************************************************************
  Function Name:
   osTimeGet

  Details:
   Obtains the current time (in seconds since epoch 00:00:00 January 1, 1970 GMT)

  Parameters:
     pTime       Pointer to memory which will be filled with the current OS time
                    .hi = seconds
                    .lo = nanoseconds
     pEpoch      Pointer to memory which will be filled with the current epoch
                    (May be null if not requested by caller)

  Return Value: OS_OK     Success obtaining time information
        OS_ERROR  Failure obtaining time information

*******************************************************************************/
osStatusT osTimeGet(Uint64S *pTime, Uint32T *pEpoch);


osStatusT osInterruptConnect(OS_VOID_FUNC_PTR *vector, OS_VOID_FUNC_PTR routine, Sint32T parameter);
Uint32T osInterruptLock(void);
void osInterruptUnlock(Uint32T lockKey);
osStatusT osInterruptEnable(Uint32T level);
osStatusT osInterruptDisable(Uint32T level);

/*******************************************************************************

  Function Name:  osDpramMalloc

  Details:   Allocates space in DPRAM of size 'length' on a byte alignment of 'alignment'

  Parameters:
   [in]   length    - Size of DPRAM space to allocate.
    [in]  alignment - Byte alignment.

  Return Value:
   Pointer of space allocated or NULL.


*******************************************************************************/

void *osDpramMalloc(Sint32T length, Sint32T alignment);

/*******************************************************************************

  Function Name:   osDpramFree

  Details:  Frees memory previously allocated using osDpramMalloc

  Parameters:
   [in]   addr        Pointer of space allocated

*******************************************************************************/

void osDpramFree(void *addr);

/*******************************************************************************

  Function Name:   osSema4Create

  Details:   Creates a counting semaphore

  Parameters:
   [in]   initialCount   The initial count value

  Return Value:   ID of the semaphore, or OS_ERROR if the semaphore cannot be created

  Notes:   If multiple tasks are pending on a semaphore they should be unblocked in priority
   order

*******************************************************************************/

OS_SEM_ID osSema4Create(Sint32T initialCount);

/*******************************************************************************

  Function Name:  osSema4CreateBinary

  Details:   Creates a binary semaphore. A binary semaphore can contain the values 0 or 1 only

  Parameters:
   [in]   initialState   Either OS_SEM_EMPTY (0x0) or OS_SEM_FULL (0x1)

  Return Value:   ID of the semaphore, or OS_ERROR if the semaphore cannot be created

  Notes:   The following constraints shall apply on the usage of binary semaphores:
   Binary semaphores are used as a one-to-one signalling mechanism between threads.
   i.e. One thread will attempt to take the binary semaphore but will block until the
   other thread gives the semaphore.
   If the second thread gives the semaphore more than once before the first thread has
   taken it then the first thread will wake up once at most.
   One binary semaphore will be shared between two threads (a giver and a taker) only

*******************************************************************************/

OS_SEM_ID osSema4CreateBinary(osMiscT initialState);

/*******************************************************************************

  Function Name:  osSema4Give

  Details:   Unlocks the specified semaphore

  Parameters:
   [in]   semId       The identifier for the semaphore

  Return Value:   OS_OK, or OS_ERROR if the semaphore cannot be unlocked

*******************************************************************************/

osStatusT osSema4Give(OS_SEM_ID semId);

/*******************************************************************************

  Function Name:   osSema4Take

  Details:   Locks the specified semaphore

  Parameters:
   [in]   semId       The identifier for the semaphore
    [in]   timeout     The time to wait, in system ticks

  Return Value:  OS_OK, or OS_ERROR if the semaphore cannot be locked

*******************************************************************************/

osStatusT osSema4Take(OS_SEM_ID semId, Sint32T timeout);

/*******************************************************************************

  Function Name:  osSema4Delete

  Details:  Destroys a previously created semaphore

  Parameters:
   [in]   semId       The identifier for the semaphore

  Return Value:   OS_OK, or OS_ERROR if the semaphore cannot be destroyed
*******************************************************************************/

osStatusT osSema4Delete(OS_SEM_ID semId);

/*******************************************************************************

  Function Name:   osMutexCreate

  Details:   Creates a mutex

  Return Value:   ID of the mutex, or OS_ERROR if the mutex cannot be created

  Notes:   The following properties are required for mutexes:
      - Tasks pending on the mutex will be released in priority order
      - The mutex protects against priority inversion due to pre-emption
         of a task holding the mutex by elevating the holding task
         to the priority of the highest pending task.
      - Multiple calls to take the mutex may be nested within the same
         calling task

*******************************************************************************/

OS_SEM_ID osMutexCreate(void);

/*******************************************************************************

  Function Name:  osMutexGive

  Details:   Performs the "give" operation on the specified mutex.

  Parameters:
   [in]   mutex    Mutex identifier

  Return Value:   OS_OK, or OS_ERROR if the mutex cannot be unlocked

  Notes:   This operation is also known as "unlock" or "down" or "signal"

*******************************************************************************/

osStatusT osMutexGive(OS_MUTEX_ID mutex);

/*******************************************************************************

  Function Name:  osMutexTake

  Details:  Performs the "take" operation on the specified mutex.

  Parameters:
   [in]   mutex    Mutex identifier

  Return Value:   OS_OK, or OS_ERROR if the mutex cannot be locked

  Notes:   This operation is also known as "lock" or "up"

*******************************************************************************/

osStatusT osMutexTake(OS_MUTEX_ID mutex);

/*******************************************************************************

  Function Name:   osMutexTakeT

  Details:   Performs the "take" operation on the specified mutex with a timeout

  Parameters:
   [in]   mutex    Mutex identifier
    [in]    timeout  Timeout in ticks or one of the following special values:
               OS_NO_WAIT (0) return immediately, even if the semaphore could not be taken
                OS_WAIT_FOREVER (-1) never time out.

  Return Value:
   OS_OK, or OS_ERROR if the mutex cannot be locked or if the task timed out

*******************************************************************************/

osStatusT osMutexTakeT(OS_MUTEX_ID mutex, Sint32T timeout);

/*******************************************************************************

  Function Name:  osMutexDelete

  Details:  Deletes a mutex

  Parameters:
   [in]   mutex    Mutex identifier

  Return Value:   OS_OK, or OS_ERROR if the mutex cannot be created

*******************************************************************************/

osStatusT osMutexDelete(OS_MUTEX_ID mutex);

/*******************************************************************************

  Function Name:   osMsgQCreate

  Details:   Creates a message queue

  Parameters:
   [in]   maxMsgs        The maximum number of messages that can be queued
    [in]   maxMsgLength   The maximum size of each message

  Return Value:   The identifier for the message queue, or NULL if the queue cannot be created

  Notes:   The following constraints shall apply to message queue usage and implementation:
     Multiple tasks may send packets to a message queue
     Only one task may receive packets from a message queue

*******************************************************************************/

OS_MSG_Q_ID osMsgQCreate(Sint32T maxMsgs, Sint32T maxMsgLength);

/*******************************************************************************

  Function Name:   osMsgQSend

  Details:   Sends the message in 'buffer' of length 'nBytes' to the message queue msgQId.

  Parameters:
   [in]   msgQId         The message queue on which to send
    [in]   buffer         The message to send
    [in]   nBytes         The length of the message
    [in]   timeout        Number of ticks to wait to send the message if there is no space
                  or one of the following special values
                     OS_NO_WAIT (0) return immediately, even if the message could not be sent
                     WAIT_FOREVER (-1) never time out.

  Return Value:   OS_OK, or OS_ERROR if the message could not be sent to the queue

  Notes:   The following constraints shall apply to message queue usage and implementation:
     Messages are always queued in FIFO order
     This routine can be called by interrupt service routines as well as by tasks. But
     when called from an interrupt service routine, timeout must be NO_WAIT.

*******************************************************************************/

osStatusT osMsgQSend(OS_MSG_Q_ID msgQId, Sint8T *buffer, Uint32T nBytes, Sint32T timeout);

/*******************************************************************************

  Function Name:   osMsgQReceive

  Details:   Receives a message from the specified message queue if available.

  Parameters:
   [in]   msgQId         The message queue on which to receive
    [in]    buffer         The buffer to receive the message
    [in]    maxNBytes      The length of the buffer
    [in]    timeout        Number of ticks to wait to receive a message if there is none pending
                  or one of the following special values
                     OS_NO_WAIT (0) return immediately, even if there was no message to receive
                     WAIT_FOREVER (-1) never time out.

  Return Value:   The number of bytes copied to the buffer or OS_ERROR if no message could be received

  Notes:   The following constraints shall apply to message queue usage and implementation:
     The received message is copied into the specified buffer, which is maxNBytes in
     length. If the message is longer than maxNBytes, the remainder of the message is discarded
      (no error indication is returned).
      This routine will not be called by interrupt service routines.

*******************************************************************************/

osStatusT osMsgQReceive(OS_MSG_Q_ID msgQId, Sint8T *buffer, Uint32T maxNBytes, Sint32T timeout);

/*******************************************************************************

  Function Name:   osMsgQDelete

  Details:   Destroys a previously created message queue

  Parameters:
   [in]   msgQId       The identifier for the semaphore

  Return Value:   OS_OK, or OS_ERROR if the message queue cannot be destroyed

*******************************************************************************/

osStatusT osMsgQDelete(OS_MSG_Q_ID msgQId);

/*******************************************************************************

  Function Name:   osMsgQNumMsgs

  Details:   Returns the number of messages currently in the queue.

  Parameters:
   [in]   msgQId       The identifier for the semaphore

  Return Value:   Sint32T  Number of messages in the queue

*******************************************************************************/

Sint32T osMsgQNumMsgs(OS_MSG_Q_ID msgQId);

/*******************************************************************************

  Function Name:   osLogInit

  Details:   Initialises logging to the specified file

  Parameters:
   [in]   fd       An already open file descriptor to use for logging
            Can also be set to one of 'stdout' or 'stderr' to use a predefined
            i/o stream

*******************************************************************************/

void osLogInit(FILE *fd);

/*******************************************************************************

  Function Name:  osLogClose

  Details:   Terminates logging to the specified file

  Parameters:
   [in]   fd       An already open file descriptor to use for logging

*******************************************************************************/

void osLogClose(FILE *fd);

/*******************************************************************************

  Function Name:   osLogMsg

  Details:   Sends a message to the logging output stream

  Parameters:
   [in]   fmt      Printf style format string
    [in]   arg1     first of six required args for fmt
    [in]   arg2
    [in]   arg3
    [in]   arg4
    [in]   arg5
    [in]   arg6

  Return Value:   The number of bytes written to the log queue, or 0 if the routine is unable to write
   a message.

  Notes:   arg1-arg6 are required.
   Logging should ideally be performed by a separate task in order to allow it to be
   called from interrupt context.

   Depending on the implementation, the interpretation of the arguments may be deferred
   to the logging task instead of at the moment when osLogMsg( ) is called. Therefore
   the arguments to logMsg( ) should not be pointers to volatile entities
   (e.g., dynamic strings on the caller stack).

*******************************************************************************/

Sint32T osLogMsg(const char *fmt, UnativeT arg1, UnativeT arg2, UnativeT arg3, UnativeT arg4, UnativeT arg5, UnativeT arg6);

/*******************************************************************************

  Function Name:   osSysTimestampEnable

  Details:   Enable the h/w timer counter used to timestamp events.

  Return Value:   OS_OK, or OS_ERROR on failure

*******************************************************************************/

osStatusT osSysTimestampEnable(void);

/*******************************************************************************

  Function Name:   osSysTimestampDisable

  Details:   Disable the h/w timer counter used to timestamp events.

  Return Value:   OS_OK, or OS_ERROR on failure

*******************************************************************************/

osStatusT osSysTimestampDisable(void);

/*******************************************************************************

  Function Name:   osSysTimestampFreq

  Details:   Returns the frequency of the timestamp counter

  Return Value:   frequency of the counter


*******************************************************************************/

Uint32T osSysTimestampFreq(void);

/*******************************************************************************

  Function Name:  osSysTimestamp

  Details:   Returns the current timestamp counter value

  Return Value:   current timestamp counter value


*******************************************************************************/

Uint32T osSysTimestamp(void);

/*******************************************************************************

  Function Name:  osImmrGet

  Details:   This routine returns the current IMMR value.


  Return Value:   Internal registers base address

  Notes:   This is specific to the PowerPC processor, but may be used to service other
   architectures.

*******************************************************************************/

Uint32T osImmrGet(void);

#ifdef OS_LINUX
/*******************************************************************************

  Function Name:   osCalloc

  Details:   Returns a pointer to newly allocated memory (initialized to 0s)

  Parameters:
   [in]   NumberOfElements o and  SizeOfElement(in bytes)

  Return Value:   Void pointer or NULL

*******************************************************************************/

void *osCalloc(size_t NumberOfElements, size_t SizeOfElement);

/*******************************************************************************

  Function Name:  osFree

  Details:  Free previously allocated memory

  Parameters:
   [in]   Pointer to memory

*******************************************************************************/

void osFree(void *ptrToMem);


#endif
#ifdef __cplusplus
}
#endif

#endif

