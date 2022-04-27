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

*/
#include "vtss_macsec_emu_base.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <map>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "reg_sim_base.hxx"
#include "venice.hxx"
#include "vtss_phy_10g_api.h"

vtss_trace_conf_t vtss_appl_trace_conf;

void vtss_callout_trace_printf(const vtss_trace_layer_t layer,
                               const vtss_trace_group_t group,
                               const vtss_trace_level_t level,
                               const char *file,
                               const int line,
                               const char *function,
                               const char *format,
                               ...)
{
    time_t  t;
    va_list va;
    int     h, m, s;

    const char * f = strrchr(file, '/');
    f = f ? f + 1 : file;

    t = ::time(NULL);
    h = (t / 3600 % 24);
    m = (t / 60 % 60);
    s = (t % 60);
    printf("%u:%02u:%02u %s:%d %s/%s %s: ",
           h, m, s, f, line,
           layer == VTSS_APPL_TRACE_LAYER ? "APPL":
           layer == VTSS_TRACE_LAYER_AIL ? "AIL" : "CIL",
           level == VTSS_TRACE_LEVEL_ERROR ? "Error" :
           level == VTSS_TRACE_LEVEL_INFO ? "Info " :
           level == VTSS_TRACE_LEVEL_DEBUG ? "Debug" :
           level == VTSS_TRACE_LEVEL_NOISE ? "Noise" : "?????",
           function);
    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    printf("\n");
}

static BOOL api_locked = FALSE;

void vtss_callout_lock(const vtss_api_lock_t *const lock)
{
  T_N("Locking function %s", lock->function);
  if (api_locked) {
    T_E("API lock problem");
  }
  api_locked = TRUE;
}

void vtss_callout_unlock(const vtss_api_lock_t *const lock)
{
  T_N("Unlocking function %s", lock->function);
  if (!api_locked) {
    T_E("API unlock problem");
  }
  api_locked = FALSE;
}

struct MmdAddress
{
    MmdAddress(uint32_t p, uint32_t d, uint32_t a) :
        port(p), device(d), address(a) { }
    uint32_t port, device, address;
};

std::ostream& operator<<(std::ostream &o, const MmdAddress & m)
{
    o << std::hex;
    o << "(Port:" << std::setfill('0') << std::setw(2) << m.port <<
         " Dev:"  << std::setfill('0') << std::setw(2) << m.device <<
         " Addr:" << std::setfill('0') << std::setw(4) << m.address << ")" <<
         std::setfill(' ') << std::setw(0) << std::dec;
    return o;
}

bool device_id_is_secondary(int i)
{
    switch (i) {
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        case 0x40:
        case 0x41:
            return true;
        default:
            return false;
    }
}

bool target_id_is_32bit(int i)
{
    if (i == 0x40 || i == 0x10 ||
        (i >= 0x3A && i <= 0x3e) ||
        (i >= 0x0A && i <= 0x0e) ||
        (i >= 0x62 && i <= 0x6a)) {
        return true;
    }

    return false;
}

struct RegisterDescriptor
{
    RegisterDescriptor() : device_id_(0), address_(0) { }
    RegisterDescriptor(uint32_t d, uint32_t a) {
        device_id_ = d;
        address_ = a;
        //uint32_t mask = device_id_to_mask(d);
        //mask <<= 2; mask |= 3;
        //address_ = mask & a;
    }

    RegisterDescriptor(const RegisterDescriptor& rhs) :
        device_id_(rhs.device_id_), address_(rhs.address_), max_width_(rhs.max_width_) { }

    RegisterDescriptor& operator=(const RegisterDescriptor& rhs) {
        device_id_ = rhs.device_id_;
        address_ = rhs.address_;
        max_width_ = rhs.max_width_;
        return *this;
    }

    bool is_32bit() const {
        return target_id_is_32bit(device_id_);
    }

    bool operator==(const RegisterDescriptor& rhs) const {
        return device_id_ == rhs.device_id_ && address_ == rhs.device_id_;
    }

    bool operator!=(const RegisterDescriptor& rhs) const {
        return device_id_ != rhs.device_id_ || address_ != rhs.device_id_;
    }

