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

#include "txint8.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txInt8,txObject)

txInt8::txInt8 (void) :
	_int_(0)
{
}

txInt8::txInt8 (signed char v) :
	_int_(v)
{
}

txInt8::txInt8 (const txInt8& obj)
{
	_int_ = obj._int_;
}

txInt8& txInt8::operator= (txInt8& obj)
{
	if (this != &obj)
	{
		obj._int_ = obj._int_;
	}

	return *this;
}

txInt8::~txInt8 (void)
{
}

int txInt8::equals (const txObject* obj) const
{
	return obj->isClass(txInt8::Type) ?
		(_int_ == ((txInt8*) obj)->_int_) : 0;
}

int txInt8::compare (const txObject* obj) const
{
	if (_int_ == ((txInt8*) obj)->_int_)
	{
		return 0;
	}
	else if (_int_ > ((txInt8*) obj)->_int_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txInt8::readObject (txIn& in)
{
	_int_ = *(in.cursor() + 3);
}

void txInt8::writeObject (txOut& out) const
{
	out.writeRawHeader(htonl(TX_SIGNED_INT8));
	out.append("\0\0\0", 3);
	out.append((char*) &_int_, 1);
}

