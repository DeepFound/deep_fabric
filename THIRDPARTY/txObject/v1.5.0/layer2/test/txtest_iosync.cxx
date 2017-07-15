///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "txsync.h"
#include "txtimer.h"

unsigned long id = 0;

void fd_func (void*, int, unsigned long)
{
	std::cout << "write to screen" << std::endl;

	txSync::unregisterIO(id);

	id = 0;
}

txTimerEnum::RETURN_STATUS timer1 (void* x)
{
	std::cout << "Call Timer of 1000 (ms)" << std::endl;

	if (!id)
	{
		id = txSync::registerIO(fd_func, 0, 0, txSync::IOWrite);
	}

	return txTimerEnum::CONTINUE;
}

int main (void)
{
	int fd = 0; // file descriptor for screen output;

	txTimer timer(timer1, 0, 1000);

	id = txSync::registerIO(fd_func, 0, fd, txSync::IOWrite);

	for (int i = 0; i < 500; i++)
	{
		txSync::IoAndTime(txTimer::processAndGetMinWait());
		
		std::cout << "." << std::flush;
	}

	return 1;
}

