# Makefile for LACP protocol system
# Only used when building the standalone simulator

OBJECTS = vtss_lacp.o vtss_os_linux.o
PROGRAMS = lacp
RM = rm -f
CC = gcc
ifdef NODEBUG
CFLAGS = -O2 -Wall -DNDEBUG=1
else
CFLAGS = -ggdb -Wall
endif
CFLAGS += -I.. -I.
LIBS = -lreadline -ltermcap

all : $(PROGRAMS)

lacp : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)

vtss_lacp.o vtss_lacp.d : ../vtss_lacp.c
	$(CC) -MMD $(CFLAGS) -c -o vtss_lacp.o $<

vtss_os_linux.o vtss_os_linux.d : vtss_os_linux.c
	$(CC) -MMD $(CFLAGS) -c -o vtss_os_linux.o $<

clean :
	$(RM) $(OBJECTS) $(PROGRAMS) $(OBJECTS:.o=.d)

config :
	$(CFG_PROPAGATE) $(TOPDIR)/config.mk config.mk

.PHONY : all clean config

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),config)
-include $(OBJECTS:.o=.d)
endif
endif
