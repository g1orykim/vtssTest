/*

 Vitesse API software.

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

#ifndef _VTSS_SIMPLE_FIFO_H_
#define _VTSS_SIMPLE_FIFO_H_

#define FIFO_DECL_STATIC(F, T, S)   \
    static T F ## _data[S];         \
    static int F ## _rd = 0;        \
    static int F ## _wr = 0;        \
    static const int F ## _size = S

#define FIFO_NEXT_INDEX(F, i) \
    ((i + 1) % F ## _size)

#define FIFO_EMPTY(F) \
    (F ## _wr == F ## _rd)

#define FIFO_CLEAR(F) \
    F ## _wr = 0, F ## _rd = 0

#define FIFO_FULL(F) \
    (FIFO_NEXT_INDEX(F, F ## _wr) == F ## _rd)

#define FIFO_PUT(F, X) \
    F ## _data[F ## _wr] = (X); \
    F ## _wr = FIFO_NEXT_INDEX(F, (F ## _wr))

#define FIFO_DEL(F) \
    F ## _rd = FIFO_NEXT_INDEX(F, (F ## _rd))

#define FIFO_HEAD(F) \
    F ## _data[F ## _rd]

#endif /* _VTSS_SIMPLE_FIFO_H_ */
