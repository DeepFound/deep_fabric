///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txstring.h"
#include "txhashmap.h"

TX_DEFINE_PARENT_TYPE(txHashMap,txHashSet)

txMapEntry::txMapEntry (txObject* key, txObject* value) :
	_key_(key),
	_value_(value)
{
}

txMapEntry::~txMapEntry (void)
{
}


const txObject* txHashMap::find (const txObject* key) const
{
	txMapEntry* x;

	x = (txMapEntry*) txHashSet::find(key);

	return (x ? x->key() : 0);
}

const txObject* txHashMap::findKeyAndValue (const txObject* key,
	txObject*& value) const
{
	txObject* k;
	txMapEntry* x;

	if (x = (txMapEntry*) txHashSet::find(key))
	{
		k = x->key();
		value = x->value();
	}
	else
	{
		k = 0;
		value = 0;
	}

	return k;
}

const txObject* txHashMap::findValue (const txObject* key) const
{
	txMapEntry* x;

	x = (txMapEntry*) txHashSet::find(key);

	return (x ? x->value() : 0);
}

txObject* txHashMap::findValue (const txObject* key, txObject* value) const
{
	txMapEntry* x;

	x = (txMapEntry*) txHashSet::find(key);

	return (x ? x->value(value) : 0);
}

const txObject* txHashMap::insertKeyAndValue (txObject* key, txObject* value)
{
	if (!_table) resize(64);

	txObject* x = 0;
	int index = _index(key);

	if (!_table[index] || (_table[index] == ((txObject*) &_fill)))
	{
		#if !defined OSTORE_SUPPORT
			x = _insert(new txMapEntry(key, value), index);
		#else
			if (TX_IS_OBJECT_PERSISTENT(this))
			{
				x = _insert(new (TX_GET_OBJECT_CLUSTER(this),
					txMapEntry::get_os_typespec())
						txMapEntry(key, value),
							index);
			}
			else
			{
				x = _insert(new txMapEntry(key, value),
					index);
			}
		#endif
	}

	return x;
}

txObject* txHashMap::remove (const txObject* key)
{
	txMapEntry* x;
	txObject* k = 0;

	if (x = (txMapEntry*) txHashSet::remove(key))
	{
		k = x->key();
		delete x; x = 0;
	}

	return k;
}

txObject* txHashMap::removeKeyAndValue (const txObject* key, txObject*& v)
{
	txMapEntry* x;
	txObject* k = 0;

	if (x = (txMapEntry*) txHashSet::remove(key))
	{
		k = x->key();
		v = x->value();
		delete x; x = 0;
	}
	else
	{
		v = 0;
	}

	return k;
}

void txHashMap::removeAndDestroy (const txObject* key)
{
	txMapEntry* x;

	if (x = (txMapEntry*) txHashSet::remove(key))
	{
		delete x->key();
		delete x->value();
		delete x; x = 0;
	}
}

void txHashMap::clear (void)
{
	txHashSet::clearAndDestroy();
}

void txHashMap::clearAndDestroy (void)
{
	if (!_table) return;

	register int i, n = _size;
	register txObject** table = _table;

	_table = 0; _size = _entries = _fill = 0;

	for (i = 0; i < n; i++)
	{
		txMapEntry* p = (txMapEntry*) table[i];

		if (p != 0 && (p != ((txObject*) &_fill)))
		{
			delete p->value();
			delete p->key();
			delete p;
		}
	}

	delete [] table; table = 0;
}

void txHashMap::readObject (txIn& in)
{
	const char* end = in.cursor() + in.objectLength();

	signed short int flag; in >> flag;

	TX_SET_AUTODEL_FLAG(_flags_, (TX_AUTODEL_FLAG) flag);

	txObject* k;
	txObject* v;

	while (in.cursor() < end)
	{
		in >> k;
		in >> v;

		insertKeyAndValue(k, v);
	}
}

void txHashMap::writeObject (txOut& out) const
{
	txObject* node;
	unsigned int hdr = out.length();

	out.putNull();

	out << (signed short int) TX_GET_AUTODEL_FLAG(_flags_);

	txHashMapIterator iter((txHashMap&) *this);

	while (node = (txObject*) iter.next())
	{
		out.put(node);
		node = (txObject*) iter.value();
		out.put(node);
	}

	out.writeHeader(hdr, out.length() - (hdr + 4), TX_MAP);
}

txObject* txHashMapIterator::remove (void)
{
	txMapEntry* x;
	txObject* k = 0;

	if (x = (txMapEntry*) txHashSetIterator::remove())
	{
		k = x->key();
		delete x; x = 0;
	}

	return k;
}

const txObject* txHashMapIterator::next (void)
{
	txMapEntry* x;

	x = (txMapEntry*) txHashSetIterator::next();

	return (x ? x->key() : 0);
}

const txObject* txHashMapIterator::key (void) const
{
	txMapEntry* x;

	x = (txMapEntry*) txHashSetIterator::key();

	return (x ? x->key() : 0);
}

const txObject* txHashMapIterator::value (void) const
{
	txMapEntry* x;

	x = (txMapEntry*) txHashSetIterator::key();

	return (x ? x->value() : 0);
}

void txHashMapIterator::reset (txHashMap& hm)
{
	txHashSetIterator::reset(hm);
}

void txHashMapIterator::reset (void)
{
	txHashSetIterator::reset();
}

