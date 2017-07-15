///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXCHAR16_H__ )
#define __TXCHAR16_H__

#include "txobject.h"

class txChar16 : public txObject
{
	TX_DEFINE_TYPE(txChar16)

	private:
		signed short _char_;

	public:
		txChar16 (void);
		txChar16 (signed short v);

		txChar16 (const txChar16& obj);

		txChar16& operator= (txChar16& obj);

		~txChar16 (void);

		signed short value (void) const
		{
			return _char_;
		}

		void value (signed char v)
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

TX_DECLARE_STREAM_TYPE_OPER(txChar16)

#endif // __TXCHAR16_H__
