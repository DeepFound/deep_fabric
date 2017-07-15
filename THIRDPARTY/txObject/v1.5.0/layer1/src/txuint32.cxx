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

#include "txstring.h"
#include "txuint32.h"

TX_DEFINE_PARENT_TYPE(txUInt32,txObject)

txUInt32::txUInt32 (void) :
	_int_(0)
{
}

txUInt32::txUInt32 (unsigned long v) :
	_int_(v)
{
}

txUInt32::txUInt32 (const txUInt32& obj)
{
	_int_ = obj._int_;
}

txUInt32& txUInt32::operator= (txUInt32& obj)
{
	if (this != &obj)
	{
		obj._int_ = obj._int_;
	}

	return *this;
}

txUInt32::~txUInt32 (void)
{
}

int txUInt32::equals (const txObject* obj) const
{
	return obj->isClass(txUInt32::Type) ?
		(_int_ == ((txUInt32*) obj)->_int_) : 0;
}

int txUInt32::compare (const txObject* obj) const
{
	if (_int_ == ((txUInt32*) obj)->_int_)
	{
		return 0;
	}
	else if (_int_ > ((txUInt32*) obj)->_int_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txUInt32::readObject (txIn& in)
{
	_int_ = ntohl(*(unsigned long*) (in.cursor()));
}

void txUInt32::writeObject (txOut& out) const
{
	out << _int_;
}

