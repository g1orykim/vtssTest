# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := language/c/libc/i18n/current
OBJECT_PREFIX := language_c_libc_i18n
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/ctype tests/setlocale tests/i18nmb

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/ctype.cxx src/locale.cxx src/mblen.cxx src/mbtowc.cxx src/mbstowcs.cxx src/wctomb.cxx src/wcstombs.cxx src/mbtowc_jp.cxx src/wctomb_jp.cxx src/mbtowc_c.cxx
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/cyg/libc/i18n/ctype.inl $(PREFIX)/include/cyg/libc/i18n/mb.h $(PREFIX)/include/cyg/libc/i18n/newlibctype.h

$(PREFIX)/include/cyg/libc/i18n/ctype.inl: $(REPOSITORY)/$(PACKAGE)/include/ctype.inl
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/i18n/mb.h: $(REPOSITORY)/$(PACKAGE)/include/mb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/libc/i18n/newlibctype.h: $(REPOSITORY)/$(PACKAGE)/include/newlibctype.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

