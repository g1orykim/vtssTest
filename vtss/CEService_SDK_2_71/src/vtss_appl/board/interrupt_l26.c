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

#include "critd_api.h"
#include "interrupt.h"
#include "interrupt_api.h"
#include "port_custom_api.h"
#include <cyg/hal/hal_io.h>
#include "vtss_misc_api.h"
#if defined(VTSS_SW_OPTION_SYNCE)
#include "synce_custom_clock_api.h"
#endif
#include "main.h"
#ifdef VTSS_SW_OPTION_PHY
#include "phy_api.h"
#endif
#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
#include "zl_3034x_api_pdv_api.h"
#endif

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>


/****************************************************************************/
/*  Global variables                                                                                                                       */
/****************************************************************************/

/* Structure for global variables */
static cyg_interrupt    ext0_int_object;
static cyg_handle_t     ext0_int_handle;

static cyg_interrupt    sgpio_int_object;
static cyg_handle_t     sgpio_int_handle;

static cyg_interrupt    dev_all_int_object;
static cyg_handle_t     dev_all_int_handle;

static cyg_interrupt    ptp_int_object;
static cyg_handle_t     ptp_int_handle;

static int board_type;
static vtss_chip_id_t  chip_id;

#define INTERRUPT_EXT0     0x01
#define INTERRUPT_SGPIO    0x02
#define INTERRUPT_DEV_ALL  0x04
#define INTERRUPT_PTP      0x08



/****************************************************************************/
/*  Module Interface                                                        */
/****************************************************************************/
#if defined(VTSS_SW_OPTION_POE)
static BOOL interrupt_poe_active = FALSE;
#endif

void interrupt_device_enable(u32     flags,  BOOL pending)
{
    if (flags & INTERRUPT_EXT0)
#if defined(VTSS_SW_OPTION_POE)
        if (pending || !interrupt_poe_active)   cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);      /* EXT0 is only enabled if Pending or POE I2C interrupt is not expected - no Pending means POE interrupt and unmask is done through POE hookup */
#else
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
#endif
    if (flags & INTERRUPT_SGPIO)
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SGPIO);
    if (flags & INTERRUPT_DEV_ALL)
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_DEV_ALL);
    if (flags & INTERRUPT_PTP)
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_PTP_SYNC);
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
    interrupt_poe_active = TRUE; /* From now on we asume that POE is handling the I2C event and EXT0 is only unmasked if pending is detected */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);        /* In case of POE the EXT0 unmask is done through POE hookup (now) */
}
#endif


void interrupt_source_enable(vtss_interrupt_source_t     source_id)
{
    u32 i, rc=VTSS_RC_OK, cu_port=0;

    if (board_type == VTSS_BOARD_LUTON10_REF)       cu_port = 8;
    if (board_type == VTSS_BOARD_LUTON26_REF)       cu_port = 24;

    switch (source_id)
    {
        case INTERRUPT_SOURCE_LOS:
            for (i=0; i<cu_port; ++i)      rc = vtss_phy_event_enable_set(PHY_INST, i, VTSS_PHY_LINK_LOS_EV, TRUE);
            rc = vtss_sgpio_event_enable(NULL, 0, 0, 24, 0, TRUE);
            rc = vtss_sgpio_event_enable(NULL, 0, 0, 25, 0, TRUE);
            break;

        case INTERRUPT_SOURCE_FLNK:
        case INTERRUPT_SOURCE_AMS:
            for (i=0; i<cu_port; ++i)      rc = vtss_phy_event_enable_set(PHY_INST, i, (source_id == INTERRUPT_SOURCE_FLNK) ? VTSS_PHY_LINK_FFAIL_EV : VTSS_PHY_LINK_AMS_EV, TRUE);
            break;

        case INTERRUPT_SOURCE_SYNC:       rc = vtss_ptp_event_enable(NULL, VTSS_PTP_SYNC_EV, TRUE);      break;
        case INTERRUPT_SOURCE_EXT_SYNC:   rc = vtss_ptp_event_enable(NULL, VTSS_PTP_EXT_SYNC_EV, TRUE);  break;
        case INTERRUPT_SOURCE_CLK_ADJ:    rc = vtss_ptp_event_enable(NULL, VTSS_PTP_CLK_ADJ_EV, TRUE);   break;

#if defined(VTSS_SW_OPTION_SYNCE)
        case INTERRUPT_SOURCE_LOCS:       clock_event_enable(CLOCK_LOCS1_EV); clock_event_enable(CLOCK_LOCS2_EV);     break;
        case INTERRUPT_SOURCE_FOS:        clock_event_enable(CLOCK_FOS1_EV); clock_event_enable(CLOCK_FOS2_EV);     break;
        case INTERRUPT_SOURCE_LOSX:       clock_event_enable(CLOCK_LOSX_EV);    break;
        case INTERRUPT_SOURCE_LOL:        clock_event_enable(CLOCK_LOL_EV);     break;
#endif
#if defined(VTSS_SW_OPTION_POE)
        case INTERRUPT_SOURCE_I2C:        interrupt_poe_source_enable(source_id);       break;
#endif

#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
        zl_3034x_event_t zl_events;
        case INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG:
            zl_events = ZL_TOP_ISR_REF_TS_ENG;
            (void) zl_3034x_event_enable_set(zl_events, TRUE);
            break;
#endif
        default: return;
    }
}


