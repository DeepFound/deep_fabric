###############################################################################
##
##   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
##
##   All Rights Reserved. See LICENSE.txt for license definition
##
###############################################################################

include ../../build.defs

OBJS = \
	txtest_thrds.cxx \
	$(ENDOFLIST)

all:
	$(CCC) $(CFLAGS) -I../inc -I../src -I../../layer1/inc  \
	-I../../layer2/inc $(OBJS) ../lib/liblayer3.a \
	../../layer2/lib/liblayer2.a ../../layer1/lib/liblayer1.a $(SYS_LIBS)

clean:
	rm -f *.o
	rm -f a.out
	rm -rf $(OS_TEMPORARY)
