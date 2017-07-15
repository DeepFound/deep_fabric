///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXCLASSB_H__ )
#define __TXCLASSB_H__

#include "txclassa.h"

class txClassB : public txStream
{
	TX_DECLARE_STREAM_TYPE(txClassB)

	private:
		txClassA _generic_;

	public:
		txClassB (void);
		txClassB (const char* data, int value);
		~txClassB (void);

		const char* data (void) const;
		int value (void) const;

		void storeInners (txOut& out) const;
		void restoreInners (txIn& in);
};

TX_DECLARE_STREAM_TYPE_OPER(txClassB)

#endif // __TXCLASSB_H__
