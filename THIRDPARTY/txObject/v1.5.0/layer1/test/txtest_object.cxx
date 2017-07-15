///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "txin.h"
#include "txout.h"
#include "txclassc.h"

void checkUser (txClassC& obj)
{
	std::cout << "NAME : " << obj.className() << std::endl;
	std::cout << " DATA : " << obj.data() << std::endl;
	std::cout << " VALUE : " << obj.value() << std::endl;

	const txList* list = obj.list();

	if (list)
	{
		std::cout << " LIST ENTRIES : " << list->entries() << std::endl;
	}
	else
	{
		std::cout << " LIST IS NULL" << std::endl;
	}
}

int main (void)
{
	txOut out;
	txClassC testObj("ABC", 123);

	out << testObj;

	txIn in(out);

	txClassC* testNew; in >> testNew;

	if (testNew)
	{
		checkUser(*testNew);
		delete testNew; testNew = 0;
	}

	in.cursor(0); // move index to the beginning of stream

	txClassC testOverlay("DUMMY", 0);

	in >> testOverlay;

	checkUser(testOverlay);

	return 1;
}

