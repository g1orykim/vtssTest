/*

 Vitesse API software.

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

/**
 * \file
 * \brief Linux OS API
 * \details This header file describes OS functions for Linux
 */

#ifndef _VTSS_OS_LINUX_H_
#define _VTSS_OS_LINUX_H_

#include <endian.h>
#include <asm/byteorder.h>

/**
 * \brief VTSS_OS_BIG_ENDIAN:
 *   If undefined, we're running little endian.
 *   If defined we're running big endian.
 */
#if __BYTE_ORDER == __BIG_ENDIAN
#define VTSS_OS_BIG_ENDIAN /**< We're running big endian */
#endif

#define VTSS_OS_NTOHL(x) __be32_to_cpu(x) /**< Convert a 32-bit value from network to host order */

#ifndef __KERNEL__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <string.h>             /* For memset and friend (not in stdlib) */

#include <errno.h>
#include <time.h>
#include <sys/time.h>

/** Sleep for \param nsec nanoseconds */
#define VTSS_NSLEEP(nsec) {                                     \
    struct timespec ts;                                         \
    ts.tv_sec = 0;                                              \
    ts.tv_nsec = nsec;                                          \
    while(nanosleep(&ts, &ts) == -1 && errno == EINTR) {        \
    }                                                           \
}

/** Sleep for \param msec milliseconds */
#define VTSS_MSLEEP(msec) {                                     \
    struct timespec ts;                                         \
    ts.tv_sec = msec / 1000;                                    \
    ts.tv_nsec = (msec % 1000) * 1000000;                       \
    while(nanosleep(&ts, &ts) == -1 && errno == EINTR) {        \
    }                                                           \
}

/** \brief Timer structure */
typedef struct {
    struct timeval timeout;   /**< Timeout */
    struct timeval now;       /**< Time right now */
} vtss_mtimer_t;

#define VTSS_MTIMER_START(timer,msec) { \
    (void) gettimeofday(&((timer)->timeout),NULL);   \
    (timer)->timeout.tv_usec+=msec*1000; \
    if ((timer)->timeout.tv_usec>=1000000) { (timer)->timeout.tv_sec+=(timer)->timeout.tv_usec/1000000; (timer)->timeout.tv_usec%=1000000; } \
} /**< Start timer */

#define VTSS_MTIMER_TIMEOUT(timer) (gettimeofday(&((timer)->now),NULL)==0 && timercmp(&((timer)->now),&((timer)->timeout),>)) /**< Timer timeout */

#define VTSS_MTIMER_CANCEL(timer) /**< No action in this implementation. */

/**< Time of day struct */
typedef struct {
    time_t sec; /**< Time of day in seconds */
} vtss_timeofday_t;

#define VTSS_TIME_OF_DAY(tod) { \
    struct timeval tve; \
    (void)gettimeofday(&tve,NULL);  \
    tod.sec = tve.tv_sec; \
} /**< Time of day macro */

// vtss_jaguar_1.c requires these defines. When compiling for the
// Genie board we're not in kernel mode and the application is 
// single-threaded, so we can live with not doing anything in these functions.
/**
 * VTSS_OS_SCHEDULER_FLAGS
 * VTSS_OS_SCHEDULER_LOCK(flags)
 * VTSS_OS_SCHEDULER_UNLOCK(flags)
 *   These functions are called by API code that consists of a user-level part
 *   and a deferred interrupt handler part.
 *   Only the user-level part will call the VTSS_OS_SCHEDULER_LOCK()/UNLOCK()
 *   functions, since it is assumed that the deferred interrupt handler will
 *   have atomic access throughout its execution.
 *   Each module within the API that contains such functionality will have
 *   an option to call either the VTSS_OS_SCHEDULER_(UN)LOCK() functions
 *   or the VTSS_OS_INTERRUPT_DISABLE()/RESTORE() functions.
 */
