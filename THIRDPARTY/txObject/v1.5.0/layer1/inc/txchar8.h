///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXCHAR8_H__ )
#define __TXCHAR8_H__

#include "txobject.h"

class txChar8 : public txObject
{
	TX_DEFINE_TYPE(txChar8)

	private:
		signed char _char_;

	public:
		txChar8 (void);
		txChar8 (signed char v);

		txChar8 (const txChar8& obj);

		txChar8& operator= (txChar8& obj);

		~txChar8 (void);

		signed char value (void) const
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

TX_DECLARE_STREAM_TYPE_OPER(txChar8)

#endif // __TXCHAR8_H__
