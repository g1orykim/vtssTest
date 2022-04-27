/*

 Vitesse API software.

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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_L2
#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

/* - CIL functions ------------------------------------------------- */

/* Wait until the MAC operation is finsished */
/* Polling method is used, the sticky interrupt is not used */
static vtss_rc jr_mac_table_idle(vtss_state_t *vtss_state)
{
    u32 cmd;    
    while (1) {
        JR_RD(LRN_COMMON, COMMON_ACCESS_CTRL, &cmd); 

        if ((VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT & cmd) == 
            0)
            break;
    }
    return VTSS_RC_OK;
}
 
 
/*******************************************************************************
 * Add/delete/get/get_next MAC address entry. 
 *
 * involved registers:
 *  MAC_ACCESS_CFG_0
 *  MAC_ACCESS_CFG_1
 *  MAC_ACCESS_CFG_2
 *  MAC_ACCESS_CFG_3
 *  COMMON_ACCESS_CTRL
 *
 ******************************************************************************/
/*
 * Add MAC address entry. 
 *
 * \param entry [IN]  MAC address entry structure.
 * \param pgid [IN]   Multicast group ID: = VTSS_PGID_NONE if IP4/IP6 multicast address
 *                      = index into the vtss_state->l2.pgid_table, which holds a portmask for the entry.
 *                        valid index in Jaguar: [0..511] 
 * \return Return code.
 **/
static vtss_rc jr_mac_table_add_chip(vtss_state_t *vtss_state,
                                     const vtss_mac_table_entry_t *const entry, u32 pgid)
{
    u32 mach, macl, mask = 0, idx = 0, aged = 0, fwd_kill = 0, cpu_copy = entry->copy_to_cpu;
    u32 entry_type = MAC_ENTRY_TYPE_STD_ENTRY;
    u32 addr_type = MAC_ENTRY_ADDR_TYPE_UPSID_PN;
    
    vtss_mach_macl_get(&entry->vid_mac, &mach, &macl);
    VTSS_D("address 0x%08x%08x", mach, macl);
   
    if (pgid == VTSS_PGID_NONE) {
        /* IPv4/IPv6 multicast entry */
        entry_type = (entry->vid_mac.mac.addr[0] == 0x01 ? MAC_ENTRY_TYPE_IPV4_MC_ENTRY :
                      MAC_ENTRY_TYPE_IPV6_MC_ENTRY);
        
        mask = vtss_jr1_port_mask(vtss_state, vtss_state->l2.pgid_table[pgid].member); 
        if (vtss_state->chip_count == 2) {
            /* Include internal ports in mask */
            mask |= vtss_state->port.mask_int_ports;
        }
    } else {
        /* Not IP multicast entry */
        /* Set FWD_KILL to make the switch discard frames in SMAC lookup */
        fwd_kill = (cpu_copy || (pgid != vtss_state->l2.pgid_drop) ? 0 : 1); /* TBD if needed in Jaguar */
        idx = vtss_jr1_chip_pgid(vtss_state, pgid);

        if (idx >= VTSS_CHIP_PORTS) {
            /* Multicast PGID */
            addr_type = MAC_ENTRY_ADDR_TYPE_MC_IDX;
            idx -= VTSS_CHIP_PORTS;
        } else {
#if defined(VTSS_FEATURE_VSTAX_V2)
            if (entry->vstax2.enable) {
                /* Use (UPSID, UPSPN) */
                idx = ((entry->vstax2.upsid<<5) + entry->vstax2.upspn);
            } else {
                /* Use local (UPSID, UPSPN) */
                idx += (vtss_state->l2.vstax_info.upsid[VTSS_CHIP_NO(pgid)]<<5);
            }
#endif /* VTSS_FEATURE_VSTAX_V2 */
        } 
    }

    VTSS_D("address 0x%x  0x%x", mach, macl);

    /* Insert/learn new entry into the MAC table  */
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_0, mach);
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_1, macl);

    // Due to a chip-bug in Jaguar (see Bugzilla#7730), the CPU must do S/W-assisted
    // H/W-based learning in a stack. For this to work, the FDMA driver re-injects all
    // learn-frames on the stack links.
    // To overcome another chip-bug (see Bugzilla#9545), the DMAC of the re-injected
    // learn-all frame must not match an entry in the MAC table with the CPU-copy
    // flag set, because that would cause the frame do get to the CPU multiple times on
    // the unit(s) that has the CPU-copy flag set in the MAC table. The entry may, however,
    // match an entry in the MAC table with a pointer to a PGID entry that has the CPU copy
    // flag set (this will not give rise to multiple copied).
    // Therefore, the FDMA driver and this piece of code has a silent agreement on a common
    // MAC address that will never have the CPU copy flag MAC address set (but use the PGID
    // table instead if needed).
    // Any MAC address can be used, but for simplicity, we use the broadcast MAC address,
    // and rely on the fact that this gets copied to the CPU through the PGID table, if needed.
    if (cpu_copy && VTSS_MAC_BC(entry->vid_mac.mac.addr)) {
        cpu_copy = 0;
    }

    /* MAC_ENTRY_VLAN_IGNORE is set to 0 */
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_2, 
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR(idx) |
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE(addr_type) |
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE(entry_type) |
          (fwd_kill ? VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_SRC_KILL_FWD : 0) |
          (cpu_copy ? VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_CPU_COPY : 0) |
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_AGE_FLAG(aged) |
          (entry->locked ? VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_LOCKED : 0) |
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_VLD);
    {
        u32 cfg2;
        JR_RD(LRN_COMMON, MAC_ACCESS_CFG_2, &cfg2);
        VTSS_D("cfg2: 0x%08x", cfg2);
    }

    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_3, mask);

    JR_WR(LRN_COMMON, COMMON_ACCESS_CTRL, 
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD(MAC_CMD_LEARN) |
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT);

    /* Wait until MAC operation is finished */
    return jr_mac_table_idle(vtss_state);
}

static vtss_rc jr_mac_table_add(vtss_state_t *vtss_state,
                                const vtss_mac_table_entry_t *const entry, u32 pgid)
{
    vtss_chip_no_t chip_no;

    /* Add in all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_mac_table_add_chip(vtss_state, entry, pgid));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_mac_table_del_chip(vtss_state_t *vtss_state,
                                     const vtss_vid_mac_t *const vid_mac)
{
    u32 mach, macl;
    
    vtss_mach_macl_get(vid_mac, &mach, &macl);
    VTSS_D("address 0x%08x%08x", mach, macl);
    
    /* Delete/unlearn the given MAC entry */
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_0, mach);
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_1, macl);


    JR_WR(LRN_COMMON, COMMON_ACCESS_CTRL, 
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD(MAC_CMD_UNLEARN) |
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT);
    
    /* Wait until MAC operation is finished */
    return jr_mac_table_idle(vtss_state);
}

static vtss_rc jr_mac_table_del(vtss_state_t *vtss_state,
                                const vtss_vid_mac_t *const vid_mac)
{
    vtss_chip_no_t chip_no;

    /* Delete from all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_mac_table_del_chip(vtss_state, vid_mac));
    }
    return VTSS_RC_OK;
}

/* Analyze the result in the MAC table */
static vtss_rc jr_mac_table_result(vtss_state_t *vtss_state,
                                   vtss_mac_table_entry_t *const entry, u32 *pgid)
{
    u32                cfg2, cfg3, mach, macl, idx;
    vtss_port_no_t     port_no;
    vtss_pgid_entry_t  *pgid_entry;
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_chip_no_t     chip_no = vtss_state->chip_no;
    u32                glag_no;
    vtss_vstax_upsid_t my_upsid, upsid;
#endif /* VTSS_FEATURE_VSTAX_V2 */
    
    JR_RD(LRN_COMMON, MAC_ACCESS_CFG_2, &cfg2);
    JR_RD(LRN_COMMON, MAC_ACCESS_CFG_3, &cfg3);
    
    VTSS_D("MAC_ACCESS_CFG_2 %x, MAC_ACCESS_CFG_3 %x",cfg2,cfg3);
    
    /* Check if entry is valid */
    if (!(cfg2 & VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_VLD)) {
        VTSS_D("not valid");
        return VTSS_RC_ERROR;
    }

#if defined(VTSS_FEATURE_VSTAX_V2)
    my_upsid = vtss_state->l2.vstax_info.upsid[chip_no];
    entry->vstax2.enable = 0;
    entry->vstax2.remote_entry = 0;
    entry->vstax2.upsid = my_upsid;
    entry->vstax2.upspn = 0;
#endif /* VTSS_FEATURE_VSTAX_V2 */
    
    /* extract fields from Jaguar registers */
    entry->aged        = VTSS_BOOL(VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_AGE_FLAG(cfg2));
    entry->copy_to_cpu = VTSS_BOOL(cfg2 & VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_CPU_COPY);
    entry->locked      = VTSS_BOOL(cfg2 & VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_LOCKED);
    idx                = VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR(cfg2); 

    JR_RD(LRN_COMMON, MAC_ACCESS_CFG_0, &mach);
    JR_RD(LRN_COMMON, MAC_ACCESS_CFG_1, &macl);
    
    entry->vid_mac.vid = ((mach>>16) & 0xFFF);
    entry->vid_mac.mac.addr[0] = ((mach>>8)  & 0xff);
    entry->vid_mac.mac.addr[1] = ((mach>>0)  & 0xff);
    entry->vid_mac.mac.addr[2] = ((macl>>24) & 0xff);
    entry->vid_mac.mac.addr[3] = ((macl>>16) & 0xff);
    entry->vid_mac.mac.addr[4] = ((macl>>8)  & 0xff);
    entry->vid_mac.mac.addr[5] = ((macl>>0)  & 0xff);
    VTSS_D("found 0x%08x%08x, cfg2: 0x%08x, cfg3: 0x%08x", mach, macl, cfg2, cfg3);

    switch (VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE(cfg2)) {
    case MAC_ENTRY_TYPE_STD_ENTRY:
        /* Normal entry */
        if (VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE(cfg2) == 
            MAC_ENTRY_ADDR_TYPE_MC_IDX) {
            /* Multicast PGID */
            idx += VTSS_CHIP_PORTS;
        }
        *pgid = vtss_jr1_vtss_pgid(vtss_state, idx);
#if defined(VTSS_FEATURE_VSTAX_V2)

        upsid = ((idx >> 5) & 0x1F);
        switch (VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE(cfg2)) {
        case  MAC_ENTRY_ADDR_TYPE_UPSID_PN:
            entry->vstax2.enable = 1;
            entry->vstax2.remote_entry = (upsid == my_upsid ? 0 : 1);
            entry->vstax2.upsid = upsid;
            entry->vstax2.upspn = idx & 0x1F;
            if (vtss_state->chip_count == 2 && 
                vtss_state->l2.vstax_info.upsid[chip_no ? 0 : 1] == upsid) {
                /* If the address is learned on the other device, we change the context to 
                   that device to make the PGID calculation done in vtss_jr1_vtss_pgid() correct */
                entry->vstax2.remote_entry = 0;
                VTSS_SELECT_CHIP(chip_no ? 0 : 1);
            }
            *pgid = vtss_jr1_vtss_pgid(vtss_state, entry->vstax2.upspn);
            VTSS_SELECT_CHIP(chip_no);
            break;
        case MAC_ENTRY_ADDR_TYPE_UPSID_CPU_OR_INT:
            entry->vstax2.remote_entry = (upsid == my_upsid ? 0 : 1);
            entry->vstax2.upsid = upsid;
            entry->vstax2.upspn = VTSS_UPSPN_CPU;
            *pgid = VTSS_PGID_NONE;
            break;
        case MAC_ENTRY_ADDR_TYPE_GLAG:
            glag_no = VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR(cfg2) & 0x1F;
            *pgid = VTSS_PGID_NONE;
            pgid_entry = &vtss_state->l2.pgid_table[*pgid];

            /* Find out if this we have local members of this GLAG */
            entry->vstax2.remote_entry = 1;
            for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
                if (vtss_state->l2.port_glag_no[port_no] == glag_no) {
                    /* Return the local members of the GLAG */
                    pgid_entry->member[port_no] = 1;
                    entry->vstax2.remote_entry = 0;
                }
            }
            if (entry->vstax2.remote_entry) {
                /* No local members - return the first upsid/upspn member entry in the GLAG */
                vtss_glag_conf_t *glag_conf = &vtss_state->l2.glag_conf[glag_no][0];

                if (glag_conf->entry.upspn == VTSS_UPSPN_NONE) {
                    /* No valid GLAG members */
                    entry->vstax2.upsid = 0;
                } else {
                    /* First GLAG member is valid */
                    entry->vstax2.upsid = glag_conf->entry.upsid;
                    entry->vstax2.upspn = glag_conf->entry.upspn;
                }
            }       
            break;
        case  MAC_ENTRY_ADDR_TYPE_MC_IDX:
            /* Already covered */
            break;
        case  MAC_ENTRY_ADDR_TYPE_CONID_FWD:
        case  MAC_ENTRY_ADDR_TYPE_PATHGRP_FWD:
        default:
            VTSS_E("unsupported addr type:%u",VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE(cfg2));
            return VTSS_RC_ERROR;
        }     
#endif /* VTSS_FEATURE_VSTAX_V2 */
        break;
    case MAC_ENTRY_TYPE_IPV4_MC_ENTRY:
    case MAC_ENTRY_TYPE_IPV6_MC_ENTRY:
        /* IPv4/IPv6 multicast address */
        *pgid = VTSS_PGID_NONE;
        /* Convert port mask */
        pgid_entry = &vtss_state->l2.pgid_table[*pgid];
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if ((cfg3 & (1 << VTSS_CHIP_PORT(port_no))) && VTSS_PORT_CHIP_SELECTED(port_no)) {
                pgid_entry->member[port_no] = 1;
            }
        }

        /* Reset the search criteria */
        JR_WRM(LRN_COMMON, MAC_ACCESS_CFG_2, 0, VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE);
        break;
    case MAC_ENTRY_TYPE_PATH_CCM_MEPID:
    default:
        VTSS_E("unsupported entry");
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_mac_table_get_chip(vtss_state_t *vtss_state,
                                     vtss_mac_table_entry_t *const entry, u32 *pgid)
{
    u32 mach, macl, type;

    vtss_mach_macl_get(&entry->vid_mac, &mach, &macl);
    VTSS_D("address 0x%08x%08x", mach, macl);
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_0, mach);
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_1, macl);

    /* BZ 10076: Setup entry type before lookup operation */
    type = (VTSS_MAC_IPV4_MC(entry->vid_mac.mac.addr) ? MAC_ENTRY_TYPE_IPV4_MC_ENTRY :
            VTSS_MAC_IPV6_MC(entry->vid_mac.mac.addr) ? MAC_ENTRY_TYPE_IPV6_MC_ENTRY :
            MAC_ENTRY_TYPE_STD_ENTRY);
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_2, VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE(type));

    /* Do a lookup */
    JR_WR(LRN_COMMON, COMMON_ACCESS_CTRL, 
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD(MAC_CMD_LOOKUP) |
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT);

    VTSS_RC(jr_mac_table_idle(vtss_state));

    return jr_mac_table_result(vtss_state, entry, pgid);
}

