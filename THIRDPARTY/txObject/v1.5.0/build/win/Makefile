###############################################################################
##
##   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
##
##   All Rights Reserved. See LICENSE.txt for license definition
##
###############################################################################

all:
	echo "NO MAKE: see Visual C++ txobject.vcproj"

install:
	mkdir stage
	mkdir stage\sys

	copy ..\..\layer1\inc\*.h stage
	copy ..\..\layer2\inc\*.h stage
	copy ..\..\layer3\inc\*.h stage

	copy ..\..\layer1\inc\sys\*.h stage\sys
	copy ..\..\layer3\inc\sys\*.h stage\sys

	copy Release\txobject.lib stage

clean:
	rmdir /s /q stage

