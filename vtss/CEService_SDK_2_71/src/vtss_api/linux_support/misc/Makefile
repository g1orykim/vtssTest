M    := $PWD
KSRC ?= ../linux-2.6-cloud/
KOPT ?= ARCH=powerpc

obj-m   += uio_serval1.o

default:
	$(MAKE) -C $(KSRC) $(KOPT) M=$$PWD

clean:
	-rm -f *.o *.ko
