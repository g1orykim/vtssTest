/*

 Vitesse Interrupt module software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "vtss_phy_ts_api.h"
#endif
#if defined(VTSS_FEATURE_WIS)
#include "vtss_wis_api.h"
#endif
#include "port_api.h"
#include "vtss_api_if_api.h"   /* For vtss_api_if_chip_count() */
#if defined(VTSS_SW_OPTION_SYNCE)
#include "synce_custom_clock_api.h"
#endif
#include "main.h"
#include "misc_api.h"
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

static cyg_interrupt    ext1_int_object;
static cyg_handle_t     ext1_int_handle;

static cyg_interrupt    sgpio_int_object;
static cyg_handle_t     sgpio_int_handle;

static cyg_interrupt    dev_all_int_object;
static cyg_handle_t     dev_all_int_handle;

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
static cyg_interrupt    dev_all_sec_int_object;
static cyg_handle_t     dev_all_sec_int_handle;
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */

static cyg_interrupt    ptp_int_object;
static cyg_handle_t     ptp_int_handle;

static cyg_vector_t     ext_irq;

static int board_type;
static u32 phy1g_cnt=0;


#define PORT_10G_MAX    4
#define PORT_1G_MAX    VTSS_PORTS
typedef struct
{
    vtss_port_no_t  port;
    BOOL            phy;
    BOOL            feature_ts;
} port_1_10g_t;
static port_1_10g_t port_10g[PORT_10G_MAX];
static vtss_port_no_t port10g_start = VTSS_PORT_NO_NONE;
static vtss_port_no_t port10g_end = VTSS_PORT_NO_NONE;

static port_1_10g_t port_1g[PORT_1G_MAX];
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static BOOL ts_10g_firsttime = TRUE;
static u32 feature_ts_1g_port_start = 0xffff;
static u32 feature_ts_1g_port_end = 0;
#endif

#define INTERRUPT_EXT0          0x01
#define INTERRUPT_EXT1          0x02
#define INTERRUPT_SGPIO         0x04
#define INTERRUPT_DEV_ALL       0x08
#define INTERRUPT_SEC_DEV_ALL   0x10
#define INTERRUPT_PTP           0x20

/****************************************************************************/
/*  Module Interface                                                        */
/****************************************************************************/
#if defined(VTSS_SW_OPTION_POE)
static BOOL interrupt_poe_active = FALSE;
#endif


void interrupt_device_enable(u32 flags, BOOL pending)
{
    T_R("flags:0x%X", flags);
    if (flags & INTERRUPT_EXT0) {
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
    }

    if (flags & INTERRUPT_EXT1)
#if defined(VTSS_SW_OPTION_POE)
        if (pending || !interrupt_poe_active)   cyg_drv_interrupt_unmask(ext_irq);      /* ext_irq is only enabled if Pending or POE I2C interrupt is not expected - no Pending means POE interrupt and unmask is done through POE hookup */
#else
        cyg_drv_interrupt_unmask(ext_irq);
#endif

    if (flags & INTERRUPT_SGPIO) {
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SGPIO);
    }

    if (flags & INTERRUPT_DEV_ALL) {
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_DEV_ALL);
    }

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
    if (flags & INTERRUPT_SEC_DEV_ALL) {
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SEC_DEV_ALL);
    }
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */

    if (flags & INTERRUPT_PTP) {
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_PTP_SYNC);
    }
}

#if defined(VTSS_SW_OPTION_POE)
void interrupt_poe_poll(BOOL interrupt, BOOL *pending)
{
    /* POE is special as there is no pending indication or interrupt enable/disable on this device */
    /* Only poll POE if interrupt and no pending on other devices on this physical interrupt */
    if (interrupt && !(*pending)) {
        interrupt_signal_source(INTERRUPT_SOURCE_I2C, 0);
    }
}

void interrupt_poe_source_enable(vtss_interrupt_source_t source_id)
{
    interrupt_poe_active = TRUE; /* From now on we asume that POE is handling the I2C event and EXT0 is only unmasked if pending is detected */
    cyg_drv_interrupt_unmask(ext_irq);    /* In case of POE the unmask is done through POE hookup (now) */
}
#else
void interrupt_poe_poll(BOOL interrupt, BOOL *pending)
{
    *pending = *pending;
}

void interrupt_poe_source_enable(vtss_interrupt_source_t source_id)
{
    source_id = source_id;
}
#endif

static BOOL init = TRUE;

static void phy_int_conf(void)
{
    vtss_rc rc = VTSS_RC_OK;
    u32 i, inx88, port;
    vtss_phy_10g_id_t          phy_id;
    vtss_gpio_10g_gpio_mode_t  gpio_mode;
    int tries;

    if (init) { /* This initialization is done here - cause now i'm sure PHY API is initialized */
        // This will wait until the PHYs are initialized.
        // This function is not working anymore as 10G setup in portmodule is no more behind critd        port_phy_wait_until_ready();
        init = FALSE;
        for (i=0, inx88=0; i<PORT_10G_MAX; i++) {
            if (port_10g[i].phy) {
                port = port_10g[i].port;
                T_W("Read 10G port %d PHY part number", iport2uport(port));
                tries = 0;
                while (VTSS_RC_ERROR == (rc = vtss_phy_10g_id_get(PHY_INST, port, &phy_id)) && ++tries < 3)      VTSS_OS_MSLEEP(10);
                if (rc == VTSS_RC_OK) {
                    if ((phy_id.part_number == 0x8484) || (phy_id.part_number == 0x8487) || (phy_id.part_number == 0x8488)
                            || (phy_id.part_number == 0x8489) || (phy_id.part_number == 0x8490) || (phy_id.part_number == 0x8491)) {
                        gpio_mode.mode = VTSS_10G_PHY_GPIO_WIS_INT;
                        gpio_mode.port = port;
                        if (board_type == VTSS_BOARD_JAG_PCB107_REF && (port == 26 || port == 27)) {
                            /*On PCB107 only GPIO_6 is used for interrupt (GPIO_7 is used for SFP control) */
                            /* Therefore both channels uses WIS_INTB */
                            rc = vtss_phy_10g_gpio_mode_set(PHY_INST, port, 6, &gpio_mode);
                            T_D("Phy part_number %x,GPIO %d set up to interrupt for port %d", phy_id.part_number, 6, port);
                        } else {
                            rc = vtss_phy_10g_gpio_mode_set(PHY_INST, port, 6+(inx88%2), &gpio_mode);
                            T_D("Phy part_number %x,GPIO %d set up to interrupt for port %d", phy_id.part_number, 6+(inx88%2), port);
                        }
                        inx88++;
                    }
                } else {
                    T_W("Failed read 10G port %d PHY part number", iport2uport(port));
                }
            }
        }
    }
}

