EXTRA_CFLAGS = -DVTSS_OPSYS_LINUX=1 -I$(obj)/../../include -I$(obj)/../../boards $(VTSS_EXTRA_CFLAGS)

APIOBJ := vtss_api.o vtss_common.o vtss_util.o vtss_l3.o vtss_mpls_common.o vtss_wis_api.o \
	vtss_b2.o vtss_luton26.o vtss_jaguar1.o vtss_luton28.o \
	vtss_serval.o vtss_serval_fdma.o \
        vtss_phy_10g.o vtss_phy.o vtss_phy_init_scripts.o vtss_phy_veriphy.o vtss_phy_ewis.o vtss_phy_ts_api.o \
	vtss_fdma.o vtss_fdma_common.o vtss_luton28_fdma.o vtss_vcoreiii_fdma.o

BRDOBJ := board_probe.o \
	port_custom_lu26.o \
	port_custom_jr1.o port_custom_jr1_cu48.o \
	port_custom_serval.o

vtss_core-objs   := vtss_switch-main.o vtss_switch-api.o \
	$(addprefix ../../base/,$(APIOBJ))  $(addprefix ../../boards/,$(BRDOBJ))
vtss_ioctl-objs  := vtss_switch-ioctl.o vtss_switch-ioctl_ext.o
vtss_port-objs   := vtss_switch-port.o
vtss_ethdrv-objs := vtss_switch-eth.o

obj-m   += vtss_core.o
obj-m   += vtss_ioctl.o
obj-m   += vtss_port.o
ifeq ($(CONFIG_VTSS_VCOREIII),y)
ifeq ($(VERSION),2)
obj-m   += vtss_ethdrv.o
endif
endif

all:
	$(MAKE) -C $(KSRC) $(KOPT) modules
install:
	$(MAKE) -C $(KSRC) $(KOPT) modules_install
clean:
	$(MAKE) -C $(KSRC) $(KOPT) clean