static vtss_rc jr_mac_table_get(vtss_state_t *vtss_state,
                                vtss_mac_table_entry_t *const entry, u32 *pgid)
{
    vtss_pgid_entry_t *pgid_entry = &vtss_state->l2.pgid_table[VTSS_PGID_NONE];
    
    /* Clear PGID entry for IPMC/GLAG entries */
    memset(pgid_entry, 0, sizeof(*pgid_entry));
    
    /* Get entry from first device */
    VTSS_RC(jr_mac_table_get_chip(vtss_state, entry, pgid));

    /* For IPMC entries, get entry (port set) from second device */
    if (vtss_state->chip_count == 2 && *pgid == VTSS_PGID_NONE) {
        VTSS_SELECT_CHIP(1);
        VTSS_RC(jr_mac_table_get_chip(vtss_state, entry, pgid));
        VTSS_SELECT_CHIP(0);
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_mac_table_get_next(vtss_state_t *vtss_state,
                                     vtss_mac_table_entry_t *const entry, u32 *pgid)
{
    u32               mach, macl;
    vtss_pgid_entry_t *pgid_entry = &vtss_state->l2.pgid_table[VTSS_PGID_NONE];
    
    /* Clear PGID entry for IPMC/GLAG entries */
    memset(pgid_entry, 0, sizeof(*pgid_entry));

    vtss_mach_macl_get(&entry->vid_mac, &mach, &macl);
    VTSS_D("address 0x%08x%08x", mach, macl);

    /* Ensure that no filtering is active */
    JR_WR(LRN_COMMON, SCAN_NEXT_CFG, 
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_TYPE_MASK(1) |
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_UNTIL_FOUND_ENA);
    /* Static entries are left out due to a bug */
   
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_0, mach);
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_1, macl);

    /* Do a get next lookup */
    JR_WR(LRN_COMMON, COMMON_ACCESS_CTRL, 
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD(MAC_CMD_FIND_SMALLEST) |
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT);

    VTSS_RC(jr_mac_table_idle(vtss_state));
    return jr_mac_table_result(vtss_state, entry, pgid);
}

/*******************************************************************************
 *  MAC ageing. 
 *
 * involved registers:
 *  AUTOAGE_CFG
 *  AUTOAGE_CFG_1
 *  SCAN_NEXT_CFG           The CCM bits are set to 0, 
 *  MAC_ACCESS_CFG_0
 *  MAC_ACCESS_CFG_2
 *  COMMON_ACCESS_CTRL
 *
 * Pre condition (currently these are the default values):
 *  LRN_CFG.AGE_SIZE = 0 (i.e. 1 bit age counter. 2 bit age counter requires a change
 *                       in the flush functions, i.e. flush is done by calling the
 *                       age function two times.
 *  ANA_L2:COMMON:FILTER_OTHER_CTRL = 0 I.E allow ageing for remote learned entries. (ignored)
 *  ANA_L2:COMMON:FILTER_LOCAL_CTRL = 0 I.E. allow autoaging for all local ports. (ignored)
 *
 ******************************************************************************/

/*
 * set automatic ageing time. 
 * time = 0 disables ageing
 * time = [10..100000] seconds
 * The value of PERIOD_VAL in the AUTOAGE_CFG register shall be > 10.000 therefore:
 * 20001 < time          => units = 3 (sec), PERIOD_VAL = time/2 -1
 * 201 <   time <= 20001 => units = 2(10 ms), PERIOD_VAL = time*100/2 -1
 *         time <= 201   => units = 1(100 us), PERIOD_VAL = time*10000/2 -1
 */
static vtss_rc jr_mac_table_age_time_set_chip(vtss_state_t *vtss_state)
{
    u32 time, units;
    /* Scan two times per age time */
    time = vtss_state->l2.mac_age_time;
    if (time <= 1) {
        units = 0; /* 0: disable ageing */
        time = 0; 
    } else {
        units = 3; /* 0: disable ageing, 3: unit = 1 s. */
        //while (time < 20002) {
        //    --units;
        //    time = time * 100;
        //}
        time = (time/2)-1;           /* age period = time+1 */
    }
        
    if (time > 0x3fffff)
        time = 0x3fffff;         /* period_val is 22 bits wide */
    /* stop aging and set counters to a low value */
    JR_WR(LRN_COMMON, AUTOAGE_CFG, 
            VTSS_F_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_UNIT_SIZE(1)  |
            VTSS_F_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_PERIOD_VAL(1));
    JR_WR(LRN_COMMON, AUTOAGE_CFG_1, 
            VTSS_F_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_FORCE_IDLE_ENA |
            VTSS_F_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_FORCE_HW_SCAN_STOP_SHOT);
    

    VTSS_D("time %d, units %d", time, units);
    JR_WR(LRN_COMMON, AUTOAGE_CFG, 
            VTSS_F_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_UNIT_SIZE(units)  |
            VTSS_F_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_PERIOD_VAL(time));
    JR_WR(LRN_COMMON, AUTOAGE_CFG_1, 
            (units ? 0:VTSS_F_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_FORCE_IDLE_ENA) |
            VTSS_F_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_FORCE_HW_SCAN_STOP_SHOT);
    return VTSS_RC_OK;
}

static vtss_rc jr_mac_table_age_time_set(vtss_state_t *vtss_state)
{
    return vtss_jr1_conf_chips(vtss_state, jr_mac_table_age_time_set_chip);
}

#if defined(VTSS_FEATURE_VSTAX_V2)
static vtss_rc jr_mac_table_upsid_upspn_flush_chip(vtss_state_t *vtss_state,
                                                   const vtss_vstax_upsid_t upsid, 
                                                   const vtss_vstax_upspn_t upspn)
{
    u32 value;

    /* Flush all upsid entries by setting up a filter */
    JR_WR(LRN_COMMON, SCAN_NEXT_CFG, 
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_ADDR_FILTER_ENA |
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_TYPE_MASK(0x1) |
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_REMOVE_FOUND_ENA);

    /* Bits 14-12 = 110. Bits 9-5 (UPSID) = 0x1F (care). Bits 4-0 (UPSPN) = 0 */
    value = ((6 << 12) | (0x1F << 5) | (upspn == VTSS_UPSPN_NONE ? 0 : 0x1F));
    JR_WR(LRN_COMMON, SCAN_NEXT_CFG_1, 
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_1_SCAN_ENTRY_ADDR_MASK(value));
          
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_2, 
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR((upsid << 5) | (upspn & 0x1F)) | 
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE(MAC_ENTRY_ADDR_TYPE_UPSID_PN) |
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE(MAC_ENTRY_TYPE_STD_ENTRY));

    JR_RD(LRN_COMMON, MAC_ACCESS_CFG_2, &value);

    /* Do the aging */
    JR_WR(LRN_COMMON, COMMON_ACCESS_CTRL, 
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD(MAC_CMD_SCAN) |
            VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT);

    /* Wait until MAC operation is finished */
    return jr_mac_table_idle(vtss_state);
}

static vtss_rc jr_mac_table_upsid_upspn_flush(vtss_state_t *vtss_state,
                                              const vtss_vstax_upsid_t upsid,
                                              const vtss_vstax_upspn_t upspn)
{
    vtss_chip_no_t chip_no;

    /* Flush for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_mac_table_upsid_upspn_flush_chip(vtss_state, upsid, upspn));
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_VSTAX_V2 */

static vtss_rc jr_mac_table_age_chip(vtss_state_t     *vtss_state,
                                     BOOL             addr_age,
                                     u32              addr,
                                     BOOL             vid_age,
                                     const vtss_vid_t vid)
{
    VTSS_D("addr_age %u, addr %u, vid_age %u, vid %u", addr_age, addr, vid_age, vid);
    
    /* Selective aging */
    JR_WR(LRN_COMMON, SCAN_NEXT_CFG, 
          (vid_age ? VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_FID_FILTER_ENA : 0)   |
          (addr_age ? VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_ADDR_FILTER_ENA : 0) |
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_TYPE_MASK(0x1)              |
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_REMOVE_FOUND_ENA       |
          VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_INC_AGE_BITS_ENA);
            
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_0, VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_0_MAC_ENTRY_FID(vid));
    JR_WR(LRN_COMMON, MAC_ACCESS_CFG_2, 
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR(addr) |
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE(MAC_ENTRY_ADDR_TYPE_UPSID_PN)  |
          VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE(MAC_ENTRY_TYPE_STD_ENTRY));

    /* Do the aging */
    JR_WR(LRN_COMMON, COMMON_ACCESS_CTRL, 
          VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD(MAC_CMD_SCAN) |
          VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT);

    /* Wait until MAC operation is finished */
    return jr_mac_table_idle(vtss_state);
}

static vtss_rc jr_mac_table_age(vtss_state_t     *vtss_state,
                                BOOL             pgid_age,
                                u32              pgid,
                                BOOL             vid_age,
                                const vtss_vid_t vid)
{
    u32            addr_age = 0, addr = 0;
    vtss_chip_no_t chip_no;

    if (pgid_age && pgid < vtss_state->port_count) {
        /* Port specific ageing, find UPSID and UPSPN */
        addr_age = 1;
        addr = VTSS_CHIP_PORT(pgid);
#if defined(VTSS_FEATURE_VSTAX_V2)
        addr += (vtss_state->l2.vstax_info.upsid[vtss_state->port.map[pgid].chip_no] << 5);
#endif /* VSTAX_V2 */
    }

    /* Age for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_mac_table_age_chip(vtss_state, addr_age, addr, vid_age, vid));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_mac_table_status_get_chip(vtss_state_t *vtss_state,
                                            vtss_mac_table_status_t *status)
{
    u32 value;
    
    /* Detect port move events */
    JR_RD(ANA_L2, STICKY_STICKY, &value);
    value &= (VTSS_F_ANA_L2_STICKY_STICKY_LOCAL_TO_REMOTE_PORTMOVE_STICKY  |
              VTSS_F_ANA_L2_STICKY_STICKY_REMOTE_TO_LOCAL_PORTMOVE_STICKY  |
              VTSS_F_ANA_L2_STICKY_STICKY_REMOTE_TO_REMOTE_PORTMOVE_STICKY |
              VTSS_F_ANA_L2_STICKY_STICKY_GLOBAL_TO_GLOBAL_PORTMOVE_STICKY |
              VTSS_F_ANA_L2_STICKY_STICKY_GLOBAL_TO_LOCAL_PORTMOVE_STICKY  |
              VTSS_F_ANA_L2_STICKY_STICKY_LOCAL_TO_GLOBAL_PORTMOVE_STICKY  |
              VTSS_F_ANA_L2_STICKY_STICKY_LOCAL_TO_LOCAL_PORTMOVE_STICKY);
    JR_WR(ANA_L2, STICKY_STICKY, value);
    if (value)
        status->moved = 1;

    /* Read and clear sticky register */
    JR_RD(LRN_COMMON, EVENT_STICKY, &value);
    JR_WR(LRN_COMMON, EVENT_STICKY, value & 
          (VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_REPLACE_STICKY |
           VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_INSERT_STICKY |
           VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_REMOVE_STICKY |
           VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_AGED_STICKY));

    /* Detect learn events */
    if (value & VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_INSERT_STICKY)
        status->learned = 1;

    /* Detect replace events */
    if (value & VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_REPLACE_STICKY)
        status->replaced = 1;

    /* Detect age events (both aged and removed event is used */
    if (value & (VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_AGED_STICKY |
                 VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_REMOVE_STICKY))
        status->aged = 1;

    return VTSS_RC_OK;
}

static vtss_rc jr_mac_table_status_get(vtss_state_t *vtss_state,
                                       vtss_mac_table_status_t *status)
{
    vtss_chip_no_t chip_no;
    
    memset(status, 0, sizeof(*status));
    
    /* Get status from all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_mac_table_status_get_chip(vtss_state, status));
    }
    return VTSS_RC_OK;
}

/* Learn mode: 'Learn frames dropped' / 'Autolearning' / 'Copy learn frame to CPU' */
static vtss_rc jr_learn_port_mode_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no)
{
    vtss_learn_mode_t *mode = &vtss_state->l2.learn_mode[port_no];
    u32               mask = (1 << VTSS_CHIP_PORT(port_no));

    JR_WRM(ANA_L2_COMMON, LRN_SECUR_CFG,        mode->discard   ? mask : 0, mask);
    JR_WRM(ANA_L2_COMMON, LRN_SECUR_LOCKED_CFG, mode->discard   ? mask : 0, mask);
    JR_WRM(ANA_L2_COMMON, AUTO_LRN_CFG,         mode->automatic ? mask : 0, mask);
    JR_WRM(ANA_L2_COMMON, LRN_COPY_CFG,         mode->cpu       ? mask : 0, mask);

    if (!mode->automatic) {
        /* If automatic ageing is disabled, flush entries previously learned on port. 
           To avoid continuous refreshing, learning must be disabled during the operation */
        VTSS_RC(jr_mac_table_age(vtss_state, 1, port_no, 0, 0));
        VTSS_RC(jr_mac_table_age(vtss_state, 1, port_no, 0, 0));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_learn_state_set(vtss_state_t *vtss_state,
                                  const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    vtss_chip_no_t chip_no;
    u32            mask;

    /* Setup learn mask for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        mask = vtss_jr1_port_mask(vtss_state, member);
        if (vtss_state->chip_count == 2) {
            /* Include internal ports in learn mask */
            mask |= vtss_state->port.mask_int_ports;
        }
        JR_WRX(ANA_L3_2, MSTP_MSTP_LRN_CFG, 0, mask);
    }
    
    return VTSS_RC_OK;
}

