/*

 Vitesse Switch API software.

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

#ifndef _PTP_API_H_
#define _PTP_API_H_

#include "main.h"
#include "vtss_os.h"
#include "vtss_ptp_api.h"

#define PTP_CLOCK_INSTANCES   4   /* Number og clock instances in a node (with different domain numbers) */
#define PTP_MAX_INSTANCES_PR_PORT 2 /* Number og clock instances that are allowed to enable the same port */
vtss_rc ptp_init(vtss_init_data_t *data);

BOOL
ptp_clock_create(const ptp_init_clock_ds_t *initData, uint instance);

BOOL
ptp_clock_delete(uint instance);

vtss_rc
ptp_port_ena(BOOL internal, uint portnum, uint instance);

BOOL
ptp_port_dis( uint portnum, uint instance);

BOOL
ptp_get_clock_default_ds(ptp_clock_default_ds_t *default_ds, uint instance);

BOOL
ptp_get_clock_set_default_ds(ptp_set_clock_ds_t *default_ds, uint instance);

void
ptp_get_default_clock_default_ds(ptp_clock_default_ds_t *default_ds);

BOOL
ptp_set_clock_default_ds(const ptp_set_clock_ds_t *default_ds, uint instance);

BOOL
ptp_get_clock_current_ds(ptp_clock_current_ds_t *status, uint instance);

BOOL
ptp_get_clock_parent_ds(ptp_clock_parent_ds_t *status, uint instance);

BOOL
ptp_get_clock_timeproperties_ds(ptp_clock_timeproperties_ds_t *timeproperties_ds, uint instance);

BOOL
ptp_get_clock_cfg_timeproperties_ds(ptp_clock_timeproperties_ds_t *timeproperties_ds, uint instance);

void
ptp_get_clock_default_timeproperties_ds(ptp_clock_timeproperties_ds_t *timeproperties_ds);

BOOL
ptp_set_clock_timeproperties_ds(const ptp_clock_timeproperties_ds_t *timeproperties_ds, uint instance);

BOOL
ptp_get_port_ds(ptp_port_ds_t *ds, int portnum, uint instance);

BOOL
ptp_set_port_ds(uint portnum,
                const ptp_set_port_ds_t *port_ds, uint instance);
BOOL
ptp_get_port_cfg_ds(uint portnum,
                ptp_set_port_ds_t *port_ds, uint instance);

BOOL
ptp_get_port_foreign_ds(ptp_foreign_ds_t *f_ds, int portnum, i16 ix, uint instance);


/**
 * \brief Set filter parameters for a Default PTP filter instance.
 *
 * \param c [IN]  pointer to a structure containing the new parameters for
 *                the filter
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_default_filter_parameters_set(const vtss_ptp_default_filter_config_t *c, uint instance);


/**
 * \brief Set filter parameters for a Default PTP filter instance.
 *
 * \param c [OUT]  pointer to a structure containing the new parameters for
 *                the filter
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_default_filter_parameters_get(vtss_ptp_default_filter_config_t *c, uint instance);

/**
 * \brief Get default filter parameters for a Default PTP filter.
 *
 */
void
ptp_default_filter_default_parameters_get(vtss_ptp_default_filter_config_t *c);
/**
 * \brief Set filter parameters for a Default PTP servo instance.
 *
 * \param c [IN]  pointer to a structure containing the new parameters for
 *                the servo
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_default_servo_parameters_set(const vtss_ptp_default_servo_config_t *c, uint instance);

/**
 * \brief Get filter parameters for a Default PTP servo instance.
 *
 * \param c [OUT]  pointer to a structure containing the new parameters for
 *                the servo
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_default_servo_parameters_get(vtss_ptp_default_servo_config_t *c, uint instance);

void
ptp_default_servo_default_parameters_get(vtss_ptp_default_servo_config_t *c);

/**
 * \brief Get servo status parameters for a Default PTP servo instance.
 *
 * \param s [OUT]  pointer to a structure containing the status for
 *                the servo
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_default_servo_status_get(vtss_ptp_servo_status_t *s, uint instance);

/**
 * \brief Set filter parameters for a Default PTP filter instance.
 *
 * \param c [IN]  pointer to a structure containing the new parameters for
 *                the filter
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_default_delay_filter_parameters_set(const vtss_ptp_default_delay_filter_config_t *c, uint instance);

/**
 * \brief Set filter parameters for a Default PTP filter instance.
 *
 * \param c [OUT]  pointer to a structure containing the new parameters for
 *                the filter
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_default_delay_filter_parameters_get(vtss_ptp_default_delay_filter_config_t *c, uint instance);

/**
 * \brief Get default filter parameters for a Default PTP delay filter.
 *
 */