    bool operator<(const RegisterDescriptor& rhs) const {
        if (device_id_ != rhs.device_id_)
            return device_id_ < rhs.device_id_;
        return address_ < rhs.address_;
    }

    uint32_t device_id() const { return device_id_; }
    uint32_t address() const { return address_; }

    static uint32_t device_id_to_mask(int i)
    {
        switch (i) {
            case 0x31:
            case 0x1:  return 0xffff;
            case 0x37:
            case 0x7:  return 0xffff;
            case 0x3F:
            case 0xF:  return 0x000f;
            case 0x40:
            case 0x10: return 0x01ff;
            case 0x32:
            case 0x2:  return 0xffff;
            case 0x33:
            case 0x3:  return 0x8fff;
            case 0x35:
            case 0x5:  return 0x00ff;
            case 0x36:
            case 0x6:  return 0x00ff;
            case 0x3C:
            case 0xC:  return 0x007f;
            case 0x3D:
            case 0xD:  return 0x007f;
            case 0x3E:
            case 0xE:  return 0x007f;
            case 0x39:
            case 0x9:  return 0xffff;
            case 0x41:
            case 0x11: return 0x03ff;
            case 0x34:
            case 0x4:  return 0x01ff;
            case 0x38:
            case 0x8:  return 0xffff;
            case 0x61: return 0x7fff;
            case 0x6B: return 0x00ff;
            case 0x6C: return 0x00ff;
            case 0x62: return 0x01ff;
            case 0x63: return 0x01ff;
            case 0x64: return 0x07ff;
            case 0x65: return 0x07ff;
            case 0x66: return 0x07ff;
            case 0x67: return 0x07ff;
            case 0x68: return 0x07ff;
            case 0x69: return 0x07ff;
            case 0x6A: return 0x007f;
            case 0x3A:
            case 0xA:  return 0x3fff;
            case 0x3B:
            case 0xB:  return 0x3fff;
        }
        return 0xffffffff;
    }

private:
    uint32_t device_id_;
    uint32_t address_;
    uint32_t max_width_;

};