static vtss_rc jr_vlan_conf_set_chip(vtss_state_t *vtss_state, BOOL ports)
{
    vtss_port_no_t port_no;
    
    JR_WR(ANA_CL_2, COMMON_VLAN_STAG_CFG, vtss_state->l2.vlan_conf.s_etype);
    
    /* Update VLAN port configuration */
    for (port_no = VTSS_PORT_NO_START; ports && port_no < vtss_state->port_count; port_no++) {
        if (VTSS_PORT_CHIP_SELECTED(port_no)) {
            VTSS_RC(vtss_cmn_vlan_port_conf_set(vtss_state, port_no));
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_vlan_conf_set(vtss_state_t *vtss_state)
{
    vtss_chip_no_t chip_no;
    
    /* Setup VLAN for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_vlan_conf_set_chip(vtss_state, 1));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_vlan_mask_apply(vtss_state_t *vtss_state, vtss_vid_t vid, u32 mask)
{
    vtss_vlan_entry_t *vlan_entry = &vtss_state->l2.vlan_table[vid];

    /* VLAN mask */
    JR_WRX(ANA_L3_2, VLAN_VLAN_MASK_CFG, vid, mask);

    /* Isolation and FID */
    JR_WRXM(ANA_L3_2, VLAN_VLAN_CFG, vid, 
            VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_FID(vid) |
            (vlan_entry->conf.learning ? 0 : VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_LRN_DIS) |
            (vlan_entry->isolated ? VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_PRIVATE_ENA : 0) |
            (vlan_entry->conf.mirror ? VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_MIRROR_ENA : 0),
            VTSS_M_ANA_L3_2_VLAN_VLAN_CFG_VLAN_FID |
            VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_LRN_DIS |
            VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_PRIVATE_ENA |
            VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_MIRROR_ENA);
    
    return VTSS_RC_OK;
}

static vtss_rc jr_vlan_mask_update(vtss_state_t *vtss_state,
                                   vtss_vid_t vid, BOOL member[VTSS_PORT_ARRAY_SIZE]) 
{
    vtss_chip_no_t chip_no;
    u32            mask;

    /* Update VLAN mask for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        mask = vtss_jr1_port_mask(vtss_state, member);
        if (vtss_state->chip_count == 2) {
            /* Include internal ports in all VLAN masks */
            mask |= vtss_state->port.mask_int_ports;
        }
        VTSS_RC(jr_vlan_mask_apply(vtss_state, vid, mask));
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_vlan_port_conf_apply(vtss_state_t *vtss_state,
                                      u32 port, vtss_vlan_port_conf_t *conf, vtss_qos_port_conf_t *qp_conf)
{
    BOOL       aware = 1, c_port = 0, s_port = 0, s_custom_port = 0, tagged, untagged, vid_ena;
    vtss_vid_t uvid = conf->untagged_vid;
    u32        tag_mode, mask, value = 0, tpid = 0;
    u32        etype = vtss_state->l2.vlan_conf.s_etype;


    /* Check port type */
    switch (conf->port_type) {
    case VTSS_VLAN_PORT_TYPE_UNAWARE:
        aware = 0;
        break;
    case VTSS_VLAN_PORT_TYPE_C:
        c_port = 1;
        break;
    case VTSS_VLAN_PORT_TYPE_S:
        s_port = 1;
        tpid = 1;
        break;
    case VTSS_VLAN_PORT_TYPE_S_CUSTOM:
        s_port = 1;
        s_custom_port = 1;
        tpid = 2;
        break;
    default:
        return VTSS_RC_ERROR;
    }
    vid_ena = (aware ? 0 : 1);

#if defined(VTSS_FEATURE_VSTAX_V2)
    {
        vtss_vstax_chip_info_t *chip_info = &vtss_state->l2.vstax_info.chip_info[vtss_state->chip_no];
        if (VTSS_BIT(port) & (chip_info->mask_a | chip_info->mask_b)) {
            /* Stacking is enabled on port */
            aware = 0;           /* Avoid popping tags on ingress */
            uvid = VTSS_VID_ALL; /* Avoid pushing tags on egress */
            vid_ena = 0;         /* Avoid changing VLAN classification */
        }
    }
#endif /* VTSS_FEATURE_VSTAX_V2 */

    /* VLAN popping is done using default actions in IS0 */
    /* BZ 4301: For VLAN unaware ports, the PVID is setup in IS0 */
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS0, port, VTSS_TCAM_CMD_READ));
    JR_WRF(VCAP_IS0, BASETYPE_ACTION_A, VLAN_POP_CNT, aware);
    JR_WRB(VCAP_IS0, BASETYPE_ACTION_A, VID_ENA, vid_ena);
    JR_WRF(VCAP_IS0, BASETYPE_ACTION_B, VID_VAL, conf->pvid);
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS0, port, VTSS_TCAM_CMD_WRITE));

    /* BZ 8810: Ports are always VLAN aware */
    /* Disable tag classification in chip if configured as VLAN unaware, even if tag classification is enabled in QoS */
    if (qp_conf) {
        if (aware && qp_conf->tag_class_enable) {
            JR_WRXM(ANA_CL_2, PORT_QOS_CFG, port,
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_QOS_ENA      |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_DP_ENA       |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_ITAG_UPRIO_QOS_ENA |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_ITAG_UPRIO_DP_ENA,
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_QOS_ENA      |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_DP_ENA       |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_ITAG_UPRIO_QOS_ENA |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_ITAG_UPRIO_DP_ENA);
        } else {
            JR_WRXM(ANA_CL_2, PORT_QOS_CFG, port,
                    0,
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_QOS_ENA      |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_DP_ENA       |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_ITAG_UPRIO_QOS_ENA |
                    VTSS_F_ANA_CL_2_PORT_QOS_CFG_ITAG_UPRIO_DP_ENA);
        }
    }

    /* Port VLAN Configuration */
    /* BZ 4301: Ports are always VLAN aware to allow IS1 to operate on tags */
    JR_WRXM(ANA_CL_2, PORT_VLAN_CTRL, port,
            VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_PORT_VID(conf->pvid) |
            VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_VLAN_AWARE_ENA,
            VTSS_M_ANA_CL_2_PORT_VLAN_CTRL_PORT_VID |
            VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_VLAN_AWARE_ENA);

    /* Drop Configuration based on port type and frame type */
    tagged = (conf->frame_type == VTSS_VLAN_FRAME_TAGGED);
    untagged = (conf->frame_type == VTSS_VLAN_FRAME_UNTAGGED);
    if (tagged && aware) {
        /* Discard untagged and priority-tagged if aware and tagged-only allowed */
        value |= VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_VLAN_UNTAG_DIS;
        value |= VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_PRIO_CTAG_DIS;
        value |= VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_PRIO_STAG_DIS;
    }
    if ((untagged && c_port) || (tagged && s_port)) {
        /* Discard C-tagged if C-port and untagged-only OR S-port and tagged-only */
        value |= VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_CTAG_DIS;
    }
    if ((untagged && s_port) || (tagged && c_port)) {
        /* Discard S-tagged if S-port and untagged-only OR C-port and tagged-only */
        value |= VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_STAG_DIS;
    }
    JR_WRXM(ANA_CL_2, PORT_FILTER_CTRL, port, value,
            VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_VLAN_UNTAG_DIS |
            VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_PRIO_CTAG_DIS |
            VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_CTAG_DIS |
            VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_PRIO_STAG_DIS |
            VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_STAG_DIS);
            
    /* Custom Ethernet Type */
    mask = (1 << port);
    JR_WRXM(ANA_CL_2, COMMON_CUSTOM_STAG_ETYPE_CTRL, 0, s_custom_port ? mask : 0, mask);

    /* Ingress filtering */
    JR_WRM(ANA_L3_2, COMMON_VLAN_FILTER_CTRL, conf->ingress_filter ? mask : 0, mask);

    /* BZ 7231: For dual Jaguar systems, tagging all frames, except VID zero is supported.
       The EVC feature requires that tagging VID zero is supported */
    tag_mode = (uvid == VTSS_VID_ALL ? 0 : uvid == VTSS_VID_NULL ? 
                (vtss_state->chip_count == 2 ? 2 : 3) : 1);
    
    /* Rewriter VLAN tag configuration */
    JR_WRXM(REW, PORT_TAG_CTRL, port,
            JR_PUT_FLD(REW, PORT_TAG_CTRL, TAG_MODE, tag_mode) |
            JR_PUT_FLD(REW, PORT_TAG_CTRL, TPI_SEL, tpid) |
            JR_PUT_FLD(REW, PORT_TAG_CTRL, VID_SEL, 2),
            JR_MSK(REW, PORT_TAG_CTRL, TAG_MODE) |
            JR_MSK(REW, PORT_TAG_CTRL, TPI_SEL) |
            JR_MSK(REW, PORT_TAG_CTRL, VID_SEL));
            JR_WRXM(REW, PORT_PORT_TAG_DEFAULT, port, 
            JR_PUT_FLD(REW, PORT_PORT_TAG_DEFAULT, PORT_TCI_VID, uvid) |
            JR_PUT_FLD(REW, PORT_PORT_TAG_DEFAULT, PORT_TPI_ETYPE, etype),
            JR_MSK(REW, PORT_PORT_TAG_DEFAULT, PORT_TCI_VID) |
            JR_MSK(REW, PORT_PORT_TAG_DEFAULT, PORT_TPI_ETYPE));

    return VTSS_RC_OK;
}

static vtss_rc jr_vlan_port_conf_update(vtss_state_t *vtss_state,
                                        vtss_port_no_t port_no, vtss_vlan_port_conf_t *conf)
{
    /* Update maximum tags allowed */
    VTSS_RC(vtss_jr1_port_max_tags_set(vtss_state, port_no));

    /* Update VLAN port configuration */
    return vtss_jr1_vlan_port_conf_apply(vtss_state, VTSS_CHIP_PORT(port_no), conf, &vtss_state->qos.port_conf[port_no]);
}

static vtss_rc jr_vcl_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    BOOL dmac_ena = vtss_state->l2.vcl_port_conf[port_no].dmac_dip;
    u32  port = VTSS_CHIP_PORT(port_no);

    /* DMAC/DIP use in IS1 is enabled using default actions in IS0 */
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS0, port, VTSS_TCAM_CMD_READ));
    JR_WRB(VCAP_IS0, BASETYPE_ACTION_A, S1_DMAC_ENA, dmac_ena);
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS0, port, VTSS_TCAM_CMD_WRITE));
    return VTSS_RC_OK;
}

static vtss_rc jr_isolated_port_members_set_chip(vtss_state_t *vtss_state)
{
    JR_WR(ANA_L3_2, COMMON_VLAN_ISOLATED_CFG, vtss_jr1_port_mask(vtss_state, vtss_state->l2.isolated_port));
    return VTSS_RC_OK;
}

static vtss_rc jr_isolated_port_members_set(vtss_state_t *vtss_state)
{
    return vtss_jr1_conf_chips(vtss_state, jr_isolated_port_members_set_chip);
}

static vtss_rc jr_aggr_mode_set_chip(vtss_state_t *vtss_state)
{
    vtss_aggr_mode_t *mode = &vtss_state->l2.aggr_mode;

    JR_WRB(ANA_CL_2, COMMON_AGGR_CFG, AGGR_SMAC_ENA, mode->smac_enable);
    JR_WRB(ANA_CL_2, COMMON_AGGR_CFG, AGGR_DMAC_ENA, mode->dmac_enable);
    JR_WRB(ANA_CL_2, COMMON_AGGR_CFG, AGGR_IP4_SIPDIP_ENA, mode->sip_dip_enable);
    JR_WRB(ANA_CL_2, COMMON_AGGR_CFG, AGGR_IP4_TCPUDP_PORT_ENA, mode->sport_dport_enable);
    JR_WRB(ANA_CL_2, COMMON_AGGR_CFG, AGGR_IP6_TCPUDP_PORT_ENA, mode->sport_dport_enable);

    return VTSS_RC_OK;
}

static vtss_rc jr_aggr_mode_set(vtss_state_t *vtss_state)
{
    return vtss_jr1_conf_chips(vtss_state, jr_aggr_mode_set_chip);
}

/* Based on mux mode, return unused port for EEE loopback and host mode purposes */
u32 vtss_jr1_unused_chip_port(vtss_state_t *vtss_state)
{
    return (vtss_state->port.mux_mode[vtss_state->chip_no] == VTSS_PORT_MUX_MODE_0 ? 26 : 27);
}

/* Find first port enabled in mask */
static u32 jr_first_port(u32 mask)
{
    u32 i, port = 0;

    for (i = 0; i < 32; i++) {
        if (mask & VTSS_BIT(i)) {
            port = i;
            break;
        }
    }
    return port;
}

/* Setup mirroring for chip */
static vtss_rc jr_mirror_conf_set_chip(vtss_state_t *vtss_state)
{
    vtss_mirror_conf_t     *conf = &vtss_state->l2.mirror_conf;
    vtss_port_no_t         port_no = conf->port_no;
    u32                    mask = 0, port, tpid, value;
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_vstax_chip_info_t *chip_info = &vtss_state->l2.vstax_info.chip_info[vtss_state->chip_no];
        
    /* Include stack ports and mirror port in GMIRROR mask */
    if (chip_info->port_conf[0].mirror)
        mask |= chip_info->mask_a;
    if (chip_info->port_conf[1].mirror || 
        (vtss_state->chip_count == 2 && port_no != VTSS_PORT_NO_NONE && 
         !VTSS_PORT_CHIP_SELECTED(port_no))) {
        /* For dual device targets, the internal ports are STACK_B.
           Mirroring is enabled if the mirror port is on the other device */
        mask |= chip_info->mask_b;
    }
#endif /* VTSS_FEATURE_VSTAX_V2 */
    /* Include mirror port or internal ports in GMIRROR mask */
    mask |= (port_no == VTSS_PORT_NO_NONE ? 0 :
             VTSS_PORT_CHIP_SELECTED(port_no) ? VTSS_BIT(VTSS_CHIP_PORT(port_no)) :
             vtss_state->port.mask_int_ports);
    JR_WR(ANA_AC, PS_COMMON_VSTAX_GMIRROR_CFG, mask);
    
    /* Arbiter: The first enabled mirror port is setup */
    port = jr_first_port(mask);
    JR_WRX(ARB, CFG_STATUS_MIRROR_CFG, JR_MIRROR_PROBE_RX,
           VTSS_F_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_CFG(port));
    JR_WRX(ARB, CFG_STATUS_MIRROR_CFG, JR_MIRROR_PROBE_TX,
           VTSS_F_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_CFG(port) |
           VTSS_F_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_TYPE);

    /* Ingress mirroring, only enabled if mirror port enabled */
    JR_WRX(ANA_AC, MIRROR_PROBE_PROBE_CFG, JR_MIRROR_PROBE_RX, 
           VTSS_F_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_DIRECTION(2));
    JR_WRX(ANA_AC, MIRROR_PROBE_PROBE_PORT_CFG, JR_MIRROR_PROBE_RX,
           mask ? vtss_jr1_port_mask(vtss_state, vtss_state->l2.mirror_ingress) : 0);
    
    /* Egress mirroring, only enabled if mirror port enabled. Probe mode Tx (1) */
    if (mask != 0)
        mask = vtss_jr1_port_mask(vtss_state, vtss_state->l2.mirror_egress);
    JR_WRX(ANA_AC, MIRROR_PROBE_PROBE_CFG, JR_MIRROR_PROBE_TX, 
           VTSS_F_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_DIRECTION(1));
    JR_WRX(ANA_AC, MIRROR_PROBE_PROBE_PORT_CFG, JR_MIRROR_PROBE_TX, mask);
    
    /* Setup mirror tagging */
    tpid = (conf->tag == VTSS_MIRROR_TAG_S ? 1 : conf->tag == VTSS_MIRROR_TAG_S_CUSTOM ? 2 : 0);
    value = (VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_UPRIO(conf->pcp) |
             (conf->dei ? VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_CFI : 0) |
             VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_VID(conf->vid) |
             VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_TPI(tpid) |
             VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_USE_MIRROR_UPRIO_ENA |
             (conf->tag == VTSS_MIRROR_TAG_NONE ? 0 : 
              VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_REMOTE_MIRROR_ENA));
    JR_WRX(REW, COMMON_MIRROR_PROBE_CFG, JR_MIRROR_PROBE_RX, value);

    /* The first port enabled for egress mirroring is setup in Rewriter */
    port = jr_first_port(mask);
    JR_WRX(REW, COMMON_MIRROR_PROBE_CFG, JR_MIRROR_PROBE_TX,
           value | VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_TX_PORT(port));

    return VTSS_RC_OK;
}

static vtss_rc jr_mirror_port_set_chip(vtss_state_t *vtss_state)
{
    /* Setup mirroring for all devices */
    return jr_mirror_conf_set_chip(vtss_state);
}

static vtss_rc jr_mirror_port_set(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_no;

    /* Update all VLANs */
    VTSS_RC(vtss_cmn_vlan_update_all(vtss_state));

    /* Update VLAN port configuration for all ports */
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        VTSS_SELECT_CHIP_PORT_NO(port_no);
        VTSS_RC(vtss_cmn_vlan_port_conf_set(vtss_state, port_no));
    }

    /* Setup mirroring for all devices */
    return vtss_jr1_conf_chips(vtss_state, jr_mirror_port_set_chip);
}

static vtss_rc jr_mirror_ingress_set(vtss_state_t *vtss_state)
{
    /* Setup mirroring for all devices */
    return vtss_jr1_conf_chips(vtss_state, jr_mirror_conf_set_chip);
}

static vtss_rc jr_mirror_egress_set(vtss_state_t *vtss_state)
{
    /* Setup mirroring for all devices */
    return vtss_jr1_conf_chips(vtss_state, jr_mirror_conf_set_chip);
}

vtss_rc vtss_jr1_src_table_write_chip(vtss_state_t *vtss_state, u32 port, u32 mask)
{
    JR_WRX(ANA_AC, SRC_SRC_CFG, port, mask);
    return VTSS_RC_OK;
}

static vtss_rc jr_src_table_write(vtss_state_t *vtss_state,
                                  vtss_port_no_t port_no, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    u32            mask, port = VTSS_CHIP_PORT(port_no);
    vtss_chip_no_t chip_no;

    VTSS_SELECT_CHIP_PORT_NO(port_no);
    mask = vtss_jr1_port_mask(vtss_state, member);
    if (vtss_state->chip_count == 2) {
        /* Include internal ports in source masks if any port on the other device is included */
        chip_no = vtss_state->chip_no;
        VTSS_SELECT_CHIP(chip_no ? 0 : 1);
        if (vtss_jr1_port_mask(vtss_state, member)) {
            mask |= vtss_state->port.mask_int_ports;
        }
        VTSS_SELECT_CHIP(chip_no);
    }
    
    return vtss_jr1_src_table_write_chip(vtss_state, port, mask);
}

