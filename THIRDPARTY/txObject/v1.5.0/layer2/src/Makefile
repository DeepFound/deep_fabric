###############################################################################
##
##   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
##
##   All Rights Reserved. See LICENSE.txt for license definition
##
###############################################################################

include ../../build.defs

OBJS = \
	txsync.cxx \
	txtimer.cxx \
	$(ENDOFLIST)

all:
	$(CCC) $(CFLAGS) -I../inc -I../../layer1/inc $(OBJS) -c

	mkdir -p ../lib 

	$(ARCHIVE) ../lib/liblayer2.a *.o

	$(RANLIB) ../lib/liblayer2.a

clean:
	rm -f *.o
	rm -rf ../lib
	rm -rf $(OS_TEMPORARY)
