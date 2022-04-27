/*

 Vitesse Interrupt module software.

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

#include "interrupt.h"
#include "port_api.h"

#ifdef VTSS_SW_OPTION_SYNCE
#include "synce_custom_clock_api.h"
#endif


/****************************************************************************/
/*  Global variables                                                                                                                       */
/****************************************************************************/


/****************************************************************************/
/*  Module Interface                                                        */
/****************************************************************************/

#ifdef VTSS_SW_OPTION_SYNCE
#define CLOCK_INTERRUPT_MASK_REG      23
#define CLOCK_INTERRUPT_PENDING_REG  131
#define CLOCK_INTERRUPT_STATUS_REG   129

#define CLOCK_LOS1_MASK              0x02
#define CLOCK_LOS2_MASK              0x04
#define CLOCK_FOS1_MASK              0x02
#define CLOCK_FOS2_MASK              0x04
#define CLOCK_LOSX_MASK              0x01
#define CLOCK_LOL_MASK               0x01

void interrupt_clock_poll(BOOL interrupt,  BOOL *pending)
{
    vtss_rc    rc;
    uint       status1, mask1, status2, mask2;
    uint       pending1, pending2;
    
    pending1 = pending2 = 0;
    
    /* Read interrupt pending and mask register 1 in Clock controller */
    rc = clock_read(CLOCK_INTERRUPT_PENDING_REG, &pending1);
    rc = clock_read(CLOCK_INTERRUPT_PENDING_REG+1, &pending2);
    rc = clock_read(CLOCK_INTERRUPT_MASK_REG, &mask1);
    rc = clock_read(CLOCK_INTERRUPT_MASK_REG+1, &mask2);
    pending2 = pending2>>1; /* for some obscure reason pending2 is rotated to the left in register */

    if (!interrupt)
    {
    /* time out */
        /* During timeout we are polling for falling edge of a source as it is not able to generate interrupt */
        rc = clock_read(CLOCK_INTERRUPT_STATUS_REG, &status1);
        rc = clock_read(CLOCK_INTERRUPT_STATUS_REG+1, &status2);
        status2 &= 0x07;  /* This is to clear bit's used for other things */
        /* This is a bit tricky. Pending is actually a latched version of status. So if pending and status not the same it's a falling edge of status */
        /* The XOR of pending and status is treated as active pending and all hooked to the source is signalled */
        pending1 ^= status1;
        pending2 ^= status2;
        /* clear pending in Clock controller */
        rc = clock_writemasked(CLOCK_INTERRUPT_PENDING_REG, 0, pending1);
        rc = clock_writemasked(CLOCK_INTERRUPT_PENDING_REG+1, 0, pending2<<1);
    }
    else
    {
    /* interrupt */
        /* only handle interrupt active on sources still enabled */
        pending1 &= ~mask1; /* remember that interrupt is enabled when bit is '0' */
        pending2 &= ~mask2; /* remember that interrupt is enabled when bit is '0' */
    }
    
    if (pending1)
    {
        /* mask has to be cleared on pending sources */
        clock_writemasked(CLOCK_INTERRUPT_MASK_REG, pending1, pending1);
        if (pending1 & CLOCK_LOSX_MASK)
        /* Change in LOSX */
            interrupt_signal_source(INTERRUPT_SOURCE_LOSX, 0);
        if (pending1 & CLOCK_LOS1_MASK)
        /* Change in LOS1 */
            interrupt_signal_source(INTERRUPT_SOURCE_LOCS, 0);
        if (pending1 & CLOCK_LOS2_MASK)
        /* Change in LOS2 */
            interrupt_signal_source(INTERRUPT_SOURCE_LOCS, 1);

        /* Indicate that pending was detected */
        *pending = TRUE;
    }
    if (pending2)
    {
        /* mask has to be cleared on pending sources */
        clock_writemasked(CLOCK_INTERRUPT_MASK_REG+1, pending2, pending2);
        if (pending2 & CLOCK_LOL_MASK)
        /* Change in LOL */
            interrupt_signal_source(INTERRUPT_SOURCE_LOL, 0);
        if (pending2 & CLOCK_FOS1_MASK)
        /* Change in FOS1 */
            interrupt_signal_source(INTERRUPT_SOURCE_FOS, 0);
        if (pending2 & CLOCK_FOS2_MASK)
        /* Change in FOS2 */
            interrupt_signal_source(INTERRUPT_SOURCE_FOS, 1);

        /* Indicate that pending was detected */
        *pending = TRUE;
    }
}

void interrupt_clock_source_enable(vtss_interrupt_source_t   source_id)
{
    uint    mask, pending, i;
    vtss_rc rc;

    switch (source_id)
    {
        case INTERRUPT_SOURCE_LOCS:
            for (i=0; i<clock_my_input_max; ++i)
            {
                mask = (i == 0) ? CLOCK_LOS1_MASK : CLOCK_LOS2_MASK;
                rc = clock_read(CLOCK_INTERRUPT_PENDING_REG, &pending);
                rc = clock_writemasked(CLOCK_INTERRUPT_MASK_REG, 0, mask&~pending);
            }
            break;
        case INTERRUPT_SOURCE_FOS:
            for (i=0; i<clock_my_input_max; ++i)
            {
                mask = (i == 0) ? CLOCK_FOS1_MASK : CLOCK_FOS2_MASK;
                rc = clock_read(CLOCK_INTERRUPT_PENDING_REG+1, &pending);
                pending = pending>>1;
                rc = clock_writemasked(CLOCK_INTERRUPT_MASK_REG+1, 0, mask&~pending);
            }
            break;
        case INTERRUPT_SOURCE_LOSX:
            mask = CLOCK_LOSX_MASK;
            rc = clock_read(CLOCK_INTERRUPT_PENDING_REG, &pending);
            rc = clock_writemasked(CLOCK_INTERRUPT_MASK_REG, 0, mask&~pending);
            break;
        case INTERRUPT_SOURCE_LOL:
            mask = CLOCK_LOL_MASK;
            rc = clock_read(CLOCK_INTERRUPT_PENDING_REG+1, &pending);
            pending = pending>>1;
            rc = clock_writemasked(CLOCK_INTERRUPT_MASK_REG+1, 0, mask&~pending);
            break;
        default: return;
    }
}
#else
void interrupt_clock_poll(BOOL interrupt,   BOOL *pending)
{
    *pending = *pending;
}

void interrupt_clock_source_enable(vtss_interrupt_source_t  source_id)
{
    source_id = source_id;
}
#endif




/****************************************************************************/
/*                                                                                                                                                    */
/*  End of file.                                                                                                                              */
/*                                                                                                                                                    */
/****************************************************************************/