#define VTSS_OS_SCHEDULER_FLAGS         int
#define VTSS_OS_SCHEDULER_LOCK(flags)   do {flags = flags;} while (0); /**< Lock scheduler */
#define VTSS_OS_SCHEDULER_UNLOCK(flags) do {flags = flags;} while (0); /**< Unlock scheduler */

#if defined(__powerpc__) || defined(__powerpc64__) || defined(__x86_64) || defined(__amd64)
#define VTSS_DIV64(dividend, divisor) ((dividend) / (divisor))
#define VTSS_MOD64(dividend, divisor) ((dividend) % (divisor))
#define VTSS_LABS(arg)                labs(arg)
__extension__ extern long long int llabs (long long int __x) __THROW __attribute__ ((__const__));
#define VTSS_LLABS(arg)               llabs(arg)
#endif /* powerpc */

#else

/* __KERNEL__ */

#include <linux/stddef.h>
#include <linux/types.h>        /* ulong and friends */
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/slab.h>         /* for kmalloc/kfree */
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/math64.h>

#define VTSS_HAVE_U_TYPES       /* Namespace coexistance - use the ones from the kernel */

typedef struct {
    struct timeval timeout;
    struct timeval now;
} vtss_mtimer_t;

#define VTSS_NSLEEP(nsec) ndelay(nsec) /**< Sleep for "msec" milliseconds */
#define VTSS_MSLEEP(msec) mdelay(msec) /**< Sleep for "nsec" nanoseconds  */

#define VTSS_MTIMER_START(timer,msec) {                                 \
        (void) do_gettimeofday(&((timer)->timeout));                     \
        (timer)->timeout.tv_usec+=msec*1000;                            \
        if ((timer)->timeout.tv_usec>=1000000) {                        \
            (timer)->timeout.tv_sec+=(timer)->timeout.tv_usec/1000000;  \
            (timer)->timeout.tv_usec%=1000000;                          \
        }                                                               \
    }

static __always_inline int vtss_mtimer_timeout(vtss_mtimer_t *t)
{
    do_gettimeofday(&t->now);
    if(timeval_compare(&t->now, &t->timeout) > 0)
        return true;
    udelay(1);                  /* Be nice */
    return false;
}

#define VTSS_MTIMER_TIMEOUT(timer) vtss_mtimer_timeout(timer)

#define VTSS_MTIMER_CANCEL(timer) /* No action in this implementation. */

#define VTSS_DIV64 div64_u64

static inline u32 vtss_mod64(u64 div, u32 base)
{
    return do_div(div, base);   /* Remainder *returned* by do_div */
}

#define VTSS_MOD64 vtss_mod64

#define VTSS_LABS(arg)                __builtin_labs(arg)

static __always_inline long long
vtss_llabs(long long j)
{
    return (j < 0 ? -j : j);
}

#define VTSS_LLABS(arg)               vtss_llabs(arg)

#if VTSS_OPT_FDMA

#include <linux/dma-mapping.h>

/******************************************************************************/
// COMPILER SECTION
/******************************************************************************/
/*
 * VTSS_OS_REORDER_BARRIER():
 *   The compiler may swap instructions to optimize performance of the final
 *   code (size- or speed-wise). When configuration of hardware is involved,
 *   it may not always be valid to swap two statements. Consider for instance
 *   the following two writes to two FDMA registers:
 *      Write the source address to the FDMA
 *      Enable the FDMA.
 *   To the compiler, these two writes can be executed in any order and still
 *   semantically yield the correct result, had it been normal RAM they were
 *   written to. But since they are written to actual hardware, it is crucial
 *   that they are executed in the correct order.
 *   The VTSS_OS_REORDER_BARRIER() macro should implement code that ensures that the
 *   compiler doesn't optimize across the barrier.
 */
#define VTSS_OS_REORDER_BARRIER() barrier()

