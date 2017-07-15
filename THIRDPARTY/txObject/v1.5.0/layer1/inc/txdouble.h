///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXDOUBLE_H__ )
#define __TXDOUBLE_H__

#include "txobject.h"

class txDouble : public txObject
{
	TX_DEFINE_TYPE(txDouble)

	private:
		double _double_;

	public:
		txDouble (void);
		txDouble (double v);

		txDouble (const txDouble& obj);

		txDouble& operator= (txDouble& obj);

		~txDouble (void);

		double value (void) const
		{
			return _double_;
		}

		void value (double v)
		{
			_double_ = v;
		}

		unsigned hash (void) const
		{
			return (unsigned) _double_;
		}

		int equals (const txObject* obj) const;

		int compare (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txDouble)

#endif // __TXDOUBLE_H__
