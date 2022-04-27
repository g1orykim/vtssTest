/*

 Vitesse Interrupt module software.

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

#include "critd_api.h"
#include "interrupt.h"
#include "interrupt_api.h"

#include <cyg/hal/vcoreii.h>
#include <cyg/hal/drv_api.h>

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>


/****************************************************************************/
/*  Global variables                                                                                                                       */
/****************************************************************************/

/* Structure for global variables */
static cyg_interrupt    pi_int_object_irq0;
static cyg_handle_t     pi_int_handle_irq0;

#define INTERRUPT_IRQ0     0x01

#if defined(VTSS_SW_OPTION_SYNCE) || defined(VTSS_SW_OPTION_POE)
static cyg_interrupt    pi_int_object_irq1;
static cyg_handle_t     pi_int_handle_irq1;

#define INTERRUPT_IRQ1     0x02
#endif



/****************************************************************************/
/*  Module Interface                                                        */
/****************************************************************************/
#if defined(VTSS_SW_OPTION_POE)
static BOOL interrupt_poe_active = FALSE;
#endif

void interrupt_device_enable(u32     flags,  BOOL pending)
{
    if (flags & INTERRUPT_IRQ0)
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INT_PI_0);
#if defined(VTSS_SW_OPTION_SYNCE) || defined(VTSS_SW_OPTION_POE)
    if (flags & INTERRUPT_IRQ1)
#if defined(VTSS_SW_OPTION_POE)
        if (pending || !interrupt_poe_active)   cyg_drv_interrupt_unmask(CYGNUM_HAL_INT_PI_1);      /* IRQ1 is only enabled if Pending or POE I2C interrupt is not expected - no Pending means POE interrupt and unmask is done through POE hookup */
#else
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INT_PI_1);
#endif
#endif
}



#if defined(VTSS_SW_OPTION_POE)
void interrupt_poe_poll(BOOL interrupt,   BOOL *pending)
{
    /* POE is special on LUTON 28 ENZO platform as there is no pending indication or interrupt enable/disable on this device */
    /* Only poll POE if interrupt and no pending on other devices on this physical interrupt */
    if (interrupt && !(*pending))   interrupt_signal_source(INTERRUPT_SOURCE_I2C, 0);
}

void interrupt_poe_source_enable(vtss_interrupt_source_t     source_id)
{
    interrupt_poe_active = TRUE; /* From now on we asume that POE is handling the I2C event and IRQ1 is only unmasked if pending is detected */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INT_PI_1);        /* In case of POE the GPIO unmask is done through POE hookup (now) */
}
#else
void interrupt_poe_poll(BOOL interrupt,   BOOL *pending)
{
    *pending = *pending;
}

void interrupt_poe_source_enable(vtss_interrupt_source_t  source_id)
{
    source_id = source_id;
}
#endif




void interrupt_source_enable(vtss_interrupt_source_t     source_id)
{
    u32 i, rc=VTSS_RC_OK;

    switch (source_id)
    {
        case INTERRUPT_SOURCE_LOS:   for (i=0; i<24; ++i)      rc = vtss_phy_event_enable_set(NULL, i, VTSS_PHY_LINK_LOS_EV, TRUE);     break;
        case INTERRUPT_SOURCE_FLNK:  for (i=0; i<24; ++i)      rc = vtss_phy_event_enable_set(NULL, i, VTSS_PHY_LINK_FFAIL_EV, TRUE);   break;
        case INTERRUPT_SOURCE_AMS:   for (i=0; i<24; ++i)      rc = vtss_phy_event_enable_set(NULL, i, VTSS_PHY_LINK_AMS_EV, TRUE);     break;

        case INTERRUPT_SOURCE_LOCS:
        case INTERRUPT_SOURCE_FOS:
        case INTERRUPT_SOURCE_LOSX:
        case INTERRUPT_SOURCE_LOL:        interrupt_clock_source_enable(source_id);     break;

        case INTERRUPT_SOURCE_I2C:        interrupt_poe_source_enable(source_id);       break;
        default: return;
    }
}


