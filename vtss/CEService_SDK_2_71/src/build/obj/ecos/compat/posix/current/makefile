# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := compat/posix/current
OBJECT_PREFIX := compat_posix
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/pthread1 tests/pthread2 tests/pthread3 tests/pmutex3 tests/pmqueue1 tests/pmqueue2 tests/signal1 tests/signal2 tests/signal3  tests/sigsetjmp tests/timer1 tests/tm_posix 

build: headers libtarget.a.stamp libextras.a.stamp

LIBRARY := libtarget.a
COMPILE := src/misc.cxx src/mutex.cxx src/sched.cxx src/pthread.cxx src/time.cxx src/sem.cxx src/mqueue.cxx src/signal.cxx src/except.cxx
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

LIBRARY := libextras.a
COMPILE := src/startup.cxx
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/cyg/posix/utsname.h $(PREFIX)/include/cyg/posix/time.h $(PREFIX)/include/cyg/posix/mutex.h $(PREFIX)/include/cyg/posix/signal.h $(PREFIX)/include/cyg/posix/semaphore.h $(PREFIX)/include/cyg/posix/limits.h $(PREFIX)/include/cyg/posix/muttypes.h $(PREFIX)/include/cyg/posix/sigsetjmp.h $(PREFIX)/include/cyg/posix/pthread.h $(PREFIX)/include/cyg/posix/export.h $(PREFIX)/include/cyg/posix/types.h $(PREFIX)/include/cyg/posix/sys/time.h

$(PREFIX)/include/cyg/posix/utsname.h: $(REPOSITORY)/$(PACKAGE)/include/utsname.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/time.h: $(REPOSITORY)/$(PACKAGE)/include/time.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/mutex.h: $(REPOSITORY)/$(PACKAGE)/include/mutex.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/signal.h: $(REPOSITORY)/$(PACKAGE)/include/signal.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/semaphore.h: $(REPOSITORY)/$(PACKAGE)/include/semaphore.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/limits.h: $(REPOSITORY)/$(PACKAGE)/include/limits.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/muttypes.h: $(REPOSITORY)/$(PACKAGE)/include/muttypes.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/sigsetjmp.h: $(REPOSITORY)/$(PACKAGE)/include/sigsetjmp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/pthread.h: $(REPOSITORY)/$(PACKAGE)/include/pthread.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/export.h: $(REPOSITORY)/$(PACKAGE)/include/export.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/types.h: $(REPOSITORY)/$(PACKAGE)/include/types.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/posix/sys/time.h: $(REPOSITORY)/$(PACKAGE)/include/sys/time.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

