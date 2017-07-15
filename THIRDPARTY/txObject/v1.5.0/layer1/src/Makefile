###############################################################################
##
##   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
##
##   All Rights Reserved. See LICENSE.txt for license definition
##
###############################################################################

include ../../build.defs

OBJS = \
	txboolean.cxx \
	txbytearray.cxx \
	txchar16.cxx \
	txchar8.cxx \
	txdouble.cxx \
	txfactory.cxx \
	txfloat.cxx \
	txgobject.cxx \
	txhashmap.cxx \
	txhashset.cxx \
	txin.cxx \
	txint16.cxx \
	txint32.cxx \
	txint64.cxx \
	txint8.cxx \
	txlist.cxx \
	txobject.cxx \
	txobjref.cxx \
	txout.cxx \
	txrefobj.cxx \
	txsmrtbuf.cxx \
	txstream.cxx \
	txstring.cxx \
	txtree.cxx \
	txtypecheckss.cxx \
	txuchar16.cxx \
	txuchar8.cxx \
	txuint16.cxx \
	txuint32.cxx \
	txuint64.cxx \
	txuint8.cxx \
	$(ENDOFLIST)

all:
	$(CCC) $(CFLAGS) -I../inc $(OBJS) -c

	mkdir -p ../lib/

	$(ARCHIVE) ../lib/liblayer1.a *.o

	$(RANLIB) ../lib/liblayer1.a

clean:
	rm -f *.o
	rm -rf ../lib
	rm -rf $(OS_TEMPORARY)
