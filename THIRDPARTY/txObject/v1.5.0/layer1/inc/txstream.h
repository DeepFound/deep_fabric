///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXSTREAM_H__ )
#define __TXSTREAM_H__

#include "txobject.h"
#include "txfactory.h"

class txStream : public txObject 
{
	TX_DECLARE_STREAM_TYPE(txStream)

	public: 
		txStream (void);
		~txStream (void);

		virtual void restoreInners (txIn& in);
		virtual void storeInners (txOut& out) const;

		virtual void readObject (txIn& in);
		virtual void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txStream)

#endif // __TXSTREAM_H__
