###############################################################################
##
##   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
##
##   All Rights Reserved. See LICENSE.txt for license definition
##
###############################################################################

include build.defs

all:
	cp -f build.defs ../..

	cd ../../layer1/src; make; \
	cd ../../layer2/src; make; \
	cd ../../layer3/src; make; \

install:
	rm -rf ../../build/target/

	mkdir -p ../../build/target/include/sys
	mkdir -p ../../build/target/lib

	$(ARCHIVE) ../../build/target/lib/libtxobject.a \
	../../layer3/src/*.o \
	../../layer2/src/*.o \
	../../layer1/src/*.o

	$(RANLIB) ../../build/target/lib/libtxobject.a

	cp ../../layer*/inc/*.h ../../build/target/include/
	cp ../../layer*/inc/sys/*.h ../../build/target/include/sys

clean:
	rm -rf ../../build/target/

	cp -f build.defs ../..

	cd ../../layer1/src; make clean; \
	cd ../../layer2/src; make clean; \
	cd ../../layer3/src; make clean; \

	cd ../../layer1/test; make clean; \
	cd ../../layer2/test; make clean; \
	cd ../../layer3/test; make clean; \

	rm -f ../../build.defs

