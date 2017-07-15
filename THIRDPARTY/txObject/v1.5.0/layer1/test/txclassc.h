///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXCLASSC_H__ )
#define __TXCLASSC_H__

#include "txlist.h"
#include "txclassb.h"

class txClassC : public txClassB
{
	TX_DECLARE_STREAM_TYPE(txClassC)

	private:
		txList _list_;

	public:
		txClassC (void);
		txClassC (const char* data, int value);
		~txClassC (void);

		const txList* list (void) const;

		void storeInners (txOut& out) const;
		void restoreInners (txIn& in);
};

TX_DECLARE_STREAM_TYPE_OPER(txClassC)

#endif // __TCLASSC_H__