vtss_rc vtss_jr1_pgid_table_write_chip(vtss_state_t *vtss_state,
                                       u32 pgid, u32 mask, BOOL cpu_copy, 
                                       vtss_packet_rx_queue_t cpu_queue, BOOL stack_type_ena)
{
    JR_WRX(ANA_AC, PGID_PGID_CFG_0, pgid, mask);
    JR_WRX(ANA_AC, PGID_PGID_CFG_1, pgid, 
           JR_PUT_BIT(ANA_AC, PGID_PGID_CFG_1, PGID_CPU_COPY_ENA, cpu_copy) |
           JR_PUT_BIT(ANA_AC, PGID_PGID_CFG_1, STACK_TYPE_ENA, stack_type_ena) |
           JR_PUT_FLD(ANA_AC, PGID_PGID_CFG_1, PGID_CPU_QU, cpu_queue));
    return VTSS_RC_OK;
}

static vtss_rc jr_pgid_table_write(vtss_state_t *vtss_state,
                                   u32 pgid, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    vtss_chip_no_t    chip_no;
    vtss_pgid_entry_t *pgid_entry = &vtss_state->l2.pgid_table[pgid];
    u32               mask, i = vtss_jr1_chip_pgid(vtss_state, pgid);

    /* Setup PGID masks for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        mask = vtss_jr1_port_mask(vtss_state, member);
        if (i < VTSS_CHIP_PORTS) {
            /* UC entry */
            if (!VTSS_PORT_CHIP_SELECTED(pgid)) {
                /* Only setup UC mask if port is on selected chip */
                continue;
            }
        } else if (vtss_state->chip_count == 2) {
            /* MC entry, include internal ports in mask */
            mask |= vtss_state->port.mask_int_ports;
        }
        VTSS_RC(vtss_jr1_pgid_table_write_chip(vtss_state, i, mask, pgid_entry->cpu_copy, 
                                               pgid_entry->cpu_queue, 0));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_aggr_table_write(vtss_state_t *vtss_state,
                                   u32 ac, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    vtss_chip_no_t chip_no;
    u32            mask;

    /* Setup aggregation mask for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        mask = vtss_jr1_port_mask(vtss_state, member);
        if (vtss_state->chip_count == 2) {
            /* Include one internal port in each aggregation mask */
            mask |= VTSS_BIT(ac & 1 ? vtss_state->port.port_int_0 : vtss_state->port.port_int_1);
        }
        // The following is required by EEE, and doesn't harm if EEE is not enabled or not included.
        mask |= VTSS_BIT(vtss_jr1_unused_chip_port(vtss_state));
        JR_WRX(ANA_AC, AGGR_AGGR_CFG, ac, mask);
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_pmap_table_write(vtss_state_t *vtss_state,
                                   vtss_port_no_t port_no, vtss_port_no_t l_port_no)
{
    BOOL glag_ena=0;
    u32 glag_no=0;
    
    VTSS_SELECT_CHIP_PORT_NO(port_no);
#if defined(VTSS_FEATURE_AGGR_GLAG )
    if ((vtss_state->l2.port_glag_no[port_no]) != VTSS_GLAG_NO_NONE) {
        glag_ena = 1;
        glag_no = vtss_state->l2.port_glag_no[port_no];
    }
#endif    
    JR_WRX(ANA_CL_2, PORT_PORT_ID_CFG, VTSS_CHIP_PORT(port_no),
           VTSS_F_ANA_CL_2_PORT_PORT_ID_CFG_LPORT_NUM(VTSS_CHIP_PORT(l_port_no)) |
           (glag_ena ? VTSS_F_ANA_CL_2_PORT_PORT_ID_CFG_PORT_IS_GLAG_ENA : 0) |
           VTSS_F_ANA_CL_2_PORT_PORT_ID_CFG_GLAG_NUM(glag_no));

    return VTSS_RC_OK;
}

/* Update PGID state for reserved entry and update chip */
static vtss_rc jr_pgid_update(vtss_state_t *vtss_state,
                              u32 pgid, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    vtss_port_no_t    port_no;
    vtss_pgid_entry_t *pgid_entry;
    
    pgid = vtss_jr1_vtss_pgid(vtss_state, pgid);
    pgid_entry = &vtss_state->l2.pgid_table[pgid];
    pgid_entry->resv = 1;
    pgid_entry->references = 1;
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++)
        pgid_entry->member[port_no] = member[port_no];
    
    return jr_pgid_table_write(vtss_state, pgid, member);
}

static vtss_rc jr_flood_conf_set(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_no;
    BOOL           member[VTSS_PORT_ARRAY_SIZE];

    /* Unicast flood mask */
    VTSS_RC(jr_pgid_update(vtss_state, PGID_UC_FLOOD, vtss_state->l2.uc_flood));

    /* Multicast flood mask */
    VTSS_RC(jr_pgid_update(vtss_state, PGID_MC_FLOOD, vtss_state->l2.mc_flood));

    /* IPv4 multicast control flood mask */
    VTSS_RC(jr_pgid_update(vtss_state, PGID_IPV4_MC_CTRL, vtss_state->l2.mc_flood));

    /* IPv4 multicast data flood mask */
    VTSS_RC(jr_pgid_update(vtss_state, PGID_IPV4_MC_DATA, vtss_state->l2.ipv4_mc_flood));

    /* IPv6 multicast control flood mask */
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++)
        member[port_no] = (vtss_state->l2.ipv6_mc_scope ? vtss_state->l2.ipv6_mc_flood[port_no] : 
                           vtss_state->l2.mc_flood[port_no]);
    VTSS_RC(jr_pgid_update(vtss_state, PGID_IPV6_MC_CTRL, member));
    
    /* Setup IPv6 MC data flood mask */
    return jr_pgid_update(vtss_state, PGID_IPV6_MC_DATA, vtss_state->l2.ipv6_mc_flood);
}

#if defined(VTSS_FEATURE_AGGR_GLAG)
static vtss_rc jr_glag_src_table_write(vtss_state_t *vtss_state,
                                       u32 glag_no, u32 port_count, 
                                       BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    vtss_chip_no_t chip_no;
    u32            mask;

    if (port_count)
        port_count--;
    
    /* Update GLAG source mask and port member count for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        mask = vtss_jr1_port_mask(vtss_state, member);
        if (vtss_state->chip_count == 2) {
            /* Include internal ports in GLAG source mask */
            mask |= vtss_state->port.mask_int_ports;
        }
        JR_WRX(ANA_AC, GLAG_GLAG_CFG, glag_no, mask);   
        JR_WRXF(ANA_AC, GLAG_MBR_CNT_CFG, glag_no, GLAG_MBR_CNT, port_count); 
    }    
    return VTSS_RC_OK;
}

