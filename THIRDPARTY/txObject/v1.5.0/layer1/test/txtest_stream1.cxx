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
#include "txlist.h"
#include "txint32.h"
#include "txhashmap.h"

#include "txclassc.h"

void showListofStrings (txList* list)
{
	for (int i = 0; i < list->entries(); i++)
	{
		txString* t = (txString*) list->at(i);
		std::cout << "   type : " << t->data() << std::endl;
	}
}

int main (void)
{
	txOut out;
	txList list;
	txInt32 integer(123);
	txString string("A to Z");

	/******************************************************************
	* Test RTTI.
	******************************************************************/

	std::cout << std::endl << "SUB AND SUPER TYPE CHECKS" << std::endl;
	std::cout << "-------------------------\n" << std::endl;

	if (integer.isClass(txInt32::Type))
	{
		std::cout << "RTTI IS WORKING 1" << std::endl;
	}
	else
	{
		std::cout << "RTTI FAILED 1" << std::endl;
	}

	if (integer.isSubClass(txObject::Type))
	{
		std::cout << "RTTI IS WORKING 2" << std::endl;
	}
	else
	{
		std::cout << "RTTI FAILED 2" << std::endl;
	}

	if (!integer.isSubClass(txString::Type))
	{
		std::cout << "RTTI IS WORKING 3" << std::endl;
	}
	else
	{
		std::cout << "RTTI FAILED 3" << std::endl;
	}

	if (integer.isSubClass(txObject::Type))
	{
		std::cout << "RTTI IS WORKING 4" << std::endl;
	}
	else
	{
		std::cout << "RTTI FAILED 4" << std::endl;
	}

	if (txTypeCheckSS::isSuperClass(txObject::Type, txString::Type))
	{
		std::cout << "RTTI IS WORKING 5" << std::endl;
	}
	else
	{
		std::cout << "RTTI FAILED 5" << std::endl;
	}

	if (!txTypeCheckSS::isSuperClass(txString::Type, txObject::Type))
	{
		std::cout << "RTTI IS WORKING 6" << std::endl;
	}
	else
	{
		std::cout << "RTTI FAILED 6" << std::endl;
	}

	if (txTypeCheckSS::isSuperClass(txObject::Type, txClassC::Type))
	{
		std::cout << "RTTI IS WORKING 7" << std::endl;
	}
	else
	{
		std::cout << "RTTI FAILED 7" << std::endl;
	}

	if (!txTypeCheckSS::isSuperClass(txClassC::Type, txObject::Type))
	{
		std::cout << "RTTI IS WORKING 8" << std::endl;
	}
	else
	{
		std::cout << "RTTI FAILED 8" << std::endl;
	}

	std::cout << std::endl << "SUB AND SUPER TYPE REQUEST" << std::endl;
	std::cout << "--------------------------\n" << std::endl;

	txList CLASSES;

	txTypeCheckSS::superClasses(txString::Type, CLASSES);
	std::cout << "SuperClasses of txString::Type : " << std::endl;

	showListofStrings(&CLASSES);
	CLASSES.clear();

	txTypeCheckSS::subClasses(txObject::Type, CLASSES);
	std::cout << "SubClasses of txObject::Type : " << std::endl;

	showListofStrings(&CLASSES);
	CLASSES.clear();

	txTypeCheckSS::superClasses(txObject::Type, CLASSES);
	std::cout << "SuperClasses of txObject::Type : " << std::endl;

	showListofStrings(&CLASSES);
	CLASSES.clear();

	/******************************************************************
	* Test Lists.
	******************************************************************/

	std::cout << std::endl << "STREAM AND DE-STREAM CHECKS" << std::endl;
	std::cout << "----------------------------\n" << std::endl;

	list.append(&string);
	list.append(&integer);

	txList streamList;
	streamList.append(&list);
	out << streamList;

	list.clear();
	streamList.clear();

	txIn in1(out);

	txObject* obj1;
	txObject* obj2;
	txObject* obj3;

	in1 >> obj3;

	if (obj3->isClass(txList::Type))
	{
		std::cout << "We Have a List Object within a List Object" << std::endl;

		obj1 = ((txList*) obj3)->get();
		((txList*) obj3)->clear();
		delete obj3; obj3 = 0;

		obj2 = ((txList*) obj1)->get();

		if (obj2->isClass(txString::Type))
		{
			std::cout << "Item One Is a String" << std::endl;
			std::cout << "  DATA : " << ((txString*) obj2)->data();
			std::cout << std::endl;
		}

		delete obj2; obj2 = 0;

		obj2 = ((txList*) obj1)->get();

		if (obj2->isClass(txInt32::Type))
		{
			std::cout << "Item Two Is an Integer" << std::endl;
			std::cout << "VALUE : " << ((txInt32*) obj2)->value();
			std::cout << std::endl;
		}

		delete obj2; obj2 = 0;
		((txList*) obj1)->clear();
		delete obj1; obj1 = 0;
	}

	/******************************************************************
	* Test Dictionary.
	******************************************************************/

	out.flush();

	txHashMap map;
	txObject* obj4;
	txObject* obj5;

	map.insertKeyAndValue(&integer, &string);

	out << map;

	map.clear();

	txIn in2(out);

	in2 >> obj4;

	txHashMapIterator iter((txHashMap&) *obj4);

	while (obj5 = (txObject*) iter.next())
	{
		std::cout << "KEY : " << ((txInt32*) obj5)->value() << std::endl;

		delete obj5; obj5 = 0;

		obj5 = (txObject*) iter.value();

		std::cout << "VALUE : " << ((txString*) obj5)->data() << std::endl;

		delete obj5; obj5 = 0;
	}

	delete obj4; obj4 = 0;

	std::cout << std::endl;

	return 1;
}