void interrupt_source_enable(vtss_interrupt_source_t     source_id)
{
    u32 i, port, rc=VTSS_RC_OK;
    vtss_phy_10g_id_t  phy_10g_id;
    vtss_phy_type_t    phy_id;
    BOOL sfp_not_present;
    vtss_gpio_10g_no_t gpio_no;

    T_I("source_id:%d", source_id);
    phy_int_conf();
    switch (source_id)
    {
        case INTERRUPT_SOURCE_LOS:
            if ((board_type == VTSS_BOARD_JAG_CU24_REF) || (board_type == VTSS_BOARD_JAG_CU48_REF))
                for (i=0; i<phy1g_cnt; i++) {
                    vtss_phy_id_get(PHY_INST, i, &phy_id);  /* This check for Tesla is only temporary */
                    if ((phy_id.part_number != VTSS_PHY_TYPE_8574) && (phy_id.part_number != VTSS_PHY_TYPE_8504) && (phy_id.part_number != VTSS_PHY_TYPE_8572) && (phy_id.part_number != VTSS_PHY_TYPE_8552) && (phy_id.part_number != VTSS_PHY_TYPE_8502))
                        rc = vtss_phy_event_enable_set(PHY_INST, i, VTSS_PHY_LINK_LOS_EV, TRUE);
                }


            if (board_type == VTSS_BOARD_JAG_SFP24_REF)
                for (i=0; i<24; ++i)      rc = vtss_sgpio_event_enable(NULL, 0, 0, i, 0, TRUE);


            if (board_type == VTSS_BOARD_JAG_PCB107_REF) {
                T_I("Source enable 1G phys - LOS");
                for (i=0; i<phy1g_cnt; i++) {
                  rc = vtss_phy_event_enable_set(PHY_INST, i, VTSS_PHY_LINK_LOS_EV, TRUE);
                }
            }

            if (board_type == VTSS_BOARD_JAG_SFP24_REF)
                for (i=0; i<24; ++i)      rc = vtss_sgpio_event_enable(NULL, 0, 0, i, 0, TRUE);
            for (i=0; i<PORT_10G_MAX; i++) {
                port = port_10g[i].port;
                if (port_10g[i].phy) {
                    rc = vtss_phy_10g_id_get(PHY_INST, port, &phy_10g_id);
                    if ((phy_10g_id.part_number == 0x8484) || (phy_10g_id.part_number == 0x8487) || (phy_10g_id.part_number == 0x8488)
                            || (phy_10g_id.part_number == 0x8489) || (phy_10g_id.part_number == 0x8490) || (phy_10g_id.part_number == 0x8491)) {
                        /* need to know if the PHY in on the PCB107 board (port 26 or 27), or if it is on the ESTAXIII-8488 plugin */
                        if (board_type == VTSS_BOARD_JAG_PCB107_REF && (port == 26 || port == 27)) {
                            gpio_no = phy_10g_id.channel_id == 1 ? 9 : 0;  /* PCB107 uses GPIO 0 and 9 */
                        } else {
                            gpio_no = phy_10g_id.channel_id == 1 ? 9 : 8;  /* Plugin uses GPIO 8 and 9 */
                        }
                        rc = vtss_phy_10g_gpio_read(PHY_INST, port, gpio_no, &sfp_not_present);
                        if (sfp_not_present) { /* receive fault may be unstable if SFP not present, use LOPC instead */
                            rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_LOPC_EV, TRUE);
                            T_D("Source enable 10G phy - LOPC port %d", iport2uport(port));
                        } else {
                            rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV, TRUE);
                            T_D("Source enable 10G phy - RX_FAULT port %d", iport2uport(port));
                        }

                    } else
                        rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_LINK_LOS_EV, TRUE);
                }
                else    // If there is no PHY - link state change is detected in DEV
                    rc = vtss_dev_all_event_enable(NULL, port, VTSS_DEV_ALL_LINK_EV, TRUE);
            }
            break;

        case INTERRUPT_SOURCE_RX_LOL:
        case INTERRUPT_SOURCE_LOPC:
        case INTERRUPT_SOURCE_RECEIVE_FAULT:
#if defined (VTSS_FEATURE_WIS)
        case INTERRUPT_SOURCE_EWIS_SEF_EV:
        case INTERRUPT_SOURCE_EWIS_FPLM_EV:
        case INTERRUPT_SOURCE_EWIS_FAIS_EV:
        case INTERRUPT_SOURCE_EWIS_LOF_EV:
        case INTERRUPT_SOURCE_EWIS_RDIL_EV:
        case INTERRUPT_SOURCE_EWIS_AISL_EV:
        case INTERRUPT_SOURCE_EWIS_LCDP_EV:
        case INTERRUPT_SOURCE_EWIS_PLMP_EV:
        case INTERRUPT_SOURCE_EWIS_AISP_EV:
        case INTERRUPT_SOURCE_EWIS_LOPP_EV:
        case INTERRUPT_SOURCE_EWIS_UNEQP_EV:
        case INTERRUPT_SOURCE_EWIS_FEUNEQP_EV:
        case INTERRUPT_SOURCE_EWIS_FERDIP_EV:
        case INTERRUPT_SOURCE_EWIS_REIL_EV:
        case INTERRUPT_SOURCE_EWIS_REIP_EV:
        case INTERRUPT_SOURCE_EWIS_B1_NZ_EV:
        case INTERRUPT_SOURCE_EWIS_B2_NZ_EV:
        case INTERRUPT_SOURCE_EWIS_B3_NZ_EV:
        case INTERRUPT_SOURCE_EWIS_REIL_NZ_EV:
        case INTERRUPT_SOURCE_EWIS_REIP_NZ_EV:
        case INTERRUPT_SOURCE_EWIS_B1_THRESH_EV:
        case INTERRUPT_SOURCE_EWIS_B2_THRESH_EV:
        case INTERRUPT_SOURCE_EWIS_B3_THRESH_EV:
        case INTERRUPT_SOURCE_EWIS_REIL_THRESH_EV:
        case INTERRUPT_SOURCE_EWIS_REIP_THRESH_EV:
