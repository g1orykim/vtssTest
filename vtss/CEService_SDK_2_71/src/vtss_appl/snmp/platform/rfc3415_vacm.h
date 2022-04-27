/*

 Vitesse Switch API software.

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

#ifndef RFC3415_VACM_H
#define RFC3415_VACM_H


/*
 * Function declarations
 */
#if RFC3415_SUPPORTED_VACMCONTEXTTABLE
/* vacmContextTable ----------------------------------------------------------*/
void            init_vacmContextTable(void);
#endif /* RFC3415_SUPPORTED_VACMCONTEXTTABLE */

#if RFC3415_SUPPORTED_VACMSECURITYTOGROUPTABLE
/* vacmSecurityToGroupTable ----------------------------------------------------------*/
void            init_vacmSecurityToGroupTable(void);
#endif /* RFC3415_SUPPORTED_VACMSECURITYTOGROUPTABLE */

#if RFC3415_SUPPORTED_VACMACCESSTABLE
/* vacmAccessTable ----------------------------------------------------------*/
void            init_vacmAccessTable(void);
#endif /* RFC3415_SUPPORTED_VACMACCESSTABLE */

#if RFC3415_SUPPORTED_VACMMIBVIEWS
/* vacmMIBViews ----------------------------------------------------------*/
void            init_vacmMIBViews(void);
#endif /* RFC3415_SUPPORTED_VACMMIBVIEWS */

#endif /* RFC3415_VACM_H */

