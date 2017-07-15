///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXUINT8_H__ )
#define __TXUINT8_H__

#include "txobject.h"

class txUInt8 : public txObject
{
	TX_DEFINE_TYPE(txUInt8)

	private:
		unsigned char _int_;

	public:
		txUInt8 (void);
		txUInt8 (unsigned char v);

		txUInt8 (const txUInt8& obj);

		txUInt8& operator= (txUInt8& obj);

		~txUInt8 (void);

		unsigned char value (void) const
		{
			return _int_;
		}

		void value (unsigned char v)
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

TX_DECLARE_STREAM_TYPE_OPER(txUInt8)

#endif // __TXUINT8_H__
