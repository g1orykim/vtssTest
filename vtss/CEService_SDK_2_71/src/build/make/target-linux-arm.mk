# 
# Vitesse Switch Software.
# 
# Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
# Rights Reserved.
# 
# Unpublished rights reserved under the copyright laws of the United States of
# America, other countries and international treaties. Permission to use, copy,
# store and modify, the software and its source code is granted. Permission to
# integrate into other products, disclose, transmit and distribute the software
# in an absolute machine readable format (e.g. HEX file) is also granted.  The
# source code of the software may not be disclosed, transmitted or distributed
# without the written permission of Vitesse. The software and its source code
# may only be used in products utilizing the Vitesse switch products.
# 
# This copyright notice must appear in any copy, modification, disclosure,
# transmission or distribution of the software. Vitesse retains all ownership,
# copyright, trade secret and proprietary rights in the software.
# 
# THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
# INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR USE AND NON-INFRINGEMENT.
# 
LINUX_ROOT	= $(TOP)/linux-2.6
LINUX_INCLUDE	= $(LINUX_ROOT)/include

MFLAGS		= -fno-strict-aliasing -fverbose-asm -Wall	\
			-Wstrict-prototypes -Wpointer-arith -Wcast-qual -Wcast-align	\
			-Wwrite-strings -Waggregate-return -Wmissing-prototypes \
			-Wmissing-declarations -Wno-long-long -ggdb -march=armv5te -O2
CFLAGS		= $(MFLAGS) $(INCLUDES) -I$(LINUX_INCLUDE) $(DEFINES)
LDFLAGS		= 

COMPILER_PREFIX = arm-linux-
XCC		= $(COMPILER_PREFIX)gcc
XLD		= $(XCC)

# compile_c <o-file> <c-file> <x-flags>
compile_c = $(XCC) -c -o $*.o -MD $(CFLAGS) $<

%.o: %.c
	$(call compile_c, $@, $<)

%.bin: %.elf
	-@echo I will not build $*.bin for this platform
