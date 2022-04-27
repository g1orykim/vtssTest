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

/*

	This is an example of how adaptive equalization can be called.
	
	Modification of ae_run() (ae_run.c) is not required unless the otherwise
	indicated for	your application.
	
	Before calling ae_run(), you must:
	  Initialize all the links using ae_init().
	  Initialize all the transmitters (this is separate from ae_init() in case the
	  transmitters are serviced by a different processor).
	
 */

#include "ae.h"
#include "ae_cfg.h" 
#include "ae_reg_access.h"
int ae_main(void); /* forward declaration to avoid compiler warning */

int ae_main(void){
	AE_LINK link[LINKS];	
	int i;
	
	for(i = 0; i < LINKS; i++){
        ae_init(&link[i],i);
        ae_init_tx(&link[i]);
	}
    	
	ae_run(link, LINKS);

	return 0;
}