RegisterDescriptor mmd_to_RegisterDescriptor(int mmd_device, int mmd_addr, bool secondary)
{
    int i = mmd_addr;
    int device_id = 0;

    switch(mmd_device) {
        case 0x01:
            if (i >= 0x0000 && i <= 0x000F)      device_id = (secondary?0x31:0x1);
            else if (i >= 0x0096 && i <= 0x009B) device_id = (secondary?0x37:0x7);
            else if (i >= 0x00AA && i <= 0x00AF) device_id = (secondary?0x31:0x1);
            else if (i >= 0x8201 && i <= 0x82FF) device_id = (secondary?0x37:0x7);
            else if (i >= 0x8300 && i <= 0x83FF) device_id = (secondary?0x31:0x1);
            else if (i >= 0x9000 && i <= 0x900F) device_id = (secondary?0x31:0x1);
            else if (i >= 0xA000 && i <= 0xA00F) device_id = (secondary?0x31:0x1);
            else if (i >= 0xA100 && i <= 0xA101) device_id = (secondary?0x31:0x1);
            else if (i >= 0xA200 && i <= 0xA201) device_id = (secondary?0x31:0x1);
            else if (i >= 0xAE00 && i <= 0xAE00) device_id = (secondary?0x31:0x1);
            else if (i >= 0xB000 && i <= 0xB00F) device_id = (secondary?0x31:0x1);
            else if (i >= 0xB100 && i <= 0xB1FF) device_id = (secondary?0x31:0x1);
            else if (i >= 0xB200 && i <= 0xB6FF) device_id = (secondary?0x31:0x1);
            else if (i >= 0xB700 && i <= 0xB7FF) device_id = (secondary?0x31:0x1);
            else if (i >= 0xB800 && i <= 0xBFFF) device_id = (secondary?0x31:0x1);
            else if (i >= 0xC000 && i <= 0xC00F) device_id = (secondary?0x3F:0xF);
            else if (i >= 0xF000 && i <= 0xF0FF) device_id = (secondary?0x40:0x10);
            else if (i >= 0xF100 && i <= 0xF1FF) device_id = (secondary?0x40:0x10);
            break;

        case 0x02:
            if (i >= 0x0000 && i <= 0x7FFF)      device_id = (secondary?0x32:0x2);
            else if (i >= 0x8000 && i <= 0xFFFF) device_id = (secondary?0x32:0x2);
            break;

        case 0x03:
            if (i >= 0x0000 && i <= 0x7FFF)      device_id = (secondary?0x33:0x3);
            else if (i >= 0x8000 && i <= 0x8FFF) device_id = (secondary?0x33:0x3);
            else if (i >= 0xE000 && i <= 0xE0FF) device_id = (secondary?0x35:0x5);
            else if (i >= 0xE100 && i <= 0xE1FF) device_id = (secondary?0x36:0x6);
            else if (i >= 0xF000 && i <= 0xF0FF) device_id = (secondary?0x3C:0xC);
            else if (i >= 0xF100 && i <= 0xF1FF) device_id = (secondary?0x3D:0xD);
            else if (i >= 0xF200 && i <= 0xF2FF) device_id = (secondary?0x3E:0xE);
           break;

        case 0x04:
            if (i >= 0x0000 && i <= 0x000F)      device_id = (secondary?0x39:0x9);
            else if (i >= 0x0018 && i <= 0x0019) device_id = (secondary?0x39:0x9);
            else if (i >= 0xE600 && i <= 0xE6FF) device_id = (secondary?0x39:0x9);
            else if (i >= 0xE800 && i <= 0xE8FF) device_id = (secondary?0x39:0x9);

            else if (i >= 0xE900 && i <= 0xE9FF) device_id = (secondary?0x41:0x11);
            else if (i >= 0xEA00 && i <= 0xEAFF) device_id = (secondary?0x41:0x11);
            else if (i >= 0xF000 && i <= 0xF1FF) device_id = (secondary?0x34:0x4);
            break;

        case 0x07:
            if (i >= 0x0000 && i <= 0x00FF)      device_id = (secondary?0x38:0x8);
            else if (i >= 0x8000 && i <= 0x80FF) device_id = (secondary?0x38:0x8);
            break;

        case 0x1e:
            if (i >= 0x0000 && i <= 0x6FFF)      device_id = 0x61;
            else if (i >= 0x7000 && i <= 0x70FF) device_id = 0x61;
            else if (i >= 0x7100 && i <= 0x71FF) device_id = 0x61;
            else if (i >= 0x8100 && i <= 0x81FF) device_id = 0x6B;
            else if (i >= 0x8100 && i <= 0x81FF) device_id = 0x6B;
            else if (i >= 0x8200 && i <= 0x82FF) device_id = 0x6C;
            else if (i >= 0x8200 && i <= 0x82FF) device_id = 0x6C;
            else if (i >= 0x9000 && i <= 0x91FF) device_id = 0x62;
            else if (i >= 0x9200 && i <= 0x93FF) device_id = 0x63;
            else if (i >= 0xA000 && i <= 0xA7FF) device_id = 0x64;
            else if (i >= 0xA800 && i <= 0xAFFF) device_id = 0x65;
            else if (i >= 0xB000 && i <= 0xB7FF) device_id = 0x66;
            else if (i >= 0xB800 && i <= 0xBFFF) device_id = 0x67;
            else if (i >= 0xC000 && i <= 0xC7FF) device_id = 0x68;
            else if (i >= 0xC800 && i <= 0xCFFF) device_id = 0x69;
            else if (i >= 0xF000 && i <= 0xF0FF) device_id = 0x6A;
            break;

        case 0x1f:
            if (i >= 0x0000 && i <= 0x7FFF)      device_id = (secondary?0x3A:0xA);
            else if (i >= 0x8000 && i <= 0xFFFF) device_id = (secondary?0x3B:0xB);
            break;

        default:
            break;
    }

    // on 32bit targets two mmd addresses are reserved to point to one
    // register
    if (target_id_is_32bit(device_id)) {
        i >>= 1;
    }

    // Depending on the device ID, part of the bits needs to be masked
    // out
    i &= RegisterDescriptor::device_id_to_mask(device_id);

    return RegisterDescriptor(device_id, i);
}


