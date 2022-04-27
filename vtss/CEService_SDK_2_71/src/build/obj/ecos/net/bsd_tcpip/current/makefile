# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := net/bsd_tcpip/current
OBJECT_PREFIX := net_bsd_tcpip
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0 -D_KERNEL
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/sysctl1

build: headers libtarget.a.stamp libextras.a.stamp

LIBRARY := libtarget.a
COMPILE := src/ecos/support.c src/ecos/synch.c src/ecos/timeout.c src/ecos/init.cxx src/sys/net/if.c src/sys/net/rtsock.c src/sys/net/raw_cb.c src/sys/net/raw_usrreq.c src/sys/net/route.c src/sys/net/radix.c src/sys/net/if_ethersubr.c src/sys/net/if_loop.c src/sys/netinet/igmp.c src/sys/netinet/raw_ip.c src/sys/netinet/in.c src/sys/netinet/in_cksum.c src/sys/netinet/in_pcb.c src/sys/netinet/in_proto.c src/sys/netinet/in_rmx.c src/sys/netinet/ip_encap.c src/sys/netinet/ip_id.c src/sys/netinet/ip_icmp.c src/sys/netinet/ip_flow.c src/sys/netinet/ip_input.c src/sys/netinet/ip_output.c src/sys/netinet/ip_mroute.c src/sys/netinet/if_ether.c src/sys/netinet/udp_usrreq.c src/sys/netinet/tcp_input.c src/sys/netinet/tcp_output.c src/sys/netinet/tcp_debug.c src/sys/netinet/tcp_usrreq.c src/sys/netinet/tcp_timer.c src/sys/netinet/tcp_subr.c src/sys/netinet/tcp_syncache.c src/sys/kern/md5c.c src/sys/kern/uipc_domain.c src/sys/kern/uipc_socket.c src/sys/kern/uipc_socket2.c src/sys/kern/uipc_mbuf.c src/sys/kern/uipc_mbuf2.c src/sys/kern/uipc_accf.c src/sys/kern/kern_subr.c src/sys/netinet6/dest6.c src/sys/netinet6/frag6.c src/sys/netinet6/icmp6.c src/sys/netinet6/in6.c src/sys/netinet6/in6_cksum.c src/sys/netinet6/in6_ifattach.c src/sys/netinet6/in6_pcb.c src/sys/netinet6/in6_proto.c src/sys/netinet6/in6_rmx.c src/sys/netinet6/in6_src.c src/sys/netinet6/ip6_forward.c src/sys/netinet6/ip6_input.c src/sys/netinet6/ip6_mroute.c src/sys/netinet6/ip6_output.c src/sys/netinet6/mld6.c src/sys/netinet6/nd6.c src/sys/netinet6/nd6_nbr.c src/sys/netinet6/nd6_rtr.c src/sys/netinet6/raw_ip6.c src/sys/netinet6/route6.c src/sys/netinet6/scope6.c src/sys/netinet6/udp6_output.c src/sys/netinet6/udp6_usrreq.c src/sys/kern/kern_sysctl.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

