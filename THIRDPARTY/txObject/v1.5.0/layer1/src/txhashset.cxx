///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txstring.h"
#include "txhashset.h"

TX_DEFINE_PARENT_TYPE(txHashSet,txObject)

static long polys[] = {
	4 + 3,
	8 + 3,
	16 + 3,
	32 + 5,
	64 + 3,
	128 + 3,
	256 + 29,
	512 + 17,
	1024 + 9,
	2048 + 5,
	4096 + 83,
	8192 + 27,
	16384 + 43,
	32768 + 3,
	65536 + 45,
	131072 + 9,
	262144 + 39,
	524288 + 39,
	1048576 + 9,
	2097152 + 5,
	4194304 + 3,
	8388608 + 33,
	16777216 + 27,
	33554432 + 9,
	67108864 + 71,
	134217728 + 39,
	268435456 + 9,
	536870912 + 5,
	1073741824 + 83,
	0
};

int txHashSet::_index (const txObject* x, int find) const
{
	register txObject* p;
	register unsigned incr;
	register long hash = x->hash();
	register unsigned int mask = _size-1;
	register TX_COMPARE_FLAG flag = TX_GET_COMPARE_FLAG(_flags_);

	register int i = (~hash) & mask;

	p = _table[i];

	if (p == 0)
	{
		return i;
	}
	else if ((p == ((txObject*) &_fill)) && !find)
	{
		return i;
	}
	else if (flag == TX_COMPARE_V2K)
	{
		if ((p != ((txObject*) &_fill)) && (p->equals(x) == 1))
		{
			return i;
		}
	}
	else // TX_COMPARE_K2V is true
	{
		if ((p != ((txObject*) &_fill)) && (x->equals(p) == 1))
		{
			return i;
		}
	}

	incr = (hash ^ ((unsigned long) hash >> 3)) & mask;

	if (!incr)
	{
		incr = mask;
	}
	else if (incr > mask)
	{
		incr ^= _poly;
	}

	for (;;)
	{
		p = _table[(i+incr)&mask];

		if (p == 0)
		{
			return (i+incr)&mask;
		}
		else if ((p == ((txObject*) &_fill)) && !find)
		{
			return (i+incr)&mask;
		}
		else if (flag == TX_COMPARE_V2K)
		{
			if ((p != ((txObject*) &_fill)) && (p->equals(x) == 1))
			{
				return (i+incr)&mask;
			}
		}
		else // TX_COMPARE_K2V is true
		{
			if ((p != ((txObject*) &_fill)) && (x->equals(p) == 1))
			{
				return (i+incr)&mask;
			}
		}

		incr = incr << 1;

		if (incr > mask) incr ^= _poly;
	}
}

txObject* txHashSet::_insert (txObject* x, int index)
{
	if (index >= 0)
	{
		if (_fill*3 >= _size*2)
		{
			if (resize(_entries*2) == -1)
			{
				if (_fill+1 > _size)
				{
					return 0;
				}
			}
			else
			{
				index = 0;
			}
		}
	}
	else if (index == -1)
	{
		index = 0;
	}

	register int i = index ? index : _index(x, 0);

	if (_table[i] == 0) _fill++;

	_entries++; _table[i] = x;

	return x;
}

int txHashSet::resize (int minused)
{
	register int i;
	register txObject* p;
	register txObject** table;
	register int size;
	register long poly;
	register int oldsize = _size;
	register txObject** oldtable = _table;

	for (i = 0, size = 4; ; i++, size <<= 1)
	{
		if (i > sizeof(polys)/sizeof(polys[0]))
		{
			return -1;
		}

		if (size > minused)
		{
			poly = polys[i];
			break;
		}
	}

	#if defined OSTORE_SUPPORT
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			table = new(TX_GET_OBJECT_CLUSTER(this),
				os_typespec::get_pointer(), size)
					txObject*[size];
		}
		else
		{
			table = new txObject*[size];
		}
	#else
		table = new txObject*[size];
	#endif

	memset(table, 0, sizeof(txObject)*size);

	_fill = 0;
	_entries = 0;

	_size = size;
	_poly = poly;
	_table = table;

	for (i = 0; i < oldsize; i++)
	{
		p = oldtable[i];

		if (p != 0 && (p != ((txObject*) &_fill)))
		{
			_insert(p, -1);
		}
	}

	delete [] oldtable; oldtable = 0;

	return _size;
}

const txObject* txHashSet::insert (txObject* x)
{
	if (!_table) resize(64);

	return _insert(x);
}

txObject* txHashSet::remove (const txObject* x)
{
	register txObject* p = 0;

	if (_table)
	{
		register int i = _index(x);

		p = _table[i];

		if (p != 0 && (p != ((txObject*) &_fill)))
		{
			_table[i] = (txObject*) &_fill;
			_entries--;
			_fill++;
		}

		if (_entries*8 < _size)
		{
			resize(_entries*2);
		}
	}

	return p;
}

void txHashSet::removeAndDestroy (const txObject* x)
{
	register txObject* p = remove(x);

	delete p; p = 0;
}

void txHashSet::clear (void)
{
	if (!_table) return;

	register txObject** table = _table;

	_table = 0; _size = _entries = _fill = 0;

	delete [] table; table = 0;
}

void txHashSet::clearAndDestroy (void)
{
	if (!_table) return;

	register int i, n = _size;
	register txObject** table = _table;

	_table = 0; _size = _entries = _fill = 0;

	for (i = 0; i < n; i++)
	{
		txObject* p = table[i];

		if (p != 0 && (p != ((txObject*) &_fill)))
		{
			delete p; p = 0;
		}
	}

	delete [] table; table = 0;
}

void txHashSet::readObject (txIn& in)
{
	const char* end = in.cursor() + in.objectLength();

	signed short int flag; in >> flag;

	TX_SET_AUTODEL_FLAG(_flags_, (TX_AUTODEL_FLAG) flag);

	txObject* o;

	while (in.cursor() < end)
	{
		in >> o;

		insert(o);
	}
}

void txHashSet::writeObject (txOut& out) const
{
	txObject* node;
	unsigned int hdr = out.length();

	out.putNull();

	out << (signed short int) TX_GET_AUTODEL_FLAG(_flags_);

	txHashSetIterator iter((txHashSet&) *this);

	while (node = (txObject*) iter.next())
	{
		out.put(node);
	}

	out.writeHeader(hdr, out.length() - (hdr + 4), TX_SET);
}

txObject* txHashSetIterator::remove (void)
{
	register txObject* p = 0;

	if (_i_ >= 0 && _i_ < _t_->buckets())
	{
		p = _t_->_table[_i_];

		_t_->_table[_i_] = (txObject*) &_t_->_fill;

		if (p != 0 && (p != ((txObject*) &_t_->_fill))) // for safety
		{
			_t_->_entries--;
			_t_->_fill++;
		}
	}

	return p;
}

const txObject* txHashSetIterator::next (void)
{
	txObject* p;

	while ((++_i_) < _t_->buckets() && (!(p = _t_->_table[_i_]) ||
		(p == ((txObject*) &_t_->_fill))));

	if (_i_ >= _t_->buckets()) p = 0;

	return p;
}

const txObject* txHashSetIterator::key (void) const
{
	register txObject* p = 0;

	if (_i_ >= 0 && _i_ < _t_->buckets())
	{
		p = _t_->_table[_i_];
	}

	return p;
}

