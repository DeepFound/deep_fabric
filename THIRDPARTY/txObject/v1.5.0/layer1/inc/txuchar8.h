///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXUCHAR8_H__ )
#define __TXUCHAR8_H__

#include "txobject.h"

class txUChar8 : public txObject
{
	TX_DEFINE_TYPE(txUChar8)

	private:
		unsigned char _char_;

	public:
		txUChar8 (void);
		txUChar8 (unsigned char v);

		txUChar8 (const txUChar8& obj);

		txUChar8& operator= (txUChar8& obj);

		~txUChar8 (void);

		unsigned char value (void) const
		{
			return _char_;
		}

		void value (unsigned char v)
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

TX_DECLARE_STREAM_TYPE_OPER(txUChar8)

#endif // __TXUCHAR8_H__