#if defined(VTSS_SW_OPTION_SYNCE)
static void clock_event_signal(clock_event_type_t events, BOOL *pending)
{
    if (events & CLOCK_LOSX_EV)   interrupt_signal_source(INTERRUPT_SOURCE_LOSX, 0);
    if (events & CLOCK_LOL_EV)    interrupt_signal_source(INTERRUPT_SOURCE_LOL, 0);
    if (events & CLOCK_LOCS1_EV)  interrupt_signal_source(INTERRUPT_SOURCE_LOCS, 0);
    if (events & CLOCK_LOCS2_EV)  interrupt_signal_source(INTERRUPT_SOURCE_LOCS, 1);
    if (events & (CLOCK_LOCS1_EV | CLOCK_LOCS2_EV))  interrupt_signal_source(INTERRUPT_SOURCE_LOCS, 2);
    if (events & CLOCK_FOS1_EV)   interrupt_signal_source(INTERRUPT_SOURCE_FOS, 0);
    if (events & CLOCK_FOS2_EV)   interrupt_signal_source(INTERRUPT_SOURCE_FOS, 1);
    if (events & (CLOCK_LOSX_EV | CLOCK_LOL_EV | CLOCK_LOCS1_EV | CLOCK_LOCS2_EV | CLOCK_FOS1_EV | CLOCK_FOS2_EV))    *pending = TRUE;
}
#endif