static vtss_rc jr_glag_port_write(vtss_state_t *vtss_state,
                                  u32 glag_no, u32 idx, vtss_vstax_glag_entry_t *entry)
{
    vtss_chip_no_t chip_no;
    u32            value;
    
    /* Update GLAG port entry for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        value = ((entry->upsid << 6) + entry->upspn); /* UPSID at bit 6:10, UPSPN at bit 0:4 */
        if (vtss_state->l2.vstax_info.upsid[vtss_state->chip_no] == entry->upsid)
            value += (entry->upspn << 16); /* If local UPSID, UPSPN at bit 16:20 */
        VTSS_RC(vtss_jr1_pgid_table_write_chip(vtss_state,
                                               PGID_GLAG_START + glag_no*VTSS_GLAG_PORTS + idx, 
                                               value, 0, 0, 1));
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_AGGR_GLAG */

#if defined(VTSS_FEATURE_VLAN_COUNTERS)
/* Read and update ANA_AC/REW counters */
#define JR_VLAN_CNT_GET(tgt, sdx, idx, id, cnt, clr)                            \
{                                                                               \
    u32 lsb = 0, msb = 0, temp_lsb, temp_msb;                                   \
    vtss_chip_no_t              chip_no;                                        \
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {            \
        VTSS_SELECT_CHIP(chip_no);                                              \
        JR_RDXY(tgt, STAT_CNT_CFG_##sdx##_STAT_LSB_CNT, idx, id, &temp_lsb);    \
        JR_RDXY(tgt, STAT_CNT_CFG_##sdx##_STAT_MSB_CNT, idx, id/2, &temp_msb);  \
        lsb += temp_lsb;                                                        \
        msb += temp_msb;                                                        \
    }                                                                           \
    vtss_cmn_counter_40_update(lsb, msb, cnt.bytes, clr);                       \
    lsb = 0;                                                                    \
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {            \
        VTSS_SELECT_CHIP(chip_no);                                              \
        JR_RDXY(tgt, STAT_CNT_CFG_##sdx##_STAT_LSB_CNT, idx, id + 1, &temp_lsb);\
        lsb += temp_lsb;                                                        \
    }                                                                           \
    vtss_cmn_counter_32_update(lsb, cnt.frames, clr);                           \
}
#define JR_VLAN_CNT_ADD(c, counters, name)          \
{                                                  \
    counters->name.frames += c->name.frames.value; \
    counters->name.bytes += c->name.bytes.value;   \
}

static vtss_rc jr_vlan_counters_update(vtss_state_t         *vtss_state,
                                       vtss_vid_t           vid,
                                       vtss_vlan_counters_t *const counters,
                                       BOOL                 clear)
{
    // #vid is in range [1; VTSS_VIDS - 1]
    vtss_vlan_chip_counters_t *pcnt = &vtss_state->l2.vlan_counters_info.counters[vid - 1];
    
    /* Read and update counters for all the chips */
    JR_VLAN_CNT_GET(ANA_AC, ISDX, vid, 0, &pcnt->rx_vlan_unicast, clear);
    JR_VLAN_CNT_GET(ANA_AC, ISDX, vid, 2, &pcnt->rx_vlan_multicast, clear);
    JR_VLAN_CNT_GET(ANA_AC, ISDX, vid, 4, &pcnt->rx_vlan_flood, clear);

    if (counters != NULL) {         
        /* Add counters */
        JR_VLAN_CNT_ADD(pcnt, counters, rx_vlan_unicast);
        JR_VLAN_CNT_ADD(pcnt, counters, rx_vlan_multicast);
        JR_VLAN_CNT_ADD(pcnt, counters, rx_vlan_flood);
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_vlan_counters_get(vtss_state_t *vtss_state,
                                    vtss_vid_t     vid,
                                    vtss_vlan_counters_t *counters)
{
    return jr_vlan_counters_update(vtss_state, vid, counters, FALSE);
}

static vtss_rc jr_vlan_counters_clear(vtss_state_t *vtss_state,
                                      vtss_vid_t vid)
{
    return jr_vlan_counters_update(vtss_state, vid, NULL, TRUE);
}
#endif /* VTSS_FEATURE_VLAN_COUNTERS */

/* ================================================================= *
 *  VSTAX (VSTAX2)
 * ================================================================= */

#if defined(VTSS_FEATURE_VSTAX_V2)
BOOL vtss_jr1_is_frontport(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return (vtss_state->l2.vstax_port_enabled[port_no] ? 0 : 1);
}


// CMEF min interval (i.e. max rate)
// 
// Values set such that rate will not exceed 100Mbps, regardless of topology.
// Max interval (i.e. periodic rate) is set to 20*min_interval.
// 
// I.e. periodic CMEFs will consume 5Mbps.
// Triggered CMEFs may *theoritically* consume up to 100Mbps.
// But in all realistic scenarios, CMEF will consume ~5Mbps.
//
// Background for selecting these values:
// max_interval cannot be set higher than 0xffff (16 bit register).
// This means that in a max stack of 32 units, CMEFs will (on some links)
// consume ~31*250e6/16/0xffff*576=4.3Mbps.
// So to keep things simple (for marketing), numbers are set such 
// that periodic CMEFs always consume max ~5Mbps.
//
// The max rate (100Mbps) is somewhat arbitrary.
//
// Spreadsheet used to calculate values:
// KT:/Documents/TGS_TargetSpecification/TGS1000-1099/TGS1000-Jaguar/Input/Features/x_Stacking-toe.xlsx
//
// First index:  Number of UPSIDs in use.
// Second index: 0: Ring, 1: Chain
static const int cmef_min_interval[VTSS_VSTAX_UPSIDS][2] = {
    {    0,    0 },
    {   90,   90 },
    {   90,  180 },
    {  180,  270 },
    {  180,  360 },
    {  270,  450 },
    {  270,  540 },
    {  360,  630 },
    {  360,  720 },
    {  450,  810 },
    {  450,  900 },
    {  540,  990 },
    {  540, 1080 },
    {  630, 1170 },
    {  630, 1260 },
    {  720, 1350 },
    {  720, 1440 },
    {  810, 1530 },
    {  810, 1620 },
    {  900, 1710 },
    {  900, 1800 },
    {  990, 1890 },
    {  990, 1980 },
    { 1080, 2070 },
    { 1080, 2160 },
    { 1170, 2250 },
    { 1170, 2340 },
    { 1260, 2430 },
    { 1260, 2520 },
    { 1350, 2610 },
    { 1350, 2700 },
    { 1440, 2790 },
};

static vtss_rc jr_update_lport(vtss_state_t *vtss_state, vtss_port_no_t port_no, i32 lport)
{
#if defined(VTSS_FEATURE_AGGR_GLAG)
    if ((vtss_state->l2.port_glag_no[port_no]) != VTSS_GLAG_NO_NONE) {
        vtss_port_no_t port_ix;
        u32 glag_no = vtss_state->l2.port_glag_no[port_no];
        for (port_ix = VTSS_PORT_NO_START; port_ix < vtss_state->port_count; port_ix++) {
            if (vtss_state->l2.port_glag_no[port_ix] == glag_no &&
                VTSS_CHIP_PORT(port_ix) < lport)
                lport = VTSS_CHIP_PORT(port_ix);
        }
    }
#endif    
    JR_WRX(DSM, CM_CFG_LPORT_NUM_CFG, VTSS_CHIP_PORT(port_no), lport);

    return VTSS_RC_OK;
}

#define CMEF_SHAPER_RATE_M(n)  (u32) (n * (1.5        * 1000000 / 100160))
#define CMEF_SHAPER_RATE_G(n)  (u32) (n * (1.5 * 1000 * 1000000 / 100160))

static u32 jr_cmef_shaper_rate(vtss_port_speed_t speed)
{
    switch (speed) {
    case VTSS_SPEED_10M:
        return CMEF_SHAPER_RATE_M(10);
    case VTSS_SPEED_100M:
        return CMEF_SHAPER_RATE_M(100);
    case VTSS_SPEED_1G:
        return CMEF_SHAPER_RATE_G(1);
    case VTSS_SPEED_2500M:
        return CMEF_SHAPER_RATE_G(2.5);
    case VTSS_SPEED_10G:
        return CMEF_SHAPER_RATE_G(10);
    case VTSS_SPEED_12G:
        return CMEF_SHAPER_RATE_G(12);
    default:;
    }
    return 0;
}

vtss_rc vtss_jr1_vstax_update_ingr_shaper(vtss_state_t *vtss_state,
                                          u32 chipport, vtss_port_speed_t speed, BOOL front_port)
{
    u32 ingr_rate = front_port ? jr_cmef_shaper_rate(speed) : 0;
    /* Set INGR_SHAPER_CFG */
    JR_WRXM(ARB, CFG_STATUS_INGR_SHAPER_CFG, chipport, 
            JR_PUT_FLD(ARB, CFG_STATUS_INGR_SHAPER_CFG, INGR_SHAPER_RATE, ingr_rate) |
            JR_PUT_FLD(ARB, CFG_STATUS_INGR_SHAPER_CFG, INGR_SHAPER_THRES, 1) | /* n * 4K */
            JR_PUT_FLD(ARB, CFG_STATUS_INGR_SHAPER_CFG, INGR_SHAPER_MODE, front_port ? 1 : 0), /* Enabled: MC once */
            JR_MSK(ARB, CFG_STATUS_INGR_SHAPER_CFG, INGR_SHAPER_RATE) |
            JR_MSK(ARB, CFG_STATUS_INGR_SHAPER_CFG, INGR_SHAPER_MODE) |
            JR_MSK(ARB, CFG_STATUS_INGR_SHAPER_CFG, INGR_SHAPER_THRES));
    return VTSS_RC_OK;
}

static vtss_rc jr_cmef_topology_change(vtss_state_t *vtss_state)
{
    JR_WR(DSM, CM_CFG_CMEF_UPSID_ACTIVE_CFG, vtss_state->l2.vstax_conf.cmef_disable ? 0 : vtss_state->l2.vstax_info.chip_info[vtss_state->chip_no].upsid_active);
    return VTSS_RC_OK;
}

static vtss_rc jr_vstax_topology_set_chip(vtss_state_t *vtss_state)
{
    u32                    upsid_active_mask = 0, upsid_active_count = 0;
    u32                    cmef_int_min, cmef_int_max;
    vtss_vstax_upsid_t     upsid;
    vtss_vstax_chip_info_t *chip_info = &vtss_state->l2.vstax_info.chip_info[vtss_state->chip_no];

    for (upsid = VTSS_VSTAX_UPSID_MIN; upsid <= VTSS_VSTAX_UPSID_MAX; upsid++) {
        vtss_vstax_route_entry_t *rt = &chip_info->rt_table.table[upsid];
        u32 mask = 0;
        if(rt->stack_port_a)
            mask |= chip_info->mask_a;
        if(rt->stack_port_b)
            mask |= chip_info->mask_b;
        JR_WRX(ANA_AC, UPSID_UPSID_CFG, upsid, mask);
        /* Unit routeable both ways? */
        if (rt->stack_port_a && rt->stack_port_b &&
            chip_info->rt_table.topology_type != VTSS_VSTAX_TOPOLOGY_RING) {
            VTSS_E("topology not ring, but equal cost routing on UPSID %d, chip_no=%d", 
                   upsid, vtss_state->chip_no);
        }
        JR_WRXB(ANA_AC, UPSID_STACK_LINK_EQUAL_COST_CFG, upsid, STACK_LINK_EQUAL_ENA,
                (rt->stack_port_a && rt->stack_port_b));
        /* Track active UPSes, i.e. reachable through stack port or ourself */
        if (mask || upsid == vtss_state->l2.vstax_info.upsid[vtss_state->chip_no]) {
            upsid_active_mask |= VTSS_BIT(upsid);
            upsid_active_count++;
        }
    }
    if (chip_info->upsid_active != upsid_active_mask) {
        chip_info->upsid_active = upsid_active_mask;
        VTSS_RC(jr_cmef_topology_change(vtss_state));
    }

    // A topology change may affect the redirection of CPU queues */
    VTSS_RC(vtss_jr1_cpu_queue_redirect_chip(vtss_state));

    /* CMEF Rates */
    cmef_int_min = cmef_min_interval[upsid_active_count-1][chip_info->rt_table.topology_type == VTSS_VSTAX_TOPOLOGY_RING ? 0 : 1];
    cmef_int_max = cmef_int_min * 20;
    if (cmef_int_min > 0xffff || cmef_int_max > 0xffff) {
        VTSS_E("Invalid CMEF interval(s): %u %u (upsid_active_count=%u)",
               cmef_int_min, cmef_int_max, upsid_active_count);
    }
    JR_WR(DSM, CM_CFG_CMEF_RATE_CFG,
          JR_PUT_FLD(DSM, CM_CFG_CMEF_RATE_CFG, CMEF_MIN_INTERVAL, cmef_int_min) |
          JR_PUT_FLD(DSM, CM_CFG_CMEF_RATE_CFG, CMEF_MAX_INTERVAL, cmef_int_max));

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_vstax_conf_set_chip(vtss_state_t *vtss_state)
{
    vtss_vstax_info_t      *vstax_info = &vtss_state->l2.vstax_info;
    vtss_vstax_chip_info_t *chip_info = &vstax_info->chip_info[vtss_state->chip_no];
    vtss_vstax_upsid_t     upsid = vstax_info->upsid[vtss_state->chip_no];
    u32                    stack_mask = (chip_info->mask_a | chip_info->mask_b);
    u32                    port, stack_port, rvalue, rmask, max_ttl = 0, ttl = 0;
    BOOL                   enable, front_port;
    BOOL                   stacking = (stack_mask ? 1 : 0);
    BOOL                   cmef_enable = (vtss_state->l2.vstax_conf.cmef_disable ? 0 : 1);
    vtss_port_no_t         port_no;

    /* UPSID setup */
    JR_WRF(ANA_CL_2, COMMON_UPSID_CFG, UPSID_NUM, upsid);
    JR_WRF(ANA_L2, COMMON_VSTAX_CTRL, OWN_UPSID, upsid);
    JR_WRF(ASM, SP_CFG_SP_UPSID_CFG, OWN_UPSID, upsid);
    JR_WRF(ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, OWN_UPSID, upsid);
    JR_WRB(ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, VSTAX2_MC_LLOOKUP_NON_FLOOD_ENA, 1); /* Use local lookup for non-flooded MC frames */
    JR_WRB(ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, VSTAX2_MC_LLOOKUP_ENA, 0);  /* BZ 6702: Do not use local lookup for flooded MC frames */
    JR_WRB(ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, VSTAX2_GLAG_ENA, 1); /* Enable GLAGs in VSTAX */
    JR_WRF(DSM, CM_CFG_CMEF_OWN_UPSID_CFG, CMEF_OWN_UPSID, upsid);

    for (port = JR_STACK_PORT_START; port < JR_STACK_PORT_END; port++) {
        /* Disable CMEF while fiddling with Rx and Tx buffers. */
        stack_port = port - JR_STACK_PORT_START;
        JR_WRXM(DSM, CM_CFG_CMEF_GEN_CFG, stack_port, 0, VTSS_F_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_RELAY_TX_ENA | VTSS_F_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_GEN_ENA);
        JR_WRXB(ASM, CM_CFG_CMEF_RX_CFG, stack_port, CMEF_RX_ENA, 0);
    }

    /* Per-stackable port setup */
    for (port = JR_STACK_PORT_START; port < JR_STACK_PORT_END; port++) {
        BOOL is_stack_a, is_stack_b, is_stack_port;
        is_stack_a    = (VTSS_BIT(port) & chip_info->mask_a) ? 1 : 0;
        is_stack_b    = (VTSS_BIT(port) & chip_info->mask_b) ? 1 : 0;
        is_stack_port = is_stack_a || is_stack_b;
        stack_port    = port - JR_STACK_PORT_START;

        rmask  = VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_FWD_CELL_BUS_ENA | VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_RX_SEL | VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_RX_ENA;
        /* Enable super-prio Rx on stack ports, except for interconnect port #1 and not on rev. A chips. */
        rvalue = ((vtss_state->misc.chip_id.revision != 0) && is_stack_port && (vtss_state->chip_count != 2 || port != vtss_state->port.port_int_1)) ? VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_RX_ENA : 0;
        /* SP_RX_SEL == 1 on external stack ports only in 48-port solutions, and on stack port 0 (A) in 24-port solutions. */
        rvalue |= is_stack_a ? VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_RX_SEL : 0;
        /* Due to a chip-bug in Jaguar 1 rev. A, it's not
         * recommended to enable super-priority Rx. Instead, use
         * analyzer to get CPU frames to the CPU.
         * (VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_FWD_CELL_BUS_ENA)
         */
        rvalue |= (vtss_state->misc.chip_id.revision == 0 && is_stack_port) ? VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_FWD_CELL_BUS_ENA : 0;
        JR_WRXM(ASM, SP_CFG_SP_RX_CFG, stack_port, rvalue, rmask);

        rmask = VTSS_F_DSM_SP_CFG_SP_TX_CFG_SP_TX_SEL | VTSS_F_DSM_SP_CFG_SP_TX_CFG_SP_TX_ENA;
        /* SP_TX_SEL = 1 on interconnect ports (stack_b) and 0 on external stack ports (stack_a) in 48-port solutions.
           SP_TX_SEL = 1 on stack_b port and 0 on stack_a port in 24-port solutions. */
        rvalue = is_stack_b ? VTSS_F_DSM_SP_CFG_SP_TX_CFG_SP_TX_SEL : 0;
        /* SP_TX_ENA = 1 on all stack ports (external as well as interconnects), except for interconnect #1,
           since it's used for CMEFs. */
        rvalue |= (is_stack_port && (vtss_state->chip_count != 2 || port != vtss_state->port.port_int_1)) ? VTSS_F_DSM_SP_CFG_SP_TX_CFG_SP_TX_ENA : 0;
        JR_WRXM(DSM, SP_CFG_SP_TX_CFG, stack_port, rvalue, rmask);

        /* TTL */
        if (is_stack_a) {
            JR_WRXF(REW, COMMON_VSTAX_PORT_GRP_CFG, 0, VSTAX_TTL, chip_info->port_conf[0].ttl);
        }
        if (is_stack_b) {
            JR_WRXF(REW, COMMON_VSTAX_PORT_GRP_CFG, 1, VSTAX_TTL, chip_info->port_conf[1].ttl);
        }

        /* Set REW:PHYSPORT[chip_port].VSTAX_STACK_GRP_SEL */
        rmask  = VTSS_F_REW_PHYSPORT_VSTAX_CTRL_VSTAX_HDR_ENA | VTSS_F_REW_PHYSPORT_VSTAX_CTRL_VSTAX_STACK_GRP_SEL;
        rvalue = is_stack_port ? VTSS_F_REW_PHYSPORT_VSTAX_CTRL_VSTAX_HDR_ENA : 0;
        if (is_stack_b) {
            rvalue |= VTSS_F_REW_PHYSPORT_VSTAX_CTRL_VSTAX_STACK_GRP_SEL; /* Stack Port "B" */
        }
        JR_WRXM(REW, PHYSPORT_VSTAX_CTRL, port, rvalue, rmask);

        /* ANA_CL:PORT[chip_port].PORT_STACKING_CTRL */
#ifdef VTSS_FEATURE_NPI
        if (is_stack_port || !vtss_state->packet.npi_conf.enable || !VTSS_PORT_CHIP_SELECTED(vtss_state->packet.npi_conf.port_no) || VTSS_CHIP_PORT(vtss_state->packet.npi_conf.port_no) != port)
#endif
        {
            rmask = VTSS_F_ANA_CL_2_PORT_STACKING_CTRL_STACKING_AWARE_ENA | VTSS_F_ANA_CL_2_PORT_STACKING_CTRL_STACKING_NON_HEADER_DISCARD_ENA;
            JR_WRXM(ANA_CL_2, PORT_STACKING_CTRL, port, is_stack_port ? rmask : 0, rmask);
            JR_WRXB(ANA_CL_2, PORT_STACKING_CTRL, port, STACKING_HEADER_DISCARD_ENA, is_stack_port ? 0 : 1);
        }

        if (is_stack_port) { /* Bugzilla#6918 */
            int pcp, dei;

            /* Analyzer: Always use PCP and DEI for QoS and DP level classification on stack ports */
            JR_WRXB(ANA_CL_2, PORT_QOS_CFG, port, UPRIO_QOS_ENA,      1);
            JR_WRXB(ANA_CL_2, PORT_QOS_CFG, port, UPRIO_DP_ENA,       1);
            JR_WRXB(ANA_CL_2, PORT_QOS_CFG, port, ITAG_UPRIO_QOS_ENA, 1);
            JR_WRXB(ANA_CL_2, PORT_QOS_CFG, port, ITAG_UPRIO_DP_ENA,  1);

            /* Map for (PCP and DEI) to (QoS class and DP level) */
            for (pcp = VTSS_PCP_START; pcp < VTSS_PCP_END; pcp++) {
                for (dei = VTSS_DEI_START; dei < VTSS_DEI_END; dei++) {
                    JR_WRXYF(ANA_CL_2, PORT_UPRIO_MAP_CFG, port, (8*dei + pcp), UPRIO_CFI_TRANSLATE_QOS_VAL, vtss_cmn_pcp2qos(pcp));
                    JR_WRXYF(ANA_CL_2, PORT_DP_CONFIG,     port, (8*dei + pcp), UPRIO_CFI_DP_VAL,            dei);
                }
            }
        }
    }
    VTSS_RC(jr_cmef_topology_change(vtss_state));

    if (cmef_enable) {
        JR_WR(DSM, CM_CFG_CMM_MASK_CFG, ~stack_mask);
    } else {
        JR_WR(DSM, CM_CFG_CMM_MASK_CFG, 0);
        /* We also must clear status when disabling (instance 0) */
        JR_WRX(DSM, CM_STATUS_CURRENT_CM_STATUS, 0, 0);
    }

    /* No ISDX in VStaX header */
    JR_WRB(ANA_CL_2, COMMON_IFH_CFG, VSTAX_ISDX_ENA, !stacking);
    JR_WRB(ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, VSTAX2_MISC_ISDX_ENA, !stacking);

    JR_WRB(ANA_CL_2, COMMON_AGGR_CFG, AGGR_USE_VSTAX_AC_ENA, stacking);

    /* Configuration of learn-all is moved to vtss_jr1_lrn_all_set_chip() because
     * it's now a S/W-based learn-all frame forwarding due to a chip-bug (bugzilla#7730 and 7737).
     * However, if the stack masks change, we need to update its configuration.
     */
    VTSS_RC(vtss_jr1_lrn_all_set_chip(vtss_state));
     
    /* Enable CM - globally */
    JR_WRB(ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, VSTAX2_USE_CM_ENA, stacking);

    /* Enable CM on non-stack ports and update MAX Frame length */
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (!VTSS_PORT_CHIP_SELECTED(port_no)) {
            continue;
        }
        port = VTSS_CHIP_PORT(port_no);
        front_port = vtss_jr1_is_frontport(vtss_state, port_no);
        VTSS_RC(jr_update_lport(vtss_state, port_no, port));
        VTSS_RC(vtss_jr1_vstax_update_ingr_shaper(vtss_state, port, vtss_state->port.conf[port_no].speed, front_port));
        VTSS_RC(vtss_jr1_setup_mtu(vtss_state, port, vtss_state->port.conf[port_no].max_frame_length, front_port));
    }

    JR_WRB(DSM, CM_CFG_CMM_TO_ARB_CFG, CMM_TO_ARB_DELAY_ENA, stacking);
    JR_WRF(DSM, CM_CFG_CMM_TO_ARB_CFG, CMM_XTRCT_ENA, stacking ? 0x3 : 0);

    /* Due to a chip-bug in Jaguar 1 rev. A, enabling super-priority Rx is not recommended.
     * Only enable SP Rx in stacking solutions, on non-rev. A chips, and on primary chip.
     * In fact, it's impossible to enable stacking in a dual-chip solution if primary chip
     * is not a non-Rev. A, because SP frames arriving on the internal ports on the primary
     * chip will have to be handled by the analyzer and due to policing, all analyzed frames
     * going to the CPU frames will be killed to avoid duplicates.
     */
    JR_WRF(DSM, SP_CFG_SP_XTRCT_CFG, SP_XTRCT_ENA, (stacking && vtss_state->misc.chip_id.revision != 0 && vtss_state->chip_no == 0) ? 0x3 : 0);

    /* Max TTL */
    if (chip_info->mask_a) {
        ttl = chip_info->port_conf[0].ttl;
    }
    if (chip_info->mask_b) {
        max_ttl = chip_info->port_conf[1].ttl;
    }
    if (ttl > max_ttl) {
        max_ttl = ttl;
    }

    /* Max TTL for equal path routing */
    JR_WRF(ANA_AC, PS_COMMON_COMMON_EQUAL_STACK_LINK_TTL_CFG, VSTAX2_EQUAL_STACK_LINK_TTL_VAL, max_ttl);

    /* Keep TTL on secondary chip if stacking */
    JR_WRB(ASM, SP_CFG_SP_KEEP_TTL_CFG, SP_KEEP_TTL_ENA, (stacking && vtss_state->chip_no == 1) ? 1 : 0);
    JR_WRB(DSM, SP_CFG_SP_KEEP_TTL_CFG, SP_KEEP_TTL_ENA, (stacking && vtss_state->chip_no == 1) ? 1 : 0);

    /* Stack port mask registers */
    JR_WR(ANA_AC, PS_COMMON_STACK_A_CFG, chip_info->mask_a);
    JR_WR(ANA_AC, PS_COMMON_STACK_CFG, stack_mask);
    JR_WR(ANA_AC, PS_COMMON_PHYS_SRC_AGGR_CFG, stack_mask);

    /* REW:COMMON:VSTAX_PORT_GRP_CFG[0-1].VSTAX_LRN_ALL_HP_ENA */
    JR_WRXB(REW, COMMON_VSTAX_PORT_GRP_CFG, 0, VSTAX_LRN_ALL_HP_ENA, chip_info->mask_a != 0);
    JR_WRXB(REW, COMMON_VSTAX_PORT_GRP_CFG, 1, VSTAX_LRN_ALL_HP_ENA, chip_info->mask_b != 0);

    /* REW:COMMON:VSTAX_PORT_GRP_CFG[0-1].VSTAX_MODE */
    JR_WRXB(REW, COMMON_VSTAX_PORT_GRP_CFG, 0, VSTAX_MODE, chip_info->mask_a != 0);
    JR_WRXB(REW, COMMON_VSTAX_PORT_GRP_CFG, 1, VSTAX_MODE, chip_info->mask_b != 0);

    /* Setup mirroring */
    VTSS_RC(jr_mirror_conf_set_chip(vtss_state));

    /* Update VStaX topology */
    VTSS_RC(jr_vstax_topology_set_chip(vtss_state));

    for (port = JR_STACK_PORT_START; cmef_enable && port < JR_STACK_PORT_END; port++) {
        /* Enable CMEF on external stack ports and (in two-chip solutions) on internal port #0 */
        stack_port = port - JR_STACK_PORT_START;
        enable = ((VTSS_BIT(port) & stack_mask) != 0 && (vtss_state->chip_count != 2 || port != vtss_state->port.port_int_1));

        if (enable) {
            BOOL is_stack_a, is_stack_b;
            is_stack_a    = (VTSS_BIT(port) & chip_info->mask_a) ? 1 : 0;
            is_stack_b    = (VTSS_BIT(port) & chip_info->mask_b) ? 1 : 0;
            ttl = (is_stack_a ? chip_info->port_conf[0].ttl : is_stack_b ? chip_info->port_conf[1].ttl : 0);
            JR_WRXF(DSM, CM_CFG_CMEF_GEN_CFG, stack_port, CMEF_TTL, ttl);

            rmask = VTSS_F_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_RELAY_TX_ENA | VTSS_F_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_GEN_ENA;
            JR_WRXM(DSM, CM_CFG_CMEF_GEN_CFG, stack_port, rmask, rmask);
            JR_WRXB(ASM, CM_CFG_CMEF_RX_CFG, stack_port, CMEF_RX_ENA, 1);
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_vstax_conf_set(vtss_state_t *vtss_state)
{
    vtss_chip_no_t         chip_no;
    u32                    mask;
    vtss_vstax_chip_info_t *chip_info;
    vtss_port_no_t         port_no;
    int                    i;

    /* Update VStaX for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        chip_info = &vtss_state->l2.vstax_info.chip_info[chip_no];

        /* Update mask_a and mask_b */
        chip_info->mask_a = 0;
        chip_info->mask_b = (vtss_state->chip_count == 2 ? vtss_state->port.mask_int_ports : 0);
        for (i = 0; i < 2; i++) {
            port_no = (i == 0 ? vtss_state->l2.vstax_conf.port_0 : vtss_state->l2.vstax_conf.port_1);
            if (port_no != VTSS_PORT_NO_NONE && VTSS_PORT_CHIP_SELECTED(port_no)) {
                mask = VTSS_BIT(VTSS_CHIP_PORT(port_no));
                /* In two-chip solutions, mask_a are the external stack ports, and mask_b are the interconnects */
                if (i == 0 || vtss_state->chip_count == 2) {
                    /* Stack port A */
                    chip_info->mask_a |= mask;
                } else {
                    /* Stack port B */
                    chip_info->mask_b |= mask;
                }
            }
        }
        VTSS_RC(vtss_jr1_vstax_conf_set_chip(vtss_state));
    }

    /* Update VStaX for all stackable ports */
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (VTSS_PORT_IS_STACKABLE(VTSS_CHIP_PORT(port_no))) {
            VTSS_SELECT_CHIP_PORT_NO(port_no);

            /* Enable/disable VCAP lookups */
            VTSS_RC(vtss_jr1_vcap_lookup_set(vtss_state, port_no));

            /* Update VLAN port configuration */
            VTSS_RC(vtss_cmn_vlan_port_conf_set(vtss_state, port_no));
        }
    }

    /* Update IS2 rules for IGMP */
    VTSS_RC(vtss_jr1_igmp_cpu_copy_set(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc jr_vstax_master_upsid_set(vtss_state_t *vtss_state)
{
    /* Setup config for all devices */
    VTSS_RC(vtss_jr1_conf_chips(vtss_state, vtss_jr1_cpu_queue_redirect_chip));

    return VTSS_RC_OK;
}

static vtss_rc jr_vstax_topology_set(vtss_state_t *vtss_state)
{
    return jr_vstax_topology_set_chip(vtss_state);
}

static vtss_rc jr_vstax_port_conf_set(vtss_state_t *vtss_state, const BOOL stack_port_a)
{
    /* Update VStaX configuration */
    return vtss_jr1_vstax_conf_set_chip(vtss_state);
}
#endif /* VTSS_FEATURE_VSTAX_V2 */

#ifdef VTSS_FEATURE_SFLOW
/* ================================================================= *
 *  SFLOW
 * ================================================================= */

static u32 next_power_of_two(u32 x)
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
}

/**
 * jr_sflow_hw_rate()
 */
static u32 jr_sflow_hw_rate(const u32 desired_sw_rate, u32 *const realizable_sw_rate)
{
    u32 hw_rate         = desired_sw_rate ? MAX(VTSS_ROUNDING_DIVISION(JR_SFLOW_MAX_SAMPLE_RATE, desired_sw_rate), JR_SFLOW_MIN_SAMPLE_RATE) : 0;
    *realizable_sw_rate = hw_rate         ?     VTSS_ROUNDING_DIVISION(JR_SFLOW_MAX_SAMPLE_RATE, hw_rate) : 0;
    return hw_rate;
}

/**
 * jr_sflow_sampling_rate_convert()
 */
static vtss_rc jr_sflow_sampling_rate_convert(vtss_state_t *const state, const BOOL power2, const u32 rate_in, u32 *const rate_out)
{
    u32 modified_rate_in;
    // Could happen that two threads call this function simultaneously at boot, but we take the risk.
    // Once sflow_max_power_of_two has been computed, it's no longer a problem with simultaneous access.
    u32 sflow_max_power_of_two = state->l2.sflow_max_power_of_two;

    if (sflow_max_power_of_two == 0) {
        sflow_max_power_of_two = next_power_of_two(JR_SFLOW_MAX_SAMPLE_RATE);
        if ((JR_SFLOW_MAX_SAMPLE_RATE & sflow_max_power_of_two) == 0) {
            sflow_max_power_of_two >>= 1;
        }
        state->l2.sflow_max_power_of_two = sflow_max_power_of_two;
    }

    // Compute the actual sampling rate given the user input.
    if (rate_in != 0 && power2) {
        // Round off to the nearest power of two.
        u32 temp1 = next_power_of_two(rate_in);
        u32 temp2 = temp1 >> 1;
        if (temp1 - rate_in < rate_in-temp2) {
            modified_rate_in = temp1;
        } else {
            modified_rate_in = temp2;
        }
        if (modified_rate_in == 0) {
            modified_rate_in = 1;
        } else if (modified_rate_in > sflow_max_power_of_two) {
            modified_rate_in = sflow_max_power_of_two;
        }
    } else {
        modified_rate_in = rate_in;
    }

    (void)jr_sflow_hw_rate(modified_rate_in, rate_out);
    return VTSS_RC_OK;
}

/**
 * jr_sflow_port_conf_set()
 */
static vtss_rc jr_sflow_port_conf_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no,
                                      const vtss_sflow_port_conf_t *const new_conf)
{
#define JR_SFLOW_ENABLED(_conf_) ((_conf_)->sampling_rate > 0 && (_conf_)->type != VTSS_SFLOW_TYPE_NONE)
    vtss_sflow_port_conf_t *cur_conf = &vtss_state->l2.sflow_conf[port_no];
    u32                    hw_rate, value, chip_port = VTSS_CHIP_PORT(port_no);
    BOOL                   globally_enable;

    VTSS_SELECT_CHIP_PORT_NO(port_no);

    globally_enable = vtss_state->l2.sflow_ena_cnt[vtss_state->chip_no] > 0;

    if (JR_SFLOW_ENABLED(new_conf) && !JR_SFLOW_ENABLED(cur_conf)) {
        if (vtss_state->l2.sflow_ena_cnt[vtss_state->chip_no] == VTSS_PORTS) {
            VTSS_E("sFlow enable counter out of sync");
        }
        if (vtss_state->l2.sflow_ena_cnt[vtss_state->chip_no]++ == 0) {
            globally_enable = TRUE;
        }
    } else if (!JR_SFLOW_ENABLED(new_conf) && JR_SFLOW_ENABLED(cur_conf)) {
        if (vtss_state->l2.sflow_ena_cnt[vtss_state->chip_no] == 0) {
            VTSS_E("sFlow enable counter out of sync");
        } else  if (--vtss_state->l2.sflow_ena_cnt[vtss_state->chip_no] == 0) {
            globally_enable = FALSE;
        }
    }

    *cur_conf = *new_conf;

    JR_WRB(ANA_AC, PS_COMMON_PS_COMMON_CFG, SFLOW_ENA,                  globally_enable);
    JR_WRB(ANA_AC, PS_COMMON_PS_COMMON_CFG, SFLOW_SMPL_ID_IN_STAMP_ENA, globally_enable);

    // Compute the actual sampling rate given the user input.
    // If the user requires power-of-two sampling rates, he is supposed to have provided
    // such a sampling rate in #new_conf.
    // We must ensure that power-of-two input sampling rates gives the same power-of-two
    // output sampling rate.
    hw_rate = jr_sflow_hw_rate(new_conf->sampling_rate, &cur_conf->sampling_rate);

    JR_RDX(ANA_AC, PS_COMMON_SFLOW_CTRL, chip_port, &value);
    value &= ~VTSS_M_ANA_AC_PS_COMMON_SFLOW_CTRL_SFLOW_DIR_SEL;
    value &= ~VTSS_M_ANA_AC_PS_COMMON_SFLOW_CTRL_SFLOW_SAMPLE_RATE;
    value |= JR_PUT_FLD(ANA_AC, PS_COMMON_SFLOW_CTRL, SFLOW_DIR_SEL, new_conf->type == VTSS_SFLOW_TYPE_NONE ? 0 :
                                                                     new_conf->type == VTSS_SFLOW_TYPE_RX   ? 1 :
                                                                     new_conf->type == VTSS_SFLOW_TYPE_TX   ? 2 : 3);
    value |= JR_PUT_FLD(ANA_AC, PS_COMMON_SFLOW_CTRL, SFLOW_SAMPLE_RATE, hw_rate);

    JR_WRX(ANA_AC, PS_COMMON_SFLOW_CTRL, chip_port, value);

    return VTSS_RC_OK;
#undef JR_SFLOW_ENABLED
}
#endif /* VTSS_FEATURE_SFLOW */

/* - Debug print --------------------------------------------------- */

static vtss_rc jr_debug_mac_table(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    /*lint --e{454, 455} */ // Due to VTSS_EXIT_ENTER
    vtss_mac_entry_t       *entry;
    vtss_mac_table_entry_t mac_entry;
    BOOL                   header = 1;
    u32                    pgid;

    /* Dump MAC address table */
    memset(&mac_entry, 0, sizeof(mac_entry));
    while (jr_mac_table_get_next(vtss_state, &mac_entry, &pgid) == VTSS_RC_OK) {
        vtss_debug_print_mac_entry(pr, "Dynamic Entries (GET_NEXT)", &header, &mac_entry, pgid);
        VTSS_EXIT_ENTER();
    }
    if (!header)
        pr("\n");

    /* Dump static entries not returned by the get_next function */
    header = 1;
    for (entry = vtss_state->l2.mac_list_used; entry != NULL; entry = entry->next) {
        vtss_mach_macl_set(&mac_entry.vid_mac, entry->mach, entry->macl);
        if (jr_mac_table_get_chip(vtss_state, &mac_entry, &pgid) == VTSS_RC_OK) {
            vtss_debug_print_mac_entry(pr, "Static Entries (GET)", &header, &mac_entry, pgid);
        }
        VTSS_EXIT_ENTER();
    }
    if (!header)
        pr("\n");
    
    /* Read and clear sticky bits */
    if (info->full) {
        vtss_jr1_debug_reg_header(pr, "STICKY");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_CL_2_STICKY_FILTER_STICKY, "ANA_CL:FILTER_STKY");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_CL_2_STICKY_CLASS_STICKY, "ANA_CL:CLASS_STCKY");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_CL_2_STICKY_CAT_STICKY, "ANA_CL:CAT_STICKY");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY, "ANA_CL:IP_HDR_CHK");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_CL_2_STICKY_VQ_STICKY_REG, "ANA_CL:VQ_STICKY");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_CL_2_STICKY_MISC_CONF_STICKY, "ANA_CL:MISC_CONF");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_L2_STICKY_CCM_STICKY, "ANA_L2:CCM_STICKY");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_L2_STICKY_STICKY, "ANA_L2:L2_STICKY");
        vtss_jr1_debug_sticky(vtss_state, pr, VTSS_ANA_AC_PS_STICKY_STICKY, "ANA_AC:PS_STICKY");
        pr("\n");
    }
    
    return VTSS_RC_OK;
}