#endif /* VTSS_FEATURE_WIS */
            for (i=0; i<PORT_10G_MAX; i++) {
                port = port_10g[i].port;
                if (port_10g[i].phy) {
                    rc = vtss_phy_10g_id_get(PHY_INST, port, &phy_10g_id);
                    if ((phy_10g_id.part_number == 0x8484) || (phy_10g_id.part_number == 0x8487) || (phy_10g_id.part_number == 0x8488)) {
                        if (source_id == INTERRUPT_SOURCE_RX_LOL)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_RX_LOL_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_LOPC)            rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_LOPC_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_RECEIVE_FAULT)   rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_MODULE_STAT)   rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_MODULE_STAT_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_TX_LOL)   rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_TX_LOL_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_HI_BER)   rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_10G_HIGH_BER_EV, TRUE);
#if defined (VTSS_FEATURE_WIS)
                        if (source_id == INTERRUPT_SOURCE_EWIS_SEF_EV)           rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_SEF_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_FPLM_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_FPLM_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_FAIS_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_FAIS_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_LOF_EV)           rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_LOF_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_RDIL_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_RDIL_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_AISL_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_AISL_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_LCDP_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_LCDP_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_PLMP_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_PLMP_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_AISP_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_AISP_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_LOPP_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_LOPP_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_UNEQP_EV)         rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_UNEQP_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_FEUNEQP_EV)       rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_FEUNEQP_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_FERDIP_EV)        rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_FERDIP_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_REIL_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_REIL_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_REIP_EV)          rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_REIP_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_B1_NZ_EV)         rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_B1_NZ_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_B2_NZ_EV)         rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_B2_NZ_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_B3_NZ_EV)         rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_B3_NZ_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_REIL_NZ_EV)       rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_REIL_NZ_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_REIP_NZ_EV)       rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_REIP_NZ_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_B1_THRESH_EV)     rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_B1_THRESH_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_B2_THRESH_EV)     rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_B2_THRESH_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_B3_THRESH_EV)     rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_B3_THRESH_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_REIL_THRESH_EV)   rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_REIL_THRESH_EV, TRUE);
                        if (source_id == INTERRUPT_SOURCE_EWIS_REIP_THRESH_EV)   rc = vtss_phy_10g_event_enable_set(PHY_INST, port, VTSS_PHY_EWIS_REIP_THRESH_EV, TRUE);
#endif /* VTSS_FEATURE_WIS */

                    }
                }
            }
            break;

        case INTERRUPT_SOURCE_FLNK:
        case INTERRUPT_SOURCE_AMS:
            if ((board_type == VTSS_BOARD_JAG_CU24_REF) || (board_type == VTSS_BOARD_JAG_CU48_REF))
                for (i=0; i<phy1g_cnt; ++i) {
                    vtss_phy_id_get(PHY_INST, i, &phy_id);  /* This check for Tesla is only temporary */
                    if ((phy_id.part_number != VTSS_PHY_TYPE_8574) && (phy_id.part_number != VTSS_PHY_TYPE_8504) && (phy_id.part_number != VTSS_PHY_TYPE_8572) && (phy_id.part_number != VTSS_PHY_TYPE_8552) && (phy_id.part_number != VTSS_PHY_TYPE_8502))
                        rc = vtss_phy_event_enable_set(PHY_INST, i, (source_id == INTERRUPT_SOURCE_FLNK) ? VTSS_PHY_LINK_FFAIL_EV : VTSS_PHY_LINK_AMS_EV, TRUE);
                }

            if (board_type == VTSS_BOARD_JAG_PCB107_REF) {
                T_D("Source enable 1G phys - AMS");
                for (i=0; i<phy1g_cnt; ++i) {
                  rc = vtss_phy_event_enable_set(PHY_INST, i, (source_id == INTERRUPT_SOURCE_FLNK) ? VTSS_PHY_LINK_FFAIL_EV : VTSS_PHY_LINK_AMS_EV, TRUE);
                }
            }
            break;

        case INTERRUPT_SOURCE_SYNC:       rc = vtss_ptp_event_enable(NULL, VTSS_PTP_SYNC_EV, TRUE);               break;
        case INTERRUPT_SOURCE_EXT_SYNC:   rc = vtss_ptp_event_enable(NULL, VTSS_PTP_EXT_SYNC_EV, TRUE);           break;
        case INTERRUPT_SOURCE_CLK_ADJ:    rc = vtss_ptp_event_enable(NULL, VTSS_PTP_CLK_ADJ_EV, TRUE);            break;
        case INTERRUPT_SOURCE_CLK_TSTAMP: {
            u32 port_count = port_isid_port_count(VTSS_ISID_LOCAL);
            for (i = 0; i < port_count; i++)
                rc = vtss_dev_all_event_enable(NULL, i, VTSS_DEV_ALL_TX_TSTAMP_EV, TRUE);
            break;
        }

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        vtss_phy_ts_event_t  event;
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

            for (i=0; i<PORT_10G_MAX; i++) {
                port = port_10g[i].port;
                if (ts_10g_firsttime && port_10g[i].phy) {
                    rc = vtss_phy_10g_id_get(PHY_INST, port, &phy_10g_id);
                    if (((phy_10g_id.part_number == 0x8488 || phy_10g_id.part_number == 0x8487) && phy_10g_id.revision >= 4)
                            || (phy_10g_id.part_number == 0x8489) || (phy_10g_id.part_number == 0x8490) || (phy_10g_id.part_number == 0x8491)) {
                        port_10g[i].feature_ts = TRUE;
                        T_W("10G phy timestamp interrupt enabled port %d", iport2uport(port));
                    }
                }
                if (port_10g[i].feature_ts) {
                    rc = vtss_phy_ts_event_enable_set(PHY_INST, port, TRUE, event);
                }
            }
            for (i=0; i<phy1g_cnt; i++) {
                port = port_1g[i].port;
                if (ts_10g_firsttime && port_1g[i].phy) {
                    vtss_phy_id_get(PHY_INST, port, &phy_id);
                    if ((phy_id.part_number == VTSS_PHY_TYPE_8574 || phy_id.part_number == VTSS_PHY_TYPE_8572
                            || phy_id.part_number == VTSS_PHY_TYPE_8582 || phy_id.part_number == VTSS_PHY_TYPE_8584
                            || phy_id.part_number == VTSS_PHY_TYPE_8575)) {
                        port_1g[i].feature_ts = TRUE;
                        if (feature_ts_1g_port_start == 0xffff) feature_ts_1g_port_start = port;
                        feature_ts_1g_port_end = port;
                        T_W("1G phy timestamp interrupt enabled port %d", iport2uport(port));
                    }
                }
                if (port_1g[i].feature_ts) {
                    rc = vtss_phy_ts_event_enable_set(PHY_INST, port, TRUE, event);
                }
            }
            ts_10g_firsttime = FALSE;
            break;
#endif
#if defined(VTSS_SW_OPTION_SYNCE)
        case INTERRUPT_SOURCE_LOCS:       clock_event_enable(CLOCK_LOCS1_EV); clock_event_enable(CLOCK_LOCS2_EV);     break;
        case INTERRUPT_SOURCE_FOS:        clock_event_enable(CLOCK_FOS1_EV); clock_event_enable(CLOCK_FOS2_EV);     break;
        case INTERRUPT_SOURCE_LOSX:       clock_event_enable(CLOCK_LOSX_EV);    break;
        case INTERRUPT_SOURCE_LOL:        clock_event_enable(CLOCK_LOL_EV);     break;
#endif

#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
            zl_3034x_event_t zl_events;
        case INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG:
            zl_events = ZL_TOP_ISR_REF_TS_ENG;
            (void) zl_3034x_event_enable_set(zl_events, TRUE);
            break;
#endif

        case INTERRUPT_SOURCE_I2C:        interrupt_poe_source_enable(source_id);       break;

        default:
            return;
    }
}

void interrupt_dev_all_poll(vtss_dev_all_event_poll_t poll_type,  BOOL *pending)
{
    u32 i, port_count = port_isid_port_count(VTSS_ISID_LOCAL);
    vtss_rc rc = VTSS_RC_OK;
    vtss_dev_all_event_type_t  dev_all_events[VTSS_PORT_ARRAY_SIZE];

    rc = vtss_dev_all_event_poll(NULL, poll_type, dev_all_events);

    for (i = 0; i < port_count; i++) {
        if (dev_all_events[i]) {
            rc = vtss_dev_all_event_enable(NULL, i, dev_all_events[i], FALSE);
            if (dev_all_events[i] & VTSS_DEV_ALL_TX_TSTAMP_EV)  interrupt_signal_source(INTERRUPT_SOURCE_CLK_TSTAMP, i);
            if (dev_all_events[i] & VTSS_DEV_ALL_LINK_EV)       interrupt_signal_source(INTERRUPT_SOURCE_LOS, i);
            if (dev_all_events[i] & (VTSS_DEV_ALL_TX_TSTAMP_EV | VTSS_DEV_ALL_LINK_EV))  *pending = TRUE;
        }
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


// Function for 1G PHY interrupt
static vtss_rc phy1g_events_handling(vtss_port_no_t start_iport, vtss_port_no_t end_iport, BOOL *pending) {
  vtss_port_no_t i;
  vtss_phy_event_t           phy_events;
  T_I("phy_1 geventhandling");
  for (i=start_iport; i<=end_iport; i++) {
    VTSS_RC(vtss_phy_event_poll(PHY_INST, i, &phy_events));
    if (phy_events) {
        VTSS_RC(vtss_phy_event_enable_set(PHY_INST, i, phy_events, FALSE));
        T_I_PORT(i, "phy_events:0x%X", phy_events);
        if (phy_events & VTSS_PHY_LINK_FFAIL_EV)   interrupt_signal_source(INTERRUPT_SOURCE_FLNK, i);
        if (phy_events & VTSS_PHY_LINK_LOS_EV)     interrupt_signal_source(INTERRUPT_SOURCE_LOS, i);
        if (phy_events & VTSS_PHY_LINK_AMS_EV)     interrupt_signal_source(INTERRUPT_SOURCE_AMS, i);
        if (phy_events & (VTSS_PHY_LINK_FFAIL_EV | VTSS_PHY_LINK_LOS_EV | VTSS_PHY_LINK_AMS_EV))    *pending = TRUE;
    }
  }
  return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
// Function for 1G PHY timestamp interrupt
static vtss_rc phy1g_ts_events_handling(vtss_port_no_t start_iport, vtss_port_no_t end_iport, BOOL *pending) {
  vtss_port_no_t i, port;
  T_I("phy_ts_eventhandling");
  for (i=start_iport; i<=end_iport; i++) {
    vtss_phy_ts_event_t  ts_events;
    if (port_1g[i].feature_ts) {
        port = port_1g[i].port;  /* 'i' is equal to the index in the port_1g table */
        VTSS_RC(vtss_phy_ts_event_poll(PHY_INST, port, &ts_events));
        T_I("port %d, ts_events:0x%X", iport2uport(port), ts_events);
        if (ts_events) {
            VTSS_RC(vtss_phy_ts_event_enable_set(PHY_INST, port, FALSE, ts_events));
            if (ts_events & VTSS_PHY_TS_INGR_ENGINE_ERR)         interrupt_signal_source(INTERRUPT_SOURCE_INGR_ENGINE_ERR, port);
            if (ts_events & VTSS_PHY_TS_INGR_RW_PREAM_ERR)       interrupt_signal_source(INTERRUPT_SOURCE_INGR_RW_PREAM_ERR, port);
            if (ts_events & VTSS_PHY_TS_INGR_RW_FCS_ERR)         interrupt_signal_source(INTERRUPT_SOURCE_INGR_RW_FCS_ERR, port);
            if (ts_events & VTSS_PHY_TS_EGR_ENGINE_ERR)          interrupt_signal_source(INTERRUPT_SOURCE_EGR_ENGINE_ERR, port);
            if (ts_events & VTSS_PHY_TS_EGR_RW_FCS_ERR)          interrupt_signal_source(INTERRUPT_SOURCE_EGR_RW_FCS_ERR, port);
            if (ts_events & VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED)  interrupt_signal_source(INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED, port);
            if (ts_events & VTSS_PHY_TS_EGR_FIFO_OVERFLOW)       interrupt_signal_source(INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW, port);
            if (ts_events & (VTSS_PHY_TS_INGR_ENGINE_ERR | VTSS_PHY_TS_INGR_RW_PREAM_ERR | VTSS_PHY_TS_INGR_RW_FCS_ERR |
                             VTSS_PHY_TS_EGR_ENGINE_ERR | VTSS_PHY_TS_EGR_RW_FCS_ERR | VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED | VTSS_PHY_TS_EGR_FIFO_OVERFLOW))    *pending = TRUE;
        }
    }
  }
  return VTSS_RC_OK;
}
#endif

#if defined(VTSS_SW_OPTION_SYNCE)
static vtss_rc synce_events_handling(BOOL *pending) {
  clock_event_type_t clock_events;
  clock_event_poll(TRUE, &clock_events);
  clock_event_signal(clock_events, pending);
  return VTSS_RC_OK;
}
#endif


static vtss_rc ptp_events_handling(BOOL *pending) {
  vtss_ptp_event_type_t      ptp_events = 0;
  VTSS_RC(vtss_ptp_event_poll(NULL, &ptp_events));
  VTSS_RC(vtss_ptp_event_enable(NULL, ptp_events, FALSE));
  if (ptp_events & VTSS_PTP_SYNC_EV)       interrupt_signal_source(INTERRUPT_SOURCE_SYNC, 0);
  if (ptp_events & VTSS_PTP_EXT_SYNC_EV)   interrupt_signal_source(INTERRUPT_SOURCE_EXT_SYNC, 0);
  if (ptp_events & VTSS_PTP_CLK_ADJ_EV)    interrupt_signal_source(INTERRUPT_SOURCE_CLK_ADJ, 0);
  if (ptp_events & (VTSS_PTP_SYNC_EV | VTSS_PTP_EXT_SYNC_EV | VTSS_PTP_CLK_ADJ_EV))    *pending = TRUE;
  return VTSS_RC_OK;
}

static vtss_rc phy10g_events_handling(vtss_port_no_t start_iport, vtss_port_no_t end_iport, BOOL *pending) {
  u32 i;
  vtss_port_no_t port;
  vtss_phy_10g_event_t       phy10g_events;
  T_I("10G events, start_iport %d, end_iport %d", start_iport, end_iport);
  for (i=0; i<PORT_10G_MAX; i++) {   // Polling of solded 8486 and 8484+87+88 in X2 slot
    if (port_10g[i].phy) {
      port = port_10g[i].port;
      if (port >= start_iport && port <= end_iport) {
          VTSS_RC(vtss_phy_10g_event_poll(PHY_INST, port, &phy10g_events));
          if (phy10g_events) {
              T_I("phy10g port %d events:0x%X", iport2uport(port), phy10g_events);
              VTSS_RC(vtss_phy_10g_event_enable_set(PHY_INST, port, phy10g_events, FALSE));
              if (phy10g_events & VTSS_PHY_10G_LINK_LOS_EV)            interrupt_signal_source(INTERRUPT_SOURCE_LOS, port);
              if (phy10g_events & VTSS_PHY_10G_RX_LOL_EV)              interrupt_signal_source(INTERRUPT_SOURCE_RX_LOL, port);
              if (phy10g_events & VTSS_PHY_10G_LOPC_EV)                interrupt_signal_source(INTERRUPT_SOURCE_LOPC, port);
              if ((phy10g_events & VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV) || (phy10g_events & VTSS_PHY_10G_LOPC_EV))  {
                interrupt_signal_source(INTERRUPT_SOURCE_LOS, port);
                interrupt_signal_source(INTERRUPT_SOURCE_RECEIVE_FAULT, port);
                T_I("phy10g port %d LOPC or RX_FAULT", iport2uport(port));
              }
              if (phy10g_events & VTSS_PHY_10G_MODULE_STAT_EV)         interrupt_signal_source(INTERRUPT_SOURCE_MODULE_STAT, port);
              if (phy10g_events & VTSS_PHY_10G_TX_LOL_EV)              interrupt_signal_source(INTERRUPT_SOURCE_TX_LOL, port);
              if (phy10g_events & VTSS_PHY_10G_HIGH_BER_EV)            interrupt_signal_source(INTERRUPT_SOURCE_HI_BER, port);

#if defined(VTSS_FEATURE_WIS)
              if (phy10g_events & VTSS_PHY_EWIS_SEF_EV)                interrupt_signal_source(INTERRUPT_SOURCE_EWIS_SEF_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_FPLM_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_FPLM_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_FAIS_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_FAIS_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_LOF_EV)                interrupt_signal_source(INTERRUPT_SOURCE_EWIS_LOF_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_RDIL_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_RDIL_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_AISL_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_AISL_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_LCDP_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_LCDP_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_PLMP_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_PLMP_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_AISP_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_AISP_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_LOPP_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_LOPP_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_UNEQP_EV)              interrupt_signal_source(INTERRUPT_SOURCE_EWIS_UNEQP_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_FEUNEQP_EV)            interrupt_signal_source(INTERRUPT_SOURCE_EWIS_FEUNEQP_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_FERDIP_EV)             interrupt_signal_source(INTERRUPT_SOURCE_EWIS_FERDIP_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_REIL_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_REIL_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_REIP_EV)               interrupt_signal_source(INTERRUPT_SOURCE_EWIS_REIP_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_B1_NZ_EV)              interrupt_signal_source(INTERRUPT_SOURCE_EWIS_B1_NZ_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_B2_NZ_EV)              interrupt_signal_source(INTERRUPT_SOURCE_EWIS_B2_NZ_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_B3_NZ_EV)              interrupt_signal_source(INTERRUPT_SOURCE_EWIS_B3_NZ_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_REIL_NZ_EV)            interrupt_signal_source(INTERRUPT_SOURCE_EWIS_REIL_NZ_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_REIP_NZ_EV)            interrupt_signal_source(INTERRUPT_SOURCE_EWIS_REIP_NZ_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_B1_THRESH_EV)          interrupt_signal_source(INTERRUPT_SOURCE_EWIS_B1_THRESH_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_B2_THRESH_EV)          interrupt_signal_source(INTERRUPT_SOURCE_EWIS_B2_THRESH_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_B3_THRESH_EV)          interrupt_signal_source(INTERRUPT_SOURCE_EWIS_B3_THRESH_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_REIL_THRESH_EV)        interrupt_signal_source(INTERRUPT_SOURCE_EWIS_REIL_THRESH_EV,port);
              if (phy10g_events & VTSS_PHY_EWIS_REIP_THRESH_EV)        interrupt_signal_source(INTERRUPT_SOURCE_EWIS_REIP_THRESH_EV,port);
#endif /* VTSS_FEATURE_WIS */

              if (phy10g_events & (VTSS_PHY_10G_LINK_LOS_EV | VTSS_PHY_10G_RX_LOL_EV | VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV | VTSS_PHY_10G_LOPC_EV
                                   | VTSS_PHY_10G_MODULE_STAT_EV | VTSS_PHY_10G_TX_LOL_EV | VTSS_PHY_10G_HIGH_BER_EV
#if defined(VTSS_FEATURE_WIS)
                                   | VTSS_PHY_EWIS_SEF_EV | VTSS_PHY_EWIS_FPLM_EV| VTSS_PHY_EWIS_FAIS_EV | VTSS_PHY_EWIS_LOF_EV  | VTSS_PHY_EWIS_RDIL_EV
                                   | VTSS_PHY_EWIS_AISL_EV | VTSS_PHY_EWIS_LCDP_EV | VTSS_PHY_EWIS_PLMP_EV | VTSS_PHY_EWIS_AISP_EV | VTSS_PHY_EWIS_LOPP_EV
                                   | VTSS_PHY_EWIS_UNEQP_EV | VTSS_PHY_EWIS_FEUNEQP_EV | VTSS_PHY_EWIS_FERDIP_EV | VTSS_PHY_EWIS_REIL_EV | VTSS_PHY_EWIS_REIP_EV
                                   | VTSS_PHY_EWIS_B1_NZ_EV | VTSS_PHY_EWIS_B2_NZ_EV | VTSS_PHY_EWIS_B3_NZ_EV | VTSS_PHY_EWIS_REIL_NZ_EV |VTSS_PHY_EWIS_REIP_NZ_EV
                                   | VTSS_PHY_EWIS_B1_THRESH_EV | VTSS_PHY_EWIS_B2_THRESH_EV | VTSS_PHY_EWIS_B3_THRESH_EV | VTSS_PHY_EWIS_REIL_THRESH_EV
                                   | VTSS_PHY_EWIS_REIP_THRESH_EV
#endif /* VTSS_FEATURE_WIS */
                                   ) )    *pending = TRUE;
          }

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
          vtss_phy_ts_event_t  ts_events;
          if (port_10g[i].feature_ts) {
            VTSS_RC(vtss_phy_ts_event_poll(PHY_INST, port, &ts_events));
            T_I("phy10g port %d ts_events:0x%X", iport2uport(port), ts_events);
            if (ts_events) {
                VTSS_RC(vtss_phy_ts_event_enable_set(PHY_INST, port, FALSE, ts_events));
                if (ts_events & VTSS_PHY_TS_INGR_ENGINE_ERR)         interrupt_signal_source(INTERRUPT_SOURCE_INGR_ENGINE_ERR, port);
                if (ts_events & VTSS_PHY_TS_INGR_RW_PREAM_ERR)       interrupt_signal_source(INTERRUPT_SOURCE_INGR_RW_PREAM_ERR, port);
                if (ts_events & VTSS_PHY_TS_INGR_RW_FCS_ERR)         interrupt_signal_source(INTERRUPT_SOURCE_INGR_RW_FCS_ERR, port);
                if (ts_events & VTSS_PHY_TS_EGR_ENGINE_ERR)          interrupt_signal_source(INTERRUPT_SOURCE_EGR_ENGINE_ERR, port);
                if (ts_events & VTSS_PHY_TS_EGR_RW_FCS_ERR)          interrupt_signal_source(INTERRUPT_SOURCE_EGR_RW_FCS_ERR, port);
                if (ts_events & VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED)  interrupt_signal_source(INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED, port);
                if (ts_events & VTSS_PHY_TS_EGR_FIFO_OVERFLOW)       interrupt_signal_source(INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW, port);
                if (ts_events & (VTSS_PHY_TS_INGR_ENGINE_ERR | VTSS_PHY_TS_INGR_RW_PREAM_ERR | VTSS_PHY_TS_INGR_RW_FCS_ERR |
                                 VTSS_PHY_TS_EGR_ENGINE_ERR | VTSS_PHY_TS_EGR_RW_FCS_ERR | VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED | VTSS_PHY_TS_EGR_FIFO_OVERFLOW))    *pending = TRUE;
            }
          }
#endif
      }
    }
  }
  return VTSS_RC_OK;
}

// Functino for handling interrupt for PCB107
// IN     - Flags   : Interrupt flags which is set.
// IN/OUT - Pending : Pointer to the pending flag.
static vtss_rc pcb107_events_handling(BOOL *pending, u32 flags, BOOL interrupt) {
  u32                i;
  BOOL               sgpio_events[VTSS_SGPIO_PORTS];
  vtss_sgpio_group_t group = 0;
  vtss_chip_no_t     chip_no = 0;
  u32                port_bit = 0; // All interrupt are in bit group 0, See schematic

  if (flags & INTERRUPT_SGPIO) {

    VTSS_RC(vtss_sgpio_event_poll(NULL, chip_no, group, port_bit, sgpio_events));
    for (i=15; i<23; i++) {
      if (sgpio_events[i]) {
        T_I("SGPIO interrupt sgpio_events[%d]:%d", i, sgpio_events[i]);

        // From UG1043, Table 13: p20b0 / p16b0 - Dedicated interrupt signal from VSC85x4 quad PHY (ports 21-24)
        if (i == 15) {
          VTSS_RC(phy1g_events_handling(0, 3, pending));
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
          VTSS_RC(phy1g_ts_events_handling(0, 3, pending));
#endif
        }
        // From UG1043, Table 13: P19b0 / p15b0 - Dedicated interrupt signal from VSC85x4 quad PHY (ports 17-20)
        if (i == 16) {
          VTSS_RC(phy1g_events_handling(4, 7, pending));
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
          VTSS_RC(phy1g_ts_events_handling(4, 7, pending));
#endif
        }
        // From UG1043, Table 13: P18b0 / p14b0 - Dedicated interrupt signal from VSC85x4 quad PHY (ports 13-16)
        if (i == 17) {
          VTSS_RC(phy1g_events_handling(8, 11, pending));
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
          VTSS_RC(phy1g_ts_events_handling(8, 11, pending));
#endif
        }
        // From UG1043, Table 13: P17b0 / p13b0 - Dedicated interrupt signal from VSC85x4 quad PHY (ports 9-12)
        if (i == 18) {
          VTSS_RC(phy1g_events_handling(12, 15, pending));
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
          VTSS_RC(phy1g_ts_events_handling(12, 15, pending));
#endif
        }
        // From UG1043, Table 13: P16b0 / p12b0 - Dedicated interrupt signal from VSC85x4 quad PHY (ports 5-8)
        if (i == 19) {
          VTSS_RC(phy1g_events_handling(16, 19, pending));
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
          VTSS_RC(phy1g_ts_events_handling(16, 19, pending));
#endif
        }
        // From UG1043, Table 13: P15b0 / p11b0 - Dedicated interrupt signal from VSC85x4 quad PHY (ports 1-4)
        if (i == 20) {
          VTSS_RC(phy1g_events_handling(20, 23, pending));
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
          VTSS_RC(phy1g_ts_events_handling(20, 23, pending));
#endif
        }
        // From UG1043, Table 13: p21b0 / p17b0 Dedicated interrupt signal from VSC8488-15/VSC8490-1x 10G PHY
        if (i == 21) {
          VTSS_RC(phy10g_events_handling(26, 27, pending));
        }

        // From UG1043, Table 13: p22b0 / p18b0 - Shared output from the NPI PHY, PTP CPLD, X2 module slots and Sync-E and PoE+ add-on boards. When an interrupt occurs, software must poll the possible sources for their current interrupt status e.g. through I2C, as it is otherwise not possible to distinguish which source has activated the shared interrupt signal.
        if (i == 22) {

            VTSS_RC(phy10g_events_handling(24, 25, pending));
#if defined(VTSS_SW_OPTION_SYNCE)
          VTSS_RC(synce_events_handling(pending));
#endif
#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
          zl_3034x_event_t zl_events;
          if((zl_3034x_event_poll(&zl_events) == VTSS_OK) && zl_events != 0) {
              T_DG(TRACE_GRP_IRQ1, "ZL event: %X", zl_events);
              (void) zl_3034x_event_enable_set(zl_events, FALSE);
              if (zl_events & ZL_TOP_ISR_REF_TS_ENG)
                  interrupt_signal_source(INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG, 0);
          }
#endif

#if defined(VTSS_SW_OPTION_POE)
          interrupt_poe_poll(interrupt, pending);    /* POE must be the last to poll as it need pending from PHY and CLOCK */
#endif
        }
      }
    }
  } else if (flags & INTERRUPT_PTP) {
    VTSS_RC(ptp_events_handling(pending));

  } else if (flags & INTERRUPT_DEV_ALL) {
    interrupt_dev_all_poll(VTSS_DEV_ALL_POLL_PRIMARY ,pending);
  } else if (flags) {
    // There are some flags that are not handled.
    return VTSS_RC_ERROR;
  }

  return VTSS_RC_OK;
}

void interrupt_device_poll(u32 flags, BOOL interrupt, BOOL onesec, BOOL *pending)
{
    vtss_rc rc;
    u32     i;
    BOOL    sgpio_events[VTSS_SGPIO_PORTS];

    T_D("Interrupt:%d, onesec:%d, flags:0x%X", interrupt, onesec, flags);

    phy_int_conf();
#if defined(VTSS_SW_OPTION_SYNCE)
    clock_event_type_t clock_events;
    if (onesec) {/* clock controller must be polled once a sec in order to detect state change to passive */
        clock_event_poll(FALSE, &clock_events);
        clock_event_signal(clock_events, pending);
    }
#endif

    if (board_type == VTSS_BOARD_JAG_PCB107_REF) {
        if ((rc = pcb107_events_handling(pending, flags, interrupt)) != VTSS_RC_OK) {
            T_E("%s", error_txt(rc));
        }
    } else {
        if (flags & INTERRUPT_EXT0) {
            if ((board_type == VTSS_BOARD_JAG_CU24_REF) || (board_type == VTSS_BOARD_JAG_CU48_REF)) {
                if ((rc = phy1g_events_handling(0, phy1g_cnt - 1, pending)) != VTSS_RC_OK) {
                    T_E("%s", error_txt(rc));
                }
            }
        }

        if (flags & INTERRUPT_EXT1) {
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
            if (feature_ts_1g_port_start < feature_ts_1g_port_end) {
                if ((rc = phy1g_ts_events_handling(feature_ts_1g_port_start, feature_ts_1g_port_end, pending)) != VTSS_RC_OK) {
                    T_E("%s", error_txt(rc));
                }
            }
#endif
            if (*pending == 0) {
                if ((rc =phy10g_events_handling(port10g_start, port10g_end, pending)) != VTSS_RC_OK) {
                    T_E("%s", error_txt(rc));
                }
            }

#if defined(VTSS_SW_OPTION_SYNCE)
            synce_events_handling(pending);
#endif
#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
            {
                zl_3034x_event_t zl_events;
                if ((zl_3034x_event_poll(&zl_events) == VTSS_OK) && zl_events != 0) {
                    T_DG(TRACE_GRP_IRQ1, "ZL event: %X", zl_events);
                    (void) zl_3034x_event_enable_set(zl_events, FALSE);
                    if (zl_events & ZL_TOP_ISR_REF_TS_ENG) {
                        interrupt_signal_source(INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG, 0);
                    }
                }
            }
#endif

#if defined(VTSS_SW_OPTION_POE)
            interrupt_poe_poll(interrupt, pending);    /* POE must be the last to poll as it need pending from PHY and CLOCK */
#endif
        }

        if (flags & INTERRUPT_SGPIO) {
            T_D("SGPIO interrupt flags:%d", flags);

            if (board_type == VTSS_BOARD_JAG_SFP24_REF) {
                if ((rc = vtss_sgpio_event_poll(NULL, 0, 0, 0, sgpio_events)) != VTSS_RC_OK) {
                    T_E("%s", error_txt(rc));
                }

                for (i = 0; i < 23; i++) {
                    if (sgpio_events[i]) {
                        *pending = TRUE;

                        if ((rc = vtss_sgpio_event_enable(NULL, 0, 0, i, 0, FALSE)) != VTSS_RC_OK) {
                            T_E("%s", error_txt(rc));
                        }

                        interrupt_signal_source(INTERRUPT_SOURCE_LOS, i);
                    }
                }
            }
        }

        if (flags & INTERRUPT_DEV_ALL) {
            // interrupt_tx_timestamp_poll(interrupt, pending);
            interrupt_dev_all_poll(VTSS_DEV_ALL_POLL_PRIMARY ,pending);
        }

        if (flags & INTERRUPT_SEC_DEV_ALL) {
            // interrupt_tx_timestamp_poll(interrupt, pending);
            interrupt_dev_all_poll(VTSS_DEV_ALL_POLL_SECONDARY ,pending);
        }

        if (flags & INTERRUPT_PTP) {
            if ((rc = ptp_events_handling(pending)) != VTSS_RC_OK) {
                T_E("%s", error_txt(rc));
            }
        }
    }
}

/****************************************************************************/
/*  Various local definitions                                               */
/****************************************************************************/
static cyg_uint32 ext0_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmasked
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void ext0_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_EXT0);
}

static cyg_uint32 ext1_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmasked
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void ext1_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_EXT1);
}

static cyg_uint32 sgpio_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmasked
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void sgpio_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_SGPIO);
}

static cyg_uint32 dev_all_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmasked
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void dev_all_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_DEV_ALL);
}


#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
static cyg_uint32 dev_all_sec_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmasked
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}


static void dev_all_sec_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_device_signal(INTERRUPT_SEC_DEV_ALL);
}
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */

static cyg_uint32 ptp_sync_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until unmasked
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
    vtss_phy_10g_event_t  phy_events;

    /* Now POE is done initializing. We need to wait for that before enabling EXT0 because some POE devices need to be cofigured in order not to force this active at all time */

    /* This is ugly - I will now clear pending on X2 PHY connected to EXT otherwise events (Fast Link for protection) will be generated long after they actually happened */
    for (i=0; i<PORT_10G_MAX; i++) {   // Polling of solded 8486 and 8484+87+88 in X2 slot
        if (port_10g[i].phy) {
            (void)vtss_phy_10g_event_poll(PHY_INST, port_10g[i].port, &phy_events);
        }
    }

    /* Enable EXT1 interrupt */
    cyg_drv_interrupt_unmask(ext_irq);
}
#endif


/* Initialize module */
void interrupt_board_init(void)
{
    u32                       rc;
    vtss_port_no_t            port_no;
    int                       idx;

    board_type = vtss_board_type();


    /* Find any 10G capability - with or without PHY. Now also count the number of 1G PHY */
    for (port_no=VTSS_PORT_NO_START, idx=0; port_no<VTSS_PORT_NO_END; port_no++) {
        T_I("idx:%d", idx);
        if (idx < PORT_10G_MAX) {
            if (port_custom_table[port_no].cap & PORT_CAP_VTSS_10G_PHY) {
                port_10g[idx].port = port_no;
                port_10g[idx].phy = TRUE;
                if (port10g_start == VTSS_PORT_NO_NONE) {
                    port10g_start = port_no;
                }
                port10g_end = port_no;
                T_I("port_10g[%d].phy:%d, uport %d", idx, port_10g[idx].phy, iport2uport(port_10g[idx].port));
                idx++;
            }
            else
            if (port_custom_table[port_no].cap & PORT_CAP_10G_FDX) {
                port_10g[idx].port = port_no;
                port_10g[idx].phy = FALSE;
                T_I("port_10g[%d].phy:%d, uport %d", idx, port_10g[idx].phy, iport2uport(port_10g[idx].port));
                idx++;
            }
            else {
                T_I("uport %d, cap %x", iport2uport(port_no), port_custom_table[port_no].cap);
            }
        }
        if (port_phy(port_no)) {
            if (port_custom_table[port_no].cap == 0) {continue;}
            port_1g[phy1g_cnt].port = port_no;
            port_1g[phy1g_cnt].phy = TRUE;
            phy1g_cnt++;
        }
    }
    if (phy1g_cnt)  phy1g_cnt--;    /* Do not count the management 1G PHY */
    T_W("phy1g_cnt:%d", phy1g_cnt);
    T_W("port10g_start: %d, port10g_end: %d", port10g_start, port10g_end);

    if (board_type != VTSS_BOARD_JAG_PCB107_REF) { // PCB107 doesn't support ext0 and ext1
      /* Handling EXT0 interrupt */
      cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EXT0,      /* Interrupt Vector */
                               0,                              /* Interrupt Priority */
                               (cyg_addrword_t)NULL,
                               ext0_isr,
                               ext0_dsr,
                               &ext0_int_handle,
                               &ext0_int_object);

      rc = vtss_gpio_mode_set(NULL, 0, 6, VTSS_GPIO_ALT_0);

      cyg_drv_interrupt_attach(ext0_int_handle);

      /* Enable EXT0 interrupt */
      cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);

      rc = vtss_gpio_mode_set(NULL, 0, 7, VTSS_GPIO_ALT_0); /* Configure EXT1 as IRQ */
