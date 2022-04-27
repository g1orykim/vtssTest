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
#include <iostream>
#include <iomanip>
#include "venice.hxx"

struct Foo : public Walker
{
    void exec(Chip *c, Target *t, RegisterGroup *g, RegisterDesc *r)
    {
        std::cout << t->channel() << "," << t->device() << "@" <<
                     "0x" << std::hex << std::setfill ('0') << std::setw (4) <<
                     (r->address() + g->address()) << " " <<
                     c->name() << "::" <<
                     t->name() << "::" <<
                     g->name() << "::" <<
                     r->name() << std::endl;
    }

    void exec(Chip *c, Target *t, RegisterGroupSet *g, uint32_t gidx, RegisterDesc *r)
    {
        std::cout << t->channel() << "," << t->device() << "@" <<
                     "0x" << std::hex << std::setfill ('0') << std::setw (4) <<
                     (r->address() + g->address(gidx)) << " " <<
                     c->name() << "::" <<
                     t->name() << "::" <<
                     g->name() << "[" << gidx << "]" << "::" <<
                     r->name() << std::endl;
    }

    void exec(Chip *c, Target *t, RegisterGroup *g, RegisterSetDesc *r, uint32_t ridx)
    {
        std::cout << t->channel() << "," << t->device() << "@" <<
                     "0x" << std::hex << std::setfill ('0') << std::setw (4) <<
                     (r->address(ridx) + g->address()) << " " <<
                     c->name() << "::" <<
                     t->name() << "::" <<
                     g->name() << "::" <<
                     r->name() << "[" << ridx << "]" << std::endl;
    }

    void exec(Chip *c, Target *t, RegisterGroupSet *g, uint32_t gidx, RegisterSetDesc *r, uint32_t ridx)
    {
        std::cout << t->channel() << "," << t->device() << "@" <<
                     "0x" << std::hex << std::setfill ('0') << std::setw (4) <<
                     (r->address(ridx) + g->address(gidx)) << " " <<
                     c->name() << "::" <<
                     t->name() << "::" <<
                     g->name() << "[" << gidx << "]" << "::" <<
                     r->name() << "[" << ridx << "]" <<
                     std::endl;
    }
};

int main()
{
    std::cout << sizeof(Venice) << " ";
    std::cout << sizeof(Venice) / 1024.0 << "kb" << " ";
    std::cout << sizeof(Venice) / 1024.0 / 1024 << "mb" << std::endl;
    Foo foo;
    Venice venice;
    venice.walk(&foo);
    return 0;
}

