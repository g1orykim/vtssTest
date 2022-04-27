/*

 Vitesse Switch Software.

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


 This file is part of SPROUT - "Stack Protocol using ROUting Technology".
*/









#ifndef _VTSS_SPROUT_XIT_H_
#define _VTSS_SPROUT_XIT_H_










BOOL vtss_sprout__rit_update(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__ri_t             *ri_p,
    const vtss_sprout__stack_port_idx_t sp_idx);





vtss_sprout__ri_t *vtss_sprout__ri_find(
    vtss_sprout__rit_t              *rit_p,
    const vtss_sprout_switch_addr_t *switch_addr_p,
    const vtss_sprout__unit_idx_t   unit_idx);





vtss_sprout__ri_t *vtss_sprout__ri_find_at_dist(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx,
    const vtss_sprout_dist_t            dist);






void vtss_sprout__rit_infinity_all(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx);






void vtss_sprout__rit_infinity_beyond_dist(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx,
    const vtss_sprout_dist_t            max_dist);






uint vtss_sprout__rit_infinity_if_not_found(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx);





void vtss_sprout__rit_clr_flags(
    vtss_sprout__rit_t               *rit_p);






vtss_sprout__ri_t *vtss_sprout__ri_get_nxt(
    vtss_sprout__rit_t *rit_p,
    vtss_sprout__ri_t  *ri_p);





vtss_sprout_dist_t vtss_sprout__get_dist(
    vtss_sprout__rit_t                  *rit_p,
    const vtss_sprout__stack_port_idx_t sp_idx,
    const vtss_sprout_switch_addr_t     *switch_addr,
    const vtss_sprout__unit_idx_t       unit_idx);













BOOL vtss_sprout__uit_update(
    vtss_sprout__uit_t      *uit_p,
    const vtss_sprout__ui_t *ui_p,
    BOOL                    pdu_rx);






vtss_sprout__ui_t *vtss_sprout__ui_find(
    vtss_sprout__uit_t              *uit_p,
    const vtss_sprout_switch_addr_t *switch_addr_p,
    const vtss_sprout__unit_idx_t   unit_idx);


void vtss_sprout__uit_clr_flags(
    vtss_sprout__uit_t                 *uit_p);






uint vtss_sprout__uit_del_unfound(
    vtss_sprout__uit_t *uit_p);







vtss_sprout__ui_t *vtss_sprout__ui_get_nxt(
    vtss_sprout__uit_t *uit_p,
    vtss_sprout__ui_t  *ui_p);












#endif 