void
ptp_default_delay_filter_default_parameters_get(vtss_ptp_default_delay_filter_config_t *c);

/**
 * \brief Get slave status parameters.
 *
 * \param s [OUT]  pointer to a structure containing the status for
 *                the slave
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_get_clock_slave_ds(ptp_clock_slave_ds_t *status, uint instance);


/**
 * \brief Set unicast slave configuration parameters.
 *
 * \param c [IN]  pointer to a structure containing the new parameters for
 *                the slave config
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_uni_slave_conf_set(const vtss_ptp_unicast_slave_config_t *c, uint index, uint instance);

/**
 * \brief Get unicast slave configuration and state parameters.
 *
 * \param c [IN]  pointer to a structure containing the new parameters for
 *                the slave config
 * \param ix [IN] Slave index
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_uni_slave_conf_state_get(vtss_ptp_unicast_slave_config_state_t *c, uint ix, uint instance);



/**
 * \brief Get unicast slave table parameters.
 * \param c [OUT]  pointer to a structure containing the table for
 *                the slave-master communication.
 * \param ix[IN]  index in the table
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_clock_unicast_table_get(vtss_ptp_unicast_slave_table_t *uni_slave_table, int ix, uint instance);

/**
 * \brief Get unicast master table data.
 * \param c [OUT]  pointer to a structure containing the table for
 *                the master-slave communication.
 * \param ix[IN]  index in the table
 * \param instance clock instance number.
 * \return TRUE if success
 */
BOOL
ptp_clock_unicast_master_table_get(vtss_ptp_unicast_master_table_t *uni_master_table, int ix, uint instance);


/* latency observed in onestep tx timestamping */
typedef struct observed_egr_lat_t {
    vtss_timeinterval_t max;
    vtss_timeinterval_t min;
    vtss_timeinterval_t mean;
    u32 cnt;
} observed_egr_lat_t;

/**
 * \brief Get observed egress latency.
 * \param c [OUT]  pointer to a structure containing the latency.
 * \return void
 */
void
ptp_clock_egress_latency_get(observed_egr_lat_t *lat);

/**
 * \brief Clear observed egress latency.
 * \return void
 */
void
ptp_clock_egress_latency_clear(void);


typedef enum  {
    VTSS_PTP_ONE_PPS_DISABLE, /* 1 pps not used */
    VTSS_PTP_ONE_PPS_OUTPUT,  /* 1 pps output */
    VTSS_PTP_ONE_PPS_INPUT,   /* 1 pps input */
} ptp_ext_clock_1pps_t;


/* external clock output configuration */
typedef struct vtss_ptp_ext_clock_mode_t {
    ptp_ext_clock_1pps_t   one_pps_mode;    /* Select 1pps mode:
                                input : lock clock to 1pps input
                                output: enable external sync pulse output
                                disable: disable 1 pps */
    BOOL clock_out_enable;  /* Enable programmable clock output 
                                clock frequency = 'freq' */
    BOOL vcxo_enable;       /* Enable use of external VCXO for rate adjustment */
    u32  freq;              /* clock output frequency (hz [1..25.000.000]). */
} vtss_ptp_ext_clock_mode_t;