void interrupt_device_poll(u32 flags,  BOOL interrupt,  BOOL onesec, BOOL *pending)
{
    vtss_rc rc=VTSS_RC_OK;
    u32     i;
    vtss_ptp_event_type_t  ptp_events;
    vtss_phy_event_t       phy_events;
    BOOL                   sgpio_events[VTSS_SGPIO_PORTS];


    if (!onesec) {
// Debug        printf("interrupt:%d, pending:%d, flags:=0x%lX \n", interrupt, *pending, flags);
    }
#if defined(VTSS_SW_OPTION_SYNCE)
    clock_event_type_t clock_events;
    if (onesec) {/* clock controller must be polled once a sec in order to detect state change to passive */
        clock_event_poll(FALSE, &clock_events);
        clock_event_signal(clock_events, pending);
    }
#endif

    if (flags & INTERRUPT_EXT0)
    {
        if (board_type == VTSS_BOARD_LUTON26_REF)
        {
            for (i=12; i<24; i++)
            { /* Poll extern PHY */
                rc = vtss_phy_event_poll(PHY_INST, i, &phy_events);
                rc = vtss_phy_event_enable_set(PHY_INST, i, phy_events, FALSE);
                if (phy_events & VTSS_PHY_LINK_FFAIL_EV)   interrupt_signal_source(INTERRUPT_SOURCE_FLNK, i);
                if (phy_events & VTSS_PHY_LINK_LOS_EV)     interrupt_signal_source(INTERRUPT_SOURCE_LOS, i);
                if (phy_events & VTSS_PHY_LINK_AMS_EV)     interrupt_signal_source(INTERRUPT_SOURCE_AMS, i);
                if (phy_events & (VTSS_PHY_LINK_FFAIL_EV | VTSS_PHY_LINK_LOS_EV | VTSS_PHY_LINK_AMS_EV))    *pending = TRUE;
            }
        }

#if defined(VTSS_SW_OPTION_SYNCE)
        clock_event_poll(TRUE, &clock_events);
        clock_event_signal(clock_events, pending);
#endif
#if defined(VTSS_SW_OPTION_POE)
        interrupt_poe_poll(interrupt, pending);    /* POE must be the last to poll as it need pending from PHY and CLOCK */
#endif
#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
        zl_3034x_event_t zl_events;
        if((zl_3034x_event_poll(&zl_events) == VTSS_OK) && zl_events != 0) {
            T_DG(TRACE_GRP_IRQ0, "ZL event: %X", zl_events);
            (void) zl_3034x_event_enable_set(zl_events, FALSE);
            if (zl_events & ZL_TOP_ISR_REF_TS_ENG)
                interrupt_signal_source(INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG, 0);
        }
#endif
    }
    if (flags & INTERRUPT_SGPIO)
    {
        rc = vtss_sgpio_event_poll(NULL, 0, 0, 0, sgpio_events);
        for (i=24; i<26; i++)
            if (sgpio_events[i])
            {
                rc = vtss_sgpio_event_enable(NULL, 0, 0, i, 0, FALSE);
                interrupt_signal_source(INTERRUPT_SOURCE_LOS, i-24+((board_type == VTSS_BOARD_LUTON10_REF) ? 8 : 24));
                *pending = TRUE;
            }
    }
    if (flags & INTERRUPT_DEV_ALL)
    {
        for (i=0; i<((board_type == VTSS_BOARD_LUTON10_REF) ? 8 : 12); i++)
        { /* Poll internal PHY */
            rc = vtss_phy_event_poll(PHY_INST, i, &phy_events);
            rc = vtss_phy_event_enable_set(PHY_INST, i, phy_events, FALSE);
            if (phy_events & VTSS_PHY_LINK_FFAIL_EV)   interrupt_signal_source(INTERRUPT_SOURCE_FLNK, i);
            if (phy_events & VTSS_PHY_LINK_LOS_EV)     interrupt_signal_source(INTERRUPT_SOURCE_LOS, i);
            if (phy_events & VTSS_PHY_LINK_AMS_EV)     interrupt_signal_source(INTERRUPT_SOURCE_AMS, i);
            if (phy_events & (VTSS_PHY_LINK_FFAIL_EV | VTSS_PHY_LINK_LOS_EV | VTSS_PHY_LINK_AMS_EV))    *pending = TRUE;
        }
        if (chip_id.revision == 1)      vtss_intr_pol_negation(NULL);  /* Negate polarity on RevB */
    }
    if (flags & INTERRUPT_PTP)
    {
        rc = vtss_ptp_event_poll(NULL, &ptp_events);
        rc = vtss_ptp_event_enable(NULL, ptp_events, FALSE);
        if (ptp_events & VTSS_PTP_SYNC_EV)       interrupt_signal_source(INTERRUPT_SOURCE_SYNC, 0);
        if (ptp_events & VTSS_PTP_EXT_SYNC_EV)   interrupt_signal_source(INTERRUPT_SOURCE_EXT_SYNC, 0);
        if (ptp_events & VTSS_PTP_CLK_ADJ_EV)    interrupt_signal_source(INTERRUPT_SOURCE_CLK_ADJ, 0);
//        if (ptp_events & VTSS_PTP_TX_TSTAMP_EV)  interrupt_signal_source(INTERRUPT_SOURCE_TX_TSTAMP, 0);
        if (ptp_events & (VTSS_PTP_SYNC_EV | VTSS_PTP_EXT_SYNC_EV | VTSS_PTP_CLK_ADJ_EV))    *pending = TRUE;
    }
}



