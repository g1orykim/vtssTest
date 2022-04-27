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

#include "ae.h"
#include "ae_cfg.h"
#include "ae_reg_access.h"



  /* This is an example of how adaptive equalization can be implemented.
     Each entry in link keeps the state for one channel (rx,tx pair).

		 done indicates that all the links have either passed or failed. If
		 link[i].status == AE_CONVERGED, the link passed, otherwise it failed
		 (NOTE: please report failures to ibm as they should be VERY infrequent
		  (less than 1 out of 500) on passable links! This success of this
			algorithm	is dependent on many variables of the total system, and while
			every effort is made to make as many links pass as possible with the
			default configuration, it is possible you may get better performance
			and faster convergence with different parameters in your
			ae_config.h file.)

     Each time ae_run_link is called, it will process samples until 
     it must wait for convergence, or a tx update is required, or training
     is complete. The code advances all the links in parallel by
     synchronizing them on each tx update. This is just one way of implementing
		 the multitasking, it is not a requirement.

     This scheme is often the best way to guarantee all the links are "mostly"
     trained even if you run out of training time. When time runs out, they
     will all be "almost" trained instead of several of the links not
     having gone through any training at all. The algorithm spends 90% of the
		 total CPU time getting the last %5 of performance.
  */
AE_STATUS ae_run(AE_LINK* link, int links){

  char done = 0;
  int i;
  AE_STATUS status;
	for(i = 0; i < links; i++)
		link[i].wait_until = 0;

  while(!done){
    done = 1;
		
    for(i = 0; i < links; i++){
			char dont_wait;
			dont_wait = (link[i].requires_wait && time_usec() > link[i].wait_until);

              if(AE_DONE(link[i].status)){				        
				
              }else if(!link[i].requires_update && (!link[i].requires_wait || dont_wait)){
				        done = 0;
				        status = ae_run_link(&link[i]);				        

				        if(AE_REQUIRES_UPDATE(status)){
					        link[i].requires_update = 1;
				        }
			  
				        if(AE_REQUIRES_WAIT(status)){
					        link[i].wait_until = time_usec() + 1000;
					        link[i].requires_wait = 1;
				        }else{
					        link[i].requires_wait = 0;
				        }

				        // if using the AEC and the max_wait_timer, poll for it here
				        // and set link[i]->abort = 1 if it expires.
				
              } else {
				        if(link[i].requires_wait) done = 0;				        
              }
    }

    //Now update all the transmitters that need updates at once. 
    send_all_updates(link, links);
  }	

  return 0;
}