std::ostream& operator<<(std::ostream &o, const RegisterDescriptor& r)
{
    o << std::hex;
    o << std::setfill('0') << std::setw(4) << r.address() << "@" <<
         std::setfill('0') << std::setw(2) << r.device_id() <<
         std::setfill(' ') << std::setw(0) << std::dec;
    return o;
}

std::ostream& operator<<(std::ostream &o, const Register& r)
{
    o << "0x" << std::hex << std::setfill('0') << std::setw(8) << r.data <<
        std::setfill(' ') << std::setw(0) << std::dec;
    return o;
}

struct RegisterMetaData
{
    RegisterMetaData() { }
    RegisterMetaData(std::string n, uint32_t m, uint32_t r, uint32_t w) :
        name(n), max_width(m), sw_read_mask(r), sw_write_mask(w) { }

    std::string name;
    uint32_t max_width;
    uint32_t sw_read_mask;
    uint32_t sw_write_mask;
};

typedef std::map<RegisterDescriptor, Register*> reg_map_t;
typedef std::map<RegisterDescriptor, RegisterMetaData> reg_meta_map_t;
typedef std::map<std::string, RegisterDescriptor> reg_reverse_map_t;


struct VenicePhy
{
    struct Bootstrap : public Walker
    {
        Bootstrap(reg_map_t&m, reg_meta_map_t &t, reg_reverse_map_t &r) :
            reg_map(m), reg_meta_map(t), reg_reverse_map(r)
        {
            if (reg_meta_map.size()) {
                build_static_maps = false;
            } else {
                build_static_maps = true;
            }
        }

        void add(const RegisterDescriptor& d, Register *reg) {
            if (reg_map.find(d) != reg_map.end()) {
                std::cout << "Duplicats" << std::endl;
            } else {
                reg_map[d] = reg;
            }
        }

        void add(const RegisterDescriptor& d, Register *reg, const RegisterMetaData& meta) {
            if (reg_map.find(d) != reg_map.end()) {
                std::cout << "Duplicats: " << d << " " << reg_meta_map[d].name << " " <<
                    meta.name << std::endl;
            } else {
                //std::cout << d << " "  << meta.name << " " << meta.max_width << " " << reg << std::endl;
                reg_reverse_map[meta.name] = d;
                reg_meta_map[d] = meta;
                reg_map[d] = reg;
            }
        }

        void exec(Chip *c, Target *t, RegisterGroup *g, RegisterDesc *r, Register *reg) {
            RegisterDescriptor d(t->device_id(), g->address() + r->address());
            reg->data = r->initial();
            if (build_static_maps) {
                std::stringstream ss;

                ss << c->name();
                if (device_id_is_secondary(t->device_id())) ss << "[1]";
                ss << "::" << t->name() << "::" << g->name() << "::" << r->name();

                add(d, reg, RegisterMetaData(ss.str(), r->max_width(), r->read_mask(), r->write_mask()));
            } else {
                add(d, reg);
            }
        }

        void exec(Chip *c, Target *t, RegisterGroupSet *g, uint32_t gidx,
                  RegisterDesc *r, Register *reg) {
            RegisterDescriptor d(t->device_id(), g->address(gidx) + r->address());
            reg->data = r->initial();

            if (build_static_maps) {
                std::stringstream ss;
                ss << c->name();
                if (device_id_is_secondary(t->device_id())) ss << "[1]";
                ss << "::" << t->name() << "::" << g->name() <<
                    "[" << gidx << "]::" << r->name();
                add(d, reg, RegisterMetaData(ss.str(), r->max_width(), r->read_mask(), r->write_mask()));
            } else {
                add(d, reg);
            }

        }

        void exec(Chip *c, Target *t, RegisterGroup *g,
                  RegisterSetDesc *r, uint32_t ridx, Register *reg) {
            RegisterDescriptor d(t->device_id(), g->address() + r->address(ridx));
            reg->data = r->initial();
            if (build_static_maps) {
                std::stringstream ss;
                ss << c->name();
                if (device_id_is_secondary(t->device_id())) ss << "[1]";
                ss << "::" << t->name() << "::" << g->name() << "::" <<
                    r->name() << "[" << ridx << "]";
                add(d, reg, RegisterMetaData(ss.str(), r->max_width(), r->read_mask(), r->write_mask()));
            } else {
                add(d, reg);
            }
        };

