# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := net/common/current
OBJECT_PREFIX := net_common
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0 -D_KERNEL -D__ECOS -D__INSIDE_NET
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/ping_lo_test  tests/tcp_lo_test  tests/udp_lo_test  tests/multi_lo_select  tests/tcp_lo_select

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/inet_addr.c src/inet_ntoa.c src/inet_ntop.c src/inet_pton.c src/bootp_support.c src/dhcp_support.c src/dhcp_prot.c src/network_support.c src/getproto.c src/getserv.c src/getaddrinfo.c src/ifaddrs.c src/tftp_client.c src/tftp_server.c src/tftp_dummy_file.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/./bootp.h $(PREFIX)/include/./ifaddrs.h $(PREFIX)/include/./dhcp.h $(PREFIX)/include/./network.h $(PREFIX)/include/./tftp_support.h $(PREFIX)/include/./net/netdb.h $(PREFIX)/include/./machine/endian.h $(PREFIX)/include/./machine/ansi.h $(PREFIX)/include/./arpa/tftp.h $(PREFIX)/include/./arpa/inet.h $(PREFIX)/include/./arpa/telnet.h $(PREFIX)/include/./arpa/ftp.h $(PREFIX)/include/./arpa/nameser.h

$(PREFIX)/include/./bootp.h: $(REPOSITORY)/$(PACKAGE)/include/bootp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./ifaddrs.h: $(REPOSITORY)/$(PACKAGE)/include/ifaddrs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./dhcp.h: $(REPOSITORY)/$(PACKAGE)/include/dhcp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./network.h: $(REPOSITORY)/$(PACKAGE)/include/network.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./tftp_support.h: $(REPOSITORY)/$(PACKAGE)/include/tftp_support.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/netdb.h: $(REPOSITORY)/$(PACKAGE)/include/net/netdb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./machine/endian.h: $(REPOSITORY)/$(PACKAGE)/include/machine/endian.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./machine/ansi.h: $(REPOSITORY)/$(PACKAGE)/include/machine/ansi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./arpa/tftp.h: $(REPOSITORY)/$(PACKAGE)/include/arpa/tftp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./arpa/inet.h: $(REPOSITORY)/$(PACKAGE)/include/arpa/inet.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./arpa/telnet.h: $(REPOSITORY)/$(PACKAGE)/include/arpa/telnet.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./arpa/ftp.h: $(REPOSITORY)/$(PACKAGE)/include/arpa/ftp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./arpa/nameser.h: $(REPOSITORY)/$(PACKAGE)/include/arpa/nameser.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

