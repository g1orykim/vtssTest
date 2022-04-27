# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := net/athttpd/current
OBJECT_PREFIX := net_athttpd
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0 -D__ECOS
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := 

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/forms.c src/handler.c src/http.c src/socket.c src/cgi.c src/md5c.c src/auth.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/cyg/athttpd/auth.h $(PREFIX)/include/cyg/athttpd/digcalc.h $(PREFIX)/include/cyg/athttpd/forms.h $(PREFIX)/include/cyg/athttpd/global.h $(PREFIX)/include/cyg/athttpd/handler.h $(PREFIX)/include/cyg/athttpd/http.h $(PREFIX)/include/cyg/athttpd/md5.h $(PREFIX)/include/cyg/athttpd/socket.h $(PREFIX)/include/cyg/athttpd/cgi.h $(PREFIX)/include/cyg/athttpd/jim.h

$(PREFIX)/include/cyg/athttpd/auth.h: $(REPOSITORY)/$(PACKAGE)/include/auth.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/digcalc.h: $(REPOSITORY)/$(PACKAGE)/include/digcalc.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/forms.h: $(REPOSITORY)/$(PACKAGE)/include/forms.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/global.h: $(REPOSITORY)/$(PACKAGE)/include/global.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/handler.h: $(REPOSITORY)/$(PACKAGE)/include/handler.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/http.h: $(REPOSITORY)/$(PACKAGE)/include/http.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/md5.h: $(REPOSITORY)/$(PACKAGE)/include/md5.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/socket.h: $(REPOSITORY)/$(PACKAGE)/include/socket.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/cgi.h: $(REPOSITORY)/$(PACKAGE)/include/cgi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/athttpd/jim.h: $(REPOSITORY)/$(PACKAGE)/include/jim.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

