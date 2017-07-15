///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXINT32_H__ )
#define __TXINT32_H__

#include "txobject.h"

class txInt32 : public txObject
{
	TX_DEFINE_TYPE(txInt32)

	private:
		signed long _int_;

	public:
		txInt32 (void);
		txInt32 (signed long v);

		txInt32 (const txInt32& obj);

		txInt32& operator= (txInt32& obj);

		~txInt32 (void);

		signed long value (void) const
		{
			return _int_;
		}

		void value (signed long v)
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

TX_DECLARE_STREAM_TYPE_OPER(txInt32)

#endif // __TXINT32_H__
