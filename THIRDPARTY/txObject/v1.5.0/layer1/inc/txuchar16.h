///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXUCHAR16_H__ )
#define __TXUCHAR16_H__

#include "txobject.h"

class txUChar16 : public txObject
{
	TX_DEFINE_TYPE(txUChar16)

	private:
		unsigned short _char_;

	public:
		txUChar16 (void);
		txUChar16 (unsigned short v);

		txUChar16 (const txUChar16& obj);

		txUChar16& operator= (txUChar16& obj);

		~txUChar16 (void);

		unsigned short value (void) const
		{
			return _char_;
		}

		void value (unsigned short v)
		{
			_char_ = v;
		}

		unsigned hash (void) const
		{
			return (unsigned) _char_;
		}

		int equals (const txObject* obj) const;

		int compare (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txUChar16)

#endif // __TXUCHAR16_H__