/**
 * \brief Get external clock output configuration.
 * \param mode [OUT]  pointer to a structure containing the configuration.
 * \return void
 */
void
vtss_ext_clock_out_get(vtss_ptp_ext_clock_mode_t *mode);

void vtss_ext_clock_out_default_get(vtss_ptp_ext_clock_mode_t *mode);

/**
 * \brief Set external clock output configuration.
 * \param mode [IN]    pointer to a structure containing the configuration.
 * \return void
 */
void
vtss_ext_clock_out_set(const vtss_ptp_ext_clock_mode_t *mode);

#if defined (VTSS_ARCH_SERVAL)

typedef enum  {
    VTSS_PTP_RS422_DISABLE,     /* RS422 mode not used */
    VTSS_PTP_RS422_MAIN_AUTO,   /* RS422 main auto mode i.e 1 pps master */
    VTSS_PTP_RS422_SUB,         /* RS422 sub mode i.e 1 pps slave */
    VTSS_PTP_RS422_MAIN_MAN,    /* RS422 main man mode i.e 1 pps master */
} ptp_rs422_mode_t;

typedef enum  {
    VTSS_PTP_RS422_PROTOCOL_SER, /* use serial protocol */
    VTSS_PTP_RS422_PROTOCOL_PIM, /* use PIM protocol */
} ptp_rs422_protocol_t;


/* RS422 PTP configuration */
typedef struct vtss_ptp_rs422_conf_t {
    ptp_rs422_mode_t     mode;   /* Select rs422 mode:
                                    DISABLE : rs422 not in use
                                    MAIN: main module function
                                    SUB: sub module function */
    u32                  delay;  /* in MAIN mode: read only, measured turn around delay in ns.
                                    in SUB mode: reload value used to compensate for path delay (in ns) */
    ptp_rs422_protocol_t proto;  /* Selected protocol */
    vtss_port_no_t       port;   /* Switch port used for the PIM protocol */
} vtss_ptp_rs422_conf_t;

/**
 * \brief Get serval rs424 external clock configuration.
 * \param mode [OUT]  pointer to a structure containing the configuration.
 * \return void
 */
void vtss_ext_clock_rs422_conf_get(vtss_ptp_rs422_conf_t *mode);

void vtss_ext_clock_rs422_default_conf_get(vtss_ptp_rs422_conf_t *mode);

/**
 * \brief Set serval rs422 external clock configuration.
 * \param mode [IN]  pointer to a structure containing the configuration.
 * \return void
 */
void vtss_ext_clock_rs422_conf_set(const vtss_ptp_rs422_conf_t *mode);

/**
 * \brief Set serval rs422 time at next 1PPS.
 * \param t [IN]  time at nexr 1PPS.
 * \return void
 */
void vtss_ext_clock_rs422_time_set(const vtss_timestamp_t *t);

#endif


typedef struct vtss_ptp_port_link_state_t {
    BOOL link_state;        /* TRUE if link is up */
    BOOL in_sync_state;     /* TRUE if port local timer is in sync (only relevant for ports with PHY timestaming */
    BOOL forw_state;        /* TRUE if port filter does not indicate 'discard'  */
    BOOL phy_timestamper;   /* TRUE if port uses phy timestamp feature */
} vtss_ptp_port_link_state_t;

/**
 * \brief Get port state.
 * \param ds [OUT]  pointer to a structure containing the port status.
 * \return TRUE if valid port number
 */
BOOL
ptp_get_port_link_state(vtss_ptp_port_link_state_t *ds, int portnum, uint instance);

/**
 * \brief Get debug mode.
 * \param debug_mode [OUT]  pointer to debug mode.
 * \return TRUE if OK
 */
BOOL ptp_debug_mode_get(int *debug_mode, uint instance);

BOOL ptp_debug_mode_set(int debug_mode, uint instance);

