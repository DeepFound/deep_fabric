///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXUINT16_H__ )
#define __TXUINT16_H__

#include "txobject.h"

class txUInt16 : public txObject
{
	TX_DEFINE_TYPE(txUInt16)

	private:
		unsigned short _int_;

	public:
		txUInt16 (void);
		txUInt16 (unsigned short v);

		txUInt16 (const txUInt16& obj);

		txUInt16& operator= (txUInt16& obj);

		~txUInt16 (void);

		unsigned short value (void) const
		{
			return _int_;
		}

		void value (unsigned short v)
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

TX_DECLARE_STREAM_TYPE_OPER(txUInt16)

#endif // __TXUINT16_H__
