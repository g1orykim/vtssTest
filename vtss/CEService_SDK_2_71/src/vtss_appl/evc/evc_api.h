/*

 Vitesse Switch API software.

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

 $Id$
 $Revision$

*/

#ifndef _EVC_API_H_
#define _EVC_API_H_

/* Number of EVCs */
#ifndef EVC_ID_COUNT
#define EVC_ID_COUNT  VTSS_EVCS
#endif /* EVC_ID_COUNT */

/* Number of ECEs */
#ifndef EVC_ECE_COUNT
#define EVC_ECE_COUNT VTSS_EVCS
#endif /* EVC_ECE_COUNT */

/* Number of policers */
#ifndef EVC_POL_COUNT
#define EVC_POL_COUNT VTSS_EVC_POLICERS
#endif /* EVC_POL_COUNT */

/* ================================================================= *
 *  Management API
 * ================================================================= */

/* - Global Configuration ------------------------------------------ */

/* EVC global configuration */
typedef struct {
    BOOL port_check; /* Enable UNI/NNI port check */
} evc_mgmt_global_conf_t;

/* Get global configuration */
vtss_rc evc_mgmt_conf_get(evc_mgmt_global_conf_t *conf);

/* Set global configuration */
vtss_rc evc_mgmt_conf_set(evc_mgmt_global_conf_t *conf);

/* - Port Configuration -------------------------------------------- */

/* EVC port configuration using VTSS API structures */
typedef struct {
    vtss_evc_port_conf_t       conf;
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_port_conf_t      vcap_conf;
#endif /* VTSS_ARCH_SERVAL */
    vtss_packet_rx_port_conf_t reg;
} evc_mgmt_port_conf_t;

/* Get port configuration */
vtss_rc evc_mgmt_port_conf_get(vtss_port_no_t port_no, evc_mgmt_port_conf_t *conf);

/* Get port default */
void evc_mgmt_port_conf_get_default(evc_mgmt_port_conf_t *conf);

/* Set port configuration */
vtss_rc evc_mgmt_port_conf_set(vtss_port_no_t port_no, evc_mgmt_port_conf_t *conf);

/* - Policer Configuration ----------------------------------------- */

/* EVC policer configuration using the VTSS API structure */
typedef struct {
    vtss_evc_policer_conf_t conf;
} evc_mgmt_policer_conf_t;

#define EVC_POLICER_RATE_MAX  10000000 /* CIR/EIR maximum 10 Gbps */
#define EVC_POLICER_LEVEL_MAX 100000   /* CBS/EBS maximum 100 kB */

/* Get policer configuration */
vtss_rc evc_mgmt_policer_conf_get(vtss_evc_policer_id_t policer_id,
                                  evc_mgmt_policer_conf_t *conf);

/* Get policer default */
void evc_mgmt_policer_conf_get_default(evc_mgmt_policer_conf_t *conf);

/* Set port configuration */
vtss_rc evc_mgmt_policer_conf_set(vtss_evc_policer_id_t policer_id,
                                  evc_mgmt_policer_conf_t *conf);

/* - EVC Configuration --------------------------------------------- */

/* EVC minimum VID */
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MPLS)
#define EVC_VID_MIN 0 /* VID zero means no tagging on NNI port */
#else
#define EVC_VID_MIN 1
#endif

/* EVC entry configuration using the VTSS API structure */
typedef struct {
    vtss_evc_conf_t conf;
    BOOL            conflict; /* VTSS API failure indication */
} evc_mgmt_conf_t;

/* Add EVC */
vtss_rc evc_mgmt_add(vtss_evc_id_t evc_id, evc_mgmt_conf_t *conf);

/* Delete EVC */
vtss_rc evc_mgmt_del(vtss_evc_id_t evc_id);

/* Get EVC or get next EVC */
#define EVC_ID_FIRST VTSS_EVC_ID_NONE
vtss_rc evc_mgmt_get(vtss_evc_id_t *evc_id, evc_mgmt_conf_t *conf, BOOL next);