/**
* \brief Execute a one-pps external clock input action.
* \param action [IN]  action:   [0]    Dump statistics
                                [1]    Clear statistics
                                [2]    Enable offset logging
                                [3]    Disable offset logging
* \return void
*/

typedef struct vtss_ptp_one_pps_statistic_t {
    i32 min;
    i32 max;
    i32 mean;
    BOOL dLos;
} vtss_ptp_one_pps_statistic_t;

void vtss_one_pps_external_input_statistic_get(vtss_ptp_one_pps_statistic_t *one_pps, BOOL clear);


/**
 * \brief Enable/disable the wireless variable tx delay feature for a port.
 * \param enable [IN]    TRUE => enable vireless, FALSE => disable vireles feature.
 * \param portnum [IN]   ptp port number.
 * \param instance [IN]  ptp instance number.
 * \return TRUE if success, false if invalid portnum or instance.
 */
BOOL ptp_port_wireless_delay_mode_set(BOOL enable, int portnum, uint instance);
                
/**
 * \brief Get the Enable/disable mode for a port.
 * \param enable [OUT]   TRUE => wireless enabled, FALSE => wireles disabled.
 * \param portnum [IN]   ptp port number.
 * \param instance [IN]  ptp instance number.
 * \return TRUE if success, false if invalid portnum or instance.
 */
BOOL ptp_port_wireless_delay_mode_get(BOOL *enable, int portnum, uint instance);
                
/**
 * \brief Pre notification sent from the wireless modem transmitter before the delay is changed.
 * \param portnum [IN]  ptp port number
 * \param instance [IN]  ptp instance number.
 * \return TRUE if success, false in invalid portnum or instance
 */
BOOL ptp_port_wireless_delay_pre_notif(int portnum, uint instance);
                
/**
 * \brief Set the delay configuration, sent from the wireless modem transmitter whenever the delay is changed.
 * \Note: the wireless delay for a packet equals: base_delay + packet_length*incr_delay.
 * \param delay_cfg [IN]  delay configuration.
 * \param portnum [IN]  ptp port number.
 * \param instance [IN]  ptp instance number.
 * \return TRUE is success, false in invalid portnum or instance
 */
BOOL ptp_port_wireless_delay_set(const vtss_ptp_delay_cfg_t *delay_cfg, int portnum, uint instance);

/**
 * \brief Get the delay configuration.
 * \param delay_cfg [OUT]  delay configuration. 
 * \param portnum [IN]  ptp port number.
 * \param instance [IN]  ptp instance number.
 * \return TRUE is success, false in invalid portnum or instance
 */
BOOL ptp_port_wireless_delay_get(vtss_ptp_delay_cfg_t *delay_cfg, int portnum, uint instance);

/**
 * \brief Get the slave configuration parameters.
 * \param cfg [OUT]  slave configuration. 
 * \param instance [IN]  ptp instance number.
 * \return TRUE is success, false in invalid portnum or instance
 */
BOOL ptp_get_clock_slave_config(ptp_clock_slave_cfg_t *cfg, uint instance);

/**
 * \brief Set the slave configuration parameters.
 * \param cfg [IN]  slave configuration. 
 * \param instance [IN]  ptp instance number.
 * \return TRUE is success, false in invalid portnum or instance
 */
BOOL ptp_set_clock_slave_config(const ptp_clock_slave_cfg_t *cfg, uint instance);

void ptp_get_default_clock_slave_config(ptp_clock_slave_cfg_t *cfg);

/**
 * \brief Set the external PDV algorithm enable/disable.
 * \param enable [IN]  enable external PDV. 
 * \return VTSS_RC_OK
 */
vtss_rc ptp_set_ext_pdv_config(BOOL enable);
vtss_rc ptp_get_ext_pdv_config(BOOL *enable);

void ptp_1pps_ptp_slave_t1_t2_rx(vtss_port_no_t port_no, vtss_ptp_timestamps_t *ts);


char *ptp_error_txt(vtss_rc rc); // Convert Error code to text

#endif // _PTP_API_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
