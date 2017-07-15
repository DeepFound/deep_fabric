///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXCLASSA_H__ )
#define __TXCLASSA_H__

#include "txint32.h"
#include "txstring.h"
#include "txstream.h"

class txClassA : public txStream
{
	TX_DECLARE_STREAM_TYPE(txClassA)

	private:
		txInt32 _integer_;
		txString _string_;

	public:
		txClassA (void);
		txClassA (const char* data, int value);
		~txClassA (void);

		const char* data (void) const;
		int value (void) const;

		void storeInners (txOut& out) const;
		void restoreInners (txIn& in);
};

TX_DECLARE_STREAM_TYPE_OPER(txClassA)

#endif // __TXCLASSA_H__