/*
 * VTSS_OS_COMPILER_ATTRIBUTE_ALIGNED(x):
 *   In some special cases, it is of utmost importance that a certain variable
 *   has a certain memory alignment. Applications for this is e.g. placing variables
 *   on cache-line boundaries.
 */
#define VTSS_OS_COMPILER_ATTRIBUTE_ALIGNED(x) __attribute ((aligned(x)))

/******************************************************************************/
// CACHE SECTION
/******************************************************************************/

/*
 * VTSS_OS_DCACHE_LINE_SIZE_BYTES:
 *   The number of bytes one DCache-line is made up of.
 */
#define VTSS_OS_DCACHE_LINE_SIZE_BYTES 32

/* VTSS_OS_DCACHE_INVALIDATE(virt_addr, size):
 *   Invalidate @size bytes at virtual address @virt_addr of the DCache.
 *   After invalidation, the invalidated area will be fetched from RAM.
 */
#define VTSS_OS_DCACHE_INVALIDATE(virt_addr, size) dma_cache_sync(NULL, (void *)(virt_addr), (size), DMA_FROM_DEVICE)

/* VTSS_OS_DCACHE_FLUSH(virt_addr, size):
 *   Force a write of @size bytes of dirty cache lines to RAM starting at
 *   virtual address @virt_addr.
 */
#define VTSS_OS_DCACHE_FLUSH(virt_addr, size)      dma_cache_sync(NULL, (void *)(virt_addr), (size), DMA_TO_DEVICE)

/* VTSS_OS_VIRT_TO_PHYS(addr):
 *   Macro that implements the conversion from a virtual to a physical address.
 *   In OSs with a flat memory layout, this could be as simple as (u32)(addr).
 */
#define VTSS_OS_VIRT_TO_PHYS(addr) virt_to_phys((void *)(addr))

#endif /* VTSS_OPT_FDMA */

/******************************************************************************/
// PLATFORM SECTION
/******************************************************************************/

#include <asm/io.h>

/******************************************************************************/
// TASK SWITCH/INTERRUPT SECTION
/******************************************************************************/

#include <linux/interrupt.h>

/*
 * VTSS_OS_SCHEDULER_FLAGS
 * VTSS_OS_SCHEDULER_LOCK(flags)
 * VTSS_OS_SCHEDULER_UNLOCK(flags)
 *   These functions are called by API code that consists of a user-level part
 *   and a deferred interrupt handler part.
 *   Only the user-level part will call the VTSS_OS_SCHEDULER_LOCK()/UNLOCK()
 *   functions, since it is assumed that the deferred interrupt handler will
 *   have atomic access throughout its execution.
 *   Each module within the API that contains such functionality will have
 *   an option to call either the VTSS_OS_SCHEDULER_(UN)LOCK() functions
 *   or the VTSS_OS_INTERRUPT_DISABLE()/RESTORE() functions.
 */
#define VTSS_OS_SCHEDULER_FLAGS         unsigned long
#define VTSS_OS_SCHEDULER_LOCK(flags)   local_irq_save(flags)
#define VTSS_OS_SCHEDULER_UNLOCK(flags) local_irq_restore(flags)

/*
 * VTSS_OS_INTERRUPT_FLAGS
 * VTSS_OS_INTERRUPT_DISABLE(flags)
 * VTSS_OS_INTERRUPT_RESTORE(flags)
 *   These functions are called by API code that consists of a user-level part
 *   and an interrupt handler part executing directly in interrupt context.
 *   Only the user-level part will call the VTSS_OS_INTERRUPT_DISABLE()/RESTORE()
 *   functions, since it is assumed that the interrupt handler will
 *   have atomic access throughout its execution.
 *   Each module within the API that contains such functionality will have
 *   an option to call either the VTSS_OS_SCHEDULER_(UN)LOCK() functions
 *   or the VTSS_OS_INTERRUPT_DISABLE()/RESTORE() functions.
 */
