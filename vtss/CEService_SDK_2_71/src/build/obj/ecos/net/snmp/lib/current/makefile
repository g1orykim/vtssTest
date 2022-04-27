# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := net/snmp/lib/current
OBJECT_PREFIX := net_snmp_lib
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0 -D_KERNEL -D__ECOS -I$(PREFIX)/include/ucd-snmp
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := 

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/asn1.c src/callback.c src/default_store.c src/int64.c src/keytools.c src/lcd_time.c src/md5.c src/mib.c src/mt_support.c src/parse.c src/read_config.c src/scapi.c src/snmp.c src/snmp_alarm.c src/snmp_api.c src/snmp_auth.c src/snmp_client.c src/snmp_debug.c src/snmp_logging.c src/snmpusm.c src/snmpv3.c src/system.c src/tools.c src/vacm.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/ucd-snmp/snmpusm.h $(PREFIX)/include/ucd-snmp/config.h $(PREFIX)/include/ucd-snmp/snmp_api.h $(PREFIX)/include/ucd-snmp/snmpv3.h $(PREFIX)/include/ucd-snmp/callback.h $(PREFIX)/include/ucd-snmp/md5.h $(PREFIX)/include/ucd-snmp/default_store.h $(PREFIX)/include/ucd-snmp/snmp_debug.h $(PREFIX)/include/ucd-snmp/int64.h $(PREFIX)/include/ucd-snmp/snmp_impl.h $(PREFIX)/include/ucd-snmp/tools.h $(PREFIX)/include/ucd-snmp/parse.h $(PREFIX)/include/ucd-snmp/scapi.h $(PREFIX)/include/ucd-snmp/snmp.h $(PREFIX)/include/ucd-snmp/snmp-tc.h $(PREFIX)/include/ucd-snmp/mib.h $(PREFIX)/include/ucd-snmp/read_config.h $(PREFIX)/include/ucd-snmp/system.h $(PREFIX)/include/ucd-snmp/snmp_client.h $(PREFIX)/include/ucd-snmp/asn1.h $(PREFIX)/include/ucd-snmp/snmp_alarm.h $(PREFIX)/include/ucd-snmp/mt_support.h $(PREFIX)/include/ucd-snmp/lcd_time.h $(PREFIX)/include/ucd-snmp/vacm.h $(PREFIX)/include/ucd-snmp/snmp_logging.h $(PREFIX)/include/ucd-snmp/keytools.h $(PREFIX)/include/ucd-snmp/transform_oids.h

$(PREFIX)/include/ucd-snmp/snmpusm.h: $(REPOSITORY)/$(PACKAGE)/include/snmpusm.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/config.h: $(REPOSITORY)/$(PACKAGE)/include/config.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp_api.h: $(REPOSITORY)/$(PACKAGE)/include/snmp_api.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmpv3.h: $(REPOSITORY)/$(PACKAGE)/include/snmpv3.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/callback.h: $(REPOSITORY)/$(PACKAGE)/include/callback.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/md5.h: $(REPOSITORY)/$(PACKAGE)/include/md5.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/default_store.h: $(REPOSITORY)/$(PACKAGE)/include/default_store.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp_debug.h: $(REPOSITORY)/$(PACKAGE)/include/snmp_debug.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/int64.h: $(REPOSITORY)/$(PACKAGE)/include/int64.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp_impl.h: $(REPOSITORY)/$(PACKAGE)/include/snmp_impl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/tools.h: $(REPOSITORY)/$(PACKAGE)/include/tools.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/parse.h: $(REPOSITORY)/$(PACKAGE)/include/parse.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/scapi.h: $(REPOSITORY)/$(PACKAGE)/include/scapi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp.h: $(REPOSITORY)/$(PACKAGE)/include/snmp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp-tc.h: $(REPOSITORY)/$(PACKAGE)/include/snmp-tc.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mib.h: $(REPOSITORY)/$(PACKAGE)/include/mib.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/read_config.h: $(REPOSITORY)/$(PACKAGE)/include/read_config.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/system.h: $(REPOSITORY)/$(PACKAGE)/include/system.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp_client.h: $(REPOSITORY)/$(PACKAGE)/include/snmp_client.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/asn1.h: $(REPOSITORY)/$(PACKAGE)/include/asn1.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp_alarm.h: $(REPOSITORY)/$(PACKAGE)/include/snmp_alarm.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/mt_support.h: $(REPOSITORY)/$(PACKAGE)/include/mt_support.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/lcd_time.h: $(REPOSITORY)/$(PACKAGE)/include/lcd_time.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/vacm.h: $(REPOSITORY)/$(PACKAGE)/include/vacm.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/snmp_logging.h: $(REPOSITORY)/$(PACKAGE)/include/snmp_logging.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/keytools.h: $(REPOSITORY)/$(PACKAGE)/include/keytools.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/ucd-snmp/transform_oids.h: $(REPOSITORY)/$(PACKAGE)/include/transform_oids.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

