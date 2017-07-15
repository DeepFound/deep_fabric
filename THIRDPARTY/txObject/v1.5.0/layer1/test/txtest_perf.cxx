///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <iostream>

#include "txin.h"
#include "txout.h"
#include "txlist.h"
#include "txuint32.h"
#include "txstring.h"

//#include "txtimer.h"

int main (int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "usgae : program number_of_object\n" << std::flush;
		exit(1);
	}

	int num_of_objs = atoi(argv[1]);

	txOut out;
	txList listobj;
	txUInt32* intobj;

	for (int j = 0; j < num_of_objs; j++)
	{
		intobj = new txUInt32(j);
		listobj.append(intobj);
	}

	double start = 0; // txTimer::currentTime();

	out << listobj;
	out.flush();
	out << listobj;
	out.flush();
	out << listobj;
	out.flush();
	out << listobj;

	double stop = 0; // txTimer::currentTime();

	std::cout << "STREAMING" << std::endl;
	std::cout << "  Length of Stream : " << out.length() << std::endl;
	std::cout << "  Time to stream : " << num_of_objs << " objects -> ";
	std::cout << stop-start;
	std::cout << std::endl;

	txIn in(out);

	start = 0; // txTimer::currentTime();

	txList tmpList; in >> tmpList;

	stop = 0; // txTimer::currentTime();

	std::cout << "STREAMING" << std::endl;
	std::cout << "  Length of Stream : " << out.length() << std::endl;
	std::cout << "  Time to destream : " << num_of_objs << " objects -> ";
	std::cout << stop-start;
	std::cout << std::endl;

	if (tmpList.entries() != num_of_objs)
	{
		std::cerr << "error : STREAM TEST FAILED" << std::endl;
	}

	tmpList.clearAndDestroy();
	listobj.clearAndDestroy();

	return 1;
}

