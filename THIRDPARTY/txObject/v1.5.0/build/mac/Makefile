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
	rm -rf stage
	mkdir -p stage/sys

	$(ARCHIVE) stage/txobject.lib \
	../../layer3/src/*.o \
	../../layer2/src/*.o \
	../../layer1/src/*.o

	$(RANLIB) stage/txobject.lib

	cp ../../layer*/inc/*.h stage
	cp ../../layer*/inc/sys/*.h stage/sys

clean:
	rm -rf stage

	cp -f build.defs ../..

	cd ../../layer1/src; make clean; \
	cd ../../layer2/src; make clean; \
	cd ../../layer3/src; make clean; \

	cd ../../layer1/test; make clean; \
	cd ../../layer2/test; make clean; \
	cd ../../layer3/test; make clean; \

	rm -f ../../build.defs

