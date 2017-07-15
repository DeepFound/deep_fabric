///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXUINT32_H__ )
#define __TXUINT32_H__

#include "txobject.h"

class txUInt32 : public txObject
{
	TX_DEFINE_TYPE(txUInt32)

	private:
		unsigned long _int_;

	public:
		txUInt32 (void);
		txUInt32 (unsigned long v);

		txUInt32 (const txUInt32& obj);

		txUInt32& operator= (txUInt32& obj);

		~txUInt32 (void);

		unsigned long value (void) const
		{
			return _int_;
		}

		void value (unsigned long v)
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

TX_DECLARE_STREAM_TYPE_OPER(txUInt32)

#endif // __TXUINT32_H__