        void exec(Chip *c, Target *t, RegisterGroupSet *g, uint32_t gidx,
                  RegisterSetDesc *r, uint32_t ridx, Register *reg) {
            RegisterDescriptor d(t->device_id(), g->address(gidx) + r->address(ridx));
            reg->data = r->initial();
            if (build_static_maps) {
                std::stringstream ss;
                ss << c->name();
                if (device_id_is_secondary(t->device_id())) ss << "[1]";
                ss << "::" << t->name() << "::" << g->name() <<
                    "[" << gidx << "]::" << r->name() << "[" << ridx << "]";
                add(d, reg, RegisterMetaData(ss.str(), r->max_width(), r->read_mask(), r->write_mask()));
            } else {
                add(d, reg);
            }
        }

        reg_map_t& reg_map;
        reg_meta_map_t& reg_meta_map;
        reg_reverse_map_t& reg_reverse_map;
        bool build_static_maps;
    };

    VenicePhy() {
        Bootstrap boot(mmd_map, reg_meta_map, txt_rmap);
        chip.walk(&boot);
    }

    // idx
    reg_map_t mmd_map;
    static reg_meta_map_t reg_meta_map;
    static reg_reverse_map_t txt_rmap;

    // chip registers
    Venice chip;
};

reg_meta_map_t VenicePhy::reg_meta_map;
reg_reverse_map_t VenicePhy::txt_rmap;

// TODO, create a port map
static VenicePhy * phy0 = 0;

void print_operation(const char * str, const MmdAddress& mmd_addr, uint16_t val)
{
    RegisterDescriptor descriptor =
        mmd_to_RegisterDescriptor(mmd_addr.device, mmd_addr.address, false);

    reg_meta_map_t::const_iterator meta_iter = VenicePhy::reg_meta_map.find(descriptor);

    std::cout << str << mmd_addr << " 0x" <<
         std::setfill('0') << std::setw(4) << std::hex << val <<
         std::setfill(' ') << std::setw(0) << std::dec;

    if (meta_iter == VenicePhy::reg_meta_map.end()) {
        std::cout << " [no-symbolic-name]";
    } else {
        std::cout << " " << meta_iter->second.name;
    }

    // Print register descriptor
    std::cout << " [" << descriptor << "] ";

    if (meta_iter != phy0->reg_meta_map.end()) {
        std::cout << meta_iter->second.max_width;
    }

    std::cout << std::endl;
}


vtss_rc _mmd_read(const vtss_inst_t    inst,
		  const vtss_port_no_t port_no,
                  const u8             mmd,
                  const u16            addr,
                  u16                  *const value)
{
    MmdAddress mmd_address(port_no, mmd, addr);
    RegisterDescriptor descriptor = mmd_to_RegisterDescriptor(mmd, addr, false);

    if (!phy0) {
        std::cout << "READ   " << mmd_address << "        ERROR: No phy instantiated" << std::endl;
        return VTSS_RC_ERROR;
    }

    reg_map_t::iterator iter = phy0->mmd_map.find(descriptor);

    if (iter == phy0->mmd_map.end()) {
        std::cout << "READ   " << mmd_address <<
            "        ERROR: No such register. Descriptor: " << descriptor <<
            std::endl;
        return VTSS_RC_ERROR;
    }

    if (descriptor.is_32bit() && addr & 1) {
        *value = ((iter->second->data) >> 16) & 0xffff;
    } else {
        *value = iter->second->data & 0xffff;
    }

    print_operation("READ   ", mmd_address, *value);
    return VTSS_RC_OK;
}

vtss_rc _mmd_read_inc(const vtss_inst_t    inst,
		      const vtss_port_no_t port_no,
                      const u8             mmd,
                      const u16            addr,
                      u16                  *const buf,
                      u8                   count)
{
    u16 *b = const_cast<u16 *>(buf);
    for (int i = 0; i < count; ++i) {
        _mmd_read(inst, port_no, mmd, addr, b++);
    }
    return VTSS_RC_OK;
}

