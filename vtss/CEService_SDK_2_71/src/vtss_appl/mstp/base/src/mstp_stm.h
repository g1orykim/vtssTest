/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_MSTP_STM_H_
#define _VTSS_MSTP_STM_H_

/** MSTI State machine */
typedef struct {
    const char *name;
    int (*begin)(struct mstp_tree *tree);
    int (*run)(struct mstp_tree *tree, uint *transitions, int state);
    const char *(*statename)(int state);
} bridge_stm_t;

/** CIST Port State machine */
typedef struct {
    const char *name;
    int (*begin)(struct mstp_port *port);
    int (*run)(struct mstp_port *port, uint *transitions, int state);
    const char *(*statename)(int state);
} port_stm_t;

extern const bridge_stm_t PortRoleSelection_stm;

extern const port_stm_t PortReceive_stm;
extern const port_stm_t PortProtocolMigration_stm;
extern const port_stm_t BridgeDetection_stm;
extern const port_stm_t PortTransmit_stm;
extern const port_stm_t PortInformation_stm;
extern const port_stm_t PortRoleTransition_stm;
extern const port_stm_t PortStateTransition_stm;
extern const port_stm_t TopologyChange_stm;

#define NewState(t, pt, s, v) do { s = stm_enter(t, pt, v); loop_count++; goto v ## _ENTER; } while(0)

#define LOOP_PROTECT(_s_,_ct_)                                          \
    do {                                                                \
        if(loop_count > _ct_) {                                         \
            T_I("Looping detect (%s): %u transitions", stm_statename(_s_), loop_count); \
            return _s_;                                                 \
        }                                                               \
    } while(0)

#endif /* _VTSS_MSTP_STM_H_ */
