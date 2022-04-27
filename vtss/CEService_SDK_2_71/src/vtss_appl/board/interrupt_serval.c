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
#ifdef VTSS_SW_OPTION_MEP
#include "mep_api.h" /* For mep_voe_interrupt() */
#endif
#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
#include "zl_3034x_api_pdv_api.h"
#endif

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/****************************************************************************/
/*  Various local definitions                                               */
/****************************************************************************/

static cyg_interrupt    ext0_int_object;
static cyg_handle_t     ext0_int_handle;
static cyg_interrupt    voe_int_object;
static cyg_handle_t     voe_int_handle;
static cyg_interrupt    ptp_int_object;
static cyg_handle_t     ptp_int_handle;
static cyg_interrupt    ptp_ts_int_object;
static cyg_handle_t     ptp_ts_int_handle;
static cyg_interrupt    sgpio_int_object;
static cyg_handle_t     sgpio_int_handle;


#define INTERRUPT_EXT0     0x01
#define INTERRUPT_VOE      0x02
#define INTERRUPT_SGPIO    0x02
//#define INTERRUPT_DEV_ALL  0x04
#define INTERRUPT_PTP      0x08
#define INTERRUPT_PTP_RDY  0x10

static BOOL phy_port[VTSS_PORT_ARRAY_SIZE];

/* manage timestamp enabled ports */
static u32 phy1g_ts_cnt=0;
static u32 phy1g_ts_port_no[VTSS_PORT_ARRAY_SIZE];

/****************************************************************************/
/*  Module Interface                                                        */
/****************************************************************************/

void interrupt_device_enable(u32 flags, BOOL pending)
{
    if (flags & INTERRUPT_EXT0)
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
    if (flags & INTERRUPT_SGPIO) {
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SGPIO);
    }
    if (flags & INTERRUPT_VOE)
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_OAM_MEP);
    if (flags & INTERRUPT_PTP)
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_PTP_SYNC);
    if (flags & INTERRUPT_PTP_RDY)
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_PTP_RDY);
    
    
    
}

