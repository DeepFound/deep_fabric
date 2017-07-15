///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXOBJREF_H__ )
#define __TXOBJREF_H__

#include "txobject.h"

class txObjRef : public txObject 
{
	TX_DEFINE_TYPE(txObjRef)

	private:
		unsigned long int _ref_;

	public: 
		txObjRef (unsigned long int ref);

		~txObjRef (void);

		unsigned long int ref (void) const
		{
			return _ref_;
		}

		unsigned hash (void) const
		{
			return (unsigned) _ref_;
		}

		int equals (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
}; 

TX_DECLARE_STREAM_TYPE_OPER(txObjRef)

#endif // __TXOBJREF_H__