vtss_rc _mmd_write(const vtss_inst_t    inst,
		   const vtss_port_no_t port_no,
                   const u8             mmd,
                   const u16            addr,
                   const u16            value)
{
    MmdAddress mmd_address(port_no, mmd, addr);
    RegisterDescriptor descriptor = mmd_to_RegisterDescriptor(mmd, addr, false);

    if (!phy0) {
        std::cout << "WRITE  " << mmd_address << "        ERROR: No phy instantiated" << std::endl;
        return VTSS_RC_ERROR;
    }

    reg_map_t::iterator iter = phy0->mmd_map.find(descriptor);

    if (iter == phy0->mmd_map.end()) {
        std::cout << "WRITE  " << mmd_address <<
            "        ERROR: No such register. Descriptor: " << descriptor <<
            std::endl;
        return VTSS_RC_ERROR;
    }

    if (descriptor.is_32bit() && addr & 1) {
        iter->second->data &= 0x0000ffff;
        iter->second->data |= (((uint32_t)value) << 16) & 0xffff0000;
    } else {
        iter->second->data &= 0xffff0000;
        iter->second->data |= ((uint32_t)value) & 0xffff;
    }

    print_operation("WRITE  ", mmd_address, value);
    return VTSS_RC_OK;
}

vtss_inst_t instance_phy_new()
{
    vtss_inst_t        inst;
    vtss_init_conf_t   init_conf;
    vtss_inst_create_t create;

    if (phy0)
        delete phy0;
    phy0 =  new VenicePhy();

    // Setup trace level for PHY group
    vtss_trace_conf_t vtss_appl_trace_conf;
    memset(&vtss_appl_trace_conf, 0, sizeof(vtss_appl_trace_conf));
    vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_AIL] = VTSS_TRACE_LEVEL_DEBUG;
    vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_CIL] = VTSS_TRACE_LEVEL_DEBUG;
    vtss_trace_conf_set(VTSS_TRACE_GROUP_MACSEC, &vtss_appl_trace_conf);

    // create instance
    vtss_inst_get(VTSS_TARGET_10G_PHY, &create);
    vtss_inst_create(&create, &inst);
    vtss_init_conf_get(inst, &init_conf);

    // Setup register read/write hooks
    init_conf.mmd_read = _mmd_read;
    init_conf.mmd_read_inc = _mmd_read_inc;
    init_conf.mmd_write = _mmd_write;
    init_conf.warm_start_enable = 0;
    init_conf.restart_info_port = 0;
    init_conf.restart_info_src = VTSS_RESTART_INFO_SRC_10G_PHY;

    if (vtss_init_conf_set(inst, &init_conf) == VTSS_RC_ERROR) {
        T_E("Could not initialize");
        return 0;
    };

    {
        vtss_phy_10g_mode_t mode;
        memset(&mode, 0, sizeof(mode));
        (void) vtss_phy_10g_mode_set (NULL, 0, &mode);
        (void) vtss_phy_10g_mode_set (NULL, 1, &mode);
    }

    return inst;
}

void instance_phy_delete(vtss_inst_t inst)
{
    printf("Destroying PHY\n");
    vtss_inst_destroy(inst);

    if (phy0)
        delete phy0;
}

static Register* register_find(VenicePhy *phy, const std::string& name)
{
    if (!phy0) {
        return 0;
    }

    reg_reverse_map_t::const_iterator s_iter = VenicePhy::txt_rmap.find(name);
    if (s_iter == VenicePhy::txt_rmap.end()) {
        return 0;
    }

    reg_map_t::const_iterator iter = phy0->mmd_map.find(s_iter->second);

    if (iter == phy0->mmd_map.end()) {
        return 0;
    }

    return iter->second;
}

vtss_rc raw_hw_read(const char *reg_name, u32 *val)
{
    Register* reg = register_find(phy0, reg_name);
    if (!reg) {
        return VTSS_RC_ERROR;
    }

    *val = reg->data;
    return VTSS_RC_OK;
}

vtss_rc raw_hw_write(const char *reg_name, u32 val)
{
    Register* reg = register_find(phy0, reg_name);
    if (!reg) {
        return VTSS_RC_ERROR;
    }

    reg->data = val;
    return VTSS_RC_OK;
}


