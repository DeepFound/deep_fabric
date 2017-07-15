###############################################################################
##
##   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
##
##   All Rights Reserved. See LICENSE.txt for license definition
##
###############################################################################

include ../../build.defs

OBJS = \
	event.cxx \
	thrdmgr.cxx \
	thread.cxx \
	thrdstats.cxx \
	txevent.cxx \
	txeventq.cxx \
	txlock.cxx \
	txthrdbase.cxx \
	txthrdmgr.cxx \
	txthrdss.cxx \
	txthread.cxx \
	linux_setjmp.S \
	$(ENDOFLIST)

all:
	$(CCC) $(CFLAGS) -I../inc -I../src \
	-I../../layer1/inc -I../../layer2/inc $(OBJS) -c

	mkdir -p ../lib

	$(ARCHIVE) ../lib/liblayer3.a *.o

	$(RANLIB) ../lib/liblayer3.a

clean:
	rm -f *.o
	rm -rf ../lib
	rm -rf $(OS_TEMPORARY)
