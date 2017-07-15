///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <iostream>

#include "txin.h"
#include "txout.h"

#include "txstring.h"

#include "txlist.h"
#include "txtree.h"
#include "txhashmap.h"
#include "txhashset.h"

int main (void)
{
	txList list;
	txTree tree;
	txHashMap dict;
	txHashSet table;
	txString* str = 0;

	for (int i = 0; i < 10; i++)
	{
		char buf[100];
		sprintf(buf, "string info : %d", i);
		str = new txString(buf);

		// INSERT

		list.insert(str);
		tree.insert(str);
		dict.insert(str);
		table.insert(str); // this container will delete the string

		txString lookup(buf);

		// FIND

		if (!list.find(&lookup))
		{
			std::cout << "failed to find : " << str->data() << std::endl;
		}

		if (!tree.find(&lookup))
		{
			std::cout << "failed to find : " << str->data() << std::endl;
		}

		if (!dict.find(&lookup))
		{
			std::cout << "failed to find : " << str->data() << std::endl;
		}

		if (!table.find(&lookup))
		{
			std::cout << "failed to find : " << str->data() << std::endl;
		}

#if 0
		// REMOVE

		if (!list.remove(&lookup))
		{
			std::cout << "failed to remove : " << str->data() << std::endl;
		}

		if (!tree.remove(&lookup))
		{
			std::cout << "failed to remove : " << str->data() << std::endl;
		}

		if (!dict.remove(&lookup))
		{
			std::cout << "failed to remove : " << str->data() << std::endl;
		}

		// REMOVE AND DESTROY

		table.removeAndDestroy(&lookup);
#endif
	}

	// TRY OUT ITERATORS

	txListIterator listIter(list);
	txTreeIterator treeIter(tree);
	txHashMapIterator dictIter(dict);
	txHashSetIterator tableIter(table);

	while (str = (txString*) listIter.next())
	{
		std::cout << "LIST : " << str->data() << std::endl;
	}

	while (str = (txString*) treeIter.next())
	{
		std::cout << "TREE : " << str->data() << std::endl;
	}

	while (str = (txString*) dictIter.next())
	{
		std::cout << "DICT : " << str->data() << std::endl;
	}

	while (str = (txString*) tableIter.next())
	{
		std::cout << "TABLE : " << str->data() << std::endl;
	}

	// free up the strings

	table.clearAndDestroy();

	return 1;
}