void interrupt_source_enable(vtss_interrupt_source_t source_id)
{
    vtss_rc rc=VTSS_RC_OK;
    vtss_phy_event_t phy_event = 0;
    vtss_phy_type_t    phy_id;
    int i;
    switch (source_id) {
    case INTERRUPT_SOURCE_LOS:
        phy_event = VTSS_PHY_LINK_LOS_EV;

        for (i=0; i<4; ++i)      rc = vtss_sgpio_event_enable(NULL, 0, 0, i, 0, TRUE);  /* Enable SGPIO for SFP ports */
        for (i=8; i<10; ++i)     rc = vtss_sgpio_event_enable(NULL, 0, 0, i, 0, TRUE);
        break;
        
    case INTERRUPT_SOURCE_FLNK:
    case INTERRUPT_SOURCE_AMS:
        phy_event = (source_id == INTERRUPT_SOURCE_FLNK) ? VTSS_PHY_LINK_FFAIL_EV : VTSS_PHY_LINK_AMS_EV;
        break;
    case INTERRUPT_SOURCE_SYNC:       rc = vtss_ptp_event_enable(NULL, VTSS_PTP_SYNC_EV, TRUE);      break;
    case INTERRUPT_SOURCE_EXT_SYNC:   rc = vtss_ptp_event_enable(NULL, VTSS_PTP_EXT_SYNC_EV, TRUE);  break;
    case INTERRUPT_SOURCE_EXT_1_SYNC: rc = vtss_ptp_event_enable(NULL, VTSS_PTP_EXT_1_SYNC_EV, TRUE);break;
    case INTERRUPT_SOURCE_CLK_ADJ:    rc = vtss_ptp_event_enable(NULL, VTSS_PTP_CLK_ADJ_EV, TRUE);   break;
    case INTERRUPT_SOURCE_CLK_TSTAMP: rc = vtss_ptp_event_enable(NULL, VTSS_PTP_TX_TSTAMP_EV, TRUE); break;

#if defined(VTSS_SW_OPTION_SYNCE)
    case INTERRUPT_SOURCE_LOCS:       clock_event_enable(CLOCK_LOCS1_EV); clock_event_enable(CLOCK_LOCS2_EV);     break;
    case INTERRUPT_SOURCE_FOS:        clock_event_enable(CLOCK_FOS1_EV); clock_event_enable(CLOCK_FOS2_EV);     break;
    case INTERRUPT_SOURCE_LOSX:       clock_event_enable(CLOCK_LOSX_EV);    break;
    case INTERRUPT_SOURCE_LOL:        clock_event_enable(CLOCK_LOL_EV);     break;
#endif
        
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        vtss_phy_ts_event_t  event;
        static BOOL first_enable = TRUE;
    case INTERRUPT_SOURCE_INGR_ENGINE_ERR:
    case INTERRUPT_SOURCE_INGR_RW_PREAM_ERR:
    case INTERRUPT_SOURCE_INGR_RW_FCS_ERR:
    case INTERRUPT_SOURCE_EGR_ENGINE_ERR:
    case INTERRUPT_SOURCE_EGR_RW_FCS_ERR:
    case INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED:
    case INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW:
        switch (source_id) {
            case INTERRUPT_SOURCE_INGR_ENGINE_ERR :        event = VTSS_PHY_TS_INGR_ENGINE_ERR; break;
            case INTERRUPT_SOURCE_INGR_RW_PREAM_ERR :      event = VTSS_PHY_TS_INGR_RW_PREAM_ERR; break;
            case INTERRUPT_SOURCE_INGR_RW_FCS_ERR :        event = VTSS_PHY_TS_INGR_RW_FCS_ERR; break;
            case INTERRUPT_SOURCE_EGR_ENGINE_ERR :         event = VTSS_PHY_TS_EGR_ENGINE_ERR; break;
            case INTERRUPT_SOURCE_EGR_RW_FCS_ERR :         event = VTSS_PHY_TS_EGR_RW_FCS_ERR; break;
            case INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED : event = VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED; break;
            case INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW :      event = VTSS_PHY_TS_EGR_FIFO_OVERFLOW; break;
            default:                                       event = 0;
        }
        if (first_enable) {
            first_enable = FALSE;
            for (i = VTSS_PORT_NO_START; i <VTSS_PORT_NO_END; i++) {
                if (VTSS_RC_OK == vtss_phy_id_get(PHY_INST, i, &phy_id)) {
                    if ((phy_id.part_number == VTSS_PHY_TYPE_8574) || (phy_id.part_number == VTSS_PHY_TYPE_8572)
                            || (phy_id.part_number == VTSS_PHY_TYPE_8582) || (phy_id.part_number == VTSS_PHY_TYPE_8584) 
                            || (phy_id.part_number == VTSS_PHY_TYPE_8575)) {
                        if (phy1g_ts_cnt < VTSS_PORT_ARRAY_SIZE) {
                            phy1g_ts_port_no[phy1g_ts_cnt++]= i;
                        }
                    }
                }
            }
        }
        for (i=0; i<phy1g_ts_cnt; i++) {
            rc = vtss_phy_ts_event_enable_set(PHY_INST, phy1g_ts_port_no[i], TRUE, event);
        }

        break;
#endif

#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
    zl_3034x_event_t zl_events;
    case INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG:
        zl_events = ZL_TOP_ISR_REF_TS_ENG;
        (void) zl_3034x_event_enable_set(zl_events, TRUE);
        break;
#endif

    case INTERRUPT_SOURCE_SGPIO_PUSH_BUTTON: 
      rc = vtss_sgpio_event_enable(NULL, 0, 0, 7, 0, TRUE); // Push button is Port 7 bit 0, See schematic.
      break;
        
    default:;
    }

    if(phy_event) {
        int i;
        for (i = 0; i < VTSS_PORTS; ++i)  
            if(phy_port[i])
                (void) vtss_phy_event_enable_set(PHY_INST, i, phy_event, TRUE);
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

void interrupt_device_poll(u32 flags, BOOL interrupt, BOOL onesec, BOOL *pending)
{
    vtss_rc rc=VTSS_RC_OK;
    vtss_ptp_event_type_t  ptp_events;
    u32 port, i;
    BOOL                   sgpio_events[VTSS_SGPIO_PORTS];
    T_D("Interrupt:%d, onesec:%d, flags:0x%X", interrupt, onesec, flags);

#if defined(VTSS_SW_OPTION_SYNCE)
    clock_event_type_t clock_events;
    if (onesec) {/* clock controller must be polled once a sec in order to detect state change to passive */
        clock_event_poll(FALSE, &clock_events);
        clock_event_signal(clock_events, pending);
    }
#endif

    if (flags & INTERRUPT_EXT0) {
        int i;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        if (!onesec) {
            for (i=0; i<phy1g_ts_cnt; i++) {
                port = phy1g_ts_port_no[i];
                vtss_phy_ts_event_t  ts_events;
                rc = vtss_phy_ts_event_poll(PHY_INST, port, &ts_events);
                if (ts_events) {
                    rc = vtss_phy_ts_event_enable_set(PHY_INST, port, FALSE, ts_events);
                    if (ts_events & VTSS_PHY_TS_INGR_ENGINE_ERR)         interrupt_signal_source(INTERRUPT_SOURCE_INGR_ENGINE_ERR, port);
                    if (ts_events & VTSS_PHY_TS_INGR_RW_PREAM_ERR)       interrupt_signal_source(INTERRUPT_SOURCE_INGR_RW_PREAM_ERR, port);
                    if (ts_events & VTSS_PHY_TS_INGR_RW_FCS_ERR)         interrupt_signal_source(INTERRUPT_SOURCE_INGR_RW_FCS_ERR, port);
                    if (ts_events & VTSS_PHY_TS_EGR_ENGINE_ERR)          interrupt_signal_source(INTERRUPT_SOURCE_EGR_ENGINE_ERR, port);
                    if (ts_events & VTSS_PHY_TS_EGR_RW_FCS_ERR)          interrupt_signal_source(INTERRUPT_SOURCE_EGR_RW_FCS_ERR, port);
                    if (ts_events & VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED)  interrupt_signal_source(INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED, port);
                    if (ts_events & VTSS_PHY_TS_EGR_FIFO_OVERFLOW)       interrupt_signal_source(INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW, port);
                    if (ts_events & (VTSS_PHY_TS_INGR_ENGINE_ERR | VTSS_PHY_TS_INGR_RW_PREAM_ERR | VTSS_PHY_TS_INGR_RW_FCS_ERR | 
                                     VTSS_PHY_TS_EGR_ENGINE_ERR | VTSS_PHY_TS_EGR_RW_FCS_ERR | VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED |
                                     VTSS_PHY_TS_EGR_FIFO_OVERFLOW))    *pending = TRUE;
                }
                if (*pending) return;
            }
        }
#endif
        
        
        for (i = 0; i < VTSS_PORTS; ++i) {
            if(phy_port[i]) {
                vtss_phy_event_t phy_events;
                if((vtss_phy_event_poll(NULL, i, &phy_events) == VTSS_OK) && phy_events != 0) {
                    T_DG(TRACE_GRP_IRQ0, "Port %d, PHY event: %x", i, phy_events);
                    (void) vtss_phy_event_enable_set(PHY_INST, i, phy_events, FALSE);
                    if (phy_events & VTSS_PHY_LINK_FFAIL_EV)
                        interrupt_signal_source(INTERRUPT_SOURCE_FLNK, i);
                    if (phy_events & VTSS_PHY_LINK_LOS_EV)     
                        interrupt_signal_source(INTERRUPT_SOURCE_LOS, i);
                    if (phy_events & VTSS_PHY_LINK_AMS_EV)
                        interrupt_signal_source(INTERRUPT_SOURCE_AMS, i);
                    if (phy_events & (VTSS_PHY_LINK_FFAIL_EV | VTSS_PHY_LINK_LOS_EV | VTSS_PHY_LINK_AMS_EV))
                        *pending = TRUE;
                }
            }
        }
#if defined(VTSS_SW_OPTION_SYNCE)
        clock_event_poll(TRUE, &clock_events);
        clock_event_signal(clock_events, pending);
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

    if (flags & INTERRUPT_SGPIO) {
        // Push button is Port 7 bit 0, See schematic.
        rc = vtss_sgpio_event_poll(NULL, 0, 0, 0, sgpio_events); // Chip_no=0, Group=0 and bit = 0)
        if (sgpio_events[7]) {
            rc = vtss_sgpio_event_enable(NULL, 0, 0, 7, 0, FALSE); // Disable interrupt.
            interrupt_signal_source(INTERRUPT_SOURCE_SGPIO_PUSH_BUTTON, 0);
            *pending = TRUE;
        }

        for (i=0; i<4; i++) {   /* Poll SGPIO LOS from SFP ports */
            if (sgpio_events[i]) {  /* LOS port 4+i */
                rc = vtss_sgpio_event_enable(NULL, 0, 0, i, 0, FALSE);
                interrupt_signal_source(INTERRUPT_SOURCE_LOS, 7-i);
                *pending = TRUE;
            }
        }
        for (i=8; i<10; i++) {
            if (sgpio_events[i]) {  /* LOS port 8+i */
                rc = vtss_sgpio_event_enable(NULL, 0, 0, i, 0, FALSE);
                interrupt_signal_source(INTERRUPT_SOURCE_LOS, i);
                *pending = TRUE;
            }
        }
    }

    if (flags & INTERRUPT_VOE) { /* This is special - only handled by MEP module */
#ifdef VTSS_SW_OPTION_MEP
        mep_voe_interrupt();
#endif
    }
    if (flags & INTERRUPT_PTP)
    {
        rc = vtss_ptp_event_poll(NULL, &ptp_events);
        rc = vtss_ptp_event_enable(NULL, ptp_events, FALSE);
        if (ptp_events & VTSS_PTP_SYNC_EV)       interrupt_signal_source(INTERRUPT_SOURCE_SYNC, 0);
        if (ptp_events & VTSS_PTP_EXT_SYNC_EV)   interrupt_signal_source(INTERRUPT_SOURCE_EXT_SYNC, 0);
        if (ptp_events & VTSS_PTP_EXT_1_SYNC_EV) interrupt_signal_source(INTERRUPT_SOURCE_EXT_1_SYNC, 0);
        if (ptp_events & VTSS_PTP_CLK_ADJ_EV)    interrupt_signal_source(INTERRUPT_SOURCE_CLK_ADJ, 0);
        if (ptp_events & (VTSS_PTP_SYNC_EV | VTSS_PTP_EXT_SYNC_EV | VTSS_PTP_EXT_1_SYNC_EV | VTSS_PTP_CLK_ADJ_EV))    *pending = TRUE;
    }
    if (flags & INTERRUPT_PTP_RDY)
    {
        interrupt_signal_source(INTERRUPT_SOURCE_CLK_TSTAMP, 0);
        *pending = TRUE;
    }
}

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


static cyg_uint32 voe_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmarsk
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void voe_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_VOE);
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

static cyg_uint32 ptp_ts_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmarsk
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void ptp_ts_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_PTP_RDY);
}