void interrupt_device_poll(u32 flags,  BOOL interrupt,  BOOL onesec, BOOL *pending)
{
    vtss_rc             rc=VTSS_RC_OK;
    u32                 i;
    vtss_phy_event_t    phy_events;

#if defined(VTSS_SW_OPTION_SYNCE)
    if (onesec) /* clock controller must be polled once a sec in order to detect state change to passive */
        interrupt_clock_poll(interrupt, pending);
#endif

    if (flags & INTERRUPT_IRQ0)
    {
        for (i=0; i<24; i++)
        {
            rc = vtss_phy_event_poll(NULL, i, &phy_events);
            rc = vtss_phy_event_enable_set(NULL, i, phy_events, FALSE);
            if (phy_events & VTSS_PHY_LINK_FFAIL_EV)   interrupt_signal_source(INTERRUPT_SOURCE_FLNK, i);
            if (phy_events & VTSS_PHY_LINK_LOS_EV)     interrupt_signal_source(INTERRUPT_SOURCE_LOS, i);
            if (phy_events & VTSS_PHY_LINK_AMS_EV)     interrupt_signal_source(INTERRUPT_SOURCE_AMS, i);
        }
    }
#if defined(VTSS_SW_OPTION_SYNCE) || defined(VTSS_SW_OPTION_POE)
    if (flags & INTERRUPT_IRQ1)
    {
#if defined(VTSS_SW_OPTION_SYNCE)
        interrupt_clock_poll(interrupt, pending);
#endif
#if defined(VTSS_SW_OPTION_POE)
        interrupt_poe_poll(interrupt, pending);    /* POE must be the last to poll as it need pending from CLOCK */
#endif
    }
#endif
}



/****************************************************************************/
/*  Various local definitions                                               */
/****************************************************************************/

static cyg_uint32 pi_irq0_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmarsk
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void pi_irq0_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // Context switches to other DSRs or threads cannot occur within this function.
    interrupt_device_signal(INTERRUPT_IRQ0);
}


#if defined(VTSS_SW_OPTION_SYNCE) || defined(VTSS_SW_OPTION_POE)
static cyg_uint32 pi_irq1_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmarsk
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void pi_irq1_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // Context switches to other DSRs or threads cannot occur within this function.
    interrupt_device_signal(INTERRUPT_IRQ1);
}
#endif /* VTSS_SW_OPTION_SYNCE || VTSS_SW_OPTION_POE */




#if defined(VTSS_SW_OPTION_POE)
void interrupt_poe_init_done()
{
    /* Enable EXT1 interrupt */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INT_PI_1);
}
#endif


/* Initialize module */
void interrupt_board_init(void)
{
    /* Hook the PI0 interrupt in eCos */
    cyg_drv_interrupt_create(CYGNUM_HAL_INT_PI_0,      /* Interrupt Vector */
                             0,                        /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             pi_irq0_isr,
                             pi_irq0_dsr,
                             &pi_int_handle_irq0,
                             &pi_int_object_irq0);

    cyg_drv_interrupt_attach(pi_int_handle_irq0);

    ulong  intr_ctrl;
    
    /* Configure PI0 IRQ in ARM to generate interrupt */
    intr_ctrl = VTSS_INTR_CTRL;
    intr_ctrl &= ~0x30000000;          /* Active low */
    intr_ctrl &= ~0x00000300;          /* Level trigged */
    VTSS_INTR_CTRL = intr_ctrl;

    /* Enable PI0 interrupts in ARM */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INT_PI_0);

#if defined(VTSS_SW_OPTION_SYNCE) || defined(VTSS_SW_OPTION_POE)
    /* Hook the PI1 interrupt in eCos */
    cyg_drv_interrupt_create(CYGNUM_HAL_INT_PI_1,      /* Interrupt Vector */
                             0,                        /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             pi_irq1_isr,
                             pi_irq1_dsr,
                             &pi_int_handle_irq1,
                             &pi_int_object_irq1);

    cyg_drv_interrupt_attach(pi_int_handle_irq1);

#if !defined(VTSS_SW_OPTION_POE)
    /* Enable PI1 interrupts in ARM */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INT_PI_1);
#endif
#endif /* VTSS_SW_OPTION_SYNCE || VTSS_SW_OPTION_POE */
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
