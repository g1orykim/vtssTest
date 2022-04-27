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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_VCAP
#include "vtss_api.h"
#include "vtss_state.h"
#include "vtss_common.h"

#if defined(VTSS_FEATURE_VCAP)

/* - Resource utilities -------------------------------------------- */

/* Initialize resource change information */
void vtss_cmn_res_init(vtss_res_t *res)
{
    memset(res, 0, sizeof(*res));
}

/* Check VCAP resource usage */
vtss_rc vtss_cmn_vcap_res_check(vtss_vcap_obj_t *obj, vtss_res_chg_t *chg)
{
    u32                  add_row, del_row, add, del, old, new, key_count, count;
    vtss_vcap_key_size_t key_size;
    
    add_row = chg->add;
    del_row = chg->del;

    /* Calculate added/deleted rows for each key size */
    for (key_size = VTSS_VCAP_KEY_SIZE_FULL; key_size <= VTSS_VCAP_KEY_SIZE_LAST; key_size++) {
        count = vtss_vcap_key_rule_count(key_size);
        add = chg->add_key[key_size];
        del = chg->del_key[key_size];
        key_count = (obj->key_count[key_size] + count - 1);
        old = (key_count / count);
        new = ((key_count + add - del) / count);
        
        if (add > del) {
            /* Adding rules may cause addition of rows */
            add_row += (new - old);
        } else {
            /* Deleting rules may cause deletion of rows */
            del_row += (old - new);
        }
    }

    if ((obj->count + add_row) > (obj->max_count + del_row)) {
        VTSS_I("VCAP %s exceeded, add: %u, del: %u, count: %u, max: %u", 
               obj->name, add_row, del_row, obj->count, obj->max_count);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

/* Check SDK and VCAP resource usage */
vtss_rc vtss_cmn_res_check(vtss_state_t *vtss_state, vtss_res_t *res)
{
#if defined(VTSS_SDX_CNT)
    vtss_sdx_info_t *sdx_info = &vtss_state->evc.sdx_info;
    
    if ((sdx_info->isdx.count + res->isdx.add - res->isdx.del) > sdx_info->max_count) {
        VTSS_I("ISDX exceeded");
        return VTSS_RC_ERROR;
    }

    if ((sdx_info->esdx.count + res->esdx.add - res->esdx.del) > sdx_info->max_count) {
        VTSS_I("ESDX exceeded");
        return VTSS_RC_ERROR;
    }
#endif /* VTSS_SDX_CNT */

#if defined(VTSS_FEATURE_IS0)
    /* VCAP IS0 */
    VTSS_RC(vtss_cmn_vcap_res_check(&vtss_state->vcap.is0.obj, &res->is0));
#endif /* VTSS_FEATURE_IS0 */
    
#if defined(VTSS_FEATURE_IS1)
    /* VCAP IS1 */
    VTSS_RC(vtss_cmn_vcap_res_check(&vtss_state->vcap.is1.obj, &res->is1));
#endif /* VTSS_FEATURE_IS1 */

#if defined(VTSS_FEATURE_IS2)
    /* VCAP IS2*/
    VTSS_RC(vtss_cmn_vcap_res_check(&vtss_state->vcap.is2.obj, &res->is2));
#endif /* VTSS_FEATURE_IS2 */

#if defined(VTSS_FEATURE_ES0)
    /* VCAP ES0 */
    VTSS_RC(vtss_cmn_vcap_res_check(&vtss_state->vcap.es0.obj, &res->es0));
#endif /* VTSS_FEATURE_IS0 */

    return VTSS_RC_OK;
}

/* - Range checkers ------------------------------------------------ */

/* Determine if UDP/TCP rule */
BOOL vtss_vcap_udp_tcp_rule(const vtss_vcap_u8_t *proto)
{
    return (proto->mask == 0xff && (proto->value == 6 || proto->value == 17));
}

/* Allocate range checker */
vtss_rc vtss_vcap_range_alloc(vtss_vcap_range_chk_table_t *range_chk_table,
                              u32 *range,
                              vtss_vcap_range_chk_type_t type,
                              u32 min,
                              u32 max)
{
    u32                   i, new = VTSS_VCAP_RANGE_CHK_NONE;
    vtss_vcap_range_chk_t *entry;
    
    for (i = 0; i < VTSS_VCAP_RANGE_CHK_CNT; i++) {
        entry = &range_chk_table->entry[i];
        if (entry->count == 0) {
            /* Free entry found, possibly reuse old range checker */
            if (new == VTSS_VCAP_RANGE_CHK_NONE || i == *range)
                new = i;
        } else if (entry->type == type && entry->min == min && entry->max == max) {
            /* Matching used entry */
            new = i;
            break;
        }
    }
    
    if (new == VTSS_VCAP_RANGE_CHK_NONE) {
        VTSS_I("no more range checkers");
        return VTSS_RC_ERROR;
    }

    VTSS_I("alloc range: %u, min: %u, max: %u", new, min, max);

    entry = &range_chk_table->entry[new];
    entry->count++;
    entry->type = type;
    entry->min = min;
    entry->max = max;
    *range = new;
    return VTSS_RC_OK;
}

/* Free VCAP range checker */
vtss_rc vtss_vcap_range_free(vtss_vcap_range_chk_table_t *range_chk_table,
                             u32 range)
{
    vtss_vcap_range_chk_t *entry;

    /* Ignore this special value */
    if (range == VTSS_VCAP_RANGE_CHK_NONE)
        return VTSS_RC_OK;

    if (range >= VTSS_VCAP_RANGE_CHK_CNT) {
        VTSS_E("illegal range: %u", range);
        return VTSS_RC_ERROR;
    }

    entry = &range_chk_table->entry[range];
    if (entry->count == 0) {
        VTSS_E("illegal range free: %u", range);
        return VTSS_RC_ERROR;
    }
    
    entry->count--;
    return VTSS_RC_OK;
}

/* Allocate UDP/TCP range checker */
vtss_rc vtss_vcap_udp_tcp_range_alloc(vtss_vcap_range_chk_table_t *range_chk_table,
                                      u32 *range, 
                                      const vtss_vcap_udp_tcp_t *port, 
                                      BOOL sport)
{
    vtss_vcap_range_chk_type_t type;
    
    if (port->low == port->high || (port->low == 0 && port->high == 0xffff)) {
        /* No range checker required */
        *range = VTSS_VCAP_RANGE_CHK_NONE;
        return VTSS_RC_OK;
    }
    type = (sport ? VTSS_VCAP_RANGE_TYPE_SPORT : VTSS_VCAP_RANGE_TYPE_DPORT);
    return vtss_vcap_range_alloc(range_chk_table, range, type, port->low, port->high);
}

/* Allocate universal range checker
   NOTE: If it is possible to change an inclusive range to a value/mask,
   the vr is modified here in order to save a range checker.
*/
vtss_rc vtss_vcap_vr_alloc(vtss_vcap_range_chk_table_t *range_chk_table,
                           u32 *range,
                           vtss_vcap_range_chk_type_t type,
                           vtss_vcap_vr_t *vr)
{
    u8 bits = (type == VTSS_VCAP_RANGE_TYPE_DSCP ? 6 :
               type == VTSS_VCAP_RANGE_TYPE_VID ? 12 : 16);
    u32 max_value = (1 << bits) - 1;

    /* Parameter check */
    if (vr->type == VTSS_VCAP_VR_TYPE_VALUE_MASK) {
        if (vr->vr.v.value > max_value) {
            VTSS_E("illegal value: 0x%X (max value is 0x%X)", vr->vr.v.value, max_value);
            return VTSS_RC_ERROR;
        }
    }
    else {
        if ((vr->vr.r.low > max_value) || (vr->vr.r.high > max_value) || (vr->vr.r.low > vr->vr.r.high)){
            VTSS_E("illegal range: 0x%X-0x%X", vr->vr.r.low, vr->vr.r.high);
            return VTSS_RC_ERROR;
        }
    }

    /* Try to convert an inclusive range to value/mask
     * The following ranges can be converted:
     * [n;n]
     * [0;((2^n)-1)]
     * [(2^n);((2^(n+1))-1)]
     * Examples:
     * [0;0], [1;1], [2;2], [3;3], [4;4], [5;5], ...
     * [0;1], [0;3], [0;7], [0;15], [0;31], [0;63],...
     * [1;1], [2;3], [4;7], [8;15], [16;31], [32;63], ...
     */
    if (vr->type == VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE) {
        vtss_vcap_vr_value_t mask;
        vtss_vcap_vr_value_t low  = vr->vr.r.low;
        vtss_vcap_vr_value_t high = vr->vr.r.high;
        int bit;

        for (bit = bits - 1; bit >= 0; bit--) {
            mask = 1 << bit;
            if ((low & mask) != (high & mask)) {
                break;
            }
        }
        mask = (1 << (bit+1)) - 1;

        if (((low & ~mask) == low) && ((high & mask) == mask)) {
            vr->type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
            vr->vr.v.value = low;            
            vr->vr.v.mask = ~mask;            
            VTSS_I("range %u-%u converted to value 0x%X, mask 0x%X", low, high, vr->vr.v.value, vr->vr.v.mask);
        }
    }

    if (vr->type == VTSS_VCAP_VR_TYPE_VALUE_MASK) {
        /* No range checker required */
        *range = VTSS_VCAP_RANGE_CHK_NONE;
        return VTSS_RC_OK;
    }
    return vtss_vcap_range_alloc(range_chk_table, range, type, vr->vr.r.low, vr->vr.r.high);
}

vtss_rc vtss_vcap_range_commit(vtss_state_t *vtss_state, vtss_vcap_range_chk_table_t *range_new)
{
    if (memcmp(&vtss_state->vcap.range, range_new, sizeof(*range_new))) {
        /* The temporary working copy has changed - Save it and commit */
        vtss_state->vcap.range = *range_new;
        return VTSS_FUNC_COLD_0(vcap.range_commit);
    }
    return VTSS_RC_OK;
}

/* - VCAP table functions ------------------------------------------ */

/* Return number of rules per row for a given key */
u32 vtss_vcap_key_rule_count(vtss_vcap_key_size_t key_size)
{
    if (key_size > VTSS_VCAP_KEY_SIZE_LAST) {
        VTSS_E("illegal key_size");
    }
    return (key_size == VTSS_VCAP_KEY_SIZE_QUARTER ? 4 :
            key_size == VTSS_VCAP_KEY_SIZE_HALF ? 2 : 1);
}

const char *vtss_vcap_key_size_txt(vtss_vcap_key_size_t key_size)
{
    return (key_size == VTSS_VCAP_KEY_SIZE_FULL ? "Full" :
            key_size == VTSS_VCAP_KEY_SIZE_HALF ? "Half" :
            key_size == VTSS_VCAP_KEY_SIZE_QUARTER ? "Quarter" : "?");
}

#if defined(VTSS_ARCH_SERVAL)
const char *vtss_vcap_key_type_txt(vtss_vcap_key_type_t key_type)
{
    return (key_type == VTSS_VCAP_KEY_TYPE_NORMAL ? "NORMAL" :
            key_type == VTSS_VCAP_KEY_TYPE_DOUBLE_TAG ? "DOUBLE_TAG" :
            key_type == VTSS_VCAP_KEY_TYPE_IP_ADDR ? "IP_ADDR" :
            key_type == VTSS_VCAP_KEY_TYPE_MAC_IP_ADDR ? "MAC_IP_ADDR" : "?");
}
vtss_vcap_key_size_t vtss_vcap_key_type_to_size(vtss_vcap_key_type_t key_type)
{
    return (key_type == VTSS_VCAP_KEY_TYPE_MAC_IP_ADDR ? VTSS_VCAP_KEY_SIZE_FULL :
            key_type == VTSS_VCAP_KEY_TYPE_DOUBLE_TAG ? VTSS_VCAP_KEY_SIZE_QUARTER :
            VTSS_VCAP_KEY_SIZE_HALF);
}

#endif /* defined(VTSS_ARCH_SERVAL) */

/* Get (row, col) position of rule */
static void vtss_vcap_pos_get(vtss_vcap_obj_t *obj, vtss_vcap_idx_t *idx, u32 ndx)
{
    u32                  cnt;
    vtss_vcap_key_size_t key_size;

    /* Use index to find (row, col) within own block */
    cnt = vtss_vcap_key_rule_count(idx->key_size);
    idx->row = (ndx / cnt);
    idx->col = (ndx % cnt);

    /* Include quarter block rows */
    if ((key_size = VTSS_VCAP_KEY_SIZE_QUARTER) > idx->key_size) {
        cnt = vtss_vcap_key_rule_count(key_size);
        idx->row += ((obj->key_count[key_size] + cnt - 1) / cnt);
    }

    /* Include half block rows */
    if ((key_size = VTSS_VCAP_KEY_SIZE_HALF) > idx->key_size) {
        cnt = vtss_vcap_key_rule_count(key_size);
        idx->row += ((obj->key_count[key_size] + cnt - 1) / cnt);
    }
}

char *vtss_vcap_id_txt(vtss_state_t *vtss_state, vtss_vcap_id_t id)
{
    u32  high = ((id >> 32) & 0xffffffff);
    u32  low = (id & 0xffffffff);
    char *txt;

    vtss_state->txt_buf_index++;
    txt = &vtss_state->txt_buf[(vtss_state->txt_buf_index & 1) ? 0 : 32];
    sprintf(txt, "0x%08x:0x%08x", high, low);
    return txt;
}

/* Lookup VCAP entry */
vtss_rc vtss_vcap_lookup(vtss_state_t *vtss_state,
                         vtss_vcap_obj_t *obj, int user, vtss_vcap_id_t id, 
                         vtss_vcap_data_t *data, vtss_vcap_idx_t *idx)
{
    u32                  ndx[VTSS_VCAP_KEY_SIZE_MAX];
    vtss_vcap_entry_t    *cur;
    vtss_vcap_key_size_t key_size;

    VTSS_D("VCAP %s, id: %s", obj->name, vtss_vcap_id_txt(vtss_state, id));
    
    memset(ndx, 0, sizeof(ndx));

    for (cur = obj->used; cur != NULL; cur = cur->next) {
        key_size = cur->data.key_size;
        if (cur->user == user && cur->id == id) {
            if (idx != NULL) {
                idx->key_size = key_size;
                vtss_vcap_pos_get(obj, idx, ndx[key_size]);
            }
            if (data != NULL)
                *data = cur->data;
            return VTSS_RC_OK;
        }
        ndx[key_size]++;
    }
    return VTSS_RC_ERROR;
}

/* Delete rule found in list */
static vtss_rc vtss_vcap_del_rule(vtss_state_t *vtss_state,
                                  vtss_vcap_obj_t *obj, vtss_vcap_entry_t *cur, 
                                  vtss_vcap_entry_t *prev, u32 ndx)
{
    vtss_vcap_key_size_t key_size;
    vtss_vcap_idx_t      idx;
    u32                  cnt;
    
    VTSS_D("VCAP %s, ndx: %u", obj->name, ndx);

    /* Move rule to free list */
    if (prev == NULL)
        obj->used = cur->next;
    else
        prev->next = cur->next;
    cur->next = obj->free;
    obj->free = cur;
    obj->rule_count--;
    
    /* Delete VCAP entry from block */
    key_size = cur->data.key_size;
    idx.key_size = key_size;
    obj->key_count[key_size]--;
    cnt = obj->key_count[key_size];
    if (!vtss_state->warm_start_cur) {
        /* Avoid VCAP update in warm start mode */
        if (ndx == cnt) {
            /* Last rule in block, just delete */
            vtss_vcap_pos_get(obj, &idx, ndx);
            VTSS_RC(obj->entry_del(vtss_state, &idx));
        } else {
            /* Delete and contract by moving rules up */
            vtss_vcap_pos_get(obj, &idx, ndx + 1);
            VTSS_RC(obj->entry_move(vtss_state, &idx, cnt - ndx, 1));
        }
    }
    
    /* Get position of the entry after the last entry in block */
    vtss_vcap_pos_get(obj, &idx, cnt);
    if (idx.col) {
        /* Done, there are more rules on the last row */
        return VTSS_RC_OK;
    }
    
    /* Delete and contract by moving rows up */
    obj->count--;
    if (!vtss_state->warm_start_cur && idx.row != obj->count) {
        cnt = (obj->count - idx.row);
        idx.key_size = VTSS_VCAP_KEY_SIZE_FULL;
        idx.row++;
        VTSS_RC(obj->entry_move(vtss_state, &idx, cnt, 1));
    }
    return VTSS_RC_OK;
}

/* Delete VCAP rule */
vtss_rc vtss_vcap_del(vtss_state_t *vtss_state,
                      vtss_vcap_obj_t *obj, int user, vtss_vcap_id_t id)
{
    vtss_vcap_entry_t    *cur, *prev = NULL;
    vtss_vcap_key_size_t key_size;
    u32                  ndx[VTSS_VCAP_KEY_SIZE_MAX];

    VTSS_D("VCAP %s, id: %s", obj->name, vtss_vcap_id_txt(vtss_state, id));

    memset(ndx, 0, sizeof(ndx));
    for (cur = obj->used; cur != NULL; prev = cur, cur = cur->next) {
        key_size = cur->data.key_size;
        if (cur->user == user && cur->id == id) {
            /* Found rule, delete it */
            return vtss_vcap_del_rule(vtss_state, obj, cur, prev, ndx[key_size]);
        }
        ndx[key_size]++;
    }

    /* Silently ignore if rule not found */
    return VTSS_RC_OK;
}

vtss_rc vtss_vcap_add(vtss_state_t *vtss_state, vtss_vcap_obj_t *obj, int user, vtss_vcap_id_t id, 
                      vtss_vcap_id_t ins_id, vtss_vcap_data_t *data, BOOL dont_add)
{
    u32                  cnt = 0, ndx_ins = 0, ndx_old = 0, ndx_old_key[VTSS_VCAP_KEY_SIZE_MAX];
    vtss_vcap_entry_t    *cur, *prev = NULL;
    vtss_vcap_entry_t    *old = NULL, *old_prev = NULL, *ins = NULL, *ins_prev = NULL;
    vtss_vcap_idx_t      idx;
    vtss_vcap_key_size_t key_size, key_size_new;
    vtss_res_chg_t       chg;
    vtss_vcap_id_t       cur_id;
    
    key_size_new = (data ? data->key_size : VTSS_VCAP_KEY_SIZE_FULL);
    memset(ndx_old_key, 0, sizeof(ndx_old_key));

    VTSS_D("VCAP %s, key_size: %s, id: %s, ins_id: %s", 
           obj->name, vtss_vcap_key_size_txt(key_size_new),
           vtss_vcap_id_txt(vtss_state, id), vtss_vcap_id_txt(vtss_state, ins_id));
    
    for (cur = obj->used; cur != NULL; prev = cur, cur = cur->next) {
        /* No further processing if bigger user found */
        if (cur->user > user)
            break;

        /* Look for existing ID and next ID */
        if (cur->user == user) {
            cur_id = cur->id;
            if (cur_id == id) {
                /* Found ID */
                VTSS_D("found old id");
                old = cur;
                old_prev = prev;
            }

            if (cur_id == ins_id) {
                /* Found place to insert */
                VTSS_D("found ins_id");
                ins = cur;
                ins_prev = prev;
            }
        }

        /* Count entries depending on key size */
        key_size = cur->data.key_size;
        if (key_size > VTSS_VCAP_KEY_SIZE_LAST) {
            VTSS_E("VCAP %s key size exceeded", obj->name);
            return VTSS_RC_ERROR;
        }
        
        /* Count number of rules smaller than insert entry */
        if (ins == NULL && key_size == key_size_new)
            ndx_ins++;

        /* Count number of rules smaller than old entry */
        if (old == NULL)
            ndx_old_key[key_size]++;
    }
    
    /* Check if insert ID is valid */
    if (ins == NULL) {
        if (ins_id != VTSS_VCAP_ID_LAST) {
            VTSS_E("VCAP %s: Could not find ID: %" PRIu64, obj->name, ins_id);
            return VTSS_RC_ERROR;
        }
        ins_prev = prev;
    }

    /* Check if resources are available */
    if (old == NULL || old->data.key_size != key_size_new) {
        memset(&chg, 0, sizeof(chg));

        /* Calculate added resources */
        if ((obj->key_count[key_size_new] % vtss_vcap_key_rule_count(key_size_new)) == 0)
            chg.add++;

        /* Calculate deleted resources */
        if (old != NULL) {
            key_size = old->data.key_size;
            if ((obj->key_count[key_size] % vtss_vcap_key_rule_count(key_size)) == 1)
                chg.del++;
        }
        VTSS_RC(vtss_cmn_vcap_res_check(obj, &chg));
    }
    
    /* Return here if only checking if entry can be added */
    if (data == NULL || dont_add)
        return VTSS_RC_OK;

    /* Read counter */
    if (old == NULL) {
        key_size = key_size_new; /* Just to please Lint */
    } else {
        key_size = old->data.key_size;
        ndx_old = ndx_old_key[key_size];
        idx.key_size = key_size;
        vtss_vcap_pos_get(obj, &idx, ndx_old);
        if (!vtss_state->warm_start_cur) {
            /* No need to read counter in warm start mode */
            VTSS_RC(obj->entry_get(vtss_state, &idx, &cnt, 0));
        }
    }
    
    if (old == NULL || key_size != key_size_new || (ndx_old + 1) != ndx_ins) {
        /* New entry or changed key size/position, delete/add is required */
        if (old == NULL) {
            VTSS_D("new rule, ndx_ins: %u", ndx_ins);
        } else {
            VTSS_D("changed key_size/position");
            VTSS_RC(vtss_vcap_del_rule(vtss_state, obj, old, old_prev, ndx_old));
            if (key_size == key_size_new) {
                VTSS_D("new position, ndx_ins: %u, ndx_old: %u", ndx_ins, ndx_old);
                if (ndx_ins > ndx_old) {
                    /* The deleted old entry was before the new index, adjust for that */
                    ndx_ins--;
                }
            } else {
                VTSS_D("new key_size, ndx_ins: %u", ndx_ins);
            }
            if (ins_prev == old) {
                /* Old entry was just deleted, adjust for that */
                ins_prev = old_prev;
            }
        }
        
        /* Insert new rule in used list */
        if ((cur = obj->free) == NULL) {
            VTSS_E("VCAP %s: No more free rules", obj->name);
            return VTSS_RC_ERROR;
        }
        obj->free = cur->next;
        if (ins_prev == NULL) {
            cur->next = obj->used;
            obj->used = cur;
        } else {
            cur->next = ins_prev->next;
            ins_prev->next = cur;
        }
        obj->rule_count++;
        cur->user = user;
        cur->id = id;
        
        /* Get position of the entry after the last entry in block */
        key_size = key_size_new;
        idx.key_size = key_size;
        vtss_vcap_pos_get(obj, &idx, key_size == VTSS_VCAP_KEY_SIZE_FULL ? ndx_ins : 
                          obj->key_count[key_size]);
        if (idx.col == 0) {
            if (!vtss_state->warm_start_cur && idx.row < obj->count) {
                /* Move rows down */
                idx.key_size = VTSS_VCAP_KEY_SIZE_FULL;
                VTSS_RC(obj->entry_move(vtss_state, &idx, obj->count - idx.row, 0));
            }
            obj->count++;
        }

        /* Move rules down */
        if (!vtss_state->warm_start_cur && key_size != VTSS_VCAP_KEY_SIZE_FULL &&
            obj->key_count[key_size] > ndx_ins) {
            idx.key_size = key_size;
            vtss_vcap_pos_get(obj, &idx, ndx_ins);
            VTSS_RC(obj->entry_move(vtss_state, &idx, obj->key_count[key_size] - ndx_ins, 0));
        }
        obj->key_count[key_size]++;
    } else {
        VTSS_D("rule unchanged");
        cur = old;
        ndx_ins = ndx_old;
    }
    
    cur->data = *data;

    /* Save a copy if storage is provided. Used for warm start and key size changes */
    data = &cur->data;
    if (obj->type == VTSS_VCAP_TYPE_IS0) {
#if defined(VTSS_FEATURE_IS0)
        vtss_is0_entry_t *copy = cur->copy;
        if (copy) {
            *copy = *data->u.is0.entry;
        }
#endif /* VTSS_FEATURE_IS0 */
    } else if (obj->type == VTSS_VCAP_TYPE_IS1) {
#if defined(VTSS_FEATURE_IS1)
        vtss_is1_entry_t *copy = cur->copy;
        if (copy) {
            *copy = *data->u.is1.entry;
        }
#endif /* VTSS_FEATURE_IS1 */
    } else if (obj->type == VTSS_VCAP_TYPE_IS2) {
#if defined(VTSS_FEATURE_IS2)
        vtss_is2_entry_t *copy = cur->copy;
        if (copy) {
            *copy = *data->u.is2.entry;
        }
#endif /* VTSS_FEATURE_IS2 */
    } else if (obj->type == VTSS_VCAP_TYPE_ES0) {
#if defined(VTSS_FEATURE_ES0)
        vtss_es0_entry_t *copy = cur->copy;
        if (copy) {
            *copy = *data->u.es0.entry;
        }
#endif /* VTSS_FEATURE_ES0 */
    }

    /* Write entry */
    if (vtss_state->warm_start_cur) {
        return VTSS_RC_OK;
    } else {
        idx.key_size = key_size_new;
        vtss_vcap_pos_get(obj, &idx, ndx_ins);
        return obj->entry_add(vtss_state, &idx, data, cnt);
    }
}

/* Get next ID for one user based on another user (special function for PTP) */
vtss_rc vtss_vcap_get_next_id(vtss_vcap_obj_t *obj, int user1, int user2, 
                              vtss_vcap_id_t id, vtss_vcap_id_t *ins_id)
{
    vtss_vcap_entry_t *cur, *next;

    /* Look for entry in user1 list */
    *ins_id = VTSS_VCAP_ID_LAST;
    for (cur = obj->used; cur != NULL; cur = cur->next) {
        if (cur->user == user1 && cur->id == id) {
            /* Found entry */
            next = cur->next;
            break;
        }
    }
    
    if (cur == NULL) {
        VTSS_E("VCAP %s: ID not found", obj->name);
        return VTSS_RC_ERROR;
    }

    /* Look for entry in user2 list */
    for (next = cur->next; next != NULL && next->user == user1; next = next->next) {
        for (cur = obj->used; cur != NULL; cur = cur->next) {
            if (cur->user == user2 && cur->id == next->id) {
                *ins_id = cur->id;
                return VTSS_RC_OK;
            }
        }
    }
    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_IS0)
void vtss_vcap_is0_init(vtss_vcap_data_t *data, vtss_is0_entry_t *entry)
{
    vtss_is0_data_t *is0 = &data->u.is0;

    memset(data, 0, sizeof(*data));
    memset(entry, 0, sizeof(*entry));
    is0->entry = entry;
}
#endif /* VTSS_FEATURE_IS0 */

#if defined(VTSS_FEATURE_IS1)
void vtss_vcap_is1_init(vtss_vcap_data_t *data, vtss_is1_entry_t *entry)
{
    vtss_is1_data_t *is1 = &data->u.is1;

    memset(data, 0, sizeof(*data));
    memset(entry, 0, sizeof(*entry));
    is1->vid_range = VTSS_VCAP_RANGE_CHK_NONE;
    is1->dscp_range = VTSS_VCAP_RANGE_CHK_NONE;
    is1->sport_range = VTSS_VCAP_RANGE_CHK_NONE;
    is1->dport_range = VTSS_VCAP_RANGE_CHK_NONE;
    is1->entry = entry;
}
#endif /* VTSS_FEATURE_IS1 */

#if defined(VTSS_FEATURE_IS2)
void vtss_vcap_is2_init(vtss_vcap_data_t *data, vtss_is2_entry_t *entry)
{
    vtss_is2_data_t *is2 = &data->u.is2;

    memset(data, 0, sizeof(*data));
    memset(entry, 0, sizeof(*entry));
    is2->srange = VTSS_VCAP_RANGE_CHK_NONE;
    is2->drange = VTSS_VCAP_RANGE_CHK_NONE;
    is2->entry = entry;
}
#endif /* VTSS_FEATURE_IS2 */

#if defined(VTSS_FEATURE_ES0)
void vtss_vcap_es0_init(vtss_vcap_data_t *data, vtss_es0_entry_t *entry)
{
    memset(data, 0, sizeof(*data));
    memset(entry, 0, sizeof(*entry));
    data->u.es0.entry = entry;
    entry->key.rx_port_no = VTSS_PORT_NO_NONE;
}

/* Update ES0 action fields based on VLAN and QoS port configuration */
void vtss_cmn_es0_action_get(vtss_state_t *vtss_state, vtss_es0_data_t *es0)
{
    vtss_es0_action_t *action = &es0->entry->action;
    
    if (es0->flags & VTSS_ES0_FLAG_TPID) {
        /* Update TPID action */
        switch (vtss_state->l2.vlan_port_conf[es0->port_no].port_type) {
        case VTSS_VLAN_PORT_TYPE_S:
            action->tpid = VTSS_ES0_TPID_S;
            break;
        case VTSS_VLAN_PORT_TYPE_S_CUSTOM:
            action->tpid = VTSS_ES0_TPID_PORT;
            break;
        default:
            action->tpid = VTSS_ES0_TPID_C;
            break;
        }
#if defined(VTSS_ARCH_SERVAL)
        if (vtss_state->arch == VTSS_ARCH_SRVL) {
            action->outer_tag.tpid = action->tpid;
        }
#endif /* VTSS_ARCH_CARACAL */
    }

    if (es0->flags & VTSS_ES0_FLAG_QOS) {
        /* Update QoS action */
        vtss_qos_port_conf_t *qos = &vtss_state->qos.port_conf[es0->port_no];
        
        action->pcp = qos->tag_default_pcp;
        action->dei = qos->tag_default_dei;
        switch (qos->tag_remark_mode) {
        case VTSS_TAG_REMARK_MODE_CLASSIFIED:
            action->qos = VTSS_ES0_QOS_CLASS; 
            break;
        case VTSS_TAG_REMARK_MODE_MAPPED:
            action->qos = VTSS_ES0_QOS_MAPPED; 
            break;
        case VTSS_TAG_REMARK_MODE_DEFAULT:
        default:
            action->qos = VTSS_ES0_QOS_ES0; 
            break;
        }
#if defined(VTSS_ARCH_SERVAL)
        if (vtss_state->arch == VTSS_ARCH_SRVL) {
            vtss_es0_tag_conf_t *tag = &action->outer_tag;

            tag->pcp.val = action->pcp;
            tag->dei.val = action->dei;
            switch (qos->tag_remark_mode) {
            case VTSS_TAG_REMARK_MODE_CLASSIFIED:
                tag->pcp.sel = VTSS_ES0_PCP_CLASS; 
                tag->dei.sel = VTSS_ES0_DEI_CLASS;
                break;
            case VTSS_TAG_REMARK_MODE_MAPPED:
                tag->pcp.sel = VTSS_ES0_PCP_MAPPED; 
                tag->dei.sel = VTSS_ES0_DEI_MAPPED;
                break;
            case VTSS_TAG_REMARK_MODE_DEFAULT:
            default:
                tag->pcp.sel = VTSS_ES0_PCP_ES0; 
                tag->dei.sel = VTSS_ES0_DEI_ES0;
                break;
            }
        }
#endif /* VTSS_ARCH_CARACAL */
    }
}

vtss_rc vtss_vcap_es0_update(vtss_state_t *vtss_state,
                             const vtss_port_no_t port_no, u16 flags)
{
    vtss_vcap_entry_t *cur;
    vtss_es0_data_t   *data;
    vtss_vcap_idx_t   idx;
    vtss_es0_entry_t  entry;
        
    /* Avoid updating ES0 in warm start mode */
    if (vtss_state->warm_start_cur)
        return VTSS_RC_OK;

    memset(&idx, 0, sizeof(idx));
    for (cur = vtss_state->vcap.es0.obj.used; cur != NULL; cur = cur->next, idx.row++) {
        data = &cur->data.u.es0;
        if ((data->port_no == port_no && (data->flags & flags & VTSS_ES0_FLAG_MASK_PORT)) || 
            (data->nni == port_no && (data->flags & flags & VTSS_ES0_FLAG_MASK_NNI))) {
            data->entry = &entry;
            vtss_cmn_es0_action_get(vtss_state, data);
            VTSS_FUNC_RC(vcap.es0_entry_update, &idx, data);
        }
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_ES0 */

/* - ACL ----------------------------------------------------------- */

/* Add ACE check */
vtss_rc vtss_cmn_ace_add(vtss_state_t *vtss_state,
                         const vtss_ace_id_t ace_id, const vtss_ace_t *const ace)
{
    /* Check ACE ID */
    if (ace->id == VTSS_ACE_ID_LAST || ace->id == ace_id) {
        VTSS_E("illegal ace id: %u", ace->id);
        return VTSS_RC_ERROR;
    }
    
    /* Check frame type */
    if (ace->type > VTSS_ACE_TYPE_IPV6) {
        VTSS_E("illegal type: %d", ace->type);
        return VTSS_RC_ERROR;
    }
    
    return VTSS_RC_OK;
}

/* Delete ACE */
vtss_rc vtss_cmn_ace_del(vtss_state_t *vtss_state, const vtss_ace_id_t ace_id)
{
    vtss_vcap_obj_t  *obj = &vtss_state->vcap.is2.obj;
    vtss_vcap_data_t data;
    
    if (vtss_vcap_lookup(vtss_state, obj, VTSS_IS2_USER_ACL, ace_id, &data, NULL) != VTSS_RC_OK) {
        VTSS_E("ace_id: %u not found", ace_id);
        return VTSS_RC_ERROR;
    }
    
    /* Delete range checkers and main entry */
    VTSS_RC(vtss_vcap_range_free(&vtss_state->vcap.range, data.u.is2.srange));
    VTSS_RC(vtss_vcap_range_free(&vtss_state->vcap.range, data.u.is2.drange));
    VTSS_RC(vtss_vcap_del(vtss_state, obj, VTSS_IS2_USER_ACL, ace_id));
    
    return VTSS_RC_OK;
}

/* Get/clear ACE counter */
static vtss_rc vtss_cmn_ace_get(vtss_state_t *vtss_state,
                                const vtss_ace_id_t ace_id, 
                                vtss_ace_counter_t *const counter, 
                                BOOL clear)
{
    vtss_vcap_idx_t idx;
    vtss_vcap_obj_t *obj = &vtss_state->vcap.is2.obj;
    
    if (vtss_vcap_lookup(vtss_state, obj, VTSS_IS2_USER_ACL, ace_id, NULL, &idx) != VTSS_RC_OK) {
        VTSS_E("ace_id: %u not found", ace_id);
        return VTSS_RC_ERROR;
    }
    VTSS_RC(obj->entry_get(vtss_state, &idx, counter, clear));
    
    return VTSS_RC_OK;
}

/* Get ACE counter */
vtss_rc vtss_cmn_ace_counter_get(vtss_state_t *vtss_state,
                                 const vtss_ace_id_t ace_id, vtss_ace_counter_t *const counter)
{
    return vtss_cmn_ace_get(vtss_state, ace_id, counter, 0);
}

/* Clear ACE counter */
vtss_rc vtss_cmn_ace_counter_clear(vtss_state_t *vtss_state, const vtss_ace_id_t ace_id)
{
    vtss_ace_counter_t counter;
    
    return vtss_cmn_ace_get(vtss_state, ace_id, &counter, 1);
}

char *vtss_acl_policy_no_txt(vtss_acl_policy_no_t policy_no, char *buf)
{
    if (policy_no == VTSS_ACL_POLICY_NO_NONE)
        strcpy(buf, "None");
    else
        sprintf(buf, "%u", policy_no);
    return buf;
}

/* - Access Control Lists ------------------------------------------ */

static vtss_rc vtss_acl_policer_no_check(const vtss_acl_policer_no_t  policer_no)
{
    if (policer_no >= VTSS_ACL_POLICERS) {
        VTSS_E("illegal policer_no: %u", policer_no);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_acl_policer_conf_get(const vtss_inst_t            inst,
                                  const vtss_acl_policer_no_t  policer_no,
                                  vtss_acl_policer_conf_t      *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("policer_no: %u", policer_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK &&
        (rc = vtss_acl_policer_no_check(policer_no)) == VTSS_RC_OK)
        *conf = vtss_state->vcap.acl_policer_conf[policer_no];
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_acl_policer_conf_set(const vtss_inst_t              inst,
                                  const vtss_acl_policer_no_t    policer_no,
                                  const vtss_acl_policer_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("policer_no: %u", policer_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK &&
        (rc = vtss_acl_policer_no_check(policer_no)) == VTSS_RC_OK) {
        vtss_state->vcap.acl_policer_conf[policer_no] = *conf;
        rc = VTSS_FUNC_COLD(vcap.acl_policer_set, policer_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_acl_port_conf_get(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_acl_port_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK)
        *conf = vtss_state->vcap.acl_port_conf[port_no];
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_acl_port_conf_set(const vtss_inst_t           inst,
                               const vtss_port_no_t        port_no,
                               const vtss_acl_port_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->vcap.acl_old_port_conf = vtss_state->vcap.acl_port_conf[port_no];
        vtss_state->vcap.acl_port_conf[port_no] = *conf;
        rc = VTSS_FUNC_COLD(vcap.acl_port_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_acl_port_counter_get(const vtss_inst_t        inst,
                                  const vtss_port_no_t     port_no,
                                  vtss_acl_port_counter_t  *const counter)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(vcap.acl_port_counter_get, port_no, counter);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_acl_port_counter_clear(const vtss_inst_t     inst,
                                    const vtss_port_no_t  port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(vcap.acl_port_counter_clear, port_no);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_ace_init(const vtss_inst_t      inst,
                      const vtss_ace_type_t  type,
                      vtss_ace_t             *const ace)
{
    VTSS_D("type: %d", type);

    memset(ace, 0, sizeof(*ace));
    ace->type = type;
#if defined(VTSS_FEATURE_ACL_V1)
    ace->action.forward = 1;
#endif /* VTSS_FEATURE_ACL_V1 */
    ace->action.learn = 1;

    switch (type) {
    case VTSS_ACE_TYPE_ANY:
    case VTSS_ACE_TYPE_ETYPE:
    case VTSS_ACE_TYPE_LLC:
    case VTSS_ACE_TYPE_SNAP:
    case VTSS_ACE_TYPE_ARP:
        break;
    case VTSS_ACE_TYPE_IPV4:
        ace->frame.ipv4.sport.high = 0xffff;
        ace->frame.ipv4.sport.in_range = 1;
        ace->frame.ipv4.dport.high = 0xffff;
        ace->frame.ipv4.dport.in_range = 1;
        break;
    case VTSS_ACE_TYPE_IPV6:
        ace->frame.ipv6.sport.high = 0xffff;
        ace->frame.ipv6.sport.in_range = 1;
        ace->frame.ipv6.dport.high = 0xffff;
        ace->frame.ipv6.dport.in_range = 1;
        break;
    default:
        VTSS_E("unknown type: %d", type);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_ace_add(const vtss_inst_t    inst,
                     const vtss_ace_id_t  ace_id,
                     const vtss_ace_t     *const ace)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("ace_id: %u before %u %s",
           ace->id, ace_id, ace_id == VTSS_ACE_ID_LAST ? "(last)" : "");

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(vcap.acl_ace_add, ace_id, ace);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_ace_del(const vtss_inst_t    inst,
                     const vtss_ace_id_t  ace_id)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("ace_id: %u", ace_id);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(vcap.acl_ace_del, ace_id);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_ace_counter_get(const vtss_inst_t    inst,
                             const vtss_ace_id_t  ace_id,
                             vtss_ace_counter_t   *const counter)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("ace_id: %u", ace_id);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(vcap.acl_ace_counter_get, ace_id, counter);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_ace_counter_clear(const vtss_inst_t    inst,
                               const vtss_ace_id_t  ace_id)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("ace_id: %u", ace_id);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(vcap.acl_ace_counter_clear, ace_id);
    VTSS_EXIT();
    return rc;
}

/* - Warm start synchronization ------------------------------------ */

static vtss_rc vtss_vcap_sync_obj(vtss_state_t *vtss_state, vtss_vcap_obj_t *obj)
{
#if defined(VTSS_OPT_WARM_START)
    vtss_vcap_entry_t    *cur;
    vtss_vcap_data_t     *data;
    vtss_vcap_idx_t      idx;
    vtss_vcap_key_size_t key_size;
    u32                  ndx[VTSS_VCAP_KEY_SIZE_MAX];

    VTSS_I("VCAP %s", obj->name);

    /* Add/update entries */
    memset(ndx, 0, sizeof(ndx));
    for (cur = obj->used; cur != NULL; cur = cur->next) {
        if (cur->copy == NULL) {
            VTSS_E("VCAP %s: No saved copy", obj->name);
            return VTSS_RC_ERROR;
        }
        data = &cur->data;
        key_size = data->key_size;
        if (obj->type == VTSS_VCAP_TYPE_IS0) {
#if defined(VTSS_FEATURE_IS0)
            data->u.is0.entry = (vtss_is0_entry_t *)cur->copy;
#endif /* VTSS_FEATURE_IS0 */
        } else if (obj->type == VTSS_VCAP_TYPE_IS1) {
#if defined(VTSS_FEATURE_IS1)
            data->u.is1.entry = (vtss_is1_entry_t *)cur->copy;
#endif /* VTSS_FEATURE_IS1 */
        } else if (obj->type == VTSS_VCAP_TYPE_IS2) {
#if defined(VTSS_FEATURE_IS2)
            data->u.is2.entry = (vtss_is2_entry_t *)cur->copy;
#endif /* VTSS_FEATURE_IS2 */
        } else if (obj->type == VTSS_VCAP_TYPE_ES0) {
#if defined(VTSS_FEATURE_ES0)
            data->u.es0.entry = (vtss_es0_entry_t *)cur->copy;
#endif /* VTSS_FEATURE_ES0 */
        }
        idx.key_size = key_size;
        vtss_vcap_pos_get(obj, &idx, ndx[key_size]);
        VTSS_RC(obj->entry_add(vtss_state, &idx, data, 0));
        ndx[key_size]++;
    }

    /* Delete trailing entries */
    for (key_size = VTSS_VCAP_KEY_SIZE_FULL; key_size <= VTSS_VCAP_KEY_SIZE_LAST; key_size++) {
        idx.key_size = key_size;
        while (1) {
            vtss_vcap_pos_get(obj, &idx, ndx[key_size]);
            if (idx.row >= obj->max_count || (key_size != VTSS_VCAP_KEY_SIZE_FULL && idx.col == 0))
                break;
            VTSS_RC(obj->entry_del(vtss_state, &idx));
            ndx[key_size]++;
        }
    }
#endif /* VTSS_OPT_WARM_START */

    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_WARM_START)
vtss_rc vtss_vcap_restart_sync(vtss_state_t *vtss_state)
{
    vtss_port_no_t        port_no;
    vtss_acl_policer_no_t policer_no;
    
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        VTSS_FUNC_RC(vcap.acl_port_set, port_no);
    }

    for (policer_no = 0; policer_no <  VTSS_ACL_POLICERS; policer_no++) {
        VTSS_FUNC_RC(vcap.acl_policer_set, policer_no);
    }
    
#if defined(VTSS_FEATURE_IS0)
    VTSS_RC(vtss_vcap_sync_obj(vtss_state, &vtss_state->vcap.is0.obj));
#endif /* VTSS_FEATURE_IS0 */
#if defined(VTSS_FEATURE_IS1)
    VTSS_RC(vtss_vcap_sync_obj(vtss_state, &vtss_state->vcap.is1.obj));
#endif /* VTSS_FEATURE_IS1 */
#if defined(VTSS_FEATURE_IS2)
    VTSS_RC(vtss_vcap_sync_obj(vtss_state, &vtss_state->vcap.is2.obj));
#endif /* VTSS_FEATURE_IS2 */
#if defined(VTSS_FEATURE_ES0)
    VTSS_RC(vtss_vcap_sync_obj(vtss_state, &vtss_state->vcap.es0.obj));
#endif /* VTSS_FEATURE_ES0 */

    /* Commit range checkers */
    return VTSS_FUNC_0(vcap.range_commit);
}
#endif /* VTSS_FEATURE_WARM_START */

/* - Instance create and initialization ---------------------------- */

vtss_rc vtss_vcap_inst_create(vtss_state_t *vtss_state)
{
    vtss_port_no_t    port_no;
    vtss_acl_action_t *action;
    vtss_vcap_entry_t *entry;
    u32               i;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        action = &vtss_state->vcap.acl_port_conf[port_no].action;
#if defined(VTSS_FEATURE_ACL_V1)
        action->forward = 1;
#endif /* VTSS_FEATURE_ACL_V1 */
        action->learn = 1;
    }

#if defined(VTSS_FEATURE_IS0)
    {
        vtss_is0_info_t *is0 = &vtss_state->vcap.is0;
        
        /* Add IS0 entries to free list */
        is0->obj.type = VTSS_VCAP_TYPE_IS0;
        is0->obj.name = "IS0";
        if (is0->obj.max_rule_count == 0)
            is0->obj.max_rule_count = is0->obj.max_count;
        for (i = 0; i < is0->obj.max_rule_count; i++) {
            entry = &is0->table[i];
            entry->next = is0->obj.free;
            is0->obj.free = entry;
#if defined(VTSS_OPT_WARM_START)
            entry->copy = &is0->copy[i];
#endif /* VTSS_OPT_WARM_START */
        }
    }
#endif /* VTSS_FEATURE_IS0 */

#if defined(VTSS_FEATURE_IS1)
    {
        vtss_is1_info_t *is1 = &vtss_state->vcap.is1;
        
        /* Add IS1 entries to free list */
        is1->obj.type = VTSS_VCAP_TYPE_IS1;
        is1->obj.name = "IS1";
        if (is1->obj.max_rule_count == 0)
            is1->obj.max_rule_count = is1->obj.max_count;
        for (i = 0; i < is1->obj.max_rule_count; i++) {
            entry = &is1->table[i];
            entry->next = is1->obj.free;
            is1->obj.free = entry;
#if defined(VTSS_OPT_WARM_START) || defined(VTSS_ARCH_SERVAL)
            entry->copy = &is1->copy[i];
#endif /* VTSS_OPT_WARM_START */
        }
    }
#endif /* VTSS_FEATURE_IS1 */

#if defined(VTSS_FEATURE_IS2)
    {
        vtss_is2_info_t *is2 = &vtss_state->vcap.is2;
        
        /* Add IS2 entries to free list */
        is2->obj.type = VTSS_VCAP_TYPE_IS2;
        is2->obj.name = "IS2";
        if (is2->obj.max_rule_count == 0)
            is2->obj.max_rule_count = is2->obj.max_count;
        for (i = 0; i < is2->obj.max_rule_count; i++) {
            entry = &is2->table[i];
            entry->next = is2->obj.free;
            is2->obj.free = entry;
#if defined(VTSS_OPT_WARM_START)
            entry->copy = &is2->copy[i];
#endif /* VTSS_OPT_WARM_START */
        }
    }
#endif /* VTSS_FEATURE_IS2 */

#if defined(VTSS_FEATURE_ES0)
    {
        vtss_es0_info_t *es0 = &vtss_state->vcap.es0;
        
        /* Add ES0 entries to free list */
        es0->obj.type = VTSS_VCAP_TYPE_ES0;
        es0->obj.name = "ES0";
        if (es0->obj.max_rule_count == 0)
            es0->obj.max_rule_count = es0->obj.max_count;
        for (i = 0; i < es0->obj.max_rule_count; i++) {
            entry = &es0->table[i];
            entry->next = es0->obj.free;
            es0->obj.free = entry;
#if defined(VTSS_OPT_WARM_START)
            entry->copy = &es0->copy[i];
#endif /* VTSS_OPT_WARM_START */
        }
    }
#endif /* VTSS_FEATURE_ES0 */

    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

void vtss_vcap_debug_print_range_checkers(vtss_state_t *vtss_state,
                                          const vtss_debug_printf_t pr,
                                          const vtss_debug_info_t   *const info)
{
    u32                   i;
    vtss_vcap_range_chk_t *entry;

    vtss_debug_print_header(pr, "Range Checkers");
    pr("Index  Type  Count  Range\n");
    for (i = 0; i < VTSS_VCAP_RANGE_CHK_CNT; i++) {
        entry = &vtss_state->vcap.range.entry[i];
        pr("%-5u  %-4s  %-5u  %u-%u\n",
           i, 
           entry->type == VTSS_VCAP_RANGE_TYPE_SPORT ? "S" :
           entry->type == VTSS_VCAP_RANGE_TYPE_DPORT ? "D" :
           entry->type == VTSS_VCAP_RANGE_TYPE_SDPORT ? "SD" :
           entry->type == VTSS_VCAP_RANGE_TYPE_VID ? "VID" :
           entry->type == VTSS_VCAP_RANGE_TYPE_DSCP ? "DSCP" : "?",
           entry->count,
           entry->min,
           entry->max);
    }
    pr("\n");
}

static void vtss_vcap_debug_print(const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info,
                                  vtss_vcap_obj_t           *obj,
                                  u32                       data_size,
                                  u32                       obj_size)
{
    u32                  i, low, high;
    vtss_vcap_entry_t    *cur;
    BOOL                 header = 1;
    vtss_vcap_user_t     user;
    const char           *name;
    vtss_vcap_key_size_t key_size;
    
    vtss_debug_print_header(pr, obj->name);

    pr("obj_size      : %u\n", obj_size);
    pr("data_size     : %u\n", data_size);
    pr("max_count     : %u\n", obj->max_count);
    pr("count         : %u\n", obj->count);
    pr("max_rule_count: %u\n", obj->max_rule_count);
    pr("rule_count    : %u\n", obj->rule_count);
    pr("full_count    : %u\n", obj->key_count[VTSS_VCAP_KEY_SIZE_FULL]);
    pr("half_count    : %u\n", obj->key_count[VTSS_VCAP_KEY_SIZE_HALF]);
    pr("quarter_count : %u\n", obj->key_count[VTSS_VCAP_KEY_SIZE_QUARTER]);
    for (cur = obj->used, i = 0; cur != NULL; cur = cur->next, i++) {
        if (header)
            pr("\nIndex  Key Size  User  Name      ID\n");
        header = 0;
        low = (cur->id & 0xffffffff);
        high = ((cur->id >> 32) & 0xffffffff);
        name = "?";
        user = cur->user;
        name = (user == VTSS_IS0_USER_EVC ? "EVC " :
                user == VTSS_IS1_USER_VCL ? "VCL" :
                user == VTSS_IS1_USER_VLAN ? "VLAN" :
                user == VTSS_IS1_USER_MEP ? "MEP " :
                user == VTSS_IS1_USER_EVC ? "EVC " :
                user == VTSS_IS1_USER_EFE ? "EFE " :
                user == VTSS_IS1_USER_QOS ? "QoS" :
                user == VTSS_IS1_USER_SSM ? "SSM" :
                user == VTSS_IS1_USER_ACL ? "ACL" :
                user == VTSS_IS2_USER_IGMP ? "IGMP" :
                user == VTSS_IS2_USER_SSM ? "SSM" :
                user == VTSS_IS2_USER_IGMP_ANY ? "IGMP_ANY" :
                user == VTSS_IS2_USER_EEE ? "EEE" :
                user == VTSS_IS2_USER_ACL_PTP ? "ACL_PTP" :
                user == VTSS_IS2_USER_ACL ? "ACL" :
                user == VTSS_IS2_USER_ACL_SIP ? "ACL_SIP " :
                user == VTSS_ES0_USER_VLAN ? "VLAN" :
                user == VTSS_ES0_USER_MEP ? "MEP " :
                user == VTSS_ES0_USER_EVC ? "EVC " :
                user == VTSS_ES0_USER_EFE ? "EFE " :
                user == VTSS_ES0_USER_TX_TAG? "TX_TAG " : "?");
        key_size = cur->data.key_size;
        pr("%-7u%-10s%-6d%-10s0x%08x:0x%08x (%u:%u)\n", 
           i, vtss_vcap_key_size_txt(key_size), user, name, high, low, high, low);
    }
    pr("\n");
}

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_EVC)
void vtss_vcap_debug_print_is0(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    vtss_vcap_debug_print(pr, info, &vtss_state->vcap.is0.obj, 
                          sizeof(vtss_is0_data_t), sizeof(vtss_is0_info_t));
}
#endif /* VTSS_ARCH_JAGUAR_1 && VTSS_FEATURE_EVC */

#if defined(VTSS_FEATURE_IS1)
void vtss_vcap_debug_print_is1(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    vtss_vcap_debug_print(pr, info, &vtss_state->vcap.is1.obj,
                          sizeof(vtss_is1_data_t), sizeof(vtss_is1_info_t));
}
#endif /* VTSS_FEATURE_IS1 */    

#if defined(VTSS_FEATURE_IS2)
void vtss_vcap_debug_print_is2(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    vtss_vcap_debug_print(pr, info, &vtss_state->vcap.is2.obj,
                          sizeof(vtss_is2_data_t), sizeof(vtss_is2_info_t));
}
#endif /* VTSS_FEATURE_IS2 */    

#if defined(VTSS_FEATURE_ES0)
void vtss_vcap_debug_print_es0(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    vtss_vcap_debug_print(pr, info, &vtss_state->vcap.es0.obj,
                          sizeof(vtss_es0_data_t), sizeof(vtss_es0_info_t));
}
#endif /* VTSS_FEATURE_ES0 */    

void vtss_vcap_debug_print_acl(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    vtss_port_no_t        port_no;
    BOOL                  header = 1;
    vtss_acl_port_conf_t  *conf;
    vtss_acl_action_t     *act;
    vtss_acl_policer_no_t policer_no;
    char                  buf[64];
    
    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_ACL))
        return;

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (!info->port_list[port_no])
            continue;
        conf = &vtss_state->vcap.acl_port_conf[port_no];
        act = &conf->action;
        if (header) {
            header = 0;
            pr("Port  Policy  CPU  Once  Queue  Policer  Learn  ");
#if defined(VTSS_FEATURE_ACL_V1)
            pr("Forward  Port Copy");
#endif /* VTSS_FEATURE_ACL_V1 */
#if defined(VTSS_FEATURE_ACL_V2)
            vtss_debug_print_port_header(vtss_state, pr, "Mirror  PTP  Port  ", 0, 0);
#endif /* VTSS_FEATURE_ACL_V2 */
            pr("\n");
        }
        pr("%-6u%-8s%-5u%-6u%-7u",
           port_no, vtss_acl_policy_no_txt(conf->policy_no, buf),
           act->cpu, act->cpu_once, act->cpu_queue);
        if (act->police)
            sprintf(buf, "%u (ACL)", act->policer_no);
#if defined(VTSS_ARCH_LUTON26) && defined(VTSS_FEATURE_QOS_POLICER_DLB)
        else if (act->evc_police)
            sprintf(buf, "%u (EVC)", act->evc_policer_id);
#endif /* VTSS_ARCH_LUTON26 && VTSS_FEATURE_QOS_POLICER_DLB */
        else
            strcpy(buf, "Disabled");
        pr("%-9s%-7u", buf, act->learn);
#if defined(VTSS_FEATURE_ACL_V1)
        pr("%-9u", act->forward);
        if (act->port_forward)
            pr("%-9u", act->port_no);
        else
            pr("%-9s", "Disabled");
#endif /* VTSS_FEATURE_ACL_V1 */
#if defined(VTSS_FEATURE_ACL_V2)
        pr("%-8u%-5s%-6s", 
           act->mirror, 
           act->ptp_action == VTSS_ACL_PTP_ACTION_NONE ? "None" :
           act->ptp_action == VTSS_ACL_PTP_ACTION_ONE_STEP ? "One" :
           act->ptp_action == VTSS_ACL_PTP_ACTION_TWO_STEP ? "Two" : "?",
           act->port_action == VTSS_ACL_PORT_ACTION_NONE ? "None" :
           act->port_action == VTSS_ACL_PORT_ACTION_FILTER ? "Filt" :
           act->port_action == VTSS_ACL_PORT_ACTION_REDIR ? "Redir" : "?");
        vtss_debug_print_port_members(vtss_state, pr, act->port_list, 0);
#endif /* VTSS_FEATURE_ACL_V2 */
        pr("\n");
    }
    if (!header)
        pr("\n");

    pr("Policer  Rate        ");
#if defined(VTSS_ARCH_LUTON26)
    pr("Count  L26 Policer");
#endif /* VTSS_ARCH_LUTON26 */    
    pr("\n");
    for (policer_no = VTSS_ACL_POLICER_NO_START; policer_no < VTSS_ACL_POLICER_NO_END; policer_no++) {
        vtss_acl_policer_conf_t *pol_conf = &vtss_state->vcap.acl_policer_conf[policer_no];
        vtss_packet_rate_t      rate;
        
#if defined(VTSS_FEATURE_ACL_V2)
        if (pol_conf->bit_rate_enable) {
            rate = pol_conf->bit_rate;
            if (rate == VTSS_BITRATE_DISABLED) {
                strcpy(buf, "Disabled");
            } else if (rate < 100000) {
                sprintf(buf, "%u kbps", rate);
            } else if (rate < 100000000) {
                sprintf(buf, "%u Mbps", rate/1000);
            } else {
                sprintf(buf, "%u Gbps", rate/1000000);
            }
        } else 
#endif /* VTSS_FEATURE_ACL_V2 */
        {
            rate = pol_conf->rate;
            if (rate == VTSS_PACKET_RATE_DISABLED) {
                strcpy(buf, "Disabled");
            } else if (rate < 100000) {
                sprintf(buf, "%u pps", rate);
            } else if (rate < 100000000) {
                sprintf(buf, "%u kpps", rate/1000);
            } else {
                sprintf(buf, "%u Mpps", rate/1000000);
            }
        }
        pr("%-9u%-12s", policer_no, buf);
#if defined(VTSS_ARCH_LUTON26)
        {
            vtss_policer_alloc_t *pol_alloc = &vtss_state->vcap.acl_policer_alloc[policer_no];
            pr("%-7u%u", pol_alloc->count, pol_alloc->policer);
        }
#endif /* VTSS_ARCH_LUTON26 */    
        pr("\n");
    }
    pr("\n");

#if defined(VTSS_FEATURE_VCAP)
    vtss_vcap_debug_print_range_checkers(vtss_state, pr, info);
#endif /* VTSS_FEATURE_VCAP */

#if defined(VTSS_ARCH_LUTON26)
    vtss_vcap_debug_print_is1(vtss_state, pr, info);
#endif /* VTSS_ARCH_LUTON26 */    

#if defined(VTSS_FEATURE_IS2)
    vtss_vcap_debug_print_is2(vtss_state, pr, info);
#endif /* VTSS_FEATURE_IS2 */    
}

#endif /* VTSS_FEATURE_VCAP */
