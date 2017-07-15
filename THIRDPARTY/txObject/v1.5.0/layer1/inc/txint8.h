///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXINT8_H__ )
#define __TXINT8_H__

#include "txobject.h"

class txInt8 : public txObject
{
	TX_DEFINE_TYPE(txInt8)

	private:
		signed char _int_;

	public:
		txInt8 (void);
		txInt8 (signed char v);

		txInt8 (const txInt8& obj);

		txInt8& operator= (txInt8& obj);

		~txInt8 (void);

		signed char value (void) const
		{
			return _int_;
		}

		void value (signed char v)
		{
			_int_ = v;
		}

		unsigned hash (void) const
		{
			return (unsigned) _int_;
		}

		int equals (const txObject* obj) const;

		int compare (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txInt8)

#endif // __TXINT8_H__
