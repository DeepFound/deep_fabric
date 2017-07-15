///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXTHRDBASE_H__ )
#define __TXTHRDBASE_H__

#include "txobject.h"

class txThrdBase : public txObject
{
	protected:
		const char* _name;
		void* _native_obj;

		txThrdBase (const txThrdBase&);
		txThrdBase& operator= (const txThrdBase&);

	public:
		txThrdBase (const char* name = "default");
		~txThrdBase (void);

		unsigned long id (void) const
		{
			return (unsigned long) this;
		}

		const char* name (void) const
		{
			return _name;
		}

		void name (const char* name)
		{
			_name = name;
		}
};

#endif // __TXTHRDBASE_H__