/* Get EVC default configuration */
void evc_mgmt_get_default(evc_mgmt_conf_t *conf);

/* - ECE Configuration --------------------------------------------- */

#if defined(VTSS_ARCH_SERVAL)
typedef enum {
    EVC_L2CP_NONE,
    EVC_L2CP_STP,
    EVC_L2CP_PAUSE,
    EVC_L2CP_LACP,
    EVC_L2CP_LAMP,
    EVC_L2CP_LOAM,
    EVC_L2CP_DOT1X,
    EVC_L2CP_ELMI,
    EVC_L2CP_PB,
    EVC_L2CP_PB_GVRP,
    EVC_L2CP_LLDP,      /* 10 */
    EVC_L2CP_GMRP,
    EVC_L2CP_GVRP,
    EVC_L2CP_ULD,
    EVC_L2CP_PAGP,
    EVC_L2CP_PVST,
    EVC_L2CP_CISCO_VLAN,
    EVC_L2CP_CDP,
    EVC_L2CP_VTP,
    EVC_L2CP_DTP,
    EVC_L2CP_CISCO_STP, /* 20 */
    EVC_L2CP_CISCO_CFM,

    EVC_L2CP_CNT /* Number of protocols */
} evc_l2cp_t;

typedef enum {
    EVC_L2CP_MODE_FORWARD,
    EVC_L2CP_MODE_TUNNEL,
    EVC_L2CP_MODE_DISCARD,
    EVC_L2CP_MODE_PEER
} evc_l2cp_mode_t;

typedef enum {
    EVC_L2CP_DMAC_CUSTOM,
    EVC_L2CP_DMAC_CISCO
} evc_l2cp_dmac_t;

typedef struct {
    evc_l2cp_t      proto;
    evc_l2cp_mode_t mode;
    evc_l2cp_dmac_t dmac;
} evc_l2cp_data_t;

typedef struct {
    evc_l2cp_data_t l2cp;
} evc_ece_data_t;
#endif /* VTSS_ARCH_SERVAL */

/* ECE entry configuration using the VTSS API structure.
   The key.tag.vid.mask must be 0 (any) or 0xfff (specific) */
typedef struct {
    vtss_ece_t     conf;
#if defined(VTSS_ARCH_SERVAL)
    evc_ece_data_t data;
#endif /* VTSS_ARCH_SERVAL */
    BOOL           conflict; /* VTSS API failure indication */
} evc_mgmt_ece_conf_t;

/* Add ECE */
vtss_rc evc_mgmt_ece_add(vtss_ece_id_t next_id, evc_mgmt_ece_conf_t *conf);

/* Delete ECE */
vtss_rc evc_mgmt_ece_del(vtss_ece_id_t id);

/* Get ECE or get next ECE */
#define EVC_ECE_ID_FIRST VTSS_ECE_ID_LAST
vtss_rc evc_mgmt_ece_get(vtss_ece_id_t id, evc_mgmt_ece_conf_t *conf, BOOL next);

/* Get ECE default configuration */
void evc_mgmt_ece_get_default(evc_mgmt_ece_conf_t *conf);

/* - Port information ---------------------------------------------- */

/* EVC port information */
typedef struct {
    u16 nni_count; /* Number of EVCs, where the port is an NNI */
    u16 uni_count; /* Number of ECEs, where the port is an UNI */
} evc_port_info_t;

/* Get port information */
vtss_rc evc_mgmt_port_info_get(evc_port_info_t info[VTSS_PORT_ARRAY_SIZE]);

/* ================================================================= *
 *  Control API
 * ================================================================= */

/* EVC change callback */
typedef void (*evc_change_callback_t)(vtss_evc_id_t evc_id);

/* EVC change callback registration */
vtss_rc evc_change_register(evc_change_callback_t callback);

/* ================================================================= *
 *  Initialization
 * ================================================================= */

/* Initialize module */
vtss_rc evc_init(vtss_init_data_t *data);

#endif /* _EVC_API_H_ */
