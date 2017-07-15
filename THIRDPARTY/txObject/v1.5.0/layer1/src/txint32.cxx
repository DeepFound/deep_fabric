///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <sys/types.h>
	#include <netinet/in.h>
#else
	#include <winsock2.h>
#endif

#include "txint32.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txInt32,txObject)

txInt32::txInt32 (void) :
	_int_(0)
{
}

txInt32::txInt32 (signed long v) :
	_int_(v)
{
}

txInt32::txInt32 (const txInt32& obj)
{
	_int_ = obj._int_;
}

txInt32& txInt32::operator= (txInt32& obj)
{
	if (this != &obj)
	{
		obj._int_ = obj._int_;
	}

	return *this;
}

txInt32::~txInt32 (void)
{
}

int txInt32::equals (const txObject* obj) const
{
	return obj->isClass(txInt32::Type) ?
		(_int_ == ((txInt32*) obj)->_int_) : 0;
}

int txInt32::compare (const txObject* obj) const
{
	if (_int_ == ((txInt32*) obj)->_int_)
	{
		return 0;
	}
	else if (_int_ > ((txInt32*) obj)->_int_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txInt32::readObject (txIn& in)
{
	_int_ = ntohl(*(unsigned long*) (in.cursor()));
}

void txInt32::writeObject (txOut& out) const
{
	out << _int_;
}

