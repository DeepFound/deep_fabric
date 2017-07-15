///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if !defined ( __TXHASHMAP_H__ )
#define __TXHASHMAP_H__
 
#include "txobject.h"
#include "txhashset.h"

class txMapEntry : public txObject
{
	TX_PERSIST

	private:
		txObject* _key_;
		txObject* _value_;

	public:
		txMapEntry (txObject* key, txObject* value);

		~txMapEntry (void);

		txObject* key (txObject* key)
		{
			register txObject* t = _key_;
			_key_ = key;
			return t;
		}

		txObject* key (void) const
		{
			return _key_;
		}

		txObject* value (txObject* value)
		{
			register txObject* t = _value_;
			_value_ = value;
			return t;
		}

		txObject* value (void) const
		{
			return _value_;
		}

		unsigned hash (void) const
		{
			return _key_->hash();
		}

		int equals (const txObject* obj) const
		{
			return obj->equals(_key_);
		}
};

class txHashMap : public txHashSet
{
	TX_DEFINE_TYPE(txHashMap)

	private:
		unsigned long _flags_;

	public:
		txHashMap (
			int n = 64,
			TX_AUTODEL_FLAG autodel = TX_AUTODEL_OFF,
			TX_COMPARE_FLAG compare = TX_COMPARE_V2K) :

			/*
			** NOTE: Auto-Delete DOES NOT WORK for derived class
			*/
			txHashSet(n, TX_AUTODEL_OFF, compare)
		{
			TX_SET_AUTODEL_FLAG(_flags_, autodel);

			/*
			** NOTE: Compare IS CURRENTLY DEFINED BY parent class
			** TX_SET_COMPARE_FLAG(_flags_, compare);
			*/
		}

		~txHashMap (void)
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

		virtual const txObject* insert (txObject* k)
		{
			return insertKeyAndValue(k, 0);
		}

		virtual const txObject* find (const txObject* key) const;

		const txObject* findKeyAndValue (const txObject* key,
			txObject*& value) const;

		const txObject* findValue (const txObject* k) const;

		txObject* findValue (const txObject* k, txObject* v) const;

		const txObject* insertKeyAndValue (txObject* k, txObject* v);

		virtual txObject* remove (const txObject* k);

		txObject* removeKeyAndValue (const txObject* k, txObject*& v);

		virtual void removeAndDestroy (const txObject* k);

		virtual void clear (void);

		virtual void clearAndDestroy (void);

		void readObject (txIn& in);
		void writeObject (txOut& out) const;

	friend class txHashMapIterator;
};

TX_DECLARE_STREAM_TYPE_OPER(txHashMap)

class txHashMapIterator : private txHashSetIterator
{
	TX_PERSIST

	public:
		txHashMapIterator (txHashMap& hm) :
			txHashSetIterator(hm)
		{
		}

		~txHashMapIterator (void)
		{
		}

		virtual txObject* remove (void);

		virtual const txObject* next (void);

		const txObject* value (void) const;

		const txObject* key (void) const;

		void reset (txHashMap& hm);

		virtual void reset (void);
};

#endif // __TXHASHMAP_H__
