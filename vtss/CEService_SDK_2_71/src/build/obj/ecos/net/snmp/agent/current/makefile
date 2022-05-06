# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := net/snmp/agent/current
OBJECT_PREFIX := net_snmp_agent
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0 -D_KERNEL -D__ECOS -DIN_UCD_SNMP_SOURCE=1 -I$(PREFIX)/include/ucd-snmp
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/snmpping

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/agent_read_config.c src/agent_registry.c src/agent_trap.c src/kernel.c src/mib_modules.c src/snmp_agent.c src/snmp_vars.c src/snmpd.c src/snmptask.c src/mibgroup/util_funcs.c src/mibgroup/mibII/vacm_vars.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/ucd-snmp/usmStats.h $(PREFIX)/include/ucd-snmp/version.h $(PREFIX)/include/ucd-snmp/usmUser.h $(PREFIX)/include/ucd-snmp/ds_agent.h $(PREFIX)/include/ucd-snmp/agent_callbacks.h $(PREFIX)/include/ucd-snmp/agent_trap.h $(PREFIX)/include/ucd-snmp/mib_module_includes.h $(PREFIX)/include/ucd-snmp/snmp_vars.h $(PREFIX)/include/ucd-snmp/mib_module_config.h $(PREFIX)/include/ucd-snmp/snmpEngine.h $(PREFIX)/include/ucd-snmp/m2m.h $(PREFIX)/include/ucd-snmp/vacm_vars.h $(PREFIX)/include/ucd-snmp/snmp_agent.h $(PREFIX)/include/ucd-snmp/var_struct.h $(PREFIX)/include/ucd-snmp/mib_module_dot_conf.h $(PREFIX)/include/ucd-snmp/mib_module_shutdown.h $(PREFIX)/include/ucd-snmp/snmpd.h $(PREFIX)/include/ucd-snmp/mibincl.h $(PREFIX)/include/ucd-snmp/agent_read_config.h $(PREFIX)/include/ucd-snmp/mibdefs.h $(PREFIX)/include/ucd-snmp/mib_modules.h $(PREFIX)/include/ucd-snmp/agent_registry.h $(PREFIX)/include/ucd-snmp/mib_module_inits.h $(PREFIX)/include/ucd-snmp/kernel.h $(PREFIX)/include/ucd-snmp/mibgroup/struct.h $(PREFIX)/include/ucd-snmp/mibgroup/util_funcs.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/interfaces.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/udp.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/tcp.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/ip.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/ifr_helper.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/snmp_mib.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/sysORTable.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/icmp.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/system_mib.h $(PREFIX)/include/ucd-snmp/mibgroup/mibII/dot3.h

$(PREFIX)/include/ucd-snmp/usmStats.h: $(REPOSITORY)/$(PACKAGE)/include/usmStats.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/version.h: $(REPOSITORY)/$(PACKAGE)/include/version.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/usmUser.h: $(REPOSITORY)/$(PACKAGE)/include/usmUser.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/ds_agent.h: $(REPOSITORY)/$(PACKAGE)/include/ds_agent.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/agent_callbacks.h: $(REPOSITORY)/$(PACKAGE)/include/agent_callbacks.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/agent_trap.h: $(REPOSITORY)/$(PACKAGE)/include/agent_trap.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mib_module_includes.h: $(REPOSITORY)/$(PACKAGE)/include/mib_module_includes.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp_vars.h: $(REPOSITORY)/$(PACKAGE)/include/snmp_vars.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mib_module_config.h: $(REPOSITORY)/$(PACKAGE)/include/mib_module_config.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmpEngine.h: $(REPOSITORY)/$(PACKAGE)/include/snmpEngine.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/m2m.h: $(REPOSITORY)/$(PACKAGE)/include/m2m.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/vacm_vars.h: $(REPOSITORY)/$(PACKAGE)/include/vacm_vars.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp_agent.h: $(REPOSITORY)/$(PACKAGE)/include/snmp_agent.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/var_struct.h: $(REPOSITORY)/$(PACKAGE)/include/var_struct.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mib_module_dot_conf.h: $(REPOSITORY)/$(PACKAGE)/include/mib_module_dot_conf.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mib_module_shutdown.h: $(REPOSITORY)/$(PACKAGE)/include/mib_module_shutdown.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmpd.h: $(REPOSITORY)/$(PACKAGE)/include/snmpd.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibincl.h: $(REPOSITORY)/$(PACKAGE)/include/mibincl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/agent_read_config.h: $(REPOSITORY)/$(PACKAGE)/include/agent_read_config.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibdefs.h: $(REPOSITORY)/$(PACKAGE)/include/mibdefs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mib_modules.h: $(REPOSITORY)/$(PACKAGE)/include/mib_modules.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/agent_registry.h: $(REPOSITORY)/$(PACKAGE)/include/agent_registry.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mib_module_inits.h: $(REPOSITORY)/$(PACKAGE)/include/mib_module_inits.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/kernel.h: $(REPOSITORY)/$(PACKAGE)/include/kernel.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/struct.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/struct.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/util_funcs.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/util_funcs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/interfaces.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/interfaces.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/udp.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/udp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/tcp.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/tcp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/ip.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/ip.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/ifr_helper.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/ifr_helper.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/snmp_mib.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/snmp_mib.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/sysORTable.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/sysORTable.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/icmp.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/icmp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/system_mib.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/system_mib.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mibgroup/mibII/dot3.h: $(REPOSITORY)/$(PACKAGE)/include/mibgroup/mibII/dot3.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

