# For any of the -D's below, if you prefer you can define them at the top
# of fw/ae/ae.h instead of passing them from the command line.

#gcc 4+ gives the error message: 'gcc: cannot specify -o with -c or -S
#  with multiple files' gcc 4+ removed support for compiling this
#  way. Working on a fix, sorry for the inconvenience!
CC=gcc



AE_DEBUG=0

all:ae_11g
    

clean_ae:
	rm -fr bin/*.o bin/*.exe *~ */*~

dist:
	if [[ -e ../../hssaet ]]; then false; fi
	rm -fr bin/*.o bin/*.exe
	rm -fr *~ */*~ \#* */\#*
	yes | rm -fr .svn */.svn




# ================= 11G ========================================================
# ae/ae_main.c is an example of how the ae code can be called.
# 11g/11g.c and ts/ts.c is the implementation for the hss 11g testsite.
# These two files must be replaced with whatever code you use to implement
# the functions in hss/hss.h and ae/ae_ffe_upd.h.
# You will get undefined references to functions *_ts if you build it as is.


ae_11g: 
	${CC} -Wall -c\
				-D AE_CONFIG_H=\"../include/cfg/ae_config_11g.h\" \
				-D AE_DEBUG=${AE_DEBUG} \
				-I ../include \
                -I ../../../include\
				obj/ae.o \
				../src/ae_reg_access.c \
				../src/ae_send_upd.c ../src/ae_main.c\
                ../src/ae.c ../src/ae_run.c