#define JR_DEBUG_VLAN(pr, addr, name) JR_DEBUG_REG_NAME(pr, ANA_L3_2, COMMON_VLAN_##addr, "VLAN_"name);

static vtss_rc jr_debug_vlan(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
    /*lint --e{454, 455} */ // Due to VTSS_EXIT_ENTER
    vtss_vid_t        vid;
    vtss_vlan_entry_t *vlan_entry;
    BOOL              header = 1;
    u32               port, value, mask = 0;
    char              buf[32];
    
    for (port = 0; port < VTSS_CHIP_PORTS_ALL; port++) {
        sprintf(buf, "Port %u", port);
        vtss_jr1_debug_reg_header(pr, buf);
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_CL_2_PORT_FILTER_CTRL(port), port, "ANA:FILTER_CTRL");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_CL_2_PORT_VLAN_CTRL(port), port, "ANA:VLAN_CTRL");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_REW_PORT_PORT_TAG_DEFAULT(port), port, "REW:TAG_DEFAULT");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_REW_PORT_TAG_CTRL(port), port, "REW:TAG_CTRL");
        pr("\n");
    }
        
    vtss_jr1_debug_reg_header(pr, "VLAN COMMON");
    JR_DEBUG_VLAN(pr, CTRL, "CTRL");
    JR_DEBUG_VLAN(pr, FILTER_CTRL, "FILTER_CTRL");
    JR_DEBUG_VLAN(pr, ISOLATED_CFG, "ISOLATED_CFG");
    JR_DEBUG_VLAN(pr, COMMUNITY_CFG, "COMMUNITY_CFG");
    JR_DEBUG_VLAN(pr, PORT_TYPE_MASK_CFG, "PORT_TYPE_MSK");
    JR_DEBUG_REG_NAME(pr, ANA_CL_2, COMMON_VLAN_STAG_CFG, "VLAN_STAG_CFG");
    pr("\n");

    for (vid = VTSS_VID_NULL; vid < VTSS_VIDS; vid++) {
        vlan_entry = &vtss_state->l2.vlan_table[vid];
        if (!vlan_entry->enabled && !info->full)
            continue;
        
        JR_RDX(ANA_L3_2, VLAN_VLAN_MASK_CFG, vid, &mask);
        JR_RDX(ANA_L3_2, VLAN_VLAN_CFG, vid, &value);
        
        if (header)
            vtss_jr1_debug_print_port_header(vtss_state, pr, "VID   Lrn  Mir  Prv  ");
        header = 0;

        pr("%-6u%-5u%-5u%-5u", vid,
           value & VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_LRN_DIS ? 0 : 1,
           value & VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_MIRROR_ENA ? 1 : 0,
           value & VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_PRIVATE_ENA ? 1 : 0);
        vtss_jr1_debug_print_mask(pr, mask);

        /* Leave critical region briefly */
        VTSS_EXIT_ENTER();
    }
    if (!header)
        pr("\n");
    
    for (vid = VTSS_VID_NULL; vid < VTSS_VIDS; vid++) {
        vlan_entry = &vtss_state->l2.vlan_table[vid];
        if (!vlan_entry->enabled && !info->full)
            continue;
        sprintf(buf, "VID: %u", vid);
        vtss_jr1_debug_reg_header(pr, buf);
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_L3_2_VLAN_BVLAN_CFG(vid), vid, "BVLAN_CFG");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_L3_2_VLAN_VLAN_CFG(vid), vid, "VLAN_CFG");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_L3_2_VLAN_VLAN_MASK_CFG(vid), vid, "VLAN_MASK_CFG");
        pr("\n");
        /* Leave critical region briefly */
        VTSS_EXIT_ENTER();
    }
    
    /* IS0 is used for basic VLAN classification */
    VTSS_RC(vtss_jr1_debug_vcap_is0(vtss_state, pr, info));

    return VTSS_RC_OK;
}

