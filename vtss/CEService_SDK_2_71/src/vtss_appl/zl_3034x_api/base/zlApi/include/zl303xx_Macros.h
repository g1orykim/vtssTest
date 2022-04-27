/*******************************************************************************
*
*  $Id: zl303xx_Macros.h 7357 2011-12-19 20:21:30Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Various macros for use anywhere in the API.
*
*******************************************************************************/

#ifndef ZL303XX_MACROS_H_
#define ZL303XX_MACROS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/
#include <stddef.h>

/*****************   DEFINES   ************************************************/

/* Gets the number of entries in a statically declared array. */
#define ZL303XX_ARRAY_SIZE(arr)  (sizeof(arr) / sizeof(*(arr)))

/* Macro to return a pointer to a structure given a pointer to a member within
 * that structure. Example usage (with a linked list):
 *
 * struct
 * {
 *    Uint32T data1;
 *    Uint32T data2;
 *    zl303xx_ListS listEntry;
 * } zl303xx_SomeStructS;
 *
 * Uint32T doWork(zl303xx_ListS *node)
 * {
 *    zl303xx_SomeStructS *worker = ZL303XX_CONTAINER_GET(node, zl303xx_SomeStructS, listEntry);
 *
 *    return worker->data1 + worker->data2;
 * }
 * 
 * Wrapping this macro in another define can make code more readable:
 *    #define LIST_TO_WORKER(ptr)  ZL303XX_CONTAINER_GET(ptr, zl303xx_SomeStructS, listEntry)
 */
#define ZL303XX_CONTAINER_GET(ptr, type, member) \
   ((type *)(void *)((char *)ptr - offsetof(type, member)))

/* Bit manipulation macros */
#define ZL303XX_BIT_SET(var, bit)     ((var) |= (1 << (bit)))
#define ZL303XX_BIT_CLEAR(var, bit)   ((var) &= ~(1 << (bit)))
#define ZL303XX_BIT_TEST(var, bit)    ((var) & (1 << (bit)))

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */
