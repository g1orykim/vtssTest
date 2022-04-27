/*

 Vitesse Clock API software.

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
 
 $Id$
 $Revision$

*/

#ifndef _CLOCK_API_H_
#define _CLOCK_API_H_

/* Get common definitions, types and macros */
#include "vtss_os.h"
#include "vtss_types.h"
#include "vtss_options.h"
#include "main.h"
#define CLOCK_INPUT_MAX 3
extern uint clock_my_input_max;
#define STATION_CLOCK_SOURCE_NO 2   /* source used to nominate the station clock input */
extern uint synce_my_prio_disabled;

/******************************************************************************
 * Description: Initialize Clock API.
 *
 * \param setup (input)
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc clock_init(BOOL  cold_init);



/******************************************************************************
 * Description: Startup the Clock Controller.
 *
 * \param setup (input)
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc clock_startup(BOOL  cold_init,
                      BOOL  pcb104_synce);



/******************************************************************************
 * Description: Set Clock selection mode
 *
 * \param mode (input)              : Mode of clock selection.
 * \param clock_input (input)   : Clock input in manual mode.
 *
 * \return : Return code.
 ******************************************************************************/
typedef enum
{
    CLOCK_SELECTION_MODE_MANUEL,
    CLOCK_SELECTION_MODE_AUTOMATIC_NONREVERTIVE,
    CLOCK_SELECTION_MODE_AUTOMATIC_REVERTIVE,
    CLOCK_SELECTION_MODE_FORCED_HOLDOVER,
    CLOCK_SELECTION_MODE_FORCED_FREE_RUN
} clock_selection_mode_t;

vtss_rc   clock_selection_mode_set(const clock_selection_mode_t   mode,
                                   const uint                     clock_input);


/******************************************************************************
 * Description: get Clock selector state
 *
 * \param selector_state (output)    : selector state
 * \param clock_input (input)          : Clock input port number - if in locked state
 *
 * \return : Return code.
 ******************************************************************************/
typedef enum
{
    CLOCK_SELECTOR_STATE_LOCKED,
    CLOCK_SELECTOR_STATE_HOLDOVER,
    CLOCK_SELECTOR_STATE_FREERUN,
} clock_selector_state_t;

vtss_rc   clock_selector_state_get(clock_selector_state_t  *const selector_state,
                                   uint                    *const clock_input);     /* clock_input range is 0 - CLOCK_INPUT_MAX */



/******************************************************************************
 * Description: Set Clock frequency
 *
 * \param clock_input (input)   : Clock input port number
 * \param frequency (input)     : Frequency for this clock input.
 *
 * \return : Return code.
 ******************************************************************************/
typedef enum
{
    CLOCK_FREQ_125MHZ,
    CLOCK_FREQ_10MHZ,
    CLOCK_FREQ_INVALID
} clock_frequency_t;

vtss_rc   clock_frequency_set(const uint                 clock_input,
                              const clock_frequency_t    frequency);


/******************************************************************************
 * Description: Set Clock input port priority
 *
 * \param clock_input (input)   : Clock input port number
 * \param priority (input)      : Priority - 0 is highest
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc   clock_priority_set(const uint   clock_input,     /* clock_input range is 0 - CLOCK_INPUT_MAX */
                             const uint   priority);



/******************************************************************************
 * Description: Get Clock input port priority
 *
 * \param clock_input (input)   : Clock input port number
 * \param priority (output)     : Priority - 0 is highest
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc   clock_priority_get(const uint   clock_input,     /* clock_input range is 0 - CLOCK_INPUT_MAX */
                             uint         *const priority);

/******************************************************************************
 * Description: Set Clock hold off time
 *
 * \param clock_input (input)   : Clock input port number
 * \param holdoff (input)       : Zero is no holdoff. Hold Off time must be between 300 and 1800 ms.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc   clock_holdoff_time_set(const uint   clock_input,     /* clock_input range is 0 - CLOCK_INPUT_MAX */
                                 const uint   ho_time);



/******************************************************************************
 * Description: Clock hold off timer run
 *
 * \param active (output)       : TRUE means some clock source hold off timer still need to run
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc   clock_holdoff_run(BOOL *const active);



/******************************************************************************
 * Description: Clock hold off event - activating control of LOCS hold off timing
 *
 * \param clock_input (input)   : Clock input port number
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc   clock_holdoff_event(const uint   clock_input);     /* clock_input range is 0 - CLOCK_INPUT_MAX */



/******************************************************************************
 * Description: Clock hold off timer active get
 *
 * \param clock_input (input)   : Clock input port number
 * \param active (output)       : TRUE means this clock source hold off timer is still active
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc   clock_holdoff_active_get(const uint   clock_input,     /* clock_input range is 0 - CLOCK_INPUT_MAX */
                                   BOOL         *const active);

/******************************************************************************
 * Description: get LOCS state
 *
 * \param clock_input (input)      : clock input port number
 * \param state (output)           : LOCS state
 *
 * \return : Return code.
 ******************************************************************************/

vtss_rc   clock_locs_state_get(const uint   clock_input,     /* clock_input range is 0 - SYNCE_CLOCK_MAX */
                               BOOL         *const state);



/******************************************************************************
 * Description: get FOS state
 *
 * \param clock_input (input)       : clock input port number
 * \param state (output)                : FOS state
 *
 * \return : Return code.
 ******************************************************************************/

vtss_rc   clock_fos_state_get(const uint   clock_input,
                              BOOL         *const state);



/******************************************************************************
 * Description: get LOSX state
 *
 * \param state (output)     : LOSX state
 *
 * \return : Return code.
 ******************************************************************************/

vtss_rc   clock_losx_state_get(BOOL         *const state);



/******************************************************************************
 * Description: get LOL state
 *
 * \param state (output)     : LOL state
 *
 * \return : Return code.
 ******************************************************************************/

vtss_rc   clock_lol_state_get(BOOL         *const state);



/******************************************************************************
 * Description: get Digital Hold Valid state
 *
 * \param state (output)     : DHOLD state
 *
 * \return : Return code.
 ******************************************************************************/

vtss_rc   clock_dhold_state_get(BOOL         *const state);



#define CLOCK_LOSX_EV   0x00000001
#define CLOCK_LOL_EV    0x00000002
#define CLOCK_LOCS1_EV  0x00000004
#define CLOCK_LOCS2_EV  0x00000008
#define CLOCK_FOS1_EV   0x00000010
#define CLOCK_FOS2_EV   0x00000020
typedef u32 clock_event_type_t;
void clock_event_poll(BOOL interrupt,  clock_event_type_t *ev_mask);
void clock_event_enable(clock_event_type_t ev_mask);


/******************************************************************************
 * Description: Set station clock output frequency
 *
 * \param freq_khz (IN)     : frequency in KHz, the frequency is rounded to the closest multiple og 8 KHz.
 *                                              freq_khz < 8 => clockoutput is disabled
 * \return : Return code.
 ******************************************************************************/
vtss_rc clock_station_clk_out_freq_set(const u32 freq_khz);

/******************************************************************************
 * Description: Set station clock input frequency
 *
 * \param freq_khz (IN)     : frequency in KHz, the frequency is rounded to the closest multiple og 8 KHz.
 *                                              freq_khz < 8 => clockinput is disabled
 * \return : Return code.
 ******************************************************************************/
vtss_rc clock_station_clk_in_freq_set(const u32 freq_khz);

/******************************************************************************
 * Description: Set station clock input frequency
 *
 * \param clock_type (OUT)     : 0 = Full featured clock type, i.e. supports both in and out, and 1,544, 2,048 and 10 MHz
 *                             : 1 = PCB104, support only 2,048 and 10 MHz clock output
 *                             : 2 = others, no station clock support
 * \return : Return code.
 ******************************************************************************/
vtss_rc clock_station_clock_type_get(uint *const clock_type);


/******************************************************************************
 * Description: Read value from Clock register.
 *
 * \param reg (input)      : Clock register address.
 * \param value (output) : Register value.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc clock_read(const uint  reg,
                   uint        *const value);



/******************************************************************************
 * Description: Write value to Clock register.
 *
 * \param reg (input)     : Clock register address.
 * \param value (input)  : Register value.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc clock_write(const uint    reg,
                    const uint    value);



/******************************************************************************
 * Description: Read, modify and write value to Clock register.
 *
 * \param reg (input)     : Clock register address.
 * \param value  (input) : Register value.
 * \param mask (input)   : Register mask, only bits enabled are changed.
 *
 * \return : Return code.
 ******************************************************************************/
vtss_rc clock_writemasked(const uint     reg,
                          const uint     value,
                          const uint     mask);


/******************************************************************************
 * Description: Set clock EEC option.
 *
 * \eec_option (input)   : Clock EEC option.
 *
 * \return : Return code.
 ******************************************************************************/
typedef enum
{
    CLOCK_EEC_OPTION_1,    /* EEC option 1: See ITU-T G.8262/Y.1362 */
    CLOCK_EEC_OPTION_2     /* EEC option 2  */
} clock_eec_option_t;

vtss_rc clock_eec_option_set(const clock_eec_option_t clock_eec_option);



#endif /* _CLOCK_API_H_ */
