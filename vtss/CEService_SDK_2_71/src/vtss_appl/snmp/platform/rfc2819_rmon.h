/*

 Vitesse Switch Software.

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

#ifndef RFC_2819_RMON_H
#define RFC_2819_RMON_H

#define RFC2819_SUPPORTED_STATISTICS   1
#define RFC2819_SUPPORTED_HISTORY      1
#define RFC2819_SUPPORTED_AlARM        1
#define RFC2819_SUPPORTED_EVENT        1

/*
 * Function declarations
 */
#if RFC2819_SUPPORTED_STATISTICS
/* statistics ----------------------------------------------------------*/
void init_rmon_statisticsMIB(void);
#endif /* RFC2819_SUPPORTED_STATISTICS */

#if RFC2819_SUPPORTED_HISTORY
/* history ----------------------------------------------------------*/
void init_rmon_historyMIB(void);
int write_historyControl(int action, u_char *var_val, u_char var_val_type,
                         size_t var_val_len, u_char *statP,
                         oid *name, size_t name_len);
#endif /* RFC2819_SUPPORTED_HISTORY */

#if RFC2819_SUPPORTED_AlARM
/* alram ----------------------------------------------------------*/
void init_rmon_alarmMIB(void);
#endif /* RFC2819_SUPPORTED_AlARM */

#if RFC2819_SUPPORTED_EVENT
/* event ----------------------------------------------------------*/
void init_rmon_eventMIB(void);

int write_eventControl(int action, u_char *var_val, u_char var_val_type,
                       size_t var_val_len, u_char *statP,
                       oid *name, size_t name_len);
#endif /* RFC2819_SUPPORTED_EVENT */

#endif /* RFC_2819_RMON_H */

