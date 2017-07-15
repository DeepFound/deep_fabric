///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "sys/txthrdbase.h"

txThrdBase::txThrdBase (const char* name) :
	_native_obj(0),
	_name(name)
{
}

txThrdBase::~txThrdBase (void)
{
	_native_obj = 0;
	_name = 0;
}

