///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXGOBJECT_H__ )
#define __TXGOBJECT_H__

#include "txlist.h"
#include "txstream.h"
#include "txuint32.h"

class txGObject : public txStream 
{
	TX_DECLARE_STREAM_TYPE(txGObject)

	private:
		txList _attrs_;
		txUInt32 _type_;
		unsigned long _length_;

	public: 
		txGObject (void);

		txGObject (const txUInt32& type, unsigned long length);

		~txGObject (void);

		const txList* attrs (void) const
		{
			return &_attrs_;
		}

		unsigned long type (void) const
		{
			return _type_.value();
		}

		const txObject* get (const txObject& k) const
		{
			return _attrs_.find(&((txObject&) k));
		}

		void restoreInners (txIn& in);
		void storeInners (txOut& out) const;

		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txGObject)

#endif // __TXGOBJECT_H__