LIBRARY := libextras.a
COMPILE := src/sys/kern/sockio.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/./net/netisr.h $(PREFIX)/include/./net/radix.h $(PREFIX)/include/./net/if_dl.h $(PREFIX)/include/./net/zlib.h $(PREFIX)/include/./net/if_llc.h $(PREFIX)/include/./net/if_types.h $(PREFIX)/include/./net/raw_cb.h $(PREFIX)/include/./net/ethernet.h $(PREFIX)/include/./net/if.h $(PREFIX)/include/./net/if_arp.h $(PREFIX)/include/./net/intrq.h $(PREFIX)/include/./net/if_gif.h $(PREFIX)/include/./net/iso88025.h $(PREFIX)/include/./net/if_sec.h $(PREFIX)/include/./net/if_var.h $(PREFIX)/include/./net/route.h $(PREFIX)/include/./net/pfkeyv2.h $(PREFIX)/include/./sys/param.h $(PREFIX)/include/./sys/endian.h $(PREFIX)/include/./sys/md5.h $(PREFIX)/include/./sys/cdefs.h $(PREFIX)/include/./sys/socketvar.h $(PREFIX)/include/./sys/ioctl.h $(PREFIX)/include/./sys/queue.h $(PREFIX)/include/./sys/mbuf.h $(PREFIX)/include/./sys/protosw.h $(PREFIX)/include/./sys/bsdtypes.h $(PREFIX)/include/./sys/sockio.h $(PREFIX)/include/./sys/ioccom.h $(PREFIX)/include/./sys/malloc.h $(PREFIX)/include/./sys/domain.h $(PREFIX)/include/./sys/socket.h $(PREFIX)/include/./sys/sysctl.h $(PREFIX)/include/./sys/errno.h $(PREFIX)/include/./sys/uio.h $(PREFIX)/include/./netinet6/mld6_var.h $(PREFIX)/include/./netinet6/in6_var.h $(PREFIX)/include/./netinet6/pim6_var.h $(PREFIX)/include/./netinet6/ip6protosw.h $(PREFIX)/include/./netinet6/ipsec.h $(PREFIX)/include/./netinet6/nd6.h $(PREFIX)/include/./netinet6/in6_ifattach.h $(PREFIX)/include/./netinet6/ip6_var.h $(PREFIX)/include/./netinet6/pim6.h $(PREFIX)/include/./netinet6/in6.h $(PREFIX)/include/./netinet6/ah.h $(PREFIX)/include/./netinet6/tcp6_var.h $(PREFIX)/include/./netinet6/ip6_fw.h $(PREFIX)/include/./netinet6/ip6_mroute.h $(PREFIX)/include/./netinet6/esp.h $(PREFIX)/include/./netinet6/esp_rijndael.h $(PREFIX)/include/./netinet6/esp_twofish.h $(PREFIX)/include/./netinet6/udp6_var.h $(PREFIX)/include/./netinet6/ipcomp.h $(PREFIX)/include/./netinet6/in6_gif.h $(PREFIX)/include/./netinet6/in6_pcb.h $(PREFIX)/include/./netinet6/scope6_var.h $(PREFIX)/include/./netinet6/raw_ip6.h $(PREFIX)/include/./netkey/key_var.h $(PREFIX)/include/./netkey/keysock.h $(PREFIX)/include/./netkey/key.h $(PREFIX)/include/./netkey/key_debug.h $(PREFIX)/include/./netkey/keydb.h $(PREFIX)/include/./machine/types.h $(PREFIX)/include/./netinet/tcp_timer.h $(PREFIX)/include/./netinet/udp_var.h $(PREFIX)/include/./netinet/in_pcb.h $(PREFIX)/include/./netinet/ip_var.h $(PREFIX)/include/./netinet/tcpip.h $(PREFIX)/include/./netinet/in_var.h $(PREFIX)/include/./netinet/udp.h $(PREFIX)/include/./netinet/ip_icmp.h $(PREFIX)/include/./netinet/ip_flow.h $(PREFIX)/include/./netinet/tcp.h $(PREFIX)/include/./netinet/tcp_debug.h $(PREFIX)/include/./netinet/ip.h $(PREFIX)/include/./netinet/ip_fw.h $(PREFIX)/include/./netinet/igmp.h $(PREFIX)/include/./netinet/ip_ecn.h $(PREFIX)/include/./netinet/tcp_seq.h $(PREFIX)/include/./netinet/ip6.h $(PREFIX)/include/./netinet/ip_mroute.h $(PREFIX)/include/./netinet/in_systm.h $(PREFIX)/include/./netinet/igmp_var.h $(PREFIX)/include/./netinet/tcp_fsm.h $(PREFIX)/include/./netinet/icmp6.h $(PREFIX)/include/./netinet/tcp_var.h $(PREFIX)/include/./netinet/icmp_var.h $(PREFIX)/include/./netinet/in.h $(PREFIX)/include/./netinet/if_ether.h $(PREFIX)/include/./netinet/ip_encap.h

