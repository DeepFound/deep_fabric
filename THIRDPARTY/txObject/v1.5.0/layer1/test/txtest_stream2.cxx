///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <iostream>

#include "txin.h"
#include "txout.h"
#include "txlist.h"

int main (void)
{
	txOut out;
	float fa = 3.45;
	double da = 3.455567;
	double db = 90848.342234;
	signed char c; char* s; float f; double d; unsigned long int i;

	std::cout << std::endl;
	std::cout << "If there was an error, a FAILED message will displayed.";
	std::cout << std::endl;

	out.putNull(); // put null
	out << (signed char) 't';
	out << (unsigned long int) 12;
	out << (unsigned long int) 1972;
	out << (signed char) 'c';
	out << (unsigned long int) 2048;
	out << (unsigned long int) 9991992;
	out << fa;
	out << da;
	out << db;
	out << "afdasfsd987";
	out.put("abcdef", 6, TX_OCTET_STRING);

	txIn in(out);

	if (in.getNull() != 0) std::cout << "    FAILED NULL TEST" << std::endl;

	in >> c;

	if ('t' != c) std::cout << "    FAILED UCHAR TEST" << std::endl;

	in >> i;

	if (12 != i) std::cout << "    FAILED UINT16 TEST" << std::endl;

	in >> i;

	if (1972 != i) std::cout << "    FAILED UINT TEST" << std::endl;

	in >> i;

	if ('c' == i) std::cout << "    FAILED CHAR8 TEST" << std::endl;

	in >> i;

	if (2048 != i) std::cout << "    FAILED INT16 TEST" << std::endl;

	in >> i;

	if (9991992 != i) std::cout << "    FAILED INT TEST" << std::endl;

	in >> f;

	if (fa != f) std::cout << "    FAILED FLOAT TEST" << std::endl;

	in >> d;

	if (da != d) std::cout << "    FAILED FLOAT64 TEST" << std::endl;

	in >> d;
	
	if (db != d) std::cout << "    FAILED FLOAT64 TEST" << std::endl;

	in >> s;

	if (s)
	{
		if (strcmp("afdasfsd987", s))
		{
			std::cout << "    FAILED ASCII STRING TEST" << std::endl;
		}

		delete s; s = 0;
	}
	else
	{
		std::cout << "    FAILED ASCII STRING TEST" << std::endl;
	}

	unsigned long length = in.get(s, TX_OCTET_STRING);

	if (s)
	{
		if (memcmp("abcdef", s, length))
		{
			std::cout << "    FAILED OCTETSTRING TEST" << std::endl;
		}

		delete s; s = 0;
	}
	else
	{
		std::cout << "    FAILED OCTETSTRING TEST" << std::endl;
	}

	std::cout << std::endl;

	return 1;
}

