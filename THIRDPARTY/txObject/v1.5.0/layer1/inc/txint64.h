///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXINT64_H__ )
#define __TXINT64_H__

#include "txobject.h"

class txInt64 : public txObject
{
	TX_DEFINE_TYPE(txInt64)

	private:
		signed long _hi_;
		signed long _lo_;

	public:
		txInt64 (void);
		txInt64 (signed long hi, signed long lo);

		txInt64 (const txInt64& obj);

		txInt64& operator= (txInt64& obj);

		~txInt64 (void);

		signed long hiValue (void) const
		{
			return _hi_;
		}

		signed long loValue (void) const
		{
			return _lo_;
		}

		void hiValue (signed long hi)
		{
			_hi_ = hi;
		}

		void loValue (signed long lo)
		{
			_lo_ = lo;
		}

		unsigned hash (void) const
		{
			return (unsigned) _hi_ + _lo_;
		}

		int equals (const txObject* obj) const;

		int compare (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txInt64)

#endif // __TXINT64_H__
