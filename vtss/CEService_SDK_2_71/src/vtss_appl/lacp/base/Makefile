# Makefile for LACP protocol system

all :
	$(MAKE) -C example all

clean :
	$(RM) *.[od]
	$(MAKE) -C example clean

config :
	$(CFG_PROPAGATE) $(TOPDIR)/config.mk config.mk
	$(MAKE) -C example config

.PHONY : all clean config
