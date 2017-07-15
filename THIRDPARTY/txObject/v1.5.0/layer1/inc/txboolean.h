///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXBOOLEAN_H__ )
#define __TXBOOLEAN_H__

#include "txobject.h"

class txBoolean : public txObject
{
	TX_DEFINE_TYPE(txBoolean)

	private:
		txObjectType::Boolean _bool_;

	public:
		txBoolean (void);
		txBoolean (txObjectType::Boolean v);

		txBoolean (const txBoolean& obj);

		txBoolean& operator= (txBoolean& obj);

		~txBoolean (void);

		txObjectType::Boolean value (void) const
		{
			return _bool_;
		}

		void value (txObjectType::Boolean v)
		{
			_bool_ = v;
		}

		unsigned hash (void) const
		{
			return (unsigned) _bool_;
		}

		int equals (const txObject* obj) const;

		int compare (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txBoolean)

#endif // __TXBOOLEAN_H__
