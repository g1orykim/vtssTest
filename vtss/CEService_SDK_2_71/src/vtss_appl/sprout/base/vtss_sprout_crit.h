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

#ifndef _VTSS_SPROUT_CRIT_H_
#define _VTSS_SPROUT_CRIT_H_

#if VTSS_SPROUT_MULTI_THREAD


#if VTSS_TRACE_ENABLED
#define VTSS_SPROUT_CRIT_INIT_TBL_RD()              vtss_sprout_crit_init(  &crit_tbl_rd, "tbl_rd", TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_ENTER_TBL_RD()             vtss_sprout_crit_lock(  &crit_tbl_rd,           TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_EXIT_TBL_RD()              vtss_sprout_crit_unlock(&crit_tbl_rd,           TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_ASSERT_TBL_RD(locked)      vtss_sprout_crit_assert(&crit_tbl_rd, locked,   TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)

#define VTSS_SPROUT_CRIT_INIT_STATE_DATA()          vtss_sprout_crit_init(  &crit_state_data, "state_data", TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_ENTER_STATE_DATA()         vtss_sprout_crit_lock(  &crit_state_data,               TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_EXIT_STATE_DATA()          vtss_sprout_crit_unlock(&crit_state_data,               TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_ASSERT_STATE_DATA(locked)  vtss_sprout_crit_assert(&crit_state_data, locked,       TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)

#define VTSS_SPROUT_CRIT_INIT_DBG()                 vtss_sprout_crit_init(  &crit_dbg, "dbg",        TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_ENTER_DBG()                vtss_sprout_crit_lock(  &crit_dbg,               TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_EXIT_DBG()                 vtss_sprout_crit_unlock(&crit_dbg,               TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#define VTSS_SPROUT_CRIT_ASSERT_DBG(locked)         vtss_sprout_crit_assert(&crit_dbg, locked,       TRACE_GRP_CRIT, VTSS_TRACE_LVL_RACKET, __FILE__, __LINE__)
#else 

#define VTSS_SPROUT_CRIT_INIT_TBL_RD()              vtss_sprout_crit_init(  &crit_tbl_rd, "tbl_rd")
#define VTSS_SPROUT_CRIT_ENTER_TBL_RD()             vtss_sprout_crit_lock(  &crit_tbl_rd)
#define VTSS_SPROUT_CRIT_EXIT_TBL_RD()              vtss_sprout_crit_unlock(&crit_tbl_rd)
#define VTSS_SPROUT_CRIT_ASSERT_TBL_RD(locked)      vtss_sprout_crit_assert(&crit_tbl_rd, locked)

#define VTSS_SPROUT_CRIT_INIT_STATE_DATA()          vtss_sprout_crit_init(  &crit_state_data, "state_data")
#define VTSS_SPROUT_CRIT_ENTER_STATE_DATA()         vtss_sprout_crit_lock(  &crit_state_data)
#define VTSS_SPROUT_CRIT_EXIT_STATE_DATA()          vtss_sprout_crit_unlock(&crit_state_data)
#define VTSS_SPROUT_CRIT_ASSERT_STATE_DATA(locked)  vtss_sprout_crit_assert(&crit_state_data, locked)

#define VTSS_SPROUT_CRIT_INIT_DBG()                 vtss_sprout_crit_init(  &crit_dbg, "dbg")
#define VTSS_SPROUT_CRIT_ENTER_DBG()                vtss_sprout_crit_lock(  &crit_dbg)
#define VTSS_SPROUT_CRIT_EXIT_DBG()                 vtss_sprout_crit_unlock(&crit_dbg)
#define VTSS_SPROUT_CRIT_ASSERT_DBG(locked)         vtss_sprout_crit_assert(&crit_dbg, locked)
#endif 

#else 

#define VTSS_SPROUT_CRIT_INIT_TBL_RD()
#define VTSS_SPROUT_CRIT_ENTER_TBL_RD()
#define VTSS_SPROUT_CRIT_EXIT_TBL_RD()
#define VTSS_SPROUT_CRIT_ASSERT_TBL_RD(locked)

#define VTSS_SPROUT_CRIT_INIT_STATE_DATA()
#define VTSS_SPROUT_CRIT_ENTER_STATE_DATA()
#define VTSS_SPROUT_CRIT_EXIT_STATE_DATA()
#define VTSS_SPROUT_CRIT_ASSERT_STATE_DATA(locked)

#define VTSS_SPROUT_CRIT_INIT_DBG()
#define VTSS_SPROUT_CRIT_ENTER_DBG()
#define VTSS_SPROUT_CRIT_EXIT_DBG()
#define VTSS_SPROUT_CRIT_ASSERT_DBG(locked)

#endif





#define VTSS_SPROUT_CRIT_NAME_LEN 16

typedef struct _vtss_sprout_crit_t {
    vtss_os_sem_t sem;
    
    char         name[VTSS_SPROUT_CRIT_NAME_LEN];

    BOOL         init_done;

#if VTSS_TRACE_ENABLED
    
    ulong        current_lock_thread_id;

    
    
    ulong        lock_thread_id;
    const char   *lock_file;
    int          lock_line;

    ulong        unlock_thread_id;
    const char   *unlock_file;
    int          unlock_line;
#endif
} vtss_sprout_crit_t;



vtss_sprout_crit_t  crit_state_data;


vtss_sprout_crit_t  crit_tbl_rd;


vtss_sprout_crit_t  crit_dbg;


void vtss_sprout_crit_init(
    vtss_sprout_crit_t *crit_p,
    const char *name
#if VTSS_TRACE_ENABLED
    ,
    int        trace_grp,
    int        trace_lvl,
    const char *file,
    const int  line
#endif
);



void vtss_sprout_crit_lock(
    vtss_sprout_crit_t *crit_p
#if VTSS_TRACE_ENABLED
    ,
    int        trace_grp,
    int        trace_lvl,
    const char *file,
    const int  line
#endif
);



void vtss_sprout_crit_unlock(
    vtss_sprout_crit_t *crit_p
#if VTSS_TRACE_ENABLED
    ,
    int        trace_grp,
    int        trace_lvl,
    const char *file,
    const int  line
#endif
);



void vtss_sprout_crit_assert(
    vtss_sprout_crit_t *crit_p,
    BOOL              locked
#if VTSS_TRACE_ENABLED
    ,
    int        trace_grp,
    int        trace_lvl,
    const char *file,
    const int  line
#endif
);




#endif 






