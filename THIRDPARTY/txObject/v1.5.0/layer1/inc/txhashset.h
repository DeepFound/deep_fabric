///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXHASHTABLE_H__ )
#define __TXHASHTABLE_H__

#include "txobject.h"

class txHashSet : public txObjectSequence
{
	TX_DEFINE_TYPE(txHashSet)

	private:
		unsigned long _flags_;

	protected:
		long _poly;
		long _fill; // _fill's address is also used as the dummy value

		int _size;
		int _entries;
		txObject** _table;

	protected:
		int _index (const txObject* x, int find = 1) const;

		txObject* _insert (txObject* x, int index = 0);

	public:
		txHashSet (
			int n = 64,
			TX_AUTODEL_FLAG autodel = TX_AUTODEL_OFF,
			TX_COMPARE_FLAG compare = TX_COMPARE_V2K) :

			_poly(0), _fill(0), _size(0), _entries(0), _table(0)
		{
			TX_SET_AUTODEL_FLAG(_flags_, autodel);
			TX_SET_COMPARE_FLAG(_flags_, compare);

			if (n) resize(n);
		}

		~txHashSet (void)
		{
			if (TX_GET_AUTODEL_FLAG(_flags_))
			{
				clearAndDestroy();
			}
			else
			{
				clear();
			}
		}

		virtual const txObject* find (const txObject* x) const
		{
			return _table ? _table[_index(x)] : 0;
		}

		virtual const txObject* insert (txObject* x);

		virtual txObject* remove (const txObject* x);

		virtual void removeAndDestroy (const txObject* x);

		int resize (int n = 0);

		virtual void clear (void);

		virtual void clearAndDestroy (void);

		int entries (void) const
		{
			return _entries;
		}

		int buckets (void) const
		{
			return _size;
		}

		void readObject (txIn& in);
		void writeObject (txOut& out) const;

	friend class txHashSetIterator;
};

TX_DECLARE_STREAM_TYPE_OPER(txHashSet)

class txHashSetIterator : public txObjectIterator
{
	TX_PERSIST

	private:
		int _i_;
		txHashSet* _t_;

	public:
		txHashSetIterator (txHashSet& h) :
			_t_(&h), _i_(-1)
		{
		}

		~txHashSetIterator (void)
		{
		}

		virtual txObject* remove (void);

		virtual const txObject* next (void);

		virtual const txObject* key (void) const;

		void reset (txHashSet& h)
		{
			_t_ = &h;

			reset();
		}

		virtual void reset (void)
		{
			_i_ = -1;
		}
};

#endif // __TXHASHTABLE_H__
