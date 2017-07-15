///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXINT16_H__ )
#define __TXINT16_H__

#include "txobject.h"

class txInt16 : public txObject
{
	TX_DEFINE_TYPE(txInt16)

	private:
		signed short _int_;

	public:
		txInt16 (void);
		txInt16 (signed short v);

		txInt16 (const txInt16& obj);

		txInt16& operator= (txInt16& obj);

		~txInt16 (void);

		signed short value (void) const
		{
			return _int_;
		}

		void value (signed short v)
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

TX_DECLARE_STREAM_TYPE_OPER(txInt16)

#endif // __TXINT16_H__
