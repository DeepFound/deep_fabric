///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txlist.h"
#include "txstring.h"
#include "txhashmap.h"
#include "sys/txtypecheckss.h"

txHashMap* txTypeCheckSS::_subclassing_ = 0;
txHashMap* txTypeCheckSS::_superclassing_ = 0;
txHashSet* txTypeCheckSS::_class_type_set_ = 0;

static void startupRuntimeTypeChecking (void)
{
	txTypeCheckSS::startup();
}

static void shutdownRuntimeTypeChecking (void)
{
	txTypeCheckSS::shutdown();
}

TX_STATIC_ALLOC_NOTIFY(startupRuntimeTypeChecking)
TX_STATIC_DEALLOC_NOTIFY(shutdownRuntimeTypeChecking)

void txTypeCheckSS::_flushClassDictionary_ (txHashMap* map)
{
	txObject* v;
	txHashMapIterator iter(*map);

	while (iter.next())
	{
		v = (txObject*) iter.value();
		iter.remove();
		delete v; v = 0;
	}
}

void txTypeCheckSS::_flushClassTypeSet_ (txHashSet* set)
{
	set->clearAndDestroy();
}

void txTypeCheckSS::_connect_ (txHashMap* map, const char* s, const char* d)
{
	txList* list;
	txString src(s);
	txString dest(d);
	txString* ptr_src;
	txString* ptr_dest;

	if (!(ptr_src = (txString*) _class_type_set_->find(&src)))
	{
		ptr_src = new txString(s);
		_class_type_set_->insert(ptr_src);
	}

	if (!(ptr_dest = (txString*) _class_type_set_->find(&dest)))
	{
		ptr_dest = new txString(d);
		_class_type_set_->insert(ptr_dest);
	}

	if (list = (txList*) map->findValue(&src))
	{
		if (!list->find(&dest))
		{
			list->append(ptr_dest);
		}
	}
	else if (memcmp(s, "txRoot", 6) != 0)
	{
		list = new txList();

		if (memcmp(d, "txRoot", 6) != 0)
		{
			list->append(ptr_dest);
		}

		map->insertKeyAndValue(ptr_src, list);
	}
}

int txTypeCheckSS::_isConnected_ (txHashMap* map, const txString& src,
	const txString& dest)
{
	txList* list;
	txString* branch;

	if (list = (txList*) map->findValue(&src))
	{
		if (list->find(&dest))
		{
			return 1;
		}
		else
		{
			int entries = list->entries();

			for (int i = 0; i < entries; i++)
			{
				branch = (txString*) list->at(i);

				if (_isConnected_(map, *branch, dest))
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

void txTypeCheckSS::_addTypeToList_ (txString& src, txList& user)
{
	user.append(&src);
}

int txTypeCheckSS::_findClassRelations_ (txHashMap* map, const txString& src,
	txList& user)
{
	txList* list;
	txString* branch;
	txHashMapIterator iter(*map);

	while (branch = (txString*) iter.next())
	{
		list = (txList*) iter.value();

		if (list->find(&src) && !user.find(branch))
		{
			_addTypeToList_(*branch, user);

			_findClassRelations_(map, *branch, user);
		}
	}

	return 0;
}

const txString* txTypeCheckSS::findRTTIType (const char* name)
{
	txString* retval = 0;

	if (!isShutdown())
	{
		txString type(name);

		retval = (txString*) _subclassing_->find(&type);
		if (retval == 0)
		{
			retval = (txString*) _superclassing_->find(&type);
		}
	}

	return retval;
}

const txList* txTypeCheckSS::immediateSubClasses (const txString& src)
{
	return !isShutdown() ? (txList*) _superclassing_->findValue(&src) : 0;
}

const txList* txTypeCheckSS::immediateSuperClasses (const txString& src)
{
	return !isShutdown() ? (txList*) _subclassing_->findValue(&src) : 0;
}

int txTypeCheckSS::subClassEntries (void)
{
	return !isShutdown() ? _subclassing_->entries() : 0;
}

int txTypeCheckSS::superClassEntries (void)
{
	return !isShutdown() ? _superclassing_->entries() : 0;
}

int txTypeCheckSS::isClass (const txString& left, const txString& right)
{
	return (left.id() == right.id());
}

void txTypeCheckSS::startup (void)
{
	if (!_subclassing_)
	{
		_subclassing_ = new txHashMap();
	}

	if (!_superclassing_)
	{
		_superclassing_ = new txHashMap();
	}

	if (!_class_type_set_)
	{
		_class_type_set_ = new txHashSet(64, TX_AUTODEL_ON);
	}
}

void txTypeCheckSS::shutdown (void)
{
	if (!isShutdown())
	{
		_flushClassDictionary_(_subclassing_);
		delete _subclassing_; _subclassing_ = 0;

		_flushClassDictionary_(_superclassing_);
		delete _superclassing_; _superclassing_ = 0;

		_flushClassTypeSet_(_class_type_set_);
		delete _class_type_set_; _class_type_set_ = 0;
	}
}

const unsigned long TX_HASH_NUMBER = 0x9e3775b1L;

unsigned long txHash (const char* stream, unsigned long length)
{
	const unsigned char* buffer = (unsigned char*) stream;
	register unsigned long bytes = sizeof(unsigned long);
	register unsigned long word = *buffer;
	register unsigned long len = length;
	register unsigned long h = len;

	while (--len)
	{
		if (!(--bytes))
		{
			h ^= word * TX_HASH_NUMBER;
			bytes = sizeof(unsigned long);
			word = 0;
		}
		else
		{
			word <<= 4;
		}

		word += *(++buffer);
	}

	h ^= word * TX_HASH_NUMBER;

	return h;
}

TX_AUTODEL_FLAG TX_GET_AUTODEL_FLAG (unsigned long value)
{
	return (TX_AUTODEL_FLAG) (((value) & ~0x0FFFFFFF) >> 28);
}

TX_COMPARE_FLAG TX_GET_COMPARE_FLAG (unsigned long value)
{
	return (TX_COMPARE_FLAG) (((value) & ~0xF0FFFFFF) >> 24);
}

void TX_SET_AUTODEL_FLAG (unsigned long& value, TX_AUTODEL_FLAG flag)
{
	unsigned long x = (TX_AUTODEL_FLAG) flag;

	value &= 0x0FFFFFFF; x <<= 28; value |= x;
}

void TX_SET_COMPARE_FLAG (unsigned long& value, TX_COMPARE_FLAG flag)
{
	unsigned long x = (TX_COMPARE_FLAG) flag;

	value &= 0xF0FFFFFF; x <<= 24; value |= x;
}