$(PREFIX)/include/./net/netisr.h: $(REPOSITORY)/$(PACKAGE)/include/net/netisr.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/radix.h: $(REPOSITORY)/$(PACKAGE)/include/net/radix.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/if_dl.h: $(REPOSITORY)/$(PACKAGE)/include/net/if_dl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/zlib.h: $(REPOSITORY)/$(PACKAGE)/include/net/zlib.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/if_llc.h: $(REPOSITORY)/$(PACKAGE)/include/net/if_llc.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/if_types.h: $(REPOSITORY)/$(PACKAGE)/include/net/if_types.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/raw_cb.h: $(REPOSITORY)/$(PACKAGE)/include/net/raw_cb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/ethernet.h: $(REPOSITORY)/$(PACKAGE)/include/net/ethernet.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/if.h: $(REPOSITORY)/$(PACKAGE)/include/net/if.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/if_arp.h: $(REPOSITORY)/$(PACKAGE)/include/net/if_arp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/intrq.h: $(REPOSITORY)/$(PACKAGE)/include/net/intrq.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/if_gif.h: $(REPOSITORY)/$(PACKAGE)/include/net/if_gif.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/iso88025.h: $(REPOSITORY)/$(PACKAGE)/include/net/iso88025.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/if_sec.h: $(REPOSITORY)/$(PACKAGE)/include/net/if_sec.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/if_var.h: $(REPOSITORY)/$(PACKAGE)/include/net/if_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/route.h: $(REPOSITORY)/$(PACKAGE)/include/net/route.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./net/pfkeyv2.h: $(REPOSITORY)/$(PACKAGE)/include/net/pfkeyv2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/param.h: $(REPOSITORY)/$(PACKAGE)/include/sys/param.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/endian.h: $(REPOSITORY)/$(PACKAGE)/include/sys/endian.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/md5.h: $(REPOSITORY)/$(PACKAGE)/include/sys/md5.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/cdefs.h: $(REPOSITORY)/$(PACKAGE)/include/sys/cdefs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/socketvar.h: $(REPOSITORY)/$(PACKAGE)/include/sys/socketvar.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/ioctl.h: $(REPOSITORY)/$(PACKAGE)/include/sys/ioctl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/queue.h: $(REPOSITORY)/$(PACKAGE)/include/sys/queue.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/mbuf.h: $(REPOSITORY)/$(PACKAGE)/include/sys/mbuf.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/protosw.h: $(REPOSITORY)/$(PACKAGE)/include/sys/protosw.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/bsdtypes.h: $(REPOSITORY)/$(PACKAGE)/include/sys/bsdtypes.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/sockio.h: $(REPOSITORY)/$(PACKAGE)/include/sys/sockio.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/ioccom.h: $(REPOSITORY)/$(PACKAGE)/include/sys/ioccom.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/malloc.h: $(REPOSITORY)/$(PACKAGE)/include/sys/malloc.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/domain.h: $(REPOSITORY)/$(PACKAGE)/include/sys/domain.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/socket.h: $(REPOSITORY)/$(PACKAGE)/include/sys/socket.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/sysctl.h: $(REPOSITORY)/$(PACKAGE)/include/sys/sysctl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/errno.h: $(REPOSITORY)/$(PACKAGE)/include/sys/errno.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./sys/uio.h: $(REPOSITORY)/$(PACKAGE)/include/sys/uio.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/mld6_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/mld6_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/in6_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/in6_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/pim6_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/pim6_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/ip6protosw.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/ip6protosw.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/ipsec.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/ipsec.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/nd6.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/nd6.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/in6_ifattach.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/in6_ifattach.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/ip6_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/ip6_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/pim6.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/pim6.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/in6.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/in6.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/ah.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/ah.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/tcp6_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/tcp6_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/ip6_fw.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/ip6_fw.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/ip6_mroute.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/ip6_mroute.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/esp.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/esp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/esp_rijndael.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/esp_rijndael.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/esp_twofish.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/esp_twofish.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/udp6_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/udp6_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/ipcomp.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/ipcomp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/in6_gif.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/in6_gif.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/in6_pcb.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/in6_pcb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/scope6_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/scope6_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet6/raw_ip6.h: $(REPOSITORY)/$(PACKAGE)/include/netinet6/raw_ip6.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netkey/key_var.h: $(REPOSITORY)/$(PACKAGE)/include/netkey/key_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netkey/keysock.h: $(REPOSITORY)/$(PACKAGE)/include/netkey/keysock.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netkey/key.h: $(REPOSITORY)/$(PACKAGE)/include/netkey/key.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netkey/key_debug.h: $(REPOSITORY)/$(PACKAGE)/include/netkey/key_debug.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netkey/keydb.h: $(REPOSITORY)/$(PACKAGE)/include/netkey/keydb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./machine/types.h: $(REPOSITORY)/$(PACKAGE)/include/machine/types.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/tcp_timer.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/tcp_timer.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/udp_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/udp_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/in_pcb.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/in_pcb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/tcpip.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/tcpip.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/in_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/in_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/udp.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/udp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip_icmp.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip_icmp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip_flow.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip_flow.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/tcp.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/tcp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/tcp_debug.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/tcp_debug.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip_fw.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip_fw.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/igmp.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/igmp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip_ecn.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip_ecn.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/tcp_seq.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/tcp_seq.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip6.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip6.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip_mroute.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip_mroute.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/in_systm.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/in_systm.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/igmp_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/igmp_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/tcp_fsm.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/tcp_fsm.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/icmp6.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/icmp6.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/tcp_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/tcp_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/icmp_var.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/icmp_var.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/in.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/in.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/if_ether.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/if_ether.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/./netinet/ip_encap.h: $(REPOSITORY)/$(PACKAGE)/include/netinet/ip_encap.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

