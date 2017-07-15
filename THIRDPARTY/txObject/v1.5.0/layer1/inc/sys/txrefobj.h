///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXREFOBJ_H__ )
#define __TXREFOBJ_H__

#include "txobject.h"

class txRefObj: public txObject 
{
	TX_DEFINE_TYPE(txRefObj)

	private:
		unsigned long int _ref_;
		const txObject* _object_;

	public: 
		txRefObj (unsigned long int ref, const txObject* obj);

		~txRefObj (void);

		unsigned long int ref (void) const
		{
			return _ref_;
		}

		const txObject* object (void) const
		{
			return _object_;
		}

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txRefObj)

#endif // __TXREFOBJ_H__