#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
      if (board_type == VTSS_BOARD_JAG_CU48_REF) {
        rc = vtss_gpio_mode_set(NULL, 1, 7, VTSS_GPIO_ALT_0);/* Configure Slave EXT1 as IRQ */
        ext_irq = CYGNUM_HAL_INTERRUPT_SEC_EXT1;
      } else {
        ext_irq = CYGNUM_HAL_INTERRUPT_EXT1;
      }
#else
      ext_irq = CYGNUM_HAL_INTERRUPT_EXT1;
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */

      /* Handling EXT1 interrupt */
      cyg_drv_interrupt_create(ext_irq, /* Interrupt Vector */
                               0,       /* Interrupt Priority */
                               (cyg_addrword_t)NULL,
                               ext1_isr,
                               ext1_dsr,
                               &ext1_int_handle,
                               &ext1_int_object);

      cyg_drv_interrupt_attach(ext1_int_handle);

#if !defined(VTSS_SW_OPTION_POE)
      /* Enable EXT1 interrupt */
      cyg_drv_interrupt_unmask(ext_irq);
#endif
    }
    /* Handling SGPIO interrupt */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_SGPIO,      /* Interrupt Vector */
                             0,                              /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             sgpio_isr,
                             sgpio_dsr,
                             &sgpio_int_handle,
                             &sgpio_int_object);

    cyg_drv_interrupt_attach(sgpio_int_handle);

    /* Enable SGPIO interrupt */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SGPIO);

    /* Handling intern interrupt - DEV_ALL */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_DEV_ALL,      /* Interrupt Vector */
                             0,                        /* Interrupt Priority */
                             (cyg_addrword_t)NULL,
                             dev_all_isr,
                             dev_all_dsr,
                             &dev_all_int_handle,
                             &dev_all_int_object);

    cyg_drv_interrupt_attach(dev_all_int_handle);

    /* Enable DEV_ALL interrupts */
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_DEV_ALL);

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
    if (board_type == VTSS_BOARD_JAG_CU48_REF) {
        /* Handling Secodary intern interrupt - DEV_ALL */
        cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_SEC_DEV_ALL,      /* Interrupt Vector */
                                 0,                        /* Interrupt Priority */
                                 (cyg_addrword_t)NULL,
                                 dev_all_sec_isr,
                                 dev_all_sec_dsr,
                                 &dev_all_sec_int_handle,
                                 &dev_all_sec_int_object);

        cyg_drv_interrupt_attach(dev_all_sec_int_handle);

        /* Enable DEV_ALL interrupts */
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SEC_DEV_ALL);
    }
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */

    /* Handling intern interrupt - PTP_SYNC */
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_PTP_SYNC,      /* Interrupt Vector */
                             0,                        /* Interrupt Priority */
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