/****************************************************************************/
/*  Various local definitions                                               */
/****************************************************************************/

static cyg_uint32 ext0_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmarsk
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void ext0_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_EXT0);
}


static cyg_uint32 sgpio_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmarsk
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void sgpio_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_SGPIO);
}


static cyg_uint32 dev_all_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmarsk
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void dev_all_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_DEV_ALL);
}


static cyg_uint32 ptp_sync_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmarsk
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void ptp_sync_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_PTP);
}


#if defined(VTSS_SW_OPTION_POE)
void interrupt_poe_init_done()
{
    u32     i;
    vtss_phy_event_t       phy_events;

    /* Now POE is done initializing. We need to wait for that before enabling EXT0 because some POE devices need to be cofigured in order not to force this active at all time */

    if (board_type == VTSS_BOARD_LUTON26_REF)
    {
        /* This is ugly - I will now clear pending on external PHY connected to EXT0 otherwise events (Fast Link for protection) will be generated long after they actually happened */
        for (i=12; i<24; i++)
            (void)vtss_phy_event_poll(PHY_INST, i, &phy_events);
    }

    /* Enable EXT0 interrupt */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
}
#endif


/* Initialize module */
void interrupt_board_init(void)
{
    u32             rc;

    board_type = vtss_board_type();
    rc = vtss_chip_id_get(NULL, &chip_id);

    /* Handling extern interrupt - EXT0 */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EXT0,      /* Interrupt Vector */
                             0,                              /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             ext0_isr,
                             ext0_dsr,
                             &ext0_int_handle,
                             &ext0_int_object);

    cyg_drv_interrupt_attach(ext0_int_handle);

    rc = vtss_gpio_mode_set(NULL, 0, 8, VTSS_GPIO_ALT_0);

#if !defined(VTSS_SW_OPTION_POE)
    /* Enable EXT0 interrupt */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
#endif
    /* Handling SGPIO interrupt */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_SGPIO,      /* Interrupt Vector */
                             0,                               /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             sgpio_isr,
                             sgpio_dsr,
                             &sgpio_int_handle,
                             &sgpio_int_object);

    cyg_drv_interrupt_attach(sgpio_int_handle);

    /* Enable SGPIO interrupt */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SGPIO);

    /* Handling intern interrupt - DEV_ALL */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_DEV_ALL,    /* Interrupt Vector */
                             0,                               /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             dev_all_isr,
                             dev_all_dsr,
                             &dev_all_int_handle,
                             &dev_all_int_object);

    cyg_drv_interrupt_attach(dev_all_int_handle);

    // Setup interrupt from PHY.
    if (chip_id.revision == 0)    vtss_intr_cfg(NULL, 1 << 28, 1, 1);  /* On RevA Atom PHY interrupt signal is used */
    else if (chip_id.revision == 1) vtss_intr_cfg(NULL, 0xFFF, 0, 1);  /* On RevB Fast Link down signal is used */
    else vtss_intr_cfg(NULL, 1 << 28, 1, 1);  /* On RevC Atom PHY interrupt signal is used */

    
    /* Enable DEV_ALL interrupts */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_DEV_ALL);

    /* Handling intern interrupt - PTP_SYNC */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_PTP_SYNC,   /* Interrupt Vector */
                             0,                               /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             ptp_sync_isr,
                             ptp_sync_dsr,
                             &ptp_int_handle,
                             &ptp_int_object);

    cyg_drv_interrupt_attach(ptp_int_handle);

    /* Enable PTP_SYNC interrupts */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_PTP_SYNC);
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