void interrupt_board_init(void)
{
    int i, board_type;

    /* Handling extern interrupt - EXT0 */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EXT0,      /* Interrupt Vector */
                             0,                              /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             ext0_isr,
                             ext0_dsr,
                             &ext0_int_handle,
                             &ext0_int_object);

    cyg_drv_interrupt_attach(ext0_int_handle);

    (void) vtss_gpio_mode_set(NULL, 0, 28, VTSS_GPIO_ALT_0); /* Ext0 == alt0 on GPIO 28 */


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

    board_type = vtss_board_type();

    for(i = 0; i < VTSS_PORTS; i++)
        phy_port[i] = (!!(vtss_board_port_cap(board_type, i) & (PORT_CAP_COPPER | PORT_CAP_DUAL_COPPER)));

    /* Enable EXT0 interrupt */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);

    /* Handling VOE interrupt */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_OAM_MEP,    /* Interrupt Vector */
                             0,                               /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             voe_isr,
                             voe_dsr,
                             &voe_int_handle,
                             &voe_int_object);

    cyg_drv_interrupt_attach(voe_int_handle);

    /* Enable VOE interrupt */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_OAM_MEP);

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

    /* Handling intern interrupt - PTP_RDY (Timestamp ready) */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_PTP_RDY,    /* Interrupt Vector */
                             0,                               /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             ptp_ts_isr,
                             ptp_ts_dsr,
                             &ptp_ts_int_handle,
                             &ptp_ts_int_object);
    
    cyg_drv_interrupt_attach(ptp_ts_int_handle);
    
    /* Enable PTP_RDY interrupts */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_PTP_RDY);


}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
