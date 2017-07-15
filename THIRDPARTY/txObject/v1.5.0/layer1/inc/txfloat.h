///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXFLOAT_H__ )
#define __TXFLOAT_H__

#include "txobject.h"

class txFloat : public txObject
{
	TX_DEFINE_TYPE(txFloat)

	private:
		float _float_;

	public:
		txFloat (void);
		txFloat (float v);

		txFloat (const txFloat& obj);

		txFloat& operator= (txFloat& obj);

		~txFloat (void);

		float value (void) const
		{
			return _float_;
		}

		void value (float v)
		{
			_float_ = v;
		}

		unsigned hash (void) const
		{
			return (unsigned) _float_;
		}

		int equals (const txObject* obj) const;

		int compare (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txFloat)

#endif // __TXFLOAT_H__
