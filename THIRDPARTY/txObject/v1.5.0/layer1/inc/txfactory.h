///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXFACTORY_H__ )
#define __TXFACTORY_H__

#include "txobject.h"

class txIn;
class txString;
class txStream;
class txHashMap;

typedef txStream* (*TX_STREAM_FUNC) (void);
typedef txStream* (*TX_EXCEPT_FUNC) (const txObject&, txIn&);

class txFactory
{
	private:
		static txHashMap* _factory_;
		static txHashMap* _excepts_;

	public:
		static void registerType (const txObject&, TX_STREAM_FUNC);

		static void unregisterType (const txObject&);

		static void registerException (const txObject&, TX_EXCEPT_FUNC);

		static void unregisterException (const txObject&);

		static txStream* create (const txObject&, int except = 0);

		static txStream* create (const txObject&, txIn&, int except=0);

		static const txString* findTypeInFactory (const txObject&);

		static const txString* findTypeInException (const txObject&);

		static void clear (void);
};

class txFactoryBinder
{
	public:
		txFactoryBinder (const txObject& type, TX_STREAM_FUNC func);
};

#endif // __TXFACTORY_H__
