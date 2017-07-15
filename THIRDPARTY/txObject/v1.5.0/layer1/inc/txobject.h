///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if !defined ( __TXOBJECT_H__ )
#define __TXOBJECT_H__

#include "txin.h"
#include "txout.h"
#include "txutil.h"
#include "txtypecodes.h"
#include "sys/txtypecheckss.h"

class txObject
{
	TX_DEFINE_TYPE(txObject)

	public:
		txObject (void);

		virtual ~txObject (void);

		virtual unsigned hash (void) const
		{
			unsigned long address = (unsigned long) this;
			return (unsigned) address ^ ((unsigned) address >> 3);
		}

		virtual unsigned long id (void) const
		{
			return (unsigned long) hash();
		}

		virtual int equals (const txObject* obj) const
		{
			return (this == obj);
		}

		virtual int compare (const txObject* obj) const;

		int isClass (const txString& type) const
		{
			return txTypeCheckSS::isClass(classType(), type);
		}

		int isSubClass (const txString& type) const
		{
			return txTypeCheckSS::isSubClass(classType(), type);
		}

		int isSuperClass (const txString& type) const
		{
			return txTypeCheckSS::isSuperClass(type, classType());
		}

		virtual void readObject (txIn&)
		{
		}

		virtual void writeObject (txOut&) const
		{
		}
};

TX_DECLARE_STREAM_TYPE_OPER(txObject)

class txObjectSequence : public txObject
{
	TX_PERSIST

	public:
		virtual const txObject* insert (txObject*);

		virtual const txObject* find (const txObject*) const;

		virtual txObject* remove (const txObject*);

		virtual void removeAndDestroy (const txObject*);

		virtual void clearAndDestroy (void);

		virtual void clear (void);

		virtual int entries (void) const;
};

class txObjectIterator : public txObject
{
	TX_PERSIST

	public:
		virtual txObject* remove (void);

		virtual const txObject* next (void);

		virtual void reset (void);
};

#endif // __TXOBJECT_H__