static vtss_rc jr_debug_src_table(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    u32 port, mask;

    vtss_debug_print_header(pr, "Source Masks");
    vtss_jr1_debug_print_port_header(vtss_state, pr, "Port  ");
    for (port = 0; port <= VTSS_CHIP_PORTS; port++) {
        JR_RDX(ANA_AC, SRC_SRC_CFG, port, &mask);
        pr("%-4u  ", port);
        vtss_jr1_debug_print_mask(pr, mask);
    }
    pr("\n");

    return VTSS_RC_OK;
}

static vtss_rc jr_debug_pvlan(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    vtss_jr1_debug_reg_header(pr, "ANA_L3");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_ANA_L3_2_COMMON_VLAN_ISOLATED_CFG, "ISOLATED_CFG");
    pr("\n");
    return jr_debug_src_table(vtss_state, pr, info);
}

static vtss_rc jr_debug_vxlat(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_jr1_debug_vcap_is1(vtss_state, pr, info));
    VTSS_RC(vtss_jr1_debug_vcap_es0(vtss_state, pr, info));
    
    return VTSS_RC_OK;
}

static vtss_rc jr_debug_aggr(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
    u32 ac, port, pgid, value, mask;
    
    vtss_jr1_debug_reg_header(pr, "Logical Ports");
    for (port = 0; port < VTSS_CHIP_PORTS_ALL; port++) {
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_CL_2_PORT_PORT_ID_CFG(port), port, "PORT_ID_CFG");
    }
    pr("\n");

    VTSS_RC(jr_debug_src_table(vtss_state, pr, info));
    
    vtss_debug_print_header(pr, "Aggregation Masks");
    vtss_jr1_debug_print_port_header(vtss_state, pr, "AC    ");
    for (ac = 0; ac < 16; ac++) {
        JR_RDX(ANA_AC, AGGR_AGGR_CFG, ac, &mask);
        pr("%-4u  ", ac);
        vtss_jr1_debug_print_mask(pr, mask);
    }
    pr("\n");
    
    vtss_debug_print_header(pr, "Destination Masks");
    vtss_jr1_debug_print_port_header(vtss_state, pr, "PGID            CPU  Queue  Stack  ");
    for (pgid = 0; pgid < VTSS_PGID_JAGUAR_1; pgid++) {
        JR_RDX(ANA_AC, PGID_PGID_CFG_0, pgid, &mask);
        JR_RDX(ANA_AC, PGID_PGID_CFG_1, pgid, &value);
        if (pgid > 50 && mask == 0 && !info->full)
            continue;
        pr("%-4u %-9s  %-3u  %-5u  %-5u  ", pgid,
           pgid == PGID_UC_FLOOD ? "UC" :
           pgid == PGID_MC_FLOOD ? "MC" :
           pgid == PGID_IPV4_MC_DATA ? "IPv4 Data" :
           pgid == PGID_IPV4_MC_CTRL ? "IPv4 Ctrl" :
           pgid == PGID_IPV6_MC_DATA ? "IPv6 Data" :
           pgid == PGID_IPV6_MC_CTRL ? "IPv6 Ctrl" :
           pgid == PGID_DROP ? "DROP" : 
           pgid == PGID_FLOOD ? "FLOOD" :
           (pgid >= PGID_GLAG_START) && (pgid < PGID_GLAG_START+VTSS_GLAGS*VTSS_GLAG_PORTS) ? "GLAG" : "",
           JR_GET_BIT(ANA_AC, PGID_PGID_CFG_1, PGID_CPU_COPY_ENA, value),
           JR_GET_FLD(ANA_AC, PGID_PGID_CFG_1, PGID_CPU_QU, value),
           JR_GET_BIT(ANA_AC, PGID_PGID_CFG_1, STACK_TYPE_ENA, value));
        vtss_jr1_debug_print_mask(pr, mask);
    }
    pr("\n");
    
    return VTSS_RC_OK;
}

static vtss_rc jr_debug_glag(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
#if defined(VTSS_FEATURE_AGGR_GLAG)
    u32 ac, id, port, pgid, value, mask, gl_memb[VTSS_GLAGS];

    pr("%-10s %-10s %-10s %-10s\n", "Port", "Lport","is-Glag","Glag_no");
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        JR_RDX(ANA_CL_2, PORT_PORT_ID_CFG, port, &value);
        pr("%-10u %-10u %-10d %-10u\n", port, 
           VTSS_X_ANA_CL_2_PORT_PORT_ID_CFG_LPORT_NUM(value), 
           ((VTSS_F_ANA_CL_2_PORT_PORT_ID_CFG_PORT_IS_GLAG_ENA & value) ? 1 : 0),
           VTSS_X_ANA_CL_2_PORT_PORT_ID_CFG_GLAG_NUM(value));
    }
    pr("\n");

    vtss_debug_print_header(pr, "GLAG member count (1-8)");
    pr("%-8s %-8s\n","Glag","Members");
    for (id = 0; id < VTSS_GLAGS; id++) {
        JR_RDX(ANA_AC, GLAG_MBR_CNT_CFG, id, &value); 
        pr("%-8u %-8u\n", id, value+1);
        gl_memb[id] = value;
    }
    pr("\n");

    vtss_debug_print_header(pr, "Aggregation code");
    JR_RD(ANA_CL_2, COMMON_AGGR_CFG, &value);
    vtss_jr1_debug_print_mask(pr, value); 
    pr("\n");

    vtss_debug_print_header(pr, "GLAG Source Masks");
    vtss_jr1_debug_print_port_header(vtss_state, pr, "Glag  ");
    for (id = 0; id < VTSS_GLAGS; id++) {
        JR_RDX(ANA_AC, GLAG_GLAG_CFG, id, &mask);
        pr("%-4u  ", id);
        vtss_jr1_debug_print_mask(pr, mask);
    }
    pr("\n");
    
    vtss_debug_print_header(pr, "Aggregation Masks");
    vtss_jr1_debug_print_port_header(vtss_state, pr, "AC    ");
    for (ac = 0; ac < 16; ac++) {
        JR_RDX(ANA_AC, AGGR_AGGR_CFG, ac, &mask);
        pr("%-4u  ", ac);
        vtss_jr1_debug_print_mask(pr, mask);
    }
    pr("\n");

    
    vtss_debug_print_header(pr, "GLAG PGID table");
    pr("%-8s %-8s %-8s %-8s %-8s %-8s %-8s\n", "PGID","GLAG","Stack","CPU","Queue","UPSID","UPSPN");
    for (pgid = PGID_GLAG_START; pgid < (PGID_GLAG_START+VTSS_GLAG_PORTS*VTSS_GLAGS); pgid++) {
        id = (pgid-PGID_GLAG_START)/VTSS_GLAG_PORTS;
        JR_RDX(ANA_AC, PGID_PGID_CFG_0, pgid, &mask);
        if (mask == 0 && gl_memb[id]==0) {
            continue;
        }
        gl_memb[id]=0;
        JR_RDX(ANA_AC, PGID_PGID_CFG_1, pgid, &value);
        pr("%-8u %-8u %-8d %-8d %-8u %-8u %-8u", pgid, id,
           JR_GET_BIT(ANA_AC, PGID_PGID_CFG_1, STACK_TYPE_ENA, value),
           JR_GET_BIT(ANA_AC, PGID_PGID_CFG_1, PGID_CPU_COPY_ENA, value),
           JR_GET_FLD(ANA_AC, PGID_PGID_CFG_1, PGID_CPU_QU, value),
           (mask >> 6 & 0x1F), (mask & 0x1F));

//        vtss_jr1_debug_print_mask(pr, mask);
        pr("\n");
    }
    pr("\n");

#endif    
    return VTSS_RC_OK;
}

static vtss_rc jr_debug_stp(vtss_state_t *vtss_state,
                            const vtss_debug_printf_t pr,
                            const vtss_debug_info_t   *const info)
{
    vtss_jr1_debug_reg_header(pr, "MSTP_0");
    vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_L3_2_MSTP_MSTP_LRN_CFG(0), 0, "MSTP_LRN_CFG");
    pr("\n");
    
    vtss_jr1_debug_reg_header(pr, "L2_COMMON");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_ANA_L2_COMMON_LRN_SECUR_CFG, "LRN_SECUR_CFG");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_ANA_L2_COMMON_AUTO_LRN_CFG, "AUTO_LRN_CFG");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_ANA_L2_COMMON_LRN_COPY_CFG, "LRN_COPY_CFG");
    pr("\n");

    return VTSS_RC_OK;
}

static vtss_rc jr_debug_mirror(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    u32 i;

    for (i = 0; i < 3; i++) {
        vtss_jr1_debug_reg_header(pr, i == JR_MIRROR_PROBE_RX ? "RX" : 
                            i == JR_MIRROR_PROBE_TX ? "TX" : "RESV");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ARB_CFG_STATUS_MIRROR_CFG(i), i, "ARB:MIRROR_CFG");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_REW_COMMON_MIRROR_PROBE_CFG(i), i, "REW:PROBE_CFG");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_AC_MIRROR_PROBE_PROBE_CFG(i), i, "ANA_AC:PROBE_CFG");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ANA_AC_MIRROR_PROBE_PROBE_PORT_CFG(i), i, "ANA_AC:PORT_CFG");
        pr("\n");
    }
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_ANA_AC_PS_COMMON_VSTAX_GMIRROR_CFG, "ANA_AC:GMIRROR_CFG");
    pr("\n");

    return VTSS_RC_OK;
}

static vtss_rc jr_debug_lrn_table(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    u32 value;
    
    vtss_debug_print_reg_header(pr, "LRN:COMMON:");

    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_COMMON_ACCESS_CTRL, "COMMON_ACCESS_CTRL");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_MAC_ACCESS_CFG_0, "MAC_ACCESS_CFG_0");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_MAC_ACCESS_CFG_1, "MAC_ACCESS_CFG_1");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_MAC_ACCESS_CFG_2, "MAC_ACCESS_CFG_2");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_MAC_ACCESS_CFG_3, "MAC_ACCESS_CFG_3");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_MAC_ACCESS_CFG_4, "MAC_ACCESS_CFG_4");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_SCAN_NEXT_CFG, "SCAN_NEXT_CFG");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_SCAN_NEXT_CFG_1, "SCAN_NEXT_CFG_1");
    //vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_SCAN_LAST_ROW_CFG, "SCAN_LAST_ROW_CFG");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_SCAN_NEXT_CNT, "SCAN_NEXT_CNT");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_AUTOAGE_CFG, "AUTOAGE_CFG");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_AUTOAGE_CFG_1, "AUTOAGE_CFG_1");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_AUTO_LRN_CFG, "AUTO_LRN_CFG");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_CCM_CTRL, "CCM_CTRL");
    //vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_CCM_TYPE_CFG(0), "CCM_TYPE_CFG(0)");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_CCM_STATUS, "CCM_STATUS");


    /* Read and clear LRN sticky bits */
    JR_RD(LRN_COMMON, EVENT_STICKY, &value); 
    JR_WR(LRN_COMMON, EVENT_STICKY, value); 
    vtss_debug_print_reg(pr, "EVENT_STICKY", value);

    vtss_jr1_debug_reg(vtss_state, pr, VTSS_LRN_COMMON_LATEST_POS_STATUS, "LATEST_POS_STATUS");
    /* Read and clear RAM_INTGR_ERR_ITENT sticky bits */
    JR_RD(LRN_COMMON, RAM_INTGR_ERR_IDENT, &value); 
    JR_WR(LRN_COMMON, RAM_INTGR_ERR_IDENT, value); 
    vtss_debug_print_reg(pr, "RAM_INTGR_ERR_IDENT", value);
    
    pr("\n");

    return VTSS_RC_OK;
}

static vtss_rc jr_debug_stack(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_vstax_upsid_t       upsid;

    vtss_jr1_debug_reg_header(pr, "ANA_L2/ANA_CL");
    JR_DEBUG_REG(pr, ANA_L2, COMMON_VSTAX_CTRL);
    JR_DEBUG_REG(pr, ANA_CL_2, COMMON_UPSID_CFG);
    pr("\n");

    vtss_jr1_debug_reg_header(pr, "REW");
    JR_DEBUG_REGX_NAME(pr, REW, COMMON_VSTAX_PORT_GRP_CFG, 0, "PORT_GRP_CFG");
    JR_DEBUG_REGX_NAME(pr, REW, COMMON_VSTAX_PORT_GRP_CFG, 1, "PORT_GRP_CFG");
    pr("\n");
    
    vtss_jr1_debug_reg_header(pr, "ANA_AC");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_ANA_AC_PS_COMMON_STACK_A_CFG, "STACK_A_CFG");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_ANA_AC_PS_COMMON_STACK_CFG, "STACK_CFG");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_ANA_AC_PS_COMMON_COMMON_EQUAL_STACK_LINK_TTL_CFG, "LINK_TTL_CFG");
    for (upsid = VTSS_VSTAX_UPSID_MIN; upsid <= VTSS_VSTAX_UPSID_MAX; upsid++) {
        JR_DEBUG_REGX(pr, ANA_AC, UPSID_UPSID_CFG, upsid);
    }
    pr("\n");
#endif  /* VTSS_FEATURE_VSTAX_V2 */
    return VTSS_RC_OK;
}