#define VTSS_OS_INTERRUPT_FLAGS          NOT_NEEDED
#define VTSS_OS_INTERRUPT_DISABLE(flags) NOT_NEEDED
#define VTSS_OS_INTERRUPT_RESTORE(flags) NOT_NEEDED

#endif /* __KERNEL__ */

// Whether __KERNEL__ or not.

/**
 * VTSS_OS_CTZ(val32)
 *
 * Count trailing zeros of a 32-bit unsigned.
 * Requirements/examples:
 *   VTSS_OS_CTZ(0x00000001) =  0
 *   VTSS_OS_CTZ(0x80000000) = 31
 *   VTSS_OS_CTZ(0x00000000) >= 32 (if result is taken as unsigned; Most implementations return -1, and (u32)(-1) >= 32).
 *
 * \param val32 The value to decode
 *
 * \return Number of trailing zeroes - or - the bit index of the
 * lowest bit set in the input given.
 * 
 * \note __builtin_ctz() is included in GCC 3.2.2 and later according
 * to http://en.wikipedia.org/wiki/Find_first_set.
 *
 * Note: __builtin_ctzl() is undefined for zero input values.
 */
#define VTSS_OS_CTZ(val32) ((val32) == 0 ? 32 : __builtin_ctzl((unsigned long)val32))

/**
 * Count trailing zeros of a 64-bit unsigned.
 * Requirements/examples:
 *   VTSS_OS_CTZ64(0x00000000_00000001) =  0
 *   VTSS_OS_CTZ64(0x00000000_80000000) = 31
 *   VTSS_OS_CTZ64(0x00000001_00000000) = 32
 *   VTSS_OS_CTZ64(0x80000000_00000000) = 63
 *   VTSS_OS_CTZ64(0x00000000_00000000) >= 64 (if result is taken as unsigned; Most implementations return -1, and (u32)(-1) >= 64).
 */
#define VTSS_OS_CTZ64(val64) ({                            \
    u32 _r = VTSS_OS_CTZ((u32)(val64));                    \
    (val64) == 0 ? 64 :                                    \
    _r < 32 ? _r : 32 + VTSS_OS_CTZ((u32)((val64) >> 32)); \
})

#if !defined(VTSS_HAS_CTZL)
static unsigned int __builtin_ctzl(unsigned long val32);
static inline unsigned int __builtin_ctzl(unsigned long val32)
{
#if defined(CONFIG_CPU_MIPSR2) || defined(CONFIG_CPU_MIPS64)
    unsigned int obits;
    __asm__("clz %0, %1" : "=r" (obits) : "r" (val32));
    return 31 - obits;
#else
    unsigned int i, mask = 1;
    for (i = 0; i < 32; i++) {
        if (val32 & mask) {
            return i;
        }
        mask <<= 1;
    }
    return i;
#endif
}
#endif

/**
 * Request OS to allocate size bytes of memory.
 *
 * The first argument is the number of bytes that must
 * be allocated. Type is size_t.
 *
 * The second argument is a mask of flags that the
 * implementation must obey. Type is vtss_mem_flags_t.
 *
 * The returned pointer should be at least 8-byte aligned,
 * to make it suitable for a struct.
 */
#ifdef __KERNEL__
#define VTSS_OS_MALLOC(size, flags) kmalloc(size, GFP_KERNEL)
#else
#define VTSS_OS_MALLOC(size, flags) malloc(size)
#endif

/**
 * Request OS to free memory previously allocated with
 * VTSS_OS_MALLOC().
 *
 * The first argument is the pointer previously
 * obtained with a call to VTSS_OS_MALLOC(). Type is
 * void *.
 *
 * The second argument is a mask of flags identical to
 * those passed to VTSS_OS_MALLOC() when the memory
 * was requested.
 */
#ifdef __KERNEL__
#define VTSS_OS_FREE(ptr, flags) kfree(ptr)
#else
#define VTSS_OS_FREE(ptr, flags) free(ptr)
#endif

#endif /* _VTSS_OS_LINUX_H_ */

