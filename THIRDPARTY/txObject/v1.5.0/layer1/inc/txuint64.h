///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXUINT64_H__ )
#define __TXUINT64_H__

#include "txobject.h"

class txUInt64 : public txObject
{
	TX_DEFINE_TYPE(txUInt64)

	private:
		unsigned long _hi_;
		unsigned long _lo_;

	public:
		txUInt64 (void);
		txUInt64 (unsigned long hi, unsigned long lo);

		txUInt64 (const txUInt64& obj);

		txUInt64& operator= (txUInt64& obj);

		~txUInt64 (void);

		unsigned long hiValue (void) const
		{
			return _hi_;
		}

		unsigned long loValue (void) const
		{
			return _lo_;
		}

		void hiValue (unsigned long hi)
		{
			_hi_ = hi;
		}

		void loValue (unsigned long lo)
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

TX_DECLARE_STREAM_TYPE_OPER(txUInt64)

#endif // __TXUINT64_H__