static vtss_rc jr_debug_cmef(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_port_no_t           chip_port;
    int                      i, prio;
    vtss_vstax_upsid_t       upsid;
    vtss_chip_no_t           chip_no = vtss_state->chip_no;
    vtss_vstax_route_entry_t *rt;

    vtss_jr1_debug_reg_header(pr, "DSM/ANA_AC General");
    JR_DEBUG_REG_NAME(pr, DSM, CM_CFG_CMM_MASK_CFG, "CMM_MASK_CFG");
    JR_DEBUG_REG_NAME(pr, ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, "COMMON_VSTAX_CFG");
    JR_DEBUG_REG_NAME(pr, DSM, CM_CFG_CMM_TO_ARB_CFG, "CMM_TO_ARB_CFG");
    JR_DEBUG_REG_NAME(pr, DSM, CM_CFG_CMEF_UPSID_ACTIVE_CFG, "CMEF_UPSID_ACTIVE");
    JR_DEBUG_REG_NAME(pr, DSM, CM_CFG_CMEF_RATE_CFG, "CMEF_RATE_CFG");
    pr("\n");

    vtss_jr1_debug_reg_header(pr, "ARB Port CFG");
    for (chip_port = 0; chip_port < VTSS_CHIP_PORTS; chip_port++) {
        JR_DEBUG_REGX_NAME(pr, ARB, CFG_STATUS_INGR_SHAPER_CFG, chip_port, "INGR_SHAPER_CFG");
    }
    pr("\n");
    
    vtss_jr1_debug_reg_header(pr, "DSM Port CFG");
    for (chip_port = 0; chip_port < VTSS_CHIP_PORTS; chip_port++) {
        JR_DEBUG_REGX_NAME(pr, DSM, CM_CFG_LPORT_NUM_CFG, chip_port, "LPORT_NUM_CFG");
    }
    pr("\n");

    vtss_jr1_debug_reg_header(pr, "OQS Port CFG");
    for (chip_port = 0; chip_port < VTSS_CHIP_PORTS; chip_port++) {
        JR_DEBUG_REGX_NAME(pr, OQS, PORT_RAM_CFG_CM_PORT_WM, chip_port, "CM_PORT_WM");
    }
    pr("\n");

    vtss_jr1_debug_reg_header(pr, "ASM/DSM");
    for (chip_port = 0; chip_port < VTSS_CHIP_PORTS; chip_port++) {
        if (VTSS_PORT_IS_STACKABLE(chip_port)) {
            u32 stackport = chip_port - JR_STACK_PORT_START; /* Stack port no */
            vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DSM_CM_CFG_CMEF_GEN_CFG(stackport), stackport, "DSM:CM_GEN_CFG");
            vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_ASM_CM_CFG_CMEF_RX_CFG(stackport),  stackport, "ASM:CM_RX_CFG ");
        }
    }
    pr("\n");

    vtss_jr1_debug_reg_header(pr, "ARB Queue Mask");
    for (prio = 0; prio < VTSS_PRIOS; prio++) {
        JR_DEBUG_REGX_NAME(pr, ARB, CFG_STATUS_CM_CFG_VEC0, prio, "CM_CFG_VEC0");
    }
    pr("\n");

    vtss_jr1_debug_reg_header(pr, "ASM/DSM Status");
    for (i = 0; i < 2; i++) {
        JR_DEBUG_REGX_NAME(pr, ASM, CM_STATUS_CMEF_CNT, i, "CMEF_CNT");
        JR_DEBUG_REGX_NAME(pr, DSM, CM_STATUS_CMEF_GENERATED_STATUS, i, "CMEF_GENERATED");
        JR_DEBUG_REGX_NAME(pr, DSM, CM_STATUS_CMEF_RELAYED_STATUS, i, "CMEF_RELAYED");
        JR_DEBUG_REGX_NAME(pr, DSM, CM_STATUS_CMM_XTRCT_CNT, i, "CMM_XTRCT_CNT");
    }
    JR_DEBUG_REG_NAME(pr, ASM, CM_STATUS_CMEF_DISCARD_STICKY, "CMEF_DISCARD");
    pr("\n");
    
    vtss_jr1_debug_reg_header(pr, "ARB Status");
    for (upsid = VTSS_VSTAX_UPSID_MIN; upsid <= VTSS_VSTAX_UPSID_MAX; upsid++) {
        rt = &vtss_state->l2.vstax_info.chip_info[chip_no].rt_table.table[upsid];
        if (vtss_state->l2.vstax_info.upsid[chip_no] == upsid || rt->stack_port_a || rt->stack_port_b) {
            i8 regname[64];
            JR_WRF(ARB, CFG_STATUS_CM_ACCESS, CM_UPSID, upsid);
            JR_WRB(ARB, CFG_STATUS_CM_ACCESS, CM_START_ACCESS, 1);
            JR_POLL_BIT(ARB, CFG_STATUS_CM_ACCESS, CM_START_ACCESS);
            (void)snprintf(regname, sizeof(regname), "CM_ACCESS(%d)", upsid);
            JR_DEBUG_REG_NAME(pr, ARB, CFG_STATUS_CM_ACCESS, regname);
            (void)snprintf(regname, sizeof(regname), "CM_DATA(%d)", upsid);
            JR_DEBUG_REG_NAME(pr, ARB, CFG_STATUS_CM_DATA, regname);
        }
    }
    pr("\n");
#endif  /* VTSS_FEATURE_VSTAX_V2 */
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_l2_debug_print(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_VLAN, jr_debug_vlan, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_PVLAN, jr_debug_pvlan, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_MAC_TABLE, jr_debug_mac_table, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_VXLAT, jr_debug_vxlat, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_AGGR, jr_debug_aggr, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_GLAG, jr_debug_glag, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_LRN, jr_debug_lrn_table, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_STP, jr_debug_stp, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_MIRROR, jr_debug_mirror, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_STACK, jr_debug_stack, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_CMEF, jr_debug_cmef, vtss_state, pr, info));
    return VTSS_RC_OK;
}

/* - Initialization ------------------------------------------------ */

static vtss_rc jr_l2_init(vtss_state_t *vtss_state)
{
    vtss_l2_state_t *state = &vtss_state->l2;
    u32             port;

    /* Disable learning */
    JR_WRX(ANA_L3_2, MSTP_MSTP_LRN_CFG, 0, 0);

    /* VLAN: Clear VID 4095 mask, enable VLAN and use default port config */
    JR_WR(ANA_L3_2, COMMON_VLAN_CTRL, VTSS_F_ANA_L3_2_COMMON_VLAN_CTRL_VLAN_ENA);
    JR_WRX(ANA_L3_2, VLAN_VLAN_MASK_CFG, 4095, 0);
    VTSS_RC(jr_vlan_mask_apply(vtss_state, 1, 0xffffffff));
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        VTSS_RC(vtss_jr1_vlan_port_conf_apply(vtss_state, port, &state->vlan_port_conf[VTSS_PORT_NO_START], NULL));
    }

    /* Disable learning in VLAN 0 (used for EVCs) */
    JR_WRXB(ANA_L3_2, VLAN_VLAN_CFG, 0, VLAN_LRN_DIS, 1);

    /* Setup VLAN configuration */
    VTSS_RC(jr_vlan_conf_set_chip(vtss_state, 0));

    /* Set MAC age time to default value */
    VTSS_RC(jr_mac_table_age_time_set_chip(vtss_state));

#if !defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    /* Enable frame aging */
    JR_WR(ARB, AGING_CFG_ERA_CTRL, 0x41); /* Set tick to 0.5 sec and enable era generator */
    JR_WR(SCH, MISC_ERA_CTRL, 0x41);      /* Set tick to 0.5 sec and enable era generator */
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        JR_WRXB(DSM, CFG_BUF_CFG, port, AGING_ENA, 1); /* Enable aging in disassembler for each port */
    }
#endif /* !defined(VTSS_ARCH_JAGUAR_1_CE_MAC) */

    /* Enable ISDX table */
    JR_WRB(ANA_L2, COMMON_FWD_CFG, ISDX_LOOKUP_ENA, 1);

    /* CMEF DMAC address */
    JR_WRF(DSM, CM_CFG_CMEF_MAC_ADDR_HIGH_CFG, CMEF_MAC_ADDR_HIGH, 0xFFFFFF);
    JR_WRF(DSM, CM_CFG_CMEF_MAC_ADDR_LOW_CFG, CMEF_MAC_ADDR_LOW, 0xFFFFFF);

    /* Enable discard of VStaX frames on all ports by default. This is whether
     * a switch is stackable or not.
     * This bit will be cleared on NPI, interconnect (JR48) , and stacking ports whenever required.
     */
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        JR_WRXB(ANA_CL_2, PORT_STACKING_CTRL, port, STACKING_HEADER_DISCARD_ENA, 1);
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l2_port_map(vtss_state_t *vtss_state)
{
    vtss_l2_state_t   *state = &vtss_state->l2;
    vtss_port_no_t    port_no;
    u32               pgid;
    BOOL              member[VTSS_PORT_ARRAY_SIZE];
    vtss_pgid_entry_t *pgid_entry;
    
    /* We only need to setup the no of avail pgids */
    state->pgid_count = (VTSS_PGID_JAGUAR_1 - VTSS_CHIP_PORTS + vtss_state->port_count);
    
    /* For dual chip targets, the calculation above must be limited */
    if (state->pgid_count > VTSS_PGID_JAGUAR_1)
        state->pgid_count = VTSS_PGID_JAGUAR_1;
    
    /* And then reserve PGIDs for flood masks */
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++)
        member[port_no] = vtss_jr1_cemax_port_is_host(vtss_state, port_no) ? 0 : 1;
    for (pgid = PGID_UC_FLOOD; pgid <= PGID_IPV6_MC_CTRL; pgid++) {
        VTSS_RC(jr_pgid_update(vtss_state, pgid, member));
    }
    
    /* Drop entry */
    pgid = vtss_jr1_vtss_pgid(vtss_state, PGID_DROP);
    state->pgid_drop = pgid;
    state->pgid_table[pgid].references = 1;
    
    /* Flood entry */
    pgid = vtss_jr1_vtss_pgid(vtss_state, PGID_FLOOD);
    state->pgid_flood = pgid;
    pgid_entry = &state->pgid_table[pgid];
    pgid_entry->references = 1;
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) 
        pgid_entry->member[port_no] = vtss_jr1_cemax_port_is_host(vtss_state, port_no) ? 0 : 1;
    
#if !defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    /* GLAG entries */
    state->pgid_glag_dest = vtss_jr1_vtss_pgid(vtss_state, PGID_GLAG_START);     
    
    /* Reserve entries for GLAG destinations */
    for (pgid = PGID_GLAG_START; pgid < (PGID_GLAG_START+VTSS_GLAG_PORTS*VTSS_GLAGS); pgid++) {
        state->pgid_table[vtss_jr1_vtss_pgid(vtss_state, pgid)].resv = 1;
    }
#endif /* !defined (VTSS_ARCH_JAGUAR_1_CE_MAC) */
    return VTSS_RC_OK;
}

static vtss_rc jr_l2_poll(vtss_state_t *vtss_state)
{
#if defined(VTSS_FEATURE_VLAN_COUNTERS)
    vtss_vlan_counter_info_t *vlan_info = &vtss_state->l2.vlan_counters_info;
    u32                      i, idx;

    /* Maximum we support 90Gbps traffic in Jaguar-48. For 100Gbps, 32-bit counter wrap time is around 26s.
       Hence, we poll 200 VLAN counters per second. This will give us approximately 20s time between the 
       fetches */
    for (i = 0; i < 200; i++) {
        // Poll in range vid == [1; VTSS_VIDS - 1], which corresponds to idx == [0; VTSS_VIDS - 2]
        idx = vlan_info->poll_idx++;
        if (vlan_info->poll_idx >= VTSS_VIDS - 1) {
            vlan_info->poll_idx = 0;
        }
        VTSS_RC(jr_vlan_counters_get(vtss_state, idx + 1, NULL));
    }
#endif /* VTSS_FEATURE_VLAN_COUNTERS */
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_l2_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_l2_state_t *state = &vtss_state->l2;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->mac_table_add  = jr_mac_table_add;
        state->mac_table_del  = jr_mac_table_del;
        state->mac_table_get = jr_mac_table_get;
        state->mac_table_get_next = jr_mac_table_get_next;
        state->mac_table_age_time_set = jr_mac_table_age_time_set;
        state->mac_table_age = jr_mac_table_age;
#if defined(VTSS_FEATURE_VSTAX_V2)
        state->mac_table_upsid_upspn_flush = jr_mac_table_upsid_upspn_flush;
#endif /* VTSS_FEATURE_VSTAX_V2 */
        state->mac_table_status_get = jr_mac_table_status_get;
        state->learn_port_mode_set = jr_learn_port_mode_set;
        state->learn_state_set = jr_learn_state_set;
        state->mstp_state_set = vtss_cmn_mstp_state_set;
        state->mstp_vlan_msti_set = vtss_cmn_vlan_members_set;
        state->erps_vlan_member_set = vtss_cmn_erps_vlan_member_set;
        state->erps_port_state_set = vtss_cmn_erps_port_state_set;
        state->vlan_conf_set = jr_vlan_conf_set;
        state->vlan_port_conf_set = vtss_cmn_vlan_port_conf_set;
        state->vlan_port_conf_update = jr_vlan_port_conf_update;
        state->vlan_port_members_set = vtss_cmn_vlan_members_set;
        state->vlan_mask_update = jr_vlan_mask_update;
        state->vlan_tx_tag_set = vtss_cmn_vlan_tx_tag_set;
        state->isolated_vlan_set = vtss_cmn_vlan_members_set;
        state->isolated_port_members_set = jr_isolated_port_members_set;
        state->aggr_mode_set = jr_aggr_mode_set;
        state->mirror_port_set = jr_mirror_port_set;
        state->mirror_ingress_set = jr_mirror_ingress_set;
        state->mirror_egress_set = jr_mirror_egress_set;
        state->flood_conf_set = jr_flood_conf_set;
#if defined(VTSS_FEATURE_VSTAX_V2)
        state->vstax_conf_set = jr_vstax_conf_set;
        state->vstax_master_upsid_set = jr_vstax_master_upsid_set;
        state->vstax_port_conf_set = jr_vstax_port_conf_set;
        state->vstax_topology_set = jr_vstax_topology_set;
#endif /* VTSS_FEATURE_VSTAX_V2 */
#if defined(VTSS_FEATURE_AGGR_GLAG)
        state->glag_src_table_write = jr_glag_src_table_write;
        state->glag_port_write = jr_glag_port_write;
#endif /*VTSS_FEATURE_AGGR_GLAG */
        state->src_table_write = jr_src_table_write;
        state->pgid_table_write = jr_pgid_table_write;
        state->aggr_table_write = jr_aggr_table_write;
        state->pmap_table_write = jr_pmap_table_write;
        state->eps_port_set = vtss_cmn_eps_port_set;
#if defined(VTSS_FEATURE_VCL)
        state->vcl_port_conf_set = jr_vcl_port_conf_set;
        state->vce_add = vtss_cmn_vce_add;
        state->vce_del = vtss_cmn_vce_del;
#endif /* VTSS_FEATURE_VCL */
#if defined(VTSS_FEATURE_VLAN_TRANSLATION)
        state->vlan_trans_group_add = vtss_cmn_vlan_trans_group_add;
        state->vlan_trans_group_del = vtss_cmn_vlan_trans_group_del;
        state->vlan_trans_group_get = vtss_cmn_vlan_trans_group_get;
        state->vlan_trans_port_conf_set  = vtss_cmn_vlan_trans_port_conf_set;
        state->vlan_trans_port_conf_get  = vtss_cmn_vlan_trans_port_conf_get;
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */
#if defined(VTSS_FEATURE_VLAN_COUNTERS)
        state->vlan_counters_get = jr_vlan_counters_get;
        state->vlan_counters_clear = jr_vlan_counters_clear;
#endif /* VTSS_FEATURE_VLAN_COUNTERS */
#if defined (VTSS_FEATURE_SFLOW)
        state->sflow_port_conf_set         = jr_sflow_port_conf_set;
        state->sflow_sampling_rate_convert = jr_sflow_sampling_rate_convert;
#endif /* VTSS_FEATURE_SFLOW */
        state->ac_count = JR_ACS;
        break;
    case VTSS_INIT_CMD_INIT:
        VTSS_RC(jr_l2_init(vtss_state));
        break;
    case VTSS_INIT_CMD_PORT_MAP:
        VTSS_RC(jr_l2_port_map(vtss_state));
        break;
    case VTSS_INIT_CMD_POLL:
        VTSS_RC(jr_l2_poll(vtss_state));
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_JAGUAR_1 */
