TOP := ../../..

INTERFACES := PORT
CHIP       := -DVTSS_CHIP_SERVAL
DEFINES    := -DVTSS_OPSYS_LINUX -DRPC_NETWORK_CODING=1 $(foreach mod,$(INTERFACES),-DVTSS_SW_OPTION_$(mod)=1 )
INCLUDES   := $(addprefix -I$(TOP)/,mgmt/include vtss_api/include vtss_appl/rpc vtss_appl/rpc_server)
CFLAGS     := $(DEFINES) $(CHIP) $(INCLUDES) -g -Wall

DIR_rpc     := $(TOP)/vtss_appl/rpc
OBJECTS_rpc := rpc_coding.o rpc_out.o rpc_in_evt.o
SOURCES_rpc := $(patsubst %.o,%.c,$(OBJECTS_rpc)) rpc_api.h

all: client event

$(OBJECTS_rpc): rpc_api.h

client: client.c rpc_coding.o rpc_out.o
	$(CC) $(CFLAGS) $^ -o $@

event:  event.c rpc_coding.o rpc_in_evt.o
	$(CC) $(CFLAGS) $^ -o $@

$(SOURCES_rpc): %: $(DIR_rpc)/%.t $(addprefix $(DIR_rpc)/,rpc_spec.xml rpcgen.pl)
	$(call what,Generating $* RPC component)
	$(Q)perl -w $(DIR_rpc)/rpcgen.pl -I $(DIR_rpc) -s $(DIR_rpc)/rpc_spec.xml -o $@ $<

clean:
	-rm -f client event $(OBJECTS_rpc) $(SOURCES_rpc)
