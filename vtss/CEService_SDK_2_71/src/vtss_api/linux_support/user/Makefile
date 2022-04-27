.PHONY: all clean install

INSTALL ?= install

LIBS := libvtss_switch.so
LIBO := vtss_switch.o vtss_switch_ext.o
PROG := switch debug_switch portmon portcfg vlancfg inject extract ccm netlink_filter

all: $(LIBS) $(PROG)

$(LIBO): %.o: %.c
	$(CC) -c -Wall $(CFLAGS) -fPIC $< -o $@

libvtss_switch.so: $(LIBO)
	$(CC) -shared -o $@ $^

$(PROG): %: %.c $(LIBS)
	$(CC) -Wall $(CFLAGS) $@.c -o $@ $(LIBS)

install:
	$(INSTALL) -d $(DESTDIR)/usr/lib
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) -d $(DESTDIR)/usr/include/vtss_api
	$(INSTALL) -m 644 ../../include/*.h $(DESTDIR)/usr/include/vtss_api
	$(INSTALL) -m 644 vtss_switch_usermode.h $(DESTDIR)/usr/include/vtss_api/vtss_switch.h
	$(INSTALL) -m 775 $(LIBS) $(DESTDIR)/usr/lib
	$(INSTALL) -m 775 $(PROG) $(DESTDIR)/usr/bin

clean:
	rm -f *.o $(PROG) $(LIBS)
