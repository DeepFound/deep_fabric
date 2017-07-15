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

txTimerEnum::RETURN_STATUS timer1 (void* x)
{
	int* i = (int*) x;

	std::cout << "Call Timer of 1000 (ms) : " << *i << std::endl;

	if (*i != 100)
	{
		return txTimerEnum::CONTINUE;
	}
	else
	{
		return txTimerEnum::STOP;
	}
}

int main (void)
{
	int* i = new int(0);

	txTimer timer(timer1, i, 1000);

	for (; *i < 500; (*i)++)
	{
		txSync::IoAndTime(txTimer::processAndGetMinWait());
		
		std::cout << "." << std::flush;
	}

	delete i; i = 0;

	return 1;
}

